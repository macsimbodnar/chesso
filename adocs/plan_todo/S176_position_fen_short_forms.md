id:         S176
goal:       `position fen` accepts the four- and five-field forms, and a FEN that fails to load leaves the engine on the position it had, moves included
accepts:    `position fen <4 fields>` and `position fen <5 fields>`, with and without a trailing `moves` list, load with the missing clocks defaulted to `0 1`, and `fen` afterwards prints the six-field form; `position fen <6 fields> [moves ...]` behaves exactly as today; fewer than four fields is still refused; a FEN that `load_FEN()` rejects leaves the board, the history and every move applied since the last `position` command exactly as they were -- reproduced red with the finding's sequence (`position startpos moves e2e4`, then `position fen 8/8/8/4k3/8/4K3/8/8 w - - moves e3d3`, then `fen` prints the **start** position today) and green after (prints the position after `e2e4`); every refusal is reported as one `info string` line on the UCI channel in every build (the S137 pattern -- `LOG_W` is silent in the shipped binary), its text recorded in `MANUAL.md`; `tests/test_engine.cpp:602-616` ("a malformed fen leaves the previous position alone") is extended with a move applied before the malformed FEN, and new cases cover the four- and five-field forms with and without moves; `adocs/specs.md` and `MANUAL.md:192` describe the accepted forms and the refusal before `test_uci_surface` is refreshed (SURFACE); `tools/search_bench.py` node counts and best moves identical at depths 9 and 12 (INV-6 -- no search path changes)
touches:    src/chesso.cpp, tests/test_engine.cpp, tests/test_uci_surface (golden), MANUAL.md, adocs/specs.md
excludes:   semantic validation of the FEN beyond what S161 does at `load_FEN()`; accepting fewer than four fields; `ucinewgame` semantics; the `fen` debug command's output format
decisions:
closes:     2026-09-03_adversarial-F04
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-03_adversarial-F04`, low. Two defects in `command_position`, both
leaving the engine on a position the GUI did not send:

- `src/chesso.cpp:1328-1333` returns when fewer than six tokens follow `fen`,
  with nothing on the UCI channel, and the board stays wherever the previous
  command left it. The FEN standard's optional-field form -- clocks omitted --
  is accepted by Stockfish, cutechess and python-chess, and `MANUAL.md:192`
  documents only the six-field form.
- `src/chesso.cpp:390-398`, on a FEN `load_FEN()` rejects, reloads
  `initial_position` -- the last **FEN**, not the last **position** -- so the
  moves applied since are gone. In the finding's reproduction the tokens
  `moves e3d3` were read as the clock fields, the load failed, and the engine
  was on the start position after having been sent `e2e4`.

`tests/test_engine.cpp:602-616` covers only the case with no moves applied,
where the FEN and the position coincide, which is why it is green.

## Impact

Low. fastchess and cutechess send six fields and no match here has seen either
path. A GUI or tool that sends the short form gets a stale board and an illegal
move in its game; the reset case turns one malformed FEN into a guaranteed
illegal move where keeping the previous position would have kept the game.
