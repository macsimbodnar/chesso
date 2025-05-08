#pragma once
#include <cstdint>
#include "data_structures.hpp"


bool load_FEN(const std::string& FEN, board_t* board);
std::string generate_FEN(const board_t* board);

void cleanup_board(board_t *board);
piece_t get_piece(const board_t* board, index_t square);