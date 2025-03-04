#pragma once
#include <vector>

#include "data_structures.hpp"

std::vector<index_t> generate_pseudo_legal_moves_for_piece(
    index_t index,
    const board_t* board);