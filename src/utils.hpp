#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "data_structures.hpp"


void init_zobrist(zobrist_randoms_t* board);
uint64_t init_zobrist_key(const board_t* board);
void cleanup_game_state(game_state_t* game_state);

void load_FEN(const std::string& FEN, board_t* board);
std::string generate_FEN(const board_t* board);

index_t position_to_index(const uint8_t file, const uint8_t rank);
position_t index_to_position(index_t index);
index_t algebraic_to_index(const std::string& p);
std::string index_to_algebraic(const index_t i);
char piece_to_char(const piece_t piece);
piece_t char_to_piece(const char c);
std::string color_to_string(color_t color);
color_t get_square_color(index_t index);
color_t get_piece_color(piece_t piece);


bool is_uint(const std::string& str);
std::vector<std::string> split_string(const std::string& str);


std::string print_board(const board_t* board);
std::string print_nice_board(const board_t* board);

piece_t remove_piece(index_t remove_at, board_t* board);
void put_piece(index_t put_at, piece_t piece, board_t* board);
piece_t move_piece(index_t from, index_t to, board_t* board);
void set_en_passant(index_t index, board_t* board);
void clear_ep_square(board_t* board);
void swap_side(board_t* board);
void update_castling_permissions(castling_t new_castling, board_t* board);
