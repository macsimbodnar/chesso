id:         P25 (proposed; the S-id is allocated at adoption)
goal:       a complexity term pulls the endgame score toward zero in structures where the stronger side cannot convert, and never flips its sign
accepts:    (1) complexity is a linear sum of features -- pawn count, pawns on both flanks, a pawn-ending flag, a constant -- applied to the endgame score as `sign(E) · max(complexity, −|E|)`, per Grant 2020 §1.5; (2) fitted through P02's complexity group with its gradient (§3.4, §3.7), others frozen; (3) a test: the adjusted score never changes sign and is zero only where the unadjusted one is; (4) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   the scale factor (P24)
closes:
paused_by:
author:
done:

## Description it is implemented from

Grant 2020 §1.5 (complexity: remaining pawns, pawns on both flanks, pawn
endgame; dampens toward a draw, never flips the side), §3.4 and §3.7
(gradients). Ethereal 11.75 release notes (complexity evaluation).
