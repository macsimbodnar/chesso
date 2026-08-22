// AUDIT EVIDENCE — 2026-08-22_adversarial-F01. Written by the 2026-08-22
// adversarial audit as the regression test for that finding, static-only: the
// audit ran while a time-controlled match held every core, so it registered
// and ran nothing. S161 registered it and observed it red as predicted — 4 of
// 4 cases failing on 0a274c9 — then fixed load_FEN. The audit cited its own
// finding as F02 in this header and in the suite name; F02 is fastchess.sh's
// default reference (S160), and the id is corrected here to F01.
//
// The finding: load_FEN() validates FEN syntax and not semantics. Two classes
// of well-formed-but-illegal FEN are accepted, and the moves they license
// corrupt the board in a Release build:
//
//   1. castling rights with the king or rook not on its castling square. The
//      generator's castling block (src/bitboard.cpp:395-435) tests the rights
//      bits, the empty squares and the attacked squares, and never that the
//      king stands on e1/e8 or the rook on its corner. make_move() then moves
//      a rook — castling_rook() answers from the target square, not from the
//      board (src/bitboard.cpp:645-659) — and move_piece()'s xors *create*
//      pieces on squares that were empty.
//
//   2. an en-passant square on any rank. load_FEN() checks only that the
//      square parses (src/bitboard.cpp:1755-1765); an ep square whose victim
//      square holds no enemy pawn licenses an en-passant capture whose
//      remove_piece() xor *adds* the missing pawn.
//
// The assertions below state the correct behaviour — no castling move and no
// en-passant capture may be generated from these positions — so they are
// red-first against the defect and fix-agnostic: they pass whether the fix
// refuses the FEN, sanitizes the rights/ep square, or teaches the generator to
// check placement. INV-1 (legal-only movegen) and the prime directive ("never
// plays or accepts an illegal move and never corrupts its own board state")
// are the properties under test.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <string>
#include "bitboard.hpp"
#include "data_structures.hpp"
#include "doctest.h"

namespace
{

struct fen_fixture_t
{
  game_t game = {};

  fen_fixture_t() { initialize_game_const_data(&game); }
};

size_t count_flagged(const game_t* game, bool castling, bool en_passant)
{
  move_t moves[MAX_MOVES];
  const size_t count =
      generate_moves(game_tables(), &((game_t*)game)->board, moves);

  size_t flagged = 0;

  for (size_t i = 0; i < count; ++i) {
    if (castling && MOVE_CASTLING(moves[i])) { flagged++; }
    if (en_passant && MOVE_EN_PASSANT(moves[i])) { flagged++; }
  }

  return flagged;
}

}  // namespace


TEST_SUITE("audit: FEN semantic validation (2026-08-22_adversarial-F01)")
{
  TEST_CASE_FIXTURE(fen_fixture_t,
                    "castling rights without the rook license no castle")
  {
    // White claims kingside rights with no rook on h1. If load_FEN accepts
    // this FEN, the position must still generate no castling move: applying
    // one would xor a rook onto h1 and f1 out of nothing.
    const std::string fen = "4k3/8/8/8/8/8/8/4K3 w K - 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, true, false), 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "castling rights with the king displaced license no castle")
  {
    // The rook is on h1 but the king is on d1. The generator's castling block
    // hard-codes e1 as the from-square, so an emitted castle here moves a king
    // that is not there.
    const std::string fen = "3k4/8/8/8/8/8/8/3K3R w K - 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, true, false), 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "an en-passant square with no victim licenses no capture")
  {
    // ep = e4 is not a square any double push can produce for White to act
    // on, and e3 — the square the capture would remove a black pawn from — is
    // empty. If load_FEN accepts the FEN, no en-passant capture may be
    // generated: applying one would xor a black pawn onto e3 out of nothing.
    const std::string fen = "4k3/8/8/8/8/3P4/8/4K3 w - e4 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, false, true), 0);
    }
  }

  TEST_CASE_FIXTURE(fen_fixture_t,
                    "a plausible-rank ep square with no victim licenses "
                    "no capture")
  {
    // Same defect on the rank en passant really uses: ep = e6 with no black
    // pawn on e5. Well-formed, wrong, and accepted today.
    const std::string fen = "4k3/8/8/3P4/8/8/8/4K3 w - e6 0 1";

    if (load_FEN(fen, &game)) {
      CHECK_EQ(count_flagged(&game, false, true), 0);
    }
  }

  // ------------------------------------------------------------------------
  // S161. The four cases above are negative assertions and every one of them
  // would also pass if load_FEN cleared *all* castling rights and *every* ep
  // square, which is a worse engine and a green test. These fix the fix from
  // the other side: what a legal position carries has to survive untouched,
  // and the clearing has to be per right rather than wholesale.
  // ------------------------------------------------------------------------

  TEST_CASE_FIXTURE(fen_fixture_t, "a legal position keeps all four rights")
  {
    REQUIRE(
        load_FEN("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", &game));

    CHECK_EQ(game.board.castling, WK | WQ | BK | BQ);
    CHECK_EQ(count_flagged(&game, true, false), 2);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a real ep square survives and is playable")
  {
    // Black has just played d7-d5. The target square is empty, the victim pawn
    // stands on d5, and White's e5 pawn may take it.
    REQUIRE(load_FEN("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", &game));

    CHECK_EQ(game.board.en_passant, d6);
    CHECK_EQ(count_flagged(&game, false, true), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "only the unsupported right is cleared")
  {
    // The a1 rook is there and the h1 rook is not, so WQ survives WK.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/R3K3 w KQ - 0 1", &game));

    CHECK_EQ(game.board.castling, WQ);
    CHECK_EQ(count_flagged(&game, true, false), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "the same holds for Black")
  {
    // Black claims both; only the h8 rook exists, so BK survives BQ.
    REQUIRE(load_FEN("4k2r/8/8/8/8/8/8/4K3 b kq - 0 1", &game));

    CHECK_EQ(game.board.castling, BK);
    CHECK_EQ(count_flagged(&game, true, false), 1);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a right with no king anywhere is cleared")
  {
    // Both white rooks in place, no white king at all. Nothing may be emitted
    // from e1, and the rights must not survive to say otherwise.
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/R6R w KQ - 0 1", &game));

    CHECK_EQ(game.board.castling, 0);
    CHECK_EQ(count_flagged(&game, true, false), 0);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "an ep square on the mover's own rank goes")
  {
    // ep = d3 with White to move: rank 3 is the rank *Black* captures on, so
    // this is White being offered its own double push back. The victim index
    // for White would be d2, and there is a white pawn there -- which is how
    // an unsanitized load turns this into a capture of one's own pawn.
    REQUIRE(load_FEN("4k3/8/8/8/2p5/8/3P4/4K3 w - d3 0 1", &game));

    CHECK_EQ(game.board.en_passant, INVALID_INDEX);
    CHECK_EQ(count_flagged(&game, false, true), 0);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "an occupied ep target square goes")
  {
    // Victim present on e5 and a white rook standing on the target e6: the
    // capture's move_piece() would xor a pawn onto a square already occupied.
    REQUIRE(load_FEN("4k3/8/4R3/3Pp3/8/8/8/4K3 w - e6 0 1", &game));

    CHECK_EQ(game.board.en_passant, INVALID_INDEX);
    CHECK_EQ(count_flagged(&game, false, true), 0);
  }

  TEST_CASE_FIXTURE(fen_fixture_t, "a cleared field is cleared in the hash")
  {
    // The rights and the ep square are Zobrist inputs, so sanitizing after the
    // fields are parsed but before compute_full_hash() is what keeps a
    // sanitized position hashing as the position it actually is. Same board,
    // one FEN carrying a right the board cannot support.
    game_t plain = {};
    initialize_game_const_data(&plain);

    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w K - 0 1", &game));
    REQUIRE(load_FEN("4k3/8/8/8/8/8/8/4K3 w - - 0 1", &plain));

    CHECK_EQ(game.board.castling, plain.board.castling);
    CHECK_EQ(game.board.hash, plain.board.hash);
  }
}
