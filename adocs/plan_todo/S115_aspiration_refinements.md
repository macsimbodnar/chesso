id:         S115
goal:       the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
accepts:    an SPRT verdict, recorded whatever it is; the changes are measured together only if a sweep shows them inert apart, and otherwise separately -- DEC-082 is a condition to be met, not a convenience; the node-count sweep is run over the 300 stratified positions of adocs/data/S021_aspiration_sweep.tsv and not over three, because S021 recorded that one sample chooses the wrong setting; the mate-appears-mid-search case S074 put in the gate still passes
touches:    src/chesso.cpp iterative_deepening_search, src/search_params.hpp
excludes:   a window width seeded from the score's own volatility -- dropped by DEC-087, no evidence at this band; the initial delta, which S127 fits
decisions:  DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## What is there

`ASPIRATION_DELTA` is 50 and widening doubles the failing side alone. What the
band's engines do that this does not: keep the search fail-soft through the
window plumbing (fail-soft in the pruning returns measured +2.6/+5.1 at
Ethereal), halve beta toward alpha on a fail-low (one line), and reduce the
root depth on a repeated fail-high so an unstable root does not burn a whole
iteration (Lynx carries it). The volatility-seeded width the strong engines
run has no measured gain below ~3100 and left this step at DEC-087; S085 and
S127 own the delta itself.
