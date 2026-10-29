id:         S298
goal:       `tools/gate_extra.sh` gains a clang stage, so a compile break that only clang reports is found weekly on the workstation instead of on the owner's Mac
accepts:    the new stage configures `build-clang` (Release, with `-stdlib=libc++` as the Mac's standard library) and `build-clang-tune` (`CHESSO_TUNE=ON`) with `clang++-22` through the script's existing configure-and-verify path, and refuses a cache whose compiler is not clang, as the sanitize stage refuses its own mismatch; it builds every target and runs both fast suites; a planted clang-only warning is observed to turn the stage red before it is called done, and the plant is reverted; the stage's wall time is measured and stated in `DEV_MANUAL.md` beside the other stages
touches:    tools/gate_extra.sh, tests/test_gate_extra_script.sh, tests/CMakeLists.txt, adocs/data/S298_script_mutants.py, adocs/data/S298_mutants.txt, DEV_MANUAL.md, TOOLCHAIN.md
excludes:   adding clang to `tools/gate.sh` (every commit); changing any warning flag
decisions:  DEC-141
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30 -- stage 6 `clang` in tools/gate_extra.sh (build-clang and build-clang-tune, clang++-22 here and Apple clang's c++ on the Mac, Ninja, libc++, -k 0, both fast labels); smoke test ok and 32 of 32 cuts killed (adocs/data/S298_mutants.txt); GCC TESTS gate green, 50/50 in both builds; the planted S297 counter red with its 10 clang errors and GCC clean, reverted, then green 50/50 in both clang directories; gate_extra GATE-EXTRA-DONE 6 stages 1322 s on f91d60b plus this tree, clang 274 s warm; DEV_MANUAL.md and TOOLCHAIN.md updated, MANUAL.md checked; prose checks clean; Tier 2 not bound, no src/ change

## Why

S297 found the owner's macOS build broken by a warning that GCC 13 does not
emit and clang does, where the workstation's gate is GCC only. S297's agent
proposed a weekly stage. Its cost is two clang builds (seconds with a warm
cache) and two fast suites of about two minutes each. The coordinator took the
proposal under the owner's standing instruction to choose the recommended
option where it can (2026-10-27). Filler, behind the next strength step.

## 2026-10-29: phase a (blind)

Written in worktree `../chesso-s298`, branch `s298` at `5ac067c`, while S121's
SPRT held every core. DEC-260 forbade any build, cmake, ctest, compiler run or
`gate_extra.sh` run, so **nothing here has been executed**: the two scripts
were read and passed `bash -n`, and the mutant anchors were counted with a
Python read of the script. Everything below marked expected is a prediction.

### The design

**The stage.** `tools/gate_extra.sh` `stage_clang`, sixth and last.
`tools/gate_extra.sh` `find_clang` picks the compiler. `tools/gate_extra.sh`
`clang_half` then runs once per directory. It configures through
`tools/gate_extra.sh` `configure_and_verify`. Then
`tools/gate_extra.sh` `clang_cache_or_fail` checks the compilers. Then it
builds every target with `-- -k 0` and runs `ctest -L fast --no-tests=error`.

- `build-clang`: `-G Ninja`, Release, `CHESSO_TUNE=OFF`, and
  `-stdlib=libc++` in both `CMAKE_CXX_FLAGS` and `CMAKE_EXE_LINKER_FLAGS`.
  Those are the flags S297's `build-clang-libcxx` probe was green with.
- `build-clang-tune`: the same with `CHESSO_TUNE=ON`, libc++ included. The
  accepts names libc++ for `build-clang` only. I read `build-clang-tune` as
  `build-clang` plus the tune option, as `build-tune` is `build` plus it,
  because the Mac's tune build is libc++ too. That is my reading; one flag
  changes it.
- `CHESSO_TUNE` is passed both ways and verified, so a directory configured by
  hand with the other value comes back to this one.
- The verified strings are `CMAKE_GENERATOR:INTERNAL=Ninja`,
  `CMAKE_BUILD_TYPE:STRING=Release`, `CHESSO_TUNE:BOOL=<OFF|ON>` and both
  flags strings. They are the forms in S297's caches in `../chesso-s297`.
- The compilers go in as `-DCMAKE_C_COMPILER` and `-DCMAKE_CXX_COMPILER`, with
  absolute paths from `command -v`. S297's `CC`/`CXX` recipe is ignored by a
  directory that already exists.
- The second directory is built and tested even when the first fails, for the
  reason the driver runs every stage. The stage returns the last non-zero
  status.

**The compiler, and macOS.** It is the first of three pairs whose C++ and C
drivers are both on PATH and both answer `--version` with `clang version`:

1. `c++` and `cc`, when they are clang;
2. `clang++-22` and `clang-22`;
3. `clang++` and `clang`.

Why the default compiler comes first:

- On the Mac it is Apple clang. That is the compiler the owner's own `build/`
  uses and the one S297's break came from, and `clang++-22` does not exist
  there.
- TOOLCHAIN.md appends Homebrew LLVM to PATH on the Mac so that `c++` stays
  `/usr/bin/c++`. So the default pair is never Homebrew's clang, which
  TOOLCHAIN.md says never to point CMake at.
- On the workstation `c++` is g++ 13.3 (DEC-049). It is asked, refused, and
  `clang++-22` is taken.
- No OS test is needed. The sandbox can exercise both machines by which stubs
  it puts on PATH.

Why the other choices:

- **Both C and C++ are checked.** The project enables C and ASM
  (`project(Chesso C CXX ASM)`), and ASM takes the C compiler. S297's caches
  show `CMAKE_ASM_COMPILER` at clang-22 when C is clang-22.
- **No clang at all is red, not skipped.** It prints the six names it tried.
  A skip would be a green that compiled nothing under clang, the class the
  script exists to remove. A machine without any clang is not the macOS case
  the brief calls a defect. The coordinator confirmed this at the fast check.
- **Ninja is required.** A machine without `ninja` is refused by name before
  anything is configured. The owner's Mac builds with ninja (the pasted
  failure was ninja output), and TOOLCHAIN.md's Mac install now lists it.

**The cache refusal.** Two checks, both before anything is built.

- `configure_and_verify` greps the cache for Ninja, Release, the directory's
  `CHESSO_TUNE` and both libc++ flags strings, as it does for every stage.
- `clang_cache_or_fail` reads `CMAKE_CXX_COMPILER` and `CMAKE_C_COMPILER` out
  of the cache, with any type tag, and asks each one `--version`
  (`is_clang`). For this stage it is a backstop: the configure always passes
  both compilers by `-D`, so a cache holding anything else means CMake did not
  do what it was told. The hazards that make a gcc build directory, `CXX`
  ignored by a directory that exists and DEC-052's VS Code, belong to S297's
  hand recipe, not to this stage.

- `configure_and_verify`'s fixed strings cannot express "is clang": the path
  is `/usr/bin/clang++-22` here and `/usr/bin/c++` on the Mac.
- The output is captured, not piped into `grep -q`. Under pipefail an early
  grep exit could turn the yes into a SIGPIPE.
- The refusal message has the same two lines as `configure_and_verify`'s:
  what is wrong, then "remove ... and run again".

**The order.** Last, as stage 6.

- Stages 3 and 4 are cited by number in `DEV_MANUAL.md` and `adocs/specs.md`
  (INV-6's row says "stage 4"). Finished step files cite them too, and those
  cannot be edited. Inserting the stage earlier would renumber them.
- The script's own comment already reserved "stage 6".
- The order was not cheapest-first before this step either. In status.md's
  runs perft takes 52 to 58 s and runs after sanitize, which takes 537 to 690 s.
  DEV_MANUAL.md's "run cheapest first" is now "run in the order below".
- Expected wall time is about 400 s cold (two builds of about 60 s, two fast
  suites of about 140 s, S297's ctest logs) and about 300 s warm.

### The test: every assertion changed, and why

Existing assertions changed only where a sixth default stage forces it:

- Case 1: `GATE-EXTRA-DONE 5 stages` became `6 stages`, and its message says
  six. The default list grew, as the accepts makes it.
- Case 11's failure message: "all five stages" became "all its default
  stages". It is message text only; the grep is unchanged.
- Comments that said five say six.

The sandbox gained four compiler stubs and a `ninja` stub. `c++` and `cc`
print the workstation's g++ line; `clang++-22` and `clang-22` print a clang
line. The cmake stub now also writes the generator, `CHESSO_TUNE`, both flags
strings and both compilers into the cache it fakes, and can leave one of them
out or swap in g++ on a flag file. `run_sandbox` also clears the two clang
directories. Nothing an existing case reads changed.

Added, all strengthening:

- **Case 1:** both clang directories are configured with Ninja, Release, the
  right `CHESSO_TUNE`, the clang++-22 and clang-22 stubs (so the g++ `c++` was
  passed over) and libc++ in both flags. Each is built with `-- -k 0` and has
  its fast label run. The clang stage runs after perft, and its six commands
  run in order. The existing order check is untouched.
- **Case 2:** the marker is exactly `sanitize clang`, because the stub's red
  fast label reaches the clang directories too. `build-clang-tune` is still
  tested after `build-clang`'s fast label failed.
- **Case 12:** the cmake stub writes g++ into the cache as
  `CMAKE_CXX_COMPILER`, then in a second run as `CMAKE_C_COMPILER`. The run is
  red, the marker is exactly `clang`, the refusal names the variable, and
  nothing is built or tested in either clang directory.
- **Case 13, the Mac:** `c++` and `cc` print Apple clang's line, and
  `clang++-22` and `clang-22` are absent. The run is green with six stages,
  and both directories are configured with the `c++` and `cc` stubs.
- **Case 14, no clang:** `clang++-22` and `clang-22` are absent and `c++` is
  g++. The run is red, the marker is exactly `clang`, the output says "no clang
  on PATH", no clang directory is configured, and no "command not found"
  appears.
- **Case 15:** the compilers are right but the cache lacks one string. Three
  runs: no Ninja generator, no libc++ (in both directories), and no
  `CHESSO_TUNE=ON` (in `build-clang-tune` alone). Each run is red, the marker
  is exactly `clang`, the refusal names the missing string in that directory,
  and that directory is not built.
- **Case 16, mixed defaults:** `c++` prints Apple clang's line and `cc` the
  g++ line. The run is green, and both directories are configured with the
  clang++-22 and clang-22 stubs, because a pair is taken only when both
  halves are clang.
- **Case 17, no ninja:** the run is red, the marker is exactly `clang`, the
  output says "no ninja on PATH", and no clang directory is configured.

`adocs/data/S298_script_mutants.py` holds sixteen cuts of the stage, each
naming the case meant to kill it, as S197's file does for the first five
stages. The five added at the fast check: no fast suite (case 1), the
stage's status dropped (case 2), `expect` reduced to the build type
(case 15), the `cc` half unchecked (case 16), and no ninja check (case 17).
All sixteen anchors, and S197's sixteen, occur exactly once in the script;
counted, not run.

### The plant (phase b runs it)

S297's own error, put back. The reverse of S297's commit `e973925` on
`tools/tuner_trace.hpp` re-adds `load()`'s counter: one declaration and one
increment, never read. `git apply -R --check` takes it on this tree, at an
offset of -1 lines. GCC 13 does not flag it and clang does, as
`-Wunused-but-set-variable`. S297 measured both.

### Phase b, in order (in `/home/max/ws/chesso-s298`)

Export `CLANG_FORMAT_MAJOR=22` in the launching shell first. Detach every run
of more than a minute, and watch it with the four-exit poll loop (DEC-061),
ceiling 2x. `$out` is any directory outside the tree, for instance
`/home/max/ws/chesso/.tuning/coord/S298/phase_b`.

1. **Smoke test and cuts, about 1 min.** Expected: `ok`, then 16 of 16 and 16
   of 16 killed. Record the smoke test's own time for DEV_MANUAL.md, which
   has `about PHASE-B s` where it said 0.4 s.

   ```
   time bash tests/test_gate_extra_script.sh tools/gate_extra.sh
   python3 adocs/data/S298_script_mutants.py $out/mut298
   python3 adocs/data/S197_script_mutants.py $out/mut197
   for m in $out/mut298/*.sh $out/mut197/*.sh; do
     if bash tests/test_gate_extra_script.sh "$m" > /dev/null 2>&1; then
       echo "SURVIVED $m"; else echo "killed $m"; fi
   done
   ```

2. **The GCC TESTS gate, about 8 min cold.** The worktree has no build
   directories. Configure `build` and `build-tune` as `DEV_MANUAL.md`
   "Build" says, then run AGENTS.md's TESTS command. This puts the new test
   under ctest in both GCC builds.
3. **The plant, red, about 3 min cold.** No fast suite runs, because the
   builds fail.

   ```
   git show e973925 -- tools/tuner_trace.hpp | git apply -R
   STAGES=clang OUT=$out/plant nohup tools/gate_extra.sh > $out/plant.log 2>&1 &
   ```

   Expected: `GATE-EXTRA-FAILED: clang`. `01_clang.log` should show (the
   subset numbers its logs from 01)
   `-Wunused-but-set-variable` on the counter in `load()` in both
   directories, in the five units S297 listed. That is ten diagnostics in
   all, and no `ctest` line.
   3b, optional and about 30 s: `cmake --build build -j12 && cmake --build
   build-tune -j12` with the plant still in place should exit 0. That shows
   the plant is clang-only on this tree.
4. **Revert, green, about 5 min.** Run
   `git checkout -- tools/tuner_trace.hpp` and confirm
   `git diff --quiet -- tools/tuner_trace.hpp`. Then repeat step 3's command
   with `OUT=$out/green` and `$out/green.log`. Expected:
   `GATE-EXTRA-DONE 1 stages`, and both fast labels pass whole (50 of 50 at
   S297).
5. **The full gate, about 23 min.** Run
   `OUT=$out/full nohup tools/gate_extra.sh > $out/full.log 2>&1 &`.
   Expected: `GATE-EXTRA-DONE 6 stages`. Its summary table gives the six wall
   times. The clang time is warm: step 4 left both directories built, through
   ccache, so it is mostly the two fast suites. DEV_MANUAL.md says so.
   `PHASE-B-SHA` names `5ac067c` with S298's changes uncommitted on top.
6. **Fill the placeholders.** Run `grep -n PHASE-B DEV_MANUAL.md`. It finds
   the six stage times, the date and sha of step 5's run, the smoke-test
   time, and the cut count. Then write this file's evidence and run
   `python3 tools/plan_prose_check.py`, all five modes. The citations mode is
   in the fast label too, so run it after the last edit to this file.

That is about 40 min of machine time in all, under DEC-155's four hours.

### Unverified

- **The test has never run.** Step 1 is the first execution of the test and
  of any of the stage's code. The 32 cuts, these 16 and S197's 16, were only
  written out and passed through `bash -n`; none is byte-identical to the
  script.
- **CMake's real cache lines** for compilers given by `-D`. The sed takes
  any type tag, but the lines were not observed for this configure. Other
  CMake behaviour was not observed either: `-G Ninja` against an existing
  Makefiles directory should fail the configure, and a `-D` compiler different
  from the cached one should make CMake reset the cache. The stage relies on
  neither, because the check after the configure is what refuses.
- **libc++ at this tree.** S297's libc++ probe was green at `5a9b931`. This
  tree has S121's three commits on top, and S121's "clang pair" was the
  libstdc++ one.
- **The Mac.** Apple clang was never run. The pair it takes, `/usr/bin/c++`
  and `/usr/bin/cc`, is inferred from TOOLCHAIN.md. Its `--version` line is
  assumed to contain `clang version`, as "Apple clang version 17.0.0" does;
  the stub prints that. Bash 3.2 compatibility was read, not run.
- **Every time above is an estimate** from S297's logs and status.md's
  gate_extra runs.
- **Two probes outside the brief's list.** Once, early, I ran
  `clang++-22 --version` and `c++ --version`. Each starts a process for a
  few milliseconds and compiles nothing; they confirm the two version strings
  the stubs copy.

### Left for the landing

- `AGENTS.md`'s TESTS rule lists gate_extra's contents ("the Debug binaries, a
  sanitizer build, deep perft, the prose and citation checks") without the
  clang stage. That list is now incomplete, not false. It is left alone,
  as the coordinator ruled.
- The coordinator edits `.moltke.local.md`'s "Do not build with clang" note at
  the landing.

## 2026-10-29/30: phase b (the machine free after S121's verdict)

Run in a fresh worktree, `../chesso-s298b` on branch `s298b` at `f91d60b`
(main's HEAD after S121 landed). Phase a's saved diff was checked against its
sha256 (`b4945fe2...`) and applied with `git apply --index --3way`. It applied
cleanly. S121's three later commits changed `DEV_MANUAL.md` and
`tools/tuner_trace.hpp` elsewhere; the plant still applies at -1 lines.
`../chesso-s298` was left as it was. Every run had `CLANG_FORMAT_MAJOR=22`
exported. The logs are under `.tuning/coord/S298/phase_b/` in the main
checkout.

1. **Smoke test: `ok`, about 5.5 s.** It read 5.56 s alone (`smoke.log`).
   At the fast check, three interleaved runs read 5.77, 5.50 and 5.82 s, and
   the eleven-case test against `f91d60b`'s script read 1.01, 1.80 and 1.80 s
   on the same machine (`smoke_timing.log`). Under ctest, the logs in
   `phase_b/` give 4.98 to 6.01 s for it. So `DEV_MANUAL.md`'s old 0.4 s was
   not this machine's figure. The growth is about twice as many sandbox runs,
   each with six stages and the compiler stubs. That is well inside the
   test's 60 s timeout.
2. **Cuts: 32 of 32 killed, none survived.** Each of S298's 16 is killed by
   the case its comment names, among others; S197's 16 are all still killed.
   The kill list, with the cases each one turned red, is
   `adocs/data/S298_mutants.txt`.
3. **GCC TESTS gate: green, 328 s cold.** It configured `build` and
   `build-tune`, ran AGENTS.md's TESTS command, and passed 50 of 50 in each
   (the new test under ctest in both GCC builds), with the format check clean.
4. **The plant, red: `GATE-EXTRA-FAILED: clang`, 82 s cold.** S297's counter
   was put back with `git show e973925 -- tools/tuner_trace.hpp | git apply
   -R` (two lines). `01_clang.log` holds exactly 10 errors, all
   `variable 'line_number' set but not used [-Werror,-Wunused-but-set-variable]`,
   and no other error. They are in the five units S297 listed
   (`test_eval_trace`, `test_tuner_gradient`, `tuner`, `truncation_scan`,
   `feature_audit`), once in each directory. Both directories ended with
   "ninja: build stopped: cannot make progress due to previous errors". No
   ctest ran.
   **3b:** with the plant still in place, `cmake --build build` and
   `cmake --build build-tune` recompiled the same ten objects under g++ 13.3
   with no warning or error in `plant_gcc.log`, in 26 s. That run's exit
   status was not logged on its own. At the fast check it was re-run with the
   plant applied again: "build exit 0, build-tune exit 0" (ccache hits, 0 s),
   10 objects rebuilt, no warning or error, and the plant reverted with a
   clean `git diff --quiet` (`plant_gcc_rerun.log`). So the plant is
   clang-only on this tree.
5. **The revert, green: `GATE-EXTRA-DONE 1 stages 259 s`.**
   `git checkout -- tools/tuner_trace.hpp` was followed by a clean
   `git diff --quiet` on that file. Both clang fast labels passed 50 of 50
   (117 s and 118 s), with no warning in the log. Both binaries link
   `libc++.so.1` and `libc++abi.so.1`.
   The cache read back `CMAKE_CXX_COMPILER:UNINITIALIZED=/usr/bin/clang++-22`
   and the same type for C, which settles phase a's open question about the
   type tag: the sed's any-type match is load-bearing. `CMAKE_ASM_COMPILER` is
   `/usr/bin/clang-22`, and the generator, flags and `CHESSO_TUNE` lines are
   the strings the verify greps for.
6. **The full gate: `GATE-EXTRA-DONE 6 stages 1322 s`.** It was detached at
   2026-10-30 00:00 under a four-exit watcher. The stage times were prose 0 s,
   citations 0 s, debug 364 s (6 of 6), sanitize 632 s (50 of 50, "INV-6
   across builds: both 6016079 nodes", the `Bench:` of S121's `e4ec786`),
   perft 52 s (1 of 1) and clang 274 s. The clang time is warm: step 5 had
   left both directories built, so it is the two fast suites, 50 of 50 each
   in 137 s apiece. The numbers are in `DEV_MANUAL.md`'s table, with the run
   named as `f91d60b` plus S298's uncommitted changes.
7. **The placeholders** in `DEV_MANUAL.md` were filled from steps 1, 2 and 6.
   `grep PHASE-B` finds none.

**The accepts, clause by clause.**

- **The two directories, configured as asked.** The stage configures
  `build-clang` (Release, `-stdlib=libc++`) and `build-clang-tune`
  (`CHESSO_TUNE=ON`) with `clang++-22` through `configure_and_verify`. On
  the workstation it took `/usr/bin/clang++-22` and `/usr/bin/clang-22`
  (step 4's log). The caches carry Ninja, Release, `CHESSO_TUNE`, the libc++
  strings and clang compilers (step 5). Case 1 holds the commands.
- **A non-clang cache is refused, as sanitize refuses its mismatch.** Case
  12 covers a g++ C++ and a g++ C compiler; case 15 covers a missing
  generator, libc++ or `CHESSO_TUNE=ON`. Cuts N2, N3 and N14 are killed by
  them.
- **Every target built, both fast suites run.** The builds use `-k 0`.
  Step 4 shows every error in one pass, and steps 5 and 6 pass 50 of 50 in
  each directory.
- **The plant turns the stage red, then is reverted.** Step 4 is red with
  exactly the planted error, and GCC is clean with the plant in place. The
  plant was reverted and checked before step 5.
- **The wall time is in `DEV_MANUAL.md` beside the other stages.** It is a
  new column holding all six stage times from one run, the clang time
  labelled warm.

**Checks.**

- Tier 2 (DEC-141) does not bind, because nothing in `src/` changed. The full
  `gate_extra` run above is green anyway.
- `DEV_MANUAL.md`: updated in the Build section, the extra gate's table and
  text, and the test's description. `MANUAL.md`: checked, with no change; it
  mentions neither gate_extra nor clang, and the UCI surface is untouched.
- `TOOLCHAIN.md`: updated, with ninja in the Mac install and the scratch
  check named as stage 6.
- `tools/plan_prose_check.py` was run one mode per call after the last edit
  to this file, all five exiting 0 (`phase_b/prose.log`).

