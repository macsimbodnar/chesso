id:         S259
goal:       the S097 multicut row and the S113 ProbCut row in "pruning does not hide a forced mate" separate their guard mutants E21 and B04 again on today's tree
accepts:    a targeted `tools/mutation_check.py --only E21` and `--only B04` on HEAD each read killed, and the kill is observed at the row named for that guard (or, if another case already kills the mutant, that is recorded and the row is still re-mined or the decision not to is recorded); every moved row is re-derived by the mining script its site names (DEC-142), with the mutant observed red first and the shipped build green; the second tier's mutant clause (DEC-141) holds for both guards
touches:    tests/test_search.cpp (the two rows and their golden comments), adocs/data/ for the re-mine's log and targeted tsv, tools/mutants/ only if an anchor is stale
excludes:   any change to src/; any change to the multicut or ProbCut rules themselves
decisions:  DEC-141, DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-06, the coordinator, from S120's finding 9)

S120's implementer re-checked the mined mate rows on the parent `1eaa776`:
the S097 row (`mate_the_multicut_hides`, depth 14) and the S113 row
(`mate_probcut_hides`, depth 11) no longer separate their mutants. E21
(`E21_multicut_mate_band_gate_dropped`) finds the S097 row's mate at d11,
d13 and d14 like the shipped build, and B04
(`B04_probcut_defender_gate_dropped`) reads the S113 row exactly as shipped
(d9, d11 to d14). Both cases stay green, so nothing is red today, but neither
row is a guard on this tree any more. The raw runs are machine-local under
`.tuning/coord/S120_files/rows/`, and `rowcheck.sh` there re-runs them.

S188 met the same thing on this row once and wrote the lesson down: a mined
row is a property of the tree it was mined on. The test's comment block above
`mate_the_multicut_hides` records every re-mine so far.

A test-strength defect that reaches no play: filler under DEC-171, behind the
next strength step, and named as an open finding in every run's
pre-registration until it closes.

## First move

Run the targeted mutation check for both mutants on HEAD before mining
anything. If the whole fast suite already kills them, the guards are still
guarded and the question is only whether the named rows are re-mined. If
either survives, a guard has no test: re-mine its row first.
