id:         S250
goal:       `test_mate_breadth`'s exact-count floor separates the shipping guard from the weakened one again, placed by its own script on today's tree
accepts:    `adocs/data/S156_mined_floor_sweep.py` re-run on the tree of the day prints the per-`RfpMinPly` table and the gate built at the weakened default goes **red**; `EXACT_FLOOR` is re-placed strictly above the weakened guard's count and at or below the shipping count, by the rule S156 placed it with (`adocs/plan_done/S156_mined_set_floor.md`), the rule and the reading quoted at the site with the command; the site's table and margin prose read that run; the stale figures in the comment around `MATE_IN_THREE_FLOOR` in `tests/test_engine.cpp` ("10 at RfpMinPly 1", "21 at RfpMinPly 4", mates in four 1/16) re-derived by `adocs/data/S154_floor_margin_sweep.py` and corrected, that floor itself unmoved unless its own sweep says it no longer separates; both fast suites green; if no floor can be placed (the gap closes to zero), stop and report -- that is a decision, not a placement
touches:    tests/test_mate_breadth.cpp (the floor and its comment), tests/test_engine.cpp (the comment beside `MATE_IN_THREE_FLOOR`), DEV_MANUAL.md if its golden rows quote either
excludes:   the mined set, its labels, the depth, any engine change; lowering any floor
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-01, the coordinator)

S249's item 1 re-read the mined set's count before editing its comment and
found more than a stale number. `adocs/data/S156_mined_floor_sweep.py` on
`0cf3ec8` reads 146 exact with the guard shipping (RfpMinPly 3 and 2) and
143 with it weakened (RfpMinPly 1 and 0); `EXACT_FLOOR` is 143, so the gate
built at the weakened default goes **green** and the script prints "THE FLOOR
NO LONGER SEPARATES" (`.tuning/coord/S249/S156_mined.log`, gitignored, quoted
in S249's stamp). The last tracked reading,
`adocs/data/S156_mined_floor_sweep.log`, was 145 against 141 with the gate
red. Nothing is red today: the shipping tree passes its floor; what is lost is
the test's power to catch the weakening it exists for. S249's excludes forbid
moving a floor, so the item moves here; raising a floor to restore its
separation tightens a test and is DEC-142's re-derivation, not a relaxation.
S249's item 2 sweep (`S154_floor_margin_sweep.py floor`) found
`MATE_IN_THREE_FLOOR`'s own comment true at 12 of 24 and its floor (11)
separating (7 of 24 weakened), but three figures in the surrounding prose
stale; those join here.

Ordered ahead of S116 by the coordinator: a test-only defect (DEC-171 scope),
but S116 is a pruning change and this is a mate guard over pruning, so the
guard gets its teeth back before the next pruning lands.
