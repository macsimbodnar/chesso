id:         S324
goal:       the six golden re-derivers that read `test_eval_positions.hpp`'s fixtures run from any directory, and the build's copy of `tests/assets/` goes once nothing reads it
accepts:    each of `adocs/data/S101_coverage_rows.cpp`, `S101_gradient_rows.cpp`, `S121_coverage_rows.cpp`, `S123_coverage_rows.cpp`, `S125_coverage_rows.cpp` and `S135_gradient_rows.cpp` gets its fixtures and `test_eval_positions.hpp` from the source tree by its build line (`-DCHESSO_SOURCE_DIR`, as `S122_curve_probe.cpp` does) and prints, from the repository root, from `build/tests` and from a third directory, the same output it printed before from `build/tests` (diffed, byte for byte); every golden it re-derives unchanged (DEC-142: a re-derivation, not a re-read); then `tests/CMakeLists.txt`'s `file(COPY ... assets)` removed, with the search for any further reader (S322's patterns and hits) re-run at landing and empty; both fast suites green with no `build*/tests/assets`
touches:    adocs/data/S101_coverage_rows.cpp, adocs/data/S101_gradient_rows.cpp, adocs/data/S121_coverage_rows.cpp, adocs/data/S123_coverage_rows.cpp, adocs/data/S125_coverage_rows.cpp, adocs/data/S135_gradient_rows.cpp, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   any change to a golden, a case or what a script computes; refactoring `load_json`
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause. The six programs take `-DCHESSO_SOURCE_DIR=\"$PWD\"` on their headers' g++ lines, `S122_curve_probe.cpp`'s form, read the JSON suites through `load_json` and, the five that read it, `tests/test_eval_positions.hpp` out of the source tree, and refuse to compile without the macro. Each built at `1bb3080` and run from `build/tests` on the copy, then built by the new line and run from the root, from `build/tests` with no copy and from an empty third directory: 18 of 18 outputs byte-equal by `cmp`, and 6 of 6 again built and run by the headers verbatim; the four coverage programs on S083's corpus in place, sha256 `2e0e3de9...`. Every golden re-derived and unchanged: S101's 6 pairs, DEC-316's pair at candidate 55, S121's 19, S123's 4 and S125's 2 each "reproduces: yes". S135's program re-derives nothing at this tree, before as after: a finding below, outside `excludes:`, planned as S326. S322's three searches re-run on the final tree, 71, 48 and 11 hits, find no reader of `build/tests/assets`, so `file(COPY ... assets)` is removed and the two comments that leaned on it corrected. Fast suites with no `build*/tests/assets`: `build` 54 of 54; `build-tune` 48 of 54 under the sample's datagen load, six Timeouts, each passing when rerun alone; the commit needs both suites green at the coordinator's gate with the machine quiet and no `build*/tests/assets`; format rc 0 with `CLANG_FORMAT_MAJOR=22`; prose checks rc 0. Fast check: `S101_gradient_rows.cpp`'s header now names its `S101_POSITIONS_HPP` override, comments only, its output rebuilt by the header and run from the root byte-equal to before. `DEV_MANUAL.md` updated, `MANUAL.md` checked, no change. No new `adocs/data/S324_*` file, so no README row. Tier 2 does not bind. Filler (DEC-171).

## Why

Found 2026-11-03 by S322's implementer. S322 put `CHESSO_SOURCE_DIR` on every doctest target by construction, but the copy
of `tests/assets/` in `build/tests` had to stay. Six golden re-derivers under `adocs/data/` (DEC-142) are built by `g++`
without the macro and run from `build/tests`. Each calls `all_test_fens()`, which goes through `load_json`'s relative path. They
also include `../../tests/test_eval_positions.hpp` by a path relative to the working directory, so they run two levels below
the root. Filler by DEC-171: the goldens re-derive today; only the directory they need is fragile.

## Done, 2026-11-03

At `1bb3080`, in the worktree `chesso-s324`, Release and tune builds, with the coordinator's datagen on the machine for
most of it. Every build and run under `nice -n 10`. Outputs, runners and the search logs are in the main tree's
`.tuning/coord/S324/logs/`, gitignored.

### What changed

- The six programs, `adocs/data/S101_coverage_rows.cpp`, `S101_gradient_rows.cpp`, `S121_coverage_rows.cpp`,
  `S123_coverage_rows.cpp`, `S125_coverage_rows.cpp` and `S135_gradient_rows.cpp`: each header's g++ line gains
  `-DCHESSO_SOURCE_DIR=\"$PWD\"`, the root it is built from, `S122_curve_probe.cpp`'s form, so `load_json` reads the
  JSON suites from the source tree. The five that read `tests/test_eval_positions.hpp` as text (all but S135's) default to
  `CHESSO_SOURCE_DIR "/tests/test_eval_positions.hpp"` for `../../tests/test_eval_positions.hpp`; each
  `S1xx_POSITIONS_HPP` override is kept. Each refuses to compile without the macro, by `#error`, so a stale build line
  fails at build and not at the first open: seen on S135's, built without it, rc 1 at the `#error`. Each run line is
  now `[S1xx_COVERAGE_DATA=<corpus>] build/S1xx_...` from any directory, for `(cd build/tests && ...)`. Nothing else:
  no rule, threshold, candidate order, print or check changed. From the fast check, `S101_gradient_rows.cpp`'s
  header names the `S101_POSITIONS_HPP` override it reads, as the other four that read the header do; comments only,
  and rebuilt by its header and run from the root it prints the before output again, `cmp` equal.
- `tests/CMakeLists.txt`: `file(COPY ... assets)` and its comment removed; the two comments that said `build/tests`
  holds the copy say it did, until S324.
- `DEV_MANUAL.md`: the paragraph on the copy says it is gone, what moved the six, and that a build directory configured
  before S324 keeps its old copy until deleted; "which holds a copy" in the paragraph above reads "held a copy then".
  `MANUAL.md` checked, no change: it names none of the six, `build/tests` or `assets` (0 hits).

Nothing under `src/`, `tools/` or `tests/*.cpp`; no golden, case, bound or assertion touched.

### Before and after, byte for byte

Before: each program built at `1bb3080` by its old header line, run from `build/tests` on the copy (`diff -r
tests/assets build/tests/assets` empty). After: built by the new line, run from the repository root, from `build/tests`
with both builds' `tests/assets` moved out, and from an empty directory in the scratchpad; root and third side by side,
each set with its own `TMPDIR`, since each program writes a fixed-name `.tsv` in the temporary directory. Compared by
`cmp`, stdout and stderr each; stderr is empty in every run. The four coverage programs took S083's corpus in place by
one absolute path, the same string in every run, because each prints the path it is given:
`/home/max/ws/chesso/.tuning/s083/curve/s0/cand_6226944_s1.tsv`, 6226944 rows, sha256 `2e0e3de9a4be...0872c53`, equal to
`adocs/data/S101_screen.sh`'s `CORPUS_SHA`; nothing copied. S101's gradient program and S135's take no corpus.

| program | corpus | before, `build/tests`: sha256, lines | root | `build/tests`, no copy | third |
|---|---|---|---|---|---|
| `S101_coverage_rows` | S083's (named, not read: `all_test_fens()` closes all 14 needs) | `b05671e0cf45`, 59 | equal | equal | equal |
| `S101_gradient_rows` | none | `8bb577582e67`, 71 | equal | equal | equal |
| `S121_coverage_rows` | S083's, read: 6226944 rows, 659 candidates kept | `e6dcd16060c7`, 166 | equal | equal | equal |
| `S123_coverage_rows` | S083's, read: 187 kept | `4acc80332389`, 47 | equal | equal | equal |
| `S125_coverage_rows` | S083's, read: 478 kept | `9648e12056a3`, 41 | equal | equal | equal |
| `S135_gradient_rows` | none | `6ad1772bca04`, 20 | equal | equal | equal |

18 of 18 equal, rc 0 throughout, the third directory empty after. The base binaries from the root read red, so the
directory mattered: `S101_gradient_rows` rc 1 at `REQUIRE( header.good() )`, `S135_gradient_rows` rc 1 at `Cannot open
assets/test_jsons/castling.json`. And by the headers as written: each new g++ line, read out of its header and run by
bash from the root, compiles to `build/S1xx_...` rc 0, and each run line from the root (`S1xx_COVERAGE_DATA=<the corpus>
build/S1xx_...`) prints the before output again, 6 of 6 equal by `cmp`. The root and third-directory sets ran while
`build/tests` still held the copy; the binaries open only absolute paths, and the `build/tests` and verbatim sets ran
with it gone.

### The goldens each re-derives

Read off the outputs above, identical in all four directories; DEC-142's re-derivation by the script, nothing re-read.

- `S101_coverage_rows`: S101's 6 pairs after the first 92 rows, "reproduces the landed block: yes, pair for pair and in
  order (12 picked rows, 12 landed)", 0 ineligible, both checks clear.
- `S101_gradient_rows`: DEC-316's pair after the first 104 rows, the pick at candidate 55 of 55, |g| 6.164e-04 against
  the 2.252e-04 the rule wants, "reproduces the landed pair: yes"; 984 reads 1.11153e-07 with it, the 1.11e-07
  `tests/test_tuner_gradient.cpp`'s comment states.
- `S121_coverage_rows`: S121's 19 pairs after the first 42, "yes (38 picked rows, 38 landed)", 0 ineligible.
- `S123_coverage_rows`: S123's 4 pairs after the first 80, "yes (8, 8)", 0 ineligible.
- `S125_coverage_rows`: S125's 2 pairs after the first 88, "yes (4, 4)", 0 ineligible.
- `S135_gradient_rows`: **re-derives nothing at this tree**, the same before and after; below, under Found.

### Readers of `build/tests/assets`, and the copy

S322's three searches, its patterns and exclusions, re-run over the final tree's tracked files: `git grep -n -I
'assets'` less `tests/doctest`, `tests/third_party`, `adocs/plan_done`, `adocs/audit`, `adocs/decisions.md` and
`adocs/status.md`, 71 hits; `git grep -n -I -E 'build[^ ]*/tests'` over `*.sh`, `*.py`, `*.cpp`, `*.hpp`, `*.json`,
`*.cmake`, `CMakeLists.txt`, `TOOLCHAIN.md` and `DEV_MANUAL.md`, 48; the programs outside `tests/` that include
`tests/test_helpers.hpp`, 11. S322's classes hold for every hit: a test binary that reads the source tree by
construction (`.vscode/launch.json`, `S122_phase1_plants.sh`, `S136_screen.sh`, `S307_matrix.py`), a binary that opens
no file (`debug_perft_app`, `bench_movegen`), the source path `tests/assets/`, a comment or prose, or evidence. Search
3's eleven: the six, now on the macro; `S122_curve_probe.cpp`, on it; `comment_study/pilot/test_mate_carry.cpp`, an
evidence copy of a test, on it; three `.diff` files. Against the same searches run before `tests/CMakeLists.txt` and
`DEV_MANUAL.md` were edited, the only lines that differ are those two files' own. No reader is left, so `file(COPY ...
assets)` is gone.

### The suites

Serial, `CLANG_FORMAT_MAJOR=22`, `nice -n 10`, on the final code, each build's `tests/assets` absent, checked before
each run. `build`: **54 of 54 in 174 s**, run in a lull between the coordinator's runs, the plant entries included
(`pawns` 10.58 s, `threats` 16.11 s). `build-tune`: 48 of 54 in 1002 s, with datagen back on twelve threads at nice 0:
six Timeouts and no failure, `test_tuner_gradient_plants_pawns`, `_threats` and `_king_safety` at 60.15 s,
`test_mate_breadth` and `test_mate_carry` at 120.15 s, `test_fastchess_script` at 60.31 s. All six rerun by hand from
the entry's own directory and command, unclocked, under the same load, pass: `pawns` 126 s, `threats` 191 s,
`king_safety` 68 s, each 1 of 1; `test_mate_breadth` 338 s and `test_mate_carry` 304 s, 1 of 1; `test_fastchess_script`
69 s, its 40 properties holding. All six passed in `build`'s suite minutes before, and this diff touches no test, source
or tool: the clock under load, not the change. No entry failed on a missing file. The coordinator's gate with the
machine paused is the reading that counts.

`CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0. `plan_prose_check.py` `--touches` and `--citations` on this
file, `--params` and `--gate`, rc 0. Tier 2 (DEC-141) does not bind: no `make_move`, generator or search change, no
pruning rule. Filler (DEC-171): nothing under `src/`, so the bench is the parent's.

### Found, not fixed

- **`adocs/data/S135_gradient_rows.cpp` no longer re-derives its pair** (DEC-142: a golden that cannot be scripted is a
  finding). Written at S135 when the fixture had 40 rows, it takes the whole landed list as its base. Its pair stands at
  rows 41 and 42 of 106 now, with S121's and later blocks after it, so the program skips its own pair as known, takes
  the first eligible candidate, `1BQKRBRN/PPPPPPPP/1N6/8/8/8/3k4/8 b - - 1 1` (|g774| 3.413e-06 over a threshold of
  1.943e-07), a row the fixture does not hold, and exits 0: it has no reproduction check. `S101_gradient_rows.cpp`'s
  shape is the fix: `BASE_ROWS` = 40 asserted against the pair's marker, and "reproduces the landed pair". Outside this
  step's `excludes:`, what a script computes. Planned as S326.
- **Build directories configured before S324 keep their copy.** Removing `file(COPY ...)` deletes nothing. The main
  tree's `build`, `build-tune`, `build-debug`, `build-prof`, `build-sanitize` and `build-release-{avx2,bmi2,portable}`,
  and `chesso-s122`'s `build` and `build-tune`, still hold `tests/assets`. Until it is deleted (`rm -r
  build*/tests/assets`), a new loader that opens `assets/...` relatively still passes under ctest there, the class S322
  meant this removal to close.
