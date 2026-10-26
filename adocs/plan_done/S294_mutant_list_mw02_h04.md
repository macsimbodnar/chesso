id:         S294
goal:       the mutant list's two off-list verdicts on the shipped tree are closed: MW02 is killed by a guard that still reaches its line, and H04 builds
accepts:    `tools/mutation_check.py --only MW02_aborted_line_no_fallback,H04_cont_hist_unread` on the tree this step leaves reads both `killed`; for MW02 the case that kills it is named in `tools/mutants/S202_mate_walk.py` with why the old killer (`tests/test_mate_carry.cpp`'s F_mate6_inherited_no_line cell, ceiling 0) stopped reaching it, read from the engine's output, not assumed; for H04 the mutant compiles under the tree's warnings (`-Werror=unused-parameter`) without changing what it removes; no assertion loosened (TESTS); the fast suite green in both builds
touches:    tools/mutants/S202_mate_walk.py, tools/mutants/S222_continuation_history.py, tests/test_mate_pv.cpp (a new case), tests/test_mate_carry.cpp (a comment), adocs/data/S294_fallback_cell.py, adocs/data/S294_fallback_pick.txt, adocs/data/S294_mw02_reach.txt, adocs/data/S294_aborted_long_line.txt, adocs/data/S294_mutation.tsv, adocs/data/README.md, DEV_MANUAL.md
excludes:   any change in src/; the S170 budgets and ceilings (DEC-299); S290's picker
decisions:  DEC-141, DEC-142, DEC-171, DEC-313
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-26 -- `mutation_check.py --only MW02 H04` on fixture `8c58124` (this tree but its own record) reads both killed, and `--only MW02` on `49c63de` after the fast check's fixes again killed: MW02 by a new `test_mate_pv` case alone, a cold-table cell (S145 mined set line 162, game 4593, `go nodes 150000`) picked and re-derived by `adocs/data/S294_fallback_cell.py` (DEC-142), checking the line reaches its mate and that the fallback ran, because an instrumented build showed F's cell, and every one of the 108 S170 cells, no longer reaching the aborted-line fallback (`S294_mw02_reach.txt`); H04 by `test_evaluation` and `test_search` once its replacement carries `(void)prev_move;`. Baseline on `dcb3d83`: MW02 survived 0/50, H04 stillborn. No `src/` change, no assertion moved, bench 5184358; 50/50 in both builds, format and the four prose modes clean; DEC-141's second tier not binding. Found and not fixed here: an iteration cut short printing a line longer than its mate (`S294_aborted_long_line.txt`), ruled BUGS-binding by DEC-317, S295 its fix, in MANUAL.md's Known bugs meanwhile. Implemented by one Opus 5.5 subagent, the fast check's six fixes included.

## Why this step exists (the coordinator, 2026-10-25)

S083's mutation pass (`adocs/data/S083s_mutation.tsv`, 231 mutants on its
ruled tree) read 224 of 225 killed with two verdicts off the list:
`MW02_aborted_line_no_fallback` (S202) survived and `H04_cont_hist_unread`
(S222) was stillborn, `-Werror=unused-parameter` stopping its build in
`src/evaluation.hpp`. Both read the same with `--only` at the shipped
`e972bd4`, so neither is S083's. The fixes they guard are in the code; what
lapsed is the guard's reach, a test-coverage gap with nothing reachable in
play or on the UCI surface. A DEC-171 filler behind the next strength step,
named in the pre-registration of every run taken while it is open.

## 2026-10-26: the work (the implementer, one Opus 5.5 subagent)

Worktree `../chesso-s294` on branch `s294` at `dcb3d83` (bench 5184358),
mutation fixture `.tuning/s294/mut`, scratch under `.tuning/s294/`. No change
in `src/`; `git diff --cached --stat -- src/` is empty.

### Baseline, on `dcb3d83`

`tools/mutation_check.py tools/mutants .tuning/s294/mut --only
MW02_aborted_line_no_fallback H04_cont_hist_unread --jobs 12`, fixture clean
at `dcb3d83`, the main tree's list (clean), wall 271 s: baseline green, 50
tests, bench 5184358. **MW02 survived**, 0 of 50, bench same, 110.8 s. **H04
stillborn**, `error: unused parameter 'prev_move' [-Werror=unused-parameter]`
on `quiet_history_sum`'s parameter in `src/evaluation.hpp`, 3.9 s. S083's reading, on HEAD
(`adocs/data/S294_mutation.tsv`, run 1).

### MW02: why F's cell stopped killing it, from the engine's output

A throwaway build of `dcb3d83` with `iterative_deepening_search()`'s aborted
path instrumented -- one `info string DIAG` line where an aborted iteration
has a line, one more with the walked length, the length the mate needs, the
last completed line's length, whether the two start with the same move and
whether the fallback's condition holds -- never committed, its worktree
removed (`adocs/data/S294_mw02_reach.txt` carries the patch, both drivers and
every reading):

- **At F's own cell** (`nodes 300000`, stride 2, from ply 64) the aborted path
  runs twice: ply 74, aborted iteration 10 beside the completed `mate 8`, the
  aborted 12-ply line walked to 15 of 15; ply 76, aborted iteration 10 beside
  `mate 7`, 10 plies walked to 13 of 13. Both `fallback 0`: the condition is a
  walked line still short, and the walk never leaves one, so MW02's `false &&`
  changes no line there. S202's cell had the aborted iteration find a longer
  mate (a mate 7 line beside `mate 6`); on this tree every mate F's last
  position reports is mate 7 and the aborted line walks to that same mate.
- **Over the whole S170 grid** (S203's 108 cells, Hash 16): 62 aborted
  iterations beside a carried mate, 61 walked to exactly their distance, 1
  short with a different first move (D, 4000000, ply 35, the residue DEC-122
  leaves visible), and **0 falling back**. So no existing `test_mate_carry`
  cell reaches the line, whatever the budgets; moving them would not help and
  is excluded (DEC-299). F's ceiling, 0 -> 3 since (DEC-282, DEC-295,
  DEC-307), would have absorbed up to three short lines besides.

### The killer: a new guard case

`tests/test_mate_pv.cpp` "an aborted iteration's line printed beside a
completed mate reaches it", in the existing binary (still 50 tests). A cold
Hash 1 scan over 408 FENs (S145's mined set and S097's candidates, 30000 to
1000000 nodes) found 2 cells falling back; `adocs/data/S294_fallback_cell.py`
then states the rule and applies it at Hash 16 over S145's mined set, 20000 to
400000 by 10000, both builds (65 s a sweep): 3 of 318 positions separate, and
it picks line 162 (game 4593), `6k1/3R4/2R1p1Bp/4P3/6q1/p7/P3KP2/8 w - - 5 39`,
at `go nodes 150000`, the middle of the run 100000..200000, margin 50000 nodes
either side (`adocs/data/S294_fallback_pick.txt`). There the completed
iteration 10 prints `mate 7` with 13 plies, iteration 11 is aborted with a
9-ply line of the same first move, the walk refuses, and the shipped build
prints the 13-ply line; MW02's prints the 9.

The case first REQUIREs the precondition, everything a test can see from
outside the engine: the last root search (`uci_last_aspiration_searches()`)
is aborted and has a line, the last completed one scored a mate one iteration
earlier, both start with the same move, and the last two `info` lines carry
the completed depth and the same mate. Then it CHECKs the property: the last
line reaches the claimed distance and ends in checkmate (the engine's own
generator), `bestmove`/`ponder` are that line's first two moves, and -- added
after the fast check -- the last line is the completed one verbatim, which is
what the fallback prints, so a tree where the walk completes the aborted line
itself goes red with a message to re-derive the cell instead of passing
vacuously. The aborted line itself is not printed, so that the walk refuses it
is read from outside only as the script's separation on the MW02 build; a last
line equal to the completed one is consistent with the fallback and does not
prove it. Its golden, the cell, is named at its site with the script
(DEC-142) and has a row in DEV_MANUAL's goldens table. Observed: green
shipped, red under MW02 at `CHECK( 9 >= 13 )`, by hand and by the tool.

`tools/mutants/S202_mate_walk.py`'s table and a new paragraph name the killer
and why F stopped; `tests/test_mate_carry.cpp`'s header paragraph on F says the
same in one sentence. No assertion moved.

### H04

The replacement text gains `(void)prev_move;` before `return score;`, the form
`tools/mutation_check.py`'s header names for a mutant that orphans a symbol;
the removed text is unchanged, so the mutant still takes the continuation term
out of `quiet_history_sum` and nothing else. In that function the term is
`prev_move`'s one use, and was already at `eb0bcd6` (S098 verdict 1), where
the anchor moved there. Built, it is killed: `test_evaluation` "the declared
history ceiling clears the band above it" (`REQUIRE_EQ( s_history,
live_span )`) and `test_search` "pruning does not hide a forced mate", bench
moved.

### The final reading

`--only` both on fixture `8c58124` -- `dcb3d83` plus this working tree but for
`S294_mutation.tsv`, its README row and this record, which no test reads; a
commit object on no branch, the S292 form -- the list read from this
worktree's `tools/mutants` (byte-identical to the fixture's), wall 385 s:
**MW02 killed**, 1 of 50, `test_mate_pv` at the new case alone, bench same,
118.0 s; **H04 killed**, 2 of 50, bench moved, 149.6 s. Mutation score 2 of 2
(`adocs/data/S294_mutation.tsv`, run 2).

### Found, not fixed: an aborted line longer than its mate

The shipped sweep shows one position whose last line is an aborted
iteration's, **longer** than the mate beside it needs and not mate at that
distance: line 168 (game 5850) of `S145_mined_set.tsv`,
`6rk/5rqp/p7/2P1p3/P3Q3/7n/1P4P1/R2R3K b - - 1 32`, cold, every budget
80000..200000. At 150000 the last line
is `mate 4 depth 8` with the 9-ply `h3f2 h1g1 f2e4 d1d2 e4d2 g2g3 g7g3 g1h1
g3h4`; python-chess: not mate after 7 plies, checkmate at 9. The fallback
does not run, its condition being a walked line too short. Stockfish at depth
20 reads the root `#+5` with `h1g1` the defence, and `#+3` after the completed
line's `h1h2`, so the completed `mate 4` is itself one move shorter than the
oracle's. `MANUAL.md`'s `depth` and `pv` rows say the pair "never claims a
mate the line does not reach" and the line is "never wrong"; this pair breaks
both. A reported line, so DEC-171 (c) on its face. `src/` is excluded here,
so nothing was changed and no failing test was added; the reproduction is
`adocs/data/S294_aborted_long_line.txt`. **The coordinator ruled it
BUGS-binding (DEC-317); S295, next in Open, is the fix**, and MANUAL.md's
Known bugs carries it until then (below).

### Gates

`CLANG_FORMAT_MAJOR=22`, `-j12`: `build/` and `build-tune/` with 0 warnings;
fast label **50 of 50 in both** (101 s and 102 s); `./clang-format.sh
--check` clean (it had flagged the new `TEST_CASE` line once, reformatted);
`tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, `--gate`,
one mode per call, nothing flagged (the two `MISSING` notes are S136's).
Bench 5184358 unchanged; `git diff --cached --stat -- src/` empty. DEC-141's
second tier does not bind: no change to `make_move`, `unmake_move`, the
generator or the search, and no new pruning rule -- so no Debug self-play and
no `gate_extra.sh`. Re-run whole after the fast check's fixes: 0 warnings, 50
of 50 in both (110 s and 111 s), format clean, the four prose modes clean,
bench 5184358.

### Documents

`DEV_MANUAL.md`: a goldens-table row for the new cell, one clause in the
`test_mate_pv` paragraph, and the S202 paragraph's F sentence put in the past
(F showed the cause on `fe5d3b3`; `test_mate_pv`'s cell guards it since).
`MANUAL.md`: a Known bugs entry for the defect above (what a client sees, the
code path -- `iterative_deepening_search()` printing the cut-short line with
`ponder` from it, `complete_mate_pv()` replaying only as far as the mate needs
and extending only to a checkmate, S202's fallback keyed on a line still too
short -- the reproduction, DEC-317 and S295), and the `depth` and `pv` rows and
the two neighbouring Known bugs entries that said a mate line is never wrong
now point to it. `adocs/data/README.md`: rows for the five `S294_*` files.
`README.md` not touched; `adocs/specs.md` left to the coordinator.

### The fast check's fixes (2026-10-26, the implementer)

1. `test_mate_pv`'s case gains the check that the fallback ran
   (`last.pv == before.pv`, its message naming `S294_fallback_cell.py`), and
   its comment no longer says nothing more is visible without a `src/`
   change.
2. The case's comment says what the grid shows, none of 62 falling back,
   and `S294_mw02_reach.txt` now carries the split behind the "61 walked, 1
   short" bullet above: the D 4000000 ply-35 lines, its aborted line starting
   with another move.
3. Stale present tense on F's ceiling of 0 in `test_mate_carry.cpp` (the
   header paragraph and the `guard` comment) and DEV_MANUAL's S202 paragraph,
   now past with "3 since, by DEC-282, DEC-295 and DEC-307".
4. MANUAL.md, as under Documents.
5. `S294_aborted_long_line.txt`'s claim about the Hash 1 scan now has its
   evidence: `S294_mw02_reach.txt` lists every cell of that scan whose walked
   length is not the mate's, line 168's 100000 cell among them.
6. Positions named by file line and game (`line 162 (game 4593)`, `line 168
   (game 5850)`) everywhere; `S294_fallback_cell.py`'s rule says what `short`
   tests (not mate at the distance, short or not), and two of its lines are
   inside 79 columns; the sweep hashes in `S294_fallback_pick.txt` are
   labelled prefixes; the over-width line in `S202_mate_walk.py` rewrapped.

`--only MW02_aborted_line_no_fallback` on the refreshed fixture `49c63de`
(this tree but for this record, run 3's lines and its README row; a commit
object on no branch): **killed**, 1 of 50, `test_mate_pv`'s case at
`CHECK( last.pv.size() >= needed )` and `CHECK( last.pv == before.pv )`, bench
same, wall 240 s (`adocs/data/S294_mutation.tsv`, run 3). The shipped build is
green (`test_mate_pv` 3 of 3 cases, then both fast suites below).

### Proposed commit message

```
Guard S202's aborted-line fallback with a cold cell; build H04 (S294)

MW02 survived the fast suite because test_mate_carry's F cell no longer
reaches the aborted-line fallback on the shipped tree: an instrumented
build shows both aborted iterations there walked to their distance, and
none of the 108 S170 cells falls back. A new test_mate_pv case at a
cold-table cell from S145's mined set, picked and re-derived by
adocs/data/S294_fallback_cell.py (DEC-142), reaches it and kills MW02.
H04 was stillborn under -Werror=unused-parameter; its replacement now
reads (void)prev_move, removing the same term, and it is killed.

Found and not fixed (no src/ here): an iteration cut short can print
a line longer than the mate beside it that is not mate at that
distance (adocs/data/S294_aborted_long_line.txt). Ruled BUGS-binding by
DEC-317 and fixed by S295; MANUAL.md's Known bugs carries it until then,
and the depth and pv rows point to it. DEC-141, DEC-142, DEC-171,
DEC-317.

No functional change
```
