id:         P27 (proposed; the S-id is allocated at adoption)
goal:       piece-square tables indexed also by the own king's region, still hand-crafted, linear and fitted
accepts:    (1) bucket scheme -- the own king's half (king side or queen side), or four regions -- chosen by a pre-registered comparison of validation loss on P26's corpus; parameter count stated; (2) opens only if P26's corpus gives every new column non-zero on at least the share of rows S100 used as its coverage floor, and on the owner's yes (O4); (3) the PSQT accumulator stays exact (INV-4): a king move that changes the bucket rebuilds that side's PSQT sum through the add/remove/move alphabet, its cost measured; (4) one SPRT `{0, 5}`
touches:    src/eval_tables.hpp, src/evaluation.cpp, src/bitboard.cpp, src/data_structures.hpp, tools/tuner_groups.hpp, tests/test_invariants.cpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   any network; any nonlinear feature transform
closes:
paused_by:
author:
done:

## Condition

Conditional, after P26 and only with the owner's yes: it adds about 768
parameters per bucket and moves toward network-style features while staying
hand-crafted.

## Description it is implemented from

Leorik 2.5 release notes: piece-square values computed from parameters that
reflect the king positions and the game phase (shipped with PEXT generation
and SMP, about +100 together, so not attributable).

## From the record

CLAUDE.md: `make_move` is the NNUE hook, and the accumulator discipline it
describes is the one this step extends.
