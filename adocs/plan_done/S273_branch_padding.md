id:         S273
goal:       the workstation's builds pad branches off 32-byte boundaries if a layout-robust timing shows it faster, and a speed verdict under about 2 % on this machine gets a reading that code layout cannot fake
accepts:    (1) an A/A of the parent and a byte copy, then the parent built with `-Wa,-mbranches-within-32B-boundaries` (g++ 13.3, GNU as) timed interleaved against the parent on `tools/perf_counters` (instructions and cycles) and hyperfine wall, S270's two workloads, node identity (INV-6: `bench` total and stream, `bench 12`, `tools/search_bench.py` at depths 9 and 12); (2) the reading is robust to layout: the same comparison repeated over at least two unrelated layout perturbations of both sides (for example a dummy function or `-falign-functions` changes in an unrelated unit), each read the same way, because S270 found layout alone moves cycles about 2 % here while an A/A of byte copies shows none; (3) if it holds, the flag is set in CMake for the `CHESSO_ARCH` values whose targets are the Skylake family's (at least `native` on this machine), guarded so a toolchain whose assembler lacks it (Apple clang) still builds, both gated builds and a Debug configure green, `-Werror` clean, `cmake/pgo.cmake`'s path built once; if it does not hold, the zero is recorded and the build left as it is; (4) the step file states a proposed reading rule for sub-2 % speed verdicts on this machine (both sides padded, or instructions moving the same way as cycles, or the perturbation repeat of (2)), as a proposal for the owner to rule on as a decision; (5) `DEV_MANUAL.md`'s Build section states the flags as they then are, traced to the CMake that sets each (DEC-017)
touches:    cmake/arch.cmake, CMakeLists.txt, DEV_MANUAL.md, adocs/data/S273_timing.txt, fastchess.sh, tests/test_fastchess_script.sh
excludes:   any change to src/; LTO (S270's zero stands; single-partition LTO is a separate question); setting the CPU governor (DEC-195)
decisions:  DEC-083, DEC-017, DEC-118, DEC-195, DEC-263
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-11 -- KEPT: -Wa,-mbranches-within-32B-boundaries on C++ units for CHESSO_ARCH native and bmi2 (cmake/arch.cmake, check_cxx_compiler_flag-guarded; a rejecting toolchain builds without it and says so, observed with clang 22; Apple clang not run). Padded vs unpadded in the tree's layout and four perturbations (-falign-functions=64, a dummy function per unit, S270's game_tables() diff applied, link order reversed), single-process: -1.5 to -3.7 % cycles for +0.5 % instructions, hyperfine +1.50 to +3.11 %, every half the same sign; the PGO bmi2 build the same sign at a similar size on its clean bench 16 half (-3.56 %), its other half disturbed. DSB / (DSB + MITE) uop share 23 to 36 % unpadded against 42 to 44 % padded, consistent with the JCC erratum (IDQ raw events, encodings typed from Intel's list and not cross-checked here; microcode-sequencer uops left out). Unrelated layout changes moved cycles up to 1.8 % unpadded and 0.3 % padded, inside the A/A floor (0.56 % on bench 16); S270's inline reads +1.7 to +2.3 % cycles unpadded and +0.2 / +0.3 % (wall 0.00 %) padded for -0.4 % instructions: padding does not make cycles follow instructions. Unmeasured: bmi2's cost on parts without the erratum, and a full-concurrency match. fastchess.sh's banner prints each side's padding from its CMake cache (CHESSO_BRANCH_PAD; unrecorded = before S273), property 32 red first. A/A clean (instructions 0.000 %, hyperfine +0.15 %). INV-6 identical for every binary. Proposed reading rule for sub-2 % verdicts, for the owner. Suites 44/44 in build and build-tune on fresh configures, format clean (DEC-146), Debug builds, PGO path builds, prose checks clean. No second tier owed: nothing in src/ moved. Fast check: six findings (the C/ASM reach of the flag, fastchess.sh's stale comment and no padding line, DEV_MANUAL overreach, L5's disturbed halves and the wall range it inflated, the uncaveated mechanism, a reading rule that did not fit its own data), all fixed before commit.

## Why

S270 (2026-10-10) timed an inline `game_tables()` that removes one function
and 32 bytes of `.text`: -0.4 % instructions, yet +1.7 to +2.5 % cycles and
-1.62 % wall, 0 of 24 pairs faster. With both sides built with
`-Wa,-mbranches-within-32B-boundaries` the same change read +0.13 and
-0.12 % cycles, both CIs across zero. The inferred cause is the Skylake
family's jump-conditional-code erratum, which this Coffee Lake has; the
inference is not read, `perf_counters` having no front-end counter. The flag
alone read -1.1 to -1.3 % cycles and -1.3 to -1.5 % wall on the parent, for
+0.5 % instructions, one layout of one binary -- a possible gain for one
assembler flag, not yet a reading, and a fix for the instrument: a padded
build should stop small unrelated edits from moving the timing by 2 %.
`adocs/data/S270_timing.txt` holds the readings.

## Lane

A workstation timing, under an hour; behaviour-neutral, so a timing and no
SPRT (DEC-083). Which shipped `CHESSO_ARCH` targets take the flag depends on
the parts they run on: `bmi2` (x86-64-v3) runs on later cores without the
erratum too, where the padding is only code size; the step states its call
with the reason.

## As built (2026-10-11, Opus subagent)

### Deviations, first

1. **`CMakeLists.txt` is not edited**, though `touches:` names it. The flag
   sits in `cmake/arch.cmake` beside the arch flags it goes with, so the tests
   and benchmarks are built as the engine is; on C++ units only, the language
   its check asks (the one assembly file holds data and no branch).
2. **`fastchess.sh` and `tests/test_fastchess_script.sh` are edited, from the
   fast check, and `touches:` now names them.** A reference before S273 builds
   unpadded while the candidate is padded, so the two sides of a match can
   differ by the build again. `cmake/arch.cmake` records what it applied as
   `CHESSO_BRANCH_PAD` (an internal cache entry), and the banner prints a
   `padding` line per side read from that side's own cache, a cache with no
   entry reading `off (unrecorded: before S273)`. Property 32 of the script's
   test covers it and was observed red first: run against `HEAD`'s
   `fastchess.sh` it failed its four assertions and nothing else
   (`.tuning/coord/S273/test_fastchess_red.log`); green after, 32 properties.
   An S272-shaped run (CAND and REF commits, BARE_FEN=1, its seed) through the
   old and the new script in the test's sandbox gives banners that differ by the
   one added line only, shas normalised; S272's real caches (`a7bb10e`,
   `16c8883`) record nothing, so it would read `off` on both sides.
3. **More layouts than asked, and three diagnostics the brief did not list.**
   Four perturbations of the tree's layout (`-falign-functions=64`, a dummy
   function in every unit, the link order reversed, and S270's
   `game_tables()` diff applied, a source change) and the shipped PGO `bmi2`
   build, instead of two; each perturbation's own effect, read unpadded and
   padded; and a front-end reading, `fe_counters`, a scratch copy of
   `tools/perf_counters` that counts the uop cache's and the legacy decoders'
   uops. Nothing of them is kept in the tree but the readings.
4. **Two sessions, and L5 was disturbed.** An API session limit stopped the
   work at 04:42 after the main run; it resumed at 11:55. L5, the PGO layout,
   ran 11:56:51 to 12:05:56. Its `bench 16` half (to 11:58:03) is clean. From
   about 11:59:35 the machine was disturbed: the 300-position pairs 1 to 7 read
   -3.1 to -3.4 % cycles each, pair 8 -8.53 % and pair 10 +1.39 %; hyperfine
   pair 6 read -10.94 %; per-pair sd 2.2 % and 3.4 % against 0.2 to 0.5 %
   elsewhere. Nothing in `runs/` records what disturbed it. Not re-run: the
   machine stayed busy. L5 is cited apart from L0 to L4 below and flagged in
   the table.
5. **The 300 positions total 26529797 nodes, not S270's 26512806**: S271 keeps
   the table across a new base FEN, and the workload sends one per position.
   Both sides of every pair print the same total.
6. **No switch to build unpadded.** A cache option would let `build/` and a
   reference disagree by choice; re-measuring the padding is a one-line edit of
   `cmake/arch.cmake`, and the banner now shows any disagreement.

### The A/A

Parent `e16b6fe` against a byte copy (`cmp`), 2026-10-11 03:30, governor
`powersave` (recorded, not set, DEC-195), microcode `0xfa`: instructions
0.000 % on both workloads; cycles -0.01 % (CI -0.56 .. +0.54) on `bench 16`
and -0.20 % (CI -0.37 .. -0.02) on the 300 positions, a CI that misses zero by
0.02; hyperfine +0.15 % (CI -0.10 .. +0.39), 8 of 16 pairs faster. The floor
this session sets is its largest CI bound: 0.56 % cycles on `bench 16`, 0.37 %
on the 300 positions, 0.39 % wall. `bench_movegen -r 40` resolution 0.1 / 0.9
/ 0.1 % before the run and 0.2 / 0.1 / 0.2 % after.

### Padded against unpadded, by layout

Every pair of binaries is node-identical to the parent (INV-6, below), and the
two sides of each layout differ by `-Wa,-mbranches-within-32B-boundaries`
alone. Instructions are every pair's; cycles and wall are the paired CI; the
last column counts the pairs where the padded side was faster.

| layout (both sides) | instructions | cycles bench 16 | cycles 300 pos | hyperfine wall | uop-cache share unpadded -> padded |
|---|---|---|---|---|---|
| L0 the tree's | +0.48 / +0.50 % | -1.53 % (-1.84 .. -1.21) | -1.61 % (-1.75 .. -1.47) | +1.50 % (+1.30 .. +1.70), 23/24 | 36.2 -> 42.1 % |
| L1 `-falign-functions=64` | +0.46 / +0.46 % | -2.48 % (-2.80 .. -2.15) | -2.02 % (-2.15 .. -1.89) | +1.94 % (+1.76 .. +2.12), 24/24 | 33.5 -> 42.8 % |
| L2 a dummy function in every unit | +0.48 / +0.50 % | -1.88 % (-2.01 .. -1.75) | -1.98 % (-2.14 .. -1.82) | +1.86 % (+1.68 .. +2.04), 24/24 | 34.4 -> 43.7 % |
| L3 S270's `game_tables()` diff applied | +0.48 / +0.48 % | -3.53 % (-3.69 .. -3.36) | -3.05 % (-3.24 .. -2.85) | +3.11 % (+2.96 .. +3.27), 24/24 | 23.1 -> 43.7 % |
| L4 link order reversed | +0.48 / +0.50 % | -3.68 % (-3.82 .. -3.54) | -2.95 % (-3.11 .. -2.80) | +3.08 % (+2.91 .. +3.25), 24/24 | 24.3 -> 43.1 % |
| L5 shipped `build_release.sh bmi2`, PGO -- **DISTURBED** from the 300-position half on | +0.82 / +0.88 % | -3.56 % (-4.01 .. -3.11), clean | -3.31 % (-4.67 .. -1.92), disturbed | +3.80 % (+2.30 .. +5.32), 21/24, disturbed | 29.5 -> 43.4 % |

The share is DSB / (DSB + MITE) uops (`IDQ.DSB_UOPS`, `IDQ.MITE_UOPS`;
microcode-sequencer uops are left out), `bench 16`, four runs each. In every
layout the padding moves 6 to 21 points of the uops from the legacy decoders to
the uop cache, **consistent with the JCC erratum (IDQ raw events, encodings
not cross-checked)**: they were typed from Intel's Skylake event list, and
with no `perf` on this machine nothing here checks them.

**Each perturbation's own effect** (no padding change, same method, 12 and 8
pairs): unpadded, `-falign-functions=64` +0.85 / +0.64 % cycles, the dummy
+0.59 / +0.42 %, the link order +1.83 / +1.35 %, instructions 0.000 %; padded,
+0.14 / +0.09 %, -0.07 / +0.13 % and -0.28 / -0.08 %. The last, `bench 16`
CI -0.55 .. -0.01, comes from a reordering that changes no function's 32-byte
phase in a padded build (each object's `.text` is 32-aligned there), so 0.3 %
is this method's noise, not a layout term; every padded reading is inside the
A/A floor. Unpadded, an unrelated layout change moved cycles by up to 1.8 %,
which no A/A sees. The padded binaries' `bench 16` cycles sit within 0.4 % of
one another across L0 to L4, the unpadded within 2.8 % (front-end runs, medians
of four, not paired). The tree's own layout is the best of the five unpadded,
which is why L0 reads the smallest gain.

**S270's inline as a perturbation.** Unpadded it reproduces S270: -0.41 /
-0.37 % instructions, **+2.33 % (CI +2.05 .. +2.62) and +1.74 % (+1.59 ..
+1.89) cycles**. Padded on both sides: -0.41 / -0.39 % instructions, **+0.20 %
(CI -0.03 .. +0.43) and +0.28 % (+0.16 .. +0.41) cycles**, hyperfine +0.00 %
(CI -0.22 .. +0.23). **Padding does not make the cycle reading match the
instruction reading**: 0.4 % fewer instructions read 0 to +0.3 % cycles. By
the proposed rule below it is a zero (no CI clears the A/A floor), so the call
and return it removes did not cost 0.4 % of the cycles here. Padding takes out
the 2 to 3.5 % layout term; it does not make cycles follow instructions.

### Keep-or-revert: KEPT

In the tree's layout and the four perturbations the padding holds in sign in
every half (cycles and wall, both workloads), every CI clear of the A/A floor,
and in size it is +1.50 to +3.11 % wall. The spread is the unpadded side's: the
padded binaries are nearly layout-independent and the unpadded ones are not.
L5 agrees in sign in every half, disturbed as it was. By the proposed rule
below: under 2 % in L0, instructions the other way, so the perturbation repeat
decides, and it agrees in sign and roughly in size.

### The ARCH call

**`native` and `bmi2` take the flag; `avx2` and `portable` do not.** `native`
is the measured machine. `bmi2`'s x86-64-v3 parts include the Skylake family
(Skylake to Comet Lake, Cascade Lake), where the padding holds in every layout
measured, and the PGO `bmi2` build read the same sign at a similar size on
`bench 16`, its clean half (-3.56 %, CI -4.01 .. -3.11). On its other parts
(Haswell, Broadwell, Ice Lake on, Zen3 on) there is no erratum and the padding
is +0.8 to +0.9 % instructions and +3 % `.text`; **what that costs in time
there is not measured** -- nothing here runs an x86 part without the erratum.
`avx2` exists for Zen1 and Zen2 and `portable` for parts older than Haswell,
none of which has the erratum.

Guarded by `check_cxx_compiler_flag`; a rejecting toolchain builds without it
and configure says so. Observed with clang 22 on this machine, which rejects
the `-Wa,` form ("unsupported argument ... to option '-Wa,'") and accepts it as
a driver flag: configure printed `Branch padding unavailable`, `chesso` built,
`bench` 4081329, and no unit carried the flag. Apple clang on the M1 (arm64)
was not run; the same guard covers it.

### INV-6

`bench` 4081329 and `bench 12` 1860699, every line identical (121 and 105);
`search_bench` (which sends `ucinewgame` before each position since S271)
32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12,
`c3d5` / `e2a6` / `d7c8q` and `c3d5` / `d5e6` / `d7c8q`: identical for all
twelve timed binaries, the final `build/` and the final `build_release.sh
bmi2`. The final `build/src/chesso` has every symbol at the timed padded
binary's address and the same `.text` size; 343 instructions differ only in
rip-relative displacements into `.rodata`, from the `-dirty` in `id name`. The
final PGO `bmi2` binary has the timed `pgo_pad`'s layout exactly.

### The proposed reading rule, for the owner to rule on

On this machine a speed verdict under 2 % (cycles or wall) is read so:

1. **Both sides padded** -- the default build from this step on, and a
   reference at or after it; `fastchess.sh`'s banner shows which.
2. **S270's method**: an A/A of byte copies in the same session, interleaved
   pairs in alternating order, `bench 16` and the 300 positions on the
   counters, hyperfine wall, the paired CI.
3. **The floor is that A/A's largest CI bound**, per workload and instrument
   (this session: 0.56 % cycles on `bench 16`, 0.37 % on the 300 positions,
   0.39 % wall). A change counts only when its cycles CI on both workloads, and
   its wall CI, lie wholly beyond the floor, the same way.
4. **Then it is credited if instructions move the same way.** If they do not
   move, or move the other way, it is credited only if the same comparison
   rebuilt under two perturbations of both sides, `-falign-functions=64` and
   the dummy function (`adocs/data/S273_timing.txt` has both recipes), agrees
   in sign and clears the floor in each.
5. **Anything else is recorded as zero.**

It reads this step's own cases as the step does. The padding: beyond the floor
everywhere, instructions the other way, the perturbations agree -- credited.
The inline padded: +0.20 / +0.28 % cycles, CIs reaching -0.03 and +0.16, so
not beyond 0.56 / 0.37 %, wall 0.00 % -- zero. The inline unpadded (+1.7 to
+2.3 % cycles, instructions the other way) is not read at all under rule 1,
and its padded repeat disagrees in size, so rule 4 would not credit it either.
The evidence: padded, unrelated layout changes moved cycles 0.3 % at most,
inside the A/A floor; unpadded, 1.8 %, which no A/A sees. S268's -11.4 % and
S269's -2.0 % stand on their instruction counts under either reading.

### Suites

On fresh configures of both trees, `CLANG_FORMAT_MAJOR=22`: `cmake --build build
-j12 && ctest --test-dir build -L fast --output-on-failure && cmake --build
build-tune -j12 && ctest --test-dir build-tune -L fast --output-on-failure &&
./clang-format.sh --check` **exit 0: 44/44, 44/44 (`test_fastchess_script` among
them, 32 properties), format clean, 0 warnings**, `-Werror` on. Every C++ compile command carries the
flag and the assembly one does not (50 of 50 C++, 0 of 1 assembly, in both trees); the cache records
`CHESSO_BRANCH_PAD:INTERNAL=ON`. A Debug configure builds, 0 warnings, padding
on, `bench` 4081329. `./build_release.sh bmi2` (the PGO path) builds with the
flag in all 50 C++ units, `bench` 4081329, the timed `pgo_pad`'s layout. `CHESSO_ARCH` `avx2` and `portable` configure with
`Branch padding off` and record `OFF`. `tools/plan_prose_check.py`
`--citations`, `--touches`, `--params`, one mode per call: each exit 0, 0 flagged. No
`fastchess` run, no `gate_extra`: nothing in `src/` moved.

`MANUAL.md` checked: no option, output or behaviour moved -- no change.
`DEV_MANUAL.md`: the Build section's flag paragraph names the padding, traced
to `cmake/arch.cmake`, with what `as` pads and what it does not, its
measurement, the configure lines, bmi2's unmeasured cost and the older-REF
caveat; the arch table's `bmi2` and `native` rows name it, the `bmi2` row with
the unmeasured cost; the `fastchess.sh` banner example shows the `padding`
line. `specs.md` names no build flag -- no change proposed.

### Proposed `done:` stamp

2026-10-11 -- KEPT: -Wa,-mbranches-within-32B-boundaries on C++ units for
CHESSO_ARCH native and bmi2 (cmake/arch.cmake, check_cxx_compiler_flag-guarded;
a rejecting toolchain builds without it and says so, observed with clang 22;
Apple clang not run). Padded vs unpadded in the tree's layout and four
perturbations (-falign-functions=64, a dummy function per unit, S270's
game_tables() diff applied, link order reversed), single-process: -1.5 to
-3.7 % cycles for +0.5 % instructions, hyperfine +1.50 to +3.11 %, every half
the same sign; the PGO bmi2 build the same sign at a similar size on its clean
bench 16 half (-3.56 %), its other half disturbed. DSB / (DSB + MITE) uop share
23 to 36 % unpadded against 42 to 44 % padded, consistent with the JCC erratum
(IDQ raw events, encodings typed from Intel's list and not cross-checked here;
microcode-sequencer uops left out). Unrelated layout changes moved cycles up to
1.8 % unpadded and 0.3 % padded, inside the A/A floor (0.56 % on bench 16);
S270's inline reads +1.7 to +2.3 % cycles unpadded and +0.2 / +0.3 % (wall
0.00 %) padded for -0.4 % instructions: padding does not make cycles follow
instructions. Unmeasured: bmi2's cost on parts without the erratum, and a
full-concurrency match. fastchess.sh's banner prints each side's padding from
its CMake cache (CHESSO_BRANCH_PAD; unrecorded = before S273), property 32 red
first. A/A clean (instructions 0.000 %, hyperfine +0.15 %). INV-6 identical for
every binary. Proposed reading rule for sub-2 % verdicts, for the owner.
Suites 44/44 in build and build-tune on fresh configures, format clean
(DEC-146), Debug builds, PGO path builds, prose checks clean. No second tier
owed: nothing in src/ moved.
