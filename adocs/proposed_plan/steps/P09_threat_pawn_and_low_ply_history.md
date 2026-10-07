id:         P09 (proposed; the S-id is allocated at adoption)
goal:       three additions to quiet-move history, one verdict each: v1 the butterfly history indexed also by whether the from and to squares are attacked, v2 a pawn-structure history, v3 a low-ply history
accepts:    (1) v1: quiet_history gains two index bits, from-square attacked by the opponent and to-square attacked by the opponent; the attacked set is every square an enemy piece attacks, computed once per node where quiets are generated; a second leg with "attacked by a lesser piece or undefended" only if pre-registered; (2) v2: a table indexed by P01's pawn key (reduced to a power-of-two range), moved piece and destination, updated by the quiet history's rule and added to the ordering sum; (3) v3: a table for plies 0 to 3 indexed by from and to, bonus on cutoffs at those plies weighted by depth, added to ordering at those plies; (4) history pruning's score (`src/search.cpp:2543`) reads what each verdict changes, and S109's guard test stays green; (5) per verdict: nps cost measured and stated in the pre-registration, Tier 2, one SPRT `{0, 5}`, each against the previous verdict's tree; (6) tables declared per-thread (DEC-175)
touches:    src/search.cpp, src/search.hpp, src/evaluation.hpp, src/data_structures.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, MANUAL.md, adocs/specs.md
excludes:   two-ply continuation history (S231 H0, DEC-224); history-scaled LMR (S098 v1, DEC-213); persistence across `go` (S093 v2, DEC-101)
closes:
paused_by:
author:
done:

## Description it is implemented from

- v1: no published description was found while writing this proposal. If none
  is pinned at step start, the step records the design as the project's own
  (CLAUDE.md allows techniques "invented on the spot").
- v2: CPW *Static Evaluation Correction History* uses the same pawn index for
  correction; a pawn-indexed *move-ordering* history is named in Stockfish's
  history work but no prose description was read. Pin one at step start or
  record the design as own.
- v3: Stockfish PR #2557 prose: a table for plies 0 to 3, structured like the
  main history with the ply in place of the colour, because histories work
  well near the leaves and poorly near the root.

## Seeds (DEC-134)

Bounds and bonus: the quiet history's own (form 2). Weight in the ordering
sum: midpoint (form 3), left to P14.

## From the record

S093 (malus and gravity, +10.7), S222 (one-ply continuation on its own scale,
+11.1), DEC-218 (two weighted history terms share one band-clearance ceiling
-- any new weight joins that ceiling).
