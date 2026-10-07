id:         P05 (proposed; the S-id is allocated at adoption)
goal:       the static evaluation the search decides on is corrected by a learned, per-pawn-structure estimate of how far search results land from it
accepts:    (1) a table indexed by side to move and P01's pawn key, updated at the end of a main-search node only when: not in check, not a singular verification (excluded move 0), the best move is absent or quiet, and the bound does not contradict the direction (a fail-high only raises, a fail-low only lowers, an exact score either way); the update is weighted by depth and clamped; (2) the corrected evaluation -- raw plus scaled entry, clamped inside the non-mate band -- replaces the raw one wherever the search reads a static evaluation: `improving`, reverse futility, razoring, futility, late move pruning, null move's conditions, ProbCut, quiescence stand pat; the TT stores the raw evaluation and the correction is applied on read; (3) never applied to a TT score, a mate score or a draw score; (4) mate suites green; a guard test asserts the corrected value stays inside the non-mate band and that the table is untouched in check and inside a verification search; a mutant dropping the bound-direction condition is killed (tools/mutation_check.py, DEC-141); (5) Tier 2 and gate_extra.sh; (6) the table is declared per-thread at its declaration (DEC-175); (7) one SPRT `{0, 5}` decides
touches:    src/search.cpp, src/search.hpp, src/data_structures.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   other correction keys (P06); using the correction's magnitude (P12); quiescence updating the table
closes:
paused_by:
author:
done:

## Why

DEC-033/035 measured the evaluation optimistic and biased by phase; the
correction learns such bias per structure during the search, at no cost to
`evaluate()`. It is the largest search-side addition of the last three years
in the engines surveyed (`report.md` §2.2), and CPW notes that it gains more
at longer controls -- the rating list's direction.

## Description it is implemented from

CPW *Static Evaluation Correction History*: definition, index features,
update conditions, the moving-average and gravity update forms. Stockfish
PR #4950 prose: pawn-structure index, capture fail-highs excluded, depth
multiplier, capped entry. Avalanche PR #75 prose: raw evaluation kept in the
TT, correction applied on read, updates at quiet-best-move or fail-low nodes
out of check and not in a singular verification. No source opened (DEC-221).

## Seeds (DEC-134)

- Table size: the smallest power of two that holds the distinct pawn keys of a
  ten-second search on each bench position with under 5 % collisions
  (derivation, form 2).
- Weight and clamp: from a census of `search score − raw static eval` at the
  update sites over the bench positions -- clamp at its p99, scale so a
  median entry moves the evaluation by the census median (form 2); range
  midpoint where the census is degenerate (form 3).
- Moving-average or gravity form: chosen in the pre-registration, not after.

## Measurement

One SPRT `{0, 5}`, pre-registration with worst-case games and abort rule
(DEC-143).

## From the record

`pruning_eval` already layers the TT bound on top of `static_eval` (S234); the
correction goes under it, so S234's estimate tightens a corrected value. The
`improving` flag reads `static_evals[ply]` (S108), which becomes corrected.
