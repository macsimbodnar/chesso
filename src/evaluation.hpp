#pragma once
#include "data_structures.hpp"

int evaluate(const board_t* board);
int evaluate_move(const move_t* move, size_t ply, search_state_t* state);
void order_moves(move_t moves[],
                 size_t moves_size,
                 size_t ply,
                 const search_state_t* state);

bool should_reduce_move(const move_t* move);
