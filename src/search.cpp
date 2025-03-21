#include "search.hpp"
#include <cassert>
#include <future>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"

#define RUN_THREADS


int alpha_beta_negamax(int alpha,
                       int beta,
                       int depth,
                       const board_t* board,
                       uint64_t* num_of_nodes_explored)
{
  if (depth == 0) {
    const int score =
        (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);

    *num_of_nodes_explored += 1;
    return score;
  }

  int max_eval = std::numeric_limits<int>::min();

  move_t moves[270];
  const size_t moves_count = generate_legal_moves(board, moves);

  if (moves_count == 0) {
    // Checkmate or stalemate handling
    // Optional: use mate/stalemate evaluation here

    const int score = evaluate(board);
    return score;
  }

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, &tmp_board,
                                   num_of_nodes_explored);


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
  int best_eval = std::numeric_limits<int>::min();
  move_t best_move;
  uint64_t num_of_nodes_explored = 0;

  move_t moves[270];
  const size_t moves_count = generate_legal_moves(board, moves);

#ifdef RUN_THREADS
  struct res_t
  {
    size_t move_index;
    int score;
  };

  std::vector<std::future<res_t>> results_futures;

  for (size_t i = 0; i < moves_count; ++i) {
    results_futures.push_back(std::async(
        std::launch::async, [moves, i, depth, board, &num_of_nodes_explored]() {
          board_t tmp_board = *board;

          const bool done = make_move(&moves[i], &tmp_board, nullptr);
          (void)done;
          assert(done);

          const int eval = -alpha_beta_negamax(
              std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
              depth - 1, &tmp_board, &num_of_nodes_explored);

          res_t result = {i, eval};
          return result;
        }));
  }

  for (auto& future : results_futures) {
    const res_t& result = future.get();

    if (result.score > best_eval) {
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

    const int eval = -alpha_beta_negamax(
        std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
        depth - 1, &tmp_board, &num_of_nodes_explored);

    if (eval > best_eval) {
      best_eval = eval;
      best_move = moves[i];
    }
  }
#endif

  return {best_move, best_eval, num_of_nodes_explored};
}
