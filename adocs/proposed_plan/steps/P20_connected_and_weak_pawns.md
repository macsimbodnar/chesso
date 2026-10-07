id:         P20 (proposed; the S-id is allocated at adoption)
goal:       pawn structure gains connected pawns (phalanx and defended, by rank) and weak unopposed pawns, refitted beside the existing isolated, doubled and backward terms
accepts:    (1) features: phalanx pawns by rank, defended pawns by rank, isolated or backward pawns on a half-open file (weak unopposed); the three existing features refitted with them as one pawn-structure group, every other constant frozen; (2) computed in the pawn cache (P15); (3) INV-5 symmetry green; (4) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   passed-pawn features (P19); pawn storms (P17)
closes:
paused_by:
author:
done:

## Description it is implemented from

CPW *Pawn Structure* (connected pawns, phalanx, isolated, backward, doubled).
Stash changelog v31: connected pawns, phalanx and defender (+25.38 STC,
+18.57 LTC). Leorik 2.1 release notes: passed, connected and protected,
isolated pawns with a pawn hash (+50, bundled).
