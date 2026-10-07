id:         P14 (proposed; the S-id is allocated at adoption)
goal:       every live search parameter tuned jointly once on the block 1 tree, then verified
accepts:    (1) axes: every parameter in src/search_params.hpp that a node-count probe reaches (S214's reachability rule; TmHardPercent excluded, DEC-200), ranges as test_search_params pins them; (2) the run sized by DEV_MANUAL's "Sizing a real run" with the iteration count stated before launch; a night (DEC-155), detached, watcher with four exits; (3) a tuned value at its range's end is reported and decided, not shipped silently (S085's RfpMinPly: 0 turned three mate cases red); (4) verification SPRT `{0, 5}` of the tuned vector against the untuned; H0 ships nothing; (5) DEC-202 reading of the vector at 32+0.32
touches:    src/search_params.hpp, tools/spsa_driver.py (only if sizing needs it), tools/spsa_*.json, MANUAL.md, adocs/specs.md, adocs/data/P14_*
excludes:   evaluation weights (the tuner's); the time manager's hard limit (DEC-200)
closes:
paused_by:
author:
done:

## Why

S085 tuned twelve of twenty-two parameters for +21.02. About sixty exist now;
most added since (S091, S095, S097, S098, S109, S113, S115, S116, S132, S222,
S234 and block 1's) were seeded by census or midpoint and never tuned
together.

## Description it is implemented from

CPW *SPSA*; Spall's SPSA papers; `zamar/spsa` (the method's description, not
its code). The project's own `tools/spsa_driver.py` (S084).

## Seeds

The shipped values (the project's own).

## From the record

S084, S085, S151 (S085's vector read at 32+0.32: +11.99 ± 11.57, an
estimate), DEC-094, DEC-200.
