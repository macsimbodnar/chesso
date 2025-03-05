#pragma once
#include <vector>

#include "data_structures.hpp"

std::vector<move_t> generate_pseudo_legal_moves_from_index(
    index_t index,
    const board_t* board);

std::vector<move_t> generate_legal_moves(const board_t* board);
