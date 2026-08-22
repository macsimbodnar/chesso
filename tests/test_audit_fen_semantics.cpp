// AUDIT EVIDENCE — 2026-08-22_adversarial-F02. Written by the 2026-08-22
// adversarial audit as the regression test for that finding. NOT registered in
// tests/CMakeLists.txt and NOT executed by the audit: the audit ran while a
// time-controlled match held every core, so it was static-only by constraint.
// Register it with add_doctest_target(test_audit_fen_semantics) and run it
// once the machine is free; by the analysis in the finding it is red on
// `293a45b` plus the S130 working tree.
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


TEST_SUITE("audit: FEN semantic validation (2026-08-22_adversarial-F02)")
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
}
