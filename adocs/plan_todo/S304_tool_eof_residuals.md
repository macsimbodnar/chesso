id:         S304
goal:       the residuals S303 found in `tools/spsa_driver.py` and `tools/depth_vs_eval.py` are closed: `check` always prints its failure marker, no malformed engine line passes or dies unnamed, and no engine outlives the tool
accepts:    `spsa_driver.py check` prints `SPSA-FAILED` as its last line on a `ConfigError`, and exits non-zero, shown on a planted stand-in; `depth_vs_eval.py`'s `best_move()` refuses a `bestmove` line with no move or a move cut short, exiting non-zero with a message naming the wait, shown on a planted stand-in with both references; `probe_nodes()` turns `info ... nodes abc` into a `ConfigError` with the process killed; an engine that never reads stdin does not outlive `depth_vs_eval.py`; `tools/spsa_s085.json` either passes `check` on the current tune build or is stated, with the reason, as the record of a finished run, and `DEV_MANUAL.md`'s sentence quoting it says so; each tool's output on a healthy real binary unchanged (nodes and moves equal, timings stripped); no timing is taken
touches:    tools/spsa_driver.py, tools/depth_vs_eval.py, tools/spsa_s085.json, tests/test_spsa_probe.py, tests/test_spsa_driver.py, DEV_MANUAL.md
excludes:   any change to a tool's protocol or workload on a healthy engine; the frozen scripts under `adocs/data/`; `tools/error_profile.py`'s `Engine` beyond what stops `depth_vs_eval.py`'s engine outliving it; any timing run
decisions:  DEC-171, DEC-061
closes:
blocks:
paused_by:
author:
done:

## Why

S303's subagent and its fast check found these while closing the end-of-file
returns (`adocs/plan_done/S303_tool_eof_returns.md`, "Found, not fixed"). Each
was observed on a planted stand-in there, old and new alike, so none is a
regression of S303.

- **`check` ends in a traceback with no marker.** `main()`'s `check` branch is
  outside any `try`, so a `ConfigError` exits 1 without `SPSA-FAILED`. Nothing
  hangs today, because the four wrappers test the exit status, but it is a gap
  against DEC-061's terminal marker.
- **A cut `bestmove`.** `bestmove e3`, cut mid-move, then the engine exits: the
  tool exits 0 with `deep_move` `e3` on the planted reference and on chesso
  (`deep_cost` -46), and exits 1 with no row on Stockfish. A bare `bestmove` at
  end of file raises `IndexError` with no message. It needs a line cut in the
  middle of a write.
- **`info ... nodes abc`** raises a bare `ValueError` without the kill. No cut of
  a number produces it; only a malformed engine does.
- **An engine that never reads stdin outlives `depth_vs_eval.py`**, whose workers
  do not kill it. `error_profile.py`'s `Engine` has no kill either (read, not
  run).
- **`tools/spsa_s085.json` fails `check` on the current tune build**: `RfpMinPly`
  config `[0, 63]` against the binary's `[3, 63]`. It may be the history of a
  finished run; `DEV_MANUAL.md` still quotes its 0.7 s.

None is reachable in ordinary play or on the UCI surface, and none moves the
engine's own score, move or line (DEC-171). Filler, behind the next strength
step. Filed by the coordinator, 2026-10-31.
