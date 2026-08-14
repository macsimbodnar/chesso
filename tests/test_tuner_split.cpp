#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "tuner_split.hpp"

// The tuner holds out a tenth of its corpus and reads the error on it as
// generalisation. Until S066 it shuffled a **row** index and took the last
// tenth of that, while `tools/datagen.cpp` writes a game's ~92 rows
// contiguously and gives every one of them the same label -- the game's result.
// Nearly every game therefore had rows on both sides of the cut, so the
// held-out error was largely measuring memorisation: 119360 of 119999
// games, 99.47 %, over the 11003693-row `selfplay_v2.tsv`.
//
// What this file holds:
//
//   1. the fixture is a corpus of multi-row games whose plies advance inside a
//      game -- the precondition for everything below, and the property that
//      makes the reconstruction sound
//   2. a row-level split cuts nearly all of those games in two -- the
//      precondition that stops case 3 passing on a corpus nothing could cut
//   3. the game-level split cuts none of them
//   4. two games the ply cannot tell apart merge into one block, and a merged
//      block still lands wholly on one side
//   5. the ply catches a boundary the FEN's move number alone misses
//   6. one row per game reproduces the old row-level split exactly, which is
//      what pins the change to the grouping
//   7. the held-out fraction lands within the largest block of --validation
//   8. the last block is never held out, so there is always something to train
//      on -- the one failure mode this change introduced, found by this test
//   9. the same seed gives the same split and a different seed a different one
//  10. a row with no usable FEN move number is refused rather than guessed at
//
// Cases 1 and 2 are the non-vacuity guard. A fixture of one-row games, or one
// small enough that a row-level split happened to miss every game, would let
// case 3 pass over nothing.

namespace
{

constexpr double VALIDATION = 0.1;

// A corpus of games, each a contiguous run of rows, written to a file and read
// back through the same parser the tuner uses. `game_of_row` is the ground
// truth the file itself does not carry, which is the whole difficulty S066
// works around.
struct fixture_t
{
  std::vector<uint32_t> ply;  // one per row, as row_ply() parsed it
  std::vector<uint32_t> game_of_row;
  std::vector<size_t> rows_of_game;
  size_t merged_pairs = 0;    // adjacent games with no ply boundary between
  size_t fullmove_equal = 0;  // ... where the move number does not descend
  size_t path_after = 0;      // 1 if the fixture file outlived the read
};


std::string fixture_path()
{
  return (std::filesystem::temp_directory_path() /
          "chesso_test_tuner_split.tsv")
      .string();
}


// One datagen-shaped row. Only fields 2 and 6 of the FEN are read by the
// splitter; the placement is a real one so the row stays something any reader
// of the four-column format could take.
std::string row_text(uint32_t ply, double result)
{
  const bool black = (ply % 2) == 1;
  const uint32_t fullmove = (ply - (black ? 1 : 0)) / 2 + 1;

  char line[256];
  snprintf(line, sizeof(line),
           "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR %s KQkq - 0 %u\t%.1f"
           "\t%d\t%d\n",
           black ? "b" : "w", fullmove, result, 12, 18);

  return line;
}


// Builds the fixture, reads it back through tuner_split::row_ply(), and removes
// it. The file exists only for the duration of the read, so no case can leave
// one behind and no case depends on another having cleaned up.
//
// The shape is deliberate. Games 0..29 with 40 to 100 rows each, plies
// advancing by one inside a game, and three kinds of join between adjacent
// games:
//
//   - the ordinary one: the next game starts near its opening ply, so both the
//     ply and the move number fall
//   - game 21, which starts one ply *below* where game 20 ended with Black to
//     move: same move number, so no descent in the move number, but the ply
//     went backwards and is caught
//   - game 11, which starts two plies *above* where game 10 ended: neither the
//     ply nor the move number falls, so the boundary is missed and the two
//     games merge into one block
fixture_t build_fixture()
{
  fixture_t fixture;

  const size_t games = 30;
  std::vector<uint32_t> first_ply(games, 0);
  std::vector<size_t> rows(games, 0);

  for (size_t g = 0; g < games; ++g) {
    rows[g] = 40 + (g * 37) % 61;
  }

  // Game 20 has to end with Black to move for the join at 21 to keep the move
  // number. first_ply[20] is 8 + 20 % 5 = 8, so the parity is rows[20]'s.
  if ((8 + (20 % 5) + rows[20] - 1) % 2 == 0) { rows[20]++; }

  for (size_t g = 0; g < games; ++g) {
    if (g == 0) {
      // datagen plays 8 uniform random plies first, so the earliest position a
      // game can record is White to move at move 5 -- ply 8.
      first_ply[g] = 8;
    } else if (g == 11) {
      first_ply[g] = static_cast<uint32_t>(first_ply[10] + rows[10] - 1) + 2;
    } else if (g == 21) {
      first_ply[g] = static_cast<uint32_t>(first_ply[20] + rows[20] - 1) - 1;
    } else {
      first_ply[g] = static_cast<uint32_t>(8 + (g % 5));
    }
  }

  const std::string path = fixture_path();

  {
    std::ofstream out(path, std::ios::trunc);

    for (size_t g = 0; g < games; ++g) {
      // 1.0, 0.5, 0.0 in turn, so the label is shared inside a game and differs
      // across most joins -- as datagen's is.
      const double result = (g % 3 == 0) ? 1.0 : ((g % 3 == 1) ? 0.5 : 0.0);

      for (size_t r = 0; r < rows[g]; ++r) {
        out << row_text(first_ply[g] + static_cast<uint32_t>(r), result);
      }
    }
  }

  {
    std::ifstream in(path);
    std::string line;
    size_t g = 0;
    size_t used = 0;

    while (std::getline(in, line)) {
      uint32_t ply = 0;

      if (!tuner_split::row_ply(line, &ply)) { break; }

      while (g < games && used == rows[g]) {
        g++;
        used = 0;
      }

      if (g >= games) { break; }

      fixture.ply.push_back(ply);
      fixture.game_of_row.push_back(static_cast<uint32_t>(g));
      used++;
    }
  }

  fixture.rows_of_game = rows;

  for (size_t g = 1; g < games; ++g) {
    const uint32_t last =
        first_ply[g - 1] + static_cast<uint32_t>(rows[g - 1]) - 1;
    const uint32_t next = first_ply[g];

    if (next > last) { fixture.merged_pairs++; }

    const bool last_black = (last % 2) == 1;
    const bool next_black = (next % 2) == 1;
    const uint32_t last_fullmove = (last - (last_black ? 1 : 0)) / 2 + 1;
    const uint32_t next_fullmove = (next - (next_black ? 1 : 0)) / 2 + 1;

    if (next <= last && next_fullmove >= last_fullmove) {
      fixture.fullmove_equal++;
    }
  }

  std::error_code error;
  std::filesystem::remove(path, error);
  fixture.path_after = std::filesystem::exists(path) ? 1 : 0;

  return fixture;
}


// `tools/tuner.cpp` at dd57c3c, transcribed: shuffle a row index, hold out the
// last `validation` of it. Kept here so the two splitters can be compared, and
// so case 6 has something to be identical to.
std::pair<std::vector<uint32_t>, size_t> row_level_index(size_t rows,
                                                         double validation,
                                                         uint64_t seed)
{
  std::vector<uint32_t> index(rows);
  for (size_t i = 0; i < index.size(); ++i) {
    index[i] = static_cast<uint32_t>(i);
  }

  std::mt19937_64 rng(seed);
  std::shuffle(index.begin(), index.end(), rng);

  const size_t validation_count =
      static_cast<size_t>(static_cast<double>(rows) * validation);

  return {index, rows - validation_count};
}


// Which rows a split held out, from the index and the cut.
std::vector<uint8_t> held_out(const std::vector<uint32_t>& index,
                              size_t train_count,
                              size_t rows)
{
  std::vector<uint8_t> held(rows, 0);

  for (size_t i = train_count; i < index.size(); ++i) {
    held[index[i]] = 1;
  }

  return held;
}


// Games with rows on both sides of the cut. This is the number S066 drives to
// zero.
size_t straddling(const fixture_t& fixture, const std::vector<uint8_t>& held)
{
  std::vector<uint8_t> has_train(fixture.rows_of_game.size(), 0);
  std::vector<uint8_t> has_held(fixture.rows_of_game.size(), 0);

  for (size_t row = 0; row < held.size(); ++row) {
    const uint32_t game = fixture.game_of_row[row];

    if (held[row]) {
      has_held[game] = 1;
    } else {
      has_train[game] = 1;
    }
  }

  size_t count = 0;

  for (size_t g = 0; g < has_train.size(); ++g) {
    if (has_train[g] && has_held[g]) { count++; }
  }

  return count;
}


// Blocks under the boundary the FEN's move number alone gives, which is the
// weaker detector case 5 measures the ply against.
size_t fullmove_blocks(const std::vector<uint32_t>& ply)
{
  if (ply.empty()) { return 0; }

  const auto fullmove = [](uint32_t p) { return (p - (p % 2)) / 2 + 1; };

  size_t blocks = 1;

  for (size_t i = 1; i < ply.size(); ++i) {
    if (fullmove(ply[i]) < fullmove(ply[i - 1])) { blocks++; }
  }

  return blocks;
}

}  // namespace


TEST_CASE("the fixture is a corpus of multi-row games")
{
  const fixture_t fixture = build_fixture();

  // Every row of the file parsed. A fixture the parser gave up on part way
  // through would make every count below smaller than it looks.
  REQUIRE(fixture.rows_of_game.size() == 30);
  REQUIRE(fixture.ply.size() == fixture.game_of_row.size());

  size_t expected_rows = 0;
  for (const size_t count : fixture.rows_of_game) {
    expected_rows += count;
  }

  REQUIRE(fixture.ply.size() == expected_rows);

  // Multi-row games, or there is nothing a split could cut in two and case 3
  // passes over an empty question.
  size_t smallest = fixture.ply.size();
  for (const size_t count : fixture.rows_of_game) {
    smallest = std::min(smallest, count);
  }

  CHECK_MESSAGE(smallest >= 2, "the fixture's smallest game has "
                                   << smallest
                                   << " rows; a corpus of "
                                      "one-row games cannot "
                                      "straddle anything");

  // The ply advances inside a game. This is the property the reconstruction
  // rests on -- it is why a non-advancing ply can only be a boundary, and so
  // why the split can never cut a game in half.
  size_t non_advancing = 0;

  for (size_t i = 1; i < fixture.ply.size(); ++i) {
    if (fixture.game_of_row[i] != fixture.game_of_row[i - 1]) { continue; }
    if (fixture.ply[i] <= fixture.ply[i - 1]) { non_advancing++; }
  }

  CHECK_MESSAGE(non_advancing == 0,
                non_advancing << " rows do not advance the ply on their "
                                 "predecessor inside the same game");

  // And the fixture cleaned up after itself.
  CHECK_MESSAGE(fixture.path_after == 0,
                "the fixture file outlived the read: " << fixture_path());
}


TEST_CASE("a row-level split cuts nearly every game in two")
{
  const fixture_t fixture = build_fixture();
  const size_t rows = fixture.ply.size();
  const size_t games = fixture.rows_of_game.size();

  REQUIRE(rows > 0);
  REQUIRE(games >= 2);

  const auto old_split = row_level_index(rows, VALIDATION, 1);
  const size_t cut =
      straddling(fixture, held_out(old_split.first, old_split.second, rows));

  // The precondition for the case below. If a row-level split cut no game here,
  // a game-level split cutting none would say nothing at all.
  CHECK_MESSAGE(cut >= games / 2,
                "only " << cut << " of " << games
                        << " games straddle a row-level split; the fixture is "
                           "too small or its games too short for the next case "
                           "to mean anything");
}


TEST_CASE("no game lands on both sides of the game-level split")
{
  const fixture_t fixture = build_fixture();
  const size_t rows = fixture.ply.size();

  REQUIRE(rows > 0);

  std::vector<uint32_t> starts;
  tuner_split::game_starts(fixture.ply, &starts);

  for (const uint64_t seed : {uint64_t{1}, uint64_t{2}, uint64_t{20260814}}) {
    std::vector<uint32_t> index;
    const size_t train_count =
        tuner_split::split(starts, rows, VALIDATION, seed, &index);

    // Every row exactly once. A splitter that dropped rows would shrink the
    // corpus silently and still hold no game on both sides.
    REQUIRE(index.size() == rows);
    std::vector<uint32_t> sorted = index;
    std::sort(sorted.begin(), sorted.end());
    for (size_t i = 0; i < sorted.size(); ++i) {
      REQUIRE(sorted[i] == static_cast<uint32_t>(i));
    }

    const size_t cut = straddling(fixture, held_out(index, train_count, rows));

    CHECK_MESSAGE(cut == 0, cut << " games straddle the game-level split at "
                                   "seed "
                                << seed);
  }
}


TEST_CASE("two games the ply cannot separate merge, and the merge is safe")
{
  const fixture_t fixture = build_fixture();
  const size_t rows = fixture.ply.size();
  const size_t games = fixture.rows_of_game.size();

  // The fixture has to contain the failure mode or this case tests nothing:
  // one join where the next game's first recorded ply is later than the
  // previous game's last, so nothing marks the boundary.
  REQUIRE(fixture.merged_pairs == 1);

  std::vector<uint32_t> starts;
  tuner_split::game_starts(fixture.ply, &starts);

  // One missed boundary, so one block fewer than there are games.
  CHECK(starts.size() == games - fixture.merged_pairs);

  // And the cost of the miss is granularity, not contamination: the merged
  // block is indivisible, so both games in it go the same way. Case 3 asserts
  // zero straddling over the same fixture, which is that statement measured.
  std::vector<uint32_t> index;
  const size_t train_count =
      tuner_split::split(starts, rows, VALIDATION, 1, &index);
  const std::vector<uint8_t> held = held_out(index, train_count, rows);

  CHECK(straddling(fixture, held) == 0);

  // The two merged games are on the same side, which is what "indivisible"
  // means here. Games 10 and 11 by construction.
  uint8_t side_10 = 2;
  uint8_t side_11 = 2;

  for (size_t row = 0; row < rows; ++row) {
    if (fixture.game_of_row[row] == 10) { side_10 = held[row]; }
    if (fixture.game_of_row[row] == 11) { side_11 = held[row]; }
  }

  REQUIRE(side_10 != 2);
  REQUIRE(side_11 != 2);
  CHECK(side_10 == side_11);
}


TEST_CASE("the ply catches a boundary the FEN move number alone misses")
{
  const fixture_t fixture = build_fixture();

  // The fixture has to contain the case: a join where the ply falls and the
  // move number does not, which happens when a game ends with Black to move and
  // the next starts at the same move number with White to move.
  REQUIRE(fixture.fullmove_equal == 1);

  std::vector<uint32_t> starts;
  tuner_split::game_starts(fixture.ply, &starts);

  CHECK_MESSAGE(
      fullmove_blocks(fixture.ply) + fixture.fullmove_equal == starts.size(),
      "move number blocks "
          << fullmove_blocks(fixture.ply) << ", ply blocks " << starts.size()
          << ", joins the move number cannot see " << fixture.fullmove_equal);
}


TEST_CASE("one row per game reproduces the old row-level split exactly")
{
  const size_t rows = 977;

  // A ply that falls on every row makes every row its own block, which is the
  // one-game-per-row corpus. game_starts() has to see all of them.
  std::vector<uint32_t> ply(rows);
  for (size_t i = 0; i < rows; ++i) {
    ply[i] = static_cast<uint32_t>(rows - i);
  }

  std::vector<uint32_t> starts;
  tuner_split::game_starts(ply, &starts);
  REQUIRE(starts.size() == rows);

  // With one row per block the block shuffle is the row shuffle, and holding
  // out whole blocks from the end of it is holding out the last rows of it. The
  // index and the cut have to come out identical to what tuner.cpp did before
  // S066 -- which is what says this change moved the grouping and nothing else.
  for (const uint64_t seed : {uint64_t{1}, uint64_t{7}, uint64_t{20260814}}) {
    for (const double validation : {0.0, 0.1, 0.5}) {
      std::vector<uint32_t> index;
      const size_t train_count =
          tuner_split::split(starts, rows, validation, seed, &index);

      const auto old_split = row_level_index(rows, validation, seed);

      CHECK(train_count == old_split.second);
      CHECK(index == old_split.first);
    }
  }
}


TEST_CASE("the held-out fraction lands within one block of --validation")
{
  const fixture_t fixture = build_fixture();
  const size_t rows = fixture.ply.size();

  std::vector<uint32_t> starts;
  tuner_split::game_starts(fixture.ply, &starts);

  size_t largest = 0;
  for (size_t b = 0; b < starts.size(); ++b) {
    const size_t end = (b + 1 < starts.size()) ? starts[b + 1] : rows;
    largest = std::max(largest, end - starts[b]);
  }

  REQUIRE(largest > 1);

  for (const double validation : {0.05, 0.1, 0.25}) {
    std::vector<uint32_t> index;
    const size_t train_count =
        tuner_split::split(starts, rows, validation, 1, &index);
    const size_t held = rows - train_count;
    const size_t target =
        static_cast<size_t>(static_cast<double>(rows) * validation);

    // A block is indivisible, so the last one taken can overshoot the target by
    // up to its own length. It can never undershoot: blocks are taken until the
    // target is reached.
    CHECK(held >= target);
    CHECK_MESSAGE(held - target < largest,
                  "held out " << held << " rows against a target of " << target
                              << ", overshooting by more than the largest "
                                 "block ("
                              << largest << ")");
  }
}


TEST_CASE("the last block is never held out")
{
  // A block is indivisible, so a corpus of one game cannot give up part of
  // itself. Giving up all of it leaves gradient() dividing by a count of zero,
  // and that was measured, not imagined: `build/tools/tuner` over a two-row
  // corpus at `--validation 0.5` printed `0 train, 2 validation (100.0000%)`
  // and then `epoch 1  train 0.000000  validation -nan`. The row-level split
  // this replaced could always cut one row off a game, so the failure mode came
  // in with the game-level split and is clamped in split() rather than left to
  // the caller. Two-row corpora are a real case here -- S040 and S041 both
  // measured the tuner's `--only` groups over one.
  //
  // The clamp is asserted three ways, not as "something survived": one whole
  // block always survives, one game keeps everything, and at --validation 1.0
  // exactly one block survives and no more.
  for (const size_t games : {size_t{1}, size_t{2}, size_t{3}}) {
    for (const size_t rows_each : {size_t{1}, size_t{2}, size_t{40}}) {
      // Every block is the same length here, which is what lets the assertions
      // below name an exact row count rather than a bound. Each game's plies
      // advance and each game restarts below where the last one ended, so
      // game_starts() finds exactly `games` blocks.
      std::vector<uint32_t> ply;

      for (size_t g = 0; g < games; ++g) {
        for (size_t r = 0; r < rows_each; ++r) {
          ply.push_back(static_cast<uint32_t>(8 + r));
        }
      }

      const size_t rows = ply.size();

      std::vector<uint32_t> starts;
      tuner_split::game_starts(ply, &starts);

      // Precondition: the corpus really is `games` blocks of `rows_each` rows.
      // Without it a case meant to be about one game could be about many, and
      // the exact counts below would be measuring a different corpus.
      REQUIRE(starts.size() == games);
      REQUIRE(rows == games * rows_each);

      for (const double validation : {0.1, 0.5, 0.9, 0.99, 1.0}) {
        std::vector<uint32_t> index;
        const size_t train_count =
            tuner_split::split(starts, rows, validation, 1, &index);

        REQUIRE(index.size() == rows);

        // One whole block survives, whatever was asked for.
        CHECK_MESSAGE(train_count >= rows_each,
                      "fewer than one whole block left to train on: "
                          << train_count << " rows of " << rows_each << ", "
                          << games << " games at --validation " << validation);

        // One game gives up nothing, because giving up anything is giving up
        // everything.
        if (games == 1) {
          CHECK_MESSAGE(train_count == rows,
                        "a one-game corpus held out "
                            << rows - train_count << " of its " << rows
                            << " rows at --validation " << validation);
        }

        // And the clamp keeps one block, not two: asking for all of it holds
        // out everything except the last block.
        if (validation >= 1.0) {
          CHECK_MESSAGE(train_count == rows_each,
                        "at --validation 1.0 the clamp kept "
                            << train_count << " rows rather than one block of "
                            << rows_each);
        }
      }
    }
  }
}


TEST_CASE("the split is deterministic and seeded")
{
  const fixture_t fixture = build_fixture();
  const size_t rows = fixture.ply.size();

  std::vector<uint32_t> starts;
  tuner_split::game_starts(fixture.ply, &starts);

  std::vector<uint32_t> first;
  std::vector<uint32_t> again;
  std::vector<uint32_t> other;

  const size_t train_first =
      tuner_split::split(starts, rows, VALIDATION, 1, &first);
  const size_t train_again =
      tuner_split::split(starts, rows, VALIDATION, 1, &again);
  const size_t train_other =
      tuner_split::split(starts, rows, VALIDATION, 2, &other);

  CHECK(train_first == train_again);
  CHECK(first == again);

  // A seed that changes nothing would make `--seed` a lie and a repeated fit
  // impossible to vary.
  CHECK(first != other);
  (void)train_other;
}


TEST_CASE("a row with no usable FEN move number is refused")
{
  uint32_t ply = 0;

  // What datagen writes, so the negative cases below are not failing for a
  // reason that has nothing to do with the field under test.
  REQUIRE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 5\t1.0\t12\t18",
      &ply));
  CHECK(ply == 8);

  REQUIRE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 5\t1.0\t12\t18",
      &ply));
  CHECK(ply == 9);

  // Five FEN fields, the move number missing.
  CHECK_FALSE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0\t1.0\t12\t18",
      &ply));

  // A move number that is not a number, and one below the first move.
  CHECK_FALSE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 x\t1.0\t12\t18",
      &ply));
  CHECK_FALSE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0\t1.0\t12\t18",
      &ply));

  // A side to move that is neither colour. Nothing downstream could place the
  // row in a game, and tempo_feature() would refuse it a few lines later.
  CHECK_FALSE(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 5\t1.0\t12\t18",
      &ply));

  CHECK_FALSE(tuner_split::row_ply("", &ply));

  // The row's own tab structure is not what this reads: the fields past the FEN
  // belong to load(), and a FEN with no tab after it still has a move number.
  CHECK(tuner_split::row_ply(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 5", &ply));
}
