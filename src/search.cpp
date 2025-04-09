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
#define MAX_MATE_DEPTH 10000

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
                      uint64_t* num_of_nodes_explored)
{
  assert(board != nullptr);
  assert(num_of_nodes_explored != nullptr);


  *num_of_nodes_explored += 1;

  const int eval =
      (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);

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

    const int score = -quiescence_search(-beta, -alpha, qs_ply + 1, &tmp_board,
                                         num_of_nodes_explored);

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

  // We just return in case we overrun the max ply
  if (ply >= MAX_PLY) { return evaluate(board); }

  int max_eval = MIN;

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
      // Checkmate. Use the depth in order to prefer the fastest mate
      // Test pos: 4k3/8/5K2/8/1Q6/8/8/8 w - - 10 1
      return -(MATE_SCORE - ply);
    } else {
      // Stalemate
      return 0;
    }
  }

  if (depth == 0) {
    return quiescence_search(alpha, beta, 0, board, &state->explored_nodes);
    // state->explored_nodes++;
    // return evaluate(board);
  }

  // Sort moves
  order_moves(moves, moves_count, ply, state->killer_moves,
              state->history_moves);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                   &tmp_board, state);

    if (eval > max_eval) {
      // Found better move
      max_eval = eval;

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

    alpha = std::max(alpha, eval);
    if (alpha >= beta) {
      // Beta cut-off

      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Store the killer move
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];
      }

      break;
    }
  }


  return max_eval;
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
  // NOTE: The output sign needs to be adjusted to be shown always from the
  // white point of view
  search_result.score =
      (board->game_state.active_color == WHITE ? 1 : -1) * eval;

  return search_result;
}
