"""The six guards S191 gave a direct case and the 2026-09-04 review did not
cover.

Every one is a guard *dropped*, never a value moved: the case it belongs
to establishes the guard's own condition at the node and asserts the rule
did not fire, so removing the clause is what the case has to see. This is
DEC-141 clause 2 in its first instance -- a new rule ships with a mutant
its test kills.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

S = "src/search.cpp"

# **Five of these anchors were re-pointed at S097** and the mutations are
# unchanged. That step gates reverse futility, the null move and the table
# cutoff on `excluded_move == 0`, so the three conditions they name grew a
# clause and the old anchors stopped matching -- `tools/mutation_check.py`
# refuses the whole run when one does, which is how it was found rather than
# silently skipped. Each anchor now carries the clause it sits beside and each
# mutant still removes exactly the guard its note names: N02 the positive mate
# edge, N03 `!is_in_check`, N04 `!is_pv`, N05 the depth bound and N06 the
# negative mate edge. **All five were re-run on the re-pointed anchors and all
# five are killed**, each by the case its note names.


m("N01_nmp_double_null", S, "search/pruning",
  "a second consecutive null move: the parent's pass no longer stops it",
  ('if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&',
   'if (!is_pv && !is_in_check && ply > 0 &&'),
  origin="S191")

m("N02_nmp_mate_band_pos", S, "search/pruning",
  'null move at beta >= MATE_MIN: the positive edge of the band dropped',
  ('      beta < MATE_MIN && beta > -MATE_MIN && game_phase(&game->board) > 0) {',
   '      beta > -MATE_MIN && game_phase(&game->board) > 0) {'),
  origin="S191")

m("N03_rfp_in_check", S, "search/pruning",
  'reverse futility fires in check, on the TT_EVAL_NONE sentinel',
  ('  if (!is_pv && !is_in_check && excluded_move == 0 &&\n      static_cast<int>(ply) >= RFP_MIN_PLY',
   '  if (!is_pv && excluded_move == 0 &&\n      static_cast<int>(ply) >= RFP_MIN_PLY'),
  origin="S191")

m("N04_rfp_pv", S, "search/pruning",
  'reverse futility fires at a PV node, so a reported line gets a bound',
  ('  if (!is_pv && !is_in_check && excluded_move == 0 &&\n      static_cast<int>(ply) >= RFP_MIN_PLY',
   '  if (!is_in_check && excluded_move == 0 &&\n      static_cast<int>(ply) >= RFP_MIN_PLY'),
  origin="S191")

m("N05_rfp_depth_bound", S, "search/pruning",
  'reverse futility is depth-unbounded: RFP_MAX_DEPTH dropped',
  ('RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&\n      beta < MATE_MIN && beta > -MATE_MIN) {',
   'RFP_MIN_PLY &&\n      beta < MATE_MIN && beta > -MATE_MIN) {'),
  origin="S191")

m("N06_rfp_mate_band_neg", S, "search/pruning",
  'reverse futility fails high against a mate bound: negative edge dropped',
  ('RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&\n      beta < MATE_MIN && beta > -MATE_MIN) {',
   'RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&\n      beta < MATE_MIN) {'),
  origin="S191")
