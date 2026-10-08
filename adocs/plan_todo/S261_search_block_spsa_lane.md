id:         S261
goal:       the search block's own SPSA lane: the search parameters added since S085 and never tuned together are fitted jointly on today's tree, then verified by an independent SPRT
accepts:    (1) the axes are listed before launch: every parameter in `src/search_params.hpp` `CHESSO_SEARCH_PARAMS` that was added or reseeded after S085's run, that a node-count probe reaches (S214's reachability rule; `TmHardPercent` stays out, DEC-200), and that no later lane fitted (S222's and S231's lanes are named and their axes excluded or included with the reason); each axis keeps the range `tests/test_search_params.cpp` pins; (2) the run is sized by `DEV_MANUAL.md` "Sizing a real run" with the iteration count and the expected hours stated before launch, in chesso's lane size (eight to nine hours, DEC-222 (8)), so a night under DEC-155; if the listed axes cannot be fitted together in that size, the split into two lanes is stated before the first one starts, never after; (3) detached with a terminal marker and a watcher with the four exits the WATCHERS rule names; (4) an axis that returns at the end of its range is reported and decided, not shipped silently -- S085's `RfpMinPly` at 0 turned three mate cases red; the mate suites (`test_mate_breadth`, `test_mate_pv`, `test_mate_carry`) are green on the tuned vector before any match; (5) the tuned vector is verified by an **independent** `{0, 5}` nElo SPRT at 8+0.08 against the untuned tree, with DEC-143's worst case and abort rule pre-registered; H0 or no verdict ships nothing; (6) at the block boundary this closes: one S199-form drift point and one DEC-202 fixed 1000-pair reading at 32+0.32, Hash=64, both read as estimates beside the verdict, never as verdicts
touches:    src/search_params.hpp, tools/spsa_driver.py only if sizing needs it, tools/ for the run's JSON, MANUAL.md for any default that moves, adocs/specs.md, adocs/data/ for the lane's log and readings
excludes:   evaluation weights, which the tuner fits; S127, the final lane over the whole set after the evaluation block; the time manager's hard limit (DEC-200); the axes S099 and S023 will add, which S127 fits
decisions:  DEC-222, DEC-200, DEC-202, DEC-143, DEC-258
closes:
blocks:
paused_by:
author:
done:

## Why now

DEC-222 (8) made SPSA a cadence -- one lane per completed block over the
axes that block added, S127 the last. The search block is complete and its
lane was never scheduled: it is the omission both the 2026-10-07 proposal and
its comparison found. S085 tuned twelve of twenty-two parameters for +21.02;
since then S091, S095, S097, S098, S109, S113, S114, S115, S116, S132, S222,
S234 and others added axes that sit at census-derived or midpoint seeds, and
several were parked explicitly "for S127 to fit". Every evaluation fit after
this point is fitted against the search these values produce, and the corpus
S082 generates is played by it, so the lane runs before both (DEC-258).

## Description it is implemented from

CPW *SPSA*; Spall's papers on simultaneous perturbation; the project's own
driver, `tools/spsa_driver.py` (S084), and its sizing section in
`DEV_MANUAL.md`. The seeds are the shipped values, which are the project's
own (DEC-134 form b or c, each already recorded at its landing step).

## From the record

S084, S085 (+21.02), S151 (S085's vector at 32+0.32: +11.99 +/- 11.57, an
estimate), S222 and S231 (narrow lanes with their own axes), DEC-200, DEC-202.
