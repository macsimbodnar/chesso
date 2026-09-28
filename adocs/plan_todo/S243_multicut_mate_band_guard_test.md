id:         S243
goal:       a direct guard test for the multicut's mate band -- the verification search's returned value is never handed back as the node's when it is in the mate range -- with a mutant only it kills, so E21's coverage no longer depends on a mined row
accepts:    a test drives a node where the multicut's verification search returns a mate-range value and asserts the node does not return it (the guard's own statement), observed red with the gate dropped (E21) and green shipped; `tools/mutation_check.py` over S097 verdict 2's list on a fresh fixture kills E21 by that test; the mined multicut row stays as it is (a second witness, re-derived by its script when the tree moves); the fast suite green in both builds
touches:    tests/test_search.cpp, tools/mutants/ (S097's list)
excludes:   the multicut's rule and its constants; any other guard's test; the mining script
decisions:  DEC-141, DEC-142, DEC-238
closes:
blocks:
paused_by:
done:

## Why this exists

S097 verdict 2's mate-band guard was covered by a mined mate row alone. The
row moved at S113's rebase and stopped separating E21 at S131's -- the third
time in three rebases that a mined mate row moved with the tree -- and
DEC-238 re-derived it under a guard mode of the mining script. A direct test
is what DEC-141 asks of a rule's guard, and it ends the corpus dependence for
this one: it drives the node, plants a mate-range return in the verification
search's path (a drive fixture in the shape of S113's `probcut_drive_t`, or a
position whose verification search returns a mate at a small depth), and
asserts the guard's statement directly. Filler under DEC-171: agent-only
work, closed by the block boundary, named in every pre-registration taken
while it is open.
