"""Guard-removal mutants for S191's pruning and reduction cases.

Same form and same driver contract as
adocs/data/2026-09-04_test_review/mutants.py, which is evidence and is never
edited (adocs/data/README.md): these are the six guards that file does not
cover, and each one is a guard S191 added a direct case for. `old` must occur
exactly once in `file`.

Every mutant here is a guard *dropped*, never a value moved: the case it
belongs to establishes the guard's own condition at the node and asserts the
rule did not fire, so removing the clause is what the case has to see.

S196 folds this file and the 2026-09-04 one into tools/mutants/.
"""

M = []


def m(mid, file, klass, note, *pairs):
    M.append(dict(id=mid, file=file, klass=klass, note=note, pairs=list(pairs)))


S = "src/search.cpp"

m("N01_nmp_double_null", S, "search/pruning",
  "a second consecutive null move: the parent's pass no longer stops it",
  ("if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&",
   "if (!is_pv && !is_in_check && ply > 0 &&"))

m("N02_nmp_mate_band_pos", S, "search/pruning",
  "null move at beta >= MATE_MIN: the positive edge of the band dropped",
  ("depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&",
   "depth - 1 - null_reduction >= 1 && beta > -MATE_MIN &&"))

m("N03_rfp_in_check", S, "search/pruning",
  "reverse futility fires in check, on the TT_EVAL_NONE sentinel",
  ("if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&",
   "if (!is_pv && static_cast<int>(ply) >= RFP_MIN_PLY &&"))

m("N04_rfp_pv", S, "search/pruning",
  "reverse futility fires at a PV node, so a reported line gets a bound",
  ("if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&",
   "if (!is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&"))

m("N05_rfp_depth_bound", S, "search/pruning",
  "reverse futility is depth-unbounded: RFP_MAX_DEPTH dropped",
  ("      depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {",
   "      beta < MATE_MIN && beta > -MATE_MIN) {"))

m("N06_rfp_mate_band_neg", S, "search/pruning",
  "reverse futility fails high against a mate bound: negative edge dropped",
  ("      depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {",
   "      depth <= RFP_MAX_DEPTH && beta < MATE_MIN) {"))
