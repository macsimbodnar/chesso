#pragma once
#include "data_structures.hpp"
#include <cstdint>
#include <string>
#include <vector>


void init_zobrist(zobrist_randoms_t* board);
uint64_t init_zobrist_key(const board_t* board);
void cleanup_game_state(game_state_t *game_state);

void load_FEN(const std::string& FEN, board_t* board);

uint8_t position_to_index(const uint8_t file, const uint8_t rank);
position_t index_to_position(uint8_t index);
uint8_t algebraic_to_index(const std::string& p);
std::string index_to_algebraic(const uint8_t i);
char piece_to_char(const piece_t piece);
piece_t char_to_piece(const char c);


bool is_uint(const std::string& str);
std::vector<std::string> split_string(const std::string& str);


std::string print_board(const board_t* board);
std::string print_nice_board(const board_t* board);
