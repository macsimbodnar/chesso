id:         S195
goal:       a fast test holds node-limited searches reproducible across `ucinewgame`, and `bench` resets the table before every position
accepts:    a case in the shape of Stockfish's `tests/reprosearch.sh`: two move sequences searched at ten node limits, each twice with `ucinewgame` between, every reported node count identical; a case that two `go depth N` on one FEN in one process, separated by `ucinewgame`, report identical nodes, and that without it the second runs on a warm table -- the hazard pinned as a documented property, not repaired; S189's `bench` sends `ucinewgame` per position (S189's accepts carries it if S189 lands first, this step adds it otherwise); `DEV_MANUAL.md` "Measure" records the warm-table rule beside the bench instructions; fast suite green in both builds
touches:    tests/test_engine.cpp, src/chesso.cpp, DEV_MANUAL.md
excludes:   the Zobrist key generator, which S179 now owns; any change to `set_position`'s reset rule
decisions:  DEC-139
closes:     2026-09-04_test_review-F08
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F08`. Stockfish's reproducibility test found a
table-ageing counter that `ucinewgame` did not reset, visible as two
alternating `bench` totals and invisible to an SPRT (Stockfish#5376,
`adocs/testing_strategy.md` section 3.1). Chesso has no equivalent:
`tests/test_search.cpp`'s determinism case repeats three searches on a fresh
table only. The review also found that `set_position` resets the table only
when the FEN string differs from the previous one, so two `go depth N` on the
same FEN in one process run the second warm; `tools/search_bench.py` is safe
because its positions differ, and a bench extended by one repeated position
would silently compare cold with warm. The third exposure of F08 -- Zobrist
keys drawn through `std::uniform_int_distribution<uint64_t>`, whose output is
implementation-defined -- is S179's, amended on 2026-09-05 to regenerate the
keys under the project generator beside the magics.

## Cost

Machine-free, hours.
