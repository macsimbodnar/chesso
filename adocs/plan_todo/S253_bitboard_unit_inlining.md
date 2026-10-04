id:         S253
goal:       make_move's accumulator hooks inline whatever else grows in src/bitboard.cpp, so a generator change is timed on its own cost and not on gcc's inlining budget
accepts:    `-fopt-info-inline-missed` shows no `eval_add_piece` / `eval_remove_piece` call left out of line in `make_move` and `unmake_move_impl` after the change (eight do at `32633c4`); INV-6 node-identical (bench and tools/search_bench.py at 9 and 12); the interleaved hyperfine timing of S020's shape (A/A first, paired ratio and CI) decides keep-or-revert; then S020's increments (a) and (b) are re-attempted on top from `.tuning/coord/S020_files/inc2_attempt_final.diff` (S020's timing scripts beside it) and timed the same way, a zero recorded as zero
touches:    src/eval_tables.hpp (the hooks), src/bitboard.cpp / .hpp, possibly CMakeLists.txt (a unit-local parameter)
excludes:   any behaviour change; raising inline-unit-growth globally without measuring every unit
decisions:  DEC-049, DEC-083
closes:
blocks:
paused_by:
done:

## Why this exists (2026-10-04, the coordinator, from S020's report)

S020 found `src/bitboard.cpp` at gcc 13.3's `inline-unit-growth` limit: at
`32633c4` gcc already refuses 61 inlinings in that unit on the budget, eight
of `make_move`'s accumulator calls among them. Every shape of S020's
generator seam grew the unit, `make_move` lost more of its inlined hooks (14
or 16 out-of-line call sites against 8, plus two in `unmake_move_impl`), and
perft fell 11 to 15 %; the search measured -1.89 % and -3.52 %. Rebuilt with
`--param inline-unit-growth=200`, parent and candidate both inline every
hook and perft reads 1075 / 1092 ms against 1082 / 1083 ms -- the seam was
not the cost. S042, S032, S030 and S117 all edit that unit next, and each
would be timed against the same cliff.

Candidate fixes, chosen by measurement: `[[gnu::always_inline]]` on the
hooks; a unit-local `#pragma GCC optimize` or a per-source compile option;
splitting the generator out of the unit. The first also shows whether the
eight refusals at the parent already cost speed today.
