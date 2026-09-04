id:         S196
goal:       the fault-injection driver becomes `tools/mutation_check.py` over a tracked mutant list, so the suite's kill rate is measured by running it and a new search rule ships with a mutant its test kills
accepts:    `tools/mutation_check.py` takes a mutant file and a worktree path, applies each mutant, builds, runs the fast label and the bench, reverts, and prints the kill table with the failing assertion per mutant; the 33 mutants of `adocs/data/2026-09-04_test_review/mutants.py` move to `tools/mutants/` in the two `(void)` forms that compile under `-Werror`; the tool refuses an ambiguous anchor and reports an equivalent mutant (bench unchanged, suite green) apart from a survivor; a full pass at the current tree reproduces 31 of 32, and 32 of 32 once S193 has landed; `DEV_MANUAL.md` "Test" documents the tool, the per-rule mutant rule of DEC-141 and the cost of a full pass; the evidence directory keeps its copy unchanged (`adocs/data/README.md`: added, never edited)
touches:    tools/mutation_check.py, tools/mutants/, DEV_MANUAL.md
excludes:   mull, dextool or any dependency (DEPS rule; considered in `adocs/testing_strategy.md` section 5); changing any mutant's meaning
decisions:  DEC-139, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why this exists

The 2026-09-04 test review measured the suite's fault detection for the first
time -- 31 of 32 non-equivalent hand-written bugs caught, about 50 minutes of
machine -- and the literature it drew on names the mutation score as the
accepted measure of a suite's effectiveness, correlated with real-fault
detection independently of coverage (`adocs/testing_strategy.md` section 3.1).
S145 applied the same discipline to one test ("observed red under a stated
mutation"); DEC-141 makes it the rule for every new search rule, and this
step is the tool the rule needs.

## Cost

An hour to make the driver a tool and move the list; a full pass is about 50
minutes of machine and is not part of any gate.
