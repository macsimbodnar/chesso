id:         S017
goal:       a test over the UCI command and option surface that fails when it changes
accepts:    a test in the fast suite that enumerates every UCI command chesso answers and every option it advertises, and fails when one is added, renamed or removed; MANUAL.md documents the same surface and the test fails until it is updated in the same commit
touches:    tests/, src/uci or equivalent, MANUAL.md
excludes:   fastchess --compliance, which checks protocol conformance rather than pinning this engine's surface; keep test_uci.sh as it is
decisions:  DEC-009
closes:
blocks:
paused_by:
done:

## Why this is first

`surface_guard` is `cli`, and AGENTS.md section 7 makes the golden surface test
mandatory. It does not exist: `test_uci.sh` is three lines running
`fastchess --compliance` and is not in ctest. Until this step lands, every step
completion asserts a check that nothing performs.

It also has to land before S021 and anything else that adds a UCI option, which
is the point: the test is what forces MANUAL.md to stay true.
