#include "leaf_walk.hpp"
#include <cassert>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "search.hpp"
#include "search_params.hpp"
#include "transposition_table.hpp"

// S082. What the walk is and the policy behind each outcome: src/leaf_walk.hpp.


const char* leaf_outcome_name(leaf_outcome_t outcome)
{
  switch (outcome) {
    case LEAF_QUIET:
      return "quiet";
    case LEAF_TERMINAL:
      return "terminal";
    case LEAF_TRUNCATED:
      return "truncated";
    case LEAF_ABORTED:
      return "aborted";
    case LEAF_UNCERTIFIED:
      return "uncertified";
    case LEAF_OUTCOME_COUNT:
      break;
  }

  return "unknown";
}


// The smallest table the engine allocates. A quiescence tree from a game
// position fits it many times over, and it is cleared before every call, so
// its size is the cost of that clear and nothing else.
leaf_walker_t::leaf_walker_t() : state(std::make_unique<search_state_t>())
{
  tt_resize(&tt, TT_MIN_MB);
  state->stop = &stop;
  state->tt = &tt;
}


leaf_walker_t::~leaf_walker_t()
{ tt_free(&tt); }


int leaf_walk_ply_bound(const leaf_walker_t* walker)
{
  assert(walker != nullptr);
  return (walker->max_plies > 0) ? walker->max_plies : MAX_QSEARCH_DEPTH;
}


namespace
{

// Whether the rules decide this position as the search would see it from the
// sampled position: the fifty-move rule, material that cannot mate, no legal
// move, or a repetition classify_repetition() calls a draw with the sampled
// position as the root -- one inside the walk, or a third occurrence anywhere.
// A single occurrence before the root is not one: the search goes on
// searching such a position and evaluate() is asked about it, so refusing it
// would drop a real leaf. That is the difference from datagen's game-ending
// test, is_position_repeated(), which adjudicates a game on any two-fold.
bool rules_decide(game_t* game, size_t root_history_size)
{
  if (game->board.halfmove_clock >= 100) { return true; }
  if (is_insufficient_material(&game->board)) { return true; }

  if (classify_repetition(&game->history, &game->board, root_history_size) ==
      repetition_kind_t::DRAW) {
    return true;
  }

  move_t moves[MAX_MOVES];
  return generate_moves(game_tables(), &game->board, moves) == 0;
}


// A move from a table entry or a line is played only if the generator
// produces it here: generate_moves() is legal-only (INV-1), and a move from
// another position would be made onto a board it does not fit.
bool is_generated(const game_t* game, move_t move)
{
  if (move == 0) { return false; }

  move_t moves[MAX_MOVES];
  const size_t count = generate_moves(game_tables(), &game->board, moves);

  for (size_t i = 0; i < count; ++i) {
    if (moves[i] == move) { return true; }
  }

  return false;
}


// Walks from wherever `game` stands until quiescence stands pat or a policy
// refuses, counting every move it makes into `made` so the caller can unmake
// them all, its own and any the caller made before it. `root_history_size` is
// the history's size at the sampled position, where the walk began.
leaf_outcome_t walk_from(leaf_walker_t* walker,
                         game_t* game,
                         size_t root_history_size,
                         int* made)
{
  const int bound = leaf_walk_ply_bound(walker);
  int plies = 0;

  for (;;) {
    if (rules_decide(game, root_history_size)) { return LEAF_TERMINAL; }
    if (plies >= bound) { return LEAF_TRUNCATED; }

    int value = 0;
    move_t chosen = 0;
    bool aborted = false;

    if (!leaf_quiescence_choice(walker, game, &value, &chosen, &aborted)) {
      return aborted ? LEAF_ABORTED : LEAF_UNCERTIFIED;
    }

    if (chosen == 0) {
      // In check quiescence searches every evasion and chooses one whenever
      // one exists, and a position with none was refused as terminal above,
      // so standing pat in check is not a leaf. Out of check the value is the
      // stand pat, which on a cleared table at the widest window is
      // evaluate()'s own number.
      if (is_check(game) || value != evaluate(&game->board)) {
        return LEAF_UNCERTIFIED;
      }

      return LEAF_QUIET;
    }

    if (!is_generated(game, chosen) || !make_move(game, chosen)) {
      return LEAF_UNCERTIFIED;
    }

    (*made)++;
    plies++;
  }
}


// Records where the walk stopped and takes the game back to where it started.
void finish(game_t* game, int made, leaf_walk_t* walk)
{
  walk->distance = made;
  walk->fen = generate_FEN(&game->board);
  walk->phase = game_phase(&game->board);

  for (int i = 0; i < made; ++i) {
    unmake_move(game);
  }
}

}  // namespace


bool leaf_quiescence_choice(leaf_walker_t* walker,
                            game_t* game,
                            int* value,
                            move_t* chosen,
                            bool* aborted)
{
  assert(walker != nullptr && game != nullptr);

  // Cleared for every call. A bound left by an earlier call would be read at
  // this call's root as a substitute for the stand pat (S130), and a walk that
  // stood pat on that would record a position some capture still improves.
  tt_reset(&walker->tt);

  search_state_t* state = walker->state.get();
  state->aborted = false;
  state->explored_nodes = 0;
  state->node_limit = walker->node_cap;

  *value = quiescence(-SEARCH_SCORE_INF, SEARCH_SCORE_INF, 0, 0, game, state);
  *chosen = 0;
  *aborted = state->aborted;

  walker->nodes += state->explored_nodes;

  if (state->aborted) { return false; }

  const tt_entry_t* entry = tt_get_entry(&walker->tt, &game->board);

  if (entry == nullptr || entry->type != TT_PV_NODE) { return false; }

  *chosen = entry->best_move;
  return true;
}


leaf_walk_t leaf_walk_quiescence(leaf_walker_t* walker, game_t* game)
{
  assert(walker != nullptr && game != nullptr);

  leaf_walk_t walk;
  int made = 0;

  walk.outcome = walk_from(walker, game, game->history.size, &made);
  finish(game, made, &walk);

  return walk;
}


leaf_walk_t leaf_walk_pv(leaf_walker_t* walker, game_t* game, const pv_t* pv)
{
  assert(walker != nullptr && game != nullptr && pv != nullptr);

  leaf_walk_t walk;
  int made = 0;
  bool refused = false;
  const size_t root_history_size = game->history.size;

  for (size_t i = 0; i < pv->length && i < MAX_PLY; ++i) {
    const move_t move = pv->table[i];

    if (!is_generated(game, move) || !make_move(game, move)) {
      walk.outcome = LEAF_UNCERTIFIED;
      refused = true;
      break;
    }

    made++;

    if (rules_decide(game, root_history_size)) {
      walk.outcome = LEAF_TERMINAL;
      refused = true;
      break;
    }
  }

  walk.pv_plies = made;

  if (!refused) {
    walk.outcome = walk_from(walker, game, root_history_size, &made);
  }

  finish(game, made, &walk);

  return walk;
}
