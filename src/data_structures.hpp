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


// The last mate line this engine has been shown to deliver, kept so that a
// later search reporting the same mate can publish a line for it.
//
// It exists because a mate score outlives the line that proves it. The score
// is one 32-bit field of one table entry and survives; the entries carrying
// the plies below it are a whole line's worth of slots and are overwritten
// within a search or two, so a search that reads the score back has nothing
// left to walk. Measured in S147's 3000-game run: 10 `info` lines claiming a
// mate whose line no table walk could complete.
//
// `keys[i]` is the position `moves[i]` is played from, which is what locates a
// later root inside the line: a search standing on `keys[i]` and still owing
// exactly `length - i` plies is on this line at that point, and the rest of it
// is the line it owes. Both halves of that test matter -- the key says where,
// the remaining length says the stored proof is of the distance being claimed
// and not of another one.
//
// REPORTING STATE, NEVER SEARCH STATE. Nothing reads this to decide a move, to
// order one, or to prune. It is written after a search returns and read only
// while a reported line is being completed, and what it offers is put through
// the same all-or-nothing gate a table move is (DEC-122). S170.
struct proven_mate_line_t
{
  size_t length = 0;
  hash_t keys[MAX_PLY];
  move_t moves[MAX_PLY];
};


enum node_type_t
{
  TT_EMPTY_NODE,
  TT_PV_NODE,  // The stored score is EXACTLY that. Exact. The node's value
               // landed strictly inside the window it was searched with, so
               // there is a real score here and not a bound, and it answers
               // whatever window a later node reads it from.
               //
               // The four lines that used to sit here were a pasted copy of
               // TT_ALPHA_NODE's text, describing the opposite thing. S106.

  TT_ALPHA_NODE,  // The stored score was at most that. Upperbound. Fail-low
                  // Alpha node.  Every move you search will have a value less
                  // than or equal to alpha, meaning that none of the moves in
                  // here will be any good, probably because the starting
                  // position is bad for the side to move.

  TT_BETA_NODE  // The stored score was at least that. Lowerbound. Fail-high
                // Beta node.  At least one of the moves will return a score
                // greater than or equal to beta.
};


// No static evaluation was recorded with this entry, so a consumer has to
// compute one or do without. INT16_MIN and not 0, because 0 is an ordinary
// score: a balanced position evaluates to it several times a search.
#define TT_EVAL_NONE INT16_MIN

struct tt_entry_t
{
  uint64_t key;
  int32_t score;
  move_t best_move;
  int16_t depth;

  // The static evaluation of this position, or TT_EVAL_NONE where no node that
  // wrote the entry ever had one to record. Not the same number as `score`,
  // which is what the search returned; this is what the position was worth
  // before anything below it was looked at. S094.
  //
  // "No node that wrote it" and not "the node that wrote it": a store carrying
  // TT_EVAL_NONE over an entry for this same position keeps the number that is
  // there rather than erasing it, since an evaluation is a property of the
  // position and not of the visit that recorded it. Across a key change it is
  // erased -- see tt_eval_to_store(). S108.
  //
  // Free in space. The struct is 8-byte aligned for the key and was 20 bytes
  // of content in 24, so this lands in padding that was already being paid
  // for: sizeof(tt_entry_t) is 24 before and after, and the entry count for a
  // given Hash is untouched. tt_resize() floors that count to a power of two
  // as well, so anything from 17 to 32 bytes an entry would have produced the
  // same count regardless.
  //
  // 16 bits is not a constraint anything real approaches. evaluate() is
  // material plus tapered tables, and a board of nine queens comes to a few
  // thousand centipawns against the 32767 this holds.
  int16_t eval;

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


// The pruning and reduction decisions of exactly one node, recorded so a test
// can watch a guard hold. S191.
//
// Null move pruning, reverse futility and late move reduction all decide *not*
// to do something, and nothing they decide is visible from outside the search:
// a node that refused its null move and a node that never had the option
// return the same score through the same table. That is why removing five of
// those guards one at a time was caught by a single golden count and by
// nothing else (2026-09-04_test_review-F02) -- a test that infers a guard from
// the shape of a tree is a test that goes green once the guard is gone.
//
// Write-only, and that is the whole of its safety argument: negamax never
// reads a field of this struct, so the tree a probed search explores is the
// tree it explores without one. The pointer in search_state_t is null in every
// caller but a test, and INV-6 at depths 9 and 12 is what holds the claim.
struct search_node_probe_t
{
  // The ply to record. Nothing is recorded at any other ply, and -1 -- what a
  // value-initialised probe holds -- records nothing at all.
  int ply = -1;

  // The null-move block passed the move and searched the child. Not "a cutoff
  // was taken": the guards decide whether the pass happens, and a pass that
  // fails low is still a pass.
  bool null_move_made = false;

  // Reverse futility returned its bound instead of searching a move.
  bool rfp_cutoff = false;

  // One entry per legal move this node searched, in the order the node
  // searched them, so index k is the move whose legal_moves_counter was k + 1
  // -- which is the number the reduction table is indexed by.
  //
  // `reduction` is what came off the first search of the move, and 0 is what a
  // guard that refused the reduction leaves there. `researched` is the
  // full-depth repeat a reduced move that beat alpha is owed.
  int move_count = 0;
  move_t moves[MAX_MOVES];
  int reduction[MAX_MOVES];
  bool researched[MAX_MOVES];
};


struct search_state_t
{
  std::atomic_bool* stop = nullptr;
  // Set when the search gave up mid-tree. Everything above the abort point
  // must be discarded rather than stored.
  bool aborted = false;
  uint64_t explored_nodes;
  uint64_t node_limit = NODE_BUDGET_UNLIMITED;

  // Attached by a test that needs to see one node's pruning and reduction
  // decisions. Null everywhere else, and written through but never read by the
  // search, so the tree is the same tree with one attached. S191.
  //
  // Only `negamax_at<true>` ever loads it, so where it sits costs the engine
  // nothing -- but it took a measurement to stop caring. Read at every node,
  // as the first version did, this field cost **1.49 % of nodes per second,
  // sd 0.66 % over 13 interleaved pairs of `chesso bench`**, and moving it up
  // here beside the fields every node already touches did not recover it. What
  // recovered it was making the read compile-time: 0.14 % +/- 0.24 % at 95 %
  // over 33 pairs, which is nothing this machine can resolve.
  search_node_probe_t* probe = nullptr;
  move_t killer_moves[2][MAX_PLY];

  // The static evaluation of the node at each ply, TT_EVAL_NONE where the node
  // was in check and never computed one. Written by every negamax node that
  // recurses, so a node at ply p can read its own ancestors' numbers; the
  // ancestor wrote its slot before it could reach the recursion that produced
  // this node, which is what makes the ply arithmetic in improving_at() a
  // sufficient guard.
  //
  // Never guard a read on the value: an unwritten slot holds 0 here, and 0 is
  // an ordinary evaluation. Guard on the ply instead. S108.
  int static_evals[MAX_PLY];

  // Butterfly history: [side to move][from][to], Hartmann 1988. It was
  // [piece][destination] until S093, which conflates a knight on b1 with one on
  // g1 going to the same square and separates two pieces of different type
  // going the same way. int16_t because QuietHistoryMax bounds every entry to
  // this type's range and 16 KB of table is cheaper to touch than 32.
  int16_t quiet_history[2][64][64];
  transposition_table_t* tt;
  move_t best_move;

  // Triangular PV table: pv_table[ply] holds the PV from that ply onward.
  move_t pv_table[MAX_PLY][MAX_PLY];
  size_t pv_length[MAX_PLY];

  // Countermove heuristic: best quiet reply to each (piece, to-square) pair.
  move_t counter_moves[12][64];

  // Where a proved mate line is left for the searches that come after this
  // one. Null unless a caller supplies one, which keeps every direct caller of
  // search() -- every test that builds a search_state_t of its own -- on
  // exactly the behaviour it had before S170: nothing is stored and nothing is
  // read back. The UCI layer owns the instance because the store has to
  // outlive a `go`, which this struct does not.
  proven_mate_line_t* proven_mate = nullptr;
};
