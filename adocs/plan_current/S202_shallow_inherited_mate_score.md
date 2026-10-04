id:         S202
goal:       a mate score inherited from the table at a depth too shallow to back it is either given a line that reaches it or not published as a mate -- the class S171's census measured and its own excludes forbade it to touch (DEC-150)
accepts:    the mechanism is reproduced from `adocs/data/S170_cases.tsv`'s `F_mate6_inherited_no_line` before anything is changed, and the reading below is confirmed or corrected from the code rather than argued; whatever is chosen is decided by SPRT if it alters play at all -- it very likely does, since withholding or re-deriving a score changes what the table answers to later nodes -- and by identical `tools/search_bench.py` node counts and best moves if it does not (INV-6); the guard row's `guard` flips to `yes` and `tests/test_mate_carry.cpp` is red before the change and green after; a `fastchess.sh --fast` census reports a candidate count **at or below 8 lines from 1 search in 3000 games**, against the reference's own count in the same run, which is the ceiling DEC-150 set and not a zero; `adocs/specs.md`, `DEV_MANUAL.md`'s instrument 3 and `MANUAL.md` take whatever the verdict is, in the same commit
touches:    src/search.cpp, src/search.hpp, src/chesso.cpp, tests/test_mate_carry.cpp, tests/test_search.cpp, tools/mutants/, adocs/data/, adocs/specs.md, DEV_MANUAL.md, MANUAL.md
excludes:   the completion walk itself -- S147, S170 and S171 are that ground and DEC-150 measured that no walk can close this case, because the line the score names is not in the table to be found -- **except, amended by DEC-251, the walk's choice of move: which of the table's answers `complete_mate_pv()` reads first (an entry exact at the owed distance, then a certified child, then a bound entry's move)**, phase 1 having measured that this closes 13 of 33 short lines on the S170 grid; the rest of that ground stands; improving mate *finding*, which is S148's and S154's ground
decisions:  DEC-154, DEC-156, DEC-122, DEC-127, DEC-150, DEC-151, DEC-162, DEC-251
closes:
blocks:
paused_by:
author:     Opus subagent, coordinator-briefed (DEC-185); phase 1 2026-10-03
done:

## What this is

S171's census, 2026-09-07 on the workstation, left **8** `Incomplete mating PV`
lines from **1** search in 3000 games. DEC-150 is the reading. The short version:

| asked | answer |
|---|---|
| is the score right | **yes** -- `stockfish` gives `#+6` at depth 20 and 30, and `#+5` after the move chesso plays |
| where does it come from | the table, at **depth 3 on 1224 nodes** -- too shallow to build the 11 plies a `mate 6` owes |
| why can the walk not finish it | the line that iteration built continues, in the table, into a chain proving **`mate 8`**; both lookups in `complete_mate_pv()` are keyed on the distance still owed, so both refuse |
| is it a regression | **no** -- three byte-identical short lines on `457e355`, the commit before `certified_mate_move()` |
| does it cost a game | unmeasured; the two builds are INV-6 identical, so nothing in S171's run says either way |

By depth 7 the same search reports `mate 8` with a complete 15-ply line, and by
depth 11 a complete 11-ply `mate 6`. Only the shallow iterations are affected,
and they are the ones a fast time control publishes most of.

## The reproduction, which exists and is cheap

**Read the 2026-09-08 replacement below before running the F command.** S203
redrew the Zobrist keys, and this class is table eviction, so the reproduction
moved with them (DEC-154, DEC-156).

**Retired, kept for the record.** Until 2026-09-08 the case was
`F_mate6_inherited_no_line` at `nodes 300000` from ply 64, **stride 2**:
deterministic at 6 mate lines and 3 short, three runs each on both binaries.
Under the redrawn keys that same row reports **4 mate lines and 0 short**, so
the command below still runs and no longer shows anything. Nothing about the
engine's behaviour changed when that happened; which entries survive to be read
back did.

```bash
# Retired 2026-09-08: reports 4 mate lines, 0 short. Kept because the numbers
# above are what it printed for two months and someone will find them quoted.
~/.venv/chess/bin/python adocs/data/S170_replay.py --engine build/src/chesso \
    --cases adocs/data/S170_cases.tsv --only F_mate6_inherited_no_line
```

**Current, from S203's sweep.** `D_mate_minus6_depth10` reproduces the class on
demand, in seconds, with no match:

```bash
~/.venv/chess/bin/python adocs/data/S170_replay.py --engine build/src/chesso \
    --cases adocs/data/S170_cases.tsv --only D_mate_minus6_depth10 \
    --go 'nodes 1200000' --stride-override 1 --hash 16 --verbose
```

At ply 35 depth 11 it prints `mate -6` with a **10-of-12-ply PV** -- and the
*same depth* also publishes a complete 12/12. So the short line is not the search
failing to find the mate: it is DEC-122's walk refusing to extend a line the
table can no longer certify, publishing the short one rather than a wrong one,
exactly as specified.

**The window is wide, which is what makes it usable**: every budget from 1200000
to 3000000 nodes reproduces it, and 4000000 does not -- which is the budget the
row carries in `S170_cases.tsv`, and why this note exists instead of the
observation being lost behind it. The alternative is a 3000-game match: S171's
census found 8 lines from 1 search in 3000 games, and S203's own 3000-game run
found 3 in total across both engines.

Both reproductions are properties of the key set and the tree together, so
re-run `adocs/data/S203_case_sweep.sh` before trusting either budget if anything
has moved. `build/tools/mate_trace --stride N` reads the entries behind a case;
`DEV_MANUAL.md` instrument 3 has the invocation, and DEC-151 is why a stride is
not optional -- a game gives one engine only its own turns.

## Where to look, and what is already ruled out

Ruled out by measurement, not by argument:

- **the score being wrong.** It is not; Stockfish confirms the distance.
- **a lost slot the walk could route around.** That is S171's
  `certified_mate_move()` and it does not apply: the 11-ply line is not in the
  table at that moment under any route, so nothing to find one ply down.
- **a forced move refused for carrying a bound.** Implemented during S171 and
  reverted -- the guard case stayed red under it. Recorded in DEC-150 so it is
  not tried a second time.

What is left, and the order they are worth separating in:

- **The score and the line come from different places.** The score is the
  table's, for the root; the line is `pv_table`'s, built by this shallow
  iteration. S170's third cause is the same shape one level up -- a score and a
  line from different *iterations* -- and its answer was to complete the line
  against the score it is printed beside. Here that completion cannot succeed,
  so the question is what to print instead.
- **Not publishing what cannot be backed.** The obvious form: where
  `complete_mate_pv()` refuses, report the bound rather than the mate. This is
  where it stops being reporting-only -- what the search *stores* is a separate
  question from what it prints, and the two must not be conflated.
- **Whether a shallow iteration should trust a mate distance from the table at
  all.** The most invasive reading and the one that plainly alters play.

## Why it is not urgent, and where it sits

It changes no game that has been measured: S171's run put the two builds at
+3.24 +/- 8.91 Elo over 3000 games with the residual present in one of them and
absent from the other only by which side met the position. It is a reporting
defect with an unmeasured search-side question behind it. It sits after the
search block's own steps rather than before them, and DEC-150's ceiling of 8
is what a later census is read against.

## Note 2026-09-13, from S109: the residue grew with the pruning block, DEC-205

S109's four shallow-depth rules take 74 % of the tree, and `tests/test_mate_carry.cpp`'s ceilings rose with them, re-derived by `adocs/data/S203_case_sweep.sh --ceilings` over the recorded grids plus `adocs/data/S109_sweep_block.txt`: D_mate_minus6_depth10 1 to 2, E_mate_minus9 8 to 9, F_mate6_inherited_no_line 2 to 5. `unreached.empty()` stayed 0 across the whole grid, so the promise held and the residue this step closes grew; the census this step's accepts asks for is taken against the tree with the block live.

## Phase 1, 2026-10-03

Tree `fe5d3b3` (`bench` 4081329), Release, Hash 16. Every command, output and
throwaway patch is under `.tuning/coord/S202/` (gitignored): `F_tsv_run{1,2,3}`,
`C_tsv_run{1,2,3}`, `F_raw_ply76`, `F_trace`, `C_trace_depth{3,7}`,
`*_sf_*`, `grid_head`, `grid_p0`, `grid3_<variant>`, `sweep_<variant>`,
`search_bench`, `ctest_*`, `patch_<variant>.diff`, and the drivers
`classify{,2,3}.py`, `raw_F.py`, `sf_check.py`, `sf_root.py`. The throwaway
worktrees were removed after.

**Deviations.** (1) The accepts' row `F_mate6_inherited_no_line`, at its own
TSV cell today, reproduces a *different* mechanism from DEC-150's: S170's
third cause, the score of the last completed iteration printed beside an
aborted iteration's line. DEC-150's class is reproduced on `C_mate7_depth11`
at its own cell instead, and on 29 more lines of the grid. (2) The step
file's reading that "both lookups in `complete_mate_pv()` are keyed on the
distance still owed, so both refuse" is wrong for this tree: see below.
(3) The accepts' guard flip is green *before* the change unless F's ceiling
also goes to 0; at ceiling 5 a flip proves nothing.

### Reproduction

```
~/.venv/chess/bin/python adocs/data/S170_replay.py --engine build/src/chesso \
    --cases adocs/data/S170_cases.tsv --only F_mate6_inherited_no_line --verbose
```

Three runs give byte-identical output (md5 `bd0f6814...`): ply 76 publishes
`mate 6` at depths 1 to 7 with 11/11 plies, then `mate 6 depth 7` with
`pv e4e7 f6f5 h7f7 f5g6 f7g7 g6f5 e7d7 f5e4`, 8/11. The raw output shows the
short line is the final `info` at `nodes 100000`, the aborted depth-8
iteration's line. `mate_trace --stride 2 --start 64 --warm 'nodes 100000'
--final 'nodes 100000'` shows that line's entries, all generation 8 and
exact, claim **mate 7** from ply 1 on: the aborted iteration found a 13-ply
mate after `f6f5`, printed beside the depth-7 `mate 6`. Stockfish at depth
30: root `#+6` (two moves); after `e4e7`, `f6f5` and `f6g6` both `#-5`.
So the score is right and the aborted line is a longer mate.

DEC-150's class, three identical runs (md5 `be0feb21...`):

```
~/.venv/chess/bin/python adocs/data/S170_replay.py --engine build/src/chesso \
    --cases adocs/data/S170_cases.tsv --only C_mate7_depth11 --verbose
```

Ply 74, depths 3 to 7: `mate 7` with a PV of 3..7 of 13 plies, then depth 11
and 12: 13/13. `mate_trace --stride 1 --start 40 --warm 'nodes 1500000'
--final 'depth 3'` shows where it comes from. The PV stops at the horizon,
ply 3. The entry there is exact, generation 35 (the previous search), depth
12, `mate 7` at the root. The table's chain is exact through ply 7. After
that it runs on bound entries (upper, lower, upper) and reaches checkmate at
ply **11**, two plies early. Stockfish at depth 30: root `#+7`; at ply 3,
`#-5` for the two best replies. Along the table's walk at ply 9, its `g1h1`
is mated in one where `a4a5` lasts to `#-2`. The score is right, and the walk
followed a defender's non-best move out of a bound entry.

**The whole grid** (9 budgets × 2 strides × 6 cases, `classify.py`) gives the
same mate and short counts as `adocs/data/S116_sweep_s170.txt`, cell for
cell: 873 mate lines, 33 short. 31 are from completed iterations (DEC-150's
class) and 2 from aborted ones (C 4000000 ply 72, F's own cell).

### Code reading (`fe5d3b3`)

- **The score.** `search()` takes it from `negamax_at` at the root. Negamax
  takes table cutoffs only at non-PV nodes (`!is_pv && ply > 0`). But
  `quiescence()` probes the table *with no PV guard* through
  `tt_entry_answers(..., TT_DEPTH_QS, ...)`, so any main-search entry answers
  it. A PV node whose depth reaches 0 hands its window to `quiescence()`,
  and a stored mate there becomes the iteration's score. The stand-pat
  substitution refuses mate scores, so that is not the route. This is the
  "inherited at the horizon" path. In C at depth 3 it is a generation-35
  depth-12 exact entry.
- **The line.** `state->pv_table[0]` holds only main-search plies, so it
  stops at the horizon. `search()` calls `complete_mate_pv()` against the
  score it returns. For an aborted iteration, `iterative_deepening_search()`
  keeps the aborted line (`result.pv = search_result.pv`) with the last
  completed `last_score` and `last_mate_in`, and calls `complete_mate_pv()`
  again against that score (S170).
- **Where the walk refuses.** It does not refuse at a lookup. Only
  `proven_mate_move()` and `certified_mate_move()` are keyed on the distance
  still owed. The table's own `best_move` is not keyed on distance, and it is
  taken from *any* entry, bounds included. It answered every walked ply
  except one. Instrumented over the grid (`patch_p0.diff`, `grid_p0.txt`;
  move sources: T exact, L lower, U upper, c certified):
  - **16** reach checkmate *before* the claimed distance (the `count == 0`
    break).
  - **14** reach the claimed length on a position that is not mate (the
    final `ends_in_mate` check).
  - **3** stall with no move, 4 plies short (C ply 73). `certified_mate_move()`
    fires once there.

  In every early or late ending, the walk enters U/L bound entries past the
  exact part. DEC-122's gate then refuses, as designed: no published
  full-length line failed to mate (0 unreached, python-chess, all variants
  except (b)).

### Options, each measured with a throwaway patch

INV-6 is measured three ways. `tools/search_bench.py` at depths 9 and 12
compares nodes and best move on its 3 positions. `chesso bench` gives a total.
A play fingerprint (best move and final node count of every search) covers
all 108 grid cells.

| option | patch | bench | search_bench 9/12 | grid play | short (of 33) | mate lines | unreached |
|---|---|---|---|---|---|---|---|
| head | -- | 4081329 | -- | -- | 33 | 873 | 0 |
| (a) print a bound where the line is short | `patch_pa` | 4081329 | identical | 0/108 differ | 0 | 840 (+33 bound lines) | 0 |
| (b) no table mate cutoff in quiescence on an open window | `patch_pb` | **4081338** | kiwipete +1 node at both depths | **105/108 differ** | 32 | 703 | **1** |
| (c1) aborted line yields to the last complete line with the same first move | `patch_pc` | 4081329 | identical | 1/108 differ, `ponder` only | 32 | 873 | 0 |
| (c2) walk takes an exact-at-owed-distance entry, then a certified child, before a bound entry's move | `patch_pd` | 4081329 | identical | 0/108 differ | 20 | 873 | 0 |
| (c1)+(c2) | `patch_pe` | 4081329 | -- | -- | 19 | 873 | 0 |

- **(a) reporting-only.** Where the printed line is shorter than the mate it
  sits beside, print `score cp 48000 lowerbound` (or `cp -48000
  upperbound`). Every mate score implies that bound, and it claims no
  distance. The table and the search are untouched. INV-6 identical, so no
  SPRT. It drives every short count to 0 *by construction*: the line is no
  longer a mate line, so the census and `test_mate_carry` stop counting it.
  That is withholding, not closing. Fast suite green apart from the
  environment-only clang-format case. Every ceiling would re-derive to 0.
  UCI surface: a new score form, which needs MANUAL.md's `score` and `pv`
  rows and `test_uci_surface`. Risks:
  - A GUI or harness sees "+480" where it saw "mate 7", 33 times in the
    grid.
  - fastchess's binary contains the `lowerbound` and `upperbound` strings, so
    how it reads bound lines for adjudication (`-resign score=400
    twosided`) and for `-check-mate-pvs` is **unmeasured**. A harness-side
    difference would move adjudications without moving a move.
  - No mate is hidden from play, but one is hidden from the reader.
- **(b) re-derive: do not trust a table mate at a PV horizon.** It alters
  play: the bench moves and 105 of 108 grid cells play differently. It does
  **not** close the class: 32 short lines remain, in different cells, and
  mate lines fall 873 to 703. It also **breaks DEC-122's guarantee**: at
  B_mate6_shallow, stride 1, 300000 nodes, ply 41, depth 8, `mate 5` is
  printed with a full 9-ply PV that is not checkmate. The mate reached the
  root through another route (a non-PV cutoff re-searched) with a line that
  is not its own. Rejected on that alone. Any form of "not trusting" a
  shallow mate distance would owe an SPRT and the same check.
- **(c1) the aborted pairing, which the step file does not name.** When the
  aborted line cannot be completed against the printed score, and the last
  completed iteration's complete line starts with the same move, print that
  line, with `ponder` taken from it. `bestmove` and node counts are
  identical. The one fingerprint difference is F's `ponder f6f5` becoming
  `ponder f6g6`. INV-6 identical, so no SPRT. Pondering is not used by
  `fastchess.sh`; a pondering GUI would ponder a different reply, which is
  surface only. It closes F's own cell. C ply 72 stays short, because the
  previous line did not qualify. Risk to a real mate: none, since both
  lines are DEC-122-complete.
- **(c2) walk exact-first.** Inside `complete_mate_pv()`, when the stored
  line has no answer: take the table's move only if its entry is exact at
  the owed distance (`mate_score_at(played, needed)`). Otherwise take
  `certified_mate_move()`. Fall back to the bound entry's move as today.
  Reporting-only, INV-6 identical, no SPRT. 33 becomes 20: all 6 of A, C's
  own cell 5 to 0, and B 1 of 14. Fast suite green, 0 unreached. **But it is
  the completion walk, which this step's `excludes` names as S147, S170 and
  S171's ground.** DEC-150's "no walk can close it" was measured on one case
  under the old keys and tree. On this tree a walk closes 13 of 33. Doing it
  here needs the coordinator's ruling, or the owner's, to amend `excludes`.
- **The guard flip.** I flipped F to `guard yes` with ceiling 0 in a
  throwaway worktree. `test_mate_carry` is **red at head** (`F reported 1
  short mating PVs of 8 mate lines, ceiling 0`) and **green under
  (c1)+(c2)**. (a) alone is also green, by construction. Ceilings
  re-derived from each grid alone, A..F:
  - head: 6, 5, 5, 0, 0, 1
  - (c1)+(c2): 0, 5, 4, 0, 0, 0
  - (a): all 0

  The recorded `--ceilings` command spans older trees' grids, so lowering
  the shipped ceilings needs DEC-162's rule applied with its grid list
  restated.

### Recommendation

**(c1)+(c2), reporting-only, no SPRT, INV-6 proved as above.** It closes the
case the accepts names (F's own cell, red before and green after) and 14 of
33 grid lines in total. It keeps every remaining line honest and visible, and
it prints a mate only beside a line that reaches it. The cost is amending
`excludes` to allow the walk change (c2). Without that amendment, (c1) alone
still turns the accepts' guard red then green at F's cell, and leaves
DEC-150's class (C, A, B) exactly as it is.

**Not (a) as the primary fix.** It zeroes every count by changing what is
counted, and its harness behaviour under fastchess is unmeasured. It is the
honest fallback for the residue (c1)+(c2) leave, 19 lines: B 13, C 5, F 1 at
stride 1. If chosen, it goes after them as its own change, with MANUAL.md and
`test_uci_surface` first.

**Not (b).** It alters play, does not close the class, and it produced the
first published full-length mate line that does not mate.

The census in the accepts (≤ 8 lines from 1 search in 3000 games) is still
owed after whichever is chosen. A reporting-only change makes it a
measurement of the residue, not a strength run.

## Phase 2, as built (2026-10-04)

DEC-251, the owner's ruling: (c1) and (c2) as measured in phase 1. Not
committed. Logs are under `.tuning/coord/S202/` (`inv6/`, `sweep/`, `mut/`,
`gate.log`, `debug_guard.log`, `prose_checks.txt`, `walk_fixture.txt`,
`phase2.diff`).

**Deviations.**

1. **The (c2) case kills its mutant, and also C.** Mutant MW01, which is
   `fe5d3b3`'s walk, is also killed by `test_mate_carry`: `C_mate7_depth11`
   reports 5 short lines at its own cell against the new ceiling of 4.
2. **The ceiling command is one grid, not several.** DEC-162 prefers several
   samples, but none of the recorded grids was taken with today's walk, so
   the `--ceilings` command now holds the S202 grid alone. The reasoning is
   under "Ceilings" below.
3. **No S171 census script exists to copy.** S171 ran
   `REF=457e355 ./fastchess.sh --fast` by hand. `S202_census.sh` therefore
   follows the `*_sprt.sh` form, with `ROUNDS=1500` in place of the SPRT.
4. **The Debug run is the guard subset only.** I ran `gate_extra.sh`'s six
   Debug guard tests. I did not run the full Debug suite or `test_mate_carry`
   in Debug.
5. **The mutation fixture was a throwaway commit.** `fe5d3b3` plus this diff
   was committed with no branch as `da6538e` in a worktree that has since been
   removed. It is unreferenced and git will collect it.
6. **The new files are intent-to-add.** `tools/mutants/S202_mate_walk.py`,
   `adocs/data/S202_walk_fixture.py` and `adocs/data/S202_census.sh` are added
   with `git add -N` so a diff against HEAD shows them.
   `adocs/data/S202_sweep_s170.txt` is untracked.

**What changed.**

- `complete_mate_pv()` (c2). When the stored mate line has no answer, the
  walk reads, in this order:
  1. the position's own entry, but only when it is `TT_PV_NODE` and its
     de-normalised score equals `mate_score_at()` for the distance still owed;
  2. `certified_mate_move()`;
  3. a bound entry's `best_move`, as the walk did before.

  The separate `certified_mate_move()` fallback below it is folded into this
  order, with the same behaviour.
- `iterative_deepening_search()` (c1). It keeps `last_complete_pv`, the line
  of the last completed iteration. On an aborted iteration whose line the walk
  cannot complete against `last_mate_in`, it prints that line instead when
  three things hold:
  - the aborted line is shorter than `plies_to_deliver(last_mate_in)`;
  - the kept line is at least that long;
  - both lines start with the same move.

  `ponder` is re-read from the new line.
- `plies_to_deliver()` is now declared in `search.hpp` and is no longer
  `static`.

The comments cite DEC-122, DEC-150 and DEC-251 by symbol.

**INV-6, against a Release build of `fe5d3b3` in a throwaway worktree, since
deleted.**

- `bench`: 4081329, with all 121 lines identical apart from nps.
- `chesso bench 12`: 1860699, with all 105 lines identical apart from nps.
- `search_bench` at depth 9: 32932 / 70095 / 25178, best moves c3d5 e2a6
  d7c8q, on both builds.
- `search_bench` at depth 12: 67792 / 280873 / 137893, best moves c3d5 d5e6
  d7c8q, on both builds.

`No functional change`.

**Tests, each red before green.**

- **`F_mate6_inherited_no_line` is guarded (`guard yes`) with ceiling 0.**
  - On `fe5d3b3` it is red: 1 short line of 8 (phase 1, throwaway `ph`).
  - Under MW02, which is (c1) removed, it is red with the same message.
  - With the fix it is green.
- **New case for (c2):** `tests/test_search.cpp`, "the walk takes a certified
  child over a bound entry's move".
  - The root is S145's mate-in-3 row `rbrb4/p1p1p1k1/P1P1P3/6K1/8/3Q4/8/8 w`,
    with `pv` = d3h3.
  - Two entries are planted:
    - after d3h3, an upper bound naming g7f8, which is mated at once;
    - after g7g8, an exact `MATE_MAX - 3` naming g5g6.
  - The case requires the line `d3h3 g7g8 g5g6 g8f8 h3h8`, and that the board
    is untouched afterwards.
  - Under MW01 (the old walk) it is red: `pv.length == 5` fails. With the fix
    it is green.
  - The line is re-derived by `adocs/data/S202_walk_fixture.py`, using
    python-chess enumeration and stockfish for g5g6.
- **Mutants:** `tools/mutants/S202_mate_walk.py`, with the new prefix MW.
  `tools/mutation_check.py --only MW01_walk_bound_move_first
  MW02_aborted_line_no_fallback` on the fresh no-branch fixture: **2 of 2
  killed**, 325 s.
  - MW01 is caught by the test_search case and by `test_mate_carry` at C.
  - MW02 is caught by `test_mate_carry` at F.

**Ceilings, under DEC-162, tightening only.**

- **The rule, stated before the grid was read:** each row takes the cheapest
  budget at its own stride whose cell reports a mate line.
- **The grid:** `adocs/data/S203_case_sweep.sh` was run on a copy of the
  Release binary (`ENGINE=...`). The result is
  `adocs/data/S202_sweep_s170.txt`: 108 cells, 873 mate lines, 19 short. It is
  identical cell for cell to phase 1's (c1)+(c2) grid. Every mate count equals
  S116's grid, so **no budget moves** (A 500000, B 100000, C 1500000,
  D 2000000, E 100000, F 100000 at stride 2).
- **The grid list, proposed:** `--ceilings adocs/data/S202_sweep_s170.txt`
  alone.
  - No recorded grid was taken with today's walk.
  - `S116_sweep_s170.txt` is today's search tree to the node (`bench`
    4081329). It covers the same 108 cells, read with the old walk, so S202's
    grid supersedes it rather than adding a sample.
  - S204's two grids, S109's and S095's are older trees with the old walk.
  - All five stay tracked as records.
  - Later tree-moving re-sweeps append their grids, which is how the list
    regains several samples.
- **Result:** 6, 15, 5, 2, 11, 5 becomes **0, 5, 4, 0, 0, 0** (A..F). F is at
  0, and none rises.
  - The old five-grid command, with S202's grid added, would still answer
    6, 15, 5, 2, 11, 5. That is why the list is restated rather than
    appended to.
- **Updated alongside:**
  - `test_mate_carry.cpp`: its header, the F guard comment, the ceiling block,
    a new S202 paragraph, and the majority wording.
  - `S170_cases.tsv`: a header note.
  - `adocs/data/README.md`: three rows.
  - `DEV_MANUAL.md`: instrument 3 (the corrected reading and the two fixes;
    the standing 8 is kept until the census), the golden row, the sweep
    paragraph, and the majority wording.
  - `MANUAL.md`: the `depth` row (an aborted line can give way to the last
    finished line, with its `ponder`) and the known-limits entry.

**Proposed `adocs/specs.md` sentence** (the coordinator's to write), appended
to the paragraph "The residual, and it is not a wrong score. DEC-150.":

> S202 corrected that reading from the code (DEC-251): only two of the walk's
> lookups are keyed on the owed distance, and the table's `best_move` was taken
> from any entry, bounds included. The walk now reads a move certified at the
> distance still owed -- the entry's own when exact there, else a certified
> child -- before a bound entry's move. A cut-off iteration's line it cannot
> complete against the last finished score gives way to the last finished
> line when both start with the move played. Both are reporting only and
> INV-6 identical. Over the S170 grid they take 33 short lines of 873 to 19.
> The residue is still DEC-122's, short and never wrong, and 8 from 1 search
> remains the ceiling a census is read against until S202's census replaces
> it.

**Gates.**

- Both fast suites: 41/41 Release and 41/41 tune.
- `clang-format.sh --check`: green, with `CLANG_FORMAT_MAJOR=22`.
- `plan_prose_check.py` `--prose`, `--citations` and `--touches`: 0 flagged
  each.
- Debug guard subset (`test_chesso`, `test_openings`, `test_movegen`,
  `test_evaluation`, `test_search`, `test_engine`): 6/6.
- Final `bench`: 4081329.

**Census.** `adocs/data/S202_census.sh` runs fixed rounds (`ROUNDS=1500`) in
the `--fast` regime, `CAND` = the landing and `REF` = its parent, and it
refuses on `PIN_ME`. The reading against DEC-150's 8 lines from 1 search is
written into it before any game, and so are the abort rule and the expected
wall time of about 1 h 25 m at 2110 games/h.

**Proposed commit text.**

```
Prefer certified mate moves; keep the complete line on abort (S202)

S202's phase 1 reproduced short mate lines on fe5d3b3 and corrected
DEC-150's reading from the code: the walk took the table's best_move
from bound entries, and 30 of 33 short lines on the S170 grid were
walks that followed them to a mate too early or to the claimed length
without one, and 3 were walks that stalled. Two of the 33, F's cell
among them, were an aborted iteration's line beside the last completed
mate.

complete_mate_pv() now reads an entry exact at the owed distance, then
a certified child, before a bound entry's move; an aborted line the
walk cannot complete gives way to the last completed line of the same
first move, ponder included. Reporting only: bench, bench 12 and
search_bench at depths 9 and 12 are fe5d3b3's node for node (INV-6).

Over the grid short lines go 33 -> 19 of 873, no budget moves, F is
guarded, and test_mate_carry's ceilings come down 6,15,5,2,11,5 ->
0,5,4,0,0,0 off adocs/data/S202_sweep_s170.txt alone; the vacuity
precondition stays a strict majority, 4 of 6. A direct case and
mutants MW01/MW02 cover each fix. DEC-251, DEC-122, DEC-162.

No functional change
```

**Fast-check fixes (FIX-FIRST, 2026-10-04).**

1. **`S202_census.sh` counted the wrong name.** With `CAND` set,
   `fastchess.sh` names the sides `cand-<sha>` and `ref-<sha>` (`cand_name`,
   and the reference's `-engine ... name=`). The script grepped `from
   candidate`, which would always have read 0. It now greps `from cand-` and
   `from ref-`. Before a zero on both sides is believed, it is checked against
   the two engine names on the banner. Those were the only greps in the
   script.
2. **The `test_mate_carry` majority was loosened by guarding F.** The old
   formula `(guarded + 1) / 2` gave 3 of 5 before F and would have given 3 of
   6, which is a relaxation. It is now `guarded / 2 + 1`: 4 of 6, a strict
   majority as 3 of 5 was. All six guarded cases report at their cells, so it
   is green in both builds. The comment at the site, the file header and
   `DEV_MANUAL.md` say so, including what the old form gave.
3. **The proposed commit text is corrected.** Phase 1's 33 short lines were
   16 early checkmates, 14 that reached the claimed length without a mate,
   and 3 stalls (C, ply 73). Two of them, overlapping the 30, were
   aborted-iteration lines: C at 4000000 ply 72, and F's cell. The code
   comment's "closes 13 of the 33" is right for (c2) alone (33 -> 20). It now
   also says that with (c1) beside it the count is 14, leaving 19.
   "Two plies early" in `DEV_MANUAL.md` became "two to four plies early",
   because B's walks mated four plies early.

Re-run after the fixes: both fast suites 41/41, `clang-format.sh --check`
green, `bench` 4081329. The fixes are comments, a test constant and a
script, so no node can move. The INV-6 readings above stand.

## Landed and pinned (the coordinator, 2026-10-04)

Landed as `dedddf6` on `248b6e2` (`No functional change`, `bench` 4081329), after
a cold fast check that read FIX-FIRST -- the census's candidate count pattern
(`from candidate` where fastchess.sh names the side `cand-<sha>`, so it would
always have read 0), the vacuity precondition (kept a strict majority, 4 of 6,
where `(guarded + 1) / 2` would have given 3 of 6) and the commit text -- all
fixed before the landing. `specs.md`'s DEC-150 paragraph takes the S202
sentence. **Second tier** (DEC-141) on `dedddf6`: Debug self-play 8 games at 4+0.04, 0 `Assertion`, 0 `disconnect`;
`gate_extra` 5 stages green in 1000 s (`.tuning/gate_extra_2026-10-04_S202.log`). The census pair is pinned in `adocs/data/S202_census.sh`.

