id:         S194
goal:       the UCI book path is executed by the fast suite, with the weighted draw seeded through an environment variable
accepts:    `CHESSO_BOOK_SEED`, read once at startup, seeds the book's `mt19937_64` when set and leaves `std::random_device` in place when absent; two fast cases on the embedded book: with `OwnBook true`, `position startpos` and `go depth 1` the `bestmove` is one of the moves the library returns for the start key, over several seeds; with `Best Book Move true` it is the heaviest entry; the S175 repaired position (`rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8`) answers `bestmove d2f3` with no `info` line; the previously unexecuted block of `src/chesso.cpp` (the probe, the draw and `Best Book Move`) is shown executed, by the S197 coverage recipe or by an observable in the test; `MANUAL.md` documents the variable in its book section; no UCI option is added and the golden surface is unchanged; fast suite green in both builds
touches:    src/chesso.cpp, tests/test_engine.cpp, MANUAL.md, DEV_MANUAL.md
excludes:   the book format, `src/openings.cpp`, `tools/make_book`
decisions:  DEC-139
closes:     2026-09-04_test_review-F06
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F06`. Coverage of the fast label shows the book probe,
the weight-proportional draw and `Best Book Move` in `src/chesso.cpp` with zero
executions; `tests/test_engine.cpp`'s book case tests `validate_book_move` on
a synthetic move and never reaches the probe, and `tests/test_openings.cpp`
tests the library. The S172 and S175 defects were in exactly this path, and
their regression guard is a command run by hand and recorded in a stamp. The
draw is seeded from `std::random_device` with no hook, which is why the path
has never been in a deterministic test.

## Cost

Machine-free, hours.
