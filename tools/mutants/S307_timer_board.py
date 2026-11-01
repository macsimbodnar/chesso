"""The hard timer, and the board that makes its bound mean anything, S307.

Two mutants over tests/test_engine.cpp's "a stop inside the first iteration cuts
it and the hard timer ends the search within its bound":

  TB01  `stop_search_after_ms` never raises    "the search ran past its hard
        the flag: a dead timer                 limit"
  TB02  quiescence's horizon cut to 12 plies:  "the first iteration on the timer's
        the first iteration on the timer's     board finished", the precondition
        board ends inside the wait             this step added

TB02 is why the precondition exists. The timer is alive and `bestmove` is as
prompt as ever, so without the precondition the case reads this engine green,
and reads it green with TB01 applied as well, the bound being longer than the
iteration. The 12 was read from a sweep: it leaves the eight-queens board above
its 3 ms floor, earlier in the same case, so the run reaches the new assertion,
and it is the deepest horizon that takes the iteration under the bound
(adocs/data/S307_mutants.txt). `tools/mutation_check.py` refuses a file outside
src/, so the plainer lightening, a lighter `timer_fen`, is run by hand in that
file and is no row here.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed" unless a
person has argued the mutant behaviourally equivalent, which no tool decides.

Ids are never reused: TB is this file's own prefix, and no file in
tools/mutants/ has used it, on any branch (`git log --all -p --
tools/mutants`).
"""

m("TB01_hard_timer_never_fires", "src/chesso.cpp", "time",
  '`stop_search_after_ms` sleeps and then raises nothing: on `go movetime 1` '
  'only the soft limit ends the search, at the end of the first iteration, '
  'whatever that costs the clock',
  ("if (session_id == session) { stop_search_signal = true; }",
   "if (session_id == session) {}"),
  origin="S307")

m("TB02_quiescence_horizon_lightens_the_board", "src/search.cpp", "time",
  'quiescence stands pat at 12 plies, and depth 1 on the timer board ends inside '
  'the wait, which leaves `go movetime 1` as prompt as ever and the bound '
  'unable to tell a dead timer from a live one',
  ("if (static_cast<int>(qply) >= MAX_QSEARCH_DEPTH) { return stand_pat; }",
   "if (static_cast<int>(qply) >= std::min<int>(MAX_QSEARCH_DEPTH, 12)) { return stand_pat; }"),
  origin="S307")
