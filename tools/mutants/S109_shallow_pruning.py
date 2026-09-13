"""The shallow-depth pruning block's guards and its four rules, S109.

Five guards and five rules, one mutant each, and every one of them is a guard
dropped or a comparison turned round -- never a constant moved. The case each
belongs to establishes the condition that would make its rule fire at the node
and asserts the guard refused it, which is the S191 shape; removing the clause
is what the case has to see.

This is DEC-141 clause 2 for the four rules that landed together: a new
pruning rule ships with a direct guard test and a mutant that test kills.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. A mutant's several pairs are applied
together as one bug. `expected` is "killed" unless a person has argued the
mutant is behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

S = "src/search.cpp"

m("P01_prune_pv", S, "search/pruning",
  'the shallow-depth block prunes at a PV node, where the line gets reported',
  ('      !is_pv && !is_in_check && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;',
   '      !is_in_check && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;'),
  origin="S109")

m("P02_prune_in_check", S, "search/pruning",
  'the block prunes evasions, with the futility margin on the TT_EVAL_NONE '
  'sentinel an in-check node leaves in the stack',
  ('      !is_pv && !is_in_check && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;',
   '      !is_pv && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;'),
  origin="S109")

m("P03_prune_first_move", S, "search/pruning",
  "a node's only legal move can be skipped, so the no-legal-moves return "
  'reports a stalemate that is not there',
  ('    const bool may_prune = pruning_node && legal_moves_counter >= 1 &&',
   '    const bool may_prune = pruning_node &&'),
  origin="S109")

m("P04_prune_mate_band_pos", S, "search/pruning",
  'the block prunes against a beta at the positive edge of the mate band',
  ('      !is_pv && !is_in_check && ply > 0 && beta < MATE_MIN && beta > -MATE_MIN;',
   '      !is_pv && !is_in_check && ply > 0 && beta > -MATE_MIN;'),
  origin="S109")

m("P05_prune_gives_check", S, "search/pruning",
  'the gives-check exemption is dropped, so a forcing quiet is skipped like '
  'any other -- the clause S109 added after its own mate case went red',
  ('    if (prune_rule != PRUNE_NONE && !is_check_move) {',
   '    if (prune_rule != PRUNE_NONE) {'),
  origin="S109")

m("P06_lmp_improving_halves", S, "search/pruning",
  'improving divides the late move pruning count instead of doubling it, '
  'which is the direction the published record closed unmerged',
  ('  return improving ? 2 * threshold : threshold;',
   '  return improving ? threshold / 2 : threshold;'),
  origin="S109")

m("P07_history_sign", S, "search/pruning",
  "history pruning's threshold loses its sign, so the rule skips the quiets "
  'the table likes best and searches the ones it has written off',
  ('                              [MOVE_TO(moves[i])] < -HP_COEFF * lmr_depth) {',
   '                              [MOVE_TO(moves[i])] > HP_COEFF * lmr_depth) {'),
  origin="S109")

m("P08_see_threshold_sign", S, "search/pruning",
  'the quiet SEE threshold is passed positive, so the rule asks whether the '
  'move gains the margin and skips every quiet that does not',
  ('                  -(SEE_QUIET_COEFF * lmr_depth * lmr_depth))) {',
   '                  (SEE_QUIET_COEFF * lmr_depth * lmr_depth))) {'),
  origin="S109")

m("P09_futility_against_beta", S, "search/pruning",
  'futility compares its margin with beta instead of alpha, so a node prunes '
  'against a bound no move of its own was measured against',
  ('          pruning_eval + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha) {',
   '          pruning_eval + FUT_BASE + FUT_SLOPE * lmr_depth <= beta) {'),
  origin="S109")

m("P0A_prune_mate_band_neg", S, "search/pruning",
  'the block prunes against an alpha inside the mate band, which is the edge '
  'a node inside a mate proof actually meets',
  ('                           alpha < MATE_MIN && alpha > -MATE_MIN;',
   '                           alpha < MATE_MIN;'),
  origin="S109")
