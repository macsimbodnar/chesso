#pragma once

#include <inttypes.h>

#define u64 uint64_t
#define EMPTY_BB 0ULL
#define ONE_BIT 1ULL
#define get_bit(bboard, square) (bboard & (1ULL << square))
#define set_bit(bboard, square) (bboard |= (1ULL << square))
#define pop_bit(bboard, square) ((bboard) &= ~(1ULL << square))

static inline int count_bits(u64 board)
{
  int result = 0;

  while (board) {
    ++result;
    board &= board - 1;
  }

  return result;
}


static inline int get_lsb_index(u64 board)
{
  if (board) {
    const int res = count_bits((board & -board) - 1);

    return res;
  }

  return -1;
}


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


static inline constexpr char square_to_str[64][3] {
  "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
  "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
  "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
  "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
  "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
  "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
  "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
  "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1"
};

// Bishop relevant occupancy bit count for every square on board 
static inline constexpr int bishop_relevant_bits_count[64] = {
  6, 5, 5, 5, 5, 5, 5, 6, 
  5, 5, 5, 5, 5, 5, 5, 5, 
  5, 5, 7, 7, 7, 7, 5, 5, 
  5, 5, 7, 9, 9, 7, 5, 5, 
  5, 5, 7, 9, 9, 7, 5, 5, 
  5, 5, 7, 7, 7, 7, 5, 5, 
  5, 5, 5, 5, 5, 5, 5, 5, 
  6, 5, 5, 5, 5, 5, 5, 6
};

// Rook relevant occupancy bit count for every square on board 
static inline constexpr int rook_relevant_bits_count[64] = {
  12, 11, 11, 11, 11, 11, 11, 12, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  11, 10, 10, 10, 10, 10, 10, 11, 
  12, 11, 11, 11, 11, 11, 11, 12
};

static inline constexpr u64 rook_magic_numbers[64] = { 0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL, 0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL, 0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL, 0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL, 0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL, 0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL, 0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL, 0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL, 0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL, 0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL, 0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL, 0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL, 0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL, 0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL, 0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL, 0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL };
static inline constexpr u64 bishop_magic_numbers[64] = { 0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL, 0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL, 0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL, 0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL, 0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL, 0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL, 0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL, 0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL, 0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL, 0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL, 0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL, 0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL, 0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL, 0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL, 0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL, 0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL };

enum sliding_piece_t { ROOK, BISHOP };
enum color_t { WHITE, BLACK };
// clang-format on


// Attack vectors
struct attack_vectors_t
{
  u64 pawn[2][64];  // [color][square]
  u64 knight[64];   // [square]
  u64 king[64];     // [square]
};

static attack_vectors_t attack_vectors;
void init_attack_vectors();

u64 generate_mask_pawn_attacks(const color_t color, const square_t square);
u64 generate_mask_knight_attacks(const square_t square);
u64 generate_mask_king_attacks(const square_t square);

u64 generate_mask_bishop_attacks(const square_t square);
u64 generate_mask_rook_attacks(const square_t square);

u64 bishop_attacks(const square_t square, const u64 blocks);
u64 rook_attacks(const square_t square, const u64 blocks);
u64 set_occupancy(const int index,
                  const int mask_bit_count,
                  const u64 attack_mask);

u64 generate_magic_number_candidate();
u64 find_magic_number(const square_t square,
                      const int num_of_relevant_bits,
                      const sliding_piece_t piece);
void print_magic_numbers();


// Utils
void print_board(const u64 board);
void print_NOT_A_FILE();
void print_NOT_H_FILE();
void print_NOT_AB_FILE();
void print_NOT_GH_FILE();

}  // namespace chesso