#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <stack>
#include <string>


//-#############################  DEFINES  ##################################-//
#define BOARD_SIZE 128
#define DEFAULT_POSITION \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

typedef uint8_t index_t;

static constexpr index_t INVALID_BOARD_INDEX = 127;

//-#############################   ENUMS   ##################################-//
enum castling_t
{
  WQ = 0b0000001,
  WK = 0b0000010,
  BQ = 0b0000100,
  BK = 0b0001000
};

enum color_t
{
  BLACK,
  WHITE
};


inline color_t operator!(const color_t& c)
{
  const color_t res = (c == color_t::WHITE) ? color_t::BLACK : color_t::WHITE;
  return res;
}


enum piece_t
{
  B_PAWN = 0,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_QUEEN,
  B_KING,
  W_PAWN,
  W_KNIGHT,
  W_BISHOP,
  W_ROOK,
  W_QUEEN,
  W_KING,
  INVALID,
  EMPTY
};


//-#############################  STRUCTS  ##################################-//
struct position_t
{
  uint8_t file;  // From 0 to 7
  uint8_t rank;  // From 0 to 7

  bool operator==(const position_t& other) const
  {
    return file == other.file && rank == other.rank;
  }

  position_t()
  {
    file = 0;
    rank = 0;
  }

  position_t(uint8_t file, uint8_t rank)
  {
    this->file = file;
    this->rank = rank;
  }
};


struct zobrist_randoms_t
{
  std::array<std::array<uint64_t, 64>, 6> piece_randoms;  // 12 pis * 64 squares
  std::array<uint64_t, 16> castling_randoms;
  std::array<uint64_t, 2> side_randoms;
  std::array<uint64_t, 65> ep_randoms;  // en-passant randoms.
};

struct move_t
{};

struct game_state_t
{
  color_t active_color;       // Side to move
  uint8_t castling;           // Castling permissions
  uint8_t half_move_clock;    // Half moves played
  index_t en_passant;         // Active en-passant square index, if any
  uint16_t full_move_number;  // Total number of full moves played
  uint64_t zobrist_key;       // Zobrist Key
  int16_t phase_value;        // Evaluation Phase Value
  move_t next_move;           // The move played in this position
};

typedef std::stack<game_state_t> history_t;

struct board_t
{
  std::array<piece_t, BOARD_SIZE> board;
  game_state_t game_state;
  history_t history;
  zobrist_randoms_t zobrist_randoms;  // The keys used for Zobrist hashing.
  std::string initial_fen;
};