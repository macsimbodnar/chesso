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

  TEST_CASE_FIXTURE(search_fixture_t, "a mated side reports mate in zero")
  {
    const search_t result = search_fen("7k/5Q1K/8/8/8/8/8/8 b - - 0 1", 3);

    REQUIRE(result.mate_found);
    REQUIRE_EQ(result.mate_in, 0);
    REQUIRE_EQ(result.best_move, 0);
    REQUIRE_EQ(result.pv.length, 0);
  }

  TEST_CASE_FIXTURE(search_fixture_t, "stalemate scores zero, not mate")
  {
    const search_t result = search_fen("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", 3);

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
      {"2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1",       a1, b2, TO_NONE,  "the king takes the loose queen"},
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
      {"8/P6k/8/8/8/8/8/4K3 w - - 0 1",          a7, a8, TO_QUEEN, "promote to a queen"},
      {"6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1",      a1, a8, TO_NONE,  "mate on the back rank"},
      {"r5k1/8/8/8/8/8/5PPP/6K1 b - - 0 1",      a8, a1, TO_NONE,  "mate on the back rank, black"},
    };
    // clang-format on

    for (const case_t& test : cases) {
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
  // times out. Depth 5 on this position costs a few hundred thousand nodes
  // today, so the budget leaves a wide margin.
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
    state.node_limit = 1000000;

    const search_t result = search(5, &game, &state);

    REQUIRE_FALSE(state.aborted);
    REQUIRE(result.best_move != 0);
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
  // already above beta stops at once and reports the static score.
  TEST_CASE_FIXTURE(search_fixture_t, "a quiet position stands pat")
  {
    // White is a rook up with nothing to capture.
    const std::string fen = "4k3/8/8/8/8/8/8/3RK3 w - - 0 1";

    REQUIRE(load_FEN(fen, &game));
    const int static_score = evaluate(&game.board);
    REQUIRE_EQ(static_score, 500);

    // Beta below the static score: the cutoff is immediate.
    REQUIRE_EQ(quiesce(fen, 0, 100), static_score);

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
    // Two pawns up, plus whatever the piece-square tables make of the squares
    // everything happens to be standing on.
    const int black_static = evaluate(&game.board);
    REQUIRE_EQ(black_static, 208);

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

    // A rook down, give or take where the piece-square tables put the kings.
    REQUIRE_EQ(quiesce(fen, -10000000, 10000000), -475);
  }

  // No legal reply to a check is mate, and quiescence has to say so on its
  // own: at depth zero it is the only thing that runs.
  TEST_CASE_FIXTURE(search_fixture_t, "mate is recognised at depth zero")
  {
    const search_t result = search_fen("7k/5Q1K/8/8/8/8/8/8 b - - 0 1", 0);

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
    // kings and the rook.
    REQUIRE_EQ(evaluate(&game.board), -482);

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
