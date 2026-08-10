id:         S033
goal:       prune a node whose static score is already far enough above beta
accepts:    an SPRT against the preceding commit returns a verdict; a position with a forced mate inside the pruned depth is in the fast suite and passes before the feature is called done
touches:    src/search.cpp negamax
excludes:   forward futility and razoring, which are S026; late move pruning, which has no step
decisions:  DEC-033
closes:
blocks:
paused_by:
done:

## Why this exists as its own step

The plan had no entry for it. S026 is "futility and razoring" and its goal line
describes forward futility, which asks whether a move can reach alpha. Reverse
futility asks the opposite question at the node itself -- is the static score so
far above beta that the opponent cannot claw it back in the remaining depth --
and prunes the whole node. Different test, different place in `negamax`,
different failure mode. DEC-033.

Of the search techniques chesso does not have, this is the one with the largest
figure in the one public per-feature log found: Blunder measured +57.1 +/- 16.9
in self-play. That is a reported figure and therefore decides what to try and
never what to conclude (DEC-019). DEC-033 puts the ceiling on the whole search
block at about 10 cp per effective doubling on the errors that matter.

## Shape

At a non-PV node, not in check, with depth below some small bound, if
`evaluate() - margin * depth >= beta`, return without searching. It is the null
move observation applied without making the null move, which is why it shares
null move pruning's guards.

## Hazard, twice observed

Both NMP and LMR shipped with a bug that hid a mate, and both were caught by a
mate test rather than by a benchmark. This is a third pruning rule with the same
shape, and the same requirement applies: a mate inside the pruned depth, in the
fast suite, red before it is green.

The specific trap here is the mate score. A static evaluation is never a mate
score, so a node holding a forced mate for the opponent can still have a static
score above beta and be pruned. Guard `beta` against the mate band exactly as
the null move guard does.

## Depends on nothing, and that is the point

The static score is already computed at every quiescence node and is one
`evaluate()` call at an interior node. No new tables, no accumulator work, no
move ordering change. It is the cheapest thing in the search block to try.
