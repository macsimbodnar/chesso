id:         S153
goal:       an agent-only step and a match-owning step may be active at once, so a document step does not leave the machine idle
accepts:    `plan_active_max` permits two active steps per author with the stated rule that at most one of them may hold the machine, or the strict sequencing is kept and `adocs/decisions.md` records why -- either outcome closes this, and it is the owner's call and not the agent's; the cost being traded is stated from the measurement rather than argued: 15.1 h of exactly stamped runs in the 65.4 h since the S088 anchor, five SPRT verdicts at one per 13 h, against 45 to 55 verdicts still owed; INV-1 and `--validate` agree with whatever the marker ends up saying
touches:    .moltke.json, adocs/decisions.md, AGENTS.md
excludes:   raising `plan_stack_max`, which is about blocking depth and not about idleness; running two match-owning steps at once, which contends for the machine and is what the concurrency default already settles; the SOTA enrichment pass, which is parked separately in `status.md`
decisions:  DEC-096
closes:     2026-08-21_adversarial-F05
blocks:
paused_by:
done:

## The measurement

DEC-096 protects machine time from document work, on the grounds that
measurement capacity is the binding constraint. The converse is not recorded: an
agent-only step leaves that same constraint idle, and the two classes contend for
nothing. Positions 15 to 18 of the plan -- S142, S139, S140, S141 -- are four
consecutive steps that owe no match, ahead of the whole search block.
