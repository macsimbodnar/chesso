id:         S097
goal:       extend the one move a verification search says is singular, and take the multicut the same search offers
accepts:    an SPRT verdict per change, measured separately -- the extension and the multicut are two changes off one verification search; the verification search excludes the table move, runs at a reduced depth against a window below the table score, and is skipped at the root and where the entry is too shallow or its bound is wrong, each condition asserted by a test that fails if the precondition is absent; the margins and the reduced depth are constants in src/search_params.hpp with stated ranges (S073); a position with a forced mate inside the multicut's pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   check extensions, which are S096; any extension not derived from the verification search
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The most expensive item in the block

It searches the node twice at some nodes and it is the one technique here whose
cost shows up as a nodes-per-second loss before any Elo appears. It also
depends on the table entry carrying a usable depth and bound, so it sits after
S094. It is kept in the plan because it is the one search feature that is on
every engine in the 3000-plus hand-crafted band and absent here.

Reported figures put it at 30 to 60 Elo. DEC-019: that decides that it is
tried, and the SPRT decides what is kept.
