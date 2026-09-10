#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "eval_tables.hpp"  // eval_refresh, for the failing branch's rebuild
#include "test_helpers.hpp"
#include "utils.hpp"

// INV-2 and INV-4 in the Release gate. Both gated build directories are
// Release (-O3 -DNDEBUG), so every assert() in src/ is dead in them and the
// only enforcement either invariant had -- assert(squares_match_bitboards) and
// assert(eval_accumulators_match) at three sites in src/bitboard.cpp -- never
// ran under `ctest -L fast`. 2026-09-04_test_review-F01.
//
// This is the same construction tests/test_engine.cpp "hash and board survive
// make/unmake" already applies to the incremental hash, and that one caught the
// en-passant hash mutant of the review's fault-injection pass. The two helpers
// are the from-scratch oracles; S190 took them out of `#ifndef NDEBUG` so a
// Release binary can call them. Nothing in the engine's behaviour changed.

static game_t game;


struct invariants_fixture_t
{
  invariants_fixture_t() { initialize_game_const_data(&game); }
};


TEST_SUITE("invariants: accumulators and squares")
{
  // Depth 2 over 2696 FENs, not the 3 the step file first asked for: depth 3
  // is 53,975,914 make_move calls and about 34 s with the compares, which no
  // fast test can hold. Owner's decision of 2026-09-09. One constant, so
  // S197's deeper gate can raise it in one place.
  static constexpr int CORPUS_DEPTH = 2;

  // Census counters. Their floors below are goldens (DEC-142).
  static uint64_t makes = 0;
  static uint64_t castlings = 0;
  static uint64_t en_passants = 0;
  static uint64_t promotions = 0;
  static uint64_t capture_promotions = 0;

  // Built only when a comparison has already failed. Per-node string building
  // is what makes a walk of this size expensive -- test_engine spends 3.2 s on
  // 426 k makes doing it for every REQUIRE_MESSAGE -- so the walk below calls
  // the oracles as bare booleans and comes here on the failing branch alone.
  static std::string mismatch(const board_t* board, move_t move,
                              const char* when)
  {
    board_t rebuilt = *board;
    eval_refresh(&rebuilt);

    std::string report =
        std::string(when) + print_move(move) + " in " + generate_FEN(board);

    if (rebuilt.material != board->material) {
      report += " material expected " + std::to_string(rebuilt.material) +
                " got " + std::to_string(board->material);
    }
    if (rebuilt.psqt_mg != board->psqt_mg) {
      report += " psqt_mg expected " + std::to_string(rebuilt.psqt_mg) +
                " got " + std::to_string(board->psqt_mg);
    }
    if (rebuilt.psqt_eg != board->psqt_eg) {
      report += " psqt_eg expected " + std::to_string(rebuilt.psqt_eg) +
                " got " + std::to_string(board->psqt_eg);
    }
    if (rebuilt.phase != board->phase) {
      report += " phase expected " + std::to_string(rebuilt.phase) + " got " +
                std::to_string(board->phase);
    }

    for (index_t square = 0; square < 64; ++square) {
      piece_t expected = EMPTY;

      for (int piece = W_PAWN; piece <= B_KING; ++piece) {
        if (GET_BIT(board->bitboards[piece], square)) {
          expected = static_cast<piece_t>(piece);
          break;
        }
      }

      if (board->squares[square] != expected) {
        report += " squares[" + std::to_string(square) + "] expected " +
                  std::to_string(static_cast<int>(expected)) + " got " +
                  std::to_string(static_cast<int>(board->squares[square]));
      }
    }

    return report;
  }


  static void walk(game_t * g, int depth)
  {
    if (depth == 0) { return; }

    move_t moves[MAX_MOVES];
    const size_t count = generate_moves(game_tables(), &g->board, moves);

    for (size_t i = 0; i < count; ++i) {
      board_t before;
      memcpy(&before, &g->board, sizeof(board_t));

      if (!make_move(g, moves[i])) { continue; }

      ++makes;
      if (MOVE_CASTLING(moves[i])) { ++castlings; }
      if (MOVE_EN_PASSANT(moves[i])) { ++en_passants; }
      if (MOVE_PROMOTED(moves[i]) != TO_NONE) {
        ++promotions;
        if (MOVE_CAPTURE(moves[i])) { ++capture_promotions; }
      }

      if (!eval_accumulators_match(&g->board) ||
          !squares_match_bitboards(&g->board)) {
        FAIL(mismatch(&g->board, moves[i], "after make "));
      }

      walk(g, depth - 1);
      unmake_move(g);

      if (!eval_accumulators_match(&g->board) ||
          !squares_match_bitboards(&g->board)) {
        FAIL(mismatch(&g->board, moves[i], "after unmake "));
      }

      // INV-2's own wording: unmake restores the board bit for bit. The
      // accumulators are four fields of that comparison, so this subsumes the
      // "after unmake" check above for everything the oracles look at -- it is
      // here because it is cheap next to a rebuild and because a drift the
      // oracles cannot see (castling rights, the en-passant square) is still a
      // drift.
      if (memcmp(&before, &g->board, sizeof(board_t)) != 0) {
        FAIL(("unmake did not restore the board after " + print_move(moves[i]) +
              " in " + generate_FEN(&before)));
      }
    }
  }


  TEST_CASE_FIXTURE(invariants_fixture_t,
                    "accumulators and squares survive make/unmake over the "
                    "corpus")
  {
    for (const std::string& fen : all_test_fens()) {
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("cannot load " + fen));
      walk(&game, CORPUS_DEPTH);
    }

    // The five positions and depths of test_engine.cpp's hash oracle, so the
    // two Release oracles cover the same tree. Deeper than the corpus walk on
    // purpose: a drift that only appears three plies in has nowhere else to
    // show up.
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
      REQUIRE_MESSAGE(load_FEN(fen, &game), ("cannot load " + fen));
      walk(&game, depth);
    }

    MESSAGE("census: " << makes << " makes, " << castlings << " castlings, "
                       << en_passants << " en passants, " << promotions
                       << " promotions, " << capture_promotions
                       << " capture promotions");

    // GOLDEN (DEC-142): the five census floors below, at about half the census
    // of 2026-09-09 -- 2,132,167 makes, 15,023 castlings, 181 en passants,
    // 221,928 promotions, 206,212 capture promotions.
    // Re-derive: python3 adocs/data/S190_walk_census.py, which counts the same
    // tree with python-chess. Moves legitimately on: a corpus edit or a change
    // to either depth -- re-run it whenever one moves.
    // Margin: a factor of about two on every one of the five, so a corpus edit
    // that halves a class stays green and one that empties it goes red.
    // Property beside it: what this case actually guards -- every make is
    // undone exactly -- which holds at any census, and the MESSAGE above, which
    // prints the counts a drift would show in before a floor is reached.
    //
    // The floors were written under a "Goldens (DEC-142)" heading from S190 and
    // are in the marker's shape since S192's fast check: the listing command
    // DEV_MANUAL.md gives is a grep for that exact string, and a golden the
    // listing misses is one nobody re-derives.
    REQUIRE(makes >= 1000000);
    REQUIRE(castlings >= 7000);
    REQUIRE(en_passants >= 90);
    REQUIRE(promotions >= 100000);
    REQUIRE(capture_promotions >= 100000);
  }


  // The precondition for the case above: a helper that returned true
  // unconditionally would pass it, and pass it faster. Plant each drift the
  // two oracles exist to see and require them to say no.
  TEST_CASE_FIXTURE(invariants_fixture_t, "the oracles see a planted drift")
  {
    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE(eval_accumulators_match(&game.board));
    game.board.phase += 1;
    REQUIRE_FALSE(eval_accumulators_match(&game.board));

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    game.board.material += 1;
    REQUIRE_FALSE(eval_accumulators_match(&game.board));

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    game.board.psqt_mg += 1;
    REQUIRE_FALSE(eval_accumulators_match(&game.board));

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    game.board.psqt_eg += 1;
    REQUIRE_FALSE(eval_accumulators_match(&game.board));

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    REQUIRE(squares_match_bitboards(&game.board));
    game.board.squares[e4] = W_QUEEN;  // a piece squares[] has and the
                                       // bitboards do not
    REQUIRE_FALSE(squares_match_bitboards(&game.board));

    REQUIRE(load_FEN(DEFAULT_POSITION, &game));
    game.board.squares[e2] = EMPTY;  // and the other direction
    REQUIRE_FALSE(squares_match_bitboards(&game.board));
  }
}
