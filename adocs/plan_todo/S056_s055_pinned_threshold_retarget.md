id:         S056
goal:       S055's accepts re-targets the pinned thresholds to the post-merge bound instead of asking for a suite state the merge makes unreachable
accepts:    S055's accepts names the re-target instead of "passes over the pinned corpus at that tolerance": the tolerance returns to 2, "the pinned positions reach the truncation bound" is kept and re-pinned to the 2 x 23/24 = 1.917 bound with each threshold re-measured in S055's own commit, and the tempo precondition message's arithmetic becomes 3 x 23/24 = 2.875 with a tolerance of 4 becoming 3; S055's body states that the thresholds are a non-vacuity guard on the pinned corpus and that the guard is kept with new numbers; tests/test_eval_model.cpp is named in S055's touches
touches:    adocs/plan_todo/S055_taper_stage_two_once.md
excludes:   merging the taperings, which is S055 itself; any edit to tests/test_eval_model.cpp, src/evaluation.cpp or tools/eval_model.hpp; any change to the tolerance DEC-053 set
decisions:  DEC-053
closes:     2026-08-13_plan_review.2-F01
blocks:
paused_by:
done:

## What is there

`S055_taper_stage_two_once.md:3` accepts "the tuner-model tolerance returns to
2 and test_eval_model passes over the pinned corpus at that tolerance". That
corpus carries two cases, not one. The second is the non-vacuity guard S038
added at `tests/test_eval_model.cpp:381-422`, and it asserts the opposite of a
small disagreement: `:412` `CHECK_MESSAGE(difference > 2.0, ...)` for each of
the four FENs in `truncation_positions` (`:215-220`), and `:420`
`CHECK_MESSAGE(worst > 2.8, ...)`.

Those four were pinned *because* they exceed 2.0 under today's three effective
truncations. S055's own arithmetic puts the merged bound at 2 x 23/24 = 1.917,
below 2.0, so no position can satisfy `difference > 2.0` after the change. The
audit measured it rather than arguing it: the four merged disagreements are
1.8750, 1.3333, 1.2500 and 1.1250, so the merge turns five passing assertions
into five failures in a test the fast suite runs (`test_eval_model`, ctest #7).
Re-pinning other positions does not help — the new bound is under the threshold
for every position.

Two more statements in the same case go false and S055 mentions neither: the
tempo precondition message at `:388-391` ("the bound is now 4 x 23/24 = 3.833
and the tolerance ... has to be 4" — after the merge that is 3 x 23/24 = 2.875
and 3), and the comment at `:366-373` that explains the thresholds as "the old
2.0".

The resolutions S055 currently invites are both bad. Lowering the two
thresholds with no authority in the step reads as editing a test to go green,
which AGENTS.md §11 prohibits and §6 allows only as a deliberate re-target.
Deleting the case removes the only reason the tolerance is a bound rather than
a number someone widened — the defect `2026-08-13_adversarial-F04` was raised
for and S038 fixed. This step writes the re-target into S055 so the diff is
reviewable as one: the guard is kept, with new numbers against a bound the step
itself tightens.

Full evidence: 2026-08-13_plan_review.2-F01.
