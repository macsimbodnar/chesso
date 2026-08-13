id:         S037
goal:       info nodes reports the whole search's node count so search_bench.py compares the whole tree
accepts:    the nodes field of successive info lines is non-decreasing within one search and the final value equals the sum of the per-iteration counts; tools/search_bench.py prints that value; the node figures quoted in DEV_MANUAL.md are restated as what they now mean
touches:    src/chesso.cpp iterative_deepening_search, tools/search_bench.py, DEV_MANUAL.md
excludes:   any change to the search itself -- this is a reporting change and must leave the tree identical
decisions:
closes:     2026-08-13_adversarial-F03
blocks:
paused_by:
done:

## What is broken

`iterative_deepening_search` zeroes the counter at the top of every iteration
(`src/chesso.cpp:571`), accumulates the total into `result.total_node_explored`
(`src/chesso.cpp:593`), and then prints `search_result.explored_nodes`
(`src/chesso.cpp:637`) — the per-iteration figure. It is not monotonic:

```
info score cp 80 time 4 depth 6 nodes 49034 pv c3d5 ...
info score cp 80 time 2 depth 7 nodes 45397 pv c3d5 ...
sum of per-iteration nodes: 122617
```

## Why this is a measurement problem, not a cosmetic one

`tools/search_bench.py:41-43` keeps the last `info` line's value, and
`DEV_MANUAL.md` names that tool as how INV-6 is discharged for a change claimed
behaviour-neutral. The quantity being compared is the final iteration alone, so
a change that alters the tree at depths 1..n-1 and leaves the last iteration
identical passes a check that is supposed to prove it changed nothing.

The knps figure is wrong in the same breath: last-iteration nodes divided by
whole-search wall time, understated 2.7-fold in the example above.

The recorded numbers inherit it. `DEV_MANUAL.md`'s "47438623 nodes either way
over the three search_bench positions" is a sum of last iterations.

Separately, `nodes` is universally read as the count for the current search, so
a GUI watching chesso sees the count fall between depths.

## Shape

Report the running total in `info nodes`, and compute `nps` from elapsed search
time rather than iteration time. That fixes the protocol and the tool in one
change. If the per-iteration figure is wanted in the log, print both and have
`search_bench.py` read the cumulative one.

Whichever is chosen, the documents that quote these numbers move in the same
commit — the figures do not become wrong, they become differently defined, and
saying so is the point.

## Ordering note

This lands before any further neutrality claim is made, because until it does,
"identical node counts" is a weaker statement than the invariant it is standing
in for.
