id:         S104
goal:       the release build targets the machine's instruction set and is profile-guided, so count_bits stops being a software popcount
accepts:    `objdump -d build/src/chesso | grep -c popcnt` is greater than zero, where it is **0** at 20d058a; identical node counts and identical best moves from tools/search_bench.py against the preceding commit at two depths, which discharges INV-6 and is why **no SPRT is owed** (DEC-083); the speed change measured by interleaved runs with the spread recorded, and converted to Elo at the published 1.43 per percent with the conversion named as a conversion; three build targets exist and the one that ships to a rating list is named -- a bmi2 target, an avx2 target with magics for pre-Zen3 AMD where PEXT is microcoded, and a portable baseline; `-march=native` never ships in a distributed binary; PGO uses a fixed-depth search over many positions as its workload, never perft and never one position, and the profile is regenerated per build and never committed
touches:    CMakeLists.txt, cmake/, DEV_MANUAL.md, build_release.sh, .gitignore
            build_release.sh and .gitignore were added to touches: during the step.
            The accepts require a profile-guided build, PGO is three passes over two
            build directories, and no single CMake configure can be both of them --
            so a driver has to exist somewhere and cmake/ is not where a shell script
            belongs. .gitignore follows from "the profile is never committed", which
            the accepts also require. Not a plan-versus-code conflict and no decision
            is owed: the step under-specified its own file list.
excludes:   using _pext_u64 in the sliding attack lookup, which is S032 and is a code change; any change under src/
decisions:  DEC-083, DEC-049
closes:
blocks:
paused_by:
done:      SHIPPED +18.22 %, held out, 95 % CI +16.19 to +20.28, ratio 0.8459 geometric mean and 0.8447 median, paired t -21.90 over 10 rotating triples of base / arch-only / arch+PGO, each run 400 positions the profile was NOT trained on at depth 10. Architecture flag +12.62 % (CI +11.19 .. +14.06, t -21.06), profile-guided optimisation the remaining +4.98 %. +26.1 Elo at the published 1.43 per percent, STATED AS A CONVERSION AND NOT AS A VERDICT. BEHAVIOUR-NEUTRAL, INV-6 discharged on node counts against 20d058a and no SPRT owed (DEC-083): 164123 / 670488 / 84351 at depth 9 and 1162576 / 5167100 / 683367 at depth 12, best moves c3d5 / e2a6 / d7c8q at both, identical for base, native, bmi2, avx2 and portable alike. popcnt 0 at 20d058a, 159 on build/ at native, 125 on each release target. Three distributable targets and they are different code: bmi2 409 BMI2 / 75 AVX2 insns (SHIPS TO A RATING LIST), avx2 0 / 75 for Zen1 and Zen2 where PEXT is microcoded, portable 0 / 0; __BMI2__ is the switch and S032 needs no macro of its own. -march=native is the default for build/ and build_release.sh refuses it by name, exit 2. Held out on purpose: the same harness over the 400 TRAINING positions reads +18.65 % (CI +18.47 .. +18.83), so the overfit is 0.43 points and the gain is a property of the code. Per-run spread 6.70 / 5.64 / 7.69 % is larger than the PGO term alone -- the pairing resolves it, not the timer. bench_movegen resolution 0.2 %, spread 0.5 %; bench_eval 53.90 ns a call against 83.35 ns earlier the same day on the unflagged build, a third of evaluate() being software popcount. ONE BUG FOUND AND FIXED BEFORE ANYTHING ELSE CONTINUED: the first design used two build directories, GCC names a .gcda by mangling the absolute object path and looks it up under the same name, so every lookup missed and all three binaries were ordinary -O3 builds wearing a PGO label -- and -Wno-missing-profile, which the first cmake/pgo.cmake set tree-wide, is what hid it. It surfaced as PGO measuring zero (base 15.26 s / bmi2 13.39 s / bmi2+pgo 13.38 s) and that first interleaved run was killed at rep 3 and discarded, not reported. Fixed by one build directory reconfigured between passes, and by silencing nothing: -Wmissing-profile is left as the error -Werror makes it, with exactly one TU exempt by name because its absence is structural -- every caller of src/search_params.cpp is inside #ifdef CHESSO_TUNE, so the release link drops the object and no workload can reach it, 8 of 9 files in src/ profiled. Observed red with the warning restored: profile count data file not found [-Werror=missing-profile]. Coverage mismatch verified rather than asserted, since two documents claim it: error: the control flow of function 'int main()' does not match its profile data (counter 'arcs') [-Werror=coverage-mismatch]. WORKLOAD is a fixed-depth search over many positions, never perft and never one position: 400 positions, 16 per each of the 25 game_phase() values from adocs/data/S018_raw.tsv, 117329931 nodes, 66 s instrumented, 8 .gcda files; serial through one process knowingly against DEC-050 so merge-on-exit does not depend on the scheduler. Its first driver piped position/go blind and the workload finished in under a second having profiled a depth-1 tree -- go runs on its own thread, DEV_MANUAL.md already warned about it, it waits for bestmove now. Profile regenerated per build, .pgo/ gitignored, never committed. SUITES 16/16 fast in build, build-tune, build-debug (INV-4 asserted, 214.87 s) and build-release-bmi2; clang-format clean. touches: amended mid-step to add build_release.sh and .gitignore -- the step under-specified its own file list, no decision owed. MANUAL.md gained the distributable-build paragraph; README.md checked, owner-written, no change needed. BASELINE CONSEQUENCE, and it is the one thing here a decision should cover: every timing and nps figure taken before this step is on a binary with no architecture flag and is not comparable with one taken after it, the same class of break DEC-049 recorded for the machine move. Node counts unaffected. Two stale figures re-measured in the same commit (bench_eval 83.35 -> 53.90 ns, search_bench depth 9 total 0.149 -> 0.143 s). native as the default for build/ is what does this and it is PROPOSED to the owner, not recorded as a decision by an agent.

## Measured before the step, 2026-08-19, at 20d058a

The shipped binary contains **zero `popcnt` instructions**. `CMakeLists.txt`
adds `-Wall -Wextra -Werror` and nothing else, so the release flags are
`-O3 -DNDEBUG -std=gnu++20`; `std::popcount` without `-mpopcnt` compiles to a
software SWAR popcount, and `count_bits` is called once per piece per
evaluation in the mobility and king-safety loops, in `game_phase`, and
throughout move generation.

Rebuilt with `-march=native`: **159 `popcnt` instructions**, and

| build | depth 14 from startpos | nodes | nps |
|---|---|---|---|
| shipping | 1170 / 1213 / 1222 ms | 6730511 | 5.76 M |
| `-march=native` | 991 / 1008 ms | **6730511**, same PV | 6.72 M |

**+16.7 %, node-identical.** At the published conversion that is about +24 Elo
of self-play at short time control, for one line of CMake. LTO on top measured
1054 / 1191 against 1076 / 1089 -- inside the noise on this machine, so it is
carried for the PGO pairing rather than claimed on its own.

## Why no SPRT

DEC-083. A speed-up below about 0.7 % is invisible to an SPRT at long time
control; this one is well above that, but the instrument is still wrong for it.
The node count is identical, so the two builds play identical games and a match
between them measures nothing but the clock. The timing is the measurement.
author:    Maksym Bodnar

## What shipped

`cmake/arch.cmake`, `cmake/pgo.cmake`, `build_release.sh`. Nothing under `src/`.

**Four arch targets, three of them distributable.** `-DCHESSO_ARCH=`, validated by
`check_cxx_compiler_flag` before use so a dropped flag is a configure-time
`FATAL_ERROR` rather than a target named for an instruction set it does not
target:

| value | flags | for | BMI2 / AVX2 insns |
|---|---|---|---|
| `bmi2` | `-march=x86-64-v3` | **ships to a rating list.** Haswell 2013+, Zen3 2020+ | 409 / 75 |
| `avx2` | `-march=x86-64-v3 -mno-bmi2` | Zen1 and Zen2, PEXT microcoded at ~18 cycles | 0 / 75 |
| `portable` | `-march=x86-64-v2` | fallback, Nehalem 2008+ / Bulldozer 2011+ | 0 / 0 |
| `native` | `-march=native` | **default, this machine only.** `build_release.sh native` exits 2 | 371 / — |

`__BMI2__` is the switch `avx2` turns off, so S032's magic path needs no macro of
its own. All three release targets carry **125 popcnt** instructions against
**0 at `20d058a`**; `build/` at native carries 159. `build_release.sh` asserts a
non-zero count per target and exits non-zero otherwise.

## INV-6, discharged on node counts

Depth 9 and depth 12, every binary, against `20d058a`:

    depth  9   164123 / 670488 / 84351          c3d5 / e2a6 / d7c8q
    depth 12   1162576 / 5167100 / 683367       c3d5 / e2a6 / d7c8q

Identical for base, `native`, `bmi2`, `avx2` and `portable`. **No SPRT owed**,
DEC-083: the node count is identical, so the binaries play identical games and a
match measures nothing but the clock. The interleaved harness re-checks node
counts every rep and aborts on a move; it never did.

## The speed change, one interleaved run

10 rotating triples of base / arch-only / arch+PGO, order rotating within the
triple so a first-or-last bias cancels, each run **400 positions the profile was
not trained on** at depth 10:

    base (no arch flag, no PGO)   median 17.409s   spread 6.70 %
    bmi2, no PGO                  median 15.432s   +12.62 %   CI +11.19 .. +14.06
    bmi2 + PGO, what ships        median 14.722s   +18.22 %   CI +16.19 .. +20.28

Ratio 0.8459 geometric mean, 0.8447 median, paired **t −21.90** over 10 pairs.
The architecture flag is +12.62 % and the profile the remaining **+4.98 %**.
**+26.1 Elo at the published 1.43 per percent — stated as a conversion, not as a
verdict.**

**Held out, and that mattered.** The same harness over the 400 *training*
positions reads +18.65 % (CI +18.47 .. +18.83) against +18.22 % held out. The
0.43-point gap is the overfit, and it is small enough that the gain is a property
of the code rather than of the workload. Timing a profile-guided binary on its own
profile is not a measurement of it, so the held-out figure is the one recorded.

**Machine resolution beside it.** `bench_movegen` 814.3 ms for 12 M perft nodes,
**resolution 0.2 %**, spread 0.5 %. `bench_eval` **53.90 ns a call, resolution
0.2 %**, spread 3.9 % — against **83.35 ns** measured earlier the same day on the
unflagged build. A third of an `evaluate()` call was software popcount, which is
what `count_bits` in the mobility and king-safety loops and in `game_phase` costs
when it is not one instruction. The per-run spread on the held-out timing, 6.7 %,
is larger than the PGO term alone: **the pairing resolves it, not the timer.**

## The bug this step found, fixed before anything else continued

**The first PGO design used two build directories and produced three binaries that
were not profile-guided at all.** GCC names a `.gcda` by mangling the absolute
path of the object that wrote it —
`#home#max#ws#chesso#build-release-bmi2#src#CMakeFiles#chesso_engine.dir#search.cpp.gcda`
— and looks it up under the same name on the way back in. An instrumented build in
`.pgo/build-generate-bmi2/` and an optimised build in `build-release-bmi2/` give
two different object paths, so every lookup missed and every translation unit
compiled with no profile.

**`-Wno-missing-profile`, which the first `cmake/pgo.cmake` set tree-wide, is what
hid it.** The symptom was PGO measuring zero: one rep read base 15.26 s, bmi2
13.39 s, bmi2+pgo 13.38 s. The first interleaved run was killed at rep 3 and
discarded rather than reported.

Fixed two ways, and the second is the one that matters:

- `build_release.sh` reconfigures **one** build directory between passes.
- **nothing silences `-Wmissing-profile`.** Under `-Werror` a missing profile stops
  the build, which is the right outcome — a binary that quietly is not
  profile-guided is worse than one that does not build. Exactly one translation
  unit is exempt, by name and for a structural reason: every caller of
  `src/search_params.cpp` is inside `#ifdef CHESSO_TUNE` (`src/chesso.cpp:934`,
  `:1043`), so the release link references none of its symbols, the linker drops
  the object out of the static library, its gcov constructor never runs, and no
  workload can reach it. 8 of the 9 files in `src/` get a profile and the error
  stays live for all 8.

Both claims verified rather than asserted. Missing profile, observed red at the
two-directory design with the warning restored: `error: '...search_params.cpp.gcda'
profile count data file not found [-Werror=missing-profile]`. Coverage mismatch,
because two documents claim it — a profile taken, the source then changed, the use
pass re-run: `error: the control flow of function 'int main()' does not match its
profile data (counter 'arcs') [-Werror=coverage-mismatch]`.

## The workload

400 positions, 16 from each of the 25 values of the engine's own `game_phase()`,
drawn from `adocs/data/S018_raw.tsv` — 13522 positions chesso reached in 210 of its
own games. Deterministic: first 16 rows per phase in file order, no sampling.
**117329931 nodes, 66 s instrumented, 8 `.gcda` files.**

Not perft, which never evaluates, never orders a move and never probes the table,
and would tell the compiler three quarters of the hot code is cold. Not one
position, because the branch that matters in an endgame is not the one that
matters in a middlegame — the same stratification, and the same reason, as
`adocs/data/S021_aspiration_sweep.py`.

Serial through one process, knowingly against DEC-050's all-12 default: twelve
concurrent instrumented processes would finish in a twelfth of the time, but
`.gcda` merge-on-exit would then depend on the scheduler and the profile would stop
being a function of the source tree. One process also leaves the table warm across
positions, which is the regime a game's later moves search in.

**The first driver piped `position` and `go` blind down a pipe and the 400-position
workload "finished" in under a second.** `go` runs on its own thread, so it started
400 searches, cancelled 399 and profiled a depth-1 tree. It waits for `bestmove`
now — the shape `tools/search_bench.py` already had and `DEV_MANUAL.md` already
warned about under "Wait for `bestmove` when you script it".

## Baseline consequence, stated because nothing else records it

**Every timing and nps figure in this repository taken before this step is on a
binary with no architecture flag and is not comparable with one taken after it.**
Node counts are unaffected and stay comparable. This is the same class of break
DEC-049 recorded for the machine move, and `specs.md` and `DEV_MANUAL.md` now say
so where the figures live. Two figures were re-measured and corrected in this
commit rather than left to rot: `bench_eval` 83.35 ns to **53.90 ns**, and
`search_bench` depth 9 total 0.149 s to **0.143 s** — the second being a
demonstration that three positions do not time anything, since it reads 4 % where
the real effect is 18 %.

**`native` as the default for `build/` is the flag that does this**, and it is a
consequence worth a `decisions.md` entry that no agent should write: decisions
belong to the owner. It is proposed, not recorded.
