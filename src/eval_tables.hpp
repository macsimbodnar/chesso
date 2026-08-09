#pragma once
#include "data_structures.hpp"

// Evaluation tables and the incremental accumulator.
//
// These live in a header rather than in evaluation.cpp because make_move()
// updates the accumulator on every piece it touches, and a call across a
// translation unit on that path would not inline. evaluate() was 40% of the
// search when it rebuilt these numbers from the bitboards each time it was
// asked; now it only interpolates what make_move already knows.

// clang-format off

#define PAWN   100
#define KNIGHT 300
#define BISHOP 300
#define ROOK   500
#define QUEEN  900

// Material, White's point of view, indexed by piece type without the colour.
// The king carries no value: both sides always have exactly one in a legal
// position, so it can only cancel, and pricing it meant an illegal position
// with an unbalanced king count produced a score larger than any mate.
static constexpr int piece_value[6] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, 0};

// Phase weights. Queens dominate, pawns and kings contribute nothing, and a
// full set on both sides comes to GAME_PHASE_MAX.
static constexpr int phase_value[6] = {0, 1, 1, 2, 4, 0};

// Piece-square tables, written from White's point of view and read for Black
// by mirroring the square vertically (sq ^ 56).
//
// Index 0 is a8 and index 63 is h1, matching bb_squares_t, so each table below
// reads like a board seen from White: the top row is the eighth rank, where
// White promotes, and the bottom row is White's own back rank.
//
// These numbers are hand-written from ordinary positional principles -
// centralise the knights, keep the king home in the middlegame and active in
// the endgame, push pawns, put rooks on the seventh. They are a starting point
// and nothing more. Phase 5 of EVAL_PLAN.md replaces the lot by fitting them to
// game outcomes, which is what makes a table actually good; until then, expect
// these to be worth much less than a tuned set.

// clang-format off
static constexpr int psqt_mg[6][64] = {
  {  // pawn
      0,   0,   0,   0,   0,   0,   0,   0,
     60,  60,  60,  60,  60,  60,  60,  60,
     15,  15,  25,  35,  35,  25,  15,  15,
      5,   5,  15,  28,  28,  15,   5,   5,
      0,   2,   6,  22,  22,   6,   2,   0,
      4,  -4,  -8,   0,   0,  -8,  -4,   4,
      4,  10,  10, -18, -18,  10,  10,   4,
      0,   0,   0,   0,   0,   0,   0,   0
  },
  {  // knight
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  20,  25,  25,  20,   0, -30,
    -30,   5,  20,  25,  25,  20,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
  },
  {  // bishop
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   8,  12,  12,   8,   0, -10,
    -10,   6,  10,  14,  14,  10,   6, -10,
    -10,   0,  12,  14,  14,  12,   0, -10,
    -10,  10,  10,  12,  12,  10,  10, -10,
    -10,   6,   0,   0,   0,   0,   6, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
  },
  {  // rook
      0,   0,   0,   0,   0,   0,   0,   0,
     10,  15,  15,  15,  15,  15,  15,  10,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   4,   8,   8,   4,   0,   0
  },
  {  // queen
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
  },
  {  // king
    -40, -50, -50, -60, -60, -50, -50, -40,
    -40, -50, -50, -60, -60, -50, -50, -40,
    -40, -50, -50, -60, -60, -50, -50, -40,
    -40, -50, -50, -60, -60, -50, -50, -40,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
     15,  15,  -5,  -5,  -5,  -5,  15,  15,
     20,  35,  10,   0,   0,  10,  35,  20
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn: nothing but how close it is to promoting
      0,   0,   0,   0,   0,   0,   0,   0,
    100, 100, 100, 100, 100, 100, 100, 100,
     60,  60,  60,  60,  60,  60,  60,  60,
     35,  35,  35,  35,  35,  35,  35,  35,
     20,  20,  20,  20,  20,  20,  20,  20,
     10,  10,  10,  10,  10,  10,  10,  10,
      5,   5,   5,   5,   5,   5,   5,   5,
      0,   0,   0,   0,   0,   0,   0,   0
  },
  {  // knight
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
  },
  {  // bishop
    -15,  -8,  -8,  -8,  -8,  -8,  -8, -15,
     -8,   0,   0,   0,   0,   0,   0,  -8,
     -8,   0,   6,   8,   8,   6,   0,  -8,
     -8,   4,   8,  10,  10,   8,   4,  -8,
     -8,   0,   8,  10,  10,   8,   0,  -8,
     -8,   6,   6,   8,   8,   6,   6,  -8,
     -8,   0,   0,   0,   0,   0,   0,  -8,
    -15,  -8,  -8,  -8,  -8,  -8,  -8, -15
  },
  {  // rook: activity matters, the seventh less so once queens are off
      5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0
  },
  {  // queen
    -10,  -5,  -5,  -3,  -3,  -5,  -5, -10,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -3,   0,   5,  10,  10,   5,   0,  -3,
     -3,   0,   5,  10,  10,   5,   0,  -3,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
    -10,  -5,  -5,  -3,  -3,  -5,  -5, -10
  },
  {  // king: the opposite of the middlegame, walk it to the centre
    -50, -30, -30, -30, -30, -30, -30, -50,
    -30, -20, -10,   0,   0, -10, -20, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  30,  40,  40,  30, -10, -30,
    -30, -10,  20,  30,  30,  20, -10, -30,
    -30, -25,   0,   0,   0,   0, -25, -30,
    -50, -30, -30, -30, -30, -30, -30, -50
  }
};
// clang-format on


// clang-format on


// A black piece is worth what the same white piece would be worth on the
// vertically mirrored square, and xor 56 is that mirror: it flips the rank bits
// of the index and leaves the file alone.
inline void eval_add_piece(board_t* board, piece_t piece, index_t square)
{
  if (piece < B_PAWN) {
    board->material += piece_value[piece];
    board->psqt_mg += psqt_mg[piece][square];
    board->psqt_eg += psqt_eg[piece][square];
    board->phase += phase_value[piece];
  } else {
    const int type = piece - B_PAWN;
    const index_t mirrored = square ^ 56;

    board->material -= piece_value[type];
    board->psqt_mg -= psqt_mg[type][mirrored];
    board->psqt_eg -= psqt_eg[type][mirrored];
    board->phase += phase_value[type];
  }
}


inline void eval_remove_piece(board_t* board, piece_t piece, index_t square)
{
  if (piece < B_PAWN) {
    board->material -= piece_value[piece];
    board->psqt_mg -= psqt_mg[piece][square];
    board->psqt_eg -= psqt_eg[piece][square];
    board->phase -= phase_value[piece];
  } else {
    const int type = piece - B_PAWN;
    const index_t mirrored = square ^ 56;

    board->material += piece_value[type];
    board->psqt_mg += psqt_mg[type][mirrored];
    board->psqt_eg += psqt_eg[type][mirrored];
    board->phase -= phase_value[type];
  }
}


// Rebuilds all four from the bitboards. Needed once when a position is loaded,
// and by the assertion that checks the incremental path has not drifted.
inline void eval_refresh(board_t* board)
{
  board->material = 0;
  board->psqt_mg = 0;
  board->psqt_eg = 0;
  board->phase = 0;

  for (index_t square = 0; square < 64; ++square) {
    const piece_t piece = board->squares[square];
    if (piece != EMPTY) { eval_add_piece(board, piece, square); }
  }
}
