id:         P08 (proposed; the S-id is allocated at adoption)
goal:       the transposition table uses all the memory `Hash` grants and keeps the entries that matter: cache-line buckets, depth-and-age replacement, a size that is not forced to a power of two, and the move kept when a store carries none
accepts:    (1) bucket layout chosen by a census -- entries per bucket against key bits kept -- with sizes stated; every field today's entry carries survives (move, score, static evaluation, depth including TT_DEPTH_QS, bound, generation); (2) `Hash=N` uses N MiB to within one bucket, the index taken by multiply-high of the key rather than a mask; S209's `Hash` refusals unchanged; (3) replacement inside a bucket: the matching key first, else the entry with the least `depth − k·age`; a store of the same position with no move keeps the stored move; (4) every reader that can act on `best_move` without matching a generated move -- `root_move_hint`, the mate-line walk in `complete_mate_pv`, `certified_mate_move` -- checks legality, with a test that plants a colliding entry, because fewer key bits raise false hits; (5) test_mate_pv and test_mate_carry green, budgets re-derived by their own scripts where the tree moved (DEC-142, DEC-161); (6) one SPRT `{0, 5}` at the harness's Hash=16, then one fixed 1000-pair reading at Hash=128 and 32+0.32, an estimate in DEC-202's form; (7) a prefetch of the child's bucket lands only if it is node-identical and hyperfine shows a gain, otherwise it is out
touches:    src/transposition_table.cpp, src/transposition_table.hpp, src/data_structures.hpp, src/search.cpp, src/chesso.cpp, tests/test_engine.cpp, tests/test_mate_pv.cpp, tests/test_mate_carry.cpp, MANUAL.md, adocs/specs.md
excludes:   a shared table for SMP (DEC-175); separate regions for quiescence entries
closes:
paused_by:
author:
done:

## Why

`tt_resize()` takes `bit_floor(MB · 2^20 / 24)` entries: Hash=16 uses 12 MiB
and Hash=128 uses 96 MiB. The table is direct-mapped, one entry a slot
(`src/transposition_table.cpp:158`): an entry an earlier `go` wrote is
overwritten by any store, however shallow, and within one search a deeper
entry turns away every shallower position that collides into its slot for the
rest of the search. Measured 2026-08-19: 16 MB against 512 MB is 36 % fewer
nodes and 21 % lower nps at `go movetime 2000`.

## Description it is implemented from

CPW *Transposition Table*: bucket systems sized to a cache line, replacement
by depth with ageing, partial keys. Lemire, "A fast alternative to the modulo
reduction" (2016): `(x · N) >> w` maps a hash fairly into `[0, N)` without a
power-of-two size.

## Seeds (DEC-134)

Age weight `k`: from a census of generation gaps between a hit and its store
over a game replay (form 2), or the midpoint of a stated range (form 3).

## From the record

DEC-088 (Hash=16 for table pressure), S094 (QS entries at TT_DEPTH_QS, the
static evaluation in the entry), S147/S170/S171/S202 (the mate-line walk reads
the table and is the reader most exposed to a false hit).
