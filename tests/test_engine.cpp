#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>  // setenv, unsetenv: POSIX, and there is no Windows build
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "bb_tables.hpp"  // rook_magic_numbers, bishop_magic_numbers
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "evaluation.hpp"
#include "openings.hpp"  // BOOK_FILE_EMBEDDED
#include "search_params.hpp"
#include "test_helpers.hpp"
#include "transposition_table.hpp"
#include "uci.hpp"

// Defined in bitboard.cpp. Not in the header because nothing in the engine
// needs it at runtime - only the tests, to check the incremental hash.
hash_t compute_full_hash(game_t* game);

// Also defined in bitboard.cpp and also out of the header: the engine calls
// neither at runtime. S179 uses them to check the two generated tables.
bool magic_is_collision_free(index_t square, bb_t magic, bool rook);
uint64_t project_random_next(uint64_t* state);
extern const uint64_t CHESSO_PROJECT_SEED;


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


TEST_SUITE("engine: generated tables")
{
  // S179. A magic number is a perfect hash of a square's relevant occupancies
  // into its attack table: every blocker pattern must land in a free slot or on
  // a slot already holding the *same* attack set. A magic that fails this
  // returns a wrong attack set for some occupancy and says nothing about it;
  // the first symptom is a perft mismatch, thousands of nodes later.
  //
  // The checker is the same one tools/magic_gen.cpp accepts a candidate with,
  // so this asserts that what the generator was told to look for is what the
  // committed constants are.
  TEST_CASE_FIXTURE(
      engine_fixture_t,
      "the magic checker rejects a bad magic, and accepts all 128")
  {
    // Precondition: the checker can fail. Zero maps every occupancy to slot 0,
    // and a8 has more than one distinct rook attack set, so it must be
    // rejected. Without this the loop below would pass against a checker that
    // returns true unconditionally.
    REQUIRE_FALSE(magic_is_collision_free(0, BB_0, true));
    REQUIRE_FALSE(magic_is_collision_free(0, BB_0, false));

    for (index_t square = 0; square < 64; ++square) {
      const std::string at = " at square " + std::to_string(square);
      const std::string rook_msg = "rook magic collides" + at;
      const std::string bishop_msg = "bishop magic collides" + at;

      CHECK_MESSAGE(
          magic_is_collision_free(square, rook_magic_numbers[square], true),
          rook_msg);
      CHECK_MESSAGE(
          magic_is_collision_free(square, bishop_magic_numbers[square], false),
          bishop_msg);
    }
  }


  // S203, DEC-139. Two things about the 851 Zobrist keys, neither of them a
  // number re-read from a run (DEC-142).
  //
  // First, that they are the project generator's output under the project seed,
  // draw order included. They came from std::uniform_int_distribution over
  // std::mt19937_64 until S203, and the standard does not fix what a
  // distribution returns for a given engine state -- so the keys, the table
  // indices and every node count this repository records were a property of the
  // standard library as much as of this code. They matched between glibc and
  // Apple libc++ when the MacBook handover check ran, which is a measurement
  // and not a guarantee. This is the guarantee.
  //
  // Second, the quality rule from
  // https://www.chessprogramming.org/Zobrist_Hashing : what matters is linear
  // independence, that no small subset of keys XORs to the same value as
  // another, since such a pair is two different positions with one key.
  // Enumerated at the sizes that can be -- distinct keys rules out subsets of
  // one and two, no pair XOR equal to a key rules out three, and no two pair
  // XORs equal rules out four.
  TEST_CASE_FIXTURE(engine_fixture_t,
                    "the zobrist keys are the project generator's, and sound")
  {
    // init_zobrist's order. A reordering there changes every key, and this is
    // what would catch it.
    std::vector<uint64_t> keys;
    keys.reserve(851);

    uint64_t state = CHESSO_PROJECT_SEED;
    for (int piece = 0; piece < 12; ++piece) {
      for (int square = 0; square < 64; ++square) {
        keys.push_back(project_random_next(&state));
      }
    }
    for (int i = 0; i < 16; ++i) {
      keys.push_back(project_random_next(&state));
    }
    for (int i = 0; i < 2; ++i) {
      keys.push_back(project_random_next(&state));
    }
    for (int i = 0; i < 65; ++i) {
      keys.push_back(project_random_next(&state));
    }

    REQUIRE_EQ(keys.size(), 851u);

    const zobrist_randoms_t& live = game.hash_randoms;
    REQUIRE(live.initialized);

    size_t n = 0;
    for (int piece = 0; piece < 12; ++piece) {
      for (int square = 0; square < 64; ++square) {
        REQUIRE_EQ(live.piece_randoms[piece][square], keys[n++]);
      }
    }
    for (int i = 0; i < 16; ++i) {
      REQUIRE_EQ(live.castling_randoms[i], keys[n++]);
    }
    for (int i = 0; i < 2; ++i) {
      REQUIRE_EQ(live.side_randoms[i], keys[n++]);
    }
    for (int i = 0; i < 65; ++i) {
      REQUIRE_EQ(live.ep_randoms[i], keys[n++]);
    }
    REQUIRE_EQ(n, 851u);

    // A zero key is a piece on a square that does not change the hash.
    CHECK_EQ(std::count(keys.begin(), keys.end(), uint64_t(0)), 0);

    const std::set<uint64_t> distinct(keys.begin(), keys.end());
    CHECK_EQ(distinct.size(), 851u);

    std::set<uint64_t> pair_xors;
    int xor_hits_a_key = 0;
    for (size_t i = 0; i < keys.size(); ++i) {
      for (size_t j = i + 1; j < keys.size(); ++j) {
        const uint64_t x = keys[i] ^ keys[j];
        if (distinct.count(x) != 0) { ++xor_hits_a_key; }
        pair_xors.insert(x);
      }
    }

    CHECK_EQ(xor_hits_a_key, 0);
    CHECK_EQ(pair_xors.size(), 851u * 850u / 2u);
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

  // This case used to search for the repetition from the start position after
  // one pawn push: a history of one entry, over which the `back = 2` loop never
  // runs, so it could not fail whatever the clock did. It establishes the
  // repetition first now, then moves only the clock, which is the claim the
  // title makes. S193, 2026-09-04_test_review-F05.
  TEST_CASE_FIXTURE(engine_fixture_t, "an irreversible move clears the window")
  {
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/N3K3 w - - 0 1", &game));

    const std::vector<std::pair<index_t, index_t>> line = {
        {a1, b3}, {e8, d8}, {b3, a1}, {d8, e8}};

    for (const auto& [from, to] : line) {
      REQUIRE(play_move(&game, from, to));
    }

    // Precondition: the window is open and the repetition is inside it.
    REQUIRE_GE(game.board.halfmove_clock, 4);
    REQUIRE(is_position_repeated(&game.history, &game.board));

    // The same board, the same history, one irreversible move's worth of clock.
    // Nothing else changes, so the window is the only thing that can answer.
    const uint8_t clock = game.board.halfmove_clock;
    game.board.halfmove_clock = 0;
    REQUIRE_FALSE(is_position_repeated(&game.history, &game.board));
    game.board.halfmove_clock = clock;
    REQUIRE(is_position_repeated(&game.history, &game.board));
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


  // S210, 2026-09-10_adversarial-F17. halfmove_clock is a uint8_t and `+= 1`
  // wrapped it: 256 reversible plies through `position startpos moves` left it
  // at 0 and 300 at 44. Both consumers then failed at once -- negamax_at's
  // `>= 100` fifty-move test stopped firing, and classify_repetition()'s
  // window, min(halfmove_clock, history size), collapsed to nothing, so a
  // subtree that had already repeated four times had no draw rule of either
  // kind. The increments saturate at HALFMOVE_CLOCK_MAX now.
  //
  // Driven through the UCI command, because that is the only way to reach it:
  // the search is bounded by MAX_PLY and cannot add 256 reversible plies of
  // its own.
  TEST_CASE("256 reversible plies do not wrap the halfmove clock")
  {
    uci_init();

    // A four-ply knight shuffle, which is reversible by construction: no
    // capture, no pawn move, and the position after every cycle is the one
    // before it.
    std::string moves = "position startpos moves";
    const char* cycle[4] = {" g1f3", " b8c6", " f3g1", " c6b8"};

    for (int ply = 0; ply < 256; ++ply) {
      moves += cycle[ply % 4];
    }

    {
      stdout_capture_t capture;
      uci_process_line(moves);

      // Nothing in that line is refusable, so a refusal here would mean the
      // shuffle is not the shuffle this case thinks it is.
      REQUIRE_MESSAGE(!capture.contains("info string refused"), capture.str());
    }

    memcpy(&game, uci_game(), sizeof(game_t));

    REQUIRE_EQ(game.history.size, 256u);

    // The wrap: 256 increments of a uint8_t land back on 0.
    CHECK_MESSAGE(game.board.halfmove_clock >= 100,
                  ("the fifty-move test reads halfmove_clock >= 100 and this "
                   "position is 256 reversible plies deep; clock is " +
                   std::to_string(int(game.board.halfmove_clock))));

    CHECK_EQ(int(game.board.halfmove_clock), HALFMOVE_CLOCK_MAX);

    // The second consumer, exercised rather than reasoned about: the position
    // on the board has occurred 64 times and classify_repetition() has to be
    // able to see it. A wrapped clock makes its window
    // min(halfmove_clock, history size) zero and the walk finds nothing.
    //
    // root_history_size is 0 here -- everything in the history was played
    // before this "search" -- which is the pre-root reading, the stricter of
    // the two classify_repetition() gives.
    const repetition_kind_t kind =
        classify_repetition(&game.history, &game.board, 0);

    CHECK(kind != repetition_kind_t::NONE);

    uci_shutdown();
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


  // S210, 2026-09-10_adversarial-F20. compute_search_time_budget() has a
  // sudden-death branch for movestogo == 0 and uci.hpp says at the field that
  // 0 is what a GUI sends for it and "is not a stand-in for some number of
  // moves" (S089). command_go clamped the token into [1, INT_MAX] before this
  // step, so `movestogo 0` arrived as `movestogo 1` and bought the whole clock
  // minus overhead for one move: 4.64 s of a 10 s clock measured, 46 %.
  //
  // The allocation is not exposed, so what is measured is the time the search
  // actually spends, against the two allocations compute_search_time_budget()
  // gives for the same clock. The third line is the precondition: it is what
  // makes the difference visible, and without it the case would be green on
  // any engine that ignores the token entirely.
  TEST_CASE("movestogo 0 buys the sudden-death allocation, not movestogo 1")
  {
    const int clock_ms = 2000;

    const search_time_budget_t sudden_death =
        compute_search_time_budget(clock_ms, 0, 0);
    const search_time_budget_t one_move_left =
        compute_search_time_budget(clock_ms, 0, 1);

    // Precondition. The two allocations have to be far enough apart that a
    // measured time can tell them apart at all.
    REQUIRE(one_move_left.soft_ms > 4 * sudden_death.soft_ms);

    auto elapsed_ms = [](const std::string& line) {
      uci_init();

      const auto started = std::chrono::steady_clock::now();
      {
        stdout_capture_t capture;
        uci_process_line("position startpos moves e2e4 e7e5");
        uci_process_line(line);
        uci_wait_for_search();
      }
      const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now() - started)
                          .count();

      uci_shutdown();

      return static_cast<int>(ms);
    };

    const std::string clock = " wtime " + std::to_string(clock_ms) + " btime " +
                              std::to_string(clock_ms);

    const int no_token = elapsed_ms("go" + clock);
    const int zero = elapsed_ms("go" + clock + " movestogo 0");
    const int one = elapsed_ms("go" + clock + " movestogo 1");

    const std::string measured = "no movestogo " + std::to_string(no_token) +
                                 " ms, movestogo 0 " + std::to_string(zero) +
                                 " ms, movestogo 1 " + std::to_string(one) +
                                 " ms";

    // The precondition again, this time on the engine rather than on the
    // function: sending `movestogo 1` really does buy a much longer search
    // here, so "movestogo 0 is not movestogo 1" is a claim with content.
    REQUIRE_MESSAGE(one > 3 * no_token, measured);

    // The claim. Halfway between the two allocations is a generous line and
    // still nowhere near `movestogo 1`, which is where the clamp put it.
    CHECK_MESSAGE(zero < (no_token + one) / 2, measured);
  }


  // S210, 2026-09-10_adversarial-F21. iterative_deepening_search() pointed
  // state.stop at a local `never_stop` and swapped the real signal in only
  // after the first search() had returned, so depth 1 answered neither `stop`
  // nor the hard timer. Depth 1 is 0.56 ms at the median over 400 corpus
  // positions, which is why no match ever noticed; on a pathological board the
  // reviewer measured 254 ms burned against a 100 ms clock, which is a forfeit.
  //
  // What is asserted is not a duration. An iteration that was cut reports no
  // completed depth -- last_complete_depth stays 0 and an aborted iteration
  // never raises it -- so "no info line reports depth 1 or more" is exactly
  // "the first iteration did not finish", and the unfixed engine finishes it
  // every time.
  TEST_CASE("the first iteration honours stop and the hard timer")
  {
    // Eight queens a side. Depth 1 here is a wide root over deep capture
    // chains in quiescence, so the stop has a window to land in. **Golden**:
    // 13.8 ms on this machine, re-derived with
    // `position fen <below>` then `go depth 1`, reading the `time` field of the
    // info line. The precondition below asserts 3 ms of it, which is where a
    // four times faster machine would still leave the case separating; under
    // that it needs a heavier position, not a smaller floor.
    const std::string fen =
        "q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1";
    const int depth_1_floor_ms = 3;

    auto deepest_completed_depth = [](const stdout_capture_t& capture) {
      int deepest = -1;

      for (const std::string& line : capture.lines()) {
        const size_t at = line.find(" depth ");

        if (line.rfind("info ", 0) != 0 || at == std::string::npos) {
          continue;
        }

        deepest = std::max(deepest, std::atoi(line.c_str() + at + 7));
      }

      return deepest;
    };

    auto answered_a_playable_move = [](const stdout_capture_t& capture) {
      for (const std::string& line : capture.lines()) {
        if (line.rfind("bestmove ", 0) != 0) { continue; }

        return line != "bestmove 0000";
      }

      return false;
    };

    // The precondition, measured from the engine rather than assumed: an
    // unbounded depth-1 iteration on this position takes long enough that a
    // command sent straight after `go` lands inside it.
    {
      uci_init();

      stdout_capture_t capture;
      uci_process_line("position fen " + fen);
      uci_process_line("go depth 1");
      uci_wait_for_search();

      REQUIRE_EQ(deepest_completed_depth(capture), 1);

      int reported_ms = -1;

      for (const std::string& line : capture.lines()) {
        const size_t at = line.find(" time ");

        if (line.rfind("info ", 0) != 0 || at == std::string::npos) {
          continue;
        }

        reported_ms = std::atoi(line.c_str() + at + 6);
      }

      REQUIRE_MESSAGE(
          reported_ms >= depth_1_floor_ms,
          ("depth 1 on this position now takes " + std::to_string(reported_ms) +
           " ms, which is too fast for this case to separate; "
           "pick a heavier position"));

      uci_shutdown();
    }

    // `stop`, with no timer anywhere: the hard limit is a hundred seconds out
    // and the only thing that can end this search is the signal.
    {
      uci_init();

      stdout_capture_t capture;
      uci_process_line("position fen " + fen);
      uci_process_line("go movetime 100000");
      uci_process_line("stop");
      uci_wait_for_search();

      CHECK_MESSAGE(deepest_completed_depth(capture) < 1,
                    ("the first iteration ran to the end through a stop:\n" +
                     capture.str()));

      CHECK_MESSAGE(
          answered_a_playable_move(capture),
          ("a cut depth-1 iteration still owes a move:\n" + capture.str()));

      uci_shutdown();
    }

    // The hard timer, which is the same pointer reached from the other side.
    {
      uci_init();

      stdout_capture_t capture;
      uci_process_line("position fen " + fen);
      uci_process_line("go movetime 1");
      uci_wait_for_search();

      CHECK_MESSAGE(
          deepest_completed_depth(capture) < 1,
          ("the first iteration ran past its hard limit:\n" + capture.str()));

      CHECK_MESSAGE(
          answered_a_playable_move(capture),
          ("a cut depth-1 iteration still owes a move:\n" + capture.str()));

      uci_shutdown();
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

  // S176, 2026-09-03_adversarial-F04. The case above covers only a FEN with no
  // moves applied, where the last FEN and the position coincide. With moves
  // applied, a failed load reloaded `initial_position` -- the last FEN -- and
  // the moves were gone: after `startpos moves e2e4` one malformed FEN put the
  // engine on the start position, and the moves after the bad FEN were then
  // applied to it.
  TEST_CASE("a malformed fen keeps the previous position and its moves")
  {
    uci_init();

    std::string after_e2e4;
    {
      stdout_capture_t capture;
      uci_process_line("position startpos moves e2e4");
      after_e2e4 = generate_FEN(&uci_game()->board);
      REQUIRE_NE(after_e2e4, std::string(DEFAULT_POSITION));

      uci_process_line(
          "position fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a9 "
          "0 1 moves e7e5");
      CHECK(capture.contains(
          "info string refused [position fen] "
          "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a9 0 1, does not "
          "load"));
    }

    // Neither the bad FEN nor the moves after it reached the board.
    REQUIRE_EQ(generate_FEN(&uci_game()->board), after_e2e4);

    uci_shutdown();
  }

  // The FEN standard's clocks are optional in practice: Stockfish, cutechess
  // and python-chess all accept four and five fields. Chesso dropped the short
  // form silently and stayed on whatever position it had (F04), and read
  // `moves` as the fifth field when the short form had moves after it.
  TEST_CASE("a four- or five-field fen loads with the clocks defaulted")
  {
    uci_init();

    const std::string kings = "8/8/8/4k3/8/4K3/8/8 w - -";

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("position fen " + kings);
    }
    REQUIRE_EQ(generate_FEN(&uci_game()->board), kings + " 0 1");

    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("position fen " + kings + " 7");
    }
    REQUIRE_EQ(generate_FEN(&uci_game()->board), kings + " 7 1");

    // With moves: the short form lands where the six-field form lands. The
    // expected FEN comes from the engine, not from a board kept in the head.
    std::string six_fields_then_move;
    {
      stdout_capture_t capture;
      uci_process_line("position fen " + kings + " 0 1 moves e3d3");
      six_fields_then_move = generate_FEN(&uci_game()->board);
      REQUIRE_NE(six_fields_then_move, kings + " 0 1");

      uci_process_line("position startpos");
      uci_process_line("position fen " + kings + " moves e3d3");
    }
    REQUIRE_EQ(generate_FEN(&uci_game()->board), six_fields_then_move);

    uci_shutdown();
  }

  TEST_CASE("a fen with fewer than four fields is refused and changes nothing")
  {
    uci_init();

    std::string after_e2e4;
    {
      stdout_capture_t capture;
      uci_process_line("position startpos moves e2e4");
      after_e2e4 = generate_FEN(&uci_game()->board);

      // e7e5 is legal on the board the engine holds, so applying it would be
      // the visible failure: the refusal has to end the whole command.
      uci_process_line("position fen 8/8/8/4k3/8/4K3/8/8 w - moves e7e5");
      CHECK(capture.contains(
          "info string refused [position fen] 8/8/8/4k3/8/4K3/8/8 w -, fewer "
          "than four fields"));
    }

    REQUIRE_EQ(generate_FEN(&uci_game()->board), after_e2e4);

    uci_shutdown();
  }


  // S210, 2026-09-04_adversarial-F02. The S176 rule above was the FEN half of
  // this command; the moves half did the opposite. A token
  // algebraic_to_uci_move() rejected was passed over, a move try_move() could
  // not find was answered with LOG_W -- `if (false)` under NDEBUG, so silent
  // in the binary that ships -- and the rest of the list was applied over the
  // hole. `position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 zzzz a7a6 b5a4`
  // ended on the board the GUI would have had after 5...a6 6.Ba4, one ply
  // short of the line it sent, and every later `bestmove` was judged against a
  // board the engine never computed on.
  //
  // The reasoning S176 wrote down for the FEN half is the reasoning here:
  // applying the moves that follow puts the engine somewhere the GUI did not
  // send it. Both refusals now end the command with the engine on the position
  // it had **before the command**, which is what the third subcase pins --
  // a legal prefix does not survive a bad token later in the same line.
  TEST_CASE("a moves token that does not play refuses the whole command")
  {
    struct case_t
    {
      std::string line;
      std::string refusal;
      std::string title;
    };

    const std::vector<case_t> cases = {
        {"position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 zzzz a7a6 b5a4",
         "info string refused [position moves] zzzz, does not parse",
         "a token that is not a move at all"},
        // Syntactically a move, and not one this board has: it is Black's turn
        // after 1.e4, and White's king is not on e1's castling squares anyway.
        {"position startpos moves e2e4 e1g1 e7e5",
         "info string refused [position moves] e1g1, not a legal move here",
         "a well-formed move that is illegal here"},
        // A promotion piece the parser does not know.
        {"position startpos moves e2e4x",
         "info string refused [position moves] e2e4x, does not parse",
         "a move with a character glued to it"},
    };

    for (const case_t& test : cases) {
      uci_init();

      // The board before the command, and deliberately not the start position:
      // a refusal that reset to `startpos` would pass a case that began there.
      std::string before;

      {
        stdout_capture_t capture;
        uci_process_line("position startpos moves d2d4 d7d5");
        before = generate_FEN(&uci_game()->board);
        REQUIRE_MESSAGE(before != std::string(DEFAULT_POSITION), test.title);

        uci_process_line(test.line);

        CHECK_MESSAGE(capture.contains(test.refusal),
                      (test.title + ": " + capture.str()));
      }

      CHECK_MESSAGE(generate_FEN(&uci_game()->board) == before, test.title);

      uci_shutdown();
    }
  }


  // S210, raised by the Tier-1 fast check over the first half. The command was
  // atomic for `game` and for nothing else. set_position() writes
  // `initial_position`, calls tt_reset() and re-arms `still_in_opening`
  // *before* the moves are applied, and the moves loop's refusal restored none
  // of the three: `position <a different base> moves zzzz` wiped a table the
  // GUI had just paid a search for, left `initial_position` naming a board the
  // engine is not standing on, and put the opening book back on for a command
  // that was refused. The board was the only thing that came back.
  //
  // The table is the half a test can see. uci_tt() is the engine's own table
  // and tt_get_entry() answers for a board; `initial_position` and
  // `still_in_opening` are file statics in src/chesso.cpp with no accessor.
  // All three are fixed the same way -- nothing is written until the whole
  // command has succeeded -- so the table standing is what pins the rule.
  TEST_CASE("a refused position command leaves the table alone")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("position startpos moves e2e4 e7e5");
      uci_process_line("go depth 6");
      uci_wait_for_search();
    }

    const std::string before = generate_FEN(&uci_game()->board);

    // The precondition, without which the check below cannot fail: the search
    // just filled the table, so this board has an entry there to lose.
    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE(load_FEN(generate_FEN(&game.board), &game));
    REQUIRE(tt_get_entry(uci_tt(), &game.board) != nullptr);

    // A *different* base, so set_position() takes its `initial_position != fen`
    // branch, followed by a token that cannot play, so the command is refused
    // after that branch would have run.
    {
      stdout_capture_t capture;
      uci_process_line("position kiwipete moves zzzz");

      REQUIRE_MESSAGE(
          capture.contains(
              "info string refused [position moves] zzzz, does not parse"),
          capture.str());
    }

    CHECK_EQ(generate_FEN(&uci_game()->board), before);

    CHECK_MESSAGE(tt_get_entry(uci_tt(), &game.board) != nullptr,
                  "a refused [position] cleared the transposition table");

    uci_shutdown();
  }


  // S210, 2026-09-10_adversarial-F18. At 4999 plies the root's own make_move()
  // refused every move, first_legal_move() could not rescue it because it calls
  // make_move() too, and the engine answered `bestmove 0000` with 22 legal
  // moves on the board. Refusing only what make_move() refuses does not close
  // it: the last accepted move leaves the history one entry from the end and
  // the search still cannot push a ply. The command keeps MAX_PLY of the stack
  // for the search instead, and POSITION_MAX_PLIES is that bound.
  //
  // Three assertions and the middle one is the finding: the bound is reachable,
  // a search from a position exactly at it still answers a move, and one ply
  // past it is refused with the board unchanged.
  TEST_CASE("a moves list past the history bound is refused and still answers")
  {
    auto shuffle_line = [](size_t plies) {
      std::string line = "position startpos moves";
      const char* cycle[4] = {" g1f3", " b8c6", " f3g1", " c6b8"};

      for (size_t ply = 0; ply < plies; ++ply) {
        line += cycle[ply % 4];
      }

      return line;
    };

    uci_init();

    std::string at_the_bound;

    {
      stdout_capture_t capture;
      uci_process_line(shuffle_line(POSITION_MAX_PLIES));

      REQUIRE_MESSAGE(!capture.contains("info string refused"), capture.str());

      REQUIRE_EQ(uci_game()->history.size, size_t(POSITION_MAX_PLIES));
      at_the_bound = generate_FEN(&uci_game()->board);
    }

    // The symptom. A position the command accepted is a position the search
    // can move in.
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();

      bool answered = false;

      for (const std::string& line : capture.lines()) {
        if (line.rfind("bestmove ", 0) != 0) { continue; }

        answered = true;
        CHECK_MESSAGE(line != "bestmove 0000",
                      "the engine answered the null move with legal moves on "
                      "the board");
      }

      REQUIRE(answered);
    }

    // One ply further is refused, and the refusal leaves the engine on the
    // board it had -- which here is the one the accepted line left.
    {
      stdout_capture_t capture;
      uci_process_line(shuffle_line(POSITION_MAX_PLIES + 1));

      CHECK_MESSAGE(capture.contains("info string refused [position moves] "),
                    capture.str());
      CHECK_MESSAGE(
          capture.contains("the move stack is full at " +
                           std::to_string(POSITION_MAX_PLIES) + " plies"),
          capture.str());
    }

    CHECK_EQ(generate_FEN(&uci_game()->board), at_the_bound);

    // The audit's own reproduction: the longest line make_move() alone
    // tolerates. Every ply of it played before this step, leaving the history
    // one entry from the end, and the search then had nowhere to put a move --
    // every root move refused, first_legal_move() refused too, `bestmove 0000`
    // with legal moves on the board. It is refused here, and the assertion
    // that matters is the one after it: whatever the engine is standing on, it
    // answers with a move.
    {
      stdout_capture_t capture;
      uci_process_line(shuffle_line(HISTORY_MAX_SIZE - 1));

      CHECK_MESSAGE(
          capture.contains("the move stack is full at " +
                           std::to_string(POSITION_MAX_PLIES) + " plies"),
          capture.str());
    }

    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();

      bool answered = false;

      for (const std::string& line : capture.lines()) {
        if (line.rfind("bestmove ", 0) != 0) { continue; }

        answered = true;
        CHECK_MESSAGE(line != "bestmove 0000",
                      ("the engine answered the null move after a "
                       "position line of " +
                       std::to_string(HISTORY_MAX_SIZE - 1) + " plies:\n" +
                       capture.str()));
      }

      REQUIRE(answered);
    }

    uci_shutdown();
  }


  // S210, and the decision S209 left open. The `go` numbers were read with
  // std::stoll(), which stops at the first character it cannot use and says
  // nothing in a release build: `wtime 0x1000` was a clock of zero, which
  // drops the search onto the no-limit fallback, and `movetime 100abc` was
  // 100 ms. Same class as the `Hash` misparse S209 removed, on the line that
  // decides how much clock a move gets, so the same whole-token rule and the
  // same two message shapes apply -- on the UCI channel, which is legal UCI in
  // every build state.
  TEST_CASE("go refuses a limit that is not an integer in full")
  {
    struct case_t
    {
      std::string line;
      std::string refusal;
    };

    // Every line carries a second, well-formed limit: a refused token is
    // dropped, and a `go` left with no limit at all falls back to a whole
    // second of searching that this case has no use for.
    const std::vector<case_t> cases = {
        {"go depth 0x4 movetime 20",
         "info string refused [go depth] 0x4, not an integer"},
        {"go depth 4abc movetime 20",
         "info string refused [go depth] 4abc, not an integer"},
        {"go depth 4.5 movetime 20",
         "info string refused [go depth] 4.5, not an integer"},
        {"go movetime 20x depth 1",
         "info string refused [go movetime] 20x, not an integer"},
        {"go nodes 1e6 depth 1",
         "info string refused [go nodes] 1e6, not an integer"},
        {"go wtime 0x1000 btime 1000 depth 1",
         "info string refused [go wtime] 0x1000, not an integer"},
        {"go movestogo abc depth 1",
         "info string refused [go movestogo] abc, not an integer"},
        // A leading sign std::from_chars does not read and std::stoll did:
        // `+5` was depth 5 until S210. `Hash` refuses `+64` the same way and
        // MANUAL.md lists both.
        {"go depth +5 movetime 20",
         "info string refused [go depth] +5, not an integer"},
        // A well-formed integer no long long can hold is out of range, not
        // malformed -- the distinction stoll()'s single catch could not make.
        {"go depth 99999999999999999999 movetime 20",
         "info string refused [go depth] 99999999999999999999, out of range"},
    };

    for (const case_t& test : cases) {
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line("position startpos");
        uci_process_line(test.line);
        uci_wait_for_search();

        CHECK_MESSAGE(capture.contains(test.refusal),
                      (test.line + " -> " + capture.str()));

        // The refused token is dropped and the rest of the line still runs, so
        // the search still answers. A `go` that says nothing back is worse
        // than a `go` that answers on a default.
        bool answered = false;

        for (const std::string& line : capture.lines()) {
          if (line.rfind("bestmove ", 0) == 0) { answered = true; }
        }

        CHECK_MESSAGE(answered, (test.line + " never answered"));
      }

      uci_shutdown();
    }
  }


  // The control the case above needs: a token that *is* a whole integer is
  // still read, and a value outside the field's range is still clamped rather
  // than refused. Without this, refusing everything would be green up there.
  TEST_CASE("go still reads a whole integer, and still clamps its range")
  {
    // `depth 999` and `depth -3` clamp into [1, MAX_DEPTH] as they always did,
    // which is why the first carries a movetime: clamped to MAX_DEPTH it is
    // otherwise an unbounded search. `nodes -1` is read as signed and becomes
    // no budget rather than an enormous one, so it carries a depth.
    const std::vector<std::string> lines = {
        "go depth 2",
        "go depth 999 movetime 20",
        "go depth -3",
        "go nodes 5000",
        "go nodes -1 depth 1",
        "go movetime 20",
        "go movestogo 0 depth 1",
    };

    for (const std::string& line : lines) {
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line("position startpos");
        uci_process_line(line);
        uci_wait_for_search();

        CHECK_MESSAGE(!capture.contains("info string refused"),
                      (line + " -> " + capture.str()));
      }

      uci_shutdown();
    }
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
    REQUIRE(capture.contains("option name OwnBook type check"));
    REQUIRE(capture.contains("option name Book File type string"));
    REQUIRE(capture.contains("option name Best Book Move type check"));
    REQUIRE(capture.contains("uciok"));

    uci_shutdown();
  }

  // S172. `Book File` is the first option here whose value can contain a space,
  // and setoption used to take the first token after `value` and drop the rest:
  // `/Users/max/My Books/x.bin` reached the loader as `/Users/max/My`. Nothing
  // caught it because Hash, Threads and the search parameters are all single
  // tokens. The refusal names the path it tried, so it is what the test reads.
  TEST_CASE("setoption carries a value containing spaces")
  {
    uci_init();

    const std::string path = "/nonexistent dir/with spaces/book.bin";

    stdout_capture_t capture;
    uci_process_line("setoption name Book File value " + path);

    REQUIRE(capture.contains("book [" + path + "] not loaded"));

    // And the engine is bookless rather than back on the built-in book: a
    // harness that asked for one book and silently got another is measuring
    // something nobody configured.
    //
    // Read off an `info score` line, not off "Found position in the opening
    // book": that string is LOG_I, which is `if (false)` in the Release build
    // the gate runs, and it goes to std::clog while this capture reads stdout,
    // so the old assertion could not fail in either build. command_go answers a
    // book hit with `bestmove` alone -- its info line is commented out -- so an
    // `info score` line is the observable that says a real search ran.
    // S193, 2026-09-04_test_review-F05.
    uci_process_line("setoption name OwnBook value true");
    uci_process_line("position startpos");
    uci_process_line("go depth 1");
    uci_wait_for_search();

    REQUIRE(capture.contains("info score "));

    uci_process_line("setoption name OwnBook value false");
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);

    uci_shutdown();
  }

  // S209, 2026-09-10_adversarial-F12. `UCI.txt`, the copy of the protocol this
  // repository ships: "The name and value of the option in <id> should not be
  // case sensitive". command_setoption compared with `==`, so `hash` and
  // `ownbook` -- both of them legal UCI -- were refused as unknown options and
  // `True` was ignored for a check option, and the release build said nothing
  // about any of it: a harness that wrote `hash` measured the default and never
  // learned. That is the failure class DEC-093 and S137 exist for, one option
  // along.
  TEST_CASE("setoption folds the option name and a check value")
  {
    uci_init();

    // There is no readback of a live option value (specs.md), so each half
    // needs an observable. For a spin option it is the table: what proves the
    // option was honoured is the size it asked for arriving.
    const size_t at_default = uci_tt()->entry_count;

    REQUIRE(at_default > 0);

    {
      stdout_capture_t capture;
      uci_process_line("setoption name hash value 64");
    }
    CHECK(uci_tt()->entry_count > at_default);

    {
      stdout_capture_t capture;
      uci_process_line("setoption name HASH value 1");
    }
    CHECK(uci_tt()->entry_count < at_default);

    // The control, in the spelling that always worked.
    uci_process_line("setoption name Hash value 16");
    REQUIRE_EQ(uci_tt()->entry_count, at_default);

    // For a check option it is the book. A book hit answers with `bestmove`
    // alone -- command_go's info line is commented out -- so an `info score`
    // line is what says a real search ran, the observable S193 put under the
    // case above. The book has to be the embedded one for that: the case above
    // leaves it there, and this says so rather than inheriting it.
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);

    {
      // The precondition. With the book off the search runs and prints, so its
      // silence below is a book move and not a dead stream.
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("go depth 1");
      uci_wait_for_search();

      REQUIRE(capture.contains("info score "));
      REQUIRE(capture.contains("bestmove "));
    }

    {
      // Both halves at once: a folded name and a folded check value.
      stdout_capture_t capture;
      uci_process_line("setoption name ownbook value True");
      uci_process_line("position startpos");
      uci_process_line("go depth 1");
      uci_wait_for_search();

      CHECK(capture.contains("bestmove "));
      CHECK_FALSE(capture.contains("info score "));
    }

    {
      // And off again, folded the other way, which is the `false` branch of
      // the same comparison.
      stdout_capture_t capture;
      uci_process_line("setoption name OWNBOOK value FALSE");
      uci_process_line("position startpos");
      uci_process_line("go depth 1");
      uci_wait_for_search();

      CHECK(capture.contains("info score "));
    }

    // Cleanup, whichever way the checks above went: these are process globals
    // and the next case would inherit a book that is still on.
    uci_process_line("setoption name OwnBook value false");

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

      // [position] stops whatever was running, and [go] is the only path that
      // clears the stop flag again - iterative_deepening_search documents that
      // its caller has cleared it before the timer is armed. S193, DEC-163,
      // 2026-09-04_test_review-F05, and this case is the one the sweep
      // missed: without it the direct call below aborts on the stale flag from
      // its first check_limits(), so the budget is not what stops it and the
      // case cannot see the bug it names. Measured on the tree that had no
      // preamble here: the same call with the budget removed entirely
      // (options.nodes = 0, nothing bounding the search at all) still returned
      // a legal move after 49 nodes. A depth-limited [go] arms no timer, so
      // the flag stays clear afterwards.
      uci_process_line("go depth 1");
      uci_wait_for_search();
    }

    // The precondition, which is what makes the one-node search below mean
    // something: with the flag clear a direct call runs to whatever budget it
    // is given, so a budget far past the 49 nodes above has to be reachable.
    // No golden -- the figure asserted is the budget itself, and check_limits()
    // stops at `explored_nodes >= node_limit`.
    {
      uci_search_options_t roomy = {};
      roomy.depth = MAX_DEPTH;
      roomy.nodes = 100000;

      uci_search_result_t reached;
      {
        stdout_capture_t capture;
        reached = iterative_deepening_search(roomy);
      }

      REQUIRE_MESSAGE(reached.total_node_explored >= roomy.nodes,
                      ("a 100000 node budget stopped after " +
                       std::to_string(reached.total_node_explored) +
                       " nodes, so something other than the budget is ending "
                       "these searches"));
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

    // `position` runs stop_and_join_search(), which *sets* stop_search_signal.
    // Without this the search below reads the stale flag at its first
    // check_limits() -- since S210's F21 the first iteration reads the real
    // signal too -- and the case measured about a millisecond while claiming to
    // bound a 200 ms search: it passed its `elapsed < 30000` for the wrong
    // reason, and it read depth 1 rather than depth 0 only because the flag
    // used to be ignored until the first iteration had returned. A completed
    // `go` is the only
    // way a test can clear the flag: begin_search_session() is what clears it
    // and no header declares it, and `ucinewgame` sets it again through the
    // same stop_and_join_search(). S193, DEC-163, 2026-09-04_test_review-F05.
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
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

    // And it is bounded from below, which is what makes the bound above mean
    // something: half the 200 ms budget, so a machine that starts no further
    // iteration still clears it. Five runs on the DEC-049 workstation read 239,
    // 235, 238, 232 and 239 ms; the same case read 0 ms before the clearing
    // above was added.
    REQUIRE(elapsed >= 100);

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
  // S192, DEC-142: **construction, not measurement.** Until this step both
  // subcases read their stability and their fall off a fixed position's tree,
  // and eight of the twenty-two search mutants of the 2026-09-04 review turned
  // this case red without being time-management defects at all -- M06a, one ply
  // off the reverse futility floor, took `REQUIRE(scaled.drop == 0)` red and
  // nothing else in this binary. A guard that fires on any change to the tree
  // says nothing about the rule it is named after.
  //
  // The root below is the suite's mate in one, tool-verified where it is used
  // for that ("mate in one" in tests/test_search.cpp, whose row for this FEN
  // asserts Ra1-a8 and mate in 1; DEC-023). The best move is the mating move at
  // every iteration and the score is the same mate score, whatever the pruning
  // rules do deeper, so at [go depth d] with no clock the loop counts a
  // stability of exactly d - 1 and a fall of exactly 0 on any machine and after
  // any search change.
  //
  // Both runs are fixed-depth with no clock and no node budget, so nothing
  // arms a timer and the iteration count is not whatever the machine got
  // through.
  TEST_CASE(
      "the iteration loop scales its soft limit by the history it counted")
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
        // Without this the direct call below aborts on the stale flag at its
        // first check_limits(), before any iteration completes, and the probe
        // reads whatever the fallback answered instead of a searched tree.
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
          "position fen 7k/6pp/8/8/8/8/8/R6K w - - 0 1";

      for (int depth : {2, 5, 8}) {
        const probe_t scaled = probe(position, depth, true);

        // The construction held. Both are exact numbers rather than
        // inequalities on whatever the tree did: the first iteration cannot
        // repeat a previous move and has no previous score, and every one
        // after it repeats Ra1-a8 at the same mate score.
        REQUIRE_EQ(scaled.stability, depth - 1);
        REQUIRE_EQ(scaled.drop, 0);

        CHECK_EQ(scaled.scale,
                 search_time_scale_percent(scaled.stability, scaled.drop));

        // TM_STABILITY_PERCENT > 0 is asserted by the pure case above, so a
        // stability of one or more has to buy a discount here.
        CHECK(scaled.scale < 100);

        // And the search does not get to move a time the GUI named. Same
        // position, same depth, same two inputs - the loop still records them -
        // and no scaling, which is what [go movetime] and the no-limit fallback
        // ask for. The run above is what makes this one non-vacuous: without it
        // a scale that never moved at all would satisfy it.
        const probe_t fixed = probe(position, depth, false);

        REQUIRE_EQ(fixed.stability, scaled.stability);
        REQUIRE_EQ(fixed.drop, scaled.drop);
        CHECK_EQ(fixed.scale, 100);
      }
    }

    SUBCASE("a fall reaches the time manager on at least one position")
    {
      // S192's fast check proved this one is needed rather than argued it:
      // with the old case's `REQUIRE(scaled.drop > 0)` gone, mutant
      // M34_score_drop_always_zero -- the loop handing the time manager a
      // constant zero fall -- **survived the whole fast label**. The identity
      // below cannot catch it, because the loop records the same zero it
      // passed.
      //
      // A fall between iterations is a property of a tree and cannot be
      // constructed, so what is asserted is over the set and not per position:
      // at least one of these falls. A search change that stops one position
      // falling leaves this green; only a loop that reports no fall anywhere
      // takes it red, which is what a defect looks like. The counts are printed
      // either way, so a set drifting toward vacuity is visible before it
      // arrives. Same shape as `test_mate_carry`'s majority (DEC-162).
      const std::vector<std::string> positions = {
          KIWIPETE_POS, BLOCKED_CENTRE_POS, KILLER_POS, DEFAULT_POSITION};

      int falling = 0;
      std::string counts;

      for (const std::string& fen : positions) {
        const probe_t p = probe("position fen " + fen, 8, true);

        // The identity, at every position and at whatever fall it produced.
        CHECK_EQ(p.scale, search_time_scale_percent(p.stability, p.drop));

        if (p.drop > 0) { falling++; }

        counts += "  " + std::to_string(p.stability) + " stable, " +
                  std::to_string(p.drop) + " cp, " + std::to_string(p.scale) +
                  "%\n";
      }

      MESSAGE("per position:\n" << counts);

      REQUIRE_MESSAGE(falling > 0,
                      ("no position reported a fall, so nothing here shows the "
                       "loop measuring one:\n" +
                       counts));
    }

    SUBCASE("the fall the loop counted is the fall the scale was computed from")
    {
      // The other half of the rule, and the half no construction reaches: a
      // root whose score falls is a property of the tree, so what is asserted
      // is the identity alone, at whatever fall this position produces today.
      // No precondition on the number - that is what made the old case fire on
      // eight mutants - and the fall itself is reported rather than pinned.
      //
      // What the fall *does* to the scale is held as arithmetic by "the time
      // scale moves with stability and with a falling score" above, which
      // cannot move with the tree because it never runs a search.
      const probe_t scaled = probe(
          "position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/"
          "R3K2R w KQkq - 0 1",
          8, true);

      MESSAGE("loop counted stability " << scaled.stability << ", fall "
                                        << scaled.drop << " cp, scale "
                                        << scaled.scale << "%");

      CHECK_EQ(scaled.scale,
               search_time_scale_percent(scaled.stability, scaled.drop));
    }
  }


  // S132. The third factor on the same soft limit, held against its own
  // arithmetic. What this case cannot show is that the search ever calls it,
  // which is what the loop case below is for -- the S089 pair above is split
  // the same way and for the same reason.
  TEST_CASE("the node factor falls as the best move takes more of the tree")
  {
    // Monotone over the whole range of shares there is. Non-strict because
    // the arithmetic is integer: at TmNodeScalePct 151 a point of share is
    // 1.51 points of factor, so some steps are 1 and some are 2.
    for (int share = 0; share < 100; ++share) {
      const int here = search_time_node_factor_percent(share);
      const int next = search_time_node_factor_percent(share + 1);

      REQUIRE_MESSAGE(
          next <= here,
          ("the factor rose from share " + std::to_string(share) + ": " +
           std::to_string(here) + " then " + std::to_string(next)));
      REQUIRE_MESSAGE(here >= 0,
                      ("negative factor at share " + std::to_string(share)));
    }

    // **TmNodeScalePct 0 is the rule's off value and one of this step's
    // pre-registered outcomes**, so everything below it is asserted about a
    // rule that is switched on. A suite that went red on the one-line revert
    // its own SPRT names would make that revert a rewrite.
    if (TM_NODE_SCALE_PCT == 0) {
      for (const int share : {0, 37, 50, 99, 100}) {
        CHECK_EQ(search_time_node_factor_percent(share), 100);
      }

      MESSAGE(
          "TmNodeScalePct is 0: the factor is 100 at every share and the "
          "soft limit is S089's alone");
      return;
    }

    // And it really falls. A rule that answered one constant would satisfy
    // every line above.
    CHECK(search_time_node_factor_percent(100) <
          search_time_node_factor_percent(0));

    // The whole swing is TmNodeScalePct, exactly: the two ends differ by
    // 100 * TmNodeScalePct / 100 in the reals and their truncations differ by
    // the same integer, because the two products differ by a whole number of
    // hundredths. This is the setting's declared meaning turned into an
    // assertion -- "the factor moves by exactly TmNodeScalePct points across
    // the full range of shares" is what src/search_params.hpp says it is.
    CHECK_EQ(search_time_node_factor_percent(0) -
                 search_time_node_factor_percent(100),
             TM_NODE_SCALE_PCT);

    // THE NEUTRAL SHARE, which is the census median the pair was solved for
    // (adocs/data/S132_node_share_census.py re-derives it, and the step's
    // stamp records it with its quartiles). Reported always, asserted only
    // where the constants keep it inside the range of shares that exist: a
    // later fit is allowed to move it out, and a case that went red for that
    // would be asserting a seeding constraint rather than a rule.
    REQUIRE(TM_NODE_SCALE_PCT > 0);

    const int neutral =
        TM_NODE_BASE_PCT - (10000 / std::max(1, TM_NODE_SCALE_PCT));

    MESSAGE("neutral share "
            << neutral << " %, factor there "
            << search_time_node_factor_percent(std::clamp(neutral, 0, 100))
            << " %, at a share of 100 % "
            << search_time_node_factor_percent(100) << " %, floor "
            << TM_SCALE_MIN_PERCENT << " %");

    if (neutral >= 0 && neutral <= 100) {
      CHECK(std::abs(search_time_node_factor_percent(neutral) - 100) <= 2);
    }

#ifdef CHESSO_TUNE
    // THE OFF VALUE, DEC-215, and this is the build that can prove it here: at
    // TmNodeScalePct 0 the factor is 100 at every share, so the soft limit is
    // S089's alone and the engine is the one before this step. The release
    // build proves the same thing on the tree instead -- the bench total and
    // the eight replies, recorded in this step's file -- because a constant it
    // folded cannot be moved from a test.
    //
    // The formula alone would answer 0 here, which floors every iteration and
    // is the opposite of off. That is the whole reason the function has a
    // branch in front of it.
    const int shipped_scale = TM_NODE_SCALE_PCT;

    REQUIRE(search_param_set("TmNodeScalePct", 0));

    for (const int share : {0, 37, 50, 99, 100}) {
      CHECK_EQ(search_time_node_factor_percent(share), 100);
    }

    // Restored, and the restoration checked: a case that left the parameter at
    // 0 would switch the rule off for every case that runs after it.
    REQUIRE(search_param_set("TmNodeScalePct", shipped_scale));
    REQUIRE(TM_NODE_SCALE_PCT == shipped_scale);
    CHECK(search_time_node_factor_percent(100) < 100);
#endif
  }


  // S132, and the same division of labour as S089's pair above: the function
  // is pure, so a loop could compute it correctly on every iteration and never
  // look at the answer. What is pinned here is the loop -- the share it
  // measured, the factor it derived, the product it floored and the limit that
  // produced, all four read back from the search that ran.
  //
  // The root is two bare kings with exactly one legal move, and every
  // precondition it is chosen for is asserted rather than described: one legal
  // move makes the best move's share of the root's nodes exactly 100 %, and a
  // position nothing can be won in scores the same at every depth, so the fall
  // is 0 and the stability is the iteration count less one, on any machine and
  // after any search change. The mate-in-one root the S089 case uses cannot do
  // this job: it has twenty-odd root moves and its share is a property of the
  // tree (DEC-142, and S192's reason for rebuilding that case by
  // construction).
  TEST_CASE_FIXTURE(
      engine_fixture_t,
      "the iteration loop scales its soft limit by the share it measured")
  {
    struct probe_t
    {
      int stability;
      int drop;
      int scale;         // S089's two scalers, floored, as before
      int node_percent;  // the share of the root's nodes the best move took
      int node_factor;   // what that share was worth
      int soft_scale;    // the three multiplied and floored
      int64_t soft_ms;   // and the limit that produced, after the clamp
    };

    // Two kings, black to move, boxed so that a8b8 is the only legal move.
    const std::string one_move_fen = "k7/8/1K6/8/8/8/8/8 b - - 0 1";

    auto probe = [](const std::string& fen, int depth, bool scale_time,
                    int hard_ms, int soft_base_ms) -> probe_t {
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line("position fen " + fen);

        // [position] stops whatever was running and [go] is the only path that
        // clears the stop flag again, exactly as the S089 probe above
        // documents: without this the direct call below aborts at its first
        // check_limits() and the probe reads the fallback.
        uci_process_line("go depth 1");
        uci_wait_for_search();
      }

      uci_search_options_t options = {};
      options.depth = depth;
      options.scale_time = scale_time;
      options.search_time_ms = hard_ms;
      options.search_soft_time_ms = soft_base_ms;

      uci_search_result_t result = {};
      {
        stdout_capture_t capture;
        result = iterative_deepening_search(options);
      }

      REQUIRE(result.best_move != 0);

      const probe_t out = {
          uci_last_best_move_stability(), uci_last_score_drop_cp(),
          uci_last_time_scale_percent(),  uci_last_bestmove_node_percent(),
          uci_last_node_factor_percent(), uci_last_soft_scale_percent(),
          uci_last_soft_limit_ms()};

      uci_shutdown();

      return out;
    };

    SUBCASE("one legal move is the whole tree and buys the floor")
    {
      // The construction, asserted. Everything below is read off a root whose
      // only move is forced and whose score cannot move.
      REQUIRE(load_FEN(one_move_fen, &game));

      move_t moves[MAX_MOVES];

      REQUIRE_EQ(legal_moves(&game, moves), 1u);

      for (const int depth : {4, 6, 8}) {
        const probe_t p = probe(one_move_fen, depth, true, 0, 0);

        const std::string where = "depth " + std::to_string(depth);

        REQUIRE_MESSAGE(p.stability == depth - 1, where);
        REQUIRE_MESSAGE(p.drop == 0, where);

        // The share. One legal move takes every node under the root.
        CHECK_MESSAGE(p.node_percent == 100, where);

        // What that share was worth, and THE IDENTITY: the product the loop
        // used is the three numbers it recorded, floored. Both hold whatever
        // the settings are, the off value included.
        CHECK_MESSAGE(
            p.node_factor == search_time_node_factor_percent(p.node_percent),
            where);
        CHECK_MESSAGE(p.soft_scale == std::max((p.scale * p.node_factor) / 100,
                                               TM_SCALE_MIN_PERCENT),
                      where);

        // The rest of the subcase is about a rule that is switched on.
        // TmNodeScalePct 0 is this step's own pre-registered revert and not a
        // failure of anything.
        if (TM_NODE_SCALE_PCT == 0) {
          CHECK_MESSAGE(p.node_factor == 100, where);
          CHECK_MESSAGE(p.soft_scale == p.scale, where);
          continue;
        }

        // It is a discount, which is the rule's whole claim: a choice that
        // was never in doubt does not need the next iteration.
        CHECK_MESSAGE(p.node_factor < 100, where);

        // And the floor is on the product. The precondition is what makes it
        // non-vacuous here: S089's discount at this stability times a factor
        // at a share of 100 % lands under TmScaleMinPercent, which is the
        // multiplicative-stacking hazard the step file's section 5 names,
        // arriving on the first position anyone would try.
        REQUIRE_MESSAGE((p.scale * p.node_factor) / 100 < TM_SCALE_MIN_PERCENT,
                        ("the construction no longer under-spends: scale " +
                         std::to_string(p.scale) + " times factor " +
                         std::to_string(p.node_factor) + ", " + where));

        CHECK_MESSAGE(p.soft_scale == TM_SCALE_MIN_PERCENT, where);
      }
    }

    SUBCASE("below the depth gate the share is measured and not used")
    {
      // TmNodeMinDepth is chesso's own guard against a shallow iteration's
      // distribution being noise. Below it the soft limit is S089's alone,
      // while the share is still recorded -- it is a property of the tree and
      // the census reads it at every depth.
      if (TM_NODE_MIN_DEPTH < 2 || TM_NODE_SCALE_PCT == 0) {
        MESSAGE("TmNodeMinDepth is "
                << TM_NODE_MIN_DEPTH << " and TmNodeScalePct is "
                << TM_NODE_SCALE_PCT
                << ", so there is no depth below the gate to probe or no rule "
                   "to gate, and this subcase is empty");
      } else {
        const probe_t gated =
            probe(one_move_fen, TM_NODE_MIN_DEPTH - 1, true, 0, 0);

        // Same share as above, and the factor neutral anyway.
        CHECK_EQ(gated.node_percent, 100);
        CHECK_EQ(gated.node_factor, 100);
        CHECK_EQ(gated.soft_scale, gated.scale);

        // Non-vacuous: at the gate's own depth the same position is scaled.
        const probe_t open = probe(one_move_fen, TM_NODE_MIN_DEPTH, true, 0, 0);

        CHECK_EQ(open.node_percent, 100);
        CHECK(open.node_factor < 100);
        CHECK(open.soft_scale < open.scale);
      }
    }

    SUBCASE("the scaled soft limit never passes the hard limit")
    {
      // The clamp is the promise compute_search_time_budget() makes about the
      // clock and this step does not touch it -- but it now sits downstream of
      // a factor that can grant as well as cut, so it is asserted rather than
      // assumed. Deterministic on both sides: the two limits are handed to the
      // loop directly, so neither this case's reading nor its precondition
      // depends on what the tree did.
      const probe_t clamped = probe(one_move_fen, 8, true, 100, 1000);

      CHECK_EQ(clamped.soft_ms, 100);

      // And it binds only where it should: the same soft base under a hard
      // limit far above it keeps the scaled number.
      const probe_t free_run = probe(one_move_fen, 8, true, 100000, 1000);

      CHECK_EQ(free_run.soft_ms, (1000 * free_run.soft_scale) / 100);
      CHECK(free_run.soft_ms > clamped.soft_ms);
    }

    SUBCASE("the share is taken over the whole search, not over one iteration")
    {
      // The loop-level identity, and it is exact on any tree and any machine:
      // every search() call counts its own root node outside every bucket,
      // and the loop makes one call per iteration plus one per aspiration
      // widening. So the nodes it reported are the buckets plus that many.
      //
      // What it fails on is the published record's own pair of bugs at this
      // technique -- buckets cleared between iterations, and re-searches left
      // out of the accumulation, which one engine shipped and fixed
      // afterwards (the step file's section 1). A percentage alone cannot see
      // either: both report a plausible share of the wrong tree.
      const int depth = 8;

      uci_init();

      uci_search_result_t result = {};
      {
        stdout_capture_t capture;
        uci_process_line("position fen " + std::string(KIWIPETE_POS));
        uci_process_line("go depth 1");
        uci_wait_for_search();

        uci_search_options_t options = {};
        options.depth = depth;
        options.scale_time = true;

        result = iterative_deepening_search(options);
      }

      const uint64_t buckets = uci_last_root_nodes_total();
      const int widenings = uci_last_aspiration_failures();

      const std::string measured =
          "nodes " + std::to_string(result.total_node_explored) + ", buckets " +
          std::to_string(buckets) + ", " + std::to_string(depth) +
          " iterations, " + std::to_string(widenings) + " widenings";

      MESSAGE(measured);

      // Preconditions: the search really ran, and the accumulation really
      // spans more than one call -- without the second the identity would be
      // satisfied by a loop that accumulated nothing at all.
      REQUIRE_MESSAGE(result.best_move != 0, measured);
      REQUIRE_MESSAGE(buckets > 0, measured);
      REQUIRE_MESSAGE(depth + widenings > 1, measured);

      CHECK_MESSAGE(result.total_node_explored ==
                        buckets + static_cast<uint64_t>(depth + widenings),
                    measured);

      uci_shutdown();
    }

    SUBCASE("a time the GUI named with movetime is not scaled by any of it")
    {
      // The accepts' own clause, through the real command path rather than
      // through a direct call: `go movetime` sets both limits to the named
      // time and clears scale_time, so all three factors are neutral and the
      // limit is the number that arrived. The share is still measured, which
      // is what makes this a claim about the scaling and not about the
      // counting.
      uci_init();

      {
        stdout_capture_t capture;
        uci_process_line("position fen " + one_move_fen);
        uci_process_line("go movetime 120");
        uci_wait_for_search();
      }

      CHECK_EQ(uci_last_soft_limit_ms(), 120);
      CHECK_EQ(uci_last_node_factor_percent(), 100);
      CHECK_EQ(uci_last_soft_scale_percent(), 100);
      CHECK_EQ(uci_last_time_scale_percent(), 100);
      CHECK_EQ(uci_last_bestmove_node_percent(), 100);

      uci_shutdown();
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
      // direct call below aborts on the stale flag at its first
      // check_limits(), so no iteration completes and there is no run of info
      // lines to compare. Until S210's F21 it got one iteration out first, the
      // one that ran against a local never-stop flag; now it gets none. A
      // depth-limited [go] arms no timer, so the flag stays clear afterwards.
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


  // S209, 2026-09-10_adversarial-F13. std::stoll() was here and it stops at the
  // first character it cannot use without complaining, so `0x40` bought 1 MB
  // (0, clamped) and `64abc` bought 64, both of them silently; the `abc` it did
  // refuse went to LOG_W, which is `if (false)` under NDEBUG. DEC-088 pins the
  // harness's hash deliberately, and a typo that buys 1 MB measures an engine
  // nobody configured. The from_chars form the search parameters have used
  // since S137 is what this asks of Hash.
  TEST_CASE("setoption Hash refuses a value that is not an integer in full")
  {
    uci_init();

    const size_t at_default = uci_tt()->entry_count;

    REQUIRE(at_default > 0);

    // The precondition for every case below: a good value does resize, and it
    // prints nothing at all. Without it an unchanged count would not be a
    // refusal and a printed line would only show that the engine narrates
    // every setoption.
    {
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value 64");

      CHECK(capture.lines().empty());
    }
    REQUIRE(uci_tt()->entry_count > at_default);

    uci_process_line("setoption name Hash value 16");
    REQUIRE_EQ(uci_tt()->entry_count, at_default);

    for (const std::string& token :
         {std::string("0x40"), std::string("64abc"), std::string("12.5"),
          std::string("+64"), std::string("abc"), std::string("")}) {
      std::vector<std::string> printed;
      {
        stdout_capture_t capture;
        uci_process_line("setoption name Hash value " + token);
        printed = capture.lines();
      }

      const std::string first =
          printed.empty() ? std::string() : printed.front();

      CHECK_MESSAGE(printed.size() == 1,
                    ("[" + token + "] was answered with " +
                     std::to_string(printed.size()) + " lines"));
      CHECK_MESSAGE(
          first == "info string refused [Hash] " + token + ", not an integer",
          ("[" + token + "] was answered [" + first + "]"));

      // The refusal is a refusal: the line is not a warning printed on the way
      // to resizing anyway.
      CHECK_MESSAGE(uci_tt()->entry_count == at_default,
                    ("[" + token + "] moved the table to " +
                     std::to_string(uci_tt()->entry_count)));
    }

    // A well-formed integer that no long long can hold is out of range, not
    // malformed, whichever end it ran off -- the distinction the search
    // parameters have made since S137 and that stoll()'s single catch could
    // not.
    for (const std::string& token : {std::string("99999999999999999999"),
                                     std::string("-99999999999999999999")}) {
      std::vector<std::string> printed;
      {
        stdout_capture_t capture;
        uci_process_line("setoption name Hash value " + token);
        printed = capture.lines();
      }

      const std::string first =
          printed.empty() ? std::string() : printed.front();

      CHECK_MESSAGE(printed.size() == 1,
                    ("[" + token + "] was answered with " +
                     std::to_string(printed.size()) + " lines"));
      CHECK_MESSAGE(
          first == "info string refused [Hash] " + token + ", out of range",
          ("[" + token + "] was answered [" + first + "]"));
      CHECK_MESSAGE(uci_tt()->entry_count == at_default,
                    ("[" + token + "] moved the table to " +
                     std::to_string(uci_tt()->entry_count)));
    }

    // What is not refused and must not become refused: a whole integer outside
    // the range is still clamped, which is what S209 excludes from its scope.
    {
      stdout_capture_t capture;
      uci_process_line("setoption name Hash value -5");

      CHECK(capture.lines().empty());
    }
    CHECK(uci_tt()->entry_count < at_default);

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


  TEST_CASE("a stopped infinite search answers with a legal move")
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


  // S209, 2026-09-10_adversarial-F11. command_clean_TT called tt_reset()
  // without joining the search, against the rule the file states at
  // stop_and_join_search(): every path that mutates the board or the table must
  // call it first. A TSan build driving `go infinite` and 40 `clean-tt`
  // reported 38, 41 and 36 races over three runs -- tt_reset()'s memset against
  // tt_store_entry() and tt_get_entry() in the search thread -- where the same
  // harness with every other mid-search command reported 0, and where the same
  // build tree with the fix in reports 0. memset clears low to high, so a probe
  // can match a key not yet cleared and read a zeroed score under an un-zeroed
  // type. The audit's own figure was 11, taken without re-running the harness;
  // these are S209's measurements (DEC-178).
  //
  // The race itself is a sanitizer's to see. What a suite in either build can
  // see is the join: the answer to the infinite search has to be on stdout the
  // moment `clean-tt` returns, with no [stop] and no uci_wait_for_search()
  // behind it.
  TEST_CASE("clean-tt joins the search before it clears the table")
  {
    uci_init();

    std::string reply;
    {
      stdout_capture_t capture;
      uci_process_line("position startpos");
      uci_process_line("go infinite");

      // Long enough that the search is inside the table rather than still
      // starting up.
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      uci_process_line("clean-tt");

      // Not bestmove_of(): a missing line is exactly the red case here, and a
      // REQUIRE inside a helper would skip the cleanup below and leave an
      // infinite search running through the rest of the suite.
      for (const std::string& line : capture.lines()) {
        if (line.rfind("bestmove ", 0) == 0) { reply = line; }
      }
    }

    CHECK_MESSAGE(!reply.empty(),
                  "clean-tt returned with the search still running");

    // Cleanup whichever way the check went.
    uci_process_line("stop");
    uci_wait_for_search();

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


  // S195, closing 2026-09-04_test_review-F08. The shape of Stockfish's
  // tests/reprosearch.sh, as adocs/testing_strategy.md section 3.1 describes
  // it: the same node-limited search, repeated across [ucinewgame], has to
  // visit the same tree. What that shape catches is Stockfish#5376 -- a table
  // generation counter [ucinewgame] did not reset, which showed up as two
  // alternating bench totals and which an SPRT cannot see, because it averages
  // over games and never repeats one search.
  //
  // No golden here (DEC-142): every assertion compares two runs inside one
  // process and no count from any run is written into this file. The limits
  // and the two move sequences are the only constants, and section 4 of
  // adocs/plan_done/S195_reproducibility_test.md derives them.
  //
  // The one hazard these cases do NOT cover is the Stockfish one itself: a
  // tt_reset() that forgot [generation] is invisible to both, since the
  // entries are zeroed and replacement compares an entry's generation against
  // the table's. That counter is guarded by construction -- tt_reset() puts
  // [generation] back to 1 -- and not by anything asserted below.
  struct search_report_t
  {
    uint64_t nodes;
    std::string best;
  };


  // The last [info] line's node count, cumulative over the whole search since
  // S037, and the [bestmove] line. Those two and nothing else: [pv] can
  // lengthen on a carried mate line (S170), and [time] and [nps] are the
  // machine's. Parsed outside the capture, so a failure message reaches a
  // stdout that is nobody's buffer.
  static search_report_t report_of(const std::vector<std::string>& lines)
  {
    search_report_t report = {0, ""};
    bool seen_nodes = false;

    for (const std::string& line : lines) {
      if (line.rfind("info ", 0) == 0) {
        const size_t at = line.find(" nodes ");

        if (at != std::string::npos) {
          report.nodes = std::stoull(line.substr(at + 7));
          seen_nodes = true;
        }
      }

      if (line.rfind("bestmove ", 0) == 0) { report.best = line; }
    }

    REQUIRE_MESSAGE(seen_nodes, "no info line carrying a node count");
    REQUIRE_MESSAGE(!report.best.empty(), "no bestmove line");

    return report;
  }


  TEST_CASE("node-limited searches repeat across ucinewgame")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("uci");

      // Pinned, as tests/test_mate_carry.cpp pins it: a future default would
      // otherwise move both what these searches cost and how warm the table
      // is when they start.
      uci_process_line("setoption name Hash value 16");
    }

    // Two sequences because one tree is one shape. Legality is all that was
    // judged of them (DEC-023) and it was judged twice: python-chess 1.11.2
    // push_uci, and the engine's own [fen] after [position startpos moves],
    // which printed the same FEN for each. The second carries captures.
    const std::vector<std::string> sequences = {
        "e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6",
        "d2d4 d7d5 c2c4 d5c4 e2e3 e7e5 f1c4 e5d4"};

    // About a factor of two apart, so each budget truncates a different
    // iteration. A linear spacing puts most of the limits inside one.
    const std::vector<uint64_t> limits = {500,   1000,  2500,   5000,   10000,
                                          25000, 50000, 100000, 150000, 250000};

    // [go nodes] arms no timer and check_limits() compares the count exactly,
    // so where a search stops is a function of the tree and of nothing else.
    // T1 of the S193 guide: the [ucinewgame] here sets the stop flag and only
    // a [go] clears it again, which is why every search goes through the UCI
    // layer instead of calling iterative_deepening_search() directly.
    auto run = [](const std::string& moves, uint64_t limit) {
      std::vector<std::string> lines;

      {
        stdout_capture_t capture;
        uci_process_line("ucinewgame");
        uci_process_line("position startpos moves " + moves);
        uci_process_line("go nodes " + std::to_string(limit));
        uci_wait_for_search();
        lines = capture.lines();
      }

      return report_of(lines);
    };

    for (const std::string& moves : sequences) {
      std::set<uint64_t> distinct;

      for (const uint64_t limit : limits) {
        const search_report_t first = run(moves, limit);
        const search_report_t second = run(moves, limit);

        // Never [nodes] == limit: the last [info] line is printed only when
        // the iteration had a result, so a final iteration that aborts before
        // it has a PV leaves the reported count under the budget. Measured
        // 2026-09-10: 2917 at [go nodes 5000] on the first sequence.
        REQUIRE_MESSAGE(first.nodes == second.nodes,
                        (moves + " at " + std::to_string(limit) + ": " +
                         std::to_string(first.nodes) + " then " +
                         std::to_string(second.nodes)));
        REQUIRE_EQ(first.best, second.best);

        distinct.insert(first.nodes);
      }

      // Precondition, without which the sweep could be one search repeated ten
      // times and would prove nothing about budgets. Measured 2026-09-10: ten
      // distinct counts on the first sequence and nine on the second.
      REQUIRE(distinct.size() >= 5);
    }

    uci_shutdown();
  }


  TEST_CASE("a repeated go depth is cold only across ucinewgame")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("uci");
      uci_process_line("setoption name Hash value 16");
    }

    // tools/search_bench.py's midgame position, which is also the first of
    // bench_positions.
    const std::string fen =
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - "
        "0 10";

    auto run = [&fen](bool newgame) {
      std::vector<std::string> lines;

      {
        stdout_capture_t capture;

        if (newgame) { uci_process_line("ucinewgame"); }

        uci_process_line("position fen " + fen);
        uci_process_line("go depth 8");
        uci_wait_for_search();
        lines = capture.lines();
      }

      return report_of(lines);
    };

    const search_report_t cold = run(true);

    // Precondition for the warm half: the table holds the root now, so a
    // second search on it has something to read back.
    memcpy(&game, uci_game(), sizeof(game_t));
    REQUIRE(tt_get_entry(uci_tt(), &game.board) != nullptr);

    // The depth-limited twin of the case above, and it takes both reset paths
    // to break before it fires: measured 2026-09-10, cutting tt_reset() out of
    // reset_for_new_game() leaves it green, because set_position() still
    // resets on the differing FEN, and cutting set_position()'s reset leaves
    // it green for the mirror reason. With both cut it reads 8460 against a
    // cold 77612.
    REQUIRE_EQ(run(true).nodes, cold.nodes);

    // F08 pinned as a property rather than repaired -- the step's excludes:.
    // set_position() resets the table only when the FEN string differs from
    // the one already loaded, so a [position] repeating the FEN keeps whatever
    // the previous search left and the second search runs warm. The Chess
    // Programming Wiki's Transposition Table page, section Aging, is why that
    // is a property and not a bug: "most todays programs do not [clear the
    // hash table between root positions], profit from entries of previous
    // searches". The inequality is not an accident of one depth -- measured
    // here 2026-09-10, cold against warm at depths 5 to 10: 15905/683,
    // 21794/7400, 51667/11926, 77612/8460, 121530/9084, 208806/8218 -- and it
    // is what goes red if the hazard is ever silently repaired: removing
    // set_position()'s [initial_position != fen] gate reads 77612 against a
    // cold 77612.
    const search_report_t warm = run(false);

    REQUIRE_MESSAGE(warm.nodes != cold.nodes,
                    ("warm repeat " + std::to_string(warm.nodes) +
                     " equals cold " + std::to_string(cold.nodes)));

    uci_shutdown();
  }


  TEST_CASE("bench searches its last position cold")
  {
    uci_init();

    {
      stdout_capture_t capture;
      uci_process_line("uci");
      uci_process_line("setoption name Hash value 16");
    }

    // [bench 9] and not the bare form: the reset under test is the same call
    // in the same loop and depth 9 costs 0.21 s here against 3.6 s at
    // BENCH_DEPTH. The bare form has its own case, in
    // tests/test_uci_surface.cpp, "bench prints one final signature line and
    // repeats its total".
    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      uci_process_line("bench 9");

      // No uci_wait_for_search(): [bench] searches on the calling thread, so
      // it has finished by here and there is no thread to join.
      lines = capture.lines();
    }

    // One count per position: the last [info] line before each [bestmove].
    std::vector<uint64_t> per_position;
    uint64_t last_info_nodes = 0;
    bool saw_info = false;

    for (const std::string& line : lines) {
      if (line.rfind("info ", 0) == 0) {
        const size_t at = line.find(" nodes ");

        if (at != std::string::npos) {
          last_info_nodes = std::stoull(line.substr(at + 7));
          saw_info = true;
        }
      }

      if (line.rfind("bestmove ", 0) == 0) {
        REQUIRE(saw_info);
        per_position.push_back(last_info_nodes);
        saw_info = false;
      }
    }

    // Eight positions, fixed in src/chesso.cpp.
    REQUIRE(per_position.size() == 8);

    std::vector<std::string> standalone;
    {
      stdout_capture_t capture;
      uci_process_line("ucinewgame");

      // The same string constant command_bench searches last, reached through
      // the [position] shorthand rather than retyped.
      uci_process_line("position mate2b");
      uci_process_line("go depth 9");
      uci_wait_for_search();
      standalone = capture.lines();
    }

    // What this proves: bench's numbers are cold numbers, end to end -- the
    // last position of the run reports what it reports when it is the only
    // thing searched. What it does NOT prove, measured rather than argued
    // (2026-09-10): commenting out the loop's reset_for_new_game() leaves
    // [bench 9]'s output byte-identical on every nodes, score and pv field and
    // on the signature total, because the eight FENs are pairwise distinct and
    // set_position() already resets on a differing FEN. The loop's reset is a
    // guard against a bench list that ever repeats a position, and no test can
    // construct that list from outside.
    //
    // A red here after the bench set is reordered or extended means this case
    // is comparing the wrong position, not that the engine broke.
    const search_report_t cold = report_of(standalone);

    REQUIRE_MESSAGE(cold.nodes == per_position.back(),
                    ("bench read " + std::to_string(per_position.back()) +
                     " on its last position, a cold search of it reads " +
                     std::to_string(cold.nodes) +
                     " -- is mate2b still the last of bench_positions?"));

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
    // The black king on a8 is S223's re-pick (DEC-197): this board had no black
    // king at all and the load boundary refuses that now. Off the pinning ray
    // and out of every white attack, so the pin is what it was.
    {
      stdout_capture_t capture;
      uci_process_line("position fen k3r3/8/8/8/4N3/8/8/4K3 w - - 0 1");
    }

    REQUIRE_EQ(
        validate_book_move(NEW_MOVE(e4, d6, W_KNIGHT, TO_NONE, 0, 0, 0, 0)),
        move_t(0));

    uci_shutdown();
  }


  // S194, closing 2026-09-04_test_review-F06. The case above tests
  // validate_book_move() on a synthetic move and never reaches the probe, and
  // tests/test_openings.cpp tests the library below the UCI layer, so coverage
  // of the fast label found command_go's book consult, search_book_move()'s
  // weight-proportional draw and its `Best Book Move` branch with zero
  // executions -- the path S172 and S175 were both defects in, guarded by a
  // command someone ran by hand and wrote into a stamp.
  //
  // What kept it out of a test is the draw: search_book_move() reads an
  // mt19937_64 that uci_init() seeds from std::random_device, so the reply is
  // a different legal book move every process. `CHESSO_BOOK_SEED` is the hook
  // that pins it (MANUAL.md, "The book it ships with"), and these three cases
  // are what it buys.
  //
  // The observable that says the block executed is the shape of the reply.
  // command_go answers a book hit with `bestmove` and returns before any
  // `info` line -- its own is commented out -- so a single-line answer to
  // `go depth 1` from the start position cannot have come from a search, and
  // the `OwnBook false` control at the end of the first case is the other half
  // of that: with the book off the same command prints `info score` too.

  struct book_entries_t
  {
    std::set<std::string> moves;
    std::string heaviest;
    uint64_t total_weight = 0;
    uint16_t heaviest_weight = 0;
    size_t count = 0;
  };


  // What the library itself offers for the position on the UCI board, as the
  // text `bestmove` would carry. This is the test's own side of the
  // comparison: whatever the draw picks has to be one of these.
  static book_entries_t book_entries_here()
  {
    book_t book;
    REQUIRE(load_book_embedded(&book));

    move_t moves[MAX_MOVES];
    uint16_t weights[MAX_MOVES];
    const size_t count =
        get_book_moves_for_key(&book, &uci_game()->board, moves, weights);

    book_entries_t entries;
    entries.count = count;

    for (size_t i = 0; i < count; ++i) {
      const uci_move_t uci = {MOVE_FROM(moves[i]), MOVE_TO(moves[i]),
                              MOVE_PROMOTED(moves[i])};
      const std::string text = uci_move_to_algebraic(&uci);

      entries.moves.insert(text);
      entries.total_weight += weights[i];

      // Strict >, first maximum wins: search_book_move()'s own tie rule, so
      // the `Best Book Move` case below is not comparing against a second
      // rule that happens to agree.
      if (weights[i] > entries.heaviest_weight) {
        entries.heaviest_weight = weights[i];
        entries.heaviest = text;
      }
    }

    return entries;
  }


  // One whole engine session with `CHESSO_BOOK_SEED` already in the
  // environment, answering `go depth 1` from the start position. A session and
  // not a `ucinewgame`, because uci_init() is the only thing that reads the
  // variable: two draws from one seed need a shutdown between them.
  //
  // Every option it depends on is set here rather than inherited. They are
  // process globals that outlive uci_shutdown(), and `Book File` in particular
  // is pointed at an unloadable path by another case in this file, which
  // restores it -- but the order cases run in is a flag and not a contract.
  static std::string book_reply_from_a_fresh_engine()
  {
    uci_init();

    uci_process_line("ucinewgame");
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);
    uci_process_line("setoption name OwnBook value true");
    uci_process_line("setoption name Best Book Move value false");
    uci_process_line("position startpos");

    std::vector<std::string> lines;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      lines = capture.lines();
    }

    uci_process_line("setoption name OwnBook value false");
    uci_shutdown();

    // Read back outside the capture: doctest reports a failure on std::cout,
    // which is what stdout_capture_t was holding.
    REQUIRE_MESSAGE(
        lines.size() == 1,
        ("a book hit is one `bestmove` line and nothing else, got " +
         std::to_string(lines.size()) + " lines"));
    REQUIRE(lines[0].rfind("bestmove ", 0) == 0);

    return lines[0].substr(std::string("bestmove ").size());
  }


  TEST_CASE(
      "OwnBook draws a book move for the start key, and the seed replays it")
  {
    uci_init();
    uci_process_line("ucinewgame");
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);
    uci_process_line("position startpos");

    const book_entries_t library = book_entries_here();

    uci_shutdown();

    // GOLDEN (DEC-142): 13 entries under the start key, 34700 total weight,
    // e2e4 heaviest at 12956. Re-derive with
    // `~/.venv/chess/bin/python adocs/data/S194_book_start_key.py`, which
    // reads the shipped book with python-chess's own Polyglot reader and its
    // own zobrist_hash() -- an implementation outside this project, the same
    // property that makes adocs/data/S175_book_conformance.py a check and not
    // a restatement. The count is the precondition and not decoration: over an
    // empty set the membership below is unfalsifiable.
    REQUIRE(library.count == 13);
    REQUIRE(library.total_weight == 34700);
    REQUIRE(library.heaviest == "e2e4");
    REQUIRE(library.heaviest_weight == 12956);

    // Membership, never "seed 7 gives d2d4". [rand.predef] fixes what
    // mt19937_64 produces from a seed, but [rand.dist.general] leaves the
    // algorithm of std::uniform_int_distribution implementation-defined, so
    // the ticket a seed buys is a fact about one standard library. The
    // thirteen seeds are labels, one per entry, and nothing is read from them.
    for (int seed = 1; seed <= 13; ++seed) {
      const std::string text = std::to_string(seed);

      setenv("CHESSO_BOOK_SEED", text.c_str(), 1);

      const std::string first = book_reply_from_a_fresh_engine();
      const std::string second = book_reply_from_a_fresh_engine();

      REQUIRE_MESSAGE(library.moves.count(first) == 1,
                      ("seed " + text + " answered [" + first +
                       "], which the book does not offer for this position"));

      // What makes the seed falsifiable instead of decorative. Ignored, these
      // are two independent draws over the same thirteen weights: they agree
      // with probability 0.298 -- the squared weights summed over 34700
      // squared -- and over thirteen seeds with probability 1.5e-7.
      REQUIRE_MESSAGE(second == first, ("seed " + text + " drew [" + first +
                                        "] and then [" + second + "]"));
    }

    unsetenv("CHESSO_BOOK_SEED");

    // The control. With the book off the same command searches and prints an
    // `info score` line, so the single-line replies above are a book hit and
    // not a dead stream.
    uci_init();
    uci_process_line("ucinewgame");
    uci_process_line("setoption name OwnBook value false");
    uci_process_line("position startpos");

    std::vector<std::string> searched;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      searched = capture.lines();
    }

    uci_shutdown();

    bool has_info = false;
    bool has_bestmove = false;

    for (const std::string& line : searched) {
      has_info = has_info || line.rfind("info score ", 0) == 0;
      has_bestmove = has_bestmove || line.rfind("bestmove ", 0) == 0;
    }

    REQUIRE(has_info);
    REQUIRE(has_bestmove);
  }


  TEST_CASE(
      "Best Book Move plays the heaviest entry, and the S175 position d2f3")
  {
    // No seed anywhere in this case, deliberately: `Best Book Move` reads the
    // weights and never the generator, so a reader who found `CHESSO_BOOK_SEED`
    // set here would think the pin depends on it.
    unsetenv("CHESSO_BOOK_SEED");

    uci_init();
    uci_process_line("ucinewgame");
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);
    uci_process_line("setoption name OwnBook value true");
    uci_process_line("setoption name Best Book Move value true");
    uci_process_line("position startpos");

    std::vector<std::string> heaviest;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      heaviest = capture.lines();
    }

    // GOLDEN (DEC-142): e2e4, the heaviest of the thirteen start-key entries
    // at 12956 against d2d4's 12493. Same script as the case above.
    REQUIRE(heaviest == std::vector<std::string>{"bestmove e2e4"});

    // S175's repaired position, an edge-file en-passant square: one entry
    // under key bff7aaf88aaf9fbb, `d2f3` at weight 1. Both selection rules
    // must answer it -- the heaviest of one entry, and a draw over a total of
    // 1 that can only land on it -- so it is checked under both, and it is the
    // end-to-end guard S175 left as a command in a stamp.
    //
    // GOLDEN (DEC-142): the single entry and its move. Re-derive with the same
    // `adocs/data/S194_book_start_key.py`, which reports this position too;
    // tests/test_audit_polyglot_key.cpp pins the key itself.
    const std::string s175 =
        "position fen rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/"
        "R1BQKBNR w KQkq h6 0 8";

    uci_process_line(s175);

    std::vector<std::string> best_rule;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      best_rule = capture.lines();
    }

    uci_process_line("setoption name Best Book Move value false");
    uci_process_line(s175);

    std::vector<std::string> draw_rule;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      draw_rule = capture.lines();
    }

    uci_process_line("setoption name OwnBook value false");
    uci_shutdown();

    // No `info` line before either of them, which is what says command_go
    // returned on the book and never started a search.
    REQUIRE(best_rule == std::vector<std::string>{"bestmove d2f3"});
    REQUIRE(draw_rule == std::vector<std::string>{"bestmove d2f3"});
  }


  // DEC-184: what the engine drops, it says so on the channel -- the S172 and
  // S176 pattern, one variable along. A seed it cannot read is refused in both
  // builds, on stdout and not through a LOG_ macro that compiles to nothing
  // under NDEBUG, and the draw then falls back to std::random_device rather
  // than to some silent default.
  TEST_CASE(
      "an unreadable CHESSO_BOOK_SEED is refused, and the engine plays on")
  {
    // `Book File` outlives uci_shutdown() and another case in this file leaves
    // it unloadable, which would put a second line under the capture below.
    uci_init();
    uci_process_line("setoption name Book File value " BOOK_FILE_EMBEDDED);
    uci_shutdown();

    setenv("CHESSO_BOOK_SEED", "abc", 1);

    std::vector<std::string> startup;
    {
      stdout_capture_t capture;
      uci_init();
      startup = capture.lines();
    }

    uci_process_line("ucinewgame");
    uci_process_line("setoption name OwnBook value true");
    uci_process_line("setoption name Best Book Move value false");
    uci_process_line("position startpos");

    const book_entries_t library = book_entries_here();

    std::vector<std::string> reply;
    {
      stdout_capture_t capture;
      uci_process_line("go depth 1");
      uci_wait_for_search();
      reply = capture.lines();
    }

    uci_process_line("setoption name OwnBook value false");
    uci_shutdown();

    REQUIRE(startup == std::vector<std::string>{
                           "info string refused [CHESSO_BOOK_SEED] abc, not an "
                           "unsigned 64-bit decimal integer. Seeding the book "
                           "draw from std::random_device"});

    // And the refusal is a refusal of the seed and not of the book: the engine
    // still answers from it, with one of the moves the library offers.
    REQUIRE(reply.size() == 1);
    REQUIRE(reply[0].rfind("bestmove ", 0) == 0);
    REQUIRE(library.moves.count(
                reply[0].substr(std::string("bestmove ").size())) == 1);

    // The two controls, without which "exactly one line" says nothing: a seed
    // it can read is silent, and so is no seed at all.
    setenv("CHESSO_BOOK_SEED", "7", 1);

    std::vector<std::string> readable;
    {
      stdout_capture_t capture;
      uci_init();
      readable = capture.lines();
    }
    uci_shutdown();

    unsetenv("CHESSO_BOOK_SEED");

    std::vector<std::string> absent;
    {
      stdout_capture_t capture;
      uci_init();
      absent = capture.lines();
    }
    uci_shutdown();

    REQUIRE(readable.empty());
    REQUIRE(absent.empty());
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


  // `2026-09-10_adversarial-F33`. Both functions used to hand a plain `char`
  // to a <cctype> classifier, which is undefined for every value outside
  // `unsigned char` and EOF -- and a `char` is signed here, so any byte above
  // 127 arrives negative. A UCI line is bytes a GUI wrote and nothing upstream
  // restricts them to ASCII, so the input exists. Both cast through
  // `unsigned char` since S213, as fold_case() has since S209.
  //
  // 0xE9 rather than a character: what is being asserted is the byte, not a
  // spelling, and it is written as an escape so that the file's own encoding
  // cannot change what the test feeds in.
  TEST_CASE("a byte above 127 classifies as neither a digit nor a space")
  {
    const std::string high = "\xE9";

    REQUIRE_FALSE(is_uint(high));
    REQUIRE_FALSE(is_uint("12" + high));
    REQUIRE(is_uint("12"));

    // Not whitespace, so it survives the trim at either end and stops the trim
    // where it stands.
    REQUIRE_EQ(trim_whitespace("  " + high + "  "), high);
    REQUIRE_EQ(trim_whitespace(high), high);
    REQUIRE_EQ(trim_whitespace(" " + high + " go "), high + " go");
  }


  // `where` is the whole bracket text the refusal prints, so it is one of the
  // names `command_go` actually passes -- `Depth` and `Nodes` were the option
  // names these two cases carried before S210 gave the helper a UCI channel,
  // and they put two refusal shapes no document describes onto the test
  // binary's own stdout. Captured for the same reason: a refusal is engine
  // output now, and it is asserted rather than spilled.
  TEST_CASE("pop_int clamps instead of accepting nonsense")
  {
    int value = -1;

    std::queue<std::string> args;
    args.push("7");
    REQUIRE(pop_int(args, "go depth", value, 1, 100));
    REQUIRE_EQ(value, 7);

    // Out of range on both sides is clamped, not rejected.
    args.push("1000");
    REQUIRE(pop_int(args, "go depth", value, 1, 100));
    REQUIRE_EQ(value, 100);

    args.push("-5");
    REQUIRE(pop_int(args, "go depth", value, 1, 100));
    REQUIRE_EQ(value, 1);

    // Garbage and a missing value both fail and leave the target alone.
    value = 42;
    args.push("abc");

    std::string said;
    {
      stdout_capture_t capture;
      REQUIRE_FALSE(pop_int(args, "go depth", value, 1, 100));
      said = capture.str();
    }

    REQUIRE_EQ(value, 42);
    REQUIRE_MESSAGE(said.find("info string refused [go depth] abc, "
                              "not an integer") != std::string::npos,
                    said);

    // A missing value says nothing on the UCI channel: there is no token to
    // name, and UCI has no reply for a truncated line.
    REQUIRE(args.empty());

    {
      stdout_capture_t capture;
      REQUIRE_FALSE(pop_int(args, "go depth", value, 1, 100));
      said = capture.str();
    }

    REQUIRE_EQ(value, 42);
    REQUIRE_MESSAGE(said.empty(), said);
  }


  TEST_CASE("pop_u64 never wraps a negative into a huge budget")
  {
    uint64_t value = 1;

    std::queue<std::string> args;
    args.push("123456789");
    REQUIRE(pop_u64(args, "go nodes", value));
    REQUIRE_EQ(value, 123456789u);

    args.push("-1");
    REQUIRE(pop_u64(args, "go nodes", value));
    REQUIRE_EQ(value, 0u);

    value = 5;
    args.push("nope");

    std::string said;
    {
      stdout_capture_t capture;
      REQUIRE_FALSE(pop_u64(args, "go nodes", value));
      said = capture.str();
    }

    REQUIRE_EQ(value, 5u);
    REQUIRE_MESSAGE(said.find("info string refused [go nodes] nope, "
                              "not an integer") != std::string::npos,
                    said);
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

    REQUIRE(set_position(KIWIPETE_POS));
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(KIWIPETE_POS));

    REQUIRE_FALSE(set_position("not a fen at all"));
    REQUIRE_EQ(generate_FEN(&uci_game()->board), std::string(KIWIPETE_POS));

    uci_shutdown();
  }


  TEST_CASE("check_move_legality accepts only playable moves")
  {
    uci_init();

    // The black king on a8 is S223's re-pick (DEC-197); see the site above.
    {
      stdout_capture_t capture;
      uci_process_line("position fen k3r3/8/8/8/4N3/8/8/4K3 w - - 0 1");
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
      // binary therefore aborts on the stale flag at its first check_limits().
      // The first version of this test measured one iteration that way, which
      // is what the never-stop flag S210's F21 removed used to buy it; the
      // same call today measures none.
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
  // score around ten centipawns-times-a-hundred for seven iterations and then
  // report a mate at depth 8 -- it was nine until S188's extension found both
  // an iteration earlier, and the GOLDEN block below is what re-derives it --
  // which is the shape that matters here: the
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

    // GOLDEN (DEC-142): `first_mate_depth`, the iteration a mate score first
    // appears at, one per position. It was **9 and 9** against the reference
    // binary the comment above names, and it is **8 and 8** since S188's check
    // extension: an extension along a forcing line finds a forced mate an
    // iteration earlier, which is what the technique is for (DEC-228).
    //
    // A bare measurement until now, with no way to re-take it -- itself a
    // DEC-142 finding, closed here. Moves legitimately on: any change to
    // extensions, pruning, reduction or ordering.
    // Margin: exact. The constant is read as `first_mate_depth - 2` and
    // `- 1` by the preconditions below, so a value one off makes them
    // vacuous rather than red.
    // Re-derive with `adocs/data/S188_repair_goldens.py first-mate`, which
    // drives `deepen()`'s own `go depth 10` over both positions and prints
    // every iteration's kind and value beside the first mate, the precondition
    // below it and the distances from there on. `mate_in` is re-derived by the
    // same command: it is the distance every iteration from the first mate on
    // reports, and the script prints the set.
    // clang-format off
    const std::vector<case_t> cases = {
      {"r3r1k1/pp3pbp/1qp1b1p1/1BB5/3P4/Q1n2N2/P4PPP/3R1K1R b - - 5 18", 5, 8},
      {"r4k2/R7/8/8/8/8/4K3/1R6 w - - 1 2",                             5, 8},
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
  // THREE MOTIFS, WHY THE NARROWNESS IS CLOSE TO FORCED, AND WHAT THIS GATE
  // THEREFORE STILL CANNOT CATCH. S155, S168.
  //
  // Counted over the tracked TSV by adocs/data/S155_motif_census.py, not
  // asserted. S145's 48 were one motif: two material signatures, one the
  // colour mirror of the other, a lone queen as the mating force in 48 of 48,
  // `lead` 760 in 48 of 48. DEC-114 is the owner's decision that one mating
  // piece across a gate whose purpose is mating-piece defects is not enough,
  // and S168 is what it bought. The 82 rows below read **five material
  // signatures and three mating forces - a queen in 48, a lone rook in 32, two
  // knights in 2 - with leads 760, 1160 and 1020**.
  //
  // The narrowness that remains is close to forced by the hazard and is not a
  // flaw in the set. The rule misfires only where the side to move is lost by
  // force while its static score is a margin clear of beta, so the mated side
  // has to be materially *ahead* and unable to use it - and a frozen clump
  // behind a blocked pawn wall is close to the only way to build that, which
  // is why every row reads one of three leads. S033 built its one position by
  // hand this way; the construction generalises that shape rather than picking
  // it.
  //
  // TWO THINGS S168 MEASURED THAT THE ARGUMENT FOR IT GOT WRONG, both worth
  // more than the rows they produced.
  //
  //   1. A king and two knights does NOT give a knight mate by construction.
  //      A mobile knight standing beside a wall pawn unfreezes the capture the
  //      non-adjacent files deny, and the freed pawn queens with check: of the
  //      first fourteen knight-motif positions, twelve were mated by a pawn.
  //      The generator now enforces the mating piece - `mates_with` and
  //      `line_mate_pieces()` - and refuses a candidate whose line offers a
  //      promotion anywhere. Under that rule three of the four knight families
  //      accept nothing at all and the fourth accepts two, which is why the
  //      knight force is 2 rows and not 14. A knight mate that is also
  //      all-quiet is rare, and that is a measurement, not a shortfall.
  //   2. The rook motif is named for its force because the geometry did not
  //      survive being checked: the mate lands on the mated side's own back
  //      rank in 13 of the 32 and elsewhere in 19. DEC-114 asked for "a
  //      back-rank mate"; what the construction yields is a lone-rook mate
  //      that is sometimes one.
  //
  // What this gate still cannot catch. A rule that hides:
  //
  //   * a knight mate deeper than a mate in two - both knight rows are mates
  //     in two, so that axis is exercised at ply 1 and nowhere else,
  //   * a smothered mate - the mated king is pocketed by the attacker's pawns
  //     and by the wall, never by its own pieces,
  //   * a king hunt, where the king is driven across the board instead of held
  //     in a pocket,
  //   * an open-line mate, or the sacrifice that opens the line - every
  //     attacker move on every proof tree here is quiet by construction,
  //   * a promotion mate - the engine's own generator emits 1981 legal moves
  //     over the 82 roots and the 186 guarded defender nodes, of which 2 are
  //     pawn moves and 0 are promotions (`S155_motif_census.py --moves`),
  //   * any mate in a position with a realistic material balance.
  //
  // A future pruning rule that loses mates in those shapes passes this suite.
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
  // position in the set is late by eight at the shipping defaults - it was
  // late by four when S145 chose this window and S165 moved it - so a tighter
  // window would report a postponed mate as a lost one, the exact conflation
  // S145 exists to remove.
  //
  // **The window is a budget and not a margin, and S154 measured what it
  // buys.** Widening it does not make the reading safer, it makes it a
  // different reading: at slack 12 the shipping guard and the removed guard
  // both find 10 of the 16 mates in three, so the separation the floor below
  // rests on is gone entirely, and the whole 48-position pass cost 4.1 s
  // against 0.7 s at 8. The 82-position pass costs 0.95 s at slack 8, three
  // runs on an idle machine reading 0.97, 0.94 and 0.95, against 0.79 s for
  // the 48 - so S168 bought 34 positions for about 0.16 s of a fast suite that
  // runs 45 s. The mate in three count is therefore a reading of how
  // late a mate arrives under a fixed budget, not of whether it is lost. The
  // mate in two assertion is the one that does not depend on the window at
  // all, because it asks for the first iteration and not the last.
  // adocs/data/S154_floor_margin_sweep.log, mode `slack`.
  //
  // NOT A GOLDEN (DEC-142): a measured window, not a pinned observation. It is
  // a depth budget the reading is taken under, priced by
  // `python3 adocs/data/S154_floor_margin_sweep.py slack`; widening it changes
  // what MATE_IN_THREE_FLOOR means rather than re-deriving it, and S154 is what
  // re-decides it.
  static constexpr int MATE_DEPTH_SLACK = 8;

  // WHAT IS ASSERTED, AND WHY IT IS NOT "EVERY MATE IS FOUND".
  //
  // Measured over these 82 positions on the shipping build at depth 2m - 1 + 8,
  // and the answer is almost entirely a function of the mate distance:
  //
  //   mate in 2   26 of 26 exact, delay 0
  //   mate in 3   12 of 24 exact, delay up to 8
  //   mate in 4    1 of 16
  //   mate in 5    0 of 16
  //
  // Re-taken 2026-09-01 by S168 over the enlarged set; the same sweep over
  // S145's 48 read 16/16, 9/16, 0/8 and 0/8. S145 itself read 8 of 16 mates in
  // three at delay up to 4, and S165 guarded null move pruning at both edges of
  // the mate band and moved that number - the only movement seventeen commits
  // produced, which S154 established by rebuilding at each of them.
  //
  // So the guard holds where the old three-position gate looked and nowhere
  // else, because all three of those cases were mates in two. Asserting that
  // every position is found would assert something this engine has never done
  // and no setting of reverse futility makes true - the mates in four and five
  // are 1 of 16 and 0 of 16 with the guard at its strictest setting too. A test
  // demanding it would be red on arrival and would be weakened to clear it,
  // which is what happened to the two surveyed projects that wrote per-position
  // mate tests.
  //
  // What is asserted instead is three things, in descending order of how
  // provable they are.
  //
  // **Two properties that are defects at any count.** No mate score for the
  // side being mated, and no mate score *closer* than the proved minimum. The
  // second is what the exhaustive proof buys: the enumeration refuted every
  // shorter distance, so a shorter claim is provably false rather than merely
  // surprising, and it is the S094 class of bug - a mate score renormalised by
  // the wrong number of plies. Measured 0 and 0 over six reverse-futility
  // settings times these 82 positions, and before that over twelve settings
  // times the 48.
  //
  // **Every mate in two, at the first iteration that can hold it.** This is the
  // assertion that fences the tuner, and it asks for two things where the
  // count alone asks for one: the mate is found, and `first_exact` is 2m - 1.
  // At RfpMinPly 2 and above it is 26 of 26, found and on time. At 1 and 0 it
  // is 21 of 26 found and only **15 of 26 on time**, so what goes red here is
  // eleven positions. 0 and 1 are the same engine - the root is exempted by
  // !is_pv, not by this parameter - so this goes red at exactly the value
  // S085's run spent 906 of 1250 iterations at. The clause got stronger when
  // S168 enlarged the set: over the 48 it was 13 of 16 found and 9 on time,
  // seven positions rather than eleven, and the ten mates in two S168 added -
  // eight rook, two knight - are all found on time at the shipping defaults.
  //
  // **A floor on the mate in three count.** 12 of 24 at the shipping floor, 10
  // at RfpMinPly 1, 21 at RfpMinPly 4. The floor is 11, strictly between the
  // shipping value and the removed-guard value, and it tolerates one loss.
  //
  // Split by motif, because the split says which rows carry the separation:
  // the 16 queen mates in three read 9 shipping, 7 removed, 14 at RfpMinPly 5,
  // which is S154's reading over the 48 reproduced exactly - 34 added positions
  // moved none of the old verdicts. The 8 rook mates in three read 3, 3 and 7,
  // so they lift both ends by a constant and do not widen the gap. The floor
  // moved from 8 to 11 for that reason and not because the guard got easier to
  // catch.
  //
  // **The floor is re-derived whenever either end of it moves, DEC-116, and
  // this is the second time.** It was 7 while the ends were 9 and 7, where
  // `7 >= 7` could not fail for the reason it exists; S154 re-derived it as 8
  // on 2026-09-01, and S168's positions moved both ends the same day - 12 and
  // 10 - so it is 11 now. The rule is the point: a set change or a search
  // change that moves either end obliges the number to be taken again, not
  // read again.
  //
  // **The claim that it fails when the guard fails and not when the tree
  // shifts underneath it is a measurement.** S154 ran the 48 through the binary
  // built at every one of the seventeen commits that touched src/ since the
  // floor was placed, and through nine transposition table sizes from 1 MB to
  // 256 MB. Positions changing verdict: **0**, at every step except S165, which
  // moved exactly one and moved it upward. The table sweep moved the node total
  // by 5.9 % over the whole set and 17 % over the mates in three, so the tree
  // did shift and the verdicts did not follow it. Against that, one ply of the
  // guard itself moves nine positions over the enlarged set - 4 to 3 is a churn
  // of 9 - so the count is sensitive to the thing it fences and inert to
  // everything else. adocs/data/S154_floor_margin_sweep.log and
  // adocs/data/S168_floor_sweep.log.
  //
  // The mate in four and five counts are **recorded and not asserted**: 1 of 16
  // and 0 of 16, and a floor of one asserts almost nothing. What recovers them
  // is the depth ceiling and not the ply floor - over the 82 the ceiling reads
  // 13 of 16 and 11 of 16 at RfpMaxDepth 0 against 1 and 0 from 10 up, and
  // S085 tuned that ceiling from S033's 6 to 15.
  //
  // **S148 re-decided that ceiling on 2026-09-09 and it stays at 15**, so
  // these two counts stay recorded and this comment is where the reason lives.
  // The largest ceiling at which both deep classes survive is 4, where the set
  // reads 52 of 82 exact and the classes 7 of 16 and 1 of 16; that candidate
  // lost its SPRT at nElo -7.31 +/- 5.60 over 14808 games at 8+0.08, H0
  // accepted against {-5, 0}. A promotion to an asserted floor was the
  // accepts' condition and it is not met in either class: four would ship at
  // 1, which is the zero-margin floor DEC-116 already rejected here, and five
  // ships at 0. What would lift them is a search that finds these mates
  // without paying for them, not a lower bound. DEC-158,
  // adocs/data/S148_rfp_ceiling_sweep.log for the grid and
  // adocs/data/S148_sprt.log for the run.
  //
  // adocs/data/S145_rfp_sweep.log holds S145's sweep and
  // adocs/data/S154_floor_margin_sweep.log holds it re-taken, 2026-09-01.
  //
  // GOLDEN (DEC-142): 11, the fewest exact mates in three the engine
  // may find over the 82 constructed positions at depth 2m - 1 +
  // MATE_DEPTH_SLACK. Its two ends are above: 12 at the shipping guard, 10 with
  // the guard weakened by one ply. Re-derive: python3
  // adocs/data/S154_floor_margin_sweep.py floor, and mode `red` for the
  // weakened end; adocs/data/S168_floor_sweep.log is the reading over the
  // enlarged set. Moves legitimately on: a search change that costs or buys
  // mate finding, and a change to the position set -- either end moving obliges
  // the number to be taken again, not read again. Margin: 1 on each side, which
  // is the narrowest DEC-116 accepts, and is why the split by motif is recorded
  // above. Property beside it: the two assertions that are defects at any count
  // -- no mate score for the side being mated, and none closer than the proved
  // minimum -- which carry no floor at all.
  static constexpr int MATE_IN_THREE_FLOOR = 11;

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
      {"1r6/7K/4k3/8/8/p1p1p2p/P1P1P2P/RBRB4 b - - 0 1",
       {"1r6/7K/5k2/8/8/p1p1p2p/P1P1P2P/RBRB4 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_black"},
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
      {"4brbr/p2p1p1p/P2P1P1P/2R5/8/8/4K3/7k w - - 0 1",
       {"4brbr/p2p1p1p/P2P1P1P/2R5/8/8/5K2/7k b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_flip"},
      {"4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1",
       {"4brbr/p2p1p1p/P2P1P1P/2R5/8/K7/8/1k6 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_flip"},
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
      {"8/4k3/1r6/8/8/p1p1p2p/P1P1P2P/RBRB2K1 b - - 0 1",
       {"8/4k3/5r2/8/8/p1p1p2p/P1P1P2P/RBRB2K1 w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_black"},
      {"8/7r/1k6/8/8/p2p1p1p/P2P1P1P/1K2BRBR b - - 0 1",
       {"8/2r5/1k6/8/8/p2p1p1p/P2P1P1P/1K2BRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_flip_black"},
      {"8/8/2n5/8/8/p1np1p1p/P2P1P1P/K1k1BRBR b - - 0 1",
       {"8/8/8/8/3n4/p1np1p1p/P2P1P1P/K1k1BRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1020, "knight0_flip_black"},
      {"8/8/3n4/8/8/pn1p1p1p/P2P1P1P/1k1KBRBR b - - 0 1",
       {"8/8/8/8/2n5/pn1p1p1p/P2P1P1P/1k1KBRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1020, "knight0_flip_black"},
      {"8/8/8/3r4/6k1/p2p1p1p/P2P1P1P/1K2BRBR b - - 0 1",
       {"8/8/8/2r5/6k1/p2p1p1p/P2P1P1P/1K2BRBR w - - 1 2",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0_flip_black"},
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
      {"rbrb1k2/p1p1p1Rp/P1P1P2P/8/8/2K5/8/8 w - - 0 1",
       {"rbrb1k2/p1p1p2p/P1P1P2P/6R1/8/2K5/8/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0"},
      {"rbrb4/p1p1p2p/P1P1P2P/2K5/8/k7/2R5/8 w - - 0 1",
       {"rbrb4/p1p1p2p/P1P1P2P/8/2K5/k7/2R5/8 b - - 1 1",
        nullptr,
        nullptr,
        nullptr},
       2, 1160, "rook0"},
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
      {"3kbrbr/p2p1p1p/P2P1P1P/8/8/8/8/2KR4 w - - 0 1",
       {"3kbrbr/p2p1p1p/P2P1P1P/3R4/8/8/8/2K5 b - - 1 1",
        "2k1brbr/p2p1p1p/P2P1P1P/1R6/8/8/8/2K5 b - - 3 2",
        nullptr,
        nullptr},
       3, 1160, "rook0_flip"},
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
      {"8/2k5/8/8/r7/p2p1p1p/P2P1P1P/K3BRBR b - - 0 1",
       {"3k4/8/8/8/r7/p2p1p1p/P2P1P1P/K3BRBR w - - 1 2",
        "3k4/8/8/8/2r5/p2p1p1p/P2P1P1P/1K2BRBR w - - 3 3",
        nullptr,
        nullptr},
       3, 1160, "rook0_flip_black"},
      {"8/3r2k1/8/8/8/p1p1p2p/P1P1P2P/RBRBK3 b - - 0 1",
       {"7k/3r4/8/8/8/p1p1p2p/P1P1P2P/RBRBK3 w - - 1 2",
        "7k/6r1/8/8/8/p1p1p2p/P1P1P2P/RBRB1K2 w - - 3 3",
        nullptr,
        nullptr},
       3, 1160, "rook0_black"},
      {"8/4q3/6K1/8/7k/p1p1p3/P1P1P3/RBRB4 b - - 0 1",
       {"8/4q3/6K1/8/6k1/p1p1p3/P1P1P3/RBRB4 w - - 1 2",
        "8/4q3/7K/5k2/8/p1p1p3/P1P1P3/RBRB4 w - - 3 3",
        nullptr,
        nullptr},
       3, 760, "shift0_black"},
      {"8/5k2/8/8/4r3/p2p1p1p/P2P1P1P/3KBRBR b - - 0 1",
       {"6k1/8/8/8/4r3/p2p1p1p/P2P1P1P/3KBRBR w - - 1 2",
        "6k1/8/8/8/1r6/p2p1p1p/P2P1P1P/2K1BRBR w - - 3 3",
        nullptr,
        nullptr},
       3, 1160, "rook0_flip_black"},
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
      {"8/8/8/2r3k1/8/p1p1p2p/P1P1P2P/RBRB3K b - - 0 1",
       {"8/8/7k/2r5/8/p1p1p2p/P1P1P2P/RBRB3K w - - 1 2",
        "8/8/7k/5r2/8/p1p1p2p/P1P1P2P/RBRB2K1 w - - 3 3",
        nullptr,
        nullptr},
       3, 1160, "rook0_black"},
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
      {"k3brbr/p2p1p1p/P2P1P1P/8/4K3/8/8/R7 w - - 0 1",
       {"k3brbr/p2p1p1p/P2P1P1P/5K2/8/8/8/R7 b - - 1 1",
        "1k2brbr/p2p1p1p/P2P1P1P/5K2/8/8/8/2R5 b - - 3 2",
        nullptr,
        nullptr},
       3, 1160, "rook0_flip"},
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
      {"rbrbk3/p1p1p2p/P1P1P2P/6K1/8/7R/8/8 w - - 0 1",
       {"rbrbk3/p1p1p2p/P1P1P2P/7K/8/7R/8/8 b - - 1 1",
        "rbrb1k2/p1p1p2p/P1P1P2P/7K/8/6R1/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 1160, "rook0"},
      {"rbrbk3/p1p1p2p/P1P1P2P/8/1R2K3/8/8/8 w - - 0 1",
       {"rbrbk3/p1p1p2p/P1P1P2P/5K2/1R6/8/8/8 b - - 1 1",
        "rbrb1k2/p1p1p2p/P1P1P2P/5K2/6R1/8/8/8 b - - 3 2",
        nullptr,
        nullptr},
       3, 1160, "rook0"},
      {"1K6/2r5/8/3k4/8/p2p1p1p/P2P1P1P/4BRBR b - - 0 1",
       {"1K6/2r5/3k4/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 1 2",
        "K7/2r5/2k5/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 3 3",
        "1K6/2r5/1k6/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 5 4",
        nullptr},
       4, 1160, "rook0_flip_black"},
      {"1k2brbr/p2pKpRp/P2P1P1P/8/8/8/8/8 w - - 0 1",
       {"1k2brbr/p2pKp1p/P2P1P1P/6R1/8/8/8/8 b - - 1 1",
        "2k1brbr/p2pKp1p/P2P1P1P/7R/8/8/8/8 b - - 3 2",
        "1k2brbr/p2pKp1p/P2P1P1P/2R5/8/8/8/8 b - - 5 3",
        nullptr},
       4, 1160, "rook0_flip"},
      {"3k2K1/2r5/8/8/8/p1p1p2p/P1P1P2P/RBRB4 b - - 0 1",
       {"4k1K1/2r5/8/8/8/p1p1p2p/P1P1P2P/RBRB4 w - - 1 2",
        "7K/2r2k2/8/8/8/p1p1p2p/P1P1P2P/RBRB4 w - - 3 3",
        "8/5k1K/2r5/8/8/p1p1p2p/P1P1P2P/RBRB4 w - - 5 4",
        nullptr},
       4, 1160, "rook0_black"},
      {"4brbr/3p1p1p/3P1P1P/3K4/2Q5/8/1k6/8 w - - 0 1",
       {"4brbr/3p1p1p/3P1P1P/8/2QK4/8/1k6/8 b - - 1 1",
        "4brbr/3p1p1p/3P1P1P/1Q6/3K4/k7/8/8 b - - 3 2",
        "4brbr/3p1p1p/3P1P1P/1Q6/8/2K5/k7/8 b - - 5 3",
        nullptr},
       4, 760, "shift0_flip"},
      {"4brbr/p2p1p1p/P2P1P1P/8/1K6/2R5/k7/8 w - - 0 1",
       {"4brbr/p2p1p1p/P2P1P1P/8/1K6/1R6/k7/8 b - - 1 1",
        "4brbr/p2p1p1p/P2P1P1P/8/8/1RK5/8/k7 b - - 3 2",
        "4brbr/p2p1p1p/P2P1P1P/8/8/1R6/k1K5/8 b - - 5 3",
        nullptr},
       4, 1160, "rook0_flip"},
      {"7K/4k3/8/8/8/p1p1p2p/P1P1P2P/RBRB3r b - - 0 1",
       {"7K/5k2/8/8/8/p1p1p2p/P1P1P2P/RBRB3r w - - 1 2",
        "8/5k1K/8/8/8/p1p1p2p/P1P1P2P/RBRB1r2 w - - 3 3",
        "7K/5k2/8/5r2/8/p1p1p2p/P1P1P2P/RBRB4 w - - 5 4",
        nullptr},
       4, 1160, "rook0_black"},
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
      {"8/7K/8/2r4k/8/p2p1p1p/P2P1P1P/4BRBR b - - 0 1",
       {"8/7K/8/6rk/8/p2p1p1p/P2P1P1P/4BRBR w - - 1 2",
        "7K/8/6k1/6r1/8/p2p1p1p/P2P1P1P/4BRBR w - - 3 3",
        "6K1/8/6k1/5r2/8/p2p1p1p/P2P1P1P/4BRBR w - - 5 4",
        nullptr},
       4, 1160, "rook0_flip_black"},
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
      {"rbrb4/p1p1p2p/P1P1P2P/8/7K/8/7k/6R1 w - - 0 1",
       {"rbrb2R1/p1p1p2p/P1P1P2P/8/7K/8/7k/8 b - - 1 1",
        "rbrb2R1/p1p1p2p/P1P1P2P/8/8/6K1/8/7k b - - 3 2",
        "rbrb1R2/p1p1p2p/P1P1P2P/8/8/6K1/8/6k1 b - - 5 3",
        nullptr},
       4, 1160, "rook0"},
      {"rbrb4/p1p1p2p/P1P1P2P/8/8/8/7R/1k1K4 w - - 0 1",
       {"rbrb4/p1p1p2p/P1P1P2P/8/8/8/6R1/1k1K4 b - - 1 1",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/8/2K3R1/k7 b - - 3 2",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/6R1/k1K5/8 b - - 5 3",
        nullptr},
       4, 1160, "rook0"},
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
      {"4brbr/p2p1p1p/P2P1P1P/8/7K/6R1/8/7k w - - 0 1",
       {"4brbr/p2p1p1p/P2P1P1P/7K/8/6R1/8/7k b - - 1 1",
        "4brbr/p2p1p1p/P2P1P1P/8/6K1/6R1/7k/8 b - - 3 2",
        "4brbr/p2p1p1p/P2P1P1P/8/8/5KR1/8/7k b - - 5 3",
        "4brbr/p2p1p1p/P2P1P1P/8/8/6R1/5K1k/8 b - - 7 4"},
       5, 1160, "rook0_flip"},
      {"4brbr/p2p1p1p/P2P1P1P/8/8/4K3/7k/6R1 w - - 0 1",
       {"4brbr/p2p1p1p/P2P1P1P/6R1/8/4K3/7k/8 b - - 1 1",
        "4brbr/p2p1p1p/P2P1P1P/6R1/5K2/7k/8/8 b - - 3 2",
        "4brbr/p2p1p1p/P2P1P1P/4R3/5K1k/8/8/8 b - - 5 3",
        "4brbr/p2p1p1p/P2P1P1P/8/5K2/7k/4R3/8 b - - 7 4"},
       5, 1160, "rook0_flip"},
      {"7K/6r1/3k4/8/8/p2p1p1p/P2P1P1P/4BRBR b - - 0 1",
       {"7K/5r2/3k4/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 1 2",
        "6K1/4kr2/8/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 3 3",
        "7K/5r2/5k2/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 5 4",
        "6K1/5r2/6k1/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 7 5"},
       5, 1160, "rook0_flip_black"},
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
      {"8/8/K7/5r2/k7/p1p1p2p/P1P1P2P/RBRB4 b - - 0 1",
       {"8/8/K7/1r6/k7/p1p1p2p/P1P1P2P/RBRB4 w - - 1 2",
        "8/K7/8/kr6/8/p1p1p2p/P1P1P2P/RBRB4 w - - 3 3",
        "K7/8/1k6/1r6/8/p1p1p2p/P1P1P2P/RBRB4 w - - 5 4",
        "1K6/8/1k6/2r5/8/p1p1p2p/P1P1P2P/RBRB4 w - - 7 5"},
       5, 1160, "rook0_black"},
      {"K3k3/1r6/8/8/8/p2p1p1p/P2P1P1P/4BRBR b - - 0 1",
       {"K3k3/7r/8/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 1 2",
        "1K1k4/7r/8/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 3 3",
        "K7/2k4r/8/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 5 4",
        "8/K1k5/7r/8/8/p2p1p1p/P2P1P1P/4BRBR w - - 7 5"},
       5, 1160, "rook0_flip_black"},
      {"K7/8/1r6/8/1k6/p1p1p2p/P1P1P2P/RBRB4 b - - 0 1",
       {"K7/8/8/1r6/1k6/p1p1p2p/P1P1P2P/RBRB4 w - - 1 2",
        "8/K7/8/kr6/8/p1p1p2p/P1P1P2P/RBRB4 w - - 3 3",
        "K7/8/1k6/1r6/8/p1p1p2p/P1P1P2P/RBRB4 w - - 5 4",
        "1K6/8/1k6/2r5/8/p1p1p2p/P1P1P2P/RBRB4 w - - 7 5"},
       5, 1160, "rook0_black"},
      {"rbrb4/p1p1p2p/P1P1P2P/8/1R6/8/8/3K3k w - - 0 1",
       {"rbrb4/p1p1p2p/P1P1P2P/8/8/8/1R6/3K3k b - - 1 1",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/8/1R6/4K1k1 b - - 3 2",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/8/1R3K2/7k b - - 5 3",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/1R6/5K1k/8 b - - 7 4"},
       5, 1160, "rook0"},
      {"rbrb4/p1p1p2p/P1P1P2P/8/8/8/1k1K4/2R5 w - - 0 1",
       {"rbrb4/p1p1p2p/P1P1P2P/8/8/2R5/1k1K4/8 b - - 1 1",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/2R5/k1K5/8 b - - 3 2",
        "rbrb4/p1p1p2p/P1P1P2P/2R5/8/8/2K5/k7 b - - 5 3",
        "rbrb4/p1p1p2p/P1P1P2P/8/8/2R5/k1K5/8 b - - 7 4"},
       5, 1160, "rook0"},
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
    // Neither is promotable today and S148 measured why, not guessed it: see
    // the comment above MATE_IN_THREE_FLOOR.
    MESSAGE("mate in 4: " << exact_by_distance[4] << " of "
                          << total_by_distance[4]
                          << " exact, mate in 5: " << exact_by_distance[5]
                          << " of " << total_by_distance[5]
                          << " -- recovered by RfpMaxDepth and not by "
                             "RfpMinPly, and lowering it lost 7.31 nElo "
                             "(S148, DEC-158); adocs/data/S145_rfp_sweep.log "
                             "and adocs/data/S148_rfp_ceiling_sweep.log");

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
