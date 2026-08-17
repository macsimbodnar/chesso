#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
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
// one. Nothing in the corpus format or in load_FEN checks that a pawn can
// actually take, which is `2026-08-13_adversarial-F08` and S042; the key covers
// the square either way and so does this pair.
const std::string AFTER_C5_EP =
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
const std::string AFTER_C5_NO_EP =
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2";

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

  const std::vector<pair_t> pairs = {
      {START, START_NO_WHITE_QUEENSIDE},
      {START, START_BLACK_TO_MOVE},
      {AFTER_C5_EP, AFTER_C5_NO_EP},
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
