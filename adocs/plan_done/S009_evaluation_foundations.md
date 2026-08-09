id:         S009
goal:       king out of material, side-to-move-relative evaluate(), game_phase()
accepts:    identical search_bench node counts and best moves before and after, since the change is behaviour-neutral by construction; game_phase covers full board, bare kings, pawn endgame, per-piece weight and the promotion overflow case
touches:    src/evaluation.cpp, src/search.cpp call sites, tests/test_evaluation.cpp, tests/test_search.cpp
excludes:   incremental versus recomputed accumulation, deferred to S014
decisions:  DEC-003
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 97e1273. README and MANUAL checked at adoption, not when this shipped.

## What shipped

- The king is out of `piece_values[]`. It could only ever cancel in a legal
  position, and pricing it at 100000 meant an illegal one scored above every
  mate. `piece_values_abs[]`, which move ordering uses, keeps its king price --
  the king really is the least desirable capturer.
- `evaluate()` returns from the side to move's point of view. The two call
  sites no longer apply a sign. See DEC-003.
- `game_phase()` returns 24 down to 0, clamped so promotions cannot push it past
  the maximum, pawns and kings contributing nothing.

## Verification

Node counts identical after the change: 43691503 / 36729994 / 27160039 on the
three `search_bench.py` positions, same best moves.

## Four test contracts changed with the code

Each rewritten to assert the new behaviour, never relaxed:

- colour symmetry now expects the mirrored score to *agree* rather than negate,
  because `mirror_fen()` swaps the side to move along with the colours;
- "score is from White's point of view" became "from the side to move's", and
  gained the mirror case so a symmetric sign error cannot pass;
- the missing-king anchor asserts the king is worth nothing and that the score
  stays inside the mate band;
- two search tests that compensated for the old convention by hand.
