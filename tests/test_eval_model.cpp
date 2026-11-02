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
  // does not enter the isolated question at all, that a file with n pawns on it
  // is worth n - 1 doubled and not one, and that an isolated pawn is never
  // backward however its stop square stands (DEC-331 (3)).
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
        // An isolated pawn with its stop attacked is never backward: every case
        // from here to the last pair asserted the opposite until S125's close,
        // and each such pawn now counts isolated_backward in the structure
        // family (DEC-331 (3), (13)).
        {"4k3/8/8/8/2p5/8/PP6/4K3",
         "a neighbour abreast stops b2 being backward, and c4, its stop "
         "attacked by b2, is isolated and so not backward",
         {0, 0, 0},
         {1, 0, 0}},
        {"4k3/8/8/3p4/2p5/8/1P6/4K3",
         "b2, its stop attacked by c4, is isolated and so not backward, and "
         "d5 behind c4 stops c4 being backward itself",
         {1, 0, 0},
         {0, 0, 0}},
        {"4k3/1p6/8/2P5/3P4/8/8/4K3",
         "the same placement mirrored, so b7 is the isolated pawn",
         {0, 0, 0},
         {1, 0, 0}},
        {"4k3/8/8/8/1p6/P7/P7/4K3",
         "a2 is isolated and doubled, and its stop attacked by b4 does not "
         "make it backward; nor is b4, its stop attacked by a2",
         {2, 1, 0},
         {1, 0, 0}},
        // S125: a backward pawn that is not isolated, one a colour. Backward
        // holds no other pawn, so these are what the backward rows say.
        {"4k3/8/8/p1P5/8/1P6/8/4K3",
         "b3 is backward with its only neighbour ahead on c5, and not "
         "isolated; a5 is isolated, its stop attacked by b3, and not backward",
         {0, 0, 1},
         {1, 0, 0}},
        {"4k3/8/1p6/8/P1p5/8/8/4K3",
         "the same placement mirrored: b6 is Black's backward pawn with its "
         "neighbour ahead on c4, and a4 White's isolated one",
         {1, 0, 0},
         {0, 0, 1}},
        // Doubled overlaps backward and is meant to, so a pawn counted in both
        // is the case that says so, one a colour.
        {"4k3/8/8/2P5/pP6/8/1P6/4K3",
         "b2 is doubled and backward at once, its only neighbour ahead on c5 "
         "and its stop attacked by a4; a4 is isolated, its stop attacked by b2",
         {0, 1, 1},
         {1, 0, 0}},
        {"4k3/1p6/8/Pp6/2p5/8/8/4K3",
         "the same placement mirrored: b7 is doubled and backward at once",
         {1, 0, 0},
         {0, 1, 1}},
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


// S125's pawn structure family against the walk and by hand. The engine's
// sets and counts per colour (structure_family_counts()) against
// walked_counts::structure_family_features(), which walks evaluation.hpp's
// definitions square by square and shares no code with the engine -- per
// colour, for the reason S123's family is: no score sees a group at zero, and
// at any weight a count credited to both sides cancels.
namespace
{

// The sets and every cell, engine against walk, per colour.
void check_structure_family(const std::string& fen,
                            structure_family_t* engine_out)
{
  CAPTURE(fen);
  REQUIRE(load_FEN(fen, &game));

  structure_family_t engine;
  structure_family_counts(&game.board, &engine);

  walked_counts::structure_family_walk_t walk;
  REQUIRE(walked_counts::structure_family_features(fen.substr(0, fen.find(' ')),
                                                   &walk));

  const auto cells = [](const char* what, const auto& mine, const auto& theirs,
                        int width) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int cell = 0; cell < width; ++cell) {
        const std::string name = what;
        CAPTURE(name);
        CAPTURE(colour);
        CAPTURE(cell);
        CHECK(mine[colour][cell] == theirs[colour][cell]);
      }
    }
  };

  for (int colour = 0; colour < 2; ++colour) {
    CAPTURE(colour);
    CHECK(engine.isolated_backward_set[colour] ==
          walk.isolated_backward_set[colour]);
    CHECK(engine.backward_only_set[colour] == walk.backward_only_set[colour]);
    CHECK(engine.phalanx_set[colour] == walk.phalanx_set[colour]);
    CHECK(engine.supported_set[colour] == walk.supported_set[colour]);
    CHECK(engine.weak_set[colour] == walk.weak_set[colour]);
    CHECK(engine.open_set[colour] == walk.open_set[colour]);
  }

  cells("isolated_backward", engine.isolated_backward, walk.isolated_backward,
        1);
  cells("isolated_file", engine.isolated_file, walk.isolated_file, 7);
  cells("doubled_file", engine.doubled_file, walk.doubled_file, 7);
  cells("backward_rank", engine.backward_rank, walk.backward_rank, 3);
  cells("phalanx", engine.phalanx, walk.phalanx, 6);
  cells("supported", engine.supported, walk.supported, 5);
  cells("weak_unopposed", engine.weak_unopposed, walk.weak_unopposed, 1);

  *engine_out = engine;
}


// One array's cell of the engine's counts, by the array's name.
int structure_cell(const structure_family_t& family,
                   const std::string& array,
                   int colour,
                   int cell)
{
  if (array == "isolated_backward") {
    return (cell < 1) ? family.isolated_backward[colour][cell] : 0;
  }
  if (array == "isolated_file") {
    return (cell < 7) ? family.isolated_file[colour][cell] : 0;
  }
  if (array == "doubled_file") {
    return (cell < 7) ? family.doubled_file[colour][cell] : 0;
  }
  if (array == "backward_rank") {
    return (cell < 3) ? family.backward_rank[colour][cell] : 0;
  }
  if (array == "phalanx") {
    return (cell < 6) ? family.phalanx[colour][cell] : 0;
  }
  if (array == "supported") {
    return (cell < 5) ? family.supported[colour][cell] : 0;
  }
  return (cell < 1) ? family.weak_unopposed[colour][cell] : 0;
}

struct structure_array_t
{
  const char* name;
  int width;
};

const structure_array_t structure_arrays[] = {
    {"isolated_backward", 1}, {"isolated_file", 7}, {"doubled_file", 7},
    {"backward_rank", 3},     {"phalanx", 6},       {"supported", 5},
    {"weak_unopposed", 1}};

}  // namespace


TEST_SUITE("eval counts: S125's pawn structure family")
{
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the structure family the walk counts")
  {
    // Every cell, per colour, non-zero somewhere, and every set non-empty:
    // the comparison above is vacuous on a cell nobody reached.
    int seen[7][2][7] = {};
    size_t sets_seen[6][2] = {};
    size_t checked = 0;

    for (const std::string& fen : family_population()) {
      structure_family_t engine;
      check_structure_family(fen, &engine);

      for (int a = 0; a < 7; ++a) {
        for (int colour = 0; colour < 2; ++colour) {
          for (int cell = 0; cell < structure_arrays[a].width; ++cell) {
            seen[a][colour][cell] +=
                (structure_cell(engine, structure_arrays[a].name, colour,
                                cell) != 0);
          }
        }
      }

      for (int colour = 0; colour < 2; ++colour) {
        const bb_t sets[6] = {engine.isolated_backward_set[colour],
                              engine.backward_only_set[colour],
                              engine.phalanx_set[colour],
                              engine.supported_set[colour],
                              engine.weak_set[colour],
                              engine.open_set[colour]};
        for (int s = 0; s < 6; ++s) {
          sets_seen[s][colour] += (sets[s] != 0);
        }
      }

      checked++;
    }

    CHECK(checked > 10000);

    for (int a = 0; a < 7; ++a) {
      for (int colour = 0; colour < 2; ++colour) {
        for (int cell = 0; cell < structure_arrays[a].width; ++cell) {
          const std::string name = structure_arrays[a].name;
          CAPTURE(name);
          CAPTURE(colour);
          CAPTURE(cell);
          CHECK(seen[a][colour][cell] > 0);
        }
      }
    }

    for (int s = 0; s < 6; ++s) {
      for (int colour = 0; colour < 2; ++colour) {
        CAPTURE(s);
        CAPTURE(colour);
        CHECK(sets_seen[s][colour] > 0);
      }
    }
  }

  // The accepts' clause, DEC-331 (3): an isolated pawn whose stop square an
  // enemy pawn attacks counts isolated_backward and is not counted backward, in
  // both colours. Each placement holds such a pawn for the colour it names, so
  // the precondition that would make a double count happen stands before the
  // two counts that say it does not. tests/test_evaluation's mirror case cannot
  // say this: backward_only_set wrong the same way for both colours still
  // mirrors, and it reads red only on a defect in one colour.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "an isolated pawn with its stop attacked is not backward")
  {
    struct by_hand_t
    {
      const char* placement;
      int colour;
      int other_backward;
    };
    static const by_hand_t cases[] = {
        // b2, its stop attacked by c4; c4 and d5 are neighbours.
        {"4k3/8/8/3p4/2p5/8/1P6/4K3", WHITE, 0},
        {"4k3/1p6/8/2P5/3P4/8/8/4K3", BLACK, 0},
        // a5 attacked on a4 by b3, and b3 backward, the other colour's and not
        // isolated, so the flat is not simply empty; then the mirror.
        {"4k3/8/8/p1P5/8/1P6/8/4K3", BLACK, 1},
        {"4k3/8/1p6/8/P1p5/8/8/4K3", WHITE, 1},
    };

    for (const by_hand_t& item : cases) {
      const std::string placement = item.placement;
      CAPTURE(placement);
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int flat[2][3];
      structure_family_t family;
      pawn_structure_counts(&game.board, flat);
      structure_family_counts(&game.board, &family);

      // The precondition: isolated, its stop attacked.
      REQUIRE(flat[item.colour][0] == 1);
      REQUIRE(family.isolated_backward[item.colour][0] == 1);

      CHECK(flat[item.colour][2] == 0);
      CHECK(family.backward_only_set[item.colour] == 0);
      CHECK(flat[1 - item.colour][2] == item.other_backward);
    }

    // Over S123's population, and the flat's own count against the family's
    // set: the flat holds the backward pawns and nothing else. Both sets met,
    // per colour, or the two checks are agreeing about nothing.
    size_t both[2][2] = {};
    size_t checked = 0;

    for (const std::string& fen : family_population()) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int flat[2][3];
      structure_family_t family;
      pawn_structure_counts(&game.board, flat);
      structure_family_counts(&game.board, &family);

      for (int colour = 0; colour < 2; ++colour) {
        CAPTURE(colour);
        CHECK((family.isolated_backward_set[colour] &
               family.backward_only_set[colour]) == 0);
        CHECK(flat[colour][2] == count_bits(family.backward_only_set[colour]));
        both[colour][0] += (family.isolated_backward_set[colour] != 0);
        both[colour][1] += (family.backward_only_set[colour] != 0);
      }

      checked++;
    }

    CHECK(checked > 10000);
    for (int colour = 0; colour < 2; ++colour) {
      CAPTURE(colour);
      CHECK(both[colour][0] > 100);
      CHECK(both[colour][1] > 100);
    }
    MESSAGE("isolated and backward apart: "
            << checked << " positions; isolated_backward on " << both[WHITE][0]
            << " and " << both[BLACK][0] << ", backward on " << both[WHITE][1]
            << " and " << both[BLACK][1]);
  }

  // Hand-worked placements from evaluation.hpp's definitions, each run as
  // written and as its colour mirror with the rows swapped: a definition is
  // the same for both colours, so the mirror's answer is the case's with
  // White and Black exchanged. Every cell of every array is reached, by each
  // colour and with each sign of the difference, which the loop at the end
  // checks. Ranks named are the side's own; a file cell is the file less one.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count the structure family by "
                    "hand")
  {
    const std::vector<family_case_t> cases = {
        {"4k3/8/8/8/8/8/1P1P1P1P/4K3",
         "b2, d2, f2 and h2 are isolated, open and weak",
         {{"isolated_file", WHITE, 0, 1},
          {"isolated_file", WHITE, 2, 1},
          {"isolated_file", WHITE, 4, 1},
          {"isolated_file", WHITE, 6, 1},
          {"weak_unopposed", WHITE, 0, 4}}},
        {"4k3/8/8/8/8/8/P1P1P1P1/4K3",
         "a2, c2, e2 and g2 the same; the a file has no cell",
         {{"isolated_file", WHITE, 1, 1},
          {"isolated_file", WHITE, 3, 1},
          {"isolated_file", WHITE, 5, 1},
          {"weak_unopposed", WHITE, 0, 4}}},
        {"4k3/8/8/8/1P1P1P1P/8/1P1P1P1P/4K3",
         "doubled on b, d, f and h: the rear pawn of each file, and all "
         "eight isolated",
         {{"isolated_file", WHITE, 0, 2},
          {"isolated_file", WHITE, 2, 2},
          {"isolated_file", WHITE, 4, 2},
          {"isolated_file", WHITE, 6, 2},
          {"doubled_file", WHITE, 0, 1},
          {"doubled_file", WHITE, 2, 1},
          {"doubled_file", WHITE, 4, 1},
          {"doubled_file", WHITE, 6, 1},
          {"weak_unopposed", WHITE, 0, 8}}},
        {"4k3/8/8/8/P1P1P1P1/8/P1P1P1P1/4K3",
         "doubled on a, c, e and g",
         {{"isolated_file", WHITE, 1, 2},
          {"isolated_file", WHITE, 3, 2},
          {"isolated_file", WHITE, 5, 2},
          {"doubled_file", WHITE, 1, 1},
          {"doubled_file", WHITE, 3, 1},
          {"doubled_file", WHITE, 5, 1},
          {"weak_unopposed", WHITE, 0, 8}}},
        {"4k3/8/8/p1P5/8/1P6/8/4K3",
         "b3 backward on its third with c5 ahead; a5 isolated, its stop a4 "
         "attacked by b3",
         {{"backward_rank", WHITE, 0, 1},
          {"weak_unopposed", WHITE, 0, 1},
          {"isolated_backward", BLACK, 0, 1},
          {"weak_unopposed", BLACK, 0, 1}}},
        {"4k3/8/p1P5/8/1P6/8/8/4K3",
         "b4 backward on its fourth with c6 ahead; a6 isolated, its stop a5 "
         "attacked by b4",
         {{"backward_rank", WHITE, 1, 1},
          {"weak_unopposed", WHITE, 0, 1},
          {"isolated_backward", BLACK, 0, 1},
          {"weak_unopposed", BLACK, 0, 1}}},
        {"4k3/p1P5/8/1P6/8/8/8/4K3",
         "b5 backward on its fifth with c7 ahead; a7 isolated, its stop a6 "
         "attacked by b5",
         {{"backward_rank", WHITE, 2, 1},
          {"weak_unopposed", WHITE, 0, 1},
          {"isolated_backward", BLACK, 0, 1},
          {"weak_unopposed", BLACK, 0, 1}}},
        {"4k3/8/8/8/p1P5/8/1P6/4K3",
         "b2 backward on its second, the baseline, in no cell",
         {{"weak_unopposed", WHITE, 0, 1},
          {"isolated_backward", BLACK, 0, 1},
          {"weak_unopposed", BLACK, 0, 1}}},
        {"4k3/8/8/3p4/2p5/8/1P6/4K3",
         "b2 isolated, its stop attacked by c4; c4 supported by d5 on Black's "
         "fifth; d5 weak with c4 ahead",
         {{"isolated_backward", WHITE, 0, 1},
          {"isolated_file", WHITE, 0, 1},
          {"weak_unopposed", WHITE, 0, 1},
          {"supported", BLACK, 2, 1},
          {"weak_unopposed", BLACK, 0, 1}}},
        {"4k3/PP6/8/PP6/8/PP6/8/4K3",
         "three phalanxes on the third, fifth and seventh, each member "
         "counted; b3 and b5 doubled",
         {{"phalanx", WHITE, 1, 2},
          {"phalanx", WHITE, 3, 2},
          {"phalanx", WHITE, 5, 2},
          {"doubled_file", WHITE, 0, 2}}},
        {"4k3/8/PP6/8/PP6/8/PP6/4K3",
         "three on the second, fourth and sixth",
         {{"phalanx", WHITE, 0, 2},
          {"phalanx", WHITE, 2, 2},
          {"phalanx", WHITE, 4, 2},
          {"doubled_file", WHITE, 0, 2}}},
        {"4k3/8/8/8/8/8/PPP5/4K3",
         "a trio counts three",
         {{"phalanx", WHITE, 0, 3}}},
        {"7k/5P2/4P3/3P4/2P5/1P6/P7/K7",
         "a chain from a2 to f7: each link supported but a2, which is weak",
         {{"supported", WHITE, 0, 1},
          {"supported", WHITE, 1, 1},
          {"supported", WHITE, 2, 1},
          {"supported", WHITE, 3, 1},
          {"supported", WHITE, 4, 1},
          {"weak_unopposed", WHITE, 0, 1}}},
        {"4k3/8/8/8/8/1P6/P1P5/4K3",
         "b3 defended twice counts once; a2 and c2 weak",
         {{"supported", WHITE, 0, 1}, {"weak_unopposed", WHITE, 0, 2}}},
        {"4k3/8/p7/8/8/8/P7/4K3",
         "a2 and a6 weak but opposed: neither unopposed",
         {}},
        {"4k3/8/8/P7/p7/8/8/4K3",
         "a5 and a4 have passed each other: an enemy pawn behind does not "
         "oppose",
         {{"weak_unopposed", WHITE, 0, 1}, {"weak_unopposed", BLACK, 0, 1}}},
    };

    int positive[7][7] = {};
    int negative[7][7] = {};
    int white_row[7][7] = {};
    int black_row[7][7] = {};

    for (const family_case_t& item : cases) {
      for (const bool mirror : {false, true}) {
        const std::string placement = item.placement;
        const std::string what = item.what;
        const std::string fen = mirror ? mirror_fen(placement + " w - - 0 1")
                                       : placement + " w - - 0 1";
        CAPTURE(what);
        CAPTURE(mirror);

        structure_family_t engine;
        check_structure_family(fen, &engine);

        for (int a = 0; a < 7; ++a) {
          const std::string array = structure_arrays[a].name;
          int expected[2][7] = {};

          for (const family_cell_t& cell : item.cells) {
            if (array == cell.array) {
              const int colour = mirror ? 1 - cell.colour : cell.colour;
              expected[colour][cell.rank] = cell.value;
            }
          }

          for (int c = 0; c < structure_arrays[a].width; ++c) {
            for (int colour = 0; colour < 2; ++colour) {
              CAPTURE(array);
              CAPTURE(colour);
              CAPTURE(c);
              CHECK(structure_cell(engine, array, colour, c) ==
                    expected[colour][c]);
            }

            const int difference = expected[WHITE][c] - expected[BLACK][c];
            if (difference > 0) { positive[a][c]++; }
            if (difference < 0) { negative[a][c]++; }
            if (expected[WHITE][c] != 0) { white_row[a][c]++; }
            if (expected[BLACK][c] != 0) { black_row[a][c]++; }
          }
        }
      }
    }

    for (int a = 0; a < 7; ++a) {
      for (int c = 0; c < structure_arrays[a].width; ++c) {
        const std::string name = structure_arrays[a].name;
        CAPTURE(name);
        CAPTURE(c);
        CHECK_MESSAGE(positive[a][c] > 0, "never White ahead");
        CHECK_MESSAGE(negative[a][c] > 0, "never Black ahead");
        CHECK_MESSAGE(white_row[a][c] > 0, "no White row claimed");
        CHECK_MESSAGE(black_row[a][c] > 0, "no Black row claimed");
      }
    }
  }
}


// S101's threat family against the walk and by hand. The engine's sets and
// counts per attacking colour (threat_family_counts()) against
// walked_counts::threat_family_features(), which walks evaluation.hpp's
// definitions square by square and shares no code with the engine -- per
// colour, for the reason S123's family is: no score sees a group at zero, and
// at any weight a count credited to both sides cancels.
namespace
{

// The test positions with their colour mirrors, the JSON suites', and seeded
// random placements of two kings, pawns and a handful of pieces a side: the
// placements put each attacker against each victim, on every edge, far more
// often than the suites do. A placement is kept with White to move when
// load_FEN() accepts it, and is skipped otherwise.
std::vector<std::string> threat_population()
{
  std::vector<std::string> out;

  for (const std::string& fen : positions) {
    out.push_back(fen);
    out.push_back(mirror_fen(fen));
  }

  for (const std::string& fen : all_test_fens()) {
    out.push_back(fen);
  }

  std::mt19937 rng(20261101U);
  std::uniform_int_distribution<int> pawns_a_side(0, 8);
  std::uniform_int_distribution<int> pieces_of_a_kind(0, 2);

  for (size_t trial = 0; trial < 20000; ++trial) {
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

    for (const char piece : {'N', 'B', 'R', 'Q', 'n', 'b', 'r', 'q'}) {
      const int wanted = pieces_of_a_kind(rng);

      for (int placed = 0; placed < wanted && next < 64; ++placed) {
        squares[order[next++]] = piece;
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


// The sets and every cell, engine against walk, per colour.
void check_threat_family(const std::string& fen, threat_family_t* engine_out)
{
  CAPTURE(fen);
  REQUIRE(load_FEN(fen, &game));

  threat_family_t engine;
  threat_family_counts(&game.board, &engine);

  walked_counts::threat_family_walk_t walk;
  REQUIRE(walked_counts::threat_family_features(fen.substr(0, fen.find(' ')),
                                                &walk));

  const auto cells = [](const char* what, const auto& mine, const auto& theirs,
                        int width) {
    for (int colour = 0; colour < 2; ++colour) {
      for (int cell = 0; cell < width; ++cell) {
        const std::string name = what;
        CAPTURE(name);
        CAPTURE(colour);
        CAPTURE(cell);
        CHECK(mine[colour][cell] == theirs[colour][cell]);
      }
    }
  };

  for (int colour = 0; colour < 2; ++colour) {
    CAPTURE(colour);
    CHECK(engine.pawn_attacks[colour] == walk.attacks[colour][0]);
    CHECK(engine.knight_attacks[colour] == walk.attacks[colour][1]);
    CHECK(engine.bishop_attacks[colour] == walk.attacks[colour][2]);
    CHECK(engine.rook_attacks[colour] == walk.attacks[colour][3]);
    CHECK(engine.queen_attacks[colour] == walk.attacks[colour][4]);
    CHECK(engine.king_attacks[colour] == walk.attacks[colour][5]);
    CHECK(engine.nonking[colour] == walk.nonking[colour]);
    CHECK(engine.all[colour] == walk.all[colour]);
    CHECK(engine.hanging_set[colour] == walk.hanging_set[colour]);
    CHECK(engine.king_only_set[colour] == walk.king_only_set[colour]);
    CHECK(engine.push_targets[colour] == walk.push_targets[colour]);
    CHECK(engine.push_hits[colour] == walk.push_hits[colour]);
  }

  cells("threat_pawn", engine.threat_pawn, walk.threat_pawn, 4);
  cells("threat_piece", engine.threat_piece, walk.threat_piece, 5);
  cells("threat_minor", engine.threat_minor, walk.threat_minor, 2);
  cells("hanging_piece", engine.hanging_piece, walk.hanging_piece, 4);
  cells("hanging_pawn", engine.hanging_pawn, walk.hanging_pawn, 1);
  cells("threat_king", engine.threat_king, walk.threat_king, 4);
  cells("threat_push", engine.threat_push, walk.threat_push, 4);

  *engine_out = engine;
}


// One array's cell of the engine's counts, by the array's name.
int threat_cell(const threat_family_t& family,
                const std::string& array,
                int colour,
                int cell)
{
  if (array == "threat_pawn") { return family.threat_pawn[colour][cell]; }
  if (array == "threat_piece") { return family.threat_piece[colour][cell]; }
  if (array == "threat_minor") { return family.threat_minor[colour][cell]; }
  if (array == "hanging_piece") { return family.hanging_piece[colour][cell]; }
  if (array == "hanging_pawn") { return family.hanging_pawn[colour][cell]; }
  if (array == "threat_king") { return family.threat_king[colour][cell]; }
  return family.threat_push[colour][cell];
}

struct threat_array_t
{
  const char* name;
  int width;
};

const threat_array_t threat_arrays[] = {
    {"threat_pawn", 4},   {"threat_piece", 5}, {"threat_minor", 2},
    {"hanging_piece", 4}, {"hanging_pawn", 1}, {"threat_king", 4},
    {"threat_push", 4}};

}  // namespace


TEST_SUITE("eval counts: S101's threat family")
{
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the threat family the walk counts")
  {
    // Every cell, per colour, non-zero somewhere: the comparison above is
    // vacuous on a cell nobody reached.
    int seen[7][2][5] = {};
    size_t checked = 0;

    for (const std::string& fen : threat_population()) {
      threat_family_t engine;
      check_threat_family(fen, &engine);

      for (int a = 0; a < 7; ++a) {
        for (int colour = 0; colour < 2; ++colour) {
          for (int cell = 0; cell < threat_arrays[a].width; ++cell) {
            seen[a][colour][cell] +=
                (threat_cell(engine, threat_arrays[a].name, colour, cell) != 0);
          }
        }
      }

      checked++;
    }

    CHECK(checked > 10000);

    int fewest = INT32_MAX;
    for (int a = 0; a < 7; ++a) {
      for (int colour = 0; colour < 2; ++colour) {
        for (int cell = 0; cell < threat_arrays[a].width; ++cell) {
          const std::string name = threat_arrays[a].name;
          CAPTURE(name);
          CAPTURE(colour);
          CAPTURE(cell);
          CHECK(seen[a][colour][cell] > 50);
          fewest = std::min(fewest, seen[a][colour][cell]);
        }
      }
    }
    MESSAGE("threat family walk: " << checked
                                   << " positions; the rarest cell is "
                                   << "non-zero on " << fewest);
  }

  // Hand-worked placements from evaluation.hpp's definitions, each run as
  // written and as its colour mirror with the rows swapped. Every cell of every
  // array is reached, by each colour and with each sign of the difference,
  // which the loop at the end checks, and the placements with no cells at all
  // hold what would make a count happen and the one clause that stops it.
  // Cells: threat_pawn and threat_push N, B, R, Q; threat_piece N>R, N>Q, B>R,
  // B>Q, R>Q; threat_minor N>B, B>N; hanging_piece N, B, R, Q; threat_king P,
  // N, B, R.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count the threat family by hand")
  {
    const std::vector<family_case_t> cases = {
        {"k7/8/8/8/3n4/4P3/8/7K",
         "e3 attacks the knight on d4, which nothing defends",
         {{"threat_pawn", WHITE, 0, 1}, {"hanging_piece", WHITE, 0, 1}}},
        {"k7/8/8/8/3b4/4P3/8/7K",
         "e3 attacks the bishop on d4, and the bishop attacks e3: both loose",
         {{"threat_pawn", WHITE, 1, 1},
          {"hanging_piece", WHITE, 1, 1},
          {"hanging_pawn", BLACK, 0, 1}}},
        {"k7/8/8/8/3r4/4P3/8/7K",
         "e3 attacks the rook on d4; e4 is attacked and undefended, so no push",
         {{"threat_pawn", WHITE, 2, 1}, {"hanging_piece", WHITE, 2, 1}}},
        {"k7/8/8/8/3q4/4P3/8/7K",
         "e3 attacks the queen on d4, and the queen attacks e3",
         {{"threat_pawn", WHITE, 3, 1},
          {"hanging_piece", WHITE, 3, 1},
          {"hanging_pawn", BLACK, 0, 1}}},
        {"k7/8/8/8/3r4/8/4N3/7K",
         "the knight on e2 attacks the rook on d4",
         {{"threat_piece", WHITE, 0, 1}, {"hanging_piece", WHITE, 2, 1}}},
        {"k7/8/8/8/3q4/8/4N3/7K",
         "the knight on e2 attacks the queen on d4",
         {{"threat_piece", WHITE, 1, 1}, {"hanging_piece", WHITE, 3, 1}}},
        {"k7/8/8/8/3r4/8/5B2/7K",
         "the bishop on f2 attacks the rook on d4",
         {{"threat_piece", WHITE, 2, 1}, {"hanging_piece", WHITE, 2, 1}}},
        {"k7/8/8/8/3q4/8/5B2/7K",
         "the bishop on f2 attacks the queen on d4, which attacks it back",
         {{"threat_piece", WHITE, 3, 1},
          {"hanging_piece", WHITE, 3, 1},
          {"hanging_piece", BLACK, 1, 1}}},
        {"k7/8/8/8/3q4/8/8/3R3K",
         "the rook on d1 attacks the queen on d4, which attacks it back",
         {{"threat_piece", WHITE, 4, 1},
          {"hanging_piece", WHITE, 3, 1},
          {"hanging_piece", BLACK, 2, 1}}},
        {"k7/8/8/8/3r4/8/4NB2/7K",
         "a rook a knight and a bishop both attack counts in both pairs, and "
         "hangs once",
         {{"threat_piece", WHITE, 0, 1},
          {"threat_piece", WHITE, 2, 1},
          {"hanging_piece", WHITE, 2, 1}}},
        {"k7/8/8/8/3b4/8/4N3/7K",
         "the knight on e2 attacks the bishop on d4: a minor on a minor",
         {{"threat_minor", WHITE, 0, 1}, {"hanging_piece", WHITE, 1, 1}}},
        {"k7/8/8/8/3n4/8/5B2/7K",
         "the bishop on f2 attacks the knight on d4",
         {{"threat_minor", WHITE, 1, 1}, {"hanging_piece", WHITE, 0, 1}}},
        {"k7/8/8/3p4/8/8/3R4/7K",
         "the rook attacks d5, which nothing defends; d4 is attacked and "
         "undefended, so Black's push is not safe",
         {{"hanging_pawn", WHITE, 0, 1}}},
        {"k7/8/2p5/3p4/8/8/3R4/7K",
         "the same d5, defended by c6: an own pawn is defence",
         {}},
        {"8/8/8/3pk3/8/8/3R4/7K",
         "the same d5, defended by the king on e5: a king is defence",
         {}},
        {"k7/8/8/8/8/8/7p/7K",
         "h2 beside the king, which it does not attack: only the king attacks "
         "it",
         {{"threat_king", WHITE, 0, 1}}},
        {"k7/8/8/8/8/8/6n1/7K",
         "the knight on g2 the same",
         {{"threat_king", WHITE, 1, 1}}},
        {"k7/8/8/8/8/8/8/6bK",
         "a bishop beside the king on its rank does not attack it",
         {{"threat_king", WHITE, 2, 1}}},
        {"k7/8/8/8/8/8/6r1/7K",
         "a rook beside the king on its diagonal does not attack it",
         {{"threat_king", WHITE, 3, 1}}},
        {"k7/8/8/8/8/8/6n1/6RK",
         "the knight attacked by the king and the rook is hanging, not the "
         "king's alone",
         {{"hanging_piece", WHITE, 0, 1}}},
        {"k7/8/8/8/8/7p/6n1/7K",
         "the knight the king attacks, defended by h3: nothing",
         {}},
        {"k7/8/8/3n4/8/4P3/3P4/7K",
         "e3 to e4 would attack the knight; e3 stands defended by d2",
         {{"threat_push", WHITE, 0, 1}}},
        {"k7/8/8/3n4/8/8/4P3/7K",
         "e2 to e4, the double push: e3 is attacked by the knight and so no "
         "target, and no enemy pawn attacks it, so e4 is",
         {{"threat_push", WHITE, 0, 1}}},
        {"k7/8/8/2b5/8/3PP3/5P2/7K",
         "d3 to d4 attacks the bishop, which attacks d4: safe only because e3 "
         "defends it",
         {{"threat_push", WHITE, 1, 1}}},
        {"k7/8/8/3r4/8/4P3/8/7K",
         "e3 to e4 would attack the rook on d5",
         {{"threat_push", WHITE, 2, 1}}},
        {"k7/8/8/3q4/8/4PP2/6P1/7K",
         "e3 to e4 attacks the queen, which attacks e4: f3 defends it",
         {{"threat_push", WHITE, 3, 1}}},
        {"k7/8/8/3n1r2/8/4P3/3P4/7K",
         "e3 to e4 forks the knight and the rook: each counted",
         {{"threat_push", WHITE, 0, 1}, {"threat_push", WHITE, 2, 1}}},
        {"k7/8/8/3n1p2/8/4P3/3P4/7K",
         "e4 would attack the knight, but f5 attacks e4: not safe",
         {}},
        {"k7/8/8/3n4/2P5/4P3/8/7K",
         "e4 would attack the knight c4 attacks already: no push cell; it "
         "attacks e3, which nothing defends",
         {{"threat_pawn", WHITE, 0, 1},
          {"hanging_piece", WHITE, 0, 1},
          {"hanging_pawn", BLACK, 0, 1}}},
        {"k7/8/8/3n4/5p2/8/4P3/7K",
         "f4 attacks e3, so e2 has no double push, though e4 would attack the "
         "knight",
         {}},
    };

    int positive[7][5] = {};
    int negative[7][5] = {};
    int white_row[7][5] = {};
    int black_row[7][5] = {};

    for (const family_case_t& item : cases) {
      for (const bool mirror : {false, true}) {
        const std::string placement = item.placement;
        const std::string what = item.what;
        const std::string fen = mirror ? mirror_fen(placement + " w - - 0 1")
                                       : placement + " w - - 0 1";
        CAPTURE(what);
        CAPTURE(mirror);

        threat_family_t engine;
        check_threat_family(fen, &engine);

        for (int a = 0; a < 7; ++a) {
          const std::string array = threat_arrays[a].name;
          int expected[2][5] = {};

          for (const family_cell_t& cell : item.cells) {
            if (array == cell.array) {
              const int colour = mirror ? 1 - cell.colour : cell.colour;
              expected[colour][cell.rank] = cell.value;
            }
          }

          for (int c = 0; c < threat_arrays[a].width; ++c) {
            for (int colour = 0; colour < 2; ++colour) {
              CAPTURE(array);
              CAPTURE(colour);
              CAPTURE(c);
              CHECK(threat_cell(engine, array, colour, c) ==
                    expected[colour][c]);
            }

            const int difference = expected[WHITE][c] - expected[BLACK][c];
            if (difference > 0) { positive[a][c]++; }
            if (difference < 0) { negative[a][c]++; }
            if (expected[WHITE][c] != 0) { white_row[a][c]++; }
            if (expected[BLACK][c] != 0) { black_row[a][c]++; }
          }
        }
      }
    }

    for (int a = 0; a < 7; ++a) {
      for (int c = 0; c < threat_arrays[a].width; ++c) {
        const std::string name = threat_arrays[a].name;
        CAPTURE(name);
        CAPTURE(c);
        CHECK_MESSAGE(positive[a][c] > 0, "never White ahead");
        CHECK_MESSAGE(negative[a][c] > 0, "never Black ahead");
        CHECK_MESSAGE(white_row[a][c] > 0, "never in White's row");
        CHECK_MESSAGE(black_row[a][c] > 0, "never in Black's row");
      }
    }
  }
}


// S122's king safety family against the walk and by hand (DEC-344 (5)): the
// trace reads the engine's own variables, so no identity scan can see a count
// that is wrong in the engine, and the walk is the one implementation of each
// count that shares no code with it. Per king, every set bit for bit and every
// cell, over S101's population, whose seeded placements put each piece against
// each king on every edge.
namespace
{

void check_king_danger(const std::string& fen, king_danger_t* engine_out)
{
  CAPTURE(fen);
  REQUIRE(load_FEN(fen, &game));

  king_danger_t engine;
  king_danger_counts(&game.board, &engine);

  walked_counts::king_danger_walk_t walk;
  REQUIRE(
      walked_counts::king_danger_features(fen.substr(0, fen.find(' ')), &walk));

  const auto cells = [](const char* what, const auto& mine, const auto& theirs,
                        int width) {
    for (int king = 0; king < 2; ++king) {
      for (int cell = 0; cell < width; ++cell) {
        const std::string name = what;
        CAPTURE(name);
        CAPTURE(king);
        CAPTURE(cell);
        CHECK(mine[king][cell] == theirs[king][cell]);
      }
    }
  };

  for (int king = 0; king < 2; ++king) {
    CAPTURE(king);
    for (int type = 0; type < 4; ++type) {
      CAPTURE(type);
      CHECK(engine.check_targets[king][type] == walk.check_targets[king][type]);
    }
    CHECK(engine.weak_set[king] == walk.weak_set[king]);
    CHECK(engine.defended_set[king] == walk.defended_set[king]);
  }

  cells("checks", engine.checks, walk.checks, 4);
  cells("checks_any", engine.checks_any, walk.checks_any, 4);
  cells("weak", engine.weak, walk.weak, 1);
  cells("defenders", engine.defenders, walk.defenders, 1);
  cells("storm", engine.storm, walk.storm, KST_CELL_COUNT);
  cells("curve", engine.curve, walk.curve, KS_CURVE_INPUTS);

  *engine_out = engine;
}


struct danger_array_t
{
  const char* name;
  int width;
};

const danger_array_t danger_arrays[] = {
    {"checks", 4},    {"checks_any", 4},         {"weak", 1},
    {"defenders", 1}, {"storm", KST_CELL_COUNT}, {"curve", KS_CURVE_INPUTS}};

int danger_cell(const king_danger_t& family,
                const std::string& array,
                int king,
                int cell)
{
  if (array == "checks") { return family.checks[king][cell]; }
  if (array == "checks_any") { return family.checks_any[king][cell]; }
  if (array == "weak") { return family.weak[king][cell]; }
  if (array == "defenders") { return family.defenders[king][cell]; }
  if (array == "storm") { return family.storm[king][cell]; }
  return family.curve[king][cell];
}


// A hand case asserts the arrays it names, every cell of them for both kings,
// and leaves the others to the walk: a position's every safe check for both
// kings by hand is a count nobody could check, and the walk checks it.
struct danger_case_t
{
  const char* placement;
  const char* what;
  std::vector<std::string> arrays;
  std::vector<family_cell_t> cells;  // `rank` is the cell, `colour` the king
};

}  // namespace


TEST_SUITE("eval counts: S122's king safety family")
{
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the engine counts the king safety family the walk counts")
  {
    // Every cell, per king, non-zero somewhere, so the comparison is not
    // vacuous; the storm's distance-1 cell only by its hand case below, since
    // a random placement rarely puts a pawn beside a king that is not in check.
    int seen[6][2][KS_CURVE_INPUTS] = {};
    size_t checked = 0;

    for (const std::string& fen : threat_population()) {
      king_danger_t engine;
      check_king_danger(fen, &engine);

      for (int a = 0; a < 6; ++a) {
        for (int king = 0; king < 2; ++king) {
          for (int cell = 0; cell < danger_arrays[a].width; ++cell) {
            seen[a][king][cell] +=
                (danger_cell(engine, danger_arrays[a].name, king, cell) != 0);
          }
        }
      }

      checked++;
    }

    CHECK(checked > 10000);

    int fewest = INT32_MAX;
    for (int a = 0; a < 6; ++a) {
      for (int king = 0; king < 2; ++king) {
        for (int cell = 0; cell < danger_arrays[a].width; ++cell) {
          const std::string name = danger_arrays[a].name;
          CAPTURE(name);
          CAPTURE(king);
          CAPTURE(cell);
          CHECK(seen[a][king][cell] > 50);
          fewest = std::min(fewest, seen[a][king][cell]);
        }
      }
    }
    MESSAGE("king safety family walk: " << checked
                                        << " positions; the rarest cell is "
                                        << "non-zero on " << fewest);
  }

  // Hand-worked placements from evaluation.hpp's definitions, each as written
  // and as its colour mirror. Every storm cell is reached by each king, among
  // them distance 1 blocked, DEC-344 (3)'s mapping: White's king on g2 with
  // its pawn on h2, and Black's pawn on h3 giving check.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "the walk and the engine count the king safety family by "
                    "hand")
  {
    const std::vector<danger_case_t> cases = {
        {"4k3/8/8/8/8/7p/6KP/8",
         "h3 beside g2 with h2 before it: distance 1 blocked, distance 1's "
         "cell",
         {"storm"},
         {{"storm", WHITE, KST_D1, 1}}},
        {"6k1/8/8/8/8/7p/6K1/8",
         "h3 beside g2 with nothing before it: distance 1",
         {"storm"},
         {{"storm", WHITE, KST_D1, 1}}},
        {"6k1/8/8/8/7p/8/6K1/8",
         "h4, two ranks in front of g2, free",
         {"storm"},
         {{"storm", WHITE, KST_D2_FREE, 1}}},
        {"6k1/8/8/8/7p/7P/6K1/8",
         "h4 with White's h3 before it: blocked",
         {"storm"},
         {{"storm", WHITE, KST_D2_BLOCKED, 1}}},
        {"6k1/8/8/7p/8/8/6K1/8",
         "h5, three in front, free",
         {"storm"},
         {{"storm", WHITE, KST_D3_FREE, 1}}},
        {"6k1/8/8/7p/7P/8/6K1/8",
         "h5 blocked by h4; for Black's king h4 stands four in front, "
         "blocked by its own h5",
         {"storm"},
         {{"storm", WHITE, KST_D3_BLOCKED, 1},
          {"storm", BLACK, KST_D4_BLOCKED, 1}}},
        {"6k1/8/7p/8/8/8/6K1/8",
         "h6, four in front, free",
         {"storm"},
         {{"storm", WHITE, KST_D4_FREE, 1}}},
        {"6k1/8/7p/7P/8/8/6K1/8",
         "h6 blocked by h5; for Black's king h5 stands three in front, "
         "blocked by h6",
         {"storm"},
         {{"storm", WHITE, KST_D4_BLOCKED, 1},
          {"storm", BLACK, KST_D3_BLOCKED, 1}}},
        {"6k1/8/8/7p/8/7p/6K1/8",
         "only the closest pawn on a file counts",
         {"storm"},
         {{"storm", WHITE, KST_D1, 1}}},
        {"6k1/8/8/8/8/8/5r2/6K1",
         "the rook attacks f1, g2 and h2 beside the king, which nothing of "
         "White's but the king defends; f1 and g2 are checks beside the king "
         "no other Black man attacks, so not safe",
         {"weak", "checks", "defenders"},
         {{"weak", WHITE, 0, 3}}},
        {"6k1/8/8/8/8/4N3/5r2/6K1",
         "the knight on e3 defends f1 and g2: h2 alone is weak",
         {"weak", "checks", "defenders"},
         {{"weak", WHITE, 0, 1}, {"defenders", WHITE, 0, 2}}},
        {"6k1/8/8/3N4/8/8/8/6K1",
         "the knight on d5 reaches e7 and f6, from which it would check g8",
         {"checks", "checks_any", "weak"},
         {{"checks", BLACK, 0, 2}, {"checks_any", BLACK, 0, 2}}},
        {"6k1/6p1/8/3N4/8/8/8/6K1",
         "g7 defends f6: e7 alone is safe",
         {"checks", "checks_any", "weak"},
         {{"checks", BLACK, 0, 1}, {"checks_any", BLACK, 0, 1}}},
        {"6k1/4b3/8/3N4/8/8/8/6K1",
         "a check that takes the bishop on e7 is S101's, by target: counted "
         "only without the empty-square clause; the bishop guards f6, and "
         "reaches c5 on g1's diagonal, which no White man guards",
         {"checks", "checks_any", "weak"},
         {{"checks_any", BLACK, 0, 1},
          {"checks", WHITE, 1, 1},
          {"checks_any", WHITE, 1, 1}}},
        {"6k1/8/8/8/8/8/8/5BK1",
         "the bishop on f1 reaches c4 on g8's diagonal",
         {"checks", "checks_any"},
         {{"checks", BLACK, 1, 1}, {"checks_any", BLACK, 1, 1}}},
        {"6k1/8/8/8/8/8/8/R5K1",
         "the rook on a1 reaches a8 on g8's rank",
         {"checks", "checks_any"},
         {{"checks", BLACK, 2, 1}, {"checks_any", BLACK, 2, 1}}},
        {"6k1/8/8/8/8/8/8/3Q2K1",
         "the queen on d1 reaches d5, d8, b3 and g4",
         {"checks", "checks_any"},
         {{"checks", BLACK, 3, 4}, {"checks_any", BLACK, 3, 4}}},
        {"6k1/8/8/8/8/8/8/2B3K1",
         "the bishop on c1 reaches no square of g8's diagonals",
         {"checks", "checks_any"},
         {}},
        {"6k1/8/7P/8/8/8/8/5RK1",
         "the rook on f1 reaches f8 beside g8, which no other White man "
         "attacks; the pawn on h6 attacks g7, which the rook does not reach",
         {"checks", "checks_any"},
         {}},
        {"6k1/8/6N1/8/8/8/8/5RK1",
         "the knight on g6 attacks f8: the rook's contact check there is "
         "safe; the knight itself reaches f8, h8 and e7, and e7 checks",
         {"checks", "checks_any"},
         {{"checks", BLACK, 2, 1},
          {"checks_any", BLACK, 2, 1},
          {"checks", BLACK, 0, 1},
          {"checks_any", BLACK, 0, 1}}},
        {"6k1/5n2/8/8/8/8/8/R5K1",
         "the rook's check on a8 stands, and the knight on f7 guards nothing "
         "on the rank: still one",
         {"checks", "checks_any"},
         {{"checks", BLACK, 2, 1}, {"checks_any", BLACK, 2, 1}}},
        {"6k1/8/8/8/8/2n5/8/R5K1",
         "Black's knight on c3 guards nothing on the eighth: the rook's check "
         "stands; it reaches e2, from which it would check g1, and no White "
         "man guards e2",
         {"checks", "checks_any"},
         {{"checks", BLACK, 2, 1},
          {"checks_any", BLACK, 2, 1},
          {"checks", WHITE, 0, 1},
          {"checks_any", WHITE, 0, 1}}},
        {"6k1/8/8/8/8/8/6B1/6K1",
         "the bishop on g2 attacks h1, f1, h3, f3: g1's defenders by square "
         "are f1 and h1; and it reaches d5 on g8's diagonal",
         {"defenders", "checks"},
         {{"defenders", WHITE, 0, 2}, {"checks", BLACK, 1, 1}}},
    };

    int positive[6][KS_CURVE_INPUTS] = {};
    int negative[6][KS_CURVE_INPUTS] = {};

    for (const danger_case_t& item : cases) {
      for (const bool mirror : {false, true}) {
        const std::string placement = item.placement;
        const std::string what = item.what;
        const std::string fen = mirror ? mirror_fen(placement + " w - - 0 1")
                                       : placement + " w - - 0 1";
        CAPTURE(what);
        CAPTURE(mirror);

        king_danger_t engine;
        check_king_danger(fen, &engine);

        for (int a = 0; a < 6; ++a) {
          const std::string array = danger_arrays[a].name;
          if (std::find(item.arrays.begin(), item.arrays.end(), array) ==
              item.arrays.end()) {
            continue;
          }

          int expected[2][KS_CURVE_INPUTS] = {};

          for (const family_cell_t& cell : item.cells) {
            if (array == cell.array) {
              const int king = mirror ? 1 - cell.colour : cell.colour;
              expected[king][cell.rank] = cell.value;
            }
          }

          for (int c = 0; c < danger_arrays[a].width; ++c) {
            for (int king = 0; king < 2; ++king) {
              CAPTURE(array);
              CAPTURE(king);
              CAPTURE(c);
              CHECK(danger_cell(engine, array, king, c) == expected[king][c]);
            }

            const int difference = expected[WHITE][c] - expected[BLACK][c];
            if (difference > 0) { positive[a][c]++; }
            if (difference < 0) { negative[a][c]++; }
          }
        }
      }
    }

    // Each storm cell reached by each king, with each sign of the difference.
    for (int c = 0; c < KST_CELL_COUNT; ++c) {
      CAPTURE(c);
      CHECK_MESSAGE(positive[4][c] > 0, "never White's");
      CHECK_MESSAGE(negative[4][c] > 0, "never Black's");
    }
  }

  // DEC-344 (3)'s position, read through the engine's own evaluation: the
  // mapping is a count and not a refusal, and the position is one the engine
  // loads and scores.
  TEST_CASE_FIXTURE(counts_fixture_t,
                    "distance 1 blocked is read on the position that has it")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/7p/6KP/8 w - - 0 1", &game));
    CHECK(is_check(&game));

    king_danger_t engine;
    king_danger_counts(&game.board, &engine);
    CHECK(engine.storm[WHITE][KST_D1] == 1);

    for (int cell = KST_D2_FREE; cell < KST_CELL_COUNT; ++cell) {
      CAPTURE(cell);
      CHECK(engine.storm[WHITE][cell] == 0);
    }
  }
}
