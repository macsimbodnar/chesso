id:         S023
goal:       history indexed by piece, target and victim, to order captures MVV-LVA rates equal
accepts:    an SPRT returns a verdict; the ordering bands stay disjoint, discharged by a case ctest runs and not by reading the diff -- a king capture of a pawn scored against a killer at the same ply, asserting the capture wins -- written in this step's own commit; the bound on the capture-history term that keeps a capture score inside its band is stated as a constant with a range in src/search_params.hpp
            (Folded in from the retired S061 by DEC-086. Order note from the
            2026-08-19 review: capture history's measured value is in the
            capture futility and capture SEE margins, not in the ordering key --
            wire it into the margins first, and let it into score_move second.)
touches:    src/evaluation.cpp score_move, src/data_structures.hpp search_state_t
excludes:
decisions:
closes:     2026-08-13_plan_review.2-F06
blocks:
paused_by:
done:

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push by the second review. The band evidence is
against it as an ordering term at this strength: Lynx failed four SPRTs on it
at ~2600 (-35.8 to -11.1) and Weiss measured it -4.2 at short control, +3.7
only at long. Its documented value arrives as an *input* -- reducing tacticals
with bad capture history measured +7.2/+2.4 at Ethereal, and capture futility
and SEE margins read it -- which is work for the 3000-plus phase this reserve
exists for. The order note below stands for whenever it runs.

## Hazard

The move-ordering bands clear each other by 100 points. A king capturing a pawn
scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer. Any new
term added to a capture score can invert that silently, and the symptom is a
strength regression rather than a wrong node count.
