id:         S014
goal:       maintain material, psqt and phase in make_move instead of recomputing them
accepts:    identical search_bench node counts and best moves, since this must be behaviour-neutral; the debug build asserts the accumulators against a full recomputation on every make and unmake
touches:    src/eval_tables.hpp eval_add_piece/eval_remove_piece/eval_refresh, src/bitboard.cpp, src/evaluation.cpp, src/data_structures.hpp
excludes:   any new evaluation term
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 2fcb15b. README and MANUAL checked at adoption, not when this shipped.

## Measurement

**-27 % on the search, behaviour-identical.** `evaluate()` is now the
interpolation and the sign and nothing else. It was about 25 % of nodes per
second before this and stopped being 40 % of the search.

`board_t` is 216 B with the four accumulators. The accumulator functions live in
a header so `make_move` can inline them.

## Why it is checkable

`eval_accumulators_match(board)` is asserted at three sites in the debug build.
That is INV-4, and it is what makes every later evaluation term safe to add
incrementally rather than recomputed.
