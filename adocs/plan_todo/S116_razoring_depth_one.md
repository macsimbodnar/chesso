id:         S116
goal:       a node whose static score is hopelessly below alpha drops straight to quiescence, at depth one only
accepts:    an SPRT verdict, recorded whatever it is; the verified form -- the quiescence score is returned only if it is itself at or below alpha; never in check, never at a PV node, never when alpha is near mate; the margin and the depth bound are constants in src/search_params.hpp with ranges; a mate inside the razored depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   the move-loop pruning rules, which are S109
decisions:  DEC-019, DEC-084
closes:
blocks:
paused_by:
done:

## This is S026's other half, and it is the smallest thing on the plan

S026 was "drop nodes near the horizon that cannot reach alpha" and covered both
futility and razoring. Its futility half is S109; this is the rest, split out
because razoring is a node-level rule rather than a move-loop one and does not
belong inside the block.

Expect very little. Stockfish's removal test reports **~0**; one engine at
about 2600 measured +7.9 over 4550 games; another added it, tuned it, and then
deleted it. The one consistent finding is that **restricting it to depth 1
gained** where the unrestricted form did not, which is why the goal says depth
one rather than leaving the bound open.
