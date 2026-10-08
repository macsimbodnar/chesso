id:         S260
goal:       both gated builds compile on Apple clang again: the constant tests/test_search.cpp declares and no code reads stops failing -Werror
accepts:    red first -- `cmake --build build -j8` on the MacBook fails with `unused variable 'ORDINARY_BETA' [-Werror,-Wunused-const-variable]` at `tests/test_search.cpp` `ORDINARY_BETA`, and the failure is recorded before the fix; the fix keeps the comments that cite the constant meaningful (they explain a choice made against it), so the constant is either marked `[[maybe_unused]]` with a one-line reason or deleted with every comment that names it re-worded in the same commit; then the TESTS rule's whole gate passes on the MacBook in **both** builds, and a full build of both trees finds no second Apple-clang-only warning (`.moltke.local.md` records the class: clang enables `-Wunused-const-variable` under `-Wall` where gcc does not); `No functional change` -- the test binary is the only thing that changes and `bench` is the parent's
touches:    tests/test_search.cpp
excludes:   any change to src/; any change to what a test asserts; turning the warning off in CMakeLists.txt, which would hide the next one
decisions:  DEC-171, DEC-261
closes:
blocks:
paused_by:
author:
done:

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
