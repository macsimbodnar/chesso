id:         P07 (proposed; the S-id is allocated at adoption)
goal:       v1 a capture history orders captures; v2 captures that lose material are searched after the quiet moves
accepts:    (1) v1: a table indexed by moved piece, destination and captured piece type, updated with the gravity rule -- bonus to a capture that fails high, malus to the captures tried before it at that node; whether a quiet cutoff also penalises the captures tried before it is fixed in the pre-registration; capture order is victim value plus history over a divisor, in the main search, quiescence and ProbCut's list; (2) v2: a capture whose SEE is below a threshold is ordered after every quiet move; (3) the ORDER_* bands keep their clearance, asserted at compile time or by a test -- CLAUDE.md's known hazard (a king taking a pawn scores 900100 against a killer's 900000) must not invert; (4) per verdict: Tier 2, mate suites green, one SPRT `{0, 5}`; v2 against v1's tree; (5) the table is declared per-thread (DEC-175)
touches:    src/evaluation.cpp, src/evaluation.hpp, src/search.cpp, src/search.hpp, src/data_structures.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, MANUAL.md, adocs/specs.md
excludes:   reducing captures by LMR (P10); capture history in SEE pruning thresholds (a later step if v1 is H1)
closes:
paused_by:
author:
done:

## Why

Every capture, a losing one included, is ordered above the killers today
(`src/evaluation.cpp:1222` before `:1227`), so a refutation that is quiet
waits behind every bad capture, and LMP and LMR count those captures as early
moves.

## Description it is implemented from

CPW *History Heuristic*: capture history indexed by moved piece, target
square and captured type (Geschwentner, 2016), replacing MVV-LVA. Stash
changelog: capture history (+4.6), staged generation with bad captures at
the end (+17.9, bundled).

## Seeds (DEC-134)

History bound and bonus polynomial: the quiet history's own (QUIET_HISTORY_MAX
and the S093 bonus), a derivation on chesso's scale (form 2). v2's threshold:
SEE below 0, the exchange's own boundary; a pawn's value below 0 is a
pre-registered second leg, not a fallback chosen after the fact.

## From the record

DEC-007 and DEC-022 deferred bad-capture ordering "until capture history
exists"; v1 creates it. CLAUDE.md records capture ordering "reported around
150 Elo and measured slower" -- taken without capture history; v2's
pre-registration names it.
