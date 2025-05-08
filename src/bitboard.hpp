#pragma once
#include <cstdint>
#include "data_structures.hpp"

// FEN
bool load_FEN(const std::string& FEN, board_t* board);
std::string generate_FEN(const board_t* board);

// Board manipulation
void cleanup_board(board_t* board);
piece_t get_piece(const board_t* board, index_t square);

// Attacks
bb_t get_bishop_attacks(const bb_tables_t* data, index_t index, bb_t occupancy);
bb_t get_rook_attacks(const bb_tables_t* data, index_t index, bb_t occupancy);
bb_t get_queen_attacks(const bb_tables_t* data, index_t index, bb_t occupancy);
bool is_attacked(const bb_tables_t* data,
                 const board_t* board,
                 index_t index,
                 color_t color);