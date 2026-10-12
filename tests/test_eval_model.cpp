#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_model.hpp"
#include "evaluation.hpp"
#include "test_eval_positions.hpp"
#include "test_temp_file.hpp"
#include "tuner_model.hpp"

// The tuner does not call evaluate(). It fits a linear model of evaluate()'s
// own constants, because a derivative is what a fit needs and because a pass
// over a million positions has to cost milliseconds. That model is only useful
// while it computes the same number the engine does, and a model that drifts
// would tune the wrong function and produce constants that look fitted and are
// not. Nothing else in the project would notice.
//
// So the claim is checked here, against the engine, on positions from every
// phase and with both sides to move. S028.

static game_t game;


struct model_fixture_t
{
  model_fixture_t() { initialize_game_const_data(&game); }
};


// The FEN's second field. Every feature but tempo is read out of the placement
// alone; that one is read out of this, and a substr that stopped at the first
// space would silently hand it whatever the caller's default was.
static std::string side_to_move_of(const std::string& fen)
{
  const size_t first = fen.find(' ');
  const size_t second = fen.find(' ', first + 1);

  return fen.substr(first + 1, second - first - 1);
}


// The engine's score is side-to-move relative (INV-5); the model's is White
// relative. One negation is the whole difference and getting it wrong is
// exactly the kind of mistake this file exists to catch.
static double model_score_white(const std::string& fen, const double* params)
{
  std::vector<uint16_t> pieces;
  const std::string placement = fen.substr(0, fen.find(' '));

  REQUIRE(eval_model::parse_placement(placement, &pieces));

  const int phase = eval_model::phase_of(pieces.data(), pieces.size());

  int mobility[4] = {0, 0, 0, 0};
  REQUIRE(eval_model::mobility_features(placement, mobility));

  int king_safety[eval_model::KING_SAFETY_COUNT] = {};
  REQUIRE(eval_model::king_safety_features(placement, king_safety));

  int passed_pawn[eval_model::PASSED_PAWN_COUNT] = {};
  REQUIRE(eval_model::passed_pawn_features(placement, passed_pawn));

  int pawn_structure[eval_model::PAWN_STRUCTURE_COUNT] = {};
  REQUIRE(eval_model::pawn_structure_features(placement, pawn_structure));

  int piece_placement[eval_model::PIECE_PLACEMENT_COUNT] = {};
  REQUIRE(eval_model::piece_placement_features(placement, piece_placement));

  int tempo = 0;
  REQUIRE(eval_model::tempo_feature(side_to_move_of(fen), &tempo));

  return eval_model::evaluate(pieces.data(), pieces.size(), phase, mobility,
                              params, king_safety, passed_pawn, pawn_structure,
                              piece_placement, tempo);
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
    pawn_structure_feature_names[eval_model::PAWN_STRUCTURE_COUNT] = {
        "isolated", "doubled", "backward"};


// The same for the three piece placement counts, in the order the weights are
// indexed.
static const char* const
    piece_placement_feature_names[eval_model::PIECE_PLACEMENT_COUNT] = {
        "bishop pair", "rook on an open file", "rook on a half open file"};


// The curated corpus moved to tests/test_eval_positions.hpp at S100 so that
// tests/test_tuner_gradient.cpp runs over the same list. The non-vacuity cases
// below are still what holds its coverage; nothing about the list changed.
using test_eval_positions::positions;


// The subset of `positions` pinned for the truncation bound rather than for a
// feature count, in the order they are listed above. Named separately because
// "the pinned positions reach the truncation bound" has to be able to fail when
// *these* stop disagreeing with the model, which is a different statement from
// the whole corpus agreeing within tolerance. S038, re-measured at S065 and at
// S076.
//
// GOLDEN (DEC-142): the four positions themselves, drawn from the rows that
// reach the maximum a chain of truncating divisions can produce. A residual
// belongs to the weights and not to the position (DEC-057), so a refit is
// expected to stop these four qualifying.
// Re-derive: build/tools/truncation_scan --data <corpus> --min 2.8, which
// prints `difference, phase, side to move, fen` per hit; the four are chosen
// from the hits at the maximum to span the taper with both sides to move.
// Measured 2026-09-10 on .tuning/selfplay_v2_dedup.tsv at the shipped weights:
// 10795695 rows, 138331 past 2.0, 105 past 2.8, 33 at the maximum 2.875, and
// all four below are among them.
// Moves legitimately on: a refit, and a move of LAZY_EVAL_MARGIN -- the model
// and evaluate() both clamp the tapered mobility-plus-king-safety sum at that
// margin, so while the clamp binds the two agree exactly and the residual this
// pins is not there to be measured. S085 raising it from 150 to 184 took the
// corpus from 99 rows past 2.8 to 105 at unmoved weights, which is half of what
// S206 bisected. Margin: each of the four is asserted past
// 2.0 and the worst of them past 2.8, against a maximum of 2.875 -- so the
// tightest of the two clauses has 0.075 of room, which is the point, and
// re-choosing four from a fresh scan is the re-derivation.
// Property beside it: "the model reproduces evaluate() on every phase", which
// holds over the whole corpus rather than over four chosen rows -- but not at
// any weights: its tolerance is a function of how many taperings truncate, and
// a fitted tempo takes the bound to 4 x 23/24 = 3.833 and the tolerance to 4,
// which "the pinned positions reach the truncation bound" asserts below.
static const std::vector<std::string> truncation_positions = {
    "8/8/8/6k1/1p1pr3/1Pp4P/2P2KP1/1N1R4 b - - 1 44",
    "1n6/1p2np2/3k2p1/3P4/8/5BPP/r1b2PN1/1R2K2R w K - 6 33",
    "r1b2rk1/1p1p2p1/p2pq2p/5B2/2PppP2/P2P2Q1/1P1N2PP/2R2RK1 b - - 0 18",
    "r2qkb1r/pp1b1ppp/3p4/2p1p3/2PnP3/2NP4/PP4PP/R1BQKBNR w kq - 2 10",
};


// One hand-built placement and the passed pawn count per bucket it must
// produce, White minus Black. Hand-built so that the answer comes from the
// specification both implementations were written from and not from either of
// them: two extractions agreeing say nothing about whether the thing they agree
// on is what was specified.
struct passed_pawn_case_t
{
  const char* placement;
  const char* what;
  int expected[eval_model::PASSED_PAWN_COUNT];
};


// The engine's passed_pawn_counts() takes its rows with a literal 5 and the
// model counts its buckets off the engine's weight array. Nothing forces the
// two to be the same number, and if they ever stop being one the loops below
// would read off the end of a row rather than fail. Six until S134 folded the
// seventh-rank bucket into the pawn table (DEC-090).
static_assert(eval_model::PASSED_PAWN_COUNT == 5,
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
                                 int out[2][eval_model::PASSED_PAWN_COUNT])
{
  constexpr int GUARD = -12345;
  int rows[3][eval_model::PASSED_PAWN_COUNT];

  for (auto& row : rows) {
    for (int& value : row) {
      value = GUARD;
    }
  }

  passed_pawn_counts(board, rows);

  for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT; ++bucket) {
    CAPTURE(bucket);
    CHECK_MESSAGE(rows[2][bucket] == GUARD,
                  "passed_pawn_counts() wrote past Black's row");
  }

  for (int colour = 0; colour < 2; ++colour) {
    for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT; ++bucket) {
      out[colour][bucket] = rows[colour][bucket];
    }
  }
}


// One hand-built placement and the pawn structure counts it must produce, per
// colour rather than as the difference the model reports.
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
  int white[eval_model::PAWN_STRUCTURE_COUNT];
  int black[eval_model::PAWN_STRUCTURE_COUNT];
};


// The engine indexes its counts with a literal 3, the model with a constant
// taken from the width of the engine's own weight array. Nothing forces those
// to be the same number, and if they ever stop being one the loops below would
// read off the end of a row rather than fail.
static_assert(eval_model::PAWN_STRUCTURE_COUNT == 3,
              "the engine's pawn_structure_counts() rows are three wide");


// One hand-built placement and the piece placement counts it must produce, per
// colour rather than as the difference the model reports.
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
  int white[eval_model::PIECE_PLACEMENT_COUNT];
  int black[eval_model::PIECE_PLACEMENT_COUNT];
};


// The engine indexes its counts with a literal 3, the model with a constant
// taken from the width of the engine's own weight array. Nothing forces those
// to be the same number, and if they ever stop being one the loops below would
// read off the end of a row rather than fail. Four until S134 deleted rook on
// the seventh (DEC-090).
static_assert(eval_model::PIECE_PLACEMENT_COUNT == 3,
              "the engine's piece_placement_counts() rows are three wide");


TEST_SUITE("eval model: agrees with the engine")
{
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the model reproduces evaluate() on every phase")
  {
    std::vector<double> params(eval_model::PARAM_COUNT, 0.0);
    eval_model::starting_params(params.data());

    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      const int engine_relative = evaluate(&game.board);
      const int engine_white = (game.board.active_color == WHITE)
                                   ? engine_relative
                                   : -engine_relative;

      const double model = model_score_white(fen, params.data());

      // evaluate() interpolates in integers and truncates towards zero, and it
      // does that four separate times, each rounding on its own, so the
      // arithmetic alone can differ from the model's floating point by more
      // than one centipawn. The four divisions by GAME_PHASE_MAX are, in the
      // order they execute:
      //
      //   src/evaluation.cpp:640  the piece-square pair plus the pawn terms
      //   src/evaluation.cpp:668  tempo
      //   src/evaluation.cpp:951  mobility
      //   src/evaluation.cpp:953  king safety
      //
      // Three of them can round today. Tempo ships at tempo_mg == tempo_eg == 0
      // (src/evaluation.cpp:582-583), so its division truncates 0 / 24 exactly
      // and contributes nothing, which puts the bound at 3 x 23/24 = 2.875 and
      // this tolerance at 3. "the pinned positions reach the truncation bound"
      // below asserts that premise, because fitting tempo makes the fourth
      // division round too and the bound 4 x 23/24 = 3.833.
      //
      // The 2.0 this used to allow, with a comment claiming two divisions, was
      // false arithmetic that only passed because the corpus above was
      // hand-picked: the audit measured 3446 of 200000 real positions past it,
      // worst exactly 2.875. Four of those positions are pinned above. S038,
      // DEC-053.
      //
      // Anything beyond the bound is a difference in what is being computed,
      // which is what this file is for.
      //
      // King safety and passed pawns are inside this number but say nothing
      // while their weights are zero: each contributes 0 on both sides however
      // far apart the two implementations' counts are. What checks the counts
      // is the two cases below; this one starts checking them for free the
      // moment the fits land. S027.
      //
      // Tempo is inside it too and is the one term with no counts to check
      // separately, because there is nothing to count: the feature is the side
      // to move. What stands in for the count cases is that the corpus carries
      // positions of each side to move -- "the positions have both sides to
      // move" below -- so that once the weights are fitted a model that read
      // the wrong field, or read it White relative when the engine reads it
      // side-to-move relative, fails here rather than on half the corpus by
      // luck.
      CHECK(std::abs(model - engine_white) <= 3.0);
    }
  }

  // Non-vacuous by construction, and the reason the tolerance above is a bound
  // rather than a number someone widened until the suite went green. The
  // twenty-six feature positions all divide evenly enough to agree within 2.0,
  // so at that tolerance the case above was passing for a reason unconnected to
  // the arithmetic it cites -- which is exactly how the real corpus came to
  // violate it on 1.6 % of positions unnoticed. Each pinned position has to
  // still disagree by more than the old 2.0, or the corpus has stopped
  // exercising the bound and the tolerance is untested again.
  //
  // 2.0 is the threshold the arithmetic gives and not a round number. Each of
  // the three divisions loses less than one unit, at most 23/24, so two of them
  // together cannot reach past 46/24 = 1.9167. A residual over 2.0 = 48/24 is
  // therefore proof that all three divisions truncated on that position, which
  // is what "this position exercises the bound" means; a position below it may
  // be exercising only two. Every position pinned above sits at 69/24 = 2.875,
  // all three losing their maximum at once.
  //
  // The precondition is the tempo weights, because the count of *effective*
  // truncations depends on them: at tempo_mg == tempo_eg == 0 the tempo
  // division truncates 0 / 24 exactly and three of the four divisions can
  // round, bounding the disagreement at 3 x 23/24 = 2.875. Fit tempo to
  // anything non-zero and the fourth division starts rounding too, the bound
  // becomes 4 x 23/24 = 3.833, and the tolerance above has to go to 4. S038.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the pinned positions reach the truncation bound")
  {
    // As a variable because doctest decomposes the expression it is handed and
    // refuses a `&&` inside one.
    const bool tempo_unfitted = (tempo_mg == 0) && (tempo_eg == 0);

    CHECK_MESSAGE(tempo_unfitted,
                  "tempo is fitted, so all four taperings can truncate: the "
                  "bound is now 4 x 23/24 = 3.833 and the tolerance in \"the "
                  "model reproduces evaluate() on every phase\" has to be 4");

    std::vector<double> params(eval_model::PARAM_COUNT, 0.0);
    eval_model::starting_params(params.data());

    double worst = 0.0;

    for (const std::string& fen : truncation_positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      const int engine_relative = evaluate(&game.board);
      const int engine_white = (game.board.active_color == WHITE)
                                   ? engine_relative
                                   : -engine_relative;

      const double model = model_score_white(fen, params.data());
      const double difference = std::abs(model - engine_white);

      worst = std::max(worst, difference);

      CHECK_MESSAGE(difference > 2.0,
                    (fen + ": difference " + std::to_string(difference) +
                     ", no longer past the old 2.0 tolerance"));
    }

    // And one of them has to sit at the top of the range, not merely past 2.0,
    // so the tolerance is pinned against the bound it claims rather than
    // against whatever the mildest of these four happens to be.
    CHECK_MESSAGE(worst > 2.8, ("worst pinned difference " +
                                std::to_string(worst) + ", short of 2.875"));
  }

  // The king safety counts themselves, per colour, engine against model. This
  // is the case with teeth today.
  //
  // Per colour and not as the White-minus-Black difference the tuner consumes,
  // because that difference cancels: a bug that credits both kings with one
  // extra attacker is invisible in it and shows up here. The two
  // implementations were written independently from the same specification and
  // neither reads the other -- that is what eval_model.hpp is for, and a shared
  // extraction would agree with itself rather than with the engine.
  TEST_CASE_FIXTURE(model_fixture_t, "the model counts what the engine counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][KS_FEATURE_COUNT];
      king_safety_features(&game.board, engine);

      int model[2][eval_model::KING_SAFETY_COUNT];
      REQUIRE(eval_model::king_safety_features_by_colour(
          fen.substr(0, fen.find(' ')), model));

      for (int colour = 0; colour < 2; ++colour) {
        for (int feature = 0; feature < KS_FEATURE_COUNT; ++feature) {
          const std::string who = (colour == WHITE) ? "White " : "Black ";

          CHECK_MESSAGE(engine[colour][feature] == model[colour][feature],
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
  TEST_CASE_FIXTURE(model_fixture_t, "the positions exercise every count")
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

  // The passed pawn counts, engine against model, over the same corpus. The
  // case with teeth for this term, for the reason the king safety pair above
  // has teeth: the weights ship at zero, so the evaluate() comparison compares
  // 0 against 0 here and would pass however far apart the two counts are.
  //
  // What can be compared is the White-minus-Black difference and not the two
  // rows: eval_model::passed_pawn_features() reports the difference and nothing
  // else, which is the form the tuner is linear in. The rows are checked
  // against hand-computed answers instead, in the case below, on the placements
  // where only one side has a pawn -- and the engine's rows are checked for
  // both signs and every bucket in the case after this one, which is what a
  // difference-only comparison cannot see: a bug that credits both sides
  // equally cancels.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the engine counts the passed pawns the model counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][eval_model::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      int model[eval_model::PASSED_PAWN_COUNT] = {};
      REQUIRE(eval_model::passed_pawn_features(fen.substr(0, fen.find(' ')),
                                               model));

      for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT;
           ++bucket) {
        CAPTURE(bucket);
        CAPTURE(engine[WHITE][bucket]);
        CAPTURE(engine[BLACK][bucket]);

        CHECK(engine[WHITE][bucket] - engine[BLACK][bucket] == model[bucket]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A bucket that is zero in every position of the corpus is a bucket the two
  // implementations agree about for free, and the two colours count their ranks
  // in opposite directions, so a rank arithmetic error that only hits one of
  // them needs both rows claimed to be caught.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the positions exercise every passed pawn bucket")
  {
    int seen[2][eval_model::PASSED_PAWN_COUNT] = {};
    int white_ahead[eval_model::PASSED_PAWN_COUNT] = {};
    int black_ahead[eval_model::PASSED_PAWN_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][eval_model::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT;
           ++bucket) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][bucket] != 0) { seen[colour][bucket]++; }
        }

        const int difference = engine[WHITE][bucket] - engine[BLACK][bucket];

        if (difference > 0) { white_ahead[bucket]++; }
        if (difference < 0) { black_ahead[bucket]++; }
      }
    }

    for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT; ++bucket) {
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
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the model and the engine count passed pawns by rank")
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
        // and the c5 pawn is what is left, so a model that lost the sign would
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

    int positive[eval_model::PASSED_PAWN_COUNT] = {};
    int negative[eval_model::PASSED_PAWN_COUNT] = {};
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

      int model[eval_model::PASSED_PAWN_COUNT] = {};
      REQUIRE(eval_model::passed_pawn_features(item.placement, model));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][eval_model::PASSED_PAWN_COUNT];
      engine_passed_counts(&game.board, engine);

      // Which rows the difference pins. A placement with no black pawn on it
      // cannot hide a Black count inside a zero difference, and the mirror.
      const bool white_only = placement.find('p') == std::string::npos;
      const bool black_only = placement.find('P') == std::string::npos;

      white_only_cases += white_only ? 1 : 0;
      black_only_cases += black_only ? 1 : 0;
      mixed_cases += (!white_only && !black_only) ? 1 : 0;

      for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT;
           ++bucket) {
        CAPTURE(bucket);
        CHECK(model[bucket] == item.expected[bucket]);
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
    for (size_t bucket = 0; bucket < eval_model::PASSED_PAWN_COUNT; ++bucket) {
      CAPTURE(bucket);
      CHECK(positive[bucket] > 0);
      CHECK(negative[bucket] > 0);
    }
  }

  // The pawn structure counts, engine against model, over the same corpus, and
  // the case with teeth for this term for the reason the two pairs above have
  // teeth: the weights ship at zero, so the evaluate() comparison compares 0
  // against 0 here and would pass however far apart the two counts are.
  //
  // eval_model::pawn_structure_features() reports White minus Black and nothing
  // else, which is the form the tuner is linear in, so that is what can be
  // compared here. The rows themselves are pinned against hand-computed answers
  // in the case below, which is what a difference cannot see: a miscount that
  // hits both sides equally cancels.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the engine counts the pawn structure the model counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      int model[eval_model::PAWN_STRUCTURE_COUNT] = {};
      REQUIRE(eval_model::pawn_structure_features(fen.substr(0, fen.find(' ')),
                                                  model));

      for (size_t feature = 0; feature < eval_model::PAWN_STRUCTURE_COUNT;
           ++feature) {
        const std::string name = pawn_structure_feature_names[feature];

        CAPTURE(name);
        CAPTURE(engine[WHITE][feature]);
        CAPTURE(engine[BLACK][feature]);

        CHECK(engine[WHITE][feature] - engine[BLACK][feature] ==
              model[feature]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A count that is zero in every position of the corpus is a count the two
  // implementations agree about for free, and the two colours read "ahead" and
  // "behind" in opposite directions, so an error that only hits one of them
  // needs both rows claimed to be caught.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the positions exercise every pawn structure count")
  {
    int seen[2][eval_model::PAWN_STRUCTURE_COUNT] = {};
    int white_ahead[eval_model::PAWN_STRUCTURE_COUNT] = {};
    int black_ahead[eval_model::PAWN_STRUCTURE_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      for (size_t feature = 0; feature < eval_model::PAWN_STRUCTURE_COUNT;
           ++feature) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][feature] != 0) { seen[colour][feature]++; }
        }

        const int difference = engine[WHITE][feature] - engine[BLACK][feature];

        if (difference > 0) { white_ahead[feature]++; }
        if (difference < 0) { black_ahead[feature]++; }
      }
    }

    for (size_t feature = 0; feature < eval_model::PAWN_STRUCTURE_COUNT;
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
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the model and the engine count pawn structure by hand")
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

    int positive[eval_model::PAWN_STRUCTURE_COUNT] = {};
    int negative[eval_model::PAWN_STRUCTURE_COUNT] = {};
    int white_row[eval_model::PAWN_STRUCTURE_COUNT] = {};
    int black_row[eval_model::PAWN_STRUCTURE_COUNT] = {};

    for (const pawn_structure_case_t& item : cases) {
      // As strings, because CAPTURE of a `const char*` logs the pointer and a
      // failure then names the case by its address.
      const std::string placement = item.placement;
      const std::string what = item.what;

      CAPTURE(placement);
      CAPTURE(what);

      int model[eval_model::PAWN_STRUCTURE_COUNT] = {};
      REQUIRE(eval_model::pawn_structure_features(item.placement, model));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][3];
      pawn_structure_counts(&game.board, engine);

      for (size_t feature = 0; feature < eval_model::PAWN_STRUCTURE_COUNT;
           ++feature) {
        const std::string name = pawn_structure_feature_names[feature];
        const int difference = item.white[feature] - item.black[feature];

        CAPTURE(name);

        CHECK(model[feature] == difference);
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
    for (size_t feature = 0; feature < eval_model::PAWN_STRUCTURE_COUNT;
         ++feature) {
      const std::string name = pawn_structure_feature_names[feature];

      CHECK_MESSAGE(positive[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(negative[feature] > 0, (name + ": never Black ahead"));
      CHECK_MESSAGE(white_row[feature] > 0, (name + ": no White row claimed"));
      CHECK_MESSAGE(black_row[feature] > 0, (name + ": no Black row claimed"));
    }
  }

  // The piece placement counts, engine against model, over the same corpus, and
  // the case with teeth for this term for the reason the three pairs above have
  // teeth: the weights ship at zero, so the evaluate() comparison compares 0
  // against 0 here and would pass however far apart the two counts are.
  //
  // eval_model::piece_placement_features() reports White minus Black and
  // nothing else, which is the form the tuner is linear in, so that is what can
  // be compared here. The rows themselves are pinned against hand-computed
  // answers in the case below, which is what a difference cannot see: a
  // miscount that hits both sides equally cancels.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the engine counts the piece placement the model counts")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      int engine[2][eval_model::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      int model[eval_model::PIECE_PLACEMENT_COUNT] = {};
      REQUIRE(eval_model::piece_placement_features(fen.substr(0, fen.find(' ')),
                                                   model));

      for (size_t feature = 0; feature < eval_model::PIECE_PLACEMENT_COUNT;
           ++feature) {
        const std::string name = piece_placement_feature_names[feature];

        CAPTURE(name);
        CAPTURE(engine[WHITE][feature]);
        CAPTURE(engine[BLACK][feature]);

        CHECK(engine[WHITE][feature] - engine[BLACK][feature] ==
              model[feature]);
      }
    }
  }

  // Non-vacuous by construction, and the reason the case above means anything.
  // A count that is zero in every position of the corpus is a count the two
  // implementations agree about for free, and the half-open file is a different
  // file set for each colour, so an error that only hits one of them needs both
  // rows claimed to be caught.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the positions exercise every piece placement count")
  {
    int seen[2][eval_model::PIECE_PLACEMENT_COUNT] = {};
    int white_ahead[eval_model::PIECE_PLACEMENT_COUNT] = {};
    int black_ahead[eval_model::PIECE_PLACEMENT_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][eval_model::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      for (size_t feature = 0; feature < eval_model::PIECE_PLACEMENT_COUNT;
           ++feature) {
        for (int colour = 0; colour < 2; ++colour) {
          if (engine[colour][feature] != 0) { seen[colour][feature]++; }
        }

        const int difference = engine[WHITE][feature] - engine[BLACK][feature];

        if (difference > 0) { white_ahead[feature]++; }
        if (difference < 0) { black_ahead[feature]++; }
      }
    }

    for (size_t feature = 0; feature < eval_model::PIECE_PLACEMENT_COUNT;
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
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the model and the engine count piece placement by hand")
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
        // Both colours at once, so a model that lost the sign reports the wrong
        // difference rather than nothing at all.
        {"4k3/8/8/8/8/8/r7/2B1KB2",
         "White has the pair, Black a rook on an open file",
         {1, 0, 0},
         {0, 1, 0}},
    };

    int positive[eval_model::PIECE_PLACEMENT_COUNT] = {};
    int negative[eval_model::PIECE_PLACEMENT_COUNT] = {};
    int white_row[eval_model::PIECE_PLACEMENT_COUNT] = {};
    int black_row[eval_model::PIECE_PLACEMENT_COUNT] = {};

    for (const piece_placement_case_t& item : cases) {
      // As strings, because CAPTURE of a `const char*` logs the pointer and a
      // failure then names the case by its address.
      const std::string placement = item.placement;
      const std::string what = item.what;

      CAPTURE(placement);
      CAPTURE(what);

      int model[eval_model::PIECE_PLACEMENT_COUNT] = {};
      REQUIRE(eval_model::piece_placement_features(item.placement, model));

      // The cases carry a placement and load_FEN wants a whole FEN, so the
      // tail is the neutral one: White to move, nothing castling, no en
      // passant. None of the four is read by the term.
      REQUIRE(load_FEN(placement + " w - - 0 1", &game));

      int engine[2][eval_model::PIECE_PLACEMENT_COUNT];
      piece_placement_counts(&game.board, engine);

      for (size_t feature = 0; feature < eval_model::PIECE_PLACEMENT_COUNT;
           ++feature) {
        const std::string name = piece_placement_feature_names[feature];
        const int difference = item.white[feature] - item.black[feature];

        CAPTURE(name);

        CHECK(model[feature] == difference);
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
    for (size_t feature = 0; feature < eval_model::PIECE_PLACEMENT_COUNT;
         ++feature) {
      const std::string name = piece_placement_feature_names[feature];

      CHECK_MESSAGE(positive[feature] > 0, (name + ": never White ahead"));
      CHECK_MESSAGE(negative[feature] > 0, (name + ": never Black ahead"));
      CHECK_MESSAGE(white_row[feature] > 0, (name + ": no White row claimed"));
      CHECK_MESSAGE(black_row[feature] > 0, (name + ": no Black row claimed"));
    }
  }

  // Non-vacuous by construction, and the reason the tempo term is inside the
  // comparison at the top of this file at all. Every other feature is checked
  // by a case of its own against the engine's counts; this one has no counts,
  // so the only thing that can make the comparison sensitive to it is a corpus
  // that contains both answers. A corpus that were all White to move would fit
  // and check a bonus for White and a bonus for the mover identically.
  TEST_CASE_FIXTURE(model_fixture_t, "the positions have both sides to move")
  {
    int white_to_move = 0;
    int black_to_move = 0;

    for (const std::string& fen : positions) {
      CAPTURE(fen);

      int tempo = 0;
      REQUIRE(eval_model::tempo_feature(side_to_move_of(fen), &tempo));

      // The model's feature and the engine's board have to agree about who is
      // on move before anything downstream of either means anything.
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
  TEST_CASE_FIXTURE(model_fixture_t,
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

  // The phase the model rebuilds from the piece list is the quantity the
  // engine tapers on. If these disagreed, the model could still match on a
  // full board and diverge everywhere else, which the check above would only
  // catch by luck.
  TEST_CASE_FIXTURE(model_fixture_t, "the model's phase is game_phase()")
  {
    for (const std::string& fen : positions) {
      CAPTURE(fen);
      REQUIRE(load_FEN(fen, &game));

      std::vector<uint16_t> pieces;
      REQUIRE(
          eval_model::parse_placement(fen.substr(0, fen.find(' ')), &pieces));

      CHECK(eval_model::phase_of(pieces.data(), pieces.size()) ==
            game_phase(&game.board));
    }
  }

  // Non-vacuous by construction: the positions above have to contain the
  // asymmetry the checks are supposed to be sensitive to. A suite of symmetric
  // positions would pass with a model that ignored colour entirely.
  TEST_CASE_FIXTURE(model_fixture_t, "the positions can tell colours apart")
  {
    std::vector<double> params(eval_model::PARAM_COUNT, 0.0);
    eval_model::starting_params(params.data());

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


// ------------------------------------------------- the mirror's dataset
//
// tools/tuner_model.hpp's dataset_t and load(): the mirror's features packed
// per row, which the tuner read until S262 and tools/feature_audit still reads
// -- its identities, its columns and its `--plant` labels are built from these
// rows. These five cases were tests/test_tuner_gradient.cpp's (S100, S134,
// S275) and moved here with the tuner: that file now holds the tuner's trace
// path, and the same checks over it, while these keep the mirror's loader
// held for the tool that still depends on it.

namespace mirror_dataset
{

using test_eval_positions::positions;

// One unique name per binary, not per call: the cases below write and then
// read the same path. S193.
std::string fixture_path()
{
  static const std::string path =
      unique_fixture_path("chesso_test_eval_model.tsv");
  return path;
}


// `fen result score phase`, datagen's format, over the curated corpus.
//
// The three labels cycle rather than being constant, and the scores span both
// signs. A constant result column would make (target - sigma) the same factor
// on every row, which is a residual a wrong per-row feature could still fit; a
// zero score column would make --lambda untestable for the same reason S075's
// parser refuses one.
//
// `phase_shift` writes a phase column that is wrong on purpose, the true phase
// plus the shift wrapped into 0..GAME_PHASE_MAX, so it still passes load()'s
// range check -- the corpus a change to phase_value would leave behind.
bool write_fixture(const std::string& path, int phase_shift = 0)
{
  std::ofstream out(path);

  if (!out) { return false; }

  static const double results[3] = {1.0, 0.5, 0.0};
  int score = -280;

  for (size_t i = 0; i < positions.size(); ++i) {
    const std::string& fen = positions[i];
    const std::string placement = fen.substr(0, fen.find(' '));

    std::vector<uint16_t> pieces;
    if (!eval_model::parse_placement(placement, &pieces)) { return false; }

    const int phase =
        (eval_model::phase_of(pieces.data(), pieces.size()) + phase_shift) %
        (GAME_PHASE_MAX + 1);

    out << fen << '\t' << results[i % 3] << '\t' << score << '\t' << phase
        << '\n';

    score += 41;
    if (score > 300) { score -= 600; }
  }

  return out.good();
}


}  // namespace mirror_dataset

using eval_model::KING_SAFETY_COUNT;
using eval_model::MOBILITY_COUNT;
using eval_model::PASSED_PAWN_COUNT;
using eval_model::PAWN_STRUCTURE_COUNT;
using eval_model::PIECE_PLACEMENT_COUNT;
using mirror_dataset::fixture_path;
using mirror_dataset::write_fixture;


TEST_CASE("the mirror dataset: the fixture corpus loads through its parser")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  CHECK(data.size() == positions.size());

  std::filesystem::remove(path);
}


// 2026-09-10_adversarial-F36: load() trusted the corpus's phase column, range
// checked only, and phase_of() sits in the same header. Verified equal on every
// corpus so far, and latent: a change to phase_value leaves every existing
// corpus tapering on the old phase while the range check still passes. S134
// makes load() recompute the phase from the placement, so the column is read
// for its range and nothing else.
TEST_CASE(
    "the mirror dataset: load() tapers on the placement's phase, not the "
    "stored column")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path, 7));

  // Precondition, read back from the file rather than recomputed: every phase
  // column the fixture wrote disagrees with its placement's phase, or the check
  // below holds for a load() that trusts the column too.
  {
    std::ifstream in(path);
    std::string line;
    size_t rows = 0;
    size_t column_wrong = 0;

    while (std::getline(in, line)) {
      std::istringstream fields(line);
      std::string fen, result, score, phase_text;
      REQUIRE(std::getline(fields, fen, '\t'));
      REQUIRE(std::getline(fields, result, '\t'));
      REQUIRE(std::getline(fields, score, '\t'));
      REQUIRE(std::getline(fields, phase_text, '\t'));

      std::vector<uint16_t> pieces;
      REQUIRE(
          eval_model::parse_placement(fen.substr(0, fen.find(' ')), &pieces));

      if (std::stoi(phase_text) !=
          eval_model::phase_of(pieces.data(), pieces.size())) {
        column_wrong++;
      }
      rows++;
    }

    REQUIRE(rows == positions.size());
    REQUIRE(column_wrong == rows);
  }

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  REQUIRE(data.size() == positions.size());

  for (size_t row = 0; row < data.size(); ++row) {
    const std::string& fen = positions[row];
    CAPTURE(fen);

    std::vector<uint16_t> pieces;
    REQUIRE(eval_model::parse_placement(fen.substr(0, fen.find(' ')), &pieces));

    CHECK(static_cast<int>(data.phase[row]) ==
          eval_model::phase_of(pieces.data(), pieces.size()));
  }

  std::filesystem::remove(path);
}


// S275. The two king gauges tools/tuner_groups.hpp holds are null only on a row
// with one king a side: + c on every king square cancels between the White king
// and the Black one and nowhere else. load_FEN() refuses any other position,
// and load() refused none of them, so a hold on a corpus carrying one would
// move that row's score -- a pin that changes the fit. Each row below is
// appended alone to the legal fixture, and the fixture is refused.
TEST_CASE("the mirror dataset: load() refuses a row without one king a side")
{
  const std::string path = fixture_path();

  const auto load_with = [&](const std::string& placement) {
    REQUIRE(write_fixture(path));
    {
      std::ofstream out(path, std::ios::app);
      out << placement << " w - - 0 40\t0.5\t0\t0\n";
    }

    tuner_model::dataset_t data;
    return tuner_model::load(path, &data);
  };

  // Precondition: a legal row in the same format loads, so a refusal below is
  // the king count and not the row's shape.
  CHECK(load_with("4k3/8/8/8/8/8/8/4K3"));

  static const char* const unbalanced[] = {
      "4k3/8/8/8/8/8/8/8",      // no White king
      "8/8/8/8/8/8/8/4K3",      // no Black king
      "4k3/8/8/8/8/8/8/3KK3",   // two White kings
      "3kk3/8/8/8/8/8/8/4K3",   // two Black kings
      "3kk3/8/8/8/8/8/8/3KK3",  // two a side: balanced, still not chess
      "8/8/8/8/8/8/8/8",        // none at all
  };

  for (const std::string placement : unbalanced) {
    CAPTURE(placement);
    CHECK_FALSE(load_with(placement));
  }

  std::filesystem::remove(path);
}


// The dataset path, end to end: every column load() stored, against the same
// feature recomputed from the FEN text outside it. A misaligned base, a wrong
// stride, or two columns written in the other order all show up here and in
// nothing else -- test_eval_model.cpp checks the extractors against the engine
// and never sees what the loader did with their answers.
TEST_CASE(
    "the mirror dataset: every stored column holds the feature its index "
    "names")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));
  REQUIRE(data.size() == positions.size());

  for (size_t row = 0; row < data.size(); ++row) {
    const std::string& fen = positions[row];
    CAPTURE(fen);

    const std::string placement = fen.substr(0, fen.find(' '));

    std::vector<uint16_t> pieces;
    REQUIRE(eval_model::parse_placement(placement, &pieces));

    // The piece list and its offsets, which is what every psqt and material
    // parameter is read through.
    REQUIRE(data.offsets[row + 1] - data.offsets[row] == pieces.size());
    for (size_t i = 0; i < pieces.size(); ++i) {
      CHECK(data.pieces[data.offsets[row] + i] == pieces[i]);
    }

    CHECK(static_cast<int>(data.phase[row]) ==
          eval_model::phase_of(pieces.data(), pieces.size()));

    int mobility[MOBILITY_COUNT] = {};
    REQUIRE(eval_model::mobility_features(placement, mobility));
    for (size_t i = 0; i < MOBILITY_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.mobility[row * MOBILITY_COUNT + i] == mobility[i]);
    }

    int safety[KING_SAFETY_COUNT] = {};
    REQUIRE(eval_model::king_safety_features(placement, safety));
    for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.king_safety[row * KING_SAFETY_COUNT + i] == safety[i]);
    }

    int passed[PASSED_PAWN_COUNT] = {};
    REQUIRE(eval_model::passed_pawn_features(placement, passed));
    for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.passed_pawn[row * PASSED_PAWN_COUNT + i] == passed[i]);
    }

    int structure[PAWN_STRUCTURE_COUNT] = {};
    REQUIRE(eval_model::pawn_structure_features(placement, structure));
    for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.pawn_structure[row * PAWN_STRUCTURE_COUNT + i] ==
            structure[i]);
    }

    int placement_counts[PIECE_PLACEMENT_COUNT] = {};
    REQUIRE(eval_model::piece_placement_features(placement, placement_counts));
    for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
      CAPTURE(i);
      CHECK(data.piece_placement[row * PIECE_PLACEMENT_COUNT + i] ==
            placement_counts[i]);
    }

    int tempo = 0;
    REQUIRE(eval_model::tempo_feature(side_to_move_of(fen), &tempo));
    CHECK(static_cast<int>(data.tempo[row]) == tempo);
  }

  std::filesystem::remove(path);
}


// Non-vacuity, and it is load bearing twice over: a finite difference on a
// parameter whose feature is zero in every row compares 0.0 with 0.0 and passes
// for free, and so does a column comparison between two zeros. Every one of the
// 54 parameters outside the piece-square tables is claimed here, in both
// directions where the feature has one.
TEST_CASE("the mirror dataset: the fixture moves every term column both ways")
{
  const std::string path = fixture_path();
  REQUIRE(write_fixture(path));

  tuner_model::dataset_t data;
  REQUIRE(tuner_model::load(path, &data));

  struct column_t
  {
    const char* name;
    const int16_t* base;
    size_t stride;
    size_t index;
  };

  std::vector<column_t> columns;
  for (size_t i = 0; i < MOBILITY_COUNT; ++i) {
    columns.push_back({"mobility", data.mobility.data(), MOBILITY_COUNT, i});
  }
  for (size_t i = 0; i < KING_SAFETY_COUNT; ++i) {
    columns.push_back(
        {"king safety", data.king_safety.data(), KING_SAFETY_COUNT, i});
  }
  for (size_t i = 0; i < PASSED_PAWN_COUNT; ++i) {
    columns.push_back(
        {"passed pawn", data.passed_pawn.data(), PASSED_PAWN_COUNT, i});
  }
  for (size_t i = 0; i < PAWN_STRUCTURE_COUNT; ++i) {
    columns.push_back({"pawn structure", data.pawn_structure.data(),
                       PAWN_STRUCTURE_COUNT, i});
  }
  for (size_t i = 0; i < PIECE_PLACEMENT_COUNT; ++i) {
    columns.push_back({"piece placement", data.piece_placement.data(),
                       PIECE_PLACEMENT_COUNT, i});
  }

  for (const column_t& column : columns) {
    CAPTURE(column.name);
    CAPTURE(column.index);

    size_t positive = 0;
    size_t negative = 0;

    for (size_t row = 0; row < data.size(); ++row) {
      const int value = column.base[row * column.stride + column.index];

      if (value > 0) { positive++; }
      if (value < 0) { negative++; }
    }

    CHECK(positive > 0);
    CHECK(negative > 0);
  }

  // Tempo is stored in its own vector and is +1 or -1 on every row, so it has
  // no zero case to guard -- what it needs is both signs present, which is the
  // corpus having positions with each side to move.
  size_t white_to_move = 0;
  size_t black_to_move = 0;
  for (size_t row = 0; row < data.size(); ++row) {
    if (data.tempo[row] > 0) { white_to_move++; }
    if (data.tempo[row] < 0) { black_to_move++; }
  }
  CHECK(white_to_move > 0);
  CHECK(black_to_move > 0);

  std::filesystem::remove(path);
}
