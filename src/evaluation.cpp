#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"

// clang-format off

#define PAWN   100
#define KNIGHT 300
#define BISHOP 300
#define ROOK   500
#define QUEEN  900
#define KING   100000

#define ORDER_TT_MOVE 2000000
#define ORDER_CAPTURE 1000000
#define ORDER_KILLER_0 900000
#define ORDER_KILLER_1 800000
#define ORDER_COUNTER 700000

/* board representation */

// Material, as evaluate() sees it. The king carries no value: both sides
// always have exactly one in a legal position, so it can only cancel, and
// pricing it meant an illegal position with an unbalanced king count produced a
// score larger than any mate and had to be guarded against everywhere it
// reached. Move ordering keeps its own table below, where the king does need a
// price.
static constexpr int piece_values[] = {
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  0,
  -PAWN,
  -KNIGHT,
  -BISHOP,
  -ROOK,
  -QUEEN,
  0,
  0
};

// Phase weights, indexed by piece. Queens dominate, pawns and kings contribute
// nothing. A full set on both sides comes to 24.
static constexpr int phase_values[] = {
  0, 1, 1, 2, 4, 0,
  0, 1, 1, 2, 4, 0,
  0
};


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


static constexpr int piece_values_abs[] = {PAWN,  KNIGHT, BISHOP, ROOK,   QUEEN,
                                           KING,  PAWN,   KNIGHT, BISHOP, ROOK,
                                           QUEEN, KING,   0};


// clang-format on

// Piece values are written from White's point of view, so the sum is
// White-relative and is negated once at the end for Black. Doing it here rather
// than at every call site is what keeps a later term from picking up the wrong
// sign.
int evaluate(const board_t* board)
{
  int material = 0;
  int middlegame = 0;
  int endgame = 0;

  for (int pc = W_PAWN; pc <= W_KING; ++pc) {
    assert((unsigned long)pc < sizeof(piece_values) / sizeof(piece_values[0]));

    bb_t white = board->bitboards[pc];
    bb_t black = board->bitboards[pc + B_PAWN];

    material += piece_values[pc] * count_bits(white);
    material += piece_values[pc + B_PAWN] * count_bits(black);

    while (white) {
      const index_t square = get_lsb_index(white);
      white &= white - 1;

      middlegame += psqt_mg[pc][square];
      endgame += psqt_eg[pc][square];
    }

    // A Black piece is worth what the same White piece would be worth on the
    // vertically mirrored square, and xor 56 is that mirror: it flips the rank
    // bits of the index and leaves the file alone.
    while (black) {
      const index_t square = get_lsb_index(black);
      black &= black - 1;

      middlegame -= psqt_mg[pc][square ^ 56];
      endgame -= psqt_eg[pc][square ^ 56];
    }
  }

  // Interpolate the two tables on how much material is left, so a term slides
  // from its middlegame value to its endgame one instead of jumping when some
  // arbitrary piece comes off.
  const int phase = game_phase(board);
  const int positional =
      ((middlegame * phase) + (endgame * (GAME_PHASE_MAX - phase))) /
      GAME_PHASE_MAX;

  const int score = material + positional;

  return (board->active_color == WHITE) ? score : -score;
}


int game_phase(const board_t* board)
{
  int phase = 0;

  for (int pc = W_PAWN; pc < EMPTY; ++pc) {
    phase += phase_values[pc] * count_bits(board->bitboards[pc]);
  }

  // Promotions can put more material on the board than the opening had.
  return (phase > GAME_PHASE_MAX) ? GAME_PHASE_MAX : phase;
}


inline piece_t captured_piece(const board_t* board, index_t square)
{
  assert(square < 64);
  return board->squares[square];
}


int capture_score(const board_t* board, move_t move)
{
  // An en-passant victim does not sit on the target square
  const piece_t victim =
      MOVE_EN_PASSANT(move) ? ((board->active_color == WHITE) ? B_PAWN : W_PAWN)
                            : captured_piece(board, MOVE_TO(move));

  // MVV-LVA
  assert(victim < sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));
  return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];
}


int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move)
{
  if (tt_move != 0 && move == tt_move) { return ORDER_TT_MOVE; }

  assert(static_cast<size_t>(MOVE_PROMOTED(move)) <
         sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));

  // Promotion_t maps onto W_KNIGHT..W_QUEEN by construction.
  const int promotion_bonus =
      MOVE_PROMOTED(move) ? piece_values_abs[MOVE_PROMOTED(move)] : 0;

  if (MOVE_CAPTURE(move)) {
    return ORDER_CAPTURE + promotion_bonus + capture_score(&game->board, move);
  }

  if (promotion_bonus != 0) { return ORDER_CAPTURE + promotion_bonus; }

  if (move == state->killer_moves[0][ply]) { return ORDER_KILLER_0; }
  if (move == state->killer_moves[1][ply]) { return ORDER_KILLER_1; }

  if (prev_move != 0 &&
      move == state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]) {
    return ORDER_COUNTER;
  }

  // Saturated on the way in, so it can never reach the killer band.
  return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
}
