id:         S113
goal:       a shallow verification search over good captures prunes a node whose score is already far above beta
accepts:    an SPRT verdict, recorded whatever it is; never at a PV node, never when beta is near mate, and skipped when the table already holds a sufficient-depth entry scoring below the ProbCut beta; the margin and the depth reduction are constants in src/search_params.hpp with ranges and are fitted, not taken (DEC-084); a mate inside the pruned depth is in the fast suite and observed red with the guard removed
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   multicut, which arrives with S097's singular search at no extra cost
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## Expect little, and be ready to record it

Reported +6.36 / +6.65 on first introduction at about 2950. One engine
**removed** ProbCut at +1.14 / +4.87 and then reintroduced it at -0.15 / +4.34
-- i.e. its value at 3400 is inside the noise at short control and small at
long. It is on this plan because it is cheap once the machinery around it
exists, not because it is expected to be large. A zero here is an ordinary
outcome and gets recorded as one.
