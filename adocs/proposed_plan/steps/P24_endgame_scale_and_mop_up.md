id:         P24 (proposed; the S-id is allocated at adoption)
goal:       the endgame score is scaled by a factor for drawish material, and lone-king endings carry a term that drives the defending king to where it can be mated
accepts:    (1) a scale factor on the endgame score: opposite-coloured bishops alone and with other pieces; one side without pawns and at most a minor piece ahead; rook endings with every pawn on one flank and pawn counts within one; scaled further by the stronger side's pawn count; fitted through P02's scale-factor group; (2) mop-up for KX vs K and for king, bishop and knight vs king: the defending king toward the edge, or toward the corner of the bishop's colour, the attacking king closer; (3) tests labelled by tools, never by reasoning (CHESS rule): drawn positions (Syzygy WDL draw, python-chess) score within a stated band of zero; won lone-king positions (Syzygy WDL win) are converted to mate by the engine in self-play at a fixed node budget within a stated move bound; (4) one SPRT `{0, 5}`
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp, tests/assets/, adocs/specs.md
excludes:   complexity (P25); probing tablebases in play (O1)
closes:
paused_by:
author:
done:

## Description it is implemented from

Grant 2020 §1.3 (scale factor on the endgame score for opposite-coloured
bishops and unwinnable material). CPW *Mop-up Evaluation*. Stash changelog
v26, v30, v31 (endgame scaling and specialised endgames, +3 to +9 each).

## From the record

The engine already scores KvK, KNvK and KBvK as dead draws, in quiescence too
since S210. Tablebase files for the tests live outside the repository; the
test skips with a stated reason when they are absent, and the step says where
they come from.
