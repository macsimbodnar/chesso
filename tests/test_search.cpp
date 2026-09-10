#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
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


// GOLDEN (DEC-142): evaluate() and evaluate_cheap() of the quiet rook position
// "4k3/8/8/8/8/8/8/3RK3 w - - 0 1", White to move, at the shipped weights. Nine
// cases in this file assert one or both, so they are named once here and a
// refit edits two lines rather than eleven (S192).
// Re-derive: python3 adocs/data/S192_anchors.py, case "rook on d1", which
// computes both from src/evaluation.cpp's specification rather than from the
// engine -- an anchor copied from the thing it anchors asserts nothing (S028).
// Moves legitimately on: a refit. Margin: exact.
// Property beside it: "the lazy shortcut cannot change a decision" in
// tests/test_evaluation.cpp, which holds over the whole corpus and does not
// move with a fit.
static constexpr int QUIET_ROOK_EVAL = 563;
static constexpr int QUIET_ROOK_EVAL_CHEAP = 567;


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
  TEST_CASE_FIXTURE(search_fixture_t, "the node budget is never exceeded")
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

    // Depth 8 rather than the 6 this ran at before S149. The duplication
    // assertion below is only as good as the number of second slots the search
    // fills: depth 6 fills 3 and duplicates 1 of them, depth 8 fills 5 and
    // duplicates 3, and it still costs 53 ms.
    const search_t result = search(8, &game, &state);
    REQUIRE(result.best_move != 0);

    size_t killers_0 = 0;
    size_t killers_1 = 0;
    size_t killers_1_duplicated = 0;

    for (size_t ply = 0; ply < MAX_PLY; ++ply) {
      if (state.killer_moves[0][ply] != 0) { killers_0++; }

      if (state.killer_moves[1][ply] != 0) {
        killers_1++;

        if (state.killer_moves[1][ply] == state.killer_moves[0][ply]) {
          killers_1_duplicated++;
        }
      }
    }

    size_t history_entries = 0;
    size_t counters = 0;

    for (int piece = W_PAWN; piece <= B_KING; ++piece) {
      for (int square = 0; square < 64; ++square) {
        if (state.counter_moves[piece][square] != 0) { counters++; }
      }
    }

    for (int side = WHITE; side <= BLACK; ++side) {
      for (int from = 0; from < 64; ++from) {
        for (int to = 0; to < 64; ++to) {
          if (state.quiet_history[side][from][to] != 0) { history_entries++; }
        }
      }
    }

    REQUIRE(killers_0 > 0);

    // The second slot only fills once a ply produces a second killer, which
    // is what the shift down from slot 0 is for. This is the precondition for
    // the assertion below and not a property being tested: it says a second
    // killer was written somewhere, so the next line cannot pass on a table
    // that is simply empty.
    REQUIRE(killers_1 > 0);

    // AND THE SECOND SLOT IS OFTEN A COPY OF THE FIRST, ON PURPOSE. This is a
    // fence, not an endorsement, and it is here so the next agent to notice the
    // duplication finds the measurement instead of repeating the night that
    // produced it.
    //
    // The shift in negamax is unguarded, so a quiet that fails high twice at
    // one ply copies slot 0 onto itself. Killers survive every iteration of one
    // `go`, so the repeat is the common case: 2026-08-21_adversarial-F01
    // counted 351422 of 532133 stores, 66.0 %, leaving both slots equal on
    // 5115505 of 11531069 negamax nodes, 44.4 %. score_move tests slot 0 first,
    // so on those nodes no distinct move can reach ORDER_KILLER_1 at all.
    //
    // CPW's Killer Heuristic replacement rule says the slots ought to hold
    // different moves. S149 implemented exactly that -- two lines, guarding the
    // shift -- and measured it: -11.02 +/- 10.53 Elo, nElo -14.21, over 2522
    // games in 1 h 05 m against ac4c588, LLR -2.97 at [-5, 5], H0 accepted, 0
    // forfeits. It cost about 3 % of the nodes and 11 Elo. The published rule
    // does not transfer to this search (DEC-019), so the duplication is kept
    // and the guard is not. Change this and the number is what you must beat.
    REQUIRE(killers_1_duplicated > 0);
    REQUIRE(history_entries > 0);
    REQUIRE(counters > 0);
  }

  // A quiet move that gives check was excluded from all three tables until
  // S107 -- the same class of quiet the engine's own late move reduction
  // refuses to reduce, on the grounds that those lines are forcing. Driven at
  // a node rather than through search() because a countermove needs a non-zero
  // prev_move and search() passes 0 at the root.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a quiet move that gives check enters the ordering tables")
  {
    // Smothered mate: Black's own rook and pawns take every flight square, so
    // the knight's quiet check is mate. Nf7 is the only mate in one here and
    // it is not a capture -- from a tool, not from the board (CLAUDE.md).
    // python-chess over the legal moves reported the whole mating set as
    // `[('Nf7#', 'h6f7', False)]`, and /usr/games/stockfish on the position
    // after `h6f7` answers `info depth 0 score mate 0` / `bestmove (none)`.
    const std::string fen = "6rk/6pp/7N/8/8/8/8/6K1 w - - 0 1";

    // The move that reached the position, so the countermove slot has an index
    // to be written under. python-chess: legal in
    // "5r1k/6pp/7N/8/8/8/8/6K1 b - - 0 1" and it lands on the FEN above.
    const move_t prev_move = NEW_MOVE(f8, g8, B_ROOK, 0, 0, 0, 0, 0);

    // Only a mate score clears this bound, so the move that fails high is the
    // mate rather than whichever move the ordering tried first -- the case
    // does not depend on move order and later ordering steps cannot move it.
    // It also sits above search.cpp's MATE_MIN, which is what switches reverse
    // futility and null move pruning off: neither may answer for this node.
    constexpr int BETA = 48500;
    constexpr int DEPTH = 1;
    constexpr size_t PLY = 1;

    REQUIRE(load_FEN(fen, &game));
    REQUIRE(position_is_reachable(&game));

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const int score =
        negamax(BETA - 1, BETA, DEPTH, PLY, &game, &state, prev_move, false);

    // Precondition, not the property under test: with no fail-high there is no
    // update site at all, and every assertion below would pass or fail for a
    // reason that has nothing to do with the check condition.
    REQUIRE(score >= BETA);

    const move_t killer = state.killer_moves[0][PLY];
    REQUIRE(killer != 0);

    // What the move is, rather than a literal it equals, so the case still
    // states its own subject.
    REQUIRE_FALSE(MOVE_CAPTURE(killer));
    REQUIRE(MOVE_PROMOTED(killer) == TO_NONE);

    REQUIRE(make_move(&game, killer));
    const bool gives_check = is_check(&game);
    unmake_move(&game);
    REQUIRE(gives_check);

    // Presence, never magnitude or formula: S093 replaced the bonus, the
    // indexing and the ageing, and only the index moved here.
    REQUIRE(state.quiet_history[game.board.active_color][MOVE_FROM(killer)]
                               [MOVE_TO(killer)] != 0);
    REQUIRE_EQ(state.counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)],
               killer);
  }


  // S093. Gravity is what bounds the table now. The `std::min` saturation the
  // old accumulator needed is gone, so the closed interval the ordering band
  // rests on is a property of the update itself and not of a clamp at one call
  // site. CLAUDE.md lists that band as a one-way door -- the symptom of getting
  // it wrong is a strength regression, not a wrong node count -- so the entry
  // is driven to each asymptote through the helper rather than assigned there.
  // An assertion on a value the case wrote itself would pass on an update that
  // cannot reach it.
  TEST_CASE("gravity holds a history entry inside the bound at both asymptotes")
  {
    const int max = QUIET_HISTORY_MAX;
    REQUIRE(max > 0);

    // The precondition, non-vacuous by construction: a bonus small against the
    // bound, applied often enough that the accumulator this replaced would be
    // far outside it. 5000 cutoffs at depth 12 sum to 720000, 87 times the
    // shipping bound.
    const int bonus = 12 * 12;
    REQUIRE(bonus * 5000 > 8 * max);

    int16_t rising = 0;

    for (int i = 0; i < 5000; ++i) {
      history_gravity_update(rising, bonus);
      REQUIRE(rising <= max);
      REQUIRE(rising >= -max);
    }

    // Converged, not merely bounded. `entry + b - entry*b/MAX` has its fixed
    // point at exactly MAX and integer truncation does not stop it arriving,
    // so an asymptote that came out short would mean the decay term is being
    // over-applied and the band is narrower than it is declared to be.
    CHECK_EQ(rising, max);

    int16_t falling = 0;

    for (int i = 0; i < 5000; ++i) {
      history_gravity_update(falling, -bonus);
      REQUIRE(falling >= -max);
      REQUIRE(falling <= max);
    }

    CHECK_EQ(falling, -max);

    // The clamp is what closes the interval. Without it one deep cutoff puts
    // an entry outside the band in a single update.
    int16_t clamped = 0;
    history_gravity_update(clamped, 100 * max);
    CHECK_EQ(clamped, max);

    // The two properties CPW states, which are one piece of algebra read
    // twice: an unexpected cutoff moves the entry by the whole bonus, an
    // expected one by nothing at all.
    int16_t cold = 0;
    history_gravity_update(cold, bonus);
    CHECK_EQ(cold, bonus);

    int16_t hot = static_cast<int16_t>(max);
    history_gravity_update(hot, bonus);
    CHECK_EQ(hot, max);
  }


  // S093. What the malus is charged to, held directly against the table rather
  // than inferred from a game -- which is what this step's accepts demands.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a quiet cutoff maluses the quiets tried before it")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t quiets[4] = {};
    size_t quiet_count = 0;

    for (size_t i = 0; i < count && quiet_count < 4; ++i) {
      if (!MOVE_CAPTURE(moves[i]) && MOVE_PROMOTED(moves[i]) == TO_NONE) {
        quiets[quiet_count++] = moves[i];
      }
    }

    REQUIRE_EQ(quiet_count, 4);

    const color_t side = game.board.active_color;

    // Precondition: four distinct butterfly cells. Two quiets sharing a
    // from-to pair share an entry, and the bonus and the malus would then be
    // arguing over one number rather than over four.
    for (size_t a = 0; a < 4; ++a) {
      for (size_t b = a + 1; b < 4; ++b) {
        const bool same_cell = MOVE_FROM(quiets[a]) == MOVE_FROM(quiets[b]) &&
                               MOVE_TO(quiets[a]) == MOVE_TO(quiets[b]);
        REQUIRE_FALSE(same_cell);
      }
    }

    search_state_t state = {};

    const move_t cutoff = quiets[0];
    const move_t tried[3] = {quiets[1], quiets[2], quiets[3]};

    history_on_quiet_cutoff(&state, side, cutoff, tried, 3, 8);

    CHECK(state.quiet_history[side][MOVE_FROM(cutoff)][MOVE_TO(cutoff)] > 0);

    // Every quiet tried before the cutoff, not all but the last one. Lynx
    // shipped exactly that off-by-one -- sparing the last tried quiet to
    // protect a cutoff move that was never in the span -- and removing it
    // measured +12.89 +/- 5.46 on its own (PR #1756).
    for (const move_t move : tried) {
      CHECK_MESSAGE(
          state.quiet_history[side][MOVE_FROM(move)][MOVE_TO(move)] < 0,
          ("A quiet tried before the cutoff scores " +
           std::to_string(
               state.quiet_history[side][MOVE_FROM(move)][MOVE_TO(move)]) +
           " and not a malus."));
    }

    // The mover indexes the table, so the other colour's half of the same
    // from-to pairs is untouched. A butterfly board that dropped the colour
    // axis would let White's cutoffs order Black's moves.
    CHECK_EQ(state.quiet_history[!side][MOVE_FROM(cutoff)][MOVE_TO(cutoff)], 0);
  }


  // S093. The malus span is built at the call site and not by the helper, so
  // the case above cannot see the two things the call site decides: which moves
  // enter the span, and whether the move that cut off is one of them. Driven
  // through negamax for that reason.
  //
  // Lynx's other published bug in this list does not exist here and is
  // deliberately not asserted. It charged the malus to moves whose make_move
  // had failed (PR #610), which needs a pseudo-legal generator; chesso's
  // generate_moves() emits legal moves only (src/bitboard.cpp:885) and
  // make_move refuses only on a full game-history stack, which the search
  // cannot reach. Building the span after a successful make_move is still the
  // right shape and costs nothing, but a case asserting that no illegal move
  // was malused would hold over an empty set and is not written.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "the cutoff move is credited and the quiets before it are "
                    "charged")
  {
    // Nf7 is mate, so the move that fails high is fixed and no later ordering
    // change can move it. From a tool and not from the board (CLAUDE.md):
    // python-chess over this FEN reports `is_valid() True`, `is_check() False`,
    // and the whole mating set as `[('Nf7#', 'h6f7')]`. The knight on g8 is
    // takeable, which is the second thing the position is for -- a capture is
    // ordered ahead of every quiet and is tried before the cutoff without ever
    // being eligible for the table.
    const std::string fen = "6rk/b5pp/7N/8/3N4/8/8/6K1 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    REQUIRE(position_is_reachable(&game));

    move_t captures[MAX_MOVES];
    const size_t captures_count =
        generate_captures(game_tables(), &game.board, captures);

    // Precondition. Without a capture the node searches before the cutoff, the
    // eligibility assertion below holds over an empty set.
    REQUIRE_EQ(captures_count, 1);
    REQUIRE_EQ(MOVE_FROM(captures[0]), h6);
    REQUIRE_EQ(MOVE_TO(captures[0]), g8);

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    move_t king_to_g2 = 0;
    move_t king_to_h2 = 0;

    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) != g1) { continue; }
      if (MOVE_TO(moves[i]) == g2) { king_to_g2 = moves[i]; }
      if (MOVE_TO(moves[i]) == h2) { king_to_h2 = moves[i]; }
    }

    REQUIRE(king_to_g2 != 0);
    REQUIRE(king_to_h2 != 0);

    // Only a mate score clears this bound, so the move that fails high is the
    // mate and not whichever move the ordering happened to try first.
    constexpr int BETA = 48500;
    constexpr int DEPTH = 1;
    constexpr size_t PLY = 1;

    static std::atomic_bool never_stop = false;
    never_stop = false;

    tt_reset(&tt);
    tt_new_search(&tt);

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    // The second precondition, and the reason this is not the S107 position.
    // Nf7 is the first quiet the generator emits, so with a cold table it is
    // also the first quiet searched and the malus span would be empty. Two
    // killers put two king moves in front of it; neither can cut off, because
    // neither is mate.
    state.killer_moves[0][PLY] = king_to_g2;
    state.killer_moves[1][PLY] = king_to_h2;

    const int score =
        negamax(BETA - 1, BETA, DEPTH, PLY, &game, &state, 0, false);

    // Precondition: with no fail-high there is no update site at all.
    REQUIRE(score >= BETA);

    const move_t cutoff = state.killer_moves[0][PLY];
    REQUIRE(cutoff != 0);
    REQUIRE_EQ(MOVE_FROM(cutoff), h6);
    REQUIRE_EQ(MOVE_TO(cutoff), f7);

    // The cutoff move is credited and never charged. It is searched after two
    // quiets that were, so an implementation that appended to the span before
    // the cutoff test rather than after it would credit and charge this one
    // cell and land it at or below zero.
    CHECK(state.quiet_history[WHITE][h6][f7] > 0);

    // The two quiets tried before it, both of them. Lynx spared the last tried
    // quiet to protect a cutoff move that was never in the span, and removing
    // that off-by-one measured +12.89 +/- 5.46 on its own (PR #1756).
    CHECK(state.quiet_history[WHITE][g1][g2] < 0);
    CHECK(state.quiet_history[WHITE][g1][h2] < 0);

    // The capture searched before both of them earns neither. The span mirrors
    // the bonus gate exactly -- `!is_capture` and nothing else -- because an
    // asymmetry between what can earn the bonus and what can earn the malus is
    // a bias with no symptom.
    CHECK_EQ(state.quiet_history[WHITE][h6][g8], 0);

    // Every child of this node is quiescence, which writes no history, so the
    // whole table belongs to the side that moved here. A butterfly board that
    // dropped the colour axis would fail here.
    size_t black_entries = 0;

    for (int from = 0; from < 64; ++from) {
      for (int to = 0; to < 64; ++to) {
        if (state.quiet_history[BLACK][from][to] != 0) { black_entries++; }
      }
    }

    CHECK_EQ(black_entries, 0);
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
  //
  // GOLDEN (DEC-142): the pair 440000 and 20000, a band around the depth-5
  // node count of TRICKY_POS from a cold table -- 109575 when measured
  // 2026-08-14, held inside [count / 5, 4 x count]. Both ratios are the band
  // this case has always carried and neither is a new constant (DEC-105 (b)).
  // Re-taken 2026-09-10 by S192: **179851 nodes**, so the tree has grown 64 %
  // under a band that did not move and the budget is 2.4x the count rather than
  // 4x. Still inside the middle half of the band, which is the condition on
  // leaving both numbers alone; the next reading outside it re-derives them.
  // Re-derive: python3 adocs/data/S192_node_budget.py, which runs this case
  // through build/tests/test_search --success, reads the count off the MESSAGE
  // below and prints both bounds by those ratios.
  // Moves legitimately on: any ordering or search change -- re-derive when the
  // count leaves the middle half of the band, and never widen the budget to
  // clear a red without taking the count again.
  // Margin: the budget is 4x the count and the floor a fifth of it, so the
  // count has to move by either factor before anything fires.
  // An in-process search(5, ...) on a cold table
  // with no aspiration is what is counted; a UCI `go depth 5` of the same FEN
  // gives a different number and is not this golden.
  // Property beside it: "a search fills the ordering tables" and the rest of
  // this suite, which assert what ordering does rather than what it costs.
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

    // What S192_node_budget.py reads. Printed rather than returned because the
    // re-derivation runs the binary and does not link against it.
    MESSAGE("ordering node count: " << result.explored_nodes);

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
    // 567. Both are the file-scope goldens above.
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);
    REQUIRE_EQ(cheap_score, QUIET_ROOK_EVAL_CHEAP);

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
  //
  // GOLDEN (DEC-142): 198, evaluate() of "4R1k1/5ppp/8/8/q7/8/8/4R1K1 b - - 0
  // 1" with Black to move and in check -- the number the case needs a stand pat
  // to be compared against. Re-derive: python3 adocs/data/S192_anchors.py, case
  // "black in check, Re8". Moves legitimately on: a refit. Margin: exact -- the
  // assertions that carry the property are the two inequalities below it, which
  // have the whole gap between 198 and a mate score of slack. Property beside
  // it: "a quiet evasion is a legal answer to a check".
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
  //
  // GOLDEN (DEC-142): -505, quiescence() of "4rk2/8/8/8/8/8/8/4K3 w - - 0 1" in
  // check -- a one-ply negamax, the best of four leaves' -evaluate(), not an
  // evaluation call.
  // Re-derive: python3 adocs/data/S192_anchors.py, LEAVES and QUIESCE_IN_CHECK.
  // Moves legitimately on: a refit; **and** any change to how quiescence treats
  // a checked side. That second end is the property under test, so before
  // re-deriving after a search change, confirm the four leaves are still the
  // four king moves each standing pat -- if they are not, the case has caught
  // something and the number is not the thing to fix.
  // Margin: exact. Property beside it: the loop inside this case, which asserts
  // that no legal reply here is a capture, and "a side in check may not stand
  // pat".
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


// The rule that lets quiescence share one table with the main search: an
// entry written by quiescence is stored below every depth the main search can
// ask for, so it answers quiescence and nothing else. S094.
TEST_SUITE("search: quiescence transposition entries")
{
  // MATE_MAX and MATE_MIN are search.cpp's, not exported, and the numbers are
  // pinned here rather than shared so that a change to either is a visible
  // disagreement instead of a silent agreement.
  static constexpr int MATE_MAX_LOCAL = 49000;
  static constexpr int MATE_MIN_LOCAL = 48000;

  TEST_CASE("a main-search node at depth 1 does not cut on a quiescence entry")
  {
    tt_entry_t entry = {};
    entry.key = 0x1234ULL;
    entry.type = TT_PV_NODE;
    entry.score = 321;
    entry.generation = 1;

    int score = 0;

    // Precondition, and the whole reason the rest is not vacuous: the same
    // score in an entry the main search did write answers a depth 1 node. A
    // probe that answered nothing at all would satisfy the assertions below
    // without the depth rule existing.
    entry.depth = 1;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 321);

    // The same entry, stored the way quiescence stores it. Depth 1 and depth
    // 0 are both main-search nodes: negamax probes before it decides to fall
    // into quiescence, so depth 0 is a probe the main search really makes.
    entry.depth = TT_DEPTH_QS;
    score = 0;
    REQUIRE_FALSE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_FALSE(tt_entry_answers(&entry, 0, 3, -100, 100, &score));
    REQUIRE_EQ(score, 0);

    // And quiescence, which asks for TT_DEPTH_QS, still reads it.
    REQUIRE(tt_entry_answers(&entry, TT_DEPTH_QS, 3, -100, 100, &score));
    REQUIRE_EQ(score, 321);
  }


  // Regression, S094. A mate with no legal reply is stored normalised as
  // exactly -MATE_MAX, "mated here", and the de-normalisation that turns it
  // back into a distance excluded that endpoint. Nothing reached it before
  // quiescence started storing, because the main search returns from a mated
  // node without storing anything.
  TEST_CASE("a mate stored at its own position reads back at the right ply")
  {
    tt_entry_t entry = {};
    entry.key = 0x2345ULL;
    entry.type = TT_PV_NODE;
    entry.depth = TT_DEPTH_QS;
    entry.generation = 1;

    // Precondition: one ply short of the endpoint is adjusted, so the case is
    // about the endpoint and not about de-normalisation being absent.
    entry.score = -(MATE_MAX_LOCAL - 1);
    int score = 0;
    REQUIRE(tt_entry_answers(&entry, TT_DEPTH_QS, 3, -100000, 100000, &score));
    REQUIRE_EQ(score, -(MATE_MAX_LOCAL - 1) + 3);

    entry.score = -MATE_MAX_LOCAL;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, TT_DEPTH_QS, 3, -100000, 100000, &score));
    REQUIRE_EQ(score, -MATE_MAX_LOCAL + 3);

    entry.score = MATE_MAX_LOCAL;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, TT_DEPTH_QS, 3, -100000, 100000, &score));
    REQUIRE_EQ(score, MATE_MAX_LOCAL - 3);
  }


  // The entry carries the static evaluation the node was scored with, and
  // carries it only where that number is the score rather than the bound the
  // lazy shortcut returns in its place. A bound is true on one side of one
  // window; the entry outlives the window. S094.
  TEST_CASE_FIXTURE(
      search_fixture_t,
      "a quiescence entry carries the static score, never a bound")
  {
    // White is a rook up with nothing to capture, so quiescence stands pat and
    // the number stored is the static evaluation of this position and nothing
    // else. The two anchors are the file-scope goldens, named there with the
    // script that re-derives them from the specification rather than reading
    // them off the engine.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    const int cheap_score = evaluate_cheap(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);
    REQUIRE_EQ(cheap_score, QUIET_ROOK_EVAL_CHEAP);

    static std::atomic_bool never_stop = false;

    auto run = [&](int alpha, int beta) -> const tt_entry_t* {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;
      tt_reset(&tt);

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      quiescence(alpha, beta, 0, 0, &game, &state);

      return tt_get_entry(&tt, &game.board);
    };

    // A window wide enough that the shortcut cannot fire: the score is the
    // score, and it is what the entry holds.
    const tt_entry_t* wide = run(-10000, 10000);
    REQUIRE(wide != nullptr);
    REQUIRE_EQ(wide->eval, static_score);

    // The precondition for the other half, and without it the assertion below
    // would pass on a build where the shortcut never fires at all.
    REQUIRE(cheap_score - LAZY_EVAL_MARGIN >= 100);

    // Beta far enough below that the expensive terms are never computed. What
    // quiescence returned is a lower bound and not this position's score, so
    // there is nothing here worth recording.
    const tt_entry_t* narrow = run(0, 100);
    REQUIRE(narrow != nullptr);
    REQUIRE_EQ(narrow->type, TT_BETA_NODE);
    REQUIRE_EQ(narrow->eval, TT_EVAL_NONE);
  }


  // And the field is read rather than only written: quiescence stands pat on
  // the number the entry carries instead of computing one of its own. S094.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "quiescence stands pat on the stored static score")
  {
    // Nothing to capture, so the value quiescence returns is the stand pat and
    // nothing else. Same anchor as the case above.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    auto run = [&]() -> int {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      return quiescence(-10000, 10000, 0, 0, &game, &state);
    };

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    // Precondition: with nothing in the table quiescence works the number out
    // for itself. Without this the assertion below would pass on an engine
    // that answered 363 for some unrelated reason.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE_EQ(run(), static_score);

    // Now an entry for this position carrying a static evaluation that is not
    // this position's. The stored *score* is different again and has to be
    // inert twice over, so that anything but 563 coming back has to have come
    // from the eval field: a lower bound of -9999 does not answer the node
    // against a beta of 10000, and it may not raise a stand pat of 563 either.
    //
    // It was an upper bound of -9999 until S130, which is inert on the first
    // count and not on the second: an upper bound below the static score is
    // exactly what the new substitution consumes, and this case answered -9999
    // instead of 363. Re-targeted rather than relaxed -- the assertion is the
    // same one, on an entry that still isolates the eval field.
    tt_reset(&tt);
    tt_new_search(&tt);

    REQUIRE(load_FEN(fen, &game));
    const int planted = static_score - 200;
    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, -9999, TT_BETA_NODE, 0,
                   planted);

    REQUIRE_EQ(run(), planted);
  }


  // The score field is read too, and only in the direction its bound
  // certifies. A lower bound says the value is at least this, so it may raise
  // the stand pat and never lower it; an upper bound says at most, so it may
  // lower it and never raise it; an exact score is the value. Any other use
  // consumes a claim the entry never made. S130.
  //
  // The two cases that hold the wrong direction shut are the point of the
  // case, not padding: an inverted comparison leaves every node count and
  // every other suite green and leaks nothing but strength.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "quiescence stands pat on the stored score where the bound "
                    "allows it")
  {
    // Nothing to capture and not in check, so what quiescence returns is the
    // stand pat and nothing else. Same anchor as the cases above.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    // Planted after the wipe, which is why this is not the suite's quiesce()
    // helper: that one clears the table on the way in.
    auto plant = [&](int score, node_type_t type) {
      tt_reset(&tt);
      tt_new_search(&tt);
      REQUIRE(load_FEN(fen, &game));
      tt_store_entry(&tt, &game.board, TT_DEPTH_QS, score, type, 0,
                     TT_EVAL_NONE);
    };

    auto run = [&](int alpha, int beta) -> int {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      return quiescence(alpha, beta, 0, 0, &game, &state);
    };

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    // Precondition for every case below. With nothing planted the node works
    // the number out for itself, so any other answer came from the entry and
    // not from some unrelated property of the position.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE_EQ(run(-10000, 10000), static_score);

    // A lower bound raises it. 600 is below beta, so the entry does not answer
    // the node outright and the number has to arrive through the stand pat.
    plant(600, TT_BETA_NODE);
    CHECK_EQ(run(-10000, 10000), 600);

    // And the raised number did not leak into the entry's static field. That
    // field is this position's static score; an improved stand pat is not one,
    // because it exists only where a bound held against this node's window and
    // the entry outlives the window. S094's semantics, unchanged by S130.
    const tt_entry_t* raised = tt_get_entry(&tt, &game.board);
    REQUIRE(raised != nullptr);
    CHECK_EQ(raised->score, 600);
    CHECK_EQ(raised->eval, static_score);

    // And never the other way. A lower bound of 500 says the value is at least
    // 500, which is no reason to believe it is only 500.
    plant(500, TT_BETA_NODE);
    CHECK_EQ(run(-10000, 10000), static_score);

    // An upper bound lowers it. alpha is 0, so 500 is above it and the entry
    // again does not answer the node outright.
    plant(500, TT_ALPHA_NODE);
    CHECK_EQ(run(0, 10000), 500);

    // And never the other way.
    plant(600, TT_ALPHA_NODE);
    CHECK_EQ(run(0, 10000), static_score);

    // An exact score is the value and replaces it. Today the probe answers
    // this node before a stand pat is ever computed, so the assertion is on
    // what comes back and not on which path served it -- it holds unchanged if
    // a later PV guard sends an exact entry down the stand-pat path instead.
    plant(600, TT_PV_NODE);
    CHECK_EQ(run(-10000, 10000), 600);
  }


  // A stand pat is a positional claim, and the fail-soft paths below it hand
  // the number to the parent and store it. A mate distance entering there is a
  // mate no search ever found: this project's recurring bug, seen from the
  // side that invents one rather than the side that hides one. S130.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a mate score is never used as a stand pat")
  {
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    auto plant = [&](int score, node_type_t type) {
      tt_reset(&tt);
      tt_new_search(&tt);
      REQUIRE(load_FEN(fen, &game));
      tt_store_entry(&tt, &game.board, TT_DEPTH_QS, score, type, 0,
                     TT_EVAL_NONE);
    };

    auto run = [&]() -> int {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      // Wide enough that neither planted score answers the node outright, so
      // each one reaches the stand pat and the band exclusion is the only
      // thing that can stop it.
      return quiescence(-100000, 100000, 0, 0, &game, &state);
    };

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    // A mating lower bound would raise the stand pat all the way to it, and
    // this node would report a mate it never searched for. Without the band
    // exclusion the answer here is 48995.
    plant(MATE_MAX_LOCAL - 5, TT_BETA_NODE);
    const int mating = run();
    CHECK_EQ(mating, static_score);
    CHECK(mating < MATE_MIN_LOCAL);

    // And the mirror: a mated upper bound would drag it down to -48995.
    plant(-(MATE_MAX_LOCAL - 5), TT_ALPHA_NODE);
    const int mated = run();
    CHECK_EQ(mated, static_score);
    CHECK(mated > -MATE_MIN_LOCAL);
  }


  // A node must never store a claim stronger than the weakest thing that
  // produced its value. Consuming a bound as the stand pat puts that bound
  // into the maximum the node takes, and the entry it writes has to carry the
  // bound out again -- otherwise a lower bound that only says `value >= s` is
  // written back as `value == s`, at the same key, over the entry that
  // certified it. S130, found by the coordinator before the first SPRT
  // finished, DEC-102.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a substituted stand pat is stored as the bound it is")
  {
    // Nothing to capture and not in check, so the value the node stores is the
    // stand pat and nothing else. Same anchor as the cases above.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    auto plant = [&](int score, node_type_t type) {
      tt_reset(&tt);
      tt_new_search(&tt);
      REQUIRE(load_FEN(fen, &game));
      tt_store_entry(&tt, &game.board, TT_DEPTH_QS, score, type, 0,
                     TT_EVAL_NONE);
    };

    auto run = [&](int alpha, int beta) -> const tt_entry_t* {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      quiescence(alpha, beta, 0, 0, &game, &state);

      return tt_get_entry(&tt, &game.board);
    };

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    // Precondition, and the whole reason the two assertions below are not
    // vacuous: with nothing planted this node stores its stand pat **exact**,
    // and that is correct -- the value of a quiescence node no capture
    // improves is the static score, by quiescence's own definition. So the
    // cases below are about the substitution and not about this site having
    // stopped storing exact scores.
    tt_reset(&tt);
    tt_new_search(&tt);
    const tt_entry_t* plain = run(-10000, 10000);
    REQUIRE(plain != nullptr);
    REQUIRE_EQ(plain->score, static_score);
    REQUIRE_EQ(plain->type, TT_PV_NODE);

    // A lower bound raised the stand pat, so all the node knows is that the
    // value is at least 600. Storing exact would replace the very entry that
    // certified `>= 600` with a flatter claim, at the same key and the same
    // depth, where every later quiescence probe answers from it outright.
    plant(600, TT_BETA_NODE);
    const tt_entry_t* raised = run(-10000, 10000);
    REQUIRE(raised != nullptr);
    CHECK_EQ(raised->score, 600);
    CHECK_EQ(raised->type, TT_BETA_NODE);

    // The mirror, and this one is unconditional rather than incidental. An
    // upper-bound entry only reaches the stand pat when its score is above
    // alpha -- `tt_entry_answers()` would have answered the node otherwise --
    // so the value it caps the stand pat to is always above `alpha0`, and the
    // store's `best_value > alpha0` test therefore always chose exact.
    plant(500, TT_ALPHA_NODE);
    const tt_entry_t* capped = run(0, 10000);
    REQUIRE(capped != nullptr);
    CHECK_EQ(capped->score, 500);
    CHECK_EQ(capped->type, TT_ALPHA_NODE);
  }


  // The third arm, and the one that is not symmetric. When a searched move
  // beats the substituted stand pat, the stand pat drops out of the maximum --
  // but only a *raised* one leaves nothing behind. A *lowered* one displaced a
  // static score that was higher than it, and that static score may be higher
  // than the winning move too, so the node's value is only bounded below.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a capped stand pat beaten by a capture is still a bound")
  {
    // One capture available, and it is what the engine plays here at depth 6.
    // The case asserts the shape of the store and never the worth of the move:
    // that a move produced the value, that it beat the planted cap, and what
    // the node is therefore allowed to claim.
    const std::string fen = "4k3/8/8/8/3q4/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);

    // Below the static score, so it caps, and above alpha, so the entry does
    // not answer the node outright. Derived from the position rather than
    // pinned, since nothing here depends on what the number is.
    const int planted = static_score - 1000;

    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE(load_FEN(fen, &game));
    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, planted, TT_ALPHA_NODE, 0,
                   TT_EVAL_NONE);

    never_stop = false;
    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;
    quiescence(-100000, 100000, 0, 0, &game, &state);

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);

    // Two preconditions. A move produced the value -- the store records one
    // only where a move beat the stand pat -- and that value is above the cap,
    // so the node really did take the maximum over a searched line rather than
    // over the planted number.
    REQUIRE_NE(entry->best_move, 0);
    REQUIRE(entry->score > planted);

    // The static score the cap displaced may still be higher than the line
    // that won, so the value is a lower bound and not the node's value.
    CHECK_EQ(entry->type, TT_BETA_NODE);
  }


  // Every case above searches a window wide enough that the lazy shortcut
  // cannot fire, so `static_eval` is the exact 563 in all of them and the
  // substitution is only ever measured against a static score. It is not
  // always one: where the probe carries no eval and the cheap score is already
  // a margin clear of the window, evaluate_lazy() returns `cheap +/-
  // LAZY_EVAL_MARGIN`, a bound. The audit verified the soundness argument for
  // substituting against that bound case by case and found nothing wrong with
  // it -- what it found was that no test reaches the input class, so a future
  // edit that reorders the lazy call and the substitution, or flips one
  // comparison on the bound path, stays green. The S106 failure class.
  // 2026-08-22_adversarial-F07, S164.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a stand pat that is itself a bound is still capped")
  {
    // Nothing to capture and not in check, so the value the node returns is
    // the stand pat and nothing else. Same anchor as the cases above.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    // Narrow, and on the beta side: cheap is 567, so 567 - 184 = 383 is
    // already clear of a beta of 100 and the shortcut fires with a lower
    // bound. That is the whole point of the window -- every case above uses
    // (-10000, 10000), where neither test in evaluate_lazy() can pass.
    constexpr int ALPHA = 0;
    constexpr int BETA = 100;

    // Above alpha, so tt_entry_answers() does not answer the node outright and
    // the number has to arrive through the stand pat; below the lazy bound, so
    // the cap fires.
    constexpr int PLANTED = 200;

    auto run = [&]() -> int {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      return quiescence(ALPHA, BETA, 0, 0, &game, &state);
    };

    REQUIRE(load_FEN(fen, &game));

    // The preconditions, and without them the assertions below are vacuous:
    // this window really does reach the shortcut, the number it hands back is
    // a bound and not the static score, and the planted score sits between
    // alpha and that bound.
    bool exact = true;
    const int lazy = evaluate_lazy(&game.board, ALPHA, BETA, &exact);
    REQUIRE_FALSE(exact);
    REQUIRE_EQ(evaluate_cheap(&game.board), QUIET_ROOK_EVAL_CHEAP);
    REQUIRE_EQ(lazy, QUIET_ROOK_EVAL_CHEAP - LAZY_EVAL_MARGIN);
    REQUIRE_EQ(lazy, 383);
    REQUIRE_NE(lazy, evaluate(&game.board));
    REQUIRE(PLANTED > ALPHA);
    REQUIRE(PLANTED < lazy);
    REQUIRE(PLANTED >= BETA);

    // Precondition: with nothing planted the node stands pat on the bound
    // itself, so anything other than 383 below came from the entry.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE_EQ(run(), lazy);

    // An upper bound of 200 says the value is at most 200, which is less than
    // the 383 the shortcut established as a lower bound on the static score --
    // the two claims are about different things and both hold, the entry's
    // being the tighter one. So the cap applies to a stand pat that is itself
    // a bound, exactly as it applies to a static score.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE(load_FEN(fen, &game));
    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, PLANTED, TT_ALPHA_NODE, 0,
                   TT_EVAL_NONE);

    CHECK_EQ(run(), PLANTED);

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);

    // 200 is still at or above beta, so the node fails high and stores a lower
    // bound. It is sound on this path for the same reason as on the static
    // one, and the reason is worth stating because `static_eval` is not exact
    // here: the shortcut certified `value >= 383`, the cap only lowered the
    // number to 200, and `value >= 200` follows. The eval field stays empty --
    // no exact static score was ever computed at this node, and S094's
    // semantics say only an exact one may be stored.
    CHECK_EQ(entry->score, PLANTED);
    CHECK_EQ(entry->type, TT_BETA_NODE);
    CHECK_EQ(entry->eval, TT_EVAL_NONE);
  }


  // The main search reads the same field. Reverse futility asked evaluate()
  // for a number the entry was already carrying on 9.8 % of the calls it made,
  // and none of them disagreed; it now reads the entry. S103.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "reverse futility prunes on the stored static score")
  {
    // Nothing to capture and nothing in check, so what the node returns when
    // reverse futility fires is the static score minus the margin and nothing
    // else. Same anchor as the two cases above.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    // Below beta by more than the margin at every value the case plants, so
    // the fail-high is never the thing under test. The node is not the root,
    // is not on the PV, is at RFP_MIN_PLY and is within RFP_MAX_DEPTH, which
    // is every guard the site carries.
    constexpr int DEPTH = 1;
    constexpr int BETA = 100;

    auto run = [&](bool is_pv) -> int {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      return negamax(BETA - 1, BETA, DEPTH, RFP_MIN_PLY, &game, &state, 0,
                     is_pv);
    };

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    const int pruned = static_score - RFP_MARGIN * DEPTH;
    REQUIRE(pruned >= BETA);

    // Precondition: with nothing in the table the site works the number out
    // for itself, and the node returns the bound reverse futility argues for.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE_EQ(run(false), pruned);

    // Second precondition, and the one that makes the number above reverse
    // futility's rather than the search's. The same node as a PV node is the
    // one place the site is not allowed to fire, and it does not return this.
    tt_reset(&tt);
    tt_new_search(&tt);
    REQUIRE_NE(run(true), pruned);

    // Now an entry for this position carrying a static evaluation that is not
    // this position's. Stored at TT_DEPTH_QS, which a depth 1 node may not cut
    // on, so anything but 563 - margin coming back has to have come from the
    // eval field.
    tt_reset(&tt);
    tt_new_search(&tt);

    REQUIRE(load_FEN(fen, &game));
    const int planted = static_score - 200;
    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, -9999, TT_ALPHA_NODE, 0,
                   planted);

    const int planted_pruned = planted - RFP_MARGIN * DEPTH;
    REQUIRE(planted_pruned >= BETA);
    REQUIRE_EQ(run(false), planted_pruned);
  }


  // The main search now records the number it computed, and the field is no
  // longer overwritten with the sentinel by a node that has one. Both halves
  // were observed red: before S108 the main search passed TT_EVAL_NONE at
  // every node but a reverse futility one, and tt_store_entry() wrote it over
  // whatever the entry held.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a main-search store records this node's evaluation")
  {
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, QUIET_ROOK_EVAL);

    tt_reset(&tt);
    tt_new_search(&tt);

    // A PV node, which is the one place reverse futility may not fire, so the
    // node searches its moves and reaches the store at the bottom instead of
    // returning a bound from the top.
    auto run = [&]() {
      REQUIRE(load_FEN(fen, &game));
      never_stop = false;

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      negamax(-100000, 100000, 1, 1, &game, &state, 0, true);
    };

    run();

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);
    CHECK_EQ(entry->eval, static_score);
  }


  // And it records it once. A node that recomputed instead of reading the
  // entry it already probed would store evaluate()'s number, so planting an
  // evaluation this position does not have is what tells the two apart -- the
  // same trick the reverse futility case above uses to prove the read.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a main-search node evaluates once and stores what it read")
  {
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    static std::atomic_bool never_stop = false;

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);

    tt_reset(&tt);
    tt_new_search(&tt);

    // Stored at TT_DEPTH_QS with a score that answers nothing, so the entry
    // reaches the node as an evaluation and never as a cutoff.
    const int planted = static_score - 200;
    REQUIRE_NE(planted, static_score);
    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, -9999, TT_ALPHA_NODE, 0,
                   planted);

    REQUIRE(load_FEN(fen, &game));
    never_stop = false;

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    negamax(-100000, 100000, 1, 1, &game, &state, 0, true);

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);

    // Depth 1 replaced the depth -1 entry, so this is the main search's own
    // store and not the planted one surviving.
    REQUIRE_EQ(entry->depth, 1);
    CHECK_EQ(entry->eval, planted);
  }


  // A store that carries no evaluation leaves the one already there alone. The
  // main search stores the sentinel at every node it was in check at, and
  // TT_DEPTH_QS sits below every main depth, so without this a check node
  // wipes the number quiescence recorded for the same position.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a store with no evaluation keeps the one on the entry")
  {
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    tt_reset(&tt);
    tt_new_search(&tt);

    tt_store_entry(&tt, &game.board, TT_DEPTH_QS, 300, TT_PV_NODE, 0, 300);

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->eval, 300);

    // Deeper, so replacement fires and every other field is overwritten.
    tt_store_entry(&tt, &game.board, 7, -50, TT_ALPHA_NODE, 0, TT_EVAL_NONE);

    entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->depth, 7);
    REQUIRE_EQ(entry->score, -50);
    CHECK_EQ(entry->eval, 300);
  }


  // The rule that keeps the line above from handing one position's evaluation
  // to another. Held on the decision itself rather than through a store,
  // because the collision it guards against -- two keys landing on one slot --
  // is not constructible from a FEN on demand, and a rule that cannot be
  // exercised is a rule that is not tested.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "an evaluation never survives a key change")
  {
    tt_entry_t entry = {};
    entry.key = 0xfeedfacecafebeefULL;
    entry.eval = 300;
    entry.generation = 1;

    // Same position, nothing to record: the number stays.
    CHECK_EQ(tt_eval_to_store(&entry, entry.key, TT_EVAL_NONE), 300);

    // A different position landing on this slot: the number is that other
    // position's business and this one has none to offer.
    CHECK_EQ(tt_eval_to_store(&entry, entry.key ^ 1ULL, TT_EVAL_NONE),
             TT_EVAL_NONE);

    // A real evaluation always wins, whichever position was here before.
    CHECK_EQ(tt_eval_to_store(&entry, entry.key, -42), -42);
    CHECK_EQ(tt_eval_to_store(&entry, entry.key ^ 1ULL, -42), -42);

    // A slot that was never written has key 0 and eval 0, and 0 is an ordinary
    // evaluation. Only the generation tells the two apart, so the preserve
    // tests it: without that, a hash of 0 would inherit a zero that nothing
    // ever computed.
    tt_entry_t fresh = {};
    CHECK_EQ(tt_eval_to_store(&fresh, 0, TT_EVAL_NONE), TT_EVAL_NONE);
  }


  // Without this, a quiescence that never stored anything would pass every
  // other case in this file.
  TEST_CASE_FIXTURE(search_fixture_t, "quiescence writes entries of its own")
  {
    const std::string fen =
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    search_fen_with(fen, 5, &tt, true);

    size_t quiescence_entries = 0;
    size_t main_entries = 0;

    for (size_t i = 0; i < tt.entry_count; ++i) {
      if (tt.entries[i].generation == 0) { continue; }

      if (tt.entries[i].depth == TT_DEPTH_QS) {
        quiescence_entries++;
      } else {
        main_entries++;
      }
    }

    // Precondition: the search wrote to the table at all. A table that was
    // never allocated, or a search that stored nothing, would otherwise leave
    // both counts at zero and the assertion below would be about nothing.
    REQUIRE(main_entries > 0);
    REQUIRE(quiescence_entries > 0);

    // Every entry the main search wrote is at a depth that could not have come
    // from quiescence, which is the storage half of the rule the first case in
    // this suite holds on the reading side.
    //
    // `>= TT_DEPTH_QS` was the whole assertion until S193, and TT_DEPTH_QS is
    // -1, the lowest depth any writer can produce: it held for every entry
    // whatever the search did. The two bands are disjoint instead - a store is
    // quiescence's own sentinel, or it is depth 1 or more. Depth 0 is the gap
    // and it is unreachable by construction: negamax hands `depth < 1` to
    // quiescence before it can store. S193, 2026-09-04_test_review-F05.
    for (size_t i = 0; i < tt.entry_count; ++i) {
      if (tt.entries[i].generation == 0) { continue; }
      REQUIRE((tt.entries[i].depth == TT_DEPTH_QS || tt.entries[i].depth >= 1));
    }
  }
}


// The improving flag has no consumer until S109, so nothing in a game reads it
// and nothing in a benchmark would notice it being wrong. A wrong improving
// calculation is a pure strength loss: it never crashes, never changes a node
// count on its own, and only shows up as a worse margin at every site that
// eventually reads it. So every branch of the definition is held directly.
//
// The definition (CPW "Improving"): compare this node's static evaluation with
// the one two plies up; if that node was in check and recorded none, with the
// one four plies up; false while in check; true when there is nothing to
// compare against. S108.
TEST_SUITE("search: improving")
{
  // The array is what the search writes and the helper reads, so the cases
  // plant it directly. A fresh state zeroes it, and 0 is an ordinary
  // evaluation -- which is exactly why the helper may never treat an unwritten
  // slot as absent data, and why every case below states each slot it depends
  // on, including the ones it wants left at zero.
  struct improving_fixture_t
  {
    search_state_t state = {};

    void plant(size_t ply, int eval) { state.static_evals[ply] = eval; }
  };


  // No ancestor to compare against, so the flag is true by definition rather
  // than by comparison. Plies 0 and 1 are the only two plies where this holds
  // for the ply-2 reason; deeper nodes reach it only through the sentinels.
  TEST_CASE_FIXTURE(improving_fixture_t, "no earlier node defaults to true")
  {
    plant(0, -400);
    CHECK(improving_at(&state, 0, false));

    plant(1, -400);
    CHECK(improving_at(&state, 1, false));
  }


  // Strictly greater. An evaluation that has not moved has not improved, and
  // the difference matters at exactly the nodes where it is easiest to get
  // wrong: a shuffling line evaluates to the same number ply after ply.
  TEST_CASE_FIXTURE(improving_fixture_t, "equal is not improving")
  {
    plant(2, 50);
    plant(0, 49);
    CHECK(improving_at(&state, 2, false));

    plant(0, 50);
    CHECK_FALSE(improving_at(&state, 2, false));

    plant(0, 51);
    CHECK_FALSE(improving_at(&state, 2, false));
  }


  // The fallback, and the case that fails against the obvious implementation.
  // A node in check writes TT_EVAL_NONE, which is INT16_MIN, so a comparison
  // that does not test for the sentinel answers "improving" for any real
  // evaluation at all -- the flag would read true at every node two plies
  // under a check. Observed red against exactly that form.
  //
  // So the case is built the only way that can tell the two apart: the ply-4
  // evaluation is *above* this node's, which makes the correct answer false,
  // while the sentinel comparison the broken form makes answers true.
  TEST_CASE_FIXTURE(improving_fixture_t,
                    "a checked node two plies up falls back four")
  {
    plant(4, 20);
    plant(2, TT_EVAL_NONE);
    plant(0, 300);

    CHECK_FALSE(improving_at(&state, 4, false));

    // And the fallback compares rather than defaulting: same shape, an
    // ancestor this node has improved on.
    plant(0, -300);
    CHECK(improving_at(&state, 4, false));
  }


  // Both ancestors in check. There is no number in reach, which is the
  // no-data case again and not a comparison against a sentinel.
  TEST_CASE_FIXTURE(improving_fixture_t,
                    "two checked ancestors default to true")
  {
    plant(4, -900);
    plant(2, TT_EVAL_NONE);
    plant(0, TT_EVAL_NONE);

    CHECK(improving_at(&state, 4, false));
  }


  // In check the node has no static score of its own to have improved, and
  // the answer is false rather than the default. The slot is planted with a
  // number the ply-2 comparison would call improving, so the case fails on any
  // implementation that reaches the comparison at all.
  TEST_CASE_FIXTURE(improving_fixture_t, "in check is never improving")
  {
    plant(2, 500);
    plant(0, -500);

    REQUIRE(improving_at(&state, 2, false));
    CHECK_FALSE(improving_at(&state, 2, true));
  }
}


// The two things a transposition table gets wrong without any test noticing:
// which side of the window a bound is allowed to answer on, and whether a mate
// score carries the ply it was seen at. Neither shows up as a wrong node count
// or a crash -- a swapped bound condition is a pure strength loss, and a
// dropped ply term is the published "announces mate and then shuffles"
// symptom. S106 is the sweep; these are the tests it leaves behind, and each
// one was observed red under a stated mutation of the code it covers.
TEST_SUITE("search: transposition bounds and mate distance")
{
  // search.cpp's, pinned here rather than shared, for the reason the suite
  // above pins them: a change to either should read as a disagreement.
  static constexpr int MATE_MAX_LOCAL = 49000;
  static constexpr int MATE_MIN_LOCAL = 48000;

  // A mate delivered at ply 1 of a position, seen from that position. What
  // negamax stores for a node one ply above a mate, once normalised.
  static constexpr int MATE_IN_ONE_PLY = MATE_MAX_LOCAL - 1;

  // Drives one node of the search directly at an arbitrary ply. Neither
  // function is reachable at ply 3 through search(), and the ply is the whole
  // subject here.
  struct node_fixture_t : search_fixture_t
  {
    std::atomic_bool never_stop{false};
    search_state_t state = {};

    void load(const std::string& fen)
    {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));
      REQUIRE_MESSAGE(position_is_reachable(&game),
                      (fen + " is not a position a legal game can reach"));

      // Wiped every time, and that is what keeps the ply-0 precondition in
      // each case below from answering the ply-3 drive out of the table
      // instead of searching it. It does not: the entry the precondition
      // writes is an exact one, every probe accepts it, and the second drive
      // returns the de-normalised score without storing anything. The whole
      // case then passes with the store arm never executed.
      tt_reset(&tt);

      never_stop = false;
      state = {};
      state.tt = &tt;
      state.stop = &never_stop;
    }

    // The entry the node just wrote for the position it was called on. The
    // board is back where it started: make_move and unmake_move balance.
    const tt_entry_t* stored() const { return tt_get_entry(&tt, &game.board); }
  };


  // Mutation: swap the two bound conditions in tt_entry_answers(), so a
  // TT_BETA_NODE is tested against alpha and a TT_ALPHA_NODE against beta.
  //
  //   search: transposition bounds and mate distance
  //    the bound conditions decide which side of the window may answer
  //   REQUIRE( tt_entry_answers(&entry, 1, 3, -100, 100, &score) )
  //   values: REQUIRE( false )
  TEST_CASE("the bound conditions decide which side of the window may answer")
  {
    tt_entry_t entry = {};
    entry.key = 0x3456ULL;
    entry.depth = 1;
    entry.generation = 1;

    int score = 0;

    // Precondition, and what stops the refusals below from being vacuous: an
    // exact entry answers whatever the window is, so the probe is reaching
    // these entries at all and the depth gate is not what is deciding.
    entry.type = TT_PV_NODE;
    entry.score = 50;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 50);
    entry.score = 500;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 500);

    // A lower bound answers a node whose beta it already clears, and the score
    // comes back unclamped: this is a fail-soft engine and a stored value
    // above the window is the normal case, not a corruption.
    entry.type = TT_BETA_NODE;
    entry.score = 500;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 500);

    // Equality is a cutoff. "At least beta" is what a fail-high proved.
    entry.score = 100;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 100);

    // Inside the window it proves nothing this node can use: the true value is
    // somewhere at or above 50 and may still be under beta.
    entry.score = 50;
    score = 0;
    REQUIRE_FALSE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 0);

    // A lower bound is never an answer at the alpha end, which is the half a
    // swapped condition gets wrong in the direction that still looks plausible.
    entry.score = -500;
    score = 0;
    REQUIRE_FALSE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 0);

    // And the mirror image for a ceiling.
    entry.type = TT_ALPHA_NODE;
    entry.score = -500;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, -500);

    entry.score = -100;
    score = 0;
    REQUIRE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, -100);

    entry.score = 50;
    score = 0;
    REQUIRE_FALSE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 0);

    entry.score = 500;
    score = 0;
    REQUIRE_FALSE(tt_entry_answers(&entry, 1, 3, -100, 100, &score));
    REQUIRE_EQ(score, 0);
  }


  // The order of the two operations in tt_entry_answers() is load bearing and
  // only a mate score can show it: for an ordinary score the ply term is zero
  // and comparing before or after adjusting is the same comparison.
  //
  // Mutation: compare entry->score against the window and de-normalise the
  // value afterwards.
  //
  //   search: transposition bounds and mate distance
  //    a mate bound is compared after the ply adjustment, not before
  //   REQUIRE_FALSE( tt_entry_answers(&entry, 1, 3, -100000, MATE_MAX_LOCAL -
  //   3, &score) ) values: REQUIRE_FALSE( true )
  TEST_CASE("a mate bound is compared after the ply adjustment, not before")
  {
    tt_entry_t entry = {};
    entry.key = 0x4567ULL;
    entry.depth = 1;
    entry.generation = 1;
    entry.type = TT_BETA_NODE;

    // "At least a mate one ply from here". Read at ply 3 that is a mate at ply
    // 4 of the search, worth MATE_MAX - 4.
    entry.score = MATE_IN_ONE_PLY;

    int score = 0;

    // Precondition: the entry does answer a node it clears, and the value that
    // comes back is the re-based one. Without this the refusal below would
    // hold for an entry that answers nothing.
    REQUIRE(
        tt_entry_answers(&entry, 1, 3, -100000, MATE_MAX_LOCAL - 8, &score));
    REQUIRE_EQ(score, MATE_MAX_LOCAL - 4);

    // Against a beta that only a mate at ply 3 would clear, the entry must
    // refuse: what it holds is a mate one ply further away. The raw stored
    // number clears that beta, so a probe that compares first takes a cutoff
    // it did not prove and claims a mate one ply sooner than exists.
    score = 0;
    REQUIRE_FALSE(
        tt_entry_answers(&entry, 1, 3, -100000, MATE_MAX_LOCAL - 3, &score));
    REQUIRE_EQ(score, 0);

    // The mirror image on the ceiling: "at worst mated one ply from here".
    entry.type = TT_ALPHA_NODE;
    entry.score = -MATE_IN_ONE_PLY;

    score = 0;
    REQUIRE(
        tt_entry_answers(&entry, 1, 3, -(MATE_MAX_LOCAL - 8), 100000, &score));
    REQUIRE_EQ(score, -(MATE_MAX_LOCAL - 4));

    score = 0;
    REQUIRE_FALSE(
        tt_entry_answers(&entry, 1, 3, -(MATE_MAX_LOCAL - 3), 100000, &score));
    REQUIRE_EQ(score, 0);
  }


  // The store arm of the pair, in quiescence, which is the only writer that
  // reaches the endpoint: a mate with no legal reply is stored as exactly
  // -MATE_MAX, "mated here", whatever ply the node sits at. The score the node
  // returns is the ply-relative one and the score it writes down is not, and
  // that difference is the whole of normalize_score().
  //
  // Mutation: drop the `- ply` from normalize_score()'s negative arm.
  //
  //   search: transposition bounds and mate distance
  //    quiescence stores a mate at its own position, not at the searching ply
  //   REQUIRE( entry->score == -MATE_MAX_LOCAL )
  //   values: REQUIRE( -48997 == -49000 )
  TEST_CASE_FIXTURE(
      node_fixture_t,
      "quiescence stores a mate at its own position, not at the searching ply")
  {
    // In check with no legal reply. The same position the suite above uses for
    // the reading half of this rule.
    const std::string fen = "7k/6Q1/6K1/8/8/8/8/8 b - - 0 1";

    // Precondition: at the root the two numbers coincide, so the case below is
    // about the ply term and not about the store happening at all.
    load(fen);
    REQUIRE_EQ(
        quiescence(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 0, 0, &game, &state),
        -MATE_MAX_LOCAL);

    const tt_entry_t* root_entry = stored();
    REQUIRE(root_entry != nullptr);
    REQUIRE_EQ(root_entry->score, -MATE_MAX_LOCAL);
    REQUIRE_EQ(root_entry->type, TT_PV_NODE);
    REQUIRE_EQ(root_entry->depth, TT_DEPTH_QS);

    // The same node three plies down. What it returns moves with the ply and
    // what it stores does not.
    load(fen);
    REQUIRE_EQ(
        quiescence(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 3, 0, &game, &state),
        -(MATE_MAX_LOCAL - 3));

    const tt_entry_t* entry = stored();
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->score, -MATE_MAX_LOCAL);
    REQUIRE_EQ(entry->type, TT_PV_NODE);
    REQUIRE_EQ(entry->depth, TT_DEPTH_QS);
  }


  // The same store arm in the main search, on the positive side: a node one
  // ply above a mate stores "mate in one ply from here" and not the distance
  // it happened to see it from.
  //
  // Mutation: drop the `+ ply` from normalize_score()'s positive arm.
  //
  //   search: transposition bounds and mate distance
  //    the main search stores a mate below it at the mate's own distance
  //   REQUIRE( entry->score == MATE_IN_ONE_PLY )
  //   values: REQUIRE( 48996 == 48999 )
  TEST_CASE_FIXTURE(node_fixture_t,
                    "the main search stores a mate below it at the mate's own "
                    "distance")
  {
    // Qxg7 is mate. Verified with /usr/games/stockfish, depth 20: score mate 1,
    // pv a7g7. A capture, so the main search reaches the mate at depth 1 and
    // the ordinary mated-node return happens one ply below this one.
    const std::string fen = "7k/Q5b1/6K1/8/8/8/8/8 w - - 0 1";

    // Precondition: at the root the stored number and the returned number
    // coincide, so the case below is about the ply term.
    load(fen);
    REQUIRE_EQ(negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 1, 0, &game, &state,
                       0, false),
               MATE_IN_ONE_PLY);

    const tt_entry_t* root_entry = stored();
    REQUIRE(root_entry != nullptr);
    REQUIRE_EQ(root_entry->score, MATE_IN_ONE_PLY);

    load(fen);
    REQUIRE_EQ(negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 1, 3, &game, &state,
                       0, false),
               MATE_MAX_LOCAL - 4);

    const tt_entry_t* entry = stored();
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->score, MATE_IN_ONE_PLY);
    REQUIRE_EQ(entry->type, TT_PV_NODE);
    REQUIRE_EQ(entry->depth, 1);
  }


  // And the negative side in the main search, which quiescence cannot produce:
  // a node that has legal moves and loses to a mate anyway. negamax returns
  // from a mated node before it stores anything, so this is the only way the
  // main search ever writes a negative mate score.
  //
  // Mutation: drop the `- ply` from normalize_score()'s negative arm.
  //
  //   search: transposition bounds and mate distance
  //    the main search stores a mate against it at the mate's own distance
  //   REQUIRE( entry->score == -(MATE_MAX_LOCAL - 2) )
  //   values: REQUIRE( -48995 == -48998 )
  TEST_CASE_FIXTURE(node_fixture_t,
                    "the main search stores a mate against it at the mate's "
                    "own distance")
  {
    // MATE_IN_2_W_POS after e5e6, derived with python-chess rather than read
    // off a board. Black is not in check, has exactly two legal moves, and
    // both lose to a mate on the next ply: /usr/games/stockfish at depth 24
    // gives the parent position mate 2 with pv e5e6 e8f8 a7f7.
    const std::string fen = "4k3/Q7/4K3/8/8/8/8/8 b - - 1 1";

    load(fen);
    REQUIRE_EQ(negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 2, 0, &game, &state,
                       0, false),
               -(MATE_MAX_LOCAL - 2));

    const tt_entry_t* root_entry = stored();
    REQUIRE(root_entry != nullptr);
    REQUIRE_EQ(root_entry->score, -(MATE_MAX_LOCAL - 2));

    load(fen);
    REQUIRE_EQ(negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 2, 3, &game, &state,
                       0, false),
               -(MATE_MAX_LOCAL - 5));

    const tt_entry_t* entry = stored();
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->score, -(MATE_MAX_LOCAL - 2));
    REQUIRE_EQ(entry->type, TT_PV_NODE);
    REQUIRE_EQ(entry->depth, 2);
  }


  // The round trip the two arms exist for, end to end and through a real
  // store: the same position written by a node at one ply and read by a node
  // at another has to describe the same mate. The distance to the mate from
  // the reading node is what must agree; the score does not, and cannot.
  //
  // Fails under any of the four mutations above.
  TEST_CASE_FIXTURE(node_fixture_t,
                    "a mate stored at one ply reads the same distance at "
                    "another")
  {
    const std::string fen = "7k/Q5b1/6K1/8/8/8/8/8 w - - 0 1";

    load(fen);
    REQUIRE_EQ(negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 1, 3, &game, &state,
                       0, false),
               MATE_MAX_LOCAL - 4);

    const tt_entry_t* entry = stored();
    REQUIRE(entry != nullptr);

    // Read back at every ply a search could reach this position at, including
    // the one that wrote it. The distance to mate is measured from the reading
    // node, which is what makes it the invariant.
    for (size_t q = 0; q < 32; ++q) {
      int score = 0;
      REQUIRE_MESSAGE(tt_entry_answers(entry, 1, q, -SEARCH_SCORE_INF,
                                       SEARCH_SCORE_INF, &score),
                      ("ply " + std::to_string(q)));
      REQUIRE_MESSAGE(
          MATE_MAX_LOCAL - score - static_cast<int>(q) == 1,
          ("ply " + std::to_string(q) + " score " + std::to_string(score)));
    }
  }


  // The published symptom in miniature: an engine whose mate scores are not
  // ply-adjusted in both directions announces a mate and then never converts
  // it, because every position it reaches reports the distance the entry was
  // written at. Here the table is warm and shared across the three searches,
  // so every score after the first is read back at a ply it was not written
  // at, and the distance has to fall by exactly the plies played.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "the announced mate distance falls by one for every ply "
                    "played")
  {
    // Mate in 2. /usr/games/stockfish at depth 24: score mate 2, pv e5e6 e8f8
    // a7f7 -- three plies.
    search_t result = search_fen_with(MATE_IN_2_W_POS, 5, &tt, true);

    // Precondition: the mate is found at all. Everything below is about the
    // distance and says nothing if there is no mate score to carry.
    REQUIRE(result.mate_found);
    REQUIRE_EQ(MATE_MAX_LOCAL - std::abs(result.score), 3);

    static std::atomic_bool never_stop = false;

    for (int played = 1; played <= 2; ++played) {
      REQUIRE(result.pv.length >= 1);
      REQUIRE(make_move(&game, result.pv.table[0]));

      // Same table, no reset: a new search in the same game, which is the
      // only condition under which the stale-distance bug is visible.
      never_stop = false;
      tt_new_search(&tt);

      search_state_t state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      result = search(5, &game, &state);

      REQUIRE_MESSAGE(result.mate_found,
                      ("after " + std::to_string(played) + " plies"));
      REQUIRE_MESSAGE(MATE_MAX_LOCAL - std::abs(result.score) == 3 - played,
                      ("after " + std::to_string(played) + " plies the score " +
                       std::to_string(result.score) + " is " +
                       std::to_string(MATE_MAX_LOCAL - std::abs(result.score)) +
                       " plies from mate"));
    }
  }


  // The ordering property the table depends on and no test held: a draw is a
  // property of the path and the Zobrist key does not carry the path, so a
  // position that has just repeated must be answered as a draw before the
  // table is allowed to answer it as a win. The entry is not wrong -- the same
  // position really is worth what it says down another path -- which is what
  // makes probe-first a silent defect rather than a crash.
  //
  // Mutation: move the three draw tests in negamax below the probe.
  //
  //   search: transposition bounds and mate distance
  //    a repetition is answered before the table is
  //   REQUIRE_EQ( repeated, DRAW_SCORE_LOCAL )
  //   values: REQUIRE_EQ( 664, 0 )
  TEST_CASE_FIXTURE(search_fixture_t,
                    "a repetition is answered before the "
                    "table is")
  {
    static constexpr int DRAW_SCORE_LOCAL = 0;

    // Rook and king against a bare king: material that can still mate, so
    // is_insufficient_material() does not answer this position and the draw
    // that fires below is the repetition one.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/R3K3 w - - 0 1", &game));

    static std::atomic_bool never_stop = false;
    never_stop = false;

    search_state_t state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    // Ply 1 rather than the root: the draw tests and the probe are both
    // guarded on ply > 0, so the root would exercise neither.
    const int fresh = negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 3, 1, &game,
                              &state, 0, false);

    // Preconditions, and what stops the assertion below from holding for a
    // position that is worth nothing anyway or for an entry that answers
    // nobody: the position is not a draw on its own, and the entry it wrote
    // does answer the probe this node makes.
    REQUIRE_NE(fresh, DRAW_SCORE_LOCAL);

    const tt_entry_t* entry = tt_get_entry(&tt, &game.board);
    REQUIRE(entry != nullptr);

    int answered = 0;
    REQUIRE(tt_entry_answers(entry, 3, 1, -SEARCH_SCORE_INF, SEARCH_SCORE_INF,
                             &answered));
    REQUIRE_EQ(answered, fresh);

    // Ra1-a3-a1 around Ke8-d8-e8 puts the same position back on the board with
    // the same side to move, and nothing irreversible happened in between.
    const std::vector<std::pair<index_t, index_t>> line = {
        {a1, a3}, {e8, d8}, {a3, a1}, {d8, e8}};

    for (const auto& [from, to] : line) {
      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      bool played = false;

      for (size_t i = 0; i < count; ++i) {
        if (MOVE_FROM(moves[i]) == from && MOVE_TO(moves[i]) == to) {
          REQUIRE(make_move(&game, moves[i]));
          played = true;
          break;
        }
      }

      REQUIRE(played);
    }

    REQUIRE(is_position_repeated(&game.history, &game.board));

    // Same table, deliberately not reset: the entry written above is still
    // there and still answers, and the draw has to be decided first anyway.
    never_stop = false;
    state = {};
    state.tt = &tt;
    state.stop = &never_stop;

    const int repeated = negamax(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 3, 1,
                                 &game, &state, 0, false);

    REQUIRE_EQ(repeated, DRAW_SCORE_LOCAL);
  }


  // The store side of the round trip, over a real search rather than one node:
  // normalize_score() adds the ply to a score already inside the mate band, so
  // a value that leaves the band on the way in is one de_normalize_score()
  // will not touch on the way out, and the mate distance stops being adjusted
  // at all. The argument that it cannot happen is that a mate seen from ply p
  // is worth at most MATE_MAX - (p + 1) -- a mated node returns before it
  // stores anything, so the nearest a stored mate can be is one ply below the
  // node storing it. This holds that argument against the table instead of
  // against the reasoning, which is where S094's defect was hiding.
  TEST_CASE_FIXTURE(search_fixture_t, "no stored score leaves the mate band")
  {
    // Two forced mates and two ordinary positions: the mates are what put
    // scores in the band at all, the rest is volume.
    const std::vector<std::pair<std::string, int>> positions = {
        {MATE_IN_2_W_POS, 6},
        {"7k/Q5b1/6K1/8/8/8/8/8 w - - 0 1", 6},
        {DEFAULT_POSITION, 6},
        {TRICKY_POS, 5},
    };

    size_t written = 0;
    size_t in_band = 0;

    for (const auto& [fen, depth] : positions) {
      search_fen_with(fen, depth, &tt, /*reset=*/false);

      for (size_t i = 0; i < tt.entry_count; ++i) {
        const tt_entry_t& entry = tt.entries[i];

        if (entry.generation == 0) { continue; }

        written++;

        if (std::abs(entry.score) > MATE_MIN_LOCAL) { in_band++; }

        REQUIRE_MESSAGE(
            std::abs(entry.score) <= MATE_MAX_LOCAL,
            ("FEN: " + fen + " stored " + std::to_string(entry.score)));
      }
    }

    // Preconditions. The sweep says nothing unless entries were written, and
    // the band assertion says nothing unless something landed in the band.
    REQUIRE(written > 0);
    REQUIRE(in_band > 0);
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
  // Two more paths break the stated purity and are already in the tree.
  // S130: quiescence() in src/search.cpp raises or lowers `stand_pat` from a
  // table entry's bound -- `stand_pat_type` carries which -- so a warm table
  // changes the stand pat. S094 and S130: tt_entry_answers() accepts a
  // main-search entry at TT_DEPTH_QS, so quiescence can answer from a node the
  // main search stored. Depth 2 to 3 over this corpus exposes neither; a
  // failure here reads as "one of three things changed".
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
    //
    // GOLDEN (DEC-142): -569, evaluate() with Black to move a rook down, after
    // the three moves above. It is the precondition, not the property: what the
    // case asserts is that a draw score beats it.
    // Re-derive: python3 adocs/data/S192_anchors.py, case
    // "black a rook down, Kh7". Moves legitimately on: a refit.
    // Margin: exact -- but the assertion it feeds needs only that the number is
    // below zero, so a refit that moves it changes nothing else here.
    // Property beside it: the repetition assertion below, which compares
    // against 0 and not against this value.
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

  // The case above cannot see the boundary. Its root is at 100 and the root is
  // exempt from the draw test, so every node it reaches is at 101 or more and a
  // rule that read `>= 101` would pass it unchanged - which is exactly what
  // mutant M19 of the 2026-09-04 test review does, surviving all 27 binaries.
  // The boundary needs a root one halfmove below it, so the children land on
  // exactly 100. S193, 2026-09-04_test_review-F04.
  TEST_CASE_FIXTURE(search_fixture_t,
                    "the fifty-move boundary lands on the hundredth halfmove")
  {
    const char* at_99 = "4k3/8/8/8/8/8/8/3QK3 w - - 99 200";
    const char* at_98 = "4k3/8/8/8/8/8/8/3QK3 w - - 98 200";

    // Preconditions, from the engine's own generator rather than from a claim
    // about the position: a reachable root, material that is not an
    // insufficient-material draw, at least one reply, and every reply quiet -
    // no capture, no pawn move - landing on clock exactly 100 without mating.
    // So at ply 1 the fifty-move rule applies to every child, and neither of
    // the two things that outrank it does: not the mate exception S162 added,
    // and not the insufficient-material return below it.
    //
    // The mate test is spelled the way negamax spells it, in check with no
    // reply, and not as "is the child in check": several of the queen's moves
    // do give check, and a check that is not mate still draws.
    REQUIRE(load_FEN(at_99, &game));
    REQUIRE(position_is_reachable(&game));
    REQUIRE_FALSE(is_insufficient_material(&game.board));

    move_t replies[MAX_MOVES];
    const size_t count = generate_moves(game_tables(), &game.board, replies);
    REQUIRE(count > 0);

    for (size_t i = 0; i < count; ++i) {
      REQUIRE_FALSE(MOVE_CAPTURE(replies[i]));
      REQUIRE_NE(MOVE_PIECE(replies[i]), W_PAWN);
      REQUIRE(make_move(&game, replies[i]));
      REQUIRE_EQ(int(game.board.halfmove_clock), 100);

      move_t escapes[MAX_MOVES];
      const bool child_is_mated =
          is_check(&game) &&
          generate_moves(game_tables(), &game.board, escapes) == 0;
      REQUIRE_FALSE(child_is_mated);
      REQUIRE_FALSE(is_insufficient_material(&game.board));

      unmake_move(&game);
    }

    // Each probe resets the table (search_fen does), which matters here: the
    // Zobrist key does not carry the halfmove clock, so a warm entry from the
    // clock-98 probe would answer the clock-99 child.
    REQUIRE_EQ(search_fen(at_99, 1).score, 0);  // every reply draws at ply 1
    REQUIRE_NE(search_fen(at_98, 1).score, 0);  // one halfmove earlier, not
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
    tt_store_entry(&table, &board, 7, -1234, TT_BETA_NODE, 0xabcd, 77);

    const tt_entry_t* entry = tt_get_entry(&table, &board);

    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->key, board.hash);
    REQUIRE_EQ(entry->depth, 7);
    REQUIRE_EQ(entry->score, -1234);
    REQUIRE_EQ(entry->type, TT_BETA_NODE);
    REQUIRE_EQ(entry->best_move, 0xabcd);
    REQUIRE_EQ(entry->eval, 77);

    // A caller that never had a static score says so, and the sentinel is not
    // a score anything could have produced. S094.
    const board_t plain = board_keyed(0x0123456789abcdeeULL);
    tt_store_entry(&table, &plain, 7, -1234, TT_BETA_NODE, 0xabcd);
    REQUIRE_EQ(tt_get_entry(&table, &plain)->eval, TT_EVAL_NONE);

    tt_free(&table);
  }


  // The hazard S094 names: the entry layout is shared with the main search, so
  // widening it could change how many entries a given Hash buys and with them
  // the replacement behaviour, which alters play on its own. It does not, and
  // this is the measurement rather than the argument.
  TEST_CASE("the static evaluation field costs no table entries")
  {
    // 20 bytes of content in 24 before the field, 22 in 24 after: it lands in
    // padding the key's alignment was already reserving.
    REQUIRE_EQ(sizeof(tt_entry_t), 24u);

    // And the count is floored to a power of two, which absorbs the entry size
    // entirely over the range this struct can plausibly occupy. Asserted at
    // both ends of the range so the claim is about the flooring and not about
    // one lucky size: 24 and 32 bytes an entry buy the same 131072 entries out
    // of 4 MB, because 4 MB / 32 is already a power of two and 4 MB / 24 sits
    // between it and the next one up.
    transposition_table_t table = {};
    tt_resize(&table, 4);

    REQUIRE_EQ(table.entry_count, 131072u);
    REQUIRE_EQ(std::bit_floor((4u * 1024 * 1024) / 32u), table.entry_count);
    REQUIRE_EQ(std::bit_floor((4u * 1024 * 1024) / sizeof(tt_entry_t)),
               table.entry_count);

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


// Every guard on null move pruning, reverse futility and late move reduction,
// one case each, with the guard's own condition established before the search
// so the guard is the only thing between the node and the rule. S191.
//
// The 2026-09-04 test review removed five of these guards one at a time and
// found each removal caught by a single golden count in test_mate_carry.cpp
// and by nothing else, three of the five leaving the depth-9 bench identical
// as well -- so INV-6 would have passed them too (F02). What the cases here
// read is `search_node_probe_t`, which records the pruning decisions of
// exactly one node and is written by negamax and never read by it.
TEST_SUITE("search: pruning and reduction guards")
{
  // search.cpp's, pinned here rather than shared, for the reason the two
  // suites above pin them: a change to either should read as a disagreement.
  static constexpr int MATE_MAX_LOCAL = 49000;
  static constexpr int MATE_MIN_LOCAL = 48000;

  // The drive depth for every null-move case. The only depth at which the
  // block's own `depth - 1 - null_reduction >= 1` clears by exactly one ply,
  // which each case asserts rather than assumes: at 4 the reduced search is
  // zero plies deep and the block is never entered at all, so a case driven
  // there would pass with the guard removed.
  static constexpr int NULL_DRIVE_DEPTH = 5;

  // An ordinary beta, inside the mate band by three orders of magnitude. Cases
  // that are about a band edge set their own.
  static constexpr int ORDINARY_BETA = 100;

  // A well-formed previous move, which is all the null-move guard reads of it
  // -- `prev_move != 0` -- and all score_move needs to index counter_moves
  // with. 1.e4, so the piece and both squares are real.
  static const move_t PREV_MOVE = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);

  // Drives one node with a probe attached to it.
  //
  // The table is wiped and a new search started before every drive, which is
  // what keeps a case from reading an entry an earlier one wrote and, more
  // to the point here, keeps `static_eval` equal to `evaluate()`: negamax
  // takes the stored evaluation where the probe found one (S103), and the
  // reverse-futility cases below set beta from a fresh `evaluate()` call.
  struct guard_fixture_t : search_fixture_t
  {
    std::atomic_bool never_stop{false};
    search_state_t state = {};
    search_node_probe_t probe = {};

    void load(const std::string& fen, int ply)
    {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("FEN: " + fen));
      REQUIRE_MESSAGE(position_is_reachable(&game),
                      (fen + " is not a position a legal game can reach"));

      tt_reset(&tt);
      tt_new_search(&tt);

      never_stop = false;
      state = {};
      state.tt = &tt;
      state.stop = &never_stop;

      probe = {};
      probe.ply = ply;
      state.probe = &probe;
    }

    // Everything the null-move block tests except the one guard the case is
    // about, asserted before the drive. Without this a case saying "no null
    // move here" passes for the wrong reason -- some other condition of the
    // block was false and the guard under test never decided anything.
    //
    // `!is_pv` and `ply > 0` are properties of the drive and are asserted at
    // the call site; the rest are properties of the position and the window.
    void require_null_move_preconditions(int depth, int beta, move_t prev)
    {
      REQUIRE_EQ(depth - 1 - (NULL_MOVE_BASE + (depth / NULL_MOVE_DIVISOR)), 1);
      REQUIRE(prev != 0);
      REQUIRE(beta < MATE_MIN_LOCAL);
      REQUIRE(beta > -MATE_MIN_LOCAL);
      REQUIRE(game_phase(&game.board) > 0);
      REQUIRE(!is_check(&game));
    }

    // The node ran far enough to have made a null move. The block sits above
    // the move loop, so a node that searched a move is a node that passed the
    // block, and "no null move was made" is then a decision and not an early
    // return from somewhere higher up.
    void require_the_node_reached_its_move_loop() const
    {
      REQUIRE_MESSAGE(probe.move_count > 0,
                      "the drive returned before its move loop, so nothing "
                      "here is evidence about the null-move block");
    }
  };


  // One row of adocs/data/S165_defender_set.tsv.
  struct defender_row_t
  {
    std::string fen;
    int mated_in;
    int ply;
  };


  // The S165 defender set: 104 nodes proved to be losing to a forced mate,
  // built to measure the negative mate-band guard and read by nothing in
  // tests/ until now.
  //
  // Same reader as test_mate_carry.cpp's read_cases(): skip blanks, comments
  // and the header row, split on tabs.
  static std::vector<defender_row_t> read_defender_set()
  {
#ifndef CHESSO_SOURCE_DIR
#error "CHESSO_SOURCE_DIR is not defined; see tests/CMakeLists.txt"
#endif

    const std::string path =
        std::string(CHESSO_SOURCE_DIR) + "/adocs/data/S165_defender_set.tsv";

    std::ifstream file(path);
    REQUIRE_MESSAGE(file.good(), ("Cannot open " + path));

    std::vector<defender_row_t> rows;
    std::string line;

    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#') { continue; }

      std::vector<std::string> field;
      std::istringstream stream(line);
      std::string cell;

      while (std::getline(stream, cell, '\t')) {
        field.push_back(cell);
      }

      if (field.size() < 6 || field[0] == "fen") { continue; }

      rows.push_back({field[0], std::stoi(field[1]), std::stoi(field[2])});
    }

    return rows;
  }


  // Mutation: M01_nmp_in_check -- drop `!is_in_check &&` from the null-move
  // condition.
  //
  //   search: pruning and reduction guards
  //    an in-check node makes no null move
  //   REQUIRE( !probe.null_move_made )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t, "an in-check node makes no null move")
  {
    // 1.e4 c5 2.Nf3 d6 3.Bb5+. From a tool and not from the board (CLAUDE.md):
    // python-chess reports `is_valid() True`, `is_check() True`,
    // `is_checkmate() False`, 4 legal replies and 24 points of phase, so the
    // only null-move condition this position denies is the one under test.
    const std::string fen =
        "rnbqkbnr/pp2pppp/3p4/1Bp5/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 3";

    load(fen, 1);

    // The precondition the case is named for, and it is the inverse of the
    // guard: the node is in check, and it has somewhere to go, so the search
    // below is a real one.
    REQUIRE(is_check(&game));

    move_t buffer[MAX_MOVES];
    REQUIRE(legal_moves(&game, buffer) > 0);

    // Everything else the block wants, except `!is_in_check` itself. The
    // fixture helper asserts that the node is not in check, which is exactly
    // what this case denies, so the remaining conditions are asserted here.
    REQUIRE_EQ(NULL_DRIVE_DEPTH - 1 -
                   (NULL_MOVE_BASE + (NULL_DRIVE_DEPTH / NULL_MOVE_DIVISOR)),
               1);
    REQUIRE(PREV_MOVE != 0);
    REQUIRE(ORDINARY_BETA < MATE_MIN_LOCAL);
    REQUIRE(ORDINARY_BETA > -MATE_MIN_LOCAL);
    REQUIRE(game_phase(&game.board) > 0);

    // Reverse futility cannot pre-empt the block at ply 1, which is what makes
    // the move loop below evidence about the null move and nothing else.
    REQUIRE(RFP_MIN_PLY > 1);

    negamax_probed(ORDINARY_BETA - 1, ORDINARY_BETA, NULL_DRIVE_DEPTH, 1, &game,
                   &state, PREV_MOVE, false);

    require_the_node_reached_its_move_loop();
    REQUIRE(!probe.null_move_made);
  }


  // Mutation: M03_nmp_zugzwang -- `game_phase(&game->board) > 0` becomes
  // `true`.
  //
  //   search: pruning and reduction guards
  //    a node with only kings and pawns makes no null move
  //   REQUIRE( !probe.null_move_made )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "a node with only kings and pawns makes no null move")
  {
    // python-chess: `is_valid() True`, `is_check() False`, 8 legal moves, and
    // the only piece types on the board are kings and pawns -- phase_value in
    // src/eval_tables.hpp scores both 0, which is the zugzwang case the guard
    // exists for.
    const std::string fen = "6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1";

    load(fen, 1);

    // The inverse of the guard.
    REQUIRE_EQ(game_phase(&game.board), 0);

    move_t buffer[MAX_MOVES];
    REQUIRE(legal_moves(&game, buffer) > 0);

    REQUIRE_EQ(NULL_DRIVE_DEPTH - 1 -
                   (NULL_MOVE_BASE + (NULL_DRIVE_DEPTH / NULL_MOVE_DIVISOR)),
               1);
    REQUIRE(PREV_MOVE != 0);
    REQUIRE(ORDINARY_BETA < MATE_MIN_LOCAL);
    REQUIRE(ORDINARY_BETA > -MATE_MIN_LOCAL);
    REQUIRE(!is_check(&game));
    REQUIRE(RFP_MIN_PLY > 1);

    negamax_probed(ORDINARY_BETA - 1, ORDINARY_BETA, NULL_DRIVE_DEPTH, 1, &game,
                   &state, PREV_MOVE, false);

    require_the_node_reached_its_move_loop();
    REQUIRE(!probe.null_move_made);
  }


  // Mutation: S191-N01 -- drop `prev_move != 0 &&` from the null-move
  // condition.
  //
  //   search: pruning and reduction guards
  //    a node whose parent already passed makes no null move
  //   REQUIRE( !probe.null_move_made )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "a node whose parent already passed makes no null move")
  {
    // The Italian after 3...Bc5. python-chess: `is_valid() True`,
    // `is_check() False`, 31 legal moves, 24 points of phase.
    const std::string fen =
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 4 3";

    load(fen, 1);

    // Zero is what negamax passes its own null child, so this drive is the
    // node one pass below another. Every other condition holds.
    require_null_move_preconditions(NULL_DRIVE_DEPTH, ORDINARY_BETA, PREV_MOVE);
    REQUIRE(RFP_MIN_PLY > 1);

    negamax_probed(ORDINARY_BETA - 1, ORDINARY_BETA, NULL_DRIVE_DEPTH, 1, &game,
                   &state, 0, false);

    require_the_node_reached_its_move_loop();
    REQUIRE(!probe.null_move_made);
  }


  // Mutation: S191-N02 -- drop `beta < MATE_MIN &&` from the null-move
  // condition.
  //
  //   search: pruning and reduction guards
  //    a node at the positive edge of the mate band makes no null move
  //   REQUIRE( !probe.null_move_made )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(
      guard_fixture_t,
      "a node at the positive edge of the mate band makes no null move")
  {
    const std::string fen =
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 4 3";

    load(fen, 1);

    // MATE_MIN itself, the first beta the guard excludes: `beta < MATE_MIN` is
    // false here and true one point below. A fail-high against a bound this
    // high is a claim about a mate, and a pass never proves one.
    const int beta = MATE_MIN_LOCAL;

    REQUIRE_EQ(NULL_DRIVE_DEPTH - 1 -
                   (NULL_MOVE_BASE + (NULL_DRIVE_DEPTH / NULL_MOVE_DIVISOR)),
               1);
    REQUIRE(PREV_MOVE != 0);
    REQUIRE(beta > -MATE_MIN_LOCAL);
    REQUIRE(game_phase(&game.board) > 0);
    REQUIRE(!is_check(&game));
    REQUIRE(RFP_MIN_PLY > 1);

    negamax_probed(beta - 1, beta, NULL_DRIVE_DEPTH, 1, &game, &state,
                   PREV_MOVE, false);

    require_the_node_reached_its_move_loop();
    REQUIRE(!probe.null_move_made);
  }


  // Mutation: M02_nmp_mate_band_neg -- drop `beta > -MATE_MIN` from the
  // null-move condition, which is the S165 guard.
  //
  //   search: pruning and reduction guards
  //    no defender node inside the mate band makes a null move
  //   REQUIRE( violations.empty() )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "no defender node inside the mate band makes a null move")
  {
    const std::vector<defender_row_t> rows = read_defender_set();

    // Golden, DEC-142. Re-derive with
    //   grep -vc '^#' adocs/data/S165_defender_set.tsv
    // which counts the header row as well, or by regenerating the file with
    //   ~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py generate
    REQUIRE_MESSAGE(rows.size() == 104,
                    ("adocs/data/S165_defender_set.tsv is not the tracked set "
                     "any more: " +
                     std::to_string(rows.size()) + " rows, expected 104"));

    std::string violations;
    size_t scored = 0;

    for (const defender_row_t& row : rows) {
      load(row.fen, row.ply);

      // A mate against the side to move in k is 2k plies away, which is what
      // adocs/data/S165_nmp_defender_sweep.py's docstring states and what the
      // mated_in column counts. `s` is that score seen from this node, and
      // beta one point above it is the bound a search proving the mate would
      // pass down.
      const int s = -(MATE_MAX_LOCAL - row.ply - 2 * row.mated_in);
      const int beta = s + 1;

      REQUIRE_MESSAGE(beta <= -MATE_MIN_LOCAL,
                      (row.fen + ": beta " + std::to_string(beta) +
                       " is not inside the mate band"));
      REQUIRE_MESSAGE(row.ply > 0, (row.fen + ": ply 0 has no null move"));
      REQUIRE_MESSAGE(!is_check(&game),
                      (row.fen + " is in check, so the block stops on the "
                                 "wrong guard"));
      REQUIRE_MESSAGE(game_phase(&game.board) > 0,
                      (row.fen + " has no phase, so the block stops on the "
                                 "wrong guard"));

      negamax_probed(beta - 1, beta, NULL_DRIVE_DEPTH,
                     static_cast<size_t>(row.ply), &game, &state, PREV_MOVE,
                     false);

      scored++;

      if (probe.null_move_made) { violations += row.fen + "\n"; }
    }

    REQUIRE_EQ(scored, rows.size());

    // Zero tolerance. One pass at a node already proved lost is one mate the
    // engine can talk itself out of seeing, which is what S165 measured.
    REQUIRE_MESSAGE(
        violations.empty(),
        ("defender nodes that passed inside the mate band:\n" + violations));
  }


  // Mutation: M04_nmp_mate_artifact -- the fail-high returns `null_score`
  // instead of the bound.
  //
  //   search: pruning and reduction guards
  //    a null-move fail-high against a mate returns the bound
  //   REQUIRE( score == 100 )
  //   values: REQUIRE( 48998 == 100 )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "a null-move fail-high against a mate returns the bound")
  {
    // White mates in one whatever Black replies, so the pass still fails high
    // on a mate score. From a tool (CLAUDE.md): python-chess reports
    // `is_valid() True`, `is_check() False`, and after a null move Black's
    // only legal moves are h6 and h5, each of which is followed by four mates
    // in one -- so the child of the pass reaches a mate at any depth that
    // holds two plies.
    const std::string fen = "k7/2Q4p/K7/8/8/8/8/8 w - - 0 1";

    // Reduction 4 and a child two plies deep: the mating move lands at depth 0
    // on quiescence's in-check-with-no-moves return, which the case "mate is
    // recognised at depth zero" already holds. NULL_DRIVE_DEPTH's child is one
    // ply and cannot see it.
    const int depth = 7;
    const int reduction = NULL_MOVE_BASE + (depth / NULL_MOVE_DIVISOR);

    // Not ORDINARY_BETA, and the reason is a measurement rather than a
    // preference. The mating node sits three plies below this one, which is
    // exactly RFP_MIN_PLY, and it inherits this beta: at 100 reverse futility
    // fires there on a static score a queen up, and 837 came back in place of
    // a mate -- the precondition below read `REQUIRE( 742 >= 48000 )`. Any
    // beta above that static score plus RFP_MARGIN keeps the node searching,
    // and this one clears it by a factor of forty while staying inside the
    // mate band the null-move guard is about. It is the "mate deeper than
    // ply 3 can still be missed for an iteration" that the reverse-futility
    // comment in src/search.cpp already names, met head on.
    const int beta = 40000;

    load(fen, 1);

    REQUIRE_EQ(depth - 1 - reduction, 2);
    REQUIRE(PREV_MOVE != 0);
    REQUIRE(beta < MATE_MIN_LOCAL);
    REQUIRE(beta > -MATE_MIN_LOCAL);
    REQUIRE(game_phase(&game.board) > 0);
    REQUIRE(!is_check(&game));
    REQUIRE(RFP_MIN_PLY > 1);

    // The precondition, taken from the engine and not assumed: the search the
    // block is about to run really does come back with a mate score. Driven
    // by hand here, so what the case asserts below is what the block does with
    // that score and not whether the score turns up.
    make_null_move(&game);

    const int null_score = -negamax(-beta, -beta + 1, depth - 1 - reduction, 2,
                                    &game, &state, 0, false);

    unmake_null_move(&game);

    REQUIRE(null_score >= MATE_MIN_LOCAL);
    REQUIRE(null_score >= beta);

    // The table now holds the hand-driven subtree; wipe it and start again so
    // the drive below searches rather than reads.
    load(fen, 1);

    const int score = negamax_probed(beta - 1, beta, depth, 1, &game, &state,
                                     PREV_MOVE, false);

    REQUIRE(probe.null_move_made);

    // The bound, not the mate. A mate out of a pass is an artefact of the
    // pass: nobody forced it, and publishing it puts a mate score into a
    // parent's window that no line reaches.
    REQUIRE_EQ(score, beta);
  }


  // Mutation: S191-N03 -- drop `!is_in_check &&` from the reverse-futility
  // condition.
  //
  //   search: pruning and reduction guards
  //    reverse futility does not fire at a node in check
  //   REQUIRE( !probe.rfp_cutoff )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "reverse futility does not fire at a node in check")
  {
    const std::string fen =
        "rnbqkbnr/pp2pppp/3p4/1Bp5/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 3";

    const int depth = 3;

    load(fen, RFP_MIN_PLY);

    REQUIRE(is_check(&game));

    // In check there is no static score: negamax puts TT_EVAL_NONE in the slot
    // rather than pricing a position whose king is attacked. That sentinel is
    // what makes the guard removable without effect at an ordinary beta -- the
    // comparison the mutant would then run is
    // `TT_EVAL_NONE - RFP_MARGIN * depth >= beta`, and it is false for every
    // beta above -32957. This beta is below it, which is what gives the case
    // something to kill.
    const int beta = -40000;

    REQUIRE(TT_EVAL_NONE - (RFP_MARGIN * depth) >= beta);

    // The rest of the reverse-futility condition, so the guard under test is
    // the only one that can stop it.
    REQUIRE(static_cast<int>(RFP_MIN_PLY) >= RFP_MIN_PLY);
    REQUIRE(depth <= RFP_MAX_DEPTH);
    REQUIRE(beta < MATE_MIN_LOCAL);
    REQUIRE(beta > -MATE_MIN_LOCAL);

    negamax_probed(beta - 1, beta, depth, static_cast<size_t>(RFP_MIN_PLY),
                   &game, &state, 0, false);

    REQUIRE(!probe.rfp_cutoff);

    // The same fact from the other side: reverse futility returns before the
    // node searches anything, so a node that searched a child did not take it.
    REQUIRE(state.explored_nodes > 1);
  }


  // Mutation: S191-N04 -- drop `!is_pv &&` from the reverse-futility
  // condition.
  //
  //   search: pruning and reduction guards
  //    reverse futility does not fire at a PV node
  //   REQUIRE( !probe.rfp_cutoff )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "reverse futility does not fire at a PV node")
  {
    const std::string fen =
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 4 3";

    const int depth = 3;

    load(fen, RFP_MIN_PLY);

    REQUIRE(!is_check(&game));

    // The table was just wiped, so negamax has no stored evaluation to reuse
    // and its static_eval is this call. Beta is set to the exact value that
    // makes `static_eval - margin >= beta` true by equality, so the only thing
    // between this node and a cutoff is the PV guard.
    const int beta = evaluate(&game.board) - (RFP_MARGIN * depth);

    REQUIRE(depth <= RFP_MAX_DEPTH);
    REQUIRE(beta < MATE_MIN_LOCAL);
    REQUIRE(beta > -MATE_MIN_LOCAL);

    negamax_probed(beta - 1, beta, depth, static_cast<size_t>(RFP_MIN_PLY),
                   &game, &state, 0, true);

    REQUIRE(!probe.rfp_cutoff);
    REQUIRE(state.explored_nodes > 1);
  }


  // Mutation: S191-N05 -- drop `depth <= RFP_MAX_DEPTH &&` from the
  // reverse-futility condition.
  //
  //   search: pruning and reduction guards
  //    reverse futility does not fire above its depth bound
  //   REQUIRE( !probe.rfp_cutoff )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "reverse futility does not fire above its depth bound")
  {
    // Four blocked pawn pairs two files apart, so no pawn can advance and none
    // can capture: python-chess reports `is_valid() True`, `is_check() False`
    // and 5 legal moves, all of them king moves. A position this narrow is
    // what makes a search one ply past the bound affordable.
    const std::string fen = "4k3/8/8/p1p1p1p1/P1P1P1P1/8/8/4K3 w - - 0 1";

    const int depth = RFP_MAX_DEPTH + 1;

    load(fen, RFP_MIN_PLY);

    REQUIRE(!is_check(&game));

    const int beta = evaluate(&game.board) - (RFP_MARGIN * depth);

    REQUIRE(beta < MATE_MIN_LOCAL);
    REQUIRE(beta > -MATE_MIN_LOCAL);

    // The cutoff is decided before the node searches anything, so a budget
    // bounds the cost of the guarded run without touching what is observed.
    // It has to be more than one node, or the limit rather than the guard is
    // what ends the drive.
    state.node_limit = 200000;

    negamax_probed(beta - 1, beta, depth, static_cast<size_t>(RFP_MIN_PLY),
                   &game, &state, 0, false);

    REQUIRE(!probe.rfp_cutoff);
    REQUIRE(state.explored_nodes > 1);
  }


  // Mutation: S191-N06 -- drop `beta > -MATE_MIN` from the reverse-futility
  // condition.
  //
  //   search: pruning and reduction guards
  //    reverse futility does not fire inside the mate band
  //   REQUIRE( !probe.rfp_cutoff )
  //   values: REQUIRE( false )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "reverse futility does not fire inside the mate band")
  {
    const std::string fen =
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 4 3";

    const int depth = 3;

    load(fen, RFP_MIN_PLY);

    REQUIRE(!is_check(&game));

    // -MATE_MIN itself, the first beta the guard excludes. Only the negative
    // edge is constructible: the positive one needs a static score near 48000,
    // and evaluate_expensive() clamps the whole king-safety correction to
    // LAZY_EVAL_MARGIN, which is why src/search_params.hpp records that half of
    // the guard as one S145 measured inert.
    const int beta = -MATE_MIN_LOCAL;

    REQUIRE(evaluate(&game.board) - (RFP_MARGIN * depth) >= beta);
    REQUIRE(depth <= RFP_MAX_DEPTH);
    REQUIRE(beta < MATE_MIN_LOCAL);

    negamax_probed(beta - 1, beta, depth, static_cast<size_t>(RFP_MIN_PLY),
                   &game, &state, 0, false);

    REQUIRE(!probe.rfp_cutoff);
    REQUIRE(state.explored_nodes > 1);
  }


  // A window no move can beat without a forced mate. Every legal move then
  // fails low, the node runs its whole move loop, and the reduction each move
  // was searched with is the reduction the guards decided on -- nothing is cut
  // short and nothing is re-searched.
  static constexpr int FAIL_LOW_BETA = 5001;

  // The drive depth for the two reduction cases. The block's own bound is
  // `depth >= 3`, and one ply above it the reduction is clamped to
  // `child_depth - 1`, which is 1.
  static constexpr int LMR_DRIVE_DEPTH = 3;


  // Mutation: M07_lmr_captures -- drop `!is_capture &&` from the reduction
  // condition.
  //
  //   search: pruning and reduction guards
  //    a capture is not reduced
  //   REQUIRE( probe.reduction[k] == 0 )
  //   values: REQUIRE( 1 == 0 )
  TEST_CASE_FIXTURE(guard_fixture_t, "a capture is not reduced")
  {
    // The standard perft position 2. python-chess: `is_valid() True`,
    // `is_check() False`, 48 legal moves of which 8 are captures and none of
    // those is a promotion -- so the fourth move the ordering reaches is a
    // capture, which is the only way a capture is ever eligible for a
    // reduction at all.
    const std::string fen =
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    load(fen, 1);

    REQUIRE(!is_check(&game));

    move_t buffer[MAX_MOVES];
    const size_t legal_count = legal_moves(&game, buffer);

    negamax_probed(FAIL_LOW_BETA - 1, FAIL_LOW_BETA, LMR_DRIVE_DEPTH, 1, &game,
                   &state, 0, false);

    // The whole loop ran, so no move was skipped and no cutoff hid one.
    REQUIRE_EQ(static_cast<size_t>(probe.move_count), legal_count);

    // The first capture the ordering put past the reduction block's own
    // `legal_moves_counter > 3`.
    int k = -1;

    for (int i = 3; i < probe.move_count; ++i) {
      if (MOVE_CAPTURE(probe.moves[i]) != 0) {
        k = i;
        break;
      }
    }

    REQUIRE_MESSAGE(k >= 3,
                    "no capture was ordered past the first three "
                    "moves, so the guard decided nothing here");

    // A promotion is refused by its own clause of the same condition, which
    // would make the observation below ambiguous.
    REQUIRE_EQ(MOVE_PROMOTED(probe.moves[k]), TO_NONE);

    // The table would have reduced this move number at this depth. Without
    // this a reduction of zero says nothing: it could be the guard, or it
    // could be a reduction table that returns zero here.
    REQUIRE(search_lmr_reduction_probe(LMR_DRIVE_DEPTH, k + 1) > 0);

    REQUIRE_EQ(probe.reduction[k], 0);
  }


  // Mutation: M08_lmr_checks -- drop `!is_check_move` from the reduction
  // condition.
  //
  //   search: pruning and reduction guards
  //    a quiet move that gives check is not reduced
  //   REQUIRE( probe.reduction[k] == 0 )
  //   values: REQUIRE( 1 == 0 )
  TEST_CASE_FIXTURE(guard_fixture_t,
                    "a quiet move that gives check is not reduced")
  {
    // python-chess: `is_valid() True`, `is_check() False`, 45 legal moves, 4
    // of them captures and none a promotion, and exactly one quiet move that
    // gives check -- Bd7+, which is not mate. Four captures ahead of every
    // quiet is what puts the checking move past the block's own move-number
    // bound.
    const std::string fen =
        "1r2kb1r/pbn1pp1p/1q1p1n1p/1pP3Q1/4P3/P1P2NPB/RP3P1P/1N2K2R w Kk - 6 "
        "17";

    load(fen, 1);

    REQUIRE(!is_check(&game));

    move_t buffer[MAX_MOVES];
    const size_t legal_count = legal_moves(&game, buffer);

    negamax_probed(FAIL_LOW_BETA - 1, FAIL_LOW_BETA, LMR_DRIVE_DEPTH, 1, &game,
                   &state, 0, false);

    REQUIRE_EQ(static_cast<size_t>(probe.move_count), legal_count);

    // Which move gives check comes from the engine's own is_check() after its
    // own make_move(), the way the guard itself decides it -- never from a
    // move list read by eye (CLAUDE.md).
    int k = -1;

    for (int i = 0; i < probe.move_count; ++i) {
      const move_t move = probe.moves[i];

      if (MOVE_CAPTURE(move) != 0) { continue; }
      if (MOVE_PROMOTED(move) != TO_NONE) { continue; }

      REQUIRE(make_move(&game, move));
      const bool gives_check = is_check(&game);
      unmake_move(&game);

      if (gives_check) {
        k = i;
        break;
      }
    }

    REQUIRE_MESSAGE(k >= 0, "no quiet move here gives check");
    REQUIRE_MESSAGE(k >= 3,
                    "the checking move was ordered inside the first "
                    "three, where the move-number bound refuses the "
                    "reduction anyway");

    REQUIRE(search_lmr_reduction_probe(LMR_DRIVE_DEPTH, k + 1) > 0);

    REQUIRE_EQ(probe.reduction[k], 0);
  }


  // Mutation: M09_lmr_no_research -- the re-search is never run.
  //
  //   search: pruning and reduction guards
  //    a reduced move that beats alpha is searched again at full depth
  //   REQUIRE( researched > 0 )
  //   values: REQUIRE( 0 > 0 )
  TEST_CASE_FIXTURE(
      guard_fixture_t,
      "a reduced move that beats alpha is searched again at full depth")
  {
    const std::string fen =
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    // Wide enough that a move beating alpha does not also reach beta, so the
    // loop keeps running past the first one that does. A null window would end
    // the node on it instead.
    const int alpha = -30000;
    const int beta = 30000;

    // Deep enough for the reduction table to return more than zero on a late
    // move and for a late move to be worth more than the ordering thought.
    const int depth = 6;

    load(fen, 1);

    REQUIRE(!is_check(&game));

    // Zero as the previous move keeps the null-move block out of the drive,
    // so what the probe records below is the move loop's own arithmetic.
    negamax_probed(alpha, beta, depth, 1, &game, &state, 0, false);

    size_t reduced = 0;
    size_t researched = 0;

    for (int i = 0; i < probe.move_count; ++i) {
      if (probe.reduction[i] <= 0) { continue; }

      reduced++;

      if (probe.researched[i]) { researched++; }
    }

    // The precondition. With nothing reduced there is no re-search to owe and
    // the assertion below would hold over an empty set.
    REQUIRE_MESSAGE(reduced > 0,
                    "no move at this node was reduced, so the re-search rule "
                    "decided nothing here");

    // A reduced search that beats alpha has proved the reduction wrong and
    // nothing else. Believing its score is how a move gets played on a search
    // that was never run at the depth its score claims.
    REQUIRE(researched > 0);
  }
}
