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
        -quiescence_search(-beta, -alpha, &tmp_board, num_of_nodes_explored);

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
                       uint64_t* num_of_nodes_explored,
                       move_t killer_moves[2][MAX_PLY],
                       int history_moves[piece_t::EMPTY + 1][BOARD_SIZE],
                       pv_t* pv)
{
  assert(board != nullptr);
  assert(num_of_nodes_explored != nullptr);
  assert(killer_moves != nullptr);
  assert(history_moves != nullptr);
  assert(pv != nullptr);

  int max_eval = MIN;

  // Init the PV length
  pv->pv_length[ply] = ply;

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
  order_moves(moves, moves_count, ply, killer_moves, history_moves);


  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    int eval = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                   &tmp_board, num_of_nodes_explored,
                                   killer_moves, history_moves, pv);

    if (eval > max_eval) {
      // Found better move
      max_eval = eval;

      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Update the history move
        assert(moves[i].piece != INVALID);
        assert(moves[i].piece != EMPTY);
        assert(moves[i].to != INVALID_BOARD_INDEX);
        history_moves[moves[i].piece][moves[i].to] += depth;
      }

      // Write PV move
      pv->pv_table[ply][ply] = moves[i];

      // Copy the moves from deeper ply into the current ply's line
      for (size_t next_ply = ply + 1; next_ply < pv->pv_length[ply + 1];
           ++next_ply) {
        pv->pv_table[ply][next_ply] = pv->pv_table[ply + 1][next_ply];
      }

      pv->pv_length[ply] = pv->pv_length[ply + 1];
    }

    alpha = std::max(alpha, eval);
    if (alpha >= beta) {
      // Beta cut-off

      // Only on quite moves
      if (moves[i].captured == INVALID) {
        // Store the killer move
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = moves[i];
      }

      break;
    }
  }

  return max_eval;
}


search_t search_best_move(int depth, const board_t* board)
{
  assert(board != nullptr);

  search_t search_result = {};
  search_result.score = MIN;
  search_result.explored_nodes = 0;
  search_result.pv = {};

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

#ifdef RUN_THREADS
  struct res_t
  {
    size_t move_index;
    int score;
    uint64_t nodes_explored;
    pv_t pv;
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

          // killer_moves[id][ply]
          move_t killer_moves[2][MAX_PLY];
          // history_moves[pieces][squares]
          int history_moves[piece_t::EMPTY + 1][BOARD_SIZE] = {};

          res_t result = {};
          result.pv = {};

          const int eval = -alpha_beta_negamax(
              MIN, MAX, depth - 1, 1, &tmp_board, &nodes_explored, killer_moves,
              history_moves, &result.pv);

          result.move_index = i;
          result.score = eval;
          result.nodes_explored = nodes_explored;

          return result;
        }));
  }

  for (auto& future : results_futures) {
    const res_t& result = future.get();

    search_result.explored_nodes += result.nodes_explored;

    // debug_print_move(&moves[result.move_index], result.score);

    if (result.score >= search_result.score) {
      search_result.score = result.score;
      search_result.best_move = moves[result.move_index];
      search_result.pv = result.pv;

      // Update the PV with the best move at ply 0
      search_result.pv.pv_table[0][0] = moves[result.move_index];
      // Copy the moves from deeper ply into the current ply's line
      for (size_t next_ply = 1; next_ply < search_result.pv.pv_length[1];
           ++next_ply) {
        search_result.pv.pv_table[0][next_ply] =
            search_result.pv.pv_table[1][next_ply];
      }
      search_result.pv.pv_length[0] = search_result.pv.pv_length[1];
    }
  }

#else
  // killer_moves[id][ply]
  move_t killer_moves[2][MAX_PLY];
  // history_moves[pieces][squares]
  int history_moves[piece_t::EMPTY + 1][BOARD_SIZE] = {};
  pv_t pv = {};

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);


    const int eval = -alpha_beta_negamax(MIN, MAX, depth - 1, 1, &tmp_board,
                                         &search_result.explored_nodes,
                                         killer_moves, history_moves, &pv);

    // debug_print_move(&moves[i], eval);

    if (eval >= search_result.score) {
      search_result.score = eval;
      search_result.best_move = moves[i];
      search_result.pv = pv;

      // Update the PV with the best move at ply 0
      search_result.pv.pv_table[0][0] = moves[i];
      // Copy the moves from deeper ply into the current ply's line
      for (size_t next_ply = 1; next_ply < search_result.pv.pv_length[1];
           ++next_ply) {
        search_result.pv.pv_table[0][next_ply] =
            search_result.pv.pv_table[1][next_ply];
      }
      search_result.pv.pv_length[0] = search_result.pv.pv_length[1];
    }
  }
#endif

  // NOTE: The output sign needs to be adjusted to be shown always from the
  // white point of view
  search_result.score =
      (board->game_state.active_color == WHITE ? 1 : -1) * search_result.score;

  return search_result;
}
