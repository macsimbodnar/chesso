#include "search.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

// static constexpr int MIN = std::numeric_limits<int>::min() + 100;
// static constexpr int MAX = std::numeric_limits<int>::max() - 100;
static constexpr int MIN = -2000000000;
static constexpr int MAX = 2000000000;


#define NULL_MOVE_REDUCTION 2
#define LMR_WHEN_START_IN_THE_LIST 4
#define LMR_START_AT_DEPTH 3


inline int normalize_score(int score, int ply)
{
  if (score > MATE_MIN && score < MATE_MAX) { return score + ply; }
  if (score < -MATE_MIN && score > -MATE_MAX) { return score - ply; }
  return score;
}


inline int de_normalize_score(int score, int ply)
{
  if (score > MATE_MIN && score < MATE_MAX) { return score - ply; }
  if (score < -MATE_MIN && score > -MATE_MAX) { return score + ply; }
  return score;
}


int quiescence(int alpha,
               int beta,
               size_t ply,
               game_t* game,
               search_state_t* state)
{
  assert(game != nullptr);

  const int stand_pat = ((game->board.active_color == WHITE) ? +1 : -1) *
                        evaluate(&game->tables, &game->board);

  state->explored_nodes++;

  if (stand_pat >= beta) { return beta; }
  if (alpha < stand_pat) { alpha = stand_pat; }

  // Time management
  if ((state->explored_nodes % 1000 == 0) && *state->stop) { return stand_pat; }
  if (ply > (MAX_PLY - 2)) { return stand_pat; }

  move_t moves[MAX_MOVES];
  const size_t n = generate_moves(&game->tables, &game->board, moves);

  order_captures(&game->board, moves, n);

  for (size_t i = 0; i < n; ++i) {
    if (!MOVE_CAPTURE(moves[i])) { continue; }
    if (!make_move(game, moves[i])) { continue; }

    const int s = -quiescence(-beta, -alpha, ply + 1, game, state);

    unmake_move(game);

    if (s >= beta) return beta;
    if (s > alpha) alpha = s;
  }

  return alpha;
}


int negamax(int alpha0,
            int beta,
            int depth,
            size_t ply,
            game_t* game,
            search_state_t* state)
{
  assert(game != nullptr);
  assert(state != nullptr);

  int best_so_far = MIN;
  int alpha = alpha0;

  // Reuse TT entry if found
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game->board);
  if (ply > 0 && tt_entry != nullptr && tt_entry->depth >= depth) {
    const int tt_score = de_normalize_score(tt_entry->score, ply);
    if (tt_entry->type == TT_PV_NODE) {
      state->best_move = tt_entry->best_move;
      return tt_score;
    } else if (tt_entry->type == TT_BETA_NODE && tt_score >= beta) {
      return tt_score;
    } else if (tt_entry->type == TT_ALPHA_NODE && tt_score <= alpha) {
      return tt_score;
    }
  }

  // Check for repetitions
  if (ply > 0 && is_position_repeated(&game->repetitions, game->board.hash)) {
    return DRAW_SCORE;
  }

  const bool is_in_check = is_check(game);

  if (is_in_check) { ++depth; }

  // Time management
  if ((state->explored_nodes % 1000 == 0) && *state->stop) {
    return (game->board.active_color == WHITE ? 1 : -1) *
           evaluate(&game->tables, &game->board);
  }

  if (depth < 1 || ply > (MAX_PLY - 2)) {
    return quiescence(alpha, beta, ply, game, state);
  }

  state->explored_nodes += 1;
  node_type_t type = TT_ALPHA_NODE;
  state->pv_length[ply] = 0;

  // Null move pruning:
  // Give the opponent a free move and see if they can still
  // beat beta. If yes, the position is so good we can prune this branch.
  // Skip when in check, at root, or with insufficient material (zugzwang risk).
  if (!is_in_check && ply > 0 && depth >= NULL_MOVE_REDUCTION + 1) {
    const bb_t side_pieces =
        (game->board.active_color == WHITE)
            ? (game->board.bitboards[W_KNIGHT] |
               game->board.bitboards[W_BISHOP] | game->board.bitboards[W_ROOK] |
               game->board.bitboards[W_QUEEN])
            : (game->board.bitboards[B_KNIGHT] |
               game->board.bitboards[B_BISHOP] | game->board.bitboards[B_ROOK] |
               game->board.bitboards[B_QUEEN]);

    if (side_pieces) {
      const index_t saved_ep = game->board.en_passant;
      set_en_passant(game, INVALID_INDEX);
      swap_side(game);

      const int null_score =
          -negamax(-beta, -beta + 1, depth - 1 - NULL_MOVE_REDUCTION, ply + 1,
                   game, state);

      swap_side(game);
      set_en_passant(game, saved_ep);

      if (null_score >= beta) { return beta; }
    }
  }

  int legal_moves_counter = 0;
  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_moves(&game->tables, &game->board, moves);

  order_moves(&game->board, state, ply, moves_count, moves);

  move_t best_move = moves[0];

  for (size_t i = 0; i < moves_count; ++i) {
    if (!make_move(game, moves[i])) { continue; }

    const bool is_capture = MOVE_CAPTURE(moves[i]);
    const bool is_check_move = is_check(game);

    legal_moves_counter++;

    // Late Move Reductions:
    // Search later moves at reduced depth.
    // If the reduced search beats alpha, re-search at full depth.
    const bool do_lmr =
        (i >= LMR_WHEN_START_IN_THE_LIST && depth >= LMR_START_AT_DEPTH &&
         !is_in_check && !is_check_move && should_reduce_move(moves[i]));

    int score;
    if (do_lmr) {
      score = -negamax(-alpha - 1, -alpha, depth - 2, ply + 1, game, state);
      if (score > alpha) {
        score = -negamax(-beta, -alpha, depth - 1, ply + 1, game, state);
      }
    } else {
      score = -negamax(-beta, -alpha, depth - 1, ply + 1, game, state);
    }

    unmake_move(game);

    // Found better scores
    if (score >= best_so_far) { best_so_far = score; }

    if (score >= beta) {
      // Fail-high
      type = TT_BETA_NODE;

      // Store killing move and history
      if (!is_capture && !is_check_move) {
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];

        const int bonus = depth * depth;
        state->history_moves[MOVE_PIECE(moves[i])][MOVE_TO(moves[i])] += bonus;
      }

      break;
    }

    if (score > alpha) {
      alpha = score;
      best_move = moves[i];
      type = TT_PV_NODE;

      // Update triangular PV table
      state->pv_table[ply][0] = moves[i];
      memcpy(&state->pv_table[ply][1], state->pv_table[ply + 1],
             state->pv_length[ply + 1] * sizeof(move_t));
      state->pv_length[ply] = 1 + state->pv_length[ply + 1];
    }
  }

  if (legal_moves_counter == 0) {
    return is_in_check ? -(MATE_MAX - ply) : DRAW_SCORE;
  }

  const int result = (best_so_far != MIN) ? best_so_far : (alpha0 - 1);
  state->best_move = best_move;

  // Store the node in TT
  // (void)type;
  const int to_store = normalize_score(result, ply);
  tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move);

  return result;
}


search_t search(int depth, game_t* game, search_state_t* state)
{
  assert(game != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);

  search_t search_result = {};

  // Aspiration windows: try a narrow window around the previous depth's score.
  // Widen to the full window if we get a fail-low or fail-high.
  int score;
  constexpr int ASPIRATION_DELTA = 50;
  if (depth > 1 && state->prev_score != 0) {
    const int lo = state->prev_score - ASPIRATION_DELTA;
    const int hi = state->prev_score + ASPIRATION_DELTA;
    score = negamax(lo, hi, depth, 0, game, state);
    if (score <= lo || score >= hi) {
      score = negamax(MIN, MAX, depth, 0, game, state);
    }
  } else {
    score = negamax(MIN, MAX, depth, 0, game, state);
  }
  state->prev_score = score;

  search_result.best_move = state->best_move;

  // Handle mate score
  search_result.mate_found = false;

  if (score >= -MATE_MAX && score <= -MATE_MIN) {
    search_result.mate_found = true;
    search_result.mate_in = -(score + MATE_MAX) / 2 - 1;
  }

  if (score >= MATE_MIN && score <= MATE_MAX) {
    search_result.mate_found = true;
    search_result.mate_in = (MATE_MAX - score) / 2 + 1;
  }

  search_result.explored_nodes = state->explored_nodes;
  search_result.score = score;
  search_result.pv.length = state->pv_length[0];
  memcpy(search_result.pv.table, state->pv_table[0],
         state->pv_length[0] * sizeof(move_t));


  // #ifndef NDEBUG
  //   const bool legal = is_pv_legal(game, &search_result.pv);
  //   if (!legal) { LOG_E << "Illegal PV" << END_E; }

  //   if (search_result.best_move != search_result.pv.pv_table[0][0]) {
  //     LOG_E << "Best move is not the same in the PV table" << END_E;
  //     LOG_E << "Best move: " << print_move(search_result.best_move) << END_E;
  //     LOG_E << "PV[0][0]:  " << print_move(search_result.pv.pv_table[0][0])
  //           << END_E;
  //   }

  // #endif

  //   assert(search_result.best_move == search_result.pv.pv_table[0][0]);

  return search_result;
}
