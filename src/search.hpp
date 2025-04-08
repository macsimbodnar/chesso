#pragma once
#include "data_structures.hpp"

struct pv_t
{
  size_t pv_length[MAX_PLY];
  move_t pv_table[MAX_PLY][MAX_PLY];
};

struct search_t
{
  move_t best_move;
  int score;
  uint64_t explored_nodes;
  pv_t pv;
};


search_t search_best_move(int depth, const board_t* board);
