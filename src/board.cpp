#include "board.hpp"
#include <cassert>
#include "log.hpp"


namespace chesso
{

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


void print_board(const u64 board)
{
  LOG_I << std::endl;

  LOG_I << "    bitboard: " << std::hex << board << std::dec << std::endl
        << std::endl;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      const int square = (rank * 8) + file;

      // Print ranks
      if (file == 0) { LOG_I << 8 - rank << "   "; }

      // Print the bit
      LOG_I << (get_bit(board, square) ? 1 : 0) << " ";
    }

    LOG_I << std::endl;
  }

  // Print the files
  LOG_I << std::endl << "    A B C D E F G H " << std::endl << END_I;
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