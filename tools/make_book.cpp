// S172. Builds a Polyglot opening book from a PGN, and reads one back.
//
//   make_book build <pgn> --out <bin> [--max-ply N] [--min-games N]
//                   [--allow-cut-short]
//   make_book dump  <bin> [--top N]
//
// Until this existed nothing in the repository could produce the book the
// engine ships with, which is half of why that book has no recorded origin
// (S146). The engine's own parser reads the moves and the engine's own
// get_key() keys the positions, deliberately: a book built against a second
// implementation of "the same position" is a book the engine cannot probe, and
// that failure is silent -- it looks like a book with no entries in it.
//
// `dump` runs the same two checks the engine's loader runs, so a book this
// accepts is a book the engine will load.
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "bitboard.hpp"
#include "openings.hpp"
#include "uci.hpp"  // trim_whitespace
#include "utils.hpp"


#define POLYGLOT_ENTRY_BYTES 16


namespace
{

game_t game;


//-###############################  THE PGN  ################################-//

enum class outcome_t
{
  WHITE_WIN,
  BLACK_WIN,
  DRAW,
  UNKNOWN
};


struct pgn_game_t
{
  std::string fen;  // Empty means the standard opening position.
  std::string movetext;
  outcome_t outcome = outcome_t::UNKNOWN;
};


outcome_t outcome_from_string(const std::string& text)
{
  if (text == "1-0") { return outcome_t::WHITE_WIN; }
  if (text == "0-1") { return outcome_t::BLACK_WIN; }
  if (text == "1/2-1/2") { return outcome_t::DRAW; }

  return outcome_t::UNKNOWN;
}


// One movetext token, twice: what the parser should be handed, and what the
// PGN wrote. They differ when a move number indication was glued to the move,
// and a cut-short message quotes the second -- `2.Qxf7` names the one line that
// holds it where `Qxf7` names every line that plays the move somewhere. S200.
struct san_token_t
{
  std::string move;
  std::string as_written;
};


// Splits movetext into SAN tokens, dropping everything PGN allows to sit
// between them: `{...}` comments (which may span lines), `;` line comments,
// `(...)` variations at any nesting depth, `$12` numeric annotation glyphs,
// move number indications -- alone, glued to the move they introduce, or with
// white space between the digits and the dots, all of which is import format,
// PGN 8.2.2.1 -- and the result token. Variations are dropped rather than
// followed: a book is built from what was played, not from what was analysed.
std::vector<san_token_t> movetext_to_san(const std::string& movetext)
{
  std::vector<san_token_t> tokens;

  size_t i = 0;
  int variation_depth = 0;

  while (i < movetext.size()) {
    const char c = movetext[i];

    if (c == '{') {
      const size_t close = movetext.find('}', i);
      i = (close == std::string::npos) ? movetext.size() : close + 1;
      continue;
    }

    if (c == ';') {
      const size_t eol = movetext.find('\n', i);
      i = (eol == std::string::npos) ? movetext.size() : eol + 1;
      continue;
    }

    if (c == '(') {
      ++variation_depth;
      ++i;
      continue;
    }

    if (c == ')') {
      if (variation_depth > 0) { --variation_depth; }
      ++i;
      continue;
    }

    if (isspace(static_cast<unsigned char>(c)) != 0) {
      ++i;
      continue;
    }

    size_t end = i;
    while (end < movetext.size() &&
           isspace(static_cast<unsigned char>(movetext[end])) == 0 &&
           movetext[end] != '{' && movetext[end] != '(' &&
           movetext[end] != ')' && movetext[end] != ';') {
      ++end;
    }

    std::string token = movetext.substr(i, end - i);
    i = end;

    if (variation_depth > 0) { continue; }
    if (token.empty() || token[0] == '$') { continue; }
    if (token == "*" || outcome_from_string(token) != outcome_t::UNKNOWN) {
      continue;
    }

    // The token as the PGN wrote it, taken before the indication is stripped
    // off it: the cut-short message quotes this one.
    const std::string as_written = token;

    // A move number indication: digits, then one or more dots. It stands alone
    // in export format ("1.", "1..."), and import format lets it be glued to
    // the move it introduces ("1.e4", "2...Nc6"), which is split off here.
    // 8.2.2.1 also allows white space between the digits and the dots, so the
    // dots arrive with no digits in front ("1 . e4", "1 .e4", "1. ... e5") and
    // the same rule applies from the front of the token -- nothing legal begins
    // with a period, which 7.3 makes a token by itself and 7.9 excludes from a
    // symbol's continuation characters. The dot test keeps "1-0" and "1/2-1/2"
    // out, though the result test above has already dropped them.
    const size_t dot = token.find_first_not_of("0123456789");
    if (dot == std::string::npos) { continue; }  // a bare number
    if (token[dot] == '.') {
      const size_t rest = token.find_first_not_of('.', dot);
      if (rest == std::string::npos) { continue; }  // the indication alone
      token.erase(0, rest);
    }

    tokens.push_back({token, as_written});
  }

  return tokens;
}


// One game per call, or false at end of file. A game ends at its result token,
// or at the first tag line of the next game for a PGN that carries none.
bool read_game(std::istream& input, std::string* pending_line, pgn_game_t* out)
{
  assert(pending_line != nullptr);
  assert(out != nullptr);

  *out = pgn_game_t{};

  std::string line;
  bool in_tags = true;
  bool seen_anything = false;

  while (true) {
    if (!pending_line->empty()) {
      line = *pending_line;
      pending_line->clear();
    } else if (!std::getline(input, line)) {
      break;
    }

    const std::string trimmed = trim_whitespace(line);

    if (!trimmed.empty() && trimmed[0] == '[') {
      if (!in_tags) {
        // The next game's tags. Hand the line back and finish this one.
        *pending_line = line;
        break;
      }

      seen_anything = true;

      const size_t open_quote = trimmed.find('"');
      const size_t close_quote = trimmed.rfind('"');

      if (open_quote == std::string::npos || close_quote <= open_quote) {
        continue;
      }

      const std::string name =
          trim_whitespace(trimmed.substr(1, open_quote - 1));
      const std::string value =
          trimmed.substr(open_quote + 1, close_quote - open_quote - 1);

      if (name == "Result") { out->outcome = outcome_from_string(value); }
      if (name == "FEN") { out->fen = value; }

      continue;
    }

    if (trimmed.empty()) {
      if (in_tags || out->movetext.empty()) { continue; }
      break;
    }

    in_tags = false;
    seen_anything = true;
    out->movetext += trimmed;
    out->movetext += "\n";

    // The result token ends the movetext wherever the Result tag said nothing.
    for (const char* result : {"1-0", "0-1", "1/2-1/2", "*"}) {
      if (trimmed.size() >= strlen(result) &&
          trimmed.compare(trimmed.size() - strlen(result), strlen(result),
                          result) == 0) {
        if (out->outcome == outcome_t::UNKNOWN) {
          out->outcome = outcome_from_string(result);
        }

        return true;
      }
    }
  }

  return seen_anything;
}


//-#############################  THE BOOK  #################################-//

// The Polyglot move word: to file and row in the low six bits, from file and
// row above them, the promotion piece on top. Castling is written as the king
// taking its own rook, which is the format's one irregularity and the reason
// the engine's reader has fix_weirdo_castling().
uint16_t move_to_polyglot(move_t move)
{
  index_t from = MOVE_FROM(move);
  index_t to = MOVE_TO(move);

  if (MOVE_CASTLING(move) != 0) {
    if (from == e1 && to == g1) {
      to = h1;
    } else if (from == e1 && to == c1) {
      to = a1;
    } else if (from == e8 && to == g8) {
      to = h8;
    } else if (from == e8 && to == c8) {
      to = a8;
    }
  }

  const position_t from_position = index_to_position(from);
  const position_t to_position = index_to_position(to);

  // promotion_t is 0 for none and 1 to 4 for knight through queen, which is the
  // Polyglot encoding already. data_structures.hpp pins the order.
  const uint16_t promotion = static_cast<uint16_t>(MOVE_PROMOTED(move));

  return static_cast<uint16_t>(to_position.file) |
         static_cast<uint16_t>(to_position.rank << 3) |
         static_cast<uint16_t>(from_position.file << 6) |
         static_cast<uint16_t>(from_position.rank << 9) |
         static_cast<uint16_t>(promotion << 12);
}


struct entry_stat_t
{
  uint64_t weight = 0;
  uint32_t games = 0;
};


struct build_options_t
{
  std::string pgn;
  std::string out;
  int max_ply = 16;
  uint32_t min_games = 1;
  // S174. A game the parser could not follow to the end is a refusal by
  // default: the positions after the bad token are unknown, and a count that
  // is only reported is a count nobody reads. The flag turns it back into the
  // report, for a collection where a few broken games are expected.
  bool allow_cut_short = false;
};


// Says which game was cut and where, so the PGN can be fixed or the flag
// passed knowingly. Capped: a collection with thousands of broken games is
// answered by the count, not by a screen per game.
void report_cut_short(uint64_t rejected_so_far,
                      uint64_t game_number,
                      const char* what)
{
  constexpr uint64_t REPORT_LIMIT = 20;

  if (rejected_so_far <= REPORT_LIMIT) {
    fprintf(stderr, "game %llu cut short: %s\n",
            (unsigned long long)game_number, what);
  } else if (rejected_so_far == REPORT_LIMIT + 1) {
    fprintf(stderr, "further cut-short games not listed\n");
  }
}


// Two for a win, one for a draw, nothing for a loss, all from the moving side's
// point of view. It is the simplest scheme that makes weight mean "how much the
// games liked this move", which is what the format's weight is for and what the
// engine's weighted draw reads.
uint64_t weight_for(outcome_t outcome, color_t mover)
{
  switch (outcome) {
    case outcome_t::DRAW:
      return 1;
    case outcome_t::WHITE_WIN:
      return (mover == WHITE) ? 2 : 0;
    case outcome_t::BLACK_WIN:
      return (mover == BLACK) ? 2 : 0;
    case outcome_t::UNKNOWN:
    default:
      // An unfinished or unmarked game still says the line was played.
      return 1;
  }
}


void write_be(std::vector<uint8_t>& out, uint64_t value, size_t bytes)
{
  for (size_t i = 0; i < bytes; ++i) {
    out.push_back(
        static_cast<uint8_t>((value >> ((bytes - 1 - i) * 8)) & 0xFF));
  }
}


int build(const build_options_t& options)
{
  std::ifstream input(options.pgn);

  if (input.fail()) {
    fprintf(stderr, "cannot open '%s'\n", options.pgn.c_str());
    return 1;
  }

  std::unordered_map<uint64_t, std::unordered_map<uint16_t, entry_stat_t>> book;

  std::string pending;
  pgn_game_t pgn_game;

  uint64_t games = 0;
  uint64_t rejected_games = 0;
  uint64_t plies = 0;

  while (read_game(input, &pending, &pgn_game)) {
    if (pgn_game.movetext.empty()) { continue; }

    const std::string& start =
        pgn_game.fen.empty() ? std::string(DEFAULT_POSITION) : pgn_game.fen;

    if (!load_FEN(start, &game)) {
      ++rejected_games;
      report_cut_short(rejected_games, games + 1, "the FEN tag does not load");
      ++games;
      continue;
    }

    const std::vector<san_token_t> tokens = movetext_to_san(pgn_game.movetext);

    int ply = 0;

    for (const san_token_t& token : tokens) {
      if (ply >= options.max_ply) { break; }

      const color_t mover = game.board.active_color;
      const uint64_t key = get_key(&game.board);
      const move_t move = algebraic_to_move(token.move, &game);

      if (move == 0 || !make_move(&game, move)) {
        // One unparseable token poisons every position after it, so the game is
        // dropped from that point rather than resynchronised at a wrong board.
        // S174: before it, a token the parser could not read came back as a
        // fabricated move in Release, so this branch never ran and the board
        // was rewritten instead.
        ++rejected_games;
        report_cut_short(
            rejected_games, games + 1,
            (std::string(move == 0 ? "cannot parse '" : "cannot play '") +
             token.as_written + "' at ply " + std::to_string(ply))
                .c_str());
        break;
      }

      entry_stat_t& stat = book[key][move_to_polyglot(move)];
      stat.weight += weight_for(pgn_game.outcome, mover);
      ++stat.games;

      ++ply;
      ++plies;
    }

    ++games;
  }

  // S174. The gate. Until now this was a line in the report, and a report of
  // "0 games cut short" from a parser that could not fail was not evidence of
  // anything; now a cut-short game refuses the build unless the caller has said
  // that is acceptable, and the refusal happens before the output is touched.
  if (rejected_games > 0 && !options.allow_cut_short) {
    fprintf(stderr,
            "%llu game(s) cut short -- book not written. Every position after "
            "an unreadable token is unknown; fix the PGN, or pass "
            "--allow-cut-short to drop each such game from that point and "
            "build from the rest.\n",
            (unsigned long long)rejected_games);
    return 1;
  }

  // Sorted by key, which the format requires and the engine's binary search
  // depends on. Within a key, heaviest first: the order does not affect which
  // moves are found, and it makes a dump readable.
  std::vector<uint64_t> keys;
  keys.reserve(book.size());
  for (const auto& it : book) {
    keys.push_back(it.first);
  }
  std::sort(keys.begin(), keys.end());

  std::vector<uint8_t> bytes;
  uint64_t written = 0;
  uint64_t dropped_min_games = 0;
  uint64_t dropped_zero_weight = 0;
  uint64_t clamped = 0;

  for (const uint64_t key : keys) {
    std::vector<std::pair<uint16_t, entry_stat_t>> moves(book[key].begin(),
                                                         book[key].end());

    std::sort(moves.begin(), moves.end(), [](const auto& a, const auto& b) {
      if (a.second.weight != b.second.weight) {
        return a.second.weight > b.second.weight;
      }
      return a.first < b.first;
    });

    for (const auto& entry : moves) {
      if (entry.second.games < options.min_games) {
        ++dropped_min_games;
        continue;
      }

      if (entry.second.weight == 0) {
        // A move that only ever lost. The engine's weighted draw can never pick
        // it while anything else is on offer, so writing it would only make the
        // book bigger.
        ++dropped_zero_weight;
        continue;
      }

      uint64_t weight = entry.second.weight;

      if (weight > UINT16_MAX) {
        weight = UINT16_MAX;
        ++clamped;
      }

      write_be(bytes, key, 8);
      write_be(bytes, entry.first, 2);
      write_be(bytes, weight, 2);
      write_be(bytes, 0, 4);  // learn, which this engine does not use
      ++written;
    }
  }

  // S146. A short write is the one failure this tool can survive and must not:
  // the entries are 16 bytes each and sorted, so any prefix of them is also a
  // whole number of sorted entries. `dump` called a truncated book loadable and
  // the engine loaded it -- observed, writing 2755712 bytes onto a 1 MB volume
  // produced 901120 bytes, 56320 entries, verdict `loadable`, exit 0.
  //
  // S173. So the destination is never opened until the bytes are safely on
  // disk: they go to a temporary beside it and are renamed over it only once
  // the write is verified. The replaced `std::ofstream output(options.out,
  // ...)` truncated the previous book the instant it opened, which is why a
  // write that then failed left nothing where a good book had been. The
  // temporary's name is fixed, `<out>.tmp`, by the owner's decision (DEC-149):
  // at most one is ever left behind -- by a kill the process cannot survive to
  // clean up after -- and the next build to the same `--out` reuses it, at the
  // cost of two concurrent builds to one destination clobbering each other's.
  // It lives in the destination's own directory because rename(2) is atomic
  // only within a file system and fails EXDEV across one.
  const std::string temporary = options.out + ".tmp";

  // 0666 is what the replaced std::ofstream asked for; the umask narrows it,
  // and rename(2) carries this mode onto the destination.
  int fd = open(temporary.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

  // Every failure below keeps S146's `book not written`, which the manual and
  // the fixture test both grep, and adds errno so a full volume and a file-size
  // limit are told apart.
  //
  // `reason` is passed rather than read from errno inside, because a library
  // call between the failure and the message -- snprintf below -- is allowed to
  // set errno of its own even when it succeeds.
  auto abandon = [&](const char* what, const int reason) {
    fprintf(stderr, "%s '%s': %s -- book not written\n", what,
            temporary.c_str(), strerror(reason));

    if (fd >= 0) { close(fd); }

    unlink(temporary.c_str());
    return 1;
  };

  if (fd < 0) {
    // Nothing to clean up, and --out has not been touched.
    fprintf(stderr, "cannot write '%s': %s -- book not written\n",
            temporary.c_str(), strerror(errno));
    return 1;
  }

  // write() may write fewer bytes than it was given without failing, and may be
  // interrupted before it writes any.
  size_t done = 0;

  while (done < bytes.size()) {
    const ssize_t n = write(fd, bytes.data() + done, bytes.size() - done);

    if (n < 0) {
      if (errno == EINTR) { continue; }

      const int reason = errno;
      char what[64];

      snprintf(what, sizeof(what), "wrote %llu of %llu bytes to",
               (unsigned long long)done, (unsigned long long)bytes.size());
      return abandon(what, reason);
    }

    done += static_cast<size_t>(n);
  }

  // One line, unmeasurable at this size, and it closes the case where the
  // rename lands but the bytes never do. The directory is deliberately not
  // synced: a rename lost to a crash leaves the old book in place, which is
  // the state this step guarantees anyway.
  if (fsync(fd) != 0) { return abandon("cannot flush", errno); }

  // Some file systems report a write error here and nowhere earlier. The
  // descriptor is released either way, so it is not closed twice.
  const int closed = close(fd);
  fd = -1;

  if (closed != 0) { return abandon("cannot close", errno); }

  if (std::rename(temporary.c_str(), options.out.c_str()) != 0) {
    return abandon("cannot rename", errno);
  }

  printf("games read           %llu\n", (unsigned long long)games);
  printf("games cut short      %llu\n", (unsigned long long)rejected_games);
  printf("plies used           %llu\n", (unsigned long long)plies);
  printf("distinct positions   %llu\n", (unsigned long long)keys.size());
  printf("entries written      %llu\n", (unsigned long long)written);
  printf("dropped, min-games   %llu\n", (unsigned long long)dropped_min_games);
  printf("dropped, zero weight %llu\n",
         (unsigned long long)dropped_zero_weight);
  printf("weights clamped      %llu\n", (unsigned long long)clamped);
  printf("bytes               %llu -> %s\n", (unsigned long long)bytes.size(),
         options.out.c_str());

  return 0;
}


//-#############################  THE DUMP  #################################-//

std::string polyglot_move_to_string(uint16_t move)
{
  const uint8_t to_file = (move >> 0) & 7;
  const uint8_t to_rank = (move >> 3) & 7;
  const uint8_t from_file = (move >> 6) & 7;
  const uint8_t from_rank = (move >> 9) & 7;
  const uint8_t promotion = (move >> 12) & 7;

  std::string text = index_to_str(position_to_index(from_file, from_rank)) +
                     index_to_str(position_to_index(to_file, to_rank));

  if (promotion != 0 && promotion < 5) { text += "nbrq"[promotion - 1]; }

  return text;
}


int dump(const std::string& path, size_t top)
{
  std::ifstream input(path, std::ios::binary);

  if (input.fail()) {
    fprintf(stderr, "cannot open '%s'\n", path.c_str());
    return 1;
  }

  const std::vector<uint8_t> bytes(std::istreambuf_iterator<char>(input), {});

  printf("file                 %s\n", path.c_str());
  printf("bytes                %llu\n", (unsigned long long)bytes.size());

  if (bytes.empty() || bytes.size() % POLYGLOT_ENTRY_BYTES != 0) {
    printf("verdict              NOT a Polyglot book: %s\n",
           bytes.empty() ? "the file is empty"
                         : "the size is not a whole number of 16-byte entries");
    return 1;
  }

  const size_t entries = bytes.size() / POLYGLOT_ENTRY_BYTES;

  // The engine reads a book with a binary search and refuses one whose keys are
  // not sorted, so this is the check that decides whether a book is loadable at
  // all, not a nicety.
  uint64_t previous_key = 0;
  size_t distinct = 0;
  size_t widest = 0;
  size_t widest_run = 0;

  std::vector<std::pair<uint16_t, size_t>> heaviest;

  for (size_t i = 0; i < entries; ++i) {
    uint64_t key = 0;
    uint16_t move = 0;
    uint16_t weight = 0;

    memcpy(&key, bytes.data() + (i * POLYGLOT_ENTRY_BYTES), 8);
    memcpy(&move, bytes.data() + (i * POLYGLOT_ENTRY_BYTES) + 8, 2);
    memcpy(&weight, bytes.data() + (i * POLYGLOT_ENTRY_BYTES) + 10, 2);

    key = __builtin_bswap64(key);
    move = __builtin_bswap16(move);
    weight = __builtin_bswap16(weight);

    if (i > 0 && key < previous_key) {
      printf("verdict              NOT loadable: keys unsorted at entry %llu\n",
             (unsigned long long)i);
      return 1;
    }

    if (i == 0 || key != previous_key) {
      ++distinct;
      widest_run = 1;
    } else {
      ++widest_run;
    }

    widest = std::max(widest, widest_run);
    previous_key = key;

    heaviest.push_back({weight, i});
  }

  std::sort(heaviest.begin(), heaviest.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });

  printf("entries              %llu\n", (unsigned long long)entries);
  printf("distinct positions   %llu\n", (unsigned long long)distinct);
  printf("most moves for one   %llu\n", (unsigned long long)widest);
  printf("verdict              loadable\n");

  const size_t shown = std::min(top, heaviest.size());

  if (shown > 0) { printf("\nheaviest %llu:\n", (unsigned long long)shown); }

  for (size_t i = 0; i < shown; ++i) {
    const size_t index = heaviest[i].second;

    uint64_t key = 0;
    uint16_t move = 0;

    memcpy(&key, bytes.data() + (index * POLYGLOT_ENTRY_BYTES), 8);
    memcpy(&move, bytes.data() + (index * POLYGLOT_ENTRY_BYTES) + 8, 2);

    key = __builtin_bswap64(key);
    move = __builtin_bswap16(move);

    printf("  %016llx  %-6s  weight %llu\n", (unsigned long long)key,
           polyglot_move_to_string(move).c_str(),
           (unsigned long long)heaviest[i].first);
  }

  return 0;
}


void usage()
{
  fprintf(stderr,
          "usage:\n"
          "  make_book build <pgn> --out <bin> [--max-ply N] [--min-games N]\n"
          "                  [--allow-cut-short]\n"
          "  make_book dump  <bin> [--top N]\n");
}

}  // namespace


int main(int argc, char** argv)
{
  // S173. RLIMIT_FSIZE kills with SIGXFSZ by default, and that killed this tool
  // mid-write: observed under `ulimit -f 0`, exit 153 and a truncated -- and
  // loadable -- book left at --out, with no guard reached because the process
  // was already dead. Ignored, the limit arrives as EFBIG from write(), which
  // the write loop reports and cleans up after.
  std::signal(SIGXFSZ, SIG_IGN);

  if (argc < 3) {
    usage();
    return 1;
  }

  initialize_game_const_data(&game);

  const std::string mode = argv[1];

  if (mode == "dump") {
    size_t top = 10;

    for (int i = 3; i + 1 < argc; i += 2) {
      if (std::string(argv[i]) == "--top") {
        top = static_cast<size_t>(std::stoul(argv[i + 1]));
      }
    }

    return dump(argv[2], top);
  }

  if (mode != "build") {
    usage();
    return 1;
  }

  build_options_t options;
  options.pgn = argv[2];

  for (int i = 3; i < argc; ++i) {
    const std::string flag = argv[i];

    if (flag == "--allow-cut-short") {
      options.allow_cut_short = true;
      continue;
    }

    // Everything else takes a value. The old pairwise loop skipped a trailing
    // flag with no value silently; a flag that is not read is a flag the caller
    // believes was.
    if (i + 1 >= argc) {
      fprintf(stderr, "flag '%s' needs a value\n", flag.c_str());
      return 1;
    }

    const std::string value = argv[++i];

    if (flag == "--out") {
      options.out = value;
    } else if (flag == "--max-ply") {
      options.max_ply = std::stoi(value);
    } else if (flag == "--min-games") {
      options.min_games = static_cast<uint32_t>(std::stoul(value));
    } else {
      fprintf(stderr, "unknown flag '%s'\n", flag.c_str());
      return 1;
    }
  }

  if (options.out.empty()) {
    fprintf(stderr, "--out is required\n");
    usage();
    return 1;
  }

  return build(options);
}
