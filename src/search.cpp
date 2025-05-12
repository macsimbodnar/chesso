#include "search.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "transposition_table.hpp"


#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

static constexpr int MIN = std::numeric_limits<int>::min() + 100;
static constexpr int MAX = std::numeric_limits<int>::max() - 100;
// static constexpr int NO_SCORE = MAX + 42;

#define NULL_MOVE_REDUCTION 2
#define LMR_WHEN_START_IN_THE_LIST 4
#define LMR_START_AT_DEPTH 3


int quiescence(int alpha,
               int beta,
               size_t ply,
               game_t* game,
               search_state_t* state,
               index_t last_to)
{
  assert(game != nullptr);

  const int stand_pat =
      ((game->board.active_color == WHITE) ? +1 : -1) * evaluate(&game->board);

  state->explored_nodes++;

  // DELTA PRUNE:
  // if (stand_pat + get_max_gain() <= alpha) {
  if (stand_pat + 200 <= alpha) {
    // At this point no capture can improve te score so we just return
    return alpha;
  }

  if (stand_pat >= beta) { return beta; }
  if (alpha < stand_pat) { alpha = stand_pat; }

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) { return stand_pat; }
  if (ply > (MAX_PLY - 2)) { return stand_pat; }

  move_t moves[MAX_MOVES];
  const size_t n = generate_moves(&game->tables, &game->board, moves);

  order_captures(moves, n);

  for (size_t i = 0; i < n; ++i) {
    // We consider only the captures that recapture the last capture
    if (!MOVE_CAPTURE(moves[i])) { continue; }
    if (last_to != INVALID_INDEX && MOVE_TO(moves[i]) != last_to) { continue; }

    if (!make_move(game, moves[i])) { continue; }

    const int s =
        -quiescence(-beta, -alpha, ply + 1, game, state, MOVE_TO(moves[i]));

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
            search_state_t* state,
            bool zero_window,
            index_t last_to)
{
  assert(game != nullptr);
  assert(state != nullptr);

  int best_so_far = MIN;
  int alpha = alpha0;

  // Reuse TT entry if found
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game->board);
  if (ply > 0 && tt_entry != nullptr && tt_entry->depth >= depth) {
    if (tt_entry->type == TT_PV_NODE) {
      state->best_move = tt_entry->best_move;
      return tt_entry->score;
    } else if (tt_entry->type == TT_BETA_NODE && tt_entry->score >= beta) {
      return tt_entry->score;
    } else if (tt_entry->type == TT_ALPHA_NODE && tt_entry->score <= alpha) {
      return tt_entry->score;
    }
  }

  // Check for repetitions
  if (ply > 0 && is_position_repeated(&game->repetitions, game->board.hash)) {
    return DRAW_SCORE;
  }

  const bool is_in_check = is_check(game);

  if (is_in_check) { ++depth; }

  // Razoring
  // if (!is_in_check && depth == 1) {
  //   int stand_pat = (board->active_color == WHITE ? +1 : -1) *
  //   evaluate(board); const int razor_margin = get_margin_value();
  //   // const int razor_margin = 200;

  //   if (stand_pat + razor_margin < alpha) {
  //     // No quiet move can possibly raise the score above alpha
  //     return stand_pat;
  //   }
  // }

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) {
    return (game->board.active_color == WHITE ? 1 : -1) *
           evaluate(&game->board);
  }

  if (depth < 1 || ply > (MAX_PLY - 2)) {
    return quiescence(alpha, beta, ply, game, state, last_to);
  }

  state->explored_nodes += 1;
  state->pv.pv_length[ply] = ply;
  node_type_t type = TT_ALPHA_NODE;

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_moves(&game->tables, &game->board, moves);

  if (moves_count == 0) { return is_in_check ? -(MATE_MAX - ply) : DRAW_SCORE; }

  order_moves(&game->board, state, ply, moves_count, moves);

  move_t* best_move = &moves[0];

  // Null move pruning
  if (depth > NULL_MOVE_REDUCTION + 1 && !is_in_check && !zero_window) {
    swap_side(game);
    const index_t en_passant = game->board.en_passant;
    set_en_passant(game, INVALID_INDEX);

    const int probe_score =
        -negamax(-beta, -beta + 1, depth - NULL_MOVE_REDUCTION - 1, ply + 1,
                 game, state, true, INVALID_INDEX);

    if (probe_score >= beta) {
      // Verified null move pruning. Going 1 ply deeper
      const int verify_score =
          -negamax(-beta, -beta + 1, depth - NULL_MOVE_REDUCTION, ply + 1, game,
                   state, true, INVALID_INDEX);

      if (verify_score >= beta) {
        swap_side(game);
        set_en_passant(game, en_passant);

        //  Now it's safe to cut off
        return beta;
      }
    }

    swap_side(game);
    set_en_passant(game, en_passant);
  }

  for (size_t i = 0; i < moves_count; ++i) {
    // TODO: Implement this check
    // if (moves[i].captured == (board->active_color == WHITE ? B_KING :
    // W_KING)) {
    //   state->best_move = moves[i];

    //   state->pv.pv_table[ply][ply] = moves[i];
    //   state->pv.pv_length[ply] = ply + 1;

    //   return MATE_MAX - ply;
    // }

    state->pv.pv_length[ply + 1] = ply + 1;

    if (!make_move(game, moves[i])) { continue; }

    const bool is_capture = MOVE_CAPTURE(moves[i]);
    const bool is_check_move = is_check(game);

    // Decide depth reduction R
    int R = 0;
    if (!zero_window && !is_capture && !is_check_move &&
        i >= LMR_WHEN_START_IN_THE_LIST && depth >= LMR_START_AT_DEPTH) {
      R = 1;  // TODO: Compute dynamically
    }

    // Decide if this is “first full” move in PVS
    const bool pvs = (!zero_window && i > 0);

    // Compute the search window
    const int low = pvs ? -(alpha + 1) : -beta;
    const int high = pvs ? -alpha : -alpha;
    const int new_depth = depth - 1 - R;
    const bool zw = R > 0 || pvs;

    // Probe search
    int score = -negamax(low, high, new_depth, ply + 1, game, state, zw,
                         MOVE_TO(moves[i]));

    // If the probe suggests it might raise alpha, do a full‐window re‐search
    const bool LMR_probe_beat_alpha = (R > 0 && score > alpha);
    const bool PVS_in_ab_interval =
        (R == 0 && pvs && score > alpha && score < beta);


    if (LMR_probe_beat_alpha || PVS_in_ab_interval) {
      score = -negamax(-beta, -alpha, depth - 1, ply + 1, game, state, false,
                       MOVE_TO(moves[i]));
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
      best_move = &moves[i];

      // Save principal variation
      state->pv.pv_table[ply][ply] = *best_move;

      memcpy(&state->pv.pv_table[ply][ply + 1],
             &state->pv.pv_table[ply + 1][ply + 1],
             (state->pv.pv_length[ply + 1] - (ply + 1)) *
                 sizeof(state->pv.pv_table[0][0]));

      state->pv.pv_length[ply] = state->pv.pv_length[ply + 1];

      type = TT_PV_NODE;
    }
  }

  // Store the node in TT
  tt_store_entry(state->tt, &game->board, depth, best_so_far, type, best_move);

  state->best_move = *best_move;
  return (best_so_far != MIN) ? best_so_far : (alpha0 - 1);
}


search_t search(int depth, game_t* game, search_state_t* state)
{
  assert(game != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);

  search_t search_result = {};

  const int score =
      negamax(MIN, MAX, depth, 0, game, state, false, INVALID_INDEX);

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
  search_result.pv = state->pv;
  search_result.score = score;


#ifndef NDEBUG
  is_pv_legal(game, &search_result.pv);
#endif

  assert(search_result.best_move == search_result.pv.pv_table[0][0]);

  return search_result;
}
