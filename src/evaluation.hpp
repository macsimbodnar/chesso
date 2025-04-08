#pragma once
#include "data_structures.hpp"

int evaluate(const board_t* board);
int evaluate_move(const move_t* move,
                  size_t ply,
                  move_t killer_moves[2][MAX_PLY],
                  int history_moves[piece_t::EMPTY + 1][BOARD_SIZE]);
void order_moves(move_t moves[],
                 size_t moves_size,
                 size_t ply,
                 move_t killer_moves[2][MAX_PLY],
                 int history_moves[piece_t::EMPTY + 1][BOARD_SIZE]);