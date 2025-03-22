#pragma once
#include "data_structures.hpp"

int evaluate(const board_t* board);
void order_moves(move_t moves[], size_t moves_size);
int evaluate_move(const move_t* move);