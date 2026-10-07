id:         P15 (proposed; the S-id is allocated at adoption)
goal:       evaluate() computes each side's attack sets once and caches pawn-only work by pawn key, so the terms of P17 to P24 cost a lookup and not a recomputation
accepts:    (1) per side, computed once per evaluation: attacks by piece type, all attacks, squares attacked twice, pawn attacks and pawn attack spans; mobility and king safety read them; (2) `bench` identical to the parent (INV-6): the same score at every node; (3) `bench_eval` ns per call and nps measured interleaved against the parent and reported; a regression above the noise floor is explained or fixed; (4) a pawn cache keyed by P01's pawn key holding pawn-only terms and derived sets (passed pawns, spans, open and half-open files); in Debug every hit is asserted equal to a recompute; hit rate measured on the bench positions; if it recovers nothing at this step (DEC-046), it ships switched off behind a constant and is re-measured by P19 and P20, where the pawn stage grows; (5) P02's trace still reconstructs evaluate() exactly; (6) the cache is declared per-thread (DEC-175)
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp, tests/test_evaluation.cpp, tests/bench_eval.cpp, adocs/specs.md
excludes:   any new term; any weight change
closes:
paused_by:
author:
done:

## Why

DEC-036/037: mobility recomputed per call cost a third of the nps and
measured −14.93. The terms of block 2 all read attack sets; computed once and
shared, each costs a few bit operations.

## Description it is implemented from

CPW *Pawn Hash Table*; CPW *Attack and Defend Maps*; Grant 2020 §1 (the
structure of a modern hand-crafted evaluation).

## Measurement

Neutral; no match. Timing per CLAUDE.md rules 4 and 5 (noise floor first,
under 3 % is noise unless hyperfine says otherwise).
