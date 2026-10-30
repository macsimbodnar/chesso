#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cstddef>
#include <random>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "test_eval_positions.hpp"
#include "test_helpers.hpp"
#include "test_walked_counts.hpp"

// The counts evaluate() weighs, the engine's own against a second
// implementation and against placements worked out by hand. S027, S028.
//
// Until S276 this file was first the check on tools/eval_model.hpp, the
// hand-written mirror of evaluate() the tuner fitted from S028 to S262: that
// the mirror's score reproduced the engine's within the truncation bound. The
// tuner and both audit tools read the engine's own trace now
// (tools/tuner_trace.hpp), which tests/test_eval_trace.cpp holds to evaluate()
// exactly, so S276 deleted the mirror and the cases that held it. What stays
// is what guarded the engine: every count, per colour, against the walk in
// tests/test_walked_counts.hpp -- the mirror's own extractors, written
// independently from the same specification -- over the curated positions,
// and against hand-worked placements, so that a count credited to both sides
// at once, which cancels in every score, still fails here.

static game_t game;


struct counts_fixture_t
{
  counts_fixture_t() { initialize_game_const_data(&game); }
};


// The FEN's second field, the side to move, which the board reads and the
// placement does not carry; a substr that stopped at the first space would
// silently hand a caller whatever its default was.
static std::string side_to_move_of(const std::string& fen)
{
  const size_t first = fen.find(' ');
  const size_t second = fen.find(' ', first + 1);

  return fen.substr(first + 1, second - first - 1);
}


// The same FEN with the other side to move, which is the second field and
// nothing else. Castling rights, the en passant square and the two clocks are
// left exactly as they were: evaluate() reads none of them, so the two boards
// differ in the one thing the tempo term is about.
static std::string with_side_to_move(const std::string& fen, char side)
{
  const size_t first = fen.find(' ');
  std::string flipped = fen;

  flipped[first + 1] = side;

  return flipped;
}


// king_safety_feature_t's enumerators, in its order, so a failure names the
// count that disagreed instead of an index the reader has to go and look up.
static const char* const king_safety_feature_names[KS_FEATURE_COUNT] = {
    "knight attackers", "bishop attackers", "rook attackers",
    "queen attackers",  "zone attacks",     "shield near",
    "shield far",       "open file",        "half open file"};


// The same for the three pawn structure counts, in the order the weights are
// indexed.
static const char* const
    pawn_structure_feature_names[walked_counts::PAWN_STRUCTURE_COUNT] = {
        "isolated", "doubled", "backward"};


// The same for the seven piece placement counts, in the order the weights are
// indexed; the last four are S135's.
static const char* const
    piece_placement_feature_names[walked_counts::PIECE_PLACEMENT_COUNT] = {
        "bishop pair", "rook on an open file", "rook on a half open file",
        "bad bishop",  "bad bishop rammed",    "trapped minor",
        "trapped rook"};


// The curated corpus moved to tests/test_eval_positions.hpp at S100 so that
// tests/test_tuner_gradient.cpp runs over the same list. The non-vacuity cases
// below are still what holds its coverage; nothing about the list changed.
//
// Its four truncation rows were pinned here, in `truncation_positions`, until
// S276 moved them and their case to tests/test_eval_trace.cpp with the bound
// they pin: the trace's smooth score against evaluate()'s, the residual the
// mirror's score used to carry.
using test_eval_positions::positions;


// One hand-built placement and the passed pawn count per bucket it must
// produce, White minus Black. Hand-built so that the answer comes from the
// specification both implementations were written from and not from either of
// them: two extractions agreeing say nothing about whether the thing they agree
// on is what was specified.
struct passed_pawn_case_t
{
  const char* placement;
  const char* what;
  int expected[walked_counts::PASSED_PAWN_COUNT];
};


// The engine's passed_pawn_counts() takes its rows with a literal 5 and the
// walk counts its buckets off the engine's weight array. Nothing forces the
// two to be the same number, and if they ever stop being one the loops below
// would read off the end of a row rather than fail. Six until S134 folded the
// seventh-rank bucket into the pawn table (DEC-090).
static_assert(walked_counts::PASSED_PAWN_COUNT == 5,
              "the engine's passed_pawn_counts() rows are five wide");


// The engine's passed pawn rows, read through a guard row of our own. A bucket
// taken past the last one -- a pawn on its own seventh left in the passer set,
// which S134 masks out -- is written one row further than its colour's own:
// White's lands in Black's bucket 0, where the per-colour checks see it, and
// Black's lands past both rows. The third row is filled with a value no count
// can take, so a write past Black's row fails this check instead of landing on
// the caller's stack, where it is undefined behaviour and, observed, a stack
// smash in whichever case happens to call next.
static void engine_passed_counts(const board_t* board,
                                 int out[2][walked_counts::PASSED_PAWN_COUNT])
{
  constexpr int GUARD = -12345;
  int rows[3][walked_counts::PASSED_PAWN_COUNT];

  for (auto& row : rows) {
    for (int& value : row) {
      value = GUARD;
    }
  }

  passed_pawn_counts(board, rows);

  for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT; ++bucket) {
    CAPTURE(bucket);
    CHECK_MESSAGE(rows[2][bucket] == GUARD,
                  "passed_pawn_counts() wrote past Black's row");
  }

  for (int colour = 0; colour < 2; ++colour) {
    for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
         ++bucket) {
      out[colour][bucket] = rows[colour][bucket];
    }
  }
}


// One hand-built placement and the pawn structure counts it must produce, per
// colour rather than as the difference the walk reports.
//
// Per colour because a backward pawn needs an enemy pawn attacking the square
// in front of it, so the trick the passed pawn cases use -- a placement with
// only one side's pawns, where the difference *is* that side's row -- cannot
// reach the feature at all. Writing both rows out pins what that trick pins and
// the backward rows besides.
//
// Hand-built so that the answer comes from the specification both
// implementations were written from and not from either of them: two
// extractions agreeing say nothing about whether the thing they agree on is
// what was specified.
struct pawn_structure_case_t
{
  const char* placement;
  const char* what;
  int white[walked_counts::PAWN_STRUCTURE_COUNT];
  int black[walked_counts::PAWN_STRUCTURE_COUNT];
};


// The engine indexes its counts with a literal 3, the walk with a constant
// taken from the width of the engine's own weight array. Nothing forces those
// to be the same number, and if they ever stop being one the loops below would
// read off the end of a row rather than fail.
static_assert(walked_counts::PAWN_STRUCTURE_COUNT == 3,
              "the engine's pawn_structure_counts() rows are three wide");


// One hand-built placement and the piece placement counts it must produce, per
// colour rather than as the difference the walk reports.
//
// Per colour for the reason the pawn structure cases carry two rows: a miscount
// that hits both sides equally cancels in a difference, and the one-sided
// placement trick the passed pawn cases use cannot reach half-open files, which
// need an enemy pawn on the rook's file by definition.
//
// Hand-built so that the answer comes from the specification both
// implementations were written from and not from either of them: two
// extractions agreeing say nothing about whether the thing they agree on is
// what was specified.
struct piece_placement_case_t
{
  const char* placement;
  const char* what;
  int white[walked_counts::PIECE_PLACEMENT_COUNT];
  int black[walked_counts::PIECE_PLACEMENT_COUNT];
};


// The engine indexes its counts with a literal 7, the walk with a constant
// taken from the width of the engine's own weight array. Nothing forces those
// to be the same number, and if they ever stop being one the loops below would
// read off the end of a row rather than fail. Four until S134 deleted rook on
// the seventh (DEC-090), three until S135 added four.
static_assert(walked_counts::PIECE_PLACEMENT_COUNT == 7,
              "the engine's piece_placement_counts() rows are seven wide");


TEST_SUITE("eval counts: the engine against the walk and by hand")
{
  // The king safety counts themselves, per colour, engine against walk. This is
  // the case with teeth, and since S276 the only check of these counts against
  // an implementation that is not the engine's: the trace records the engine's
  // counts, so test_eval_trace's identity holds them to evaluate() and to
  // nothing else.
  //
  // Per colour and not as the White-minus-Black difference the score weighs,
  // because that difference cancels: a bug that credits both kings with one
  // extra attacker is invisible in it and shows up here. The two
  // implementations were written independently from the same specification and
  // neither reads the other -- that is what tests/test_walked_counts.hpp is
  // for, and a shared extraction would agree with itself rather than with the
  // engine.
  TEST_CASE_FIXTURE(counts_fixture_t, "the walk counts what the engine counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][KS_FEATURE_COUNT];
      king_safety_features(&game.board, engine);

      int walk[2][walked_counts::KING_SAFETY_COUNT];
      REQUIRE(walked_counts::king_safety_features_by_colour(
          fen.substr(0, fen.find(' ')), walk));

      for (int colour = 0; colour < 2; ++colour) {
        for (int feature = 0; feature < KS_FEATURE_COUNT; ++feature) {
          const std::string who = (colour == WHITE) ? "White " : "Black ";

          CHECK_MESSAGE(engine[colour][feature] == walk[colour][feature],
                        (who + king_safety_feature_names[feature]));
        }
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything:
  // a feature that is zero in every position of the corpus is a feature the
  // comparison agrees about for free. Every count has to appear, and the
  // difference the tuner fits has to take both signs, or a whole side of the
  // term is untested and a sign error in it would pass.
  TEST_CASE_FIXTURE(counts_fixture_t, "the positions exercise every count")
  {
    int seen[KS_FEATURE_COUNT] = {};
    int white_ahead[KS_FEATURE_COUNT] = {};
    int black_ahead[KS_FEATURE_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][KS_FEATURE_COUNT];
      king_safety_features(&game.board, engine);

      for (int feature = 0; feature < KS_FEATURE_COUNT; ++feature) {
        if (engine[WHITE][feature] != 0 || engine[BLACK][feature] != 0) {
          seen[feature]++;
        }

        const int difference = engine[WHITE][feature] - engine[BLACK][feature];

        if (difference > 0) { white_ahead[feature]++; }
        if (difference < 0) { black_ahead[feature]++; }
      }
    }

    for (int feature = 0; feature < KS_FEATURE_COUNT; ++feature) {
      const std::string name = king_safety_feature_names[feature];

      CHECK_MESSAGE(seen[feature] > 0, (name + ": never non-zero"));
      CHECK_MESSAGE(white_ahead[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(black_ahead[feature] > 0, (name + ": never Black ahead"));
    }
  }

  // The passed pawn counts, engine against walk, over the same corpus. The
  // case with teeth for this term, for the reason the king safety pair above
  // has teeth: the trace records the engine's own counts, so no score
  // comparison can tell a miscounted bucket from a correctly counted one.
  //
  // What can be compared is the White-minus-Black difference and not the two
  // rows: walked_counts::passed_pawn_features() reports the difference and
  // nothing else, which is the form the score weighs. The rows are checked
  // against hand-computed answers instead, in the case below, on the placements
  // where only one side has a pawn -- and the engine's rows are checked for
  // both signs and every bucket in the case after this one, which is what a
  // difference-only comparison cannot see: a bug that credits both sides
  // equally cancels.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the passed pawns the walk counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][walked_counts::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      int walk[walked_counts::PASSED_PAWN_COUNT] = {};
      REQUIRE(walked_counts::passed_pawn_features(fen.substr(0, fen.find(' ')),
                                                  walk));

      for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
           ++bucket) {
        CAPTURE(bucket);
        CAPTURE(engine[WHITE][bucket]);
        CAPTURE(engine[BLACK][bucket]);

        CHECK(engine[WHITE][bucket] - engine[BLACK][bucket] == walk[bucket]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A bucket that is zero in every position of the corpus is a bucket the two
  // implementations agree about for free, and the two colours count their ranks
  // in opposite directions, so a rank arithmetic error that only hits one of
  // them needs both rows claimed to be caught.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the positions exercise every passed pawn bucket")
  {
    int seen[2][walked_counts::PASSED_PAWN_COUNT] = {};
    int white_ahead[walked_counts::PASSED_PAWN_COUNT] = {};
    int black_ahead[walked_counts::PASSED_PAWN_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][walked_counts::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
           ++bucket) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][bucket] != 0) { seen[colour][bucket]++; }
        }

        const int difference = engine[WHITE][bucket] - engine[BLACK][bucket];

        if (difference > 0) { white_ahead[bucket]++; }
        if (difference < 0) { black_ahead[bucket]++; }
      }
    }

    for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
         ++bucket) {
      const std::string which = "bucket " + std::to_string(bucket);

      CHECK_MESSAGE(seen[WHITE][bucket] > 0, (which + ": no White passer"));
      CHECK_MESSAGE(seen[BLACK][bucket] > 0, (which + ": no Black passer"));
      CHECK_MESSAGE(white_ahead[bucket] > 0, (which + ": never White ahead"));
      CHECK_MESSAGE(black_ahead[bucket] > 0, (which + ": never Black ahead"));
    }
  }

  // The same counts against placements whose answer was worked out by hand
  // rather than by either implementation, which is what says the two agree on
  // the specification and not merely with each other.
  //
  // It is also where the engine's rows are pinned per colour. The hand-computed
  // answer is a difference, but on a placement carrying only one side's pawns
  // that difference *is* that side's row and the other row has to be empty --
  // so a count credited to both kings at once, which cancels in every
  // difference, fails here.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count passed pawns by rank")
  {
    static const passed_pawn_case_t cases[] = {
        {"4k3/8/8/8/8/8/P7/4K3",
         "white pawn on its own second rank",
         {1, 0, 0, 0, 0}},
        {"4k3/8/3P4/2P5/1P6/P7/8/4K3",
         "white pawns on the third to the sixth fill buckets 1 to 4",
         {0, 1, 1, 1, 1}},
        {"4k3/p7/8/8/8/8/8/4K3",
         "black pawn on its own second rank",
         {-1, 0, 0, 0, 0}},
        {"4k3/8/p7/1p6/2p5/3p4/8/4K3",
         "black pawns on the third to the sixth fill buckets 1 to 4",
         {0, -1, -1, -1, -1}},
        // A pawn one square from promotion is a passer by definition and, since
        // S134, in no bucket: its old bucket was exactly eight piece-square
        // columns and was folded into the pawn table (DEC-090). These two were
        // "one square from promotion", expecting bucket 5. What makes an
        // all-zero answer a claim is where a bucket 5 would land if the engine
        // stopped leaving the pawn out: White's in Black's bucket 0, which the
        // per-colour rows below catch, and Black's past both rows, which the
        // guard row in engine_passed_counts() catches.
        {"4k3/P7/8/8/8/8/8/4K3",
         "a white pawn on its own seventh is in no bucket",
         {0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/p7/4K3",
         "a black pawn on its own seventh is in no bucket",
         {0, 0, 0, 0, 0}},
        // Both colours at once: the two bucket-0 pawns cancel in the difference
        // and the c5 pawn is what is left, so a walk that lost the sign would
        // report -1 here rather than nothing at all.
        {"4k3/7p/8/2P5/8/8/P7/4K3",
         "one passed pawn each in the same bucket",
         {0, 0, 0, 1, 0}},
        {"4k3/8/8/P3p3/4P3/8/8/4K3",
         "e4 and e5 block each other head on",
         {0, 0, 0, 1, 0}},
        // The black passer that keeps this answer non-zero stood on a2 and
        // counted in bucket 5 until S134; it moved to a3, bucket 4. What the
        // case is about -- f6 stopping e4 and e4 stopping f6 -- did not move.
        {"4k3/8/5p2/8/4P3/p7/8/4K3",
         "f6 stops e4 from a neighbouring file",
         {0, 0, 0, 0, -1}},
        // The two directions that must not block, in one placement: d5 is level
        // with e5 and f3 is behind it. Reverse "ahead" and every one of these
        // three answers changes.
        {"4k3/8/8/3pP3/8/5p2/8/4K3",
         "level and behind do not block",
         {0, 0, -1, 1, -1}},
        {"4k3/8/8/3pP3/4P3/8/8/4K3",
         "doubled pawns, only the front one passed",
         {0, 0, 0, 1, 0}},
    };

    int positive[walked_counts::PASSED_PAWN_COUNT] = {};
    int negative[walked_counts::PASSED_PAWN_COUNT] = {};
    int white_only_cases = 0;
    int black_only_cases = 0;
    int mixed_cases = 0;

    for (const passed_pawn_case_t& item : cases) {
      // As strings, because CAPTURE of a `const char*` logs the pointer and a
      // failure then names the case by its address.
      const std::string placement = item.placement;
      const std::string what = item.what;

      CAPTURE(placement);
      CAPTURE(what);

      int walk[walked_counts::PASSED_PAWN_COUNT] = {};
      REQUIRE(walked_counts::passed_pawn_features(item.placement, walk));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][walked_counts::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      // Which rows the difference pins. A placement with no black pawn on it
      // cannot hide a Black count inside a zero difference, and the mirror.
      const bool white_only = placement.find('p') == std::string::npos;
      const bool black_only = placement.find('P') == std::string::npos;

      white_only_cases += white_only ? 1 : 0;
      black_only_cases += black_only ? 1 : 0;
      mixed_cases += (!white_only && !black_only) ? 1 : 0;

      for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
           ++bucket) {
        CAPTURE(bucket);
        CHECK(walk[bucket] == item.expected[bucket]);
        CHECK(engine[WHITE][bucket] - engine[BLACK][bucket] ==
              item.expected[bucket]);

        if (white_only) {
          CHECK(engine[WHITE][bucket] == item.expected[bucket]);
          CHECK(engine[BLACK][bucket] == 0);
        }

        if (black_only) {
          CHECK(engine[BLACK][bucket] == -item.expected[bucket]);
          CHECK(engine[WHITE][bucket] == 0);
        }

        if (item.expected[bucket] > 0) { positive[bucket]++; }
        if (item.expected[bucket] < 0) { negative[bucket]++; }
      }
    }

    // The per-colour pinning above is only worth something if placements of
    // each kind are actually in the list, and the mixed ones are what stop the
    // whole case from being a pair of one-sided boards.
    CHECK(white_only_cases > 0);
    CHECK(black_only_cases > 0);
    CHECK(mixed_cases > 0);

    // Non-vacuous by construction. A bucket no case ever reaches is a bucket
    // the extractor could compute any way it liked, and the mirror between the
    // colours is exactly where a rank arithmetic error hides, so each one has
    // to be claimed from both directions.
    for (size_t bucket = 0; bucket < walked_counts::PASSED_PAWN_COUNT;
         ++bucket) {
      CAPTURE(bucket);
      CHECK(positive[bucket] > 0);
      CHECK(negative[bucket] > 0);
    }
  }

  // The pawn structure counts, engine against walk, over the same corpus, and
  // the case with teeth for this term for the reason the two pairs above have
  // teeth: the trace records the engine's own counts, so no score comparison
  // can tell a miscount from a count.
  //
  // walked_counts::pawn_structure_features() reports White minus Black and
  // nothing else, which is the form the score weighs, so that is what can be
  // compared here. The rows themselves are pinned against hand-computed answers
  // in the case below, which is what a difference cannot see: a miscount that
  // hits both sides equally cancels.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the pawn structure the walk counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      int walk[walked_counts::PAWN_STRUCTURE_COUNT] = {};
      REQUIRE(walked_counts::pawn_structure_features(
          fen.substr(0, fen.find(' ')), walk));

      for (size_t feature = 0; feature < walked_counts::PAWN_STRUCTURE_COUNT;
           ++feature) {
        const std::string name = pawn_structure_feature_names[feature];

        CAPTURE(name);
        CAPTURE(engine[WHITE][feature]);
        CAPTURE(engine[BLACK][feature]);

        CHECK(engine[WHITE][feature] - engine[BLACK][feature] == walk[feature]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A count that is zero in every position of the corpus is a count the two
  // implementations agree about for free, and the two colours read "ahead" and
  // "behind" in opposite directions, so an error that only hits one of them
  // needs both rows claimed to be caught.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the positions exercise every pawn structure count")
  {
    int seen[2][walked_counts::PAWN_STRUCTURE_COUNT] = {};
    int white_ahead[walked_counts::PAWN_STRUCTURE_COUNT] = {};
    int black_ahead[walked_counts::PAWN_STRUCTURE_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      for (size_t feature = 0; feature < walked_counts::PAWN_STRUCTURE_COUNT;
           ++feature) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][feature] != 0) { seen[colour][feature]++; }
        }

        const int difference = engine[WHITE][feature] - engine[BLACK][feature];

        if (difference > 0) { white_ahead[feature]++; }
        if (difference < 0) { black_ahead[feature]++; }
      }
    }

    for (size_t feature = 0; feature < walked_counts::PAWN_STRUCTURE_COUNT;
         ++feature) {
      const std::string name = pawn_structure_feature_names[feature];

      CHECK_MESSAGE(seen[WHITE][feature] > 0, (name + ": never White"));
      CHECK_MESSAGE(seen[BLACK][feature] > 0, (name + ": never Black"));
      CHECK_MESSAGE(white_ahead[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(black_ahead[feature] > 0, (name + ": never Black ahead"));
    }
  }

  // The same counts against placements whose answer was worked out by hand
  // rather than by either implementation, which is what says the two agree on
  // the specification and not merely with each other. Each clause of that
  // specification that could be read another way has a case here: the rank a
  // neighbour has to be on before it stops a pawn being backward, that rank
  // does not enter the isolated question at all, and that a file with n pawns
  // on it is worth n - 1 doubled and not one.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count pawn structure by hand")
  {
    static const pawn_structure_case_t cases[] = {
        {"4k3/8/8/8/8/8/P7/4K3",
         "a lone pawn has no neighbour and is isolated",
         {1, 0, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/8/P7/P7/4K3",
         "two on a file are two isolated and one doubled",
         {2, 1, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/P7/P7/P7/4K3",
         "three on a file are three isolated and two doubled",
         {3, 2, 0},
         {0, 0, 0}},
        {"4k3/p7/p7/p7/8/8/8/4K3",
         "the same three for Black, whose file runs the other way",
         {0, 0, 0},
         {3, 2, 0}},
        {"4k3/1P6/8/8/8/8/P7/4K3",
         "rank does not enter isolated: a neighbour five ranks "
         "ahead still answers it",
         {0, 0, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/2p5/8/PP6/4K3",
         "a neighbour abreast stops b2 being backward, and c4 is "
         "backward on a2",
         {0, 0, 0},
         {1, 0, 1}},
        {"4k3/8/8/3p4/2p5/8/1P6/4K3",
         "b2 is backward on c4, and d5 behind c4 stops c4 being "
         "backward itself",
         {1, 0, 1},
         {0, 0, 0}},
        {"4k3/1p6/8/2P5/3P4/8/8/4K3",
         "the same placement mirrored, so Black is the backward "
         "side",
         {0, 0, 0},
         {1, 0, 1}},
        // The three features are not exclusive and are not meant to be, so one
        // pawn counted in all three is the case that says so.
        {"4k3/8/8/8/1p6/P7/P7/4K3",
         "a2 is isolated, doubled and backward at once",
         {2, 1, 1},
         {1, 0, 1}},
    };

    int positive[walked_counts::PAWN_STRUCTURE_COUNT] = {};
    int negative[walked_counts::PAWN_STRUCTURE_COUNT] = {};
    int white_row[walked_counts::PAWN_STRUCTURE_COUNT] = {};
    int black_row[walked_counts::PAWN_STRUCTURE_COUNT] = {};

    for (const pawn_structure_case_t& item : cases) {
      // As strings, because CAPTURE of a `const char*` logs the pointer and a
      // failure then names the case by its address.
      const std::string placement = item.placement;
      const std::string what = item.what;

      CAPTURE(placement);
      CAPTURE(what);

      int walk[walked_counts::PAWN_STRUCTURE_COUNT] = {};
      REQUIRE(walked_counts::pawn_structure_features(item.placement, walk));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      for (size_t feature = 0; feature < walked_counts::PAWN_STRUCTURE_COUNT;
           ++feature) {
        const std::string name = pawn_structure_feature_names[feature];
        const int difference = item.white[feature] - item.black[feature];

        CAPTURE(name);

        CHECK(walk[feature] == difference);
        CHECK(engine[WHITE][feature] == item.white[feature]);
        CHECK(engine[BLACK][feature] == item.black[feature]);

        if (difference > 0) { positive[feature]++; }
        if (difference < 0) { negative[feature]++; }
        if (item.white[feature] != 0) { white_row[feature]++; }
        if (item.black[feature] != 0) { black_row[feature]++; }
      }
    }

    // Non-vacuous by construction. A count no case ever reaches is a count the
    // extractors could compute any way they liked, and a row no case ever
    // claims is a row the per-colour pinning above never pins -- which is the
    // whole reason these cases carry two rows instead of a difference.
    for (size_t feature = 0; feature < walked_counts::PAWN_STRUCTURE_COUNT;
         ++feature) {
      const std::string name = pawn_structure_feature_names[feature];

      CHECK_MESSAGE(positive[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(negative[feature] > 0, (name + ": never Black ahead"));
      CHECK_MESSAGE(white_row[feature] > 0, (name + ": no White row claimed"));
      CHECK_MESSAGE(black_row[feature] > 0, (name + ": no Black row claimed"));
    }
  }

  // The piece placement counts, engine against walk, over the same corpus, and
  // the case with teeth for this term for the reason the three pairs above have
  // teeth: the trace records the engine's own counts, and the two bad bishop
  // columns still weigh zero besides, so no score comparison can see a
  // miscount in those at all.
  //
  // walked_counts::piece_placement_features() reports White minus Black and
  // nothing else, which is the form the score weighs, so that is what can be
  // compared here. The rows themselves are pinned against hand-computed
  // answers in the case below, which is what a difference cannot see: a
  // miscount that hits both sides equally cancels.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the piece placement the walk counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][walked_counts::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      int walk[walked_counts::PIECE_PLACEMENT_COUNT] = {};
      REQUIRE(walked_counts::piece_placement_features(
          fen.substr(0, fen.find(' ')), walk));

      for (size_t feature = 0; feature < walked_counts::PIECE_PLACEMENT_COUNT;
           ++feature) {
        const std::string name = piece_placement_feature_names[feature];

        CAPTURE(name);
        CAPTURE(engine[WHITE][feature]);
        CAPTURE(engine[BLACK][feature]);

        CHECK(engine[WHITE][feature] - engine[BLACK][feature] == walk[feature]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A count that is zero in every position of the corpus is a count the two
  // implementations agree about for free, and the half-open file is a different
  // file set for each colour, so an error that only hits one of them needs both
  // rows claimed to be caught.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the positions exercise every piece placement count")
  {
    int seen[2][walked_counts::PIECE_PLACEMENT_COUNT] = {};
    int white_ahead[walked_counts::PIECE_PLACEMENT_COUNT] = {};
    int black_ahead[walked_counts::PIECE_PLACEMENT_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][walked_counts::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      for (size_t feature = 0; feature < walked_counts::PIECE_PLACEMENT_COUNT;
           ++feature) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][feature] != 0) { seen[colour][feature]++; }
        }

        const int difference = engine[WHITE][feature] - engine[BLACK][feature];

        if (difference > 0) { white_ahead[feature]++; }
        if (difference < 0) { black_ahead[feature]++; }
      }
    }

    for (size_t feature = 0; feature < walked_counts::PIECE_PLACEMENT_COUNT;
         ++feature) {
      const std::string name = piece_placement_feature_names[feature];

      CHECK_MESSAGE(seen[WHITE][feature] > 0, (name + ": never White"));
      CHECK_MESSAGE(seen[BLACK][feature] > 0, (name + ": never Black"));
      CHECK_MESSAGE(white_ahead[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(black_ahead[feature] > 0, (name + ": never Black ahead"));
    }
  }

  // The same counts against placements whose answer was worked out by hand
  // rather than by either implementation, which is what says the two agree on
  // the specification and not merely with each other. Each clause that could be
  // read another way has a case here: that a pair is not a count of bishops and
  // does not ask about square colour, that a file with an own pawn on it is
  // neither open nor half-open however many enemy pawns stand there, that a
  // piece does not close a file, and that the seventh rank, a feature until
  // S134, is nothing to this term any more.
  //
  // The rows written three wide are the cases from before S135, and C++ fills
  // the four columns S135 added with zeros: none of those placements has a
  // bishop beside an own pawn, a minor on a trap square or a rook beside its
  // king's corner. The S135 cases below carry all seven. Their answers were
  // checked by a second piece of code written from evaluation.hpp's
  // definitions, by (file, rank) and not by bitboard (S135's step file), and
  // each case says the arithmetic: a square's colour is the parity of its file
  // plus its rank, a1 odd, so c1 and b2 and d2 share one and c2 does not.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count piece placement by hand")
  {
    static const piece_placement_case_t cases[] = {
        {"4k3/8/8/8/8/8/8/2B1KB2",
         "two bishops are a pair",
         {1, 0, 0},
         {0, 0, 0}},
        {"2b1kb2/8/8/8/8/8/8/4K3",
         "the same two for Black",
         {0, 0, 0},
         {1, 0, 0}},
        {"4k3/8/8/8/8/8/8/2B1KB1B",
         "three bishops are still one pair, not three",
         {1, 0, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/8/8/8/2B1K1B1",
         "two bishops on the same colour square are still a pair",
         {1, 0, 0},
         {0, 0, 0}},
        {"2b1kb2/8/8/8/8/8/8/2B1K3",
         "one bishop is not a pair and two are",
         {0, 0, 0},
         {1, 0, 0}},
        {"4k3/8/8/8/8/8/8/R3K3",
         "a file with no pawn of either colour on it is open",
         {0, 1, 0},
         {0, 0, 0}},
        {"r3k3/8/8/8/8/8/8/4K3", "the same for Black", {0, 0, 0}, {0, 1, 0}},
        {"4k3/8/8/8/8/8/P7/R3K3",
         "an own pawn makes the file neither open nor half open",
         {0, 0, 0},
         {0, 0, 0}},
        {"4k3/p7/8/8/8/8/P7/R3K3",
         "an own pawn still closes it with an enemy pawn there too",
         {0, 0, 0},
         {0, 0, 0}},
        {"4k3/p7/8/8/8/8/8/R3K3",
         "an enemy pawn alone makes the file half open",
         {0, 0, 1},
         {0, 0, 0}},
        {"r3k3/8/8/8/8/8/P7/4K3", "the same for Black", {0, 0, 0}, {0, 0, 1}},
        {"4k3/8/8/8/n7/8/8/R3K3",
         "a piece is not a pawn and does not close a file",
         {0, 1, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/8/8/8/R3K2R",
         "two rooks on open files count two, not one",
         {0, 2, 0},
         {0, 0, 0}},
        // Rook on the seventh was a fourth feature until S134 folded it into
        // the rook table (DEC-090). These four placements pinned it; they now
        // pin that a rook on either side's seventh, or on its own second,
        // counts for its file and for nothing else.
        {"8/R7/8/8/4k3/8/8/4K3",
         "a White rook on the seventh counts for its file alone",
         {0, 1, 0},
         {0, 0, 0}},
        {"4k3/8/8/8/8/8/r7/4K3",
         "a Black rook on its own seventh, rank 2, counts for its file "
         "alone",
         {0, 0, 0},
         {0, 1, 0}},
        {"4k3/8/8/8/8/8/R7/4K3",
         "a White rook on rank 2 counts for its file alone",
         {0, 1, 0},
         {0, 0, 0}},
        {"4k3/1R6/8/1p6/8/8/8/4K3",
         "a rook on the seventh on a half open file counts once",
         {0, 0, 1},
         {0, 0, 0}},
        // Both colours at once, so a walk that lost the sign reports the wrong
        // difference rather than nothing at all.
        {"4k3/8/8/8/8/8/r7/2B1KB2",
         "White has the pair, Black a rook on an open file",
         {1, 0, 0},
         {0, 1, 0}},
        // S135: bad bishop. Own pawns on the bishop's colour, per bishop.
        {"4k3/8/8/8/8/8/1PPP4/2B1K3",
         "the bishop on c1 counts b2 and d2, its colour, and not c2: 1 x 2",
         {0, 0, 0, 2, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"2b1k3/1ppp4/8/8/8/8/8/4K3",
         "the same for Black: c8 counts b7 and d7 and not c7",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 2, 0, 0, 0}},
        {"4k3/8/8/8/8/4B3/1P6/2B1K3",
         "two bishops on one colour each count its pawn: 2 x 1",
         {1, 0, 0, 2, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        // Rammed: the square ahead holds an enemy pawn, and only a pawn rams.
        {"4k3/8/8/8/1p6/1P6/8/3BK3",
         "b3 is on d1's colour and rammed by b4: counted in both",
         {0, 0, 0, 1, 1, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"3bk3/8/1p6/1P6/8/8/8/4K3",
         "the same for Black: b6 on d8's colour, rammed by b5",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 1, 1, 0, 0}},
        {"4k3/8/8/8/1n6/1P6/8/3BK3",
         "a knight in front of b3 does not ram it",
         {0, 0, 0, 1, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        // Trapped minor, each pattern evaluation.hpp lists.
        {"4k2N/7p/8/8/8/8/8/4K3",
         "a knight on h8 with an enemy pawn on h7",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k2N/5p2/8/8/8/8/8/4K3",
         "a knight on h8 with an enemy pawn on f7 instead",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/6pN/7p/8/8/8/8/4K3",
         "a knight on h7 with enemy pawns on g7 and h6",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/6pN/8/8/8/8/8/4K3",
         "a knight on h7 with g7 alone is not trapped",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/6pN/5p1p/8/8/8/8/4K3",
         "a knight meeting both its h7 patterns counts once",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/5p1B/6p1/8/8/8/8/4K3",
         "a bishop on h7 with enemy pawns on f7 and g6",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/B1p5/1p6/8/8/8/8/4K3",
         "its mirror on the a side: a7 with c7 and b6",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/8/5p1B/6p1/8/8/8/4K3",
         "a bishop on h6 with enemy pawns on g5 and f6",
         {0, 0, 0, 0, 0, 1, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/6pB/7p/8/8/8/8/4K3",
         "a bishop is not held to a knight's h7 pattern",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/7P/4K2n",
         "Black's knight on h1 with a White pawn on h2, the h8 one mirrored",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 1, 0}},
        {"4k3/8/8/8/8/6P1/5P1b/4K3",
         "Black's bishop on h2 with White pawns on f2 and g3",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 1, 0}},
        // Trapped rook: the corner beside the king; R3K2R above is the king
        // that has not moved, which shuts nothing in.
        {"4k3/8/8/8/8/8/8/5K1R",
         "a rook on h1 behind its king on f1, on an open file as well",
         {0, 1, 0, 0, 0, 0, 1},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/6R1/6K1",
         "a rook on g2 beside its king on g1",
         {0, 1, 0, 0, 0, 0, 1},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/8/R1K5",
         "the queen side: a rook on a1 behind its king on c1",
         {0, 1, 0, 0, 0, 0, 1},
         {0, 0, 0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/7R/5K1R",
         "two rooks shut in count two",
         {0, 2, 0, 0, 0, 0, 2},
         {0, 0, 0, 0, 0, 0, 0}},
        {"5k1r/8/8/8/8/8/8/4K3",
         "Black's rook on h8 behind its king on f8",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 1, 0, 0, 0, 0, 1}},
        {"1k6/r7/8/8/8/8/8/4K3",
         "Black's rook on a7 beside its king on b8",
         {0, 0, 0, 0, 0, 0, 0},
         {0, 1, 0, 0, 0, 0, 1}},
        // All four at once and both colours, so a lost sign shows: White's
        // knight h8 and rook h1 trapped, g2 on e2's colour; Black's c8 counts
        // b7, d5 and h7, and d5 is rammed by d4.
        {"r1b1k2N/pp5p/8/3p4/3P4/8/4BPPP/6KR",
         "both colours, every S135 count",
         {0, 0, 0, 1, 0, 1, 1},
         {0, 0, 0, 3, 1, 0, 0}},
    };

    int positive[walked_counts::PIECE_PLACEMENT_COUNT] = {};
    int negative[walked_counts::PIECE_PLACEMENT_COUNT] = {};
    int white_row[walked_counts::PIECE_PLACEMENT_COUNT] = {};
    int black_row[walked_counts::PIECE_PLACEMENT_COUNT] = {};

    for (const piece_placement_case_t& item : cases) {
      // As strings, because CAPTURE of a `const char*` logs the pointer and a
      // failure then names the case by its address.
      const std::string placement = item.placement;
      const std::string what = item.what;

      CAPTURE(placement);
      CAPTURE(what);

      int walk[walked_counts::PIECE_PLACEMENT_COUNT] = {};
      REQUIRE(walked_counts::piece_placement_features(item.placement, walk));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][walked_counts::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      for (size_t feature = 0; feature < walked_counts::PIECE_PLACEMENT_COUNT;
           ++feature) {
        const std::string name = piece_placement_feature_names[feature];
        const int difference = item.white[feature] - item.black[feature];

        CAPTURE(name);

        CHECK(walk[feature] == difference);
        CHECK(engine[WHITE][feature] == item.white[feature]);
        CHECK(engine[BLACK][feature] == item.black[feature]);

        if (difference > 0) { positive[feature]++; }
        if (difference < 0) { negative[feature]++; }
        if (item.white[feature] != 0) { white_row[feature]++; }
        if (item.black[feature] != 0) { black_row[feature]++; }
      }
    }

    // Non-vacuous by construction. A count no case ever reaches is a count the
    // extractors could compute any way they liked, and a row no case ever
    // claims is a row the per-colour pinning above never pins -- which is the
    // whole reason these cases carry two rows instead of a difference.
    for (size_t feature = 0; feature < walked_counts::PIECE_PLACEMENT_COUNT;
         ++feature) {
      const std::string name = piece_placement_feature_names[feature];

      CHECK_MESSAGE(positive[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(negative[feature] > 0, (name + ": never Black ahead"));
      CHECK_MESSAGE(white_row[feature] > 0, (name + ": no White row claimed"));
      CHECK_MESSAGE(black_row[feature] > 0, (name + ": no Black row claimed"));
    }
  }

  // Non-vacuous by construction. Until S276 this was what made the mirror's
  // score comparison, then at the top of this file, see the tempo term: tempo
  // has no count to check, so only a corpus holding both answers could tell a
  // bonus for White from a bonus for the mover. That comparison went with the
  // mirror, and what is left is the engine's half -- load_FEN() reads the side
  // to move the FEN states -- over a list that carries both.
  //
  // Since S276 it guards the position set and nothing else: no remaining case
  // needs the list to hold both sides to move. test_eval_trace and
  // test_tuner_gradient's trace_positions() run every position both ways round,
  // the gradient check claims every term parameter itself, and "the move is
  // worth the same to either side" below flips each position. Kept rather than
  // deleted, so that a later case which does need it finds it here.
  TEST_CASE_FIXTURE(counts_fixture_t, "the positions have both sides to move")
  {
    int white_to_move = 0;
    int black_to_move = 0;

    for (const std::string& fen : positions) {
      CAPTURE(fen);

      // The FEN's own field, read as text: +1 White and -1 Black, the form the
      // mirror's tempo feature took.
      const std::string side = side_to_move_of(fen);
      REQUIRE((side == "w" || side == "b"));
      const int tempo = (side == "w") ? 1 : -1;

      // The FEN and the engine's board have to agree about who is on move
      // before anything downstream of either means anything.
      REQUIRE(load_FEN(fen, &game));
      CHECK(tempo == ((game.board.active_color == WHITE) ? 1 : -1));

      if (tempo > 0) { white_to_move++; }
      if (tempo < 0) { black_to_move++; }
    }

    CHECK(white_to_move > 0);
    CHECK(black_to_move > 0);
  }

  // The tempo term's own property, and the one thing about it that can be
  // wrong: it belongs to the side to move and not to White.
  //
  // Every other term in evaluate() is a function of the board, so it is White
  // relative and negates when the side to move changes; this one does not
  // change at all. The two therefore separate cleanly in the sum of the two
  // scores -- everything else cancels and twice the bonus is what is left. A
  // bonus added before the sign resolves would cancel here instead and leave 0,
  // whatever the weights were.
  //
  // With the weights at zero the sum is 0, which is the same statement as "this
  // commit changes nothing": the shipped engine has to answer a position and
  // its side-to-move flip with scores that negate exactly. Fitting the weights
  // does not make the case vacuous, it makes it stronger, which is why it is
  // not written as a comparison against zero.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the move is worth the same to either side")
  {
    int checked = 0;

    for (const std::string& fen : positions) {
      CAPTURE(fen);

      REQUIRE(load_FEN(with_side_to_move(fen, 'w'), &game));
      const int white = evaluate(&game.board);
      const int phase = game_phase(&game.board);

      REQUIRE(load_FEN(with_side_to_move(fen, 'b'), &game));
      const int black = evaluate(&game.board);

      // The taper evaluate_cheap() applies, on the phase both boards share.
      // Exact rather than approximate: the same truncation happens on both
      // sides and the White-relative part negates without one.
      const int tempo =
          ((tempo_mg * phase) + (tempo_eg * (GAME_PHASE_MAX - phase))) /
          GAME_PHASE_MAX;

      CAPTURE(white);
      CAPTURE(black);

      CHECK(white + black == 2 * tempo);
      checked++;
    }

    CHECK(checked == static_cast<int>(positions.size()));
  }

  // Non-vacuous by construction: the positions above have to contain the
  // asymmetry the checks are supposed to be sensitive to. A suite of symmetric
  // positions would pass with a count that ignored colour entirely.
  //
  // Written for the mirror's score comparison, deleted at S276, and since then
  // it guards the position set and nothing else: the count cases above claim
  // both colours and both directions per feature themselves, and
  // test_tuner_gradient's gradient check claims every term parameter, phase
  // coverage included. Kept rather than deleted, so that a later case which
  // needs scored, colour-asymmetric positions at both ends of the taper finds
  // the guarantee here.
  TEST_CASE_FIXTURE(counts_fixture_t, "the positions can tell colours apart")
  {
    int non_zero = 0;
    int phases_seen = 0;
    bool seen_low_phase = false;
    bool seen_high_phase = false;

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      if (evaluate(&game.board) != 0) { non_zero++; }

      const int phase = game_phase(&game.board);
      if (phase <= 6) { seen_low_phase = true; }
      if (phase >= 22) { seen_high_phase = true; }
      phases_seen++;
    }

    CHECK(non_zero >= 5);
    CHECK(phases_seen == static_cast<int>(positions.size()));
    CHECK(seen_low_phase);
    CHECK(seen_high_phase);
  }
}


// S123's passed pawn family against the walk and by hand. The engine's counts
// per colour (passer_family_counts()) against walked_counts::
// passer_family_features(), which walks evaluation.hpp's definitions square
// by square and shares no code with the engine -- per colour, not as the
// difference, because at the landing's zero weights no score saw these
// counts, and at any weight a count credited to both sides cancels in any
// difference.
namespace
{

// The test positions with their colour mirrors, the JSON suites', and seeded
// random placements of two kings and up to eight pawns a side. The suites'
// tactics put pieces against the push squares far more often than the
// fixture does; the placements put pawns against each other on every rank and
// edge, which is where candidates on their own second and third rank are, and
// play and the fixture almost never have one there. A placement is kept with
// White to move when load_FEN() accepts it, and is skipped otherwise.
std::vector<std::string> family_population()
{
  std::vector<std::string> out;

  for (const std::string& fen : positions) {
    out.push_back(fen);
    out.push_back(mirror_fen(fen));
  }

  for (const std::string& fen : all_test_fens()) {
    out.push_back(fen);
  }

  std::mt19937 rng(20261031U);
  std::uniform_int_distribution<int> pawns_a_side(0, 8);

  for (size_t trial = 0; trial < 10000; ++trial) {
    std::vector<int> order(64);
    for (int square = 0; square < 64; ++square) {
      order[static_cast<size_t>(square)] = square;
    }
    std::shuffle(order.begin(), order.end(), rng);

    char squares[64] = {};
    size_t next = 0;
    squares[order[next++]] = 'K';
    squares[order[next++]] = 'k';

    for (const char pawn : {'P', 'p'}) {
      int placed = 0;
      const int wanted = pawns_a_side(rng);

      while (placed < wanted && next < 64) {
        const int square = order[next++];
        if ((square / 8) == 0 || (square / 8) == 7) { continue; }
        squares[square] = pawn;
        placed++;
      }
    }

    std::string fen;
    for (int row = 0; row < 8; ++row) {
      int empty = 0;
      for (int file = 0; file < 8; ++file) {
        const char c = squares[row * 8 + file];
        if (c == '\0') {
          empty++;
          continue;
        }
        if (empty > 0) { fen += static_cast<char>('0' + empty); }
        empty = 0;
        fen += c;
      }
      if (empty > 0) { fen += static_cast<char>('0' + empty); }
      if (row < 7) { fen += '/'; }
    }
    fen += " w - - 0 1";

    if (load_FEN(fen, &game)) { out.push_back(fen); }
  }

  return out;
}


void check_family(const std::string& fen, passer_family_t* engine_out)
{
  CAPTURE(fen);
  REQUIRE(load_FEN(fen, &game));

  passer_family_t engine;
  passer_family_counts(&game.board, &engine);

  walked_counts::passer_family_walk_t walk;
  REQUIRE(walked_counts::passer_family_features(fen.substr(0, fen.find(' ')),
                                                &walk));

  for (int colour = 0; colour < 2; ++colour) {
    CAPTURE(colour);

    for (int rank = 0; rank < 6; ++rank) {
      CAPTURE(rank);
      CHECK(engine.free[colour][rank] == walk.free[colour][rank]);
      CHECK(engine.safe[colour][rank] == walk.safe[colour][rank]);
      CHECK(engine.king_own[colour][rank] == walk.king_own[colour][rank]);
      CHECK(engine.king_enemy[colour][rank] == walk.king_enemy[colour][rank]);
    }

    for (int rank = 0; rank < 5; ++rank) {
      CAPTURE(rank);
      CHECK(engine.candidate[colour][rank] == walk.candidate[colour][rank]);
    }
  }

  *engine_out = engine;
}


// One hand-worked placement and its answer, per colour: the cells that are
// not zero, every other cell zero. Hand-worked from evaluation.hpp's
// definitions, so the two implementations are held to the specification and
// not only to each other.
struct family_cell_t
{
  const char* array;  // free, safe, king_own, king_enemy, candidate
  int colour;
  int rank;
  int value;
};

struct family_case_t
{
  const char* placement;
  const char* what;
  std::vector<family_cell_t> cells;
};

int family_cell(const passer_family_t& family,
                const std::string& array,
                int colour,
                int rank)
{
  if (array == "free") { return family.free[colour][rank]; }
  if (array == "safe") { return family.safe[colour][rank]; }
  if (array == "king_own") { return family.king_own[colour][rank]; }
  if (array == "king_enemy") { return family.king_enemy[colour][rank]; }
  return (rank < 5) ? family.candidate[colour][rank] : 0;
}

}  // namespace


TEST_SUITE("eval counts: S123's passed pawn family")
{
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the passer family the walk counts")
  {
    // Every cell of every array, per colour, non-zero somewhere: the
    // comparison above is vacuous on a cell nobody reached.
    passer_family_t seen{};
    size_t checked = 0;

    for (const std::string& fen : family_population()) {
      passer_family_t engine;
      check_family(fen, &engine);

      for (int colour = 0; colour < 2; ++colour) {
        for (int rank = 0; rank < 6; ++rank) {
          seen.free[colour][rank] += (engine.free[colour][rank] != 0);
          seen.safe[colour][rank] += (engine.safe[colour][rank] != 0);
          seen.king_own[colour][rank] += (engine.king_own[colour][rank] != 0);
          seen.king_enemy[colour][rank] +=
              (engine.king_enemy[colour][rank] != 0);
        }

        for (int rank = 0; rank < 5; ++rank) {
          seen.candidate[colour][rank] += (engine.candidate[colour][rank] != 0);
        }
      }

      checked++;
    }

    CHECK(checked > 10000);

    for (int colour = 0; colour < 2; ++colour) {
      for (int rank = 0; rank < 6; ++rank) {
        CAPTURE(colour);
        CAPTURE(rank);
        CHECK(seen.free[colour][rank] > 0);
        CHECK(seen.safe[colour][rank] > 0);
        CHECK(seen.king_own[colour][rank] > 0);
        CHECK(seen.king_enemy[colour][rank] > 0);
      }

      for (int rank = 0; rank < 5; ++rank) {
        CAPTURE(colour);
        CAPTURE(rank);
        CHECK(seen.candidate[colour][rank] > 0);
      }
    }
  }

  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count the passer family by hand")
  {
    // Ranks are the pawn's own from its second: e5 is White's 3, e4 Black's.
    const std::vector<family_case_t> cases = {
        {"4k3/8/8/4P3/8/8/8/4K3",
         "a free and safe passer, both kings' distances to e6",
         {{"free", WHITE, 3, 1},
          {"safe", WHITE, 3, 1},
          {"king_own", WHITE, 3, 5},
          {"king_enemy", WHITE, 3, 2}}},
        {"4k3/8/4n3/4P3/8/8/8/4K3",
         "a piece on the push square: neither free nor safe",
         {{"king_own", WHITE, 3, 5}, {"king_enemy", WHITE, 3, 2}}},
        {"2b1k3/8/8/4P3/8/8/8/4K3",
         "free, but a bishop on c8 attacks e6: not safe",
         {{"free", WHITE, 3, 1},
          {"king_own", WHITE, 3, 5},
          {"king_enemy", WHITE, 3, 2}}},
        {"2b1k3/3N4/8/4P3/8/8/8/4K3",
         "the bishop's ray stopped by a knight on d7: no x-ray, safe",
         {{"free", WHITE, 3, 1},
          {"safe", WHITE, 3, 1},
          {"king_own", WHITE, 3, 5},
          {"king_enemy", WHITE, 3, 2}}},
        {"8/4P3/8/8/8/8/k7/4K3",
         "the own seventh is counted, its push square the eighth",
         {{"free", WHITE, 5, 1},
          {"safe", WHITE, 5, 1},
          {"king_own", WHITE, 5, 7},
          {"king_enemy", WHITE, 5, 6}}},
        {"4k3/8/8/8/4p3/8/8/4K3",
         "Black's passer on e4 is its own fifth, push square e3",
         {{"free", BLACK, 3, 1},
          {"safe", BLACK, 3, 1},
          {"king_own", BLACK, 3, 5},
          {"king_enemy", BLACK, 3, 2}}},
        // CPW Candidate Passed Pawn's own diagram, kings added on e1 and e8:
        // its caption names b5 and g5 the candidates.
        {"4k3/p7/8/PP3ppp/8/5P1P/8/4K3",
         "CPW's diagram: b5 and g5 are candidates, nothing is passed",
         {{"candidate", WHITE, 3, 1}, {"candidate", BLACK, 2, 1}}},
        {"4k3/2p5/8/1P6/8/8/8/4K3",
         "one sentry and no helper: b5 is no candidate, nor c7",
         {}},
        {"4k3/2p5/8/PP6/8/8/8/4K3",
         "a5 helps on b6: b5 a candidate, a5 itself passed",
         {{"free", WHITE, 3, 1},
          {"safe", WHITE, 3, 1},
          {"king_own", WHITE, 3, 5},
          {"king_enemy", WHITE, 3, 4},
          {"candidate", WHITE, 3, 1}}},
    };

    for (const family_case_t& item : cases) {
      const std::string placement = item.placement;
      const std::string what = item.what;
      CAPTURE(placement);
      CAPTURE(what);

      passer_family_t engine;
      check_family(placement + " w - - 0 1", &engine);

      for (const std::string array :
           {"free", "safe", "king_own", "king_enemy", "candidate"}) {
        for (int colour = 0; colour < 2; ++colour) {
          for (int rank = 0; rank < 6; ++rank) {
            int expected = 0;
            for (const family_cell_t& cell : item.cells) {
              if (array == cell.array && colour == cell.colour &&
                  rank == cell.rank) {
                expected = cell.value;
              }
            }

            CAPTURE(array);
            CAPTURE(colour);
            CAPTURE(rank);
            CHECK(family_cell(engine, array, colour, rank) == expected);
          }
        }
      }
    }
  }
}
