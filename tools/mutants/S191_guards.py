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

m("N01_nmp_double_null", S, "search/pruning",
  "a second consecutive null move: the parent's pass no longer stops it",
  ('if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&',
   'if (!is_pv && !is_in_check && ply > 0 &&'),
  origin="S191")

m("N02_nmp_mate_band_pos", S, "search/pruning",
  'null move at beta >= MATE_MIN: the positive edge of the band dropped',
  ('depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&',
   'depth - 1 - null_reduction >= 1 && beta > -MATE_MIN &&'),
  origin="S191")

m("N03_rfp_in_check", S, "search/pruning",
  'reverse futility fires in check, on the TT_EVAL_NONE sentinel',
  ('if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&',
   'if (!is_pv && static_cast<int>(ply) >= RFP_MIN_PLY &&'),
  origin="S191")

m("N04_rfp_pv", S, "search/pruning",
  'reverse futility fires at a PV node, so a reported line gets a bound',
  ('if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&',
   'if (!is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&'),
  origin="S191")

m("N05_rfp_depth_bound", S, "search/pruning",
  'reverse futility is depth-unbounded: RFP_MAX_DEPTH dropped',
  ('      depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {',
   '      beta < MATE_MIN && beta > -MATE_MIN) {'),
  origin="S191")

m("N06_rfp_mate_band_neg", S, "search/pruning",
  'reverse futility fails high against a mate bound: negative edge dropped',
  ('      depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {',
   '      depth <= RFP_MAX_DEPTH && beta < MATE_MIN) {'),
  origin="S191")
