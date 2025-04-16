#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>


// clang-format off
#define VALUE_W_PAWN    100
#define VALUE_W_KNIGHT  300
#define VALUE_W_BISHOP  350
#define VALUE_W_ROOK    525
#define VALUE_W_QUEEN   1000
#define VALUE_W_KING    10000
#define VALUE_B_PAWN    -VALUE_W_PAWN
#define VALUE_B_KNIGHT  -VALUE_W_KNIGHT
#define VALUE_B_BISHOP  -VALUE_W_BISHOP
#define VALUE_B_ROOK    -VALUE_W_ROOK
#define VALUE_B_QUEEN   -VALUE_W_QUEEN
#define VALUE_B_KING    -VALUE_W_KING


// static const std::array<int, BOARD_SIZE> debug_postion_value_table = {
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0, 10,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0
// };

static const int pawn_postion_value_table[BOARD_SIZE] = {
  90, 90, 90, 90, 90, 90, 90, 90,         0,  0,  0,  0,  0,  0,  0,  0,
  50, 50, 50, 50, 50, 50, 50, 50,         0,  0,  0,  0,  0,  0,  0,  0,
  10, 10, 20, 30, 30, 20, 10, 10,         0,  0,  0,  0,  0,  0,  0,  0,
   5,  5, 10, 25, 25, 10,  5,  5,         0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0, 20, 20,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
   5, -5,-10,  0,  0,-10, -5,  5,         0,  0,  0,  0,  0,  0,  0,  0,
   5, 10, 10,-20,-20, 10, 10,  5,         0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0
};

static const int knight_postion_value_table[BOARD_SIZE] = {
 -50,-40,-30,-30,-30,-30,-40,-50,         0,  0,  0,  0,  0,  0,  0,  0,
 -40,-20,  0,  0,  0,  0,-20,-40,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,  0, 10, 15, 15, 10,  0,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,  5, 15, 20, 20, 15,  5,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,  0, 15, 20, 20, 15,  0,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,  5, 10, 15, 15, 10,  5,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -40,-20,  0,  5,  5,  0,-20,-40,         0,  0,  0,  0,  0,  0,  0,  0,
 -50,-40,-30,-30,-30,-30,-40,-50,         0,  0,  0,  0,  0,  0,  0,  0
};

static const int bishop_postion_value_table[BOARD_SIZE] = {
 -20,-10,-10,-10,-10,-10,-10,-20,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0,  0,  0,  0,  0,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0,  5, 10, 10,  5,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  5,  5, 10, 10,  5,  5,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0, 10, 10, 10, 10,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10, 10, 10, 10, 10, 10, 10,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  5,  0,  0,  0,  0,  5,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -20,-10,-10,-10,-10,-10,-10,-20,         0,  0,  0,  0,  0,  0,  0,  0
};

static const int rook_postion_value_table[BOARD_SIZE] = {
   0,  0,  0,  0,  0,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0,
   5, 10, 10, 10, 10, 10, 10,  5,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  0,  0,  0,  0,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  5,  5,  0,  0,  0,         0,  0,  0,  0,  0,  0,  0,  0
};

static const int queen_postion_value_table[BOARD_SIZE] = {
 -20,-10,-10, -5, -5,-10,-10,-20,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0,  0,  0,  0,  0,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0,  5,  5,  5,  5,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
  -5,  0,  5,  5,  5,  5,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  5,  5,  5,  5,  0, -5,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  5,  5,  5,  5,  5,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,  0,  5,  0,  0,  0,  0,-10,         0,  0,  0,  0,  0,  0,  0,  0,
 -20,-10,-10, -5, -5,-10,-10,-20,         0,  0,  0,  0,  0,  0,  0,  0
};

static const int king_postion_value_table[BOARD_SIZE] = {
 -30,-40,-40,-50,-50,-40,-40,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,-40,-40,-50,-50,-40,-40,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,-40,-40,-50,-50,-40,-40,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -30,-40,-40,-50,-50,-40,-40,-30,         0,  0,  0,  0,  0,  0,  0,  0,
 -20,-30,-30,-40,-40,-30,-30,-20,         0,  0,  0,  0,  0,  0,  0,  0,
 -10,-20,-20,-20,-20,-20,-20,-10,         0,  0,  0,  0,  0,  0,  0,  0,
  20, 20,  0,  0,  0,  0, 20, 20,         0,  0,  0,  0,  0,  0,  0,  0,
  20, 30, 10,  0,  0, 10, 30, 20,         0,  0,  0,  0,  0,  0,  0,  0
 };

static const int white_indexes[BOARD_SIZE] = {
  0x70,  0x71,  0x72,  0x73,  0x74,  0x75,  0x76,  0x77,  0x78,  0x79,  0x7A,  0x7B,  0x7C,  0x7D,  0x7E,  0x7F,
  0x60,  0x61,  0x62,  0x63,  0x64,  0x65,  0x66,  0x67,  0x68,  0x69,  0x6A,  0x6B,  0x6C,  0x6D,  0x6E,  0x6F,
  0x50,  0x51,  0x52,  0x53,  0x54,  0x55,  0x56,  0x57,  0x58,  0x59,  0x5A,  0x5B,  0x5C,  0x5D,  0x5E,  0x5F,
  0x40,  0x41,  0x42,  0x43,  0x44,  0x45,  0x46,  0x47,  0x48,  0x49,  0x4A,  0x4B,  0x4C,  0x4D,  0x4E,  0x4F,
  0x30,  0x31,  0x32,  0x33,  0x34,  0x35,  0x36,  0x37,  0x38,  0x39,  0x3A,  0x3B,  0x3C,  0x3D,  0x3E,  0x3F,
  0x20,  0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x27,  0x28,  0x29,  0x2A,  0x2B,  0x2C,  0x2D,  0x2E,  0x2F,
  0x10,  0x11,  0x12,  0x13,  0x14,  0x15,  0x16,  0x17,  0x18,  0x19,  0x1A,  0x1B,  0x1C,  0x1D,  0x1E,  0x1F,
  0x00,  0x01,  0x02,  0x03,  0x04,  0x05,  0x06,  0x07,  0x08,  0x09,  0x0A,  0x0B,  0x0C,  0x0D,  0x0E,  0x0F,
};


// Most valuable victim & less valuable attacker
/**
 *   (Victims) Pawn Knight Bishop   Rook  Queen   King
 * (Attackers)
 *       Pawn   105    205    305    405    505    605
 *     Knight   104    204    304    404    504    604
 *     Bishop   103    203    303    403    503    603
 *       Rook   102    202    302    402    502    602
 *      Queen   101    201    301    401    501    601
 *       King   100    200    300    400    500    600
*/

// [attacker][victim]
static const int mvv_lva[12][12] = {
 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

 {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
 {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
 {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
 {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
 {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
 {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600}
};

// clang-format on


int evaluate(const board_t* board)
{
  assert(board != nullptr);

  int evaluation = 0;

  for (index_t index = 0; index < BOARD_SIZE; ++index) {
    const piece_t piece = board->board[index];

    if (piece != INVALID && piece != EMPTY) {
      switch (piece) {
        case B_PAWN:
          evaluation += VALUE_B_PAWN;
          evaluation -= pawn_postion_value_table[index];
          // evaluation -= debug_postion_value_table[index];
          break;
        case B_KNIGHT:
          evaluation += VALUE_B_KNIGHT;
          evaluation -= knight_postion_value_table[index];
          break;
        case B_BISHOP:
          evaluation += VALUE_B_BISHOP;
          evaluation -= bishop_postion_value_table[index];
          break;
        case B_ROOK:
          evaluation += VALUE_B_ROOK;
          evaluation -= rook_postion_value_table[index];
          break;
        case B_QUEEN:
          evaluation += VALUE_B_QUEEN;
          evaluation -= queen_postion_value_table[index];
          break;
        case B_KING:
          evaluation += VALUE_B_KING;
          evaluation -= king_postion_value_table[index];
          break;
        case W_PAWN:
          evaluation += VALUE_W_PAWN;
          evaluation += pawn_postion_value_table[white_indexes[index]];
          // index_t mapping = white_indexes[index];
          // evaluation += debug_postion_value_table[mapping];
          break;
        case W_KNIGHT:
          evaluation += VALUE_W_KNIGHT;
          evaluation += knight_postion_value_table[white_indexes[index]];
          break;
        case W_BISHOP:
          evaluation += VALUE_W_BISHOP;
          evaluation += bishop_postion_value_table[white_indexes[index]];
          break;
        case W_ROOK:
          evaluation += VALUE_W_ROOK;
          evaluation += rook_postion_value_table[white_indexes[index]];
          break;
        case W_QUEEN:
          evaluation += VALUE_W_QUEEN;
          evaluation += queen_postion_value_table[white_indexes[index]];
          break;
        case W_KING:
          evaluation += VALUE_W_KING;
          evaluation += king_postion_value_table[white_indexes[index]];
          break;
        case INVALID:
        case EMPTY:
        default:
          assert(false);
          break;
      }
    }
  }

  return evaluation;
}


int evaluate_move(const move_t* move, size_t ply, search_state_t* state)
{
  assert(move != nullptr);
  assert(state != nullptr);

  if (move->captured != INVALID) {
    const piece_t attacker = move->piece;
    const piece_t victim = move->captured;

    assert(attacker != INVALID);
    assert(attacker != EMPTY);
    assert(victim != INVALID);
    assert(victim != EMPTY);

    const int score = mvv_lva[attacker][victim];
    return 10000 + score;
  } else if (move->promoted_to != TO_NONE) {
    switch (move->promoted_to) {
      case TO_QUEEN:
        return 10000 + 500;
      case TO_ROOK:
        return 10000 + 350;
      case TO_KNIGHT:
      case TO_BISHOP:
        return 10000 + 100;

      case TO_NONE:
      default:
        return 0;
    }
  } else {
    if (*move == state->killer_moves[0][ply]) {
      return 10000 - 1000;
    } else if (*move == state->killer_moves[1][ply]) {
      return 10000 - 2000;
    } else {
      return state->history_moves[move->piece][move->to];
    }
  }

  return 0;
}


void order_moves(move_t moves[],
                 size_t moves_size,
                 size_t ply,
                 search_state_t* state)
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);
  assert(state != nullptr);
  assert(state->killer_moves[0] != nullptr);
  assert(state->killer_moves[1] != nullptr);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    scores[i] = evaluate_move(&moves[i], ply, state);
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }


  // Selection Sort
  // for (size_t i = 0; i < moves_size; ++i) {
  //   size_t best_index = i;

  //   for (size_t candidate = i; candidate < moves_size; ++candidate) {
  //     if (scores[candidate] > scores[best_index]) { best_index = candidate; }
  //   }

  //   // Swapping elements
  //   const int old_score = scores[i];
  //   const move_t old_move = moves[i];

  //   scores[i] = scores[best_index];
  //   moves[i] = moves[best_index];

  //   scores[best_index] = old_score;
  //   moves[best_index] = old_move;
  // }
}
