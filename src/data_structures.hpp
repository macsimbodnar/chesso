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

#define CLOSED_POSITION "3k4/8/7p/2p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1"
// clang-format on


//-#############################  DEFINES  ##################################-//
#ifndef STR
#define STR(_N_) std::to_string(_N_)
#endif

typedef uint8_t index_t;
typedef uint8_t castling_t;
typedef uint64_t bb_t;
typedef uint64_t hash_t;
typedef uint32_t move_t;

// The maximum number of legal moves that is possible to generate
#define MAX_MOVES 270
#define MAX_PLY 100
#define MAX_DEPTH MAX_PLY
#define REPETITION_MAX_SIZE 5000
#define HISTORY_MAX_SIZE 1000000

// Transposition table size
#define TT_SIZE 4194301

/**
 *   0000 0000 0000 0000 0011 1111    source square       0x3f
 *   0000 0000 0000 1111 1100 0000    target square       0xfc0
 *   0000 0000 1111 0000 0000 0000    piece               0xf000
 *   0000 0111 0000 0000 0000 0000    promoted to         0x70000
 *   0000 1000 0000 0000 0000 0000    NOT USED            0xf0000
 *   0001 0000 0000 0000 0000 0000    capture flag        0x100000
 *   0010 0000 0000 0000 0000 0000    double push flag    0x200000
 *   0100 0000 0000 0000 0000 0000    en-passant flag     0x400000
 *   1000 0000 0000 0000 0000 0000    castling flag       0x800000
 */

// clang-format off
#define NEW_MOVE(source, target, piece, promoted, capture, double, en_passant, castling) \
                ((source) | \
                 ((target) << 6) | \
                 ((piece) << 12) | \
                 ((promoted) << 16) | \
                 ((capture) << 20) | \
                 ((double) << 21) | \
                 ((en_passant) << 22) | \
                 ((castling) << 23))
// clang-format on

#define MOVE_FROM(move) (static_cast<index_t>((move) & 0x3f))
#define MOVE_TO(move) (static_cast<index_t>(((move) & 0xfc0) >> 6))
#define MOVE_PIECE(move) (static_cast<piece_t>(((move) & 0xf000) >> 12))
#define MOVE_PROMOTED(move) (static_cast<promotion_t>(((move) & 0x70000) >> 16))
#define MOVE_CAPTURE(move) ((move) & 0x100000)
#define MOVE_DOUBLE_PUSH(move) ((move) & 0x200000)
#define MOVE_EN_PASSANT(move) ((move) & 0x400000)
#define MOVE_CASTLING(move) ((move) & 0x800000)

//-#######################   BITBOARD SPECIFIC   ############################-//
#define BB_1 1ULL
#define BB_0 0ULL

#define NOT_A_FILE 0xFEFEFEFEFEFEFEFEULL
#define NOT_H_FILE 0x7F7F7F7F7F7F7F7FULL
#define NOT_GH_FILES 0x3F3F3F3F3F3F3F3FULL
#define NOT_AB_FILES 0xFCFCFCFCFCFCFCFCULL

#define GET_BIT(bboard, square) ((bboard) & (BB_1 << (square)))
#define SET_BIT(bboard, square) ((bboard) |= (BB_1 << (square)))
#define POP_BIT(bboard, square) ((bboard) &= ~(BB_1 << (square)))

//-#############################   ENUMS   ##################################-//
enum castling_rights_t
{
  WK = 0b0000001,
  WQ = 0b0000010,
  BK = 0b0000100,
  BQ = 0b0001000
};

enum color_t
{
  WHITE,
  BLACK,
  BOTH
};

inline color_t operator!(const color_t& c)
{
  const color_t res = (c == WHITE) ? BLACK : WHITE;
  return res;
}


enum promotion_t
{
  TO_NONE = 0,  // MUST be zero
  TO_KNIGHT,
  TO_BISHOP,
  TO_ROOK,
  TO_QUEEN
};


enum piece_t
{
  W_PAWN,    // 'P'   MUST be first
  W_KNIGHT,  // 'N'
  W_BISHOP,  // 'B'
  W_ROOK,    // 'R'
  W_QUEEN,   // 'Q'
  W_KING,    // 'K'
  B_PAWN,    // 'p'
  B_KNIGHT,  // 'n'
  B_BISHOP,  // 'b'
  B_ROOK,    // 'r'
  B_QUEEN,   // 'q'
  B_KING,    // 'k'   MUST be last
  EMPTY
};


// clang-format off
enum bb_squares_t {
  a8, b8, c8, d8, e8, f8, g8, h8,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a1, b1, c1, d1, e1, f1, g1, h1, INVALID_INDEX
};
// clang-format on


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
  uint64_t piece_randoms[12][64];  // Each piece on each square
  uint64_t castling_randoms[16];
  uint64_t side_randoms[2];
  uint64_t ep_randoms[65];  // en-passant for each square plus invalid
  bool initialized = false;
};

struct unpacked_move_t
{
  index_t from;
  index_t to;
  piece_t piece;
  promotion_t promoted_to;
  bool capture;
  bool double_push;
  bool en_passant;
  bool castling;

  unpacked_move_t(move_t move)
      : from(MOVE_FROM(move)),
        to(MOVE_TO(move)),
        piece(MOVE_PIECE(move)),
        promoted_to(MOVE_PROMOTED(move)),
        capture(MOVE_CAPTURE(move)),
        double_push(MOVE_DOUBLE_PUSH(move)),
        en_passant(MOVE_EN_PASSANT(move)),
        castling(MOVE_CASTLING(move))
  {}

  // Comparison operator
  bool operator==(const unpacked_move_t& other) const
  {
    return (from == other.from && to == other.to && piece == other.piece &&
            promoted_to == other.promoted_to);
  }

  // Boolean conversion operator. Is required to use inside if statements
  explicit operator bool() const
  {
    return (from != INVALID_INDEX || to != INVALID_INDEX);
  }

  move_t pack() const
  {
    const move_t move = NEW_MOVE(from, to, piece, promoted_to, capture,
                                 double_push, en_passant, castling);

    return move;
  }
};


struct bb_tables_t
{
  bb_t pawn_attacks[2][64];  // [color][squares]
  bb_t knight_attacks[64];
  bb_t king_attacks[64];

  bb_t bishop_masks[64];         // [square]
  bb_t rook_masks[64];           // [square]
  bb_t bishop_attacks[64][512];  // [square][occupancies]
  bb_t rook_attacks[64][4096];   // [square][occupancies]
};


struct board_t
{
  bb_t bitboards[12];
  bb_t occupancies[3];

  color_t active_color;       // Side to move
  uint8_t castling;           // Castling permissions
  uint8_t halfmove_clock;     // Moves with respect to the 50 move draw rule
  index_t en_passant;         // Active en-passant square index, if any
  uint16_t fullmove_counter;  // Total number of full moves played
  hash_t hash;                // Zobrist Key
};


struct history_entry_t
{
  board_t board;
  size_t repetition_size;
};


struct history_t
{
  history_entry_t entries[HISTORY_MAX_SIZE];
  size_t size = 0;
};


struct repetition_t
{
  hash_t entries[REPETITION_MAX_SIZE];
  size_t size = 0;
};


struct game_t
{
  bb_tables_t tables;
  board_t board;
  history_t history;
  repetition_t repetitions;
  zobrist_randoms_t hash_randoms;
};


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
  uint64_t key;
  node_type_t type;
  int depth;
  int score;
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
  int history_moves[12][64];  // [piece][destination]
  pv_t pv;
  bool search_in_tt = true;
  transposition_table_t* tt;  // Too big to keep on the stack
  move_t best_move;
};
