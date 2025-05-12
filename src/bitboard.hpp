#pragma once
#include <cstdint>
#include "data_structures.hpp"

void initialize_game_const_data(game_t* game);

// FEN
bool load_FEN(const std::string& FEN, game_t* game);
std::string generate_FEN(const board_t* board);

// Board manipulation
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
bool is_move_legal(game_t* game, move_t move);  // NOTE: only for debug

size_t generate_moves(const bb_tables_t* tables,
                      const board_t* board,
                      move_t moves[]);

bool make_move(game_t* game, move_t move);
void unmake_move(game_t* game);

// Utils
move_t fix_weirdo_castling(const board_t* board, move_t move);
std::string move_to_algebraic(game_t* game,
                              move_t encoded_move,
                              const move_t moves[],
                              size_t moves_size);
move_t algebraic_to_move(std::string notation, game_t* game);