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

  return eval_model::evaluate(pieces.data(), pieces.size(), phase, params);
}


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

      // evaluate() interpolates in integers and truncates towards zero, so one
      // centipawn of disagreement is the arithmetic and anything more is a
      // difference in what is being computed.
      CHECK(std::abs(model - engine_white) <= 1.0);
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
      REQUIRE(eval_model::parse_placement(fen.substr(0, fen.find(' ')),
                                          &pieces));

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
