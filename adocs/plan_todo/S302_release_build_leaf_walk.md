id:         S302
goal:       `./build_release.sh` builds a release again: its profile-guided pass 3 stops failing on `src/leaf_walk.cpp`, a source no profiled binary runs
accepts:    `./build_release.sh portable`, `bmi2` and `avx2` (each the machine can run) complete all passes and print their bench, equal to `chesso bench` from `build/`; pass 3 raises no `-Werror=missing-profile`; the chosen fix keeps `-Werror` and keeps every file the engine binary links profiled; `datagen` still builds and links `leaf_walk`; the fix is stated with why it is the right scope (moving `leaf_walk` out of the engine library, or excluding it from the profiled set), and the gate is green in both builds
touches:    cmake/pgo.cmake or src/CMakeLists.txt (the agent states which), DEV_MANUAL.md if a statement about the release build moves
excludes:   any change to what the engine computes, so bench equals the parent's; any change to datagen's behaviour; any timing claim about the release binary
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

S301's subagent ran `./build_release.sh portable` at `733ba63` to check
the workload heredoc it changed. Pass 2, the workload, passed. Pass 3
failed with `-Werror=missing-profile` on `src/leaf_walk.cpp`.

Since `5b0e170` (S082, 2026-10-16), `src/CMakeLists.txt`'s `SRCS` glob puts
`leaf_walk.cpp` into `chesso_engine`, but only `datagen` links it. The
profiled engine binary never runs it, so no profile is written for it, and
the profile-use pass refuses it under `-Werror`. The last release build on
record is S274's, 2026-10-12, before the file existed.

S301's fast check confirmed it by reading. None of `leaf_walk.cpp`'s
global symbols are in `build/src/chesso`. The only other such object is
`search_params.cpp.o`, which `cmake/pgo.cmake` exempts by name; that file's
comment about "the other eight" is stale, since there are nine now.

The defect fails loudly and moves no score, move or line. It is not
reachable in ordinary play or on the UCI surface: the SPRT harness and the
gate build with CMake and no profile (DEC-171). Filler, behind the next
strength step. The owner is told, because a release binary cannot be made
until it lands. Filed by the coordinator, 2026-10-30.
