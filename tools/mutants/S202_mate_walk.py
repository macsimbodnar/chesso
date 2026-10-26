"""The mate line's reporting fixes: S202's two (DEC-251) and S295's (DEC-317).

Seven mutants, each one fix or one branch or check of it taken back out and
nothing else. All are reporting only, so none moves the bench signature -- the
search never reads what they change -- and only the case written for each sees
it:

  MW01  the walk reads a bound entry's move   tests/test_search.cpp, "the walk
        before a certified child              takes a certified child over a
                                              bound entry's move"
  MW02  an aborted line the walk cannot       tests/test_mate_pv.cpp, "an
        complete is printed short             aborted iteration's line printed
                                              beside a completed mate reaches
                                              it", since S294 (its cell
                                              re-picked at S296); until then
                                              tests/test_mate_carry.cpp's
                                              F_mate6_inherited_no_line cell
  MW03  an aborted line longer than the       tests/test_mate_pv.cpp, "the
        mate beside it, not mate at its       aborted path's line choice, on
        distance, is printed as it is         constructed lines"; S295's own
                                              cell for it retired at S296
  MW04  such a line with no completed line   tests/test_mate_pv.cpp, "the
        to give way to is printed as it is,   aborted path's line choice, on
        not cut below the distance            constructed lines", S295
  MW05  a completed line that does not        the same case, S295
        reach its own mate stands in
  MW06  a line at or past its distance and    tests/test_mate_pv.cpp, "a mate
        not mate there is not cut, in         line at or past its distance and
        search() and on the aborted path      not mate there is cut, on
                                              constructed lines", S295, and
                                              the case above
  MW07  the cut's clamp dropped: beside       the same case, its `mate 1`
        `mate 1` a refused line is cut to     check, S295
        no moves at all

Since S295's run 2 the constructed-lines case kills MW02 and MW03 as well
(adocs/data/S295_mutation.tsv). Since S296 it is MW03's only killer: S296's
rule removed the short `mate 4` S295's node-limited cell rested on, and
adocs/data/S295_long_line.py finds no cell that separates on its tree
(adocs/data/S296_goldens.txt, adocs/data/S296_mutation.tsv). Not in the list, and why: search()'s call of
cut_unreached_mate_pv() itself. Taking it out survives every case, because no
search measured prints a completed line at or past its distance that is not
mate there (adocs/data/S295_census.txt); the function it calls is what MW06
and MW07 guard.

WHY MW02'S KILLER MOVED, S294, read from the engine's output and not assumed.
S202 wrote F's row to kill MW02 at F's own cell, ceiling 0. On dcb3d83 (S135's
fit) MW02 survived the whole fast suite, 0 of 50, as it had on S083's pass and
at e972bd4 (adocs/data/S083s_mutation.tsv). A build with the aborted path
instrumented, never committed, printed what the path does at F's cell (300000
nodes, stride 2, from ply 64): it runs twice, at ply 74 and 76, each time an
aborted iteration 10 beside a completed mate 8 and mate 7, and each time the
walk completes the aborted line to its distance, 15 of 15 and 13 of 13 plies,
so the fallback's condition -- a walked line still short -- is never met and
MW02 changes no line there. Over all 108 cells of the S170 grid it runs 62
times and falls back none (adocs/data/S294_mw02_reach.txt). S202's own cell
had the aborted iteration find a longer mate than the score beside it, a mate
7 line beside `mate 6`; on this tree every mate F's last position reports is
mate 7 and the aborted line walks to that same mate. F's ceiling, 0 -> 3
since (DEC-282, DEC-295, DEC-307), would have let it absorb up to three short
lines anyway. The new case is a cold-table
cell from S145's mined set, at `go nodes 150000`, where the shipped build
prints a line that reaches its mate and MW02's prints the aborted line 9 of 13
plies; adocs/data/S294_fallback_cell.py picks it and re-derives it (at
S296, line 101 of the mined set at `go nodes 40000`).

MW01 is `complete_mate_pv()` as it stood before S202: the table's move from
any entry first, a certified child only where the entry named nothing. MW02 is
`iterative_deepening_search()`'s aborted path as it stood before S202: the
aborted line completed against the last score and printed short where that
walk refuses, with the last completed iteration's line beside it unused.
Since S295 it takes out the short half of the fallback's condition, which is
S202's; the other half, a line long enough and not mate, is S295's and stays.

MW03 takes `wrong_line` out of the aborted path: a line the walk leaves at or
past the claimed distance is taken as reaching the mate, so one that runs to a
longer mate is printed beside the shorter score, `ponder` its second move
(adocs/data/S294_aborted_long_line.txt), with neither the fallback to the
completed line nor the cut below the distance. That is 4fb915e's behaviour on
the `info` lines and not its code: `ponder` is still taken from the line
printed, and the fallback's test that the completed line reaches its mate is
kept. Its build's mined sweep matched 4fb915e's `info` lines line for line,
which is all that was compared. Its cell was picked by
adocs/data/S295_long_line.py and is retired since S296, which finds none.

MW04 takes out S295's other branch only: a wrong line with no completed line
to give way to -- another first move, or a completed line that does not reach
its own mate -- is no longer cut; its id names the first case only. No search measured reaches that branch (adocs/data/S295_census.txt), so
its killer drives the line choice directly, through
uci_aborted_mate_line_probe(), on a position and lines constructed for it.

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
  ("  if ((short_line || wrong_line) && same_choice) {\n",
   "  if (((false && short_line) || wrong_line) && same_choice) {\n"),
  origin="S202")

m("MW03_long_aborted_line_kept", C, "uci/reporting",
  'an aborted line the walk leaves at or past the claimed distance is taken '
  'as reaching the mate, so one that runs to a longer mate is printed beside '
  'the shorter completed score with ponder from it, although the completed '
  'line starts with the same move and reaches the mate',
  ("  const bool wrong_line = !short_line && !mate_pv_reaches(root, line, mate_in);\n",
   "  const bool wrong_line =\n"
   "      false && !short_line && !mate_pv_reaches(root, line, mate_in);\n"),
  origin="S295")

m("MW04_wrong_line_other_move_kept", C, "uci/reporting",
  'an aborted line at or past the claimed distance and not checkmate there, '
  'with no completed line to give way to -- another first move, or a '
  'completed line that does not reach its own mate -- is printed as it is '
  'beside the completed mate score instead of being cut below the distance',
  ("  } else if (wrong_line) {\n",
   "  } else if (false && wrong_line) {\n"),
  origin="S295")

m("MW05_completed_line_unchecked", C, "uci/reporting",
  'the last completed line stands in for a short or wrong aborted line on its '
  'first move alone, so a completed line that does not reach its own mate '
  'replaces one that the cut would have left short',
  ("                           mate_pv_reaches(root, last_complete, mate_in);\n",
   "                           (true || mate_pv_reaches(root, last_complete,\n"
   "                                                    mate_in));\n"),
  origin="S295")

m("MW06_completed_mate_line_uncut", S, "search/reporting",
  'a line at or past its mate\'s distance that is not checkmate there is left '
  'as it is, so search() prints a completed iteration\'s line beside its own '
  'mate score that does not deliver it, and the aborted path\'s wrong line '
  'with no completed line to give way to is no longer cut',
  ("  if (needed == 0 || pv->length < needed ||\n",
   "  if (true || needed == 0 || pv->length < needed ||\n"),
  origin="S295")

m("MW07_cut_below_first_move", S, "search/reporting",
  'the cut drops its clamp, so beside `mate 1` a refused one-move line is cut '
  'to no moves: a printed line without the move played, and a finished '
  'iteration whose line comes back empty, which the loop reads as no result',
  ("  pv->length = std::max<size_t>(1, needed - 1);\n",
   "  pv->length = needed - 1;\n"),
  origin="S295")
