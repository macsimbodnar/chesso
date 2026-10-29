id:         S297
goal:       the tree builds clean under clang with the project's -Wall -Wextra -Werror, as it does under GCC, starting with the set-but-unused `line_number` in `tools/tuner_trace.hpp` that stops the owner's macOS arm64 build
accepts:    the owner's error is reproduced on the workstation with `clang++-22` before any fix, observed and not assumed; a Release and a tune build configured with clang++-22 and the project's flags then build every target, and both fast suites pass under them; each warning is fixed at its cause, never silenced by a pragma, a flag or a cast to void that hides a real defect; the GCC builds and `tools/gate.sh` stay green; bench is unchanged (`No functional change`), with `tools/search_bench.py` identical if any src/ line moves (INV-6); `TOOLCHAIN.md` or `DEV_MANUAL.md` says how to run the clang check
touches:    tools/tuner_trace.hpp, DEV_MANUAL.md, TOOLCHAIN.md
excludes:   adding a clang build to `tools/gate.sh` or `tools/gate_extra.sh` (proposed only, the owner decides); any change of warning flags
decisions:  DEC-118, DEC-140, DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent (S297 brief), 2026-10-29
done:       2026-10-29 -- clang++-22 reproduced the owner's error in load() and found no other clang-only error; line_number removed, no message changed; after its last write, clean -k 0 builds of build-clang, build-clang-tune and a libc++ probe build every target and pass fast 50/50 (logs under .tuning/coord/S297/logs); GCC TESTS gate and gate.sh --message green, bench 5069022 in both builds, no src/ change; prose checks clean; Tier 2 not bound; DEV_MANUAL.md and TOOLCHAIN.md updated, MANUAL.md checked

## Why

Reported by the owner on 2026-10-29 from a MacBook Air M5 on the latest macOS:
`ninja` in `build/` stops at `tests/test_eval_trace.cpp` and
`tests/test_tuner_gradient.cpp`, both including `tools/tuner_trace.hpp`, at
`load()`'s counter `line_number`:

    error: variable 'line_number' set but not used [-Werror,-Wunused-but-set-variable]
          uint64_t line_number = 0;

The variable is incremented per line read and never read. GCC 13 does not
warn on a variable that is only incremented, and clang does, so the
workstation's GCC gate never saw it. The build stopped there, so other
clang-only warnings may lie behind it. A clang build of the whole tree on the
workstation finds them all at once.

DEC-171's scope: a build failure on the owner's machine is not reachable in
play, so it is not BUGS-binding. The owner asked for it as an extra job, to be
done as soon as the machine allows.

## 2026-10-29: reproduced, fixed and proved (the implementer, one Opus 5.5 subagent)

Worktree `../chesso-s297`, branch `s297` at `5a9b931`.

**Reproduced before any change.** Two scratch builds, `build-clang` (Release)
and `build-clang-tune` (Release, `CHESSO_TUNE=ON`), both
`CC=clang-22 CXX=clang++-22`, Ninja and ccache, the project's flags and nothing
added. Configure printed `Branch padding unavailable: Clang 22.1.8 rejected
...`, the guard `DEV_MANUAL.md` describes. `cmake --build <dir> -j12 -- -k 0`
exited 1 in each build with exactly one diagnostic: the owner's
`variable 'line_number' set but not used [-Werror,-Wunused-but-set-variable]`,
in `load()`. It stopped the five units that include the header, the same in
both builds: `tests/test_eval_trace.cpp`, `tests/test_tuner_gradient.cpp`,
`tools/tuner.cpp`, `tools/truncation_scan.cpp` and `tools/feature_audit.cpp`.
Every other unit compiled with no warning of any kind, and no target is
`EXCLUDE_FROM_ALL`, so `all` is every target. There was no other clang-only
error behind it. The build output is kept, outside the repository, in
`.tuning/coord/S297/logs/repro_build-clang.log` and
`repro_build-clang-tune.log` in the main checkout.

**The fix.** Removed `load()`'s `line_number`: the declaration and the
increment. It came in with S262 (`bee84d8`) and was never read, and the loader
S262 replaced never reported a line number either. I removed it rather than
putting it into the message, for three reasons. The failure message already
quotes the failing line's text. Failure depends on that text alone (it goes
through `parse_line()`, `load_FEN()` and `trace_board()`). And the loader
reports the earliest failing line, so `grep -n -m1 -Fx` recovers the number.
No message changes. No test in `tests/` compares a `load()` message; the two
that call it in `tests/test_tuner_gradient.cpp` read only its `bool`. No
pragma, flag or cast. No `src/` file moved.

**Docs.** `DEV_MANUAL.md`'s Build section gains the clang check: the commands,
and why it exists (GCC 13 does not flag a variable that is only incremented,
clang does, and the owner's macOS build is clang). `TOOLCHAIN.md`'s "Do not
point CMake at `clang++-22`" note now names its two exceptions, neither
measured: the coverage build and this scratch check. Its `clang++-22` row now
reads "never the measured build compiler". `MANUAL.md` checked: no change,
because the UCI surface is untouched.

**Evidence.**

- clang, after the last write of `tools/tuner_trace.hpp`:
  `cmake --build <dir> --clean-first -j12 -- -k 0` exits 0 in `build-clang`,
  `build-clang-tune` and `build-clang-libcxx`. Each runs 113 of 113 edges and
  prints no diagnostic. With `CLANG_FORMAT_MAJOR=22` exported, `ctest -L fast`
  then passes 50/50 in each (138 s, 139 s, 141 s). The logs are the
  `after_<dir>.build.log` and `after_<dir>.ctest.log` files beside the
  reproduction's.
- The first clang suite run failed two tests. `test_clang_format_script`
  failed because `CLANG_FORMAT_MAJOR=22` was not exported, an environment
  fault (DEC-146). `test_plan_citation_freshness` failed on this file's quoted
  compiler line, a line-form citation (DEC-135); the coordinator re-worded the
  quote, and both now pass.
- GCC: the TESTS command at `-j12` exits 0. Both suites pass 50/50 and the
  format check is clean.
- Bench is 5069022 in `build/` and in `build-tune/`, the parent's (the last
  commit to touch `src/` is S296's `25b332b`). No `src/` line moved, so INV-6's
  `tools/search_bench.py` comparison is not owed.
- `tools/plan_prose_check.py` `--citations`, `--touches`, `--params` and
  `--gate`: exit 0 each.
- Tier 2 (DEC-141) does not bind: the search, the generator and make/unmake
  are untouched.
- `tools/gate.sh --message` over the staged tree, with the message ending
  `No functional change`, printed `GATE-DONE 5069022 (no functional change)`.
- A probe beyond the brief, `build-clang-libcxx`: Release with
  `-stdlib=libc++` in the compile and link flags. Apple clang uses libc++,
  and clang++-22 here compiles against GCC's libstdc++ 14 by default. It
  is in the clang run above, and its binary links `libc++.so.1`. That is
  nearer the owner's toolchain, not the same: Apple clang and the macOS SDK
  were not run.
