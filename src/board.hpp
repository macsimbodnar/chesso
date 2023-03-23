#pragma once

#include <inttypes.h>

#define u64 uint64_t
#define glob static
#define EMPTY_BB 0ULL
#define ONE_BIT 1ULL
#define get_bit(bboard, square) (bboard & (1ULL << square))
#define set_bit(bboard, square) (bboard |= (1ULL << square))
#define pop_bit(bboard, square) ((bboard) &= ~(1ULL << square))

/**
 *      NOT A FILE         NOT H FILE         NOT GH FILES       NOT AB FILES
 *  8   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  7   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  6   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  5   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  4   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  3   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  2   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *  1   0 1 1 1 1 1 1 1    1 1 1 1 1 1 1 0    1 1 1 1 1 1 0 0    0 0 1 1 1 1 1 1
 *
 *      A B C D E F G H    A B C D E F G H    A B C D E F G H    A B C D E F G H
 */
#define NOT_A_FILE 0xfefefefefefefefeULL
#define NOT_H_FILE 0x7f7f7f7f7f7f7f7fULL
#define NOT_GH_FILES 0x3f3f3f3f3f3f3f3fULL
#define NOT_AB_FILES 0xfcfcfcfcfcfcfcfcULL


namespace chesso
{

// clang-format off
enum square_t {
  A8, B8, C8, D8, E8, F8, G8, H8,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A1, B1, C1, D1, E1, F1, G1, H1
};

enum color_t { WHITE, BLACK };
// clang-format on


// Attack vectors
struct attack_vectors_t
{
  u64 pawn[2][64];  // [color][square]
  u64 knight[64];   // [square]
  u64 king[64];     // [square]
};

glob attack_vectors_t attack_vectors;
void init_attack_vectors();

// Pawns [side][squares]
u64 generate_mask_pawn_attacks(const color_t color, const square_t square);
u64 generate_mask_knight_attacks(const square_t square);
u64 generate_mask_king_attacks(const square_t square);

u64 generate_mask_bishop_attacks(const square_t square);
u64 generate_mask_rook_attacks(const square_t square);

u64 bishop_attacks(const square_t square, const u64 blocks);
u64 rook_attacks(const square_t square, const u64 blocks);


// Utils
void print_board(const u64 board);
void print_NOT_A_FILE();
void print_NOT_H_FILE();
void print_NOT_AB_FILE();
void print_NOT_GH_FILE();

}  // namespace chesso