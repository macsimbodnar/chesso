id:         S059
goal:       S025's gate books the outcome its own evidence predicts: a timing that is worse is the recorded verdict
accepts:    S025's accepts reads outcome-neutral in the shape S051 gave S020 and S030 — fixed-depth time measured against the two-stage build on all three search_bench positions with its noise floor recorded, the SPRT run only if the timing is not worse, and a timing that is worse recorded as the verdict with the step completing on it; no clause in S025 still requires a non-worse timing to complete
touches:    adocs/plan_todo/S025_bad_capture_ordering_retry.md
excludes:   rebuilding bad-capture ordering, which is S025 itself; any edit to src/search.cpp or src/evaluation.cpp; re-measuring the three timings recorded in S025
decisions:  DEC-019, DEC-022
closes:     2026-08-13_plan_review.2-F04
blocks:
paused_by:
done:

## What is there

`S025_bad_capture_ordering_retry.md:3` accepts "fixed-depth time is not worse
than the two-stage build on all three search_bench positions; only then an
SPRT". The same file records the measurement already taken, at depth 13 over
those three positions: exact `see()` in `score_move` +17.6 %, `see_ge` +13 %,
lazily in the picker +3 % — all worse, which is why DEC-022 set the step aside.

So the gate admits one outcome and its own evidence predicts the other. There
is no "measured worse, recorded, not retained" path, which is how S005, S006
and S015 completed. If the rebuilt version measures slower again the step cannot
complete as written, and the pressure lands where CLAUDE.md foundation 2 says it
must not — on finding a number rather than measuring one.

This is the shape `2026-08-13_plan_review-F08` identified and S051 fixed, in a
third file S051 could not reach: its `touches:` names only
`adocs/plan_todo/S020_single_check_computation.md` and
`adocs/plan_todo/S030_move_encoding_16_bit.md`, and its accepts is scoped to
"both steps' accepts". S020's rewritten gate is the model — "the cost measured
with hyperfine over interleaved fixed-depth runs, its noise floor recorded, and
the keep-or-revert call made from that number — a zero is recorded as zero and
does not block completion".

Low severity while S025 sits behind five other steps in the order. It is worth
fixing before it starts rather than while it is running.

Full evidence: 2026-08-13_plan_review.2-F04.
