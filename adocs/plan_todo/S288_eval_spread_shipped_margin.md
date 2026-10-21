id:         S288
goal:       tools/eval_spread reports the margin that ships, read from src/search_params.hpp rather than typed
accepts:    (1) eval_spread's candidate margins include LAZY_EVAL_MARGIN's value as `src/search_params.hpp` defines it, read from the parameter list and not typed as a literal, first in the list as its comment says the current one is; (2) a fast test fails when the shipped margin is absent from the tool's candidates (observed red first by removing it); (3) its comment no longer names 150 as current; (4) no `src/` change, bench the parent's, `No functional change`; the fast suite green in both builds, format clean, the five prose modes clean
touches:    tools/eval_spread.cpp, tests/
excludes:   any change to the margin, the shortcut or the clamp; re-measuring the spread
decisions:  DEC-171
closes:     2026-09-10_adversarial-F25
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-10_adversarial-F25`: `tools/eval_spread.cpp`'s `MARGINS` is
`{150, 200, 250, 300, 400}`, and the margin has shipped at 184 since S085.
S039 named the finding and would have retired the tool's question with the
clamp; S039 read H0 (record `ca578de`) and kept both, so the defect is live
again. S287's restated comment in `src/evaluation.hpp` says eval_spread does
not report 184 and points at the tuner's load report instead. A tool, not
play: filler under DEC-171, created 2026-10-21 by the coordinator from
S287's fast check.
