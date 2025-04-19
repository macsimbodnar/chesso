#include "search.hpp"
#include <cassert>
#include <future>
#include <iostream>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"


#define MATE_SCORE 1000000

static constexpr int MIN = std::numeric_limits<int>::min() + 100;
static constexpr int MAX = std::numeric_limits<int>::max() - 100;

#define FULL_DEPTH_MOVES 4
#define REDUCTION_LIMIT 3

// NOTE: This is used for null move pruning. The null move pruning should not be
// used in late game. It can make bad things happen
#define REDUCTION_FACTOR 2


void debug_print_move(const move_t* move, int score)
{
  std::cout << *move << "        " << score << "\n";
}


int quiescence_search(int alpha,
                      int beta,
                      size_t qs_ply,
                      const board_t* board,
                      search_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);

  state->explored_nodes += 1;

  const int stand_pat =
      (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);

  int best_value = stand_pat;

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) { return stand_pat; }

  if (qs_ply > 4) { return stand_pat; }

  if (stand_pat >= beta) { return stand_pat; }
  if (alpha < stand_pat) { alpha = stand_pat; }


  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  for (size_t i = 0; i < moves_count; ++i) {
    // We process only captures and promotions
    if (moves[i].captured == INVALID && moves[i].promoted_to == TO_NONE) {
      continue;
    }

    board_t tmp_board = *board;
    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    const int score =
        -quiescence_search(-beta, -alpha, qs_ply + 1, &tmp_board, state);

    if (score >= beta) { return score; }
    if (score > best_value) { best_value = score; }
    if (score > alpha) { alpha = score; }
  }

  return best_value;
}


int alpha_beta_negamax(int alpha,
                       int beta,
                       int depth,
                       size_t ply,
                       const board_t* board,
                       search_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);

  bool follow_pv = false;

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) {
    return evaluate(board);
  }

  // We just return in case we overrun the max ply
  if (ply >= MAX_PLY) { return evaluate(board); }

  // Init the PV length
  state->pv.pv_length[ply] = ply;

  const bool is_in_check = is_check(board);

  if (is_in_check) {
    // if we are in check we want to search deeper
    ++depth;
  }

  // Null-move forward pruning.
  // TODO: Disable in late game
  if (depth > REDUCTION_FACTOR && !is_in_check) {
    // The null move is just the current position with switched side
    board_t swapped_board = *board;
    swap_side(&swapped_board);
    clear_ep_square(&swapped_board);

    const int eval =
        -alpha_beta_negamax(-beta, -beta + 1, depth - 1 - REDUCTION_FACTOR,
                            ply + 1, &swapped_board, state);

    if (eval >= beta) {
      // Beta cut-off
      return beta;
    }
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  if (moves_count == 0) {
    // Checkmate or stalemate handling
    if (is_in_check) {
      return -(MATE_SCORE - ply);
    } else {
      // Stalemate
      return 0;
    }
  }

  // NOTE: Check if < 1 instead of == 0 because some time we subtract
  // 2 to the depth in recursive calls during LMR
  if (depth < 1) { return quiescence_search(alpha, beta, 0, board, state); }

  // Sort moves
  order_moves(moves, moves_count, ply, state);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval;
    if (follow_pv) {
      // PV Sorting. We follow the principal variation
      eval = -alpha_beta_negamax(-alpha - 1, -alpha, depth - 1, ply + 1,
                                 &tmp_board, state);
    }

    if (!follow_pv || (eval > alpha && eval < beta)) {
      // In case we don't follow PV yet or PV following failed

      if (i == 0) {
        // In case of first move we perform the full depth search based on LMR
        eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                   &tmp_board, state);
      } else {
        // Here we are in the logic of Late Move Reduction
        if (i >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT &&
            should_reduce_move(&moves[i]) && !is_in_check) {
          // Search with reduced depth
          eval = -alpha_beta_negamax(-alpha - 1, -alpha, depth - 2, ply + 1,
                                     &tmp_board, state);
        } else {
          // Hack to ensure that full-depth search is done.
          eval = alpha + 1;
        }

        // If good good move found in the reduced depth
        if (eval > alpha) {
          // Search deeper but with narrow window
          eval = -alpha_beta_negamax(-alpha - 1, -alpha, depth - 1, ply + 1,
                                     &tmp_board, state);

          // Search deeper in normal window
          if (eval > alpha && eval < beta)
            eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                       &tmp_board, state);
        }
      }
    }

    if (eval >= beta) {
      // Beta cut-off

      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Store the killer move
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];
      }

      return beta;
    }

    if (eval > alpha) {
      // Found better move

      alpha = eval;

      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Update the history move
        assert(moves[i].piece != INVALID);
        assert(moves[i].piece != EMPTY);
        assert(moves[i].to != INVALID_BOARD_INDEX);
        state->history_moves[moves[i].piece][moves[i].to] += depth;
      }

      follow_pv = true;

      // Write PV move
      state->pv.pv_table[ply][ply] = moves[i];

      // Copy the moves from deeper ply into the current ply's line
      for (size_t i = ply + 1; i < state->pv.pv_length[ply + 1]; ++i) {
        assert(ply < MAX_PLY);
        assert(i < MAX_PLY);
        state->pv.pv_table[ply][i] = state->pv.pv_table[ply + 1][i];
      }

      state->pv.pv_length[ply] = state->pv.pv_length[ply + 1];
    }
  }

  return alpha;
}


search_t search_best_move(int depth,
                          const board_t* board,
                          search_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);

  search_t search_result = {};

  const int eval = alpha_beta_negamax(MIN, MAX, depth, 0, board, state);

  search_result.best_move = state->pv.pv_table[0][0];
  search_result.explored_nodes = state->explored_nodes;
  search_result.pv = state->pv;
  search_result.score = eval;

  return search_result;
}
