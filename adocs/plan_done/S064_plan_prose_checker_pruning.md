id:         S064
goal:       plan.md and testing.md describe the checker's retention as the last five completed entries in list order
accepts:    plan.md's list rule says the checker keeps the last five completed entries in list order and that a ledger row leaves when every step id it names was pruned in that pass; no sentence in plan.md still says the oldest completed entry is pruned or implies retention is by completion time; testing.md's header carries the same correction, since it states the stronger and equally false version
touches:    adocs/plan.md, adocs/testing.md
excludes:   changing the checker; changing what is pruned; adding or removing ledger rows
decisions:
closes:     2026-08-13_plan_review.2-F09
blocks:
paused_by:
done:      plan.md's list rule and testing.md's header both state the retention the checker implements: the last five completed entries in list order, bin/moltke.py:1698-1700 with PLAN_DONE_KEPT = 5 at :1681.
                Demonstrated on the live window: S067, S065, S033, S062, S053 are listed, and S053 completed at 18:42 on 2026-08-13 while S037 at 19:13, S043 at 19:29 and S066 the next day completed later and are pruned. S053 survives on position, not recency.
                Second correction, same sentence, beyond what the finding recorded: the header claimed naming an invariant or a decision protects a row. The predicate is ids <= dropped_ids over S<nnn> alone, and git log shows 17 pruned rows naming an INV-n or a DEC-n, including S037's INV-6 row. Rows naming no step id are what actually stay.
                Scope held: no checker change, no change to what is pruned, no ledger row added or removed beyond this step's own.
                Gate: 12 of 12 fast, format clean, tools/plan_prose_check.py exit 0.

## What is there

`plan.md`'s list rule: the workflow checker "prunes the oldest completed entry —
taking its testing.md rows with it — as newer completions land". It prunes by
position in the file, not by when the step completed
(`bin/moltke.py:1698-1700`, `PLAN_DONE_KEPT = 5` at `:1681`):

```
    entry_lines = [i for i, line in enumerate(lines)
                   if (m := PLAN_ENTRY_RE.match(line)) and m.group(1) in done_ids]
    drop = set(entry_lines[:-PLAN_DONE_KEPT]) if len(entry_lines) > PLAN_DONE_KEPT else set()
```

The current file shows the difference. The five completed entries still listed
are S040 (completed 19:55), S041 (21:05), S054 (21:31), S038 (22:38) and S053
(18:42) — while S037 (19:13) and S043 (19:29), both completed after S053, are
gone. S053 survives because its entry is the last line of the list, appended at
creation, not because it is recent. The ledger follows the same rule:
`grep -c S043 adocs/testing.md` is 0, `grep -c S053 adocs/testing.md` is 1, and
`git log -S"S043" -- adocs/testing.md` shows the row leaving in `b904d1b`, the
S038 completion.

Informational, and it costs a reader the wrong model of what the list contains:
retention is a window over list positions, so a step appended late outlives
newer completions and keeps its ledger rows while theirs are pruned.
`adocs/testing.md`'s header states the stronger and plainly false version — "it
keeps the newest five completed entries listed" — which the audit put outside
its scope and which is the same sentence; it is corrected here rather than left
to be found again.

Full evidence: 2026-08-13_plan_review.2-F09.
author:    Maksym Bodnar
