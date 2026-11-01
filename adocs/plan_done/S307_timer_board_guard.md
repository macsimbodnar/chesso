id:         S307
goal:       the timer case in `tests/test_engine.cpp` establishes that its board's first iteration really outlasts the bound, so a dead timer cannot pass on a lighter board
accepts:    the case, or one beside it, asserts its precondition from something a run produces, with the margin stated; a mutant that disarms the timer is killed by it, and a second that lightens the board fails the new assertion rather than passing, each observed; `stop_search_after_ms` is named by a mutant; the added time in the fast suite is stated and the cheaper form chosen if two would do
touches:    tests/test_engine.cpp, tools/mutants/, adocs/data/S307_matrix.py, adocs/data/S307_mutants.txt, adocs/data/README.md
excludes:   moving the 402 ms bound; any change under `src/`; the comment work S308 holds
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 21:50 -- the timer case now sends `go depth 1`, waits 403 ms, sends `stop` and requires no completed depth before its hard-timer half runs: the iteration read 36024721 nodes and 7060 to 7134 ms at `36b5659`, 17.6 times the wait, and the assertion adds no cushion, its reason in the section below. Case 21.1 to 22.0 ms -> 424.2 to 424.9 ms, +0.403 s, against +7.1 s for reading the iteration to its end, +173 s in Debug. `tools/mutants/S307_timer_board.py`: TB01 names `stop_search_after_ms` and is killed by `CHECK( elapsed_ms <= bestmove_bound_ms )`, 7128 <= 402; TB02, quiescence's horizon at 12, killed by `REQUIRE( still_running )` alone, fast 1 of 50, bench moved; a lighter `timer_fen` by hand fails the same assertion, alone and with TB01, and all of those were green on the test before this step; green after each revert, `adocs/data/S307_mutants.txt`. 50 of 50 in both builds, no warning, format clean, the five `tools/plan_prose_check.py` modes clean, 264 of 264 anchors. The bound unmoved, nothing under `src/`, bench 5482277. Tier 2 (DEC-141) does not bind: no search, `make_move`, `unmake_move` or generator code and no new rule. `tools/gate_extra.sh` not run; the Debug binary ran this case, green, 0.70 s. `MANUAL.md` and `DEV_MANUAL.md` checked, no change.

## Why

Found 2026-11-01 by S306. The case asserts `elapsed_ms <= 402` and a playable move on a board whose
first iteration takes about 6.75 s and 35516352 nodes with the timer disarmed, about 17 times the bound.
Nothing asserts that figure, and it moved from 25933707 with no step noticing. A later change that made
that iteration short would leave the case green with a dead timer. `AGENTS.md`'s TESTS rule asks a test
that a thing does not happen to establish first the precondition that would make it happen, and no
mutant names `stop_search_after_ms`.

The obvious precondition costs 6.75 s in every fast run, so the step weighs a node floor from a cheaper
reading against it. Filler by DEC-171: the shipped timer works on this board, with 1.9 to 2.1 ms over
20 runs in S306's table.

## 2026-11-01: the precondition, read and chosen (Opus 5.5 subagent, coordinated)

Base `36b5659`, bench 5482277, S101's paste in. Everything below is in `adocs/data/S307_mutants.txt`,
by `adocs/data/S307_matrix.py` where a script ran it.

### What was read

The figures above moved with S101. `go depth 1` on the board reads **36024721 nodes and 7060 to 7134
ms**, five idle runs, 17.6 times the 402 ms bound, and `bestmove` 7069 to 7143 ms from `go`. In Debug
173420 ms at 207730 nps. Beside 24 busy loops on the 12 threads, 37859 to 40021 ms, 5.4 times idle.

A cut run prints `bestmove` and no info line, so nothing reports its node count, and whether it was
cut is all it can say: `deepest_completed_depth` is -1. That is the reading the stop half has used
since S242.

How easily the iteration moves, from `MaxQsearchDepth` in the tune build: 7322 ms at the shipped 19,
2808 at 16, 668 at 13, **277 at 12**, under the bound. The eight-queens board stays at 8 to 12 ms for
every horizon from 10 up, above its 3 ms floor, so a change of this kind reaches this half.

### The form

| form | what it establishes | adds, Release | adds, Debug | what it stores |
|---|---|---|---|---|
| the iteration to its end, reading `time` | its length | 7.1 s | 173 s | a floor |
| `go nodes N`, no depth reported | more than N nodes | 0.40 s at N = 2.05 M, computed | 9.9 s, computed | N, this machine's rate times the bound |
| **`go depth 1`, then `stop` at 403 ms, no depth reported** | still running at 403 ms | **0.403 s** | **0.403 s** | nothing |

**The third, by cost and by unit.** The bound is on a clock, so a clock reading answers it on each
machine, where a node floor means the bound only at the rate it was taken at, and would be a golden with a
script (DEC-142). `go depth 1` arms no timer (`src/chesso.cpp`, the branch on `depth_given`), so a
dead timer cannot help it, and `stop` from the test's thread is the only thing that can end it short.

**What it gives up: it reads leniently on a loaded machine**, and the node floor would not. A board
reading 120 ms idle would pass here at 5.4 times. It takes that knowingly: the assertion can only be
wrong in that direction, and the next idle run reads the truth.

**The margin.** The wait is the bound plus one millisecond, the first `elapsed_ms`, truncated, that the
bound refuses. The assertion adds no cushion, and the margin is the iteration against the wait: **17.6
times**. The case goes red at that shrink. S022's early-out moved this iteration about 1.5 times; a
horizon of 12 moves it 25.

**A first form, withdrawn by a reading.** It waited 1.5 times the bound, 603 ms, on S306's 6746 and
8962 ms, 1.33 times, its comment saying that past the wait on a busy machine is still past the bound
on an idle one. It was built and it caught the same mutants, at 0.625 s for the case. The 24-loop
reading then read 5.4 times, which makes that sentence false, and a cushion that does not cover the
spread buys nothing for its 0.2 s. Covering it would cost 2.2 s every run, so the cheaper form does.

### What it costs

Case, `-d` over 8 idle runs each: 0.0211 to 0.0220 s -> **0.4242 to 0.4249 s, +0.403 s**. Debug 0.70 s.
`test_engine`'s line 8.04 -> 8.41 s, one run each. Fast suite 147.29 s after in Release, 160.79 s in the
tune build, so +0.27 % of the Release figure.

### The mutants

`tools/mutants/S307_timer_board.py`, by `tools/mutation_check.py --only TB01 --only TB02` on fixture
`9afbc2c` (this tree through a temporary index and `git commit-tree`, no ref moved, S279's procedure;
the worktree removed after): baseline green at 50 tests, bench 5482277, **2 of 2 killed**, wall 464 s.
The tool refuses a file outside `src/`, so a lighter `timer_fen` runs by hand, through the script.

| run alone, the one case | this tree | the test before S307 |
|---|---|---|
| as shipped | green, 0.425 s | green, 0.021 s |
| TB01, `stop_search_after_ms` raises nothing | **red**, `CHECK( 7128 <= 402 )`, the bound | red, `CHECK( 7054 <= 402 )` |
| TB02, quiescence's horizon at 12 | **red**, `REQUIRE( still_running )`, iteration 264 ms | **green** |
| `timer_fen` the eight-queens board | **red**, the same, iteration 9 ms | **green** |
| TB01 and TB02 | **red**, the same, 265 ms | **green** |
| TB01 and the eight-queens board | **red**, the same, 9 ms | **green** |

The last two rows of the right column are the gap S306 named, observed: a dead timer on a light board,
the old case green. Each mutated row of this tree is followed by the case green on the reverted tree.

**TB02's kill is this assertion alone**: 1 of 50, bench moved. On the old tree that mutant is a green
suite and a moved bench, a survivor by the tool's table. TB01's is the bound alone, bench same.

**Red on a healthy engine:** 50 of 50 green idle and 30 of 30 beside 24 busy loops, the case 0.424 to
0.425 s and 0.498 to 0.540 s. It needs `stop` held off for the iteration's length.

### The gate

50 of 50 in `build` and in `build-tune`, no warning in either, `./clang-format.sh --check` clean, bench
5482277, `tools/plan_prose_check.py` clean in each of its five modes. 264 anchors by the tool's own
`load_mutants` and `validate`.

**Tier 2 (DEC-141) does not bind, and `tools/gate_extra.sh` was not run.** The diff is one test case, one
mutant file and data: nothing in `make_move`, `unmake_move`, the generator or the search, and no new
pruning, reduction or extension rule. The Debug binary ran this case once, green.

**Manuals.** `MANUAL.md`: the UCI surface did not move, so no change. `DEV_MANUAL.md`: its goldens table
has no row for this case and the case stores no number from a run; the mutation section dates its
counts and lists no file; its lines on `stop_search_after_ms` are about the race and still true. No change.

### The `accepts`, clause by clause

- A precondition from something a run produces, with the margin stated: `REQUIRE( still_running )` from
  the run's own output; 17.6 times, no cushion, stated here and at its site. Held.
- A mutant that disarms the timer, killed by it: TB01. Held.
- A second that lightens the board fails the new assertion rather than passing: TB02 and the lighter
  `timer_fen`, each `REQUIRE( still_running )`, each green before. Held.
- `stop_search_after_ms` named by a mutant: TB01's note. Held.
- The added time stated and the cheaper form chosen: +0.403 s, 7.1 s the form not taken. Held.
