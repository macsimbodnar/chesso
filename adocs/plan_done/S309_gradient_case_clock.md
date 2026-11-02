id:         S309
goal:       `test_tuner_gradient` stops running close to its 60 s clock, so a planted recovery added by a later family cannot turn it red on time alone
accepts:    the case's time in the fast suite reads under half its clock on the workstation under the suite's own load, with how it was measured; no bound moves and no plant is dropped or weakened (a split into separate ctest entries or a shared fit is the expected form, and the step says which); both builds green, Debug's clock read too; a plant still goes red on what it exists to catch, observed
touches:    tests/test_tuner_gradient.cpp, tests/CMakeLists.txt, DEV_MANUAL.md, adocs/plan_todo/S310_gradient_case_gaps.md
excludes:   any change to the tuner; raising the clock, which hides what this step exists to fix
decisions:  DEC-171, DEC-316
closes:
blocks:
paused_by:
author:     Opus 5.5 subagents, coordinated: one wrote it, a second finished the readings
done:       2026-11-02 -- the form is separate ctest entries by family, one binary by doctest filter: `test_tuner_gradient` and `test_tuner_gradient_plants_` `base`, `pawns`, `threats`, 12 + 1 + 1 + 1 = 15 cases with none in two entries, read by `--list-test-cases`; no shared fit, which was reasoned against and not run. Every entry declares `PROCESSORS 4`, the literal 4 `fit()` passes to `tuner_trace::gradient()`, found by this step's fast check. Before, one entry against Release's 60: 22.76 s alone, 22.80 serial, 35.34 at `-j12 -L fast`, over the bar of 30. After, the largest entry, `threats`: 11.20 serial, and at `-j12` 18.93, 18.84, 18.79 settled and 19.57 unsettled, tune 11.16, 18.37 and 18.79; `accepts` holds by 10.43 s at the worst, every entry of every reading in this file's table. The split alone read 28.72 to 29.59 at `-j12`, 0.41 s under the bar: that was contention, four threads counted as one slot, and declaring them cost the suite no wall, 29.65 to 30.07 against 30.94 to 31.09. How the 35.34 divides between the two changes is unmeasured, `PROCESSORS` on the parent not read. A later family's budget counts on its own clock and serially on no other; under `-j` it can still slow a neighbour by sharing the machine, not measured. Debug, against 600 and a bar of 300: 5.58, 13.25, 33.84, 49.62 s, against 100.40 for the one entry, read before `PROCESSORS` and not again because the property changes nothing without `-j`, the serial figures moving 0.16 s at most. No bound, budget, plant or clock moved: 16 plants and 132000 epochs either side, the test file's lines sorted against the parent's differ in structure and one case name. Both serial fast suites 53 of 53 with `CLANG_FORMAT_MAJOR=22`, as were all ten `-j12` runs of 2026-11-02; format rc 0, the five prose checks rc 0. A plant observed red after the split, by a temporary line since removed: the gradient of `TKG_EG_BASE + 3` zeroed in `fit()`, `threats` red, group `threat_king`, 2 of 13784 assertions, `base` and `pawns` green; a word with no case red by its regex. Tier 2 (DEC-141) does not bind: no `make_move`, generator or search change, no pruning rule. `DEV_MANUAL.md` takes the split and the slots; `MANUAL.md` checked, no change. Filler (DEC-171): no file under `src/` or `tools/` changed, no score, move or line moved.

## Why

Found 2026-11-01 by S101's fast check. The case took 33.2 s in the suite's log and 23.1 s alone on the
idle machine, against 11.6 s in S125-era logs and a Release `TIMEOUT` of 60. S101's plants are 68000 of
its 132000 epochs. S102 and S122 add families and plants, and a suite on a shared machine has less room.
Green today, so filler by DEC-171; it should land before S102's plants.

## Done, 2026-11-01 and 2026-11-02

The code and the first readings are 2026-11-01, when the workstation went down with Debug's clock and both serial
suites unread. Those and the further `-j12` readings are 2026-11-02. The step's fast check then found the slots below
undeclared, and every `-j12` figure was read again with them declared.

**The form: separate ctest entries, one per family, one binary, by doctest filter.** Entries:
`test_tuner_gradient` (the 12 other cases) and `test_tuner_gradient_plants_` `base`, `pawns`, `threats`, each on the
build's `CHESSO_TEST_TIMEOUT`. The suite goes from 50 entries to 53. Why this and not a shared fit: one fit over a
family frees the family's union, so the frozen-rest assertion could no longer see a group's mask freeing a neighbour,
and the budgets would have to move. That is reasoned; a shared fit was not run.

**The slots: every entry declares `PROCESSORS 4`,** `CHESSO_FIT_THREADS` in `tests/CMakeLists.txt`. `fit()` passes a
literal 4 to `tuner_trace::gradient()`, which spawns them every epoch, so a plant entry runs four threads, and the main
entry's "at any thread count", "stuck at zero" and "adagrad" cases run four too. ctest counts an undeclared entry as one
slot. The literal stays in the test and the comment names it.

A later family: a case and a word in `tests/CMakeLists.txt`. Its plants cost its own entry, at about 165 us an epoch
serial (21.55 s alone and 21.97 s in the serial suite, over 132000), and count on its own clock. Serially that is the
whole of it: entries run one at a time, so S102's and S122's budgets cannot redden another entry by adding epochs. Under
`-j` they can slow a neighbour, since entries share the machine even with their slots declared: `threats` reads 18.8 at
`-j12` against 11.2 serial, and what a given budget costs a neighbour is not measured. Rows and entries they add do slow
every entry's epochs; a family that nears half its clock takes another word. Not forecast further: their budgets are not
known until they are fitted.

Exactly once, by construction and read: the main entry's exclusions and the filters come from one list. Listing each
entry's selection reads 12 + 1 + 1 + 1 = 15 cases, and the union is the whole binary's list with no name twice. A case
with no word stays in the main entry. A word with no case selects nothing and doctest exits 0, so
`FAIL_REGULAR_EXPRESSION` turns that red, observed below.

### Readings, workstation, seconds against a Release clock of 60 (bar 30) and Debug's 600 (bar 300)

ctest's own time. Serial is how the gate runs; `-j12 -L fast` shares the machine and is what explains the 33.2 s the
step opened on: at the parent the same run reads 35.34, over the bar. Which tests overlap decides a parallel figure.

Before, the parent, one entry:

| build | alone x3 | serial suite, wall | -j12 suite, wall |
|---|---|---|---|
| Release | 22.81 22.76 22.73 | 22.80, 139.97 | 35.34, 35.35 |
| tune | 22.96 22.71 22.90 | 22.68, 141.34 | not read |
| Debug | 100.40, once | not read | stopped unread |

The split with `PROCESSORS 4`, what the step lands:

| build, reading | main | base | pawns | threats | suite wall |
|---|---|---|---|---|---|
| Release, -j12 suite, three runs | 1.57 1.47 1.41 | 5.25 5.28 5.48 | 12.64 12.59 12.58 | 18.93 18.84 18.79 | 30.07 29.86 29.65 |
| Release, -j12 suite, the last green check, load unsettled | 1.88 | 4.43 | 9.95 | **19.57** | 29.84 |
| tune, -j12 suite | 1.63 | 4.71 | 12.65 | 18.37 | 30.81 |
| tune, -j12 suite, the last green check, load unsettled | 1.46 | 5.24 | 12.33 | 18.79 | 30.20 |
| Release, serial suite | 1.26 | 2.95 | 7.50 | 11.20 | 132.09 |
| tune, serial suite | 1.28 | 2.96 | 7.50 | 11.16 | 132.92 |

The split before `PROCESSORS`:

| build, reading | main | base | pawns | threats | suite wall |
|---|---|---|---|---|---|
| Release, alone in a serial ctest | 1.26 | 2.95 | 7.48 | 11.12 | |
| Release, serial suite | 1.31 | 2.98 | 7.63 | 11.36 | 134.23 |
| Release, -j12 suite, 2026-11-01 | 2.92 | 9.48 | 24.50 | 28.72 | 30.50 |
| Release, -j12 suite, 2026-11-02, three runs | 3.93 3.65 4.27 | 13.06 13.95 14.55 | 25.40 25.47 25.17 | 29.36 29.59 29.39 | 30.98 31.09 30.94 |
| tune, serial suite | 1.29 | 3.02 | 7.59 | 11.26 | 135.33 |
| tune, -j12 suite, 2026-11-01, entries started last | 4.62 | 5.76 | 12.64 | 16.55 | 30.52 |
| tune, -j12 suite, 2026-11-02 | 3.50 | 14.15 | 25.19 | 29.05 | 31.13 |
| Debug, serial ctest of the four, `-R '^test_tuner_gradient'` | 5.58 | 13.25 | 33.84 | 49.62 | 102.30, the four |

Every reading of every entry is under its bar, either side of `PROCESSORS`. With it the largest clears 30 by 10.43 s at
`-j12` and by 18.80 s serial. Debug's clears 300 by 250.38 s; it was read before `PROCESSORS` and not again, since the
property changes nothing without `-j`: Release's and tune's serial figures moved 0.16 s at most across it.

What each change bought under load. The split alone took the largest `-j12` figure from 35.34 to 29.59 at worst, where
alone it went from 22.76 to 11.12. That gap was contention and not the split: same binary, four threads declared,
`threats` reads 18.79 to 18.93 against 29.36 to 29.59, `pawns` 12.58 to 12.64 against 25.17 to 25.47, `base` 5.25 to
5.48 against 13.06 to 14.55, the main entry 1.41 to 1.57 against 3.65 to 4.27, Release's three settled runs each way.
The suite's wall did not pay for it, 29.65 to 30.07 against 30.94 to 31.09, and is now `test_mate_breadth`'s, 29.65 of
29.65 in the third run. What remains, about 18.8 against 11.2 serial, is the machine shared at all 12 slots on 6
physical cores; not broken down further. The unsettled run reorders them: `pawns` started late there and read 9.95, and
`threats` 19.57, so who overlaps still moves these figures by a second or more.
`PROCESSORS` on the parent's one entry was not read, so how the 35.34 divides between the two changes is unmeasured.

Tune's 16.55 is no second size for this work. Its log starts the three plant entries last (its lines 92 to 96 of 121),
the other parallel logs before `PROCESSORS` start them among the first, and that run was the tune tree's first with
entries ctest had no cost on record for; that last is inferred from the order and ctest's cost ordering, not traced.
With them early the same build read 29.05, beside Release's. So a `-j12` reading of a new entry counts from its tree's
second run on.

Load: 1.2 to 2.8 before the parent's serial readings. 2026-11-02, the one-minute figure, each run after waiting for it
to fall under 1.0 except the first two serial: `PROCESSORS` runs 0.00 0.92 0.99 0.88 at `-j12` and 0.86 0.84 serial;
before it 0.85 0.86 0.90 0.92 at `-j12` and 1.8 1.5 serial; the two last green checks 1.4 and not settled. Governor
`performance`. Not read, and `accepts` does not ask
for them: tune alone after, the parent's tune at `-j12`, the parent's Debug in a suite, Debug's whole suite after.

### Counts

Before: 13 cases, 51119 assertions. After, the whole binary: 15 cases, 62963. The difference is 11844 = 2 x 5922, the
two further builds of `trace_positions()`, one build being two `REQUIRE`s over 2908 rows (2 x 106 curated and 2696 from
the JSON suites) and `mirror_fen`'s one over the 106: 2 x 2908 + 106 = 5922. Plants: 16 x (1 free_mask + 1121
parameters) = 17952, and 27245 + 17952 + 5922 = 51119, the main entry's 27245 holding the one first use of the JSON
suites. That first use is 8 assertions a process: `REQUIRE_MESSAGE` in `load_json`, `tests/test_helpers.hpp`, once per
JSON file behind the static in `all_test_fens()`, read from the source and in the entries' sum (62987 = 62963 + 3 x 8,
the entries 27245, 9296, 12662, 13784). No plant, budget or bound moved: the parent reads 16 plants over 132000 epochs
as the three families do (3 and 18000, 6 and 46000, 7 and 68000), and the test file's lines sorted against the parent's
show structure added and one case name gone, nothing else.

### The plant, observed after the change

A temporary line in the case's `fit()`, zeroing the gradient of `TKG_EG_BASE + 3` every epoch, the "nothing was pushing
them" failure, since removed: `test_tuner_gradient_plants_threats` red, group `threat_king`, 2 of 13784 assertions
(`CHECK( 52.388 < 0.5 )` at parameter 1108, `TKG_MG_BASE + 3`, and `CHECK( 21 < 0.5 )` at 1112, `TKG_EG_BASE + 3`),
`base` and `pawns` green. A word `nosuch` added to the list at the same time: its entry red, "Error regular expression
found in output". Taken 2026-11-01 and not run again, `PROCESSORS` not touching the binary; the indexes read from
`tools/tuner_layout.hpp` 2026-11-02.

### Checks

Both builds no warning. Serial fast suites with `CLANG_FORMAT_MAJOR=22` exported, with `PROCESSORS`: 53 of 53 in each.
All ten `-j12` runs of 2026-11-02 likewise 53 of 53. `./clang-format.sh --check` rc 0. The `-j12` runs of 2026-11-01
read 52 of 53, the one red `test_clang_format_script`, because that call had not exported the override. Tier 2 (DEC-141)
does not bind: no `make_move`, generator or search change, no pruning rule. `DEV_MANUAL.md` takes the split and the
slots; `MANUAL.md` needs nothing, no UCI surface moved. No list in `DEV_MANUAL.md` or `tools/gate.sh` holds the count 50,
and nothing in `DEV_MANUAL.md`, `AGENTS.md`, `tools/` or the test scripts runs `ctest -R` on this binary's name or reads
ctest's label summary, whose `sec*proc` now weighs these entries by four. No file under `src/` or `tools/` changed. A
thread-count reading of `fit()` was planned and not taken; its count, 4, is read from the source. Seen and left, no
drive-by: `tests/test_tuner_gradient.cpp` says 2764 positions above `trace_positions()` and above the nonlinear
gradient case, where this reads 2908.
