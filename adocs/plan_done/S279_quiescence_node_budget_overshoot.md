id:         S279
goal:       `go nodes N` never searches more than N nodes: quiescence checks the limit as soon as it counts its node, as `negamax_at` does, so a table cutoff can no longer count the node that reaches the budget without aborting
accepts:    (1) the defect reproduced first with a minimized failing test on achesso's head, observed red, not assumed -- a position and budget where quiescence's table cutoff counts the budget's last node and the search reports N+1 (S261's probe `.tuning/coord/S261/nodes_over.py` found 30 of 1028 budgets over on `fd9e44c`); (2) the cause stated from the code: `quiescence` increments `explored_nodes` and reaches `check_limits` only after its early exits, so the stop flag's 2048-node poll is skipped the same way -- say whether that poll skip is real and what the fix does to it; (3) the fix makes the limit exact -- the minimized test green, the probe's 1028 budgets 0 over, `test_search`'s "the node budget is never exceeded" and "an aborted search reports no move at all" green; (4) what moves is proved, not argued: `chesso bench` (fixed depth, no budget) and `tools/search_bench.py`'s node counts and best moves identical to the parent (INV-6), and the budgeted searches that move are named; (5) whether the change needs an SPRT is proposed with its reason and the precedent found in `adocs/decisions.md` and `adocs/plan_done/` (a stop-path fix whose timed-play effect is the stop flag's poll granularity) -- the coordinator decides; (6) the goldens a node budget drives are re-derived by their scripts where they move (DEC-142): the S170 carry cases and their ceilings (`adocs/data/S203_case_sweep.sh`), any `go nodes` count in `tests/`; the fast suite green in both builds, format clean, DEC-141's second tier (it touches the search)
touches:    src/search.cpp, tests/test_search.cpp or tests/test_engine.cpp, adocs/specs.md and MANUAL.md if the `nodes` wording moves, goldens a node budget drives, DEV_MANUAL.md, tools/mutants/S279_qs_node_budget.py
excludes:   S261's vector (paused behind this step); any other change to quiescence's order of work; the `info` reporting of an aborted iteration (S195, S210)
decisions:  DEC-171, DEC-141, DEC-142, INV-6
closes:
blocks:     S261
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S279_brief.md), 2026-10-13
done:       2026-10-14 -- `quiescence` checks the limits right after its count, above the table probe: a table cutoff that spent a `go nodes` budget or landed on the stop flag's 2048-node poll went unchecked. Red first: a three-node quiescence on a hand-prepared table (budget 2 explored 3, no abort), a sweep of every budget through `search` on that table, its precondition asserted (3 explored 4), and startpos at depth 2 as a supplement (100 explored 101). S261's probe 30 of 1028 over (up to +3) -> 0; the flag set before the search read late in 6724 of 150932 book positions -> 0. INV-6: `bench` 3872032 and `search_bench` depths 9 and 13 node-identical, same best moves; the 30 over-budget probe searches move to exactly N, no `bestmove` moves. S203 `--at` identical, no golden re-derived. Fast suite 46/46 both builds, format clean; Debug self-play 8 games, 0 `Assertion`; NB01 killed by both cases (re-run after the fixes); `gate_extra.sh` five stages green (1001 s) on the final tree, prose and citations included. Fast check: 4 findings fixed (the sweep's run-derived floor replaced by an asserted precondition, DEV_MANUAL's overrun stated as measured, the two readers of an aborted quiescence's 0 named, the test states on the heap); the recipe's file-transport failure filed as S280. SPRT: none, discharged by INV-6 and the counted reach (DEC-279). specs.md gains the rule; MANUAL.md checked, unchanged, its "bounds the whole search" now true. Implemented by an Opus subagent briefed by the coordinator.

## Why this exists

Found 2026-10-13 by S261's phase 2: on the lane's vector `test_search` "the
node budget is never exceeded" and "an aborted search reports no move at all"
read budget 4096, explored 4097. The vector only moves which budget lands on a
quiescence table hit: over UCI `go nodes N` for 1028 values of N reports N+1
in 30 searches on the incumbent `fd9e44c` and 22 on the vector. A throwaway
build with `check_limits` right after quiescence's increment turned both cases
green. It is on the UCI surface, so DEC-171's first clause puts it before
anything else; S261 is paused behind it, and the carry family S261 measured
at node budgets is re-measured after it lands.

## As built

Opus subagent, 2026-10-13/14, worktree `/home/max/ws/chesso-s279` on
`bcd5786`, uncommitted. Evidence under `.tuning/coord/S279/` in the main
repository (gitignored); the probe and scan sources are copied there.

### Deviations

1. **The overshoot is N + 1 to N + 3, not only N + 1.** Consecutive table
   cutoffs each count a node and none checks, so the search runs on until the
   next node that does. The probe reads +1 on 24 of the 30 budgets, +2 on 5,
   +3 on one (kiwipete `go nodes 5329` reported 5332).
2. **Two cases, not one.** The minimized reproduction drives `quiescence`
   directly on a table prepared by hand, because that is the only form in
   which the precondition (the node that spends the budget is a table cutoff)
   is asserted rather than assumed; a `go nodes` reproduction cannot see which
   node was a cutoff. The second case sweeps every budget of two trees through
   `search()`: the same prepared position at depth 1, where the precondition
   is asserted, and startpos at depth 2, where the defect was found with the
   engine (budget 100, explored 101), as a supplement that claims no cutoff.
   Neither carries a chosen budget, so neither has a golden to go stale.
3. **`adocs/specs.md` is not edited**, being the coordinator's document; no
   sentence in it is false after the change (it states neither the budget's
   exactness nor the poll). The sentence proposed below is the coordinator's
   to write, as ruled after the fast check. `MANUAL.md` is checked and
   unchanged: "The budget bounds the whole search" was false before this step,
   by up to three nodes as measured, and is true after it. `DEV_MANUAL.md`
   gains one sentence under "Depth at a fixed node budget", whose "The budget
   is enforced exactly" was the same kind of false.
4. **`touches:` amended** after the fast check: it omitted `DEV_MANUAL.md`
   (item 3) and `tools/mutants/S279_qs_node_budget.py` (DEC-141's mutant),
   which the change needed, and now names both.
5. **The mutation fixture's `doctest` submodule needed
   `git -c protocol.file.allow=always`**: the submodule's URL is the main
   tree's local path, and this git refuses the `file` transport by default,
   so `DEV_MANUAL.md`'s recipe line `git -C .ref-builds/mut submodule update
   --init tests/doctest` fails here with "transport 'file' not allowed".
   Independent of this step and not fixed here: the coordinator filed it as
   S280.
6. **The fast check's four findings, fixed.** (1) The startpos sweep's floor
   `whole.explored_nodes > 100` was run-derived and asserted nothing about a
   table cutoff; the sweep now asserts its precondition on the prepared tree,
   and startpos stays as a supplement that claims none. (2) `DEV_MANUAL.md`'s
   "one to three nodes past it" stated the probe's maximum as a limit; it now
   says what was measured. (3) The source comment said the aborted node's 0
   decides nothing; it now names the two readers that see it. (4) Case 1 held
   four `search_state_t` of 1.2 MiB each on one frame; both cases now take
   them from `std::make_unique`, as `tests/test_engine.cpp` does.

### Cause

`src/search.cpp` `quiescence` incremented `explored_nodes`, probed the table,
and returned the entry's score at once when `tt_entry_answers` accepted it.
`check_limits` ran only below that, after the static evaluation and the
stand-pat substitution, so a node answered from the table was counted and
never checked. `check_limits` is an exact compare against `node_limit` plus
the stop flag's poll at every count divisible by 2048; both halves were
skipped on such a node. `negamax_at` checks right after its own count, and
the table cutoff was quiescence's only exit above its check, so this was the
whole gap.

- **Budget**: when the node that reached `node_limit` was a table cutoff,
  nothing aborted; the parent searched on and the next checked node aborted.
- **Poll: real, measured.** With the stop flag set before the search, every
  one of the 150932 positions of `noob_3moves.epd`, searched cold at depth 30,
  should stop at node 2048. On the parent 144208 did, 6438 stopped at 4096,
  278 at 6144 and 8 at 8192: **6724 of 150932, 4.45 %, read the flag 2048 to
  6144 nodes late** (`stop_poll_parent.txt`, `scan_stop.cpp`). At the
  `bench` rate of about 5 Mnps that is 0.4 to 1.2 ms. After the fix: 150932 of
  150932 at 2048 (`stop_poll_fixed.txt`). Every counted node is now a checked
  node, so the poll fires at every multiple of 2048.

### Fix

`check_limits` moved to the line after the count (after the tune build's
`f22_census.qnodes` increment, so the census still counts every call), above
the probe, returning 0; the check below the probe is removed, since it ran at
the same count and could differ only by the stop flag read a few
instructions later. Every node above tests `aborted` before it acts on the
returned score (the quiescence move loop, the razor, ProbCut's preliminary,
and `negamax_at`'s depth-0 return, whose own callers test it). Two readers
still see the 0 where they saw the stand pat, and the comment names both: a
probed node's `razor_score`, written before the razor tests `aborted`, and
`search` called at depth 0, which returns quiescence's score without testing
`aborted`. Neither is reached in play: the probe is a test's, and every
caller of `search` in the engine and `tools/datagen.cpp` starts at depth 1.
No case reads either after an abort (fast suite green). The comment above
the ply cap says what it still guards.

### Tests

`tests/test_search.cpp`, suite "search: budgets"
(`red_parent_test_search.txt`). Both cases take their states from
`budget_state`, heap-owned, and share `prepare_qs_cutoff_table`:

- "a table cutoff that spends the node budget stops the search": White's e4
  pawn with two loose black pawns to take, an exact `TT_DEPTH_QS` entry stored
  on each child and none on the root. Preconditions asserted: each child
  searched cold returns something other than the stored -150, and with the
  entry it returns -150 at one node; unbounded, the root returns 150 over
  exactly 3 nodes without aborting. Then budget 2: **red on the untouched
  tree, `REQUIRE( 3 <= 2 )`, and `aborted` false** (the latter observed with
  both assertions as `CHECK`, reverted). Green after.
- "every node budget stops on its own node", through `search`. First the
  same position at depth 1. Preconditions asserted: from an empty table the
  root scores something other than 150; on the prepared table it scores
  exactly 150 without aborting, which only a quiescence table cutoff can hand
  it, since each child's depth-0 node asks the table for depth 0 and a
  `TT_DEPTH_QS` entry does not answer that. Then every budget from 1 to that
  tree's count, the table prepared again for each, asserting explored ==
  budget: **red under the parent's order at budget 3, explored 4** (NB01
  applied by hand, then by `tools/mutation_check.py`). Then startpos at depth
  2, every budget to its 302, as a supplement that claims no cutoff: **red on
  the untouched tree at budget 100, explored 101**, the first of 21 (the
  scratch scan, `scan_search_parent.txt`, reads the same 21 and 143 of 4096
  at depth 12; 0 of 4096 at both depths after). Release 0.04 s.
- The two existing cases, "the node budget is never exceeded" and "an aborted
  search reports no move at all", green before and after (neither budget list
  lands on a cutoff on this tree; on S261's vector 4096 did).

S261's probe (`.tuning/coord/S261/nodes_over.py`, unchanged) on the parent:
**30 of 1028 over**; on the fix: **0 of 1028**. The copy that prints every
over budget and its size, `nodes_over_all.py`: 30, worst +3, then 0, worst +0.

### INV-6

- `chesso bench`: **3872032** on both, and all 121 lines of its output
  identical with `time` and `nps` stripped (eight `bestmove` lines, every
  `info` line's `nodes`, `score`, `hashfull` and `pv`).
- `tools/search_bench.py` depth 9: 32829 / 69892 / 25174, c3d5 / e2a6 /
  d7c8q, identical; depth 13: 106091 / 323668 / 148755, c3d5 / d5e6 /
  d7c8q, identical.
- What moves: only a search stopped by its budget or by the flag. Over the
  probe's 1028 budgets (`budget_diff.py`, both binaries), **the 30 over-budget
  searches move, each from N + k to exactly N, and no `bestmove` line moves**;
  the other 998 are identical in reported nodes and `bestmove`. Unbounded
  depth-2 and depth-12 trees from startpos keep their counts (302, 496266).

### Goldens

- `adocs/data/S203_case_sweep.sh --at`, parent and fix: **identical**, A 9/0,
  B 20/0, C 12/0, D 3/0, E 0/0, F 10/0 (mates/short), five of six reporting
  against the majority of four; no short line, so no ceiling's trigger fires
  and nothing is re-derived (`case_sweep_at_parent.txt`, `_fixed.txt`). E's
  0 is the parent's too.
- Other `go nodes` counts in `tests/`: none pinned. `tests/test_engine.cpp`
  "node-limited searches repeat across ucinewgame" compares two runs; "ordering
  keeps the tree small" finishes inside its 65024 budget (`REQUIRE_FALSE
  aborted`); the depth-driven goldens move with INV-6, not at all.

### Second tier, DEC-141

- Fast suite 46 of 46 in `build` and in `build-tune`, `CLANG_FORMAT_MAJOR=22`,
  `./clang-format.sh --check` clean; taken again after the fast check's fixes,
  46 of 46 in both (`fast_build_r2.log`, `fast_build_tune_r2.log`), `bench`
  3872032.
- Debug self-play, DEV_MANUAL's recipe, 4 rounds at 4+0.04, 8 games: **0
  `Assertion`** in the trace log and the output, 0 `disconnect`, 0 forfeits.
- `tools/mutation_check.py`, NB01 (`tools/mutants/S279_qs_node_budget.py`,
  the check moved back below the probe, the parent's order), on fixture
  `bb4dbc9` (`bcd5786` plus this tree through a temporary index and `git
  commit-tree`, no ref moved; the worktree under this worktree's `.ref-builds/`,
  removed after): baseline green 46 tests, bench 3872032; **NB01 killed**,
  fast 1 of 46, bench same, both new cases red (`3 <= 2`, `101 == 100`).
  Wall 260 s (`mutation/`). After the fast check's fixes, `--only NB01` on
  fixture `597a59b` (the same procedure; its anchor moved with the comment):
  baseline green 46 tests, bench 3872032; **NB01 killed**, fast 1 of 46,
  bench same, both cases red (`3 <= 2`; `prepared tree, budget 3 explored
  4`). Wall 252 s (`mutation_r2/`); worktree removed after.
- `tools/gate_extra.sh` on this tree, `CLANG_FORMAT_MAJOR=22`:
  **GATE-EXTRA-DONE, 5 stages, 932 s** -- prose, citations, debug (333 s),
  sanitize (547 s) and perft (51 s) all ok (`gate_extra/`,
  `gate_extra.log`). Run before this section was written and before the
  fast check's fixes, which touch a test, a source comment and prose; the
  prose and citation checks were run again after both.

### SPRT, proposed (DEC-265 form); ruled none, DEC-279

**Why the question.** The change alters play in two places only, and
neither is a fixed-depth search. (a) A search ended by a `go nodes` budget
stops on the budget's node instead of past it (by up to three as measured);
nothing the SPRT harness sends uses `go nodes`, but `tools/datagen.cpp`
(5000 nodes a move by default) and the S170 replay do. (b) Under a clock
the hard timer raises the stop flag; a poll that landed on a quiescence
table cutoff was skipped, so in about 4 % of stops the flag was read 0.4 to
1.2 ms later. The soft limit is decided between iterations and does not
read the flag.

**Precedent.** S163 (the stale timer's stop store): no SPRT, "bench INV-6-
identical, since the timer path is outside fixed-depth benching". S210's F20
and F21 (`movestogo 0`; the first iteration honouring `stop` and the hard
timer): "alter play only under inputs no harness here sends", discharged
node-identical at fixed depth in `No functional change` commits. Against
them, S210's F22 (quiescence's dead-board draw): reach counted first, 1.69 %
of root answers moved, so DEC-107's census discharge did not apply and one
`--nonreg {-5, 0}` SPRT was owed (H1, Elo 0.01, 28598 games, 13 h 16 m).
Here the reach at fixed depth is zero by INV-6, and at node budgets 30 of 1028
probe searches move in nodes and 0 in `bestmove`.

**Options.**

- **No SPRT, discharged by INV-6 and the reach above (recommended).** Costs
  nothing. Risks an unmeasured effect on timed play bounded by a stop read
  up to 6144 nodes earlier in about 4 % of hard stops, below the timer's own
  scheduling jitter, and the same direction as the hard limit's intent.
  Leaves unmeasured exactly that, which a match could not resolve anyway.
- **One `--nonreg {-5, 0}` SPRT, S210 F22's form.** Costs a night (F22's took
  13 h at 2155 games an hour). Measures a change whose expected effect is
  zero to within any reachable resolution, so its likely outcome is an H1
  at zero that says only "not a 5 nElo regression".

Recommendation: no SPRT. The fix restores a contract `check_limits` already
claimed ("can be enforced exactly"); what it changes in timed play is the
latency of the flag, not a decision any search makes.

### Proposed for `adocs/specs.md`; the coordinator writes it

> `go nodes N` stops on its Nth node, never past it: every node checks the
> limits as soon as it is counted, quiescence's table cutoff included, and
> the stop flag is polled at every count divisible by 2048. Since
> 2026-10-14, S279; before it a quiescence table cutoff went unchecked, so a
> budget could be overrun (by up to three nodes on 30 of 1028 probe budgets)
> and 4.45 % of first polls were missed.

### Proposed `done:` stamp

> 2026-10-14. `quiescence` checks the limits right after its count, above
> the table probe, and the check below it goes: a table cutoff that spent a
> `go nodes` budget or landed on the stop flag's 2048-node poll went
> unchecked. Red first: a three-node quiescence on a hand-prepared table
> (budget 2 explored 3, no abort), and a sweep of every budget through
> `search` on that table at depth 1, its precondition asserted (3 explored 4
> under the old order), with startpos at depth 2 as a supplement (100
> explored 101). S261's probe 30 of 1028 over (up to +3) -> 0;
> stop flag set before the search noticed late in 6724 of 150932 book
> positions -> 0. INV-6: `bench` 3872032 and `search_bench` depths 9 and 13
> node-identical with the same best moves; the 30 over-budget probe searches
> move to exactly N, no `bestmove` moves. S203 `--at` identical, no golden
> re-derived. Fast suite 46/46 both builds, format clean; Debug self-play 8
> games, 0 `Assertion`; NB01 killed; `gate_extra.sh` five stages green. The
> fast check's four findings fixed: the sweep's run-derived floor replaced by
> an asserted precondition, DEV_MANUAL's measured overrun, the two readers of
> an aborted quiescence's 0 named, the test states on the heap. SPRT: none,
> discharged by INV-6 and the counted reach (DEC-279). Implemented by an Opus
> subagent briefed by the coordinator.

### Proposed commit

```
Check quiescence's limits before its table probe

S279. quiescence() counted its node, probed the table and returned a
hit before check_limits() ran, so a table cutoff went unchecked. When
that node spent a `go nodes` budget the search ran on to the next
checked node: S261's probe read N + 1 to N + 3 on 30 of 1028 budgets.
When it landed on the stop flag's 2048-node poll the poll was skipped:
with the flag set before the search, 6724 of 150932 book positions
read it 2048 to 6144 nodes late.

check_limits() now runs right after the count, as in negamax_at(), and
the check below the probe, at the same count, goes. Two cases in
test_search, red first: a three-node quiescence on a prepared table
(budget 2 explored 3), and every budget through search() on that table
at depth 1 (3 explored 4) and of startpos at depth 2 (100 explored 101).
Mutant NB01 restores the old order; both cases kill it.

INV-6: bench and search_bench at depths 9 and 13 are node-identical
with the same best moves. Only a search its budget or the flag stops
moves: the 30 over-budget probe searches end on exactly N, and no
bestmove line moves.

Bench: 3872032
```

`Bench: 3872032` and not `No functional change`, as ruled after the fast
check: `tools/gate.sh` accepts either at an unchanged total, and the change
does alter what a budgeted or stopped search does.
