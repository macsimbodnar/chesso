#include "experimental_search.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include "board.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "move_generator.hpp"
#include "search.hpp"
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


int negamax(int alpha0,
            int beta,
            int depth,
            size_t ply,
            const board_t* board,
            search_state_t* state,
            bool zero_window)
{
  int best_so_far = MIN;
  int alpha = alpha0;


  // Reuse TT entry if found
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, board);
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

  state->explored_nodes += 1;
  state->pv.pv_length[ply] = ply;
  node_type_t type = TT_ALPHA_NODE;

  // Leaf node
  if (depth < 1) {
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  const bool is_in_check = is_check(board);
  if (moves_count == 0) { return is_in_check ? -(MATE_MAX - ply) : DRAW_SCORE; }

  order_moves(moves, moves_count, ply, state);

  move_t* best_move = &moves[0];

  // Null move pruning
  if (depth > NULL_MOVE_REDUCTION + 1 && !is_in_check && !zero_window) {
    board_t swapped_board = *board;
    swap_side(&swapped_board);
    clear_ep_square(&swapped_board);

    const int probe_score =
        -negamax(-beta, -beta + 1, depth - NULL_MOVE_REDUCTION - 1, ply + 1,
                 &swapped_board, state, true);

    if (probe_score >= beta) {
      // Verified null move pruning. Going 1 ply deeper
      const int verify_score =
          -negamax(-beta, -beta + 1, depth - NULL_MOVE_REDUCTION, ply + 1,
                   &swapped_board, state, true);

      if (verify_score >= beta) {
        //  Now it's safe to cut off
        return beta;
      }
    }
  }

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;
    // Reset the pv length
    state->pv.pv_length[ply + 1] = ply + 1;
    make_move(&moves[i], &tmp_board, nullptr);

    const bool move_gives_check = is_check(&tmp_board);
    const bool is_capture =
        (moves[i].captured != INVALID && moves[i].captured != EMPTY);

    int score = MIN;

    const bool do_LMR =
        (!zero_window && !is_capture && !move_gives_check &&
         i >= LMR_WHEN_START_IN_THE_LIST && depth >= LMR_START_AT_DEPTH);

    if (do_LMR) {  // LMR Logic
      const int reduction_factor = 1;

      // Shallow null window search at reduced depth
      score = -negamax(-alpha - 1, -alpha, depth - reduction_factor - 1,
                       ply + 1, &tmp_board, state, true);

      if (score > alpha) {
        // Full re-search if it looks promising
        score = -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state,
                         false);
      }
    } else {  // PVS Logic
      if (!zero_window && i > 0) {
        score = -negamax(-(alpha + 1), -alpha, depth - 1, ply + 1, &tmp_board,
                         state, true);

        if (score < alpha) { continue; }

        score = -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state,
                         false);

      } else {
        score = -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state,
                         zero_window);
      }
    }

    if (score >= best_so_far) { best_so_far = score; }

    if (score >= beta) {
      // Fail-high
      type = TT_BETA_NODE;

      // Store killing move and history
      if (!is_capture && !move_gives_check) {
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];

        const int bonus = depth * depth;
        state->history_moves[moves[i].piece][moves[i].to] += bonus;
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
  tt_store_entry(state->tt, board, depth, best_so_far, type, best_move);

  state->best_move = *best_move;
  return (best_so_far != MIN) ? best_so_far : (alpha0 - 1);
}


search_t experimental_search(int depth,
                             const board_t* board,
                             search_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);

  search_t search_result = {};

  const int score = negamax(MIN, MAX, depth, 0, board, state, false);

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
  is_pv_legal(board, &search_result.pv);
#endif

  assert(search_result.best_move == search_result.pv.pv_table[0][0]);

  return search_result;
}
