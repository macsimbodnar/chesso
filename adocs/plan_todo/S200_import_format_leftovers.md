id:         S200
goal:       `movetext_to_san()` reads the whitespace form of a move number indication (`1 . e4`, `1 .e4`, `1. ... e5`), and a cut-short message names the token as the PGN wrote it
accepts:    a token that is one or more dots, alone or followed by a move, is dropped or split the way a digits-then-dots token is -- `.` and `...` alone are dropped, `.e4` yields `e4`, `...Nc6` yields `Nc6`; a token of digits alone is still dropped and a real move is still untouched; `movetext_to_san()` returns the move and the token as written, and `build()`'s `cannot parse '<x>' at ply N` and `cannot play '<x>' at ply N` quote the token as written, so a glued `2.Qxf7` reports `2.Qxf7` and not `Qxf7`; new properties in `tests/test_make_book_tools.sh` cover both halves, observed red first and green after, with `1 . e4 e5 2 . Nf3 Nc6 *` building byte-identically to `1. e4 e5 2. Nf3 Nc6 *`; the shipped `src/openings.bin` rebuilt from `books/8moves_v3.pgn` is byte-identical (no `src/` change, so INV-6 is discharged by the unchanged binary); `DEV_MANUAL.md`'s book section drops the sentence saying the whitespace form is not read
touches:    tools/make_book.cpp, tests/test_make_book_tools.sh, DEV_MANUAL.md
excludes:   `algebraic_to_move()` and everything in `src/` -- the parser's own leniency on a leading non-piece character is a separate finding, reported to the owner 2026-09-07, and if it becomes a step this one does not wait for it; `pgn_to_positions`, which documents "no move numbers"; every other import-format leniency of S178's `excludes:` (`e8Q`, `0-0` with zeros, a `P` prefix)
decisions:  DEC-147
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S178 taught the splitter the glued move number indication and left two
questions in its section 10, both outside its `accepts:`. The owner answered
both on 2026-09-07 and put them in one step; DEC-147 records the ruling and why
they ride together rather than as two steps.

**One, the whitespace form.** PGN 8.2.2.1: "one or more white space characters
may appear between the digit sequence and the period(s)". `1 . e4` therefore
splits into `1`, `.` and `e4`; `1 .e4` into `1` and `.e4`; `1. ... e5` into
`1.`, `...` and `e5`. The digits half is dropped as a bare number already. The
dots half has `dot == 0`, misses both of S178's tests and reaches the parser.
Observed at `0075624`, on the tree S178 completed:

```
1 . e4 e5 *        -> game 1 cut short: cannot parse '.' at ply 0
1. e4 1. ... e5 *  -> game 1 cut short: cannot parse '...' at ply 1
1 .e4 e5 *         -> builds; see the note below, this one does not refuse
```

**Two, the message.** S178 erases the indication in place, so the token handed
to the parser is the move alone and `report_cut_short()` quotes that. A reader
fixing a PGN gets `cannot parse 'Qxf7'`, which matches many lines in a file,
where `2.Qxf7` matches one. Observed at `0075624`: `1.e4 e5 2.Qxf7 *` reports
`cannot parse 'Qxf7' at ply 2`.

## Implementation guide

### 1. The edit

Both halves are in `tools/make_book.cpp` and nothing else in the tree names
`movetext_to_san()` (`grep -rn movetext_to_san src/ tests/ tools/`).

**The dots.** In `movetext_to_san()`, after S178's digits-then-dots block, the
same rule with no digits in front: when `token[0] == '.'`, find the first
non-dot; `npos` drops the token, otherwise erase the prefix. Written as one
block with S178's, the shape is: let `dot` be the first non-digit; drop when
`dot == npos` (a bare number); then, when `dot < token.size() && token[dot] ==
'.'` -- which now admits `dot == 0` -- find the first non-dot and drop or
erase. S178 required `dot > 0` and that is the only clause that changes.

Nothing legal begins with `.`: PGN 7.3 makes the period a token by itself and
7.9 excludes it from symbol continuation characters, so a leading run of dots
is a move number indication and never part of a move.

**The message.** `movetext_to_san()` returns `std::vector<std::string>`. Give
it a two-field struct -- the move as the parser should see it, and the token as
the PGN wrote it -- and return a vector of that. Its one caller is `build()`:
the loop passes the first field to `algebraic_to_move()` and the second to the
two `report_cut_short()` calls it builds a message for. About fifteen lines.
Set the second field at the single `push_back` site, before the erase.

### 2. Traps

- **The remainder is still not re-classified.** `.2` reaches the parser as `2`
  and cuts the game short as `2`; `1.2` already does. Do not add a bare-number
  drop on the remainder -- silently dropping broken input is the fault class
  2026-09-03_adversarial-F02 closed and S178 refused for the same reason.
- **`1 .e4` builds today and it is not this step's doing.** `.e4` reaches
  `algebraic_to_move()`, which drops the leading `.` and reads `e4`, so the
  book is right by accident. `.Nf3` under the same path is read as a *pawn*
  move to f3 and the book is silently wrong -- that is the finding in
  `excludes:`, reported to the owner on 2026-09-07 with its evidence, and it is
  the parser's, not the splitter's. This step removes the tool's ability to
  hand the parser such a token at all, which is worth having either way; it
  does not fix the parser and must not claim to. Should the parser be fixed
  first, `1 .e4` starts refusing before this step and starts building correctly
  after it -- write the fixture expectations from the twin's actual output on
  the tree at hand, which is S174's and S178's lesson both.
- The `cannot play` message shares the token with `cannot parse`; both take the
  token as written.
- `write_pgn` in the fixture passes movetext through `printf '%s'`: no `%`.

### 3. Tests

Red first, in `tests/test_make_book_tools.sh`, beside S178's properties 7
and 8, with the header comment extended:

- **9**, the whitespace form: `1 . e4 e5 2 . Nf3 Nc6 *` and `1. e4 1. ... e5 *`
  build with `games cut short 0`, and the first is byte-identical to the
  existing `control.bin`.
- **10**, the message: a fixture whose bad token is glued, `1.e4 e5 2.Qxf7 *`,
  is refused with a report naming `2.Qxf7`. Grep for the token as written, and
  assert the bare form is *not* what is printed, or the property passes on a
  substring.

Take every expected entry count from the spaced twin's own output on the tree
at hand, never from this file.

Not owed: no pruning rule, no `make_move`, generator or search change, so
DEC-141's second tier does not apply -- no guard test, no mutant, no Debug
self-play. No SPRT and no `hyperfine`: no `src/` file changes, so the binary,
INV-6's node counts and `src/openings.bin` are unchanged by construction.
Quote the `search_bench` triples in the stamp as S174 and S178 did.

Independent oracle: `adocs/data/S175_book_conformance.py` under
`~/.venv/chess`, which reads both forms natively, should report `missing 0
extra 0 weight_mismatch 0` on the new fixtures.

### 4. Completion checklist

- Gate green in `build` and `build-tune`, `./clang-format.sh --check` clean.
  This machine needs `CLANG_FORMAT_MAJOR=22` exported (DEC-146).
- No `src/` file in the diff: DEC-140 owes neither a `Bench:` line nor
  `No functional change`. If `tools/gate.sh` (S189) exists by then, run it.
- `DEV_MANUAL.md`: S178 added a paragraph ending "Whitespace *between* the
  digits and the dots ... is not read". That sentence becomes false; replace it
  with what is read, and keep the trace to `movetext_to_san`.
- `adocs/specs.md`: the book paragraph's S178 clause covers this; propose a
  widening to the coordinator or conclude no change, and say which in the stamp.
- `MANUAL.md`: no UCI surface touched; record "checked".
- Stamp, `plan_done/`, `plan.md` and `status.md` through the coordinator,
  commit referencing S200 and DEC-147; no push.

### 5. Sources

- https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm, sections 7.3,
  7.9, 8.2.2.1 -- read for S178 on 2026-09-05, same sections.
- `adocs/plan_done/S178_movetext_glued_move_numbers.md`, its section 10 and its
  stamp; DEC-147; commit `0075624`.
