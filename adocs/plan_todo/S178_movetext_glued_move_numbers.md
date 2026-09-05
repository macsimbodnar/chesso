id:         S178
goal:       `movetext_to_san()` splits a move number glued to its move, so PGN import format (`1.e4 e5 2.Nf3`, `1...e5`) parses instead of cutting the game short at ply 0
accepts:    a token of digits, one or more dots, then a move (`1.e4`, `12...Nf6`, `3.O-O`) yields the move alone; a token of digits and dots only is still dropped as a move number and a bare number is still dropped; a fixture PGN in `tests/test_make_book_tools.sh` written in the glued form builds byte-identically to the same game written as `1. e4 e5 2. Nf3 Nc6 *`, observed red first (today: `game 1 cut short: cannot parse '1.e4' at ply 0`, exit 1, no file) and green after; the shipped `src/openings.bin` rebuilt from `books/8moves_v3.pgn` is byte-identical to the committed file, since that PGN has no glued token (`grep -c -E '(^|[[:space:]])[0-9]+\.[a-hNBRQKO]'` is 0); `tools/search_bench.py` node counts and best moves identical (INV-6, no search path); `DEV_MANUAL.md`'s make_book section says the import form is accepted
touches:    tools/make_book.cpp, tests/test_make_book_tools.sh, DEV_MANUAL.md
excludes:   `pgn_to_positions`, which reads whitespace-separated SAN and documents "no move numbers"; any other PGN import-format leniency (e.g. `e8Q` without `=`, `0-0` with zeros, `P` prefixes) -- each is its own small decision; the parser `algebraic_to_move()` itself
decisions:
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found while doing S174, 2026-09-04, by running the fixed tools on the PGN
import form. `movetext_to_san()` (`tools/make_book.cpp:72-140`) drops a token
that is digits followed only by dots, and passes everything else to the parser,
so `1.e4` reaches `algebraic_to_move()` whole. Before S174 the parser
fabricated a move from it and the game was built from a rewritten board; since
S174 it returns 0 and the game is cut short at ply 0, the build is refused and
the message names the token:

```
game 1 cut short: cannot parse '1.e4' at ply 0
1 game(s) cut short -- book not written. ...
exit=1
```

Correct, and useless for the half of the world's PGN files that write the
import form -- the standard's export format is `1. e4`, its import format
allows `1.e4`, and tools emit both. The shipped PGN is all export form (the
grep above is 0), so nothing shipped is affected and this does not jump the
queue: the tool refuses honestly today, it does not lie. Filed as its own step
rather than folded into S174 because S174's goal is that the tool fails closed,
and this is about what it accepts.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

`make_book` builds the engine's Polyglot book from a PGN. Its first stage,
`movetext_to_san()` in `tools/make_book.cpp`, splits the movetext at whitespace
and drops what is not a move: comments, variations, `$n` glyphs, the result,
move numbers. It knows a move number only as a token on its own, `1.` or
`12...`; glued to its move, `1.e4` reaches `algebraic_to_move()` in
`src/bitboard.cpp` whole, gets 0 (S174), and `build()` refuses the file at ply
0. Reproduced 2026-09-05 at `2445d23`: `1.e4 e5 2.Nf3 Nc6 *` gives `game 1 cut
short: cannot parse '1.e4' at ply 0`, exit 1, no file; `1. e4 e5 2. Nf3 Nc6 *`
builds 4 entries. This step teaches the splitter the glued form. No engine code
and no shipped byte changes: `books/8moves_v3.pgn` is all spaced form.

### 2. The technique as published

"Portable Game Notation Specification and Implementation Guide", Steven J.
Edwards, 1994, https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm.
The revision "Version 1.1, Revised 2026-04-18" at
https://raw.githubusercontent.com/fsmosca/PGN-Standard/master/PGN-Standard.txt
has identical wording in every section quoted. Both fetched 2026-09-05.

Sections 3.1 and 3.2: import format is "rather flexible", for data "prepared by
hand", and "a program that can read PGN data should be able to handle the
somewhat lax import format"; export format is "rather strict", for data
"prepared under program control".

Why the glued form is legal: the standard's lexer is not whitespace-split. 7.2:
an integer token "is terminated just prior to the first non-symbol character
following the integer digit sequence". 7.3: a period "is a token by itself" and
"is self terminating". 7.9: symbol continuation characters are letters, digits,
`_`, `+`, `#`, `=`, `:`, `-`; the period is not one. So `1.e4` lexes as
integer, period, symbol, exactly like `1. e4`.

8.2.2: a move number indication is "one or more adjacent digits (an integer
token) followed by zero or more periods". 8.2.2.1, import: not required,
superfluous ones allowed "as long as the move numbers are correct", "zero or
more period characters following the digit sequence", and "one or more white
space characters may appear between the digit sequence and the period(s)".
8.2.2.2, export: white is the integer "with a single period character
appended", black "with three period characters appended", and a black
indication appears only after intervening commentary or when the game starts
with Black to move.

Chesso's rule: a token beginning with digits then at least one period is an
indication glued to what follows; drop the indication, keep the rest. Chesso
does not check that "the move numbers are correct"; it never did for spaced
ones, and a wrong number is not a wrong move.

What NOT to do here, the `excludes`, each its own leniency and decision:

- `e8Q` without `=`: 8.2.3.3 defines promotion as "the equal sign "="
  immediately following the destination square" plus a piece letter and names
  no other form. 8.2.3.7: import "is somewhat more relaxed", suggested
  transformations are "letter case remapping, capture indicator insertion,
  check indicator insertion, and checkmate indicator insertion", and "these
  allowances may differ among different PGN reader programs".
- `0-0` with zeros: 8.2.3.3, "the upper case letter "O" is used, not the digit
  zero", since a zero "can also confuse parsing algorithms which also have to
  understand about move numbers and game termination markers" -- the ambiguity
  this step's digit rule lives in.
- `P` prefix: 8.2.3.2, the pawn letter "is not used for SAN moves in PGN export
  format movetext", though "some PGN import software disambiguation code may
  allow" it.
- Whitespace between digits and period(s), `1 . e4`, `1 .e4`, or a token of
  periods alone: allowed by 8.2.2.1, not in `accepts:`; section 10.

### 3. What chesso has today, and where the change plugs in

Data flow in `tools/make_book.cpp`: `read_game()` joins movetext lines with
`\n` into `pgn_game_t::movetext`; `build()` calls `movetext_to_san()` per game
and hands each token to `algebraic_to_move()` then `make_move()`; a 0 move goes
to `report_cut_short()` as `cannot parse '<token>' at ply N`, bumps
`rejected_games`, and the write is refused unless `--allow-cut-short`.

In `movetext_to_san()`, after a token is cut at whitespace or `{ ( ) ;`, the
order is: variation-depth skip; empty or `$` skip; `*` and
`outcome_from_string()` skip; the move-number test, `dot =
token.find_first_not_of("0123456789")`, dropping when `dot > 0` and
`token.find_first_not_of('.', dot)` is `npos`; `dot == npos` drops a bare
number; the rest is pushed. For `1.e4`, `dot` is 1, the second search finds
`e` at 2, neither drop fires, the whole token is pushed. That is the defect.

The edit, entirely inside `movetext_to_san()`:

1. `const std::string token` becomes `std::string token`.
2. Replace the two drop tests, in this order: `dot == npos`, `continue` (bare
   number, unchanged); if `dot > 0 && token[dot] == '.'`, set `rest =
   token.find_first_not_of('.', dot)`; `rest == npos`, `continue` (`1.`,
   `1...` alone, unchanged); else `token.erase(0, rest)`, so `1.e4` gives
   `e4`, `12...Nf6` gives `Nf6`, `3.O-O` gives `O-O`; push. The `token[dot] ==
   '.'` guard keeps `1-0`, `0-1`, `1/2-1/2` (all `dot` 1, non-period there)
   out of the split even though the result test already removed them.
3. The function comment says "move numbers"; make it "move number indications,
   alone or glued to the move they introduce (import format, PGN 8.2.2.1)".

Order of work: test first (section 6), observe red, code, `DEV_MANUAL.md`.

### 4. Constants and seeds

None: the change introduces no constant. Omitted.

### 5. Interactions and traps

- No pruning, ordering band, evaluation term, table bound or time path is
  touched; `chesso` is built from `src/` only, no `src/` file changes, so the
  binary is byte-identical before and after.
- The remainder after the split is not re-classified: `1.2` reaches the parser
  as `2` and is cut short as `2`, `1.xyz` as `xyz`. That is S174's honest
  failure. Do not add a bare-number drop on the remainder: silently dropping
  `1.2` hides broken input, the fault class of 2026-09-03_adversarial-F02.
- So the cut-short message names the move part, not the token as written.
  Acceptable; section 10 if the owner wants the original.
- `movetext_to_san()` is not shared: `grep -rn movetext_to_san src/ tests/
  tools/` finds only `tools/make_book.cpp`. `pgn_to_positions` reads
  whitespace SAN via `std::cin >> token` and documents "no move numbers";
  excluded, leave it. `algebraic_to_move()` is excluded and needs nothing: it
  strips `+ # ! ?` and reads `O-O`.
- S174's stamp: fixture expectations were "corrected twice while writing them,
  not the code". Take the expected entry counts from the spaced twin's actual
  output, not from memory.
- `write_pgn` passes the movetext through `printf '%s'`; a `%` in a movetext
  would break it, none here has one. `README.md` is never written by an agent.

### 6. Tests

Red first, the S174 pattern, in `tests/test_make_book_tools.sh`: extend the
header comment with properties 7 and 8, write three fixtures beside the
existing ones, add the checks after property 4:

```
write_pgn '1.e4 e5 2.Nf3 Nc6 *' "$tmp/glued.pgn"
write_pgn '1. e4 e5 2. Nf3 Nc6 3. Bc4 Nf6 4. O-O *' "$tmp/castle_spaced.pgn"
write_pgn '1.e4 e5 2.Nf3 {c} 2...Nc6 3.Bc4 Nf6 4.O-O *' "$tmp/castle_glued.pgn"

# 7. Import-format move numbers: glued builds byte-identically to the control.
if ! "$make_book" build "$tmp/glued.pgn" --out "$tmp/glued.bin" \
    > "$tmp/glued.out" 2>&1; then
  fail "glued: build exited non-zero: $(cat "$tmp/glued.out")"
fi
grep -q 'games cut short      0' "$tmp/glued.out" || fail "glued: cut short"
grep -q 'entries written      4' "$tmp/glued.out" || fail "glued: expected 4"
{ [[ -f "$tmp/glued.bin" ]] && cmp -s "$tmp/control.bin" "$tmp/glued.bin"; } \
  || fail "glued: book differs from the control's"
# 8. Black indication after a comment, castling glued to its number: the same
#    four checks, castle_glued against castle_spaced (built first), 7 entries.
```

Unlike property 2's `[[ -f a && -f b ]]` guard, a missing `glued.bin` must be
a failure; the braced form does that. The lines are legal by two tools, not by
reading: python-chess in `~/.venv/chess` and `build/tools/pgn_to_positions` on
`e4 e5 Nf3 Nc6 Bc4 Nf6 O-O` both give 7 plies ending `e1g1` (2026-09-05); the
spaced twins build 4 and 7 entries at `2445d23`.

Red: `cmake --build build -j8 --target make_book pgn_to_positions && ctest
--test-dir build -R test_make_book_tools --output-on-failure`; on the unfixed
tree 7 and 8 each fail the build check (exit 1, `cannot parse '1.e4' at ply 0`)
and the `cmp`; count the `FAIL:` lines for the stamp. Fix, rebuild, rerun:
`ok`. Then the full gate in both builds, the TESTS command in `AGENTS.md`.

Independent oracle, cheap: `~/.venv/chess/bin/python
adocs/data/S175_book_conformance.py <glued.pgn> <glued.bin>` prints `missing 0
extra 0 weight_mismatch 0`; python-chess reads the glued form natively (checked
2026-09-05 on all three fixtures). No venv: skip, say so in the stamp.

Not owed: guard test, mutant, mate instruments, Debug self-play -- no pruning
rule, no `make_move`, generator or search change, so DEC-141's second tier does
not apply. INV-6 as `accepts:` asks: `python3 tools/search_bench.py
./build/src/chesso 9` and `... 12` before and after, identical node counts and
best moves, necessarily, since the binary is unchanged; quote both triples in
the stamp as S174 did.

Shipped book: `build/tools/make_book build books/8moves_v3.pgn --out
<scratch>/openings.bin && cmp <scratch>/openings.bin src/openings.bin &&
shasum -a 256 src/openings.bin` gives
`77f47f1bd184df6d1e6be539c354b526558d2e4970c6dc565c5ea53e5db06b58`, the S175
digest; the `3b89a4ad...` S174 quotes is the superseded file. Precondition per
`accepts:`, checked 2026-09-05: `grep -c -E '(^|[[:space:]])[0-9]+\.[a-hNBRQKO]'
books/8moves_v3.pgn` prints 0, as does `grep -c '\.\.\.'`.

### 7. Measurement

Neither lane. No SPRT: nothing alters play; `OwnBook` defaults false, the book
is byte-identical, the engine binary unchanged. No `hyperfine`: no speed claim.
INV-6 is discharged by section 6's `search_bench` identity, as S174 did for the
same tool. No `adocs/data/S178_*.sh` is created.

### 8. Completion checklist

- Gate green in `build` and `build-tune`, `./clang-format.sh --check` clean.
- No `src/` file in the diff: neither `Bench:` nor `No functional change` is
  owed (DEC-140 binds `src/`). If S189's `tools/gate.sh` exists by then, run
  it and follow what it prints.
- `DEV_MANUAL.md`, "The engine's own opening book", says nothing about
  move-number forms today. Add one sentence to the S174 paragraph (beginning
  "A cut-short game refuses the build"): export form `1. e4 e5` and import form
  `1.e4 e5`, `2...Nc6` are both read, the indication is split off the move it
  is glued to (S178); whitespace between digits and period is not accepted,
  while section 10's first question is open. Trace it to `movetext_to_san()`.
  "Analyse a game" keeps "no move numbers" for `pgn_to_positions`.
- `MANUAL.md`: no UCI surface touched; "checked" in the stamp.
- `adocs/specs.md` book paragraph: propose to the coordinator "and since S178
  reads import-format move numbers glued to the move", or conclude no change;
  either goes in the stamp. The coordinator writes it.
- Stamp: date; red count with the unfixed tree's sha; green; both
  `search_bench` triples; book digest and `cmp`; conformance result or "not
  run, no venv"; the `DEV_MANUAL.md` sentence; `MANUAL.md` checked; the
  `specs.md` decision.
- File to `plan_done/`; `plan.md` and `status.md` through the coordinator;
  commit body references S178 and INV-6; no push.

### 9. Sources read

- https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm: the 1994
  standard, curl 2026-09-05; sections 3.1, 3.2, 7.2, 7.3, 7.9, 8.2.2,
  8.2.2.1, 8.2.2.2, 8.2.3.2, 8.2.3.3, 8.2.3.7, 8.2.3.8 quoted above.
- https://raw.githubusercontent.com/fsmosca/PGN-Standard/master/PGN-Standard.txt:
  the 2026-04-18 revision, fetched; same wording, used to locate sections.
- https://www.thechessdrum.net/PGN_Reference.txt: HTTP 403, not read.
- `tools/make_book.cpp` `movetext_to_san`, `read_game`, `build`,
  `report_cut_short`; `src/bitboard.cpp` `algebraic_to_move`;
  `tools/pgn_to_positions.cpp`; `tests/test_make_book_tools.sh`;
  `tests/CMakeLists.txt` `test_make_book_tools`;
  `adocs/plan_done/S174_san_parser_fails_closed.md`; commits `35d02b7`,
  `9f3d98d`; `DEV_MANUAL.md` "The engine's own opening book", "Analyse a
  game"; `adocs/specs.md` book paragraph and INV-6;
  `adocs/data/S175_book_conformance.py`; `books/8moves_v3.pgn`.

### 10. Questions deferred to the owner

1. 8.2.2.1 allows "one or more white space characters may appear between the
   digit sequence and the period(s)": `1 . e4`, `1 .e4`, `1. ... e5`. A token
   of periods alone, or periods then a move, has `dot == 0`, goes to the
   parser and cuts the game short honestly; `accepts:` requires digits before
   the periods. Extend this step to drop a leading run of periods too (same
   rule, a few lines), or file it as its own step?
2. The cut-short message names the move part (`e4`), not the token as written
   (`1.e4`). Keep, or carry the original for the message?
3. Whether `adocs/specs.md`'s book paragraph takes section 8's half-sentence.

Corrections to older sections: no "Technical details (SOTA research ...)"
section exists. "Why this exists" cites `movetext_to_san()` by a line range
that no longer holds at `2445d23`; cite `tools/make_book.cpp`
`movetext_to_san()` instead. Left as written, outside this section's mandate.
