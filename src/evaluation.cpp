#include "evaluation.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"

// clang-format off

#define PAWN   1
#define KNIGHT 3
#define BISHOP 3
#define ROOK   5
#define QUEEN  9
#define KING   10000

/* board representation */

static const int piece_values[] = {
  PAWN, 
  KNIGHT, 
  BISHOP, 
  ROOK, 
  QUEEN, 
  KING, 
  -PAWN, 
  -KNIGHT, 
  -BISHOP, 
  -ROOK, 
  -QUEEN, 
  -KING,
  0
};

// clang-format on

int evaluate(const board_t* board)
{
  int score = 0;

  for (int pc = W_PAWN; pc < EMPTY; ++pc) {
    bb_t current_bb = board->bitboards[pc];

    int num_of_pieces = count_bits(current_bb);
    score += piece_values[pc] * num_of_pieces;
  }

  return score;
}
