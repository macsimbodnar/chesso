id:         S115
goal:       the window's half-width comes from the score's own volatility, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
accepts:    an SPRT verdict, recorded whatever it is; the three changes are measured together only if a sweep shows them inert apart, and otherwise separately -- DEC-082 is a condition to be met, not a convenience; the node-count sweep is run over the 300 stratified positions of adocs/data/S021_aspiration_sweep.tsv and not over three, because S021 recorded that one sample chooses the wrong setting; the mate-appears-mid-search case S074 put in the gate still passes
touches:    src/chesso.cpp iterative_deepening_search, src/search_params.hpp
excludes:   the initial delta, which S127 fits
decisions:  DEC-084
closes:
blocks:
paused_by:
done:

## What is there

`ASPIRATION_DELTA` is 50 and widening doubles the failing side alone. Two
things the strong engines do that this does not: seed the half-width from the
score's own variance rather than from a constant, and reduce the root depth on
a repeated fail-high so an unstable root does not burn a whole iteration. The
third, halving beta toward alpha on a fail-low, is one line.
