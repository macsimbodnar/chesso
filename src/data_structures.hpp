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
// The fixed positions the engine is developed and self-tested against. Each is
// named for what it holds, so that a bench or test line says which case it is
// reporting on.
//
// KIWIPETE_POS is the perft position the Chess Programming Wiki publishes under
// that name -- castling both ways for both sides, an en-passant-free but pin-
// and check-heavy tree -- and `tools/search_bench.py` already calls it that.
// BLOCKED_CENTRE_POS is named for its placement, checked with python-chess
// 2026-09-12 and not judged by eye: the square in front of each of the four
// central pawns is occupied, d3 against d4 and e4 against e5, so all four are
// immobile while every piece of both sides is still on the board.
//
// S211 gave those two their names. The first was TRICKY_POS, which said
// nothing; the second was named after the author of the tutorial series the
// `bitboard` branch followed, which is not a thing this project's source
// should carry. 2026-09-10_adversarial-F02.
//
// EMPTY_POS, the bare board, was here until S223 and is gone with the
// `position empty` shortcut that was its only caller: the load boundary
// requires one king of each colour, so a constant no loader accepts is a
// constant nothing can use. `2026-09-12_adversarial-F01`, DEC-197.
// clang-format off
#define DEFAULT_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define KIWIPETE_POS "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define KILLER_POS "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P4/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define BLOCKED_CENTRE_POS "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9"
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

// The move buffer, and what actually defends it. Every caller declares
// move_t[MAX_MOVES] on the stack and src/search.cpp negamax_at appends
// captures and quiets into one such array, so an overrun is a stack smash and
// not a dropped move.
//
//   218  the largest number of legal moves any legal position is known to
//        allow -- the published maximum, a bound on legal chess.
//   224  the largest count the 2026-09-10 audit's maximiser found while
//        searching placements constrained to 16 pieces a side. **A search
//        result, not a proof**: nothing says 224 is the maximum under that
//        rule, only that the maximiser did not beat it.
//   270  this buffer. 46 of headroom over the 224 above.
//
// So 270 is defended by a measurement plus the load boundary that makes the
// measurement apply: src/bitboard.cpp load_FEN() refuses more than 16 pieces
// of one colour, which is what stops an unconstrained placement reaching the
// generator (S208, 2026-09-10_adversarial-F09 -- a 27-piece placement
// generated 277 moves and aborted the Release binary). **Re-run the maximiser
// if the generator changes**, and if the honest bound is ever needed, it is a
// bound on the move count rather than on the piece count: DEC-177 records why
// that form was not taken here.
#define MAX_MOVES 270
#define MAX_PLY 128
#define MAX_DEPTH (MAX_PLY - 2)  // Must be +2 in order to be safe

// Holds the game moves replayed by [position ... moves ...] plus MAX_PLY of
// search on top. 5000 is far past the longest game the 75-move rule allows,
// and keeps game_t at a few megabytes instead of 150.
#define HISTORY_MAX_SIZE 5000
#define NODE_BUDGET_UNLIMITED 0

// The halfmove clock is 8 bits and saturates here instead of wrapping. See
// board_t below for why the field is not widened and why 255 is enough.
#define HALFMOVE_CLOCK_MAX 255

// The longest game prefix [position ... moves ...] may leave on the board.
//
// make_move() refuses once the history holds HISTORY_MAX_SIZE - 1 entries, so
// a `position` line stopped only by that guard leaves a board the search
// cannot push a single ply from: every root move is refused, first_legal_move()
// is refused too because it calls make_move(), and the engine answers
// `bestmove 0000` in a position with legal moves
// (2026-09-10_adversarial-F18). The command therefore keeps MAX_PLY of the
// history for the search, which is more than the search can use: no node makes
// a move at ply MAX_PLY - 1 or deeper (src/search.cpp), so the deepest line
// pushes MAX_PLY - 1 entries above the root.
//
// 4871 plies is 2435 moves, several times what the 75-move rule allows, so no
// game and no GUI meets this bound; an `absurdly long moves list` does. S210.
#define POSITION_MAX_PLIES (HISTORY_MAX_SIZE - MAX_PLY - 1)

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

  color_t active_color;  // Side to move
  uint8_t castling;      // Castling permissions

  // Moves with respect to the 50 move draw rule. **Saturates at
  // HALFMOVE_CLOCK_MAX, it does not wrap**, and every site that raises it says
  // so: 256 reversible plies -- reachable from a [position ... moves ...] list,
  // never from the search -- took this back to 0 and switched off both of its
  // consumers at once, the `>= 100` fifty-move test in negamax_at() and
  // classify_repetition()'s window min(halfmove_clock, history size).
  // 2026-09-10_adversarial-F17, S210.
  //
  // 8 bits and not 16 because the same field is in history_entry_t, one per
  // ply of an array HISTORY_MAX_SIZE long: there the record is exactly 16
  // bytes and widening any field of it pushes the record to 24 under an
  // 8-byte alignment, growing history_t from 80 KB to 120 KB and, worse,
  // costing classify_repetition() half its entries per cache line on a walk it
  // runs at every node. Saturation buys the same correctness for one cmov in
  // make_move. What the ceiling costs is a FEN field that reads 255 where the
  // line played 300, in a position the fifty-move rule called dead 155 plies
  // earlier.
  uint8_t halfmove_clock;

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

  // Saturating, like the board's. Sixteen bytes exactly, and board_t's comment
  // on this field is why it stays that way.
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
  // Free in space. The struct is 8-byte aligned for the key, and the fields
  // around this one come to 20 bytes of content in 24 -- 8 for the key, 4 for
  // the score, 4 for the move, 2 for the depth, 1 each for the type and the
  // generation -- so this lands in padding that was already being paid for and
  // the content is 22 bytes in 24 with it. sizeof(tt_entry_t) is 24 before and
  // after, and the entry count for a given Hash is untouched. tt_resize()
  // floors that count to a power of two as well, so anything from 17 to 32
  // bytes an entry would have produced the same count regardless.
  //
  // 16 bits is not a constraint anything real approaches. evaluate() is
  // material plus tapered tables: S213's case measured a maximum |evaluate()|
  // of 22945 over 66430 accepted generated placements (fifteen queens against
  // a lone king among them), against the 32767 this holds -- 1.4x headroom,
  // and the case asserts the score stays below MATE_MIN's 48000.
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

// `prune_rule_t` -- which rule of the shallow-depth block skipped a quiet, as
// the probe below records it.
//
// All four are in the list: late move pruning decides at the generation
// stage, with its own flag, but skips the move after `make_move` like the
// other three so that the gives-check exemption can bind (S109, DEC-180).
// `PRUNE_SEE` is the quiet rule and `PRUNE_SEE_CAPTURE` the capture one: two
// rules, two margins and two caps, so a probe that could not tell them apart
// would leave a case unable to say which one decided (S091).
enum prune_rule_t
{
  PRUNE_NONE = 0,
  PRUNE_FUTILITY = 1,
  PRUNE_HISTORY = 2,
  PRUNE_SEE = 3,
  PRUNE_LATE_MOVE = 4,
  PRUNE_SEE_CAPTURE = 5
};


struct search_node_probe_t
{
  // The ply to record. Nothing is recorded at any other ply, and -1 -- what a
  // value-initialised probe holds -- records nothing at all.
  int ply = -1;

  // The null-move block passed the move and searched the child. Not "a cutoff
  // was taken": the guards decide whether the pass happens, and a pass that
  // fails low is still a pass.
  bool null_move_made = false;

  // The type this node predicted for the child it searched after the pass,
  // valid only where `null_move_made` is true. Two published rules disagree
  // about it (src/search.cpp `null_move_child`), so the one this engine
  // applies is recorded rather than inferred. S098.
  bool null_child_is_pv = false;
  bool null_child_cut_node = false;

  // Reverse futility returned its bound instead of searching a move.
  bool rfp_cutoff = false;

  // One entry per legal move this node searched, in the order the node
  // searched them, so index k is the move whose legal_moves_counter was k + 1
  // -- which is the number the reduction table is indexed by.
  //
  // `reduction` is what came off the first search of the move, and 0 is what a
  // guard that refused the reduction leaves there. `researched` is the
  // zero-window repeat a reduced move that beat alpha is owed -- at
  // `child_depth` or a ply past it since S098 verdict 3, and no longer always
  // at the depth the move would otherwise have got.
  //
  // `child_is_pv` and `child_cut_node` are the type this node predicted for
  // that first search, so a case can read the alternation off the site that
  // applies it and not only off the rule as a function. S098.
  //
  // `research_depth` is the depth that repeat actually ran at and the three
  // numbers beside it are the inputs it was decided on, all valid only where
  // `researched` is true: the reduced search's score, the node's alpha at that
  // moment and its fail-soft best before this move. Recorded rather than
  // recomputed so a case can replay `lmr_research_depth` on the node's own
  // inputs and compare -- a site that stopped consulting the rule reads as a
  // disagreement and not as a number that happens to look plausible. S098
  // verdict 3. `research_alpha` is kept although the rule compares against
  // nothing but the fail-soft best: it is the site's own precondition and it is
  // the variable the margin could be measured from by mistake, which is the bug
  // the field below exists to catch.
  //
  // `child_depth` is the depth that first search actually ran at before the
  // reduction was taken off it, which is `depth - 1` at every move but the one
  // S097's verification search called singular: that one is searched a ply
  // deeper, and an extension landing on the wrong move is silent -- no crash,
  // no wrong node count, only rating -- so the site's own number is recorded
  // rather than inferred from the tree it left behind. S097.
  int move_count = 0;
  move_t moves[MAX_MOVES];
  int reduction[MAX_MOVES];
  int child_depth[MAX_MOVES];
  bool researched[MAX_MOVES];
  bool child_is_pv[MAX_MOVES];
  bool child_cut_node[MAX_MOVES];
  int research_depth[MAX_MOVES];
  int research_score[MAX_MOVES];
  int research_alpha[MAX_MOVES];
  int research_best[MAX_MOVES];

  // The base the rule reports having measured the deeper margin from, echoed
  // out of `lmr_research_depth` rather than recomputed here. It must equal
  // `research_best` at every site, and the two are written from different
  // places so that a call handing the rule the window instead of the node's own
  // best score is a disagreement a case can read. A replay cannot see that bug:
  // it moves both sides of its comparison together. S098 verdict 3.
  int research_base[MAX_MOVES];

  // The singular extension block, S097, and what it decided at this node.
  //
  // `se_verified` is "the verification search ran here", which is the whole of
  // the gate list: a case that plants an entry and asserts no verification
  // happened is reading this field and not a node count. The five beside it are
  // valid only where it is true -- whether the table move was extended, whether
  // the multicut returned the verification's score as the node's own, the
  // window that search was run against, the score it came back with, and the
  // depth it ran at.
  //
  // `se_multicut` is false in every release build the gate runs, because
  // `SE_MULTICUT` is 0 there and the compiler folds the branch away. That is
  // the point: the field is what the tune build's case reads to show the off
  // value is off rather than assumed (DEC-215).
  bool se_verified = false;
  bool se_extended = false;
  bool se_multicut = false;
  int se_singular_beta = 0;
  int se_vscore = 0;
  int se_vdepth = 0;

  // Late move pruning set its flag at this node, so the quiet stage ended
  // early -- either ungenerated or unsearched from the first quiet on.
  bool skip_quiets_set = false;

  // One entry per quiet the three per-move rules skipped, in the order they
  // were skipped, with the rule that did it. A pruned move never reaches
  // `moves` above: it is not counted as a legal move searched, which is the
  // whole reason the first-move guard exists. S109.
  int pruned_count = 0;
  move_t pruned_moves[MAX_MOVES];
  int pruned_rule[MAX_MOVES];
};


// The gravity bound on a continuation history entry, and the depth its graded
// update is expressed at. Both are definitions and not settings, which is why
// they are here and not in src/search_params.hpp -- that file's own header
// lists the class.
//
// CONT_HIST_BOUND is the entry type's own ceiling. It is fixed rather than
// tuned because a bound and a weight over the same table are one degree of
// freedom and not two: scaling the bound scales every entry, and only the
// product of the bound and ContHistWeight reaches the ordering. Fixing it at
// the widest value the storage admits leaves ContHistWeight carrying the whole
// of that product, and it is the strongest answer available to DEC-194's
// second suspect -- a table clipped by a bound it shares with another table
// cannot arise when the bound is the type's own. search_params.hpp's
// ContHistBonus comment carries the argument in full. S222.
inline constexpr int CONT_HIST_BOUND = 32767;

// The remaining depth ContHistBonus and ContHistMalus are quoted at: chesso's
// own median, recorded in search_params.hpp's RfpMaxDepth comment and measured
// at S085's control. Nothing about the mechanism depends on the number -- it
// is the unit the two shares are read in, and moving it would rescale them
// both by the same factor.
inline constexpr int CONT_HIST_REF_DEPTH = 11;

static_assert(CONT_HIST_BOUND == INT16_MAX,
              "cont_hist entries are int16_t and gravity holds them inside "
              "this bound, so a bound above the type's own would be a promise "
              "the storage cannot keep");


struct search_state_t
{
  std::atomic_bool* stop = nullptr;
  // Set when the search gave up mid-tree. Everything above the abort point
  // must be discarded rather than stored.
  bool aborted = false;
  uint64_t explored_nodes;
  uint64_t node_limit = NODE_BUDGET_UNLIMITED;

  // history_t::size at the moment search() was entered, which is the boundary
  // between the game and this search's own tree: the entry at this index is
  // the root position itself -- written when the search played its first move
  // -- so a match above it was pushed by the search and a match at or below it
  // is an occurrence from before or at the root. classify_repetition() wants
  // it; the search scores a two-fold as a draw only inside the tree. S207.
  //
  // search() sets it unconditionally and is the only way into ply 0, so the
  // default is seen only by a test driving negamax() directly. SIZE_MAX and
  // not 0 on purpose: an unset boundary then reads the whole history as
  // pre-root, which can only miss a draw, where 0 would score a two-fold from
  // the game before the root as one -- the defect S207 removed.
  size_t root_history_size = SIZE_MAX;

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

  // The last completed iteration's best move, supplied by the iterative
  // deepening loop and read at ply 0 only, and only when the root's table entry
  // is gone.
  //
  // The root has no move list of its own: score_move() orders it by the
  // tt_move and nothing else puts a move first, so "the first move the root
  // searches is the previous iteration's best" -- the sentence that licenses
  // keeping an aborted iteration's move -- held only while the root's entry
  // survived the iteration, which the replacement rule does not promise. This
  // makes it hold either way. 0 from every other caller, which is exactly the
  // behaviour they had before. 2026-09-04_adversarial-F03, S210.
  move_t root_move_hint = 0;

  // Triangular PV table: pv_table[ply] holds the PV from that ply onward.
  move_t pv_table[MAX_PLY][MAX_PLY];
  size_t pv_length[MAX_PLY];

  // Countermove heuristic: best quiet reply to each (piece, to-square) pair.
  move_t counter_moves[12][64];

  // One-ply continuation history -- countermove history, the CPW History
  // Heuristic page's Continuation History section, Geschwentner's device.
  // [previous move's piece][previous move's to][this move's piece][this move's
  // to], graded at every quiet cutoff and summed into the quiet ordering score
  // in score_move beside quiet_history. Reached only through
  // continuation_entry() below, so the write in history_on_quiet_cutoff and the
  // read in score_move cannot disagree about index order.
  //
  // 12 * 64 * 12 * 64 int16_t is 1.125 MiB. int16_t for quiet_history's own
  // reason: CONT_HIST_BOUND holds every entry inside this type's range, and
  // half the table is half the cache traffic.
  //
  // A value member and not a heap allocation behind a pointer: this struct is
  // rebuilt zeroed at the top of every iterative_deepening_search(), so
  // cont_hist clears exactly the way quiet_history and counter_moves already
  // do and `ucinewgame` needs no separate clear for any of the three.
  //
  // What that costs is where the struct may be built. 1.2 MiB does not fit the
  // 512 KiB a macOS std::thread gets by default, which the discarded MacBook
  // attempt at S024 is the record of. So every instance built off the main
  // thread is heap-owned by its builder -- iterative_deepening_search()'s one
  // per `go`, run_search()'s one per datagen worker. The tests build theirs on
  // the main thread, 8 MiB on both platforms.
  //
  // Guarded on the previous move existing, at ply 0 and at the node right
  // after a null move: both pass 0, and move 0 decodes to a legitimate
  // (W_PAWN, a8) cell rather than an out-of-range index, so a dropped guard
  // costs a silent wrong-cell write and not a crash. S222, and S024 before it.
  int16_t cont_hist[12][64][12][64];

  // Where a proved mate line is left for the searches that come after this
  // one. Null unless a caller supplies one, which keeps every direct caller of
  // search() -- every test that builds a search_state_t of its own -- on
  // exactly the behaviour it had before S170: nothing is stored and nothing is
  // read back. The UCI layer owns the instance because the store has to
  // outlive a `go`, which this struct does not.
  proven_mate_line_t* proven_mate = nullptr;

  // PER-ROOT-MOVE NODE ATTRIBUTION, S132. How many nodes the subtree under
  // each root move has cost, as two parallel arrays -- the move, and its
  // nodes -- filled at ply 0 only and reached through root_nodes_* below.
  //
  // Keyed on the **whole move encoding** and not on an index, because
  // pick_next_move() reorders the root's list in place and index i is a
  // different move from one iteration to the next; and not on (from, to)
  // either, because the four promotions of one pawn push share that pair and
  // the best move is allowed to be a promotion.
  //
  // It lives here because this struct is what survives a `go`: the iterative
  // deepening loop builds one per search and every iteration and every
  // aspiration re-search of that `go` writes into the same buckets, which is
  // the accumulation the published form reads -- the denominator is the whole
  // search's root nodes so far, not one iteration's.
  //
  // Zeroed with the struct, so a caller that never reaches ply 0 sees an
  // empty census rather than a stale one.
  move_t root_move_keys[MAX_MOVES];
  uint64_t root_move_nodes[MAX_MOVES];
  size_t root_move_count = 0;
};


// THE ONE KEY INTO THE ROOT BUCKETS, S132, for the reason continuation_entry()
// below is one function: the write in negamax's root branch and the two reads
// in iterative_deepening_search() cannot disagree about what identifies a root
// move if there is only one place that decides.
//
// Linear, and deliberately so. The root has at most MAX_MOVES moves, the scan
// runs once per root move per iteration -- never below ply 0 -- and a hash of
// a 32-bit key would cost more to write and to reason about than the whole
// thing saves. A root whose move list somehow overflows the array drops the
// overflow rather than writing past it: the fraction is then taken over fewer
// moves, which moves a time decision and never the tree.
inline void root_nodes_add(search_state_t* state, move_t move, uint64_t nodes)
{
  for (size_t i = 0; i < state->root_move_count; ++i) {
    if (state->root_move_keys[i] == move) {
      state->root_move_nodes[i] += nodes;
      return;
    }
  }

  if (state->root_move_count >= MAX_MOVES) { return; }

  state->root_move_keys[state->root_move_count] = move;
  state->root_move_nodes[state->root_move_count] = nodes;
  state->root_move_count++;
}


// What one root move has cost, 0 for a move this search never made.
inline uint64_t root_nodes_of(const search_state_t* state, move_t move)
{
  for (size_t i = 0; i < state->root_move_count; ++i) {
    if (state->root_move_keys[i] == move) { return state->root_move_nodes[i]; }
  }

  return 0;
}


// What every root move has cost together. This is the fraction's denominator
// and it is **not** the search's node count: the root's own node is outside
// every bucket by construction, which is what the sum identity in
// tests/test_search.cpp pins.
inline uint64_t root_nodes_total(const search_state_t* state)
{
  uint64_t total = 0;

  for (size_t i = 0; i < state->root_move_count; ++i) {
    total += state->root_move_nodes[i];
  }

  return total;
}


// The one index into cont_hist. Both the write in history_on_quiet_cutoff and
// the read in score_move go through one of these two overloads, so the two
// sites cannot disagree about order -- swapping the pairs inside here is a
// symmetric relabelling of the whole table and not a bug either site could
// observe on its own, which is the point of having one copy of the index
// rather than two.
//
// Keyed on MOVE_PIECE and not MOVE_FROM, matching counter_moves: a promotion's
// mover is the pawn standing on the source square and not the piece that ends
// up on the target, and counter_moves already commits to reading it that way.
// S222.
inline int16_t& continuation_entry(search_state_t* state,
                                   move_t prev_move,
                                   move_t move)
{
  return state->cont_hist[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]
                         [MOVE_PIECE(move)][MOVE_TO(move)];
}

inline int16_t continuation_entry(const search_state_t* state,
                                  move_t prev_move,
                                  move_t move)
{
  return state->cont_hist[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]
                         [MOVE_PIECE(move)][MOVE_TO(move)];
}
