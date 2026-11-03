id:         S322
goal:       every doctest binary reads its fixtures from the source tree by construction, so a new binary cannot pass under ctest and fail from another directory, and the build's unread copy of `tests/assets/` goes
accepts:    `CHESSO_SOURCE_DIR` reaches every doctest target from one place, or the reason it cannot is stated; a planted new binary calling `load_json` without the per-target line reads the same tally from the repository root, `build/tests` and a third directory, seen red first on the parent; `tests/CMakeLists.txt`'s `file(COPY ... assets)` removed only after every reader of `build/tests/assets` is shown gone (tests, scripts under `adocs/data/` and `tools/`, `DEV_MANUAL.md`), each reader named with what replaced it; no case, bound or assertion changes; both builds green
touches:    tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   refactoring `load_json` or the per-binary loaders beyond S310's `#ifdef CHESSO_SOURCE_DIR` form; changing what any case asserts
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause. `CHESSO_SOURCE_DIR` is defined once, in `add_doctest_target_linking`, and reaches all 26 doctest targets, read in both builds' `flags.make`, once each; `test_perft`, not doctest, keeps its own. A planted doctest binary calling `load_json` with no line of its own, on scratch copies only, read red from the root and an empty directory on `d3541c6` (0 of 3, `Cannot open assets/test_jsons/castling.json`) and 3 of 3 in `build/tests` and under ctest; with this diff 3 of 3, 29 assertions, from all three. The copy stays: by two stated searches every reader of `build/tests/assets` is gone but six, `adocs/data/`'s golden re-derivers of `tests/test_eval_positions.hpp`'s rows (S101's two, S121's, S123's, S125's, S135's), which build without the macro and run from `build/tests`; a reader outside `touches:` holds it, as the step allows, and the fix is a finding below. All 27 test binaries read one tally from the root, `build/tests` and a third directory in both builds, each equal to S320's after table, by script; no case, bound, assertion or clock changed. Fast suites 51 of 54 in both builds with the copy moved aside, the reds the two plant clocks at 60.07 s under the top-up's load and `test_clang_format_script` without `CLANG_FORMAT_MAJOR=22`, green rerun with it; the green run is the coordinator's gate with datagen paused, recorded by the coordinator at landing; format rc 0 with `CLANG_FORMAT_MAJOR=22`; prose checks rc 0. `DEV_MANUAL.md` takes the construction and why the copy stays; `MANUAL.md` checked, no change. Tier 2 does not bind. Filler (DEC-171).

## Why

Found 2026-11-02 by S320's implementer, and left by it as beyond its step. S320 gave eight binaries `CHESSO_SOURCE_DIR`
and a root `WORKING_DIRECTORY` per ctest entry. The guard covers only the entries it moved. A new doctest binary that
calls `load_json` without the macro still passes under ctest, which runs it from `build/tests`, where the
`file(COPY .../assets)` at the top of `tests/CMakeLists.txt` puts a copy, and it fails from anywhere else. Once every
reader takes the source tree, nothing reads that copy. S320's implementer named `add_doctest_target_linking` as the one
place a definition would reach every target. Filler by DEC-171: green under ctest, not reachable in play.

## Done, 2026-11-03

At `d3541c6`, in the worktree `chesso-s322`, Release and tune builds, with the coordinator's top-up datagen and S122's
work on the machine. A first agent was cut off by a shutdown while building; its unstaged edits to `tests/CMakeLists.txt`
and `DEV_MANUAL.md` (`.tuning/coord/S322/partial_unverified.diff`) were read, kept where verified below and corrected
where not: its comment in `add_doctest_target_linking` left out `adocs/data/S024_census_positions.txt`, which
`test_search` reads through the macro, and its two notes on the copy named the programs by step rather than by file.
Logs, the tally script and the planted file are in the main tree's `.tuning/coord/S322/logs/`, gitignored.

### What changed

- `tests/CMakeLists.txt`: `CHESSO_SOURCE_DIR` is defined once, in `add_doctest_target_linking`, and so reaches all 26
  doctest targets, the trace ones through `add_trace_doctest_target`. The per-target lines on `test_engine`,
  `test_tuner_gradient`, `test_search`, `test_uci_surface`, `test_mate_breadth`, `test_mate_pv` and `test_mate_carry`
  and the definition in S320's loop of seven are gone; the loop keeps its root `WORKING_DIRECTORY`. `test_perft` is not
  a doctest target and keeps its own line. The comments that justified the removed lines are folded into one in the
  function, history kept; the comment over `file(COPY ... assets)` says why the copy stays. Read in each build's
  `tests/CMakeFiles/*/flags.make`: the 26 doctest targets and `test_perft` carry the definition once each, `bench_eval`,
  `bench_movegen`, `debug_main` and `debug_perft_app` not at all. The comment over
  `add_trace_doctest_target(test_eval_trace)` is untouched, for S323.
- `DEV_MANUAL.md`: "Test binaries run from any directory" names `add_doctest_target_linking` and `test_perft`'s own
  line, and a paragraph says why `build/tests` still holds the copy and for which six programs. `MANUAL.md` checked, no
  change: it names no test binary's working directory.

Nothing under `src/`, `tools/` or `tests/*.cpp`; no case, bound, assertion or clock touched.

### The planted binary, red on the parent

On scratch copies only, never the tree: `tests/test_planted_fixture.cpp`, three cases calling `load_json` on the eight
JSON suites and both perft assets and `all_test_fens()`, registered by `add_doctest_target(test_planted_fixture)` and no
other line, appended to a `git archive` of `d3541c6` and to one of `d3541c6` with this diff applied. Each built to that
target alone, Release, and run from the copy's root, its `build/tests` and an empty directory in the scratchpad.

| tree | root | `build/tests` | third | ctest |
|---|---|---|---|---|
| `d3541c6` | 0/3, 4 assertions, rc 1, `Cannot open assets/test_jsons/castling.json` | 3/3, 29 | 0/3, 4, rc 1, the same | Passed |
| `d3541c6` with this diff | 3/3, 29 | 3/3, 29 | 3/3, 29 | Passed |

The parent is the class: green under ctest, red anywhere else. With this diff it also reads 3/3, 29 from `build/tests`
with that directory's `assets` moved aside, so it reads the source tree and not the copy.

### Readers of `build/tests/assets`, and why the copy stays

Two searches over `d3541c6`'s tracked files, less `tests/doctest`, `tests/third_party` and the history and coordinator
files (`adocs/plan_done`, `adocs/audit`, `adocs/decisions.md`, `adocs/status.md`): `git grep -n -I 'assets'`, every
mention of the directory, and `git grep -n -I -E 'build[^ ]*/tests'` over `*.sh`, `*.py`, `*.cpp`, `*.hpp`, `*.json`,
`*.cmake`, `CMakeLists.txt`, `TOOLCHAIN.md` and `DEV_MANUAL.md`, everything run from a build's test directory; 114 hits,
listed in the logs. A third over the programs outside `tests/` that include `tests/test_helpers.hpp`, and whether each
defines the macro. What each hit came to:

- The 26 doctest binaries and `test_perft`: read the source tree, by construction now. That moves every script that
  runs one of them from `build/tests`: `adocs/data/S136_screen.sh` (`test_eval_trace`), `adocs/data/S192_node_budget.py`
  (`test_search`), `adocs/data/S122_phase1_plants.sh` (`test_evaluation`, `test_eval_model`, `test_eval_trace`,
  `test_tuner_gradient`), `adocs/data/S307_matrix.py` (`test_engine`), `.vscode/launch.json` (`test_chesso`,
  `test_perft`; `test_openings`, `debug_main` and `debug_perft_app` open no file). The S033 sweeps run from `tests/`,
  the source.
- Not readers: `tools/` has no hit. `test_bare_fen_relay` is handed its session's source path. `DEV_MANUAL.md`'s
  `cd tests` lines run from the source tree. `adocs/data/S122_curve_probe.cpp` defines the macro on its own g++ line.
  `bench_movegen` (`TOOLCHAIN.md`) compiles its tables in, and `adocs/data/S155_motif_census.py` drives
  `debug_perft_app`, which opens no file. The rest name `tests/assets/` by its source path, are comments, or are
  evidence files (`.diff`, `.txt`).
- **Readers outside `touches:`, which hold the copy:** `adocs/data/S101_coverage_rows.cpp`,
  `S101_gradient_rows.cpp`, `S121_coverage_rows.cpp`, `S123_coverage_rows.cpp`, `S125_coverage_rows.cpp` and
  `S135_gradient_rows.cpp`, the re-derivers of `tests/test_eval_positions.hpp`'s rows, goldens by DEC-142. Each header
  builds its program by g++ without `CHESSO_SOURCE_DIR` and runs it from `build/tests`; each calls `all_test_fens()`,
  whose `load_json` then opens `assets/test_jsons/` relative to that directory, which is the copy.

So `file(COPY ... assets)` stays, and `accepts:`'s "removed only after every reader is shown gone" is answered the way the
step allows: a reader outside `touches:` holds it. Below, under Found.

### Tallies

All 27 compiled test binaries, run by hand from the worktree's root, from the build's `tests/` and from an empty
directory in the scratchpad, in both builds, against S320's after table (its `tally.tsv` per build, read by script).
Cases passed of total and doctest's assertion count; `test_perft` the positions it started and the depth rows it
printed. Every cell held in all three directories, rc 0 throughout, and equals S320's; the script exits 1 on any
disagreement and exited 0. No run left a file in the third directory or in the worktree. A cell holds for both builds
unless a tune figure follows it.

| binary | root, `build/tests` and third | S320, after |
|---|---|---|
| `test_chesso` | 12/12, 31041 | same |
| `test_openings` | 6/6, 62 | same |
| `test_movegen` | 22/22, 93019551 | same |
| `test_evaluation` | 55/55, 11095854 | same |
| `test_search` | 223/223, 1039070 / tune 230/230, 1039336 | same |
| `test_engine` | 87/87, 1002719 / tune 88/88, 1005246 | same |
| `test_eval_model` | 24/24, 3308650 | same |
| `test_tuner_groups` | 17/17, 23842 | same |
| `test_tuner_split` | 12/12, 6535 | same |
| `test_leaf_walk` | 8/8, 36587 | same |
| `test_tuner_gradient` | 17/17, 99229 | same |
| `test_eval_trace` | 12/12, 1924972 | same |
| `test_tuner_target` | 7/7, 418 | same |
| `test_corpus_hash` | 7/7, 627 | same |
| `test_corpus_dedupe` | 12/12, 70 | same |
| `test_eval_spread` | 2/2, 13 / tune 3/3, 26 | same |
| `test_audit_fen_semantics` | 29/29, 100 | same |
| `test_en_passant_key` | 4/4, 54 | same |
| `test_audit_polyglot_key` | 1/1, 36 | same |
| `test_audit_go_infinite` | 3/3, 29 | same |
| `test_invariants` | 2/2, 2736 | same |
| `test_search_params` | 6/6, 837 / tune 14/14, 1218 | same |
| `test_uci_surface` | 16/16, 181 / tune 16/16, 3506 | same |
| `test_mate_breadth` | 1/1, 4 | same |
| `test_mate_pv` | 5/5, 1214 | same |
| `test_mate_carry` | 1/1, 105 | same |
| `test_perft` | 21 positions, 58 depth rows | same |

### The suites

Serial, both builds, on the final code, each build's `tests/assets` moved aside for its run and put back after, so the
suite is also the proof that no ctest entry reads the copy; with the top-up datagen running and a gate in the main
tree. `build`: 51 of 54 in 507 s, `build-tune`: 51 of 54 in 514 s. The same three red in both:
`test_tuner_gradient_plants_pawns` and `test_tuner_gradient_plants_threats`, Timeout at 60.07 s each, the load S320
read (the tally runs both cases by hand, passing, 17 of 17, from every directory in both builds); and
`test_clang_format_script`, run without `CLANG_FORMAT_MAJOR=22`, so it asked for clang-format 23 and found 18.1.3 --
rerun alone with the variable and the copy aside, it passes in both builds, 2.23 s and 2.34 s. No entry failed on a
missing file. `test_perft`, `slow`, passes through ctest in `build` with the copy aside, 119.42 s. Prose checks on this
file, `--touches` and `--citations`, rc 0; `--params` and `--gate` rc 0.

`CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0. Tier 2 (DEC-141) does not bind: no `make_move`, generator or
search change, no pruning rule. Filler (DEC-171): nothing under `src/`, so the bench is the parent's.

### Found, not fixed

- **The six re-derivers above hold the copy.** Fix: add `-DCHESSO_SOURCE_DIR=\"$PWD\"` (the repository root, from
  which each header already builds) to the six g++ lines, as `S122_curve_probe.cpp`'s does, re-derive each block once to
  show it unmoved, then remove `file(COPY ... assets)` and the two notes on it, in `tests/CMakeLists.txt` and
  `DEV_MANUAL.md`. Each still reads `../../tests/test_eval_positions.hpp` relative to its working directory, so it still
  runs from `build/tests`, only no longer off the copy. Until the copy goes, the class is closed for any binary that
  reads through `load_json` or the two `#ifdef` loaders, not for a new loader that opens `assets/...` without the
  prefix: that one still passes under ctest in `build/tests` on the copy. With the copy gone it reads red there too, so
  removing it is what closes the class by construction.
