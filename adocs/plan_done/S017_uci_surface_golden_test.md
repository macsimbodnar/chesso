id:         S017
goal:       a test over the UCI command and option surface that fails when it changes
accepts:    a test in the fast suite that enumerates every UCI command chesso answers and every option it advertises, and fails when one is added, renamed or removed; MANUAL.md documents the same surface and the test fails until it is updated in the same commit
touches:    tests/, src/uci or equivalent, MANUAL.md
excludes:   fastchess --compliance, which checks protocol conformance rather than pinning this engine's surface; keep test_uci.sh as it is
decisions:  DEC-024
closes:
blocks:
paused_by:
done:      2026-08-09. tests/test_uci_surface.cpp added to the fast suite: 9 cases, 82 assertions, 0.03 s. Commands read from the new uci_command_names(), option lines read from the uci reply, both compared against a golden list and against MANUAL.md. Red observed three ways before green: a 17th command reported 'Present but not in the golden list: [eval]'; 'Threads ... max 4' reported both directions of the option diff; deleting the clean-tt and fine70 rows from MANUAL.md failed the two documentation cases. MANUAL.md checked and updated: position shortcuts, position/moves semantics, debug on|off, test <depth>. DEV_MANUAL.md checked and updated with what a surface failure means. README.md checked, owner-written, no change needed. go and position arguments are pinned by hand, not enumerated -- DEC-028. Gate green: cmake --build build -j8, ctest -L fast 7/7, clang-format.sh --check clean.

## Why this is first

`surface_guard` is `cli`, and AGENTS.md section 7 makes the golden surface test
mandatory. It does not exist: `test_uci.sh` is three lines running
`fastchess --compliance` and is not in ctest. Until this step lands, every step
completion asserts a check that nothing performs.

It also has to land before S021 and anything else that adds a UCI option, which
is the point: the test is what forces MANUAL.md to stay true.
