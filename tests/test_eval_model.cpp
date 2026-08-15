#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
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


// The same for the four piece placement counts, in the order the weights are
// indexed.
static const char* const
    piece_placement_feature_names[eval_model::PIECE_PLACEMENT_COUNT] = {
        "bishop pair", "rook on an open file", "rook on a half open file",
        "rook on the seventh"};


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
    // Passed pawns, added at S027 for the reason it added the two above: the
    // thirteen original positions leave the White-minus-Black difference at
    // zero in every bucket but two, and the eight-against-eight promotion race
    // cancels exactly. These two put a passed pawn on each side at different
    // distances from promotion, so the evaluate() comparison has something to
    // disagree about the moment the weights stop being zero.
    "4k3/8/8/3pP3/8/5p2/8/4K3 w - - 0 1",
    "4k3/8/8/P3p3/4P3/8/8/4K3 w - - 0 1",
    // Five buckets at a time, per colour, added when the engine's own counts
    // were exposed and the corpus turned out to reach almost none of them: no
    // White passer existed anywhere on the third, fourth or sixth rank, no
    // Black one on the fourth or sixth, and the only position touching the last
    // bucket is the eight-against-eight promotion race, where the two sides
    // cancel exactly and the difference says nothing. A ladder of unopposed
    // pawns and its mirror claim every bucket but the first from both
    // directions, which the first already had.
    "4k3/4P3/3P4/2P5/1P6/P7/8/4K3 w - - 0 1",
    "4k3/8/p7/1p6/2p5/3p4/4p3/4K3 b - - 0 1",
    // Pawn structure, added at S027 when the engine's own counts were exposed
    // and the twenty-two positions above turned out to reach one of the three
    // features from one direction only: no side ever had a doubled pawn
    // anywhere, and only White was ever backward. A doubled f-pawn, an isolated
    // and backward d-pawn stopped by an enemy pawn on c5, and the same position
    // mirrored so that Black is the side carrying all three.
    "r2q1rk1/pp3ppp/4pn2/2p5/8/2NP1P2/PP3PPP/R2Q1RK1 w - - 0 14",
    "r2q1rk1/pp3ppp/2np1p2/8/2P5/4PN2/PP3PPP/R2Q1RK1 b - - 0 14",
    // Piece placement, added at S027 when the engine's own counts were exposed
    // and the twenty-four positions above turned out never to put a rook on the
    // seventh at all, never to give Black a rook on an open file and never to
    // give White one on a half-open file. Only the bishop pair was reached from
    // both directions. A rook lifted to the seventh beside a second one on a
    // half-open file, and the same position with the colours swapped so that
    // Black is the side holding all three.
    "r5k1/ppppRppp/8/8/8/8/PPP2PPP/3R2K1 w - - 0 1",
    "3r2k1/ppp2ppp/8/8/8/8/PPPPrPPP/R5K1 b - - 0 1",
    // The truncation bound itself, added at S038 and re-measured at S065.
    // Everything above is hand-picked to reach a feature, and hand-picked
    // positions are exactly the ones whose taperings happen to divide evenly:
    // over the real corpus 175415 of 11003693 positions disagree with the model
    // by more than the 2.0 this file used to allow, and none of the twenty-six
    // above does. These four are here so the bound is exercised by the test
    // rather than only by a corpus under `.tuning/` that is gitignored and does
    // not exist on every machine. "the pinned positions reach the truncation
    // bound" below is what says they still do.
    //
    // **A residual belongs to the weights, not to the position**, which is why
    // this list is re-measured whenever the constants are refitted rather than
    // carried forward. S038 pinned the four worst the 2026-08-13 audit found
    // under the constants shipping then; S065's fit moved all four to between
    // 0.25 and 1.25 and left the case asserting a property of weights that were
    // no longer in the tree. Each of these four is at the arithmetic maximum,
    // 69/24 = 2.875 exactly, measured over all 11003693 rows of
    // `.tuning/selfplay_v2.tsv`, and they span the taper from phase 5 to phase
    // 23 with both sides to move so that a model error confined to one end of
    // it
    // is still reachable. DEC-057 is the decision to re-measure; what it does
    // not permit is lowering the thresholds below to whatever came out.
    "8/5R2/2nk2K1/8/1r3P2/8/8/8 b - - 3 54",
    "r3r1k1/p5p1/1p1Pb2p/5P2/8/7P/P2N1B2/bN3RK1 w - - 0 24",
    "5rk1/5ppp/p1b4q/8/2QP2P1/5N1n/PP3P2/4RR1K w - - 2 26",
    "r1bq1rk1/1ppp1p1p/n4np1/pNP3N1/4p2P/4P3/PBPP1PP1/R2QKB1R b KQ - 5 9",
};


// The subset of `positions` pinned for the truncation bound rather than for a
// feature count, in the order they are listed above. Named separately because
// "the pinned positions reach the truncation bound" has to be able to fail when
// *these* stop disagreeing with the model, which is a different statement from
// the whole corpus agreeing within tolerance. S038, re-measured at S065.
static const std::vector<std::string> truncation_positions = {
    "8/5R2/2nk2K1/8/1r3P2/8/8/8 b - - 3 54",
    "r3r1k1/p5p1/1p1Pb2p/5P2/8/7P/P2N1B2/bN3RK1 w - - 0 24",
    "5rk1/5ppp/p1b4q/8/2QP2P1/5N1n/PP3P2/4RR1K w - - 2 26",
    "r1bq1rk1/1ppp1p1p/n4np1/pNP3N1/4p2P/4P3/PBPP1PP1/R2QKB1R b KQ - 5 9",
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


// The engine indexes its buckets with a literal 6 and the model with a named
// constant. Nothing forces the two to be the same number, and if they ever stop
// being one the loops below would read off the end of a row rather than fail.
static_assert(eval_model::PASSED_PAWN_COUNT == 6,
              "the engine's passed_pawn_counts() rows are six wide");


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


// The engine indexes its counts with a literal 4, the model with a constant
// taken from the width of the engine's own weight array. Nothing forces those
// to be the same number, and if they ever stop being one the loops below would
// read off the end of a row rather than fail.
static_assert(eval_model::PIECE_PLACEMENT_COUNT == 4,
              "the engine's piece_placement_counts() rows are four wide");


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

      int engine[2][6];
      passed_pawn_counts(&game.board, engine);

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
    int seen[2][6] = {};
    int white_ahead[6] = {};
    int black_ahead[6] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][6];
      passed_pawn_counts(&game.board, engine);

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
         {1, 0, 0, 0, 0, 0}},
        {"4k3/P7/8/8/8/8/8/4K3",
         "white pawn one square from promotion",
         {0, 0, 0, 0, 0, 1}},
        {"4k3/8/3P4/2P5/1P6/P7/8/4K3",
         "white pawns filling the four buckets "
         "between the ends",
         {0, 1, 1, 1, 1, 0}},
        {"4k3/p7/8/8/8/8/8/4K3",
         "black pawn on its own second rank",
         {-1, 0, 0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/p7/4K3",
         "black pawn one square from promotion",
         {0, 0, 0, 0, 0, -1}},
        {"4k3/8/p7/1p6/2p5/3p4/8/4K3",
         "black pawns filling the four buckets "
         "between the ends",
         {0, -1, -1, -1, -1, 0}},
        // Both colours at once: the two bucket-0 pawns cancel in the difference
        // and the c5 pawn is what is left, so a model that lost the sign would
        // report -1 here rather than nothing at all.
        {"4k3/7p/8/2P5/8/8/P7/4K3",
         "one passed pawn each in the same bucket",
         {0, 0, 0, 1, 0, 0}},
        {"4k3/8/8/P3p3/4P3/8/8/4K3",
         "e4 and e5 block each other head on",
         {0, 0, 0, 1, 0, 0}},
        {"4k3/8/5p2/8/4P3/8/p7/4K3",
         "f6 stops e4 from a neighbouring file",
         {0, 0, 0, 0, 0, -1}},
        // The two directions that must not block, in one placement: d5 is level
        // with e5 and f3 is behind it. Reverse "ahead" and every one of these
        // three answers changes.
        {"4k3/8/8/3pP3/8/5p2/8/4K3",
         "level and behind do not block",
         {0, 0, -1, 1, -1, 0}},
        {"4k3/8/8/3pP3/4P3/8/8/4K3",
         "doubled pawns, only the front one passed",
         {0, 0, 0, 1, 0, 0}},
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

      int engine[2][6];
      passed_pawn_counts(&game.board, engine);

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

      int engine[2][4];
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
  // implementations agree about for free, and the seventh rank is a different
  // rank for each colour, so an error that only hits one of them needs both
  // rows claimed to be caught.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the positions exercise every piece placement count")
  {
    int seen[2][eval_model::PIECE_PLACEMENT_COUNT] = {};
    int white_ahead[eval_model::PIECE_PLACEMENT_COUNT] = {};
    int black_ahead[eval_model::PIECE_PLACEMENT_COUNT] = {};

    for (const std::string& fen : positions) {
      REQUIRE(load_FEN(fen, &game));

      int engine[2][4];
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
  // piece does not close a file, and that the seventh rank is the rank alone.
  TEST_CASE_FIXTURE(model_fixture_t,
                    "the model and the engine count piece placement by hand")
  {
    static const piece_placement_case_t cases[] = {
        {"4k3/8/8/8/8/8/8/2B1KB2",
         "two bishops are a pair",
         {1, 0, 0, 0},
         {0, 0, 0, 0}},
        {"2b1kb2/8/8/8/8/8/8/4K3",
         "the same two for Black",
         {0, 0, 0, 0},
         {1, 0, 0, 0}},
        {"4k3/8/8/8/8/8/8/2B1KB1B",
         "three bishops are still one pair, not three",
         {1, 0, 0, 0},
         {0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/8/2B1K1B1",
         "two bishops on the same colour square are still a pair",
         {1, 0, 0, 0},
         {0, 0, 0, 0}},
        {"2b1kb2/8/8/8/8/8/8/2B1K3",
         "one bishop is not a pair and two are",
         {0, 0, 0, 0},
         {1, 0, 0, 0}},
        {"4k3/8/8/8/8/8/8/R3K3",
         "a file with no pawn of either colour on it is open",
         {0, 1, 0, 0},
         {0, 0, 0, 0}},
        {"r3k3/8/8/8/8/8/8/4K3",
         "the same for Black",
         {0, 0, 0, 0},
         {0, 1, 0, 0}},
        {"4k3/8/8/8/8/8/P7/R3K3",
         "an own pawn makes the file neither open nor half open",
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
        {"4k3/p7/8/8/8/8/P7/R3K3",
         "an own pawn still closes it with an enemy pawn there too",
         {0, 0, 0, 0},
         {0, 0, 0, 0}},
        {"4k3/p7/8/8/8/8/8/R3K3",
         "an enemy pawn alone makes the file half open",
         {0, 0, 1, 0},
         {0, 0, 0, 0}},
        {"r3k3/8/8/8/8/8/P7/4K3",
         "the same for Black",
         {0, 0, 0, 0},
         {0, 0, 1, 0}},
        {"4k3/8/8/8/n7/8/8/R3K3",
         "a piece is not a pawn and does not close a file",
         {0, 1, 0, 0},
         {0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/8/R3K2R",
         "two rooks on open files count two, not one",
         {0, 2, 0, 0},
         {0, 0, 0, 0}},
        {"8/R7/8/8/4k3/8/8/4K3",
         "the seventh is the rank alone, with the enemy king off "
         "its back rank",
         {0, 1, 0, 1},
         {0, 0, 0, 0}},
        {"4k3/8/8/8/8/8/r7/4K3",
         "Black's seventh is rank 2",
         {0, 0, 0, 0},
         {0, 1, 0, 1}},
        {"4k3/8/8/8/8/8/R7/4K3",
         "a White rook on rank 2 is on its own second, not the "
         "seventh",
         {0, 1, 0, 0},
         {0, 0, 0, 0}},
        {"4k3/1R6/8/1p6/8/8/8/4K3",
         "a rook on the seventh on a half open file counts in both",
         {0, 0, 1, 1},
         {0, 0, 0, 0}},
        // Both colours at once, so a model that lost the sign reports the wrong
        // difference rather than nothing at all.
        {"4k3/8/8/8/8/8/r7/2B1KB2",
         "White has the pair, Black the rook on the seventh",
         {1, 0, 0, 0},
         {0, 1, 0, 1}},
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

      int engine[2][4];
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
