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
aimed at the depth this step prunes. The general cases — `tests/test_search.cpp`
"mate in one" and "mate in two is found at the right distance" — do not state
that they sit inside the depth window futility and razoring prune, and neither
title asserts it. The completion gate is the accepts, so S026 can complete on
pre-existing coverage alone.

The adjacent step with the identical hazard spells the requirement into its
gate: `adocs/plan_done/S033_reverse_futility_pruning.md:3` — "a position with a
forced mate inside the pruned depth is in the fast suite and passes before the
feature is called done". Two steps, one hazard, two different gates.

## The shape to copy

Do not invent one. S033 added `tests/test_search.cpp` "pruning does not hide a
mate against the material leader" on 2026-08-16 (`6bd650e`, the implementation
commit; S033 completed at `fca9522` the same day), against the same
hazard: a static score is never a mate score, so a node whose true value is
mated can fail high on material and take its whole subtree with it. Its comment
names the three properties that make the position bite — the mated side is
ahead by 500 cp after the key move, the key is quiet, and the mate is inside the
pruned depth — and asserts the first two as preconditions rather than assuming
them:

```cpp
REQUIRE(load_FEN(after_key, &game));
REQUIRE_FALSE(is_check(&game));
REQUIRE(evaluate(&game.board) > 300);
```

That is what makes it non-vacuous. Without those three lines the case passes on
an engine that never comes near the rule: a checking key would leave the side to
move in check, where the pruning is already forbidden, and a static score that
never clears beta means no static cutoff was reachable in the first place. The
mate is then asserted over a range of depths rather than one bound:

```cpp
for (int depth = 3; depth <= 6; ++depth)
```

S026's two cases copy that shape — a position, the preconditions that put the
node where the rule fires, the mate asserted over a range of depths. The
positions themselves are S026's to build; the form has already been through a
review and does not need re-deriving.

CLAUDE.md names pruning that hides a mate as the recurring bug in this engine —
null move pruning reduced to depth 0, late move reduction reduced the mating
move at the root — and both times a mate test caught it rather than a benchmark.
The clause goes in once per technique, since forward futility and razoring prune
at different places.

Full evidence: 2026-08-13_plan_review.2-F05.
