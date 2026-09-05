id:         S197
goal:       `tools/gate_extra.sh` runs what the fast label cannot hold -- the Debug binaries, a sanitizer build, deep perft, the prose and citation checks -- and prints a terminal marker; the coverage recipe is documented beside it
accepts:    the script builds `build-debug` and runs its six invariant-carrying binaries from `tests/` (INV-2, INV-4), configures and builds a sanitizer directory with the existing `SANITIZER` option and runs the fast label and `bench` under it, runs `ctest -L slow`, runs `tools/plan_prose_check.py --prose` and `--citations`, and ends with `GATE-EXTRA-DONE` or `GATE-EXTRA-FAILED` on every exit path (WATCHERS rule); every stage's exit status reaches the shell; its wall time on this machine is measured and recorded in this file; `DEV_MANUAL.md` "Test" documents the script, the DEC-141 cadence, and the `llvm-cov` coverage recipe of the 2026-09-04 test review as an on-demand command whose output is compared with `adocs/data/2026-09-04_test_review/coverage_unexecuted.txt`; `tests/test_gate_extra_script.sh` smoke-tests both markers in a sandbox like the other script tests; fast suite green in both builds
touches:    tools/gate_extra.sh, tests/, tests/CMakeLists.txt, DEV_MANUAL.md, CMakeLists.txt
excludes:   putting any of it in the automatic TESTS gate (DEC-025); a remote CI
decisions:  DEC-139, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why this exists

The 2026-08-14 test review listed sanitizer runs and the Debug assertions as
"absent from the gate" and the 2026-09-04 review found them still by hand;
Stockfish runs the equivalent in CI (`sanitizers.yml`, `games.yml`,
`adocs/testing_strategy.md` section 3.1). DEC-025 keeps the automatic gate to
the fast suite; DEC-141 fixes when this second tier runs. R13 of the strategy
document -- coverage as a periodic report, never a gate -- folds in here as a
documented command.

## Cost

Machine-free to write; about 20 minutes to run, measured when it exists.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

The automatic gate (the TESTS rule command, about 108 s) builds two Release
directories and runs the `fast` label. Everything that gate cannot hold is today
run by hand when somebody remembers: the Debug build, whose `assert(` lines are
the only enforcement of INV-2 and INV-4 (`src/bitboard.cpp`
`squares_match_bitboards` and `eval_accumulators_match`, called from every
make and unmake inside `#ifndef NDEBUG`); a sanitizer build, which exists as the
`SANITIZER` option in `CMakeLists.txt` and was last run at the 2026-08-13 audit;
deep perft (`ctest -L slow`, `tests/test_perft.cpp`, INV-1); and the two
`tools/plan_prose_check.py` modes that are kept out of the fast label because
they go red on unrelated commits. This step writes one script,
`tools/gate_extra.sh`, that runs all of it in a fixed order, prints one terminal
marker on every exit path, and is smoke-tested in a sandbox so the marker
cannot be lost again. DEC-141 clause 3 fixes when it runs: before a step that
touched `make_move`, `unmake_move`, the generator or the search completes, and
otherwise weekly. DEC-025 keeps it out of the automatic gate. The coverage
recipe the 2026-09-04 review used is written down beside it as an on-demand
command (R13), not as a stage.

### 2. The technique as published

Three practices, each with its source.

**Asserts and sanitizers in CI.** Stockfish's `sanitizers.yml`
(https://raw.githubusercontent.com/official-stockfish/Stockfish/master/.github/workflows/sanitizers.yml,
read as CI configuration only) builds `debug=yes optimize=no` with `-O1
-fno-inline` under g++ on Ubuntu 22.04 and runs one matrix entry per tool:
thread sanitizer, undefined-behaviour sanitizer, Valgrind, Valgrind with
threads, a non-instrumented baseline and a `-D_GLIBCXX_ASSERTIONS` build, each
driving the engine's command set through `tests/instrumented.py`. Chesso takes
the shape and not the matrix: one ASan+UBSan directory (the option already
exists), the fast label and `bench` under it. TSan is pointless while the
search is single-threaded; Valgrind is not installed on either machine.

**What the sanitizers promise.** ASan: "Typical slowdown introduced by
AddressSanitizer is 2x"; "To get a reasonable performance add -O1 or higher. To
get nicer stack traces in error messages add -fno-omit-frame-pointer"; it
"exits on the first detected error" with a non-zero code; "The leak detection
is turned on by default on Linux, and can be enabled using
ASAN_OPTIONS=detect_leaks=1 on macOS" (https://clang.llvm.org/docs/AddressSanitizer.html).
UBSan: "For most checks, the instrumented program prints a verbose error
report and continues execution upon a failed check"; `-fno-sanitize-recover=`
makes it "print a verbose error report and exit the program"
(https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html). GCC says the
same in its own words: for `-fsanitize=undefined` "error recovery is turned
on by default" and `-fno-sanitize-recover=` turns it off
(https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html). The
common runtime flag `exitcode` defaults to 1
(https://github.com/google/sanitizers/wiki/SanitizerCommonFlags). LSan "adds
almost no performance overhead until the very end of the process"
(https://clang.llvm.org/docs/LeakSanitizer.html).

**Source-based coverage.** Compile with `-fprofile-instr-generate
-fcoverage-mapping`, run under `LLVM_PROFILE_FILE` where `%p` "expands out to
the process ID" and `%m` "expands out to the instrumented binary's
signature", merge with `llvm-profdata merge -sparse`, report with `llvm-cov
report|show|export -instr-profile PROFILE BIN [-object BIN]...
-ignore-filename-regex=PATTERN`; `show -show-branches=count` prints branch
counts, `export -format=text` is JSON with "regions, functions, branches,
expansions, and summaries" (https://clang.llvm.org/docs/SourceBasedCodeCoverage.html,
https://llvm.org/docs/CommandGuide/llvm-cov.html). The gcc route is
`--coverage`, "a synonym for -fprofile-arcs -ftest-coverage (when compiling)
and -lgcov (when linking)" (GCC docs above); it produces gcov output, not the
review's format, so the documented recipe is clang's.

**The marker discipline** is this repository's own: DEC-061 and the WATCHERS
rule -- a detached run "prints a terminal marker as its last action, success
and failure both", and a watcher polls the whole file with four exits.

### 3. What chesso has today, and where the change plugs in

- `CMakeLists.txt` `option(SANITIZER ...)`: `add_compile_options` and
  `add_link_options` with `-fsanitize=address` and `-fsanitize=undefined`,
  nothing else. It lacks `-fno-sanitize-recover=undefined`, so a UBSan report
  today leaves the exit status 0 (section 5, the first trap). Add
  `-fno-sanitize-recover=undefined` and `-fno-omit-frame-pointer` to the same
  block; `CMakeLists.txt` is in `touches:`, and no shipping binary is built
  with the option.
- `tests/CMakeLists.txt` `CHESSO_TEST_TIMEOUT`: 60 s for `Release` and
  `MinSizeRel`, 600 s for any other build type; the three mate binaries take
  120 s / 1500 s on the same split. `add_doctest_target` registers every doctest
  binary under `fast`; `test_perft` is `slow` with `TIMEOUT 1800`.
- **The six invariant-carrying binaries** are the six `add_doctest_target`
  calls above the tuner block: `test_chesso`, `test_openings`, `test_movegen`,
  `test_evaluation`, `test_search`, `test_engine`. They are the engine tests
  that drive `make_move` and `unmake_move` in Debug; the tuner, script, audit
  and surface tests either do not link a search or make no moves, and the three
  mate binaries are excluded on cost (section 10, question 2). The 7-minute
  figure R10 quotes is `test_movegen` 165 s plus `test_search` 216 s (the
  DEC-049 machine, 2026-08-14, `tests/CMakeLists.txt` header comment) plus the
  four small ones. Run them through ctest with an anchored regex --
  `-R '^(test_chesso|test_openings|test_movegen|test_evaluation|test_search|test_engine)$'`
  -- because an unanchored `test_search` also matches `test_search_params`.
- `tools/plan_prose_check.py` `main`: the first argument selects `--prose`
  (tense over `adocs/plan.md`), `--citations` (`citations()` over
  `pending_step_files`, printing `citations flagged: N over M files`),
  `--touches`, `--params`; exit 1 when anything is flagged, 2 on a usage error.
- `fastchess.sh` and `rating.sh` are the marker template: `marked=0`,
  `fail()` sets it and prints the FAILED marker, an EXIT trap armed before the
  first command that can fail reads `marked` and `completed` rather than `$?`
  (S167: bash 3.2's trap sees `$? == 0` on a `set -u` abort). `fastchess.sh`
  writes to `outdir="${OUT:-/tmp/chesso_sprt_${tag}_${stamp}}"`; mirror the
  `OUT` convention.
- `tests/test_rating_script.sh` is the smoke-test template: a throwaway
  directory, `PATH` holding only linked-in utilities and stubs, `run_sandbox`
  returning the exit status, `markers` counting marker lines.
- `bench` is S189's; S189 precedes this step in `plan.md`'s Open list, so the
  command exists when this step starts. If it does not, the stage fails on the
  missing `<nodes> nodes <nps> nps` line and the marker says so.

**The script, stage by stage.** Every stage is a shell function; the driver
runs each one with stdout and stderr redirected to `$outdir/NN_<stage>.log`,
records its exit status and wall time, and continues to the next. Nothing is
piped: `ctest | tail` hides the exit status (`DEV_MANUAL.md` "Test": "a gate
written that way reports success on a failed suite"). All stages run even
after a failure, because the weekly run's value is the whole picture in one
pass; the marker lists every failed stage. Cheapest first:

| # | stage | command | expected wall, workstation | what it catches that the fast gate cannot |
|---|---|---|---|---|
| 1 | `prose` | `python3 tools/plan_prose_check.py --prose` | seconds | stale tense in `adocs/plan.md` |
| 2 | `citations` | `python3 tools/plan_prose_check.py --citations` | seconds | a code citation in a pending step that no longer resolves |
| 3 | `debug` | configure `build-debug` if it has no `CMakeCache.txt` (`-DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`), `cmake --build build-debug -j"$JOBS"`, then the anchored `ctest -R` above with `--output-on-failure` | about 7 min (165 s + 216 s measured there) | INV-2 and INV-4, the PV assert in `src/search.cpp` `search` under `#ifndef NDEBUG`, every other `assert(` in `src/` |
| 4 | `sanitize` | configure `build-sanitize` if absent (`-DCMAKE_BUILD_TYPE=RelWithDebInfo -DSANITIZER=ON -DCMAKE_CXX_COMPILER_LAUNCHER=ccache`), build, `ctest --test-dir build-sanitize -L fast --output-on-failure`, then `printf 'bench\nquit\n' | build-sanitize/src/chesso` must print a line matching `^[0-9]+ nodes [0-9]+ nps$` | cold build 1-2 min, label 2-3 min (2x of about 52 s), bench under 1 min | out-of-bounds reads, use-after-free, signed overflow, bad shifts, misaligned loads, leaks (Linux) |
| 5 | `perft` | `cmake --build build -j"$JOBS"` then `ctest --test-dir build -L slow --output-on-failure` | about 1 min (S161: 57.92 s on the MacBook) | INV-1 at the depths the fast label does not reach |

Estimated total 12 to 15 minutes; R10 said about 20. The first real run
measures it and the table in the stamp replaces these estimates.

`RelWithDebInfo` for the sanitizer directory, not `Release` and not `Debug`:
`-O2 -g` gives the "-O1 or higher" ASan asks for and symbolised reports, and
it falls on the 600 s side of `CHESSO_TEST_TIMEOUT`, so a 2x slowdown of
`test_mate_breadth` (18 s in Release) cannot hit the 60 s Release ceiling and
read as a failure that is the clock (the DEC-052 class). `Debug` plus ASan
would put the two invariants under the sanitizer as well at roughly 2x of 381 s
for two binaries alone; section 10 leaves that to the owner, default no.

Export before stage 4, in the script: `ASAN_OPTIONS=detect_leaks=1`
(explicit, so the Linux default and the macOS opt-in read the same) and
`UBSAN_OPTIONS=print_stacktrace=1`. Do not set `detect_leaks=0`: a leak the
run reports is a defect and the BUGS rule applies.

**Environment and output.** `JOBS` defaults to `nproc`, else `sysctl -n
hw.logicalcpu`, else a `fail` naming cores (the S177 chain); `OUT` defaults to
`/tmp/chesso_gate_extra_<stamp>`; `STAGES` optionally names a subset
(`STAGES="sanitize perft"`) for re-running one stage after a fix -- the default
is all five and the smoke test uses the default. The last lines are the
summary table (stage, status, seconds, log path), then for each failed stage
its name, exit status and the last 40 lines of its log to stderr, then the
marker: `GATE-EXTRA-DONE <n> stages <total_s> s <outdir>` or
`GATE-EXTRA-FAILED: <stage> <stage>... <outdir>`. The trap covers every other
exit exactly as `rating.sh` does.

**Order of edits.** `CMakeLists.txt` flags first and observe the red of
section 6 (a); `tools/gate_extra.sh`; `tests/test_gate_extra_script.sh` and
its `tests/CMakeLists.txt` entry; the first real run, detached; `DEV_MANUAL.md`
"Test"; the stamp.

### 4. Constants and seeds

No `src/search_params.hpp` entry: the engine is untouched. The script's
numbers, each form (c) or a repository convention: `JOBS` from the core count
(the TESTS rule's `-j8` is the MacBook's count, the workstation has 12); the
smoke test's `TIMEOUT 60`, the value every other script test carries in
`tests/CMakeLists.txt` (`test_rating_script`); 40 tail lines per failed stage,
a midpoint between a screen and a full log; a watcher ceiling of 60 min, 2x
the upper estimate per the WATCHERS rule, revised to 2x the measured total
once the stamp has it.

### 5. Interactions and traps

- **UBSan is green by default.** Without `-fno-sanitize-recover=undefined` a
  report is printed and the binary exits 0, and the stage passes with
  undefined behaviour on the log. Both compilers document this (section 2).
  The `UBSAN_OPTIONS=halt_on_error=1` runtime form is `unverified` in the
  docs fetched; use the compile flag.
- **LeakSanitizer differs by platform.** On by default on Linux, off on macOS
  (ASan docs). A macOS run of this script sees one class less; the
  workstation's run is the one that counts. Its exit code on a leak is not 1
  (`unverified`: 23) -- read the report, do not pattern-match the status.
- **A sanitizer that changes the tree is a finding.** After stage 4, compare
  the sanitizer `bench` total with `build/src/chesso bench`'s: they must be
  equal (INV-6 across builds). A difference is uninitialised or out-of-bounds
  reading that the sanitizer did not report. Section 10, question 1, asks
  whether the script asserts it or the stamp records it.
- **`-Werror` under gcc with sanitizers** may raise warnings the plain build
  does not (`unverified`: `-Wmaybe-uninitialized` under `-fsanitize=undefined`
  at `-O2`). A new warning is read and fixed or recorded, never silenced with a
  blanket flag; DEC-049 made gcc the reference and the tree already compiles
  clean under it.
- **`set -e` is off inside an `if` condition.** A stage function called as
  `if stage_x; then` runs its body without errexit; write every stage body with
  explicit `&&` chains and `return $?`.
- **bash 3.2 versus bash 5.** The smoke test runs under `/bin/bash` on the
  MacBook (3.2.57) and the workstation has bash 5; write for 3.2: `marked` and
  `completed` in the trap instead of `$?`, no `command` builtin left of `||`
  (S167), no associative arrays, no `mapfile`, `[[ -n "${x:-}" ]]` before every
  `rm`. `date +%s`, indexed arrays and `$(( ))` are fine in both.
- **`--citations` is red on purpose sometimes.** `DEV_MANUAL.md` keeps it out
  of the fast label because "any source commit shifts lines under fifty step
  files at once"; here a red is the signal to fix the citation (DEC-135 says
  by symbol) in the step file, never to relax the check.
- **`test_perft` before S193 passes vacuously on a missing asset** (S193's
  R5: `assert(file)` compiles out of Release). S193 precedes this step in the
  Open list; if the order changes, the perft stage's log must show a non-zero
  case count, not only `Passed`.
- **The machine holder.** The Debug binaries and the ASan build load every
  core; never run this beside a match or an SPSA (PLAN and MACHINE rules).
- **A run is longer than one tool call.** Twelve to twenty minutes exceeds the
  Bash tool's 10-minute ceiling: launch detached, `nohup tools/gate_extra.sh >
  .tuning/gate_extra_<date>.log 2>&1 &` (`.tuning/` is gitignored), and arm
  the WATCHERS poll loop on `GATE-EXTRA-(DONE|FAILED)`, ceiling 60 min, pid
  check after the marker grep as `DEV_MANUAL.md` "Detach the run" shows.
- **`.gitignore` already covers `build-*`**, so `build-sanitize/` and
  `build-coverage/` need no entry; keep `.profraw` and `.profdata` inside
  `build-coverage/` so nothing new is ignored.
- **ccache is cold for the sanitizer flags**; the first stage-4 build is a
  full compile and the estimate in the table says so.

### 6. Tests

(a) **Red-first for the recovery flag.** In a scratch worktree of `HEAD`,
insert into `src/chesso.cpp` `command_test` a signed overflow (`int x =
INT_MAX; x += 1; (void)x;`), configure with `-DSANITIZER=ON` at
`RelWithDebInfo` without the new flag, run `printf 'test 1\nquit\n' |
build-sanitize/src/chesso`, observe `runtime error: signed integer overflow`
and exit 0; add `-fno-sanitize-recover=undefined`, rebuild, observe a non-zero
exit. Quote both in the stamp; remove the worktree.

(b) **`tests/test_gate_extra_script.sh`**, registered in `tests/CMakeLists.txt`
beside `test_rating_script` (`LABELS "fast" TIMEOUT 60`), invoked as
`bash test_gate_extra_script.sh <gate_extra.sh>`. It copies the script to
`$tmp/tools/gate_extra.sh` so the root it derives from `$0` is `$tmp`; `PATH`
is `$tmp/stub` only. Stubs, each appending its name and arguments to
`$tmp/calls.log`: `cmake` (exit 0), `ctest` (exit 0, or 1 when
`$tmp/fail_ctest_fast` exists and `-L fast` is among the arguments), `python3`
(exit 0), `nproc` (`echo 8`); `$tmp/build-sanitize/src/chesso` and
`$tmp/build/src/chesso` are one-line scripts printing `12345 nodes 100 nps`.
Four properties, in the style of the rating test's numbered cases:

1. every stub succeeds: exit 0, exactly one marker line and it is
   `GATE-EXTRA-DONE`, and `calls.log` shows the stage order of section 3 --
   `--prose`, `--citations`, `build-debug` build then `ctest -R`,
   `build-sanitize` configure, build, `ctest -L fast`, then `ctest -L slow`;
2. `fail_ctest_fast` present: exit non-zero, exactly one marker,
   `GATE-EXTRA-FAILED:` naming `sanitize`, and `calls.log` still contains the
   `-L slow` call after it (all stages run);
3. `cmake` removed from the stub directory: exit non-zero, exactly one marker,
   FAILED, no second marker from the trap;
4. static: `bash -n tools/gate_extra.sh`, and no bare `$(nproc)` outside the
   fallback chain (the S177 grep).

No real stage runs: the stubs build nothing. Goldens: none. Mutant: none, no
search rule. INV-6 and Debug self-play: not owed, no `src/` change.

### 7. Measurement

No lane. The engine's play and node counts are untouched, so neither an SPRT
nor the INV-6 identity is owed. What is measured is the script itself: one
real, detached run on the workstation, on mains, nothing else running,
recording per stage the wall time and status, plus the total, the sha, the
compiler (`g++ 13.3`, DEC-049) and the date -- into this file's stamp as the
table the accepts asks for, replacing section 3's estimates. Record also the
sanitizer `bench` total beside `build/src/chesso bench`'s. A failed stage on
that first run is a finding: fix it or record it before the step completes
(BUGS rule); the run is repeated until `GATE-EXTRA-DONE`, and both outcomes are
quoted.

**The coverage recipe for `DEV_MANUAL.md` "Test"**, on demand, clang only,
re-run when a subsystem is added, never a stage:

```bash
# workstation: clang++ and llvm-cov installed (apt: clang llvm); macOS: prefix llvm-* with xcrun
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"
cmake --build build-coverage -j"$(nproc)"
LLVM_PROFILE_FILE="$PWD/build-coverage/prof/%m-%p.profraw" ctest --test-dir build-coverage -L fast
llvm-profdata merge -sparse build-coverage/prof/*.profraw -o build-coverage/chesso.profdata
objs=$(for b in build-coverage/tests/test_* build-coverage/tools/*; do [ -x "$b" ] && [ ! -d "$b" ] && printf -- '-object %s ' "$b"; done)
llvm-cov report -instr-profile=build-coverage/chesso.profdata build-coverage/src/chesso $objs \
  -ignore-filename-regex='tests/|tools/|doctest|json' > build-coverage/summary.txt
llvm-cov show -instr-profile=build-coverage/chesso.profdata build-coverage/src/chesso $objs \
  -show-branches=count -show-line-counts-or-regions -sources src/ > build-coverage/show.txt
```

`summary.txt` compares with `adocs/data/2026-09-04_test_review/coverage_summary.txt`
(its "89 functions have mismatched data" warning is expected across 25
binaries); lines whose count column is `0` in `show.txt` compare with
`coverage_unexecuted.txt`, by region and by eye, since every commit since
`5cffb70` has shifted its line numbers. Note that the review used Apple clang
and `-march=native`, so the workstation's numbers are the same recipe on a
different compiler; coverage is reach, not speed, and transfers.

### 8. Completion checklist

- Gate: `cmake --build build -j8 && ctest --test-dir build -L fast --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh --check`
  (`-j` at the machine's count), then `tools/gate.sh` once S189 has landed.
  The commit touches no `src/`, so no `Bench:` line and `gate.sh` asks for
  none.
- One real `GATE-EXTRA-DONE` run recorded in the stamp with the stage table;
  the red-first of section 6 (a) quoted; the smoke test's four cases green
  under `/bin/bash` here and under bash 5 there.
- `DEV_MANUAL.md` "Test": a subsection "The extra gate" -- the five stages,
  the markers, `JOBS`/`OUT`/`STAGES`, the detached launch and the poll loop,
  the DEC-141 cadence, the coverage recipe above; the `-j12` in its existing
  `build-debug` line is stale and may be corrected in passing. `MANUAL.md`:
  unchanged, no UCI surface moves. `adocs/specs.md`: unchanged unless the
  owner takes question 1, in which case the INV-6 row gains "and the sanitizer
  build's `bench` total equals the Release one". `README.md` untouched.
- Stamp: what each stage catches, the table, the two markers observed, the
  CMake flags added and why, the `.gitignore` finding (no change needed), and
  the coverage run if one was made.
- `plan.md` and `status.md` through the coordinator; `status.md` gains the
  cadence line -- section 10, question 6 -- with the first run's date and sha.

### 9. Sources read

- https://clang.llvm.org/docs/AddressSanitizer.html -- 2x slowdown, `-O1`,
  `-fno-omit-frame-pointer`, exit on first error, `detect_leaks` per platform.
- https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html -- recovery by
  default, `-fno-sanitize-recover`, `print_stacktrace`, the `undefined` group.
- https://clang.llvm.org/docs/LeakSanitizer.html -- platforms, cost.
- https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html -- the gcc
  wording of recovery, `-fsanitize=leak`, `--coverage`.
- https://github.com/google/sanitizers/wiki/SanitizerCommonFlags -- `exitcode`
  default 1, `abort_on_error` true on Darwin; `halt_on_error` absent
  (unverified for UBSan).
- https://github.com/google/sanitizers/wiki/AddressSanitizerFlags -- flag
  syntax; `detect_leaks` not in its table.
- https://clang.llvm.org/docs/SourceBasedCodeCoverage.html and
  https://llvm.org/docs/CommandGuide/llvm-cov.html -- the recipe's flags.
- https://raw.githubusercontent.com/official-stockfish/Stockfish/master/.github/workflows/sanitizers.yml
  -- the CI matrix, as configuration.
- Repository: `CMakeLists.txt`, `tests/CMakeLists.txt`, `src/CMakeLists.txt`,
  `cmake/arch.cmake`, `src/bitboard.cpp`, `src/search.cpp`, `src/chesso.cpp`,
  `tests/test_perft.cpp`, `tests/test_rating_script.sh`,
  `tests/test_fastchess_script.sh`, `fastchess.sh`, `rating.sh`,
  `tools/plan_prose_check.py`, `.gitignore`, `.moltke.local.md`,
  `DEV_MANUAL.md` "Build", "Test", "Detach the run", `adocs/specs.md`
  invariants table, `adocs/testing_strategy.md` 0, 3.1, R2, R10, R13,
  `adocs/audit/2026-09-04_test_review.md` (method, F01, coverage table),
  `adocs/audit/2026-08-14_test_review.md` (the 165 s / 216 s Debug timings),
  `adocs/audit/2026-08-13_adversarial.md` (the by-hand `-DSANITIZER=ON`
  RelWithDebInfo run), `adocs/data/2026-09-04_test_review/coverage_summary.txt`
  and `coverage_unexecuted.txt`, `adocs/plan_todo/S189_*`, `S190_*`, `S193_*`,
  `S196_*`, `adocs/plan_done/S161_*` (57.92 s perft), `S167_*`, `S177_*`,
  DEC-025, DEC-049, DEC-061, DEC-118, DEC-139, DEC-140, DEC-141, DEC-142.

### 10. Questions deferred to the owner

1. **Assert the sanitizer `bench` total against the Release one** inside stage
   4 (a mismatch fails the stage), or only record both in the stamp? Beyond
   `accepts:`; recommended: assert, it is a UB detector the sanitizers lack.
2. **The three mate binaries in the Debug stage.** They also drive
   `make_move` under the asserts but cost about 697 s for `test_mate_breadth`
   alone in Debug on the MacBook (`tests/CMakeLists.txt`), roughly tripling the
   stage. Recommended: no; the six stay as written.
3. **S196's full mutation pass** (about 50 minutes of machine) in the weekly
   run, as S196's guide asked? Recommended: not in the default five; an
   opt-in `STAGES=mutation` stage that calls `tools/mutation_check.py`, run
   after a change to `tests/`, with `status.md` noting when it last ran.
4. **A `tools/coverage_unexecuted.py`** (stdlib, over `llvm-cov export
   -format=text`) that reproduces `coverage_unexecuted.txt`'s format so the
   comparison is mechanical -- outside `touches:`. Recommended: yes, small;
   otherwise the `show.txt` grep in section 7 stands.
5. **The `SANITIZER` block gains `-fno-sanitize-recover=undefined` and
   `-fno-omit-frame-pointer`.** Within `touches:` and it changes no measured
   binary; stated here because it alters what the option means. Recommended:
   do it, with section 6 (a) as the observed red.
6. **Where the weekly note lives in `status.md`**: a dated clause inside the
   existing `Watching:` bullet, or its own bullet `Extra gate: last
   GATE-EXTRA-DONE <date> <sha> <mm:ss>`. Recommended: its own bullet, so a
   missed week is visible when `Watching:` reads "nothing".
