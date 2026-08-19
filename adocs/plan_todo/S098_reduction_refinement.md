id:         S098
goal:       the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
accepts:    an SPRT verdict per adjustment, measured separately -- history scaling, node type and the re-search rule are three changes and one at a time is the rule; every constant introduced goes into src/search_params.hpp with a stated range (S073), including the reduction table's own shape if it becomes a formula; the "pruning does not hide a forced mate" case re-run after each adjustment, since S013 shipped an LMR that reduced the mating move at the root; a mate found at the root is never reduced, asserted with the precondition that would otherwise reduce it; the fast suite green
touches:    src/search.cpp late move reduction, src/search_params.hpp, tests/test_search.cpp
excludes:   late move pruning, which is S090; the improving flag itself, which is S092 and is an input here
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why it comes after the history steps

The refinement's largest single input is the move's history score, and the
tables it reads are queued ahead of it: S093 malus and gravity, then S024
continuation history. Scaling a reduction by a table that is about to change
means measuring it twice. The improving flag is S108's, ahead of it too.
Capture history is **not** an input here -- this engine reduces only quiets --
and S023 sits in the reserve tail (DEC-087); if it ever lands, reducing
tacticals with bad capture history is its consumer, back in this file's scope
at that time (Ethereal measured that consumer at +7.2/+2.4).
