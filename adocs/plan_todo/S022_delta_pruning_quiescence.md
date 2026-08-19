id:         S022
goal:       decide between delta pruning and the per-move futility S112 adds, by measurement -- deleting delta pruning is a valid recorded outcome
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


## Re-targeted 2026-08-19

The step used to be "add delta pruning". It is now "choose", and the reason is
that one engine **gained** by deleting delta pruning once per-move futility
(S112) existed -- the two prune overlapping sets and having both was worse than
having the better one. `specs.md` already folds the S015 re-run into this step;
that re-run belongs here too, for the same reason: SEE pruning in quiescence
measured 0 here at a time when per-move futility was absent, which is exactly
when the surveyed record expects it to measure 0.

So this step now decides three things against one another with S112 in the
tree: per-move futility alone, futility plus SEE, futility plus delta. Deleting
is a valid outcome and gets recorded as one (DEC-019, and S005/S006/S015 are
the precedent for keeping or dropping on a measured zero).
