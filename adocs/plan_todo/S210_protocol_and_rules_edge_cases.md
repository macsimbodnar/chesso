id:         S210
goal:       the seven low-severity engine defects of the 2026-09-10 audit are closed as one batch -- the halfmove clock cannot wrap, a full history refuses rather than answers `bestmove 0000`, `go infinite` waits for `stop`, `movestogo 0` means sudden death, the first iteration can be stopped, quiescence scores a dead position as a draw, and the same-coloured-bishops comment tells the truth -- with F02 and F03 of the 2026-09-04 report and two `MANUAL.md` sentences folded in (DEC-181, DEC-184)
accepts:    **F17** the halfmove clock cannot wrap -- either `uint16_t` in `board_t` and `history_entry_t` with the record size cost stated, or saturation at 255 with the reason written at the increment sites -- and a red-first case plays 256 reversible plies through `position startpos moves` and asserts the fifty-move test still fires and the repetition window is still open; **F18** a `position` line whose moves would overflow `HISTORY_MAX_SIZE` is refused in the S176 shape with the board unchanged, the comment at `make_move`'s refusal in `src/bitboard.cpp` stops saying the search can never reach it, and `MANUAL.md` states the bound; **F19** `go infinite` prints `bestmove` only after `stop` whatever else the line carries: `command_go` in `src/chesso.cpp` pre-seeds `depth = MAX_DEPTH` and keeps a `nodes` budget, and `infinite` clears neither -- measured 2026-09-12 on the Release binary, `go infinite depth 1` and `go infinite nodes 1` both answer before `stop`, while `movetime` and the clock tokens already yield to `infinite` and answered after it -- so `infinite` clears every finite stop condition, red-first for `go infinite nodes 1` as well as `go infinite depth N`, and `go depth N` and `go nodes N` without `infinite` are unchanged; this also closes the still-open `2026-09-04_adversarial-F01` root-collapse case; **F20** `movestogo 0` is treated as no `movestogo` at all, as `src/uci.hpp`'s own comment and S089 state, with a red-first case asserting the allocation equals the sudden-death one; **F21** the first iteration honours `stop` and the hard timer -- `iterative_deepening_search` installs the real signal before the first `search()` -- while a move is still produced on an abort at depth 1, red-first through `tools/timer_race_stress.py` or a direct case, and the p99 depth-1 wall time is re-measured and recorded; **F22** quiescence in `src/search.cpp` returns `DRAW_SCORE` where a capture leaves `is_insufficient_material` true, and **the reach is counted before any match is booked**: over the S198 calibration PGN's positions, or a 1000-game sample, how many quiescence captures land on a dead position -- F22 lands in its own commit with `Bench: <nodes>` (it alters the tree), and a count near zero discharges its SPRT by census -- DEC-107's precedent, stated in the stamp -- while a larger one owes one `--nonreg` SPRT for that commit alone; F20 and F21 alter play only under inputs no harness here sends -- `movestogo 0`, and a hard limit that falls inside a depth-1 iteration of about a millisecond -- so each states its reachability in the stamp and is discharged node-identical at fixed depth, the other four being node-identical by construction, all in commits marked `No functional change`; **F23** the comment at `is_insufficient_material` stops justifying the same-coloured-bishops omission as "not by the laws" (FIDE 5.2.2 says otherwise) and names the accepted behaviour and its record instead; also decided here: whether the `go` tokens `std::stoll` reads in `command_go` get S209's whole-token rule, and the decision is written; every UCI-visible change is in `MANUAL.md` and `adocs/specs.md` before `test_uci_surface` is refreshed (SURFACE); Debug self-play four rounds at 4+0.04, `Assertion` grepped, before completion (DEC-141), because F22 touches the search; **2026-09-04 F02** a `moves` token that does not parse or is not legal refuses the whole `position` command in the S176 shape -- the board unchanged from before the command and one `info string refused [position moves] <token>: <why>` line on the UCI channel in both builds -- with a red-first case in `tests/test_engine.cpp` that sends a legal prefix, a bad token and a legal suffix and asserts the board is the pre-command one; **2026-09-04 F03** the root's answer and move order after an aborted iteration no longer rest on the root's table entry having survived: either the previous iteration's best move is carried in the search state and used for both, or the premise is asserted and the number of aborted iterations whose root entry is gone is counted over the F22 census sample and recorded, and the comment in `src/chesso.cpp` states what is enforced; **DEC-184** `MANUAL.md` gains one sentence saying what `ucinewgame` resets, traced to `reset_for_new_game` in `src/chesso.cpp`, and one saying that `nodes` on the last `info` line can be under the `go nodes` budget when the final iteration aborts before it has a line (S195 measured 2917 of 5000)
touches:    src/data_structures.hpp, src/bitboard.cpp, src/chesso.cpp, src/search.cpp, src/uci.hpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp (golden), MANUAL.md, adocs/specs.md, adocs/data/
excludes:   any change to the draw rules' semantics beyond the sites named -- the repetition rule is S207; `make_null_move`'s dead assert at a full history, unreachable while null move pruning forbids two passes in a row, which is recorded here and not repaired; the mate-carry and mate-PV instruments
decisions:  DEC-170, DEC-171, DEC-181, DEC-184
closes:     2026-09-10_adversarial-F17, 2026-09-10_adversarial-F18, 2026-09-10_adversarial-F19, 2026-09-10_adversarial-F20, 2026-09-10_adversarial-F21, 2026-09-10_adversarial-F22, 2026-09-10_adversarial-F23, 2026-09-04_adversarial-F01, 2026-09-04_adversarial-F02, 2026-09-04_adversarial-F03
blocks:
paused_by:
author:
done:

## Why this exists, and why it sits behind the strength steps

Seven low findings of `2026-09-10_adversarial`, Part E, F17 to F23, each
verified by its reviewer. None fires in an adjudicated match here and none
moves a reported score in ordinary play, which is why DEC-171 schedules them
behind the next strength step rather than first: the BUGS rule's "before
anything else starts" is scoped by reach, on the owner's instruction of
2026-09-11. They are still bugs and they are still fixed -- as one batch, in
the daytime between two night runs.

- **F17** `halfmove_clock` is a `uint8_t` and wraps at 256; `load_FEN` refuses
  a clock above 255 with a comment naming exactly the harm the increment then
  produces. Both consumers fail past the wrap: the `>= 100` draw test stops
  firing and the repetition window collapses to `min(0, size)`.
- **F18** at 4999 plies the root's own `make_move` refuses, every move is
  skipped, `first_legal_move()` cannot rescue it because it calls `make_move`
  too, and the engine answers `bestmove 0000` with 22 legal moves on the board.
- **F19** `command_go` pre-seeds `depth = MAX_DEPTH` and `infinite` does not
  clear it, so `go infinite depth N` returns without `stop`, against
  `UCI.txt`'s "Do not exit the search without being told so in this mode!";
  the same mechanism is `2026-09-04_adversarial-F01`'s root-collapse case.
- **F20** `movestogo 0` is clamped to 1, spending 46 % of the clock on one move
  (4.64 s of 10 s measured), where `src/uci.hpp` says 0 means sudden death.
- **F21** `state.stop = &never_stop` for the first iteration, so depth 1 ignores
  `stop` and the hard timer; median 0.56 ms and p99 1.04 ms over 400 corpus
  positions, 254 ms on a pathological `STATUS_VALID` board against a 100 ms
  clock -- a forfeit, latent.
- **F22** `is_insufficient_material` has one call site, in `negamax_at`; a
  capture inside quiescence that leaves KvK, KNvK or KBvK scores -103, +190,
  +235 instead of 0. The interior node catches it one ply later, which is why
  the census comes before the match.
- **F23** the comment at `is_insufficient_material` says same-coloured bishops
  are "drawn in practice but not by the laws"; FIDE 5.2.2 and 6.9 say the
  family is dead. The behaviour stays as `2026-08-14_test_review-F05`
  accepted and S067 pinned; the justification is what is wrong.

## Cost

Agent work, half a day with the tests; the F22 census is a script over a PGN
already on disk. One `--nonreg` SPRT only if the census says the changed class
is reached -- state the count and the decision before booking it (DEC-143).

## Amended 2026-09-12, DEC-197: F19 is wider than its depth reproducer

The 2026-09-12 audit (`adocs/audit/2026-09-12_adversarial.md`, prior-finding
reassessment) re-triggered F19 with `go infinite nodes 1`: `bestmove a2a3`
before `stop`. The coordinator re-ran it and its neighbours the same day on
the Release binary built from `ecdfadb`'s source: `depth 1` and `nodes 1`
answer before `stop`; `movetime 100` and `wtime 50 btime 50` answer after it,
because `command_go`'s infinite branch already takes precedence over both
(`mate` is not parsed as a limit at all). The accepts' F19 clause names all
four so the fix is not scoped to the one token each report happened to use.
