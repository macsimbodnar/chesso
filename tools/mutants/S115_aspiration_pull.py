"""The aspiration loop's fail-low pull and its widening ratio, S115.

Four mutants over what this step adds to `src/chesso.cpp`
`aspiration_after_fail`, the schedule `iterative_deepening_search` re-searches
a failed root with: the pull that brings beta toward alpha on a fail-low, the
guard that keeps it off an infinite bound, and the ratio the band widens by.
Each is a rule dropped, a weight at its extreme, a guard removed or a unit
misread -- never a constant moved; `AspirationFailLowPull` 0 is the off value
DEC-215 asks to be proved on the tree and is not a bug. Each is killed by a
case in tests/test_engine.cpp's "engine: aspiration windows" suite, named per
mutant:

  AW01  the pull dropped: a fail-low     "a fail-low pulls beta toward alpha
        leaves beta where it was         and pushes alpha below the score";
                                         also "the loop re-searches a failed
                                         root with the window the schedule
                                         gives", which sees beta stay put in
                                         the loop itself
  AW02  the pull at full weight: beta    the same two cases, on the check that
        goes all the way to the old      beta stays above the old alpha
        alpha
  AW03  the infinity guard dropped: the  "the pull is never computed while
        pull is computed on a window     either bound is infinite"; nothing
        whose top is SEARCH_SCORE_INF    else can see it, since the loop's own
                                         windows never bring an infinite bound
                                         to the pull at the declared ranges --
                                         the bench signature stays the
                                         parent's and only the direct case
                                         reads the function's contract
  AW04  the widening ratio added as      "each failure widens the band by
        centipawns instead of            AspirationWidenPct and the schedule
        multiplied as a percentage       ends whichever way it fails"

THE PUBLISHED COMPANION IS NOT HERE. S115 also built a root fail-high depth
reduction -- a ply off each consecutive root fail-high's re-search -- with a
switch, a floor, a counter reset and a mate guard, and seven mutants were
planned over it. It was refused on the fast suite's mate guards before landing
and its code left (DEC-245; adocs/data/S115_reduction_as_built.diff), so no
mutant of it exists and none of its ids was ever allocated.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: AW is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch.
"""

C = "src/chesso.cpp"


m("AW01_pull_dropped", C, "search/aspiration",
  'a fail-low pushes alpha below the score and leaves beta where the band put '
  'it -- the loop before S115, which is what AspirationFailLowPull 0 is by '
  'design and what the shipped 2 must not be. The local the pull read is '
  'kept alive with a (void) so the Release build still compiles',
  ("      window.beta -= static_cast<int>((width * ASPIRATION_FAIL_LOW_PULL) / 4);",
   "      (void)width;"),
  origin="S115")

m("AW02_pull_full_weight", C, "search/aspiration",
  'a fail-low moves beta the whole width of the window, onto the old alpha: '
  'the score that failed low is an upper bound on a tree the re-search will '
  'not repeat node for node, so a beta on it leaves no room for the re-search '
  'to come back a little higher, which is the reason the weight is a '
  'fraction',
  ("(width * ASPIRATION_FAIL_LOW_PULL) / 4",
   "width"),
  origin="S115")

m("AW03_pull_infinity_guard_dropped", C, "search/aspiration",
  'the pull is computed whatever the bounds are, so a window whose top is '
  'SEARCH_SCORE_INF has its beta pulled to about a billion: neither the open '
  'window nor a band around anything. Unreachable from the loop at the '
  'declared ranges, which is why the guard is held on the function directly',
  ("    if (window.alpha > -SEARCH_SCORE_INF && window.beta < SEARCH_SCORE_INF) {",
   "    if (true) {"),
  origin="S115")

m("AW04_widen_percent_misapplied", C, "search/aspiration",
  'the ratio is read as centipawns: a failure adds AspirationWidenPct / 100 '
  'to the band instead of multiplying it, so at the shipped 200 the band '
  'grows by two centipawns a failure and the schedule to the full window is '
  'about two hundred failures long instead of six',
  ("(window.delta * ASPIRATION_WIDEN_PCT) / 100",
   "window.delta + ASPIRATION_WIDEN_PCT / 100"),
  origin="S115")
