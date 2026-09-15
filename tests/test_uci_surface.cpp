#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "uci.hpp"

// The golden surface test required by AGENTS.md section 7 and DEC-024. Chesso's
// whole product surface is the UCI protocol, so `surface_guard` is `cli` and
// this file is what holds MANUAL.md to account: adding, renaming or removing a
// command or an option fails here until MANUAL.md is updated in the same
// commit.
//
// The commands and the option lines are read out of the running engine rather
// than restated from the source. uci_command_names() is the dispatch table that
// uci_process_line() dispatches through, and the option lines are the reply a
// GUI actually reads, so a surface addition cannot pass unnoticed.
//
// The [go] and [position] argument tokens are the exception. They are an
// if-else chain inside their handlers with nothing to enumerate at runtime, so
// the lists here are maintained by hand: a rename or a removal fails, a
// brand-new token does not. DEC-028 records that limit and why it was taken
// instead of rewriting the two parsers into dispatch tables.

#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR must be defined so the test can read MANUAL.md"
#endif


// clang-format off
static const std::vector<std::string> expected_commands = {
  // standard UCI
  "uci", "debug", "isready", "setoption", "register", "ucinewgame",
  "position", "go", "stop", "ponderhit", "quit",
  // non-standard convenience commands
  "pb", "fen", "help", "test", "bench", "clean-tt",
};

// Verbatim, including every default and every range. A changed Hash range is a
// changed surface and has to reach MANUAL.md like anything else.
static const std::vector<std::string> expected_option_lines = {
  "option name OwnBook type check default false",
  "option name Book File type string default <embedded>",
  "option name Best Book Move type check default false",
  "option name Hash type spin default 16 min 1 max 4096",
  "option name Threads type spin default 1 min 1 max 1",
};

static const std::vector<std::string> expected_option_names = {
  "OwnBook", "Book File", "Best Book Move", "Hash", "Threads",
};

// **Golden, DEC-142 and DEC-184.** The field names of one `info` line, in the
// order the engine writes them and with every value dropped -- `cp` is here
// because `score cp` and `score mate` are two different fields as far as a GUI
// reading the number is concerned, and the start position at a fixed depth is a
// `cp` line. S037 found this set outside the golden: a field added, removed or
// reordered passed every case in this file.
//
// Re-derive from the binary rather than from here. `go` is asynchronous, so a
// `quit` on the same pipe kills the search mid-iteration and prints a depth 1
// line -- the trap TOOLCHAIN.md records for stockfish, one engine over:
//   (printf 'position startpos\ngo depth 6\n'; sleep 2; printf 'quit\n') |
//     ./build/src/chesso
// MANUAL.md's "What a search prints" is the other end of this golden and is
// rewritten first (SURFACE); the case below reads the field names back out of
// its example line and requires the three to agree.
static const std::vector<std::string> expected_info_fields = {
  "score", "cp", "time", "depth", "nodes", "nps", "pv",
};
// clang-format on


// **Golden, DEC-142.** The ply bound in the refusal below and in MANUAL.md is
// POSITION_MAX_PLIES, defined in src/data_structures.hpp as
// `HISTORY_MAX_SIZE - MAX_PLY - 1`. The template is built from the constant, so
// moving either of those two fails the MANUAL.md case rather than passing over
// a stale number -- and this assert is what tells whoever moved them where the
// number they now have to rewrite lives: MANUAL.md writes it out three times,
// in the `position moves` refusal block, in the paragraph headed "The bound is
// 4871 plies", and in the sentence converting it to 2435 moves. Re-derive with
// `printf 'position startpos moves zzzz\nquit\n' | ./build/src/chesso` after a
// line long enough to trip it, or read the two constants.
static_assert(POSITION_MAX_PLIES == 4871,
              "POSITION_MAX_PLIES moved: update MANUAL.md's three statements "
              "of the 4871-ply bound, then this assert");


// S137. The tune build answers a `setoption` it cannot honour with one
// `info string` line, because the refusal used to go to a macro that compiles
// to nothing in a release build and `build-tune` is one -- so a tuner sending a
// misspelled name or an impossible value could not tell it apart from success
// (DEC-093). The templates are what MANUAL.md documents; the concrete lines are
// built from the parameter table below and compared verbatim, so a reworded
// message fails both halves.
static const std::vector<std::string> expected_refusal_templates = {
    "info string refused [<name>] value <value>, outside [<min>, <max>]",
    "info string refused [<name>] value <value>, not an integer, range [<min>, "
    "<max>]",
    "info string refused [<name>], unknown option",
    // S209: the two `Hash` refusals, which are release-build surface -- the
    // three above are `CHESSO_TUNE` only. MANUAL.md and specs.md described
    // them before these lines were added (SURFACE).
    "info string refused [Hash] <value>, not an integer",
    "info string refused [Hash] <value>, out of range",
    // S176: `position fen` refusals. MANUAL.md and specs.md described them
    // before these lines were added (SURFACE).
    "info string refused [position fen] <fen>, fewer than four fields",
    "info string refused [position fen] <fen>, does not load",
    // S210: the `moves` half of the same command, which skipped a token it
    // could not play and applied the rest of the list over the hole
    // (2026-09-04_adversarial-F02), and the history bound that made the engine
    // answer `bestmove 0000` in a position with legal moves
    // (2026-09-10_adversarial-F18). Release-build surface, like the two above.
    // MANUAL.md and specs.md described them before these lines were added
    // (SURFACE).
    "info string refused [position moves] <move>, does not parse",
    "info string refused [position moves] <move>, not a legal move here",
    "info string refused [position moves] <move>, the move stack is full at " +
        std::to_string(POSITION_MAX_PLIES) + " plies",
    // S210: the `go` numbers, which std::stoll read up to the first character
    // it could not use and reported through a macro that compiles to nothing
    // under NDEBUG -- the class S209 removed from `Hash`, one command along.
    // `<token>` is the UCI word, so the bracket reads `[go wtime]`; `bench` and
    // `test` name themselves the same way.
    //
    // Re-derive the five S210 lines from the binary rather than from here:
    //   printf 'position startpos moves zzzz\nquit\n'      | ./build/src/chesso
    //   printf 'position startpos moves e1g1\nquit\n'      | ./build/src/chesso
    //   printf 'go depth 0x4 movetime 20\nquit\n'          | ./build/src/chesso
    //   printf 'go depth 99999999999999999999 movetime 20\nquit\n' | ...
    // and the ply bound from POSITION_MAX_PLIES in src/data_structures.hpp.
    // The concrete lines are asserted verbatim in tests/test_engine.cpp; what
    // is golden here is that MANUAL.md documents each shape.
    "info string refused [go <token>] <value>, not an integer",
    "info string refused [go <token>] <value>, out of range",
    // S223: the last two placement rules, 2026-09-12_adversarial-F01.
    // MANUAL.md and specs.md described them before these lines were added
    // (SURFACE). Re-derive from the binary rather than from here:
    //   printf 'position fen 8/3p4/8/8/8/8/3P4/8 w - - 0 1\nquit\n' | ...
    //   printf 'position fen 7k/8/8/8/8/8/8/K6R w - - 0 1\nquit\n'  | ...
    "info string refused [position fen] <fen>, other than one king of each "
    "colour (<n> white, <n> black)",
    "info string refused [position fen] <fen>, the side not to move is in "
    "check",
    // S194: the book draw's seed from the environment, refused when it is not
    // an unsigned 64-bit decimal integer -- startup surface in every build,
    // not a UCI option. MANUAL.md and specs.md described it before this line
    // was added (SURFACE).
    "info string refused [CHESSO_BOOK_SEED] <value>, not an unsigned 64-bit "
    "decimal integer. Seeding the book draw from std::random_device",
};


// S073's tune build declares one spin option per search parameter and the
// release build declares none, so the golden lists above are the release
// surface and this is what the other configuration adds to them. Generated from
// the same table the engine prints from, which makes the line comparison a
// tautology there -- what is not a tautology is the MANUAL.md case below, which
// requires every parameter name to be documented by hand.
static std::vector<std::string> tune_option_lines()
{
  std::vector<std::string> lines;

#ifdef CHESSO_TUNE
  for (size_t i = 0; i < search_param_count(); ++i) {
    const search_param_t& param = search_param_info(i);

    lines.push_back("option name " + std::string(param.name) +
                    " type spin default " +
                    std::to_string(param.default_value) + " min " +
                    std::to_string(param.min_value) + " max " +
                    std::to_string(param.max_value));
  }
#endif

  return lines;
}


static std::vector<std::string> tune_option_names()
{
  std::vector<std::string> names;

#ifdef CHESSO_TUNE
  for (size_t i = 0; i < search_param_count(); ++i) {
    names.push_back(search_param_info(i).name);
  }
#endif

  return names;
}


static std::vector<std::string> with_tune_options(
    std::vector<std::string> release_surface,
    const std::vector<std::string>& added)
{
  release_surface.insert(release_surface.end(), added.begin(), added.end());
  return release_surface;
}


// clang-format off

static const std::vector<std::string> expected_go_tokens = {
  "depth", "movetime", "nodes", "wtime", "btime", "winc", "binc", "movestogo",
  "infinite",
  // accepted and ignored, which is still surface a GUI can send
  "mate", "searchmoves", "ponder",
};

static const std::vector<std::string> expected_position_tokens = {
  "startpos", "fen", "moves",
  // non-standard shortcuts. `empty` left with S223: the load boundary requires
  // one king of each colour, so a shortcut for the bare board named a position
  // the engine no longer accepts (DEC-197).
  "mate2w", "mate2b", "3frep", "kiwipete", "killer", "blocked", "fine70",
};
// clang-format on


static std::vector<std::string> sorted(std::vector<std::string> values)
{
  std::sort(values.begin(), values.end());
  return values;
}


// Reports both directions of a mismatch, so a failure names the command that
// was added and the one that went away instead of only saying they differ.
static std::string set_difference_report(const std::vector<std::string>& actual,
                                         const std::vector<std::string>& golden)
{
  const std::vector<std::string> a = sorted(actual);
  const std::vector<std::string> g = sorted(golden);

  std::vector<std::string> unexpected;
  std::vector<std::string> missing;

  std::set_difference(a.begin(), a.end(), g.begin(), g.end(),
                      std::back_inserter(unexpected));
  std::set_difference(g.begin(), g.end(), a.begin(), a.end(),
                      std::back_inserter(missing));

  if (unexpected.empty() && missing.empty()) { return ""; }

  std::string report = "UCI surface changed.";

  if (!unexpected.empty()) {
    report += " Present but not in the golden list:";
    for (const std::string& value : unexpected) {
      report += " [" + value + "]";
    }
  }

  if (!missing.empty()) {
    report += " In the golden list but gone from the engine:";
    for (const std::string& value : missing) {
      report += " [" + value + "]";
    }
  }

  report += ". Update MANUAL.md and this test together.";

  return report;
}


// A token that stands for a value rather than naming a field. Two shapes: an
// integer, which is what `time`, `depth`, `nodes`, `nps` and the number after
// `cp` or `mate` are -- signed, because a mate score for the other side is
// negative -- and a UCI move, which is what `pv` is followed by.
static bool is_value_token(const std::string& token)
{
  if (token.empty()) { return false; }

  size_t at = (token[0] == '-' || token[0] == '+') ? 1 : 0;
  bool integer = at < token.size();

  for (size_t i = at; i < token.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(token[i]))) {
      integer = false;
      break;
    }
  }

  if (integer) { return true; }

  const bool sized = token.size() == 4 || token.size() == 5;

  if (!sized) { return false; }

  const bool squares = token[0] >= 'a' && token[0] <= 'h' && token[1] >= '1' &&
                       token[1] <= '8' && token[2] >= 'a' && token[2] <= 'h' &&
                       token[3] >= '1' && token[3] <= '8';

  if (!squares) { return false; }

  return token.size() == 4 || token[4] == 'q' || token[4] == 'r' ||
         token[4] == 'b' || token[4] == 'n';
}


// The field names of one `info` line, values dropped, in the order they appear.
// Anything that is not a value is a name, so a field this engine does not print
// today would come out as a name rather than being silently swallowed -- which
// is the whole point of the golden it feeds.
static std::vector<std::string> info_fields(const std::string& line)
{
  std::vector<std::string> fields;

  if (line.rfind("info", 0) != 0) { return fields; }

  std::istringstream stream(line.substr(4));
  std::string token;

  while (stream >> token) {
    if (!is_value_token(token)) { fields.push_back(token); }
  }

  return fields;
}


static std::string joined(const std::vector<std::string>& values)
{
  std::string text;

  for (const std::string& value : values) {
    if (!text.empty()) { text += " "; }
    text += value;
  }

  return text;
}


static std::string read_manual()
{
  const std::string path = std::string(CHESSO_SOURCE_DIR) + "/MANUAL.md";
  std::ifstream file(path);
  REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

  std::stringstream contents;
  contents << file.rdbuf();

  return contents.str();
}


// MANUAL.md writes every name of the surface in backticks. The [go] arguments
// that are accepted and ignored are written with their command, so both
// spellings count as documented.
static bool manual_documents(const std::string& manual,
                             const std::string& command,
                             const std::string& name)
{
  if (manual.find("`" + name + "`") != std::string::npos) { return true; }
  if (command.empty()) { return false; }

  return manual.find("`" + command + " " + name + "`") != std::string::npos;
}


// Everything [help] prints between its two banner lines.
static std::vector<std::string> commands_from_help()
{
  stdout_capture_t capture;
  uci_process_line("help");

  std::vector<std::string> names;
  bool inside = false;

  for (const std::string& line : capture.lines()) {
    if (line == "--- Help: available commands ---") {
      inside = true;
      continue;
    }

    if (line == "--------------------------------") {
      inside = false;
      continue;
    }

    if (inside) { names.push_back(line); }
  }

  return names;
}


static std::vector<std::string> option_lines_from_uci()
{
  stdout_capture_t capture;
  uci_process_line("uci");

  std::vector<std::string> options;

  for (const std::string& line : capture.lines()) {
    if (line.rfind("option ", 0) == 0) { options.push_back(line); }
  }

  return options;
}


// The names out of the engine's own `option` lines, which is the list a GUI or
// a tuner reads and therefore the list `setoption` has to recognise.
static std::vector<std::string> option_names_from_uci()
{
  const std::string prefix = "option name ";
  std::vector<std::string> names;

  for (const std::string& line : option_lines_from_uci()) {
    const size_t type_at = line.find(" type ");

    if (line.rfind(prefix, 0) != 0 || type_at == std::string::npos) {
      continue;
    }

    names.push_back(line.substr(prefix.size(), type_at - prefix.size()));
  }

  return names;
}


// One setoption, and the lines it printed. Read back outside the capture scope
// on purpose: doctest reports a failure on std::cout, which is exactly what
// stdout_capture_t is holding, so an assertion made while the capture is alive
// fails invisibly.
static std::vector<std::string> lines_from_setoption(const std::string& line)
{
  stdout_capture_t capture;
  uci_process_line(line);

  return capture.lines();
}


// The FEN [fen] prints for the position the engine is holding.
static std::string current_fen()
{
  stdout_capture_t capture;
  uci_process_line("fen");

  const std::vector<std::string> lines = capture.lines();
  REQUIRE(lines.size() == 1);

  return lines.back();
}


// The two readers the bench case needs, written the way `tools/gate.sh` reads
// the same output: the signature is the last line matching
// `^[0-9]+ nodes [0-9]+ nps$`, and a count is the token after a named field.
// S189.
static bool is_signature_line(const std::string& line)
{
  std::istringstream stream(line);
  uint64_t nodes = 0;
  uint64_t nps = 0;
  std::string nodes_word;
  std::string nps_word;
  std::string trailing;

  if (!(stream >> nodes >> nodes_word >> nps >> nps_word)) { return false; }
  if (stream >> trailing) { return false; }

  return nodes_word == "nodes" && nps_word == "nps";
}


// An empty key reads the line's first token, which is where the signature puts
// its total; a named key reads the token after that word.
static bool field_after(const std::string& line,
                        const std::string& key,
                        uint64_t& out)
{
  std::istringstream stream(line);
  std::string token;

  if (key.empty()) { return static_cast<bool>(stream >> out); }

  while (stream >> token) {
    if (token == key) { return static_cast<bool>(stream >> out); }
  }

  return false;
}


// S212's `id name` form, read by hand instead of by `std::regex`.
//
//     id name Chesso <sha>[-dirty] <arch>[ tune]
//
// S225 replaced the pattern with these four functions and dropped `<regex>`
// from this file: gcc 13's `<regex>` header emits -Wmaybe-uninitialized inside
// libstdc++'s own `std::function` moves under -O2, which the Release and tune
// builds compile anyway and the sanitizer build turns into a -Werror failure,
// so every automatic gate since S212 was green while `build-sanitize` could not
// be built at all. The acceptance below is the pattern's, token for token; it
// is not suppressed with a pragma, because a pragma would leave the header
// included and the next test to reach for it in the same trap.
//
// GOLDEN (DEC-142): the form these four accept. Re-derive what it has to match
// with
//   printf 'uci\nquit\n' | ./build/src/chesso | sed -n 's/^id name //p'
//   printf 'uci\nquit\n' | ./build-tune/src/chesso | sed -n 's/^id name //p'
// -- no search runs on either pipe, so `quit` cannot truncate it. The arch
// names are `cmake/arch.cmake`'s four plus the `unknown` that
// `cmake/build_info.cmake` writes when ARCH does not reach it. Read them off
// the binary and off those two files, never off this one.

// Split on single spaces and not on whitespace runs: a double space leaves an
// empty field here and every check below rejects it, which is what anchoring
// the pattern at both ends did. A tab or a newline stays inside its token and
// fails the same way.
static std::vector<std::string> split_on_space(const std::string& line)
{
  std::vector<std::string> fields;
  std::string::size_type start = 0;

  for (;;) {
    const std::string::size_type space = line.find(' ', start);

    if (space == std::string::npos) {
      fields.push_back(line.substr(start));
      return fields;
    }

    fields.push_back(line.substr(start, space - start));
    start = space + 1;
  }
}


// `[0-9a-f]{7,}`: seven or more, and no upper bound, because `git rev-parse
// --short` lengthens the abbreviation as a repository grows. Lower case only --
// git emits lower case and `fastchess.sh` compares the two ends as strings.
static bool is_short_sha(const std::string& token)
{
  if (token.size() < 7) { return false; }

  for (const char c : token) {
    const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');

    if (!hex) { return false; }
  }

  return true;
}


// `unknown|[0-9a-f]{7,}(-dirty)?`. A sha carries no `-`, so stripping one
// trailing `-dirty` and asking for hex accepts exactly what the alternation
// did: `unknown-dirty` is rejected, since `-dirty` hangs off the hex branch
// alone, and so is `<sha>-dirty-dirty`.
static bool is_build_sha(const std::string& token)
{
  if (token == "unknown") { return true; }

  const std::string dirty = "-dirty";

  if (token.size() > dirty.size() &&
      token.compare(token.size() - dirty.size(), dirty.size(), dirty) == 0) {
    return is_short_sha(token.substr(0, token.size() - dirty.size()));
  }

  return is_short_sha(token);
}


// cmake/arch.cmake's four CHESSO_ARCH values, and cmake/build_info.cmake's
// `unknown` for a build whose ARCH did not reach the stamping script.
static bool is_build_arch(const std::string& token)
{
  return token == "bmi2" || token == "avx2" || token == "portable" ||
         token == "native" || token == "unknown";
}


static bool has_id_name_form(const std::string& line)
{
  const std::vector<std::string> fields = split_on_space(line);

  if (fields.size() != 5 && fields.size() != 6) { return false; }
  if (fields[0] != "id" || fields[1] != "name" || fields[2] != "Chesso") {
    return false;
  }
  if (!is_build_sha(fields[3])) { return false; }
  if (!is_build_arch(fields[4])) { return false; }
  if (fields.size() == 6 && fields[5] != "tune") { return false; }

  return true;
}


// The tune stamp, asked of the whole line so the case below can feed it a
// string. Within the form above the two readings agree: no field can hold a
// space, `tune` is not a sha and not an arch, so ` tune` occurs in an accepted
// line exactly when it is the last token.
static bool ends_with_tune(const std::string& line)
{
  const std::string suffix = " tune";

  return line.size() >= suffix.size() &&
         line.compare(line.size() - suffix.size(), suffix.size(), suffix) == 0;
}


TEST_SUITE("uci surface")
{
  TEST_CASE("the command set is exactly the documented one")
  {
    uci_init();

    const std::vector<std::string> actual = uci_command_names();

    // An empty enumeration would satisfy the comparison below by accident. The
    // count is deliberately not asserted here: a mismatch has to reach the
    // report, which names what moved, instead of stopping at a number.
    REQUIRE(!actual.empty());

    const std::string report = set_difference_report(actual, expected_commands);
    CHECK_MESSAGE(report.empty(), report);

    uci_shutdown();
  }


  TEST_CASE("help prints the dispatch table and nothing else")
  {
    uci_init();

    const std::vector<std::string> listed = commands_from_help();

    // [help] is user-facing surface in its own right. If it ever stops
    // reporting the table it walks, the list above stops being evidence of
    // anything.
    REQUIRE(!listed.empty());
    CHECK(listed == uci_command_names());

    uci_shutdown();
  }


  // S189. The signature is a number a script reads off the binary and compares
  // with a commit message, so what is asserted here is the shape it is read by
  // and its repeatability -- not the value, which lives in the commit messages
  // and in DEV_MANUAL.md and moves with every functional change by design.
  TEST_CASE("bench prints one final signature line and repeats its total")
  {
    uci_init();

    std::string first_total;
    std::vector<std::string> first_best_moves;

    {
      const stdout_capture_t capture;
      uci_process_line("bench");

      // `bench` searches on the calling thread, so by here it has finished --
      // no uci_wait_for_search(), which would join a thread that never ran.
      const std::vector<std::string> lines = capture.lines();
      REQUIRE(!lines.empty());

      std::vector<std::string> signature_lines;
      uint64_t summed_nodes = 0;
      uint64_t last_info_nodes = 0;
      bool saw_info_since_bestmove = false;

      for (const std::string& line : lines) {
        if (is_signature_line(line)) { signature_lines.push_back(line); }

        if (line.rfind("info ", 0) == 0) {
          uint64_t nodes = 0;

          if (field_after(line, "nodes", nodes)) {
            last_info_nodes = nodes;
            saw_info_since_bestmove = true;
          }
        }

        if (line.rfind("bestmove ", 0) == 0) {
          first_best_moves.push_back(line);

          // The precondition for the sum below: every search reports its count
          // on an info line before it names its move.
          REQUIRE(saw_info_since_bestmove);

          summed_nodes += last_info_nodes;
          saw_info_since_bestmove = false;
        }
      }

      // Eight positions, one move each, and the set is fixed in the source.
      CHECK(first_best_moves.size() == 8);

      REQUIRE(signature_lines.size() == 1);
      CHECK(signature_lines.front() == lines.back());

      uint64_t total = 0;
      REQUIRE(field_after(signature_lines.front(), "", total));

      // The signature is the sum of what the eight searches reported, so a
      // count the engine prints and a count the gate reads cannot diverge.
      CHECK(total == summed_nodes);

      first_total = signature_lines.front();
    }

    {
      const stdout_capture_t capture;
      uci_process_line("bench");

      const std::vector<std::string> lines = capture.lines();
      REQUIRE(!lines.empty());

      std::vector<std::string> best_moves;
      std::string total;

      for (const std::string& line : lines) {
        if (line.rfind("bestmove ", 0) == 0) { best_moves.push_back(line); }
        if (is_signature_line(line)) {
          uint64_t nodes = 0;
          REQUIRE(field_after(line, "", nodes));
          total = std::to_string(nodes);
        }
      }

      uint64_t first_nodes = 0;
      REQUIRE(field_after(first_total, "", first_nodes));

      // Two runs in one process. The table is cleared per position, so the
      // second run cannot inherit the first one's entries.
      CHECK(total == std::to_string(first_nodes));
      CHECK(best_moves == first_best_moves);
    }

    uci_shutdown();
  }


  TEST_CASE("an unknown command is answered with silence")
  {
    uci_init();

    // The precondition for the assertion below: a name that is a command does
    // reach stdout, so an empty capture means the input was refused rather
    // than that nothing ever prints here.
    {
      stdout_capture_t capture;
      uci_process_line("isready");
      REQUIRE(!capture.str().empty());
    }

    {
      stdout_capture_t capture;
      uci_process_line("nosuchcommand");
      CHECK(capture.str().empty());
    }

    uci_shutdown();
  }


  TEST_CASE("the option declarations are exactly the documented ones")
  {
    uci_init();

    const std::vector<std::string> actual = option_lines_from_uci();

    REQUIRE(!actual.empty());

    const std::string report = set_difference_report(
        actual, with_tune_options(expected_option_lines, tune_option_lines()));
    CHECK_MESSAGE(report.empty(), report);

    // The release build declares exactly the five golden lines -- three until
    // S172 added `Book File` and `Best Book Move` beside the renamed `OwnBook`
    // -- and S073 did not move that. Stated as its own assertion because the
    // comparison above grows a generated half in the tune build, and this half
    // must not.
    //
    // GOLDEN (DEC-142): 5, the number of `option name` lines the release
    // binary prints.
    // Re-derive with
    //   printf 'uci\nquit\n' | ./build/src/chesso | grep -c '^option name'
    // -- no search runs on that pipe, so `quit` cannot truncate it. Read off
    // the binary and not off `expected_option_lines`, which until S193 was
    // compared with its own literal and could not fail. DEC-142, S193,
    // 2026-09-04_test_review-F05.
    CHECK(actual.size() - tune_option_lines().size() == 5);
    CHECK(actual.size() ==
          expected_option_lines.size() + tune_option_lines().size());

    uci_shutdown();
  }


  TEST_CASE("the uci reply carries the identification a GUI needs")
  {
    uci_init();

    stdout_capture_t capture;
    uci_process_line("uci");

    // S212. `id name` carries the build and not only the engine:
    //
    //     id name Chesso <sha>[-dirty] <arch>[ tune]
    //
    // The short commit the binary was compiled from, `-dirty` when tracked
    // files were modified at build time, the CHESSO_ARCH target, and ` tune`
    // in the tune build and nowhere else. MANUAL.md and specs.md describe the
    // form; `fastchess.sh` refuses a run whose engine answers a sha other than
    // the one it labelled that side with, which is the loop the project
    // already closed on every opponent and could not close on itself
    // (DEC-068, 2026-09-10 adversarial F05).
    //
    // A shape and not a literal, because the sha moves with every commit -- so
    // what is pinned here is the form, and the form is the surface. `unknown`
    // is the sha of a binary built outside a git checkout, from a tarball say,
    // and is accepted for the same reason cmake emits it rather than failing
    // the build. `has_id_name_form` above is the check and carries the golden;
    // the case below it is what proves the check can say no.
    std::string id_line;

    for (const std::string& line : capture.lines()) {
      if (line.rfind("id name ", 0) == 0) { id_line = line; }
    }

    REQUIRE_MESSAGE(!id_line.empty(),
                    "the uci reply carries no 'id name' line");
    CHECK_MESSAGE(has_id_name_form(id_line), id_line);

    // Which build answered is not decoration: the tune build exposes every
    // search parameter as a UCI option and is never the binary a strength
    // figure is taken from, so a run that played it has to be able to say so
    // from the PGN alone. Asserted in both directions -- a stamp that always
    // said `tune`, or never did, would satisfy the form above.
#ifdef CHESSO_TUNE
    CHECK_MESSAGE(ends_with_tune(id_line),
                  ("the tune build does not say so in id name: " + id_line));
#else
    CHECK_MESSAGE(!ends_with_tune(id_line),
                  ("the release build says tune in id name: " + id_line));
#endif

    CHECK(capture.contains("id author MazerFaker"));
    CHECK(capture.contains("uciok"));

    uci_shutdown();
  }


  TEST_CASE("the id name form check refuses a malformed identification")
  {
    // The case above can only ever feed `has_id_name_form` one string, the one
    // this build answers, so on its own it does not show the check can say no
    // -- a matcher that returned true unconditionally would pass it. That is
    // what a hand-written check costs against a pattern, and this is the
    // payment. Every string below was run against the check inverted, and each
    // negative was observed red before being written this way round. S225.
    CHECK(has_id_name_form("id name Chesso 600f448 native"));
    CHECK(has_id_name_form("id name Chesso 600f448-dirty bmi2"));
    CHECK(has_id_name_form("id name Chesso 47be85b avx2 tune"));
    CHECK(has_id_name_form("id name Chesso unknown unknown"));
    // No upper bound on the abbreviation: git lengthens it as a repository
    // grows, and a binary built from a longer one is not malformed.
    CHECK(has_id_name_form("id name Chesso 0123456789abcdef portable tune"));

    // The pre-stamp literal every binary before S212 answers. DEC-204 (b) lets
    // such a side play a match with the identity check announced as skipped;
    // it is not the form this build may answer in.
    CHECK_FALSE(has_id_name_form("id name Chesso"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f44 native"));   // 6 digits
    CHECK_FALSE(has_id_name_form("id name Chesso 600F448 native"));  // upper
    CHECK_FALSE(has_id_name_form("id name Chesso 600g448 native"));  // not hex
    CHECK_FALSE(has_id_name_form("id name Chesso unknown-dirty native"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448-dirty-dirty native"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448-clean native"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448 riscv"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448 tune"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448 native tune extra"));
    CHECK_FALSE(
        has_id_name_form("id name Chesso  600f448 native"));  // 2 spaces
    CHECK_FALSE(has_id_name_form("id name chesso 600f448 native"));
    CHECK_FALSE(has_id_name_form("id author Chesso 600f448 native"));
    CHECK_FALSE(has_id_name_form("id name Chesso 600f448 native\n"));
    CHECK_FALSE(has_id_name_form(""));

    // And the direction check the two builds read in opposite senses.
    CHECK(ends_with_tune("id name Chesso 600f448 native tune"));
    CHECK_FALSE(ends_with_tune("id name Chesso 600f448 native"));
    CHECK_FALSE(ends_with_tune("id name Chesso 600f448 native tuned"));
  }


  TEST_CASE("MANUAL.md documents every command and every option")
  {
    const std::string manual = read_manual();

    // A MANUAL.md that failed to load would document nothing, and every name
    // below would be reported missing for the wrong reason.
    REQUIRE(manual.find("# Chesso") != std::string::npos);

    for (const std::string& name : expected_commands) {
      CHECK_MESSAGE(manual_documents(manual, "", name),
                    ("MANUAL.md does not document the command [" + name + "]"));
    }

    for (const std::string& name :
         with_tune_options(expected_option_names, tune_option_names())) {
      CHECK_MESSAGE(manual_documents(manual, "", name),
                    ("MANUAL.md does not document the option [" + name + "]"));
    }
  }


  TEST_CASE("MANUAL.md documents every go and position argument")
  {
    const std::string manual = read_manual();

    REQUIRE(manual.find("# Chesso") != std::string::npos);

    for (const std::string& name : expected_go_tokens) {
      CHECK_MESSAGE(manual_documents(manual, "go", name),
                    ("MANUAL.md does not document [go " + name + "]"));
    }

    for (const std::string& name : expected_position_tokens) {
      CHECK_MESSAGE(manual_documents(manual, "position", name),
                    ("MANUAL.md does not document [position " + name + "]"));
    }
  }


  TEST_CASE("the ignored go arguments leave the rest of the line working")
  {
    uci_init();

    // [go depth] and the time controls are covered end to end in test_engine.
    // What only exists as surface is the promise that mate, searchmoves and
    // ponder are swallowed rather than rejected, and that the limit sitting
    // behind them is still read.
    stdout_capture_t capture;
    uci_process_line("position startpos");
    uci_process_line("go mate 2 searchmoves e2e4 ponder depth 1");
    uci_wait_for_search();

    CHECK(capture.contains("bestmove"));

    uci_shutdown();
  }


  TEST_CASE("every position argument still reaches the board")
  {
    uci_init();

    const std::string start_fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    {
      uci_process_line("position startpos");
      REQUIRE(current_fen() == start_fen);
    }

    // Each shortcut has to land on a position of its own. A token that stopped
    // being parsed would leave the board where the preceding command put it,
    // and two shortcuts would then report the same FEN.
    std::vector<std::string> reached;

    for (const std::string& token : expected_position_tokens) {
      if (token == "fen" || token == "moves") { continue; }

      uci_process_line("position startpos");
      uci_process_line("position " + token);

      const std::string fen = current_fen();

      if (token != "startpos") {
        CHECK_MESSAGE(fen != start_fen,
                      ("[position " + token + "] left the start position"));
      }

      reached.push_back(fen);
    }

    std::vector<std::string> distinct = sorted(reached);
    distinct.erase(std::unique(distinct.begin(), distinct.end()),
                   distinct.end());

    CHECK(distinct.size() == reached.size());

    {
      // `empty` left the command with S223 (DEC-197): the load boundary
      // requires one king of each colour, so the bare board it named no longer
      // loads and the token is not parsed at all. It therefore moves nothing,
      // and this is what would fail if it came back.
      uci_process_line("position fine70");
      const std::string parked = current_fen();

      uci_process_line("position empty");
      CHECK(current_fen() == parked);

      uci_process_line("position fen " + start_fen);

      CHECK(current_fen() == start_fen);
    }

    {
      uci_process_line("position startpos moves e2e4");

      CHECK(current_fen() != start_fen);
    }

    uci_shutdown();
  }


  // DEC-184, the second of the seven parked findings it settled, and the gap
  // S037 left: the `info` line is the entire running commentary a GUI reads,
  // and until now nothing here looked at what it carries. Values are excluded
  // deliberately -- a node count moves with every search change and a golden
  // over one would be re-derived away within a week. The field set does not
  // move unless the surface moves.
  //
  // Three ends held together: this file's golden, the engine's own line, and
  // MANUAL.md's example. A field added to the engine and written into the
  // manual still fails here, and a field added to both without touching the
  // manual's example fails too.
  TEST_CASE("the info line's fields are exactly the documented ones")
  {
    uci_init();

    std::vector<std::string> lines;

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("go depth 6");
      uci_wait_for_search();
      lines = capture.lines();
    }

    // The last completed iteration's line, which is the one a GUI acts on.
    // `info string` is a different surface with its own cases above.
    std::string last_info;

    for (const std::string& line : lines) {
      if (line.rfind("info ", 0) == 0 && line.rfind("info string", 0) != 0) {
        last_info = line;
      }
    }

    REQUIRE_MESSAGE(!last_info.empty(),
                    "a fixed-depth search on the start position printed no "
                    "info line for the golden to read");

    CHECK_MESSAGE(
        info_fields(last_info) == expected_info_fields,
        ("the info line's fields changed. Engine: [" +
         joined(info_fields(last_info)) + "], golden: [" +
         joined(expected_info_fields) +
         "]. Rewrite MANUAL.md's \"What a search prints\" first, then this "
         "golden. Line: " +
         last_info));

    // MANUAL.md's example, read through the same extractor. It is the only
    // line in the manual that starts an `info score`, and the section says in
    // so many words that it is the specification and not an illustration.
    const std::string manual = read_manual();
    const size_t at = manual.find("\ninfo score ");

    REQUIRE_MESSAGE(at != std::string::npos,
                    "MANUAL.md no longer carries an example info line in "
                    "\"What a search prints\"");

    const size_t end = manual.find('\n', at + 1);
    const std::string documented = manual.substr(at + 1, end - at - 1);

    CHECK_MESSAGE(info_fields(documented) == expected_info_fields,
                  ("MANUAL.md's example line does not carry the golden field "
                   "set. Manual: [" +
                   joined(info_fields(documented)) + "], golden: [" +
                   joined(expected_info_fields) + "]. Line: " + documented));

    uci_shutdown();
  }


  TEST_CASE("MANUAL.md documents the refusal lines")
  {
    const std::string manual = read_manual();

    REQUIRE(manual.find("# Chesso") != std::string::npos);

    for (const std::string& line : expected_refusal_templates) {
      CHECK_MESSAGE(manual.find(line) != std::string::npos,
                    ("MANUAL.md does not document the line [" + line + "]"));
    }
  }


  // S209's precondition, and it is the one thing case-insensitive names can
  // break that no other case here would catch: two options whose names differ
  // only in case are one option to a folded comparison, and the first branch of
  // the chain would silently take both. 33 names in the tune build, 5 in the
  // release build, and nothing stops a future parameter from colliding with an
  // existing one except this.
  TEST_CASE("no two advertised option names collide when folded")
  {
    uci_init();

    const std::vector<std::string> names = option_names_from_uci();

    REQUIRE(!names.empty());

    std::vector<std::string> folded;

    for (const std::string& name : names) {
      std::string lowered = name;

      std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                     [](char c) {
                       return static_cast<char>(
                           std::tolower(static_cast<unsigned char>(c)));
                     });

      for (const std::string& seen : folded) {
        CHECK_MESSAGE(seen != lowered,
                      ("[" + name + "] folds onto another advertised option"));
      }

      folded.push_back(lowered);
    }

    uci_shutdown();
  }


  TEST_CASE("what uci advertises, setoption recognises")
  {
    // command_setoption keeps its own list of the option names that are not
    // search parameters, so an option added to [uci] and not to that list would
    // be reported unknown to a tuner sending exactly what [uci] advertised.
    // Sent with no value on purpose: every option refuses that, and what is
    // asserted is only that none of them refuses it as an unknown *name*.
    uci_init();

    const std::vector<std::string> names = option_names_from_uci();

    REQUIRE(!names.empty());

#ifdef CHESSO_TUNE
    // Non-vacuity. Only the tune build reports an unknown name at all, so this
    // is where the assertion below can be shown to have something to fail on:
    // a name that is not advertised does come back unknown.
    REQUIRE(lines_from_setoption("setoption name NoSuchOption").size() == 1);
#endif

    for (const std::string& name : names) {
      const std::vector<std::string> printed =
          lines_from_setoption("setoption name " + name);

      bool unknown = false;

      for (const std::string& line : printed) {
        if (line.find("unknown option") != std::string::npos) {
          unknown = true;
        }
      }

      CHECK_MESSAGE(!unknown, ("[uci] advertises [" + name +
                               "] and [setoption] does not know it"));
    }

    uci_shutdown();
  }


#ifdef CHESSO_TUNE
  TEST_CASE("a setoption the tune build cannot honour says so")
  {
    uci_init();

    size_t index = search_param_count();

    for (size_t i = 0; i < search_param_count(); ++i) {
      if (std::string(search_param_info(i).name) == "RfpMargin") { index = i; }
    }

    REQUIRE(index < search_param_count());

    const search_param_t& param = search_param_info(index);
    const std::string range = "[" + std::to_string(param.min_value) + ", " +
                              std::to_string(param.max_value) + "]";
    const std::string too_high = std::to_string(param.max_value + 1);

    // Precondition for all three cases below: a legal value prints nothing at
    // all. Without it a line would only show that the engine narrates every
    // setoption, which is not observability of a refusal.
    CHECK(lines_from_setoption("setoption name RfpMargin value " +
                               std::to_string(param.default_value))
              .empty());

    const std::vector<std::string> out_of_range =
        lines_from_setoption("setoption name RfpMargin value " + too_high);

    REQUIRE(out_of_range.size() == 1);
    CHECK(out_of_range.front() == "info string refused [RfpMargin] value " +
                                      too_high + ", outside " + range);

    // The refusal is a refusal: the line is not a warning printed on the way to
    // taking the value anyway.
    CHECK(search_param_value(index) == param.default_value);

    // Every token that is not an integer, not only one with no digits in it.
    // std::stoi() was here and stops at the first character it cannot use
    // without complaining, so `0x50` set 0, `120.9` set 120 and `12x` set 12,
    // each of them silently -- the same failure S137 exists to remove, one
    // layer down. Measured 2026-08-20 on build-tune.
    for (const std::string& token :
         {std::string("nonsense"), std::string("0x50"), std::string("120.9"),
          std::string("12x"), std::string("+120"), std::string(""),
          std::string("--120")}) {
      const std::vector<std::string> not_a_number =
          lines_from_setoption("setoption name RfpMargin value " + token);

      REQUIRE_MESSAGE(not_a_number.size() == 1,
                      ("[" + token + "] was answered with " +
                       std::to_string(not_a_number.size()) + " lines"));
      CHECK(not_a_number.front() == "info string refused [RfpMargin] value " +
                                        token + ", not an integer, range " +
                                        range);

      CHECK_MESSAGE(search_param_value(index) == param.default_value,
                    ("[" + token + "] moved the parameter to " +
                     std::to_string(search_param_value(index))));
    }

    // A well-formed integer no int can hold is out of range, not malformed:
    // stoi() threw out_of_range and the single catch collapsed it into the
    // wrong one of the two lines.
    for (const std::string& token :
         {std::string("99999999999"), std::string("-99999999999")}) {
      const std::vector<std::string> too_big =
          lines_from_setoption("setoption name RfpMargin value " + token);

      REQUIRE(too_big.size() == 1);
      CHECK(too_big.front() == "info string refused [RfpMargin] value " +
                                   token + ", outside " + range);

      CHECK(search_param_value(index) == param.default_value);
    }

    // A misspelled name, which is the other half of DEC-093 and the one no
    // range check can catch. `Rfpmargin` stood here until S209 and is not a
    // misspelling any more: the protocol's case rule makes it the parameter, so
    // what is left for this half is a name that is no option in any casing.
    // test_search_params "a mis-cased parameter name is still the parameter"
    // holds the other side of that change.
    const std::vector<std::string> unknown =
        lines_from_setoption("setoption name RfpMargn value 100");

    REQUIRE(unknown.size() == 1);
    CHECK(unknown.front() == "info string refused [RfpMargn], unknown option");

    uci_shutdown();
  }
#else
  TEST_CASE("the release build answers an unknown option with silence")
  {
    // The other side of the pair. S137 is tune-build surface and the release
    // binary's stdout is untouched by it: the parameter names are not options
    // here at all, so the two cases the tune build reports are exactly the
    // cases this build must stay quiet about.
    uci_init();

    // Precondition: this build does print on stdout when asked something it
    // knows, so an empty capture below is silence rather than a dead stream.
    CHECK(!lines_from_setoption("uci").empty());

    const std::vector<std::string> commands = {
        "setoption name RfpMargin value 100",
        "setoption name RfpMargin value 999999",
        "setoption name RfpMargin value nonsense",
        "setoption name RfpMargin value 0x50",
        "setoption name RfpMargin value 99999999999",
        "setoption name Rfpmargin value 100",
        "setoption name RfpMargn value 100",
        "setoption name NoSuchOption value 1",
        "setoption name Threads value 4",
    };

    for (const std::string& command : commands) {
      const std::vector<std::string> printed = lines_from_setoption(command);

      CHECK_MESSAGE(
          printed.empty(),
          ("[" + command + "] printed [" +
           (printed.empty() ? std::string() : printed.front()) + "]"));
    }

    uci_shutdown();
  }
#endif
}
