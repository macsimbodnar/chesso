id:         S021
goal:       start the root search in a narrow window around the previous score
accepts:    an SPRT with bounds matched to the expected effect size returns a verdict; and, checked before that verdict is read, the fast suite's three mate cases -- "mate in one", "mate in two is found at the right distance" and "pruning does not hide a mate against the material leader" -- are green at the window schedule that ships
touches:    src/chesso.cpp iterative_deepening_search, where the aspiration loop lives; src/search.cpp only to plumb the window through search()'s signature
excludes:
decisions:  DEC-063
closes:
blocks:
paused_by:
done:      2026-08-17. Aspiration windows ship at AspirationMinDepth 5, AspirationDelta 50, AspirationMaxDelta 400, all three in S073's parameter set.
                SPRT adocs/data/S021_sprt.sh, elo0=-5 elo1=5 under DEC-063 against 2b54a4f: H1 accepted in 00:36:41 over 824 games, Elo 35.12 +/- 19.06, nElo 44.04 +/- 23.72, LOS 99.99 %, Ptnml [29, 80, 142, 101, 60], LLR 2.97. +35.12 is NOT the effect size -- optional stopping biases it upward and one run has nothing to pool against -- so the recorded verdict is the pre-registered claim: not a regression of 5 Elo or more, sign positive. Contamination checked before the number was read (DEC-020): identical Release/-O3/compiler/CHESSO_TUNE=OFF on both sides, reference worktree clean with 0 ASPIRATION hits, candidate snapshot md5-identical to build/src/chesso.
                Schedule chosen by measurement, adocs/data/S021_aspiration_sweep.tsv: 21 schedules over 300 positions in three independent stratified samples, depth 11 through build-tune. Shipped row 0.9248 pooled, nothing else below 0.9439; the first sample alone would have shipped min_depth 4 delta 12, which is 1.0661 on the third sample and 0.9916 pooled. AspirationMaxDelta flat 100 to 2000, left at 400 unfitted.
                Plumbing proved neutral before the loop was added (INV-6): identical nodes and best moves at depth 9 and 12 on all three bench positions.
                S074's mate clause: three named cases 3 passed, 123 assertions at the shipping schedule -- and they cannot discharge the clause alone, since search() called directly never narrows a window. engine: aspiration windows drives iterative_deepening_search over two positions found by measurement where a mate first appears at depth 9 above a centipawn score, 2 passed, 69 assertions. Observed red twice with every precondition holding.
                One bug found and fixed inside the step: a root fail-high published the cutoff move against a PV row belonging to an earlier move, reachable only because S021 gave the root a window. Found by ctest --test-dir build-debug -L fast aborting on search()'s assert, which the Release gate compiles out; regression test search: windowed root is Release-visible and was observed red. The fix is play-neutral against the binary the SPRT played: 200 positions, two samples, depths 11 and 13, 0 node-count, 0 best-move and 0 pv differences, so the verdict transfers.
                Gate: ctest -L fast 13/13 in build (20.19 s) and build-tune (20.21 s), build-debug 13/13 (485.23 s) with INV-2/INV-4 and the postcondition on; clang-format.sh --check exit 0; plan_prose_check.py 0 flagged; moltke --validate clean. README.md owner-written, no change needed; MANUAL.md, DEV_MANUAL.md and specs.md updated.

## The mate clause, and why this step in particular

The three cases are `TEST_CASE_FIXTURE`s in `tests/test_search.cpp`, all under
the `fast` label that `add_doctest_target` sets (`tests/CMakeLists.txt`), so
`ctest -L fast` runs them. Follow them by title, not by line: "mate in one" and
"mate in two is found at the right distance" are in the `search: mate
detection` suite, and "pruning does not hide a mate against the material
leader" is the position S033 built for reverse futility.

DEC-060 measured that a pruning rule's mate exposure is a property of **the
bound the parent passes down**, not of the static score: the harmful prune fired
at `alpha=-965 beta=-964` because the parent was a null-window scout hunting a
mate score, so the mate-band guard on `beta` did nothing. Aspiration windows
change exactly that quantity -- the bounds every node below the root inherits.
Whether narrowing the root window moves any node into that shape is unmeasured.
The expected effect here is +9 +/- 17, a size at which an SPRT cannot separate a
missed mate from noise, so the suite has to say it rather than the match.
S074, DEC-060.

## Expected

Small. One engine reported +9 with an error bar of +/-17, which is a reported
figure and therefore direction only (DEC-019). `elo0=0 elo1=10` cannot resolve
an effect this size -- use `elo0=-5 elo1=5` or similar or the run random-walks,
as S006's did for 340 games.

**2026-08-17: the paragraph above was right and S068 paid for not reading it.**
S068 ran `fastchess.sh`'s default `elo0=0 elo1=5` on an effect near +3 Elo and
got no verdict in 6 h 36 m and 9036 games; `elo0=-5 elo1=5` returned one in
1 h 41 m on 2312 games, same two binaries. The prescription is now **DEC-063**,
where every step can find it, and it binds this one: `./fastchess.sh` with its
default bounds will not do. Write the invocation, state the expected effect and
the reading of each outcome before launching, as
`adocs/data/S068_sprt_run2.sh` does.

## What shipped

`search()` takes a window, defaulted to the full one, and the loop is in
`iterative_deepening_search` as S052 established. From `ASPIRATION_MIN_DEPTH`
the root is searched in a band around the previous iteration's score; a score
outside the band is a bound, not an answer, so the failing side alone is pushed
out and the iteration repeated. Widening doubles until `ASPIRATION_MAX_DELTA`
and then goes to the full window in one move, and a mate score ends the
schedule at once -- a band 50 centipawns wide doubling towards 48000 would pay
several full searches to arrive where one gets to now.

Three parameters, into S073's set rather than as constants here, because that
is what S073 built it for and S085 will want them:

| symbol | UCI | shipped |
|---|---|---|
| `ASPIRATION_MIN_DEPTH` | `AspirationMinDepth` | 5 |
| `ASPIRATION_DELTA` | `AspirationDelta` | 50 |
| `ASPIRATION_MAX_DELTA` | `AspirationMaxDelta` | 400 |

## The schedule was measured, and one sample would have picked another

`adocs/data/S021_aspiration_sweep.tsv` and the `.py` beside it. 21 schedules
against 300 positions -- three independent 100-position samples of
`adocs/data/S018_raw.tsv`, stratified by `game_phase()` -- at depth 11 through
the tune build, so one binary answers for every setting and no rebuild sits
between two numbers. The off row is `AspirationMinDepth` 64.

Pooled, the shipped schedule costs **0.9248** of the nodes the feature switched
off costs, and nothing else swept is below 0.9439. `AspirationMaxDelta` is flat
from 100 to 2000 and was left at 400 unfitted.

**The first sample alone would have chosen `delta` 12 at `min_depth` 4**, which
reads 0.9075 there and **1.0661** on the third -- worse than having no windows
at all. Pooled it is 0.9916, the fourth worst row in the table. Three samples
cost twelve minutes and the ranking between settings is not stable across one.

The three-position `tools/search_bench.py` disagrees with all of it, which is
the same lesson at a smaller n: at depth 12 it reads midgame +10.2 %, kiwipete
-6.8 %, tactical +63.6 %.

## The verdict

`adocs/data/S021_sprt.sh`, `elo0=-5 elo1=5` under DEC-063, against `2b54a4f`:

```
Elo: 35.12 +/- 19.06, nElo: 44.04 +/- 23.72
Games: 824, Wins: 293, Losses: 210, Draws: 321, Points: 453.5 (55.04 %)
Ptnml(0-2): [29, 80, 142, 101, 60], LOS: 99.99 %
LLR: 2.97 (100.8%) (-2.94, 2.94) [-5.00, 5.00]
SPRT ([-5.00, 5.00]) completed - H1 was accepted
```

36 m 41 s. **+35.12 is not the effect size**: an SPRT stops when the evidence
crosses a bound, so it stops early exactly when the observed effect has run
favourable, and one run has nothing to pool the bias away with. What the run
establishes is its pre-registered claim, not a regression of 5 Elo or more, and
that the sign is positive at `LOS: 99.99 %`.

The expectation written into this file before the run was +9 +/- 17. The
outcome is outside that bar, which is DEC-019 again from the other direction:
a reported figure sized the bounds correctly and predicted the magnitude
badly.

A number this large off a small diff is also the shape of a contaminated match
(DEC-020), so the two binaries were compared before the verdict was read rather
than after. Both `CMakeCache.txt`s: `Release`, `-O3 -DNDEBUG`, `/usr/bin/c++`,
`CHESSO_TUNE:BOOL=OFF`. `.ref-builds/2b54a4f` clean at that commit with
`grep -c ASPIRATION src/search_params.hpp` = 0. The candidate snapshot
md5-identical to `build/src/chesso`.

## The bug this found, and why the Release gate could not

Fixed inside the step, trivial and in scope. `ctest --test-dir build-debug -L
fast` aborted `test_engine` on search()'s own postcondition:

```
PVMISMATCH depth=5 alpha=-32 beta=68 score=68 aborted=0 best=b1c3 pv0=d2d4 pvlen=5
```

A root fail-high. The cutoff `break` in `negamax()` jumps out before the block
that maintains the principal variation, so the tail `if (ply == 0) {
state->best_move = best_move; }` published the move that caused the cutoff
while the PV row still described whatever earlier move last beat alpha. **It is
this step's bug and not an old one**: with `beta = MAX` at the root, `score >=
beta` cannot happen there, so aspiration windows are what made it reachable.

Nothing reached a GUI. The aspiration loop discards a failed-high result and
re-searches, and the loop cannot exit while the window is still narrow and the
score still outside it, so the shape is transient by construction. That is why
the fix is measured neutral rather than argued neutral -- and it is why the
test pins `search()`'s postcondition rather than any UCI output, since a test
written against `bestmove` and `pv` passes with the bug in.

The Release gate could not see it. The assertion is `assert()`, `ctest -L fast`
runs the Release build, and `-DNDEBUG` compiles it out. So the regression test
is a Release-visible case, `search: windowed root`, driving the engine's own
loop -- one `search_state_t` and one warm table across deepening iterations,
window around the previous score. A single windowed search cannot produce the
shape: ordering searches the best root move first, so it causes the cutoff
before anything has beaten alpha and the PV is empty. It takes ordering that is
good but not perfect, which is what a warm table across depths gives.
author:    Maksym Bodnar
