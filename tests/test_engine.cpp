#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "transposition_table.hpp"
#include "uci.hpp"

// Defined in bitboard.cpp. Not in the header because nothing in the engine
// needs it at runtime - only the tests, to check the incremental hash.
hash_t compute_full_hash(game_t* game);


static game_t game;


struct engine_fixture_t
{
  engine_fixture_t() { initialize_game_const_data(&game); }
};


TEST_SUITE("engine: zobrist and unmake")
{
  // Walks the move tree and checks two invariants after every make/unmake:
  // the incrementally maintained hash equals a full recomputation, and unmake
  // restores the board bit for bit. A drift in either poisons the
  // transposition table and shows up as unexplainable moves much later.
  static void walk(game_t * g, int depth, const std::string& root_fen)
  {
    if (depth == 0) { return; }

    move_t moves[MAX_MOVES];
    const size_t count = generate_moves(game_tables(), &g->board, moves);

    for (size_t i = 0; i < count; ++i) {
      board_t before;
      memcpy(&before, &g->board, sizeof(board_t));

      if (!make_move(g, moves[i])) { continue; }

      REQUIRE_MESSAGE(g->board.hash == compute_full_hash(g),
                      ("incremental hash drifted after " +
                       print_move(moves[i]) + " from " + root_fen));

      walk(g, depth - 1, root_fen);

      unmake_move(g);

      REQUIRE_MESSAGE(memcmp(&before, &g->board, sizeof(board_t)) == 0,
                      ("unmake did not restore the board after " +
                       print_move(moves[i]) + " from " + root_fen));
    }
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "hash and board survive make/unmake")
  {
    // clang-format off
    const std::vector<std::pair<std::string, int>> positions = {
      {DEFAULT_POSITION, 4},
      {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3},
      {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4},
      {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3},
      {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3},
    };
    // clang-format on

    for (const auto& [fen, depth] : positions) {
      REQUIRE(load_FEN(fen, &game));

      REQUIRE_MESSAGE(game.board.hash == compute_full_hash(&game),
                      ("load_FEN produced a wrong hash for " + fen));

      walk(&game, depth, fen);
    }
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "equal positions hash equally")
  {
    // Reached by two different move orders, so the incremental hash must agree
    // with a position loaded straight from the FEN.
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    const std::vector<std::pair<index_t, index_t>> line = {
        {e2, e4}, {e7, e5}, {g1, f3}, {b8, c6}};

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

    const hash_t reached = game.board.hash;
    const std::string fen = generate_FEN(&game.board);

    REQUIRE(load_FEN(fen, &game));
    REQUIRE_EQ(game.board.hash, reached);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "unmake on an empty stack is a no-op")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    board_t before;
    memcpy(&before, &game.board, sizeof(board_t));

    unmake_move(&game);

    REQUIRE_EQ(memcmp(&before, &game.board, sizeof(board_t)), 0);
  }
}


TEST_SUITE("engine: repetition")
{
  TEST_CASE_FIXTURE(engine_fixture_t, "a shuffled knight repeats")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/N3K3 w - - 0 1", &game));

    // Na1-b3-a1 puts the side to move back on the same position.
    const std::vector<std::pair<index_t, index_t>> line = {
        {a1, b3}, {e8, d8}, {b3, a1}, {d8, e8}};

    for (const auto& [from, to] : line) {
      move_t moves[MAX_MOVES];
      const size_t count = legal_moves(&game, moves);

      for (size_t i = 0; i < count; ++i) {
        if (MOVE_FROM(moves[i]) == from && MOVE_TO(moves[i]) == to) {
          REQUIRE(make_move(&game, moves[i]));
          break;
        }
      }
    }

    REQUIRE(is_position_repeated(&game.history, &game.board));
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "an irreversible move clears the window")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    // Any pawn push resets the halfmove clock, so nothing can be a repetition.
    for (size_t i = 0; i < count; ++i) {
      if (MOVE_PIECE(moves[i]) != W_PAWN) { continue; }

      REQUIRE(make_move(&game, moves[i]));
      REQUIRE_EQ(game.board.halfmove_clock, 0);
      REQUIRE_FALSE(is_position_repeated(&game.history, &game.board));
      unmake_move(&game);
    }
  }
}


TEST_SUITE("engine: transposition table")
{
  TEST_CASE_FIXTURE(engine_fixture_t, "store then probe returns what went in")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);
    REQUIRE(table.entries != nullptr);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    const move_t move = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    tt_store_entry(&table, &game.board, 7, 123, TT_PV_NODE, move);

    const tt_entry_t* entry = tt_get_entry(&table, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->depth, 7);
    REQUIRE_EQ(entry->score, 123);
    REQUIRE_EQ(entry->best_move, move);
    REQUIRE_EQ(entry->type, TT_PV_NODE);

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "a different position misses")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const move_t move = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    tt_store_entry(&table, &game.board, 7, 123, TT_PV_NODE, move);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &game));
    REQUIRE(tt_get_entry(&table, &game.board) == nullptr);

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "reset clears every entry")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const move_t move = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    tt_store_entry(&table, &game.board, 7, 123, TT_PV_NODE, move);
    REQUIRE(tt_get_entry(&table, &game.board) != nullptr);

    tt_reset(&table);
    REQUIRE(tt_get_entry(&table, &game.board) == nullptr);

    // Reset must not drop the allocation.
    REQUIRE(table.entries != nullptr);
    REQUIRE(table.entry_count > 0);

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "deeper entries win within one search")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const move_t deep = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    const move_t shallow = NEW_MOVE(d2, d4, W_PAWN, TO_NONE, 0, 1, 0, 0);

    tt_store_entry(&table, &game.board, 9, 1, TT_PV_NODE, deep);
    tt_store_entry(&table, &game.board, 2, 2, TT_PV_NODE, shallow);

    const tt_entry_t* entry = tt_get_entry(&table, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->best_move, deep);
    REQUIRE_EQ(entry->depth, 9);

    tt_free(&table);
  }

  // Depth preferred, but not depth exclusive: a re-search of the same node at
  // the same depth carries the newer bound and has to win the slot. Only a
  // strictly shallower entry is refused.
  TEST_CASE_FIXTURE(engine_fixture_t,
                    "an equal depth entry replaces the older one")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const move_t first = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    const move_t second = NEW_MOVE(d2, d4, W_PAWN, TO_NONE, 0, 1, 0, 0);

    tt_store_entry(&table, &game.board, 5, 1, TT_ALPHA_NODE, first);
    tt_store_entry(&table, &game.board, 5, 2, TT_PV_NODE, second);

    const tt_entry_t* entry = tt_get_entry(&table, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->best_move, second);
    REQUIRE_EQ(entry->score, 2);
    REQUIRE_EQ(entry->type, TT_PV_NODE);

    // A shallower one still loses.
    tt_store_entry(&table, &game.board, 4, 3, TT_BETA_NODE, first);
    entry = tt_get_entry(&table, &game.board);
    REQUIRE_EQ(entry->best_move, second);

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "a new search may replace a deeper entry")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    const move_t deep = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    const move_t shallow = NEW_MOVE(d2, d4, W_PAWN, TO_NONE, 0, 1, 0, 0);

    tt_store_entry(&table, &game.board, 9, 1, TT_PV_NODE, deep);

    tt_new_search(&table);
    tt_store_entry(&table, &game.board, 2, 2, TT_PV_NODE, shallow);

    const tt_entry_t* entry = tt_get_entry(&table, &game.board);
    REQUIRE(entry != nullptr);
    REQUIRE_EQ(entry->best_move, shallow);

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t,
                    "generation never lands on the reserved 0")
  {
    transposition_table_t table = {};
    tt_resize(&table, 1);

    for (int i = 0; i < 600; ++i) {
      tt_new_search(&table);
      REQUIRE_NE(table.generation, 0);
    }

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "resize clamps and keeps a power of two")
  {
    transposition_table_t table = {};

    for (const size_t requested :
         {size_t(0), size_t(1), size_t(3), size_t(16), size_t(100000)}) {
      tt_resize(&table, requested);

      REQUIRE(table.entries != nullptr);
      REQUIRE(table.entry_count > 0);
      REQUIRE_EQ(table.index_mask, table.entry_count - 1);

      // Power of two, so masking is a valid substitute for a modulo.
      REQUIRE_EQ(table.entry_count & table.index_mask, 0);

      const size_t bytes = table.entry_count * sizeof(tt_entry_t);
      REQUIRE(bytes <= size_t(TT_MAX_MB) * 1024 * 1024);
    }

    tt_free(&table);
  }

  TEST_CASE_FIXTURE(engine_fixture_t, "an unallocated table is inert")
  {
    transposition_table_t table = {};

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));

    // Must not dereference the null entries pointer.
    REQUIRE(tt_get_entry(&table, &game.board) == nullptr);
    tt_store_entry(&table, &game.board, 1, 1, TT_PV_NODE,
                   NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0));
    tt_reset(&table);
    tt_free(&table);
  }
}


TEST_SUITE("engine: move stack limits")
{
  // make_move used to assert after writing, so an overlong game corrupted
  // memory in a release build instead of refusing the move.
  TEST_CASE_FIXTURE(engine_fixture_t,
                    "make_move refuses once the stack is full")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/N3K2N w - - 0 1", &game));

    size_t applied = 0;

    while (true) {
      move_t moves[MAX_MOVES];
      const size_t count = generate_moves(game_tables(), &game.board, moves);
      REQUIRE(count > 0);

      bool made = false;
      for (size_t i = 0; i < count; ++i) {
        if (make_move(&game, moves[i])) {
          made = true;
          applied++;
          break;
        }
      }

      if (!made) { break; }

      REQUIRE(applied < HISTORY_MAX_SIZE + 16);
      if (applied >= HISTORY_MAX_SIZE + 8) { break; }
    }

    // The guard, not an assert or a crash, is what stops the loop.
    REQUIRE(applied < HISTORY_MAX_SIZE);
    REQUIRE(game.history.size < HISTORY_MAX_SIZE);
  }
}


TEST_SUITE("engine: uci layer")
{
  // These reach into the UCI translation unit directly. Protocol level
  // conformance is covered by test_uci.sh (fastchess --compliance); what is
  // here are the regressions that a compliance run does not exercise.

  // S089 split the single budget into a soft limit, which decides whether to
  // begin another iteration, and a hard limit, which is what a timer is armed
  // at and what stops the search inside one. The hard limit is the dangerous
  // half: a forfeit is a whole point rather than noise.
  TEST_CASE("the hard limit never exceeds the clock")
  {
    struct case_t
    {
      int remaining;
      int increment;
      int movestogo;  // 0 is sudden death - the GUI sent no [movestogo]
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {100000, 1000, 20},
      {100000, 0,    20},
      {1000,   0,    20},
      {1000,   0,    1},
      {100,    0,    1},
      {60,     0,    1},
      {51,     0,    40},
      {300000, 5000, 1},
      // Sudden death, which is what a match actually plays. Before S089 these
      // reached the same formula with a fabricated movestogo of 20 in them.
      {600000, 0,    0},
      {100000, 1000, 0},
      {10000,  200,  0},
      {1000,   0,    0},
      {100,    0,    0},
      // S036. Below 2 * MOVE_OVERHEAD_MS the cap goes negative and only the
      // floor is left; at remaining == 1 the floor halves to zero too. A zero
      // budget arms no timer, so the search never ends.
      {50,     0,    1},
      {2,      0,    1},
      {1,      0,    20},
      {1,      100,  1},
      {1,      0,    0},
      {2,      100,  0},
    };
    // clang-format on

    for (const case_t& test : cases) {
      const search_time_budget_t budget = compute_search_time_budget(
          test.remaining, test.increment, test.movestogo);

      const std::string title = "remaining " + std::to_string(test.remaining) +
                                " inc " + std::to_string(test.increment) +
                                " movestogo " + std::to_string(test.movestogo);

      REQUIRE_MESSAGE(budget.soft_ms > 0, title);
      REQUIRE_MESSAGE(budget.hard_ms > 0, title);

      // The soft limit decides whether to begin an iteration the hard limit
      // will then cut off, so above the hard limit it decides nothing.
      REQUIRE_MESSAGE(budget.soft_ms <= budget.hard_ms, title);

      // Never more than the clock.
      REQUIRE_MESSAGE(budget.hard_ms <= test.remaining, title);

      // Strictly less wherever a clock that size leaves room for it: something
      // has to be left for getting the move out of the door, or the flag falls
      // while the engine is still talking. At remaining == 1 no positive
      // budget is strictly less, and the flag falls either way -- answering
      // late is a lost game, not answering hangs the match. S036.
      if (test.remaining > 1) {
        REQUIRE_MESSAGE(budget.hard_ms < test.remaining, title);
      }

      // And a floor, so a nearly empty clock still buys a real search rather
      // than a move picked at depth one.
      const int floor_ms = std::max(1, std::min(50, test.remaining / 2));

      REQUIRE_MESSAGE(budget.hard_ms >= floor_ms, title);

      // The whole move overhead, on every clock large enough to hold both it
      // and the floor. Below that the floor is the only thing left and it wins
      // on purpose, which is the S036 rows above.
      if (test.remaining - MOVE_OVERHEAD_MS >= floor_ms) {
        REQUIRE_MESSAGE(budget.hard_ms <= test.remaining - MOVE_OVERHEAD_MS,
                        title);
      }
    }
  }


  // S089. Sudden death used to be handled by pretending the GUI had sent
  // [movestogo 20], so the engine played every clock as though a time control
  // boundary sat twenty moves out. What replaces it is a share of what is
  // left, and the property that separates the two is the one below: a share is
  // the same fraction of any clock and implies no boundary at all.
  TEST_CASE("a sudden-death allocation is a share of the clock")
  {
    // Zero increment throughout, so every number here is the clock alone. With
    // an increment folded in the ratio would not be constant and the case
    // would prove nothing about the share.
    const std::vector<int> clocks = {800000, 400000, 200000, 100000, 50000};

    std::vector<double> shares;

    for (const int clock : clocks) {
      const search_time_budget_t budget =
          compute_search_time_budget(clock, 0, 0);

      shares.push_back(static_cast<double>(budget.hard_ms) / clock);
    }

    // Precondition: there are two shares to compare.
    REQUIRE(shares.size() >= 2);

    for (size_t i = 1; i < shares.size(); ++i) {
      REQUIRE_MESSAGE(
          std::abs(shares[i] - shares[0]) < 0.001,
          ("the share moved with the clock: " + std::to_string(shares[0]) +
           " then " + std::to_string(shares[i])));
    }

    // And it is small enough that one move cannot drain the clock, whatever
    // the settings are moved to. This is the safety half of the same property.
    REQUIRE(shares[0] > 0.0);
    REQUIRE(shares[0] < 0.25);
  }


  // S089. The two factors, held against their own arithmetic. What this case
  // cannot show is that the search calls the function at all, which is what
  // the loop case below is for.
  TEST_CASE("the time scale moves with stability and with a falling score")
  {
    // Precondition and the point every comparison below is measured from.
    REQUIRE(search_time_scale_percent(0, 0) == 100);

    // A best move that has survived an iteration buys less time, and each
    // further iteration that keeps it buys a little less again.
    REQUIRE(TM_STABILITY_PERCENT > 0);
    CHECK(search_time_scale_percent(1, 0) < 100);
    CHECK(search_time_scale_percent(2, 0) < search_time_scale_percent(1, 0));

    // The discount stops counting at TM_STABILITY_MAX rather than running on
    // to zero over the depths a real search reaches.
    CHECK_EQ(search_time_scale_percent(TM_STABILITY_MAX, 0),
             search_time_scale_percent(TM_STABILITY_MAX + 1, 0));
    CHECK_EQ(search_time_scale_percent(TM_STABILITY_MAX, 0),
             search_time_scale_percent(MAX_DEPTH, 0));

    // A score that fell buys more, up to the whole grant at TM_FALLING_MAX_CP.
    REQUIRE(TM_FALLING_PERCENT > 0);
    CHECK(search_time_scale_percent(0, TM_FALLING_MAX_CP) > 100);
    CHECK(search_time_scale_percent(0, TM_FALLING_MAX_CP) >
          search_time_scale_percent(0, TM_FALLING_MAX_CP / 2));
    CHECK_EQ(search_time_scale_percent(0, TM_FALLING_MAX_CP),
             100 + TM_FALLING_PERCENT);

    // And the grant stops there. A mate score on the other side of a trade is
    // a fall of tens of thousands of centipawns and must not buy time without
    // bound.
    CHECK_EQ(search_time_scale_percent(0, TM_FALLING_MAX_CP),
             search_time_scale_percent(0, 100000));

    // The two are independent settings whose product no single range can
    // bound, so the floor is what keeps the soft limit off zero. At the
    // shipping defaults it does not bind; the case that builds the
    // precondition for it lives in tests/test_search_params.cpp, which is the
    // binary that can move a parameter.
    CHECK(search_time_scale_percent(MAX_DEPTH, TM_FALLING_MAX_CP) > 0);
  }


  TEST_CASE("uci_init leaves the engine on the start position")
  {
    uci_init();

    REQUIRE(uci_is_running());
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));

    uci_shutdown();
  }

  TEST_CASE("position and moves are applied")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos moves e2e4 e7e5 g1f3");
    }

    REQUIRE_EQ(
        generate_FEN(&uci_game()->board),
        std::string(
            "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"));

    uci_shutdown();
  }

  TEST_CASE("a malformed fen leaves the previous position alone")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line(
          "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a9 "
          "0 1");
    }

    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));

    uci_shutdown();
  }

  TEST_CASE("unknown and empty input is ignored")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("");
      uci_process_line("   ");
      uci_process_line("not_a_command with args");
      uci_process_line("position startpos");
    }

    REQUIRE(uci_is_running());
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));

    uci_shutdown();
  }

  TEST_CASE("uci reports the options the GUI needs")
  {
    uci_init();

    stdout_capture_t capture;
    uci_process_line("uci");

    REQUIRE(capture.contains("id name Chesso"));
    REQUIRE(capture.contains("option name Hash type spin"));
    REQUIRE(capture.contains("option name Threads type spin"));
    REQUIRE(capture.contains("option name Use Book type check"));
    REQUIRE(capture.contains("uciok"));

    uci_shutdown();
  }

  TEST_CASE("first_legal_move agrees with the generator")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
    }

    const move_t fallback = first_legal_move();
    REQUIRE(fallback != 0);

    // uci_game() hands back a const view, so copy it to replay the move.
    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE(move_is_legal(&game, fallback));

    uci_shutdown();
  }

  // Terminal means no legal move, and there are exactly two ways to get there:
  // mate and stalemate. Both are here, because the count alone does not say
  // which one it got, and an engine that stopped distinguishing them would
  // still return 0 for both.
  //
  // This loaded 7k/5Q1K/8/8/8/8/8/8 b until S070. The only attacker of the
  // black king there was the white king on h7 - adjacent kings, a position no
  // legal game reaches. The case passed on a board that could not exist, which
  // is why the legality precondition below runs before the count and not after
  // it. 2026-08-14_test_review-F02, still open in this file as
  // 2026-08-16_plan_review-F06.
  //
  // Both replacements were chosen by tool, not by reading the board:
  //
  //   $ stockfish
  //   position fen 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1
  //   go depth 5
  //   info depth 0 score mate 0
  //   bestmove (none)
  //   position fen 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1
  //   go depth 5
  //   info depth 0 score cp 0
  //   bestmove (none)
  //
  //   python-chess, both positions: status Status.VALID, legal_moves 0.
  //   7k/6Q1/6K1 is_checkmate True, is_stalemate False.
  //   7k/5Q2/6K1 is_checkmate False, is_stalemate True.
  TEST_CASE("first_legal_move reports nothing in a terminal position")
  {
    struct terminal_case_t
    {
      const char* fen;
      bool in_check;  // true for the mate, false for the stalemate
    };

    const terminal_case_t cases[] = {
        {"7k/6Q1/6K1/8/8/8/8/8 b - - 0 1", true},
        {"7k/5Q2/6K1/8/8/8/8/8 b - - 0 1", false},
    };

    for (const terminal_case_t& terminal : cases) {
      // doctest stringifies a const char* as its address, so the FEN has to
      // reach the message as a std::string or a failure names nothing.
      const std::string fen(terminal.fen);

      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line("position fen " + fen);
      }

      // uci_game() hands back a const view, so copy it to run the null move
      // position_is_reachable() needs.
      memcpy(&game, uci_game(), sizeof(game_t));

      REQUIRE_MESSAGE(position_is_reachable(&game),
                      (fen + " is not a position a legal game can reach"));
      REQUIRE_MESSAGE(is_check(&game) == terminal.in_check,
                      (fen + " is not in check as expected"));

      REQUIRE_MESSAGE(first_legal_move() == 0, (fen + " has a legal move"));

      uci_shutdown();
    }
  }

  // Regression: a one node budget aborts before the first root move finishes.
  // This used to leave best_move at zero and print "bestmove 0000".
  TEST_CASE("a one node search still answers with a legal move")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
    }

    uci_search_options_t options = {};
    options.depth = MAX_DEPTH;
    options.nodes = 1;

    uci_search_result_t result;
    {
      // The search reports its info lines on stdout; keep them out of the
      // test output.
      stdout_capture_t capture;
      result = iterative_deepening_search(options);
    }

    REQUIRE(result.best_move != 0);

    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE(move_is_legal(&game, result.best_move));

    uci_shutdown();
  }

  // Regression: a [go] with no depth, no nodes and a clock already at zero used
  // to leave the search unbounded, so the engine never answered at all.
  TEST_CASE("a search with no limit is still bounded")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
    }

    uci_search_options_t options = {};
    options.depth = MAX_DEPTH;
    options.nodes = 0;
    options.movetime_ms = 200;
    options.search_time_ms = 200;

    const auto start = std::chrono::steady_clock::now();

    uci_search_result_t result;
    {
      stdout_capture_t capture;
      result = iterative_deepening_search(options);
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    REQUIRE(result.best_move != 0);

    // The soft limit is checked between iterations, so allow a generous
    // margin over the 200ms budget while still catching an unbounded search.
    REQUIRE(elapsed < 30000);

    uci_shutdown();
  }

  // Regression, S036: [go wtime 1] computed a zero budget, which armed no
  // timer, and with no depth and no node limit nothing else bounded the search.
  // Measured before the fix: 30s, depth 19, 137396943 nodes, no bestmove.
  TEST_CASE("a one millisecond clock answers without a stop")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
    }

    // The watchdog is what keeps a failure a failure instead of a hang: an
    // unbounded search would otherwise run until CTest kills the binary, and a
    // killed run reports nothing about which assertion was missed. It fires
    // only if the answer never came, so on a working engine no stop is ever
    // sent - which is the property under test.
    std::atomic_bool search_finished(false);
    std::atomic_bool watchdog_fired(false);

    std::vector<std::string> lines;

    {
      stdout_capture_t capture;

      uci_process_line("go wtime 1 btime 1");

      std::thread watchdog([&search_finished, &watchdog_fired]() {
        for (int waited = 0; waited < 300 && !search_finished; ++waited) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (!search_finished) {
          watchdog_fired = true;
          uci_process_line("stop");
        }
      });

      uci_wait_for_search();
      search_finished = true;
      watchdog.join();

      lines = capture.lines();
    }

    REQUIRE_FALSE(watchdog_fired);

    std::string best_move_line;
    for (const std::string& line : lines) {
      if (line.rfind("bestmove ", 0) == 0) { best_move_line = line; }
    }

    REQUIRE_FALSE(best_move_line.empty());
    REQUIRE_NE(best_move_line, std::string("bestmove 0000"));

    uci_shutdown();
  }


  // S089. search_time_scale_percent() is a pure function and the search could
  // compute it correctly on every iteration and never look at the answer, so
  // what is pinned here is the loop: the stability it counted, the fall it
  // measured and the scale it ended on have to be the same three numbers the
  // function relates.
  //
  // Both runs are fixed-depth with no clock and no node budget, so nothing
  // arms a timer, the iteration count is not whatever the machine got through,
  // and every number below is the same on any machine.
  TEST_CASE("the iteration loop scales its soft limit by what the search found")
  {
    struct probe_t
    {
      int stability;
      int drop;
      int scale;
    };

    auto probe = [](const std::string& position, int depth,
                    bool scale_time) -> probe_t {
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line(position);

        // [position] stops whatever was running, and [go] is the only path
        // that clears the stop flag again - iterative_deepening_search
        // documents that its caller has cleared it before the timer is armed.
        // Without this the direct call below aborts after its first iteration.
        // A depth-limited [go] arms no timer, so the flag stays clear.
        uci_process_line("go depth 1");
        uci_wait_for_search();
      }

      uci_search_options_t options = {};
      options.depth = depth;
      options.scale_time = scale_time;

      uci_search_result_t result;
      {
        stdout_capture_t capture;
        result = iterative_deepening_search(options);
      }

      REQUIRE(result.best_move != 0);

      const probe_t out = {uci_last_best_move_stability(),
                           uci_last_score_drop_cp(),
                           uci_last_time_scale_percent()};

      uci_shutdown();

      return out;
    };

    SUBCASE("a settled best move ends up with less time than the allocation")
    {
      const std::string position =
          "position fen r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/"
          "RNB1K1NR b KQkq - 3 3";

      const probe_t scaled = probe(position, 8, true);

      // Preconditions. Without a run of iterations that kept the same move
      // there is no discount to find, and with a fall in the last one the
      // discount would not be the only thing moving the scale.
      REQUIRE(scaled.stability > 0);
      REQUIRE(scaled.drop == 0);

      CHECK_EQ(scaled.scale,
               search_time_scale_percent(scaled.stability, scaled.drop));
      CHECK(scaled.scale < 100);

      // And the search does not get to move a time the GUI named. Same
      // position, same depth, same two inputs - the loop still records them -
      // and no scaling, which is what [go movetime] and the no-limit fallback
      // ask for. The run above is what makes this one non-vacuous: without it
      // a scale that never moved at all would satisfy it.
      const probe_t fixed = probe(position, 8, false);

      REQUIRE(fixed.stability == scaled.stability);
      REQUIRE(fixed.drop == scaled.drop);
      CHECK_EQ(fixed.scale, 100);
    }

    SUBCASE("a score that fell buys time back")
    {
      // The score at depth 8 here is 29 centipawns below the score at depth 7,
      // and the best move has been stable for three iterations. The fall has
      // to outweigh that discount or it is not reaching the scale at all: 14
      // points of grant against 12 of discount, so the scale lands at 102.
      //
      // Depth 6 until S094, where the score fell 62 between depths 5 and 6.
      // The quiescence transposition probe changed the scores the search
      // reports and this position no longer falls there -- -68 at both depths.
      // Re-targeted rather than relaxed: same position, same assertions, a
      // depth at which the precondition the case is about is really present.
      // Measured at S094's first commit: cp -34 -34 -68 -59 -68 -68 -80 -109
      // for depths 1 to 8, best move e2a6 from depth 5 on.
      const probe_t scaled = probe(
          "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/"
          "R3K2R w KQkq - 0 1",
          8, true);

      // Precondition: the score actually fell. On a loop that passed a
      // constant zero for the fall this is what goes red.
      REQUIRE(scaled.drop > 0);

      CHECK_EQ(scaled.scale,
               search_time_scale_percent(scaled.stability, scaled.drop));

      // Against the same position's own stability, so the comparison isolates
      // the fall rather than reading a number that a settled move would have
      // produced anyway.
      CHECK(scaled.scale > search_time_scale_percent(scaled.stability, 0));
    }
  }

  // Regression, S037: the info line carried the *current iteration's* node
  // count, so `nodes` fell between depths and tools/search_bench.py - which
  // keeps the last info line and is how INV-6 is discharged for a change
  // claimed behaviour-neutral - compared the final iteration alone. Measured
  // before the fix on this position at depth 6: 149, 1568, 4482, 12980, 8891,
  // 49034, against a whole-search total of 77104. Both halves failed - the
  // count fell from 12980 to 8891, and the last line said 49034.
  //
  // The per-iteration figure is not printed alongside it. It is the difference
  // between two successive lines, so nothing is lost by leaving it out.
  TEST_CASE("info nodes is cumulative over the whole search")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line(
          "position fen r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/"
          "1PP1QPPP/R4RK1 w - - 0 10");

      // [position] stops whatever was running, and [go] is the only path that
      // clears the stop flag again - iterative_deepening_search documents that
      // its caller has cleared it before the timer is armed. Without this the
      // direct call below aborts after its first iteration, since that is the
      // one that runs against a local never-stop flag. A depth-limited [go]
      // arms no timer, so the flag stays clear afterwards.
      uci_process_line("go depth 1");
      uci_wait_for_search();
    }

    // No clock and no node budget, so nothing arms a timer and the iteration
    // count is fixed rather than whatever the machine got through.
    uci_search_options_t options = {};
    options.depth = 6;

    uci_search_result_t result;
    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      result = iterative_deepening_search(options);
      lines = capture.lines();
    }

    std::vector<uint64_t> reported;
    for (const std::string& line : lines) {
      if (line.rfind("info ", 0) != 0) { continue; }

      const size_t at = line.find(" nodes ");
      REQUIRE_MESSAGE(at != std::string::npos,
                      ("info line without a node count: " + line));

      reported.push_back(std::stoull(line.substr(at + 7)));
    }

    // Preconditions, without which the two assertions below cannot fail.
    // Monotonicity over a single line is vacuous, and a cumulative total
    // equals the last iteration's own count whenever only one iteration ran or
    // every earlier one explored nothing. Both are ruled out here, so the
    // per-iteration reporting this replaces would print something smaller than
    // the total.
    REQUIRE(reported.size() >= 2);
    REQUIRE(reported.front() > 0);
    REQUIRE(reported[reported.size() - 2] > 0);

    for (size_t i = 1; i < reported.size(); ++i) {
      REQUIRE_MESSAGE(reported[i] >= reported[i - 1],
                      ("info nodes fell between depths: " +
                       std::to_string(reported[i - 1]) + " then " +
                       std::to_string(reported[i])));
    }

    // total_node_explored is the accumulation of every iteration's own count,
    // so this is the "final value equals the sum of the per-iteration counts"
    // half of the property.
    REQUIRE_EQ(reported.back(), result.total_node_explored);

    uci_shutdown();
  }


  TEST_CASE("best_move_to_string emits the UCI null move for nothing")
  {
    uci_search_result_t result = {};
    result.best_move = 0;

    REQUIRE_EQ(best_move_to_string(result), std::string("0000"));
  }

  TEST_CASE("algebraic move parsing round trips")
  {
    const std::vector<std::string> moves = {"e2e4",  "a1h8",  "b7b8q",
                                            "b7b8n", "h2h1r", "d7d8b"};

    for (const std::string& text : moves) {
      const auto parsed = algebraic_to_uci_move(text);
      REQUIRE_MESSAGE(parsed.has_value(), text);
      REQUIRE_EQ(uci_move_to_algebraic(&parsed.value()), text);
    }
  }

  TEST_CASE("garbage move text is rejected")
  {
    const std::vector<std::string> bad = {"",     "e2",   "e2e",  "z2e4",
                                          "e9e4", "e2e9", "e2e4x"};

    for (const std::string& text : bad) {
      REQUIRE_MESSAGE(algebraic_to_uci_move(text).has_value() == false, text);
    }
  }

  TEST_CASE("setoption Hash resizes and the engine keeps working")
  {
    uci_init();

    size_t smallest = 0;
    size_t previous = 0;

    for (const std::string& value :
         {std::string("1"), std::string("64"), std::string("4096")}) {
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value " + value);
      uci_process_line("position startpos");

      // Whatever the value, the table stays usable. And it has to actually
      // grow: without this the option could be dropped on the floor and every
      // other assertion here would still hold.
      REQUIRE_MESSAGE(uci_tt()->entries != nullptr, value);
      REQUIRE_MESSAGE(uci_tt()->entry_count > previous, value);

      previous = uci_tt()->entry_count;
      if (smallest == 0) { smallest = previous; }
    }

    {
      // Not a number: nothing happens at all.
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value abc");
    }
    REQUIRE_EQ(uci_tt()->entry_count, previous);

    {
      // Out of range: clamped to the minimum rather than refused.
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value -5");
    }
    REQUIRE_EQ(uci_tt()->entry_count, smallest);

    REQUIRE(uci_is_running());
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));

    uci_shutdown();
  }
}


TEST_SUITE("engine: uci go")
{
  // Everything above drives the UCI layer one command at a time. Nothing runs
  // a [go] end to end, which is the only thing a GUI actually asks for: one
  // bestmove line, on stdout, naming a move that can be played.

  // Pulls the move text out of the single bestmove line the reply must carry.
  static std::string bestmove_of(const stdout_capture_t& capture)
  {
    std::string found;
    size_t count = 0;

    for (const std::string& line : capture.lines()) {
      if (line.rfind("bestmove ", 0) != 0) { continue; }

      count++;
      found = line.substr(std::string("bestmove ").length());
    }

    REQUIRE_MESSAGE(count == 1, ("expected one bestmove line, got " +
                                 std::to_string(count) + "\n" + capture.str()));

    // Strip a ponder move if one is ever attached.
    const size_t space = found.find(' ');
    if (space != std::string::npos) { found = found.substr(0, space); }

    return found;
  }


  static void require_playable(const std::string& move_text)
  {
    REQUIRE_MESSAGE(move_text != "0000", "engine answered with the null move");

    const auto parsed = algebraic_to_uci_move(move_text);
    REQUIRE_MESSAGE(parsed.has_value(), move_text);

    memcpy(&game, uci_game(), sizeof(game_t));

    move_t moves[MAX_MOVES];
    const size_t count = legal_moves(&game, moves);

    bool found = false;
    for (size_t i = 0; i < count; ++i) {
      if (MOVE_FROM(moves[i]) == parsed->from &&
          MOVE_TO(moves[i]) == parsed->to &&
          MOVE_PROMOTED(moves[i]) == parsed->promotion) {
        found = true;
        break;
      }
    }

    REQUIRE_MESSAGE(found, ("not a legal move here: " + move_text));
  }


  TEST_CASE("every kind of go answers with one legal bestmove")
  {
    // clang-format off
    const std::vector<std::string> commands = {
      "go depth 4",
      "go nodes 20000",
      "go movetime 100",
      "go wtime 1000 btime 1000 movestogo 20",
      "go wtime 1000 btime 1000 winc 50 binc 50",
      // S036, and the legality half of it that the watchdog case above cannot
      // reach from its own suite: a 1ms clock still answers with a move that
      // can be played. Since S089 this is the sudden-death path, which used to
      // be a fabricated [movestogo 20].
      "go wtime 1 btime 1",
      "go depth 1",
      // No depth, no nodes, and a clock already at zero: the fallback budget
      // is the only thing that ends this one.
      "go wtime 0 btime 0",
      "go",
    };
    // clang-format on

    for (const std::string& command : commands) {
      uci_init();

      std::string reply;
      {
        stdout_capture_t capture;
        uci_process_line("position startpos moves e2e4 e7e5");
        uci_process_line(command);
        uci_wait_for_search();
        reply = bestmove_of(capture);
      }

      require_playable(reply);

      uci_shutdown();
    }
  }


  TEST_CASE("an infinite search answers only once stop arrives")
  {
    uci_init();

    std::string reply;
    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("go infinite");

      // Nothing bounds this one, so the reply is owed to [stop] and to
      // nothing else.
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      uci_process_line("stop");
      uci_wait_for_search();

      reply = bestmove_of(capture);
    }

    require_playable(reply);

    uci_shutdown();
  }


  TEST_CASE("isready answers readyok")
  {
    uci_init();

    stdout_capture_t capture;
    uci_process_line("isready");

    REQUIRE(capture.contains("readyok"));

    uci_shutdown();
  }


  TEST_CASE("ucinewgame puts the board and the table back")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos moves e2e4 e7e5 g1f3");
      uci_process_line("go depth 4");
      uci_wait_for_search();
    }

    REQUIRE_NE(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));

    // The search just filled the table, so a reset has something to clear.
    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE(load_FEN(generate_FEN(&game.board), &game));
    REQUIRE(tt_get_entry(uci_tt(), &game.board) != nullptr);

    {
      stdout_capture_t capture;
      uci_process_line("ucinewgame");
    }

    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(DEFAULT_POSITION));
    REQUIRE(tt_get_entry(uci_tt(), &game.board) == nullptr);

    uci_shutdown();
  }


  TEST_CASE("a book move is checked against the real move list")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
    }

    // From and to are what a book entry carries; the flags come from the
    // generator. A move that is not in the list has to come back as nothing.
    REQUIRE_EQ(
        validate_book_move(NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 0, 0, 0)),
        NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0));

    REQUIRE_EQ(
        validate_book_move(NEW_MOVE(e2, e5, W_PAWN, TO_NONE, 0, 0, 0, 0)),
        move_t(0));
    REQUIRE_EQ(
        validate_book_move(NEW_MOVE(d4, d5, W_PAWN, TO_NONE, 0, 0, 0, 0)),
        move_t(0));

    // A pinned piece is in the generated list but never survives make_move.
    {
      stdout_capture_t capture;
      uci_process_line("position fen 4r3/8/8/8/4N3/8/8/4K3 w - - 0 1");
    }

    REQUIRE_EQ(
        validate_book_move(NEW_MOVE(e4, d6, W_KNIGHT, TO_NONE, 0, 0, 0, 0)),
        move_t(0));

    uci_shutdown();
  }
}


TEST_SUITE("engine: uci parsing")
{
  TEST_CASE("tokenize_input splits on the delimiter and drops the padding")
  {
    std::queue<std::string> tokens = tokenize_input("  go   depth 7  ", " ");

    REQUIRE_EQ(tokens.size(), 3);
    REQUIRE_EQ(tokens.front(), "go");
    tokens.pop();
    REQUIRE_EQ(tokens.front(), "depth");
    tokens.pop();
    REQUIRE_EQ(tokens.front(), "7");

    REQUIRE_EQ(tokenize_input("", " ").size(), 0);
    REQUIRE_EQ(tokenize_input("   ", " ").size(), 0);
    REQUIRE_EQ(tokenize_input("single", " ").size(), 1);
  }


  TEST_CASE("trim_whitespace strips both ends")
  {
    REQUIRE_EQ(trim_whitespace("  go  "), "go");
    REQUIRE_EQ(trim_whitespace("go"), "go");
    REQUIRE_EQ(trim_whitespace("   "), "");
    REQUIRE_EQ(trim_whitespace(""), "");
    REQUIRE_EQ(trim_whitespace("\t go \n"), "go");
  }


  TEST_CASE("pop_int clamps instead of accepting nonsense")
  {
    int value = -1;

    std::queue<std::string> args;
    args.push("7");
    REQUIRE(pop_int(args, "Depth", value, 1, 100));
    REQUIRE_EQ(value, 7);

    // Out of range on both sides is clamped, not rejected.
    args.push("1000");
    REQUIRE(pop_int(args, "Depth", value, 1, 100));
    REQUIRE_EQ(value, 100);

    args.push("-5");
    REQUIRE(pop_int(args, "Depth", value, 1, 100));
    REQUIRE_EQ(value, 1);

    // Garbage and a missing value both fail and leave the target alone.
    value = 42;
    args.push("abc");
    REQUIRE_FALSE(pop_int(args, "Depth", value, 1, 100));
    REQUIRE_EQ(value, 42);

    REQUIRE(args.empty());
    REQUIRE_FALSE(pop_int(args, "Depth", value, 1, 100));
    REQUIRE_EQ(value, 42);
  }


  TEST_CASE("pop_u64 never wraps a negative into a huge budget")
  {
    uint64_t value = 1;

    std::queue<std::string> args;
    args.push("123456789");
    REQUIRE(pop_u64(args, "Nodes", value));
    REQUIRE_EQ(value, 123456789u);

    args.push("-1");
    REQUIRE(pop_u64(args, "Nodes", value));
    REQUIRE_EQ(value, 0u);

    value = 5;
    args.push("nope");
    REQUIRE_FALSE(pop_u64(args, "Nodes", value));
    REQUIRE_EQ(value, 5u);
  }


  TEST_CASE("pv_to_string prints the line in order")
  {
    pv_t pv = {};
    pv.length = 2;
    pv.table[0] = NEW_MOVE(e2, e4, W_PAWN, TO_NONE, 0, 1, 0, 0);
    pv.table[1] = NEW_MOVE(b7, b8, B_PAWN, TO_QUEEN, 0, 0, 0, 0);

    // Every move carries a trailing space, including the last one.
    REQUIRE_EQ(pv_to_string(&pv), std::string("e2e4 b7b8q "));

    pv.length = 0;
    REQUIRE_EQ(pv_to_string(&pv), std::string(""));
  }


  TEST_CASE("set_position keeps the old board when the FEN is bad")
  {
    uci_init();

    REQUIRE(set_position(TRICKY_POS));
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(TRICKY_POS));

    REQUIRE_FALSE(set_position("not a fen at all"));
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(TRICKY_POS));

    uci_shutdown();
  }


  TEST_CASE("check_move_legality accepts only playable moves")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position fen 4r3/8/8/8/4N3/8/8/4K3 w - - 0 1");
    }

    REQUIRE(check_move_legality(NEW_MOVE(e1, d1, W_KING, TO_NONE, 0, 0, 0, 0)));

    // The knight on e4 is pinned by the rook on e8.
    REQUIRE_FALSE(
        check_move_legality(NEW_MOVE(e4, d6, W_KNIGHT, TO_NONE, 0, 0, 0, 0)));

    // Nothing on that square at all.
    REQUIRE_FALSE(
        check_move_legality(NEW_MOVE(a3, a4, W_PAWN, TO_NONE, 0, 0, 0, 0)));

    uci_shutdown();
  }
}


TEST_SUITE("engine: aspiration windows")
{
  // One iterative deepening search, with every info line it printed.
  struct iteration_t
  {
    int depth;
    std::string kind;  // "cp" or "mate"
    int value;
  };

  static std::vector<iteration_t> deepen(const std::string& fen, int depth)
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position fen " + fen);
    }

    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE_MESSAGE(position_is_reachable(&game),
                    (fen + " is not a position a legal game can reach"));

    std::vector<std::string> lines;
    {
      // The search reports on stdout; keep it out of the test output and read
      // it back rather than restating what the engine decided.
      //
      // Driven through [go] rather than by calling
      // iterative_deepening_search() directly, because that function does not
      // clear stop_search_signal - its comment says the caller does, and
      // uci_shutdown() sets it. A direct call after any earlier case in this
      // binary therefore aborts after its first iteration, which is what the
      // first version of this test measured.
      stdout_capture_t capture;
      uci_process_line("go depth " + std::to_string(depth));
      uci_wait_for_search();
      lines = capture.lines();
    }

    std::vector<iteration_t> iterations;

    for (const std::string& line : lines) {
      std::istringstream stream(line);
      std::vector<std::string> token{std::istream_iterator<std::string>(stream),
                                     std::istream_iterator<std::string>()};

      if (token.size() < 8 || token[0] != "info" || token[1] != "score") {
        continue;
      }

      const auto depth_at = std::find(token.begin(), token.end(), "depth");
      REQUIRE(depth_at != token.end());
      REQUIRE(depth_at + 1 != token.end());

      iterations.push_back(
          {std::stoi(*(depth_at + 1)), token[2], std::stoi(token[3])});
    }

    return iterations;
  }

  // S021 and S074, and the case the three fast-suite mate cases cannot make.
  // Those call search() directly at one depth (tests/test_search.cpp), so no
  // window is ever narrowed in them; the schedule only exists inside
  // iterative_deepening_search().
  //
  // Both positions were found by measurement, not chosen: the reference binary
  // at 2b54a4f, which has no windows at all, was run over every FEN in
  // tests/assets/test_jsons/ and asked where a mate first appears. These two
  // score around ten centipawns-times-a-hundred for eight iterations and then
  // report a mate at depth 9, which is the shape that matters here - the
  // window is +/-AspirationDelta around a normal score when a mate score
  // arrives, so the iteration must fail high and be repeated wide.
  //
  // Non-vacuous by construction, and the preconditions are the point:
  //
  //   1. the mate must first appear ABOVE AspirationMinDepth, or no window was
  //      ever narrow when it did
  //   2. the iteration before it must report a centipawn score, or the window
  //      it was searched with was already the full one
  //   3. the schedule must actually have failed and re-searched at least once
  //
  // Remove the widening from src/chesso.cpp and 3 still holds while the mate
  // distance goes wrong, which is what this is here to catch.
  TEST_CASE("a narrowed window still finds a mate that appears mid-search")
  {
    struct case_t
    {
      std::string fen;
      int mate_in;
      int first_mate_depth;
    };

    // clang-format off
    const std::vector<case_t> cases = {
      {"r3r1k1/pp3pbp/1qp1b1p1/1BB5/3P4/Q1n2N2/P4PPP/3R1K1R b - - 5 18", 5, 9},
      {"r4k2/R7/8/8/8/8/4K3/1R6 w - - 1 2",                             5, 9},
    };
    // clang-format on

    for (const case_t& test : cases) {
      const std::string title = "FEN: " + test.fen;
      const std::vector<iteration_t> iterations = deepen(test.fen, 10);

      REQUIRE_MESSAGE(iterations.size() == 10, title);

      // Precondition 1. A mate first seen at or below AspirationMinDepth would
      // have been found before any window was narrowed at all.
      REQUIRE_MESSAGE(test.first_mate_depth > ASPIRATION_MIN_DEPTH, title);

      // Precondition 2. The iteration below it scores in centipawns, so the
      // window the mate had to arrive through was AspirationDelta wide and not
      // the full one.
      REQUIRE_MESSAGE(iterations[test.first_mate_depth - 2].kind == "cp",
                      title);

      // Precondition 3. The schedule was in force and did fail.
      REQUIRE_MESSAGE(uci_last_aspiration_failures() > 0, title);

      // And the answer. Every iteration from the first mate on reports the
      // same distance: a window that hid the mate for an iteration would show
      // up here as a centipawn score in the middle of the run.
      for (size_t i = test.first_mate_depth - 1; i < iterations.size(); ++i) {
        const std::string at = title + " depth " + std::to_string(i + 1);

        REQUIRE_MESSAGE(iterations[i].kind == "mate", at);
        REQUIRE_MESSAGE(iterations[i].value == test.mate_in, at);
      }

      uci_shutdown();
    }
  }

  // The window is only ever built around a score the search actually returned.
  // A fail-high or a fail-low returns a bound instead, and centring the next
  // iteration on a bound would narrow the window around a number the search
  // never claimed. Depth 1 has no previous iteration at all, which is why
  // AspirationMinDepth cannot be set below 2.
  TEST_CASE("the first iterations are searched with the full window")
  {
    // Not the start position: [go] with no node limit consults the opening
    // book first, and a book answer would return before a single iteration.
    // The exempt region is ASPIRATION_MIN_DEPTH - 1 iterations wide, so this
    // case is only as strong as that constant is large. S085's SPSA run took
    // it from 5 to 2 -- its arithmetic floor -- which shrank this from four
    // iterations to one without failing, and nothing said so. The floor is
    // asserted rather than assumed, because at ASPIRATION_MIN_DEPTH == 1 the
    // region would be empty and the case would pass while checking nothing.
    //
    // REQUIRE and not static_assert: the parameter is `inline constexpr int` in
    // the shipping build but a plain `int` in the tune build, so a static
    // assertion on it compiles in one and breaks the other -- and the suite
    // gate only builds the shipping one, so it would have gone unnoticed.
    REQUIRE(ASPIRATION_MIN_DEPTH >= 2);

    const std::vector<iteration_t> iterations =
        deepen("r4k2/R7/8/8/8/8/4K3/1R6 w - - 1 2", ASPIRATION_MIN_DEPTH - 1);

    REQUIRE(iterations.size() == static_cast<size_t>(ASPIRATION_MIN_DEPTH - 1));
    REQUIRE(uci_last_aspiration_failures() == 0);

    // The count alone does not say the iterations were the expected ones. Every
    // depth from 1 up to the exempt boundary has to be present and in order, so
    // a search that reported the same number of iterations at the wrong depths
    // fails here rather than passing on arithmetic.
    for (size_t i = 0; i < iterations.size(); ++i) {
      REQUIRE(iterations[i].depth == static_cast<int>(i) + 1);
    }

    uci_shutdown();
  }
}


TEST_SUITE("engine: mate safety")
{
  // THE SET, AND WHY IT IS BUILT AND NOT CHOSEN. S145.
  //
  // Reverse futility pruning returns a static score instead of searching when
  // that score is a margin clear of beta, and a static score is never a mate
  // score - so a node whose true value is "mated" can fail high on material and
  // take the mating line with it. S033 found that, contained it with a ply
  // floor and a depth ceiling, and left one hand-built position behind as the
  // gate.
  //
  // Three things were wrong with that gate, all measured in S145 rather than
  // argued:
  //
  //   1. It was three cases over one geometry. tests/test_search.cpp's "mate in
  //      two is found at the right distance" and "pruning does not hide a
  //      forced mate" hold the same two FENs, and "pruning does not hide a mate
  //      against the material leader" is the second of those with White
  //      material added. All three are a mate in two, so all three exercise ply
  //      1 and nothing else, and RFP_MIN_PLY's own comment concedes that no
  //      test covers ply 2.
  //   2. Two of the three were picked for a different engine. They enter at
  //      3ed3b11 and are on master and bitboard as well, which is the owner's
  //      objection in DEC-095 and the reason S142 waited for this step.
  //   3. They are asserted by search() at a fixed depth from a cold table,
  //   which
  //      cannot tell a lost mate from a postponed one. Measured on the S033
  //      position at RfpMinPly 1: the mate appears at iteration 8 with
  //      RfpMaxDepth 6 and never at all with RfpMaxDepth 15. A call at exactly
  //      2m - 1 reports both as "no mate".
  //
  // So the positions below are constructed, spanning mate distances two to five
  // so the guarded defender nodes land at plies 1, 3, 5 and 7, and they are
  // asserted through the engine's own iterative deepening at a depth above the
  // minimum. Every one is a forced mate proved twice and by neither chesso: an
  // exhaustive AND/OR enumeration over python-chess, iterative-deepening in the
  // distance so the answer is exact rather than an upper bound, and stockfish
  // at a node limit. adocs/data/S145_mate_set.py is the construction and
  // adocs/data/S145_mate_set.tsv is what it produced; nothing here is a FEN
  // whose derivation was lost.
  //
  // A sampled set cannot replace this and no sample size would. Of 191
  // positions in a 6347-position sample where this engine says the side to move
  // is mated within six, one has a non-negative score for the mated side and
  // the median is -1093. The hazard needs the mated side to be *ahead*, and
  // that does not occur in play. adocs/data/S145_mined_set.py covers breadth
  // instead, scored as a count with a floor, because per-position pass/fail
  // over mined mates is exactly what made two surveyed projects switch their
  // mate tests off rather than their pruning.
  //
  // ONE MOTIF, WHY THAT IS CLOSE TO FORCED, AND WHAT THIS GATE THEREFORE
  // CANNOT CATCH. S155.
  //
  // The set is broad in mate distance and narrow in shape. Counted over the
  // tracked TSV by adocs/data/S155_motif_census.py, not asserted: two material
  // signatures and one is the colour mirror of the other - the mating side is
  // a king, a queen and three pawns, the mated side a king, two rooks, two
  // bishops and three pawns. The mating force is a lone queen in 48 of 48,
  // `lead` is 760 in 48 of 48, the pawn wall stands on three non-adjacent
  // files in every position, and the eight family labels are one geometry
  // under two file shifts, a mirror and a colour swap.
  //
  // That narrowness is close to forced by the hazard and is not a flaw in the
  // set. The rule misfires only where the side to move is lost by force while
  // its static score is a margin clear of beta, so the mated side has to be
  // materially *ahead* and unable to use it - and a frozen clump behind a
  // blocked pawn wall is close to the only way to build that, which is why
  // every row reads the same lead. S033 built its one position by hand this
  // way; the construction generalises that shape rather than picking it.
  //
  // What it bounds is what this gate can ever catch. It cannot catch a rule
  // that hides:
  //
  //   * a back-rank mate,
  //   * a smothered mate, or any mate delivered by a knight - the mating piece
  //     here is a queen in all 48,
  //   * a king hunt, where the king is driven across the board instead of held
  //     in a pocket,
  //   * an open-line mate, or the sacrifice that opens the line,
  //   * a promotion mate - the engine's own generator emits 1292 legal moves
  //     over the 48 roots and the 104 guarded defender nodes and 0 of them is
  //     a pawn move, the pawns being mutually blocked on non-adjacent files
  //     (`S155_motif_census.py --moves`),
  //   * any mate in a position with a realistic material balance.
  //
  // A future pruning rule that loses mates in those shapes passes this suite.
  // Whether a second motif is worth constructing was asked and answered yes,
  // by the owner on 2026-09-01: the two signatures above are the reason, and
  // S168 is the step, because a new family owes its own two proofs and its own
  // reverse-futility sweep rather than a paragraph here.
  struct mate_case_t
  {
    const char* root;      // the position under test, mating side to move
    const char* nodes[4];  // the guarded defender nodes, plies 1, 3, 5, 7
    int distance;          // proved, in moves
    int lead;              // the mated side's material lead, standard cp
    const char* family;
  };

  struct report_t
  {
    int depth;
    bool is_mate;
    int value;
  };

  // One iterative deepening search, every finished iteration's score.
  //
  // Driven through [go] and not by calling search() once per depth, because the
  // aspiration schedule, the table carried across iterations and the bounds
  // each iteration hands down are all part of what decides whether a mate
  // survives - DEC-060 measured that a pruning rule's mate exposure is a
  // property of the bound its parent passes down rather than of the static
  // score. A loop over search() reproduces none of that.
  static std::vector<report_t> deepen_scores(const std::string& fen, int depth)
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position fen " + fen);
    }

    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      uci_process_line("go depth " + std::to_string(depth));
      uci_wait_for_search();
      lines = capture.lines();
    }

    std::vector<report_t> out;

    for (const std::string& line : lines) {
      std::istringstream stream(line);
      std::vector<std::string> token{std::istream_iterator<std::string>(stream),
                                     std::istream_iterator<std::string>()};

      if (token.size() < 8 || token[0] != "info" || token[1] != "score") {
        continue;
      }

      const auto depth_at = std::find(token.begin(), token.end(), "depth");
      REQUIRE(depth_at != token.end());
      REQUIRE(depth_at + 1 != token.end());

      out.push_back({std::stoi(*(depth_at + 1)), token[2] == "mate",
                     std::stoi(token[3])});
    }

    return out;
  }

  // Eight iterations of slack above 2m - 1, and the number is measured. One
  // position in the set is late by four at the shipping defaults and by eight
  // at RfpMinPly 1, so a tighter window would report a postponed mate as a lost
  // one
  // - the exact conflation this step exists to remove. A whole search of one of
  // these positions at depth 17 costs about 17 ms, so there is no reason to be
  // tight.
  static constexpr int MATE_DEPTH_SLACK = 8;

  // WHAT IS ASSERTED, AND WHY IT IS NOT "EVERY MATE IS FOUND".
  //
  // Measured over these 48 positions on the shipping build at depth 2m - 1 + 8,
  // and the answer is almost entirely a function of the mate distance:
  //
  //   mate in 2   16 of 16 exact, delay 0
  //   mate in 3    8 of 16 exact, delay up to 4
  //   mate in 4    0 of 8
  //   mate in 5    0 of 8
  //
  // So the guard holds where the old three-position gate looked and nowhere
  // else, because all three of those cases were mates in two. Asserting that
  // every position is found would assert something this engine has never done
  // and no setting of reverse futility makes true - with the rule switched off
  // entirely it is 34 of 48, not 48. A test demanding it would be red on
  // arrival and would be weakened to clear it, which is what happened to the
  // two surveyed projects that wrote per-position mate tests.
  //
  // What is asserted instead is three things, in descending order of how
  // provable they are.
  //
  // **Two properties that are defects at any count.** No mate score for the
  // side being mated, and no mate score *closer* than the proved minimum. The
  // second is what the exhaustive proof buys: the enumeration refuted every
  // shorter distance, so a shorter claim is provably false rather than merely
  // surprising, and it is the S094 class of bug - a mate score renormalised by
  // the wrong number of plies. Measured 0 and 0 over twelve reverse-futility
  // settings times these 48 positions.
  //
  // **Every mate in two, at the first iteration that can hold it.** This is the
  // assertion that fences the tuner. At RfpMinPly 2 and above it is 16 of 16
  // with delay 0; at 1 and 0 it is 13 of 16 with delays up to 7. 0 and 1 are
  // the same engine - the root is exempted by !is_pv, not by this parameter -
  // so this goes red at exactly the value S085's run spent 906 of 1250
  // iterations at.
  //
  // **A floor on the mate in three count.** 8 of 16 at the shipping floor, 6 at
  // RfpMinPly 1, 11 at RfpMinPly 4. The floor is 7, placed strictly between the
  // shipping value and the removed-guard value so it fails when the guard fails
  // and not when the tree shifts underneath it.
  //
  // The mate in four and five counts are **recorded and not asserted**: they
  // are 0 of 8, and a floor of zero asserts nothing. What recovers them is the
  // depth ceiling and not the ply floor - 4 of 8 and 3 of 8 at RfpMaxDepth 0,
  // still 0 and 0 at 10 and above, and S085 tuned that ceiling from S033's 6
  // to 15. That trade is a default change, so it is an SPRT and its own step;
  // adocs/data/S145_rfp_sweep.log is the evidence.
  static constexpr int MATE_IN_THREE_FLOOR = 7;

  TEST_CASE_FIXTURE(engine_fixture_t,
                    "a proved mate is never mis-scored, and every mate in two "
                    "is found on time")
  {
    // clang-format off
    // Generated by `S145_mate_set.py emit-cpp` from adocs/data/S145_mate_set.tsv.
    // Do not edit here: edit the script, regenerate, re-emit.
    const std::vector<mate_case_t> cases = {
      {"1krbrb2/2p1p1p1/K1P1P1P1/8/8/2Q5/8/8 w - - 0 1",
       {"1krbrb2/2p1p1p1/K1P1P1P1/2Q5/8/8/8/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2"},
      {"2brbr1k/1p1p1p2/1P1P1P2/8/5K2/8/8/Q7 w - - 0 1",
       {"2brbr1k/1p1p1p2/1P1P1P2/8/5K2/8/8/6Q1 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_flip"},
      {"2brbr2/1p1p1p2/1P1P1P2/8/2Q5/5K1k/8/8 w - - 0 1",
       {"2brbr2/1p1p1p2/1P1P1P2/8/5Q2/5K1k/8/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_flip"},
      {"2k1brbr/3p1p1p/3P1P1P/8/8/8/5QK1/8 w - - 0 1",
       {"2k1brbr/Q2p1p1p/3P1P1P/8/8/8/6K1/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_flip"},
      {"2k5/8/8/8/3q4/2p1p1p1/2P1P1P1/2RBRBK1 b - - 0 1",
       {"2k4q/8/8/8/8/2p1p1p1/2P1P1P1/2RBRBK1 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_black"},
      {"2rbrbk1/2p1p1p1/2P1P1P1/8/2Q5/8/8/2K5 w - - 0 1",
       {"2rbrbk1/2p1p1p1/2P1P1P1/8/7Q/8/8/2K5 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2"},
      {"3k4/8/K7/5q2/8/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"8/2k5/K7/5q2/8/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_flip_black"},
      {"4K3/8/8/4k3/5q2/p1p1p3/P1P1P3/RBRB4 b - - 0 1",
       {"4K3/8/4k3/8/5q2/p1p1p3/P1P1P3/RBRB4 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_black"},
      {"4brbr/3p1p1p/3P1P1P/8/8/8/2Q1K3/6k1 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/8/8/5K2/2Q5/6k1 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_flip"},
      {"4k3/8/3q4/8/8/1p1p1p2/1P1P1P2/2BRBR1K b - - 0 1",
       {"4k3/8/6q1/8/8/1p1p1p2/1P1P1P2/2BRBR1K w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_flip_black"},
      {"7q/8/5k2/8/8/3p1p1p/3P1P1P/3KBRBR b - - 0 1",
       {"q7/8/5k2/8/8/3p1p1p/3P1P1P/3KBRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_flip_black"},
      {"8/3k4/8/7q/8/1p1p1p2/1P1P1P2/1KBRBR2 b - - 0 1",
       {"8/3k4/8/q7/8/1p1p1p2/1P1P1P2/1KBRBR2 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_flip_black"},
      {"8/8/8/4k3/6q1/p1p1p3/P1P1P3/RBRB1K2 b - - 0 1",
       {"8/8/5k2/8/6q1/p1p1p3/P1P1P3/RBRB1K2 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0_black"},
      {"8/8/8/7q/8/2p1p1p1/2P1PkP1/K1RBRB2 b - - 0 1",
       {"8/8/8/1q6/8/2p1p1p1/2P1PkP1/K1RBRB2 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift2_black"},
      {"rbrb1K2/p1p1p3/P1P1P2k/Q7/8/8/8/8 w - - 0 1",
       {"rbrb4/p1p1pK2/P1P1P2k/Q7/8/8/8/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0"},
      {"rbrb4/p1p1p3/P1P1P3/k7/2K5/7Q/8/8 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/k7/2K5/1Q6/8/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 760, "shift0"},
      {"1k6/8/8/8/8/1p1p1p2/1P1PqP2/2BRBRK1 b - - 0 1",
       {"1k6/8/4q3/8/8/1p1p1p2/1P1P1P2/2BRBRK1 w - - 1 2",
        "1k6/8/8/8/6q1/1p1p1p2/1P1P1P1K/2BRBR2 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift2_flip_black"},
      {"1krbrb1Q/2p1p1p1/2P1P1P1/2K5/8/8/8/8 w - - 0 1",
       {"1krbrb2/2p1p1p1/2P1P1P1/2K5/7Q/8/8/8 b - - 1 1",
        "k1rbrb2/2p1p1p1/2P1P1P1/2K5/1Q6/8/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift2"},
      {"1krbrb1Q/2p1p1p1/K1P1P1P1/8/8/8/8/8 w - - 0 1",
       {"1krbrb2/2p1p1p1/K1P1P1P1/7Q/8/8/8/8 b - - 1 1",
        "k1rbrb2/2p1p1p1/K1P1P1P1/2Q5/8/8/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift2"},
      {"2brbrk1/Qp1p1p2/1P1P1P2/8/8/1K6/8/8 w - - 0 1",
       {"2brbrk1/1p1p1p2/1P1P1P2/Q7/8/1K6/8/8 b - - 1 1",
        "2brbr1k/1p1p1p2/1P1P1P2/6Q1/8/1K6/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift2_flip"},
      {"4K3/1q5k/8/8/8/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"4K3/1q4k1/8/8/8/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        "3K4/1q6/5k2/8/8/3p1p1p/3P1P1P/4BRBR w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift0_flip_black"},
      {"4brbr/3p1p1p/3P1P1P/8/1K6/2Q5/8/1k6 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/8/1K6/8/3Q4/1k6 b - - 1 1",
        "4brbr/3p1p1p/3P1P1P/8/8/2K5/3Q4/k7 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift0_flip"},
      {"4brbr/3p1p1p/3P1P1P/8/8/2Q5/4k1K1/8 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/8/3Q4/8/4k1K1/8 b - - 1 1",
        "4brbr/3p1p1p/3P1P1P/8/3Q4/5K2/8/4k3 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift0_flip"},
      {"7K/8/3qk3/8/8/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"7K/5k2/3q4/8/8/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        "5k2/7K/3q4/8/8/3p1p1p/3P1P1P/4BRBR w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift0_flip_black"},
      {"8/4q3/6K1/8/7k/p1p1p3/P1P1P3/RBRB4 b - - 0 1",
       {"8/4q3/6K1/8/6k1/p1p1p3/P1P1P3/RBRB4 w - - 1 2",
        "8/4q3/7K/5k2/8/p1p1p3/P1P1P3/RBRB4 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift0_black"},
      {"8/8/2k3q1/8/8/2p1p1p1/2P1P1P1/1KRBRB2 b - - 0 1",
       {"6q1/8/2k5/8/8/2p1p1p1/2P1P1P1/1KRBRB2 w - - 1 2",
        "1q6/8/2k5/8/8/2p1p1p1/2P1P1P1/K1RBRB2 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift2_black"},
      {"8/8/3q4/8/8/p1p1p3/P1P1P3/RBRBk2K b - - 0 1",
       {"8/8/6q1/8/8/p1p1p3/P1P1P3/RBRBk2K w - - 1 2",
        "8/8/6q1/8/8/p1p1p3/P1P1Pk1K/RBRB4 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift0_black"},
      {"8/8/7k/8/8/1p1p1p2/1P1P1P2/q1BRBRK1 b - - 0 1",
       {"8/8/7k/8/q7/1p1p1p2/1P1P1P2/2BRBRK1 w - - 1 2",
        "8/8/7k/8/6q1/1p1p1p2/1P1P1P1K/2BRBR2 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift2_flip_black"},
      {"8/8/8/5k2/8/2p1p1pq/2P1P1P1/1KRBRB2 b - - 0 1",
       {"8/8/8/5k2/7q/2p1p1p1/2P1P1P1/1KRBRB2 w - - 1 2",
        "8/8/8/5k2/1q6/2p1p1p1/K1P1P1P1/2RBRB2 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift2_black"},
      {"Q1brbrk1/1p1p1p2/1P1P1P2/8/8/5K2/8/8 w - - 0 1",
       {"2brbrk1/1p1p1p2/1P1P1P2/Q7/8/5K2/8/8 b - - 1 1",
        "2brbr1k/1p1p1p2/1P1P1P2/6Q1/8/5K2/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift2_flip"},
      {"rbrb4/p1p1p1k1/P1P1P3/6K1/8/3Q4/8/8 w - - 0 1",
       {"rbrb4/p1p1p1k1/P1P1P3/6K1/8/7Q/8/8 b - - 1 1",
        "rbrb2k1/p1p1p3/P1P1P1K1/8/8/7Q/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift0"},
      {"rbrb4/p1p1p3/P1P1P3/8/8/8/5k1K/1Q6 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/8/4Q3/8/5k1K/8 b - - 1 1",
        "rbrb4/p1p1p3/P1P1P3/8/4Q3/6K1/8/5k2 b - - 3 2",
        nullptr,
        nullptr},
       3, 760, "shift0"},
      {"4brbr/3p1p1p/3P1P1P/3K4/2Q5/8/1k6/8 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/8/2QK4/8/1k6/8 b - - 1 1",
        "4brbr/3p1p1p/3P1P1P/1Q6/3K4/k7/8/8 b - - 3 2",
        "4brbr/3p1p1p/3P1P1P/1Q6/8/2K5/k7/8 b - - 5 3",
        nullptr},
       4, 760, "shift0_flip"},
      {"7K/8/1q6/8/4k3/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"7K/2q5/8/8/4k3/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        "6K1/2q5/8/5k2/8/3p1p1p/3P1P1P/4BRBR w - - 3 3",
        "7K/2q5/6k1/8/8/3p1p1p/3P1P1P/4BRBR w - - 5 4",
        nullptr},
       4, 760, "shift0_flip_black"},
      {"7K/8/3k4/8/q7/p1p1p3/P1P1P3/RBRB4 b - - 0 1",
       {"7K/4k3/8/8/q7/p1p1p3/P1P1P3/RBRB4 w - - 1 2",
        "6K1/8/5k2/8/q7/p1p1p3/P1P1P3/RBRB4 w - - 3 3",
        "7K/5k2/8/8/q7/p1p1p3/P1P1P3/RBRB4 w - - 5 4",
        nullptr},
       4, 760, "shift0_black"},
      {"8/8/8/8/7K/p1p1pq2/P1P1P3/RBRB1k2 b - - 0 1",
       {"8/8/8/5q2/7K/p1p1p3/P1P1P3/RBRB1k2 w - - 1 2",
        "8/8/8/8/4q3/p1p1p1K1/P1P1P3/RBRB1k2 w - - 3 3",
        "8/8/8/8/4q3/p1p1p2K/P1P1Pk2/RBRB4 w - - 5 4",
        nullptr},
       4, 760, "shift0_black"},
      {"k1K5/8/8/4q3/8/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"k1K5/6q1/8/8/8/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        "3K4/1k4q1/8/8/8/3p1p1p/3P1P1P/4BRBR w - - 3 3",
        "4K3/6q1/2k5/8/8/3p1p1p/3P1P1P/4BRBR w - - 5 4",
        nullptr},
       4, 760, "shift0_flip_black"},
      {"k3brbr/3p1p1p/3P1P1P/8/K7/8/8/5Q2 w - - 0 1",
       {"k3brbr/3p1p1p/3P1P1P/1K6/8/8/8/5Q2 b - - 1 1",
        "1k2brbr/3p1p1p/1K1P1P1P/8/8/8/8/5Q2 b - - 3 2",
        "2k1brbr/3p1p1p/1K1P1P1P/8/8/5Q2/8/8 b - - 5 3",
        nullptr},
       4, 760, "shift0_flip"},
      {"rbrb4/p1p1p3/P1P1P3/8/1k1K4/6Q1/8/8 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/3K4/1k6/6Q1/8/8 b - - 1 1",
        "rbrb4/p1p1p3/P1P1P3/1k1K4/8/Q7/8/8 b - - 3 2",
        "rbrb4/p1p1p3/PkP1P3/8/2K5/Q7/8/8 b - - 5 3",
        nullptr},
       4, 760, "shift0"},
      {"rbrb4/p1p1p3/P1P1P3/k3K3/2Q5/8/8/8 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/k2K4/2Q5/8/8/8 b - - 1 1",
        "rbrb4/p1p1p3/PkP1P3/3K4/8/8/4Q3/8 b - - 3 2",
        "rbrb4/p1p1p3/P1P1P3/k1K5/8/8/4Q3/8 b - - 5 3",
        nullptr},
       4, 760, "shift0"},
      {"2K5/7q/8/7k/8/p1p1p3/P1P1P3/RBRB4 b - - 0 1",
       {"2K5/q7/8/7k/8/p1p1p3/P1P1P3/RBRB4 w - - 1 2",
        "3K4/1q6/8/7k/8/p1p1p3/P1P1P3/RBRB4 w - - 3 3",
        "4K3/1q6/6k1/8/8/p1p1p3/P1P1P3/RBRB4 w - - 5 4",
        "5K2/3q4/6k1/8/8/p1p1p3/P1P1P3/RBRB4 w - - 7 5"},
       5, 760, "shift0_black"},
      {"4brbr/3p1p1p/1k1P1P1P/8/K1Q5/8/8/8 w - - 0 1",
       {"4brbr/3p1p1p/1k1P1P1P/3Q4/K7/8/8/8 b - - 1 1",
        "4brbr/k2p1p1p/3P1P1P/1K1Q4/8/8/8/8 b - - 3 2",
        "1k2brbr/3p1p1p/1K1P1P1P/3Q4/8/8/8/8 b - - 5 3",
        "2k1brbr/K2p1p1p/3P1P1P/3Q4/8/8/8/8 b - - 7 4"},
       5, 760, "shift0_flip"},
      {"4brbr/3p1p1p/3P1P1P/4Q3/k7/8/8/4K3 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/3Q4/k7/8/8/4K3 b - - 1 1",
        "4brbr/3p1p1p/3P1P1P/3Q4/1k6/8/3K4/8 b - - 3 2",
        "4brbr/1Q1p1p1p/3P1P1P/8/k7/8/3K4/8 b - - 5 3",
        "4brbr/1Q1p1p1p/3P1P1P/k7/8/2K5/8/8 b - - 7 4"},
       5, 760, "shift0_flip"},
      {"8/1q6/8/4k3/8/3p1p1p/3P1P1P/K3BRBR b - - 0 1",
       {"8/1q6/8/3k4/8/3p1p1p/3P1P1P/K3BRBR w - - 1 2",
        "8/1q6/8/8/2k5/3p1p1p/K2P1P1P/4BRBR w - - 3 3",
        "8/8/8/8/2k5/K2p1p1p/3P1P1P/1q2BRBR w - - 5 4",
        "1q6/8/8/8/K1k5/3p1p1p/3P1P1P/4BRBR w - - 7 5"},
       5, 760, "shift0_flip_black"},
      {"8/5q2/8/6K1/8/p1p1p2k/P1P1P3/RBRB4 b - - 0 1",
       {"8/5q2/8/6K1/8/p1p1p1k1/P1P1P3/RBRB4 w - - 1 2",
        "6q1/8/7K/8/8/p1p1p1k1/P1P1P3/RBRB4 w - - 3 3",
        "6q1/8/8/7K/5k2/p1p1p3/P1P1P3/RBRB4 w - - 5 4",
        "6q1/8/7K/5k2/8/p1p1p3/P1P1P3/RBRB4 w - - 7 5"},
       5, 760, "shift0_black"},
      {"8/8/4K3/8/3qk3/3p1p1p/3P1P1P/4BRBR b - - 0 1",
       {"3q4/8/4K3/8/4k3/3p1p1p/3P1P1P/4BRBR w - - 1 2",
        "3q4/5K2/8/5k2/8/3p1p1p/3P1P1P/4BRBR w - - 3 3",
        "4q3/6K1/8/5k2/8/3p1p1p/3P1P1P/4BRBR w - - 5 4",
        "4q3/7K/5k2/8/8/3p1p1p/3P1P1P/4BRBR w - - 7 5"},
       5, 760, "shift0_flip_black"},
      {"rbrb4/p1p1p3/P1P1P3/6Q1/8/1K5k/8/8 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/6Q1/2K5/7k/8/8 b - - 1 1",
        "rbrb4/p1p1p3/P1P1P3/3K2Q1/8/8/7k/8 b - - 3 2",
        "rbrb4/p1p1p3/P1P1P3/6Q1/4K3/7k/8/8 b - - 5 3",
        "rbrb4/p1p1p3/P1P1P3/6Q1/8/5K2/7k/8 b - - 7 4"},
       5, 760, "shift0"},
      {"rbrb4/p1p1p3/P1P1P3/8/1k6/8/2QK4/8 w - - 0 1",
       {"rbrb4/p1p1p3/P1P1P3/8/1k6/4K3/2Q5/8 b - - 1 1",
        "rbrb4/p1p1p3/P1P1P3/1k6/3K4/8/2Q5/8 b - - 3 2",
        "rbrb4/p1p1p3/PkP1P3/8/2K5/8/2Q5/8 b - - 5 3",
        "rbrb4/p1p1p3/P1k1P3/5Q2/2K5/8/8/8 b - - 1 4"},
       5, 760, "shift0"},
    };
    // clang-format on

    // The set has to be the set S145 describes, or the loop below is a weaker
    // test than it reads as. Asserted, not assumed: twenty positions is the
    // floor the step states, and the distances have to span two to five or the
    // deeper plies are not exercised at all.
    REQUIRE(cases.size() >= 20);

    std::set<int> distances;
    for (const mate_case_t& test : cases) {
      distances.insert(test.distance);
    }
    REQUIRE(distances == std::set<int>({2, 3, 4, 5}));

    std::map<int, int> exact_by_distance;
    std::map<int, int> total_by_distance;

    for (const mate_case_t& test : cases) {
      const std::string title = std::string(test.family) + ", mate in " +
                                std::to_string(test.distance) + ", " +
                                test.root;

      ++total_by_distance[test.distance];

      // Precondition 1. A position no legal game reaches proves nothing about a
      // search that only ever meets positions legal games reach.
      REQUIRE_MESSAGE(load_FEN(test.root, &game), title);
      REQUIRE_MESSAGE(position_is_reachable(&game), title);

      const int minimum = 2 * test.distance - 1;

      // Precondition 2, and this is the one that makes the case non-vacuous.
      // Every guarded defender node on the mating line has to be a node reverse
      // futility is allowed to fire at - not in check, or the rule is already
      // off there and the node proves nothing - and at least one of them has to
      // be a node it would actually fire at: a static score high enough that
      // subtracting the margin the rule uses *at that node's own ply* still
      // leaves it positive. Without the second half the case could pass on an
      // engine that never comes near the rule at all.
      //
      // "At least one" and not "all", because the margin is RFP_MARGIN per
      // remaining ply and therefore largest at ply 1: at the first iteration
      // that can hold a mate in m, ply 1 has 2m - 2 plies left where the
      // deepest guarded node has 2. One mate in four here scores 343 at ply 1
      // against a 378 margin and clears its deepest node's 126 nearly
      // threefold, so requiring every node would refuse a position whose hazard
      // is real one ply further down. The number of guarded nodes is asserted
      // too, so "this case reaches ply 7" is checked rather than inferred from
      // the distance.
      int guarded_nodes = 0;
      bool reachable_cutoff = false;

      for (int i = 0; i < 4; ++i) {
        if (test.nodes[i] == nullptr) { break; }

        const int ply = 2 * i + 1;
        const std::string at = title + ", ply " + std::to_string(ply);

        REQUIRE_MESSAGE(load_FEN(test.nodes[i], &game), at);
        REQUIRE_MESSAGE(!is_check(&game), at);

        ++guarded_nodes;

        if (evaluate(&game.board) > RFP_MARGIN * (minimum - ply)) {
          reachable_cutoff = true;
        }
      }

      REQUIRE_MESSAGE(guarded_nodes == test.distance - 1, title);
      REQUIRE_MESSAGE(reachable_cutoff, title);

      const std::vector<report_t> iterations =
          deepen_scores(test.root, minimum + MATE_DEPTH_SLACK);

      REQUIRE_MESSAGE(
          iterations.size() == static_cast<size_t>(minimum + MATE_DEPTH_SLACK),
          title);

      int first_exact = 0;

      for (const report_t& iteration : iterations) {
        if (!iteration.is_mate) { continue; }

        const std::string at =
            title + ", iteration " + std::to_string(iteration.depth);

        // Never a mate for the side that is being mated.
        REQUIRE_MESSAGE(iteration.value > 0, at);

        // And never closer than the proved minimum. The enumeration refuted
        // every shorter distance by exhausting the tree, so a shorter claim is
        // false and not merely optimistic.
        REQUIRE_MESSAGE(iteration.value >= test.distance, at);

        if (iteration.value == test.distance && first_exact == 0) {
          first_exact = iteration.depth;
        }
      }

      const report_t& last = iterations.back();

      if (last.is_mate && last.value == test.distance) {
        ++exact_by_distance[test.distance];
      }

      // Every mate in two, at the first iteration that can hold it. This is the
      // case that goes red when the ply floor drops below 2.
      if (test.distance == 2) {
        REQUIRE_MESSAGE(last.is_mate, title);
        REQUIRE_MESSAGE(last.value == 2, title);
        REQUIRE_MESSAGE(first_exact == minimum,
                        (title + ", first reported at iteration " +
                         std::to_string(first_exact) + " and not " +
                         std::to_string(minimum)));
      }

      uci_shutdown();
    }

    // The two counts the guard does not reach are recorded rather than
    // asserted, and the message carries them, so a change that improves them
    // says so in the log instead of going silently green at the old number.
    MESSAGE("mate in 4: " << exact_by_distance[4] << " of "
                          << total_by_distance[4]
                          << " exact, mate in 5: " << exact_by_distance[5]
                          << " of " << total_by_distance[5]
                          << " -- recovered by RfpMaxDepth and not by "
                             "RfpMinPly; adocs/data/S145_rfp_sweep.log");

    REQUIRE(exact_by_distance[2] == total_by_distance[2]);
    REQUIRE(exact_by_distance[3] >= MATE_IN_THREE_FLOOR);
  }


  // CHECKMATE OUTRANKS THE HUNDREDTH HALFMOVE. FIDE 5.1.1 ends the game the
  // instant checkmate is delivered, and 9.6.2 spells the same exception out for
  // the 75-move rule -- so a node whose clock has reached 100 is a draw only
  // once mate has been ruled out. src/search.cpp returned DRAW_SCORE before
  // generating a move, which scored a mate the engine had in hand as a draw
  // and, wherever anything else on the board was worth more than nothing, threw
  // the win away to take it.
  //
  // Observed red on the tree of 2026-08-22, before the fix, at depth 1:
  //
  //   7k/6pp/8/8/8/8/8/R6K w - - 99 60       info score cp 0   pv a1a8
  //   7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60    info score cp 448 pv g2h3
  //
  // The first is the scoring defect alone: every root move there reaches the
  // clock, so every one of them scored zero and the mate was chosen by move
  // order and nothing else. The second adds one capture that resets the clock,
  // which is all it takes for the wrong score to become the wrong move -- the
  // engine gave up a mate in one to win a knight.
  //
  // Neither FEN was read off a board. python-chess reports both VALID with
  // exactly one mate in one, Ra8#, whose child carries a halfmove clock of
  // exactly 100 and no legal reply; stockfish scores both Mate(+1) at
  // `go depth 20`. S162, DEC-023.
  TEST_CASE("checkmate outranks the hundredth halfmove")
  {
    struct clock_mate_case_t
    {
      const char* root;   // clock 99, the mating side to move
      const char* mated;  // after the mating move: clock 100, no legal reply
      const char* mate;   // the mating move, UCI
    };

    const clock_mate_case_t cases[] = {
        {"7k/6pp/8/8/8/8/8/R6K w - - 99 60",
         "R6k/6pp/8/8/8/8/8/7K b - - 100 60", "a1a8"},
        {"7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60",
         "R6k/6pp/8/8/8/7n/6P1/7K b - - 100 60", "a1a8"},
    };

    for (const clock_mate_case_t& test : cases) {
      const std::string title(test.root);

      // The precondition, established rather than assumed: the child really is
      // mate, really sits at a clock of exactly 100, and is a position a legal
      // game can reach. Drop any one of the three and the case below asserts
      // nothing -- S070 passed for months on a board with adjacent kings.
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line(std::string("position fen ") + test.mated);
      }

      memcpy(&game, uci_game(), sizeof(game_t));

      REQUIRE_MESSAGE(position_is_reachable(&game), title);
      REQUIRE_MESSAGE(int(game.board.halfmove_clock) == 100, title);
      REQUIRE_MESSAGE(is_check(&game), title);

      move_t replies[MAX_MOVES];
      REQUIRE_MESSAGE(legal_moves(&game, replies) == 0, title);

      uci_shutdown();

      // Depth 1 is the tightest form available: the mate is one ply away, so
      // what this asserts is the boundary node's own score and nothing about
      // the search above it.
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line(std::string("position fen ") + test.root);
      }

      std::vector<std::string> lines;
      {
        stdout_capture_t capture;
        uci_process_line("go depth 1");
        uci_wait_for_search();
        lines = capture.lines();
      }

      std::string score_line;
      std::string best;

      for (const std::string& line : lines) {
        if (line.rfind("info score ", 0) == 0) { score_line = line; }

        if (line.rfind("bestmove ", 0) == 0) {
          best = line.substr(std::string("bestmove ").length());

          // Strip a ponder move if one is ever attached.
          const size_t space = best.find(' ');
          if (space != std::string::npos) { best = best.substr(0, space); }
        }
      }

      const std::string at = title + "\n" + score_line + "\nbestmove " + best;

      // Both halves, because the defect could hide in either: the mate has to
      // be scored as a mate at the right distance, and it has to be the move
      // that comes back.
      REQUIRE_MESSAGE(score_line.rfind("info score mate 1 ", 0) == 0, at);
      REQUIRE_MESSAGE(best == std::string(test.mate), at);

      uci_shutdown();
    }
  }
}
