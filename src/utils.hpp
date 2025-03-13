#pragma once
#include <cstdint>
#include <string>
#include "data_structures.hpp"


index_t position_to_index(const uint8_t file, const uint8_t rank);
position_t index_to_position(index_t index);

index_t string_coordinates_to_index(const std::string& p);
std::string index_to_string_coordinates(const index_t i);

char piece_to_char(const piece_t piece);
piece_t char_to_piece(const char c);

std::string color_to_string(color_t color);

color_t get_square_color(index_t index);
color_t get_piece_color(piece_t piece);

std::string print_board(const board_t* board);
std::string print_nice_board(const board_t* board);
