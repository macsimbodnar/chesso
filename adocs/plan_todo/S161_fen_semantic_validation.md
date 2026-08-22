id:         S161
goal:       load_FEN clears castling rights and en-passant squares the position on the board cannot support, so accepted input can no longer corrupt the board
accepts:    the three FENs in the finding load with the unsupported right or en-passant square cleared rather than rejected — clearing is what fastchess and cutechess do to the same input, and a hard reject would break GUIs that send stale fields; `tests/test_audit_fen_semantics.cpp`, written red-first by the audit, is registered in `tests/CMakeLists.txt`, observed red against the unfixed tree, and green after; the Debug `squares_match_bitboards` and king-safety asserts hold across the suite; `tools/search_bench.py` node counts and best moves are identical, because the sanitization can only alter positions the engine previously corrupted (INV-6); MANUAL.md's UCI surface and known-bugs sections checked
touches:    src/bitboard.cpp, tests/CMakeLists.txt, tests/test_audit_fen_semantics.cpp, MANUAL.md
excludes:   full position-legality validation (side not to move in check, pawn counts, doubled kings) — only the two semantic classes the finding proves corrupting; hardening `castling_rook()` or `make_move()` themselves — the load boundary is the contract every downstream consumer already assumes
decisions:
closes:     2026-08-22_adversarial-F01
blocks:
paused_by:
done:

## Evidence

2026-08-22_adversarial-F01, the audit's one medium correctness finding and
the one prime-directive violation found. `load_FEN()` validates syntax only:
castling rights are parsed as bits with no check that king or rook stands on
its square (`src/bitboard.cpp:1699-1737`), the en-passant square only has to
parse as `[a-h][1-8]` (`src/bitboard.cpp:1755-1771`). The generator and
`make_move()` trust both, and the piece updates are xors — applied to a
square the piece is not on, they create pieces. In Release the corruption is
silent and every subsequent probe, generation and evaluation runs on a board
that is no position. Perft (INV-1) can never see it: every perft FEN is
semantically valid. Exposure is any `position fen` from a GUI, test or tool
carrying stale rights or a stale ep square.

## Cost

Small boundary change in `load_FEN`, one test registration, no match — the
INV-6 node-identity check is the verdict.
