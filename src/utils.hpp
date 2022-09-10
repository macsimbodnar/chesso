#pragma once
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "log.hpp"
#include "exceptions.hpp"


enum class color_t
{
  BLACK,
  WHITE
};


enum class piece_t
{
  INVALID,
  EMPTY,
  B_KING,
  B_QUEEN,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_PAWN,
  W_KING,
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


struct coordinates_t
{
  uint8_t x;
  uint8_t y;
};


struct piece_info_t
{
  piece_t piece;
  position_t position;
  uint8_t index;
};


inline void print_move(const uint8_t from_file,
                       const uint8_t from_rank,
                       const uint8_t to_file,
                       const uint8_t to_rank)
{
  LOG_I << std::string(1, 'a' + from_file) << from_rank + 1 << " -> "
        << std::string(1, 'a' + to_file) << to_rank + 1 << END_I;
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
      piece = piece_t::B_KNIGHT;
      break;
    default:
      throw input_exception("Invalid piece: " + std::string(1, p));
      break;
  }

  return piece;
}