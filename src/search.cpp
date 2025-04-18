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

  const int eval =
      (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) { return eval; }

  if (qs_ply > 4) { return eval; }

  if (eval >= beta) { return beta; }
  if (eval > alpha) { alpha = eval; }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  // Sort moves
  // order_moves(moves, moves_count);

  for (size_t i = 0; i < moves_count; ++i) {
    if (moves[i].captured == INVALID && moves[i].promoted_to == TO_NONE) {
      // Skip
      continue;
    }

    board_t tmp_board = *board;
    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    const int score =
        -quiescence_search(-beta, -alpha, qs_ply + 1, &tmp_board, state);

    if (score >= beta) { return beta; }
    if (score >= alpha) { alpha = score; }
  }

  return alpha;
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

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) {
    return evaluate(board);
  }

  // We just return in case we overrun the max ply
  if (ply >= MAX_PLY) { return evaluate(board); }

  // Init the PV length
  state->pv.pv_length[ply] = ply;

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);
  const bool is_in_check = is_check(board);

  if (is_in_check) {
    // if we are in check we want to search deeper
    ++depth;
  }

  if (moves_count == 0) {
    // Checkmate or stalemate handling
    if (is_in_check) {
      return -(MATE_SCORE - ply);
    } else {
      // Stalemate
      return 0;
    }
  }

  if (depth == 0) { return quiescence_search(alpha, beta, 0, board, state); }

  // Sort moves
  order_moves(moves, moves_count, ply, state);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                   &tmp_board, state);


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

      // Write PV move
      state->pv.pv_table[ply][ply] = moves[i];

      // Copy the moves from deeper ply into the current ply's line
      for (size_t next_ply = ply + 1; next_ply < state->pv.pv_length[ply + 1];
           ++next_ply) {
        state->pv.pv_table[ply][next_ply] =
            state->pv.pv_table[ply + 1][next_ply];
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

  const int eval = -alpha_beta_negamax(MIN, MAX, depth, 0, board, state);

  search_result.best_move = state->pv.pv_table[0][0];
  search_result.explored_nodes = state->explored_nodes;
  search_result.pv = state->pv;
  search_result.score = eval;

  return search_result;
}
