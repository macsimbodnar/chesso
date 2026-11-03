#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <bitset>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "corpus_dedupe.hpp"

// S076. `tools/datagen` writes a row per position and never asks whether it has
// written that position before, so a position reached in two games -- or
// repeated inside one game on the way to a repetition draw -- carries its
// weight once per occurrence. `tools/corpus_dedupe` reduces a corpus to one row
// per distinct position, keyed on the engine's own zobrist key.
//
// What this file holds:
//
//   1. the two rows of the clock-only pair are different text -- the
//      precondition without which case 2 would be asserting that identical
//      lines collapse, which no key is needed for
//   2. a position repeated with different clocks, labels and scores collapses
//      to one row, and it is the **first** row, byte for byte
//   3. every field the key covers separates two rows: castling rights, the en
//      passant square and the side to move each keep both
//   4. distinct positions all survive, in input order
//   5. the same input twice gives the same output and the same counts
//   6. the repeat histogram counts what repeated and how often
//   7. a row with fewer than four fields is refused, not skipped
//   8. a row whose FEN does not load is refused, not skipped
//   9. hashed_fields() -- what `--verify` compares -- reads the four fields the
//      key covers and is equal across a clock-only difference
//  10. `--verify` changes no output and reports no collision on a corpus that
//      has none
//  11. `--verify` reads an en passant square no pawn can take as the key does:
//      rows S042's key merges are not counted as collisions (S327)
//  12. hashed_fields() still reads an en passant square a pawn can take, so a
//      comparison that dropped every square would not pass case 11
//  13. `--verify` counts a real 64-bit collision, built from the engine's own
//      randoms -- the precondition without which cases 10 and 11 would pass
//      against a counter that never counts
//
// Case 1 is the non-vacuity guard, and case 3 is the other half of it: a tool
// that collapsed *everything* would pass case 2 and fail case 3.

namespace
{

// The start position, and the same position after seven irreversible-clock
// moves that cannot have happened. Same placement, same side to move, same
// castling rights, same en passant square, so the same key -- and different
// text, which is case 1.
const std::string START =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string START_LATER_CLOCKS =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 7 12";

// One field of the key changed in each, everything else held.
const std::string START_NO_WHITE_QUEENSIDE =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Kkq - 0 1";
const std::string START_BLACK_TO_MOVE =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

// A real position with an en passant square, and the same placement without
// one -- except the square is not one a pawn can reach: no white pawn stands
// on b5 or d5, confirmed uncapturable with python-chess 1.11.2
// (`chess.Board(AFTER_C5_EP).has_pseudo_legal_en_passant()` is False, xfen
// prints "-"). Before S042 the key covered the square either way, so this
// pair separated; after it, load_FEN's sanitiser clears AFTER_C5_EP's fourth
// field on load and the two key identically, correctly, since X-FEN makes
// them the same position (S042/DEC-187, the owner's 2026-09-08 choice). That
// merge is asserted below, in "an en passant square with no capturing pawn no
// longer separates two positions". Kept here too as a filler distinct
// position for the tests that just need a third FEN unrelated to castling or
// side to move: its placement alone already separates it from START and
// AFTER_E4, regardless of the en-passant convention.
const std::string AFTER_C5_EP =
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
const std::string AFTER_C5_NO_EP =
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";

// The pair that now exercises "the en passant square separates two
// positions" in its place: a white pawn on e5 stands beside black's fresh
// double push to d5, so d6 is genuinely capturable -- confirmed with
// python-chess 1.11.2 (`has_pseudo_legal_en_passant()` is True; xfen keeps
// "d6" unchanged). S042.
const std::string AFTER_D5_EP =
    "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3";
const std::string AFTER_D5_NO_EP =
    "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3";

// A different placement altogether.
const std::string AFTER_E4 =
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";


std::string row(const std::string& fen,
                const std::string& result,
                const std::string& score,
                const std::string& phase)
{ return fen + "\t" + result + "\t" + score + "\t" + phase; }


std::string corpus(const std::vector<std::string>& rows)
{
  std::string text;

  for (const std::string& line : rows) {
    text += line + "\n";
  }

  return text;
}


// The whole pass over an in-memory corpus. Returns what run() returned; the
// output and the counts come back through the arguments.
bool dedupe(const std::string& text,
            bool verify,
            std::string* out,
            corpus_dedupe::stats_t* stats,
            std::string* error)
{
  std::istringstream in(text);
  std::ostringstream sink;

  const bool ok = corpus_dedupe::run(in, sink, verify, stats, error);

  *out = sink.str();
  return ok;
}

}  // namespace


TEST_CASE("the clock-only pair is two different lines")
{
  // Without this, case 2 would be asserting that a tool which writes every line
  // once collapses two identical lines, which needs no key at all.
  CHECK(START != START_LATER_CLOCKS);

  const std::string first = row(START, "1.0", "31", "24");
  const std::string second = row(START_LATER_CLOCKS, "0.0", "-58", "24");

  CHECK(first != second);
}


TEST_CASE("a position repeated with different clocks keeps its first row")
{
  const std::string first = row(START, "1.0", "31", "24");
  const std::string second = row(START_LATER_CLOCKS, "0.0", "-58", "24");

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe(corpus({first, second}), false, &out, &stats, &error));

  CHECK(stats.rows_read == 2);
  CHECK(stats.rows_written == 1);
  CHECK(stats.rows_dropped == 1);

  // Byte for byte the first row, label and score included: the pass is a filter
  // over rows and never rewrites one. DEC-065.
  CHECK(out == first + "\n");
}


TEST_CASE(
    "castling rights, the en passant square and the side to move all separate")
{
  struct pair_t
  {
    std::string left;
    std::string right;
  };

  // AFTER_C5_EP/AFTER_C5_NO_EP moved out of this list, S042: that pair's en
  // passant square (c6) has no capturing pawn, so under X-FEN the two are the
  // same position and are expected to merge, not separate -- see the
  // dedicated case below. AFTER_D5_EP/AFTER_D5_NO_EP takes its place: d6 is
  // genuinely capturable, so that pair still must separate.
  const std::vector<pair_t> pairs = {
      {START, START_NO_WHITE_QUEENSIDE},
      {START, START_BLACK_TO_MOVE},
      {AFTER_D5_EP, AFTER_D5_NO_EP},
  };

  for (const pair_t& pair : pairs) {
    const std::string first = row(pair.left, "1.0", "31", "24");
    const std::string second = row(pair.right, "0.0", "-58", "24");

    std::string out;
    std::string error;
    corpus_dedupe::stats_t stats;

    REQUIRE(dedupe(corpus({first, second}), false, &out, &stats, &error));

    CHECK(stats.rows_written == 2);
    CHECK(stats.rows_dropped == 0);
    CHECK(out == first + "\n" + second + "\n");
  }
}


// AFTER_C5_EP/AFTER_C5_NO_EP used to be one of the pairs above, back when the
// key covered any en passant square the FEN carried, whether or not a pawn
// could reach it. S042 changed that to X-FEN: a pawn of the side to move must
// stand where it attacks the target. AFTER_C5_EP's fourth field (c6) fails
// that test -- no white pawn on b5 or d5, confirmed with python-chess 1.11.2
// (`has_pseudo_legal_en_passant()` is False, `fen(en_passant="xfen")` prints
// "-"). load_FEN's sanitiser now clears it on load, so AFTER_C5_EP and
// AFTER_C5_NO_EP key identically -- correctly, since under X-FEN they are the
// same position (S042/DEC-187, the owner's 2026-09-08 choice). The intent the
// old pairing encoded, "the en passant field always separates", does not
// survive that convention; this case restates it as its replacement, the two
// rows collapsing to one.
TEST_CASE(
    "an en passant square with no capturing pawn no longer separates two "
    "positions")
{
  const std::string first = row(AFTER_C5_EP, "1.0", "31", "24");
  const std::string second = row(AFTER_C5_NO_EP, "0.0", "-58", "24");

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe(corpus({first, second}), false, &out, &stats, &error));

  CHECK(stats.rows_written == 1);
  CHECK(stats.rows_dropped == 1);

  // The first row survives, byte for byte, the same rule DEC-065 states above.
  CHECK(out == first + "\n");
}


TEST_CASE("distinct positions all survive, in input order")
{
  const std::vector<std::string> rows = {
      row(START, "1.0", "31", "24"),
      row(AFTER_E4, "1.0", "28", "24"),
      row(AFTER_C5_EP, "0.5", "12", "24"),
  };

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe(corpus(rows), false, &out, &stats, &error));

  CHECK(stats.rows_read == 3);
  CHECK(stats.rows_written == 3);
  CHECK(stats.rows_dropped == 0);
  CHECK(out == corpus(rows));
}


TEST_CASE("the same input twice gives the same output")
{
  const std::vector<std::string> rows = {
      row(START, "1.0", "31", "24"),
      row(AFTER_E4, "1.0", "28", "24"),
      row(START_LATER_CLOCKS, "0.0", "-58", "24"),
      row(AFTER_C5_EP, "0.5", "12", "24"),
      row(AFTER_E4, "0.5", "3", "24"),
  };

  std::string first_out;
  std::string second_out;
  std::string error;
  corpus_dedupe::stats_t first_stats;
  corpus_dedupe::stats_t second_stats;

  REQUIRE(dedupe(corpus(rows), false, &first_out, &first_stats, &error));
  REQUIRE(dedupe(corpus(rows), false, &second_out, &second_stats, &error));

  CHECK(first_out == second_out);
  CHECK(first_stats.rows_written == second_stats.rows_written);
  CHECK(first_stats.rows_dropped == second_stats.rows_dropped);

  // And the run that proves the two are not both empty.
  CHECK(first_stats.rows_read == 5);
  CHECK(first_stats.rows_written == 3);
  CHECK(first_stats.rows_dropped == 2);
}


TEST_CASE("the repeat histogram counts what repeated and how often")
{
  const std::vector<std::string> rows = {
      row(START, "1.0", "31", "24"),
      row(START_LATER_CLOCKS, "0.0", "-58", "24"),
      row(START, "0.5", "4", "24"),
      row(AFTER_E4, "1.0", "28", "24"),
      row(AFTER_E4, "0.0", "-11", "24"),
      row(AFTER_C5_EP, "0.5", "12", "24"),
  };

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe(corpus(rows), false, &out, &stats, &error));

  CHECK(stats.rows_read == 6);
  CHECK(stats.rows_written == 3);
  CHECK(stats.rows_dropped == 3);
  CHECK(stats.most_repeated == 3);

  CHECK(stats.buckets[0] == 1);  // AFTER_C5_EP, once
  CHECK(stats.buckets[1] == 1);  // AFTER_E4, twice
  CHECK(stats.buckets[2] == 1);  // the start position, three times
  CHECK(stats.buckets[3] == 0);
  CHECK(stats.buckets[4] == 0);
  CHECK(stats.buckets[5] == 0);
}


TEST_CASE("blank lines are skipped and are not written through")
{
  const std::string first = row(START, "1.0", "31", "24");

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe("\n" + first + "\n\n", false, &out, &stats, &error));

  CHECK(stats.rows_read == 1);
  CHECK(stats.blank_skipped == 2);
  CHECK(out == first + "\n");
}


TEST_CASE("a row with fewer than four fields is refused")
{
  const std::string good = row(START, "1.0", "31", "24");
  const std::string short_row = START + "\t1.0\t31";

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  CHECK_FALSE(dedupe(corpus({good, short_row}), false, &out, &stats, &error));
  CHECK(error.find("four fields") != std::string::npos);
  CHECK(error.find(short_row) != std::string::npos);

  // Refused after reading it, not before: the count is what the report prints.
  CHECK(stats.rows_read == 2);
}


TEST_CASE("a row whose FEN does not load is refused")
{
  const std::string good = row(START, "1.0", "31", "24");
  const std::string broken = row("not a fen at all", "1.0", "31", "24");

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  CHECK_FALSE(dedupe(corpus({good, broken}), false, &out, &stats, &error));
  CHECK(error.find("loadable FEN") != std::string::npos);
  CHECK(stats.rows_read == 2);
}


TEST_CASE("hashed_fields reads the four fields the key covers")
{
  std::string start_fields;
  std::string later_fields;
  std::string castling_fields;

  REQUIRE(corpus_dedupe::hashed_fields(START, &start_fields));
  REQUIRE(corpus_dedupe::hashed_fields(START_LATER_CLOCKS, &later_fields));
  REQUIRE(
      corpus_dedupe::hashed_fields(START_NO_WHITE_QUEENSIDE, &castling_fields));

  CHECK(start_fields == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -");

  // What --verify compares. Equal across a clock-only difference, so a
  // collision report cannot be produced by the clocks; different when a field
  // the key covers moves, so the comparison is not a constant.
  CHECK(start_fields == later_fields);
  CHECK(start_fields != castling_fields);

  std::string missing;
  CHECK_FALSE(corpus_dedupe::hashed_fields("8/8/8 w", &missing));
}


TEST_CASE("--verify changes no output and reports no collision")
{
  const std::vector<std::string> rows = {
      row(START, "1.0", "31", "24"),
      row(START_LATER_CLOCKS, "0.0", "-58", "24"),
      row(AFTER_E4, "1.0", "28", "24"),
  };

  std::string plain_out;
  std::string verified_out;
  std::string error;
  corpus_dedupe::stats_t plain_stats;
  corpus_dedupe::stats_t verified_stats;

  REQUIRE(dedupe(corpus(rows), false, &plain_out, &plain_stats, &error));
  REQUIRE(dedupe(corpus(rows), true, &verified_out, &verified_stats, &error));

  CHECK(plain_out == verified_out);
  CHECK(verified_stats.rows_dropped == 1);
  CHECK(verified_stats.collisions == 0);
}


namespace
{

// 2026-08-13_adversarial-F08's form: one position under three FENs that differ
// only in the en passant field. The first two are the FENs F08 printed for
// 1.d4 d5 2.c4 and 1.c4 d5 2.d4, which make_move wrote before S042 with the
// square of whichever pawn pushed last; the third carries none. No square here
// is one a black pawn can take on, confirmed with python-chess 1.11.2, re-run
// for S327: `has_pseudo_legal_en_passant()` is False for all three and
// `fen(en_passant="xfen")` prints "-" for each, as S325 recorded in
// `adocs/data/S325_ep_key.txt`. So since S042 the three key identically, and
// before S327 `--verify` compared their raw text and counted two collisions --
// which is what that file read on `1bb3080`.
const std::string F08_C3 =
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2";
const std::string F08_D3 =
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq d3 0 2";
const std::string F08_NO_EP =
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq - 0 2";


// The placement field of a FEN for a board indexed from a8, as the engine's
// is: square 0 is a8, 7 is h8, 56 is a1.
std::string placement(const piece_t squares[64])
{
  static const char names[] = "PNBRQKpnbrqk";
  std::string text;

  for (int row = 0; row < 8; ++row) {
    int empty = 0;

    for (int file = 0; file < 8; ++file) {
      const piece_t piece = squares[row * 8 + file];

      if (piece == EMPTY) {
        ++empty;
        continue;
      }

      if (empty > 0) { text += static_cast<char>('0' + empty); }

      empty = 0;
      text += names[piece];
    }

    if (empty > 0) { text += static_cast<char>('0' + empty); }

    if (row < 7) { text += '/'; }
  }

  return text;
}


struct collision_t
{
  std::string left;
  std::string right;
};


// Two FENs load_FEN accepts whose keys are equal and whose placements are not:
// a real 64-bit collision, for the path of run() only a collision reaches,
// which no real corpus can be relied on to carry. Built rather than searched
// for. A key is the xor of the randoms of what a position holds, so two
// positions share one exactly when the randoms of the pieces they do not share
// xor to zero, and among any 65 values of 64 bits some subset does; Gaussian
// elimination over GF(2) finds one. The randoms are the engine's own, from a
// game initialize_game_const_data() filled as run() fills its own, so nothing
// in the tool is touched.
//
// The pool keeps every split of the subset loadable. White is to move, the
// kings stand on a1 and h8 in both, and everything else is a knight: black ones
// anywhere, because a black piece cannot check the black king, and white ones
// off f7 and g6, the two squares a knight attacks h8 from. Of the attack
// questions load_FEN asks only whether the side not to move is in check, so a
// black knight checking the white king is allowed. What can still refuse a
// split is a 17th piece of one colour, and that is checked here and asserted by
// the caller through load_FEN itself.
bool plant_collision(collision_t* planted)
{
  struct candidate_t
  {
    piece_t piece;
    index_t square;
  };

  std::vector<candidate_t> pool;

  for (index_t square = a8; square <= h1; ++square) {
    if (square == a1 || square == h8) { continue; }

    pool.push_back({B_KNIGHT, square});

    if (square != f7 && square != g6) { pool.push_back({W_KNIGHT, square}); }
  }

  // 122 candidates, and the masks below have room for 128.
  if (pool.size() > 128) { return false; }

  game_t game;
  initialize_game_const_data(&game);

  uint64_t basis[64] = {};
  std::bitset<128> made_of[64];
  std::bitset<128> subset;

  for (size_t i = 0; i < pool.size() && subset.none(); ++i) {
    uint64_t value =
        game.hash_randoms.piece_randoms[pool[i].piece][pool[i].square];
    std::bitset<128> combination;
    combination.set(i);

    bool independent = false;

    for (int bit = 63; bit >= 0; --bit) {
      if (((value >> bit) & 1ULL) == 0) { continue; }

      if (basis[bit] == 0) {
        basis[bit] = value;
        made_of[bit] = combination;
        independent = true;
        break;
      }

      value ^= basis[bit];
      combination ^= made_of[bit];
    }

    // Reduced to zero: this candidate and the ones `combination` names xor to
    // zero, which is the subset.
    if (!independent) { subset = combination; }
  }

  if (subset.none()) { return false; }

  // Split the subset between the two placements, each colour as evenly as it
  // goes. A square holds at most two candidates, one of each colour, so when
  // the subset takes both the second goes to the side the first did not.
  piece_t left[64];
  piece_t right[64];
  size_t left_count[2] = {0, 0};
  size_t right_count[2] = {0, 0};

  for (index_t square = a8; square <= h1; ++square) {
    left[square] = EMPTY;
    right[square] = EMPTY;
  }

  for (size_t i = 0; i < pool.size(); ++i) {
    if (!subset.test(i)) { continue; }

    const index_t square = pool[i].square;
    const int colour = (pool[i].piece == W_KNIGHT) ? WHITE : BLACK;

    bool to_left = left_count[colour] <= right_count[colour];

    if (to_left && left[square] != EMPTY) { to_left = false; }
    if (!to_left && right[square] != EMPTY) { to_left = true; }

    if (to_left) {
      left[square] = pool[i].piece;
      left_count[colour]++;
    } else {
      right[square] = pool[i].piece;
      right_count[colour]++;
    }
  }

  // Sixteen a colour with its king, which load_FEN refuses past.
  for (const int colour : {WHITE, BLACK}) {
    if (left_count[colour] > 15 || right_count[colour] > 15) { return false; }
  }

  left[a1] = W_KING;
  right[a1] = W_KING;
  left[h8] = B_KING;
  right[h8] = B_KING;

  planted->left = placement(left) + " w - - 0 1";
  planted->right = placement(right) + " w - - 0 1";
  return true;
}

}  // namespace


TEST_CASE(
    "--verify reads an en passant square no pawn can take as the key does")
{
  // The two preconditions, established rather than assumed: the rows disagree
  // as text in the en passant field, which is what a raw comparison counts,
  // and the engine gives them one key, which is what puts them in one bucket.
  CHECK(F08_C3 != F08_D3);
  CHECK(F08_C3 != F08_NO_EP);
  CHECK(F08_D3 != F08_NO_EP);

  game_t game;
  initialize_game_const_data(&game);

  uint64_t c3_key = 0;
  uint64_t d3_key = 0;
  uint64_t none_key = 0;

  REQUIRE(corpus_dedupe::position_key(F08_C3, &game, &c3_key));
  REQUIRE(corpus_dedupe::position_key(F08_D3, &game, &d3_key));
  REQUIRE(corpus_dedupe::position_key(F08_NO_EP, &game, &none_key));
  REQUIRE(c3_key == d3_key);
  REQUIRE(c3_key == none_key);

  // What --verify compares, read as the key reads it: the square load_FEN
  // keeps, which is none.
  std::string c3_fields;
  std::string d3_fields;
  std::string none_fields;

  REQUIRE(corpus_dedupe::hashed_fields(F08_C3, &c3_fields));
  REQUIRE(corpus_dedupe::hashed_fields(F08_D3, &d3_fields));
  REQUIRE(corpus_dedupe::hashed_fields(F08_NO_EP, &none_fields));

  CHECK(c3_fields ==
        "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq -");
  CHECK(d3_fields == c3_fields);
  CHECK(none_fields == c3_fields);

  const std::vector<std::string> rows = {
      row(F08_C3, "0.5", "10", "24"),
      row(F08_D3, "0.5", "10", "24"),
      row(F08_NO_EP, "0.5", "10", "24"),
  };

  std::string out;
  std::string error;
  corpus_dedupe::stats_t stats;

  REQUIRE(dedupe(corpus(rows), true, &out, &stats, &error));

  CHECK(stats.rows_written == 1);
  CHECK(stats.rows_dropped == 2);
  CHECK(stats.collisions == 0);
  CHECK(out == rows[0] + "\n");
}


TEST_CASE("hashed_fields keeps an en passant square a pawn can take")
{
  // AFTER_D5_EP's d6 is one White's e5 pawn can take (python-chess, above), so
  // the key keeps it and the fields must too: a hashed_fields() that wrote "-"
  // for every square would pass the case above and fail this one.
  std::string with_square;
  std::string without_square;

  REQUIRE(corpus_dedupe::hashed_fields(AFTER_D5_EP, &with_square));
  REQUIRE(corpus_dedupe::hashed_fields(AFTER_D5_NO_EP, &without_square));

  CHECK(with_square ==
        "rnbqkbnr/ppp2ppp/4p3/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6");
  CHECK(with_square != without_square);
}


TEST_CASE("--verify counts a real 64-bit collision")
{
  collision_t planted;
  REQUIRE(plant_collision(&planted));

  // The collision is the engine's, not this file's arithmetic: both FENs load,
  // load_FEN's key is the same for the two, and the placements differ.
  game_t game;
  initialize_game_const_data(&game);

  uint64_t left_key = 0;
  uint64_t right_key = 0;

  REQUIRE(corpus_dedupe::position_key(planted.left, &game, &left_key));
  REQUIRE(corpus_dedupe::position_key(planted.right, &game, &right_key));
  REQUIRE(left_key == right_key);

  std::string left_fields;
  std::string right_fields;

  REQUIRE(corpus_dedupe::hashed_fields(planted.left, &left_fields));
  REQUIRE(corpus_dedupe::hashed_fields(planted.right, &right_fields));
  REQUIRE(left_fields != right_fields);

  const std::string first = row(planted.left, "1.0", "31", "24");
  const std::string second = row(planted.right, "0.0", "-58", "24");

  std::string plain_out;
  std::string verified_out;
  std::string error;
  corpus_dedupe::stats_t plain_stats;
  corpus_dedupe::stats_t verified_stats;

  REQUIRE(
      dedupe(corpus({first, second}), false, &plain_out, &plain_stats, &error));
  REQUIRE(dedupe(corpus({first, second}), true, &verified_out, &verified_stats,
                 &error));

  // The cost the header names: a collision drops one distinct position, with
  // or without --verify, and only --verify says so.
  CHECK(plain_out == first + "\n");
  CHECK(verified_out == plain_out);
  CHECK(plain_stats.collisions == 0);
  CHECK(verified_stats.rows_dropped == 1);
  CHECK(verified_stats.collisions == 1);
}
