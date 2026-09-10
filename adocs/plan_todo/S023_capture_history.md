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
at ~2600 (-35.8 to -11.1, **unverified** -- no source was located for those
four runs, and the band is S181's) and Weiss measured it **-4.17 +/- 4.83** at
short control against **+3.66 +/- 3.29** at long -- Weiss pull request #428,
"Capture History", 2021-06-04, 9088 games at 10+0.1 and 15936 at 60+0.6, the
author's own summary being "hurts STC, but LTC shows decent gain"
(https://github.com/TerjeKir/weiss/pull/428; the -4.2 and +3.7 quoted here
before 2026-09-11 were that pair rounded). Its documented value arrives as an
*input* -- reducing tacticals with bad capture history measured +7.2/+2.4 at
Ethereal, **unverified**: the 2026-09-04 literature check found no source for
that pair, and S186 owns resolving it -- and capture futility
and SEE margins read it, which is work for the 3000-plus phase this reserve
exists for. The order note below stands for whenever it runs.

## Hazard

The move-ordering bands clear each other by 100 points. A king capturing a pawn
scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer. Any new
term added to a capture score can invert that silently, and the symptom is a
strength regression rather than a wrong node count.
