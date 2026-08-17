id:         S061
goal:       S023's disjoint-bands criterion names a test that exercises the 100-point capture-to-killer clearance
accepts:    S023's accepts names the case that discharges "the ordering bands stay disjoint" — a king capture of a pawn scored against a killer at the same ply, asserting the capture wins — and states the bound on the new capture-history term that keeps a capture score inside its band; the criterion is discharged by something ctest runs rather than by reading the diff; the test itself is written in S023's own commit
touches:    adocs/plan_todo/S023_capture_history.md
excludes:   implementing capture history, which is S023 itself; writing the test now; any edit to src/evaluation.cpp or tests/test_evaluation.cpp
decisions:
closes:     2026-08-13_plan_review.2-F06
blocks:
paused_by:
done:

## What is there

`S023_capture_history.md:3` accepts "an SPRT returns a verdict; the ordering
bands stay disjoint, which the move-ordering hazard in CLAUDE.md makes easy to
break silently". Nothing in `ctest` can discharge the second half.

The hazard is the 100-point clearance at the bottom of the capture band: a king
capturing a pawn scores `ORDER_CAPTURE + MVV_PAWN - MVV_KING = 900100` against
`ORDER_KILLER_0 = 900000` (`src/evaluation.cpp:20-36`, whose own comment says
the symptom of losing it "is a strength regression, not a wrong node count").

The only test over the bands is `tests/test_evaluation.cpp` "bands are strictly
ordered", and it compares `a_capture` — whichever capture the generator returns
first — against the killers. Measured in the position it loads
(`r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`),
re-run at HEAD against `libchesso_engine.a`:

```
legal moves 48  captures 8  king captures 0
first capture score 1000000
worst capture score 999200   killer band 900000
clearance over the killer band: 99200
```

So `REQUIRE(s_capture > s_killer0)` holds with 99200 points to spare and the
pair the hazard is about is not reachable in that position at all.
`grep -rn "900100\|MVV_KING" tests/` returns nothing.

S023 adds a term to the capture score, which is exactly the change that can
consume the clearance, and the project's own documentation says that failure is
invisible to node counts and shows up only as lost Elo. Non-vacuity comes free
in the case the accepts should name: it fails if the position stops offering a
king capture.

Full evidence: 2026-08-13_plan_review.2-F06.
