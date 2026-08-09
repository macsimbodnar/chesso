id:         S023
goal:       history indexed by piece, target and victim, to order captures MVV-LVA rates equal
accepts:    an SPRT returns a verdict; the ordering bands stay disjoint, which the move-ordering hazard in CLAUDE.md makes easy to break silently
touches:    src/evaluation.cpp score_move, src/data_structures.hpp search_state_t
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Hazard

The move-ordering bands clear each other by 100 points. A king capturing a pawn
scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer. Any new
term added to a capture score can invert that silently, and the symptom is a
strength regression rather than a wrong node count.
