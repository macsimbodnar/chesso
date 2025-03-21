#pragma once
#include "data_structures.hpp"

struct search_t
{
  move_t best_move;
  int score;
  uint64_t explored_nodes;
};


search_t search_best_move(int depth, const board_t* board);
