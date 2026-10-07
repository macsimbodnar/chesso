id:         P12 (proposed; the S-id is allocated at adoption)
goal:       where the applied correction is large, the static evaluation is unreliable and the node's moves are reduced less
accepts:    (1) one term in the LMR adjustment: reduction lowered by a function of |correction| above a threshold, in the fixed-point ticks S236 introduced; (2) a direct guard test and a killed mutant (DEC-141), mate suites green, Tier 2; (3) one SPRT `{0, 5}`; (4) DEC-202 reading at the block 1 boundary
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   correction magnitude in reverse futility, futility or null move (a later step if this is H1)
closes:
paused_by:
author:
done:

## Condition

Opens only if P05 or P06 lands H1. Without a correction there is no
magnitude.

## Description it is implemented from

CPW *Static Evaluation Correction History*: "correction values can be used to
tweak many search heuristics, such as LMR, RFP, and many more".

## Seeds (DEC-134)

Threshold: the p75 of |correction| at LMR sites over the bench positions
(form 2). Slope: range midpoint (form 3).
