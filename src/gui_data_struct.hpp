#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "utils.hpp"

namespace gui
{

struct position_t
{
  uint8_t rank;  // rows y    [1-8]
  uint8_t file;  // columns x [A-H]
};

inline bool operator==(const position_t& lhs, const position_t& rhs)
{
  return lhs.rank == rhs.rank && lhs.file == rhs.file;
}

inline bool operator!=(const position_t& lhs, const position_t& rhs)
{
  return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& os, const position_t& pos)
{
  const char file = 'a' + static_cast<char>(pos.file);
  const char rank = '1' + static_cast<char>(pos.rank);

  os << file << rank;

  return os;
}


namespace castling_rights_t
{
constexpr uint8_t NONE = 0b00000000;
constexpr uint8_t WQ = 0b00000001;
constexpr uint8_t WK = 0b00000010;
constexpr uint8_t BQ = 0b00000100;
constexpr uint8_t BK = 0b00001000;
constexpr uint8_t ALL = 0b00001111;
}  // namespace castling_rights_t

enum color_t
{
  BLACK = 0x01000000,
  WHITE = 0x10000000
};

enum class piece_t
{
  EMPTY = 0x00000000,
  // Black pieces can be checked with B_KING & BLACK
  B_KING = 0x01000000,
  B_QUEEN,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_PAWN,
  // Black pieces can be checked with W_KING & WHITE
  W_KING = 0x10000000,
  W_QUEEN,
  W_KNIGHT,
  W_BISHOP,
  W_ROOK,
  W_PAWN
};


struct state_t
{
  std::array<std::array<piece_t, 8>, 8> board;
  color_t active_color = color_t::WHITE;
  uint8_t castling_rights = castling_rights_t::ALL;
  bool is_en_passant = false;
  position_t en_passant_target_square;
  int half_move_clock = 0;
  int full_move_clock = 1;
};

piece_t get_piece(const state_t& state, const position_t& pos);
void set_piece(state_t& state, const position_t& pos, const piece_t piece);
piece_t c_to_piece(const char p);
color_t get_piece_color(const piece_t t);
state_t load_FEN(const std::string& fen);
std::string state_to_FEN(const state_t &state);
position_t algebraic_to_pos(const std::string& p);
std::string pos_to_algebraic(const position_t& p);

};  // namespace gui
