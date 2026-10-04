"""The mate line's two reporting fixes, S202 (DEC-251).

Two mutants, one per fix, each the fix taken back out and nothing else. Both
are reporting only, so neither moves the bench signature -- the search never
reads what they change -- and only the case written for each sees it:

  MW01  the walk reads a bound entry's move   tests/test_search.cpp, "the walk
        before a certified child              takes a certified child over a
                                              bound entry's move"
  MW02  an aborted line the walk cannot       tests/test_mate_carry.cpp, "a mate
        complete is printed short             score carried across searches
                                              keeps a line that reaches it", at
                                              F_mate6_inherited_no_line's own
                                              cell, ceiling 0

MW01 is `complete_mate_pv()` as it stood before S202: the table's move from
any entry first, a certified child only where the entry named nothing. MW02 is
`iterative_deepening_search()`'s aborted path as it stood before S202: the
aborted line completed against the last score and printed short where that
walk refuses, with the last completed iteration's line beside it unused.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed".

Prefix MW is new: every single letter and AW, NG, NT, PD and RZ are taken
(`git log --all -p -- tools/mutants`).
"""

S = "src/search.cpp"
C = "src/chesso.cpp"

m("MW01_walk_bound_move_first", S, "search/reporting",
  'the walk takes the table\'s move from any entry before a certified child, '
  'so a defender\'s reply out of an upper bound -- mated two plies early -- '
  'is walked and the line is refused short, where an exact child at the owed '
  'distance would have completed it',
  ("        if (exact_at_distance) {\n",
   "        (void)exact_at_distance;\n"
   "        if (entry != nullptr && entry->best_move != 0) {\n"),
  origin="S202")

m("MW02_aborted_line_no_fallback", C, "uci/reporting",
  'an aborted line the walk cannot complete against the last completed '
  'score is printed short beside it, although the last completed line '
  'starts with the same move and reaches the mate',
  ("      if (result.pv.length < needed && last_complete_pv.length >= needed &&\n",
   "      if (false && result.pv.length < needed &&\n"
   "          last_complete_pv.length >= needed &&\n"),
  origin="S202")
