#include "board.hpp"
#include <cassert>
#include <cstring>
#include "log.hpp"


static uint32_t seed = 1804289383;

inline static uint32_t get_32bit_random_number()
{
  uint32_t result = seed;

  result ^= result << 13;
  result ^= result >> 17;
  result ^= result << 5;

  seed = result;

  return result;
}


static u64 get_random_number()
{
  // Get a random number and take only the 16 LSBs
  const u64 n1 = static_cast<u64>(get_32bit_random_number()) & 0xFFFF;
  const u64 n2 = static_cast<u64>(get_32bit_random_number()) & 0xFFFF;
  const u64 n3 = static_cast<u64>(get_32bit_random_number()) & 0xFFFF;
  const u64 n4 = static_cast<u64>(get_32bit_random_number()) & 0xFFFF;

  // We compose the result taking the the 16 LSBs from the 4 32 numbers and glue
  // them. This is done because we need a high density of 1s in the 64 bit
  // number.
  const u64 result = n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);

  return result;
}


namespace chesso
{


u64 generate_magic_number_candidate()
{
  const u64 result =
      get_random_number() & get_random_number() & get_random_number();

  return result;
}


#define MAX_SIZE 4096
u64 find_magic_number(const square_t square,
                      const int num_of_relevant_bits,
                      const sliding_piece_t piece)
{
  u64 occupancies[MAX_SIZE];
  u64 attacks[MAX_SIZE];
  u64 used_attacks[MAX_SIZE];

  // Get the attack mask for the current piece
  const u64 attack_mask = (piece == BISHOP)
                              ? generate_mask_bishop_attacks(square)
                              : generate_mask_rook_attacks(square);

  const int occupancy_indicies = 1 << num_of_relevant_bits;
  assert(occupancy_indicies <= MAX_SIZE);

  // Loop over the occupancy indicies and init the tables
  for (int i = 0; i < occupancy_indicies; ++i) {
    occupancies[i] = set_occupancy(i, num_of_relevant_bits, attack_mask);
    attacks[i] = (piece == BISHOP) ? bishop_attacks(square, occupancies[i])
                                   : rook_attacks(square, occupancies[i]);
  }

  // Test the magic number
  for (int attempt = 0; attempt < 100000000; ++attempt) {
    const u64 magic_number = generate_magic_number_candidate();

    // Skip non valid magic numbers
    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) {
      continue;
    }

    // Reset the used attacks array
    memset(used_attacks, EMPTY_BB, sizeof(used_attacks));

    int i;
    bool fails;
    for (i = 0, fails = false; !fails && i < occupancy_indicies; ++i) {
      // Create the magic index
      const int magic_index = static_cast<int>(
          (occupancies[i] * magic_number) >> (64 - num_of_relevant_bits));

      if (used_attacks[magic_index] == EMPTY_BB) {  // Magic index works
        used_attacks[magic_index] = attacks[i];
      } else if (used_attacks[magic_index] != attacks[i]) {  // Collision
        fails = true;
      }
    }

    if (!fails) {  // If no failures return the magic number
      return magic_number;
    }
  }

  LOG_E << "No magic number found" << END_E;
  assert(false);
  return EMPTY_BB;
}


void print_magic_numbers()
{
  LOG_I << "\n- ROOK MAGIC NUMBERS -\n";
  for (int i = 0; i < 64; ++i) {
    const square_t square = static_cast<square_t>(i);
    const u64 magic_number =
        find_magic_number(square, rook_relevant_bits_count[square], ROOK);

    LOG_I << std::hex << "0x" << magic_number << "ULL,\n";
  }

  LOG_I << "\n- BISHOP MAGIC NUMBERS -\n";
  for (int i = 0; i < 64; ++i) {
    const square_t square = static_cast<square_t>(i);
    const u64 magic_number =
        find_magic_number(square, bishop_relevant_bits_count[square], BISHOP);

    LOG_I << std::hex << "0x" << magic_number << "ULL,\n";
  }

  LOG_I << END_I;
}


void init_attack_vectors()
{
  for (int i = 0; i < 64; ++i) {
    assert(i >= A8);
    assert(i <= H1);
    const square_t square = static_cast<square_t>(i);

    // Init pawn attacks
    attack_vectors.pawn[WHITE][i] = generate_mask_pawn_attacks(WHITE, square);
    attack_vectors.pawn[BLACK][i] = generate_mask_pawn_attacks(BLACK, square);

    // Init knight attacks
    attack_vectors.knight[i] = generate_mask_knight_attacks(square);

    // Init king attacks
    attack_vectors.king[i] = generate_mask_king_attacks(square);
  }

  LOG_S << "Attack vectors generated!" << END_S;
}


u64 generate_mask_pawn_attacks(const color_t color, const square_t square)
{
  u64 attacks = EMPTY_BB;

  u64 board = EMPTY_BB;
  set_bit(board, square);

  switch (color) {
    case WHITE:
      if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
      if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
      break;

    case BLACK:
      if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
      if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
      break;
  }

  return attacks;
}


u64 generate_mask_knight_attacks(const square_t square)
{
  u64 attacks = EMPTY_BB;

  u64 board = EMPTY_BB;
  set_bit(board, square);

  if ((board >> 17) & NOT_H_FILE) { attacks |= (board >> 17); }
  if ((board >> 15) & NOT_A_FILE) { attacks |= (board >> 15); }
  if ((board >> 10) & NOT_GH_FILES) { attacks |= (board >> 10); }
  if ((board >> 6) & NOT_AB_FILES) { attacks |= (board >> 6); }

  if ((board << 17) & NOT_A_FILE) { attacks |= (board << 17); }
  if ((board << 15) & NOT_H_FILE) { attacks |= (board << 15); }
  if ((board << 10) & NOT_AB_FILES) { attacks |= (board << 10); }
  if ((board << 6) & NOT_GH_FILES) { attacks |= (board << 6); }

  return attacks;
}


u64 generate_mask_king_attacks(const square_t square)
{
  u64 attacks = EMPTY_BB;

  u64 board = EMPTY_BB;
  set_bit(board, square);

  if (board >> 8) { attacks |= (board >> 8); }
  if ((board >> 9) & NOT_H_FILE) { attacks |= (board >> 9); }
  if ((board >> 7) & NOT_A_FILE) { attacks |= (board >> 7); }
  if ((board >> 1) & NOT_H_FILE) { attacks |= (board >> 1); }

  if (board << 8) { attacks |= (board << 8); }
  if ((board << 9) & NOT_A_FILE) { attacks |= (board << 9); }
  if ((board << 7) & NOT_H_FILE) { attacks |= (board << 7); }
  if ((board << 1) & NOT_A_FILE) { attacks |= (board << 1); }

  return attacks;
}


u64 generate_mask_bishop_attacks(const square_t square)
{
  u64 attacks = EMPTY_BB;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r < 7 && f < 7; ++r, ++f) {
    attacks |= (ONE_BIT << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf + 1; r > 0 && f < 7; --r, ++f) {
    attacks |= (ONE_BIT << ((r * 8) + f));
  }
  for (r = tr + 1, f = tf - 1; r < 7 && f > 0; ++r, --f) {
    attacks |= (ONE_BIT << ((r * 8) + f));
  }
  for (r = tr - 1, f = tf - 1; r > 0 && f > 0; --r, --f) {
    attacks |= (ONE_BIT << ((r * 8) + f));
  }

  return attacks;
}


u64 generate_mask_rook_attacks(const square_t square)
{
  u64 attacks = EMPTY_BB;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r < 7; ++r) {
    attacks |= (ONE_BIT << ((r * 8) + tf));
  }
  for (r = tr - 1; r > 0; --r) {
    attacks |= (ONE_BIT << ((r * 8) + tf));
  }
  for (f = tf + 1; f < 7; ++f) {
    attacks |= (ONE_BIT << ((tr * 8) + f));
  }
  for (f = tf - 1; f > 0; --f) {
    attacks |= (ONE_BIT << ((tr * 8) + f));
  }

  return attacks;
}


u64 bishop_attacks(const square_t square, const u64 blocks)
{
  u64 attacks = EMPTY_BB;

  (void)blocks;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; ++r, ++f) {
    const u64 candidate = (ONE_BIT << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; --r, ++f) {
    const u64 candidate = (ONE_BIT << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; ++r, --f) {
    const u64 candidate = (ONE_BIT << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; --r, --f) {
    const u64 candidate = (ONE_BIT << ((r * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


u64 rook_attacks(const square_t square, const u64 blocks)
{
  u64 attacks = EMPTY_BB;

  const int tr = square / 8;  // Target rank
  const int tf = square % 8;  // Target file

  int r, f;  // Current rank and file
  for (r = tr + 1; r <= 7; ++r) {
    const u64 candidate = (ONE_BIT << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (r = tr - 1; r >= 0; --r) {
    const u64 candidate = (ONE_BIT << ((r * 8) + tf));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf + 1; f <= 7; ++f) {
    const u64 candidate = (ONE_BIT << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }
  for (f = tf - 1; f >= 0; --f) {
    const u64 candidate = (ONE_BIT << ((tr * 8) + f));
    attacks |= candidate;
    if (candidate & blocks) { break; }
  }

  return attacks;
}


u64 set_occupancy(const int index, const int mask_bit_count, u64 attack_mask)
{
  u64 occupancy = EMPTY_BB;

  for (int count = 0; count < mask_bit_count; ++count) {
    const int square = get_lsb_index(attack_mask);

    assert(square >= A8);
    assert(square <= H1);

    pop_bit(attack_mask, square);

    // Check if on board
    if (index & (ONE_BIT << count)) { occupancy |= ONE_BIT << square; }
  }

  return occupancy;
}


void print_board(const u64 board)
{
  LOG_I << "\n";

  LOG_I << "    bitboard: " << std::hex << board << std::dec << "\n\n";

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      // Print ranks
      if (file == 0) { LOG_I << 8 - rank << "   "; }

      // Print the bit
      LOG_I << (get_bit(board, square) ? 1 : 0) << " ";
    }

    LOG_I << "\n";
  }

  // Print the files
  LOG_I << "\n    A B C D E F G H \n" << END_I;
}


void print_NOT_A_FILE()
{
  u64 not_a_file = EMPTY_BB;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      if (file > 0) { set_bit(not_a_file, square); }
    }
  }

  print_board(not_a_file);
}


void print_NOT_H_FILE()
{
  u64 not_h_file = EMPTY_BB;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      if (file < 7) { set_bit(not_h_file, square); }
    }
  }

  print_board(not_h_file);
}


void print_NOT_AB_FILE()
{
  u64 not_ab_file = EMPTY_BB;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      if (file > 1) { set_bit(not_ab_file, square); }
    }
  }

  print_board(not_ab_file);
}


void print_NOT_GH_FILE()
{
  u64 not_gh_file = EMPTY_BB;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      if (file < 6) { set_bit(not_gh_file, square); }
    }
  }

  print_board(not_gh_file);
}

}  // namespace chesso