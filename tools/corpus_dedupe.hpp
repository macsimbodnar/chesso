#pragma once
// One row per distinct position in a tuning corpus, keyed on the engine's own
// zobrist key. S076.
//
// `adocs/eval_tuning_strategy.md` section 2.6 lists it among the filters "all
// of which matter": "Deduplicate by Zobrist key; heavily repeated positions
// bias the fit." `tools/datagen.cpp` does not dedupe, so a position reached in
// two games -- or repeated inside one game on the way to a repetition draw --
// carries its weight once per occurrence, and the weight is whatever the
// opening randomiser happened to produce rather than a modelling choice anyone
// made.
//
// The pass is a pure row filter: the first row of a repeated position is
// written through byte for byte and the rest are dropped, so the output is
// still exactly what `datagen` wrote and still loads in anything that reads the
// four-column format. Nothing is replayed and no label is averaged. DEC-065.
//
// **The key is the engine's, not the FEN text.** Two FENs that differ only in
// halfmove clock or fullmove number are the same position to `evaluate()`, and
// `compute_full_hash()` in `src/bitboard.cpp` covers exactly the four
// FEN fields that decide one: placement, side to move, castling rights and the
// en passant square. Keying on the text would treat a clock as a position, and
// when this pass was written it would have inherited the defect S042 was then
// open on -- `make_move` wrote an en passant square whether or not a capture
// was available (`2026-08-13_adversarial-F08`), so two transposing move orders
// reached one position under two FENs. That defect was in the key here too,
// and it is the project's own definition of a position rather than a second
// one invented for this tool. S042 closed it in `50b1ff9`, 2026-09-12, in both
// places that decide the square: make_move_impl() sets it only when a pawn of
// the side to move attacks it, and load_FEN() drops one no such pawn attacks.
// So a key taken since then is the same for both move orders, a row written
// before S042 included. A corpus deduped before S042 was keyed the old way and
// keeps one row of such a position per en passant field its rows carry.
//
// The identity test is 64-bit equality and nothing else. Over 11.0 M rows the
// birthday exposure is about 3e-06 and a collision would drop one distinct
// position; `--verify` holds the four hashed fields per key and counts
// key-equal rows that disagree on them, so the exposure is measured on the real
// corpus instead of being argued from the bound. DEC-065.
//
// Until S327 the fields it held were the row's own text, and from S042 on that
// was no longer what the key reads: the key takes the en passant field as
// load_FEN() keeps it, so a row whose square no pawn of the side to move
// attacks keys with the same position written without one, and the text
// comparison counted each such merged row as a collision -- a row repeating an
// earlier merged row's en passant field too, though that one is a duplicate
// `a3e84e1` drops as well. None of them is one. On F08's three rows at
// `1bb3080` it counted 2 (`adocs/data/S325_ep_key.txt`); on `selfplay_v2.tsv`,
// a corpus written before S042, it counted 0 at S042's parent `a3e84e1`, as at
// S076, and 503 at S327's base, where the pass drops 492 more rows than
// `a3e84e1`: the other 11 are such repeats (`adocs/data/S327_verify.txt`,
// `adocs/data/S327_gap.cpp`). Since S327 the fields come from the board
// the key was taken from, by hashed_fields() below, so a field load_FEN()
// clears reads cleared on both sides and two rows disagree only when their
// keys collided: 0 on that corpus, whose kept rows are the same before and
// after S327.
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>
#include "bitboard.hpp"

namespace corpus_dedupe
{

// Keys taken by exactly 1, 2, 3, 4..7, 8..15 and 16 or more rows. The pass's
// first product is how big the effect is, which nobody has measured, and one
// drop count cannot tell a corpus where every position repeats twice from one
// where a handful repeat thousands of times.
constexpr size_t BUCKET_COUNT = 6;

struct stats_t
{
  uint64_t rows_read = 0;
  uint64_t rows_written = 0;  // distinct positions
  uint64_t rows_dropped = 0;
  uint64_t blank_skipped = 0;  // a line with nothing on it carries no position
  uint64_t most_repeated = 0;  // rows sharing one key, the largest
  uint64_t collisions = 0;     // --verify only, see the header
  uint64_t buckets[BUCKET_COUNT] = {0, 0, 0, 0, 0, 0};
};


// The first tab-separated field of a `fen result score phase` row. False when
// the row has fewer than the four fields the format names -- refused rather
// than skipped, because a row this cannot read is a corpus the tuner would read
// differently, and a filter that silently drops rows is the thing this tool
// exists to make visible.
inline bool row_fen(const std::string& line, std::string* fen)
{
  size_t field = 0;
  size_t start = 0;

  for (; field < 4; ++field) {
    const size_t tab = line.find('\t', start);

    if (field == 0) {
      *fen = line.substr(0, (tab == std::string::npos) ? line.size() : tab);
    }

    if (tab == std::string::npos) { break; }

    start = tab + 1;
  }

  return field >= 3 && !fen->empty();
}


// The four FEN fields the zobrist key covers, as the key sees them: placement,
// side to move, castling rights, en passant square, written by generate_FEN()
// from a board load_FEN() filled. That board is the one compute_full_hash()
// keys, so a field load_FEN() clears reads cleared here too -- an en passant
// square no pawn of the side to move attacks, by en_passant_is_capturable(),
// reads "-". Two rows whose keys are equal and whose fields here are not have
// collided. `--verify` only.
inline std::string hashed_fields(const board_t* board)
{
  // generate_FEN() writes its six fields one space apart, so the four the key
  // covers end at the fourth space.
  const std::string fen = generate_FEN(board);
  size_t end = 0;

  for (size_t field = 0; field < 4; ++field) {
    end = fen.find(' ', end + 1);
  }

  return fen.substr(0, end);
}


// The same, from a FEN, for a caller that holds no loaded board. False when the
// FEN does not load.
inline bool hashed_fields(const std::string& fen, std::string* out)
{
  game_t game;
  initialize_game_const_data(&game);

  if (!load_FEN(fen, &game)) { return false; }

  *out = hashed_fields(&game.board);
  return true;
}


// The engine's key for the position a FEN names. `game` must have been through
// initialize_game_const_data() -- the randoms live in it, and a game that skips
// that hashes every position to zero.
inline bool position_key(const std::string& fen, game_t* game, uint64_t* key)
{
  if (!load_FEN(fen, game)) { return false; }

  *key = game->board.hash;
  return true;
}


// One pass: read rows, write the first row of each distinct position in file
// order, and count what was dropped. Returns false and fills `error` on a row
// that cannot be read, leaving whatever was already written to the caller to
// discard -- a corpus with one unreadable row is refused, not filtered.
//
// Deterministic by construction: single threaded, one pass, output in input
// order, and the decision to write depends only on the keys of the rows before
// it. The key itself is stable across processes because init_zobrist() is
// fixed-seed (`CHESSO_PROJECT_SEED` in `src/bitboard.cpp`).
inline bool run(std::istream& in,
                std::ostream& out,
                bool verify,
                stats_t* stats,
                std::string* error)
{
  game_t game;
  initialize_game_const_data(&game);

  // Counts per key, and -- in verify mode only -- the four hashed fields of the
  // row that claimed the key. The second map is what makes a collision a
  // measurement; it roughly doubles the pass's memory, which is why it is a
  // flag.
  std::unordered_map<uint64_t, uint32_t> seen;
  std::unordered_map<uint64_t, std::string> witness;

  std::string line;
  std::string fen;
  std::string fields;

  while (std::getline(in, line)) {
    if (line.empty()) {
      stats->blank_skipped++;
      continue;
    }

    stats->rows_read++;

    if (!row_fen(line, &fen)) {
      *error = "row does not carry four fields: " + line;
      return false;
    }

    uint64_t key = 0;

    if (!position_key(fen, &game, &key)) {
      *error = "row does not carry a loadable FEN: " + line;
      return false;
    }

    // From the board position_key() just loaded, the one the key was taken
    // from. It cannot fail: a FEN that loads carried every field.
    if (verify) { fields = hashed_fields(&game.board); }

    const uint32_t count = ++seen[key];

    if (count == 1) {
      stats->rows_written++;
      out << line << '\n';

      if (verify) { witness[key] = fields; }
    } else {
      stats->rows_dropped++;

      if (verify && witness[key] != fields) { stats->collisions++; }
    }
  }

  for (const auto& entry : seen) {
    const uint32_t count = entry.second;

    if (count > stats->most_repeated) { stats->most_repeated = count; }

    if (count >= 16) {
      stats->buckets[5]++;
    } else if (count >= 8) {
      stats->buckets[4]++;
    } else if (count >= 4) {
      stats->buckets[3]++;
    } else {
      stats->buckets[count - 1]++;
    }
  }

  return true;
}

}  // namespace corpus_dedupe
