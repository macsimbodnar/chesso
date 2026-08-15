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

#define PAWN   95
#define KNIGHT 327
#define BISHOP 306
#define ROOK   487
#define QUEEN  716

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
     255,  129,  102,  217,  149,   22,   -5,  113,
     -28,  -22,    4,   25,   19,   15,  -24,   21,
     -37,   17,    6,    8,   16,    6,   27,  -29,
     -38,  -27,    0,    3,    4,  -10,  -10,  -33,
     -18,  -24,   -3,  -31,  -21,  -30,   20,  -14,
     -42,  -17,  -25,  -66,  -59,  -24,   18,  -29,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -183, -177, -125, -124,    2, -244,   82,  -70,
     -17,  -56,   85, -100,  -49,   89,  -71,   12,
      17,   87,  -15,  109,   87,   75,   84,   10,
      -2,   39,   52,   66,   39,   73,   30,   55,
      13,  -24,   39,   25,   37,   63,   29,   24,
     -26,    7,   24,   46,   46,   42,   48,   -6,
     -34,  -26,   12,   16,   25,   23,   19,    0,
    -137,  -30,  -49,  -41,    8,  -19,  -20, -119
  },
  {  // bishop
      33,  -82, -234, -209,  -66, -263,  -66,   74,
      18,   44,   36,  -14, -164,   59,   11,   53,
     -57,   46,   12,   38,   66, -157,   94,   14,
      32,   70,   49,   84,   58,   35,   61,   67,
      81,   10,   69,   57,   64,   60,   30,   54,
      65,   75,   68,   64,   54,   76,   72,   67,
      72,   89,   60,   60,   75,   74,   98,   48,
      47,   59,   56,   17,   50,   48,   34,   32
  },
  {  // rook
      13,   52,   -2,   31,   44,   98,  121,   53,
       7,   -9,   20,   91,   85,   74,  -17,   14,
     -39,  -11,  -32,   47,   29,   11,    0,  -16,
     -42,  -47,  -28,  -22,  -19,   -4,  -29,  -49,
     -59,  -96,  -65,  -66,  -64,  -42,  -66,  -44,
     -70,  -66,  -73,  -71,  -41,  -54,  -35,  -62,
     -60,  -63,  -64,  -49,  -35,  -52,  -21,  -68,
     -37,  -31,  -27,   -7,   -4,  -16,  -38,  -18
  },
  {  // queen
     706,  694,  740,  746,  802,  805,  743,  769,
     720,  696,  721,  703,  721,  787,  782,  744,
     746,  727,  748,  760,  797,  787,  779,  756,
     720,  726,  736,  730,  728,  750,  712,  775,
     743,  709,  730,  721,  741,  729,  736,  748,
     708,  748,  727,  739,  728,  744,  756,  730,
     749,  744,  770,  758,  768,  773,  792,  750,
     732,  740,  741,  758,  750,  718,  770,  759
  },
  {  // king
      45,  311, -202, -189,   15,  629,  787,  888,
     112,  221, -194, -505, -207,  414,  585, 1152,
      31, -454, -668, -443, -149,  -27,  409,  471,
    -129, -310, -362, -287, -145,  -76,    7, -106,
    -198, -129, -105, -164, -153, -110, -122, -166,
    -112,  -65, -104, -120, -125, -107,  -80,  -87,
       5,  -41,  -82,  -93,  -91,  -70,  -26,  -37,
    -167,   -6,  -18,  -94,  -48,  -68,    8,   -9
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     159,  218,  196,  154,  152,  219,  259,  195,
      86,   81,   67,   25,   30,   53,   90,   68,
      63,   40,   35,   21,   30,   39,   35,   51,
      53,   42,   21,   29,   31,   34,   45,   45,
      46,   33,   29,   28,   42,   56,   25,   38,
      50,   37,   35,   47,   61,   59,   39,   43,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -103,   12,    8,  -23,  -42,   20,  -63, -123,
     -67,  -27,  -51,   51,   19,  -28,  -21,  -18,
     -62,  -39,   34,   -7,  -11,    0,  -23,  -57,
     -29,  -11,   -6,    2,   29,    6,   25,  -60,
     -45,  -15,   -2,    7,    7,  -15,  -22,  -67,
     -75,  -38,  -20,  -22,  -12,  -28,  -40,  -82,
     -94,  -96,  -54,  -44,  -51,  -73,  -86,  -98,
    -105, -118,  -82,  -53,  -91,  -71, -104, -112
  },
  {  // bishop
     -23,   11,   44,   29,   26,   43,   24,  -28,
     -25,   -7,   -2,   -1,   55,  -11,   -6,  -44,
      38,   -7,    4,    4,  -16,   74,  -13,   18,
     -23,    3,   -2,   -7,  -12,   -6,   -4,  -11,
     -35,   -2,   -7,   -8,  -16,    1,  -15,  -32,
     -32,  -28,   -7,    3,    6,  -24,  -17,  -14,
     -80,  -46,  -42,  -38,  -34,  -44,  -27,  -31,
     -82,  -76,  -60,  -36,  -51,  -37,  -45,  -63
  },
  {  // rook
      77,   64,   84,   72,   63,   35,   50,   75,
      98,  100,  108,   79,   70,   64,   91,   88,
      97,   92,  109,   67,   72,   84,   89,   85,
      85,   84,   90,   78,   76,   67,   69,   83,
      72,   93,   90,   81,   80,   66,   72,   57,
      70,   63,   71,   58,   45,   48,   42,   49,
      30,   37,   38,   42,   26,   24,   23,   30,
      40,   43,   62,   41,   43,   48,   50,   13
  },
  {  // queen
     116,  175,  134,  169,  121,   81,   97,   51,
      99,  189,  186,  245,  245,  128,  111,  110,
      59,  140,  164,  184,  145,  147,  115,   74,
     109,  129,  174,  187,  218,  178,  185,   56,
      78,  127,  144,  175,  165,  133,   96,   37,
      70,   35,  106,   64,   83,   81,   16,   38,
     -52,  -36,  -32,  -26,  -15,  -62, -159, -144,
      23,  -47,  -20,  -53,  -45,  -31,  -84,  -29
  },
  {  // king
     -33, -185,  -83,  116,   33, -120, -297, -291,
      27,   85,  176,  268,  186,    7, -133, -319,
      69,  316,  352,  290,  189,  160,    1, -122,
      99,  239,  265,  226,  194,  162,  113,   94,
     131,  148,  154,  183,  178,  152,  142,  114,
      89,  108,  137,  153,  158,  143,  115,   90,
      66,  102,  120,  139,  129,  121,   98,   69,
     127,   52,   83,   90,   82,   74,   68,   28
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
