id:         S174
goal:       `algebraic_to_move()` fails by returning 0 in every build instead of fabricating a move, and `make_book` refuses to write a book from a game it could not parse
accepts:    the three failure paths in `algebraic_to_move()` (`src/bitboard.cpp:2418-2424`, `src/bitboard.cpp:2430-2435`, `src/bitboard.cpp:2483-2488`: a destination that is not two characters, a destination off the board, a token matching no legal move) return 0 in Release and Debug alike, with no `assert` on the path -- an unparseable token is input, not an invariant violation, so a tool reading external PGN must be able to see it; the parser strips trailing `!`/`?` runs (`!`, `?`, `!?`, `?!`, `!!`, `??`) in the same loop that strips `+` and `#` -- in `algebraic_to_move()` rather than in `movetext_to_san()` (`tools/make_book.cpp:72-140`) as first written, because `pgn_to_positions` has no `movetext_to_san()` and needs it too -- so annotated PGN parses to the same moves as the unannotated form; `make_book build` exits non-zero and writes no output file when any game was cut short, unless `--allow-cut-short` is given, so the "games cut short" counter becomes a gate and not a report; a ctest fixture over the two tools, written red-first with the finding's three one-game PGNs (`1. e4!? e5 2. Nf3 Nc6 *`, `1. e4 e5 2. Qxf7 Nc6 *`, and the control `1. e4 e5 2. Nf3 Nc6 *`) asserting: the annotated game builds byte-identically to the control; the illegal game makes `build` exit non-zero with no file at `--out`, and exit 0 with the game dropped under `--allow-cut-short`; `pgn_to_positions` on `e4!? e5 Nf3 Nc6` prints the four true FENs and on `e4 e5 Qxf7` exits non-zero -- observed red on the unfixed tree (today the annotated game writes the start position with move `a8a7` and exits 0) and green after; a unit case that the three failure classes return 0; `tools/search_bench.py` node counts and best moves identical at depths 9 and 12 (INV-6 -- the parser is on no search path, and the rule is applied anyway); the shipped `src/openings.bin` is untouched by this step and its digest `3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd` unchanged; `DEV_MANUAL.md`'s `make_book` section states the gate and the flag; `MANUAL.md` checked, no UCI surface is touched
touches:    src/bitboard.cpp, tools/make_book.cpp, tests/CMakeLists.txt, tests/ (new fixture test), DEV_MANUAL.md
excludes:   the Polyglot key and the book rebuild (S175); the atomic replacement of the output file (S173); any SAN grammar beyond the suffix annotations -- `$N` glyphs, comments and variations are already stripped; changes to `tools/pgn_to_positions.cpp` beyond what the parser fix gives it, because its zero guard at `tools/pgn_to_positions.cpp:32-37` is already written and only needs a parser that returns zero
decisions:
closes:     2026-09-03_adversarial-F02
blocks:     S175
paused_by:
author:     claude (Fable 5.1), 2026-09-04
done:       2026-09-04. `algebraic_to_move()`'s three failure paths return 0 in every build with no assert on the path, the destination square is range-checked before `str_to_index()`, and `!`/`?` suffix annotations are stripped in the same loop as `+`/`#` -- in the parser rather than in `movetext_to_san()` as the accepts first said, so `pgn_to_positions` gets it without a change; `make_book build` refuses to write when any game was cut short, names the game and the token on stderr (first twenty), and takes `--allow-cut-short` for the old report; its flag loop also now rejects a trailing flag with no value instead of skipping it silently. Red observed first on the unfixed tree at `1dac39d`: `test_chesso` 2 new cases, 23 assertions failed (fabricated moves such as `7680` for `e4!?`); `test_make_book_tools` 9 of its checks failed, including the `a8a7` book and `--allow-cut-short` silently ignored by the pairwise flag loop. Green after; `ctest -L fast` 25/25 in `build` and `build-tune`, `clang-format --check` clean. INV-6: `search_bench` 121512 / 800769 / 62907 at depth 9 and 639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q`, identical. Shipped book untouched, sha256 `3b89a4ad9146e266ae9296778067aaedcb7f57f3cf0ff2086b9ae6df15b873dd`; rebuilt through the gated tool from `books/8moves_v3.pgn` it is byte-identical, 34700 games, 0 cut short. `DEV_MANUAL.md` (make_book section, analyse-a-game section) and `adocs/specs.md` (book paragraph) updated; `MANUAL.md` checked, no UCI surface moved. Fixture test expectations corrected twice while writing them, not the code: `pgn_to_positions` prints the terminal position as a fifth line, and echoes the token as given so the annotated/clean comparison excludes column 2.

## Why this exists

`2026-09-03_adversarial-F02`, one of the audit's two medium findings. Under
`NDEBUG` the parser's three failure paths fall through to
`src/bitboard.cpp:2490-2492` and return a move built from whatever the partial
parse left behind -- `from` 0, a piece letter, a `to` that can exceed 63 and
shift into the piece field. Both callers test for zero and never see it
(`tools/make_book.cpp:344-351`, `tools/pgn_to_positions.cpp:32-37`), and
`make_move()` applies the fabricated move with no legality check, so the board
is rewritten rather than left illegal.

Reproduced by the audit on the Release tools: `1. e4!? e5 2. Nf3 Nc6 *` makes
`make_book build` write a book whose first entry is the start position with
move `a8a7`, report `games cut short 0`, exit 0, and `dump` calls the result
`loadable`. `pgn_to_positions` on the same line prints a FEN with a white
bishop on a7 and the a8 rook gone, exit 0. That tool is the one `CLAUDE.md`
names for putting a position on a board (DEC-023), so on an annotated game the
agent's chess oracle would score positions that never occurred and report
success.

## What it does not change

The shipped book is verified correct by the audit's independent re-derivation
with python-chess: every move and every weight agrees, and the only
disagreement is the seven keys S175 owns. So nothing shipped changes here, and
the S146 stamp's "0 games cut short" claim is true of that PGN even though the
counter could not have detected the failure it claims to exclude. The book is
not rebuilt in this step; S175 rebuilds it after this lands so the rebuild runs
on a parser that cannot fabricate a move.

## Order

First of the 2026-09-03 batch, ahead of S175, because S175's rebuild uses
`make_book`. No match is owed: the parser is on no search path, and the default
configuration never probes the book.
