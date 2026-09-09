#pragma once
#include <bit>
#include <cassert>
#include <cstdint>
#include "bb_tables.hpp"
#include "data_structures.hpp"

void initialize_game_const_data(game_t* game);

// The attack tables, which are read-only once built and the same for every
// game. initialize_game_const_data() fills them on the first call and does
// nothing on later ones, so calling it per game_t is cheap.
const bb_tables_t* game_tables();

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

// The same list, split in two. Captures carries every capture, en passant and
// every promotion; quiets carries everything else, including castling. The two
// partition generate_moves() exactly: concatenated they are the same set, and
// no move appears in both.
size_t generate_captures(const bb_tables_t* tables,
                         const board_t* board,
                         move_t moves[]);

size_t generate_quiets(const bb_tables_t* tables,
                       const board_t* board,
                       move_t moves[]);

bool make_move(game_t* game, move_t move);
void unmake_move(game_t* game);

// The two INV-2 / INV-4 oracles: rebuild the evaluation accumulators, and
// squares[], from scratch and compare against what make_move/unmake_move
// maintained incrementally. The search calls neither, in any build -- they are
// asserted inside make_move_impl and unmake_move_impl in Debug, and called by
// tests/test_invariants.cpp in every build (S190). Both are O(64) per call and
// eval_accumulators_match() copies the board, so neither belongs on a hot path.
bool eval_accumulators_match(const board_t* board);
bool squares_match_bitboards(const board_t* board);

// Pass the turn without moving, for null move pruning. Must be undone with
// unmake_null_move(), not unmake_move(): the history entry it pushes carries no
// move to reverse.
void make_null_move(game_t* game);
void unmake_null_move(game_t* game);
bool is_capturing_king(const board_t* board, move_t move);

// Utils that can be slow
move_t fix_weirdo_castling(const board_t* board, move_t move);
void fix_weirdo_castling(const board_t* board, unpacked_move_t* move);
std::string move_to_algebraic(game_t* game,
                              move_t encoded_move,
                              const move_t moves[],
                              size_t moves_size);
// SAN to a legal move in the current position, or 0 when the token does not
// parse or matches no legal move. Trailing `+`, `#`, `!` and `?` are ignored.
move_t algebraic_to_move(std::string notation, game_t* game);
bool is_pv_legal(game_t* game, const pv_t* pv);

// Utils that must run fast
bool is_position_repeated(const history_t* history, const board_t* board);

// True when neither side can force mate with what is left on the board.
bool is_insufficient_material(const board_t* board);

// Static exchange evaluation: what a capture is worth in material once both
// sides have finished taking on the target square, cheapest piece first.
// Negative means the exchange loses material for the side to move.
int see(const board_t* board, move_t move);

// Whether the exchange is worth at least `threshold`, which is all any caller
// in the search actually asks. Stops as soon as the comparison is settled
// instead of resolving the sequence, so it is much cheaper than see().
bool see_ge(const board_t* board, move_t move, int threshold);

// True when a capture cannot lose material without working the exchange out:
// the victim is worth at least as much as the piece taking it. Answers most
// captures worth making without touching an attack table.
bool capture_cannot_lose(const board_t* board, move_t move);

bool is_check(const game_t* game);
void swap_side(game_t* game);
void set_en_passant(game_t* game, index_t ep_index);
