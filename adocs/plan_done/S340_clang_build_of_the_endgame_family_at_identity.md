id:         S340
goal:       the tree builds under clang with `-Werror` whatever S124's family ships, its identity included
accepts:    (1) `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE` on a tree whose `endgame_scale` is all 128 and whose mop-up arrays are all zero, where at `7214ce5` it ends `GATE-EXTRA-FAILED: clang`, the failure seen first; (2) the same on a tree with mop-up live and the factor at its identity, and on one with the factor live and mop-up zero, the two bisection parts' shapes, each by a throwaway edit of the three arrays; (3) `chesso bench` reads the parent's total in both gated builds and `tools/search_bench.py` reads identical node counts and best moves (INV-6); the commit says `No functional change`; (4) no warning is silenced tree-wide and no flag is added or removed: the two functions say at their sites why a build may not need them
touches:    src/evaluation.cpp, DEV_MANUAL.md
excludes:   any change to what the evaluation computes; `tools/gate_extra.sh` and `tools/gate.sh`; moving the clang stage into the automatic gate, which is a decision of its own
decisions:  DEC-171, DEC-141, DEC-350
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator, 2026-11-12
done:       2026-11-13, on S341's landing: `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE` at the family's identity (`endgame_scale` all 128, mop-up zero), both clang directories compiling every target with no warning and 61 of 61 each, where on `dbef91e` it ended `GATE-EXTRA-FAILED: clang` on the two `-Wunneeded-internal-declaration` errors (seen first); both bisection shapes compile both libraries in both clang directories with no warning (throwaway, reverted); `chesso bench` 4707943 in both gated builds, `search_bench` identical at depths 9 and 12, No functional change; `[[maybe_unused]]` on the two definitions, no flag added or removed; the fix landed as `5048611`.

## Why

Found by the weekly `tools/gate_extra.sh` on `7214ce5`, 2026-11-12 00:09
(`.tuning/gate_extra_2026-11-11.log`; prose, citations, debug, sanitize and perft ok, clang exit 1):

```
src/evaluation.cpp:2076:20: error: function 'mopup_terms' is not needed and will not be emitted [-Werror,-Wunneeded-internal-declaration]
src/evaluation.cpp:2097:19: error: function 'scaled_taper' is not needed and will not be emitted [-Werror,-Wunneeded-internal-declaration]
```

in `build-clang` and `build-clang-tune` alike. Both functions are called only under
`if constexpr (mopup_live || trace_build)` and `if constexpr (endgame_scale_live || trace_build)`, and at the identity
both flags are false in the engine library, so clang sees two `static inline` functions no translation unit needs. GCC,
which both gated builds use, says nothing. The tree has been in this state since S124's phase 1 landed the family at its
identity on 2026-11-06; the last `GATE-EXTRA-DONE` before that was 2026-11-05.

What it breaks: any clang build with `-Werror`, the owner's MacBook's among them (S297, S298), of every commit from
phase 1's landing on. It moves no play, no score and no UCI output, so by DEC-171 it is a filler step, named in the
pre-registration of every run taken while it is open. S124's SPRT is the first such run.

It binds whatever S124's verdict is. On H1 the pasted values make both functions needed and the warning goes with them,
but only while both stay live; on H0 the identity stays, and each bisection part leaves one of the two unneeded.

## Shape

One Opus subagent, agent-only, no run. The fix is at the two definitions, not in the build flags. It lands when no
timed run holds the machine, and not in a worktree a run is playing from.

## 2026-11-12: done

The compile break is fixed; **accept (1) does not hold**, because the fix lets the fast suite run under clang for the
first time since S124's phase 1, and one test is red there for a reason that is not this step's. (2), (3) and (4) hold.

**The fix**, `src/evaluation.cpp` only: `[[maybe_unused]]` on `mopup_terms()` and `scaled_taper()`, the file's own form
for the same situation (`pawn_entry_of()`, `pawn_shelter_of()`), each with a comment saying which build may not need it
(the engine library while mop-up's weights are all zero, or while every cell's factor is F) and that clang says so
unless told. No flag added or removed; nothing else in `src/` touched; `./clang-format.sh --check` clean.

**(1) Red first.** On `dbef91e`, `export CLANG_FORMAT_MAJOR=22; STAGES=clang OUT=.tuning/coord/S340/red
tools/gate_extra.sh` ended `GATE-EXTRA-FAILED: clang /home/max/ws/chesso/.tuning/coord/S340/red`, with both
`-Wunneeded-internal-declaration` errors (`mopup_terms` at 2076:20, `scaled_taper` at 2097:19) in `build-clang` and
`build-clang-tune`. **On the fix**, the same command with `OUT=.tuning/coord/S340/green` compiles every target in both
directories with no warning and no error, and ends `GATE-EXTRA-FAILED: clang`: 60 of 61 `fast` tests pass in each,
and `test_tuner_gradient` fails in each, the case "the endgame factor's gradient is the derivative, chain rule in",
`CHECK( check.worst < 1e-6 )` reading `4.70154e-06 at parameter 541 of 482, over 5696 rows`, the same in both
directories. **Not caused here:** HEAD's `evaluation.cpp` and the fixed one, compiled by `clang++-22` with
`build-clang`'s own flags (`-Wno-error` added for HEAD's only, to get an object at all), give byte-identical objects for
the engine library and the trace library alike (`cmp`, under `.tuning/coord/S340/objcmp/`). The test binary is the one
HEAD would have built, and the test never ran under clang because HEAD's clang build stopped before it. **Likely
cause, not verified further:** `tests/test_endgame_positions.hpp`'s `placements()` draws squares with `std::shuffle`,
whose algorithm is implementation-defined; libc++ and libstdc++ give different orders from the same `mt19937`, so
clang's population is 5696 rows where the case's comment records 5717 under GCC (6.27485e-7 at 739), and DEC-356's
pair was chosen on GCC's population. Reported to the coordinator; the test was not touched.

**(2) The two bisection shapes**, each a throwaway edit of the three arrays by `.tuning/coord/S340/shape.sh`,
reverted after and checked by `sha256sum` (`665c365d...` = the fixed file) and `git diff --stat` (the fix alone).
(a) `mopup_mg = {96, 149, 0}`, `mopup_eg = {15, 13, 6}`, `endgame_scale` all 128; (b) `endgame_scale = {45, 0, 9, 29,
81, 114, 86, 128, 123, 19, 128, 128, 128}`, mop-up zero. `cmake --build <dir> --target chesso_engine
chesso_engine_trace -j12 -- -k 0` exits 0 with no warning in `build-clang` and `build-clang-tune` for both. The
shapes exercise the halves: on the unfixed file, `build-clang`'s `chesso_engine` stops on `scaled_taper` alone for
(a) and on `mopup_terms` alone for (b). The test suite was not run on either shape, by design.

**(3)** `export CLANG_FORMAT_MAJOR=22 && cmake --build build -j12 && ctest --test-dir build -L fast
--output-on-failure -j12 && cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast --output-on-failure
-j12 && ./clang-format.sh --check` exits 0, 61 of 61 in each build, no compiler warning. `chesso bench` reads
`4707943 nodes` in `build` and `build-tune`, the parent's total. `tools/search_bench.py` at depths 9 and 12, both
builds, against the parent's binaries: identical node counts and best moves, 25473 / 127664 / 34197 with c3d5 / e2a6
/ d7c8q at depth 9 and 94541 / 353715 / 409231 with c3d5 / e2a6 / d7c8r at depth 12, in both builds. The parent's
binaries are `build/` and `build-tune/` as they stood at the start of the step, copied before any rebuild: their
`evaluation.cpp.o` dates from 2026-11-06 12:05, before `96b70fc`, the last commit to touch `src/`, and both read
`4707943` on `bench`. The commit says `No functional change`.

**(4)** No flag added or removed, no warning silenced tree-wide: the diff is the two attributes and their comments.

**Docs.** `DEV_MANUAL.md`: one paragraph under the clang check in the build section, naming the second thing clang
refuses and GCC does not and the form used for it. `MANUAL.md`: checked, unchanged (no UCI surface moved).

**Not done:** the Debug self-play of DEC-141's second tier (the step touches no `make_move`, generator or search
code); a clang build on the Mac; anything about `test_tuner_gradient` under libc++.

## 2026-11-12: paused by S341, the coordinator

The fix is landed, `No functional change`: both gated builds 61 of 61, `chesso bench` 4707943, accepts (2), (3) and (4)
held. Accept (1) does not hold: the clang stage now compiles everything and reads `test_tuner_gradient` red in both
clang directories, a defect of its own that this step's two errors had hidden. S341 is filed for it in `plan_current/`
and blocks this step. S340 completes when S341 lands and `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE`;
accept (1) is not reworded.
