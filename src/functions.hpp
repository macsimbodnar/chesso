#pragma once
#include <string>
#include <vector>
#include "data_structures.hpp"


void init_board(const std::string& fen, board_t* board);
void reset(board_t* board);

color_t us(const board_t* board);
color_t opponent(const board_t* board);
std::array<piece_t, BOARD_SIZE> get_chess_board(const board_t* board);
position_t king_square(color_t color, const board_t* board);
bool has_bishop_pair(color_t color, const board_t* board);

std::string move_to_algebraic(const move_t* move,
                              const std::vector<move_t>* moves,
                              const board_t* board);

bool make_move(const move_t* move, board_t* board);
