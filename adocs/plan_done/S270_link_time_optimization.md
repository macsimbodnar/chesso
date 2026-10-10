id:         S270
goal:       the engine is built with link-time optimization if a counter-based interleaved timing on the workstation shows it faster, and `game_tables()` becomes an inline accessor
accepts:    (1) an A/A of two copies of the parent's binary first, with `perf stat -e instructions,cycles`, to read the counter's own noise; (2) the parent rebuilt with CMake's `INTERPROCEDURAL_OPTIMIZATION`, timed interleaved against it on the same counters and on nps; (3) enabled for the engine targets only if it holds, in both gated builds, with `-Werror` still on and the PGO path in `cmake/pgo.cmake` checked to build; (4) `src/bitboard.cpp` `game_tables` moved inline into `src/bitboard.hpp` as a separate change, timed on its own (CLAUDE.md rule 6); (5) **node identity** (INV-6) for both changes: `bench` and `tools/search_bench.py` identical to the parent at two depths; (6) `DEV_MANUAL.md` states the build flags as they then are, and the LTO remark in `src/evaluation.hpp` follows the result
touches:    CMakeLists.txt, src/CMakeLists.txt, cmake/pgo.cmake, src/bitboard.cpp, src/bitboard.hpp, src/evaluation.hpp, DEV_MANUAL.md
excludes:   moving definitions between translation units by hand beyond `game_tables`; compiler or flag changes other than LTO
decisions:  DEC-083, DEC-263
closes:     2026-10-08_performance-F03
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-10 -- ZERO, BOTH CHANGES REVERTED, the build is as it was. LTO (CMake INTERPROCEDURAL_OPTIMIZATION, -flto=auto, g++ 13.3) node-identical (bench 4081329 and bench 12 1860699 streams, search_bench 32932 / 70095 / 25178 and 67792 / 280873 / 137893 identical) and SLOWER: +0.91 / +1.13 % instructions, +1.55 / +1.57 % cycles, -1.59 % wall (CI -1.75 .. -1.44, 0/24); build +17 % (53.9 s vs 46.1 s, -j12, no ccache). game_tables() inline node-identical, -0.4 % instructions, +1.7 to +2.5 % cycles, -1.62 % wall (CI -1.81 .. -1.42, 0/24); with -Wa,-mbranches-within-32B-boundaries on both sides +0.13 / -0.12 % cycles, CIs across zero: padded on both sides it reads zero, no gain in any layout measured. A/A clean in both sessions (hyperfine +0.11 % and +0.17 %, instructions 0.000 %); the same base binary drifted -1.6 % cycles between two runs 15 minutes apart, a drift each pair cancels. FOUND: cycles depend on code layout by about 2 % on this Coffee Lake (JCC erratum inferred, not read), invisible to an A/A; the padded parent read -1.1 to -1.3 % cycles with +0.5 % instructions in one layout, not established, for S273; -Werror does not reach LTO link-time warnings unless the link line repeats it (two-file case logged, planted mismatch in the tree). Recipes in adocs/data/S270_lto.diff and S270_inline.diff, readings in adocs/data/S270_timing.txt. Suites 43/43 in build and build-tune on fresh configures, format clean (DEC-146), Debug builds, prose checks clean. No second tier owed: nothing in src/ moved but a comment. Fast check: six wording and evidence findings (nm addresses, the OOM's timing, INV-6 scope of the padded sides, padding overreach, A/A scope and drift, the unlogged -Werror half), all fixed before commit; the two-file -Werror case re-run and logged.

## Why

No build enables LTO, so every cross-unit call on the search path --
`tt_get_entry`, `tt_store_entry`, `score_move`, `see_ge`, `is_check`,
`make_move`, `game_tables()` -- stays out of line. S104 recorded LTO as inside
the noise from two wall-clock runs each, an instrument whose resolution was
coarser than the effect (CLAUDE.md rules 4 and 5). On the M1, with cycle
counters, an LTO build was node-identical and measured -1.2 to -1.7 % cycles.
The workstation's gcc has not been measured.
`adocs/audit/2026-10-08_performance.md`, F03.

## Lane

Build work on the workstation, where it is timed; no match (DEC-083).

## As built (2026-10-08 to 2026-10-10, Opus subagent)

### Deviations, first

1. **Neither change is kept, so the build is as it was.** LTO measured slower
   and the inline accessor measured slower; both were reverted, as the brief
   says for a change that does not hold. What landed is the record: this file,
   `adocs/data/S270_timing.txt`, the two reverted changes as
   `adocs/data/S270_lto.diff` and `adocs/data/S270_inline.diff` (the LTO one
   with its comment's "faster" corrected to the measurement; both apply to
   `b3bcca5` with `git apply`), one paragraph in `DEV_MANUAL.md` and one clause
   of the remark in `src/evaluation.hpp`. `CMakeLists.txt` and
   `cmake/pgo.cmake` were never edited; `src/CMakeLists.txt`,
   `src/bitboard.cpp` and `src/bitboard.hpp` were edited and restored.
2. **Accepts (3)'s PGO and macOS checks were not run**: they apply only to an
   LTO that holds. The reverted diff carries the guard they asked for
   (`check_ipo_supported`, LTO off and said so when the toolchain cannot).
3. **Two diagnostics the brief did not list**, measurement only, nothing
   kept, each started because a reading needed explaining: a single-partition
   LTO link (`part1`), and both sides of the inline rebuilt with the
   assembler's 32-byte branch padding (`_jcc`). Both are the subject of the
   surprises below.
4. **Two sessions.** The host terminal was OOM-killed on 2026-10-08 at 23:54
   (its own memory growth, 12.4 GB, not a build) and the work stopped again on
   an API limit. The A/A and the LTO timing ran on 2026-10-09 at 00:09; the
   inline ran on 2026-10-10 at 19:50 behind a second A/A of its own. Every
   LTO build after the crash ran under `memwatch.sh`, which records peak use
   and kills the build if available memory falls under 3 GB; it never fired.
5. **The instrument is `tools/perf_counters`** (S268), `perf` being absent.

### What this step found, ahead of its two verdicts

**Code layout moves cycles here by about 2 %, and the A/A cannot see it.**
The inline accessor removes one function and 32 bytes of `.text`; the same
functions stay out of line (`nm`), but not at the same addresses: 140 of 352
shared text symbols moved, 102 by +32, 25 by +16, 7 by -32, 5 by -16 and 1
by +48 (`.tuning/coord/S270/symshift.py`, nm types t, T, w, W), and each
+-16 move changes that function's 32-byte phase. It reads -0.4 %
instructions on both workloads and **+1.7 to +2.5 % cycles**, 0 of 24
hyperfine pairs faster. Built with `-Wa,-mbranches-within-32B-boundaries` on
both sides -- the assembler pads so no jump crosses or ends on a 32-byte
boundary, which is the trigger of the Skylake-family JCC erratum this i7-8700K
carries -- the same change reads **+0.13 % and -0.12 % cycles, both CIs
across zero**. Padding gives one other layout, not none, so this shows the
cost depends on layout; it does not show the padded layout is the true one.
The A/A floor (0.2 to 0.4 %) bounds the noise between the pairs of one run,
not layout: two byte copies share one layout. Nor does it bound drift between
runs: the same parent binary read 1.0402e10 cycles on `bench 16` in the
inline run and 1.0232e10 in the `part1` run about 15 minutes later, -1.6 %
(-1.3 % on the 300 positions, nps +3.8 %); paired verdicts are unaffected,
since both sides of a pair share the drift. So on this machine a cycle or wall
change under about 2 % with no instruction change behind it is not
attributable to the change; one with instructions behind it (S268's -11.4 %,
S269's -2.0 %) is, but its size can carry a layout term. The mechanism is
inferred, not read: `perf_counters` has no front-end or DSB counter.

**The padded parent read faster than the parent, once**: +0.48 / +0.50 %
instructions (the pad bytes), -1.31 / -1.12 % cycles, -1.29 / -1.46 % wall
on the counters' own clock. By the rule just above this is not established:
instructions moved the other way and it is one layout of one binary. A step
of its own would have to establish it (proposed below); not this step's
(`excludes:` flag changes other than LTO).

### The A/A

Parent `b3bcca5` against a byte copy (`cmp`). Session 1: instructions 0.000 %
on both workloads; cycles -1.29 % (CI -3.03 .. +0.48, per-pair sd 2.8 %) on
`bench 16`, firefox having been busy a minute before, and -0.43 % (CI -1.64 ..
+0.79) on the 300 positions; hyperfine +0.11 % (CI -0.11 .. +0.32), 16 pairs.
Session 2: instructions 0.000 %; cycles -0.22 % (CI -1.12 .. +0.69) and -0.22 %
(CI -0.48 .. +0.04); hyperfine +0.17 % (CI -0.04 .. +0.38). `bench_movegen -r
40` resolution 0.5 / 1.9 / 0.7 % in session 1, 0.1 / 0.1 / 0.1 % in session 2.
Governor `powersave`, recorded not set (DEC-195).

### Change 1: LTO -- REVERTED

`src/CMakeLists.txt` only (`adocs/data/S270_lto.diff`): option `CHESSO_LTO`,
default on; off in Debug and the sanitizer build; `check_ipo_supported` first;
`INTERPROCEDURAL_OPTIMIZATION` on `chesso_engine` and `chesso`, which g++ 13
gets from CMake 3.28 as `-flto=auto -fno-fat-lto-objects` with `gcc-ar-13`
and `gcc-ranlib-13` for the archive. With bytecode-only objects every target
that links the library (every test and tool) takes the link-time step too.

**INV-6**: `bench` 4081329 and `bench 12` 1860699, every line identical (121
and 105); `search_bench` 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 /
137893 at depth 12, `c3d5` / `e2a6` / `d7c8q` and `c3d5` / `d5e6` / `d7c8q`,
identical.

**Timing**: instructions **+0.909 %** (`bench 16`, 16 pairs) and **+1.126 %**
(300 positions, 12 pairs), every pair; cycles +1.55 % (CI +0.14 .. +2.97) and
**+1.57 % (CI +1.48 .. +1.66)**; nps 4257558 -> 4190802; hyperfine **-1.59 %
(CI -1.75 .. -1.44), 0 of 24 pairs faster**. More instructions, so this is a
property of the code and not of its layout. The M1's clang read -1.45 %
instructions on the same change (audit F03); g++ 13 does not transfer it.

**Why, partly**: the same objects linked once with `-flto-partition=one`
(by hand, no CMake) read -0.18 / -0.07 % instructions, -0.75 / -1.07 % cycles
and **+1.01 % wall (CI +0.85 .. +1.17), 24 of 24**, node-identical. So gcc's
balanced partitioning (12 under `-flto=auto`) costs about 1 % of instructions
here; the out-of-line function sets of the two links differ by 35 and 26
names and the cause was not taken further. The +1.01 % has no instruction
change behind it, so by the layout finding above it is not attributed.

**Keep-or-revert: REVERTED** on +0.9 to +1.1 % instructions and -1.59 % wall.

**`-Werror` and the link-time step**, for whoever enables LTO next. Under LTO
the optimizer and its diagnostics run again at the link, and
`add_compile_options()` never reaches the link line. Observed with g++ 13: two
units disagreeing on a variable's type, linked with CMake's own LTO line,
printed `-Wlto-type-mismatch` and exited 0; with `-Werror` on the link they
failed (both halves logged in `.tuning/coord/S270/werror_link_repro.log`, a
re-run of the first observation, which was not logged). The diff therefore puts `-Wall -Wextra -Werror` on the link line as an
`INTERFACE` option of `chesso_engine`, and with it the same mismatch planted
in `src/main.cpp` and `src/utils.cpp` stopped the build (`-Werror=lto-type-mismatch`,
exit 2); removed after. The tree linked with 0 warnings under it.

**Build cost**, measured from clean with `CCACHE_DISABLE=1`: the full tree at
`-j12` 53.9 s with LTO against 46.1 s without (+17 %); `--target chesso` alone
4.6 s with LTO. Memory, after the terminal crash: peak used 7.9 GB with LTO
(lowest available 8.0 GB, at most 14 `lto1` processes at once) against 8.1 GB
without; the `chesso` link alone +0.85 GB. The LTO build was not the memory
risk; the tests' compiles dominate either way.

### Change 2: `game_tables()` inline -- REVERTED

`src/bitboard.hpp` defines it inline over two `extern` declarations; the
storage in `src/bitboard.cpp` loses `static`
(`adocs/data/S270_inline.diff`). Timed on the non-LTO parent, the tree it
would have landed on once LTO was reverted -- the order the brief asked to be
stated.

**INV-6**: identical, every stream and count above.

**Timing**: instructions **-0.405 %** and **-0.367 %**, every pair; cycles
**+2.47 % (CI +1.95 .. +2.99)** and **+1.66 % (CI +1.43 .. +1.89)**; nps
4404448 -> 4295869; hyperfine **-1.62 % (CI -1.81 .. -1.42), 0 of 24**. Under
the 32-byte padding on both sides: instructions -0.407 / -0.388 %, cycles
**+0.13 % (CI -0.33 .. +0.58) and -0.12 % (CI -0.34 .. +0.09)**: padded on
both sides, it reads zero. The cost depends on layout, and no layout measured
shows a gain.

**Keep-or-revert: REVERTED.** In the layout the tree builds it is 1.6 %
slower, and padded on both sides it reads zero. Keeping it for the
non-LTO toolchains would need a gain to cite, and there is none.

### Suites

On fresh configures of both trees, the final tree (the record only):
`cmake --build build -j12 && ctest --test-dir build -L fast
--output-on-failure && cmake --build build-tune -j12 && ctest --test-dir
build-tune -L fast --output-on-failure && ./clang-format.sh --check` under
`CLANG_FORMAT_MAJOR=22` (DEC-146): **43/43, 43/43, format clean**, exit 0, 0
warnings (`.tuning/coord/S270/gate.log`). A Debug configure builds, 0
warnings. `bench` 4081329 in both builds; the final binary's `bench 16`
instruction count is the parent's to 77 in 19.1 G, inside the counter's own
repeat. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`,
one mode per call: each exit 0, 0 flagged. No `fastchess`, no `gate_extra`.

`MANUAL.md` checked: no option, output or behaviour moved -- no change.
`DEV_MANUAL.md`: the Build section now states the compile flags, traced to the
CMake that sets each, and that there is no LTO, with the measurement and the
build-time line. `specs.md` names no build flag -- no change proposed.

### Proposed follow-ups (the coordinator's and the owner's to decide)

- **The 32-byte branch padding as its own step (S273)**: one layout of one
  binary read -1.1 to -1.3 % cycles with +0.5 % instructions, which by this
  step's own rule is not established; S273 has to establish it, over more than
  one layout. The padded binaries matched the parent's `bench`, `bench 16`
  and 300-position totals here; `inv6.sh` was not run on them. The erratum
  is the Skylake family's (this machine), so whether it belongs in what ships
  as well as in `build/` depends on the parts the shipped targets run on.
- **A layout-robust reading for speed steps on this machine** before the next
  sub-2 % verdict: time both sides padded, or require an instruction change in
  the same direction. S268 and S269 stand on their instruction counts.
- **Single-partition LTO** re-measured under that reading, PGO pairing
  included, if anyone wants LTO back: +1.01 % wall here with instructions flat.

### Proposed `done:` stamp

2026-10-10 -- ZERO, BOTH CHANGES REVERTED, the build is as it was.
LTO (CMake INTERPROCEDURAL_OPTIMIZATION, -flto=auto, g++ 13.3) node-identical
(bench 4081329 and bench 12 1860699 streams, search_bench 32932 / 70095 /
25178 and 67792 / 280873 / 137893 identical) and SLOWER: +0.91 / +1.13 %
instructions, +1.55 / +1.57 % cycles, -1.59 % wall (CI -1.75 .. -1.44, 0/24);
build +17 % (53.9 s vs 46.1 s, -j12, no ccache). game_tables() inline
node-identical, -0.4 % instructions, +1.7 to +2.5 % cycles, -1.62 % wall (CI
-1.81 .. -1.42, 0/24); with -Wa,-mbranches-within-32B-boundaries on both sides
+0.13 / -0.12 % cycles, CIs across zero: padded on both sides it reads zero,
no gain in any layout measured. A/A clean in both sessions (hyperfine +0.11 %
and +0.17 %, instructions 0.000 %); the same base binary drifted -1.6 % cycles
between two runs 15 minutes apart, a drift each pair cancels.
FOUND: cycles depend on code layout by about 2 % on this Coffee Lake (JCC
erratum inferred, not read), invisible to an A/A; the padded parent read -1.1
to -1.3 % cycles with +0.5 % instructions in one layout, not established, for
S273; -Werror does not reach LTO link-time warnings unless the link line
repeats it (two-file case logged, planted mismatch in the tree). Recipes in
adocs/data/S270_lto.diff and S270_inline.diff, readings in
adocs/data/S270_timing.txt. Suites 43/43 in build and build-tune on fresh
configures, format clean (DEC-146), Debug builds, prose checks clean. No
second tier owed: nothing in src/ moved but a comment.
