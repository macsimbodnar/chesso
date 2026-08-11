#include "evaluation.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "bb_tables.hpp"
#include "bitboard.hpp"
#include "eval_tables.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"

// clang-format off

// Move-ordering piece values. These are deliberately not the evaluation's
// piece_value[] and they carry their own names so that the two cannot be
// confused or accidentally unified. They used to be spelled PAWN..QUEEN, the
// same names eval_tables.hpp defines, and only compiled because both sides
// happened to hold the same numbers.
//
// Ordering needs a stable ranking, not an accurate price, and the bands below
// clear each other by exactly 100 points: a king capturing a pawn scores
// ORDER_CAPTURE + MVV_PAWN - MVV_KING = 900100, against 900000 for a killer.
// Feed a fitted queen value of 1026 into that and the arithmetic still holds,
// but nothing in the tree would say so out loud if it stopped holding. The
// symptom of getting it wrong is a strength regression, not a wrong node count.
#define MVV_PAWN   100
#define MVV_KNIGHT 300
#define MVV_BISHOP 300
#define MVV_ROOK   500
#define MVV_QUEEN  900
#define MVV_KING   100000

#define ORDER_TT_MOVE 2000000
#define ORDER_CAPTURE 1000000
#define ORDER_KILLER_0 900000
#define ORDER_KILLER_1 800000
#define ORDER_COUNTER 700000

/* board representation */

static constexpr int piece_values_abs[] = {
    MVV_PAWN,  MVV_KNIGHT, MVV_BISHOP, MVV_ROOK, MVV_QUEEN, MVV_KING, MVV_PAWN,
    MVV_KNIGHT, MVV_BISHOP, MVV_ROOK,  MVV_QUEEN, MVV_KING, 0};


// clang-format on

// Everything here was already computed by make_move: the material balance, the
// two piece-square sums and the phase are maintained as pieces move rather than
// rebuilt from the bitboards. All that is left is the interpolation and the
// sign, which is why this function stopped being 40% of the search.
//
// The accumulators are White relative, so the sum is negated once at the end
// for Black. Doing it here rather than at every call site is what keeps a later
// term from picking up the wrong sign.
int evaluate_cheap(const board_t* board)
{
  // Interpolate the two tables on how much material is left, so a term slides
  // from its middlegame value to its endgame one instead of jumping when some
  // arbitrary piece comes off.
  const int phase = game_phase(board);
  const int positional =
      ((board->psqt_mg * phase) + (board->psqt_eg * (GAME_PHASE_MAX - phase))) /
      GAME_PHASE_MAX;

  const int score = board->material + positional;

  return (board->active_color == WHITE) ? score : -score;
}


// Mobility, and the reason this file now has two stages. It cannot be
// accumulated: it is a function of occupancy, not of a piece and its square, so
// moving any piece changes it for every slider whose ray crosses the from or
// the to square and there is no O(1) delta for make_move to apply. Measured at
// 56 ns per node in a real search, DEC-039, where the isolated benchmark said
// 14.6 -- the difference is cache misses in 2 MB of rook attack tables that a
// ten-position benchmark loop never pays.
//
// The weights are hand-picked from ordinary positional principles and no
// published table was consulted, DEC-016. They are not fitted yet; fitting them
// is the experiment after this one and is deliberately not bundled with it.
const int mobility_mg[4] = {0, 5, 9, 2};  // knight bishop rook queen
const int mobility_eg[4] = {-1, 6, 1, 3};

static int evaluate_mobility(const board_t* board)
{
  const bb_tables_t* tables = game_tables();
  const bb_t occupancy = board->occupancies[BOTH];

  int mg = 0;
  int eg = 0;

  for (int colour = 0; colour < 2; ++colour) {
    const int sign = (colour == WHITE) ? 1 : -1;
    const bb_t own = board->occupancies[colour];
    const int base = (colour == WHITE) ? W_KNIGHT : B_KNIGHT;

    for (int type = 0; type < 4; ++type) {
      bb_t pieces = board->bitboards[base + type];

      while (pieces) {
        const index_t square = get_lsb_index(pieces);
        pieces &= pieces - 1;

        bb_t attacks;
        switch (type) {
          case 0:
            attacks = tables->knight_attacks[square];
            break;
          case 1:
            attacks = get_bishop_attacks(tables, square, occupancy);
            break;
          case 2:
            attacks = get_rook_attacks(tables, square, occupancy);
            break;
          default:
            attacks = get_queen_attacks(tables, square, occupancy);
            break;
        }

        // Squares the piece could move to, own pieces excluded. Not safe
        // mobility: no enemy-pawn-attack mask, which is the expensive variant.
        const int count = count_bits(attacks & ~own);

        mg += sign * count * mobility_mg[type];
        eg += sign * count * mobility_eg[type];
      }
    }
  }

  const int phase = game_phase(board);
  int score = (mg * phase + eg * (GAME_PHASE_MAX - phase)) / GAME_PHASE_MAX;

  // Clamped so that the lazy shortcut's margin is a guarantee rather than a
  // hope. Without this the bound is an observation about the positions someone
  // sampled, and a position outside the sample silently makes the shortcut
  // unsound -- which is not hypothetical: the corpus contains
  // 1Bk5/B1B5/1B1B4/B1B4B/8/B1B5/5K2/8, nine white bishops from promotions,
  // where the term reached 155 against a margin of 150 and the test caught it.
  //
  // The clamp costs nothing in real play. Over 149084 positions of S028
  // self-play the term ran p99 81 and a maximum of 143, so it does not bind
  // there at all; it binds only in the promotion pile-ups where the number was
  // never meaningful anyway.
  score = std::clamp(score, -LAZY_EVAL_MARGIN, LAZY_EVAL_MARGIN);

  return (board->active_color == WHITE) ? score : -score;
}


int evaluate(const board_t* board)
{ return evaluate_cheap(board) + evaluate_mobility(board); }


int evaluate_lazy(const board_t* board, int alpha, int beta)
{
  const int cheap = evaluate_cheap(board);

  // Both tests are one-sided on purpose. If the cheap score is already a
  // margin clear of beta then the full score is above beta too, so the caller
  // fails high on either number and the expensive stage would change nothing it
  // does. Same argument mirrored at alpha. Anywhere between the two, the
  // correction can decide the node and has to be computed.
  if (cheap - LAZY_EVAL_MARGIN >= beta) { return cheap; }
  if (cheap + LAZY_EVAL_MARGIN <= alpha) { return cheap; }

  return cheap + evaluate_mobility(board);
}


int game_phase(const board_t* board)
{
  // Promotions can put more material on the board than the opening had.
  return (board->phase > GAME_PHASE_MAX) ? GAME_PHASE_MAX : board->phase;
}


inline piece_t captured_piece(const board_t* board, index_t square)
{
  assert(square < 64);
  return board->squares[square];
}


int capture_score(const board_t* board, move_t move)
{
  // An en-passant victim does not sit on the target square
  const piece_t victim =
      MOVE_EN_PASSANT(move) ? ((board->active_color == WHITE) ? B_PAWN : W_PAWN)
                            : captured_piece(board, MOVE_TO(move));

  // MVV-LVA
  assert(victim < sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));
  return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];
}


int score_move(const game_t* game,
               const search_state_t* state,
               move_t move,
               move_t tt_move,
               size_t ply,
               move_t prev_move)
{
  if (tt_move != 0 && move == tt_move) { return ORDER_TT_MOVE; }

  assert(static_cast<size_t>(MOVE_PROMOTED(move)) <
         sizeof(piece_values_abs) / sizeof(piece_values_abs[0]));

  // Promotion_t maps onto W_KNIGHT..W_QUEEN by construction.
  const int promotion_bonus =
      MOVE_PROMOTED(move) ? piece_values_abs[MOVE_PROMOTED(move)] : 0;

  if (MOVE_CAPTURE(move)) {
    return ORDER_CAPTURE + promotion_bonus + capture_score(&game->board, move);
  }

  if (promotion_bonus != 0) { return ORDER_CAPTURE + promotion_bonus; }

  if (move == state->killer_moves[0][ply]) { return ORDER_KILLER_0; }
  if (move == state->killer_moves[1][ply]) { return ORDER_KILLER_1; }

  if (prev_move != 0 &&
      move == state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]) {
    return ORDER_COUNTER;
  }

  // Saturated on the way in, so it can never reach the killer band.
  return state->history_moves[MOVE_PIECE(move)][MOVE_TO(move)];
}
