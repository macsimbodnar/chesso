id:         S044
goal:       plan.md and specs.md state the DEC-041 boundary: only S029's network training is owner-run
accepts:    plan.md names S029's network training as the one step the owner runs, citing DEC-015 as amended by DEC-041, with S028's mention reading as history; specs.md's non-goal carries the same boundary with a dated inline note; no line in either file still assigns fits or measurements to the owner
touches:    adocs/plan.md, adocs/specs.md
excludes:   any change to decisions.md; any change to the delegation itself
decisions:  DEC-041, DEC-015
closes:     2026-08-13_plan_review-F01
blocks:
paused_by:
done:      plan.md:11-14 and specs.md:165-170 both state the boundary as DEC-015 amended by DEC-041: the owner runs only S029's network training, fits and measurements are the agent's, S028 reads as history. grep for DEC-015/owner finds no line assigning fits to the owner. Dated inline note in specs.md. Gate green.

## What is there

`plan.md:11-13`: "S028 and S029 are the two steps where the agent stops short
of the run itself: it delivers the tuner, the data and the training program,
and the owner executes them (DEC-015)." `specs.md:165-167` says the same
("**The agent does not run training or table tuning.** [...] The owner
executes the run. DEC-015").

DEC-041 (2026-08-11) superseded DEC-015 for evaluation tuning and every kind
of measurement; only S029's network training stays with the owner. S028 is in
`plan_done/` and its fit was agent-run (DEC-034, then DEC-041). Specs outrank
plan, so the stale copy is also the authoritative one: a cold session reading
either file re-inherits the handoff cost DEC-041 was taken to remove.

Full evidence: 2026-08-13_plan_review-F01.
author:    Maksym Bodnar
