#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cmath>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_model.hpp"
#include "evaluation.hpp"

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

  return eval_model::evaluate(pieces.data(), pieces.size(), phase, mobility,
                              params, king_safety);
}


// king_safety_feature_t's enumerators, in its order, so a failure names the
// count that disagreed instead of an index the reader has to go and look up.
static const char* const king_safety_feature_names[KS_FEATURE_COUNT] = {
    "knight attackers", "bishop attackers", "rook attackers",
    "queen attackers",  "zone attacks",     "shield near",
    "shield far",       "open file",        "half open file"};


static const std::vector<std::string> positions = {
    // Opening, both sides to move.
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    // Middlegame, material imbalances and castled kings.
    "r1bq1rk1/pp2ppbp/2np1np1/8/2BNP3/2N1B3/PPP2PPP/R2Q1RK1 w - - 0 9",
    "r2q1rk1/pb1nbppp/1p2pn2/2pp4/2PP4/1PN1PN2/PB2BPPP/R2Q1RK1 b - - 2 10",
    "r4rk1/1bq2ppp/p2bpn2/1p6/3P4/P1NBPN2/1P3PPP/R2Q1RK1 w - - 0 15",
    "2r3k1/5ppp/p2q4/1p1Pn3/8/2P2Q2/PP3PPP/3R2K1 b - - 0 24",
    // Endgames down to bare pawns, where the tapering weight is at the other
    // end of its range.
    "8/5pk1/6p1/7p/5P1P/6P1/5K2/8 w - - 0 1",
    "8/8/4k3/8/8/3K4/4P3/8 b - - 0 1",
    "6k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1",
    "8/2k5/8/8/3B4/2K5/8/8 b - - 0 1",
    // Promotion, which is how the phase sum can exceed a full board and where
    // game_phase() clamps.
    "8/PPPPPPPP/8/2k5/2K5/8/pppppppp/8 w - - 0 1",
    // Lopsided material, so a wrong sign cannot cancel itself.
    "3qk3/8/8/8/8/8/8/3QK2R w K - 0 1",
    // King safety, added at S027 because the positions above leave most of the
    // term at zero: no knight and no rook ever bore on a king zone across all
    // thirteen, and seven of the nine counts only ever pointed one way. A count
    // that is zero everywhere is a count the engine and the model agree about
    // for free. "the positions exercise every count" is what says so out loud;
    // each line below is here to move one of the entries it checks.
    //
    // A knight and a rook on the black king, and the same position mirrored so
    // that the White-ahead direction is covered too.
    "r1bq1rk1/ppp2ppp/2n5/3p2N1/1b1P4/2N4R/PPP2PPP/R1BQ2K1 w - - 0 1",
    "r1bq2k1/ppp2ppp/2n4r/1B1p4/3P2n1/2N5/PPP2PPP/R1BQ1RK1 b - - 0 1",
    // A bishop and a queen on the white king, from a black king with three
    // open files and no shelter of its own.
    "1k6/b7/8/8/7q/8/5PPP/6K1 w - - 0 1",
    // Shelter at both distances: White's pawns are two ranks ahead of its king
    // and Black's are one.
    "6k1/5ppp/8/8/8/5PPP/8/6K1 w - - 0 1",
    // Three half-open files in front of the white king, against a black king
    // standing behind its own pawns.
    "1k6/ppp2ppp/8/8/8/8/PP6/6K1 w - - 0 1",
};


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

      // evaluate() interpolates in integers and truncates towards zero. Since
      // S034 it does that twice, once for the tables and once for mobility,
      // each stage rounding on its own, so the arithmetic can differ by two
      // centipawns rather than one. Anything beyond that is a difference in
      // what is being computed, which is what this file is for.
      //
      // King safety is inside this number but says nothing while its weights
      // are zero: the term contributes 0 on both sides however far apart the
      // two implementations' counts are. What checks the counts is the case
      // below; this one starts checking them for free the moment the fit
      // lands. S027.
      CHECK(std::abs(model - engine_white) <= 2.0);
    }
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
