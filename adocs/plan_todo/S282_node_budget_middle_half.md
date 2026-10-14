id:         S282
goal:       `adocs/data/S192_node_budget.py`'s "middle half" verdict can read inside on the band it derives itself
accepts:    (1) the script's middle-half bounds are stated in terms of the count the band is derived from (the band is `[count / FLOOR_DIVISOR, BUDGET_RATIO x count]`), and a count equal to the derivation count reads inside, shown by running the script on a build whose shipping band was derived from that build's own count; (2) the rule DEV_MANUAL.md's goldens row and `tests/test_search.cpp`'s comment at "ordering keeps the tree small" state for when the band is re-derived is the rule the script prints, word for word in substance, and the row's history sentence that relied on the old reading ("inside the pair's middle half") is checked against the new bounds and corrected if it no longer holds; (3) no golden number moves in this step: the band 65024 / 3251 stays until a step that moves the count re-derives it by the script
touches:    adocs/data/S192_node_budget.py, DEV_MANUAL.md (the goldens row), tests/test_search.cpp (comment only)
excludes:   re-deriving the band itself; any change to the case's assertion or to search
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found by S023's phase 2 (2026-10-14, Opus subagent), checked by the
coordinator by reading the script. The band is `[c / 5, 4c]` for a derivation
count `c` (`FLOOR_DIVISOR` 5, `BUDGET_RATIO` 4). The script's middle half is
`low = floor + (budget - floor) // 4`, which is `0.2c + 0.95c = 1.15c`, and
`high = 4c - 0.95c = 3.05c`. The count the band was derived from, `c`, is
below `low`, so a freshly derived band always prints "OUTSIDE" for its own
count, and the reading DEC-142's re-derivation rule leans on says nothing.

On the shipping band 65024 / 3251 (from 16256): the middle half is
[18694, 49581]; 16256 itself, the count the band was taken from, is outside.

Tool and documentation only: no play, no reported score, no UCI surface, so
filler under DEC-171, scheduled behind the next strength step and named in
the pre-registration of any run taken while it is open.
