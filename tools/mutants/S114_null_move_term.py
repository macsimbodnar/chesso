"""The null move's static-score term, S114.

Six mutants over what this step adds to the null-move block in
`src/search.cpp` `negamax_at`: the term
`min(max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN, NULL_MOVE_EVAL_CAP)`,
the floor that tests the whole reduction, and the entry gate's switch. Each is a
bound dropped, a sign flipped, a wrong input read or a switch ignored -- never a
constant moved; `NullMoveEvalCap` 0 is the off value DEC-215 asks to be proved
on the tree and is not a bug. Each is killed by a case in tests/test_search.cpp's
"search: pruning and reduction guards" suite, named per mutant:

  NT01  the cap dropped                 "the null move reduction adds a ply per
                                         margin the static score stands above
                                         beta"
  NT02  the floor tests the old part     "the floor refuses a pass the
        of R only                        static-score term leaves no ply for";
                                         also "the static-score term does not
                                         hide a forced mate"
  NT03  the term's sign flipped         "the null move reduction adds a ply per
                                         margin ...", "a static score below beta
                                         takes nothing off the null move
                                         reduction"
  NT04  the sentinel read: the term     "the null move reduction adds a ply per
        takes the table's evaluation     margin ..."
        slot, TT_EVAL_NONE wherever
        the entry carries none
  NT05  the gate's switch ignored       "a static score below beta takes nothing
                                         off the null move reduction"
  NT06  the clamp at zero dropped       the same case, run alone; in the whole
                                         suite a static score below beta gives
                                         the null search plies back everywhere,
                                         the tree explodes and six binaries time
                                         out before they report

THE SENTINEL HAS TWO ROUTES AND THIS FILE CARRIES ONE. In check `static_eval`
is itself TT_EVAL_NONE, and the block is kept out by `!is_in_check` -- dropping
that guard is `M01_nmp_in_check` in tools/mutants/search.py. In a whole-suite
run M01 makes a null move in check and test_search crashes, six other binaries
with it, before S114's "the static-score term is never computed at a node in
check" runs, so that case was observed red alone, on a release build with M01
applied: R 11 against -1 (`.tuning/coord/S114b_logs/m01_sentinel_case.log`,
on the tree this lands on, as on the first build's).
NT04 is the other route: the term's input read from `tt_eval`, the slot
`static_eval` is taken from when the entry has one. The clamp swallows the
sentinel, so the term is silently zero on every node whose entry carries no
evaluation -- a wiped table's every node, which is what the case drives.

THE DEMOLITION IS NOT A MUTANT HERE. S114's accepts asks for the floor and the
cap lifted together and a mate case observed red; that build is a throwaway
copy, run once per tree and logged
(`.tuning/coord/S114b_logs/demolition_red.log` on the tree this lands on), and
not a bug a person could ship by one edit. NT01 and NT02 are its two halves,
each killed on its own.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: NT is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch.
"""

S = "src/search.cpp"

TERM = ("        std::min(std::max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN,\n"
        "                 NULL_MOVE_EVAL_CAP);")


m("NT01_null_eval_cap_dropped", S, "search/pruning",
  'the static-score term is uncapped: a node twenty pawns above beta takes '
  'twenty plies off the null search. With the floor kept that makes the pass '
  'impossible rather than blind -- the node is searched -- so it is a lost '
  'reduction and not a hidden mate; with the floor gone too it is the '
  'demolition',
  (TERM,
   "        std::max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN;"),
  origin="S114")

m("NT02_null_eval_floor_misses_term", S, "search/pruning",
  'the floor tests the reduction before S114 and not the whole of it, so a '
  'lead that takes the last ply off the null search still lets the pass be '
  'made: the null search falls to quiescence, answers with the static score, '
  'and a mate two plies away is pruned -- the bug the floor exists for',
  ("    if (depth - 1 - null_reduction >= 1) {",
   "    if (depth - 1 - (NULL_MOVE_BASE + depth / NULL_MOVE_DIVISOR) >= 1) {"),
  origin="S114")

m("NT03_null_eval_sign_flipped", S, "search/pruning",
  'the lead is measured the wrong way round: a node below beta is reduced '
  'more and a node far above it not at all',
  ("std::max(static_eval - beta, 0)",
   "std::max(beta - static_eval, 0)"),
  origin="S114")

m("NT04_null_eval_reads_table_slot", S, "search/pruning",
  'the term reads `tt_eval`, the table\'s evaluation slot, in place of the '
  'node\'s static score. The slot is TT_EVAL_NONE wherever the entry carries '
  'no evaluation, and the clamp turns that sentinel into a silent zero, so the '
  'term is off on every such node',
  ("        std::min(std::max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN,",
   "        std::min(std::max(tt_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN,"),
  origin="S114")

m("NT05_null_eval_gate_switch_ignored", S, "search/pruning",
  'the entry gate applies whatever `NullMoveEvalGate` says: at the shipped 0 '
  'a node whose static score is below beta makes no null move, which is S114\'s '
  'second verdict shipped inside its first (DEC-243)',
  ("(NULL_MOVE_EVAL_GATE == 0 || static_eval >= beta)",
   "(static_eval >= beta)"),
  origin="S114")

m("NT06_null_eval_clamp_dropped", S, "search/pruning",
  'the term is not clamped at zero, so a static score below beta gives the '
  'null search plies back -- a different rule, and the reason cap 0 is the '
  'tree before S114 at gate 0 stops being true for every other cap',
  ("std::max(static_eval - beta, 0) / NULL_MOVE_EVAL_MARGIN",
   "(static_eval - beta) / NULL_MOVE_EVAL_MARGIN"),
  origin="S114")
