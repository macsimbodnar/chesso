#pragma once
#include <bit>
#include <cassert>
#include <cstdint>
#include "bb_tables.hpp"
#include "data_structures.hpp"

void initialize_game_const_data(game_t* game);

// FEN
bool load_FEN(const std::string& FEN, game_t* game);
std::string generate_FEN(const board_t* board);

// Board manipulation
piece_t get_piece(const board_t* board, index_t square);


inline int count_bits(bb_t board)
{ return std::popcount(board); }


inline index_t get_lsb_index(bb_t board)
{
  // If 64 then invalid
  return static_cast<index_t>(std::countr_zero(board));
}


// Attacks
inline bb_t get_bishop_attacks(const bb_tables_t* tables,
                               index_t index,
                               bb_t occupancy)
{
  assert(tables != nullptr);
  assert(index < 64);

  occupancy &= tables->bishop_masks[index];
  occupancy *= bishop_magic_numbers[index];
  occupancy >>= 64 - bishop_relevant_bits_count[index];
  return tables->bishop_attacks[index][occupancy];
}


inline bb_t get_rook_attacks(const bb_tables_t* tables,
                             index_t index,
                             bb_t occupancy)
{
  assert(tables != nullptr);
  assert(index < 64);

  occupancy &= tables->rook_masks[index];
  occupancy *= rook_magic_numbers[index];
  occupancy >>= 64 - rook_relevant_bits_count[index];
  return tables->rook_attacks[index][occupancy];
}


inline bb_t get_queen_attacks(const bb_tables_t* tables,
                              index_t index,
                              bb_t occupancy)
{
  return get_bishop_attacks(tables, index, occupancy) |
         get_rook_attacks(tables, index, occupancy);
}


bool is_attacked(const bb_tables_t* tables,
                 const board_t* board,
                 index_t index,
                 color_t color);

// Moves
bool is_move_legal(game_t* game, move_t move);  // NOTE: only for debug

bool move_belongs_to_side_to_move(const board_t* board, move_t move);

// Legal moves only: every move returned can be played and none is missing.
size_t generate_moves(const bb_tables_t* tables,
                      const board_t* board,
                      move_t moves[]);

bool make_move(game_t* game, move_t move);
void unmake_move(game_t* game);
bool is_capturing_king(const board_t* board, move_t move);

// Utils that can be slow
move_t fix_weirdo_castling(const board_t* board, move_t move);
void fix_weirdo_castling(const board_t* board, unpacked_move_t* move);
std::string move_to_algebraic(game_t* game,
                              move_t encoded_move,
                              const move_t moves[],
                              size_t moves_size);
move_t algebraic_to_move(std::string notation, game_t* game);
bool is_pv_legal(game_t* game, const pv_t* pv);

// Utils that must run fast
bool is_position_repeated(const repetition_t* rep, const board_t* board);

bool is_check(const game_t* game);
void swap_side(game_t* game);
void set_en_passant(game_t* game, index_t ep_index);
