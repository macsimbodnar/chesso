#pragma once
#include <cassert>
#include "data_structures.hpp"

// Evaluation tables and the incremental accumulator.
//
// These live in a header rather than in evaluation.cpp because make_move()
// updates the accumulator on every piece it touches, and a call across a
// translation unit on that path would not inline. evaluate() was 40% of the
// search when it rebuilt these numbers from the bitboards each time it was
// asked; now it only interpolates what make_move already knows.

// clang-format off

#define PAWN   94
#define KNIGHT 327
#define BISHOP 308
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
     216,  118,   96,  206,  172,   43,   12,  112,
     -33,  -25,   -7,   20,   11,    6,  -25,   15,
     -40,   10,    1,    3,   12,    2,   23,  -31,
     -40,  -31,   -3,    0,    1,  -11,  -13,  -34,
     -21,  -29,   -7,  -34,  -26,  -34,   14,  -18,
     -45,  -22,  -29,  -67,  -61,  -25,   14,  -33,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -169, -153, -160, -146,  -18, -257,  130,  -78,
     -39,  -58,   50, -125,  -65,   60,  -96,   -7,
     -19,   63,  -44,   93,   52,   39,   62,   -7,
     -20,   14,   25,   35,   14,   46,    9,   29,
     -17,  -53,   16,    1,   11,   38,    6,   -7,
     -50,  -15,   -1,   24,   21,   17,   19,  -27,
     -63,  -66,  -15,   -8,    0,   -3,   -1,  -34,
    -184,  -53,  -71,  -70,  -26,  -46,  -44, -147
  },
  {  // bishop
       0, -103, -242, -231,  -76, -278,  -94,   41,
     -20,    8,    2,  -50, -181,   20,  -26,   24,
     -81,   11,  -16,   -2,   30, -179,   63,  -15,
      12,   43,    9,   53,   24,    7,   34,   41,
      57,  -13,   39,   27,   38,   33,    0,   24,
      34,   45,   40,   35,   27,   49,   48,   39,
      43,   60,   41,   33,   45,   44,   69,    0,
      15,   14,   28,  -16,   26,   20,  -21,   -4
  },
  {  // rook
     -36,  -44,  -94,  -42,  -75,  -40,   46,    7,
     -39,  -59,  -36,   21,    0,  -18,  -75,  -21,
     -79,  -66,  -84,  -16,  -23,  -45,  -46,  -49,
     -92,  -99,  -76,  -76,  -64,  -58,  -71,  -82,
    -100, -143, -111, -113, -100,  -81, -113,  -82,
    -110, -109, -115, -114,  -77,  -96,  -72, -101,
     -99, -108, -100,  -95,  -83,  -91,  -58, -110,
     -77,  -73,  -70,  -50,  -46,  -59,  -76,  -56
  },
  {  // queen
     693,  673,  712,  749,  785,  793,  734,  750,
     699,  677,  697,  679,  695,  753,  740,  726,
     719,  703,  722,  732,  757,  763,  749,  736,
     693,  706,  716,  705,  705,  721,  691,  746,
     723,  690,  709,  703,  716,  704,  712,  724,
     687,  726,  706,  717,  708,  721,  731,  702,
     724,  725,  744,  735,  743,  748,  762,  723,
     703,  719,  715,  732,  728,  699,  749,  731
  },
  {  // king
      93,  161, -160,  -92,    0,  420,  104,  841,
      87,  231,   27, -179,  -80,  260,  -23,  391,
      42,  -65, -147, -136,   61,   38,  114,  -32,
    -121,  -50, -114, -139,  -74,   -6,   15,  -96,
    -156,  -90,  -52, -117, -108,  -67,  -71,  -95,
     -38,  -13,  -54,  -75,  -71,  -49,  -27,  -28,
      64,   20,  -20,  -37,  -33,   -9,   40,   35,
     -68,   53,   44,  -32,   14,   -4,   73,   59
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     187,  228,  199,  156,  137,  204,  248,  203,
      97,   95,   79,   38,   41,   57,   94,   74,
      70,   46,   42,   26,   38,   45,   41,   57,
      59,   52,   29,   33,   39,   39,   51,   51,
      54,   43,   36,   37,   48,   62,   29,   41,
      57,   46,   41,   59,   70,   62,   43,   45,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
     -96,   27,   39,    4,  -14,   52,  -36,  -92,
     -43,   -2,  -12,   86,   51,   -3,    8,   14,
     -25,   -7,   68,   23,   32,   42,   11,  -22,
       2,   22,   34,   41,   62,   42,   45,  -18,
      -7,   23,   31,   42,   46,   22,    5,  -23,
     -42,   -4,   12,   13,   20,    5,   -6,  -47,
     -59,  -42,  -21,  -12,  -19,  -34,  -50,  -69,
     -70,  -85,  -43,  -17,  -52,  -42,  -72,  -58
  },
  {  // bishop
      25,   50,   71,   71,   53,   68,   55,    6,
      19,   31,   40,   38,   92,   26,   33,  -12,
      70,   38,   37,   46,   25,  111,   27,   53,
      13,   36,   43,   27,   32,   32,   29,   24,
      -7,   31,   27,   32,   21,   34,   25,    5,
       4,    6,   27,   39,   41,   12,   18,   15,
     -39,  -11,   -9,   -4,    5,   -3,   11,   24,
     -49,  -21,  -20,    0,  -16,   -3,    8,  -16
  },
  {  // rook
     137,  142,  161,  142,  155,  135,  125,  131,
     158,  166,  171,  146,  151,  145,  158,  141,
     152,  151,  168,  129,  135,  154,  148,  139,
     144,  150,  151,  138,  135,  133,  126,  131,
     129,  153,  148,  138,  131,  117,  130,  110,
     129,  121,  128,  115,   98,  102,   95,  100,
      82,   98,   86,  103,   84,   78,   74,   83,
      92,   93,  113,   92,   98,  102,   99,   59
  },
  {  // queen
     139,  193,  158,  173,  141,  108,  118,   67,
     137,  213,  211,  265,  278,  175,  159,  140,
      99,  174,  190,  209,  190,  191,  170,  106,
     141,  155,  195,  206,  232,  200,  224,  102,
     107,  147,  168,  184,  188,  161,  118,   76,
      97,   54,  122,   83,  111,  104,   53,   79,
     -19,   -4,    3,    8,   15,  -31, -107,  -91,
      65,   -6,   27,  -14,  -21,    6,  -40,   -3
  },
  {  // king
     -84, -162,  -91,   63,   22,  -62, -131, -269,
      10,   64,   93,  141,  124,   47,   38,  -84,
      56,  154,  169,  170,  114,  128,   78,   30,
      86,  132,  164,  168,  157,  125,   98,   77,
     114,  125,  126,  157,  150,  127,  111,   83,
      61,   84,  114,  130,  132,  111,   84,   57,
      29,   67,   91,  109,   95,   88,   63,   31,
      49,   20,   53,   61,   47,   43,   32,  -12
  }
};
// clang-format on


// clang-format on


// A black piece is worth what the same white piece would be worth on the
// vertically mirrored square, and xor 56 is that mirror: it flips the rank bits
// of the index and leaves the file alone.
//
// The hooks come in two forms, S253. make_move's helpers know the piece's
// colour at compile time, and the `Side` form takes it from there: one branch
// fewer and half the code at each inlined copy -- make_move_impl makes eight
// hook calls and is instantiated for both colours, so sixteen copies -- which
// is what lets every copy be inlined. Forced (CHESSO_ALWAYS_INLINE) because
// src/bitboard.cpp sits at gcc 13.3's inline-unit-growth limit: on the plain
// hint gcc left eight of those copies out of line, more whenever the unit
// grew, so a generator change was timed on the inliner's budget and not on its
// own cost (S020).
template <color_t Side>
CHESSO_ALWAYS_INLINE inline void eval_add_piece(board_t* board,
                                                piece_t piece,
                                                index_t square)
{
  assert((piece < B_PAWN) == (Side == WHITE));

  if constexpr (Side == WHITE) {
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


template <color_t Side>
CHESSO_ALWAYS_INLINE inline void eval_remove_piece(board_t* board,
                                                   piece_t piece,
                                                   index_t square)
{
  assert((piece < B_PAWN) == (Side == WHITE));

  if constexpr (Side == WHITE) {
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


CHESSO_ALWAYS_INLINE inline void eval_add_piece(board_t* board,
                                                piece_t piece,
                                                index_t square)
{
  if (piece < B_PAWN) {
    eval_add_piece<WHITE>(board, piece, square);
  } else {
    eval_add_piece<BLACK>(board, piece, square);
  }
}


CHESSO_ALWAYS_INLINE inline void eval_remove_piece(board_t* board,
                                                   piece_t piece,
                                                   index_t square)
{
  if (piece < B_PAWN) {
    eval_remove_piece<WHITE>(board, piece, square);
  } else {
    eval_remove_piece<BLACK>(board, piece, square);
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
