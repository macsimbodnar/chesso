#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
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
 ****************************************************************************HEX****
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
 ***********************************************************************************

 ************************************************************************DECIMAL****
 *     A    B    C    D    E    F    G    H
 * 8 |112 |113 |114 |115 |116 |117 |118 |119 |120 |121 |122 |123 |124 |125 |126 |127
 * 7 | 96 | 97 | 98 | 99 |100 |101 |102 |103 |104 |105 |106 |107 |108 |109 |110 |111
 * 6 | 80 | 81 | 82 | 83 | 84 | 85 | 86 | 87 | 88 | 89 | 90 | 91 | 92 | 93 | 94 | 95
 * 5 | 64 | 65 | 66 | 67 | 68 | 69 | 70 | 71 | 72 | 73 | 74 | 75 | 76 | 77 | 78 | 79
 * 4 | 48 | 49 | 50 | 51 | 52 | 53 | 54 | 55 | 56 | 57 | 58 | 59 | 60 | 61 | 62 | 63
 * 3 | 32 | 33 | 34 | 35 | 36 | 37 | 38 | 39 | 40 | 41 | 42 | 43 | 44 | 45 | 46 | 47
 * 2 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31
 * 1 | 00 | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 | 11 | 12 | 13 | 14 | 15
 *     A    B    C    D    E    F    G    H
 ***********************************************************************************/
// clang-format on

//-############################# POSITIONS ##################################-//
// clang-format off
#define DEFAULT_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define EMPTY_POS "8/8/8/8/8/8/8/8 b - - 0 1"
#define TRICKY_POS "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define KILLER_POS "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define CMK_POS "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
#define FINE_70_POS "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1"  // best move: Kb1
#define MATE_IN_2_W_POS "4k3/Q7/8/4K3/8/8/8/8 w - - 0 1"
#define MATE_IN_2_B_POS "4K3/q7/8/4k3/8/8/8/8 b - - 0 1"
#define THREE_FOLD_REP_POS "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 1"
#define THREE_FOLD_REP_2_POS "1r3rk1/R2PR3/1nB2p2/5P2/8/PP5P/2K5/8 w - - 3 55"
// clang-format on


//-#############################  DEFINES  ##################################-//
#define BOARD_SIZE 128

#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

typedef uint8_t index_t;
typedef uint8_t castling_t;

static constexpr index_t INVALID_BOARD_INDEX = 127;

// The maximum number of legal moves that is possible to generate
#define MAX_MOVES 270
#define MAX_PLY 100
#define MAX_DEPTH MAX_PLY
#define REPETITION_MAX_SIZE 500

// Transposition table size
// #define TT_SIZE 8388608
// #define TT_SIZE 4194304
#define TT_SIZE 4194301

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
  const color_t res = (c == WHITE) ? BLACK : WHITE;
  return res;
}


enum piece_t
{
  B_PAWN = 0,  // 'p'
  B_KNIGHT,    // 'n'
  B_BISHOP,    // 'b'
  B_ROOK,      // 'r'
  B_QUEEN,     // 'q'
  B_KING,      // 'k'
  W_PAWN,      // 'P'
  W_KNIGHT,    // 'N'
  W_BISHOP,    // 'B'
  W_ROOK,      // 'R'
  W_QUEEN,     // 'Q'
  W_KING,      // 'K'
  INVALID,
  EMPTY
};


enum promotion_t
{
  TO_NONE,
  TO_KNIGHT,
  TO_BISHOP,
  TO_ROOK,
  TO_QUEEN
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

  position_t() : file(0), rank(0) {}
  position_t(uint8_t file, uint8_t rank) : file(file), rank(rank) {}
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
  // Here we will use 128 instead 64 squares in order to include
  // INVALID_BOARD_INDEX
  uint64_t piece_randoms[12][BOARD_SIZE];  // 12 pieces
  uint64_t castling_randoms[16];
  uint64_t side_randoms[2];
  uint64_t ep_randoms[BOARD_SIZE];  // en-passant randoms.
  bool initialized = false;
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

  // Comparison operator
  bool operator==(const move_t& other) const
  {
    return (from == other.from && to == other.to && piece == other.piece &&
            promoted_to == other.promoted_to);
    // return (from == other.from && to == other.to && piece == other.piece &&
    //         promoted_to == other.promoted_to && captured == other.captured &&
    //         double_pawn_move == other.double_pawn_move &&
    //         en_passant_capture == other.en_passant_capture &&
    //         castling_move == other.castling_move);
  }

  // Boolean conversion operator. Is required to use inside if statements
  explicit operator bool() const
  {
    return (from != INVALID_BOARD_INDEX && to != INVALID_BOARD_INDEX);
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


inline std::ostream& operator<<(std::ostream& os, const move_t& move)
{
  const position_t from(move.from & 7, move.from >> 4);
  const position_t to(move.to & 7, move.to >> 4);

  const char from_file = 'a' + static_cast<char>(from.file);
  const char from_rank = '1' + static_cast<char>(from.rank);
  const char to_file = 'a' + static_cast<char>(to.file);
  const char to_rank = '1' + static_cast<char>(to.rank);

  os << from_file << from_rank << to_file << to_rank;

  return os;
}


struct game_state_t
{
  color_t active_color;       // Side to move
  uint8_t castling;           // Castling permissions
  uint8_t halfmove_clock;     // Moves with respect to the 50 move draw rule
  index_t en_passant;         // Active en-passant square index, if any
  uint16_t fullmove_counter;  // Total number of full moves played
  uint64_t zobrist_key;       // Zobrist Key
};

struct board_t
{
  piece_t board[BOARD_SIZE];
  game_state_t game_state;
  zobrist_randoms_t zobrist_randoms;  // The keys used for Zobrist hashing.
  size_t repetition_size;
  uint64_t repetitions[REPETITION_MAX_SIZE];
};

struct history_entry_t
{
  // game_state_t game_state;
  board_t board;
  move_t move_applied;
};

// TODO: Make heep allocation during initialization
typedef std::stack<history_entry_t> history_t;

struct pv_t
{
  size_t pv_length[MAX_PLY];
  move_t pv_table[MAX_PLY][MAX_PLY];
};


enum node_type_t
{
  TT_EMPTY_NODE,
  TT_PV_NODE,     // The stored score is EXACTLY that
  TT_ALPHA_NODE,  // The stored score was at most that. Upperbound. Fail-low
  TT_BETA_NODE    // The stored score was at least that. Lowerbound. Fail-high
};


struct tt_entry_t
{
  uint64_t key = 0;
  node_type_t type = TT_EMPTY_NODE;
  int depth = 0;
  int score = 0;
  move_t best_move;
};


struct transposition_table_t
{
  tt_entry_t entries[TT_SIZE];
};


struct search_t
{
  move_t best_move;
  int score;
  uint64_t explored_nodes;
  pv_t pv;
  bool mate_found;
  int mate_in;
};


struct search_state_t
{
  std::atomic_bool* stop = nullptr;
  uint64_t explored_nodes;
  move_t killer_moves[2][MAX_PLY];
  int history_moves[piece_t::EMPTY + 1][BOARD_SIZE];
  pv_t pv;
  bool search_in_tt = true;
  transposition_table_t* tt;  // Too big to keep on the stack
  move_t best_move;
};
