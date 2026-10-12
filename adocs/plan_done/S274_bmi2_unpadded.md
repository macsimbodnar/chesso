id:         S274
goal:       the shipped bmi2 binary is built without branch padding, native keeps it, and the documentation states that a Skylake-family build gains 1.5 to 3 % from the flag and how to get it
accepts:    (1) `cmake/arch.cmake` applies `-Wa,-mbranches-within-32B-boundaries` for `CHESSO_ARCH` native only, and `CHESSO_BRANCH_PAD` records OFF for bmi2, avx2 and portable (each configured and checked); (2) `DEV_MANUAL.md`'s build paragraph, padding paragraph and arch-table rows state it, each statement traced to the CMake that sets it (DEC-017), and say that a Skylake-family part (Intel 6th to 10th generation desktop) gains 1.5 to 3 % wall from the flag, measured single-process on this machine (S273), with the exact way to build such a binary, while its cost on parts without the erratum stays unmeasured; (3) `build_release.sh bmi2` (the PGO path) builds with the flag in none of its compile commands and `bench` reads 4081329; native's `build/` is unchanged (flag in every C++ compile command, INV-6 trivially: no source moves); (4) both fast suites green, format clean, prose checks clean
touches:    cmake/arch.cmake, DEV_MANUAL.md
excludes:   any change to src/; a second shipped binary (DEC-267 rejected it); native's padding
decisions:  DEC-267, DEC-266, DEC-017
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-12 -- cmake/arch.cmake pads native only: CHESSO_BRANCH_PAD records ON for native and OFF for bmi2, avx2 and portable, each configured (C++ commands carrying -Wa,-mbranches-within-32B-boundaries: 50/50, 0/50, 0/50, 0/50; the assembly unit 0/1 throughout); native's 51 compile commands identical to the parent's. build_release.sh bmi2 (PGO) builds with the flag in none of its compile commands or flags.make, bench 3872032 (the parent's total; the accepts' 4081329 predates S119 Part C). DEV_MANUAL.md (flag paragraph, padding paragraph, a new Skylake-family paragraph, the REF note, the arch table's bmi2 row) and the arch.cmake comment state it per DEC-267: a Skylake-family part (Intel 6th to 10th generation desktop) gains 1.5 to 3 % wall from the flag, single-process on the workstation (S273); a padded bmi2 is one edit of the padding if() in cmake/arch.cmake, then ./build_release.sh bmi2 (configured and checked: ON, 50/50); not CXXFLAGS (checked: the flag lands, the cache records OFF); its cost on parts without the erratum is unmeasured. No switch, deliberately. Fast check: three DEV_MANUAL.md fixes (DEC-017) -- the PGO workload figure (S104's 117329931 nodes / 63 s) restated as what the script prints, dated 2026-10-12 at 298788b (21219197 nodes, 20 s); the search_params.cpp callers named as command_uci and command_setoption instead of stale line numbers; the S273-to-S274 bmi2 window covering the candidate under CAND too and excluding S274's commit. cmake/pgo.cmake's same stale line numbers left, outside touches. MANUAL.md checked, no change. Suites 44/44 in build and build-tune on fresh configures, format clean, prose checks clean, re-run after the fixes. No second tier owed: nothing in src/ moved.

## Why

The owner ruled on S273's bmi2 call (DEC-267): bmi2 runs on Haswell,
Broadwell, Ice Lake on and Zen3 on, which lack the JCC erratum, and the
padding's cost there is unmeasured, so the shipped binary stays unpadded; the
Skylake-family gain is documented as available rather than shipped.

## Lane

Agent work, about 15 minutes of builds; it needs the machine free of timings
and matches, so it runs between S119's hand-back and its SPRT.

## As built (2026-10-12, Opus subagent)

### Deviations, first

1. **`bench` reads 3872032, not the 4081329 accepts (3) names.** The step file
   was written before S119 Part C (`298788b`, `Bench: 3872032`) landed; that is
   this step's parent, and the brief asks for the parent's total. Both the PGO
   `bmi2` binary and the native `build/` read it.
2. **No switch to build a padded `bmi2`** (S273's deviation 6 kept). DEC-267
   rejected a second shipped binary, and a cache option would let a build and a
   reference disagree by choice. The way is one edit of `cmake/arch.cmake`,
   documented there and in `DEV_MANUAL.md`, and configured here to check it
   (below).
3. **A trap found while checking that edit.** `cmake/arch.cmake` has two
   identical `if(CHESSO_ARCH STREQUAL "native")` lines, the Arch message's and
   the padding's. A blanket `sed` over both, as first tried here, configures a
   padded `bmi2` that prints `-- Arch native (-march=x86-64-v3) -- THIS MACHINE
   ONLY, never distributed`. Both documents name the right `if()`, the one that
   sets `chesso_branch_pad_flag`.
4. **Three DEV_MANUAL.md fixes from the fast check (DEC-017), beyond the
   padding text.** The PGO workload's "117329931 nodes, about 63 s
   instrumented" dated from S104; it now says what the script prints (the sum
   over the positions of the last `info ... nodes`, `build_release.sh`'s
   driver, and the instrumented pass's wall), that it moves with the search,
   and today's reading, 21219197 nodes in 20 s at `298788b`. The
   `#ifdef CHESSO_TUNE` callers of `src/search_params.cpp` were cited by line
   number in `src/chesso.cpp`, 934 and 1043, and now sit at 1530 and 1708; the
   text names `command_uci` and `command_setoption` instead (no other `src/`
   caller of its four functions, grepped). The REF note's S273-to-S274 `bmi2` window
   now covers the candidate under `CAND` too (`build_ref` builds both sides
   with `build/`'s arch, `fastchess.sh` lines 584 to 590 and 736) and
   excludes S274's own commit.
5. **Left as found, outside `touches:`.** `fastchess.sh`'s comment at
   `build_ref` (lines 710 to 713) says only that a commit before S273 builds
   unpadded, true for native. `cmake/pgo.cmake` lines 39 and 40 carry the same
   two stale line numbers.

### Per arch

Scratch configures under the worktree (`build-scratch-<arch>`, Release,
removed after), counted from `compile_commands.json`; the parent's from a
`git archive` of `298788b` configured in the scratchpad.
`.tuning/coord/S274/per_arch.txt` has the output.

| `CHESSO_ARCH` | configure prints | `CHESSO_BRANCH_PAD` | C++ commands with the flag | assembly | parent `298788b` |
|---|---|---|---|---|---|
| `native` | `Branch padding on (-Wa,-mbranches-within-32B-boundaries)` | `ON` | 50 of 50 | 0 of 1 | `ON`, 50 of 50; the 51 commands identical, paths normalised |
| `bmi2` | `Branch padding off for bmi2` | `OFF` | 0 of 50 | 0 of 1 | `ON`, 50 of 50 |
| `avx2` | `Branch padding off for avx2` | `OFF` | 0 of 50 | 0 of 1 | `OFF` (S273's As built, not re-run) |
| `portable` | `Branch padding off for portable` | `OFF` | 0 of 50 | 0 of 1 | `OFF` (S273's As built, not re-run) |

Also configured: a `native` directory reconfigured to `bmi2` reads `OFF` and
0 of 50 (the cache entry is `INTERNAL`, so forced); the documented edit (the
padding `if()` given `OR CHESSO_ARCH STREQUAL "bmi2"`) on a fresh `bmi2` reads
`Arch bmi2`, `Branch padding on`, `ON`, 50 of 50, and was reverted (`cmp`
against the kept copy); `CXXFLAGS=-Wa,-mbranches-within-32B-boundaries` on a
fresh `bmi2` reads 50 of 50 but `OFF`, which is why the documents warn off it.

### Docs

- `cmake/arch.cmake`: the padding `if()` is `native` only; the comment says
  `bmi2` does not take it and why (S274, DEC-267), that a Skylake-family part
  (Intel 6th to 10th generation desktop) gains 1.5 to 3 % wall, single-process
  on the workstation (S273), the edit that builds such a `bmi2` and the `if()`
  it is not, that `CXXFLAGS` leaves `CHESSO_BRANCH_PAD` lying, that the default
  native build on such a machine has it, and that the cost elsewhere is
  unmeasured. "The shipped PGO bmi2 build" in S273's reading is now "a padded
  PGO bmi2 build".
- `DEV_MANUAL.md`, Build: the flag paragraph says padding where the arch is
  `native` (traced to the `if()`); the padding paragraph says only `native`
  takes it and why, the configure example shows `Branch padding off for bmi2`
  (observed), a new paragraph gives the Skylake-family gain, the edit, what
  configure then prints and records (observed), the `-dirty` in `id name`
  (`cmake/build_info.cmake`, `git diff --quiet HEAD`), the `CXXFLAGS` trap
  (observed) and the unmeasured cost; the REF paragraph adds the S273-to-S274
  `bmi2` window, both sides under `CAND`, S274's commit excluded. Release
  section: the workload figure and the `search_params.cpp` callers (deviation
  4). Arch table: `bmi2`'s flags drop "branch padding", and its row
  says unpadded (DEC-267), the gain, the unmeasured cost and the edit;
  `native`'s row unchanged.
- `MANUAL.md` checked: no option, output or behaviour a user sees moved -- no
  change. `specs.md` names no padding (its `bmi2` is `-march=x86-64-v3`) -- no
  change proposed. `TOOLCHAIN.md` names no padding.

### The PGO path

`./build_release.sh bmi2`, exit 0 in 30 s
(`.tuning/coord/S274/build_release_bmi2.log`): both configures printed
`Branch padding off for bmi2`; workload 400 positions, 21219197 nodes, depth 10
in 20 s, 8 profile files; 140 `popcnt`. `CHESSO_BRANCH_PAD:INTERNAL=OFF`; the
flag in 0 of 50 C++ commands and 0 of 1 assembly, and in none of the 43
`flags.make` (all 43 carry `-march=x86-64-v3`); 10 commands carry
`-fprofile-use`. `bench` **3872032** nodes, the parent's total;
`id name Chesso 298788b-dirty bmi2`. Native `build/`: fresh configure, `ON`,
the flag in 50 of 50 C++ commands and 0 of 1 assembly, the commands identical
to the parent's; `bench` 3872032. INV-6 trivially: no source moved.

### Suites

On fresh configures of both trees (ccache), `CLANG_FORMAT_MAJOR=22`:
`cmake --build build -j12 && ctest --test-dir build -L fast --output-on-failure
&& cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast
--output-on-failure && ./clang-format.sh --check` **exit 0: 44/44, 44/44
(`test_fastchess_script` among them), format clean, 0 warnings**, `-Werror`
on (`.tuning/coord/S274/gate.log`). Both caches `CHESSO_BRANCH_PAD ON`, the
flag in 50 of 50 C++ commands and 0 of 1 assembly in each.
`tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, one mode
per call: each exit 0, 0 flagged (re-run after this section was written). No
timing, no `fastchess`, no `gate_extra`: nothing in `src/` moved. Re-run in
full after the fast check's three `DEV_MANUAL.md` fixes: the first re-run
went red on `test_plan_citation_freshness` alone, from two line-form citations
(`path:line`, DEC-135) this section had quoted while describing deviation 4,
reworded; the second **exit 0, 44/44, 44/44, format clean, 0 warnings**
(`.tuning/coord/S274/gate3.log`), and the three prose modes each exit 0, 0
flagged.

### Proposed `done:` stamp

2026-10-12 -- cmake/arch.cmake pads native only: CHESSO_BRANCH_PAD records
ON for native and OFF for bmi2, avx2 and portable, each configured (C++
commands carrying -Wa,-mbranches-within-32B-boundaries: 50/50, 0/50, 0/50,
0/50; the assembly unit 0/1 throughout); native's 51 compile commands identical
to the parent's. build_release.sh bmi2 (PGO) builds with the flag in none of
its compile commands or flags.make, bench 3872032 (the parent's total; the
accepts' 4081329 predates S119 Part C). DEV_MANUAL.md (flag paragraph, padding
paragraph, a new Skylake-family paragraph, the REF note, the arch table's bmi2
row) and the arch.cmake comment state it per DEC-267: a Skylake-family part
(Intel 6th to 10th generation desktop) gains 1.5 to 3 % wall from the flag,
single-process on the workstation (S273); a padded bmi2 is one edit of the
padding if() in cmake/arch.cmake, then ./build_release.sh bmi2 (configured
and checked: ON, 50/50); not CXXFLAGS (checked: the flag lands, the cache
records OFF); its cost on parts without the erratum is unmeasured. No switch,
deliberately. Fast check: three DEV_MANUAL.md fixes (DEC-017) -- the PGO
workload figure (S104's 117329931 nodes / 63 s) restated as what the script
prints, dated 2026-10-12 at 298788b (21219197 nodes, 20 s); the
search_params.cpp callers named as command_uci and command_setoption instead
of stale line numbers; the S273-to-S274 bmi2 window covering the candidate
under CAND too and excluding S274's commit. cmake/pgo.cmake's same stale line
numbers left, outside touches. MANUAL.md checked, no change. Suites 44/44 in
build and build-tune on fresh configures, format clean, prose checks clean,
re-run after the fixes. No second tier owed: nothing in src/ moved.

### Commit text

```
Build the shipped bmi2 binary without branch padding (S274)

DEC-267: bmi2's x86-64-v3 parts include Haswell, Broadwell, Ice Lake on
and Zen3 on, which lack the Skylake family's JCC erratum. There S273's
padding is +0.8 to +0.9 % instructions and +3 % code in the PGO build,
and what it costs in time nothing here can measure, so the shipped
binary goes unpadded.
cmake/arch.cmake now pads native only, and CHESSO_BRANCH_PAD records OFF
for bmi2, avx2 and portable. Native's compile commands are unchanged,
and every match, timing and rating run builds native.

DEV_MANUAL.md and the arch.cmake comment say what DEC-267 asks: a
Skylake-family part gains 1.5 to 3 % wall from the flag (S273,
single-process, the workstation), a padded bmi2 is one edit of the
padding if() plus ./build_release.sh bmi2, and the cost elsewhere is
unmeasured. No CMake option: DEC-267 keeps one shipped binary.

From the fast check (DEC-017): DEV_MANUAL.md's PGO workload figure was
S104's and is now dated, the search_params.cpp callers are named rather
than cited by stale line numbers, and the padding window covers a CAND
candidate and excludes S274's own commit.

S274, DEC-267.
```
