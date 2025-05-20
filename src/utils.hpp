#pragma once
#include <string>
#include <vector>
#include "data_structures.hpp"


bool is_uint(const std::string& str);
std::vector<std::string> split_string(const std::string& str);

// Coordinates
std::string index_to_str(index_t index);
index_t position_to_index(uint8_t file, uint8_t rank);
position_t index_to_position(index_t index);
inline uint8_t index_to_file(index_t index)
{
  const uint8_t file = index % 8;
  return file;
}
index_t str_to_index(const std::string& str);

// Pieces
piece_t char_to_piece(char c);
std::string piece_to_str(piece_t piece);
std::string promotion_to_str(promotion_t piece);

// Board
std::string print_bboard(bb_t board);
std::string print_nice_board(const board_t* board);

// Moves
std::string print_move(move_t move);
