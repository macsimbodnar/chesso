#pragma once
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "exceptions.hpp"
#include "log.hpp"

#ifdef I
static_assert(false);
#endif

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

#define U32(x) static_cast<uint32_t>(x)
#define INT(x) static_cast<int>(x)

static constexpr size_t BOARD_ARRAY_SIZE = 128;
static constexpr uint8_t INVALID_BOARD_POS = 127;

// Castling rights
static constexpr uint8_t WQ = 0b0000001;
static constexpr uint8_t WK = 0b0000010;
static constexpr uint8_t BQ = 0b0000100;
static constexpr uint8_t BK = 0b0001000;

enum class color_t
{
  BLACK = 0x01000000,
  WHITE = 0x10000000
};

inline color_t operator!(const color_t& c)
{
  color_t res = (c == color_t::WHITE) ? color_t::BLACK : color_t::WHITE;
  return res;
}

inline std::ostream& operator<<(std::ostream& os, const color_t& dt)
{
  switch (dt) {
    case color_t::WHITE:
      os << "WHITE";
      break;

    case color_t::BLACK:
      os << "BLACK";
      break;

    default:
      assert(false);
      break;
  }

  return os;
}

static constexpr uint32_t COLOR_MASK =
    U32(color_t::BLACK) | U32(color_t::WHITE);


enum class piece_t
{
  INVALID = 0x00000000,
  EMPTY,
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


struct position_t
{
  uint8_t file;
  uint8_t rank;
};

inline bool operator==(const position_t& lhs, const position_t& rhs)
{
  bool result = false;
  if (lhs.file == rhs.file && lhs.rank == rhs.rank) { result = true; }
  return result;
}

inline bool operator!=(const position_t& lhs, const position_t& rhs)
{
  bool result = !(lhs == rhs);
  return result;
}

struct coordinates_t
{
  uint8_t x;
  uint8_t y;
};

inline bool operator==(const coordinates_t& lhs, const coordinates_t& rhs)
{
  bool result = false;
  if (lhs.x == rhs.x && lhs.y == rhs.y) { result = true; }
  return result;
}

inline bool operator!=(const coordinates_t& lhs, const coordinates_t& rhs)
{
  bool result = !(lhs == rhs);
  return result;
}


struct move_t
{
  uint8_t from;
  uint8_t to;
};


struct piece_info_t
{
  piece_t piece;
  position_t position;
  uint8_t index;
};


struct board_state_t
{
  std::array<piece_t, BOARD_ARRAY_SIZE> board;
  color_t active_color;
  uint8_t available_castling;
  uint8_t en_passant_target_square;
};


inline void print_move(const position_t& from, const position_t& to)
{
  LOG_I << std::string(1, 'a' + from.file) << from.rank + 1 << " -> "
        << std::string(1, 'a' + to.file) << to.rank + 1 << END_I;
}

inline void print_index(const uint8_t index)
{
  const uint8_t file = index & 7;
  const uint8_t rank = index >> 4;

  LOG_I << std::string(1, 'a' + file) << rank + 1 << END_I;
}

template <typename T>
inline std::string PTR2STR(T* ptr)
{
  const void* address = static_cast<const void*>(ptr);
  std::stringstream ss;
  ss << address;
  return ss.str();
}


inline uint8_t to_index(const position_t& position)
{
  assert(position.file >= 0 && position.file < 8);
  assert(position.rank >= 0 && position.rank < 8);

  const uint8_t index = (position.rank << 4) + position.file;
  assert(index < BOARD_ARRAY_SIZE);
  assert(!(index & 0x88));

  return index;
}


inline position_t to_position(const uint8_t index)
{
  // Check if we are inside the board
  assert(!(index & 0x88));
  assert(index < BOARD_ARRAY_SIZE);

  position_t result;
  result.file = index & 7;
  result.rank = index >> 4;

  return result;
}


inline std::vector<std::string> split_string(const std::string& str)
{
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  std::vector<std::string> tokens(begin, end);

  return tokens;
}


inline bool is_uint(const std::string& str)
{
  for (const char c : str) {
    if (!isdigit(c)) { return false; }
  }

  return true;
}


inline piece_t c_to_piece(const char p)
{
  piece_t piece = piece_t::INVALID;

  switch (p) {
    case 'P':
      piece = piece_t::W_PAWN;
      break;
    case 'N':
      piece = piece_t::W_KNIGHT;
      break;
    case 'B':
      piece = piece_t::W_BISHOP;
      break;
    case 'R':
      piece = piece_t::W_ROOK;
      break;
    case 'Q':
      piece = piece_t::W_QUEEN;
      break;
    case 'K':
      piece = piece_t::W_KING;
      break;
    case 'p':
      piece = piece_t::B_PAWN;
      break;
    case 'n':
      piece = piece_t::B_KNIGHT;
      break;
    case 'b':
      piece = piece_t::B_BISHOP;
      break;
    case 'r':
      piece = piece_t::B_ROOK;
      break;
    case 'q':
      piece = piece_t::B_QUEEN;
      break;
    case 'k':
      piece = piece_t::B_KING;
      break;
    default:
      throw input_exception("Invalid piece: " + std::string(1, p));
      break;
  }

  return piece;
}


inline uint8_t algebraic_to_index(const std::string& p)
{
  uint8_t result = INVALID_BOARD_POS;

  if (p.size() != 2) { return INVALID_BOARD_POS; }

  char file = p[0];
  char rank = p[1];

  assert(file >= 'a');
  assert(file <= 'h');
  assert(rank >= '1');
  assert(rank <= '8');

  position_t pos;
  pos.file = file - 'a';
  pos.rank = rank - '1';

  result = to_index(pos);

  return result;
}


inline std::string index_to_algebraic(const uint8_t i)
{
  if (i == INVALID_BOARD_POS) { return "-"; }

  position_t p = to_position(i);
  std::string result;
  result.reserve(2);

  result.push_back('a' + p.file);
  result.push_back('1' + p.rank);

  return result;
}


inline color_t get_color(const piece_t p)
{
  assert(p != piece_t::EMPTY);
  assert(p != piece_t::INVALID);

  color_t color = static_cast<color_t>(U32(p) & COLOR_MASK);

  return color;
}
