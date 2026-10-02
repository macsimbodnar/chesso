id:         S251
goal:       `RfpMinPly`'s declared range holds a mate-safe value at every setting now that razoring reads it too, or razoring takes a floor of its own
accepts:    one of two forms, chosen with the reason and decided before the change: (a) `RfpMinPly`'s minimum moves 2 -> 3, the S145 mate-in-two case re-run at 3 green and the range's comment and `MANUAL.md` row saying why; or (b) razoring reads a separate `RazorMinPly` (seed 3, range with a mate-safe minimum shown by the same case) and `RfpMinPly` keeps 2 for reverse futility alone; either way the tune build at the range's minimum finds all 26 S145 mates in two at iteration 3, observed, and both fast suites are green; the shipped engine is node-identical (INV-6) unless the step says otherwise and measures it
touches:    src/search_params.hpp, src/search.cpp (only under form b), MANUAL.md, tests/test_search_params.cpp (golden_defaults)
excludes:   the shipped value 3; any change to the rule's verdict, which is S116's SPRT
decisions:  DEC-095, DEC-171, DEC-215, DEC-248
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-02, the coordinator)

S116's build (finding 1 in its "As built" section) found that razoring, which
reads `RfpMinPly` as its ply floor by DEC-248, reaches the ply-2 node holding a
mate in two's quiet mating move at `RfpMinPly` 2: the tune build there finds 17
of the 26 S145 mates in two at iteration 3. The range's minimum 2 was measured
for reverse futility alone (S145, DEC-095). Unreachable in ordinary play (the
Release build compiles the shipped 3; only the tune build sets it), so a filler
behind S116 (DEC-171), named in `adocs/data/S116_sprt.sh`'s open findings as
item 20.
