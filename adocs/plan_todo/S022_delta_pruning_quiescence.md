id:         S022
goal:       skip a quiescence capture that cannot reach alpha even if it wins outright
accepts:    an SPRT returns a verdict; the same run re-measures the S015 quiescence SEE pruning, which measured 0 Elo when see() cost 12 % more than it does now
touches:    src/search.cpp quiescence
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Outstanding question this step answers

S015's quiescence pruning measured 0 Elo as a trade against a `see()` that was
subsequently made 12.1 % cheaper. Nobody has rerun it. Do that here rather than
as a separate step, since both changes are in the same function and neither is
worth a machine-hour on its own -- but measure them one at a time, or neither
number means anything.
