#pragma once
#include "data_structures.hpp"


// The widest window the root can be searched with. It is what search() used
// unconditionally before S021, and it is still the default, so a caller that
// does not aspirate searches exactly the tree it searched before.
inline constexpr int SEARCH_SCORE_INF = 2000000000;

// The window is the root's, and the whole tree below it inherits the bounds.
// Narrowing it is the point of an aspiration search and also its risk: what a
// node is allowed to prune is a property of the bound its parent passed down
// (DEC-060), so a caller that narrows this owns the mate exposure too.
search_t search(int depth,
                game_t* game,
                search_state_t* state,
                int alpha = -SEARCH_SCORE_INF,
                int beta = SEARCH_SCORE_INF);

// Whether a table entry answers this node outright, and with what score.
// `depth` is what the caller needs to have been searched: negamax passes its
// own remaining depth, quiescence passes TT_DEPTH_QS, and the comparison
// against the entry's depth is the whole of the rule that keeps a quiescence
// entry out of the main search. Split out of negamax and declared here so a
// test can hold that rule directly rather than inferring it from a tree. S094.
bool tt_entry_answers(const tt_entry_t* entry,
                      int depth,
                      size_t ply,
                      int alpha,
                      int beta,
                      int* score);

// Whether the side to move is better off here than it was the last time it
// moved. The comparison is against the static evaluation two plies up, four
// when the node two plies up was in check and recorded no number, and it is
// true when there is nothing to compare against.
//
// Declared here because S108 supplies the input and S109 supplies the
// consumers: until then the only caller is the test that holds each branch of
// the definition, and a broken improving calculation costs Elo without ever
// crashing. S108.
bool improving_at(const search_state_t* state, size_t ply, bool in_check);

// The leaf search. Declared here only so the tests can drive it directly;
// nothing outside search.cpp calls it.
int quiescence(int alpha,
               int beta,
               size_t ply,
               size_t qply,
               game_t* game,
               search_state_t* state);

// One interior node. Declared here for the same reason quiescence is: reverse
// futility only fires at a non-PV node past RFP_MIN_PLY, which no call to
// search() can place a test on directly. Already external linkage, so this
// declaration changes no code the compiler emits. Nothing outside search.cpp
// calls it. S103.
//
// `cut_node` is the other half of the node's predicted type since S098 verdict
// 2: `is_pv` alone names PV against not-PV, and the pair names CPW's three --
// PV is (true, false), CUT is (false, true), ALL is (false, false).
//
// It is defaulted on these two entry points and on nothing else. Inside
// negamax_at every recursion states the label its own rule produces, because
// that is the alternation; a test driving one node is choosing which node type
// to ask its question at, and `false` is the choice every case made before this
// step existed -- PV where `is_pv`, ALL otherwise, which is the label that adds
// nothing to the reduction.
int negamax(int alpha0,
            int beta,
            int depth,
            size_t ply,
            game_t* game,
            search_state_t* state,
            move_t prev_move,
            bool is_pv,
            bool cut_node = false);


// The same node with `state->probe` honoured, so a test can watch this node's
// null-move, reverse-futility and reduction decisions instead of inferring
// them from the tree it left behind. Every node below it is an ordinary one:
// a probe names one ply and no child is at that ply.
//
// A separate entry point rather than a flag, because the engine's own node
// must hold no probe code at all -- the branchy version measured 1.49 % of
// nodes per second. Nothing outside a test calls this. S191.
int negamax_probed(int alpha0,
                   int beta,
                   int depth,
                   size_t ply,
                   game_t* game,
                   search_state_t* state,
                   move_t prev_move,
                   bool is_pv,
                   bool cut_node = false);

// Completes a reported mate line so that it reaches the mate it claims, and
// keeps a line that does reach one for the searches that follow.
//
// ALL OR NOTHING (DEC-122): the line is replaced only when what is built is
// legal from this position and ends in checkmate at exactly the distance
// `mate_in` claims. Anything else leaves it exactly as it was -- short, and
// visible to the checks that found the truncation in the first place.
//
// Reporting only: nothing here searches a node, counts one, or writes to the
// transposition table, and `game` is left on the position it was called with.
//
// search() already calls it with the score it is about to return. It is
// declared here for the one caller that cannot: the reporting layer prints the
// last *completed* iteration's score beside an aborted iteration's line, and a
// mate score paired with a line from another iteration has to be completed
// against the score it is printed with, or it claims a mate that line does not
// reach. S170.
void complete_mate_pv(game_t* game,
                      search_state_t* state,
                      pv_t* pv,
                      int mate_in);

// The published history update, in one place because three tables will use it:
// S222's continuation history and S023's capture history share the clamp, the
// overflow discipline and the bonus/malus split with this one.
//
//   entry += clamp(bonus) - entry * |clamp(bonus)| / max
//
// Two properties follow from the algebra and both are load-bearing. An entry in
// [-max, max] stays there, which is what makes the ordering band a closed
// interval rather than an accumulator that has to be saturated. And every
// update shrinks the old value by (1 - |b|/max), so ageing is by construction
// and a periodic halving on top of it would age twice.
//
// Two tables and two bounds since S222, so the bound is a parameter here and
// the one-argument form is the quiet_history call the engine has always made.
// The bound is not a scale the caller may choose freely: it is the bound the
// caller's own table is declared with, and passing another table's turns the
// closed interval into a promise nothing keeps.
//
// Declared here so a test can drive an entry to each asymptote directly. S093.
void history_gravity_update(int16_t& entry, int bonus, int max);
void history_gravity_update(int16_t& entry, int bonus);

// One fail-high on a quiet move: the bonus to the move that cut off, the malus
// to every quiet tried at that node before it. `quiets_tried` never contains
// `cutoff_move` -- the caller appends after the cutoff test, so the exclusion
// is structural. S093.
//
// `prev_move` additionally grades the one-ply continuation table
// (`continuation_entry`, src/data_structures.hpp) over the same two spans, on
// its own coefficients and its own bound, and only when it is non-zero: 0 means
// there is no previous move to index -- ply 0, or the node right after a null
// move -- and the table is left alone rather than crediting or charging its
// (W_PAWN, a8) cell. S222.
void history_on_quiet_cutoff(search_state_t* state,
                             color_t side,
                             move_t cutoff_move,
                             const move_t* quiets_tried,
                             size_t quiets_tried_count,
                             int depth,
                             move_t prev_move);

// The reduction the built table holds for a (depth, move number) pair. It
// exists for two tests. S073's: LMR_BASE and LMR_DIVISOR are read once, when
// the table is built, so a setoption that moved a coefficient without
// rebuilding would be invisible from outside. And S191's, which is why it is
// no longer tune-only -- a case asserting that a guard refused to reduce a
// move says nothing unless the table would have reduced it, and that has to be
// checkable in the build the gate ships as well as the one it tunes.
int search_lmr_reduction_probe(int depth, int move_number);

// The node-type adjustment S098 verdict 2 adds to that table, as a function of
// the five conditions, so a case can hold the arithmetic and the signs
// directly: four terms lengthen the reduction and the PV term shortens it,
// and a sign slip there reduces exactly the nodes whose lines get reported.
// Compiled in both builds for the reason above.
//
// `no_tt_move` is S095's, appended so the four older inputs keep their
// positions: the node's table entry carries no move -- no entry, or one
// quiescence wrote without a move -- and a late quiet there is reduced by
// LMR_NO_TT_MOVE more.
int search_lmr_node_adjustment_probe(bool cut_node,
                                     bool improving,
                                     bool tt_move_is_capture,
                                     bool is_pv,
                                     bool no_tt_move);

// The reduction the two consumers share: the raw table plus that adjustment,
// unclamped, which is what makes "at the off values the engine is the one
// before this step" a property a test can assert rather than a claim. S098.
int search_lmr_adjusted_reduction_probe(int depth,
                                        int move_number,
                                        int node_adjustment);

// The depth the zero-window re-search of a reduced move runs at, as a pure
// function of the five numbers the site has: the child's depth, the reduction
// taken, the reduced search's score, the node's alpha and its fail-soft best
// before the move. It exists so a case can hold the cap, the lower bound, the
// strict inequality against the reduced depth and the one condition the rule
// still has directly, including at inputs the engine's own clamps never produce
// -- a clamp that cannot be reached from the search is still a clamp a later
// step can reach. Its preconditions are the site's: a child depth of at least
// one, a reduction of at least one and a score above alpha. S098 verdict 3, and
// the shallower path it also carried was removed after the bisection measured
// it.
int search_lmr_research_depth_probe(int child_depth,
                                    int reduction,
                                    int score,
                                    int alpha,
                                    int best);

// Which child of a node is searched with which predicted type, one enumerator
// per recursion site in negamax_at. A wrong prediction is silent -- no crash,
// no wrong node count, only rating -- so the rules are named functions and
// this is how a test walks them. CPW Node Types, Garms's list with Kannan's
// summary beside it; src/search.cpp's child_label_t carries which is followed
// where the two disagree. S098.
enum child_kind_t
{
  CHILD_FIRST = 0,
  CHILD_SCOUT,
  CHILD_ZW_RESEARCH,
  CHILD_FULL_RESEARCH,
  CHILD_NULL_MOVE
};

void search_child_label_probe(int kind,
                              bool parent_is_pv,
                              bool parent_cut_node,
                              bool* child_is_pv,
                              bool* child_cut_node);

// The late move pruning threshold the block computes, in hundredths of a move,
// so a test can hold the doubling rule directly instead of inferring it from a
// tree: the count is doubled exactly when `improving_at()` is true and at no
// other time, and a wrong-side default there costs rating without ever
// crashing. Compiled in both builds for the reason the reduction probe is,
// that a case asserting a guard refused to prune says nothing unless the rule
// would otherwise have pruned. S109.
int search_lmp_threshold_probe(int lmr_depth, bool improving);
