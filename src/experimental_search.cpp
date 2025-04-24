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
// static constexpr int NO_SCORE = MAX + 42;


inline const tt_entry_t* get_entry_from_tt(const transposition_table_t* tt,
                                           const board_t* board,
                                           int depth)
{
  assert(tt != nullptr);
  assert(board != nullptr);
  assert(depth >= 0);

  const uint64_t hash = board->game_state.zobrist_key;
  const tt_entry_t* res = &tt->entries[hash % TT_SIZE];

  assert(res != nullptr);

  if (res->key == hash && res->depth >= depth) { return res; }
  // if (res->key == hash) { return res; }

  return nullptr;
}


void store_entry_to_tt(transposition_table_t* tt,
                       const board_t* board,
                       int depth,  // This is not the search depth but the ply
                       int score,
                       node_type_t type,
                       const move_t* best_move)
{
  const uint64_t hash = board->game_state.zobrist_key;
  tt_entry_t* entry = &tt->entries[hash % TT_SIZE];

  // // only apply replacement policy if same generation AND same key
  // if (entry->key == hash && entry->generation == tt->current_generation) {
  //   if (type == TT_ALPHA_NODE) {
  //     // new is an upperbound → only overwrite an existing upperbound
  //     // at lesser‐or‐equal depth
  //     if (entry->type != TT_ALPHA_NODE || depth < entry->depth) { return; }
  //   } else {
  //     // new is lowerbound or exact → only skip if:
  //     //   - it's shallower, AND
  //     //   - existing is NOT an upperbound
  //     if (depth < entry->depth && entry->type != TT_ALPHA_NODE) { return; }
  //   }
  // }

  entry->key = hash;
  entry->type = type;
  entry->depth = depth;
  entry->score = score;
  entry->best_move = *best_move;
  entry->generation = tt->current_generation;
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


  const int stand_pat =
      (board->game_state.active_color == WHITE ? 1 : -1) * evaluate(board);
  int best_value = stand_pat;

  state->explored_nodes += 1;

  if (qs_ply > 3) { return best_value; }


  if (alpha < stand_pat) {
    alpha = stand_pat;

    if (stand_pat >= beta) { return stand_pat; }
  }

  move_t moves[MAX_MOVES];
  const size_t moves_count = generate_legal_moves(board, moves);

  for (size_t i = 0; i < moves_count; ++i) {
    if ((moves[i].captured == INVALID || moves[i].captured == EMPTY) &&
        moves[i].promoted_to == TO_NONE) {
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


int negamax(int alpha,
            int beta,
            int depth,
            size_t ply,
            const board_t* board,
            search_state_t* state)
{
  int best_so_far = MIN;
  int alpha0 = alpha;

  // Reuse TT entry if found
  const tt_entry_t* tt_entry = get_entry_from_tt(state->tt, board, depth);
  if (ply > 0 && tt_entry != nullptr) {
    if (tt_entry->type == TT_PV_NODE) {
      state->best_move = tt_entry->best_move;
      return tt_entry->score;
    } else if (tt_entry->type == TT_BETA_NODE && tt_entry->score >= beta) {
      return tt_entry->score;
    } else if (tt_entry->type == TT_ALPHA_NODE && tt_entry->score <= alpha) {
      return tt_entry->score;
    }
  }

  // Leaf node
  if (depth < 1) { return quiescence_search(alpha, beta, 0, board, state); }

  state->explored_nodes += 1;

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

  move_t best_move = moves[0];

  for (size_t i = 0; i < moves_count; ++i) {
    board_t tmp_board = *board;

    make_move(&moves[i], &tmp_board, nullptr);
    const int score =
        -negamax(-beta, -alpha, depth - 1, ply + 1, &tmp_board, state);

    if (score >= best_so_far) {
      best_so_far = score;
      best_move = moves[i];

      if (score >= alpha) { alpha = score; }
      if (score >= beta) { break; }
    }
  }

  // Store the node in TT
  node_type_t type = TT_ALPHA_NODE;
  if (best_so_far <= alpha0) {
    type = TT_ALPHA_NODE;
  } else if (best_so_far >= beta) {
    type = TT_BETA_NODE;
  } else {
    type = TT_PV_NODE;
  }
  store_entry_to_tt(state->tt, board, depth, best_so_far, type, &best_move);


  state->best_move = best_move;
  return best_so_far;
}


void generate_pv(const board_t* board, search_state_t* state, int depth)
{
  assert(board != nullptr);
  assert(state != nullptr);
  board_t tmp_board = *board;
  state->pv.pv_length[0] = 0;

  const move_t* next_move = &state->best_move;
  while (next_move != nullptr && depth > 0) {
    state->pv.pv_table[0][state->pv.pv_length[0]] = *next_move;
    state->pv.pv_length[0]++;

    make_move(next_move, &tmp_board, nullptr);
    const tt_entry_t* next_entry =
        &state->tt->entries[tmp_board.game_state.zobrist_key % TT_SIZE];

    if (next_entry == nullptr ||
        next_entry->key != tmp_board.game_state.zobrist_key ||
        next_entry->type == TT_ALPHA_NODE) {
      return;
    }

    next_move = &next_entry->best_move;
    --depth;
  }
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

  const int score = negamax(MIN, MAX, depth, 0, board, state);

  // generate_pv(board, state, depth);

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
