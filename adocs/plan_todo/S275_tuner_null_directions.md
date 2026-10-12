id:         S275
goal:       every exact null direction of the 823-parameter evaluation is pinned in the tuner, or recorded as harmless with a measurement, before the next fit
accepts:    (1) `tools/feature_audit`'s identity report (S134: rank 784 of 823, nullity 39 -- 32 always-zero pawn columns on ranks 1 and 8, the five documented material-against-table directions, and two new king directions, one per king table, since one king a side makes each king table's signed occupancy sum to zero) is re-run on HEAD and the 39 are listed by name; (2) for each class, the tuner either pins it (a gauge: a fixed coordinate, a projection after each step, or the columns dropped from the fitted vector) or the step records, by a measured fit, that Adam does not drift along it, with the drift read off the fitted values; (3) a test that fails if a fit moves along a pinned direction, observed red with the pin removed; (4) a fit taken before and after on the same corpus slice agrees in loss to the last printed digit, so the pin changes nothing the evaluation can see
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tools/tuner_groups.hpp, tools/feature_audit.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   any change to src/ or to a shipped weight; regularisation (DEC-170 ruled against it); the five material/table directions' convention if one is already enforced -- then it is cited, not re-done
decisions:  DEC-170, DEC-090, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Why

S134's accepts required that no unreported exact degeneracy remain after its
folds, "or a new finding is filed before a fit". Its identity report
(`adocs/data/S134_feature_audit.txt`, exact rank over integer X'X) found
two it had no record of: with one king a side, the signed occupancy of
`psqt_mg[KING]` sums to zero on every row, and so does `psqt_eg[KING]`. A
constant added to a king table moves no evaluation, so its level is
unidentified. Adam rescales each coordinate, so its update need not stay
orthogonal to a null direction, and a fit can drift along one: harmless to
the score, but it moves 64 shipped numbers for nothing and makes two fits'
tables incomparable. This is that filing.

## Lane

Agent work, tuner tooling only; a short fit on a corpus slice, minutes.
Filler under DEC-171 (no defect reaches play), ordered before the next fit
(S262's trace tuner).
