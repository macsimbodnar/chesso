"""Razoring, S116.

Eleven mutants over the one block this step adds, between reverse futility
and the null move in `negamax_at`. Every one is a guard dropped or widened,
the verification taken away, the margin moved off its boundary or the
fail-soft return made hard -- never a constant moved; `RazorDepth` 0 is the
off value DEC-215 asks to be proved on the tree and is not a bug. Each is
killed by a case in tests/test_search.cpp's "search: pruning and reduction
guards" suite, named per mutant:

  RZ01  the verification arm dropped     "razoring falls through where
        (the unverified drop)              quiescence lifts the node above
                                           alpha", "razoring does not hide a
                                           quiet mate the verification lifts"
  RZ02  the in-check guard dropped       "razoring never runs in check"
  RZ03  the PV guard dropped             "razoring never runs at a PV node"
  RZ04  the alpha mate-band guard        "razoring does not run with alpha in
        dropped, positive edge             the mate band"
  RZ05  the same, negative edge          declared equivalent: no static score
                                           comes within the margin of -MATE_MIN
  RZ06  the depth gate a ply wider       "razoring does not run above its
                                           depth"
  RZ07  the ply floor dropped (DEC-248)  "razoring does not run near the
                                           root"; also test_engine's "every
                                           mate in two is found on time" and
                                           "pruning does not hide a mate
                                           against the material leader"
  RZ08  the excluded-move guard dropped  "razoring does not run while a move
                                           is excluded"
  RZ09  the margin test made strict      "razoring drops a hopeless node to
                                           quiescence and returns its score"
  RZ10  the margin not added             the same case, its second leg
  RZ11  alpha returned, not the score    the same case

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: RZ is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch.
"""

S = "src/search.cpp"

GATE = ("  if (!is_pv && !is_in_check && excluded_move == 0 && "
        "depth <= RAZOR_DEPTH &&\n"
        "      static_cast<int>(ply) >= RFP_MIN_PLY && alpha < MATE_MIN &&\n"
        "      alpha > -MATE_MIN && static_eval + RAZOR_MARGIN <= alpha) {")


def gate_with(old_term, new_term):
    """The gate with one term replaced; the term must occur in it once."""
    assert GATE.count(old_term) == 1, old_term
    return (GATE, GATE.replace(old_term, new_term))


m("RZ01_razor_verification_dropped", S, "search/pruning",
  'the quiescence score ends the node whatever it is: the unverified drop, '
  'which hands back a capture\'s score where the node\'s own moves would '
  'have been searched, and a quiet mate among them with it',
  ('    if (razor_score <= alpha) {', '    if (true) {'),
  origin="S116")

m("RZ02_razor_in_check_gate_dropped", S, "search/pruning",
  'the block runs in check, where the static score is the sentinel and '
  'every alpha clears it: an evasion node answered by quiescence',
  gate_with("!is_in_check && ", ""),
  origin="S116")

m("RZ03_razor_pv_gate_dropped", S, "search/pruning",
  'the block runs at a PV node, whose line is reported and played',
  gate_with("!is_pv && ", ""),
  origin="S116")

m("RZ04_razor_mate_band_gate_dropped", S, "search/pruning",
  'alpha at or above MATE_MIN is let in, where every static score is a '
  'margin below it and whole subtrees under a mate bound drop to captures',
  gate_with("alpha < MATE_MIN &&", "true &&"),
  origin="S116")

# Equivalent, and argued rather than inferred: with alpha at or below
# -MATE_MIN the margin test needs a static score at or below -MATE_MIN minus
# the margin, and test_evaluation "the static score never reaches the mate
# band" holds |evaluate()| thousands of centipawns inside it. The term is the
# negative edge's half of the band guard, kept for symmetry with reverse
# futility's and for a future input that is not a static score.
m("RZ05_razor_negative_band_gate_dropped", S, "search/pruning",
  'alpha at or below -MATE_MIN is let in; no static score can then satisfy '
  'the margin test, so nothing changes',
  gate_with("alpha > -MATE_MIN && ", ""),
  expected="equivalent",
  origin="S116")

m("RZ06_razor_depth_gate_widened", S, "search/pruning",
  'the block runs a ply above its depth, the multi-depth form with a flat '
  'margin',
  gate_with("depth <= RAZOR_DEPTH &&", "depth <= RAZOR_DEPTH + 1 &&"),
  origin="S116")

m("RZ07_razor_ply_gate_dropped", S, "search/pruning",
  'the block runs at plies 1 and 2, where the quiet mating move of a mate in '
  'two sits at a depth-1 node quiescence cannot see it from (DEC-248)',
  gate_with("static_cast<int>(ply) >= RFP_MIN_PLY && ", ""),
  origin="S116")

m("RZ08_razor_excluded_gate_dropped", S, "search/pruning",
  'the block runs inside S097\'s verification, where quiescence would '
  'search the excluded move and a fail-low answers the verification unasked',
  gate_with("excluded_move == 0 && ", ""),
  origin="S116")

m("RZ09_razor_margin_strict", S, "search/pruning",
  'the first alpha the margin admits is refused: the boundary moves a point',
  gate_with("static_eval + RAZOR_MARGIN <= alpha",
            "static_eval + RAZOR_MARGIN < alpha"),
  origin="S116")

m("RZ10_razor_margin_dropped", S, "search/pruning",
  'any static score at or below alpha is razored, not one a margin below it',
  gate_with("static_eval + RAZOR_MARGIN <= alpha",
            "static_eval + 0 * RAZOR_MARGIN <= alpha"),
  origin="S116")

m("RZ11_razor_fail_hard", S, "search/pruning",
  'the node returns alpha in place of the verification\'s own score, a '
  'fail-hard bound that throws away how far below alpha the node stood',
  ('      return razor_score;\n', '      return alpha;\n'),
  origin="S116")
