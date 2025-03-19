#include "search.hpp"
#include <cassert>
#include <future>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"

#define RUN_THREADS


int alpha_beta_minimax(int alpha, int beta, int depth, const board_t* board)
{
  const int current_eval = evaluate(board);

  if (depth == 0) { return current_eval; }

  const auto moves = generate_legal_moves(board);

  if (moves.size() == 0) { return current_eval; }

  int max_eval = std::numeric_limits<int>::min();

  if (board->game_state.active_color == WHITE) {  // Maximizing
    for (const auto& move : moves) {
      board_t tmp_board = *board;
      (void)make_move(&move, &tmp_board, nullptr);
      const int eval = alpha_beta_minimax(alpha, beta, depth - 1, board);
      max_eval = (eval > max_eval) ? eval : max_eval;
      alpha = (eval > alpha) ? eval : alpha;

      if (beta <= alpha) { break; }
    }

    return max_eval;

  } else {  // Minimizing
    int min_eval = std::numeric_limits<int>::max();

    for (const auto& move : moves) {
      board_t tmp_board = *board;
      (void)make_move(&move, &tmp_board, nullptr);
      const int eval = alpha_beta_minimax(alpha, beta, depth - 1, board);
      min_eval = (eval < min_eval) ? eval : min_eval;
      beta = (eval < beta) ? eval : beta;

      if (beta <= alpha) { break; }
    }

    return min_eval;
  }
}


int minimax(int depth, const board_t* board)
{
  const int current_eval = evaluate(board);

  if (depth == 0) { return current_eval; }

  const auto moves = generate_legal_moves(board);

  if (moves.size() == 0) { return current_eval; }

  if (board->game_state.active_color == WHITE) {  // Maximizing
    int max_eval = std::numeric_limits<int>::min();

    for (const auto& move : moves) {
      board_t tmp_board = *board;
      (void)make_move(&move, &tmp_board, nullptr);
      const int eval = minimax(depth - 1, &tmp_board);
      max_eval = (eval > max_eval) ? eval : max_eval;
    }

    return max_eval;

  } else {  // Minimizing
    int min_eval = std::numeric_limits<int>::max();

    for (const auto& move : moves) {
      board_t tmp_board = *board;
      (void)make_move(&move, &tmp_board, nullptr);
      const int eval = minimax(depth - 1, &tmp_board);
      min_eval = (eval < min_eval) ? eval : min_eval;
    }

    return min_eval;
  }

  return current_eval;
}


move_t search_best_move(int depth, const board_t* board)
{
  const color_t side = board->game_state.active_color;
  const auto moves = generate_legal_moves(board);
  assert(moves.size() > 0);

  int best_score = (side == WHITE) ? std::numeric_limits<int>::min()
                                   : std::numeric_limits<int>::max();
  size_t best_move_index = 1;

#ifdef RUN_THREADS

  struct res_t
  {
    size_t move_index;
    int score;
  };


  std::vector<std::future<res_t>> results;
  for (size_t i = 0; i < moves.size(); ++i) {
    move_t move = moves[i];
    results.push_back(std::async(std::launch::async, [i, board, move, depth]() {
      board_t tmp_board = *board;
      move_t tmp_move = move;
      (void)make_move(&tmp_move, &tmp_board, nullptr);

      // const int current_score = minimax(depth, &tmp_board);

      const int current_score = alpha_beta_minimax(
          std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
          depth, &tmp_board);

      res_t res = {i, current_score};
      return res;
    }));
  }

  for (auto& res : results) {
    const res_t& result = res.get();
    if (side == WHITE) {
      if (result.score > best_score) {
        best_score = result.score;
        best_move_index = result.move_index;
      }
    } else {
      if (result.score < best_score) {
        best_score = result.score;
        best_move_index = result.move_index;
      }
    }
  }
#else

  for (size_t i = 0; i < moves.size(); ++i) {
    board_t tmp_board = *board;
    const move_t& move = moves[i];
    const bool done = make_move(&move, &tmp_board, nullptr);
    (void)done;
    assert(done);

    // const int current_score = minimax(depth, &tmp_board);

    const int current_score =
        alpha_beta_minimax(std::numeric_limits<int>::min(),
                           std::numeric_limits<int>::max(), depth, &tmp_board);

    if (tmp_board.game_state.active_color == WHITE) {
      if (current_score > best_score) {
        best_score = current_score;
        best_move_index = i;
      }
    } else {
      if (current_score < best_score) {
        best_score = current_score;
        best_move_index = i;
      }
    }
  }
#endif

  return moves[best_move_index];
}