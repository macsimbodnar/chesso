id:         S175
goal:       `get_key()` finds the en-passant capturer by file so the Polyglot key never wraps round the board edge, and the shipped book is rebuilt to the format's keys
accepts:    `tests/test_audit_polyglot_key.cpp`, written red-first by the audit, is registered in `tests/CMakeLists.txt`, observed red against the unfixed tree (7 of 10 assertions failing at `1d8cbac`) and green after; `get_key()` (`src/openings.cpp:562-593`) tests the two candidate squares by file -- for an en-passant square on file `f`, file `f - 1` only when `f > 0` and file `f + 1` only when `f < 7`, on the rank the capturing pawn stands on -- instead of by `+7/+9` and `-9/-7` index offsets; the nine format example keys in `tests/test_openings.cpp` still pass; `src/openings.bin` is rebuilt with `build/tools/make_book build books/8moves_v3.pgn --out src/openings.bin` at the tool's defaults after S174 has landed; the audit's independent re-derivation is committed as `adocs/data/S175_book_conformance.py` (python-chess, `~/.venv/chess`) and reports `missing 0 extra 0 weight_mismatch 0` against the rebuilt file, with 172232 entries over 129613 positions unchanged; the new sha256 replaces the old `3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd` in `adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `src/openings_embedded.S` and every test that pins it, and a grep for the old digest finds it only where it is history -- `adocs/plan_done/`, `adocs/decisions.md`, `adocs/audit/`, the lines in `adocs/plan.md`'s Done list and `adocs/status.md` that record what S146 produced and what S174 checked (the S146 lines now also name the S175 digest), and the sentences in `adocs/specs.md` and `DEV_MANUAL.md` that name the 2026-09-03 file as the one this step replaced; `tools/search_bench.py` node counts and best moves identical at depths 9 and 12 (INV-6 -- the default configuration never probes the book); `MANUAL.md`'s "The book it ships with" section checked and its digest updated
touches:    src/openings.cpp, tests/CMakeLists.txt, tests/test_audit_polyglot_key.cpp, src/openings.bin, src/openings_embedded.S, adocs/specs.md, MANUAL.md, DEV_MANUAL.md, adocs/data/S175_book_conformance.py, tests/test_openings.cpp
excludes:   the parser and the write gate (S174); the atomic replacement (S173); `polyglot_randoms[781]` (DEC-121, settled); book selection and the UCI options (S172, settled); any change to `books/8moves_v3.pgn` or to `make_book`'s defaults
decisions:
closes:     2026-09-03_adversarial-F01
blocks:
paused_by:
author:     claude (Fable 5.1), 2026-09-04
done:       2026-09-04. `get_key()` takes the two capturer squares by file with bounds on the rank the mover's pawns capture from (`src/openings.cpp`, block marked S175); `tests/test_audit_polyglot_key.cpp` registered and observed red through ctest on the unfixed tree, 7 of 10 assertions, then 10 of 10 green; the nine format example keys in `test_openings` unchanged and green. `src/openings.bin` rebuilt through the S174-gated `make_book` from `books/8moves_v3.pgn`: 34700 games, 0 cut short, 2755712 bytes, 172232 entries over 129613 positions, sha256 `77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`, swapped into `adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`, `src/openings_embedded.S` and S173's accepts. `adocs/data/S175_book_conformance.py` committed: against the old file it reproduces the audit exactly (`missing 7 extra 7 weight_mismatch 0`, the same seven keys), against the new file `missing 0 extra 0 weight_mismatch 0 duplicate_book_entries 0`, exit 0; an empty PGN or book exits 1 (fast-check nit, fixed in scope). INV-6: `search_bench` 121512 / 800769 / 62907 and 639228 / 3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`, identical. End to end: `OwnBook` on, `position fen rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq h6 0 8` (one of the seven) answers `bestmove d2f3` from the book with no `info` line. Fast suite 26/26 in `build` and `build-tune`, `clang-format --check` clean. `MANUAL.md` checked: digest updated, no option or command changed, golden surface untouched. Fast check's other finding -- the old digest still on the S146 lines of `plan.md` and `status.md` -- answered by naming the S175 digest beside it rather than rewriting what S146 produced.

## Why this exists

`2026-09-03_adversarial-F01`, the audit's other medium finding. The Polyglot
key carries an en-passant component only when a pawn of the side to move
stands beside the pushed pawn. `get_key()` locates that pawn with index offsets
from the en-passant square, and on the a8 = 0 index scheme those offsets wrap
at the edge files: white to move with en-passant `h6` tests **a4**, `a6` tests
**h6**; black to move with `a3` tests **h5**, `h3` tests **a3**. A same-side
pawn on the wrapped square switches the component on, against the format
quoted in the file's own comment at `src/openings.cpp:549-552`.

Measured over the whole shipped book by the audit, not argued: every mainline
of `books/8moves_v3.pgn` replayed to 16 plies with python-chess 1.11.2 and keyed
with `chess.polyglot.zobrist_hash()`, moves encoded in the Polyglot word, the
multiset compared with `src/openings.bin`:

```
games 34700 plies 555200 derived_entries 172232 book_entries 172232 distinct_positions 129613
missing 7 extra 7 weight_mismatch 0
```

All seven have the en-passant square on an edge file and a same-side pawn on
the wrapped square, and in each the book holds chesso's key and not the
format's. The seven positions are in the finding and in the audit's test.

## Impact, and why it is not a verdict

Three effects, none a measurement. The shipped book is non-conforming for 7 of
its 172232 entries, so a spec-conforming reader never finds them. With `Book
File` pointing at a third-party book, chesso computes a key the book does not
hold at any such position, `get_book_moves_for_key()` returns 0, and
`search_book_move()` abandons the book for the rest of the game silently
(`src/chesso.cpp:678-681`). Every book `make_book` builds inherits the
deviation. `OwnBook` defaults false and S158 established that no measurement on
record has played a book move, so no verdict is touched and INV-6 is discharged
on node counts as S172 and S146 discharged it.

## Why the test missed it

`tests/test_openings.cpp:22-47` holds the format's own nine example keys, and
none of them has an en-passant square on an edge file. The registered audit
test is the durable reproduction; the conformance script is the check that the
shipped file is the format's, which S146's digest could never say.

## Order

Second of the 2026-09-03 batch, after S174, because the rebuild runs
`make_book` and that tool must be unable to fabricate a move first. The digest
moves and S146's reproducibility claim survives: the writer still sorts, and two
runs are still byte-identical.
