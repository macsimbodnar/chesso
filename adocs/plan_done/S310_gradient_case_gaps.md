id:         S310
goal:       `test_tuner_gradient`'s clamp block names S101's cells, its AdaGrad hold reads a margin and not a chance, and it runs from any directory
accepts:    the clamp case asserts every stage-two weight that the clamp silences, S101's cells among them, whichever family is live, its failure seen on a planted cell; the AdaGrad hold case's step is chosen by a stated rule that keeps its reading clear of its bound on the shipped constants, with the reading across epochs shown; the binary run from the repository root and from `build/tests` both read the same tally; no bound moves and no plant is dropped; both builds green
touches:    tests/test_tuner_gradient.cpp, tests/test_helpers.hpp, tests/CMakeLists.txt, adocs/data/S083_tuner_preconditions.cpp, adocs/data/S310_adagrad_step.cpp, adocs/data/S310_adagrad_step.txt, DEV_MANUAL.md
excludes:   any change to the tuner; the case's clock, which is S309's
decisions:  DEC-171, DEC-316, DEC-334
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-02 -- the three gaps closed, `accepts` by clause. The clamp case derives its block from `tuner_trace::clamped_site` over the feature table and binds every row alone on each side of the clamp, so all 198 stage-two halves, S101's 48 among them, are asserted silent on a row that reads them, the family live or zeroed; planted one at a time from a scratch header, never in the tree, the parent's case read 12 of 198 red and 2 of S101's 48, this one 198 and 48, and a lower clamp branch that sets no flag reads 103 rows red at `side := -1` where the upward-only sweep was silent; the case holds 5137 assertions, 220 before. AdaGrad's step is 2 and not 10, `ADAGRAD_HOLD_STEP`, by `adocs/data/S310_adagrad_step.cpp`'s rule: the largest of 14 steps at most half the smallest at which any of 98 starts passes 1e-9 through 1200 epochs, that onset 5, the walk and travel clear; `apart` 2.84e-12 at epoch 300 in Release and tune and 2.61e-12 in Debug, at most 6.68e-11 through 1200 and 1.24e-10 over the family, against the unmoved 1e-6, and the table by epoch reads 10 chaotic by epoch 900 on the shipped constants. The rule was written after a first sweep, and its first full run exited 2 against my guess of 3; the rule stood. A broken hold reads red at 2: the absorb line removed, 2 assertions, `apart` 26.98 and 5.73; each of the 7 gauges skipped, 2 each. The cost is stated: the red threshold on an absorb error moves 5.28e-9 to 3.955e-8, 7.49x, on a level skew 8.44e-8 to 4.924e-7, 5.83x, and the family's smallest walk 0.0853 to 0.0295. `ADAGRAD_HOLD_STEP` and `CLAMP_PUSH` are `GOLDEN (DEC-142)` and in the table, and both scripts read them from the test's source, each disagreement observed rc 2. The binary reads 15 of 15 and 67880 assertions from the repository root and from `build/tests`, against 10 of 15 and 15871 from the root before, all 5 `load_json`'s relative path; the fix is in `tests/test_helpers.hpp` by `CHESSO_SOURCE_DIR`, the main entry stays on the root as the guard, observed red first, `test_search` follows and six other binaries stay relative. No bound, plant, budget or clock moved, nothing under `src/` or `tools/`. Both serial fast suites 53 of 53 with `CLANG_FORMAT_MAJOR=22`, format rc 0, the prose checks rc 0. S309's bar of 30: `threats` 19.57 s at `-j12` on the final file, 20.85 the most of any run, 11.35 serial; Debug 51.44 against 600. Tier 2 (DEC-141) does not bind: no `make_move`, generator or search change, no pruning rule. `DEV_MANUAL.md` takes the directory rule, `test_uci_surface`'s line, the entries' tally, the gauge paragraph and two golden rows; `MANUAL.md` checked, no change. Unmeasured: `apart` on Apple silicon. Filler (DEC-171): no file under `src/` changed, no score, move or line moved.

## Why

Found 2026-11-01 by S101's phase 2, on its paste, none reachable in play, so filler by DEC-171.

- The clamp case's block stops at king safety, so S101's cells are covered on a bound row only by the
  finite difference.
- AdaGrad at step 10 goes chaotic late on the paste: `apart` read 5e-9 at epoch 250 and 5e-6 at 300. S101's
  fixture pair moved it to 7.39e-12, which is a reading and not a margin, and a later fit can bring it back.
- Three of the 13 cases fail when the binary runs from the repository root and pass from `build/tests`, where
  ctest runs it. Cause not investigated.
  [2026-11-02, S309: the binary now holds 15 cases; how many of the 15 fail from the root is unmeasured, the three are of 13.]

Depends on S101's verdict, since which cells are live decides what the clamp case can plant.

## Done, 2026-11-02

All on S101's constants, `30d22cd`. `touches:` widened: gap 3's cause is in the shared `tests/test_helpers.hpp`, and
`adocs/data/S083_tuner_preconditions.cpp` runs this case's own block and step, so it follows both. Logs, plant headers and
the sweeps named below are in the main tree's `.tuning/coord/S310/`, gitignored; `fc/plants.diff` is every plant, each off
unless its environment variable is set, and no file of the tree ever held one.

### Gap 1: the clamp block, wider than found

The hand list was half of it. At the +30 push two rows of the 106 bind, and they read 12 of the block's 198 parameters
and 2 of S101's 48, `hanging_piece[2]`'s two halves. A weight a row does not read is zero there whatever the clamp does,
so a block derived from the site table alone would still have been free for 46 of the 48.

So the case now does two things. The block comes from `tuner_trace::clamped_site` over the feature table, with a floor
of the families shipped there today, and no list a later family can miss. And every row is bound alone, once on each side
of the clamp: only the clamped weights it reads move, each the way its coefficient counts, until they move its sum by
twice the margin, up and then down. On each side every weight of the block must read a gradient of exactly 0 and the rest
above 0, and the score must not move at all when a weight it reads moves by 0.005. A weight counts as silenced only on a
row that gives it a gradient at the shipped constants, where no row binds, and all 198 must be silenced on one. 103 rows
take part; 3 have no stage-two entry. The finite difference, +30 and its preconditions are unchanged. The case goes from
220 assertions to 5137, the binary from 62963 to 67880, the difference being exactly those 4917.

**The plant** is one parameter's gradient let through on bound rows, each of the 198 halves in turn, reading the case's
red and which assertions went. The second side could only add assertions, so it could not lower a red count; this table
was read again on the final file all the same, and did not move.

| the case | red | mobility + king safety | threats | `check.worst` | `clamped_block` | row sweep |
|---|---|---|---|---|---|---|
| parent | 12 of 198 | 10 of 150 | 2 of 48 | 12 | 10 | none |
| this step | 198 of 198 | 150 of 150 | 48 of 48 | 12 | 12 | 198 |
| this step, the sweep's start with all 48 threat weights zeroed | 198 of 198 | 150 of 150 | 48 of 48 | 12 | 12 | 198 |

The parent's two red threat plants went by the finite difference alone, as the step said; its other 46 were green, which
it did not say. Example, `threat_piece_eg[4]`, parameter 1090, green on the parent: `CHECK( 0.000130467 == 0 )` with
`loudest_param := 1090` and `clamped_block := 0`, from the first form of this case. Unplanted the third row reads 5137 of
5137, which is what "whichever family is live" rests on.

**Each side, because the clamp has two branches.** The sweep first bound each row upward only. A plant in `smooth()` whose
lower branch clamps without setting `clamp_binds` read red at +30's rows alone, `clamped_block` and `check.worst`, one of
each, and the sweep was silent; the fast check found that, reading the block alone, and the negative branch rested on two
rows of one re-derived push. Observed before the change (`flag_lower.before.log`), then with both sides
(`flag_lower.after.log`): 103 of `CHECK( loudest == 0.0 )` red, the first `side := -1`, `loudest_param := 776`,
`0.000467907`, beside the same two. The same plant on the upper branch read 103 either way.

**The score's own assertions, by what the plant is.** The first draft of this section said "`threat_piece_mg[4]` summed
outside the clamp reads 28 of the score assertions red and the finite difference red". That was true of a plant it did not
name, and the fast check's plant, which moved the half, read otherwise. All three at one cell, `threat_piece[4]`,
parameters 1085 and 1090, on every row (`S310_SMOOTH` in `fc/plants.diff`, `fc/smooth_*.log`):

| in `smooth()` | score assertions red, one side | both sides | finite difference |
|---|---|---|---|
| `doubled`: both halves summed outside the clamp and left inside it, the first draft's | 28 | 56 | red |
| `moved_mg`: the mg half on the positional site and not its own, the fast check's | 14 | 28 | green |
| `moved_both`: both halves so | 28 | 56 | green |

Why the difference reads differently is reasoned, not traced: `gradient()` still gives the doubled cell one share on every
row, and gives a moved cell its true derivative off the bound rows, which the +30 rows do not read this cell on. And
`threat_pawn` declared on `SITE_POSITIONAL` reads 8 of the floor's assertions red and `load()`'s mismatch count.

### Gap 2: the AdaGrad step

**The rule is `adocs/data/S310_adagrad_step.cpp`'s header**, and `ADAGRAD_HOLD_STEP` the golden it answers, 2. Fourteen
steps from 1 to 30. 98 starts: the case's fixture and the same less its last pair, each at the shipped constants and at
16 draws of every parameter moved by up to 1, 3 and 10 centipawns. `apart` every 25 epochs through 1200, four times the
case's. The onset is the smallest step past 1e-9 at any read of any start, three decades under the bound. The step is the
largest at most half of that with the shipped walk at 10 times its floor and the travel at twice its. The bound, 1e-6,
did not move.

**Written after a first sweep had been read, so it predicted nothing, and it corrected me once:** the first sweep
suggested 3 and I set the script to that. The first full run exited 2, onset 5, half 2.5, answer 2, because the
less-a-pair fixture at 10 cp is harsher than anything that sweep read. The rule stayed and the case took 2. 3 and 4 read
quiet too.

`apart` by epoch, the shipped constants on the case's 110 rows, and the family (the whole table and the 108 rows' are in
`adocs/data/S310_adagrad_step.txt`):

| step | 100 | 300 | 500 | 800 | 900 | 1000 | 1200 | worst of 98 | past 1e-9 | walk at 300 | travel | smallest walk of 98 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 7e-13 | 1e-12 | 1e-12 | 2e-12 | 1e-12 | 1e-12 | 1e-12 | 7.00e-11 | 0 | 0.5208 | 1158.1 | 0.0123 |
| **2** | 1e-12 | **3e-12** | 3e-11 | 4e-12 | 3e-12 | 3e-12 | 3e-12 | 1.24e-10 | 0 | 0.7369 | 1425.3 | 0.0295 |
| 3 | 1e-12 | 1e-12 | 1e-12 | 3e-12 | 3e-12 | 2e-12 | 2e-12 | 5.33e-11 | 0 | 0.6396 | 1567.0 | 0.0410 |
| 4 | 3e-12 | 9e-13 | 9e-13 | 1e-12 | 1e-12 | 8e-13 | 9e-13 | 3.50e-11 | 0 | 0.7929 | 1666.3 | 0.0399 |
| 5 | 2e-12 | 2e-12 | 8e-13 | 1e-12 | 2e-12 | 1e-12 | 1e-12 | 2.44e-08 | 2 | 1.0447 | 1725.4 | 0.2297 |
| 6 | 1e-12 | 1e-12 | 2e-12 | 2e-12 | 3e-12 | 2e-12 | 3e-12 | 1.64 | 19 | 1.0427 | 1754.2 | 0.4902 |
| 8 | 3e-12 | 1e-12 | 1e-12 | 8e-13 | 1e-12 | 1e-12 | 1e-12 | 2.32 | 42 | 0.7917 | 1809.1 | 0.2951 |
| 10, before | 2e-11 | 7e-12 | 5e-12 | 2e-09 | 7e-05 | 1e-02 | 6e-01 | 2.20e+02 | 78 | 0.6157 | 1854.0 | 0.0853 |
| 20 | 1e-06 | 4e+01 | 6e-01 | 6e-01 | 6e-01 | 6e-01 | 7e-01 | 1.12e+03 | 98 | 0.7903 | 2283.7 | 0.0075 |
| 30, the default | 4e-07 | 1e-07 | 2e+01 | 1e+01 | 1e+01 | 1e+01 | 1e+01 | 2.08e+03 | 97 | 2.1112 | 2536.8 | 0.0096 |

10 was a reading indeed: 7.39e-12 at 300, S101's figure reproduced, and chaotic by 900 on the shipped constants
themselves. S101's 4.96e-6 at 300 on 108 rows reproduces too. At 2 `apart` is 2.84e-12 at the case's epochs and never
past 6.68e-11 through 1200, 4.2 decades under the bound, 3.9 over the family. The members' own onsets are 10, 9, 9, 9 on
the case's fixture and 8, 6, 6, 5 less the pair, a factor of two, which is the reason for the half.

Read by the case itself, which now prints it by optimiser: Release and tune AdaGrad 2.84217e-12 and Adam 1.36424e-12,
Debug 2.6148e-12 and 9.09495e-13. Adam's step is not this step's and reads quiet at every read through 1200.

**A broken hold, observed at 2.** The fast check's point: `accepts` forbids a weakened test and this step had never seen
the case red on the thing it guards at the new step. In a scratch copy of `hold_gauges`, the full case, both optimisers
(`fc/hold_sweep.py`, `fc/hold_sweep.txt`):

| `hold_gauges` | at 2 | at 10 |
|---|---|---|
| `params[gauge.absorb] += drift` removed | red, 2 assertions, `apart < 1e-6`: AdaGrad 26.98, Adam 5.73 | red, 2: AdaGrad 8.07, Adam 5.73 |
| each of the 7 gauges skipped, in turn | red, 2 each, the held level `< 1e-9`, one per optimiser | the same, 2 each |

The fast check read the same. The hold's skip leaves `apart` at rounding, 8e-13 to 4e-12 at 2, so the level assertion is
what catches that one, and the absorb's removal is the one `apart` catches.

**What the lower step costs: the case sees a small error in the hold less.** The smallest error per hold that reads red, by
bisection, 26 halvings in log space, re-derived here and agreeing with the fast check's 5.3e-9, 3.95e-8, 8.4e-8 and 4.9e-7:

| the error | at 10 | at 2 | cost | what reads it first at 2 |
|---|---|---|---|---|
| absorb: `params[absorb] += drift * (1 + e)` | 5.28e-9 | 3.955e-8 | 7.49x | AdaGrad, 1.00121e-6 at e = 3.96e-8; Adam 2.43e-7 |
| skew: `params[level[0]] -= e * drift` | 8.44e-8 | 4.924e-7 | 5.83x | Adam, 1.00131e-6 at e = 4.93e-7, which the step does not touch; AdaGrad 2.15e-7, and still 8.71e-7 at e = 2e-6 |

At 10 AdaGrad read each first. So on a skew Adam now sets the threshold, and AdaGrad alone has lost more than 5.83x. These
are the harness's figures, `-O2` and not Release's flags, whose unplanted AdaGrad reads 1.82e-12 against 2.84e-12.

**And the walk.** On the shipped constants the smallest plain walk moves from 0.6157 to 0.7369, no loss, but across the
family's starts its minimum falls from 0.0853 at 10 to 0.0295 at 2, 2.95 times the floor of 0.01. The rule reads the walk
on the shipped constants only. A later fit under the floor is reported by `S083_tuner_preconditions.cpp`'s `walk`, as
before, and the floor is not this step's to move.

**The two goldens are listed and the scripts read them.** `ADAGRAD_HOLD_STEP` and the clamp case's `CLAMP_PUSH` are marked
`GOLDEN (DEC-142)` at their sites and hold rows in `DEV_MANUAL.md`'s table. Both scripts read them from
`tests/test_tuner_gradient.cpp`, 17 lines of the same `case_constant()` in each, so S083 was no harder than the new one.
Both now run from the repository root. Observed by moving the value in a copy of the test file and running from that copy's root
(`fc/moved_step`, `fc/moved_push`):

| script | the value moved | read |
|---|---|---|
| `S310_adagrad_step` | `ADAGRAD_HOLD_STEP` 3 | rc 2, `the rule picks 2, the case carries 3` |
| `S083_tuner_preconditions clamp` | `CLAMP_PUSH` 40 | rc 2, `the rule picks +30 ... the case carries +40` |
| `S083_tuner_preconditions walk` | `ADAGRAD_HOLD_STEP` 3 | rc 0, `adagrad at lr 3`, smallest walk 0.6396: it runs what the case runs, so there is nothing left to disagree |
| either, from a directory with no such file | | rc 2, `no ADAGRAD_HOLD_STEP in tests/test_tuner_gradient.cpp: run from the repository root` |

At the tree `S310_adagrad_step` and `both` exit 0. `both` still answers +30, two rows bound, 198.7, 158.8, every row 14.67
from the kink, finite differences 1.52e-7, as `adocs/data/S101_goldens.txt` reads, and its `clamp` derives the block from
the sites. Its readings at 2 are in `adocs/data/S310_adagrad_step.txt`, every hold at most 6.66e-15 under AdaGrad.

**Still unguarded, copied by hand and compared with nothing, as both headers now say:** S083's `WALK_FLOOR`; the new
script's `CASE_BOUND`, `WALK_FLOOR`, `TRAVEL_FLOOR` and `CASE_EPOCHS`; each script's copy of the fixture. `case_constant()`
needs the text `constexpr double NAME = ` to stay and exits 2 where it does not.

To re-derive, both files' headers hold the build lines; the rule's script takes about 150 s.

### Gap 3: the directory

Observed before any change (`before_from_root.log`, `before_from_build_tests.log`):

| the binary, whole | from the repository root | from `build/tests` |
|---|---|---|
| before | 15 cases, 10 passed, 5 failed; 15871 assertions, 5 failed | 15 of 15; 62963 of 62963 |
| after, Release and tune | 15 of 15; 67880 of 67880 | 15 of 15; 67880 of 67880 |

The reviewer's inference holds. All five failures are `Cannot open assets/test_jsons/castling.json` at `load_json`'s
`REQUIRE`, the three plant cases and the two `danger_positions` ones, opened relative to the working directory. After the
fix a third directory, the session's scratch, reads 15 of 15 and 67880 too, Release.

The fix is `load_json` reading from the source tree when the binary has `CHESSO_SOURCE_DIR`, the macro the suite's other
source-tree readers already use, and exactly as before when it does not, which keeps hand-built scripts such as
`adocs/data/S135_gradient_rows.cpp` unchanged. `tests/CMakeLists.txt` gives it to this binary. `tests/assets/test_jsons`
and `build/tests/assets/test_jsons` read identical under `diff -r` before the change, which is why the count does not move.

The failing test first, per TESTS: `WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}` on the main entry, observed red before the
fix, the two `danger_positions` cases with `Cannot open`, the plant entries green (`gap3_guard_red.log`). That property
stays, so the suite itself reads the binary from both directories, with no new entry and no new time.

Two side effects. `test_search` already had the macro, so it follows: it reads 223 of 223 from the root and from the
scratch directory. And six other binaries stay as they were, read from the root and not touched: `test_eval_trace` 6 of
10 red, `test_invariants` 1 of 2, `test_leaf_walk` 4 of 8, `test_eval_model` 4 of 21, `test_evaluation` 17 of 52,
`test_movegen` 5 of 22, every red `Cannot open`. The coordinator takes those as their own step.

### Readings

ctest's own seconds, against Release's clock of 60 (S309's bar 30) and Debug's 600. Governor `performance`, one-minute
load 0.80 to 0.90 before each run on the final file, except Debug's, 3.99.

| build, reading, the final file | main | base | pawns | threats | suite wall |
|---|---|---|---|---|---|
| Release, -j12 suite | 1.87 | 4.44 | 10.00 | 19.57 | 29.70 |
| Release, serial suite | 1.27 | 2.96 | 7.54 | 11.35 | 131.94 |
| tune, serial suite | 1.27 | 2.96 | 7.50 | 11.34 | 133.15 |
| Debug, serial ctest of the four, `-R '^test_tuner_gradient'` | 5.48 | 13.57 | 34.88 | 51.44 | 105.38 |

Before the second side and the goldens, the same work on the plant entries: Release at -j12 three runs and one, `threats`
19.72 20.85 19.29 19.61, tune 19.79, walls 29.55 to 30.16. The largest of any reading is that 20.85, 9.15 s under S309's
bar and over S309's own worst of 19.57. This step adds nothing to that entry's work: the plant cases' code is untouched
and the JSON files are the same bytes. S309 records who overlaps moving these figures by a second or more; that is the
likely reading of the 20.85, not traced. The second side doubled the sweep's assertions and the main entry reads no
slower than noise, 1.27 serial against 1.26 at S309. Debug clears 300 by 248.56 s.

### Checks

Both builds no warning. Serial fast suites with `CLANG_FORMAT_MAJOR=22`: 53 of 53 each, as is the -j12 run above and every
one before it. `./clang-format.sh --check` rc 0. `tools/plan_prose_check.py` rc 0, with this file as it stands. No file
under `src/` or `tools/` changed, no plant, budget or bound moved: 16 plants and 132000 epochs either side, and the bound
is still 1e-6, the walk floor 0.01 and the travel floor 50. Tier 2 (DEC-141) does not bind: no `make_move`, generator or
search change, no pruning rule.

`DEV_MANUAL.md`, by a re-grep for the step and the directory rule: its rule for running test binaries from `tests/`; that
`test_uci_surface` is no longer "the one test binary" that need not, read 16 of 16 from the root and from the scratch
directory; how the entries read the tally; the gauge paragraph's "the case's step of 10", now past tense with step 2's
figures; two rows in the golden table. `MANUAL.md` checked, no change, no UCI surface moved. `DEV_MANUAL.md`'s line on
running `test_perft` from `build/tests` and its `cd tests` recipes for `test_search` still hold.

Unmeasured: `apart` on Apple silicon, the other machine, whose rounding differs, and at any constants other than these
and the draws. Inferred: that the draws stand for a later fit; they are the rule's assumption, and the script is run again
when constants move for that reason. Seen and left: this case's `CAPTURE(tuner_optimiser::kind_name(kind))` prints an
address and not the name, which is older than this step; the new `MESSAGE` names the optimiser. `tests/test_tuner_gradient.cpp`'s
"2764 positions" comments are S319's row and were left.
