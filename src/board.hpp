#pragma once
#include <string>
#include <vector>
#include "data_structures.hpp"


void init_board(const std::string& fen, board_t* board);
void reset(board_t* board);

void load_FEN(const std::string& FEN, board_t* board);
std::string generate_FEN(const board_t* board);

color_t us(const board_t* board);
color_t opponent(const board_t* board);
bool has_bishop_pair(color_t color, const board_t* board);
bool contains_opponent(index_t i, color_t opponent_color, const board_t* board);

index_t get_king_index(color_t color, const board_t* board);

piece_t remove_piece(index_t remove_at, board_t* board);
void put_piece(index_t put_at, piece_t piece, board_t* board);
piece_t move_piece(index_t from, index_t to, board_t* board);
void set_en_passant(index_t index, board_t* board);
void clear_ep_square(board_t* board);
void swap_side(board_t* board);
void update_castling_permissions(castling_t new_castling, board_t* board);

bool make_move(const move_t* move, board_t* board);
