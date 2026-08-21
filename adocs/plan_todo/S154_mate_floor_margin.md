id:         S154
goal:       the mate-in-three floor's tolerance to a neutral tree change is measured rather than asserted, and the RfpMinPly-1 mate-in-two count is restated as what the assertion actually fails
accepts:    `MATE_IN_THREE_FLOOR`'s tolerance is measured rather than asserted: how many of the sixteen mate-in-three detections move under a change already known to be behaviour-neutral or near-neutral, so the comment's claim that the floor "fails when the guard fails and not when the tree shifts underneath it" rests on a number; the floor is then either widened with the tolerance stated, or kept at 7 with the measurement recorded beside it; the step file and `adocs/data/S145_rfp_sweep.log`'s characterisation of the `RfpMinPly` 1 failure as "13 of 16" is restated as what the assertion actually fails -- 9 of 16 pass, because it also requires `first_exact == 2d-1` -- since the fence is stronger than documented and the next reader sizing the margin needs the real number; no default changes and the fast suite stays green
touches:    tests/test_engine.cpp, adocs/data/, adocs/plan_done/ is excluded -- see excludes
excludes:   rewriting `adocs/plan_done/S145_mate_safety_test_set.md`, which is history and is never rewritten -- the restatement lands in the test comment and in the sweep log's own header; the constructed set itself, which S145 built and which this step only measures against; `RfpMaxDepth`, which is S148
decisions:  DEC-019, DEC-095
closes:     2026-08-21_adversarial-F06
blocks:
paused_by:
done:

## Why the margin matters now

Plan positions 19 to 31 -- S093, S130, S108, S024, S109, S091, S098, S095, S099,
S097, S112, S131, S022 -- all reshape the tree, several substantially. The floor
sits one position of sixteen below the shipping count of 8. A floor that reddens
on an unrelated tree change is the mechanism S145 itself documented in two
surveyed projects, which "switched the tests off rather than the pruning".
