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

#define PAWN   87
#define KNIGHT 366
#define BISHOP 339
#define ROOK   505
#define QUEEN  1067

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
      79,   98,  178,  129,  171,    2,  214,  100,
     -51,  -31,   28,   84,   45,   34,  -41,   62,
     -55,  -27,  -38,   21,    6,  -17,  -17,  -26,
     -55,  -21,  -24,    4,    2,  -10,  -15,  -40,
     -52,  -13,  -22,  -15,  -15,  -14,   21,  -26,
     -56,  -15,  -25,  -27,  -31,   -1,   19,  -39,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -207,  126,  -93,   -6,  -29, -326, -265, -205,
     -33,   32,    9,   99,   11,   72,   77,  -12,
      11,  111,   51,  118,  139,   58,   81,  -26,
      88,   37,   47,   61,   47,   97,   27,   50,
      -9,  -23,   30,   52,   38,   36,   42,    8,
      -2,   14,   19,   42,   28,   32,   10,    0,
     -38,  -27,   -9,   18,   14,   29,    6,   -8,
     -69,    4,   12,   33,  -32,   -9,    8, -122
  },
  {  // bishop
     -79,   91,  -28,  -46,   39, -149,   93,   33,
      53,   25,    8,   78,   50,   80,   91,  -10,
       1,   79,   76,   59,  123,   61,   45,   28,
       6,   39,   61,  100,   56,   38,   46,    4,
      57,   39,   56,   63,   51,   54,   29,   38,
      48,   39,   49,   36,   37,   42,   49,   41,
      53,   46,   17,   46,   47,   33,   62,  -90,
     -35,  -48,   46,    5,   24,   39,   23,   15
  },
  {  // rook
      16,   22,  -14,   50,   31,   67,   -2,   32,
     -22,    8,   76,   45,   65,   32,    5,   74,
      11,    5,   50,   87,   43,   76,   20,   30,
     -27,  -61,  -35,    2,   10,    1,   -8,   -1,
     -69,  -55,  -65,   -2,  -63,   -9,  -21,  -59,
     -61,  -25,  -29,  -34,  -23,  -22,   -9,  -31,
     -61,  -54,  -41,  -15,    0,  -30,  -16,  -66,
     -34,  -24,  -12,   -7,    4,    3,  -45,  -40
  },
  {  // queen
     -64,   47,  158,  104,  112,  169,  -23,    5,
      60,   14,   -3,  108,   78,   43,   84,  126,
      -8,   38,   11,   41,   22,  109,   85,   33,
      31,   12,    9,   18,   37,   48,   53,   54,
      16,    7,   38,   28,   41,   34,   53,   37,
      53,   35,   36,   43,   27,   35,   71,   29,
      47,   29,   62,   54,   44,   53,   57,   73,
      16,   59,   45,   50,   40,   77,   92,   71
  },
  {  // king
    -323,  393,  608,  223,  459,  732,  283,   55,
     505,  240,  290,  129,  159,  -66,   -8, -314,
     -37,  -23,   -1, -142,  -95,   11,  -84,    9,
      14,   18, -177, -326, -193, -151,   48,  -27,
    -148,   -5,   -7,  -72, -108,   27, -138, -141,
     -21,  -60,  -60,  -85, -112, -103,  -79, -133,
      34,  -46,  -72,  -64,  -63,  -40,  -38,  -16,
       7,   22,   21,  -42,   14,  -38,   25,   23
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     184,  206,  175,  154,  115,  190,  152,  165,
     136,  103,  118,   93,  112,   76,  140,   83,
      93,   76,   61,   44,   33,   54,   67,   65,
      69,   59,   55,   28,   34,   42,   52,   44,
      58,   44,   41,   34,   25,   47,   23,   35,
      65,   50,   60,    4,   30,   45,   39,   36,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
     -54,  -57,  -72,  -46,  -60,  -30,  -24,  -96,
    -101,  -61,  -39,  -46,  -50,  -54,  -71, -113,
     -57,  -49,  -51,  -28,  -37,  -37,  -65,  -74,
    -100,  -59,  -13,  -17,  -12,  -47,   -1,  -22,
     -67,  -18,  -27,  -44,  -15,    4,  -59,  -38,
    -103,  -37,  -29,  -26,  -21,  -36,  -24,  -98,
     -12, -108,  -68,  -74,  -55,  -79,  -60,  -33,
     -93, -130,  -80,  -52,  -45,  -39, -102,  -44
  },
  {  // bishop
      -8,  -36,  -26,  -20,  -76,   -8,  -82,  -81,
     -59,  -25,  -12,  -45,  -12,  -48,  -55,  -73,
     -10,  -25,  -35,  -25,  -38,  -23,  -53,   -8,
     -27,  -28,  -45,  -31,  -43,  -34,  -13,  -14,
     -56,  -30,  -55,  -43,  -45,  -34,  -10,  -35,
     -65,  -49,  -43,  -21,  -21,  -35,  -25,  -44,
     -81,  -53,  -75,  -53,  -36,  -37,  -69,   47,
     -60,  -43,  -60,  -22,  -60,  -57,  -87,  -54
  },
  {  // rook
      69,   57,   57,   50,   52,   24,   70,   60,
      79,   51,   40,   64,   57,   46,   69,   49,
      61,   63,   49,   36,   54,   40,   51,   38,
      61,   90,   70,   70,   69,   56,   58,   57,
      60,   74,   85,   39,   75,   42,   46,   50,
      31,   41,   32,   30,   36,   25,   25,   17,
       2,   34,   15,   23,   14,   21,    8,   44,
      11,   36,   33,   22,   15,   17,   53,   15
  },
  {  // queen
      43,  -32,  -51,  -48,  -48, -100,   67,   73,
     -68,   32,   99,  -40,   -8,   70,  -39,   -3,
      54,  -24,   47,   35,   94,   31,   70,  101,
     -19,   20,   19,   12,   23,   14,   27,   45,
     -33,    1,  -17,   10,   14,   16,    8,    3,
    -108,  -62,  -46, -104,  -70,  -15,  -54,  -38,
     -95, -146, -126, -104,  -98,  -64, -120, -203,
       7, -149,  -67,  -46,  -65, -248, -213, -165
  },
  {  // king
    -204,   25,  -16,  -52, -131,  -94,   46, -151,
    -152,  -40,  -17,    4,  -13,   67,   64,   83,
     -33,   36,   29,   30,   25,   31,   71,   53,
     -26,   37,   31,   37,   16,   47,   17,   12,
      -4,  -20,   -5,   18,   17,    5,   32,  -29,
     -31,   -1,    9,   12,   26,   19,    1,    1,
     -23,   -9,   -2,   -1,   -5,  -15,  -21,  -33,
     -70,  -45,  -58,  -49,  -72,  -37,  -60,  -73
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
