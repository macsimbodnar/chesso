id:         S095
goal:       reduce a node whose table entry carries no move instead of searching it at full depth
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the depth threshold and the reduction amount are constants in src/search_params.hpp with stated ranges (S073); the reduction applies only where the entry genuinely has no move, with a test asserting the precondition -- a node whose entry does have a move must not be reduced, and the test fails if the precondition is absent; a position with a forced mate inside the reduced depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   internal iterative deepening, the older and more expensive form, unless the step measures both and says which it kept
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## What it replaces

The older technique is internal iterative deepening: search the node to a
shallower depth first, purely to get a move to order with. The reduction is the
modern form -- if there is no table move, the node is cheaper than its depth
claims and is searched one or two plies shallower instead. Both are documented;
this step implements the reduction and measures it, and the deepening form is
out of scope unless the measurement says otherwise.
