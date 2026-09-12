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
against it as an ordering term at this strength. **The Lynx half of that
evidence is withdrawn** (S181, 2026-09-11, DEC-176): this file read "Lynx
failed four SPRTs on it at ~2600 (-35.8 to -11.1)", no source was ever located
for those four runs, and the API record says the opposite of the headline --
**Lynx merged capture history**, pull request #634, 2024-02-02, into v1.3.0 at
a banded **2653** (`adocs/data/S181_lynx_bands.md`). The closed-unmerged capture-history pull
requests in that repository are later refinements of a feature already in the
tree, dated 2024-09 onward at 2925 and above, not four attempts at the
feature. What is left, and it is enough: Weiss measured it **-4.17 +/- 4.83** at
short control against **+3.66 +/- 3.29** at long -- Weiss pull request #428,
"Capture History", 2021-06-04, 9088 games at 10+0.1 and 15936 at 60+0.6, the
author's own summary being "hurts STC, but LTC shows decent gain"
(https://github.com/TerjeKir/weiss/pull/428; the -4.2 and +3.7 quoted here
before 2026-09-11 were that pair rounded). Its documented value arrives as an
*input* -- reducing tacticals with bad capture history measured **+7.22 +/-
4.72** at 10.0+0.1s over 7696 games and **+2.37 +/- 1.90** at 60.0+0.6s over
31984 games at Ethereal, **sourced 2026-09-13** (after S186's fast check):
commit dcb8560cb3c8b6bb37b6fbaa5cc57c681db7809b, 2020-09-26, "Use Capture
History to apply LMR to some Tactical Moves", whose message says the reduction
goes "from R=1 to R=2, for Tactical moves which have a negative Capture
History score"
(https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+%22capture+history%22;
commit message only, DEC-016). The "+7.2/+2.4" this file quoted before was
that pair rounded. Note the direction: the *input* use **shrinks** at long
control, where the ordering use above **grows** (Weiss, -4.17 short against
+3.66 long), so the two halves of this step are not priced by one number --
and capture futility
and SEE margins read it, which is work for the 3000-plus phase this reserve
exists for. The order note below stands for whenever it runs.

## Hazard

The move-ordering bands clear each other by 100 points. A king capturing a pawn
scores `1000000 + 100 - 100000 = 900100`, against 900000 for a killer. Any new
term added to a capture score can invert that silently, and the symptom is a
strength regression rather than a wrong node count.
