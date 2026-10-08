id:         S263
goal:       evaluate() computes each side's attack sets once per call and every term that needs them reads the shared sets, so the new mobility, threat, king-safety and outpost terms cost a lookup rather than a recomputation
accepts:    (1) per side, computed once per evaluation where the expensive stage already computes piece attacks: attacks by piece type, all attacks, squares attacked twice, pawn attacks and pawn attack spans; mobility and king safety read them instead of computing their own; (2) **score identity**: `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths (INV-6) -- the same score at every node; (3) `tests/bench_eval.cpp` ns per call and nps measured interleaved against the parent with the noise floor read first (CLAUDE.md rules 4 and 5), a regression above it explained or fixed; (4) only sets with a consumer in the tree or in the next step that lands are built -- a set nobody reads is a per-node cost with no return; (5) S262's trace still reconstructs `evaluate()` exactly; (6) any table this adds states per-thread or shared at its declaration (DEC-175)
touches:    src/evaluation.cpp, src/evaluation.hpp, tests/test_evaluation.cpp, tests/bench_eval.cpp
excludes:   any new term or weight; the pawn-structure cache, which is S118 and lands after the pawn terms it caches (DEC-087); pin detection, which S121 adds with its consumer
decisions:  DEC-258, DEC-036, DEC-037, DEC-175
closes:
blocks:
paused_by:
author:
done:

## Why

DEC-036/037: mobility recomputed per call once cost a third of the nps and
measured -14.93. S121, S101, S122 and S102 all read attack sets; computed once
and shared, each costs a few bit operations. Score identity makes this a
timing and not a match (DEC-083).

## Description it is implemented from

CPW *Attack and Defend Maps*; Grant 2020, the structure of a modern
hand-crafted evaluation. The 2026-10-07 proposal's P15, without its pawn
cache, which stays S118's in DEC-087's order.

## Lane

Agent work under DEC-260; compiled and timed between runs.
