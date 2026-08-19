id:         S099
goal:       a static evaluation correction learned from the difference between the static score and what the search returned, keyed on the pawn structure
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); a table keyed on the pawn structure, updated with a running average of the difference between the search score and the static score at nodes where that difference is meaningful, and applied as a bounded correction to the static score; the correction is bounded so it can never turn a non-mate score into a mate score or cross a mate bound, asserted by a test; INV-5 still holds -- the corrected score is side-to-move relative and mirroring a position agrees rather than negates; the correction is cleared on ucinewgame; every constant in src/search_params.hpp with a stated range (S073); the fast suite green
touches:    src/search.cpp, a new table beside the transposition table, src/search_params.hpp, tests/test_search.cpp, tests/test_evaluation.cpp for the mirror property
excludes:   any change to evaluate() itself; NNUE, which is parked at DEC-054; continuation-indexed and material-indexed correction tables, which are a later step if the pawn-keyed one measures positive
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## What it is and is not

It is not a second evaluation. It is a per-key running correction on the static
score, learned inside one search from the only stronger estimator available --
what the search itself returned for that position. It is the last item in the
DEC-071 search block because it needs the static evaluation to be cheap to
obtain (S094) and stable (the block before it).

## Hazard

Anything that adjusts a score near a mate bound can manufacture or destroy a
mate score. The bound is asserted by a test, and the "pruning does not hide a
forced mate" case is re-run.


## Scoped to the pawn table, 2026-08-19

The family is three steps -- this one, S110 non-pawn, S111 continuation --
because the surveyed record measures them separately and they are **not** inert
apart, so DEC-082 does not apply. This one is first because it is the one every
engine added first and the one with the largest reported figure.

Two constraints that are this project's rather than the literature's. The
pawn-structure key must be maintained in `add_piece`, `remove_piece` and
`move_piece` and never rebuilt in `evaluate()` -- that is INV-4, and rebuilding
it is how the 25 % of nodes per second S014 removed comes back. And the
corrected score must never reach the mate or decisive band; one engine carries a
commit named for exactly that bug.

It reads the static evaluation S108 supplies and everything downstream of it --
reverse futility, null move, the S109 block, razoring -- shifts when it lands,
so the margins are re-fitted after it rather than before.
