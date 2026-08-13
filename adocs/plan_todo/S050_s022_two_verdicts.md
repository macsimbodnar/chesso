id:         S050
goal:       S022 accepts one verdict per change instead of one run measuring two
accepts:    S022's accepts requires two SPRT verdicts — delta pruning, and the S015 SEE-pruning re-measure — one per change, each against the commit before it, in either order; no acceptance line asks one run to measure two changes; the see() cost figure it quotes carries its machine condition
touches:    adocs/plan_todo/S022_delta_pruning_quiescence.md
excludes:   implementing delta pruning; running either measurement now
decisions:
closes:     2026-08-13_plan_review-F07
blocks:
paused_by:
done:

## What is there

`S022_delta_pruning_quiescence.md` accepts: "an SPRT returns a verdict; **the
same run** re-measures the S015 quiescence SEE pruning." The body of the same
file: "but measure them **one at a time**, or neither number means anything"
— which is also the house rule. Executed either way the step deviates from
its own text: two runs where the accepts says one, or one run producing two
meaningless numbers at a cost S027 prices in hours. Secondary: the "see()
cost 12.1 % more than it does now" premise predates DEC-049 and carries no
machine condition.

Full evidence: 2026-08-13_plan_review-F07.
