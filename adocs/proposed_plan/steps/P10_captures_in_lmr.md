id:         P10 (proposed; the S-id is allocated at adoption)
goal:       captures and promotions late in the move list are reduced on their own, smaller base, adjusted by capture history; the SEE-losing extra ply stays on top
accepts:    (1) a separate reduction table for captures and promotions with its own base and divisor; adjusted by P07's capture history; (2) the existing exclusions hold: no reduction for a checking capture (`capture_gives_check`), in check, at the root, below depth 3, for the first moves; (3) a direct guard test of the new rule and a mutant it kills (DEC-141), mate suites green, Tier 2; (4) one SPRT `{0, 5}`; (5) its class read once more at 32+0.32 at the block 1 boundary (DEC-202)
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   changing the quiet reduction; capture history in SEE pruning
closes:
paused_by:
author:
done:

## Why

Only quiet moves are reduced (`src/search.cpp:2732`); a late capture that
SEE calls safe is searched at full depth however unlikely it is to matter.

## Description it is implemented from

CPW *Late Move Reductions*: several engines reduce captures and promotions on
a separate, smaller formula and adjust by history. Stash changelog v31: one
ply of reduction allowed for captures and promotions (+4.9).

## Seeds (DEC-134)

Base and divisor: from a census over the bench positions of the move number
at which capture cutoffs occur (the tune build's `census_cutoff`, landed with
S109, counts quiet cutoffs; extend it to captures), the derivation written before the census runs (form 2), or range
midpoints (form 3). History divisor: the capture history bound (form 2).

## Depends on

P07 v1 H1. If P07 v1 is H0, this step runs without the history term.
