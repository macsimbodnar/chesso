id:         S094
goal:       quiescence probes and stores the transposition table, and an entry carries the static evaluation it was scored with
accepts:    an SPRT verdict per change, measured separately -- the probe and the stored static evaluation are two changes; a quiescence entry is stored at a depth that cannot satisfy a main-search probe, with a test that a main-search node at depth 1 does not cut on a quiescence entry; the mate-score adjustment on store and probe still holds "mate in N from here" and tests/test_search.cpp's existing table mate-score case passes unmodified; the static evaluation in the entry is used in place of a recomputation and INV-4 still holds, since the accumulators are what it is derived from; the fast suite green
touches:    src/transposition_table.cpp and .hpp for the entry layout, src/search.cpp quiescence, tests/test_search.cpp
excludes:   correction history, which is S099; the improving flag, which is S092 and is a consumer of the stored value
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why it comes early in the block

Two later steps are consumers. S092's flag compares a static score across plies
and S099 learns a correction from the difference between the static score and
what the search returned; both want the value already in the entry rather than
recomputed. `evaluate()` runs at every quiescence node and S014 removed 25 % of
nodes per second by making the accumulators incremental (INV-4) -- a step that
recomputes puts that back.

## Hazard

The entry layout is shared with the main search. Widening it changes how many
entries fit a bucket and therefore the replacement behaviour, which alters play
on its own. The step measures the layout change and the use of the new field
separately, or states why it could not.
