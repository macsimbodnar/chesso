#pragma once
#include "data_structures.hpp"


size_t generate_pseudo_legal_moves_from_index(index_t index,
                                              const board_t* board,
                                              move_t result[]);

size_t generate_legal_moves(board_t* board,
                            global_state_t* state,
                            move_t result[]);

std::string move_to_algebraic(const move_t* move,
                              const move_t moves[],
                              size_t moves_size,
                              board_t* board,
                              global_state_t* state);

move_t algebraic_to_move(std::string notation,
                         board_t* board,
                         global_state_t* state);

bool is_check(board_t* board, const zobrist_randoms_t* rands);

size_t generate_captures(board_t* board,
                         global_state_t* state,
                         move_t result[]);
