id:         P06 (proposed; the S-id is allocated at adoption)
goal:       two more correction tables, each its own verdict: v1 one table per side keyed by that side's non-pawn key, v2 a continuation table keyed by the previous two plies' moves
accepts:    (1) v1: two tables, indexed by side to move and by White's and Black's P01 non-pawn keys respectively, both entries added to P05's; same update rule and conditions as P05; (2) v2: a table indexed by the moved piece and destination of the move one ply back and of the move two plies back, same rule; a null move as either previous move skips both read and update (DEC-219: the node two plies after a pass carries no key); (3) each correction's weight in the sum is a parameter; (4) per verdict: P05's guard test extended, a killed mutant, Tier 2, one SPRT `{0, 5}`; v2 measured against v1's tree, or P05's if v1 is H0 and reverted; (5) tables declared per-thread (DEC-175)
touches:    src/search.cpp, src/search.hpp, src/data_structures.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   minor, major, material and threat keys (a new step if v1 and v2 are both H1); the magnitude consumer (P12)
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *Static Evaluation Correction History*: non-pawn "all non-pawn pieces by
colour, typically paired, one per side"; continuation correction "previous
two plies' moves". Stockfish PRs #5816 and #5841 (topic only: both sides share
an index layout; kings' role in the minor key). Their numbers are not used.

## Seeds (DEC-134)

Weights in the sum: equal, the range midpoint (form 3), left to P14's SPSA.
Table sizes: P05's derivation repeated on each key.

## Measurement

Two SPRTs `{0, 5}`.

## From the record

S231 measured the two-ply continuation *history* at H0 (DEC-224); this is a
correction table on the same index, a different technique, and the
pre-registration names S231 so the reading is not confused with it.
