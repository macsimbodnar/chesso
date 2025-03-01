#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <stack>


//-#############################  DEFINES  ##################################-//
#define BOARD_SIZE 128


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
  WHITE,
  BLACK
};

enum piece_t
{
  B_PAWN = 'p',
  B_KNIGHT = 'n',
  B_BISHOP = 'b',
  B_ROOK = 'r',
  B_QUEEN = 'q',
  B_KING = 'k',
  W_PAWN = 'P',
  W_KNIGHT = 'N',
  W_BISHOP = 'B',
  W_ROOK = 'R',
  W_QUEEN = 'Q',
  W_KING = 'K',
  INVALID = '*',
  EMPTY = ' '
};


//-#############################  STRUCTS  ##################################-//
struct move_t
{};

struct game_state_t
{
  color_t active_color;               // Side to move
  uint8_t castling;                   // Castling permissions
  uint8_t half_move_clock;            // Half moves played
  std::optional<uint8_t> en_passant;  // Active en-passant square, if any
  uint16_t full_move_number;          // Total number of full moves played
  uint64_t zobrist_key;               // Zobrist Key
  int16_t phase_value;                // Evaluation Phase Value
  move_t next_move;                   // The move played in this position
};

typedef std::stack<game_state_t> history_t;

struct board_t
{
  std::array<piece_t, BOARD_SIZE> board;
  game_state_t game_state;
  history_t history;
  std::list<piece_t> piece_list;
  // Zobrist random;
};