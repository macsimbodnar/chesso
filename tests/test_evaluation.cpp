#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cstdlib>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "test_helpers.hpp"


static game_t game;
static game_t mirrored;


struct eval_fixture_t
{
  eval_fixture_t()
  {
    initialize_game_const_data(&game);
    initialize_game_const_data(&mirrored);
  }
};


// NOTE ON STYLE
//
// These are written as invariants - symmetry and ordering - rather than as
// expected score values, so that they keep their meaning when evaluate() grows
// piece-square tables or is replaced by a network. The only two assertions
// pinned to today's material-only numbers are marked ANCHOR and have to be
// revisited whenever the evaluation changes.


TEST_SUITE("evaluation: score")
{
  // The strongest property an evaluation has: mirroring the board and swapping
  // both colours must negate the score exactly. A one-sided term, a wrong
  // sign, or a table indexed from the wrong side all show up here.
  TEST_CASE_FIXTURE(eval_fixture_t, "colour symmetry over every test position")
  {
    size_t checked = 0;

    for (const std::string& fen : all_test_fens()) {
      const std::string flipped = mirror_fen(fen);

      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));
      REQUIRE_MESSAGE(load_FEN(flipped, &mirrored),
                      ("mirrored FEN: " + flipped + " from " + fen));

      const int score = evaluate(&game.board);
      const int mirrored_score = evaluate(&mirrored.board);

      // evaluate() answers from the side to move's point of view, and
      // mirror_fen() swaps the side to move along with the colours. The two
      // therefore agree rather than negate: the same player, looking at the
      // same position, has to reach the same number.
      REQUIRE_MESSAGE(score == mirrored_score,
                      ("FEN: " + fen + "\nmirror: " + flipped));
      checked++;
    }

    REQUIRE(checked > 100);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "the start position is balanced")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE_EQ(evaluate(&game.board), 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a mirrored start position is balanced")
  {
    // Same material, black to move. Still symmetric, so still zero.
    REQUIRE(load_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
                     &game));
    REQUIRE_EQ(evaluate(&game.board), 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t,
                    "score is from the side to move's point of view")
  {
    // The same position, read by each side in turn. White is a whole queen up,
    // so it is winning for White and losing for Black, and the number has to
    // change sign with the side to move rather than stay put.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1", &game));
    const int white_to_move = evaluate(&game.board);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 b - - 0 1", &game));
    const int black_to_move = evaluate(&game.board);

    REQUIRE(white_to_move > 0);
    REQUIRE(black_to_move < 0);
    REQUIRE_EQ(white_to_move, -black_to_move);

    // And the mirror image, so a sign error that happens to be symmetric does
    // not slip through.
    REQUIRE(load_FEN("3qk3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE(evaluate(&game.board) < 0);

    REQUIRE(load_FEN("3qk3/8/8/8/8/8/8/4K3 b - - 0 1", &game));
    REQUIRE(evaluate(&game.board) > 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "removing a piece moves the score")
  {
    struct case_t
    {
      std::string with;
      std::string without;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", "white queen"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", "white rook"},
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1","4k3/8/8/8/8/8/8/4K3 w - - 0 1", "white pawn"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE(load_FEN(test.with, &game));
      const int with = evaluate(&game.board);

      REQUIRE(load_FEN(test.without, &game));
      const int without = evaluate(&game.board);

      REQUIRE_MESSAGE(with - without >= 100, test.title);
    }
  }

  // ANCHOR: pinned to today's numbers. Every value here is material plus the
  // piece-square term for the one square the piece is standing on, so editing
  // either table moves them and this test is what makes that deliberate rather
  // than accidental.
  //
  // It did its job at S028: the tuned tables moved all five and the suite said
  // so. It did it again at S034, when mobility became part of the score. The
  // values below were recomputed by a second implementation of evaluate()
  // written for the purpose -- one that walks the rays by hand rather than
  // through the magic tables -- and not read off the engine, because an anchor
  // copied from the thing it anchors asserts nothing.
  //
  // Symmetry and ordering say nothing about what a piece is actually worth:
  // every one of these values can be changed without moving any other
  // assertion in this file, and a wrong one costs games rather than crashes.
  TEST_CASE_FIXTURE(eval_fixture_t, "each piece is worth what the tables say")
  {
    struct case_t
    {
      std::string fen;
      int score;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 117, "pawn on e2"},
      {"4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", 240, "knight on b1"},
      {"4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", 325, "bishop on c1"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 540, "rook on d1"},
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", 1085, "queen on d1"},
      {"4k3/8/8/8/8/8/8/4K3 w - - 0 1",     0, "bare kings cancel"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(evaluate(&game.board) == test.score, test.title);
    }
  }

  // The lazy shortcut is sound only while the expensive terms cannot move the
  // score by more than LAZY_EVAL_MARGIN. That is a claim about every position,
  // not about the ten someone thought of, so it is asserted over the whole
  // corpus. S034.
  //
  // The margin is a bound on the sum of those terms and not on each of them,
  // which is why evaluate_expensive() clamps once over the total. Two terms
  // each bounded by the margin can correct by twice it between them, and the
  // shortcut is unsound the moment that happens.
  //
  // The distribution the margin was chosen from is mobility's alone: 149084
  // self-play positions, p99 81 and a maximum of 143, measured before king
  // safety existed. King safety shares the budget now and ships at zero weight,
  // so `worst` below is still mobility's number and this case cannot witness
  // the second term at all -- test_eval_model checks its counts instead, which
  // is the only thing a zero-weighted term leaves checkable. Once the weights
  // are fitted the margin is re-decided from measured data, and if it is left
  // too small this is where that shows up rather than in lost games. S027.
  TEST_CASE_FIXTURE(eval_fixture_t,
                    "the lazy shortcut cannot change a decision")
  {
    size_t checked = 0;
    int worst = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const int full = evaluate(&game.board);
      const int cheap = evaluate_cheap(&game.board);
      const int correction = std::abs(full - cheap);

      if (correction > worst) { worst = correction; }

      REQUIRE_MESSAGE(
          correction <= LAZY_EVAL_MARGIN,
          ("FEN: " + fen + " correction " + std::to_string(correction)));

      // A window that contains the score: no shortcut, the exact number.
      REQUIRE_MESSAGE(
          evaluate_lazy(&game.board, full - 1000, full + 1000) == full,
          ("FEN: " + fen));

      // A beta the cheap score already clears by the margin: the shortcut
      // fires, returns the cheap score, and the exact score is above beta too
      // -- which is the whole soundness claim, that the caller's decision is
      // the same either way.
      const int beta = cheap - LAZY_EVAL_MARGIN;
      REQUIRE_MESSAGE(evaluate_lazy(&game.board, beta - 1000, beta) == cheap,
                      ("FEN: " + fen));
      REQUIRE_MESSAGE(full >= beta, ("FEN: " + fen));

      // The same mirrored at alpha.
      const int alpha = cheap + LAZY_EVAL_MARGIN;
      REQUIRE_MESSAGE(evaluate_lazy(&game.board, alpha, alpha + 1000) == cheap,
                      ("FEN: " + fen));
      REQUIRE_MESSAGE(full <= alpha, ("FEN: " + fen));

      checked++;
    }

    REQUIRE(checked > 100);

    // Non-vacuous by construction: if the expensive terms never moved the score
    // at all, every assertion above would hold for a reason that has nothing to
    // do with the margin being right.
    REQUIRE(worst > 0);
  }

  // The piece-square tables have to actually prefer the squares they are meant
  // to. These are the two clearest cases and they pull in opposite directions,
  // so a table pasted in upside down fails one of them.
  TEST_CASE_FIXTURE(eval_fixture_t, "the tables prefer the right squares")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/N3K3 w - - 0 1", &game));
    const int knight_corner = evaluate(&game.board);

    REQUIRE(load_FEN("4k3/8/8/3N4/8/8/8/4K3 w - - 0 1", &game));
    const int knight_centre = evaluate(&game.board);

    REQUIRE_MESSAGE(knight_centre > knight_corner,
                    "a centralised knight must beat one in the corner");

    // With a full board the king belongs at home; with nothing left it belongs
    // in the middle. Same two squares, opposite verdicts. The middlegame case
    // needs an actual full board: with one queen each the phase is already 8 of
    // 24, which is mostly endgame and the tables correctly say so.
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const int home_mg = evaluate(&game.board);

    REQUIRE(load_FEN("rnbqkbnr/pppppppp/8/8/3K4/8/PPPPPPPP/RNBQ1BNR w kq - 0 1",
                     &game));
    const int centre_mg = evaluate(&game.board);

    REQUIRE_MESSAGE(home_mg > centre_mg,
                    "with a full board, the king is safer at home");

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    const int home_eg = evaluate(&game.board);

    REQUIRE(load_FEN("4k3/8/8/8/3K4/8/8/8 w - - 0 1", &game));
    const int centre_eg = evaluate(&game.board);

    REQUIRE_MESSAGE(centre_eg > home_eg,
                    "with the board bare, the king wants the centre");
  }

  // The king carries no material. Both sides always have exactly one in a legal
  // position, so the term could only ever cancel, and pricing it meant an
  // illegal position with an unbalanced king count scored above every mate -
  // which search() then had to guard against when deciding whether a result was
  // a mate at all.
  TEST_CASE_FIXTURE(eval_fixture_t, "a missing king is not worth anything")
  {
    // A lone king still moves the score, because it stands on a square the
    // tables have an opinion about. What it must not do is carry material: a
    // whole missing king has to be worth less than a single pawn.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/8 w - - 0 1", &game));
    REQUIRE(std::abs(evaluate(&game.board)) < 100);

    REQUIRE(load_FEN("8/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE(std::abs(evaluate(&game.board)) < 100);

    // The score has to stay well inside the mate band, or search() reports a
    // material imbalance as a mate.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1", &game));
    REQUIRE(std::abs(evaluate(&game.board)) < 48000);
  }
}


TEST_SUITE("evaluation: game phase")
{
  TEST_CASE_FIXTURE(eval_fixture_t, "runs from a full board down to bare kings")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE_EQ(game_phase(&game.board), 0);

    // Pawns are not part of it, so a pawn endgame is still phase 0.
    REQUIRE(load_FEN("4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1", &game));
    REQUIRE_EQ(game_phase(&game.board), 0);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "weights the pieces the tables expect")
  {
    struct case_t
    {
      std::string fen;
      int phase;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", 1, "knight"},
      {"4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", 1, "bishop"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 2, "rook"},
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", 4, "queen"},
      {"3qk3/8/8/8/8/8/8/3QK3 w - - 0 1", 8, "a queen each"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(game_phase(&game.board) == test.phase, test.title);
    }
  }

  // Promotions can put more material on the board than the opening had, and a
  // tapered term that interpolates on an out-of-range phase reads off the end
  // of its own tables.
  TEST_CASE_FIXTURE(eval_fixture_t, "never exceeds the maximum")
  {
    REQUIRE(load_FEN("qqqqkqqq/qqqqqqqq/8/8/8/8/QQQQQQQQ/QQQQKQQQ w - - 0 1",
                     &game));
    REQUIRE_EQ(game_phase(&game.board), GAME_PHASE_MAX);
  }
}


TEST_SUITE("evaluation: capture_score")
{
  // Finds the move matching from/to in the current position, so the tests can
  // name moves without hand-encoding move_t.
  static move_t find_move(game_t * g, index_t from, index_t to)
  {
    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(g, moves);

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) == from && MOVE_TO(moves[i]) == to) {
        return moves[i];
      }
    }

    return 0;
  }

  // One position, one victim, two attackers. Comparing two positions with
  // different victims instead would be satisfied by a score that ignores the
  // attacker entirely, which is exactly the half of MVV-LVA under test here.
  TEST_CASE_FIXTURE(eval_fixture_t, "MVV-LVA prefers a cheap attacker")
  {
    // The rook on d5 is attacked by the pawn on c4 and by the queen on d1.
    REQUIRE(load_FEN("4k3/8/8/3r4/2P5/8/8/3QK3 w - - 0 1", &game));

    const move_t pawn_takes = find_move(&game, c4, d5);
    const move_t queen_takes = find_move(&game, d1, d5);

    REQUIRE(pawn_takes != 0);
    REQUIRE(queen_takes != 0);
    REQUIRE(MOVE_PIECE(pawn_takes) == W_PAWN);
    REQUIRE(MOVE_PIECE(queen_takes) == W_QUEEN);

    REQUIRE(capture_score(&game.board, pawn_takes) >
            capture_score(&game.board, queen_takes));
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a richer victim scores higher")
  {
    // Same attacker, different victim: rook must beat knight.
    REQUIRE(load_FEN("4k3/8/8/3r4/4P3/8/8/4K3 w - - 0 1", &game));
    const int pxr = capture_score(&game.board, find_move(&game, e4, d5));

    REQUIRE(load_FEN("4k3/8/8/3n4/4P3/8/8/4K3 w - - 0 1", &game));
    const int pxn = capture_score(&game.board, find_move(&game, e4, d5));

    REQUIRE(pxr > pxn);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "en passant victim is priced as a pawn")
  {
    // The captured pawn does not sit on the target square, so capture_score
    // has a dedicated branch for it. It must agree with an ordinary pawn take.
    REQUIRE(load_FEN("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", &game));
    const move_t en_passant = find_move(&game, e5, d6);
    REQUIRE(en_passant != 0);
    REQUIRE(MOVE_EN_PASSANT(en_passant));
    const int ep_score = capture_score(&game.board, en_passant);

    REQUIRE(load_FEN("4k3/8/3p4/4P3/8/8/8/4K3 w - - 0 1", &game));
    const move_t ordinary = find_move(&game, e5, d6);
    REQUIRE(ordinary != 0);
    const int ordinary_score = capture_score(&game.board, ordinary);

    REQUIRE_EQ(ep_score, ordinary_score);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a quiet move ranks below every capture")
  {
    REQUIRE(load_FEN("4k3/8/8/3q4/4P3/8/8/4K3 w - - 0 1", &game));

    const int capture = capture_score(&game.board, find_move(&game, e4, d5));
    const int quiet = capture_score(&game.board, find_move(&game, e1, d1));

    REQUIRE(capture > quiet);
  }
}


TEST_SUITE("evaluation: score_move ordering")
{
  // The ordering bands are private to evaluation.cpp, so these assert the
  // relative order rather than the constants. That is what the search relies
  // on anyway.
  TEST_CASE_FIXTURE(eval_fixture_t, "bands are strictly ordered")
  {
    REQUIRE(
        load_FEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/"
                 "R3K2R w KQkq - 0 1",
                 &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);
    REQUIRE(count > 8);

    move_t a_capture = 0;
    move_t quiets[4] = {};
    size_t quiet_count = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_CAPTURE(moves[i]) && a_capture == 0) {
        a_capture = moves[i];
      } else if (!MOVE_CAPTURE(moves[i]) &&
                 MOVE_PROMOTED(moves[i]) == TO_NONE && quiet_count < 4) {
        quiets[quiet_count++] = moves[i];
      }
    }

    REQUIRE(a_capture != 0);
    REQUIRE_EQ(quiet_count, 4);

    const size_t ply = 3;

    search_state_t state = {};
    state.killer_moves[0][ply] = quiets[0];
    state.killer_moves[1][ply] = quiets[1];

    // The counter move is keyed on the previous move, so it needs one.
    const move_t prev_move = quiets[3];
    state.counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] = quiets[2];

    // A plain quiet move only carries its history score.
    const move_t plain = quiets[3];
    state.history_moves[MOVE_PIECE(plain)][MOVE_TO(plain)] = 42;

    const move_t tt_move = a_capture;

    const int s_tt =
        score_move(&game, &state, tt_move, tt_move, ply, prev_move);
    const int s_capture =
        score_move(&game, &state, a_capture, 0, ply, prev_move);
    const int s_killer0 =
        score_move(&game, &state, quiets[0], 0, ply, prev_move);
    const int s_killer1 =
        score_move(&game, &state, quiets[1], 0, ply, prev_move);
    const int s_counter =
        score_move(&game, &state, quiets[2], 0, ply, prev_move);
    const int s_history = score_move(&game, &state, plain, 0, ply, prev_move);

    REQUIRE(s_tt > s_capture);
    REQUIRE(s_capture > s_killer0);
    REQUIRE(s_killer0 > s_killer1);
    REQUIRE(s_killer1 > s_counter);
    REQUIRE(s_counter > s_history);
    REQUIRE_EQ(s_history, 42);
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "a promotion outranks a plain quiet move")
  {
    REQUIRE(load_FEN("6k1/4P3/8/8/8/8/8/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t promotion = 0;
    move_t quiet = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_PROMOTED(moves[i]) == TO_QUEEN) { promotion = moves[i]; }
      if (MOVE_PROMOTED(moves[i]) == TO_NONE && quiet == 0) {
        quiet = moves[i];
      }
    }

    REQUIRE(promotion != 0);
    REQUIRE(quiet != 0);

    const search_state_t state = {};

    REQUIRE(score_move(&game, &state, promotion, 0, 0, 0) >
            score_move(&game, &state, quiet, 0, 0, 0));
  }

  TEST_CASE_FIXTURE(eval_fixture_t, "queen promotion outranks knight promotion")
  {
    REQUIRE(load_FEN("6k1/4P3/8/8/8/8/8/4K3 w - - 0 1", &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t to_queen = 0;
    move_t to_knight = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_PROMOTED(moves[i]) == TO_QUEEN) { to_queen = moves[i]; }
      if (MOVE_PROMOTED(moves[i]) == TO_KNIGHT) { to_knight = moves[i]; }
    }

    REQUIRE(to_queen != 0);
    REQUIRE(to_knight != 0);

    const search_state_t state = {};

    REQUIRE(score_move(&game, &state, to_queen, 0, 0, 0) >
            score_move(&game, &state, to_knight, 0, 0, 0));
  }
}
