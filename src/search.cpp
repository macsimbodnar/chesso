#include "search.hpp"
#include <cassert>
#include <future>
#include <iostream>
#include <limits>
#include <thread>
#include "board.hpp"
#include "evaluation.hpp"
#include "experimental_search.hpp"
#include "log.hpp"
#include "move_generator.hpp"


#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

static constexpr int MIN = std::numeric_limits<int>::min() + 100;
static constexpr int MAX = std::numeric_limits<int>::max() - 100;
static constexpr int NO_SCORE = MAX + 42;


#define FULL_DEPTH_MOVES 4
#define REDUCTION_LIMIT 3

// NOTE: This is used for null move pruning. The null move pruning should not be
// used in late game. It can make bad things happen
#define REDUCTION_FACTOR 2
#define NMP_DEPTH_LIMIT 2


void debug_print_move(const move_t* move, int score)
{
  LOG_I << *move << "        " << score << END_I;
}


inline void store_to_tt(const board_t* board,
                        search_state_t* state,
                        int depth,
                        hash_flag_t flag,
                        int score,
                        int ply)
{
  assert(board != nullptr);
  assert(state != nullptr);

  const uint64_t index = board->game_state.zobrist_key % TT_SIZE;
  tt_hash_t* elem = &state->tt[index];
  assert(elem != nullptr);

  // Handle mate score. It needs to be independent from the path so we remove
  // the ply
  if (score < -MATE_MIN) { score -= ply; }
  if (score > MATE_MIN) { score += ply; }

  elem->key = board->game_state.zobrist_key;
  elem->depth = depth;
  elem->flag = flag;
  elem->score = score;
}


inline int get_from_tt(const board_t* board,
                       const search_state_t* state,
                       int depth,
                       int alpha,
                       int beta,
                       int ply)
{
  assert(board != nullptr);
  assert(state != nullptr);

  const uint64_t index = board->game_state.zobrist_key % TT_SIZE;
  const tt_hash_t* elem = &state->tt[index];

  assert(elem != nullptr);

  if (elem->key == board->game_state.zobrist_key) {
    if (elem->depth >= depth) {
      int score = elem->score;

      // Adjust the mate score to the ply we are in now
      if (score < -MATE_MIN) { score += ply; }
      if (score > MATE_MIN) { score -= ply; }

      // Return the score
      if (elem->flag == TT_TYPE_EXACT) { return score; }
      if ((elem->flag == TT_TYPE_ALPHA) && (score <= alpha)) { return alpha; }
      if ((elem->flag == TT_TYPE_BETA) && (score >= beta)) { return beta; }
    }
  }

  return NO_SCORE;
}


bool is_pv_legal(const board_t* board, const pv_t* pv)
{
  assert(board != nullptr);
  assert(pv != nullptr);

  // Empty pv is illegal
  if (pv->pv_length[0] < 1) {
    LOG_W << "Empty PV" << END_W;
    return false;
  }

  board_t tmp_board = *board;

  for (size_t i = 0; i < pv->pv_length[0]; ++i) {
    const move_t* move_to_test = &pv->pv_table[0][i];

    move_t moves[MAX_MOVES];
    const size_t moves_count = generate_legal_moves(&tmp_board, moves);

    if (moves_count < 1) { return false; }

    bool found = false;
    for (size_t move_index = 0; move_index < moves_count; ++move_index) {
      if (*move_to_test == moves[move_index] &&
          move_to_test->captured == moves[move_index].captured) {
        found = true;
        break;
      }
    }

    if (!found) {
      LOG_W << "PV with illegal moves" << END_W;
      return false;
    }

    make_move(move_to_test, &tmp_board, nullptr);
  }

  return true;
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

  if (qs_ply > 3) { return stand_pat; }

  if (alpha < stand_pat) {
    alpha = stand_pat;

    if (stand_pat >= beta) { return stand_pat; }
  }

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


    if (score > best_value) { best_value = score; }
    if (score > alpha) {
      alpha = score;
      if (score >= beta) { return score; }
    }
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

  // Check for repetitions
  if (is_position_repeated(board)) { return DRAW_SCORE; }

  int score = 0;
  hash_flag_t hash_flag = TT_TYPE_ALPHA;

  const bool pv_node = (beta - alpha) > 1;

  // Check the TT
  if (ply > 0 && !pv_node && state->search_in_tt) {
    score = get_from_tt(board, state, depth, alpha, beta, ply);

    if (score != NO_SCORE) {
      // If the move is found in the TT then we return the set score
      return score;
    }
  }

  // Reset the score just in case
  score = 0;

  // Time management
  if ((state->explored_nodes % 1000) && *state->stop) {
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  // We just return in case we overrun the max ply
  if (ply >= MAX_PLY) {
    return (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  }

  // Init the PV length
  state->pv.pv_length[ply] = ply;

  const bool is_in_check = is_check(board);

  if (is_in_check) {
    // if we are in check we want to search deeper
    ++depth;
  }

  // Null-move forward pruning.
  // TODO: Disable in late game
  if (depth > NMP_DEPTH_LIMIT && !is_in_check && ply > 0) {
    // The null move is just the current position with switched side
    board_t swapped_board = *board;
    swap_side(&swapped_board);
    clear_ep_square(&swapped_board);

    score = -alpha_beta_negamax(-beta, -beta + 1, depth - 1 - REDUCTION_FACTOR,
                                ply + 1, &swapped_board, state);

    if (score >= beta) {
      // Beta cut-off
      return beta;
    }
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  if (moves_count == 0) {
    // Checkmate or stalemate handling
    if (is_in_check) {
      return -(MATE_MAX - ply);
    } else {
      // Stalemate
      return DRAW_SCORE;
    }
  }

  // NOTE: Check if < 1 instead of == 0 because some time we subtract
  // 2 to the depth in recursive calls during LMR
  if (depth < 1) { return quiescence_search(alpha, beta, 0, board, state); }

  // Sort moves
  // order_moves(moves, moves_count, ply, state);
  experimental_order_moves(moves, moves_count, board);

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    const bool done = make_move(&moves[i], &tmp_board, nullptr);
    (void)done;
    assert(done);

    if (i == 0) {
      // In case of first move we perform the full depth search based on LMR
      score = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board,
                                  state);
    } else {
      // Here we are in the logic of Late Move Reduction
      if (i >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT &&
          should_reduce_move(&moves[i]) && !is_in_check) {
        // Search with reduced depth
        score = -alpha_beta_negamax(-alpha - 1, -alpha, depth - 2, ply + 1,
                                    &tmp_board, state);
      } else {
        // Hack to ensure that full-depth search is done.
        score = alpha + 1;
      }

      // Here we search PV
      if (score > alpha) {
        // Search deeper but with narrow window
        score = -alpha_beta_negamax(-alpha - 1, -alpha, depth - 1, ply + 1,
                                    &tmp_board, state);

        // Search deeper in normal window
        if (score > alpha && score < beta) {
          score = -alpha_beta_negamax(-beta, -alpha, depth - 1, ply + 1,
                                      &tmp_board, state);
        }
      }
    }

    if (score > alpha) {
      // Found better move

      alpha = score;

      // Update TT flag
      hash_flag = TT_TYPE_EXACT;

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
      for (size_t i = ply + 1; i < state->pv.pv_length[ply + 1]; ++i) {
        assert(ply < MAX_PLY);
        assert(i < MAX_PLY);
        state->pv.pv_table[ply][i] = state->pv.pv_table[ply + 1][i];
      }

      state->pv.pv_length[ply] = state->pv.pv_length[ply + 1];

      if (score >= beta) {
        // Beta cut-off

        // Update TT
        store_to_tt(board, state, depth, TT_TYPE_BETA, beta, ply);

        // Only on quite moves
        if (moves[i].captured == INVALID) {
          // Store the killer move
          state->killer_moves[1][ply] = state->killer_moves[0][ply];
          state->killer_moves[0][ply] = moves[i];
        }

        return beta;
      }
    }
  }

  // Update TT
  store_to_tt(board, state, depth, hash_flag, alpha, ply);
  return alpha;
}


search_t search_best_move(int depth,
                          const board_t* board,
                          search_state_t* state)
{
  assert(board != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);

  search_t search_result = {};

  // Set backup move just in case the PV is empty
  move_t moves[MAX_MOVES];
  const size_t moves_size = generate_legal_moves(board, moves);
  experimental_order_moves(moves, moves_size, board);
  assert(moves_size > 0);
  search_result.best_move = moves[0];

  // Here comes the search
  state->search_in_tt = true;
  int score = alpha_beta_negamax(MIN, MAX, depth, 0, board, state);

  const bool pv_legal = is_pv_legal(board, &state->pv);
  if (pv_legal) {
    search_result.best_move = state->pv.pv_table[0][0];
  } else if (!*state->stop) {
    // NOTE: This is a workaround until find a way to deal with PV and TT
    LOG_W << "Invalid PV. Researching with no TT at depth " << depth << END_W;

    state->search_in_tt = false;
    score = alpha_beta_negamax(MIN, MAX, depth, 0, board, state);

    assert(state->pv.pv_length[0] > 0);

    search_result.best_move = state->pv.pv_table[0][0];
  }

  // Just ot be sure assign again the fallback move
  if (!search_result.best_move) { search_result.best_move = moves[0]; }

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
