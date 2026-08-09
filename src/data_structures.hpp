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
#define MAX_PLY 128
#define MAX_DEPTH (MAX_PLY - 2)  // Must be +2 in order to be safe

// Holds the game moves replayed by [position ... moves ...] plus MAX_PLY of
// search on top. 5000 is far past the longest game the 75-move rule allows,
// and keeps game_t at a few megabytes instead of 150.
#define HISTORY_MAX_SIZE 5000
#define NODE_BUDGET_UNLIMITED 0

// Transposition table size, in megabytes. The table is heap allocated so the
// UCI [Hash] option can pick the size.
#define TT_DEFAULT_MB 16
#define TT_MIN_MB 1
#define TT_MAX_MB 4096

// Fixed size table used by the perft tests, unrelated to the engine's TT.
#define TT_SIZE 4194301

/**
 *   0000 0000 0000 0000 0011 1111    source square       0x3f
 *   0000 0000 0000 1111 1100 0000    target square       0xfc0
 *   0000 0000 1111 0000 0000 0000    piece               0xf000
 *   0000 0111 0000 0000 0000 0000    promoted to         0x70000
 *   0000 1000 0000 0000 0000 0000    NOT USED            0x80000
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

// uint8_t for the same reason as piece_t: these are stored in board_t, and an
// int-backed enum costs three bytes of padding each time.
enum color_t : uint8_t
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


// What a caller wants out of the generator. A search spends most of its nodes
// failing high on one of the first captures, so generating the 30-odd quiet
// moves at those nodes is wasted work.
//
// GEN_CAPTURES and GEN_QUIETS partition GEN_ALL exactly: every legal move
// belongs to one of them and none to both. Promotions count as captures
// whether or not anything is taken, because they are tactical and quiescence
// wants them; castling counts as quiet.
enum gen_type_t : uint8_t
{
  GEN_ALL,
  GEN_CAPTURES,
  GEN_QUIETS
};


enum promotion_t : uint8_t
{
  TO_NONE = 0,  // MUST be zero
  TO_KNIGHT,
  TO_BISHOP,
  TO_ROOK,
  TO_QUEEN
};


// uint8_t so that board_t can carry a piece-per-square array without paying
// four bytes a square for it.
enum piece_t : uint8_t
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
  { return file == other.file && rank == other.rank; }

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
  { return (from != INVALID_INDEX || to != INVALID_INDEX); }

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

  // Both are empty for square pairs that do not share a rank, file or
  // diagonal. between[a][b] excludes a and b; line[a][b] includes them and
  // runs the full length of the board.
  bb_t between[64][64];
  bb_t line[64][64];
};


// Field order is deliberate. The scalars below are read and written on every
// single move, so they are packed together and follow `hash` immediately
// rather than being separated from it by padding: with the old layout the
// group straddled a cache line boundary and make_move touched two lines to
// update state that fits comfortably in one.
struct board_t
{
  bb_t bitboards[12];
  bb_t occupancies[3];

  // Redundant with the bitboards, kept in sync by the same code that xors
  // them. Answers "what is on this square" in one load instead of a scan.
  piece_t squares[64];

  hash_t hash;  // Zobrist Key

  // Maintained by make_move and unmake_move rather than recomputed. evaluate()
  // was 40% of the search when it rebuilt these by walking the bitboards on
  // every call, which quiescence does at every node. All four are White
  // relative; `phase` counts both sides and is clamped by game_phase().
  int32_t material;
  int32_t psqt_mg;
  int32_t psqt_eg;
  int32_t phase;

  color_t active_color;       // Side to move
  uint8_t castling;           // Castling permissions
  uint8_t halfmove_clock;     // Moves with respect to the 50 move draw rule
  index_t en_passant;         // Active en-passant square index, if any
  uint16_t fullmove_counter;  // Total number of full moves played
};


// Everything unmake_move() cannot recompute from the move itself. The pieces,
// the occupancies and the square array are undone by re-applying the same xors
// make_move() applied, so none of them are stored here.
//
// `hash` doubles as the repetition history: it is the key of the position this
// move was played from, which is exactly what a threefold test looks for.
struct history_entry_t
{
  hash_t hash;
  move_t move;
  piece_t captured;  // piece taken off the target square, EMPTY if none
  uint8_t castling;
  index_t en_passant;
  uint8_t halfmove_clock;
};


struct history_t
{
  history_entry_t entries[HISTORY_MAX_SIZE];
  size_t size = 0;
};


// The attack tables are not in here. They are 2.3 MB of constants that are
// identical for every game, so one shared instance serves all of them - see
// game_tables() in bitboard.hpp. Keeping them per game made sizeof(game_t)
// 2.5 MB, which means a search thread cannot cheaply own a board.
struct game_t
{
  board_t board;
  history_t history;
  zobrist_randoms_t hash_randoms;
};


struct pv_t
{
  size_t length;
  move_t table[MAX_PLY];
};


enum node_type_t
{
  TT_EMPTY_NODE,
  TT_PV_NODE,  // The stored score is EXACTLY that.
               // Alpha node.  Every move you search will have a value less than
               // or equal to alpha, meaning that none of the moves in here will
               // be any good, probably because the starting position is bad for
               // the side to move.

  TT_ALPHA_NODE,  // The stored score was at most that. Upperbound. Fail-low
                  // Alpha node.  Every move you search will have a value less
                  // than or equal to alpha, meaning that none of the moves in
                  // here will be any good, probably because the starting
                  // position is bad for the side to move.

  TT_BETA_NODE  // The stored score was at least that. Lowerbound. Fail-high
                // Beta node.  At least one of the moves will return a score
                // greater than or equal to beta.
};


struct tt_entry_t
{
  uint64_t key;
  int32_t score;
  move_t best_move;
  int16_t depth;
  uint8_t type;        // node_type_t
  uint8_t generation;  // search that wrote it; 0 means never written
};


struct transposition_table_t
{
  uint8_t generation = 0;

  // entry_count is always a power of two, so the index is a mask instead of a
  // division on every probe.
  size_t entry_count = 0;
  size_t index_mask = 0;

  tt_entry_t* entries = nullptr;
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
  // Set when the search gave up mid-tree. Everything above the abort point
  // must be discarded rather than stored.
  bool aborted = false;
  uint64_t explored_nodes;
  uint64_t node_limit = NODE_BUDGET_UNLIMITED;
  move_t killer_moves[2][MAX_PLY];
  int history_moves[12][64];  // [piece][destination]
  transposition_table_t* tt;
  move_t best_move;

  // Triangular PV table: pv_table[ply] holds the PV from that ply onward.
  move_t pv_table[MAX_PLY][MAX_PLY];
  size_t pv_length[MAX_PLY];

  // Countermove heuristic: best quiet reply to each (piece, to-square) pair.
  move_t counter_moves[12][64];
};
