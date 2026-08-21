id:         S156
goal:       the mined breadth set is either asserted as the count with a floor its accepts asked for, or the accepts is discharged in the stamp with the reason
accepts:    `adocs/data/S145_mined_set.tsv` is either asserted in the suite as the count with a floor its `accepts` asked for -- with the floor placed from the measured 146 at `RfpMinPly` 2 and 3 against 139 at 1 and 0, and observed red below it -- or the clause is discharged in writing with the reason the measurement in the step body satisfies it; whichever way it goes, the file stops being data that nothing reads; if it is asserted, the cost it adds to the fast suite is measured and stated, since the suite runs at every step completion
touches:    tests/test_engine.cpp, adocs/data/, adocs/testing.md
excludes:   the constructed set and its assertions, which S145 landed; per-position pass/fail over mined mates, which S145's research shows is what made two surveyed projects disable their tests; re-mining the set from a different game source
decisions:  DEC-019
closes:     2026-08-21_adversarial-F08
blocks:
paused_by:
done:

## The gap

`grep -rn "mined_set|S145_mined" tests/ src/ CMakeLists.txt` returns one
comment at `tests/test_engine.cpp:1694` and nothing else. The 318 positions and
their script exist; no assertion, no floor, no ctest registration, and the
`done:` stamp does not mention them.
