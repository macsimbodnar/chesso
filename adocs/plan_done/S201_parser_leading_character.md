id:         S201
goal:       `algebraic_to_move()` refuses a token whose first character is not a piece or file letter, instead of reading it as a pawn move and swallowing the real piece letter
accepts:    `.Nf3`, `.e4`, `..e4`, `-e4` and `xd5` return 0 from the start position, where `.Nf3` returned the pawn move `f2f3` before, observed red first; `Nf3`, `e4`, `exd5`, `Nbd2`, `O-O`, `O-O-O`, `e8=Q` and every token the existing round-trip case generates are unchanged; the fast suite is green in both builds; `tools/search_bench.py` node counts and best moves identical (INV-6 -- the parser is not on the search path); the shipped `src/openings.bin` rebuilt from `books/8moves_v3.pgn` is byte-identical, since that PGN has no such token; `adocs/specs.md`, `DEV_MANUAL.md` and `MANUAL.md` checked, and the specs clause on the parser failing closed says what it now refuses
touches:    src/bitboard.cpp, tests/test_chesso.cpp, adocs/specs.md, DEV_MANUAL.md
excludes:   the disambiguation walk itself, which stays as lenient as it is for characters *after* the first -- `N.f3` gives `g1f3`, lenient but correct, and tightening it is a separate decision the owner took the narrow option against (DEC-148); `movetext_to_san()`, which S200 owns; the other import-format leniencies of S178's `excludes:`
decisions:  DEC-148
closes:
blocks:
paused_by:
author:     Claude Opus 5, coordinator
done:       2026-09-07. One gate in `algebraic_to_move()`, after the suffix strip and the two castling returns: an empty token, or one whose first character is not in `[a-hKQRBN]`, returns 0. **Red first at `1705357`**, a new `SUBCASE` in `tests/test_chesso.cpp`: five `CHECK`s failed, `.Nf3` coming back as `2933` -- the pawn move -- and `.e4`, `..e4`, `-e4` and `xd5` as their own pawn moves. Green after. The precondition was established before the negative, as the TESTS rule requires: `REQUIRE(algebraic_to_move("Nf3", &game) != 0)` runs first, and it is the **first positive assertion in that TEST_CASE** -- every existing subcase asserts `== 0`, which is why running the binary with a `-tc=` filter and no earlier case to initialise `game_tables()` made them all pass vacuously while the new `REQUIRE` failed. Under `ctest`, which is how the gate runs it, the case is red for the right reason and green after. Worth S193's attention; not fixed here. **Verified over the corpus, not argued**: `books/8moves_v3.pgn` rebuilt through the changed parser gives 34700 games read, 0 cut short, 172232 entries and a book byte-identical to `src/openings.bin` at `77f47f1b...db06b58`, so roughly 278000 real SAN tokens -- captures, disambiguation, promotions, castling -- parse exactly as before. **INV-6 identical**: depth 9 `121512 / 800769 / 62907`, depth 12 `639228 / 3430710 / 367858`, best moves `c3d5 / e2a6 / d7c8q`; the parser is not on the search path. **Gate**: 27/27 in `build` and 27/27 in `build-tune`, `./clang-format.sh --check` exit 0 under `CLANG_FORMAT_MAJOR=22` (DEC-146). DEC-140's `Bench:` line binds from S189's completing commit on and S189 is open, so none is owed; `tools/gate.sh` does not exist yet. **Refused after, and each was checked**: `.Nf3`, `.e4`, `..e4`, `-e4`, `xd5`, and `Zf3` as before. **Unchanged**: `Nf3`, `e4`, `O-O`, and `N.f3` still giving `g1f3` -- lenient after the first character, by decision. **Documents**: `adocs/specs.md`'s book paragraph and `DEV_MANUAL.md`'s parser sentence say what is now refused and why; `MANUAL.md` checked -- the UCI move path is long algebraic and never reaches this function, so nothing there changed. DEC-148 records the ruling, the narrow choice and the three rejected options.

## Why this exists

Found 2026-09-07 while sizing S200's second half, and created directly in
`plan_current/` under the BUGS rule rather than filed: a known defect in the
tree contaminates every measurement taken after it. The owner was shown the
evidence and ruled on both the timing and the strictness; DEC-148 records it.

`algebraic_to_move()` reads the piece letter at position 0 only --
`if (pos < notation.size() && std::isupper(notation[pos]))` -- so a leading
character that is not an uppercase piece letter falls to the `else`, which
declares the move a pawn move. The disambiguation walk that follows records
only characters in `a`-`h` and `1`-`8`, so it silently consumes the real piece
letter on its way to the destination square. Measured at `1705357`, through
`build/tools/pgn_to_positions` from the start position:

```
.Nf3   -> f2f3      a pawn push; the token means g1f3
.e4    -> e2e4      right by accident
..e4   -> e2e4
-e4    -> e2e4
xd5    -> a pawn move to d5
N.f3   -> g1f3      lenient, and correct: after the first character
```

python-chess refuses `.Nf3`, `.e4` and `..e4` (checked the same day, the oracle
CLAUDE.md names for this). This is the shape S174 closed for a token the parser
*cannot* read, still open for one it reads as something else: the move is legal,
`make_move()` applies it, and the caller is told nothing.

Reachable, not theoretical. `1 .Nf3` is legal PGN import format -- 8.2.2.1
allows whitespace between the digit sequence and the period(s) -- so
`movetext_to_san()` hands `.Nf3` to the parser and `make_book` builds a book
from a wrong board with `games cut short 0`. S200 stops the tool emitting such
a token; this stops the parser accepting one, and each is worth having without
the other.

**Bounded.** `algebraic_to_move()` is called from `tools/make_book.cpp`,
`tools/pgn_to_positions.cpp` and `tests/test_chesso.cpp` and nowhere else: the
engine's UCI move path is long algebraic and never reaches it. `books/8moves_v3.pgn`
carries no such token, so `src/openings.bin` is unaffected and no shipped byte
moves. The suite tests `Zf3`, which is refused because `Z` is uppercase and
falls to the piece switch's `default`; nothing covers a non-uppercase leading
character, which is why the round-trip case never saw it.

## The edit

One gate in `algebraic_to_move()`, after the suffix strip and after the two
castling forms return, before the piece letter is read. A token that reaches
that point must begin with a piece letter or a file letter; PGN 8.2.3 gives SAN
no other first character once castling and the suffix marks are out. Empty is
tested first, since the suffix strip can empty the token (`!?`).

Narrow by decision. The wider gate -- every character in `[a-h1-8KQRBNx=]` --
would also close the lenient interior forms and match python-chess, and the
owner took the narrow option: it closes every case that produces a *wrong*
move and changes nothing that is merely tolerant. DEC-148.

## Tests

Red first, a new `SUBCASE` in `tests/test_chesso.cpp`'s existing
`algebraic_to_move returns 0 for what it cannot parse`. The precondition is
established before the negative, as the TESTS rule requires: `Nf3` is asserted
to be a real move from the start position, and only then is `.Nf3` asserted to
be 0 -- otherwise the case would pass on a board where the knight move was
illegal anyway.

Not owed: `algebraic_to_move()` is not `make_move`, `unmake_move`, the
generator or the search, so DEC-141's second tier brings no Debug self-play, no
guard test and no mutant. No SPRT: the parser is not on the search path and no
game the engine plays reaches it. INV-6 is the proof of that and is owed --
identical node counts and best moves at depths 9 and 12.

DEC-140's `Bench:` line binds from S189's completing commit on, and S189 is
still open, so this commit owes none.
