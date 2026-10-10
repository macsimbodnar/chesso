id:         S273
goal:       the workstation's builds pad branches off 32-byte boundaries if a layout-robust timing shows it faster, and a speed verdict under about 2 % on this machine gets a reading that code layout cannot fake
accepts:    (1) an A/A of the parent and a byte copy, then the parent built with `-Wa,-mbranches-within-32B-boundaries` (g++ 13.3, GNU as) timed interleaved against the parent on `tools/perf_counters` (instructions and cycles) and hyperfine wall, S270's two workloads, node identity (INV-6: `bench` total and stream, `bench 12`, `tools/search_bench.py` at depths 9 and 12); (2) the reading is robust to layout: the same comparison repeated over at least two unrelated layout perturbations of both sides (for example a dummy function or `-falign-functions` changes in an unrelated unit), each read the same way, because S270 found layout alone moves cycles about 2 % here while an A/A of byte copies shows none; (3) if it holds, the flag is set in CMake for the `CHESSO_ARCH` values whose targets are the Skylake family's (at least `native` on this machine), guarded so a toolchain whose assembler lacks it (Apple clang) still builds, both gated builds and a Debug configure green, `-Werror` clean, `cmake/pgo.cmake`'s path built once; if it does not hold, the zero is recorded and the build left as it is; (4) the step file states a proposed reading rule for sub-2 % speed verdicts on this machine (both sides padded, or instructions moving the same way as cycles, or the perturbation repeat of (2)), as a proposal for the owner to rule on as a decision; (5) `DEV_MANUAL.md`'s Build section states the flags as they then are, traced to the CMake that sets each (DEC-017)
touches:    cmake/arch.cmake, CMakeLists.txt, DEV_MANUAL.md, adocs/data/S273_timing.txt
excludes:   any change to src/; LTO (S270's zero stands; single-partition LTO is a separate question); setting the CPU governor (DEC-195)
decisions:  DEC-083, DEC-017, DEC-118, DEC-195, DEC-263
closes:
blocks:
paused_by:
author:
done:

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
