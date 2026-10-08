id:         S260
goal:       both gated builds compile on Apple clang again: the constant tests/test_search.cpp declares and no code reads stops failing -Werror
accepts:    red first -- `cmake --build build -j8` on the MacBook fails with `unused variable 'ORDINARY_BETA' [-Werror,-Wunused-const-variable]` at `tests/test_search.cpp` `ORDINARY_BETA`, and the failure is recorded before the fix; the fix keeps the comments that cite the constant meaningful (they explain a choice made against it), so the constant is either marked `[[maybe_unused]]` with a one-line reason or deleted with every comment that names it re-worded in the same commit; then the TESTS rule's whole gate passes on the MacBook in **both** builds, and a full build of both trees finds no second Apple-clang-only warning (`.moltke.local.md` records the class: clang enables `-Wunused-const-variable` under `-Wall` where gcc does not); `No functional change` -- the test binary is the only thing that changes and `bench` is the parent's
touches:    tests/test_search.cpp
excludes:   any change to src/; any change to what a test asserts; turning the warning off in CMakeLists.txt, which would hide the next one
decisions:  DEC-171, DEC-261
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-08. Red first: `cmake --build build -j8` at `629d0b3` stopped at `tests/test_search.cpp:6055:24: error: unused variable 'ORDINARY_BETA' [-Werror,-Wunused-const-variable]`, recorded before the fix. `ORDINARY_BETA` is `[[maybe_unused]]` with a one-line reason; the five comments that cite it are unchanged. `--clean-first` rebuilds of `build` and `build-tune` print no warning: no second Apple-clang-only one. Gate green in both builds (43/43) and clang-format, after the owner installed coreutils 9.12 for `test_fastchess_script` (below); `tools/gate.sh` then timed that test out once at its 60 s limit, and the owner ruled the build is the bar here (DEC-262). Bench 4081329, the parent's: No functional change. DEV_MANUAL and MANUAL checked, no change. Not a search, generator or make_move change, so no Debug self-play or gate_extra.

## Why it is a step

Found 2026-10-07 by the corrected plan comparison and confirmed 2026-10-08:
`cmake --build build --target test_search` stops at the constant. gcc on the
workstation does not flag it, so the gate is green there and red on the
MacBook. It is not reachable in play and moves no score, so under DEC-171 it
is filler, not fix-first; it is placed first anyway because every commit made
on the MacBook until it lands is red, and the owner authorised the plan
rewrite's two commits that way once (DEC-261), not as a habit.

The constant sits in the guard-fixture namespace; its later mentions are all
comments recording why another value was chosen instead. S167 fixed the first
member of this class.

## Red first, 2026-10-08

At `629d0b3`, before any change, `cmake --build build -j8` stops at the
test binary and nowhere else:

```
/Users/max/ws/chesso/tests/test_search.cpp:6055:24: error: unused variable 'ORDINARY_BETA' [-Werror,-Wunused-const-variable]
 6055 |   static constexpr int ORDINARY_BETA = 100;
      |                        ^~~~~~~~~~~~~
1 error generated.
ninja: build stopped: subcommand failed.
```

Apple clang 16, `-O3 -DNDEBUG -std=gnu++20 -arch arm64 -march=native -Wall
-Wextra -Werror`; the engine library linked before the failure.

## The fix

`ORDINARY_BETA` is marked `[[maybe_unused]]`, with one line added under its
comment: no code reads it, and it stays because five later comments cite it
(four null-move drives that take the node's static score instead, and the
mate case that cites the constant and the value 100). Deleting it would
have meant re-wording all five in the same commit; the attribute keeps every
one of them true verbatim. No assertion, no `src/`, no `CMakeLists.txt`.

## Verified

- **Sweep, both trees.** `cmake --build build -j8 --clean-first` (Ninja) and
  `cmake --build build-tune -j8 --clean-first` (Unix Makefiles) each
  recompiled 49 objects under `-Wall -Wextra -Werror` and printed no
  `warning` or `error` line. No second Apple-clang-only warning exists.
- **Gate, both builds: 42 of 43 fast tests pass, the same 42 in each.**
  The one failure is `test_fastchess_script`, in both, and it is not this
  step's: every case stops at `SPRT-RUN-FAILED: neither timeout nor gtimeout
  on PATH, so neither side can be asked what it is -- brew install coreutils
  on macOS`. `fastchess.sh` has needed a GNU `timeout` since S212
  (`f9d705c`), and coreutils is not installed on the MacBook, so the script
  itself refuses here too; `.moltke.local.md`'s "`fastchess.sh`: yes" dates
  from S177, before that. Installing coreutils is the owner's call, not this
  step's. `test_search` passes in both builds.
- **`./clang-format.sh --check`**: exit 0, no output.
- **Bench: 4081329**, the total `65e29ba` records and the six `No functional
  change` commits since carry. The engine binary does not change.

## Coreutils, 2026-10-08

With the build fixed, the gate read 42 of 43 in both builds, the failure
above. The owner installed coreutils by hand (9.12, `gtimeout` and `timeout`
now in `/opt/homebrew/bin/`), and the coordinator re-ran the TESTS rule's
whole gate: both builds compile, 43 of 43 fast tests pass in each,
`./clang-format.sh --check` exits 0. `chesso bench` reads 4081329. When
coreutils went missing on this machine is not recorded; S120 and S258's
43/43 were taken after `ORDINARY_BETA` went dead (`bd5a6cb`, 2026-10-01),
so they were not taken on the MacBook.

The commit gate, `tools/gate.sh --message`, then failed once on
`test_fastchess_script` by its 60 s ctest timeout: run alone three times it
takes 59.2 to 59.3 s, now that every case runs to its end instead of refusing
at the missing `timeout`. The owner ruled the build is the bar here and
nothing is fixed (DEC-262); S260 lands on it.
