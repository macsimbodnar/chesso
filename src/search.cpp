#include "search.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

// Keeps an accumulated history score from ever outranking a killer.
#define ORDER_HISTORY_MAX 600000

static constexpr int MIN = -2000000000;
static constexpr int MAX = 2000000000;


// NOTE: there is no null move pruning, no late move reduction and no PVS
// re-search yet. The tuning constants for them used to live here and read as
// if the pruning existed.
#define MAX_QSEARCH_DEPTH 8


// Selection sort, one step per visited move. Most nodes fail high on one of the
// first moves, so sorting the whole list up front would be wasted work.
inline void pick_next_move(move_t moves[], int scores[], size_t count, size_t i)
{
  size_t best = i;

  for (size_t j = i + 1; j < count; ++j) {
    if (scores[j] > scores[best]) { best = j; }
  }

  if (best != i) {
    std::swap(moves[i], moves[best]);
    std::swap(scores[i], scores[best]);
  }
}


inline bool check_limits(search_state_t* state)
{
  if (state->aborted) { return true; }

  const bool out_of_nodes = (state->node_limit != NODE_BUDGET_UNLIMITED) &&
                            (state->explored_nodes >= state->node_limit);

  // The node budget is a plain integer compare, so it can be enforced exactly.
  // Reading the stop flag is an atomic load, so it is only polled periodically.
  if (out_of_nodes || (((state->explored_nodes & 2047) == 0) && *state->stop)) {
    state->aborted = true;
    return true;
  }

  return false;
}


inline int normalize_score(int score, int ply)
{
  if (score > MATE_MIN && score < MATE_MAX) { return score + ply; }
  if (score < -MATE_MIN && score > -MATE_MAX) { return score - ply; }
  return score;
}


inline int de_normalize_score(int score, int ply)
{
  if (score > MATE_MIN && score < MATE_MAX) { return score - ply; }
  if (score < -MATE_MIN && score > -MATE_MAX) { return score + ply; }
  return score;
}


int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state)
{
  assert(game != nullptr);

  state->explored_nodes++;

  const int stand_pat = evaluate(&game->board);

  if (check_limits(state)) { return stand_pat; }
  if (ply + 1 >= MAX_PLY) { return stand_pat; }

  const bool in_check = is_check(game);

  // Standing pat means "I could just stop here", which is not on offer while in
  // check: the side to move is forced to reply. So no early return on the
  // static score, and every evasion is searched rather than only the captures.
  if (!in_check) {
    if (stand_pat >= beta) { return stand_pat; }
    if (stand_pat > alpha) { alpha = stand_pat; }
  }

  // The depth bound applies to evasions as well, otherwise a perpetual check
  // would recurse without end. At the bound the static score is all there is.
  if (qply >= MAX_QSEARCH_DEPTH) { return stand_pat; }

  move_t moves[MAX_MOVES];
  int scores[MAX_MOVES];

  // Out of check, this node only ever searches captures, so only captures are
  // generated. In check every evasion has to be considered, quiet ones
  // included, so the full list is needed.
  const size_t n = in_check
                       ? generate_moves(game_tables(), &game->board, moves)
                       : generate_captures(game_tables(), &game->board, moves);

  // Compacted to the front of the same array: captures only, or everything when
  // the move is forced. capture_score() ranks a quiet evasion below any
  // capture, which is the order wanted here too.
  //
  // The filter still runs when out of check: generate_captures() also emits
  // promotions that capture nothing, and this node did not search those before.
  // Keeping them out means this change is a pure speed-up with no effect on
  // what the search explores. Letting them through is very likely an
  // improvement, but it is a search change and needs to be measured in games.
  size_t count = 0;
  for (size_t i = 0; i < n; ++i) {
    if (!in_check && !MOVE_CAPTURE(moves[i])) { continue; }

    scores[count] = capture_score(&game->board, moves[i]);
    moves[count] = moves[i];
    count++;
  }

  int best_value = in_check ? MIN : stand_pat;
  int legal_moves = 0;

  for (size_t i = 0; i < count; ++i) {
    pick_next_move(moves, scores, count, i);

    if (!make_move(game, moves[i])) { continue; }

    legal_moves++;

    const int score =
        -quiescence(-beta, -alpha, ply + 1, qply + 1, game, state);

    unmake_move(game);

    if (state->aborted) { return stand_pat; }

    if (score >= beta) { return score; }
    if (score > best_value) { best_value = score; }
    if (score > alpha) { alpha = score; }
  }

  // No legal reply to a check is mate. "No captures available" says nothing of
  // the sort, which is why this is guarded by in_check.
  if (in_check && legal_moves == 0) {
    return -(MATE_MAX - static_cast<int>(ply));
  }

  return best_value;
}


// `is_pv` marks the nodes that lie on the principal variation: the root, and
// then the first move searched at every PV node. They are the only nodes whose
// PV row ends up in the reported line, so they never take a transposition table
// cutoff - a cutoff returns a score without a move sequence and would chop the
// PV short.
int negamax(int alpha0,
            int beta,
            int depth,
            size_t ply,
            game_t* game,
            search_state_t* state,
            move_t prev_move,
            bool is_pv)
{
  assert(game != nullptr);
  assert(state != nullptr);

  // Every path out of this function must leave the PV row for this ply valid,
  // so it is cleared before the early exits. Leaving it stale let a parent
  // memcpy a line belonging to an unrelated node into its own PV.
  state->pv_length[ply] = 0;

  // Counted before any of the early exits below, so the counter advances once
  // per visited node and the limit checks fire at a predictable rate.
  state->explored_nodes++;

  if (check_limits(state)) { return 0; }

  if (ply + 1 >= MAX_PLY) { return evaluate(&game->board); }

  int best_so_far = MIN;
  int alpha = alpha0;

  // Draws are a property of the path, not of the position, and the Zobrist key
  // does not carry the path. Both tests therefore have to run before the
  // transposition table is allowed to answer for this position.
  if (ply > 0) {
    if (is_position_repeated(&game->history, &game->board)) {
      return DRAW_SCORE;
    }

    if (game->board.halfmove_clock >= 100) { return DRAW_SCORE; }

    // Nothing on the board can force mate, so there is nothing below this node
    // worth looking at. The root is exempt: it still has to produce a move.
    if (is_insufficient_material(&game->board)) { return DRAW_SCORE; }
  }

  // Reuse TT entry if found
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game->board);

  // Copied out now: a deeper node can overwrite this slot while we recurse, and
  // the move is wanted for ordering even when no cutoff is taken.
  const move_t tt_move = (tt_entry != nullptr) ? tt_entry->best_move : 0;

  if (!is_pv && ply > 0 && tt_entry != nullptr && tt_entry->depth >= depth) {
    const int tt_score = de_normalize_score(tt_entry->score, ply);
    if (tt_entry->type == TT_PV_NODE) {
      return tt_score;
    } else if (tt_entry->type == TT_BETA_NODE && tt_score >= beta) {
      return tt_score;
    } else if (tt_entry->type == TT_ALPHA_NODE && tt_score <= alpha) {
      return tt_score;
    }
  }

  // Quiescence search in leaves
  if (depth < 1) { return quiescence(alpha, beta, ply, 0, game, state); }

  // Below the leaf test: every leaf used to pay for this and throw it away.
  const bool is_in_check = is_check(game);

  node_type_t type = TT_ALPHA_NODE;

  int legal_moves_counter = 0;
  move_t moves[MAX_MOVES];
  int scores[MAX_MOVES];

  // Staged generation. Most nodes fail high on one of the first few captures
  // and never look at a quiet move, so the quiets are not generated until the
  // captures have been exhausted without a cutoff.
  //
  // This does not change the order anything is searched in. score_move() puts
  // every capture and promotion at ORDER_CAPTURE or above, and the worst
  // possible capture - a king taking a pawn, 1000000 + 100 - 100000 - still
  // scores 900100, above the 900000 a killer gets. The two stages are already
  // disjoint bands, so selecting within each in turn is the same sequence as
  // selecting across both at once.
  //
  // The exception is the transposition table move, which outranks everything.
  // If it is a quiet then it has to be available before the captures are
  // searched, and this node generates both stages up front. Its flags say which
  // stage it belongs to without needing the move list to find out.
  const bool tt_move_is_quiet =
      (tt_move != 0) && !MOVE_CAPTURE(tt_move) && !MOVE_PROMOTED(tt_move);

  size_t moves_count = generate_captures(game_tables(), &game->board, moves);
  bool quiets_generated = false;

  // With no captures there is nothing to fail high on, so the second stage is
  // needed immediately and staging saves nothing here.
  if (tt_move_is_quiet || moves_count == 0) {
    moves_count +=
        generate_quiets(game_tables(), &game->board, moves + moves_count);
    quiets_generated = true;
  }

  for (size_t i = 0; i < moves_count; ++i) {
    scores[i] = score_move(game, state, moves[i], tt_move, ply, prev_move);
  }

  int score = 0;

  // Left at zero until a move is actually searched, so an aborted node never
  // publishes an unsearched move as the search result or as the TT move.
  move_t best_move = 0;

  for (size_t i = 0;; ++i) {
    if (i == moves_count) {
      // The captures ran out without a cutoff, so the quiets are needed after
      // all. This is the branch staging exists to avoid.
      if (quiets_generated) { break; }

      const size_t added =
          generate_quiets(game_tables(), &game->board, moves + moves_count);

      for (size_t j = moves_count; j < moves_count + added; ++j) {
        scores[j] = score_move(game, state, moves[j], tt_move, ply, prev_move);
      }

      moves_count += added;
      quiets_generated = true;

      if (i == moves_count) { break; }
    }

    pick_next_move(moves, scores, moves_count, i);

    if (!make_move(game, moves[i])) { continue; }

    const bool is_capture = MOVE_CAPTURE(moves[i]);

    // Only ever needed to decide whether a quiet move may become a killer, and
    // is_check() is an attack scan - do not pay for it on captures.
    const bool is_check_move = is_capture ? false : is_check(game);
    legal_moves_counter++;

    // Principal variation search. Move ordering is good enough that the first
    // move is usually the best one, which makes every later move a claim that
    // has to be *disproved* rather than measured. A null window - one point
    // wide, (alpha, alpha + 1) - answers "is this better than alpha?" and
    // nothing else, and a window that narrow cuts off far sooner than a full
    // one.
    //
    // When the answer comes back yes, the score is only a bound and the move
    // has to be searched again properly. That costs a whole re-search, which is
    // why this is a win only while the first move really is usually best; it
    // pays for the move ordering the rest of the engine does.
    if (legal_moves_counter == 1) {
      // The first legal move of a PV node continues the principal variation.
      score = -negamax(-beta, -alpha, depth - 1, ply + 1, game, state, moves[i],
                       is_pv);
    } else {
      score = -negamax(-alpha - 1, -alpha, depth - 1, ply + 1, game, state,
                       moves[i], false);

      // Beat alpha without reaching beta, so the null window has told us the
      // move is interesting and nothing more. Only then is the full search
      // worth doing. Skipped when the search was abandoned mid-way, because
      // the score is meaningless then.
      if (!state->aborted && score > alpha && score < beta) {
        score = -negamax(-beta, -alpha, depth - 1, ply + 1, game, state,
                         moves[i], is_pv);
      }
    }

    unmake_move(game);

    if (state->aborted) { return 0; }

    // best_move follows best_so_far, so a fail-low node still reports the move
    // it liked most and a fail-high node reports the move that caused the
    // cutoff (its score is above beta, hence above every earlier score).
    if (score > best_so_far) {
      best_so_far = score;
      best_move = moves[i];
    }

    if (score >= beta) {
      // Fail-high
      type = TT_BETA_NODE;

      // Store killing move, history, and counter move
      if (!is_capture && !is_check_move) {
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];

        // Saturating: an unbounded accumulator overflows in a long search.
        const int bonus = depth * depth;
        int& history =
            state->history_moves[MOVE_PIECE(moves[i])][MOVE_TO(moves[i])];
        history = std::min(history + bonus, ORDER_HISTORY_MAX);

        if (prev_move != 0) {
          state->counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)] =
              moves[i];
        }
      }

      break;
    }

    if (score > alpha) {
      alpha = score;
      type = TT_PV_NODE;

      // Publish the root move as soon as it is proven better, so a search that
      // is aborted mid-iteration still reports the best move it has completed.
      if (ply == 0) { state->best_move = moves[i]; }

      // Update triangular PV table
      state->pv_table[ply][0] = moves[i];
      memcpy(&state->pv_table[ply][1], state->pv_table[ply + 1],
             state->pv_length[ply + 1] * sizeof(move_t));
      state->pv_length[ply] = 1 + state->pv_length[ply + 1];
    }
  }

  if (legal_moves_counter == 0) {
    return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;
  }

  assert(best_move != 0);

  // Only the root knows which move the caller is allowed to play. Publishing
  // from every ply meant an aborted search handed back a move belonging to a
  // deep node, and usually to the other side.
  if (ply == 0) { state->best_move = best_move; }

  const int to_store = normalize_score(best_so_far, ply);
  tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move);

  return best_so_far;
}


search_t search(int depth, game_t* game, search_state_t* state)
{
  assert(game != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);

  search_t search_result = {};

  state->aborted = false;

  int score = negamax(MIN, MAX, depth, 0, game, state, 0, true);

  search_result.best_move = state->best_move;

  // A mate score is +-(MATE_MAX - ply of the mate), so the distance in plies
  // falls straight out of it. UCI wants full moves, and the side that delivers
  // the mate is given by the sign.
  const int plies_to_mate = MATE_MAX - std::abs(score);

  // The lower bound is now belt and braces: evaluate() no longer prices the
  // king, so a position with an unbalanced king count scores like any other
  // material imbalance instead of above every mate. It is kept because nothing
  // else stops a future term from returning something larger than MATE_MAX.
  search_result.mate_found =
      (plies_to_mate >= 0) && (plies_to_mate < (MATE_MAX - MATE_MIN));

  if (search_result.mate_found) {
    const int moves_to_mate = (plies_to_mate + 1) / 2;
    search_result.mate_in = (score > 0) ? moves_to_mate : -moves_to_mate;
  }

  search_result.explored_nodes = state->explored_nodes;
  search_result.score = score;
  search_result.pv.length = state->pv_length[0];

  memcpy(search_result.pv.table, state->pv_table[0],
         state->pv_length[0] * sizeof(move_t));

#ifndef NDEBUG
  // The caller is allowed to keep the root result of an aborted iteration, so
  // the invariant has to hold there too: whenever a PV exists it is legal and
  // starts with the reported best move. Having no PV at all is only valid for
  // an abort or a terminal position.
  if (search_result.pv.length > 0) {
    if (!is_pv_legal(game, &search_result.pv)) {
      LOG_E << "Illegal PV" << END_E;
    }

    assert(search_result.best_move == search_result.pv.table[0]);
  }
#endif

  return search_result;
}
