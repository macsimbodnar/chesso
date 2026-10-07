id:         P11 (proposed; the S-id is allocated at adoption)
goal:       the singular extension gains two arms, one verdict each: v1 a double extension when the TT move is singular by a wide margin, v2 a negative extension when it is not singular but the table says the node fails high
accepts:    (1) v1: at a non-PV node, a verification score below `singular_beta` by more than a margin extends the TT move by two plies; a per-path count of double extensions in the search state caps them; (2) v2: when the verification fails to prove singularity and the TT score is at least beta, or the node is a cut node, the TT move is searched one ply shallower; the multicut return (S097 v2) is unchanged; (3) a constructed position where the cap is the only thing bounding the tree terminates, asserted; (4) per rule: a direct guard test and a killed mutant (DEC-141), mate suites green, Tier 2, gate_extra.sh; (5) one SPRT `{0, 5}` each, v2 against v1's tree; (6) DEC-202 reading at the block 1 boundary
touches:    src/search.cpp, src/search.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tests/test_uci_surface.cpp, tools/mutants/, MANUAL.md, adocs/specs.md
excludes:   triple extensions; extensions at PV nodes beyond +1; the singular verification's depth and margin (P14 tunes them)
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *Singular Extensions* describes the base technique and its relaxed modern
form but not these two arms. **A description must be pinned at step start**
(release notes, PR prose or a write-up describing double and negative
extensions); if none is found the step does not start (DEC-221).

## Seeds (DEC-134)

Margin for v1 and the cap: midpoints of stated ranges (form 3), left to P14.

## From the record

S097 v1 measured the +1 extension at −0.81 and DEC-227 kept it as the
multicut's carrier; S097 v2's multicut measured +7.17. DEC-226: a
verification search is answered only by moves. S188 (check extension) H0,
DEC-230.
