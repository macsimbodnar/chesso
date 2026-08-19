id:         S124
goal:       the endgame half of the score is scaled toward a draw by what is actually on the board
accepts:    an SPRT verdict, recorded whatever it is; the factor scales **only the endgame half** of the tapered score and not the whole score; the cases are added one at a time with a verdict each -- strong-side pawn count first, opposite-coloured bishops second -- and not as one bundle, because the surveyed record shows several of the further cases measuring zero and being removed again; every constant is fitted (DEC-084); the existing insufficient-material draw detection is not duplicated by this and the step says how the two divide
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp
excludes:   tablebases, which are S129; the 50-move decay, which is a separate small term
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## Why this is high on the evaluation block

It is the largest evaluation group in the surveyed record that chesso has
**none** of, it is cheap, and it is low-risk: reported +11.3 for a drawish-endgame
factor at about 2600, +13.72 and +7.56 for scaling by strong-side pawn count,
+9.41 for the specific change from scaling the whole score to scaling only the
endgame half.

The warning is the shape of the tail. One engine at 3300 measured +0.29, +0.94,
+0.18 and +0.11 for four further opposite-bishop special cases and **removed
them as neutral simplifications**. Build the two that pay, measure, and stop --
do not build the matrix.

`specs.md` records that the endgame is the cheapest phase per move in chesso's
own error profile and that DEC-033 found endgame errors the least
depth-fixable. That is an argument for this step, not against it: what is least
fixable by searching deeper is exactly what has to be fixed by knowing more.
