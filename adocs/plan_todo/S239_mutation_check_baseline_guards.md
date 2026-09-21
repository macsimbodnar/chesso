id:         S239
goal:       tools/mutation_check.py refuses a baseline that ran no tests and refuses a fixture whose tests/ or adocs/ differ from the tree it claims to measure
accepts:    a run whose ctest label selects zero tests stops at the baseline with a failure marker and a message naming the label, never a score (a fixture with `--label` set to a ctest label that matches nothing reproduces today's "baseline green, ? tests" and must fail); `require_clean_src` (or its successor) covers `tests/` and `adocs/` as well as `src/` in the fixture worktree, and the run's log header names the sha the fixture is actually at plus "dirty" with the paths when it is not clean; each refusal has a case in `tests/test_mutation_check.py`'s fast suite that fails when the guard is removed; `DEV_MANUAL.md`'s mutation section states both refusals; no change to any mutant file, to the kill logic, or to `src/`
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   any mutant, any engine code, the mutation score's arithmetic; a re-run of past mutation results (they stand as recorded, with S097's two-run history as the example of what the gap cost)
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S097 verdict 2's agent (2026-09-21) found both gaps by walking into them.
Its first mutation launch passed `--label S097_v2`, a ctest label no test
carries; the tool printed `baseline green, ? tests`, ran every mutant against
an empty suite in 154 s and scored all twenty as survivors -- a score that
would have read as twenty gaps in coverage had the agent not noticed the
wall time. Its second run measured a fixture whose `src/` was clean but whose
`tests/test_search.cpp` and `adocs/specs.md` had been edited in place, so the
log header named a commit whose tests did not contain the mined mate row the
run's E21 kill depended on; `require_clean_src` guards `src/` alone and said
nothing. The kills were valid for the landing tree -- the cold fast check
diffed the fixture to prove it -- but the evidence had to be reconstructed
by hand, and a tool that accepts a zero-test baseline will one day print a
clean 100 % over nothing.

Test-side under DEC-171: neither gap can move a reported score, move or
line, so it is filler behind the next strength step and is named by id in
the pre-registration of every run taken while it is open. Created
2026-09-22 by the coordinator from the agent's Report 4 and the fast check's
finding 2.

## Cost

A few dozen lines of Python and two cases; no machine beyond the fast suite.
