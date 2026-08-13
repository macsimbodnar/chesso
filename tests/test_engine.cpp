#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
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

  TEST_CASE("time budget never exceeds the clock")
  {
    struct case_t
    {
      int remaining;
      int increment;
      int movestogo;
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
    };
    // clang-format on

    for (const case_t& test : cases) {
      const int budget = compute_search_time_ms(test.remaining, test.increment,
                                                test.movestogo);

      const std::string title = "remaining " + std::to_string(test.remaining) +
                                " inc " + std::to_string(test.increment) +
                                " movestogo " + std::to_string(test.movestogo);

      REQUIRE_MESSAGE(budget > 0, title);

      // Strictly less: something has to be left for getting the move out of
      // the door, or the flag falls while the engine is still talking.
      REQUIRE_MESSAGE(budget < test.remaining, title);

      // And a floor, so a nearly empty clock still buys a real search rather
      // than a move picked at depth one.
      REQUIRE_MESSAGE(budget >= std::min(50, test.remaining / 2), title);
    }
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

  TEST_CASE("first_legal_move reports nothing in a terminal position")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position fen 7k/5Q1K/8/8/8/8/8/8 b - - 0 1");
    }

    REQUIRE_EQ(first_legal_move(), 0);

    uci_shutdown();
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
    options.movestogo = DEFAULT_MOVES_TO_GO;

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
    options.movestogo = DEFAULT_MOVES_TO_GO;
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
