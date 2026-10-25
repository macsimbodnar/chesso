#pragma once
#include <algorithm>
#include <array>
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

#define PAWN   84
#define KNIGHT 371
#define BISHOP 352
#define ROOK   519
#define QUEEN  983

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
// These numbers, and the five piece values above them, are fitted. Since S083
// they are its curve 0 candidate, `.tuning/s083/curve/s0/fit_6226944_s1.hpp`
// (sha256 f7ccdb7f...): tools/tuner over 6226944 qs-leaf rows of S082's pool's
// training side -- 137998 games of chesso's own self-play at 100000 nodes a
// move, 46 rows a game, not deduplicated -- minimising the squared error of
// sigmoid(K * score) against the result of the game each row came from, K =
// 0.7237 fitted from the data, tempo and piece placement frozen at 0.
// Held-out error on S082's fixed validation set 0.072864 before, 0.071838
// after (S083's step file, DEC-304). The queen was re-anchored before the
// paste, DEC-059's form: 273 moved from her 128 squares into QUEEN, 710 ->
// 983, by .tuning/reanchor.py, the same evaluation
// (`.tuning/coord/S083/reland/reanchor/fit_6226944_s1_q273.hpp`, DEC-307).
//
// Before S083: S028's fit first replaced a hand-written set chosen from
// ordinary positional principles (1490839 quiet positions from 20000 games,
// held-out error 0.113852 before, 0.108043 after); S065 refitted it on
// `.tuning/selfplay_v2.tsv` and S076 on that corpus deduplicated, and S134
// folded two columns into these tables bit-exactly (DEC-090).
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
//
// The pawn tables' seventh rank, squares 8..15, carries what passer bucket 5
// used to: S134 added that bucket's mg -17 and eg +42 to all sixteen entries
// and deleted the bucket, because a pawn on its own seventh is a passer by
// definition and the bucket was exactly the sum of those eight columns. Rook on
// the seventh was the same sum over the rook table's squares 8..15 and its
// weight, 0, folded into them the same way. The evaluation is identical, to the
// centipawn on every position: DEC-090.

// clang-format off
static constexpr int psqt_mg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     222,  158,  185,  171,  197,   39,   32,   82,
      12,   25,   43,   68,   74,   72,    8,   43,
     -20,   14,   11,   26,   31,   18,   25,  -24,
     -25,  -14,    9,   16,   21,    8,    0,  -23,
     -20,  -11,   -2,  -12,   -2,  -11,   32,  -10,
     -33,   -5,  -16,  -36,  -29,    3,   29,  -26,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
    -183,  -16, -130,  -36,   10, -225,   69,  -54,
     -33,  -10,   40,   62,   11,   72,  -61,   11,
      -7,   61,   43,  100,  108,  106,   75,   21,
      11,   23,   39,   54,   27,   70,   26,   43,
     -16,   -8,   25,   22,   38,   38,   39,   18,
     -25,    1,   11,   32,   34,   29,   31,  -12,
     -40,  -28,    3,   10,   14,   19,   10,   -8,
     -16,  -25,  -19,  -28,  -15,   -2,  -16, -113
  },
  {  // bishop
      51,  -60, -138, -109,  -57, -239,  -31,   89,
      24,   27,   21,   40,  -21,   58,   10,   30,
      17,   54,   51,   60,   75,   13,   80,   32,
      36,   55,   48,   80,   56,   46,   49,   38,
      67,   29,   54,   64,   62,   51,   37,   43,
      41,   71,   58,   51,   47,   67,   72,   50,
      56,   66,   58,   48,   59,   66,   80,   34,
      20,   41,   42,   17,   50,   36,   26,   34
  },
  {  // rook
      -6,   -2,  -48,  -15,  -18,    9,  -13,    6,
     -21,  -64,  -11,   28,   31,   12,  -70,   -8,
     -43,  -24,  -26,   10,   48,   16,    4,  -39,
     -64,  -60,  -43,  -39,  -19,  -20,  -63,  -60,
     -87,  -76,  -77,  -66,  -62,  -54,  -78,  -80,
     -85,  -77,  -71,  -77,  -50,  -54,  -69,  -79,
     -70,  -50,  -60,  -55,  -44,  -50,  -42, -118,
     -52,  -42,  -36,  -16,   -9,  -14,  -72,  -56
  },
  {  // queen
     341,  350,  370,  417,  401,  417,  424,  419,
     367,  334,  353,  357,  344,  402,  381,  426,
     363,  371,  366,  405,  415,  430,  407,  404,
     364,  369,  383,  371,  382,  385,  384,  405,
     394,  364,  378,  369,  389,  387,  381,  382,
     386,  400,  382,  388,  379,  403,  399,  389,
     395,  397,  420,  410,  420,  419,  431,  390,
     397,  380,  399,  413,  399,  354,  373,  417
  },
  {  // king
      86,  291,   -5,    6,   79,  243,  191,  661,
      89,   40,   58, -130,   87,  102,    6,  196,
      -6,    6,  -72, -127,  -59,   15,  -11, -219,
     -46,  -68,  -41,  -63,  -41,  -11,  -91, -173,
    -144,  -47,  -38,  -61, -107,  -50,  -54,  -99,
     -66,   13,  -51,  -47,  -44,  -37,    2,  -46,
      57,    7,    9,  -21,  -19,    3,   33,   24,
     -21,   70,   70,  -30,   47,    2,   59,   49
  }
};

static constexpr int psqt_eg[6][64] = {
  {  // pawn
       0,    0,    0,    0,    0,    0,    0,    0,
     190,  229,  215,  170,  161,  223,  262,  224,
      74,   62,   43,    7,   -2,   24,   57,   48,
      54,   37,   27,    8,   11,   26,   27,   41,
      41,   33,   17,   13,   15,   19,   22,   27,
      29,   20,   20,   28,   30,   33,    6,   15,
      37,   25,   35,   47,   39,   32,   15,   16,
       0,    0,    0,    0,    0,    0,    0,    0
  },
  {  // knight
     -91,  -35,    1,  -32,  -37,   26,  -48, -122,
     -70,  -46,  -25,  -10,   -8,  -19,  -19,  -49,
     -51,  -22,    5,  -17,  -24,  -35,  -22,  -40,
     -40,  -16,    5,    9,   29,   -6,    3,  -41,
     -37,  -23,    3,   14,    8,   -1,  -31,  -48,
     -67,  -29,  -18,   -3,  -16,  -29,  -43,  -64,
     -81,  -43,  -49,  -32,  -38,  -55,  -69,  -69,
    -138,  -99,  -44,  -44,  -35,  -68, -101,  -41
  },
  {  // bishop
       5,    3,   33,    6,   14,   59,   -6,  -24,
     -34,    2,   -8,   -9,   16,  -22,    0,  -44,
       3,  -12,   -7,   -6,  -15,   12,  -16,  -12,
     -24,   -3,   -4,  -16,   -7,   -1,   -8,    0,
     -43,   -3,   -8,  -12,  -10,  -12,   -8,  -20,
     -12,  -24,   -7,    2,    5,  -23,  -30,  -21,
     -42,  -33,  -22,  -21,  -23,  -28,  -28,  -45,
     -36,  -36,  -36,  -15,  -45,  -25,  -18,  -33
  },
  {  // rook
      95,  102,  125,  116,  110,   99,  115,  100,
     110,  131,  123,  105,  109,  106,  130,  106,
     105,  105,  110,   88,   73,   92,   97,  102,
     106,  106,  107,   96,   91,   94,  112,  100,
     104,  103,  110,  101,   93,   98,  104,   97,
     105,   95,   99,  102,   79,   88,   92,   91,
      84,   74,   77,   80,   75,   77,   74,   96,
      75,   74,   84,   68,   64,   67,   95,   59
  },
  {  // queen
     -68,  -63,  -63,  -97,  -68,  -81, -111, -127,
     -81,  -48,  -20,  -27,  -11,  -65,  -50,  -79,
     -75,  -44,  -43,  -59,  -75,  -49,  -38,  -40,
     -87,  -85,  -78,  -64,  -51,  -46,  -19,  -70,
    -156,  -80,  -95,  -46,  -75,  -55,  -67,  -55,
    -181, -176, -106, -123,  -91, -111, -107, -137,
    -181, -171, -200, -157, -170, -177, -234, -166,
    -183, -182, -179, -192, -175, -162, -215, -174
  },
  {  // king
     -28,   54,   91,  102,   71,   54,  110, -180,
      31,  115,  105,  165,   94,   89,  136,  -17,
      85,  112,  133,  131,  113,  117,  124,   95,
      60,  106,  109,  104,  100,  105,  117,   83,
      86,   81,   94,  105,  112,   94,   78,   63,
      41,   52,   81,   91,   88,   73,   50,   43,
       7,   39,   56,   64,   63,   50,   34,   14,
      -7,   -5,   10,   38,   -6,   16,    0,  -33
  }
};
// clang-format on


// clang-format on


// The two tables above packed into one, S117, which is what the accumulator
// hooks below read: one load and one add per piece where there were two. The
// plain tables stay the source of truth -- tools/tuner writes them and
// tools/tuner_layout.hpp reads them -- and this one is derived at compile time,
// so the two cannot disagree.
static constexpr std::array<std::array<score_t, 64>, 6> psqt_score = [] {
  std::array<std::array<score_t, 64>, 6> packed{};
  for (int type = 0; type < 6; ++type) {
    for (int square = 0; square < 64; ++square) {
      packed[type][square] =
          make_score(psqt_mg[type][square], psqt_eg[type][square]);
    }
  }
  return packed;
}();

// The packed sum is exact only while each half of it fits 16 bits. A position
// has at most 32 pieces, so 32 times the largest entry bounds either half of
// the board's sum whatever the position -- crude, and still 26912 at the
// shipped fit. A refit that breaks it fails here, at compile time, instead of
// wrapping one half into the other at run time.
static constexpr bool psqt_fits_packed = [] {
  int largest = 0;
  for (int type = 0; type < 6; ++type) {
    for (int square = 0; square < 64; ++square) {
      const int mg = psqt_mg[type][square];
      const int eg = psqt_eg[type][square];
      largest = std::max({largest, (mg < 0) ? -mg : mg, (eg < 0) ? -eg : eg});
    }
  }
  return 32 * largest <= 32767;
}();
static_assert(psqt_fits_packed,
              "a piece-square sum can overflow a packed score's 16-bit half");


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
    board->psqt += psqt_score[piece][square];
    board->phase += phase_value[piece];
  } else {
    const int type = piece - B_PAWN;
    const index_t mirrored = square ^ 56;

    board->material -= piece_value[type];
    board->psqt -= psqt_score[type][mirrored];
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
    board->psqt -= psqt_score[piece][square];
    board->phase -= phase_value[piece];
  } else {
    const int type = piece - B_PAWN;
    const index_t mirrored = square ^ 56;

    board->material += piece_value[type];
    board->psqt += psqt_score[type][mirrored];
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


// Rebuilds all three from the bitboards. Needed once when a position is loaded,
// and by the assertion that checks the incremental path has not drifted.
inline void eval_refresh(board_t* board)
{
  board->material = 0;
  board->psqt = 0;
  board->phase = 0;

  for (index_t square = 0; square < 64; ++square) {
    const piece_t piece = board->squares[square];
    if (piece != EMPTY) { eval_add_piece(board, piece, square); }
  }
}
