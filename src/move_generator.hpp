#pragma once
#include "data_structures.hpp"


size_t generate_pseudo_legal_moves_from_index(index_t index,
                                              const board_t* board,
                                              move_t result[]);

size_t generate_legal_moves(const board_t* board, move_t result[]);

std::string move_to_algebraic(const move_t* move,
                              const move_t moves[],
                              size_t moves_size,
                              const board_t* board);

move_t algebraic_to_move(std::string notation, const board_t* board);

bool is_checkmate(const board_t* board);
