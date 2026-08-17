#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "search.hpp"
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "transposition_table.hpp"


static game_t game;
static transposition_table_t tt;


struct search_fixture_t
{
  search_fixture_t()
  {
    initialize_game_const_data(&game);

    // 4MB is plenty for the shallow searches here and keeps the binary cheap
    // to start up.
    if (tt.entries == nullptr) { tt_resize(&tt, 4); }
    tt_reset(&tt);
  }
};


// Runs one fixed depth search from a fresh state, so every case is
// reproducible and independent of whatever ran before it.
static search_t search_fen(const std::string& fen,
                           int depth,
                           uint64_t node_limit = NODE_BUDGET_UNLIMITED)
{
  REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

  static std::atomic_bool never_stop = false;
  never_stop = false;

  tt_reset(&tt);
  tt_new_search(&tt);

  search_state_t state = {};
  state.tt = &tt;
  state.stop = &never_stop;
  state.node_limit = node_limit;

  return search(depth, &game, &state);
}


// Same thing, but the caller supplies the table and decides whether to wipe it
// first. Used by the transposition table suite, which needs a cold run, a warm
// run and a run against a table that was never allocated.
static search_t search_fen_with(const std::string& fen,
                                int depth,
                                transposition_table_t* table,
                                bool reset)
{
  REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

  static std::atomic_bool never_stop = false;
  never_stop = false;

  if (reset) { tt_reset(table); }
  tt_new_search(table);

  search_state_t state = {};
  state.tt = table;
  state.stop = &never_stop;

  return search(depth, &game, &state);
}


TEST_SUITE("search: mate detection")
{
  TEST_CASE_FIXTURE(search_fixture_t, "mate in one")
  {
    struct case_t
    {
      std::string fen;
      std::string best_move;
      int mate_in;
    };

    // mate_in follows the UCI convention: it is relative to the side to move,
    // so a mate delivered by Black while Black is on move is still positive.
    // clang-format off
    const std::vector<case_t> cases = {
      {"6k1/5ppp/8/8/8/8/8/R3K2R w KQ - 0 1",  "a1a8", 1},
      {"r3k2r/8/8/8/8/8/5PPP/6K1 b kq - 0 1",  "a8a1", 1},
      {"7k/6pp/8/8/8/8/8/R6K w - - 0 1",       "a1a8", 1},
    };
    // clang-format on

    for (const case_t& test : cases) {
      const search_t result = search_fen(test.fen, 3);

      REQUIRE_MESSAGE(result.mate_found, ("FEN: " + test.fen));
      REQUIRE_MESSAGE(result.mate_in == test.mate_in,
                      ("FEN: " + test.fen + " got mate in " +
                       std::to_string(result.mate_in)));

      REQUIRE(load_FEN(test.fen, &game));
      REQUIRE_MESSAGE(move_is_legal(&game, result.best_move),
                      ("FEN: " + test.fen));

      const std::string played = index_to_str(MOVE_FROM(result.best_move)) +
                                 index_to_str(MOVE_TO(result.best_move));
      REQUIRE_MESSAGE(played == test.best_move,
                      ("FEN: " + test.fen + " played " + played));
    }
  }

  // The distance is asserted exactly. A range would pass with the mate score
  // normalised by the wrong number of plies on its way through the
  // transposition table, which is the whole thing worth checking here.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "mate in two is found at the right distance")
  {
    // Needs depth 3 at least: white move, black reply, white mate.
    for (int depth = 3; depth <= 6; ++depth) {
      const std::string title = "depth " + std::to_string(depth);

      const search_t white = search_fen(MATE_IN_2_W_POS, depth);
      REQUIRE_MESSAGE(white.mate_found, title);
      REQUIRE_MESSAGE(white.mate_in == 2, title);

      // Black to move and mating: still positive, the score is relative to the
      // side to move.
      const search_t black = search_fen(MATE_IN_2_B_POS, depth);
      REQUIRE_MESSAGE(black.mate_found, title);
      REQUIRE_MESSAGE(black.mate_in == 2, title);
    }
  }

  // The mate and the stalemate below are a matched pair: same bare material,
  // same side with no move, and the only difference is whether that side is in
  // check. Both positions have to be reachable or the pair separates nothing.
  //
  // The mate half used 7k/5Q1K/8/8/8/8/8/8 b until S067, where the *only*
  // attacker of the black king was the white king on h7 - adjacent kings, which
  // no legal game reaches, and a "check" delivered by a king rather than by a
  // checking piece. It asserted mate in zero and passed, and would have gone on
  // passing through a regression that only broke checks from real pieces.
  // 2026-08-14_test_review-F02.
  TEST_CASE_FIXTURE(search_fixture_t, "a mated side reports mate in zero")
  {
    const std::string fen = "7k/6Q1/6K1/8/8/8/8/8 b - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    REQUIRE_MESSAGE(position_is_reachable(&game),
                    (fen + " is not a position a legal game can reach"));
    REQUIRE(is_check(&game));

    const search_t result = search_fen(fen, 3);

    REQUIRE(result.mate_found);
    REQUIRE_EQ(result.mate_in, 0);
    REQUIRE_EQ(result.best_move, 0);
    REQUIRE_EQ(result.pv.length, 0);
  }

  TEST_CASE_FIXTURE(search_fixture_t, "stalemate scores zero, not mate")
  {
    const std::string fen = "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    REQUIRE_MESSAGE(position_is_reachable(&game),
                    (fen + " is not a position a legal game can reach"));
    REQUIRE_FALSE(is_check(&game));

    const search_t result = search_fen(fen, 3);

    REQUIRE_FALSE(result.mate_found);
    REQUIRE_EQ(result.score, 0);
    REQUIRE_EQ(result.best_move, 0);
  }

  TEST_CASE_FIXTURE(search_fixture_t, "a balanced position is not a mate")
  {
    const search_t result = search_fen(DEFAULT_POSITION, 5);

    REQUIRE_FALSE(result.mate_found);
    REQUIRE(result.best_move != 0);
  }
}


TEST_SUITE("search: invariants")
{
  // Every result invariant is checked in one pass: a search over the whole
  // JSON corpus is the expensive part, so running it once and asserting four
  // properties keeps this suite inside the "fast" label.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "search results are well formed everywhere")
  {
    size_t checked = 0;

    for (const std::string& fen : all_test_fens()) {
      const search_t result = search_fen(fen, 3);

      REQUIRE(load_FEN(fen, &game));

      move_t moves[MAX_MOVES];
      const size_t legal = legal_moves(&game, moves);

      // Whatever comes back must be playable. This is the property that
      // matters most: an illegal best move forfeits the game.
      if (legal == 0) {
        // Terminal position: no move to report is the correct answer.
        REQUIRE_MESSAGE(result.best_move == 0, ("FEN: " + fen));
      } else {
        REQUIRE_MESSAGE(result.best_move != 0, ("FEN: " + fen));
        REQUIRE_MESSAGE(
            move_is_legal(&game, result.best_move),
            ("FEN: " + fen + " returned " + print_move(result.best_move)));
      }

      REQUIRE_MESSAGE(result.pv.length <= MAX_PLY, ("FEN: " + fen));

      if (result.pv.length > 0) {
        // A reported line has to be playable end to end, and it has to start
        // with the move the caller is being told to make.
        REQUIRE_MESSAGE(result.pv.table[0] == result.best_move,
                        ("FEN: " + fen));

        REQUIRE(load_FEN(fen, &game));
        REQUIRE_MESSAGE(is_pv_legal(&game, &result.pv), ("FEN: " + fen));
      }

      checked++;
    }

    REQUIRE(checked > 100);
  }

  TEST_CASE_FIXTURE(search_fixture_t,
                    "the same search twice gives the same answer")
  {
    // A fresh state and a fresh table must make the search a pure function of
    // position and depth. Anything else means state is leaking between runs.
    const std::vector<std::string> positions = {
        DEFAULT_POSITION,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    };

    for (const std::string& fen : positions) {
      const search_t first = search_fen(fen, 5);
      const search_t second = search_fen(fen, 5);

      REQUIRE_MESSAGE(first.best_move == second.best_move, ("FEN: " + fen));
      REQUIRE_MESSAGE(first.score == second.score, ("FEN: " + fen));
      REQUIRE_MESSAGE(first.explored_nodes == second.explored_nodes,
                      ("FEN: " + fen));
    }
  }

  TEST_CASE_FIXTURE(search_fixture_t, "deeper search never loses the move")
  {
    for (int depth = 1; depth <= 6; ++depth) {
      const search_t result = search_fen(DEFAULT_POSITION, depth);

      REQUIRE_MESSAGE(result.best_move != 0,
                      ("depth " + std::to_string(depth)));

      REQUIRE(load_FEN(DEFAULT_POSITION, &game));
      REQUIRE(move_is_legal(&game, result.best_move));
    }
  }
}


TEST_SUITE("search: budgets")
{
  TEST_CASE_FIXTURE(search_fixture_t, "the node budget is honoured exactly")
  {
    for (const uint64_t budget : {uint64_t(1), uint64_t(64), uint64_t(4096)}) {
      const search_t result = search_fen(DEFAULT_POSITION, 12, budget);

      REQUIRE_MESSAGE(result.explored_nodes <= budget,
                      ("budget " + std::to_string(budget) + " explored " +
                       std::to_string(result.explored_nodes)));
    }
  }

  // A budget this small is spent inside the first root move, so the search
  // never finishes one and has nothing to report. Reporting nothing is the
  // contract the UCI layer is built on: it substitutes first_legal_move(),
  // covered by "a one node search still answers with a legal move" in
  // test_engine. What must never happen is a non-zero move that is not
  // playable, which is what used to reach the GUI as "bestmove 0000".
  TEST_CASE_FIXTURE(search_fixture_t,
                    "an aborted search reports no move at all")
  {
    for (const uint64_t budget :
         {uint64_t(1), uint64_t(2), uint64_t(8), uint64_t(64), uint64_t(512),
          uint64_t(4096)}) {
      const search_t result = search_fen(DEFAULT_POSITION, 12, budget);
      const std::string title = "budget " + std::to_string(budget);

      REQUIRE_MESSAGE(result.explored_nodes <= budget, title);
      REQUIRE_MESSAGE(result.best_move == 0, title);
      REQUIRE_MESSAGE(result.pv.length == 0, title);
    }
  }

  // The other side of the same contract, and what keeps the assertion above
  // from being satisfied by a search that never answers anything.
  TEST_CASE_FIXTURE(search_fixture_t, "a budget large enough to finish answers")
  {
    // Depth 2 from the start position costs a few hundred nodes, so this
    // budget cannot be what stops it.
    const search_t result = search_fen(DEFAULT_POSITION, 2, 1000000);

    REQUIRE(result.best_move != 0);
    REQUIRE(result.pv.length > 0);
    REQUIRE_EQ(result.pv.table[0], result.best_move);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE(move_is_legal(&game, result.best_move));
  }

  TEST_CASE_FIXTURE(search_fixture_t, "the stop flag ends the search")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    std::atomic_bool stop = true;

    tt_reset(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &stop;

    // Already set on entry, so this must come back rather than run to depth 30.
    const search_t result = search(30, &game, &state);

    REQUIRE(state.aborted);
    REQUIRE(result.explored_nodes < 100000);
  }
}


TEST_SUITE("search: tactics")
{
  // Positions where one move is objectively best and the reason is material,
  // which is all this evaluation understands. Every other test in this file
  // checks that the search is well formed; these check that it is any good.
  TEST_CASE_FIXTURE(search_fixture_t, "the winning move is found")
  {
    struct case_t
    {
      std::string fen;
      index_t from;
      index_t to;
      promotion_t promotion;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"4k3/8/8/3q4/8/8/8/3RK3 w - - 0 1",       d1, d5, TO_NONE,  "take the hanging queen"},
      {"4k3/8/q7/1N6/8/8/8/4K3 w - - 0 1",       b5, c7, TO_NONE,  "fork the king and the queen"},
      {"4k3/8/8/8/8/8/4r3/4K1R1 w - - 0 1",      e1, e2, TO_NONE,  "the king takes the loose rook"},
      // The white king stands on e4 in the open on purpose. The position this
      // case used to hold, 2k5/8/8/8/8/8/1q6/K1R5 w, had **one legal move** --
      // and was not a legal position either: stockfish refused the FEN with
      // "Unsupported position. King can be captured" and python-chess reported
      // OPPOSITE_CHECK. It passed for the same reason any engine would.
      //
      // Here the king has somewhere to run, so taking the queen is chosen
      // rather than forced: python-chess reports 3 legal moves and stockfish at
      // depth 20 MultiPV 3 gives e4e3 mate 20, e4d5 mate -12, e4f5 mate -11.
      // The rook on h1 is why: the same position without it is e4e3 cp 0 by the
      // same measurement.
      {"4k3/8/8/8/4K3/4q3/8/7R w - - 0 1",       e4, e3, TO_NONE,  "the king takes the loose queen"},
      // The black king stands on d8 rather than e8 on purpose. With the king on
      // e8 the queen is pinned to it by the rook on e2 and cannot leave the
      // file, so every white move wins it and the position has no unique
      // answer - it passed only because captures happen to be ordered first.
      // On d8 the queen is free to run, so Rxe7 has to be played at once.
      {"3k4/4q3/8/8/8/8/4R3/4KR2 w - - 0 1",     e2, e7, TO_NONE,  "the doubled rooks win the queen"},
      // The kings are parked out of the way on purpose. With them on e1 and e8
      // the black king walks back and wins the pawn again, so the position is
      // drawn whatever White plays and the case is not about a free pawn at
      // all - it passed only because captures are ordered first.
      {"k7/8/8/3p4/4P3/8/8/6K1 w - - 0 1",       e4, d5, TO_NONE,  "take the free pawn"},
      // The black king stands on c7 on purpose. The position this case used to
      // hold, with the black king on h7 and nothing else near, has four moves
      // that all mate in ten by Stockfish at depth 20 -- a8=Q, Kd2, Ke2 and Kf2
      // -- so it asserted a preference among equals and passed only because the
      // promotion happened to be ordered first. S028's tuned tables order them
      // differently and it failed. On c7 the black king is close enough that
      // dawdling throws the win away: a8=Q is mate in 14, a8=R is +387, a8=B is
      // +10 and Ke2 is 0.
      {"8/P1k5/8/8/8/8/8/4K3 w - - 0 1",         a7, a8, TO_QUEEN, "promote to a queen"},
      {"6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1",      a1, a8, TO_NONE,  "mate on the back rank"},
      {"r5k1/8/8/8/8/8/5PPP/6K1 b - - 0 1",      a8, a1, TO_NONE,  "mate on the back rank, black"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      // Preconditions, asserted before any search runs. A case whose position
      // is unreachable is not about the search, and a case with one legal move
      // is passed by any engine that returns a legal move at all - which is
      // what "the king takes the loose queen" did on
      // 2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1 until S067. The class is what is
      // closed here; the one instance was the symptom.
      // 2026-08-14_test_review-F01.
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);

      REQUIRE_MESSAGE(position_is_reachable(&game),
                      (test.title + ": " + test.fen +
                       " is not a position a legal game can reach"));

      move_t precondition_moves[MAX_MOVES];
      const size_t choices = legal_moves(&game, precondition_moves);

      REQUIRE_MESSAGE(
          choices > 1,
          (test.title + ": " + test.fen + " has " + std::to_string(choices) +
           " legal move(s); a forced move asserts nothing about "
           "which move the search prefers"));

      // Stable across depths, so a change of search depth cannot quietly turn
      // a solved position into an unsolved one.
      for (int depth = 4; depth <= 6; ++depth) {
        const search_t result = search_fen(test.fen, depth);

        const std::string title = test.title + " at depth " +
                                  std::to_string(depth) + ", played " +
                                  print_move(result.best_move);

        REQUIRE_MESSAGE(MOVE_FROM(result.best_move) == test.from, title);
        REQUIRE_MESSAGE(MOVE_TO(result.best_move) == test.to, title);
        REQUIRE_MESSAGE(MOVE_PROMOTED(result.best_move) == test.promotion,
                        title);
      }
    }
  }
}


TEST_SUITE("search: move ordering state")
{
  // test_evaluation checks that score_move ranks a killer above a counter
  // above a history entry. Nothing there checks that the search ever writes
  // one, and a search that fills none of these tables plays the same moves,
  // only slower.
  TEST_CASE_FIXTURE(search_fixture_t, "a search fills the ordering tables")
  {
    REQUIRE(load_FEN(TRICKY_POS, &game));

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    // Owned here rather than by search_fen(), so the tables survive the call.
    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const search_t result = search(6, &game, &state);
    REQUIRE(result.best_move != 0);

    size_t killers_0 = 0;
    size_t killers_1 = 0;

    for (size_t ply = 0; ply < MAX_PLY; ++ply) {
      if (state.killer_moves[0][ply] != 0) { killers_0++; }
      if (state.killer_moves[1][ply] != 0) { killers_1++; }
    }

    size_t history_entries = 0;
    size_t counters = 0;

    for (int piece = W_PAWN; piece <= B_KING; ++piece) {
      for (int square = 0; square < 64; ++square) {
        if (state.history_moves[piece][square] != 0) { history_entries++; }
        if (state.counter_moves[piece][square] != 0) { counters++; }
      }
    }

    REQUIRE(killers_0 > 0);

    // The second slot only fills once a ply produces a second killer, which
    // is what the shift down from slot 0 is for.
    REQUIRE(killers_1 > 0);
    REQUIRE(history_entries > 0);
    REQUIRE(counters > 0);
  }

  // Filling the tables is not the point: searching a smaller tree is. A
  // budget rather than a plain node count assertion, so a search that has
  // stopped ordering anything fails here instead of running until the test
  // times out.
  //
  // The budget was 1000000 until S067 and the search costs 109575, so ordering
  // had to get nine times worse before the case could fire. Measured rather
  // than guessed: `CHECK(result.explored_nodes == 0)` in this case on
  // 2026-08-14 reported `CHECK( 109575 == 0 )`, cold table, depth 5, this
  // position. The budget is **4x** that, rounded, which fires on the kind of
  // regression the case exists for while leaving room for the tree to move
  // when the evaluation constants change - they order the moves, and S065 is a
  // pending 827-constant paste. Tighten it when a fit lands, not before.
  // 2026-08-14_test_review-F06.
  TEST_CASE_FIXTURE(search_fixture_t, "ordering keeps the tree small")
  {
    REQUIRE(load_FEN(TRICKY_POS, &game));

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;
    state.node_limit = 440000;

    const search_t result = search(5, &game, &state);

    REQUIRE_FALSE(state.aborted);
    REQUIRE(result.best_move != 0);

    // The budget has to stay a bound on something, not a number nothing
    // approaches: if the tree ever shrinks far below it the case has stopped
    // discriminating and the budget wants re-measuring rather than leaving.
    REQUIRE_MESSAGE(
        result.explored_nodes > 20000,
        ("depth 5 on TRICKY_POS cost " + std::to_string(result.explored_nodes) +
         " nodes; the 440000 budget was set from 109575 and no "
         "longer bounds anything - re-measure it"));
  }
}


TEST_SUITE("search: quiescence")
{
  // quiescence() is where every leaf score comes from, and none of the
  // assertions elsewhere in this file can tell a working one from a stub.
  static int quiesce(const std::string& fen, int alpha, int beta)
  {
    REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    return quiescence(alpha, beta, 0, 0, &game, &state);
  }

  // Standing pat means "I do not have to do anything here". A side that is
  // already above beta stops at once and reports a score above beta.
  //
  // Since S034 that score is not always the exact one. When the cheap terms
  // alone are a margin clear of beta, quiescence never computes the expensive
  // ones and what it returns is a bound instead of a score. It has to be a
  // bound on the right side of both beta and the truth: this is fail-soft, so
  // the number is stored as a lower bound and read back as one. The exact score
  // is what comes back when the window contains it, and that is asserted below
  // too.
  TEST_CASE_FIXTURE(search_fixture_t, "a quiet position stands pat")
  {
    // White is a rook up with nothing to capture.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    const int cheap_score = evaluate_cheap(&game.board);
    // S065's fit, DEC-059: 530 to 509. The cheap score does not move -- the
    // fit changed what the tables say, not the two terms evaluate_cheap() adds.
    // S076's refit on the deduplicated corpus moved both: 509 to 563 and 524 to
    // 567. Both re-derived by `.tuning/anchors.py`, which computes this
    // position's evaluate() and evaluate_cheap() from the specification rather
    // than from the engine.
    REQUIRE_EQ(static_score, 563);
    REQUIRE_EQ(cheap_score, 567);

    // The precondition for the shortcut. Without it the assertions below would
    // pass on a build where the shortcut never fires at all.
    REQUIRE(cheap_score - LAZY_EVAL_MARGIN >= 100);

    // Beta far enough below: the cutoff is immediate and the expensive terms
    // are never computed, so what comes back is a lower bound rather than the
    // score.
    const int cut = quiesce(fen, 0, 100);

    // The cutoff it reports is a real one: the number returned and the exact
    // score are both above beta, so nothing was cut that should not have been.
    REQUIRE(cut >= 100);
    REQUIRE(static_score >= 100);

    // And the number is one the parent can keep. This is fail-soft, so it is
    // propagated and stored as a lower bound on the true score and must not be
    // above it. Which number the shortcut picks to satisfy that is the
    // implementation's business and is deliberately not pinned here.
    //
    // Strictly below, and that is what shows the shortcut fired at all: the
    // expensive terms are worth +6 between them here, mobility +16 and king
    // safety -10 since S027 fitted it, so a quiescence that had computed them
    // would have answered 530 on the nose. The witness is thinner than it was
    // when mobility had the stage to itself and the gap was the whole +16.
    REQUIRE(cut < static_score);

    // A window that contains it: still the static score, there are no
    // captures to change it.
    REQUIRE_EQ(quiesce(fen, -10000, 10000), static_score);
  }

  // The one case where standing pat is not on offer: the side to move is in
  // check and has to reply, so every evasion is searched instead.
  TEST_CASE_FIXTURE(search_fixture_t, "a side in check may not stand pat")
  {
    // Black is ahead on material, so standing pat would report a comfortable
    // score. Black is also in check from the rook on e8 with exactly one
    // legal reply, Qxe8, and after Rxe8 it is mate on the back rank.
    const std::string fen = "4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0 1";

    REQUIRE(load_FEN(fen, &game));

    // evaluate() and quiescence both answer from the side to move's point of
    // view, so Black's static score is what evaluate() returns.
    // Two pawns up, plus whatever the positional terms make of the squares
    // everything happens to be standing on.
    //
    // S027's passed pawn term moved this from 292: White has no pawns at all,
    // so all three of Black's are passed and all three sit on Black's own
    // second rank, bucket 0. Three of them at phase 8 -- two rooks and a queen
    // -- come to 27 centipawns White-relative, which is 27 off Black's score,
    // and the last centipawn is the single truncating division evaluate_cheap()
    // does over the summed pair.
    // S065's fit, DEC-059, then moved it again: 266 to 224, and S076's refit on
    // the deduplicated corpus to 198 -- the one anchor of the five that went
    // down.
    const int black_static = evaluate(&game.board);
    REQUIRE_EQ(black_static, 198);

    const int score = quiesce(fen, -10000000, 10000000);

    REQUIRE_MESSAGE(score < black_static,
                    ("stood pat on " + std::to_string(score)));

    // Not just worse: the line is forced all the way to mate, and only a
    // quiescence that searches evasions can see it.
    REQUIRE(score < -10000);
  }

  // Being in check is not the same as being mated, and the reply need not be
  // a capture. A quiescence that only ever looks at captures finds no move
  // here and calls a perfectly ordinary position a mate.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a quiet evasion is a legal answer to a check")
  {
    // The rook on e8 checks down the open e file. White has four king moves
    // and not one of them takes anything.
    const std::string fen = "4rk2/8/8/8/8/8/8/4K3 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    REQUIRE(is_check(&game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);
    REQUIRE(count > 0);

    for (size_t i = 0; i < count; ++i) {
      REQUIRE_FALSE(MOVE_CAPTURE(moves[i]));
    }

    // A rook down, give or take what the positional terms make of where the
    // kings end up. Not a call to evaluate() and not an evaluation anchor: in
    // check, quiescence searches the evasions instead of standing pat, so this
    // is a one-ply negamax. Each of the four leaves has Black to move, not in
    // check and with no capture, so each returns its own stand pat, and the
    // root is the best of them. S065's fit, DEC-059: -491 to -446, and S076's
    // refit on the deduplicated corpus to -505, derived leaf by leaf from the
    // specification rather than read off the engine both times. Ke1-d2 is still
    // the evasion that survives: the four leaves are -555, -505, -570 and -522
    // from the root's side, so it is the best by 17 rather than by 1.
    REQUIRE_EQ(quiesce(fen, -10000000, 10000000), -505);
  }

  // No legal reply to a check is mate, and quiescence has to say so on its
  // own: at depth zero it is the only thing that runs. Same position as
  // "a mated side reports mate in zero" and the same precondition on it, for
  // the same reason. 2026-08-14_test_review-F02.
  TEST_CASE_FIXTURE(search_fixture_t, "mate is recognised at depth zero")
  {
    const std::string fen = "7k/6Q1/6K1/8/8/8/8/8 b - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    REQUIRE_MESSAGE(position_is_reachable(&game),
                    (fen + " is not a position a legal game can reach"));

    const search_t result = search_fen(fen, 0);

    REQUIRE(result.mate_found);
    REQUIRE_EQ(result.mate_in, 0);
  }

  // "No captures available" is not mate. Without the check test guarding it,
  // a quiet position with nothing to take would be scored as one.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a quiet position at depth zero is not mate")
  {
    const search_t result = search_fen("4k3/8/8/8/8/8/8/3RK3 w - - 0 1", 0);

    REQUIRE_FALSE(result.mate_found);
  }

  // Evasions are searched as well as captures, so a long forced sequence of
  // checks is bounded only by the quiescence depth limit. This says nothing
  // about the value, only that the recursion ends and stays cheap.
  TEST_CASE_FIXTURE(search_fixture_t, "quiescence stays bounded")
  {
    const std::vector<std::string> busy = {
        "4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    };

    for (const std::string& fen : busy) {
      REQUIRE(load_FEN(fen, &game));

      static std::atomic_bool never_stop = false;
      never_stop = false;

      tt_reset(&tt);

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      quiescence(-10000000, 10000000, 0, 0, &game, &state);

      REQUIRE_MESSAGE(state.explored_nodes > 0, ("FEN: " + fen));
      REQUIRE_MESSAGE(
          state.explored_nodes < 100000,
          ("FEN: " + fen + " nodes " + std::to_string(state.explored_nodes)));
    }
  }
}


TEST_SUITE("search: transposition table")
{
  // Quiet middlegame and endgame positions with no forced mate inside the
  // depths used here, so every line runs the full depth and the assertions
  // below are about the table rather than about a terminal position.
  // clang-format off
  static const std::vector<std::string> tt_positions = {
    DEFAULT_POSITION,
    TRICKY_POS,
    CMK_POS,
    KILLER_POS,
    FINE_70_POS,
    CLOSED_POSITION,
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "4k3/8/8/3q4/8/8/8/3RK3 w - - 0 1",
  };
  // clang-format on

  // The strongest property the table has: it is an accelerator, not part of
  // the answer. A wrong bound test, a cutoff taken at the wrong depth or a
  // score stored without its mate normalisation all show up as a search that
  // answers differently depending on what happens to be in the table.
  //
  // Only holds while the search is a pure function of position and depth.
  // Reductions break that - the table supplies a move, which changes the
  // ordering, which changes which moves are late enough to be reduced - so this
  // runs below the depth where any of them engage. Late move reduction needs
  // ply > 0 and depth >= 3, so a root search of depth 3 or less has none
  // anywhere in its tree, and null move pruning needs more depth still.
  //
  // The lazy evaluation shortcut breaks it too, and unlike the reductions there
  // is no depth below which it does not. Whether the expensive terms are
  // computed at a leaf depends on the window, the table changes the windows the
  // search arrives with, so a leaf can be scored exactly on one run and by a
  // bound on the next. It caught the shortcut returning the cheap score, which
  // is not a bound at all: the same position answered 110 or 59 depending on
  // what was cached. That is fixed - the shortcut returns the guaranteed bound
  // now - and this case is green again, but green here means only that the
  // corpus and the depths below do not expose the difference. It is not the
  // purity the case claims, and a position where a bound and the exact score
  // land on opposite sides of a cutoff would fail here without anything being
  // wrong with the table. Read a failure as "one of these two things changed"
  // and not as "the table is broken". S027.
  //
  // The table's own logic is pinned directly by "transposition table: storage"
  // below, which does not care what the search does.
  TEST_CASE_FIXTURE(search_fixture_t, "the table never changes the answer")
  {
    // Never allocated, so every probe misses and every store is dropped. That
    // makes it a reference search with the table taken out of the picture.
    transposition_table_t inert = {};

    for (int depth = 2; depth <= 3; ++depth) {
      for (const std::string& fen : tt_positions) {
        const std::string title =
            "depth " + std::to_string(depth) + " FEN: " + fen;

        const search_t cold = search_fen_with(fen, depth, &tt, true);
        const search_t without = search_fen_with(fen, depth, &inert, false);
        const search_t warm = search_fen_with(fen, depth, &tt, false);

        REQUIRE_MESSAGE(cold.score == without.score, title);
        REQUIRE_MESSAGE(warm.score == without.score, title);

        REQUIRE(load_FEN(fen, &game));
        REQUIRE_MESSAGE(move_is_legal(&game, warm.best_move), title);
      }
    }

    REQUIRE(inert.entries == nullptr);
  }

  // And the reason it is worth having at all. Without this, a search that
  // never stores or never probes passes every other assertion in this file.
  TEST_CASE_FIXTURE(search_fixture_t, "a warm table costs fewer nodes")
  {
    for (const std::string& fen : tt_positions) {
      const search_t cold = search_fen_with(fen, 5, &tt, true);
      const search_t warm = search_fen_with(fen, 5, &tt, false);

      REQUIRE_MESSAGE(
          warm.explored_nodes < cold.explored_nodes,
          ("FEN: " + fen + " cold " + std::to_string(cold.explored_nodes) +
           " warm " + std::to_string(warm.explored_nodes)));
    }
  }

  // A mate score in the table has to mean "mate in N from here", not "mate in
  // N from wherever the search happened to start". The distance is folded in
  // on the way in and taken back out on the way out, and the whole point of
  // that is visible on the stored line: one ply closer at every step, sign
  // alternating with the side to move. Without it the same entry read from a
  // different distance reports the wrong mate.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a stored mate score counts from its own position")
  {
    for (const char* fen : {MATE_IN_2_W_POS, MATE_IN_2_B_POS}) {
      const search_t result = search_fen_with(fen, 4, &tt, true);

      REQUIRE_MESSAGE(result.mate_found, fen);
      REQUIRE_MESSAGE(result.pv.length >= 3, fen);

      REQUIRE(load_FEN(fen, &game));

      const tt_entry_t* root = tt_get_entry(&tt, &game.board);
      REQUIRE(root != nullptr);
      REQUIRE_EQ(root->score, result.score);

      int previous = root->score;

      // The last move of the line delivers mate, and a position with no legal
      // move returns before it stores anything. Everything before it is in.
      for (size_t i = 0; i + 1 < result.pv.length; ++i) {
        REQUIRE(make_move(&game, result.pv.table[i]));

        const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
        const std::string title =
            std::string(fen) + " ply " + std::to_string(i + 1);

        REQUIRE_MESSAGE(entry != nullptr, title);
        REQUIRE_MESSAGE(std::abs(entry->score) == std::abs(previous) + 1,
                        (title + " stored " + std::to_string(entry->score) +
                         " after " + std::to_string(previous)));
        REQUIRE_MESSAGE((entry->score > 0) != (previous > 0), title);

        previous = entry->score;
      }
    }
  }

  TEST_CASE_FIXTURE(search_fixture_t,
                    "the root is stored with the move it played")
  {
    for (const std::string& fen : tt_positions) {
      const search_t result = search_fen_with(fen, 4, &tt, true);

      REQUIRE(load_FEN(fen, &game));
      const tt_entry_t* root = tt_get_entry(&tt, &game.board);

      REQUIRE_MESSAGE(root != nullptr, ("FEN: " + fen));
      REQUIRE_MESSAGE(root->best_move == result.best_move, ("FEN: " + fen));
      REQUIRE_MESSAGE(root->depth == 4, ("FEN: " + fen));
    }
  }

  // A table cutoff hands back a score without a move sequence, so taking one
  // on the principal variation chops the reported line short. Outside of a
  // mate the line is exactly as long as the search was deep.
  TEST_CASE_FIXTURE(search_fixture_t, "the reported line runs the full depth")
  {
    for (int depth = 4; depth <= 5; ++depth) {
      for (const std::string& fen : tt_positions) {
        const std::string title =
            "depth " + std::to_string(depth) + " FEN: " + fen;

        // Cold first, then warm: the warm run is the one that can take
        // cutoffs on the way down.
        search_fen_with(fen, depth, &tt, true);
        const search_t warm = search_fen_with(fen, depth, &tt, false);

        if (warm.mate_found) { continue; }

        REQUIRE_MESSAGE(warm.pv.length == static_cast<size_t>(depth), title);

        REQUIRE(load_FEN(fen, &game));
        REQUIRE_MESSAGE(is_pv_legal(&game, &warm.pv), title);
      }
    }
  }
}


TEST_SUITE("search: draws")
{
  TEST_CASE_FIXTURE(search_fixture_t, "a bare king endgame is a draw")
  {
    // Nothing left to do for either side, so every line is a draw. This holds
    // because the search knows the material is insufficient; without that it
    // would report the small advantage the piece-square tables see in walking
    // one king toward the middle.
    const search_t result = search_fen("7k/8/8/8/8/8/8/K7 w - - 0 1", 6);

    REQUIRE(result.best_move != 0);
    REQUIRE_EQ(result.score, 0);
    REQUIRE_FALSE(result.mate_found);
  }

  // Null move pruning leans on make_null_move leaving the position exactly as
  // it found it. A hash that does not come back poisons the transposition
  // table with entries filed under the wrong position, which shows up much
  // later as a wrong move in a position that has nothing to do with the bug.
  TEST_CASE_FIXTURE(search_fixture_t, "a null move undoes itself exactly")
  {
    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      const board_t before = game.board;
      const size_t history_before = game.history.size;

      make_null_move(&game);

      // The passed position must be a different position: the side to move
      // changed, so the key has to change with it.
      REQUIRE_MESSAGE(game.board.hash != before.hash, ("FEN: " + fen));
      REQUIRE_MESSAGE(game.board.active_color != before.active_color,
                      ("FEN: " + fen));
      REQUIRE_MESSAGE(game.board.en_passant == INVALID_INDEX, ("FEN: " + fen));

      unmake_null_move(&game);

      REQUIRE_MESSAGE(std::memcmp(&before, &game.board, sizeof(board_t)) == 0,
                      ("FEN: " + fen));
      REQUIRE_MESSAGE(game.history.size == history_before, ("FEN: " + fen));
    }
  }

  // The reduced search a null move runs must keep at least one real ply. At
  // zero it is quiescence, which only looks at captures, cannot see a mate two
  // plies away, and answers with the static score - so the pass looks safe and
  // the mating line is pruned. This caught exactly that at depth 4.
  TEST_CASE_FIXTURE(search_fixture_t, "pruning does not hide a forced mate")
  {
    for (int depth = 3; depth <= 6; ++depth) {
      const search_t white = search_fen(MATE_IN_2_W_POS, depth);

      REQUIRE_MESSAGE(white.mate_found,
                      ("white, depth " + std::to_string(depth)));
      REQUIRE_MESSAGE(white.mate_in == 2,
                      ("white, depth " + std::to_string(depth)));

      const search_t black = search_fen(MATE_IN_2_B_POS, depth);

      REQUIRE_MESSAGE(black.mate_found,
                      ("black, depth " + std::to_string(depth)));
      REQUIRE_MESSAGE(black.mate_in == 2,
                      ("black, depth " + std::to_string(depth)));
    }
  }

  // The same hazard from the other side, and the one reverse futility pruning
  // walks into. That rule returns a static score instead of searching whenever
  // the score clears beta by its margin, and a static score is never a mate
  // score - so a node whose true value is "mated in one" can fail high on
  // material and take its whole subtree with it. S033.
  //
  // Three properties make the position bite, and the first two are asserted
  // below rather than assumed:
  //
  //   the mated side is ahead   White leads by 500 cp after the key, which is
  //                             what puts the node above beta
  //   the key is quiet          a checking key would leave White in check,
  //                             where the pruning is already forbidden and the
  //                             case would prove nothing
  //   the mate is inside the pruned depth   two moves, against a depth bound of
  //                             several plies
  //
  // Built by adding White material to MATE_IN_2_B_POS until White led, keeping
  // a mate in two that no checking move also forces. python-chess enumerated it
  // exhaustively and Stockfish at depth 18 agrees: mate 2, key e5e6.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "pruning does not hide a mate against the material leader")
  {
    const std::string root = "4K1R1/q7/5P2/4k3/8/1P6/2P5/1B2N3 b - - 0 1";
    const std::string after_key = "4K1R1/q7/4kP2/8/8/1P6/2P5/1B2N3 w - - 1 2";

    // Precondition. Without these two the case would pass on an engine that
    // never comes near the rule: the node has to be quiet, and its static score
    // has to be high enough for a static cutoff to be possible at all.
    REQUIRE(load_FEN(after_key, &game));
    REQUIRE_FALSE(is_check(&game));
    REQUIRE(evaluate(&game.board) > 300);

    for (int depth = 3; depth <= 6; ++depth) {
      const std::string title = "depth " + std::to_string(depth);
      const search_t result = search_fen(root, depth);

      REQUIRE_MESSAGE(result.mate_found, title);
      REQUIRE_MESSAGE(result.mate_in == 2, title);
    }
  }

  TEST_CASE_FIXTURE(search_fixture_t, "which material can still mate")
  {
    struct case_t
    {
      std::string fen;
      bool dead;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"7k/8/8/8/8/8/8/K7 w - - 0 1",        true,  "king against king"},
      {"7k/8/8/8/8/8/8/KN6 w - - 0 1",       true,  "one knight"},
      {"7k/8/8/8/8/8/8/KB6 w - - 0 1",       true,  "one bishop"},
      {"6nk/8/8/8/8/8/8/KN6 w - - 0 1",      false, "a knight each is not dead by rule"},
      {"7k/8/8/8/8/8/8/KNN5 w - - 0 1",      false, "two knights"},
      {"7k/8/8/8/8/8/8/KR6 w - - 0 1",       false, "a rook mates"},
      {"7k/8/8/8/8/8/8/KQ6 w - - 0 1",       false, "a queen mates"},
      {"7k/8/8/8/8/8/P7/K7 w - - 0 1",       false, "a pawn can promote"},

      // The boundary. is_insufficient_material() is count_bits(minors) <= 1
      // once pawns, rooks and queens are excluded, so it answers false for
      // every two-minor ending, and until S067 the case list had no
      // bishop-against-bishop entry at all and no bishop-against-knight one.
      // Both answers are pinned here in the direction the code takes today.
      //
      // The first of the three is the one where that is not the rule-following
      // answer. Measured, not reasoned about: `python-chess`
      // `Board.is_insufficient_material()` over these three FENs returns
      // **True**, False, False, and its bishops are b1 and g2, both light.
      // src/search.cpp:264 returns DRAW_SCORE off this function at every node
      // above the root, so agreeing with the rule would change what the search
      // scores and is an SPRT, not a test edit. S067 pins the disagreement
      // rather than resolving it. 2026-08-14_test_review-F05.
      {"7k/8/8/8/8/8/6b1/KB6 w - - 0 1",     false, "two bishops, same square colour: chesso says a mate is still possible, python-chess says dead"},
      {"7k/8/8/8/8/8/7b/KB6 w - - 0 1",      false, "two bishops, opposite square colours"},
      {"6nk/8/8/8/8/8/8/KB6 w - - 0 1",      false, "a bishop against a knight"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);
      REQUIRE_MESSAGE(is_insufficient_material(&game.board) == test.dead,
                      test.title);
    }
  }

  // Regression: an illegal FEN where the side to move can capture the enemy
  // king used to leave a side with no king on the board, and is_check() then
  // indexed the attack tables with square 64.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a position with a capturable king is survivable")
  {
    // Black is in check with White to move, which cannot arise in a real game
    // but does arrive through [position fen].
    const search_t result = search_fen("7k/8/8/8/8/8/8/K6R w - - 0 1", 4);

    REQUIRE(result.best_move != 0);
  }

  TEST_CASE_FIXTURE(search_fixture_t, "a board with no kings is survivable")
  {
    const search_t result = search_fen("8/3p4/8/8/8/8/3P4/8 w - - 0 1", 4);

    REQUIRE(result.best_move != 0);
  }

  // A repetition is a property of the path, not of the position, so it has to
  // be seen through the moves played before the search started. The losing
  // side takes the draw over the loss, which is the only way the rule shows
  // up in a score.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "the losing side takes an available repetition")
  {
    REQUIRE(load_FEN("7k/8/8/8/8/8/R7/K7 w - - 0 1", &game));

    // Ra2-b2 Kh8-h7 Rb2-a2 leaves Black to move with Kh7-h8 recreating the
    // position the game started from.
    REQUIRE(play_move(&game, a2, b2));
    REQUIRE(play_move(&game, h8, h7));
    REQUIRE(play_move(&game, b2, a2));

    REQUIRE_EQ(game.board.active_color, BLACK);

    // Black is a rook down and Black is to move, and evaluate() answers from
    // the side to move's point of view, so anything other than the repetition
    // is losing by about that much - the rest is where the tables put the
    // kings and the rook. S065's fit, DEC-059: -491 to -537, and S076's refit
    // on the deduplicated corpus to -569.
    REQUIRE_EQ(evaluate(&game.board), -569);

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const search_t result = search(4, &game, &state);

    REQUIRE_EQ(result.score, 0);
    REQUIRE_EQ(MOVE_FROM(result.best_move), h7);
    REQUIRE_EQ(MOVE_TO(result.best_move), h8);
  }

  TEST_CASE_FIXTURE(search_fixture_t, "the fifty move rule is a draw")
  {
    // Halfmove clock already at 100: every node below the root is a draw.
    const search_t result = search_fen("4k3/8/8/8/8/8/8/3QK3 w - - 100 200", 4);

    REQUIRE_EQ(result.score, 0);
  }
}


// The table's own behaviour, pinned without going through a search. The
// whole-search comparison above can only infer these, and only while the
// search has no reductions in it; these hold whatever the search does.
TEST_SUITE("transposition table: storage")
{
  static board_t board_keyed(hash_t key)
  {
    board_t board = {};
    board.hash = key;
    return board;
  }


  TEST_CASE("a stored entry comes back with every field intact")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    tt_new_search(&table);

    const board_t board = board_keyed(0x0123456789abcdefULL);
    tt_store_entry(&table, &board, 7, -1234, TT_BETA_NODE, 0xabcd);

    const tt_entry_t* entry = tt_get_entry(&table, &board);

    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->key, board.hash);
    REQUIRE_EQ(entry->depth, 7);
    REQUIRE_EQ(entry->score, -1234);
    REQUIRE_EQ(entry->type, TT_BETA_NODE);
    REQUIRE_EQ(entry->best_move, 0xabcd);

    tt_free(&table);
  }


  // Two positions sharing a slot must not be confused for one another. The
  // whole point of storing the key is that a collision reads as a miss, and a
  // probe that skipped the comparison would return a score belonging to an
  // unrelated position.
  TEST_CASE("a colliding key is a miss, not somebody else's entry")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    tt_new_search(&table);

    const board_t stored = board_keyed(0x1000ULL);
    tt_store_entry(&table, &stored, 5, 42, TT_PV_NODE, 0x1111);

    // Same slot by construction: the index is the key masked to the table size.
    const board_t colliding = board_keyed(0x1000ULL + (table.index_mask + 1));

    REQUIRE_EQ(stored.hash & table.index_mask,
               colliding.hash & table.index_mask);
    REQUIRE(tt_get_entry(&table, &colliding) == nullptr);
    REQUIRE(tt_get_entry(&table, &stored) != nullptr);

    tt_free(&table);
  }


  TEST_CASE("within one search the deeper entry keeps the slot")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    tt_new_search(&table);

    const board_t board = board_keyed(0x2000ULL);

    tt_store_entry(&table, &board, 9, 100, TT_PV_NODE, 0x1111);
    tt_store_entry(&table, &board, 4, 200, TT_PV_NODE, 0x2222);

    const tt_entry_t* entry = tt_get_entry(&table, &board);
    REQUIRE(entry != nullptr);
    REQUIRE_MESSAGE(entry->depth == 9, "a shallower result overwrote a deeper");
    REQUIRE_EQ(entry->score, 100);

    // Equal depth is allowed to replace: the later result is the more recent.
    tt_store_entry(&table, &board, 9, 300, TT_PV_NODE, 0x3333);
    REQUIRE_EQ(tt_get_entry(&table, &board)->score, 300);

    tt_free(&table);
  }


  // Across searches the depth rule is dropped, or a deep entry from an early
  // iteration would hold a slot for the rest of the game.
  TEST_CASE("a new search may replace at any depth")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    tt_new_search(&table);

    const board_t board = board_keyed(0x3000ULL);
    tt_store_entry(&table, &board, 12, 100, TT_PV_NODE, 0x1111);

    tt_new_search(&table);
    tt_store_entry(&table, &board, 2, 500, TT_ALPHA_NODE, 0x2222);

    const tt_entry_t* entry = tt_get_entry(&table, &board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->depth, 2);
    REQUIRE_EQ(entry->score, 500);

    tt_free(&table);
  }


  // Generation zero marks a slot that was never written, so the counter must
  // skip it when it wraps - otherwise every entry in the table suddenly looks
  // like it belongs to the current search.
  TEST_CASE("the generation counter never wraps to zero")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    for (int i = 0; i < 600; ++i) {
      tt_new_search(&table);
      REQUIRE_MESSAGE(table.generation != 0, ("after " + std::to_string(i)));
    }

    tt_free(&table);
  }


  // Every caller has to tolerate a table that could not be allocated, because
  // tt_resize() gives up rather than refusing to play.
  TEST_CASE("an unallocated table stores nothing and answers nothing")
  {
    transposition_table_t table = {};
    const board_t board = board_keyed(0x4000ULL);

    tt_store_entry(&table, &board, 5, 100, TT_PV_NODE, 0x1111);
    REQUIRE(tt_get_entry(&table, &board) == nullptr);
  }


  TEST_CASE("reset clears the entries but keeps the allocation")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    tt_new_search(&table);

    const board_t board = board_keyed(0x5000ULL);
    tt_store_entry(&table, &board, 5, 100, TT_PV_NODE, 0x1111);
    REQUIRE(tt_get_entry(&table, &board) != nullptr);

    const size_t entries_before = table.entry_count;
    tt_reset(&table);

    REQUIRE(tt_get_entry(&table, &board) == nullptr);
    REQUIRE_EQ(table.entry_count, entries_before);
    REQUIRE(table.entries != nullptr);

    tt_free(&table);
  }
}


// Static exchange evaluation, checked against positions where the answer can
// be worked out by hand. This is the kind of function that looks right, passes
// a search, and quietly throws away a piece a hundred games later.
TEST_SUITE("search: static exchange evaluation")
{
  static move_t find_move(game_t * g, index_t from, index_t to)
  {
    move_t moves[MAX_MOVES];
    const size_t count = generate_moves(game_tables(), &g->board, moves);

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) == from && MOVE_TO(moves[i]) == to) {
        return moves[i];
      }
    }

    return 0;
  }


  TEST_CASE_FIXTURE(search_fixture_t,
                    "the exchange comes out at the right value")
  {
    struct case_t
    {
      std::string fen;
      index_t from;
      index_t to;
      int value;
      std::string title;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      // Nothing defends the pawn: a clean win of one pawn.
      {"4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1",        e4, d5,  100, "free pawn"},

      // The pawn on c6 recaptures, so it is pawn for pawn.
      {"4k3/8/2p5/3p4/4P3/8/8/4K3 w - - 0 1",      e4, d5,    0, "pawn takes pawn, defended by a pawn"},

      // Queen takes a defended pawn: wins 100, loses a 900 queen.
      {"4k3/8/2p5/3p4/8/8/8/3QK3 w - - 0 1",       d1, d5, -800, "queen grabs a pawn a pawn defends"},

      // Rook takes an undefended rook.
      {"4k3/8/8/3r4/8/8/8/3RK3 w - - 0 1",         d1, d5,  500, "rook takes a loose rook"},

      // Rook takes a rook that a pawn defends: 500 for a 500 rook, then the
      // pawn takes, so the exchange is a rook down.
      {"4k3/8/2p5/3r4/8/8/8/3RK3 w - - 0 1",       d1, d5,    0, "rook takes rook, pawn recaptures"},

      // Both sides pile on: pawn takes pawn, knight recaptures, bishop takes
      // the knight, and nothing is left to answer.
      {"4k3/8/2n5/3p4/4P3/5B2/8/4K3 w - - 0 1",    e4, d5,  100, "the exchange runs and White comes out ahead"},
    };
    // clang-format on

    for (const case_t& test : cases) {
      REQUIRE_MESSAGE(load_FEN(test.fen, &game), test.title);

      const move_t move = find_move(&game, test.from, test.to);
      REQUIRE_MESSAGE(move != 0, (test.title + ": move is not legal here"));

      const int value = see(&game.board, move);
      REQUIRE_MESSAGE(value == test.value,
                      (test.title + ": got " + std::to_string(value) +
                       " expected " + std::to_string(test.value)));
    }
  }


  // A quiet move takes nothing, so the exchange starts from zero and can only
  // be negative if the destination is attacked.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a quiet move into an attack loses material")
  {
    REQUIRE(load_FEN("4k3/8/2p5/8/8/8/8/3QK3 w - - 0 1", &game));

    const move_t safe = find_move(&game, d1, d4);
    REQUIRE(safe != 0);
    REQUIRE_EQ(see(&game.board, safe), 0);

    // d5 is attacked by the pawn on c6 and defended by nothing.
    const move_t hangs = find_move(&game, d1, d5);
    REQUIRE(hangs != 0);
    REQUIRE_MESSAGE(
        see(&game.board, hangs) < 0,
        "moving a queen onto a square a pawn covers must be losing");
  }
}


// see_ge() answers the comparison that see() would, but stops early. The two
// are separate implementations of the same idea, so the only way to trust the
// fast one is to check it against the exact one everywhere the search could
// call it.
TEST_SUITE("search: see_ge agrees with see")
{
  TEST_CASE_FIXTURE(search_fixture_t,
                    "over every legal move of every test position")
  {
    const std::vector<int> thresholds = {-900, -500, -100, -1, 0, 1, 100, 500};
    size_t checked = 0;

    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));

      move_t moves[MAX_MOVES];
      const size_t count = generate_moves(game_tables(), &game.board, moves);

      for (size_t i = 0; i < count; ++i) {
        const int exact = see(&game.board, moves[i]);

        for (const int threshold : thresholds) {
          const bool fast = see_ge(&game.board, moves[i], threshold);

          REQUIRE_MESSAGE(fast == (exact >= threshold),
                          ("FEN: " + fen + " move " + print_move(moves[i]) +
                           " see " + std::to_string(exact) + " threshold " +
                           std::to_string(threshold) + " see_ge said " +
                           (fast ? "yes" : "no")));
          checked++;
        }
      }
    }

    REQUIRE(checked > 10000);
  }
}


TEST_SUITE("search: windowed root")
{
  // Regression, S021. search() now takes a window, so the root can fail high --
  // with beta = MAX it never could. The cutoff `break` in negamax() jumps out
  // before the block that maintains the principal variation, so a root that
  // failed high published the move that caused the cutoff while the PV row
  // still described whatever earlier move last beat alpha.
  //
  // Found by the Debug build's assert at the bottom of search(), which is the
  // only thing that said so: the step-completion gate runs `ctest -L fast`
  // against the Release build, where the assertion is compiled out. Reproduced
  // there as `best=b1c3 pv0=d2d4 pvlen=5` at depth 5, alpha -32, beta 68,
  // score 68, from `position startpos moves e2e4 e7e5` and `go nodes 20000`.
  //
  // The aspiration loop discards a failed-high result and re-searches, so
  // nothing ever reached a GUI. This pins search()'s postcondition rather than
  // any UCI output: the next caller to keep a windowed result is the one it
  // protects, and the assert cannot protect it in the build the gate runs.
  //
  // The loop below is the engine's own, deliberately: one search_state_t and
  // one warm transposition table across deepening iterations, with a window
  // around the previous iteration's score. A single windowed search cannot
  // produce the shape -- move ordering searches the best root move first, so it
  // causes the cutoff before anything has beaten alpha, and the PV is empty.
  // It takes an ordering that is good but not perfect, which is what a warm
  // table across depths gives.
  //
  // Non-vacuous by construction: the case REQUIREs that the run actually
  // reached a root that failed high while holding a non-empty PV. A run that
  // never reaches it proves nothing and says so.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a root that fails high still starts its own pv")
  {
    // clang-format off
    const std::vector<std::string> fens = {
      DEFAULT_POSITION,
      "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    };
    // clang-format on

    static std::atomic_bool never_stop = false;
    int fail_highs_with_a_pv = 0;

    for (const std::string& fen : fens) {
      REQUIRE(load_FEN(fen, &game));

      never_stop = false;
      tt_reset(&tt);
      tt_new_search(&tt);

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      int previous = 0;

      for (int depth = 1; depth <= 8; ++depth) {
        int alpha = -SEARCH_SCORE_INF;
        int beta = SEARCH_SCORE_INF;

        if (depth >= ASPIRATION_MIN_DEPTH) {
          alpha = previous - ASPIRATION_DELTA;
          beta = previous + ASPIRATION_DELTA;
        }

        const search_t result = search(depth, &game, &state, alpha, beta);

        const std::string title = fen + " depth " + std::to_string(depth) +
                                  " window [" + std::to_string(alpha) + ", " +
                                  std::to_string(beta) + "]";

        if (result.pv.length > 0) {
          REQUIRE_MESSAGE(result.best_move == result.pv.table[0], title);

          if (result.score >= beta) { fail_highs_with_a_pv++; }
        }

        // Only a score inside the window is a score; a bound is not a centre to
        // build the next window around. Same rule the engine follows.
        if (result.score > alpha && result.score < beta) {
          previous = result.score;
        }
      }
    }

    // The precondition. Without it every assertion above is skippable by a run
    // that never failed high at the root at all.
    REQUIRE(fail_highs_with_a_pv > 0);
  }
}
