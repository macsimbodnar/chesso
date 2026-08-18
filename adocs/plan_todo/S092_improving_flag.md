id:         S092
goal:       pruning and reduction margins know whether the static score is rising over the ply stack
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a static evaluation kept per ply in the search stack and a flag derived from comparing this ply's score with the same side's score two plies back, defined for the case where that ply was in check and the score is absent; the flag feeds the reverse futility margin and the late move reduction, and any further consumer is named; the flag never changes what is searched at a PV node in a way the mate test does not cover, with the "pruning does not hide a forced mate" case re-run; identical behaviour is not claimed -- this alters play by construction; the fast suite green
touches:    src/search.cpp negamax and the search stack, src/search_params.hpp for any margin the flag switches between, tests/test_search.cpp
excludes:   correction history, which is S099; storing the static evaluation in the transposition entry, which is S094
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Ordering note

Cheaper and cleaner if S094 lands first: the static evaluation the flag compares
is exactly what a table entry would already carry, and recomputing it per ply
puts back part of the 25 % of nodes-per-second S014 removed. If S094 has not
landed when this starts, the accumulator is read rather than `evaluate()`
re-run, and the step says which.
