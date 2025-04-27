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


int negamax(int alpha,
            int beta,
            int depth,
            size_t ply,
            const board_t* board,
            search_state_t* state)
{
  int best_so_far = MIN;
  const int alpha0 = alpha;


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

  // Leaf node
  if (depth < 1) {
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  const bool is_in_check = is_check(board);
  if (moves_count == 0) { return is_in_check ? -(MATE_MAX - ply) : DRAW_SCORE; }

  order_moves(moves, moves_count, ply, state);

  move_t best_move = moves[0];

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    // Reset the pv length
    state->pv.pv_length[ply + 1] = ply + 1;

    make_move(&moves[i], &tmp_board, nullptr);
    const int score =
        -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state);

    if (score >= best_so_far) {
      best_so_far = score;
      best_move = moves[i];

      if (score >= alpha) {
        alpha = score;

        // Save principal variation
        state->pv.pv_table[ply][ply] = best_move;

        memcpy(&state->pv.pv_table[ply][ply + 1],
               &state->pv.pv_table[ply + 1][ply + 1],
               (state->pv.pv_length[ply + 1] - (ply + 1)) *
                   sizeof(state->pv.pv_table[0][0]));

        state->pv.pv_length[ply] = state->pv.pv_length[ply + 1];
      }

      if (score >= beta) { break; }
    }
  }

  // Store the node in TT
  node_type_t type = TT_ALPHA_NODE;
  if (best_so_far <= alpha0) {
    type = TT_ALPHA_NODE;
  } else if (best_so_far >= beta) {
    type = TT_BETA_NODE;
  } else {
    type = TT_PV_NODE;
  }
  tt_store_entry(state->tt, board, depth, best_so_far, type, &best_move);

  state->best_move = best_move;
  return best_so_far;
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

  const int score = negamax(MIN, MAX, depth, 0, board, state);

  search_result.best_move = state->best_move;

  // Handle mate score
  search_result.mate_found = false;

  if (score > -MATE_MAX && score < -MATE_MIN) {
    search_result.mate_found = true;
    search_result.mate_in = -(score + MATE_MAX) / 2 - 1;
  }

  if (score > MATE_MIN && score < MATE_MAX) {
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
