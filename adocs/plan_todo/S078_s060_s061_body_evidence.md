id:         S078
goal:       S060 points at the mate-inside-the-pruned-depth test S033 added, and S061's citation names the case it means
accepts:    `S060`'s body stops saying no test asserts the property and points at `tests/test_search.cpp` "pruning does not hide a mate against the material leader" as the template S026's two cases copy, naming how it asserts its preconditions; `S061`'s citation names the band case by its `TEST_CASE_FIXTURE` title rather than by a line range; neither step's `accepts:` changes, because both are correct
touches:    adocs/plan_todo/S060_s026_mate_test_in_gate.md, adocs/plan_todo/S061_s023_band_clearance_test.md
excludes:   the line-number repairs in the other four step files, which are S079's; S026's and S023's own gates, which S060 and S061 exist to set and which the audit found sound
decisions:
closes:     2026-08-16_plan_review-F09
blocks:
paused_by:
done:

## Why this exists

Both steps argue from an absence that S033 filled on 2026-08-16 (`fca9522`).

`S060:19-24` says:

> Nothing states that the existing cases -- `tests/test_search.cpp:82` "mate in
> one" and `:124` "mate in two is found at the right distance" -- sit inside the
> depth window futility and razoring prune, and no test name asserts it.

Both citations still resolve, and the sentence is now false of the file:

```
$ grep -n 'TEST_CASE_FIXTURE' tests/test_search.cpp | grep -i 'prun'
974:  TEST_CASE_FIXTURE(search_fixture_t, "pruning does not hide a forced mate")
1013:  TEST_CASE_FIXTURE(search_fixture_t,
1014:                    "pruning does not hide a mate against the material leader")
```

`tests/test_search.cpp:999-1008` states the three properties that make the second
bite, including that the mate is inside the pruned depth, and asserts two of them
as preconditions at `:1022-1024`.

**Neither `accepts:` is wrong.** S026's gate really does not carry the clause and
S023's really has no instrument, which is what S060 and S061 were created for.
What is wrong is that S060 argues from a blank page when a worked example now
exists in the same file -- and a step that says "no test does this" invites
whoever executes it to invent a shape instead of copying one that has already
been through a review.

S061's range is off by nine lines at both ends: it names
`tests/test_evaluation.cpp:643-704`; the case runs `:652-713`. Titles do not
move; that is why this step replaces the range rather than correcting it.

## Cost

Minutes, no build, no match.
