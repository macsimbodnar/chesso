#pragma once
#include <string>
#include <vector>
#include "data_structures.hpp"

struct piece_count_t
{
  int white = 0;
  int black = 0;
};


void init_board(const std::string& fen, board_t* board, global_state_t* state);
void reset(board_t* board, global_state_t* state);

void load_FEN(const std::string& FEN, board_t* board, global_state_t* state);
std::string generate_FEN(const board_t* board);

bool contains_opponent(index_t i, color_t opponent_color, const board_t* board);

index_t get_king_index(color_t color, const board_t* board);

piece_t remove_piece(index_t remove_at,
                     board_t* board,
                     const zobrist_randoms_t* randoms);

void put_piece(index_t put_at,
               piece_t piece,
               board_t* board,
               const zobrist_randoms_t* randoms);

piece_t move_piece(index_t from,
                   index_t to,
                   board_t* board,
                   const zobrist_randoms_t* randoms);

void set_en_passant(index_t index, board_t* board, const global_state_t* state);
void clear_ep_square(board_t* board, const global_state_t* state);
void swap_side(board_t* board, const global_state_t* state);
void update_castling_permissions(castling_t new_castling,
                                 board_t* board,
                                 const global_state_t* state);

bool make_move(const move_t* move, board_t* board, global_state_t* state);
bool unmake_move(board_t* board, global_state_t* state);

bool is_position_repeated(const board_t* board, const global_state_t* state);

bool has_bishop_pair(color_t color, const board_t* board);
bool is_double_pawn(index_t index, const board_t* board);
bool is_passed_pawn(index_t index, const board_t* board);
bool is_isolated_pawn(index_t index, const board_t* board);
piece_count_t count_pieces_on_file(index_t index, const board_t* board);
bool is_king_shielded(index_t index, const board_t* board);

bool is_square_attacked(index_t index, color_t color, const board_t* board);
