id:         S022
goal:       skip a quiescence capture that cannot reach alpha even if it wins outright
accepts:    two SPRT verdicts, one per change and each against the commit before it, in either order: delta pruning, and the re-measure of the S015 quiescence SEE pruning, which measured 0 Elo when see() cost 12.1 % more than it does now (an Apple-machine figure, pre-DEC-049)
touches:    src/search.cpp quiescence
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Outstanding question this step answers

S015's quiescence pruning measured 0 Elo as a trade against a `see()` that was
subsequently made 12.1 % cheaper (measured on the Apple machine, pre-DEC-049).
Nobody has rerun it. Do that here rather than as a separate step, since both
changes are in the same function -- but measure them one at a time, one SPRT
each, or neither number means anything. The accepts asks for exactly that:
two verdicts, either order.
