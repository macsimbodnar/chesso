"""The null move's entry gate, S114's second verdict.

Three mutants over the clause `(NULL_MOVE_EVAL_GATE == 0 || static_eval >=
beta)` in the null-move condition of `src/search.cpp` `negamax_at`, which is
on at `NullMoveEvalGate` 1 since that verdict (DEC-243). Each is a guard
dropped, a comparison inverted or its boundary moved -- never the switch's
value; `NullMoveEvalGate` 0 is the off value DEC-215 asks to be proved on the
tree and is not a bug. All three are killed by one case in
tests/test_search.cpp's "search: pruning and reduction guards" suite, "the
null move's entry gate refuses a static score below beta", which drives the
castled pawn wall at its own static score and one point above it:

  NG01  the gate's clause dropped      the leg one point above: a pass is made
  NG02  the comparison inverted,       both legs: a pass at the point above and
        `static_eval < beta`            none at the static score
  NG03  the boundary moved,            the leg at the static score: no pass
        `static_eval > beta`

THE SENTINEL IS NOT A MUTANT HERE. In check `static_eval` is TT_EVAL_NONE,
and the block is kept out by `!is_in_check` before the gate reads it --
dropping that guard is `M01_nmp_in_check` in tools/mutants/search.py, whose
case "an in-check node makes no null move" drives beta at TT_EVAL_NONE since
this verdict, where the gate would admit the sentinel.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: NG is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch.
"""

S = "src/search.cpp"

GATE = "(NULL_MOVE_EVAL_GATE == 0 || static_eval >= beta)"

m("NG01_null_gate_dropped", S, "search/pruning",
  "the entry gate's clause dropped: the null move is tried at any static "
  "score, the tree before S114's second verdict",
  ("excluded_move == 0 && " + GATE + " &&", "excluded_move == 0 &&"),
  origin="S114")

m("NG02_null_gate_inverted", S, "search/pruning",
  "the entry gate's comparison inverted: the null move is tried only where "
  "the static score is below beta",
  (GATE, "(NULL_MOVE_EVAL_GATE == 0 || static_eval < beta)"),
  origin="S114")

m("NG03_null_gate_strict", S, "search/pruning",
  "the entry gate's boundary moved: a static score equal to beta is refused",
  (GATE, "(NULL_MOVE_EVAL_GATE == 0 || static_eval > beta)"),
  origin="S114")
