id:         S060
goal:       S026's accepts carries the mate-inside-the-pruned-depth clause its own body demands, per technique
accepts:    S026's accepts requires, once per technique, that a position with a forced mate inside the depth that technique prunes is in the fast suite and passes before the feature is called done — the clause S033's accepts already carries; the accepts and the body no longer disagree about whether a new test is required
touches:    adocs/plan_todo/S026_futility_and_razoring.md
excludes:   implementing futility or razoring, which is S026 itself; writing the mate test, which belongs to S026's own commit; any edit to src/search.cpp or tests/
decisions:
closes:     2026-08-13_plan_review.2-F05
blocks:
paused_by:
done:

## What is there

`S026_futility_and_razoring.md:3` accepts "an SPRT per technique, measured
separately; mate and tactical tests in the fast suite still pass". Its body,
two paragraphs down, asks for something else: "a position with a forced mate
inside the pruned depth, in the fast suite, before the feature is called done."

The accepts asks that existing tests keep passing; the body asks for a new test
aimed at the depth this step prunes. Nothing states that the existing cases —
`tests/test_search.cpp:82` "mate in one" and `:124` "mate in two is found at the
right distance" — sit inside the depth window futility and razoring prune, and
no test name asserts it. The completion gate is the accepts, so S026 can
complete on pre-existing coverage alone.

The adjacent step with the identical hazard spells the requirement into its
gate: `S033_reverse_futility_pruning.md:3` — "a position with a forced mate
inside the pruned depth is in the fast suite and passes before the feature is
called done". Two steps, one hazard, two different gates.

CLAUDE.md names pruning that hides a mate as the recurring bug in this engine —
null move pruning reduced to depth 0, late move reduction reduced the mating
move at the root — and both times a mate test caught it rather than a benchmark.
The clause goes in once per technique, since forward futility and razoring prune
at different places.

Full evidence: 2026-08-13_plan_review.2-F05.
