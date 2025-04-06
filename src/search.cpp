#include "search.hpp"
#include <cassert>
#include <future>
#include <iostream>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"


#define RUN_THREADS

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
                      const board_t* board,
                      uint64_t* num_of_nodes_explored)
{
  assert(board != nullptr);
  assert(num_of_nodes_explored != nullptr);

  *num_of_nodes_explored += 1;

  const int eval =
      (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);

  if (eval >= beta) { return beta; }
  if (eval > alpha) { alpha = eval; }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  // Sort moves
  order_moves(moves, moves_count);

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
        -quiescence_search(-beta, -alpha, &tmp_board, num_of_nodes_explored);

    if (score >= beta) { return beta; }
    if (score >= alpha) { alpha = score; }
  }

  return alpha;
}


int alpha_beta_negamax(int alpha,
                       int beta,
                       int depth,
                       int ply,
                       const board_t* board,
                       uint64_t* num_of_nodes_explored)
{
  assert(board != nullptr);
  assert(num_of_nodes_explored != nullptr);

  int max_eval = MIN;

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
    return quiescence_search(alpha, beta, board, num_of_nodes_explored);
  }

  // Sort moves
  order_moves(moves, moves_count);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                   &tmp_board, num_of_nodes_explored);

    if (eval > max_eval) { max_eval = eval; }

    alpha = std::max(alpha, eval);
    if (alpha >= beta) {
      // Beta cut-off
      break;
    }
  }

  return max_eval;
}


search_t search_best_move(int depth, const board_t* board)
{
  assert(board != nullptr);

  int best_eval = MIN;
  move_t best_move;
  uint64_t num_of_nodes_explored = 0;

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

#ifdef RUN_THREADS
  struct res_t
  {
    size_t move_index;
    int score;
    uint64_t nodes_explored;
  };

  std::vector<std::future<res_t>> results_futures;

  for (size_t i = 0; i < moves_count; ++i) {
    results_futures.push_back(
        std::async(std::launch::async, [moves, i, depth, board]() {
          board_t tmp_board = *board;

          uint64_t nodes_explored = 0;
          const bool done = make_move(&moves[i], &tmp_board, nullptr);
          (void)done;
          assert(done);

          const int eval = -alpha_beta_negamax(MIN, MAX, depth - 1, 1,
                                               &tmp_board, &nodes_explored);

          res_t result = {i, eval, nodes_explored};
          return result;
        }));
  }

  for (auto& future : results_futures) {
    const res_t& result = future.get();

    num_of_nodes_explored += result.nodes_explored;

    // debug_print_move(&moves[result.move_index], result.score);

    if (result.score >= best_eval) {
      best_eval = result.score;
      best_move = moves[result.move_index];
    }
  }

#else
  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    const int eval = -alpha_beta_negamax(MIN, MAX, depth - 1, 1, &tmp_board,
                                         &num_of_nodes_explored);

    debug_print_move(&moves[i], eval);

    if (eval >= best_eval) {
      best_eval = eval;
      best_move = moves[i];
    }
  }
#endif

  // NOTE: The output sign needs to be adjusted to be shown always from the
  // white point of view
  return {best_move,
          (board->game_state.active_color == WHITE ? 1 : -1) * best_eval,
          num_of_nodes_explored};
}
