#pragma once
#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <ostream>
#include <stack>
#include <string>


// clang-format off
/**
 * 
 * Mailbox 0x88
 * 
 * 128 byte array
 * Files A - H        X
 * Ranks 1 - 8        7 - Y
 ************************************************************************************
 *     A    B    C    D    E    F    G    H
 * 8 | 70 | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79 | 7A | 7B | 7C | 7D | 7E | 7F
 * 7 | 60 | 61 | 62 | 63 | 64 | 65 | 66 | 67 | 68 | 69 | 6A | 6B | 6C | 6D | 6E | 6F
 * 6 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 5A | 5B | 5C | 5D | 5E | 5F
 * 5 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47 | 48 | 49 | 4A | 4B | 4C | 4D | 4E | 4F
 * 4 | 30 | 31 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 3A | 3B | 3C | 3D | 3E | 3F
 * 3 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 2A | 2B | 2C | 2D | 2E | 2F
 * 2 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B | 1C | 1D | 1E | 1F
 * 1 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D | 0E | 0F
 *     A    B    C    D    E    F    G    H
 ***********************************************************************************/
// clang-format on


//-#############################  DEFINES  ##################################-//
#define BOARD_SIZE 128
#define DEFAULT_POSITION \
  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

typedef uint8_t index_t;
typedef uint8_t castling_t;

static constexpr index_t INVALID_BOARD_INDEX = 127;

//-#############################   ENUMS   ##################################-//
enum castling_rights_t
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


enum promotion_t
{
  TO_NONE,
  TO_QUEEN,
  TO_KNIGHT,
  TO_ROOK,
  TO_BISHOP
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

  bool operator!=(const position_t& other) const { return !(*this == other); }

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


inline std::ostream& operator<<(std::ostream& os, const position_t& pos)
{
  const char file = 'a' + static_cast<char>(pos.file);
  const char rank = '1' + static_cast<char>(pos.rank);

  os << file << rank;

  return os;
}


struct zobrist_randoms_t
{
  std::array<std::array<uint64_t, 64>, 6> piece_randoms;  // 12 pis * 64 squares
  std::array<uint64_t, 16> castling_randoms;
  std::array<uint64_t, 2> side_randoms;
  std::array<uint64_t, 65> ep_randoms;  // en-passant randoms.
};

struct move_t
{
  index_t from;             // Source square
  index_t to;               // Destination square
  piece_t piece;            // Moved piece
  promotion_t promoted_to;  // Eventual promotion
  piece_t captured;         // If capture happened then the captured piece
  bool double_pawn_move;    // Double pawn move. Eventually set en-passant
  bool en_passant_capture;  // Set if this is en-passant capture happened
  bool castling_move;       // Set if castling happened
  // TODO: add if check
  // TODO: add ifdiscovery check
  // TODO: add if double check
  // TODO: add if checkmate


  bool operator==(const move_t& other) const
  {
    return (from == other.from && to == other.to && piece == other.piece &&
            promoted_to == other.promoted_to && captured == other.captured &&
            double_pawn_move == other.double_pawn_move &&
            en_passant_capture == other.en_passant_capture &&
            castling_move == other.castling_move);
  }

  move_t() : move_t(INVALID_BOARD_INDEX, INVALID_BOARD_INDEX, INVALID) {}

  move_t(index_t from, index_t to, piece_t piece)
      : from(from),
        to(to),
        piece(piece),
        promoted_to(TO_NONE),
        captured(INVALID),
        double_pawn_move(false),
        en_passant_capture(false),
        castling_move(false)
  {}
};

struct game_state_t
{
  color_t active_color;       // Side to move
  uint8_t castling;           // Castling permissions
  uint8_t halfmove_clock;     // Moves with respect to the 50 move draw rule
  index_t en_passant;         // Active en-passant square index, if any
  uint16_t fullmove_counter;  // Total number of full moves played
  uint64_t zobrist_key;       // Zobrist Key
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
