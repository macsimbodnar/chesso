#include "search.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#ifdef CHESSO_TUNE
// The late move pruning census below writes a file at exit, and only the tune
// build has one. Nothing the release binary compiles needs either header.
#include <cstdlib>
#include <fstream>
#endif
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
{ history_gravity_update(entry, bonus, QUIET_HISTORY_MAX); }


void history_gravity_update(int16_t& entry, int bonus, int max)
{
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


// The continuation table's graded update at one remaining depth. `share` is
// ContHistBonus or ContHistMalus, in thousandths of CONT_HIST_BOUND at
// CONT_HIST_REF_DEPTH; the depth grading is the published quadratic, and the
// unit is what gives the two axes resolution an integer coefficient on
// `depth * depth` does not have (src/search_params.hpp carries the argument).
//
// 64-bit intermediate because the numerator reaches
// CONT_HIST_BOUND * 1000 * MAX_DEPTH * MAX_DEPTH -- 5.2e11 at the declared
// ceilings, four orders past int32. It is not on the hot path: this runs once
// per quiet fail-high and not once per node.
static int continuation_grade(int share, int depth)
{
  assert(share >= 0);
  assert(depth >= 0);

  const int64_t numerator =
      static_cast<int64_t>(CONT_HIST_BOUND) * share * depth * depth;
  const int64_t denominator =
      1000LL * CONT_HIST_REF_DEPTH * CONT_HIST_REF_DEPTH;

  // Clamped here as well as inside history_gravity_update, so the return value
  // is an int by construction rather than by the caller's good behaviour.
  return static_cast<int>(
      std::min<int64_t>(numerator / denominator, CONT_HIST_BOUND));
}


void history_on_quiet_cutoff(search_state_t* state,
                             color_t side,
                             move_t cutoff_move,
                             const move_t* quiets_tried,
                             size_t quiets_tried_count,
                             int depth,
                             move_t prev_move)
{
  const int bonus = HISTORY_BONUS_QUAD * depth * depth +
                    HISTORY_BONUS_LIN * depth + HISTORY_BONUS_CONST;
  const int malus = HISTORY_MALUS_QUAD * depth * depth +
                    HISTORY_MALUS_LIN * depth + HISTORY_MALUS_CONST;

  // S222. Guards both spans below: 0 is not "no previous move" to the index
  // arithmetic, it is the real (W_PAWN, a8) cell, so the branch has to be
  // explicit rather than left to an index that would happily write it.
  const bool has_prev = prev_move != 0;
  const int cont_bonus = continuation_grade(CONT_HIST_BONUS, depth);
  const int cont_malus = continuation_grade(CONT_HIST_MALUS, depth);

  // The maluses first, so that when two moves in the span share a butterfly
  // cell with the cutoff move -- two promotions from the same square, say --
  // the move that actually caused the cutoff is the one whose update lands
  // last. Same cell, different moves, is what butterfly indexing costs.
  for (size_t i = 0; i < quiets_tried_count; ++i) {
    assert(quiets_tried[i] != cutoff_move);

    history_gravity_update(state->quiet_history[side][MOVE_FROM(
                               quiets_tried[i])][MOVE_TO(quiets_tried[i])],
                           -malus);

    // The same malus for the continuation table, charged to this node's own
    // (prev_move, quiets_tried[i]) cell and on that table's own scale.
    if (has_prev) {
      history_gravity_update(
          continuation_entry(state, prev_move, quiets_tried[i]), -cont_malus,
          CONT_HIST_BOUND);
    }
  }

  history_gravity_update(
      state->quiet_history[side][MOVE_FROM(cutoff_move)][MOVE_TO(cutoff_move)],
      bonus);

  if (has_prev) {
    history_gravity_update(continuation_entry(state, prev_move, cutoff_move),
                           cont_bonus, CONT_HIST_BOUND);
  }
}


// THE ACCUMULATOR'S UNIT, S236. Every input to a late move reduction -- the
// table below and the five node terms -- is carried in **ticks of a ply** and
// the sum is rounded to whole plies once, at `lmr_adjusted_reduction`'s return.
// Before that step the table truncated a `double` into a `uint8_t` and the node
// terms were whole plies added to it, so a term that wanted to say "reduce a
// third of a ply less here" could say nothing or say a whole ply, and S098
// verdict 1's history term had to be the second (DEC-213, two recorded zeros).
//
// **The term that unit was built for is not here.** S236 added it -- a late
// quiet's own history, scaled to a fraction of a ply -- and measured it twice:
// a walk at two plies of reach and a second walk at one, 15658 and 40000 games
// (DEC-231). The term left and the unit stayed, because it is behaviour-neutral
// at `LmrRoundBias` 0 and because S237 and S238 both want a reduction that can
// carry a fraction. What is here is therefore scaffolding that plays exactly as
// the whole-ply engine did, proved by the bench signature at every landing, and
// not a rule with an unmeasured effect.
//
// **A power of two, and it is a design constant and not a tuned one** (DEC-134
// (c)). Two properties decide it and neither is a guess at a good value:
//
//   the shift    a power of two makes the rounding divide an arithmetic right
//                shift, which is an exact floor for a negative sum where C's
//                `/` is a truncation toward zero -- and the sum is negative
//                whenever LmrPv outweighs a small table value. C++20 defines
//                `>>` on a signed value as a floor division; the static_assert
//                below pins that rather than trusting the sentence.
//   the range    the widest intermediate the accumulator can be given is a
//                term's own value scaled into ticks. The one S236 measured was
//                `hist_sum * LMR_SCALE`, where `hist_sum`'s band is
//                `QuietHistoryMax + ContHistWeight * CONT_HIST_BOUND / 100` --
//                17350 at the shipped values and 688107 at the two range tops,
//                so the product at 1024 was 7.05e8, a factor of three inside
//                `int32_t` at the worst vector a tune build could be driven to.
//                The next power of two would have left that worst case at
//                1.41e9, inside the type but with no headroom worth the extra
//                bit of resolution. A later term's own band is checked the same
//                way against the same number.
//
// One tick is under a thousandth of a ply, which is finer than any term this
// engine has had: the history term's own census put the 90th percentile sum at
// half a ply, 512 ticks.
inline constexpr int LMR_SCALE_SHIFT = 10;
inline constexpr int LMR_SCALE = 1 << LMR_SCALE_SHIFT;

static_assert((-1 >> 1) == -1,
              "the reduction's rounding is an arithmetic shift: a negative sum "
              "must floor, not truncate toward zero");


// The table is `int32_t` and no longer `uint8_t`: at the range tops of LmrBase
// and LmrDivisor the formula reaches 1720.5 plies, which is 1.76e6 ticks, so
// nothing narrower holds the whole of what a tune build can ask for. That is
// 16 KB against the 4 KB of whole plies, and the price is paid for the
// resolution rather than saved by capping a value a sweep can still reach.
using lmr_table_t = std::array<std::array<int32_t, 64>, 64>;


// How much depth a late quiet move gives up, by remaining depth and by how far
// down the move order it is. Both axes are logarithmic: the first few moves are
// where the good ones live, so the penalty grows quickly at first and then
// flattens, and a deeper search can afford to give up more of it.
//
// Built once rather than computed per move - two logarithms in the innermost
// loop of the search is not a trade worth making.
//
// **In ticks since S236.** The cast still truncates and the formula is
// untouched, so the whole-ply value the engine shipped before this step is
// `table[depth][move_number] >> LMR_SCALE_SHIFT` exactly: flooring a floored
// product by a power of two is flooring the real value once
// (`floor(floor(r * S) / S) == floor(r)` for `r >= 0`, and `r` is
// non-negative here because LmrBase is and both logarithms are).
static lmr_table_t build_lmr_table()
{
  lmr_table_t table{};

  for (int depth = 1; depth < 64; ++depth) {
    for (int move_number = 1; move_number < 64; ++move_number) {
      const double r =
          (LMR_BASE / 100.0) +
          (std::log(depth) * std::log(move_number)) / (LMR_DIVISOR / 100.0);
      table[depth][move_number] = static_cast<int32_t>(r * LMR_SCALE);
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


static inline int lmr_reduction_ticks(int depth, int move_number)
{
  const int d = (depth < 63) ? depth : 63;
  const int m = (move_number < 63) ? move_number : 63;
  return lmr_table[d][m];
}


// The one place ticks become plies, S236. `LMR_ROUND_BIAS` is added before the
// shift, so the parameter *is* the rounding rule: 0 floors, which is what the
// `uint8_t` cast did to the table before that step, is the rule's off value and
// is **what ships** -- round-to-nearest loses a mate the fast suite guards, and
// every bias from one tick upward did so with S236's history term live.
// LMR_SCALE / 2 rounds to nearest and the range top rounds up. Nothing else in
// the search rounds a reduction, which is the property the accumulator exists
// for.
//
// The shift and not `/ LMR_SCALE`: the sum is negative at a PV node whose table
// value is small, and C's division would truncate that toward zero -- the same
// input rounding two ways depending on its sign. The static_assert above holds
// the shift to a floor.
static inline int lmr_plies_of(int ticks)
{ return (ticks + LMR_ROUND_BIAS) >> LMR_SCALE_SHIFT; }


int search_lmr_scale_probe()
{ return LMR_SCALE; }


int search_lmr_reduction_ticks_probe(int depth, int move_number)
{ return lmr_reduction_ticks(depth, move_number); }


int search_lmr_reduction_probe(int depth, int move_number)
{ return lmr_plies_of(lmr_reduction_ticks(depth, move_number)); }


// THE SCALE THE REVERSE-FUTILITY RETURN IS BLENDED ON, S235. `RfpReturnWeight`
// is in hundredths of the gap between beta and the bound that site's own test
// argued for, and this is the hundred. A definition and not a setting: it fixes
// the unit the parameter's range is declared in, and the probe below is how
// tests/test_search.cpp holds the two together, so a scale that moved without
// the range moving fails there instead of quietly making 100 something other
// than the rule's off value.
//
// Percent and not a power of two. The blend divides a gap that is non-negative
// at its one site, where `/` already floors, so the exact shift LMR_SCALE's
// power of two buys the reduction buys nothing here; percent is the unit this
// engine states its other shares in (TmSoftPercent, ContHistWeight,
// TmNodeScalePct).
inline constexpr int RFP_RETURN_SCALE = 100;


int search_rfp_return_scale_probe()
{ return RFP_RETURN_SCALE; }


// What the table above is worth once the **node's own type** is taken into
// account, S098 verdict 2 and S095. Five signed plies, each behind its own
// constant whose off value is 0, and every one of them a property of the node
// rather than of the move -- which is why the whole adjustment is computed once
// per node and passed in here as a single integer.
//
// **Returned in the accumulator's ticks since S236**, one ply being LMR_SCALE
// of them, so the sum this joins is in one unit and rounds once. The five
// constants stay whole plies on the UCI surface and in their declared ranges:
// what changed is where they are scaled, not what they mean, and a whole
// number of plies added to a tick sum survives the rounding unchanged --
// `(t + k * LMR_SCALE + bias) >> shift` is `((t + bias) >> shift) + k` for any
// integer k, which is why the off configuration is the parent exactly.
//
//   + LMR_CUTNODE        this node is predicted to fail high, so a late quiet
//                        is even less likely to be the move that does it
//   + LMR_NOT_IMPROVING  the side to move is worse off than it was two plies
//                        ago; the later quiets are worth less of a look
//   + LMR_TT_CAPTURE     the entry's move is a capture, so this node is
//                        tactical and the quiet under the table is not what it
//                        is about
//   - LMR_PV             a principal variation node, whose line gets reported
//                        and played
//   + LMR_NO_TT_MOVE     the entry carries no move at all -- no entry, or one
//                        only quiescence ever wrote -- so nothing has searched
//                        this node properly yet and it is cheaper than its
//                        depth claims. S095
//
// The fifth input is appended rather than slotted in beside the other additive
// terms: the four before it keep their positions, and a caller that transposed
// two bools would be a bug no type can catch. The sum does not care about the
// order.
//
// Unclamped, exactly as the raw table is: the two call sites clamp, and they
// clamp differently -- the reduction against the child's depth, the gate at
// zero from below. A helper that clamped here would have to pick one.
static inline int lmr_node_adjustment(bool cut_node,
                                      bool improving,
                                      bool tt_move_is_capture,
                                      bool is_pv,
                                      bool no_tt_move)
{
  int adjustment = 0;

  if (cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }
  if (!improving) { adjustment += LMR_NOT_IMPROVING * LMR_SCALE; }
  if (tt_move_is_capture) { adjustment += LMR_TT_CAPTURE * LMR_SCALE; }
  if (is_pv) { adjustment -= LMR_PV * LMR_SCALE; }
  if (no_tt_move) { adjustment += LMR_NO_TT_MOVE * LMR_SCALE; }

  return adjustment;
}


int search_lmr_node_adjustment_probe(bool cut_node,
                                     bool improving,
                                     bool tt_move_is_capture,
                                     bool is_pv,
                                     bool no_tt_move)
{
  return lmr_node_adjustment(cut_node, improving, tt_move_is_capture, is_pv,
                             no_tt_move);
}


// The reduction a move actually gets: the table's own guess at this (depth,
// move number), moved by the node's type, summed in ticks and **rounded to
// whole plies once, here**. This is the only rounding site in the reduction,
// which is what the accumulator is for: a term worth a third of a ply can move
// the reduction where a third of a ply crosses the boundary, instead of having
// to choose between nothing and a whole ply.
//
// **No term in the tree needs that yet.** S236 added one -- a late quiet's own
// history, scaled to a fraction -- and its two verdicts read a walk and then a
// zero at half the reach, so the term left and the unit stayed (DEC-231). The
// five node constants are whole plies, `LmrRoundBias` ships at 0, and this
// function is therefore the parent's own arithmetic exactly: the table
// truncated, plus whole plies. What it buys is that S237 and S238 can express a
// fraction without moving the rounding again.
//
// With the four node constants at 0 this is the raw table and the engine is the
// one before S098 verdict 2, bench signature included -- the property the
// bisection protocol rests on.
static inline int lmr_adjusted_reduction(int depth,
                                         int move_number,
                                         int node_adjustment)
{
  return lmr_plies_of(lmr_reduction_ticks(depth, move_number) +
                      node_adjustment);
}


int search_lmr_adjusted_reduction_probe(int depth,
                                        int move_number,
                                        int node_adjustment)
{ return lmr_adjusted_reduction(depth, move_number, node_adjustment); }


// The depth the shallow-depth rules are gated on: what late move reduction
// would leave below this move, and not the node's own remaining depth. A move
// the ordering put late is already searched shallower than the node is deep, so
// the margin it is pruned against should be the shallow one. Clamped at zero --
// the reduction can exceed the depth on a very late move at a shallow node, and
// a negative margin multiplier would invert every rule below it. S109.
//
// It reads the **same adjusted reduction the move is searched with** since S098
// verdict 2: the gate and the reduction are one number, so a node type that
// reduces harder also prunes earlier, and the off values restore S109's exact
// gate in one release rebuild. `node_adjustment` is never negative at a call
// site of this function -- the PV term is the only negative one and a PV node
// is not a pruning node -- but nothing here depends on that, since the clamp
// below is on the result.
static inline int lmr_depth_of(int depth, int move_number, int node_adjustment)
{
  const int left =
      depth - lmr_adjusted_reduction(depth, move_number, node_adjustment);

  return (left > 0) ? left : 0;
}


// The depth the zero-window re-search of a reduced move runs at, S098
// verdict 3. Until this verdict it was `child_depth` always; it now answers the
// reduced search that produced it, and it answers it **only upward**:
//
//   deeper     the move cleared the node's own fail-soft best by
//              LMR_DEEPER_MARGIN with a reduction of at least
//              LMR_DEEPER_MIN_REDUCTION, so the reduction was wrong by a lot
//              and this is the move the node is about
//   otherwise  `child_depth`, which is where the re-search ran before this
//              step and where it still runs at everything else
//
// THE OTHER HALF WAS MEASURED HERE AND IS GONE. Verdict 3 shipped a second
// path beside this one -- a ply *off* the re-search where the reduced score
// beat alpha by less than a margin of its own -- and the pair measured
// `Elo -9.97 +/- 7.56` over 4496 games, the whole interval below zero. The
// pre-registered bisection's leg 1 switched that path off against the same
// reference and read `Elo 5.75 +/- 4.37`, `nElo 7.44 +/- 5.65` over 14510
// games, so the deeper path alone is what gains and the shallower one was the
// loss. It left with its margin, with the `reduction >= 2` guard that existed
// only to keep it off a depth the reduced search had already run, and with the
// floor at 1, which nothing else could reach.
//
// `best` is `best_so_far` before this move -- the fail-soft base, not alpha.
// The two are the same number only at a node that has already raised alpha;
// everywhere else `best` sits below it, which is why the deeper margin is
// measured from `best` and not from the window. It is also why the path fires
// at all: at a scout node that has not yet found a move `best` is far below
// the window, and the census measured the condition true on 6.89 % of
// re-search sites at depth 12.
//
// THE RE-SEARCH IS NEVER THE REDUCED SEARCH REPEATED: the returned depth is
// always strictly greater than `child_depth - reduction`, and with the
// shallower path gone that is arithmetic -- the rule returns `child_depth` or
// one more, and the site's own reduction is at least 1. The guard that remains
// is the tunable one, LMR_DEEPER_MIN_REDUCTION, and it is the published one:
// the bare form measured negative where the guarded form measured positive
// (S098 section 1(d)).
//
// WHAT IT HANDS BACK, AND WHY IT IS TWO NUMBERS. The depth, and **the base it
// measured the deeper margin from**, echoed straight back out. The second one
// buys a test the first cannot: a case that replays this function on the
// numbers the node recorded moves both sides of the comparison together, so it
// is blind to a *call site* that hands over the wrong variable -- and the two
// candidates here are both plain `int`s a line of code apart. The echo is what
// a case reads to assert what the site actually passed instead of what it
// should have passed. It is free in the shipping build: nothing outside the
// probe block reads it, so the field folds away in `negamax_at<false>`.
struct research_decision_t
{
  int depth;
  int base;
};


// The cap at `child_depth + 1` never binds at any input the engine itself
// produces: the rule answers `child_depth` or one more and nothing else. It is
// kept anyway -- S127 tunes the branch set and S097 extends `child_depth`, and
// a bound that has to be put back later is one somebody has already shipped
// without. A mutant that deletes it alone is equivalent for that reason and is
// declared so rather than written; the killable forms are the bound moved up
// with the branch that would then reach it and the bound moved down on its own,
// which is what tools/mutants/S098_research_rule.py holds.
//
// **The floor at 1 left with the shallower path.** It could only ever bind
// below `child_depth`, and nothing goes there any more; the assertion below
// keeps the bound as a claim about the result, where it is now a consequence of
// the preconditions rather than something a clamp has to produce.
//
// `alpha` is the node's own window and it is read by nothing here since the
// shallower path left: what the rule compares against is the fail-soft best,
// which is the published re-basing and the one thing D09 exists to hold. The
// parameter stays for two reasons and both are load-bearing. It is the
// precondition this function asserts -- a re-search happens only where the
// reduced score beat the window -- and it is the wrong variable the deeper
// margin could be measured from, so a rule that reads it instead of `best` is a
// bug a case can drive rather than one no signature admits.
static inline research_decision_t lmr_research_depth(int child_depth,
                                                     int reduction,
                                                     int score,
                                                     [[maybe_unused]] int alpha,
                                                     int best)
{
  // The site's own preconditions: this is the re-search a *reduced* move that
  // beat alpha is owed, at the child of a node deep enough to have reduced --
  // `child_depth` is `depth - 1` and the reduction is eligible only from depth
  // 3. All three are what the bounds below rest on.
  assert(child_depth >= 1);
  assert(reduction >= 1);
  assert(score > alpha);

  int depth = child_depth;

  if (reduction >= LMR_DEEPER_MIN_REDUCTION &&
      score > best + LMR_DEEPER_MARGIN) {
    depth = child_depth + 1;
  }

  if (depth > child_depth + 1) { depth = child_depth + 1; }

  assert(depth <= child_depth + 1);
  assert(depth >= 1);
  assert(depth > child_depth - reduction);

  return {depth, best};
}


int search_lmr_research_depth_probe(int child_depth,
                                    int reduction,
                                    int score,
                                    int alpha,
                                    int best)
{ return lmr_research_depth(child_depth, reduction, score, alpha, best).depth; }


// CPW's Node Types, and the whole of what this engine predicts about a child
// before searching it. The pair the search carries names three labels and only
// three:
//
//   PV   (is_pv, !cut_node)   on the principal variation
//   CUT  (!is_pv, cut_node)   expected to fail high
//   ALL  (!is_pv, !cut_node)  expected to fail low
//
// `is_pv && cut_node` is not a node type; negamax_at asserts it never happens.
//
// The rules are Onno Garms's, with Pradu Kannan's summary beside them on the
// same page (https://www.chessprogramming.org/Node_Types). They agree
// everywhere except the child after a null move, where this engine follows
// Kannan -- see null_move_child below. A wrong prediction is silent: no crash,
// no wrong node count, only rating, which is why each rule is a named function
// a test can walk (search_child_label_probe) instead of an expression inlined
// at its recursion.
struct child_label_t
{
  bool is_pv;
  bool cut_node;
};


// "The first child of a PV-node is a PV-node"; "The first child of a CUT-node
// is an ALL-node"; "Children of ALL-nodes are CUT-nodes" (Garms). One
// expression covers all three: only an ALL parent hands its first child the
// CUT label.
static constexpr child_label_t first_child(bool is_pv, bool cut_node)
{ return {is_pv, !is_pv && !cut_node}; }


// "The further children are searched by a scout search as CUT-nodes" and
// "Further children of a CUT-node are CUT-nodes" (Garms); "Children of
// PV-nodes that are searched with a zero-window scout search are Cut-nodes"
// (Kannan). Every parent label gives the same answer, so the parent is not
// read.
static constexpr child_label_t scouted_child(bool, bool)
{ return {false, true}; }


// The zero-window repeat a reduced move that beat alpha is owed. Still a
// scout -- the move has beaten alpha on a shallower search and nothing more --
// so it keeps the scouted label. Kannan's "re-searched because the scout
// search failed high, are PV-nodes" is the **full-window** re-search below and
// not this one; conflating the two would label a zero-window child PV, which
// is the one node type this engine has never searched with a zero window.
static constexpr child_label_t zero_window_research(bool is_pv, bool cut_node)
{ return scouted_child(is_pv, cut_node); }


// The full-window re-search a move that beat alpha without reaching beta gets.
// "PVS re-search is done as PV-node" (Garms); "Children of PV-nodes that have
// to be re-searched because the scout search failed high, are PV-nodes"
// (Kannan). At a PV parent that is PV, and `is_pv` already carried it before
// this step; at any other parent -- reachable only from a probe driving a
// non-PV node with a full window, since a zero window cannot produce
// `alpha < score < beta` -- it is ALL, which is the label a node about to raise
// alpha has.
static constexpr child_label_t full_window_research(bool is_pv, bool)
{ return {is_pv, false}; }


// The child after a null move. **The two published lists disagree here and
// this follows Kannan**, which is what S098's file names: the null move is one
// of a Cut-node's "candidate cutoff moves", and Kannan's line makes those
// children All-nodes, so a CUT parent's null child is ALL and -- by "Children
// of All-nodes are Cut-nodes" -- an ALL parent's is CUT. Garms's list says
// "The node after a null move is a CUT-node" unconditionally.
//
// Kannan's reading is also the one the window argues for: the parent passes
// (-beta, -beta+1) hoping the child comes back at or below -beta, which is the
// child failing low, which is an All-node. The null-move block never runs at a
// PV node, so the pair returned is never (true, ...).
static constexpr child_label_t null_move_child(bool, bool cut_node)
{ return {false, !cut_node}; }


void search_child_label_probe(int kind,
                              bool parent_is_pv,
                              bool parent_cut_node,
                              bool* child_is_pv,
                              bool* child_cut_node)
{
  child_label_t label = {false, false};

  switch (kind) {
    case CHILD_FIRST:
      label = first_child(parent_is_pv, parent_cut_node);
      break;
    case CHILD_SCOUT:
      label = scouted_child(parent_is_pv, parent_cut_node);
      break;
    case CHILD_ZW_RESEARCH:
      label = zero_window_research(parent_is_pv, parent_cut_node);
      break;
    case CHILD_FULL_RESEARCH:
      label = full_window_research(parent_is_pv, parent_cut_node);
      break;
    case CHILD_NULL_MOVE:
      label = null_move_child(parent_is_pv, parent_cut_node);
      break;
    default:
      break;
  }

  *child_is_pv = label.is_pv;
  *child_cut_node = label.cut_node;
}


// The move count late move pruning stops generating quiets past, in hundredths
// of a move -- the same reason LMR_BASE and LMR_DIVISOR are hundredths: the
// fit that produced it is not an integer and rounding it to one at the source
// throws away what the census measured.
//
// Doubled when the side to move is improving, which is S108's flag doing the
// one job the published record prices highest for it: a position that is
// getting better is one where the later quiets are worth another look.
static inline int lmp_threshold_x100(int lmr_depth, bool improving)
{
  const int threshold = LMP_BASE + LMP_DEPTH_COEFF * lmr_depth;

  return improving ? 2 * threshold : threshold;
}


int search_lmp_threshold_probe(int lmr_depth, bool improving)
{ return lmp_threshold_x100(lmr_depth, improving); }


#ifdef CHESSO_TUNE
// The census the late move pruning constants were fitted from, kept in the
// build that cannot ship so the fit can be re-run rather than re-read. Two
// tables and the reduction the third is derived from:
//
//   cutoff[depth][n]   a beta cutoff on a quiet move at this remaining depth,
//                      taken by the n-th legal move searched at the node
//   quiets[depth][k]   a node at this remaining depth generated k quiet moves
//
// Written on every tune-build search and dumped at exit only when
// CHESSO_LMP_CENSUS names a file, so a tuning run pays two increments and no
// I/O. Never compiled into the binary an SPRT measures. S109,
// adocs/data/S109_lmp_census.py.
namespace
{
constexpr int CENSUS_DEPTHS = 64;
constexpr int CENSUS_MOVES = 64;
constexpr int CENSUS_QUIETS = 256;

struct lmp_census_t
{
  uint64_t cutoff[CENSUS_DEPTHS][CENSUS_MOVES] = {};
  uint64_t quiets[CENSUS_DEPTHS][CENSUS_QUIETS] = {};

  ~lmp_census_t();
};

lmp_census_t census;

lmp_census_t::~lmp_census_t()
{
  const char* path = std::getenv("CHESSO_LMP_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "# S109 late move pruning census. depth is remaining depth at the\n"
         "# node; move is the index of the legal move searched, from 1.\n"
         "kind\tdepth\tmove\tcount\n";

  for (int depth = 0; depth < CENSUS_DEPTHS; ++depth) {
    for (int move = 0; move < CENSUS_MOVES; ++move) {
      if (census.cutoff[depth][move] != 0) {
        out << "cutoff\t" << depth << '\t' << move << '\t'
            << census.cutoff[depth][move] << '\n';
      }
    }
  }

  for (int depth = 0; depth < CENSUS_DEPTHS; ++depth) {
    for (int count = 0; count < CENSUS_QUIETS; ++count) {
      if (census.quiets[depth][count] != 0) {
        out << "quiets\t" << depth << '\t' << count << '\t'
            << census.quiets[depth][count] << '\n';
      }
    }
  }

  // The reduction table the analysis turns (depth, move) into an lmr depth
  // with, emitted by the same binary so the script does none of the engine's
  // own arithmetic a second time.
  //
  // **Whole plies, under this tree's own rounding rule, and four columns.**
  // Since S236 the table holds ticks, and what this row carries is what the
  // engine would use for that cell -- `LmrRoundBias` applied -- because that is
  // the number `adocs/data/S109_lmp_census.py` turns into an lmr depth. The
  // column count is part of that file's parser, so the ticks are not appended
  // here: `search_lmr_reduction_ticks_probe` is where a reader gets them.
  for (int depth = 1; depth < CENSUS_DEPTHS; ++depth) {
    for (int move = 1; move < CENSUS_MOVES; ++move) {
      out << "reduction\t" << depth << '\t' << move << '\t'
          << lmr_plies_of(lmr_reduction_ticks(depth, move)) << '\n';
    }
  }
}

inline void census_cutoff(int depth, int move_number)
{
  if (depth < 0 || depth >= CENSUS_DEPTHS) { return; }
  if (move_number < 0 || move_number >= CENSUS_MOVES) { return; }

  census.cutoff[depth][move_number]++;
}

inline void census_quiets(int depth, size_t count)
{
  if (depth < 0 || depth >= CENSUS_DEPTHS) { return; }
  if (count >= CENSUS_QUIETS) { count = CENSUS_QUIETS - 1; }

  census.quiets[depth][count]++;
}

// The reach of quiescence's insufficient-material test, counted rather than
// argued (2026-09-10_adversarial-F22, S210). Three totals, because the
// question the count has to answer is "what fraction of the tree does the new
// rule touch":
//
//   qnodes  quiescence nodes entered
//   qmoves  moves quiescence made that were legal, so one child node each
//   dead    ... of which left a position is_insufficient_material() calls dead
//
// `dead` is the rule's firing count. Before the fix it is the number of
// subtrees scored by the static evaluation where the laws say nothing can
// happen; after it, the number of subtrees the rule cuts. Same counter either
// way, which is what makes the two runs comparable.
//
// Dumped at exit only when CHESSO_F22_CENSUS names a file, so an ordinary tune
// run pays three increments and no I/O, and the release binary compiles none of
// it. adocs/data/S210_f22_census.py.
struct f22_census_t
{
  uint64_t qnodes = 0;
  uint64_t qmoves = 0;
  uint64_t dead = 0;

  ~f22_census_t();
};

f22_census_t f22_census;

f22_census_t::~f22_census_t()
{
  const char* path = std::getenv("CHESSO_F22_CENSUS");

  if (path == nullptr) { return; }

  std::ofstream out(path);

  if (!out) { return; }

  out << "# S210 F22 census. Quiescence's reach into positions no legal\n"
         "# sequence can mate from. One line per counter, whole run.\n"
         "counter\tcount\n"
      << "qnodes\t" << f22_census.qnodes << '\n'
      << "qmoves\t" << f22_census.qmoves << '\n'
      << "dead\t" << f22_census.dead << '\n';
}
}  // namespace
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

#ifdef CHESSO_TUNE
  f22_census.qnodes++;
#endif

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

    // Nothing left on the board can force mate, so there is nothing below this
    // move worth a look and nothing the evaluation may say about it: the game
    // is already over and drawn. negamax_at makes the same test at the top of
    // every node above the root; quiescence had none until S210, and a capture
    // that took the last piece able to force anything was scored on the
    // material left standing instead -- measured -103, +190 and +235 by
    // 2026-09-10_adversarial-F22, whose interior-node parent caught it only one
    // ply later.
    //
    // Here rather than at the top of the node, which is the placement
    // negamax_at uses. The two are the same set: material is what this function
    // reads, a quiet move cannot change it, and the node quiescence was entered
    // at cannot be dead already because negamax_at answered it first. Testing a
    // made move instead of an entered node skips the child outright -- no
    // probe, no evaluation, no move generation -- and leaves the test off the
    // entry path of every quiescence node there is.
    //
    // Unconditional and not gated on MOVE_CAPTURE. A promotion also changes the
    // material, a quiet one included, and quiescence searches quiet moves
    // whenever the side to move is in check.
    //
    // The reach was counted before the rule was written, over the 11503 sampled
    // positions of the S219 A/A games at depth 10: 158685 of 254361785 moves
    // made in quiescence, 0.062 %, and **194 of the 11503 root answers move**,
    // 1.69 %. That second number is why this commit owes an SPRT rather than
    // being discharged by census the way DEC-107's was.
    // adocs/data/S210_f22_census.py.
    const bool dead = is_insufficient_material(&game->board);

#ifdef CHESSO_TUNE
    f22_census.qmoves++;
    if (dead) { f22_census.dead++; }
#endif

    // Written in the child's frame and negated like every other child score,
    // so the site stays right if DRAW_SCORE ever stops being 0 (contempt).
    const int score =
        dead ? -DRAW_SCORE
             : -quiescence(-beta, -alpha, ply + 1, qply + 1, game, state);

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
//
// PROBING is false in every node the engine ever searches and true only at the
// one node a test drives directly. It exists because the alternative measured:
// resolving `state->probe` at every node and testing it once per move cost
// **1.49 % of nodes per second, sd 0.66 % over 13 interleaved pairs of
// `chesso bench`** -- a real number, not the machine's noise, and not a price
// worth paying for observability the shipping binary never uses. With this
// parameter `negamax_at<false>` holds no probe code at all, and the recursion
// below is always `<false>` because a probe records exactly one ply and every
// child is at another one. S191.
template <bool PROBING>
static int negamax_at(int alpha0,
                      int beta,
                      int depth,
                      size_t ply,
                      game_t* game,
                      search_state_t* state,
                      move_t prev_move,
                      bool is_pv,
                      bool cut_node,
                      move_t excluded_move)
{
  assert(game != nullptr);
  assert(state != nullptr);

  // The two flags name one of three node types and never a fourth: a node on
  // the principal variation is not one that is expected to fail high. S098
  // verdict 2, and it is an assert rather than a guard because a wrong
  // prediction costs rating and nothing else -- there is no wrong answer to
  // return here, only a plumbing bug to find in the Debug binary.
  assert(!(is_pv && cut_node));

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
    // Only a repetition this search itself walked into, or a third occurrence
    // of the position wherever the earlier two lie. A single occurrence from
    // the game before the root is not a draw: the side to move at the root
    // gets to choose again, and the opponent cannot force the third occurrence
    // alone. Scoring that one as a draw published `cp 0` for positions the
    // engine was losing by several pawns and made it play for them
    // (2026-09-10_adversarial-F08, S207, DEC-173).
    if (classify_repetition(&game->history, &game->board,
                            state->root_history_size) ==
        repetition_kind_t::DRAW) {
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
  move_t tt_move = (tt_entry != nullptr) ? tt_entry->best_move : 0;

  // At the root, and only where the table has nothing, the previous completed
  // iteration's best move orders the node instead. Any unreduced ply-1 node of
  // this iteration can evict the root's slot, and an aspiration re-search then
  // orders the root by captures, killers and history and can publish a move
  // that never beat the previous best -- which the caller keeps if the hard
  // timer aborts right there. Ordering is all this changes: the hint is a move
  // the previous iteration already returned from this position, so it is legal
  // here, and a node that had an entry is untouched.
  // 2026-09-04_adversarial-F03, S210.
  if (ply == 0 && tt_move == 0) { tt_move = state->root_move_hint; }

  // The static evaluation this position was already scored with, where an
  // earlier node recorded one. Copied out here for the reason above and read
  // by reverse futility below. TT_EVAL_NONE where there is nothing to read.
  // S103.
  const int tt_eval = (tt_entry != nullptr) ? tt_entry->eval : TT_EVAL_NONE;

  // THE THREE FIELDS THE SINGULAR EXTENSION READS, copied out here for exactly
  // the reason `tt_move` and `tt_eval` are, S097: `tt_entry` is the slot's own
  // address and the block that reads them sits **below the null-move search**,
  // which recurses. A node in that subtree can store into this slot -- another
  // position whose key lands on the same index -- and the gate would then
  // derive its window from that position's score while extending the move this
  // entry named. Nothing would crash and no node count would be wrong.
  //
  // It is masked at today's seeds and that is not a reason to leave it: the
  // null subtree stores below `depth - SeTtDepthMargin` at the shipped 4, and
  // at 8 -- inside the declared range S127 sweeps -- or at a smaller null
  // reduction it is not masked at all.
  //
  // The score is de-normalised here rather than at the gate because this is
  // where the entry is still known to be the one the probe found; this is the
  // second reader of that field beside `tt_entry_answers` and S106's round
  // trip applies to it. The three no-entry values are never read -- the gate
  // tests the pointer before it reads a copy -- and are the ones that would
  // fail it anyway: a depth below every main-search depth and the one bound
  // type the rule refuses.
  const int tt_entry_depth =
      (tt_entry != nullptr) ? tt_entry->depth : TT_DEPTH_QS;
  const uint8_t tt_entry_type = (tt_entry != nullptr)
                                    ? tt_entry->type
                                    : static_cast<uint8_t>(TT_ALPHA_NODE);
  const int tt_entry_score =
      (tt_entry != nullptr) ? de_normalize_score(tt_entry->score, ply) : 0;

  // No cutoff while a move is excluded, S097. The entry was written by a
  // search of this position that was allowed to play the very move this node
  // is being asked to do without, so letting it answer would answer the
  // verification with the thing being verified -- an entry whose lower bound
  // already clears the verification's window would fail the node high every
  // time, vacuously, and the table move would never be called singular. The
  // probe above still runs and `tt_move` is still copied out: the subtree below
  // orders and cuts on the table exactly as it always did, and masking the move
  // here would switch a later step's rules on and off inside the verification
  // for no stated reason.
  if (!is_pv && ply > 0 && excluded_move == 0) {
    int tt_score = 0;

    if (tt_entry_answers(tt_entry, depth, ply, alpha, beta, &tt_score)) {
      return tt_score;
    }
  }

  // Quiescence search in leaves
  if (depth < 1) { return quiescence(alpha, beta, ply, 0, game, state); }

  // Below the leaf test: every leaf used to pay for this and throw it away.
  const bool is_in_check = is_check(game);

  // Null unless a test attached a probe and this is the node it asked for.
  // Resolved once, and only in the instantiation a test drives. S191.
  search_node_probe_t* probe = nullptr;

  if constexpr (PROBING) {
    if (state->probe != nullptr && state->probe->ply == static_cast<int>(ply)) {
      probe = state->probe;
    }
  }

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

  // S108's deferred layer (c), which the owner made this step's first line.
  // The static evaluation is what this node looks like standing still; the
  // table's score for the same position is what a search of it came back with,
  // and where the entry's bound type certifies a direction that score is the
  // better input to a pruning margin.
  //
  // Only a certified direction is taken. A TT_BETA_NODE's score is a lower
  // bound, so it may raise the input and never lower it; a TT_ALPHA_NODE's is
  // an upper bound, so it may lower it and never raise it. The mate band is
  // excluded outright -- a mate score is a distance and not a value, and
  // feeding one to a margin would prune against a number that means something
  // else. TT_PV_NODE is deliberately not taken: the entry is exact and
  // certifies both directions, but the licence the step file states covers the
  // two bound types, and widening it is a change with its own verdict.
  //
  // **The stack and the stored evaluation keep the raw static score.**
  // `improving` compares statics across plies and must never compare a search
  // score against one, and the entry this node stores must hold what
  // `evaluate()` said or the correction compounds through the table, which is
  // the rule S099 inherits.
  //
  // **Two consumers since S234, and one switch over the second of them.** The
  // futility margin in the move loop has read this value since S109 and is not
  // switched: it is the tree the S109 block's own verdict measured. Reverse
  // futility below reads it at `RfpTtEstimate` 1 and `static_eval` at 0, which
  // is this step's candidate and its off value. The rule computing the value
  // does not move; what moves is how many margins consult it.
  int pruning_eval = static_eval;

  if (tt_entry != nullptr && !is_in_check) {
    const int tt_score = de_normalize_score(tt_entry->score, ply);

    if (tt_score < MATE_MIN && tt_score > -MATE_MIN) {
      if ((tt_entry->type == TT_BETA_NODE && tt_score > static_eval) ||
          (tt_entry->type == TT_ALPHA_NODE && tt_score < static_eval)) {
        pruning_eval = tt_score;
      }
    }
  }

  // Reverse futility pruning, also called static null move pruning. The null
  // move observation without the null move: if the node's own price is so far
  // above beta that the opponent cannot claw the difference back in the plies
  // that are left, the node fails high and nothing below it is worth searching.
  //
  // **That price is `pruning_eval` and not `static_eval` since S234**, at
  // `RfpTtEstimate` 1: where the table's entry certifies which way its score
  // has moved from the static one, the searched number is the better input to
  // a margin than the standing-still one. The site is at the foot of this
  // comment and the switch is what a verdict moves.
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
  // and in this engine it cannot approach one.
  //
  // **What bounds it is the material and table sums, not the lazy clamp.** The
  // clamp inside evaluate_expensive() bounds one stage -- mobility and king
  // safety -- to +/-LAZY_EVAL_MARGIN, and until S213 this comment named it as
  // the whole reason, which it never was: evaluate_cheap() carries material,
  // the two tapered piece-square sums and the pawn terms, and the clamp says
  // nothing about any of them. They are bounded by their own arithmetic
  // instead. Each is a sum of table entries and piece values over at most
  // sixteen men a side -- the load boundary refuses a seventeenth -- and every
  // one of those constants is a two- or three-digit number, so the total is
  // thousands of centipawns where MATE_MIN is 48000.
  //
  // Asserted rather than argued, because an argument from the size of some
  // constants is exactly the kind that a refit walks through: test_evaluation
  // "the static score never reaches the mate band" runs |evaluate()| against
  // MATE_MIN over the whole test corpus and over pathological placements built
  // to make the sums as large as the boundary permits.
  //
  // S033 measured five guards against a hidden mate and none of them worked;
  // the depth and ply bounds are what contain it. A mate deeper than
  // RFP_MIN_PLY can still be missed for an iteration, and no test covers that.
  //
  // The cast on ply is the tune build's, for the reason quiescence's depth
  // bound carries: a parameter is a plain int there and ply is a size_t.
  //
  // **Off while a move is excluded, S097, and that is this step's decision
  // rather than a traced practice.** The verification search asks whether any
  // move other than the table move reaches a window below the entry's score,
  // and a static bound answers that with no move searched at all: it can only
  // fail the node high -- the rule returns nothing below its own beta -- so it
  // can only say "not singular", and under the multicut it would return a
  // static margin as the node's value at a depth the parent's own reverse
  // futility already refused. The verification is cheap and there is one of it
  // per eligible node; a second pruning rule folded inside it is a change
  // nothing could attribute.
  if (!is_pv && !is_in_check && excluded_move == 0 &&
      static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&
      beta < MATE_MIN && beta > -MATE_MIN) {
    const int margin = RFP_MARGIN * depth;

    // WHICH NUMBER THE MARGIN IS SUBTRACTED FROM, S234. `pruning_eval` is the
    // estimate computed above -- the static score, or the table's own score
    // where the entry's bound certifies the direction it moved in -- and it is
    // read here rather than re-derived, so this site and the futility site
    // below decide on the same number at the same node. One local, used by
    // both the comparison and the return, so `RfpTtEstimate` at 0 is the tree
    // before this step and cannot be half taken (DEC-215).
    //
    // The block excludes a node in check, where the estimate is the sentinel;
    // the futility site asserts the same thing for the same reason.
    const int rfp_eval = (RFP_TT_ESTIMATE != 0) ? pruning_eval : static_eval;

    assert(rfp_eval != TT_EVAL_NONE);

    // Fail soft, and the bound returned is the one actually argued for: the
    // number this node was priced at minus everything the opponent was assumed
    // able to win back. **The estimate and not the static score**, where the
    // two differ. On the lower-bound branch that is a weaker claim than the
    // entry already carries -- a TT_BETA_NODE says a search of this position
    // came back at or above its score, so `score - margin` is below something
    // already proved -- and on the upper-bound branch it lowers the bound,
    // which a fail-soft return may always do. Returning `static_eval - margin`
    // while deciding on the estimate was the alternative and it is rejected:
    // it would hand a parent a bound the node's own test did not argue for,
    // and on the lower-bound branch that bound is the smaller of the two,
    // which throws away the certificate the entry brought.
    //
    // **WHAT IS ACTUALLY HANDED BACK IS A POINT BETWEEN BETA AND THAT BOUND,
    // S235**, `RfpReturnWeight` hundredths of the way from the first to the
    // second. The bound above is what the node's own test argued and the node
    // did not search: it is a claim about a static number extrapolated over
    // the remaining plies, and the further above beta it sits the more of it
    // is assumption rather than evidence. Beta is the other end and is the
    // part the parent asked about -- returning it alone is a legal fail-soft
    // answer, because every fail-soft return may weaken its own bound. The
    // weight is which of the two the parent is told, and where it sits between
    // them is what the SPRT prices.
    //
    // Independent of `RfpTtEstimate` above: this blends whichever number that
    // switch left the margin to be subtracted from, so the two verdicts do not
    // interact through the code.
    //
    // THE ARITHMETIC, and all three of its properties are asserted rather than
    // argued. The gap is non-negative because the block returns only where the
    // bound reaches beta, so the integer division floors and the returned point
    // rounds **toward beta**, never past it; at `RfpReturnWeight`
    // RFP_RETURN_SCALE the second term is the whole gap and the site returns
    // the bound itself, which is the tree before this step exactly. Both ends
    // are strictly inside the mate band -- beta by this block's own guard,
    // the bound because the number the margin came off is either the static
    // score, which never reaches the band, or a table score the tightening
    // above refused from inside it -- so the point between them is too, and
    // this rule can no more return a mate score than the rule it blends could.
    if (rfp_eval - margin >= beta) {
      if constexpr (PROBING) {
        if (probe != nullptr) { probe->rfp_cutoff = true; }
      }

      const int rfp_bound = rfp_eval - margin;
      const int rfp_blended =
          beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE;

      assert(rfp_blended >= beta);
      assert(rfp_blended <= rfp_bound);
      assert(rfp_blended > -MATE_MIN && rfp_blended < MATE_MIN);

      return rfp_blended;
    }
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
  //   excluded_move  S097. A null-move bound answers the verification search
  //                 without any alternative having been searched, which is the
  //                 one thing that search exists to do. Gated on the exclusion
  //                 directly and never by abusing `prev_move`, which has to
  //                 keep flowing for the countermove and continuation tables
  if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&
      excluded_move == 0 && depth - 1 - null_reduction >= 1 &&
      beta < MATE_MIN && beta > -MATE_MIN && game_phase(&game->board) > 0) {
    const int reduction = null_reduction;
    const child_label_t child = null_move_child(is_pv, cut_node);

    if constexpr (PROBING) {
      if (probe != nullptr) {
        probe->null_move_made = true;
        probe->null_child_is_pv = child.is_pv;
        probe->null_child_cut_node = child.cut_node;
      }
    }

    make_null_move(game);

    const int null_score =
        -negamax_at<false>(-beta, -beta + 1, depth - 1 - reduction, ply + 1,
                           game, state, 0, child.is_pv, child.cut_node, 0);

    unmake_null_move(game);

    if (state->aborted) { return 0; }

    if (null_score >= beta) {
      // A mate score out of a null move search is not a mate anyone can force,
      // it is an artefact of the pass. Report the bound instead.
      return (null_score >= MATE_MIN) ? beta : null_score;
    }
  }

  // SINGULAR EXTENSION AND MULTICUT, S097. One verification search, two
  // answers, and the second of them ships switched off.
  //
  // The table's move at a node deep enough to be worth the nodes is searched
  // one ply deeper when **every other move** fails a search against a window a
  // margin below the entry's own score: nothing else here is close, so the line
  // is the node, and the plies are better spent inside it. The same search
  // failing high says the opposite -- some other move already clears a bar just
  // under the entry's score -- and that is the multicut, which returns the
  // score rather than searching the node at all.
  //
  // Below the null-move block on purpose: a node the pass already cut off never
  // pays for a verification, and the ordering makes the cheap answer first.
  //
  // WHAT EACH CONDITION IS FOR. `SE_EXTEND` is the switch and not a setting:
  // at 0 the whole block is skipped and the tree is the one before this step,
  // exactly, which is the off value DEC-215 requires a shipped rule to have and
  // which `SeMinDepth`'s range top is not. `ply > 0` keeps it off the root,
  // which has to produce a move and whose one move is not a candidate for
  // anything.
  // `excluded_move == 0` is the no-recursion rule: a verification inside a
  // verification asks about a node two moves have been taken from.
  // `tt_move != 0` with a non-null entry is the technique's whole premise --
  // there is a move to call singular and an entry to measure it against -- and
  // at ply 0 the root hint could make the first true with the second false,
  // which is why the entry is tested in its own right and not inferred.
  // `tt_entry_depth >= depth - SE_TT_DEPTH_MARGIN` is how deep that entry had
  // to have been searched for its move to be worth verifying, and the bound
  // types are the two that certify a score at or above the entry's number: an
  // upper bound says the position is worth *at most* this, which is not a claim
  // a margin can be subtracted from.
  //
  // MATE SCORES, TWICE. The entry's score arrives de-normalised from the
  // copy-out above, where the round trip is done once and where the entry is
  // still known to be the one the probe found. A mate score is a distance and
  // not a value: a margin subtracted from one means nothing, so the band is
  // excluded outright, and the derived window is checked in its own right
  // because a score just inside the band minus a margin lands outside it.
  int se_extension = 0;

  if (SE_EXTEND != 0 && ply > 0 && excluded_move == 0 && tt_move != 0 &&
      tt_entry != nullptr && depth >= SE_MIN_DEPTH &&
      tt_entry_depth >= depth - SE_TT_DEPTH_MARGIN &&
      (tt_entry_type == TT_BETA_NODE || tt_entry_type == TT_PV_NODE) &&
      static_cast<int>(ply) < SE_PLY_FACTOR * depth) {
    const int singular_beta = tt_entry_score - SE_MARGIN_PER_DEPTH * depth;

    if (tt_entry_score < MATE_MIN && tt_entry_score > -MATE_MIN &&
        singular_beta > -MATE_MIN) {
      // About half of what is left, so the verification costs a fraction of the
      // node it is about. Never zero: the shallowest eligible node is
      // SeMinDepth's floor of 4 and `(4 - 1) / 2` is 1, so the search that
      // answers is always a real one and never quiescence.
      const int verification_depth = (depth - 1) / 2;

      assert(verification_depth >= 1);

      // `negamax_at<false>` and not the probing instantiation, whatever this
      // node is. A probe records exactly one ply, and this search is at **this
      // node's own ply** -- the one place in the engine where that is true --
      // so the probing instantiation would overwrite the record of the node
      // that asked the question with the record of the question. S191's
      // parameter is what makes that a compile-time fact rather than a
      // convention.
      //
      // Not a PV node and not a cut node: a zero window under the entry's score
      // is asked in the expectation that everything fails low, which is CPW's
      // ALL node. The label is a prediction and a wrong one costs rating and
      // nothing else.
      const int vscore = negamax_at<false>(singular_beta - 1, singular_beta,
                                           verification_depth, ply, game, state,
                                           prev_move, false, false, tt_move);

      if (state->aborted) { return 0; }

      // The verification is a search of this ply and it wrote this ply's PV
      // row. The row belongs to a node that was searched without one of its
      // moves, so it is cleared again here for the same reason the top of the
      // function clears it: every path out of this function leaves the row
      // either valid or empty, and a line this node never played is neither.
      state->pv_length[ply] = 0;

      if constexpr (PROBING) {
        if (probe != nullptr) {
          probe->se_verified = true;
          probe->se_singular_beta = singular_beta;
          probe->se_vscore = vscore;
          probe->se_vdepth = verification_depth;
        }
      }

      if (vscore < singular_beta) {
        // Singular: nothing else here reached a window the table move already
        // clears. One ply, once, at this node -- the published amount at every
        // traced introduction, and the cap that keeps the line finite together
        // with `ply < SE_PLY_FACTOR * depth` and the MAX_PLY walls.
        se_extension = 1;

        if constexpr (PROBING) {
          if (probe != nullptr) { probe->se_extended = true; }
        }
      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&
                 vscore < MATE_MIN && vscore > -MATE_MIN && beta > -MATE_MIN) {
        // MULTICUT, and it is the step's second verdict: at `SeMultiCut` 0 the
        // release build compiles this branch away entirely and the engine is
        // the one the extension landed as.
        //
        // The verification failed high, so some move other than the table's
        // reaches a bar just under the entry's score -- and here it reaches
        // this node's own beta as well, at a reduced depth. Two moves are then
        // claimed to be worth at least beta and the node is taken to fail high
        // without being searched.
        //
        // **The fail-soft score and never `singular_beta`**: the bound is what
        // the window was set to and the score is what the search found, and
        // returning the bound throws away everything the search established
        // above it.
        //
        // Guarded like every other bound-returning rule in this function. Not
        // at a PV node, whose line gets reported and played. Never a mate-range
        // value -- a reduced search is the instrument that misses mates, and a
        // mate score returned from one is a distance nothing proved. And never
        // against a beta inside the negative mate band, which is S165's guard:
        // there the node sits inside a mate proof as the defender, every score
        // clears beta, and a reduced search's word is exactly what must not
        // decide it.
        if constexpr (PROBING) {
          if (probe != nullptr) { probe->se_multicut = true; }
        }

        return vscore;
      }
    }
  }

  node_type_t type = TT_ALPHA_NODE;

  // The shallow-depth pruning block's node-level guards, S109. Four rules --
  // late move pruning, futility, history pruning and quiet SEE -- and not one
  // of them fires at a node that fails any of these:
  //
  //   PV node       these lines get reported and played, and a pruned quiet is
  //                 a move the node never looked at
  //   in check      the move list is evasions; pruning one risks a mate this
  //                 search never sees. It is off by data as well as by guard --
  //                 `static_evals[ply]` is the sentinel here
  //   ply 0         the root decides the move that gets played. `!is_pv`
  //                 already covers it at every call search() makes; this is
  //                 what covers a test driving the root directly
  //   beta near mate  a bound inside the mate band means this node sits inside
  //                 a mate proof, where a static margin and a history count
  //                 are answering a question nobody asked them
  //
  // The alpha edge of the same band and the first-move guard are per move and
  // sit in the loop. This is the fourth, fifth, sixth and seventh pruning rule
  // in this engine, and three of the three before them hid a mate at least
  // once: the guards are the accepts' own list and the mate cases in
  // tests/test_search.cpp are what holds them.
  const bool pruning_node =
      !is_pv && !is_in_check && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;

  // S108 supplies it and this is its first in-search call site: the side to
  // move is better off here than it was the last time it moved. Late move
  // pruning doubles its count when it is true, and S098 verdict 2's second
  // term reads it below.
  const bool improving = improving_at(state, ply, is_in_check);

  // S098 verdict 2's four terms and S095's fifth, summed once for the node.
  // Every input is a property of this node and not of a move -- the predicted
  // type, the improving flag, the class of the entry's own move, whether there
  // is an entry move at all -- so the sum is computed here and read by both
  // consumers below: the reduction each late quiet is searched with, and the
  // shallow-depth gate those quiets are pruned against. At the five off values
  // it is 0 and both consumers see the raw table.
  //
  // `tt_move` is the entry's move, or the root hint where the entry is gone
  // (ply 0, which neither consumer reaches). A capture there says the node is
  // tactical; the quiet the table is about to reduce is not what it is about.
  const bool tt_move_is_capture =
      (tt_move != 0) && (MOVE_CAPTURE(tt_move) != 0);

  // S095's condition, and it is the same variable read the other way: no entry
  // at all, or an entry whose move field is empty, which since S094 means only
  // quiescence has ever resolved this position. Both are "the table has no
  // move to order by", which is the published condition and the union of the
  // two the record measured separately. At ply 0 the root hint can make this
  // false with the entry gone; neither consumer runs at ply 0, so the hint
  // decides nothing here either.
  const bool no_tt_move = tt_move == 0;

  const int node_adjustment = lmr_node_adjustment(
      cut_node, improving, tt_move_is_capture, is_pv, no_tt_move);

  // Set once by late move pruning and never cleared: past its count the quiet
  // stage is over for this node.
  bool skip_quiets = false;

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
    const size_t added =
        generate_quiets(game_tables(), &game->board, moves + moves_count);

#ifdef CHESSO_TUNE
    census_quiets(depth, added);
#endif

    moves_count += added;
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
    // Everything the four shallow-depth rules require of the node and of the
    // window, read once per candidate. `alpha` is read live rather than from
    // `alpha0`: a move that beat alpha earlier in this loop can have carried it
    // into the mate band, and the guard has to follow it there.
    //
    // `legal_moves_counter >= 1` is the first-move guard and it is
    // load-bearing twice over. It is CPW's own "requires the existence of at
    // least one legal move", and it is what keeps the no-legal-moves return
    // below unreachable by pruning: these rules skip legal moves without
    // counting them, so a node whose every legal move was pruned would report
    // a mate or a stalemate that is not there.
    const bool may_prune = pruning_node && legal_moves_counter >= 1 &&
                           alpha < MATE_MIN && alpha > -MATE_MIN;

    // Late move pruning. Past a move count that grows with the reduced depth
    // the node gives up on its quiets: the flag is set here, before any move of
    // this iteration is made, and every quiet from here on is skipped unless it
    // gives check.
    //
    // **The gives-check exemption binds this rule too, and that is S109's own
    // finding rather than its plan.** The published form ends the quiet stage
    // outright, which is what the accepts asked for and what this shipped
    // first; DEC-180 reserved the exemption to S218 *unless* the mate case went
    // red without it, and it did. `4K3/q7/8/4k3/8/8/8/8 b`, mate in two by
    // a7b8 after e5e6 e8d8, is lost at depth 3 at every count below 30 moves --
    // measured, one release rebuild per setting -- because the mating move is
    // one of twenty-eight queen moves at a node whose history table has never
    // seen any of them. A count of 30 is a rule that never fires. So the choice
    // was not between a safe count and an unsafe one; it was the exemption or
    // the rule, and DEC-180's own words are that a red guard is a bug and not
    // an option.
    //
    // The shape is the first of the two that decision names: the flag is still
    // set here, by a count read before any move of this iteration is made, and
    // the skip it causes happens **after `make_move`**, where `is_check_move`
    // exists. The price is that the quiet stage can no longer be left
    // ungenerated -- a stage that is never generated cannot be searched for the
    // checking move inside it -- so what the rule saves is the subtrees and not
    // the move list, and every quiet it skips costs one make, one unmake and
    // one attack scan.
    if (!skip_quiets && may_prune) {
      const int move_number = legal_moves_counter + 1;
      const int lmr_depth = lmr_depth_of(depth, move_number, node_adjustment);

      if (lmr_depth < LMP_MAX_LMRDEPTH &&
          100 * move_number > lmp_threshold_x100(lmr_depth, improving)) {
        skip_quiets = true;

        if constexpr (PROBING) {
          if (probe != nullptr) { probe->skip_quiets_set = true; }
        }
      }
    }

    if (i == moves_count) {
      // The captures ran out without a cutoff, so the quiets are needed after
      // all. This is the branch staging exists to avoid.
      if (quiets_generated) { break; }

      const size_t added =
          generate_quiets(game_tables(), &game->board, moves + moves_count);

#ifdef CHESSO_TUNE
      census_quiets(depth, added);
#endif

      for (size_t j = moves_count; j < moves_count + added; ++j) {
        scores[j] = score_move(game, state, moves[j], tt_move, ply, prev_move);
      }

      moves_count += added;
      quiets_generated = true;

      if (i == moves_count) { break; }
    }

    pick_next_move(moves, scores, moves_count, i);

    // S097, and the whole mechanism of a verification search: this node is
    // being asked what it is worth without this move. Skipped before make_move
    // and before `legal_moves_counter`, so the move is not a legal move this
    // node searched by any of the counts the rules below read -- the move
    // number the reduction table is indexed at, the first-move guard, and the
    // no-legal-move return at the bottom.
    //
    // score_move() still ranks it first, so the selection sort above has
    // already spent one pick on it. That is one comparison pass per
    // verification and is left alone deliberately: a stale or colliding table
    // move that the generator never emits costs the same pick whether it is
    // tested for here or not.
    if (moves[i] == excluded_move) { continue; }

    const bool is_capture = MOVE_CAPTURE(moves[i]);
    const bool is_quiet = !is_capture && !MOVE_PROMOTED(moves[i]);

    // The three per-move rules. Their inputs are all properties of **this**
    // position -- the node's own static score, the history table and the
    // exchange evaluation -- so the decision is taken before make_move, where
    // the board is still this one, and applied after it, where `is_check_move`
    // exists and the gives-check exemption can bind. Lynx measured moving such
    // rules before make at -0.6 +/-2.6, so the wasted make costs nothing worth
    // chasing and the subtree saved dominates it either way.
    prune_rule_t prune_rule = PRUNE_NONE;

    if (may_prune && is_quiet) {
      const int move_number = legal_moves_counter + 1;
      const int lmr_depth = lmr_depth_of(depth, move_number, node_adjustment);

      // A node in check has no static score and `pruning_node` excludes one,
      // so the margin below never reads the sentinel.
      assert(pruning_eval != TT_EVAL_NONE);

      // Futility. The static score plus everything the remaining plies are
      // assumed able to win back still does not reach alpha, so neither this
      // quiet nor any quiet ordered behind it is worth a subtree. The margin
      // shrinks as lmr_depth falls with the move number, so later quiets are
      // pruned more easily -- monotone at a fixed node.
      if (lmr_depth < FUT_MAX_LMRDEPTH &&
          pruning_eval + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha) {
        prune_rule = PRUNE_FUTILITY;
      }

      // History pruning. The raw butterfly entry, read through the table and
      // never through score_move(), whose killer and countermove bands would
      // exempt themselves silently. The threshold is negative because history
      // is signed since S093: a sign slip here prunes the *good* quiets and
      // has no symptom but lost rating.
      //
      // **Still the plain entry alone, with the continuation table live.**
      // S222 lands that table and DEC-205 records what this rule costs without
      // it -- 0.8 % of the nodes at depth 10, because plain history alone
      // rarely reaches the threshold -- so reading the sum is the obvious next
      // move and it is deliberately not made here. Two reasons. The step's one
      // SPRT already prices a new table and a fitted history scale, and a
      // third change inside it could not be attributed (MEASUREMENT). And
      // HistPruneCoeff's declared range is derived from the plain band's own
      // edge, `QuietHistoryMax/64` to `QuietHistoryMax/8` with a range top at
      // twice that edge; against a sum whose span is QuietHistoryMax plus
      // ContHistWeight * CONT_HIST_BOUND / 100 that derivation has to be taken
      // again, which is a decision with its own seed and not a line moved.
      // S222's lane fits HistPruneCoeff where it stands (DEC-205) and its SPRT
      // re-prices the rule; the sum belongs with S098, which scales its
      // reduction by that same sum by design.
      if (prune_rule == PRUNE_NONE && lmr_depth < HP_MAX_LMRDEPTH &&
          state->quiet_history[game->board.active_color][MOVE_FROM(moves[i])]
                              [MOVE_TO(moves[i])] < -HP_COEFF * lmr_depth) {
        prune_rule = PRUNE_HISTORY;
      }

      // Quiet SEE pruning. see_ge() scores a quiet as a capture of nothing --
      // gain 0 -- so this asks whether the move loses more than the margin once
      // the opponent is allowed to take the piece and the exchange resolves.
      if (prune_rule == PRUNE_NONE && lmr_depth < SEE_QUIET_MAX_LMRDEPTH &&
          !see_ge(&game->board, moves[i],
                  -(SEE_QUIET_COEFF * lmr_depth * lmr_depth))) {
        prune_rule = PRUNE_SEE;
      }
    }

    // Capture SEE pruning, S091, and the fifth rule to read `may_prune`. A
    // capture the exchange evaluation says loses more than the margin is not
    // searched at all, where quiescence has only ever declined one -- that
    // difference is why S015's measured zero for the quiescence rule does not
    // settle this one. The margin is linear in the reduced depth where the
    // quiet margin is quadratic, which is the pair of shapes the wiki
    // publishes; the two rules are disjoint by move class and neither can
    // reach the other's moves.
    //
    // Promotions are inside this class when they capture and outside it when
    // they do not, which is where the quiet rules leave them: a promotion that
    // takes nothing is pruned by no rule in this loop, deliberately, because a
    // promotion mate is the shape the hazard likes best.
    if (may_prune && is_capture) {
      const int move_number = legal_moves_counter + 1;
      const int lmr_depth = lmr_depth_of(depth, move_number, node_adjustment);

      if (lmr_depth < SEE_CAPT_MAX_LMRDEPTH &&
          !see_ge(&game->board, moves[i], -(SEE_CAPT_COEFF * lmr_depth))) {
        prune_rule = PRUNE_SEE_CAPTURE;
      }
    }

    // The extra reduction's own question, S091, asked here for the reason the
    // rules above are decided here: `see_ge` reads the position the move is
    // made from, and after `make_move` the board is the child's.
    //
    // It cannot share an answer with either skip. Those ask whether the move
    // loses more than a margin; this asks whether it loses anything at all, and
    // a move that clears `-(COEFF * lmr_depth)` may still be under zero. What
    // keeps the second call off the hot path is the eligibility in front of it,
    // which is late move reduction's own: no node under depth 3 pays it, no
    // move inside the first three of a node's order pays it, and a move
    // already skipped above pays it never.
    const bool see_loses_material =
        SEE_LMR_EXTRA > 0 && prune_rule == PRUNE_NONE && ply > 0 &&
        depth >= 3 && legal_moves_counter + 1 > 3 && !is_in_check &&
        !MOVE_PROMOTED(moves[i]) && !see_ge(&game->board, moves[i], 0);

    // S132, the first half of the node-fraction time manager: what this root
    // move is about to cost. Read here, immediately before the move is made,
    // because everything above is decided on the parent's board and counts no
    // nodes -- so a snapshot taken any earlier in the loop would be the same
    // number and one taken after `make_move` would already be missing the
    // child's own node. The published record's own bug of this class is a
    // snapshot that drifted away from the move (section 5 of the step file).
    //
    // Zero below the root, where the ternary folds to a constant the branch
    // predictor never has to think about; it is the one line of this step that
    // every node in the tree runs, and DEC-083's interleaved timing is what
    // prices it rather than an argument that it is free.
    const uint64_t nodes_before_move = (ply == 0) ? state->explored_nodes : 0;

    if (!make_move(game, moves[i])) { continue; }

    // The late move reduction guard below is the only consumer: a move that
    // gives check is not reduced. It is deliberately *not* consulted by the
    // fail-high block, which admits every quiet that caused a cutoff to the
    // killer, history and countermove tables, checks included -- excluding
    // them made the engine refuse to remember the one class of refutation its
    // own reductions call forcing. S107.
    //
    // is_check() is an attack scan - do not pay for it on captures.
    const bool is_check_move = is_capture ? false : is_check(game);

    // S091's two rules do want it on a capture, and this is where they ask.
    // A capture that gives check is forcing, and neither an exchange
    // evaluation nor a move number says anything about a line the opponent has
    // no choice in -- the same argument that put the exemption on the four
    // quiet rules (DEC-180, DEC-205). The scan is paid only by the captures
    // one of those two rules is about to act on, which is what keeps
    // `is_check_move` above hardcoded false: reusing it for capture logic is
    // the trap S107 left the comment against.
    const bool capture_gives_check =
        is_capture && (prune_rule != PRUNE_NONE || see_loses_material) &&
        is_check(game);

    // Late move pruning's own skip, here rather than at the generation stage
    // so that the exemption above it can bind. A quiet past the count is
    // searched only if it gives check.
    if (prune_rule == PRUNE_NONE && skip_quiets && is_quiet) {
      prune_rule = PRUNE_LATE_MOVE;
    }

    // The gives-check exemption. A checking quiet is forcing, and none of the
    // four inputs -- a static score, a history count, an exchange evaluation,
    // a move number -- says anything about a line the opponent has no choice
    // in. It binds every rule of the block since the mate case above forced it
    // onto late move pruning as well (DEC-180), and S091's capture rule with
    // them -- through `capture_gives_check`, because `is_check_move` is
    // hardcoded false on a capture.
    if (prune_rule != PRUNE_NONE && !is_check_move && !capture_gives_check) {
      if constexpr (PROBING) {
        if (probe != nullptr && probe->pruned_count < MAX_MOVES) {
          probe->pruned_moves[probe->pruned_count] = moves[i];
          probe->pruned_rule[probe->pruned_count] = prune_rule;
          probe->pruned_count++;
        }
      }

      unmake_move(game);
      continue;
    }

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
    //
    // **The extension is here and nowhere else, S097**: the one move the
    // verification called singular is searched a ply deeper, and `child_depth`
    // is where every rule below reads the move's depth from -- the reduction's
    // clamp to `child_depth - 1`, S098 verdict 3's re-search and the four
    // recursion sites. Folding it into this line is what makes them all follow
    // the extended depth instead of each needing to know about it, which is the
    // edit S098's file assigns to this step.
    //
    // `se_extension` is 0 at every node that ran no verification and at every
    // one whose verification failed high, and `tt_move` is non-zero wherever it
    // is not, so a move can only match the table's own.
    const int child_depth =
        depth - 1 + ((moves[i] == tt_move) ? se_extension : 0);

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

    // What makes a move reducible at all, independent of its class: the node is
    // deep enough, the move is late enough, and neither side is in check. The
    // class conditions -- not a capture, not a promotion -- belong to the rule
    // and not to this list, which is what lets S091's extra ply read the same
    // eligibility without inheriting an exemption that is about the table's
    // guess rather than about safety.
    const bool may_reduce = ply > 0 && depth >= 3 && legal_moves_counter > 3 &&
                            !is_in_check && !is_check_move &&
                            !capture_gives_check;

    if (may_reduce) {
      if (!is_capture && !MOVE_PROMOTED(moves[i])) {
        reduction = lmr_adjusted_reduction(
            depth, static_cast<int>(legal_moves_counter), node_adjustment);
      }

      // S091. A move that loses material is one the ordering already put late
      // and the exchange evaluation has now also written off, so it is searched
      // one ply shallower still. Additive on a quiet, where late move reduction
      // has already decided a reduction; the whole of it on a capture, which
      // that rule refuses -- a capture is tactics the ordering promoted, and
      // being promoted is not the same as being sound.
      if (see_loses_material) { reduction += SEE_LMR_EXTRA; }

      if (reduction > child_depth - 1) { reduction = child_depth - 1; }
      if (reduction < 0) { reduction = 0; }
    }

    // The type this node predicts for the child it is about to search: the
    // principal variation continues through the first legal move and every
    // later move is scouted. S098 verdict 2.
    const child_label_t child = (legal_moves_counter == 1)
                                    ? first_child(is_pv, cut_node)
                                    : scouted_child(is_pv, cut_node);

    if constexpr (PROBING) {
      if (probe != nullptr) {
        const int k = legal_moves_counter - 1;

        probe->moves[k] = moves[i];
        probe->reduction[k] = reduction;
        probe->child_depth[k] = child_depth;
        probe->researched[k] = false;
        probe->child_is_pv[k] = child.is_pv;
        probe->child_cut_node[k] = child.cut_node;
        probe->move_count = legal_moves_counter;
      }
    }

    if (legal_moves_counter == 1) {
      // The first legal move of a PV node continues the principal variation.
      score =
          -negamax_at<false>(-beta, -alpha, child_depth, ply + 1, game, state,
                             moves[i], child.is_pv, child.cut_node, 0);
    } else {
      score = -negamax_at<false>(-alpha - 1, -alpha, child_depth - reduction,
                                 ply + 1, game, state, moves[i], child.is_pv,
                                 child.cut_node, 0);

      // A reduced search that beats alpha has proved only that the reduction
      // was wrong, not what the move is worth. Repeat it before believing
      // anything -- at a depth that answers how wrong the reduction turned out
      // to be, and no longer at `child_depth` always. S098 verdict 3.
      //
      // `best_so_far` is read before this move updates it, so it is the node's
      // fail-soft best over the moves already searched. It is a real score and
      // never MIN here: the reduction is non-zero only past the third legal
      // move, so three moves have already written it.
      if (!state->aborted && reduction > 0 && score > alpha) {
        assert(best_so_far > MIN);

        const research_decision_t research = lmr_research_depth(
            child_depth, reduction, score, alpha, best_so_far);

        if constexpr (PROBING) {
          if (probe != nullptr) {
            const int k = legal_moves_counter - 1;

            probe->researched[k] = true;
            probe->research_depth[k] = research.depth;
            probe->research_score[k] = score;
            probe->research_alpha[k] = alpha;

            // Two numbers that must agree and are read from different places
            // on purpose: `best_so_far` straight off the node, and the base the
            // rule says it used. A call that handed over the window instead
            // shows up as a disagreement here and nowhere else.
            probe->research_best[k] = best_so_far;
            probe->research_base[k] = research.base;
          }
        }

        const child_label_t again = zero_window_research(is_pv, cut_node);

        score = -negamax_at<false>(-alpha - 1, -alpha, research.depth, ply + 1,
                                   game, state, moves[i], again.is_pv,
                                   again.cut_node, 0);
      }

      // Beat alpha without reaching beta, so the null window has told us the
      // move is interesting and nothing more. Only then is the full search
      // worth doing. Skipped when the search was abandoned mid-way, because
      // the score is meaningless then.
      if (!state->aborted && score > alpha && score < beta) {
        const child_label_t on_the_line = full_window_research(is_pv, cut_node);

        score = -negamax_at<false>(-beta, -alpha, child_depth, ply + 1, game,
                                   state, moves[i], on_the_line.is_pv,
                                   on_the_line.cut_node, 0);
      }
    }

    unmake_move(game);

    // S132. The delta lands in the move's own bucket, and it lands **before**
    // the abort check above all else: an iteration the hard timer cut in half
    // still spent those nodes on that move, and the buckets are the whole
    // search's and not one iteration's. The other exit from this loop that
    // runs past a make_move -- the pruning block's `unmake_move; continue` --
    // is unreachable at ply 0, because every rule behind it reads
    // `pruning_node`, which is false at the root; and it explores no node
    // anyway, so the identity `1 + sum(buckets) == the call's node growth`
    // holds whether or not that ever changes.
    if (ply == 0) {
      root_nodes_add(state, moves[i],
                     state->explored_nodes - nodes_before_move);
    }

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
                                quiets_tried, quiets_tried_count, depth,
                                prev_move);

#ifdef CHESSO_TUNE
        // The one observation S109's late move pruning constants were fitted
        // from: how far down the order the quiet that actually cut off sits.
        census_cutoff(depth, legal_moves_counter);
#endif

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
    // S097. A node whose only legal move was the excluded one is neither mated
    // nor stalemated: the move that answers is on the board and this search set
    // it aside. Both terminal scores would be a claim about a position nobody
    // is in -- a mate score is a distance nothing proved, and DRAW_SCORE reads
    // as a **fail-high** to a verification whose window sits below zero, which
    // under the multicut would hand the parent 0 as the node's value on the
    // strength of a stalemate that does not exist.
    //
    // What is true is that nothing here reached the window, so the node fails
    // low against it -- and the table move is then singular in the only sense
    // the rule means, being the one legal move there is. `alpha0` and not
    // `alpha`: no move was searched, so nothing raised it.
    if (excluded_move != 0) { return alpha0; }

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

  // No store while a move is excluded, S097. What this node just computed is
  // the value of a position **with one of its moves removed**, and the key it
  // would be written under is the position with the move in it. Stored, it
  // poisons every later probe of the position -- including the one the
  // verification's own parent is about to make -- with a score that is wrong by
  // construction and an ordering move that is not the best one.
  if (excluded_move == 0) {
    const int to_store = normalize_score(best_so_far, ply);
    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,
                   static_eval);
  }

  return best_so_far;
}


// The engine's node, and the only one it ever searches.
int negamax(int alpha0,
            int beta,
            int depth,
            size_t ply,
            game_t* game,
            search_state_t* state,
            move_t prev_move,
            bool is_pv,
            bool cut_node,
            move_t excluded_move)
{
  return negamax_at<false>(alpha0, beta, depth, ply, game, state, prev_move,
                           is_pv, cut_node, excluded_move);
}


// The same node with `state->probe` honoured. Tests only; every child it
// searches is an ordinary node. S191.
int negamax_probed(int alpha0,
                   int beta,
                   int depth,
                   size_t ply,
                   game_t* game,
                   search_state_t* state,
                   move_t prev_move,
                   bool is_pv,
                   bool cut_node,
                   move_t excluded_move)
{
  return negamax_at<true>(alpha0, beta, depth, ply, game, state, prev_move,
                          is_pv, cut_node, excluded_move);
}


// The plies a mate at this distance takes. The side to move delivering it
// moves last, so its mate in N is 2N - 1 plies; one it receives is 2|N|.
static size_t plies_to_deliver(int mate_in)
{
  return (mate_in > 0) ? static_cast<size_t>(2 * mate_in - 1)
                       : static_cast<size_t>(-2 * mate_in);
}


// The move the stored line plays from this position, or 0 when this position
// is not on it at this distance.
//
// The index falls out of the distance still owed: a line of `length` plies
// that owes `remaining` of them is at index `length - remaining`. So the
// lookup is one comparison and not a scan, and it cannot answer for a position
// that is on the line at some other point -- which is what stops a repetition
// inside a mating line from handing back the wrong remainder. S170.
static move_t proven_mate_move(const proven_mate_line_t* line,
                               hash_t key,
                               size_t remaining)
{
  if (line == nullptr || remaining == 0 || remaining > line->length) {
    return 0;
  }

  const size_t index = line->length - remaining;

  return (line->keys[index] == key) ? line->moves[index] : 0;
}


// Keeps a line that has been shown to deliver its mate, replacing whatever was
// there. The newest proof is the one a later search is most likely to be
// standing on, and every line that reaches this has passed the same
// all-or-nothing gate, so an older one is never the safer of the two. S170.
static void record_mate_line(proven_mate_line_t* store,
                             const hash_t* keys,
                             const move_t* moves,
                             size_t length)
{
  if (store == nullptr || length == 0 || length > MAX_PLY) { return; }

  memcpy(store->keys, keys, length * sizeof(hash_t));
  memcpy(store->moves, moves, length * sizeof(move_t));
  store->length = length;
}


// The score a node `ply` plies from the root carries while a mate is delivered
// at ply `needed`. Two things decide it. The magnitude is the mate's distance
// from the *root*, because that is what negamax returns and what the pair of
// normalisation functions preserves. The sign alternates, because every score
// in this tree is relative to the side to move, and the side that delivers the
// mate is the one an odd number of plies away from it. S171.
static int mate_score_at(size_t ply, size_t needed)
{
  const int value = MATE_MAX - static_cast<int>(needed);

  return ((needed - ply) % 2 == 1) ? value : -value;
}


// The move whose child the table certifies at exactly the distance the line
// still owes, or 0 when no child carries that claim.
//
// Reporting only, like everything else on this path: a generator, a make and
// unmake per move and a probe. No store, no search, no node counted.
//
// Exact entries only. A bound is not a score -- a lower bound of "mate in n"
// says the value is at least that and leaves open a faster mate, so a line
// built on one can be a line the position does not play. The all-or-nothing
// gate would catch a walk that fails to reach the mate, but not one that
// reaches it at the claimed distance by a road the position would not take.
// DEC-102 is the same rule on the other side of the table. S171.
static move_t certified_mate_move(game_t* game,
                                  const search_state_t* state,
                                  const move_t* moves,
                                  size_t count,
                                  size_t played,
                                  size_t needed)
{
  const int required = mate_score_at(played + 1, needed);

  for (size_t i = 0; i < count; ++i) {
    if (!make_move(game, moves[i])) { continue; }

    const tt_entry_t* entry = tt_get_entry(state->tt, &game->board);

    const bool certified =
        entry != nullptr && entry->type == TT_PV_NODE &&
        de_normalize_score(entry->score, played + 1) == required;

    unmake_move(game);

    if (certified) { return moves[i]; }
  }

  return 0;
}


// Reporting only, and it must stay that way: nothing here searches a node,
// counts one, or writes to the table.
//
// A mate found inside quiescence puts a correct mate score at a node whose
// line continues below the deepest main-search ply, and pv_table holds one
// move per main-search ply, so the line stops where the table stops. Measured
// in S145: of 31 info lines carrying a mate score, 2 were short and 0 had the
// wrong distance -- the score is right and the line is short.
//
// The repair walks from the end of the stored line, taking the move each
// position offers, until the position has no legal reply. Two things offer
// one. The transposition table, which stores its quiescence nodes too
// (TT_DEPTH_QS) including the mated one, so the plies the main search never
// reached are usually still there to be read. And the line this engine last
// proved a mate on, which is what answers when the table no longer can --
// S170, for the case S147 measured and could not close: a score read back from
// an entry an earlier search wrote, whose line has been overwritten since. The
// one ply that is looked for rather than read is the last, for the reason the
// loop below records.
//
// ALL OR NOTHING, and that is the whole safety argument. The walk builds its
// moves aside and the principal variation is only extended when the walk ends
// in checkmate at exactly the distance the score claims. A table entry that
// was overwritten, a bound node with no move, or a walk that wanders off the
// mating line leaves the line exactly as the search produced it -- short, and
// visible to the same check that found the truncation in the first place. It
// can never publish a line that does not deliver the mate it claims. S147.
void complete_mate_pv(game_t* game,
                      search_state_t* state,
                      pv_t* pv,
                      int mate_in)
{
  const size_t needed = plies_to_deliver(mate_in);

  if (pv->length == 0 || needed == 0 || needed >= MAX_PLY) { return; }

  // The line as it is being built, and the position each of its moves is
  // played from. Kept aside from pv->table for the reason the all-or-nothing
  // rule above states, and kept with its keys because a line that reaches its
  // mate is stored for the searches that come after this one -- which is what
  // makes it usable from a later root. S170.
  hash_t keys[MAX_PLY];
  move_t line[MAX_PLY];

  size_t played = 0;
  bool replayed = true;
  bool ends_in_mate = false;

  const size_t from_pv = std::min(pv->length, needed);

  for (size_t i = 0; i < from_pv; ++i) {
    keys[i] = game->board.hash;
    line[i] = pv->table[i];

    if (!make_move(game, pv->table[i])) {
      replayed = false;
      break;
    }

    ++played;
  }

  if (replayed) {
    while (played < needed) {
      move_t moves[MAX_MOVES];
      const size_t count = generate_moves(game_tables(), &game->board, moves);

      // The line ran out of replies before the distance it claims. Nothing to
      // extend and nothing to store: the length check below refuses it.
      if (count == 0) { break; }

      const size_t remaining = needed - played;

      // The stored line is asked first, and it is asked with the distance
      // still owed rather than only with the position: a line that proves a
      // mate at another distance is not this one, even from the same square.
      move_t next =
          proven_mate_move(state->proven_mate, game->board.hash, remaining);

      if (next == 0) {
        const tt_entry_t* entry = tt_get_entry(state->tt, &game->board);
        next = (entry != nullptr) ? entry->best_move : 0;
      }

      // This position's entry is gone, but its children's need not be: a slot
      // is lost to a collision one position at a time, and a mating line's
      // nodes are scattered across the table rather than adjacent in it. So
      // the move the missing entry would have named is looked for one ply
      // down, in the children that still carry an exact score at the distance
      // this line owes.
      //
      // S171, for the residual S170's own 3000-game run left: the walk stalled
      // eight plies from the mate on a single missing slot, with the entry
      // certifying the continuation sitting one ply below it. The claim it
      // publishes is the table's own, at one more remove -- which is what the
      // rest of this walk already does when it reads a best move.
      if (next == 0) {
        next = certified_mate_move(game, state, moves, count, played, needed);
      }

      // Two plies from the mate with nothing left to read: the table's entry
      // is gone and the stored line does not cover this position. What is
      // missing is the defender's last move, and at this distance it can be
      // looked for instead of read -- but only if *every* legal reply is mated
      // in one, because that is what the claimed distance asserts about this
      // position. Requiring all of them rather than taking the first one found
      // is what keeps the published line a principal variation instead of a
      // defender's blunder that happens to end in mate; where the requirement
      // fails the score is claiming a distance this position is not at, and
      // the line is left short and visible.
      //
      // Bounded by one move list inside one move list, run once per reported
      // mate line and only where the walk has already stalled. S170, for the
      // case S147 measured at depth 10 and could not close: a mate proved by
      // this search whose mid-line entry was overwritten before the line could
      // be walked, reproducible on a cold table and so not inherited from
      // anywhere.
      if (remaining == 2 && next == 0) {
        move_t forced = 0;

        for (size_t i = 0; i < count; ++i) {
          if (!make_move(game, moves[i])) { continue; }

          move_t replies[MAX_MOVES];
          const size_t reply_count =
              generate_moves(game_tables(), &game->board, replies);

          bool mated = false;

          for (size_t r = 0; r < reply_count && !mated; ++r) {
            if (!make_move(game, replies[r])) { continue; }

            move_t after[MAX_MOVES];
            mated = (generate_moves(game_tables(), &game->board, after) == 0) &&
                    is_check(game);

            unmake_move(game);
          }

          unmake_move(game);

          if (!mated) {
            forced = 0;
            break;
          }

          if (forced == 0) { forced = moves[i]; }
        }

        next = forced;
      }

      // The last ply of a mating line is a mating move, and it is the ply the
      // table is least likely to answer for: it is scored inside quiescence,
      // which stores no move at all for a node it stood pat on and writes at
      // TT_DEPTH_QS, so it loses every collision. On that one ply the move is
      // looked for rather than read, and what is found wins over what the
      // table or the stored line said -- checkmate is decided by the generator
      // here exactly as it is at every other site in the tree. Measured:
      // without this, 1 line of 524 in the mined set stayed short, its entry
      // gone from the table inside the same iteration that wrote it.
      if (remaining == 1) {
        for (size_t i = 0; i < count; ++i) {
          if (!make_move(game, moves[i])) { continue; }

          move_t replies[MAX_MOVES];
          const bool mates =
              (generate_moves(game_tables(), &game->board, replies) == 0) &&
              is_check(game);

          unmake_move(game);

          if (mates) {
            next = moves[i];
            break;
          }
        }
      }

      if (next == 0) { break; }

      // A move out of the table is checked against the generator rather than
      // trusted: tt_get_entry() compares the full key, but a 64-bit key still
      // collides and a stored move is not re-validated anywhere else. The same
      // holds for a move out of the stored line, which is reached by a key
      // comparison of its own. The scan above already produces a generated
      // move and pays the check twice, which costs nothing on the one ply it
      // runs on.
      bool playable = false;
      for (size_t i = 0; i < count; ++i) {
        if (moves[i] == next) {
          playable = true;
          break;
        }
      }

      if (!playable) { break; }

      keys[played] = game->board.hash;
      line[played] = next;

      if (!make_move(game, next)) { break; }

      ++played;
    }

    if (played == needed) {
      move_t replies[MAX_MOVES];

      // generate_moves() is legal-only (INV-1), so no legal reply while in
      // check is checkmate and needs no per-move pass.
      ends_in_mate =
          (generate_moves(game_tables(), &game->board, replies) == 0) &&
          is_check(game);
    }
  }

  for (size_t i = 0; i < played; ++i) {
    unmake_move(game);
  }

  if (!ends_in_mate) { return; }

  // The line delivers the mate it claims, so it is worth keeping for the
  // searches that follow. A line the search produced whole is stored too, and
  // that is where most of the value is: the search that proves a mate is
  // usually not the one that ends up short of it.
  record_mate_line(state->proven_mate, keys, line, needed);

  if (pv->length < needed) {
    memcpy(&pv->table[pv->length], &line[pv->length],
           (needed - pv->length) * sizeof(move_t));
    pv->length = needed;
  }
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

  // The boundary between the game and this search's tree, fixed here because
  // this is the only way into ply 0: every entry the history holds now was
  // played before the root, and everything the search pushes lands above it.
  // Re-set on every call, so an aspiration re-search and the next iteration of
  // iterative deepening each get the same root. S207.
  state->root_history_size = game->history.size;

  // "The root node is a PV-node" -- CPW Node Types, the first rule of both
  // published lists, and the one this search has always followed through
  // `is_pv`. `cut_node` false is the other half of that label. S098.
  int score =
      negamax_at<false>(alpha, beta, depth, 0, game, state, 0, true, false, 0);

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

  // After the copy, so the extension lands on the reported line and not on the
  // search state, and before the debug PV check below, so a walked line is held
  // to the same legality invariant a searched one is.
  if (search_result.mate_found) {
    complete_mate_pv(game, state, &search_result.pv, search_result.mate_in);
  }

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
