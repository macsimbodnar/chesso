#pragma once
#include <cstdint>
#include "data_structures.hpp"

// FEN
bool load_FEN(const std::string& FEN, board_t* board, history_t* history);
std::string generate_FEN(const board_t* board);

// Board manipulation
void cleanup_board(board_t* board, history_t* history);
piece_t get_piece(const board_t* board, index_t square);

// Attacks
bb_t get_bishop_attacks(const bb_tables_t* tables,
                        index_t index,
                        bb_t occupancy);
bb_t get_rook_attacks(const bb_tables_t* tables, index_t index, bb_t occupancy);
bb_t get_queen_attacks(const bb_tables_t* tables,
                       index_t index,
                       bb_t occupancy);
bool is_attacked(const bb_tables_t* tables,
                 const board_t* board,
                 index_t index,
                 color_t color);

// Moves
size_t generate_moves(const bb_tables_t* tables,
                      const board_t* board,
                      move_t moves[]);

bool make_move(game_t* game, move_t move);

// std::string move_to_algebraic(const move_t* move,
//                               const move_t moves[],
//                               size_t moves_size,
//                               board_t* board);


// move_t algebraic_to_move(std::string notation, board_t* board);
