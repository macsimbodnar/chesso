id:         S223
goal:       the load boundary requires exactly one king a side and refuses a position whose side not to move is in check, so no accepted `position` line can leave a board without a king or let a supplied move capture one
accepts:    `load_FEN()` in `src/bitboard.cpp` refuses two further classes after S208's two, each with its own reason through the `reason` out-parameter S208 added: a placement with other than one king of either colour, and a placement in which the side **not** to move is in check -- the engine's own attack test from that king's square, which also makes two adjacent kings a refusal -- and `position fen` reports each in the S176 shape with the board, the history and every move applied since the last `position` left as they were; **red first** in `tests/test_audit_fen_semantics.cpp` on the audit's `7k/8/8/8/8/8/8/K6R w - - 0 1` (observed before the fix: `moves h1h8` accepted and the `fen` command answering `7R/8/8/8/8/8/8/K7 b - - 0 1`, the black king gone; after: one `info string refused` line and the prior FEN echoed back), on the kingless `8/3p4/8/8/8/8/3P4/8 w - - 0 1` and on a two-white-king board, with controls that the start position and a position whose side **to** move is in check still load; the refusal agrees with python-chess 1.11.2 `Board.status()` -- `STATUS_NO_WHITE_KING`, `STATUS_NO_BLACK_KING`, `STATUS_TOO_MANY_KINGS`, `STATUS_OPPOSITE_CHECK` -- over every FEN literal in `tests/`, `src/` and `tools/` (3204 on 2026-09-12 by `adocs/data/S223_fen_census.py`, re-run at completion), 0 disagreements on the positions S208's classes do not refuse first; the seven positions the census names below are **re-picked to legal ones with the oracle's word and none deleted**, each re-pick keeping the property its case asserts and any golden derived from it re-derived by its script (DEC-142); the S208 control "the kingless debug positions still load" and the two "survivable" cases in `tests/test_search.cpp` are **re-stated as refusals at load** -- a re-statement and not a relaxation, their premise being a board the boundary no longer admits, recorded in DEC-197; `make_move()` gains a Debug assertion that each side still has exactly one king after the move, exercised by DEC-141's four-round Debug self-play with `Assertion` grepped, so "no accepted input can remove a king" rests on the load bound, legal-only generation (INV-1) and that assertion rather than on a universal test; the no-king branches in `is_check()`, `generate_moves_impl()` and `king_shelter_features()` become Debug assertions, with `bench_movegen` read before and after and its resolution stated; the `empty` shortcut leaves `position`, `EMPTY_POS` leaves `src/data_structures.hpp`, its row leaves `MANUAL.md`, and `test_uci_surface`'s token list is refreshed **after** `MANUAL.md` and `adocs/specs.md` describe the two refusals and the removed shortcut (SURFACE); `adocs/specs.md`'s position-input row names the two classes beside S208's and stops saying legality at large is unchecked in those two respects; INV-6 discharged on identical `tools/search_bench.py` node counts and best moves at two depths and an identical `bench` total, the commit footer `No functional change`; the fast suite green in both builds and `tools/gate_extra.sh` run, because `make_move` and the generator are touched
touches:    src/bitboard.cpp, src/chesso.cpp, src/data_structures.hpp, src/evaluation.cpp, tests/test_audit_fen_semantics.cpp, tests/test_search.cpp, tests/test_chesso.cpp, tests/test_engine.cpp, tests/test_movegen.cpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp, tests/test_uci_surface.cpp (golden), MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/
excludes:   any change to move generation or make/unmake beyond the assertion; pawn counts below the 16-a-side bound, promoted-piece plausibility, checker-count plausibility (python-chess's `TOO_MANY_CHECKERS` and `IMPOSSIBLE_CHECK`) and reachability at large -- the engine plays those correctly and no consumer indexes on them; a permissive internal loader beside the public one, which DEC-177 rejected in as many words; a run-time bound inside the hot loop
decisions:  DEC-171, DEC-177, DEC-197
closes:     2026-09-12_adversarial-F01
blocks:
paused_by:
author:
done:

## Why this exists, and where it sits

`2026-09-12_adversarial-F01`, the owner's Codex audit against `98af071`.
`position fen 7k/8/8/8/8/8/8/K6R w - - 0 1 moves h1h8` is accepted and the
`fen` command then answers `7R/8/8/8/8/8/8/K7 b - - 0 1`: the black king has
been captured. Reproduced by the coordinator the same day on the Release
binary. The position is one no game reaches -- python-chess reports
`STATUS_OPPOSITE_CHECK` -- and S161 and S208 both left it admitted on purpose:
"legality at large is deliberately unchecked; one king a side is not
required, because `EMPTY_POS` and two survivable cases are kingless on
purpose" (`src/bitboard.cpp`, section 6a; S208's `excludes:`).

**Why it is not the fix-first bug the report calls it.** DEC-171 scopes BUGS
by reach: a defect is fixed before the next strength step when it is
reachable in ordinary play, on the UCI surface as GUIs and harnesses drive
it, or able to move a reported score. This one needs an illegal position;
no GUI, book, harness, corpus tool or `datagen` sends one; no memory is
corrupted -- the bitboards and the square array agree after the capture,
which is exactly what S067's two "survivable" cases pin, unlike F09's stack
overrun and F10's out-of-bounds write; and no legal position's score, move
or line changes. So it is filler behind S109, beside S210, closed by the
block boundary it sits in and named in the pre-registration of every run
taken while it is open. DEC-197.

**Why it is still worth doing.** It is the class S208 closed, one boundary
further, and it is free where S208's was free: a check at load time and
nothing in the hot loop. Three branches exist today only for the state it
removes -- `is_check()` returns false on an empty king bitboard,
`generate_moves_impl()` carries an `INVALID_INDEX` king square, and
`king_shelter_features()` returns early -- and each is a hot-path branch that
becomes an assertion. Two planned steps index by the king's square with no
guard of their own: S133's king-relative tables and S029's network features.
Closing the boundary now means neither has to learn about kingless boards.
And the prime directive's board-state clause reads the way the report reads
it once the position is refused rather than survived.

## What the census found, 2026-09-12

`adocs/data/S223_fen_census.py` extracts every FEN literal from `tests/`
(excluding the vendored `tests/json/`), `src/` and `tools/` and asks
python-chess 1.11.2 for `Board.status()`: 3204 unique FENs, 3188 pass the
king and opposite-check gate, 16 do not (`adocs/data/S223_fen_census.txt`).
The sixteen, by what this step does with them:

| class | positions | what happens |
|---|---|---|
| already refused by S208 before any king check | the F09 27-queen board; `pppppppp/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1` in `tests/test_movegen.cpp` (a pawn on rank 8) | nothing; the piece-count and back-rank reasons must keep firing first, their tests pin the reason text |
| syntax fixtures the parser already refuses | the nine-piece rank and the seven-piece rank in `tests/test_movegen.cpp` | nothing |
| become refusal cases | `EMPTY_POS`; `8/3p4/8/8/8/8/3P4/8 w - - 0 1`; the audit's `7k/8/8/8/8/8/8/K6R w - - 0 1` | the S208 control and the two survivable cases in `tests/test_search.cpp` re-stated as refusals at load |
| a comment, never loaded | `2k5/8/8/8/8/8/1q6/K1R5 w - - 0 1` in `tests/test_search.cpp`, the S067 anecdote | nothing |
| re-picked, seven FENs over nine sites | `4k3/8/8/8/8/8/8/R6R w KQ - 0 1` ("a right with no king anywhere is cleared"): the premise is gone, the case re-states as a refusal and the off-square-king case beside it keeps the clearing property; `4k3/8/4R3/3Pp3/8/8/8/4K3 w - e6 0 1` ("an occupied ep target square goes"): the rook on e6 checks the king not to move, so the occupant is re-picked to a piece that does not; `3k4/8/7p/Q1p3pP/1pPpPpP1/1P1PpP2/N7/2K5 w - - 0 1` and `3k4/8/7p/2p3pP/1pPpPpPQ/1P1PpP2/N7/2K4r b - - 0 1` in `tests/test_chesso.cpp` ("Test is in check", "Test is_attacking_king"): both have the side not to move in check, re-picked with `is_check()` and `gives_check()` from the oracle as the expected values; `4r3/8/8/8/4N3/8/8/4K3 w - - 0 1` in `tests/test_engine.cpp` (two sites) and `tests/test_movegen.cpp` (the pinned knight): no black king, one added off the ray and out of every white attack, one FEN for all three sites; `8/PPPPPPPP/8/2k5/2K5/8/pppppppp/8 w - - 0 1` in `tests/test_eval_positions.hpp` (the promotion-phase anchor): adjacent kings, re-picked with the kings apart and the phase sum still above a full board, which is what it is there for; `4k3/8/8/8/8/8/8/8 w - - 0 1` and `8/8/8/8/8/8/8/4K3 w - - 0 1` in `tests/test_evaluation.cpp` (a king is worth nothing to the material accumulator): lone kings, re-picked as the two-king board, which asserts the same thing about both kings at once | each with the oracle's `Status.VALID` quoted at the site |

## Hazards

- **Order.** S208's two refusals come first and keep their reasons; two of
  their tests pin the text ("27 white"). The king count comes before the
  check test: an attack test from a lone king's square on an empty bitboard
  reads square 64, the very read the survivable cases were written for.
- **The eval anchor feeds two test binaries.** `test_eval_positions.hpp` is
  read by `test_eval_model` and `test_tuner_gradient`; a golden that any of
  them derives from the list is re-derived by its script, never re-read
  (DEC-142).
- **`test_chesso.cpp`'s expected booleans come from the oracle**, not from
  the old table. `python-chess` `Board.is_check()` and `gives_check(move)`.
- **No `Bench:` line is expected.** Every bench position is legal since
  DEC-177. If the total moves, stop and find out why before committing.
- **`gate_extra.sh` binds.** `make_move` gains an assertion and the generator
  loses a branch; DEC-141's Debug self-play and the sanitizer build run
  before completion.

## Cost

Agent work, half a day to a day with the census re-run and the re-picks; no
match. Filler behind S109 under DEC-171; `2026-09-12_adversarial-F01` is
named in every pre-registration while this step is open.
