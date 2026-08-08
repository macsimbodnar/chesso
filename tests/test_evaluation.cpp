#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
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

      REQUIRE_MESSAGE(score == -mirrored_score,
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

  TEST_CASE_FIXTURE(eval_fixture_t, "score is from White's point of view")
  {
    // White is a whole queen up. The sign must not depend on side to move.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 w - - 0 1", &game));
    REQUIRE(evaluate(&game.board) > 0);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/3QK3 b - - 0 1", &game));
    REQUIRE(evaluate(&game.board) > 0);

    REQUIRE(load_FEN("3qk3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE(evaluate(&game.board) < 0);
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

  // ANCHOR: pinned to the material-only evaluation. Update when the evaluation
  // gains positional terms.
  //
  // Symmetry and ordering say nothing about what a piece is actually worth:
  // every one of these values can be changed without moving any other
  // assertion in this file, and a wrong one costs games rather than crashes.
  TEST_CASE_FIXTURE(eval_fixture_t, "each piece is worth what the table says")
  {
    struct case_t
    {
      std::string fen;
      int score;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 100, "pawn"},
      {"4k3/8/8/8/8/8/8/1N2K3 w - - 0 1", 300, "knight"},
      {"4k3/8/8/8/8/8/8/2B1K3 w - - 0 1", 300, "bishop"},
      {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 500, "rook"},
      {"4k3/8/8/8/8/8/8/3QK3 w - - 0 1", 900, "queen"},
      {"4k3/8/8/8/8/8/8/4K3 w - - 0 1",    0, "bare kings cancel"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(evaluate(&game.board) == test.score, test.title);
    }
  }

  // ANCHOR: the king price only ever shows up when the two sides do not have
  // one each, which cannot happen in a legal game but does arrive through
  // [position fen]. search() leans on it being far above any mate score when
  // it decides whether a result is a mate at all.
  TEST_CASE_FIXTURE(eval_fixture_t, "a missing king outweighs every mate score")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/8 w - - 0 1", &game));
    REQUIRE_EQ(evaluate(&game.board), -100000);

    REQUIRE(load_FEN("8/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE_EQ(evaluate(&game.board), 100000);
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
