id:         S048
goal:       plan.md's list rule names a real property and holds; stale harness and price prose refreshed
accepts:    the list/step-file correspondence rule no longer cites INV-3, and either states the property inline or points at a real numbered invariant added to specs.md; one convention is picked (all step files listed, or pending-only) and the file satisfies it under the finding's comm check; plan.md no longer claims the SPRT harness has not run since 44877c4; the S030-S032 price line agrees with S031's own file
touches:    adocs/plan.md, adocs/specs.md if the property gets an invariant number
excludes:   renumbering step ids; changing the plan order
decisions:
closes:     2026-08-13_plan_review-F05
blocks:
paused_by:
done:      The rule reads: pending files and list entries correspond, the checker adds and prunes entries, plan_done/ and git keep what is pruned; no INV-3 citation remains. Both comm checks empty at completion. Harness prose reads past tense with S035/S036 done and S037 the last pending instrument; the S030-S032 price line carries S031's under-1 % figure. Gate green.

## What is there

`plan.md:60-61`: "Every step file must appear as a list entry, and every list
entry must have a step file — both are INV-3." INV-3 (`specs.md:35-37`) is
the movegen partition invariant; no plan-structure invariant exists. The rule
is also false of the file: 18 done step files have no list entry while 5 do,
so neither convention holds and a reader cannot tell intent from drift.

Two stale prose points in the same file: `plan.md:39-41` still says the SPRT
harness "has not run since `44877c4`" although S035 and S036 are in
`plan_done/` and the harness ran a 30-game live match per S035's stamp; and
`plan.md:35` prices "S030 to S032" at "1-3 % each" while S031's own file says
"**Under 1 %**".

Full evidence: 2026-08-13_plan_review-F05.
author:    Maksym Bodnar
