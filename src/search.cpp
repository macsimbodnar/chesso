#include "search.hpp"
#include <cassert>
#include <future>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "move_generator.hpp"

// #define RUN_THREADS


int alpha_beta_negamax(int alpha, int beta, int depth, const board_t* board)
{
  if (depth == 0) {
    const int evaluation = evaluate(board);
    return evaluation;
  }

  int best_val = std::numeric_limits<int>::min();

  const auto moves = generate_legal_moves(board);

  for (const auto& move : moves) {
    board_t tmp_board = *board;

    (void)make_move(&move, &tmp_board);

    const int score = -alpha_beta_negamax(-beta, -alpha, depth - 1, &tmp_board);

    if (score > best_val) {
      best_val = score;
      if (score > alpha) alpha = score;  // alpha acts like max in MiniMax
    }

    if (score >= beta) {
      //  fail soft beta-cutoff, existing the loop here is also fine
      return best_val;
    }
  }

  return best_val;
}


int negamax(int depth, const board_t* board)
{
  if (depth == 0) {
    const int eval = evaluate(board);
    return eval;
  }

  int max = std::numeric_limits<int>::min();


  const auto moves = generate_legal_moves(board);

  for (const auto& move : moves) {
    board_t tmp_board = *board;
    const bool move_happened = make_move(&move, &tmp_board);
    (void)move_happened;
    assert(move_happened);

    const int score = -negamax(depth - 1, &tmp_board);

    // const bool move_unmaked = unmake_move(board);
    // (void)move_unmaked;
    // assert(move_unmaked);

    if (score > max) { max = score; }
  }

  return max;
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
      (void)make_move(&tmp_move, &tmp_board);
      // const int negamax_score = negamax(depth, &tmp_board);

      const int alpha_beta_score = alpha_beta_negamax(
          std::numeric_limits<int>::min(), std::numeric_limits<int>::max(),
          depth - 1, &tmp_board);

      res_t res = {i, alpha_beta_score};
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
    const bool done = make_move(&move, &tmp_board);
    (void)done;
    assert(done);

    const int negamax_score = negamax(depth, board);

    if (board->game_state.active_color == WHITE) {
      if (negamax_score > best_score) {
        best_score = negamax_score;
        best_move_index = i;
      }
    } else {
      if (negamax_score < best_score) {
        best_score = negamax_score;
        best_move_index = i;
      }
    }
  }
#endif

  return moves[best_move_index];
}