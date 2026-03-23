#pragma once
#include "data_structures.hpp"

int evaluate(const bb_tables_t* tables, const board_t* board);

void order_moves(const board_t* board,
                 const search_state_t* state,
                 size_t ply,
                 size_t moves_size,
                 move_t moves[]);

bool should_reduce_move(move_t move);

void order_captures(const board_t* board, move_t moves[], size_t moves_size);

int get_max_gain();
int get_margin_value();
int get_piece_value(piece_t piece);
int get_futility_margin();
int get_reverse_futility_margin();
int get_delta_margin();
