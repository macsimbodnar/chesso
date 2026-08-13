id:         S037
goal:       info nodes reports the whole search's node count so search_bench.py compares the whole tree
accepts:    the nodes field of successive info lines is non-decreasing within one search and the final value equals the sum of the per-iteration counts; tools/search_bench.py prints that value; the node figures quoted in DEV_MANUAL.md are restated as what they now mean
touches:    src/chesso.cpp iterative_deepening_search, tools/search_bench.py, DEV_MANUAL.md
excludes:   any change to the search itself -- this is a reporting change and must leave the tree identical
decisions:
closes:     2026-08-13_adversarial-F03
blocks:
paused_by:
done:      info nodes and time are the whole search's and nps is reported: search_bench reads 609848 / 2058510 / 468039 at depth 9 where the last info line used to read 254082 / 1022573 / 168767.
            INV-6 discharged, tree identical: the new cumulative totals equal the pre-change sums of per-iteration counts to the digit, every single iteration matches as a successive difference, best moves c3d5 / e2a6 / d7c8q either way. No SPRT owed.
            Red first: test_engine 'info nodes is cumulative over the whole search' observed failing before the fix at REQUIRE( 8891 >= 12980 ), the per-iteration sequence being 149, 1568, 4482, 12980, 8891, 49034 against a whole-search total of 77104. Preconditions asserted first and passed, so the red was the property and not a vacuous test.
            The accepts named DEV_MANUAL.md for the 47438623 figure; it lives in src/evaluation.cpp:144 and was restated there instead. DEV_MANUAL.md, MANUAL.md, specs.md INV-6 and search_bench.py all now state what the count means and that a pre-S037 figure is a sum of last iterations. DEV_MANUAL's stale 'about ten seconds per binary' replaced by the measured 3136397 nodes in about 0.44 s.
            Gate green: build clean under -Wall -Wextra -Werror, ctest -L fast 9/9, clang-format --check clean.

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
author:    Maksym Bodnar
