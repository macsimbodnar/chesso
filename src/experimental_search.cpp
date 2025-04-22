#include "experimental_search.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include "board.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "move_generator.hpp"

#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

static constexpr int MIN = std::numeric_limits<int>::min() + 100;
static constexpr int MAX = std::numeric_limits<int>::max() - 100;
static constexpr int NO_SCORE = MAX + 42;


int evaluate_move(const move_t* move, const board_t* board)
{
  assert(move != nullptr);
  assert(board != nullptr);

  board_t tmp_board = *board;
  const color_t color = board->game_state.active_color;

  (void)make_move(move, &tmp_board, nullptr);
  const int score = evaluate(&tmp_board);

  return (color == WHITE) ? score : -score;
}


void experimental_order_moves(move_t moves[],
                              size_t moves_size,
                              const board_t* board)
{
  assert(moves != nullptr);
  assert(moves_size <= MAX_MOVES);
  assert(board != nullptr);

  int scores[MAX_MOVES];
  for (size_t i = 0; i < moves_size; ++i) {
    scores[i] = evaluate_move(&moves[i], board);
  }

  // Insertion sort
  for (size_t i = 1; i < moves_size; ++i) {
    const int value = scores[i];
    const move_t move = moves[i];
    int j = i;

    while (j != 0 && scores[j - 1] < value) {
      scores[j] = scores[j - 1];
      moves[j] = moves[j - 1];
      --j;
    }

    scores[j] = value;
    moves[j] = move;
  }
}


int search(int alpha0,
           int beta,
           bool zero_window,
           int depth,
           size_t ply,
           const board_t* board,
           search_state_t* state)
{
  if (depth < 1) {
    ++(state->explored_nodes);
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  int score = MIN;
  int best_score = NO_SCORE;
  move_t best_move;
  int alpha = alpha0;

  state->pv.pv_length[ply] = ply;

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  const bool is_in_check = is_check(board);
  if (moves_count == 0) {
    // Checkmate or stalemate handling
    if (is_in_check) {
      return -(MATE_MAX - ply);
    } else {
      // Stalemate
      return DRAW_SCORE;
    }
  }

  order_moves(moves, moves_count, ply, state);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    if (!zero_window && i > 0) {
      score =
          -search(-alpha, -alpha, true, depth - 1, ply + 1, &tmp_board, state);

      if (score < alpha) { continue; }

      if (score > alpha) {
        score = -search(-beta, -alpha, false, depth - 1, ply + 1, &tmp_board,
                        state);
      }
    } else {
      score = -search(-beta, -alpha, zero_window, depth - 1, ply + 1,
                      &tmp_board, state);
    }

    if (i == 0) {
      best_score = score;
      best_move = moves[i];
    } else {
      if (score > best_score) {
        best_score = score;
        best_move = moves[i];
        state->pv.pv_table[ply][ply] = moves[i];

        // Copy the moves from deeper ply into the current ply's line
        for (size_t i = ply + 1; i < state->pv.pv_length[ply + 1]; ++i) {
          assert(ply < MAX_PLY);
          assert(i < MAX_PLY);
          state->pv.pv_table[ply][i] = state->pv.pv_table[ply + 1][i];
        }

        state->pv.pv_length[ply] = state->pv.pv_length[ply + 1];

        // Only on quite moves
        if (moves[i].captured == INVALID) {
          // Update the history move
          assert(moves[i].piece != INVALID);
          assert(moves[i].piece != EMPTY);
          assert(moves[i].to != INVALID_BOARD_INDEX);
          state->history_moves[moves[i].piece][moves[i].to] += depth;
        }
      }
    }

    if (beta < best_score) {
      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Store the killer move
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];
      }
      break;
    }

    alpha = std::max(alpha, score);
  }

  if (best_score == NO_SCORE) {
    best_score = alpha0 - 1;
    assert(moves_count > 0);
    best_move = moves[0];
  }

  state->best_move = best_move;
  return best_score;
}


int negamax(int alpha,
            int beta,
            int depth,
            size_t ply,
            const board_t* board,
            search_state_t* state)
{
  int score = MIN;

  if (depth < 1) {
    ++(state->explored_nodes);
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  const bool is_in_check = is_check(board);
  if (moves_count == 0) {
    // Checkmate or stalemate handling
    if (is_in_check) {
      return -(MATE_MAX - ply);
    } else {
      // Stalemate
      return DRAW_SCORE;
    }
  }

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    score = -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state);

    if (score >= alpha) {
      alpha = score;
      state->best_move = moves[i];
    }

    if (alpha >= beta) break;
  }

  return alpha;
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

  // const int score = search(MIN, MAX, false, depth, 0, board, state);
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

  return search_result;
}
