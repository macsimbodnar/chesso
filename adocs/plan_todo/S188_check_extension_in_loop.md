id:         S188
goal:       a move that gives check is extended by one ply inside the move loop, bounded so a chain of checks cannot run away, and decided by SPRT whatever it returns
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the extension applies inside the move loop to a move that gives check, never at the root and never past a stated depth cap, and the total extension along a line is bounded by the extension budget and depth guard S097 lands, so a sequence of checks cannot exceed the cap -- each condition asserted by a test in `tests/test_search.cpp` that fails when the precondition is removed; the late-move-reduction exemption for a checking move at `src/search.cpp` `is_check_move` stays as it is and the step states how the two interact; the mate cases in the fast suite pass -- `tests/test_search.cpp` "pruning does not hide a forced mate", "a side in check may not stand pat", "mate is recognised at depth zero" -- and the S145 mate sets are re-run with the found counts compared against the pre-change figures; every constant lives in `src/search_params.hpp` with a range and is seeded in a DEC-105 form (the wiki's one-ply form or the range midpoint), never from another engine's value; `MANUAL.md` and `DEV_MANUAL.md` checked; fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp
excludes:   extensions not triggered by a check -- singular, negative, double -- which are S097 and later; a check extension before the move loop, the form Ethereal removed; any change to which moves the reduction exempts; the retired S096's id, which is not reused
decisions:  DEC-087, DEC-133, DEC-105
closes:
blocks:
paused_by:
author:
done:

## Why this exists

DEC-087 (a) retired S096, check extensions, on two records: "Ethereal removed
check extensions for +4.1/+4.5 and Stormphrax removed them too". The
2026-09-04 review's literature pass fetched both. Ethereal commit `3f4ef537`
(2018-06-25) is titled "Simplify and remove the pre-moveloop check extensions"
-- it removed the extension applied before the move loop, at bounds [-3, 1],
+4.14 +/- 4.15 and +4.54 +/- 4.13 -- and Ethereal's master `search.c` still
extends a checking move inside the move loop, which is where the wiki's
Ethereal page lists the technique. Stormphrax's commit #67 (2024-03-04)
removed check extensions outright and carries a bench and no Elo; Stormphrax
is a network engine far above the band. Of the hand-crafted engines the plan
reads as existence proofs, Weiss 1.2 at 3055 and Stash both carry the in-loop
form. Chesso has no extension of any kind; the reduction exempts a checking
move and nothing extends one.

DEC-133 is the owner's decision of 2026-09-04: the retirement stands by id,
and the in-loop form is a new step with one verdict, placed after S097 because
S097 builds the extension plumbing -- the depth guard and the budget that stop
extensions compounding -- and a check extension is the smallest thing that
plumbing carries. DEC-019 is why it is measured rather than argued either
way: the record says every band engine has it; only a verdict says whether
chesso wants it.

## The hazard

Pruning that hides a mate is this engine's recurring bug, and an extension has
the opposite failure: a chain of checks that extends without bound blows the
tree up at the horizon. The cap and the budget are the guard, and the accepts
requires each to be observed load-bearing -- remove it, watch the test go red
-- rather than assumed.

## Cost

A few lines of code and one verdict at the ledger's price, about five hours of
machine at DEC-063's pairs.
