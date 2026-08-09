id:         S010
goal:       tapered piece-square tables and an insufficient-material draw rule
accepts:    an SPRT against the preceding commit passes; the bare-king endgame test is true for the right reason rather than by accident of a flat evaluation
touches:    src/eval_tables.hpp, src/evaluation.cpp, src/search.cpp, tests/test_evaluation.cpp
excludes:   tuning the table values, which is S028
decisions:  DEC-002
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in cd3c905. README and MANUAL checked at adoption, not when this shipped.

## Result

SPRT passed 110-1-1. Two tables per piece, middlegame and endgame, interpolated
on `game_phase()`. **Hand-written from ordinary positional principles, not taken
from a published set** -- see DEC-002.

Cost at the time: about 25 % of nodes per second, 13.0 down to 10.1 Mnps,
because the tables were recomputed from the bitboards on every call. Left that
way deliberately so the SPRT measured the evaluation and not the evaluation
plus an optimisation. S014 removed the cost.

## Insufficient material, bundled in

Without it the search prefers one drawn position to another -- a centralised
king scores better than a cornered one -- and reports a bare king endgame as a
small advantage. Deliberately strict: king against king, and a single minor
against a bare king. Knight against knight and same-colour bishops are drawn in
practice but not by the laws, and claiming them throws away positions still
winnable on the clock. Applied at every node except the root, which still has
to return a move.

## Three test positions were over-specified, all passing by accident

- "the doubled rooks win the queen" had the black king on e8, where the queen is
  pinned to it and *every* white move wins it. King moved to d8.
- "take the free pawn" had the kings close enough that the black king walks back
  and wins the pawn again, so the position is drawn whatever White plays. Kings
  moved to opposite corners.
- "a bare king endgame is a draw" asserted exactly zero, which only held because
  a material-only evaluation is flat.
