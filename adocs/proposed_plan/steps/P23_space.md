id:         P23 (proposed; the S-id is allocated at adoption)
goal:       a space term for the middlegame: safe central squares behind or at the own pawn chain, weighted by the own piece count
accepts:    (1) feature: squares on the four central files and the own second to fourth ranks, not attacked by enemy pawns, counting squares behind own pawns extra; scaled by own non-pawn piece count; middlegame only; (2) fitted from zero, others frozen; (3) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   mobility (P18)
closes:
paused_by:
author:
done:

## Why

The S028 profile: the early middlegame carries 38.8 % of the centipawns lost
after the fit, the largest share of any phase (DEC-032, DEC-035).

## Description it is implemented from

A standard term of the hand-crafted era; no description was read while
writing this. Pin one at step start (CPW's evaluation pages) or record the
design as the project's own.
