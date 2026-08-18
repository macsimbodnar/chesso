id:         S090
goal:       skip late quiet moves near the horizon by move count, once the ordering has been given its chance
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the move-count threshold grows with depth and is a constant set in src/search_params.hpp with a stated range (S073); the threshold reads the improving flag if S092 has landed, and the step says which of the two it was measured with; a position with a forced mate inside the pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed and the printout recorded; no quiet move is pruned while in check, at a PV node, or when the move gives check, and the test asserts the precondition that would otherwise prune it; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   futility pruning and razoring, which are S026; SEE pruning, which is S091
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Hazard, three times observed

Null move pruning hid a mate in two by reducing to depth 0; late move reduction
reduced the mating move at the root; reverse futility returns a static bound and
therefore cannot see a mate at all, which is why S033 bounded it to depth 6 and
ply 3. All three were caught by a mate test rather than by a benchmark. This is
the fourth pruning technique and it gets the same treatment before it is called
done -- the clause is in the accepts above rather than left for a later step to
add, which is what S060 exists to fix for S026.
