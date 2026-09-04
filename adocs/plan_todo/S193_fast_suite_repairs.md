id:         S193
goal:       the fast suite's vacuous assertions are made falsifiable, its two overclaiming titles honest, the fifty-move boundary pinned, `test_perft` Release-safe, and the temp-file, date and case-order hazards removed
accepts:    a fifty-move boundary pair -- a root at clock 99 whose quiet replies land on clock 100 scores exactly 0 at depth 1 and the same root at clock 98 does not -- observed red under mutant M19 of `adocs/data/2026-09-04_test_review/mutants.py`, then green; `ucinewgame` before `tests/test_engine.cpp`'s "a search with no limit is still bounded" and the case shown to exercise its 200 ms budget; the `LOG_I`-string assertion in "setoption carries a value containing spaces" replaced by an observable or removed; a `count > 0` precondition in `tests/test_movegen.cpp`'s pinned-piece case; `tests/test_perft.cpp` refuses a missing or unparseable asset in Release, every column fails the run, and the dead `RUN_THREADS` block is removed; the titles of "the node budget is honoured exactly" and "an infinite search answers only once stop arrives" say what their bodies assert; `tests/test_helpers.hpp`'s `legal_moves()` comment and `tests/test_chesso.cpp`'s `test_generate_legal_moves` say what is true -- legality is pinned by the perft and JSON counts -- or assert it; the three fixed temp-file names replaced by `mktemp`; `tests/test_fastchess_script.sh`'s banner date taken from `git show` rather than `date +%F`; `tests/test_chesso.cpp` and `tests/test_openings.cpp` initialise the tables explicitly instead of through case order; `tests/test_audit_go_infinite.cpp` registered with its 200 ms sleep replaced by a condition that cannot pass on a slow machine **if** `2026-09-04_adversarial-F01`'s engine fix has landed, otherwise left unregistered and this stamp says so; each repair observed red under its own mutation before green where a mutation exists; fast suite green in both builds
touches:    tests/, tests/CMakeLists.txt
excludes:   any engine change; the `go infinite` engine fix itself (`2026-09-04_adversarial-F01`)
decisions:  DEC-139
closes:     2026-09-04_test_review-F04, 2026-09-04_test_review-F05, 2026-09-04_test_review-F09
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F04`, F05 and F09. One injected bug survived the whole
fast suite: the fifty-move draw claimed one halfmove late, because the only
direct case searches a root already at clock 100 and the root is exempt from
the draw test. Eleven assertions cannot fail for a reason unrelated to their
title -- a stop flag set by `position` before a timed search, a `LOG_I` string
that is `if (false)` in Release, a history of one entry, a literal compared
with itself, a clamp asserted after the clamp -- and `test_perft`'s asset check
is an `assert` compiled out of Release, so a missing asset exits 0. None is an
engine defect; together they are a dozen places where the green count
overstates what the suite holds. F09's items are the mechanical hazards the
same review listed: fixed temp-file names that collide under a parallel
`ctest`, a banner date that fails across midnight, two files whose later cases
depend on doctest's file order.

## Cost

Machine-free, hours.
