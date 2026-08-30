#include "search.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include "bitboard.hpp"
#include "evaluation.hpp"
#include "log.hpp"
#include "search_params.hpp"
#include "transposition_table.hpp"
#include "utils.hpp"


#define MATE_MAX 49000
#define MATE_MIN 48000
#define DRAW_SCORE 0

// The bottom of the full window. Its counterpart is SEARCH_SCORE_INF itself,
// named in search.hpp because a caller now has to be able to ask for it
// explicitly.
static constexpr int MIN = -SEARCH_SCORE_INF;


// QUIET_HISTORY_MAX, the six HISTORY_BONUS_*/HISTORY_MALUS_* coefficients,
// MAX_QSEARCH_DEPTH, RFP_MARGIN, RFP_MAX_DEPTH, RFP_MIN_PLY, NULL_MOVE_BASE,
// NULL_MOVE_DIVISOR, LMR_BASE and LMR_DIVISOR are in search_params.hpp with the
// rest of the tunable set, together with the comment that says what each one is
// for. S073.


void history_gravity_update(int16_t& entry, int bonus)
{
  const int max = QUIET_HISTORY_MAX;
  const int clamped = std::clamp(bonus, -max, max);

  assert(entry >= -max && entry <= max);

  // Multiply before dividing. The other association, `entry * (|b| / max)`,
  // truncates the whole decay term to zero at every bonus smaller than the
  // bound -- which is all of them -- and gravity silently stops ageing while
  // still looking like the published line.
  //
  // The interval is closed under this: `entry*(1 - |b|/max) + b` maps
  // [-max, max] onto itself for |b| <= max, and truncation toward zero moves
  // the result no further out than the exact value already is.
  const int updated = entry + clamped - (entry * std::abs(clamped)) / max;

  assert(updated >= -max && updated <= max);

  entry = static_cast<int16_t>(updated);
}


void history_on_quiet_cutoff(search_state_t* state,
                             color_t side,
                             move_t cutoff_move,
                             const move_t* quiets_tried,
                             size_t quiets_tried_count,
                             int depth)
{
  const int bonus = HISTORY_BONUS_QUAD * depth * depth +
                    HISTORY_BONUS_LIN * depth + HISTORY_BONUS_CONST;
  const int malus = HISTORY_MALUS_QUAD * depth * depth +
                    HISTORY_MALUS_LIN * depth + HISTORY_MALUS_CONST;

  // The maluses first, so that when two moves in the span share a butterfly
  // cell with the cutoff move -- two promotions from the same square, say --
  // the move that actually caused the cutoff is the one whose update lands
  // last. Same cell, different moves, is what butterfly indexing costs.
  for (size_t i = 0; i < quiets_tried_count; ++i) {
    assert(quiets_tried[i] != cutoff_move);

    history_gravity_update(state->quiet_history[side][MOVE_FROM(
                               quiets_tried[i])][MOVE_TO(quiets_tried[i])],
                           -malus);
  }

  history_gravity_update(
      state->quiet_history[side][MOVE_FROM(cutoff_move)][MOVE_TO(cutoff_move)],
      bonus);
}


using lmr_table_t = std::array<std::array<uint8_t, 64>, 64>;


// How much depth a late quiet move gives up, by remaining depth and by how far
// down the move order it is. Both axes are logarithmic: the first few moves are
// where the good ones live, so the penalty grows quickly at first and then
// flattens, and a deeper search can afford to give up more of it.
//
// Built once rather than computed per move - two logarithms in the innermost
// loop of the search is not a trade worth making.
static lmr_table_t build_lmr_table()
{
  lmr_table_t table{};

  for (int depth = 1; depth < 64; ++depth) {
    for (int move_number = 1; move_number < 64; ++move_number) {
      const double r =
          (LMR_BASE / 100.0) +
          (std::log(depth) * std::log(move_number)) / (LMR_DIVISOR / 100.0);
      table[depth][move_number] = static_cast<uint8_t>(r);
    }
  }

  return table;
}


#ifdef CHESSO_TUNE
// Mutable, and rebuilt by every setoption. A coefficient that moved without the
// table being rebuilt would report success and change nothing.
static lmr_table_t lmr_table = build_lmr_table();

void search_params_rebuild_derived()
{ lmr_table = build_lmr_table(); }
#else
static const lmr_table_t lmr_table = build_lmr_table();
#endif


static inline int lmr_reduction(int depth, int move_number)
{
  const int d = (depth < 63) ? depth : 63;
  const int m = (move_number < 63) ? move_number : 63;
  return lmr_table[d][m];
}


#ifdef CHESSO_TUNE
int search_lmr_reduction_probe(int depth, int move_number)
{ return lmr_reduction(depth, move_number); }
#endif


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


// Inclusive at +/-MATE_MAX where normalize_score() is exclusive, and the
// asymmetry is what makes the pair a round trip rather than nearly one.
// normalize_score() maps "mate in p plies, seen from ply p" onto exactly
// +/-MATE_MAX, so that is a value the table holds and this function has to
// undo; on the way in the same value means "mate here", which is already
// normalised and must be left alone. The bound was exclusive on both sides
// until S094 and nothing reached it: the main search never stores +/-MATE_MAX,
// because a mated node returns before it stores anything and a mating score at
// ply p is at most MATE_MAX - (p + 1). Quiescence does store it -- it is the
// only place a mate with no legal reply is both detected and written down --
// and the whole mate distance came back three plies short.
inline int de_normalize_score(int score, int ply)
{
  if (score > MATE_MIN && score <= MATE_MAX) { return score - ply; }
  if (score < -MATE_MIN && score >= -MATE_MAX) { return score + ply; }
  return score;
}


// negamax's smallest depth. It recurses with depth - 1 and only from depth >=
// 1, and every reduction is clamped to leave the child at least one ply, so no
// probe from the main search ever asks for less than this. TT_DEPTH_QS sits
// below it, which is what makes a quiescence entry unusable up there.
static_assert(TT_DEPTH_QS < 0,
              "a quiescence entry must not satisfy a main-search probe");

// The band the pair above recognises a mate score by. A mate at the deepest
// ply the search can reach is worth MATE_MAX - MAX_PLY, and that has to stay
// above MATE_MIN: below it, normalize_score() and de_normalize_score() stop
// recognising the number as a mate score, silently leave the ply term off it,
// and the distance stored is the distance seen from whichever node happened to
// write it. Nothing held this before S106 -- the margin is wide today (1000
// against 128) and it is a constant either side of it that would close it.
static_assert(MATE_MAX - MATE_MIN > MAX_PLY,
              "a mate score must not decay out of the mate band");


bool tt_entry_answers(const tt_entry_t* entry,
                      int depth,
                      size_t ply,
                      int alpha,
                      int beta,
                      int* score)
{
  assert(score != nullptr);

  if (entry == nullptr || entry->depth < depth) { return false; }

  // The stored score counts from the position it was stored at, so the
  // distance to this node has to come back out of it before it means anything
  // here. That is what "mate in N from here" costs, and it is the reason a
  // mate score is never read straight out of the entry.
  const int tt_score = de_normalize_score(entry->score, ply);

  if (entry->type == TT_PV_NODE) {
    *score = tt_score;
    return true;
  }

  if (entry->type == TT_BETA_NODE && tt_score >= beta) {
    *score = tt_score;
    return true;
  }

  if (entry->type == TT_ALPHA_NODE && tt_score <= alpha) {
    *score = tt_score;
    return true;
  }

  return false;
}


bool improving_at(const search_state_t* state, size_t ply, bool in_check)
{
  assert(state != nullptr);
  assert(ply < MAX_PLY);

  // In check the node has no static score of its own, so there is nothing that
  // could have improved. False rather than the no-data default: the node is
  // forced, and the consumers at S109 should treat it as the worse case.
  if (in_check) { return false; }

  const int current = state->static_evals[ply];

  // The offsets are even by necessity and not by taste. INV-5 values are
  // relative to the side to move, so an odd offset would compare White's score
  // against Black's; 2 is the nearest same-colour ancestor and 4 the next.
  //
  // Guarded on the ply and never on the value: the slot for a ply this search
  // has not reached holds 0, and 0 is an ordinary evaluation. A node at ply p
  // wrote its slot before it could recurse, so every slot below this one at
  // the same parity has been written by an ancestor of this node.
  if (ply >= 2 && state->static_evals[ply - 2] != TT_EVAL_NONE) {
    return current > state->static_evals[ply - 2];
  }

  // The node two plies up was in check and left the sentinel, so the nearest
  // ancestor carrying a number is four plies up. Without this the comparison
  // above runs against INT16_MIN and answers "improving" for every real
  // evaluation there is.
  if (ply >= 4 && state->static_evals[ply - 4] != TT_EVAL_NONE) {
    return current > state->static_evals[ply - 4];
  }

  // Nothing in reach to compare against, at the top of the tree or under two
  // checks. True, per the reference definition.
  return true;
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

  // The bound the node started with. What gets stored below is exact only if
  // the node beat it; otherwise all the search established is a ceiling.
  const int alpha0 = alpha;

  // Probed before anything is computed, so a hit pays for neither the
  // evaluation nor the move generation. TT_DEPTH_QS is below every entry in
  // the table, so this accepts a main-search entry as well as its own -- a
  // node searched with quiet moves has seen strictly more than this one needs.
  // The main search asks for its own depth and therefore accepts none of
  // these. S094.
  //
  // No PV guard, unlike negamax's probe. Quiescence is below the reported
  // line: negamax clears pv_length[ply] and returns this score without
  // touching the PV table, so there is no line here for a cutoff to chop.
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game->board);

  {
    int tt_score = 0;

    if (tt_entry_answers(tt_entry, TT_DEPTH_QS, ply, alpha, beta, &tt_score)) {
      return tt_score;
    }
  }

  // Lazy: quiescence is where the evaluation is called most, and most of those
  // nodes are nowhere near the window. S034.
  //
  // Except where the entry above already carries the number. evaluate() is a
  // function of the position alone and the key that matched covers every field
  // it reads, so recomputing it would produce what is already here -- and the
  // probe has been paid for whether or not it answered the node, which makes
  // the field free to read. What is stored is the score and never a bound, so
  // this is strictly the better of the two numbers: the shortcut would have
  // handed back cheap +/- LAZY_EVAL_MARGIN at the same node.
  //
  // INV-4 is untouched by this. The number was derived from the accumulators
  // make_move maintains, and nothing here stops maintaining them; what is
  // avoided is the second and third call that would rebuild the same score
  // from them. S094.
  bool static_eval_is_exact =
      tt_entry != nullptr && tt_entry->eval != TT_EVAL_NONE;

  const int static_eval =
      static_eval_is_exact
          ? tt_entry->eval
          : evaluate_lazy(&game->board, alpha, beta, &static_eval_is_exact);

  // Only a number the shortcut did not replace with a bound is worth keeping:
  // a bound holds on one side of one window and this entry will be read from
  // others. Every store below carries it, so a later reader finds the static
  // score wherever this node had one. S094.
  //
  // Fixed from the static number and never from the stand pat below it. The
  // substitution is window-relative -- it happens only where a bound held
  // against this node's window -- and the entry outlives the window, which is
  // the same reason a bound is never stored here. S130 changes nothing about
  // what is written.
  const int stored_eval = static_eval_is_exact ? static_eval : TT_EVAL_NONE;

  // The entry did not answer the node outright, but its score is still a bound
  // on this position, and a bound a search established beats a static guess.
  // Which way it may move the stand pat is exactly what the entry's type
  // certifies: a lower bound says the value is at least this and may only
  // raise, an upper bound says at most and may only lower, an exact score is
  // the value. Anything else consumes a claim the entry never made. S130.
  int stand_pat = static_eval;

  // And what the stand pat is worth as a *claim*, which is not the same
  // question. The static score is exact by quiescence's own definition -- the
  // value of a node no capture improves is what standing pat is worth -- so
  // it starts exact and only the substitution can weaken it, to whichever
  // bound the entry it came from carries. The store at the bottom reads this
  // back, because a node must never write a claim stronger than the weakest
  // thing that produced its value. DEC-102.
  node_type_t stand_pat_type = TT_PV_NODE;

  // Never a mate score, whichever bound carries it. A stand pat is a
  // positional claim and every path below hands it to the parent or stores it:
  // the fail-high store, the ply-cap return, the fail-soft floor. A mate
  // distance entering there is a mate no search found, which is the recurring
  // bug from the side that invents one rather than the side that hides one.
  //
  // Tested on the raw score, which is ply-independent: the band is 1000 wide
  // against a MAX_PLY of 128, so de-normalisation cannot carry a score over
  // either edge and the raw number is in the band exactly when the
  // de-normalised one is. The static_assert above the probe holds that width.
  if (tt_entry != nullptr && std::abs(tt_entry->score) <= MATE_MIN) {
    const int tt_score = de_normalize_score(tt_entry->score, ply);

    // The exact arm is unreachable under today's probe -- tt_entry_answers()
    // returns an exact entry whatever the window is, so one never gets here.
    // It is written anyway because the rule is stated by bound type and not by
    // which arms the probe happens to leave over.
    const bool substitutes =
        tt_entry->type == TT_PV_NODE ||
        (tt_entry->type == TT_BETA_NODE && tt_score > stand_pat) ||
        (tt_entry->type == TT_ALPHA_NODE && tt_score < stand_pat);

    if (substitutes) {
      stand_pat = tt_score;
      stand_pat_type = static_cast<node_type_t>(tt_entry->type);
    }
  }

  // Nothing below this line is stored when the search is being abandoned or
  // truncated. An aborted node has no score, and a node that gave up on the
  // ply cap returns a static number in place of a search - both would be read
  // back later as though a search had produced them.
  if (check_limits(state)) { return stand_pat; }
  if (ply + 1 >= MAX_PLY) { return stand_pat; }

  const bool in_check = is_check(game);

  // Standing pat means "I could just stop here", which is not on offer while in
  // check: the side to move is forced to reply. So no early return on the
  // static score, and every evasion is searched rather than only the captures.
  if (!in_check) {
    if (stand_pat >= beta) {
      // The most common thing quiescence ever concludes, and it is worth
      // keeping: evaluate_lazy() returns a genuine lower bound on this branch,
      // and the option to stand pat makes it a lower bound on the node too,
      // whatever the ply cap does below. There is no move to record with it.
      //
      // A lower bound is what this store claims, so a substitution cannot make
      // it false. A raised stand pat is below beta by construction and never
      // arrives here at all; a lowered one is below the static score, so
      // `value >= stand_pat` follows from `value >= static_eval`, which is the
      // claim the node was already making. DEC-102.
      tt_store_entry(state->tt, &game->board, TT_DEPTH_QS,
                     normalize_score(stand_pat, ply), TT_BETA_NODE, 0,
                     stored_eval);
      return stand_pat;
    }

    if (stand_pat > alpha) { alpha = stand_pat; }
  }

  // The depth bound applies to evasions as well, otherwise a perpetual check
  // would recurse without end. At the bound the static score is all there is.
  // The cast is the tune build's: a parameter is a plain int there and this
  // counter is a size_t, so the comparison is signed against unsigned. Both
  // sides are small and non-negative, so it is the same comparison either way.
  if (static_cast<int>(qply) >= MAX_QSEARCH_DEPTH) { return stand_pat; }

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

    // A capture that loses material to the recapture is not worth searching:
    // whatever it leads to, the side to move could have declined it and stood
    // pat instead. This is where quiescence spends most of its time, and these
    // subtrees are the bulk of it.
    //
    // Not while in check, where every move is forced and standing pat is not on
    // offer, so a losing capture may still be the only legal reply.
    // The cheap test first: most captures worth searching take something at
    // least as valuable as the piece taking it, and those cannot lose material
    // whatever the defenders do. Only the rest are worth an exchange analysis.
    if (!in_check && !capture_cannot_lose(&game->board, moves[i]) &&
        !see_ge(&game->board, moves[i], 0)) {
      continue;
    }

    scores[count] = capture_score(&game->board, moves[i]);
    moves[count] = moves[i];
    count++;
  }

  int best_value = in_check ? MIN : stand_pat;
  int legal_moves = 0;

  // The floor the maximum below starts from, and what is known about it. In
  // check the stand pat is not on offer, so it is not an input to the maximum
  // and carries nothing into what this node may claim. DEC-102.
  const node_type_t floor_type = in_check ? TT_PV_NODE : stand_pat_type;

  // What the value currently held is worth as a claim. It follows best_value:
  // while the floor is still the maximum, it is the floor's.
  node_type_t value_type = floor_type;

  // Left at zero until a move actually beats what standing pat already gave,
  // so a node that stood pat stores no move rather than an arbitrary one.
  move_t best_move = 0;

  for (size_t i = 0; i < count; ++i) {
    pick_next_move(moves, scores, count, i);

    if (!make_move(game, moves[i])) { continue; }

    legal_moves++;

    const int score =
        -quiescence(-beta, -alpha, ply + 1, qply + 1, game, state);

    unmake_move(game);

    if (state->aborted) { return stand_pat; }

    if (score >= beta) {
      tt_store_entry(state->tt, &game->board, TT_DEPTH_QS,
                     normalize_score(score, ply), TT_BETA_NODE, moves[i],
                     stored_eval);
      return score;
    }

    if (score > best_value) {
      best_value = score;
      best_move = moves[i];

      // A searched line is the maximum now, and the two substitutions do not
      // leave the same thing behind. A stand pat *raised* by a lower bound sat
      // above the static score, so a line that beat it beat the static score
      // too and the maximum is the one quiescence defines -- exact. A stand
      // pat *lowered* by an upper bound sat below the static score, so the
      // static score it displaced may beat this line as well; all that is
      // established is that the value is at least this much. Computed from
      // floor_type and never from itself, so a second improvement cannot
      // launder the first. DEC-102.
      value_type = (floor_type == TT_ALPHA_NODE) ? TT_BETA_NODE : TT_PV_NODE;
    }

    if (score > alpha) { alpha = score; }
  }

  // No legal reply to a check is mate. "No captures available" says nothing of
  // the sort, which is why this is guarded by in_check.
  if (in_check && legal_moves == 0) {
    const int mated = -(MATE_MAX - static_cast<int>(ply));

    // Exact, and no truncation is hiding in it: there is no legal move, so
    // there was nothing below this node to cut short. Stored normalised, which
    // makes it -MATE_MAX in the table - "mated here" - and turns back into the
    // right distance at whatever ply reads it.
    tt_store_entry(state->tt, &game->board, TT_DEPTH_QS,
                   normalize_score(mated, ply), TT_PV_NODE, 0, stored_eval);
    return mated;
  }

  // Exact only if something beat the bound this node was given; below it all
  // that was established is a ceiling. The value is what quiescence resolves
  // to and not what a full search would, since the losing captures were
  // declined - but that is the number this node already hands its parent, so
  // storing it adds no claim the search was not making already.
  //
  // And exact only where nothing weaker went into it. The window test above is
  // the right question once the value is known exactly; where the stand pat
  // was a bound, that bound is the ceiling on what may be claimed here and the
  // window test cannot raise it back. DEC-102.
  const node_type_t stored_type =
      (value_type == TT_PV_NODE)
          ? ((best_value > alpha0) ? TT_PV_NODE : TT_ALPHA_NODE)
          : value_type;

  tt_store_entry(state->tt, &game->board, TT_DEPTH_QS,
                 normalize_score(best_value, ply), stored_type, best_move,
                 stored_eval);

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

    // The clock alone is not a draw. FIDE 5.1.1 ends the game the moment
    // checkmate is delivered, and 9.6.2 states the same exception for the
    // 75-move rule in so many words, so the mate has to be ruled out before
    // the 100th halfmove is allowed to score anything. Measured, not argued:
    // on 7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60 at depth 1 -- the clock reaches
    // 100 on the mating move -- this engine reported cp 448 and played g2h3,
    // discarding a mate in one it had already generated. Stockfish scores the
    // same position Mate(+1). S162, 2026-08-22_adversarial-F03.
    //
    // generate_moves() emits legal moves only, so an empty list while in check
    // is mate by definition and no per-move legality pass is wanted here.
    // Neither call is paid at a node whose clock has not reached 100, which is
    // every node any benchmark visits.
    //
    // A mated node falls through rather than returning a mate score computed
    // here: the two sites that already know the distance -- negamax's own
    // no-legal-move return and quiescence's, one of which every path below
    // reaches -- are the tested ones, and a second copy of that arithmetic is
    // the S094 bug class waiting to be written. The insufficient-material test
    // immediately below cannot intercept it: over all 1700560 legal positions
    // it calls a draw (two kings, with at most one knight or bishop, either
    // side to move) exactly 0 are checkmate, enumerated with python-chess.
    if (game->board.halfmove_clock >= 100) {
      move_t replies[MAX_MOVES];

      const bool is_mated =
          is_check(game) &&
          generate_moves(game_tables(), &game->board, replies) == 0;

      if (!is_mated) { return DRAW_SCORE; }
    }

    // Nothing on the board can force mate, so there is nothing below this node
    // worth looking at. The root is exempt: it still has to produce a move.
    if (is_insufficient_material(&game->board)) { return DRAW_SCORE; }
  }

  // Reuse TT entry if found
  const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game->board);

  // Copied out now: a deeper node can overwrite this slot while we recurse, and
  // the move is wanted for ordering even when no cutoff is taken.
  const move_t tt_move = (tt_entry != nullptr) ? tt_entry->best_move : 0;

  // The static evaluation this position was already scored with, where an
  // earlier node recorded one. Copied out here for the reason above and read
  // by reverse futility below. TT_EVAL_NONE where there is nothing to read.
  // S103.
  const int tt_eval = (tt_entry != nullptr) ? tt_entry->eval : TT_EVAL_NONE;

  if (!is_pv && ply > 0) {
    int tt_score = 0;

    if (tt_entry_answers(tt_entry, depth, ply, alpha, beta, &tt_score)) {
      return tt_score;
    }
  }

  // Quiescence search in leaves
  if (depth < 1) { return quiescence(alpha, beta, ply, 0, game, state); }

  // Below the leaf test: every leaf used to pay for this and throw it away.
  const bool is_in_check = is_check(game);

  // The static evaluation of this node, computed once here and read by
  // everything below that wants one. Reverse futility was the only consumer
  // until S108 and computed its own inside its guard; improving and the
  // shallow-depth margins each need one at their own node, and every one of
  // them has to be the same number, so it is computed at the top instead.
  //
  // In check there is no number to have. The side to move is forced, a static
  // score bounds nothing, and evaluating a position whose king is attacked
  // prices material that is about to move. The sentinel goes in the slot and
  // improving_at() falls back four plies rather than two.
  //
  // evaluate() and never evaluate_lazy(): the lazy shortcut returns a bound
  // that holds against one window, and this number is compared against another
  // ply's and stored in an entry that outlives every window.
  //
  // The entry already has the number where the probe above found one. It is
  // the same number: evaluate() is a function of the position alone, the key
  // that matched covers every field it reads, and what is stored is always the
  // score and never the bound the lazy shortcut hands back in its place. So
  // this is a call skipped, not a value approximated. Instrumented and counted
  // rather than argued, at the reverse futility site this read was hoisted
  // out of: 3368027 of the 14589403 calls that site made over 300 positions at
  // depth 10, 23.1 %, and 0 of them disagreed with a fresh call. The rate is a
  // property of how warm the table is -- the same count over the three
  // search_bench positions at depth 12 reads 173440 of 1617617, 10.7 %, and
  // zero disagreements there too.
  //
  // INV-4 is untouched. The stored number was derived from the accumulators
  // make_move maintains and nothing here stops maintaining them; what is
  // avoided is rebuilding the same score from them a second time. S094, S103,
  // S108.
  const int static_eval =
      is_in_check
          ? TT_EVAL_NONE
          : ((tt_eval != TT_EVAL_NONE) ? tt_eval : evaluate(&game->board));

  // On every path that recurses, and before the first of them. A slot left
  // unwritten holds whatever the last node at this ply put there, which is a
  // different line, and improving would then compare against a position that
  // is not this one's ancestor -- an error that costs Elo without ever
  // crashing. Null move pruning below recurses before the move loop, so the
  // write cannot wait for it. S108.
  state->static_evals[ply] = static_eval;

  // Reverse futility pruning, also called static null move pruning. The null
  // move observation without the null move: if the static score is so far
  // above beta that the opponent cannot claw the difference back in the plies
  // that are left, the node fails high and nothing below it is worth searching.
  //
  // Where null move pruning pays a reduced search to find that out, this pays
  // one evaluate() and assumes RFP_MARGIN per ply. That is an assumption, not a
  // proof, which is why the guards are the same ones and why the bound on depth
  // is low - the further from the leaves, the less a static score says.
  //
  //   in check      the side to move is forced to reply, so "I could stop here"
  //                 is not on offer and the static score bounds nothing
  //   PV node       these lines get reported and played, and this returns a
  //                 bound rather than a score
  //   beta near mate  a fail-high against a mate bound would claim a mate this
  //                 never proved. A static score is never a mate score
  //   ply < 3       see RFP_MIN_PLY: the top of the tree decides the move
  //
  // Not guarded on game_phase: no pass is made here, so zugzwang does not enter
  // it. It is guarded on depth instead, which null move pruning is not.
  //
  // What this rule cannot do, and no setting of it can. The bound it returns is
  // a lower bound on the node, and a forced mate for the opponent is the one
  // thing that bound cannot respect: a static evaluation is never a mate score,
  // and in this engine it provably cannot approach one, because
  // evaluate_expensive() clamps the whole king-safety correction to
  // +/-LAZY_EVAL_MARGIN. S033 measured five guards against that and none of
  // them worked; the depth and ply bounds are what contain it. A mate deeper
  // than ply 3 can still be missed for an iteration, and no test covers that.
  //
  // The cast on ply is the tune build's, for the reason quiescence's depth
  // bound carries: a parameter is a plain int there and ply is a size_t.
  if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&
      depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {
    const int margin = RFP_MARGIN * depth;

    // Fail soft, and the bound returned is the one actually argued for: the
    // static score minus everything the opponent was assumed able to win back.
    if (static_eval - margin >= beta) { return static_eval - margin; }
  }

  // Null move pruning. Give the opponent a free move; if the position is still
  // good enough to fail high after that, it is won by so much that searching it
  // properly is wasted effort, and the whole subtree is skipped.
  //
  // The reasoning only holds where having to move is an advantage, which is why
  // the guards matter more than the idea:
  //
  //   in check      a pass is not merely illegal, the search after it is
  //                 meaningless - the king is captured
  //   PV node       these are the lines that get reported and played, and the
  //                 bound this returns is not a real score
  //   game_phase 0  zugzwang. With only kings and pawns left, being obliged to
  //                 move is often the losing part of the position, so "I am
  //                 fine even after passing" stops being evidence of anything.
  //                 This is what game_phase() was added for
  //   prev_move 0   the parent was itself a null move. Two passes in a row is
  //                 the same position with less depth
  //   beta near mate  a fail-high against a mate bound would return a mate
  //                 score this search never proved. **Both** edges, and the
  //                 negative one is the whole of S165: with beta <= -MATE_MIN
  //                 the node is a defender node inside a mate proof, so beta
  //                 is a mate bound against the side to move and any null
  //                 result clears it. The node then fails high on a reduced
  //                 search's word, and the reduced search is exactly the
  //                 instrument that misses mates -- this rule hid a mate in
  //                 two once already. Reverse futility guarded both edges from
  //                 the start and no comment, decision or step ever said why
  //                 this one did not. 2026-08-22_adversarial-F04.
  //
  //                 Measured before it was added, not argued: over 400 corpus
  //                 positions at depth 10 the band is reached on **0 of
  //                 301620** otherwise-eligible nodes, and over the 104
  //                 proved defender nodes of adocs/data/S165_defender_set.tsv
  //                 on **3079 of 6252, 49.2 %**. So it is inert in ordinary
  //                 play and fires on half the eligible nodes of a mate proof,
  //                 where it buys two more mates found on the defender sweep
  //                 with no delay regression anywhere.
  //
  // Deeper searches can afford to give up more, since what is left is still
  // enough to answer the question.
  const int null_reduction = NULL_MOVE_BASE + (depth / NULL_MOVE_DIVISOR);

  // The reduced search has to keep at least one real ply. Let it fall to zero
  // and it becomes pure quiescence, which only looks at captures and therefore
  // cannot see a mate that is two plies away - it answers with the static score
  // and the pass looks safe. That is not a theoretical risk: it lost a mate in
  // two at depth 4, where this node had three plies left and the null search
  // had none.
  if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&
      depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&
      game_phase(&game->board) > 0) {
    const int reduction = null_reduction;

    make_null_move(game);

    const int null_score = -negamax(-beta, -beta + 1, depth - 1 - reduction,
                                    ply + 1, game, state, 0, false);

    unmake_null_move(game);

    if (state->aborted) { return 0; }

    if (null_score >= beta) {
      // A mate score out of a null move search is not a mate anyone can force,
      // it is an artefact of the pass. Report the bound instead.
      return (null_score >= MATE_MIN) ? beta : null_score;
    }
  }

  node_type_t type = TT_ALPHA_NODE;

  int legal_moves_counter = 0;
  move_t moves[MAX_MOVES];
  int scores[MAX_MOVES];

  // The quiets this node actually searched, in search order, for the malus.
  // moves[0..i-1] is not that span: it holds captures, and it holds
  // pseudo-legal moves whose make_move failed. Both are published bugs -- Lynx
  // PR #610 charged the malus to illegal moves -- so the list is built at the
  // one place where legality and eligibility are both already known.
  move_t quiets_tried[MAX_MOVES];
  size_t quiets_tried_count = 0;

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

    // The late move reduction guard below is the only consumer: a move that
    // gives check is not reduced. It is deliberately *not* consulted by the
    // fail-high block, which admits every quiet that caused a cutoff to the
    // killer, history and countermove tables, checks included -- excluding
    // them made the engine refuse to remember the one class of refutation its
    // own reductions call forcing. S107.
    //
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
    const int child_depth = depth - 1;

    // Late move reduction. Move ordering puts the moves worth searching first,
    // so a quiet move this far down the list is unlikely to be the best one.
    // Searching it shallower costs almost nothing when that guess is right and
    // is paid back by a re-search when it is wrong.
    //
    // Not reduced: captures and promotions, because they are the tactics the
    // ordering already promoted; moves that give check or answer one, because
    // those lines are forcing and a shallow look at them is worthless; and the
    // first few moves, which is where the ordering expects the answer to be.
    //
    // The reduced search keeps at least one real ply, for the same reason null
    // move pruning does: at zero it becomes quiescence, which sees only
    // captures and will happily report that a quiet move is fine.
    int reduction = 0;

    if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&
        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {
      reduction = lmr_reduction(depth, static_cast<int>(legal_moves_counter));

      if (reduction > child_depth - 1) { reduction = child_depth - 1; }
      if (reduction < 0) { reduction = 0; }
    }

    if (legal_moves_counter == 1) {
      // The first legal move of a PV node continues the principal variation.
      score = -negamax(-beta, -alpha, child_depth, ply + 1, game, state,
                       moves[i], is_pv);
    } else {
      score = -negamax(-alpha - 1, -alpha, child_depth - reduction, ply + 1,
                       game, state, moves[i], false);

      // A reduced search that beats alpha has proved only that the reduction
      // was wrong, not what the move is worth. Repeat it at full depth before
      // believing anything.
      if (!state->aborted && reduction > 0 && score > alpha) {
        score = -negamax(-alpha - 1, -alpha, child_depth, ply + 1, game, state,
                         moves[i], false);
      }

      // Beat alpha without reaching beta, so the null window has told us the
      // move is interesting and nothing more. Only then is the full search
      // worth doing. Skipped when the search was abandoned mid-way, because
      // the score is meaningless then.
      if (!state->aborted && score > alpha && score < beta) {
        score = -negamax(-beta, -alpha, child_depth, ply + 1, game, state,
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
      if (!is_capture) {
        // The shift is deliberately unguarded, and that is a measured choice
        // rather than an oversight. A repeat copies slot 0 onto itself, so both
        // slots hold one move on 44 % of nodes and nothing distinct can reach
        // ORDER_KILLER_1. CPW's replacement rule says to guard it; guarding it
        // measured -11.02 +/- 10.53 Elo over 2522 games, H0 accepted. S149.
        state->killer_moves[1][ply] = state->killer_moves[0][ply];
        state->killer_moves[0][ply] = moves[i];

        // unmake_move ran above, so active_color is the side that played this
        // move again -- the same side score_move indexed the table with when it
        // ordered the node.
        history_on_quiet_cutoff(state, game->board.active_color, moves[i],
                                quiets_tried, quiets_tried_count, depth);

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

    // Appended last, past the `break` a cutoff takes, so the move that caused
    // the cutoff is structurally absent from the span the malus walks rather
    // than excluded from it by an index. Lynx shipped the inverse off-by-one --
    // dropping the last tried quiet to protect a cutoff move that was never in
    // the list -- and the fix alone measured +12.89 (PR #1756).
    //
    // The gate is `!is_capture` and nothing else, exactly the gate the bonus
    // uses above. An asymmetry between what can earn the bonus and what can
    // earn the malus is a bias with no symptom.
    if (!is_capture) {
      assert(quiets_tried_count < MAX_MOVES);
      quiets_tried[quiets_tried_count++] = moves[i];
    }
  }

  if (legal_moves_counter == 0) {
    return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;
  }

  assert(best_move != 0);

  // Only the root knows which move the caller is allowed to play. Publishing
  // from every ply meant an aborted search handed back a move belonging to a
  // deep node, and usually to the other side.
  if (ply == 0) {
    state->best_move = best_move;

    // A root fail-high leaves the two disagreeing, and S021 is what made a root
    // fail-high reachable: before aspiration windows the root was always
    // searched with beta = MAX and `score >= beta` could not happen there.
    //
    // The cutoff `break` above jumps out before the `score > alpha` block, so
    // the PV row still describes whatever earlier move last beat alpha while
    // best_move is the move that caused the cutoff. Measured, not reasoned
    // about: `position startpos moves e2e4 e7e5` then `go nodes 20000` reported
    // `best=b1c3 pv0=d2d4 pvlen=5` at depth 5, alpha -32, beta 68, score 68.
    //
    // The line is replaced rather than repaired because there is no line to
    // repair. A fail-high proves one thing about this node -- that best_move is
    // worth at least beta -- and nothing whatever about the continuation, so
    // one move is exactly what is known. The alternative, leaving the row
    // alone, is what shipped a `bestmove` that did not start the `pv` beside
    // it.
    if (best_so_far >= beta) {
      state->pv_table[0][0] = best_move;
      state->pv_length[0] = 1;
    }
  }

  const int to_store = normalize_score(best_so_far, ply);
  tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,
                 static_eval);

  return best_so_far;
}


search_t search(int depth,
                game_t* game,
                search_state_t* state,
                int alpha,
                int beta)
{
  assert(game != nullptr);
  assert(state != nullptr);
  assert(state->stop != nullptr);
  assert(state->tt != nullptr);
  assert(alpha < beta);

  search_t search_result = {};

  state->aborted = false;

  int score = negamax(alpha, beta, depth, 0, game, state, 0, true);

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
