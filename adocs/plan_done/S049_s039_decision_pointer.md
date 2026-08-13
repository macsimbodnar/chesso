id:         S049
goal:       S039 cites the decision that governs the lazy margin, not the one-night fit delegation
accepts:    S039's decisions field names DEC-039, or is empty; DEC-034 appears nowhere in the file
touches:    adocs/plan_todo/S039_lazy_margin_redecide.md
excludes:   the margin re-decision itself
decisions:  DEC-039
closes:     2026-08-13_plan_review-F06
blocks:
paused_by:
done:      S039's decisions field reads DEC-039, the skip-not-remember decision the margin implements; DEC-034 appears nowhere in the file. Gate green.

## What is there

`S039_lazy_margin_redecide.md` carries `decisions: DEC-034`. DEC-034 is "the
owner delegated the S028 fit to the agent for one run" — workflow delegation,
nothing to do with the lazy-evaluation margin. The governing decision is
DEC-039 ("Skip the expensive evaluation work rather than remember it", tagged
s034); the step's own excludes line names S034, suggesting a DEC-034/S034
transposition. Traceability is what the field exists for: grep from DEC-034
lands on a step it does not constrain, and DEC-039 is cited nowhere.

Full evidence: 2026-08-13_plan_review-F06.
author:    Maksym Bodnar
