id:         S053
goal:       testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
accepts:    acceptance criteria, testable
touches:    areas of the codebase
excludes:   explicitly out of scope
decisions:
closes:
blocks:
paused_by:
done:      The header states the retention as prune_plan() implements it: newest five completed entries kept in plan.md, a row leaves only when every S-id it mentions was pruned in that pass, plan_done/ and git keep everything. Traced to moltke.py:1684-1727 rather than inferred from the prune messages. Gate green.
author:    Maksym Bodnar
