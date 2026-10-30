id:         S302
goal:       `./build_release.sh` builds a release again: its profile-guided pass 3 stops failing on `src/leaf_walk.cpp`, a source no profiled binary runs
accepts:    `./build_release.sh portable`, `bmi2` and `avx2` (each the machine can run) complete all passes and print their bench, equal to `chesso bench` from `build/`; pass 3 raises no `-Werror=missing-profile`; the chosen fix keeps `-Werror` and keeps every file the engine binary links profiled; `datagen` still builds and links `leaf_walk`; the fix is stated with why it is the right scope (moving `leaf_walk` out of the engine library, or excluding it from the profiled set), and the gate is green in both builds
touches:    cmake/pgo.cmake (the exemption and its comment), DEV_MANUAL.md (the release build's exemption paragraph, and a line recording that release builds work again)
excludes:   any change to what the engine computes, so bench equals the parent's; any change to datagen's behaviour; any timing claim about the release binary
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30, Opus 5.5 subagent, coordinated: reproduced at `173ade1`, `./build_release.sh portable` pass 3 stopping on `-Werror=missing-profile` for `src/leaf_walk.cpp`; `cmake/pgo.cmake` now exempts `src/leaf_walk.cpp` beside `src/search_params.cpp`, the two objects whose every global symbol is absent from `build/src/chesso` (no third source in `SRCS` is so, and pass 2 writes 8 `.gcda`, the other eight C++ units), and its comment names both files, why each has no profile, `search_params.cpp`'s callers by function in place of two stale line numbers, and eight as the count the error stays live for; `src/CMakeLists.txt` untouched, the exemption chosen over moving `leaf_walk` out of `chesso_engine` because it is the existing rule on a second file and moving it buys the same binary for three CMake files; `portable`, `bmi2` and `avx2` each 3 of 3 passes, 41, 42 and 41 s wall, 8 profile files, `chesso bench` 5434393 each, equal to `build/`'s; `build_release.sh` prints the workload total and no bench, so bench was run on each binary after it and the script not changed; with `utils.cpp.gcda` moved aside pass 3 stopped on it, so the error is live for every linked file, and `-Werror` is unchanged; `datagen` builds and defines `leaf_walk_pv` in `build/` and under `CHESSO_PGO=use`, `test_leaf_walk` passing there; the whole `build-release-bmi2` built and its fast suite passed 49 of 50, `test_clang_format_script` failing only for the missing `CLANG_FORMAT_MAJOR=22` and passing with it; DEV_MANUAL.md's exemption paragraph corrected and a paragraph records that release builds work again, MANUAL.md checked, no change; no `src/` change, `build/` benches the parent's 5434393; both builds and fast suites green, 50 of 50 each, `./clang-format.sh --check` clean under `CLANG_FORMAT_MAJOR=22`, the five `tools/plan_prose_check.py` modes 0 flagged; Tier 2 does not bind, no search, generator or make/unmake change; release directories and `.pgo/` removed

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

## 2026-10-30: the fix and its evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s302`, branch `s302`, from `173ade1`. The machine was
the subagent's: nothing else ran, every build at `-j12`. No timing claim is
made about any binary.

### Reproduced

`./build_release.sh portable` at `173ade1`: pass 1 built, pass 2 ran the
workload (400 positions, 29465966 nodes, S301's total at `733ba63`) and
wrote 8 profile files, and pass 3 stopped, exit 2, 40 s wall:

```
src/leaf_walk.cpp: In member function 'void std::__new_allocator<_Tp>::deallocate(_Tp*, size_type) [with _Tp = char]':
src/leaf_walk.cpp:<line>:1: error: '.pgo/portable/#home#max#ws#chesso-s302#build-release-portable#src#CMakeFiles#chesso_engine.dir#leaf_walk.cpp.gcda' profile count data file not found [-Werror=missing-profile]
cc1plus: all warnings being treated as errors
gmake[3]: *** [src/CMakeFiles/chesso_engine.dir/build.make] Error 1
```

Paths are shortened to the tree, and the line, the file's closing brace,
and the make rule's line are written as `<line>` or dropped, plan files
carrying no line citations (DEC-135). The 8 `.gcda` written are `bitboard`,
`chesso`, `evaluation`, `main`, `openings`, `search`,
`transposition_table` and `utils`: the ten C++ files in `src/` less
`leaf_walk` and `search_params`.

### Which sources share the property

For every object of `chesso_engine` and `chesso` in `build/`, its defined
strong global symbols (`nm --extern-only`, types T, D, B and R) against the
symbols `build/src/chesso` defines: `leaf_walk.cpp.o` 7 of 7 absent,
`search_params.cpp.o` 3 of 3 absent, every other object (the seven other
library sources, `openings_embedded.S` and `main.cpp`) 0 absent. No third
source has it, which the `.gcda` list above says independently.

### The fix, and why this scope

`cmake/pgo.cmake` names `src/leaf_walk.cpp` beside `src/search_params.cpp`
in the one `set_source_files_properties` that gives them
`-Wno-missing-profile` under `CHESSO_PGO=use`. The header comment now says
two files are exempt and why each has no profile, names
`src/chesso.cpp` `command_uci` and `src/chesso.cpp` `command_setoption` for
`search_params.cpp`'s tune-only callers in place of two stale line numbers,
and gives the count the error stays live for as eight, every C++ unit the
engine binary links, `main.cpp` included -- the number of profile files
pass 2 prints. `src/CMakeLists.txt` is not touched.

The exemption over moving `leaf_walk.cpp` out of `chesso_engine`: its
absence is the same structural absence as `search_params.cpp`'s, the
engine binary referencing none of its symbols, so it is the existing rule
applied to a second file and not a new mechanism. Moving it would break the
convention that `chesso_engine` and `chesso_engine_trace` are the `SRCS`
glob, take a new library and new link lines in `tools/CMakeLists.txt` and
`tests/CMakeLists.txt`, and buy the same `chesso` binary. `-Werror` stays,
`-Wmissing-profile` stays an error for the eight, and the two exempt
objects are not in the binary. Observed rather than assumed: in
`build-release-portable` after pass 3, `flags.make` carries
`-Wno-missing-profile` for those two objects only, and with
`utils.cpp.gcda` moved aside and `utils.cpp.o` removed, the rebuild
stopped on `-Werror=missing-profile` for `src/utils.cpp`; with the file put
back it rebuilt and benched 5434393.

### The three release builds

Each `./build_release.sh <arch>` from a clean directory, the fix applied.
`build_release.sh` prints the workload's total and not a `chesso bench`, so
the bench is each binary's `bench` command, run after the script:

| arch | passes | wall | workload | profile files | popcnt | `chesso bench` |
|---|---|---|---|---|---|---|
| `portable` | 3 of 3, exit 0 | 41 s | 29465966 nodes, 30 s | 8 | 176 | 5434393 |
| `bmi2` | 3 of 3, exit 0 | 42 s | 29465966 nodes, 31 s | 8 | 174 | 5434393 |
| `avx2` | 3 of 3, exit 0 | 41 s | 29465966 nodes, 30 s | 8 | 174 | 5434393 |

`build/src/chesso bench`: 5434393, the parent's. No warning in any log.
`id name` reads `Chesso 173ade1-dirty <arch>`, the edit being uncommitted.

### datagen

`build/tools/datagen` builds and defines `leaf_walk_pv`, 13 of its
symbols naming `leaf_walk`. In `build-release-portable`, after pass 3,
`--target datagen test_leaf_walk` built under `CHESSO_PGO=use`, `datagen`
defines `leaf_walk_pv`, and `ctest -R '^test_leaf_walk$'` passed.

### The suite against a release directory

`cmake --build build-release-bmi2 -j12 && ctest --test-dir build-release-bmi2
-L fast`, DEV_MANUAL's recipe for the suite against the shipping binary,
which compiles `chesso_engine` with the same flags and so met the same
error from S082 on (read, not re-run on the parent): the whole directory built
under `CHESSO_PGO=use`, and 49 of 50 passed, the failure being
`test_clang_format_script` run without `CLANG_FORMAT_MAJOR=22` (this
machine's override, DEC-146); with it exported that test passed alone.

### Accepts, clause by clause

- **The three arches complete all passes and print their bench, equal to
  `build/`'s:** held, the table above. `build_release.sh` itself prints
  the workload total and no `chesso bench`; each binary's `bench` was run
  after it, 5434393 three times against `build/`'s 5434393. Adding a bench
  line to the script was outside `touches:` and was not done.
- **Pass 3 raises no `-Werror=missing-profile`:** held, no warning or
  error in any of the three logs.
- **`-Werror` kept, every file the engine binary links profiled:** held.
  `-Werror` is on every command line; the exemption covers the two objects
  absent from the binary, and the moved-aside `utils.cpp.gcda` showed the
  error still fires for a linked file.
- **`datagen` builds and links `leaf_walk`:** held, in `build/` and in a
  `CHESSO_PGO=use` directory, `test_leaf_walk` passing there.
- **The fix stated with why it is the scope:** the exemption in
  `cmake/pgo.cmake`, reasons above.
- **Gate green in both builds:** the completion command, both builds, 50 of
  50 each, `./clang-format.sh --check` clean under `CLANG_FORMAT_MAJOR=22`,
  on the tree as committed less the `done:` stamp and the closing
  paragraph, both re-read by the five prose modes after.
- **Excludes:** no `src/` file changed, `build/` benches the parent's
  5434393, datagen's sources and link lines untouched, and no timing is
  claimed: the wall times above are the script's own.

Docs: DEV_MANUAL.md's exemption paragraph said one file was exempt and
eight of nine got a profile; it now names both files and eight of ten, and
a paragraph records that release builds work again. MANUAL.md checked: its
release section says the three binaries are the same engine node for node,
which the bench above confirms, and needs no change. The five
`tools/plan_prose_check.py` modes flag nothing. Tier 2 does not bind: no
change to `make_move`, `unmake_move`, the generator or the search, only a
compile option on two files the engine binary does not link.

`build-release-portable`, `build-release-bmi2`, `build-release-avx2` and
`.pgo/` were removed from the worktree after the readings above, 124 MB in
all.
