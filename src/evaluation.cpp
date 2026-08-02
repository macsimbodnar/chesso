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
