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

#define PAWN   78
#define KNIGHT 338
#define BISHOP 339
#define ROOK   503
#define QUEEN  1026

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
// These numbers, and the five piece values above them, are fitted. S028 ran
// tools/tuner over 1490839 quiet positions from 20000 self-play games at 100000
// nodes a move, minimising the squared error of sigmoid(K * evaluate()) against
// the result of the game each position came from, K = 1.1141 fitted from the
// data. Held-out error 0.113852 before, 0.108043 after. They replaced a
// hand-written set chosen from ordinary positional principles.
//
// Do not read meaning into an individual number. The parameterisation is
// degenerate by five dimensions -- adding c to every square of psqt_mg[t] and
// psqt_eg[t] is the same evaluation as adding c to piece_value[t] -- so the
// split between a piece's value and its table is arbitrary and only the sum is
// fitted. A pawn priced at 78 does not mean the tuner thinks a pawn is worth
// less than a pawn.
//
// Nothing here is copied. The data is chesso's own self-play and no other
// engine's evaluation, search or published table went into it. DEC-016.

// clang-format off
static constexpr int psqt_mg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
      89,  101,  175,  130,  168,   17,  204,  105,
     -42,  -25,   35,   87,   52,   40,  -34,   60,
     -52,  -24,  -31,   21,    8,  -12,  -14,  -25,
     -56,  -18,  -26,    3,    0,  -13,  -14,  -41,
     -61,  -11,  -21,  -22,  -19,  -18,   21,  -34,
     -72,  -19,  -26,  -41,  -44,   -5,   13,  -52,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -204,  141,  -57,    2,  -24, -300, -244, -187,
     -13,   30,   18,   98,   16,   76,   75,    0,
      18,  113,   49,  117,  135,   54,   82,  -17,
      84,   31,   49,   57,   47,   93,   22,   46,
      -9,  -18,   24,   46,   32,   28,   41,    7,
      -5,   12,   15,   32,   18,   24,    9,   -2,
     -51,  -30,  -12,    4,    1,   23,    1,  -15,
     -63,   -8,  -11,   10,  -51,  -19,   -2, -113
  },
  {  // bishop
     -95,   91,  -12,  -27,   49, -142,   95,   25,
      58,   37,   27,   96,   63,   85,   97,    0,
       4,   80,   84,   74,  133,   72,   48,   33,
       9,   40,   73,  106,   63,   53,   48,    5,
      49,   41,   59,   61,   53,   58,   31,   31,
      38,   33,   42,   35,   33,   33,   41,   32,
      31,   31,   14,   34,   35,   25,   49,  -87,
     -54,  -65,   17,  -23,    2,   18,    6,   -3
  },
  {  // rook
      67,   86,   56,  120,  103,  131,   73,   82,
      28,   62,  121,   99,  120,   85,   51,  115,
      46,   50,   91,  134,   88,  128,   62,   65,
       4,  -18,    0,   43,   54,   43,   29,   28,
     -40,  -22,  -40,   25,  -35,   19,    7,  -29,
     -46,   -2,   -9,  -21,  -11,   -5,   11,  -19,
     -63,  -42,  -27,  -13,   -2,  -19,   -8,  -70,
     -34,  -21,  -15,  -11,    0,   -2,  -45,  -48
  },
  {  // queen
     -74,   58,  170,  117,  134,  194,  -17,    2,
      66,   19,   11,  112,   84,   51,   86,  127,
      -7,   45,   19,   52,   33,  111,   83,   33,
      31,   13,   20,   24,   41,   54,   52,   49,
      11,    9,   35,   28,   39,   31,   50,   32,
      42,   29,   29,   33,   19,   28,   62,   21,
      20,   14,   50,   32,   24,   33,   41,   41,
      -6,   23,    8,   27,   10,   41,   52,   39
  },
  {  // king
    -302,  357,  572,  204,  428,  654,  242,   34,
     483,  223,  269,  119,  145,  -72,  -14, -321,
     -39,  -23,   -4, -135,  -88,   16,  -86,   -2,
      19,   18, -167, -307, -179, -136,   47,  -31,
    -132,   -7,    0,  -58,  -94,   35, -126, -124,
     -21,  -49,  -49,  -73,  -95,  -90,  -67, -120,
      34,  -39,  -57,  -58,  -58,  -30,  -26,   -9,
      19,   23,   16,  -42,   17,  -42,   22,   27
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     169,  191,  160,  139,  104,  174,  143,  152,
     128,   96,  109,   84,  103,   70,  132,   80,
      89,   73,   56,   42,   31,   50,   64,   63,
      68,   57,   53,   28,   33,   42,   51,   44,
      60,   43,   39,   36,   25,   47,   23,   38,
      71,   50,   60,    9,   34,   45,   41,   43,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
     -47,  -58,  -76,  -45,  -57,  -29,  -24,  -90,
    -100,  -59,  -41,  -49,  -52,  -56,  -68, -107,
     -57,  -50,  -52,  -34,  -39,  -41,  -66,  -73,
     -92,  -55,  -18,  -20,  -16,  -49,   -1,  -21,
     -60,  -19,  -29,  -45,  -17,    1,  -59,  -36,
     -95,  -37,  -30,  -25,  -21,  -34,  -26,  -92,
      -6,  -98,  -62,  -69,  -49,  -75,  -55,  -34,
     -87, -134,  -75,  -48,  -40,  -37, -103,  -39
  },
  {  // bishop
      -5,  -29,  -18,  -13,  -64,    4,  -70,  -72,
     -53,  -11,    4,  -29,    4,  -29,  -38,  -67,
       0,   -9,   -9,   -3,  -15,    1,  -34,   -1,
     -14,   -5,  -19,    4,   -8,   -8,    9,   -3,
     -42,   -7,  -23,   -8,   -9,   -5,   11,  -20,
     -48,  -30,  -16,    3,    3,   -6,  -10,  -31,
     -67,  -37,  -56,  -34,  -15,  -21,  -52,   46,
     -57,  -33,  -66,  -10,  -46,  -62,  -75,  -51
  },
  {  // rook
      37,   23,   21,   14,   17,   -9,   32,   27,
      45,   18,   10,   30,   23,   13,   35,   16,
      33,   32,   20,    4,   23,    5,   20,   10,
      32,   58,   42,   39,   36,   24,   26,   27,
      28,   44,   57,   13,   47,   12,   18,   19,
       4,   15,    7,    8,   14,    0,   -1,   -9,
     -19,   12,   -7,    4,   -4,    0,  -11,   23,
      -8,   19,   18,   10,    3,    1,   39,   -2
  },
  {  // queen
      60,  -26,  -43,  -40,  -48, -100,   71,   82,
     -66,   42,   99,  -27,    2,   77,  -26,   -5,
      60,  -20,   60,   44,   97,   43,   75,   98,
     -17,   37,   25,   33,   45,   23,   31,   47,
     -22,   13,    3,   29,   34,   32,   14,    9,
     -91,  -41,  -25,  -79,  -55,   -3,  -46,  -29,
     -65, -122, -106,  -75,  -72,  -45,  -97, -166,
      16, -126,  -50,  -72,  -52, -208, -184, -139
  },
  {  // king
    -187,   24,  -14,  -48, -122,  -83,   42, -138,
    -143,  -37,  -16,    4,  -12,   63,   59,   82,
     -31,   33,   28,   27,   22,   27,   67,   52,
     -27,   35,   29,   36,   14,   44,   15,   13,
      -5,  -19,   -5,   15,   16,    4,   29,  -29,
     -29,   -2,    6,   10,   22,   16,   -1,    0,
     -22,   -9,   -4,   -1,   -3,  -16,  -22,  -32,
     -65,  -43,  -57,  -50,  -80,  -38,  -55,  -68
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
