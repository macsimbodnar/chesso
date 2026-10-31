id:         S304
goal:       the residuals S303 found in `tools/spsa_driver.py` and `tools/depth_vs_eval.py` are closed: `check` always prints its failure marker, no malformed engine line passes or dies unnamed, and no engine outlives the tool
accepts:    `spsa_driver.py check` prints `SPSA-FAILED` as its last line on a `ConfigError`, and exits non-zero, shown on a planted stand-in; `depth_vs_eval.py`'s `best_move()` refuses a `bestmove` line with no move or a move cut short, exiting non-zero with a message naming the wait, shown on a planted stand-in with both references; `probe_nodes()` turns `info ... nodes abc` into a `ConfigError` with the process killed; an engine that never reads stdin does not outlive `depth_vs_eval.py`; `tools/spsa_s085.json` either passes `check` on the current tune build or is stated, with the reason, as the record of a finished run, and `DEV_MANUAL.md`'s sentence quoting it says so; each tool's output on a healthy real binary unchanged (nodes and moves equal, timings stripped); no timing is taken
touches:    tools/spsa_driver.py, tools/depth_vs_eval.py, tools/error_profile.py, tests/test_spsa_probe.py, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   any change to a tool's protocol or workload on a healthy engine; the frozen scripts under `adocs/data/`; `tools/error_profile.py`'s `Engine` beyond what stops `depth_vs_eval.py`'s engine outliving it; any timing run
decisions:  DEC-171, DEC-061
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-31, `check` ends in `SPSA-FAILED` on every exception, a cut or bare `bestmove` and `nodes abc` stop naming the wait, no engine outlives `depth_vs_eval.py`, a healthy engine still ends on `quit`; the real-binary comparison `failures=0`; both fast suites 50 of 50, format clean; fast-checked, its four and the ruling fixed; tools only, tier two does not bind

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

## 2026-10-31: the fix and its evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s304`, branch `s304`, from `9bcafb9`, beside the coordinator's
S125 SPRT on all twelve cores. Nothing compiled, no `ctest`, no gate, no engine, no
Stockfish, no timing. Only the two Python test files and planted `/bin/sh` stand-ins
ran, each short, under `nice -n 19`, and every wedged stand-in is an `exec sleep`
with a bound. The stand-ins and the old tools (`git show 9bcafb9:...`) live in the
session scratchpad. **The accepts' real-binary comparison is pending**: it runs after
the match, from the script below. Seconds quoted here are a stand-in failing on a
loaded machine, not a measurement of anything. The fast check's four and its ruling
are folded in below, each reproduced first, with their own section.

`touches:` is corrected above against what the step did. `tools/error_profile.py`,
inside the excludes' allowance, because a constructor that raises leaves nothing able
to kill its process. `tests/CMakeLists.txt`, one comment, because its "12 cases, ~1 s"
became false. `tools/spsa_s085.json` and `tests/test_spsa_driver.py` did not change;
the second is green and has no case that probes an engine.

### What changed

- `tools/spsa_driver.py`, `main()`: the whole `check` branch runs under `except
  Exception`, prints `ERROR: <type>: <message>` and `SPSA-FAILED`, returns 1, and the
  config load before it takes every exception too. Every exception and not only
  `ConfigError`, because the goal says always and an engine also fails by `OSError`
  (gone from under a write, not there), `TimeoutExpired` and `UnicodeDecodeError`, and
  a config by `AttributeError`, `TypeError`, `OverflowError` and `RecursionError`.
  Nothing is swallowed: it exits 1 and names the fault. What is lost against the
  traceback is a bug's line number.
- `tools/spsa_driver.py`, `probe_nodes()`: a count that is not ASCII digits raises
  `ConfigError: <engine> printed a node count that is no number under `<go>`: <line>`.
  Digits and not `int()`, which also takes `-5` and `1_000`, and the old tool returned
  both as numbers. S303's kill at the end-of-file branch moved to one `except
  Exception` over the whole exchange, so every raise kills, the malformed count, the
  decode error and a dead pipe alike.
- `tools/depth_vs_eval.py`, `best_move()`: the `bestmove` line must match whole,
  newline included, `bestmove <from><to>[nbrq]` and anything after a space, or it
  raises `RuntimeError: waiting for bestmove, engine sent <line>, which holds no whole
  move`. The newline is what catches the cut no pattern can: only end of file returns
  a line without one, and `bestmove e7e8` reads as a move when it is `e7e8q` cut short.
  `(none)` and `0000` are refused too: this function only reads positions a game
  played a move from, so "no move" is a fault there, and a reference given `moves
  (none)` or `moves 0000` is what S303 traced. `Engine.evaluate()`'s `bestmove (none)`
  after a mate is another path and is untouched.
- `tools/depth_vs_eval.py`, the workers: a `multiprocessing.util.Finalize` closes both
  engines however a worker returns: `quit`, a wait of `QUIT_WAIT_S`, 5 s, and a kill
  only for an engine still there or one whose stdin is gone. `atexit` does not run:
  Python 3.12's forked worker leaves by `os._exit` (`multiprocessing/popen_fork.py`,
  read). Engines open in the first task (`_open_engines()`) and not in the pool's
  initializer, which is a stated departure, below.
- `tools/error_profile.py`, `Engine.__init__`: on a raise it kills what it started.
  Without this the accepts' literal case stays, an engine that never reads at all.

**Why the first task and not the initializer.** An initializer that raises breaks the
pool, the pool sends the other workers SIGTERM, and a signal ends a worker without its
Finalize. My first form kept the initializer and handled the signal: a handler that
raises while a constructor waits and one that kills and `os._exit`s after, because a
worker turns a `SystemExit` under a task into that task's result and waits on, and the
pool waits for the worker. It left one survivor in six on a stand-in built to hit it,
most likely the signal landing inside `Popen`, where nothing holds the child yet; that
cause is reasoned and not shown. Opening in a task
needs no signal handling, so the failure ends the ordinary way, and the last line of
the failure names the wait (`before uciok`) where it read `BrokenProcessPool`. Its
price is in "Found, not fixed", second item.

### Three stated departures from the excludes' letter

The excludes say no change to a tool's protocol or workload on a healthy engine. Three
things change on one, all accepted by the coordinator's ruling on the fast check:

- **The engine count.** A worker with no task now opens no engine. `--sample 1 --workers
  3` starts 2 processes where it started 6, observed on stand-ins, 3 and 3 against 1 and
  1. The reason is the broken pool's SIGTERM, above. Rows are unaffected: `best_move()`
  and `cost_of()` each call `new_game()` before every search, so no row depends on what
  its worker's engine did before. Section 1b tests that claim on the real binaries.
- **A healthy sibling beside a failed one finishes its in-flight probe.** An engine dead
  at birth used to break the pool, and SIGTERM cut the sibling's search at once. It now
  fails its own task, and the tool's exit waits for what the others have in flight, up
  to the deep search's node budget. A failure inside a task already behaved this way.
- **How a healthy engine ends.** It used to end on end of file when its worker left. It
  now gets `quit`. In chesso both reach `uci_shutdown()` and return from `main`, read in
  `src/main.cpp`, so destructors run either way, and section 6 tests it on the census.

Per engine that opens, the commands are the same in the same order, with `quit` added at
the end. Old against new on a healthy stand-in, one position and six on three workers,
both planted references: `--out` and stdout identical by `cmp`.

### `check`'s marker: before and after

`spsa_driver.py check`, a one-axis config, end to end:

| engine | old (`9bcafb9`) | new |
|---|---|---|
| closes its output after an `info` line, reads on | exit 1, empty stdout, `ConfigError` traceback | exit 1, `ERROR: ConfigError: ... closed its output before bestmove under `go depth 9``, `SPSA-FAILED` last, empty stderr |
| exits there | the same | the same |
| `info depth 1 nodes abc` | exit 1, empty stdout, `ValueError` traceback | exit 1, `ERROR: ConfigError: ... printed a node count that is no number ...: info depth 1 nodes abc`, `SPSA-FAILED` last |
| answers `bestmove` with no count | exit 1, empty stdout, traceback | exit 1, `ERROR: ConfigError: ... reported no node count ...`, `SPSA-FAILED` last |
| `/bin/true` | exit 1, empty stdout, traceback | exit 1, `ERROR: ConfigError: /bin/true printed no spin options; ...`, `SPSA-FAILED` last |
| `/nonexistent` | exit 1, empty stdout, `FileNotFoundError` traceback | exit 1, `ERROR: FileNotFoundError: ...`, `SPSA-FAILED` last |
| closes its output, never answers `uci` | not run | exit 1 after `read_engine_options`' 30 s, `ERROR: TimeoutExpired: ...`, `SPSA-FAILED` last |
| an undecodable byte in a count | exit 1, empty stdout, `UnicodeDecodeError` traceback | exit 1, `ERROR: UnicodeDecodeError: ...`, `SPSA-FAILED` last |

| config `[]`, fast check | exit 1, empty stdout, `AttributeError` traceback | exit 1, `ERROR: <config>: 'list' object has no attribute 'get'`, `SPSA-FAILED` last |
| `"iterations": null`, `"start": null`, `"params": 5`, `"params": [5]`, fast check | exit 1, empty stdout, `TypeError` traceback | exit 1, `ERROR: <config>: ...`, `SPSA-FAILED` last |
| `"iterations": 1e400`; `[` nested 200000 deep | exit 1, empty stdout, `OverflowError`; `RecursionError` | the same |
| `"iterations": 1` and 400 zeros, which loads and fails in `warnings()` | exit 1, empty stdout, `OverflowError` | exit 1, `ERROR: OverflowError: ...`, `SPSA-FAILED` last |
| a count too long to print | not run | exit 1, `ERROR: ValueError: ...`, `SPSA-FAILED` last |

A dead axis's `PROBLEM` path printed its marker before and prints the same now, old
and new identical on a stand-in. A live axis ends in `SPSA-DONE`, exit 0 (new test).

**What is then true of `check`.** Every way out of the command prints a marker last,
with three exceptions, none of them a run that started. A bad command line is argparse's
exit 2 with usage on stderr, before a config exists: `spsa_driver.py check` with no
arguments, observed. A signal prints nothing: `KeyboardInterrupt` is not an `Exception`,
and SIGTERM and SIGKILL give Python no chance. The watcher's process-death exit and the
wrappers' status test cover those. And a stdout that cannot be written has nowhere to put
a marker. `run` is not covered, "Found, not fixed". The load is shared, so a config of a
wrong shape now ends `run` in the marker too, a by-product and not a claim about `run`:
`run` on `[]`, exit 1, `SPSA-FAILED` last, observed.

### A `bestmove` with no whole move: before and after

`depth_vs_eval.py`, one position, one worker, the first `go` answered whole and the
line below at the second, the deep search. Two planted references, because the real
ones wait for the match: `ref_answers` answers whatever it is handed, chesso's shape
in S303's table, and `ref_stops` prints `Illegal move` and exits on a move of the
wrong shape, Stockfish's. Both logged what they took.

| line at the deep `go` | old, `ref_answers` | old, `ref_stops` | new, on both |
|---|---|---|---|
| `bestmove e3`, no newline, exit | exit 0, a row with `deep_move` `e3`, handed `moves e3` | exit 1, handed `moves e3`, `... before bestmove`, which is the reference's wait and names the wrong process | exit 1, the message below, no `--out` file, reference handed nothing |
| `bestmove`, no newline, exit | exit 1, `IndexError`, no message | the same | the same |
| `bestmove\n`, exit | exit 1, `IndexError`, no message | the same | the same |
| `bestmove e3\n`, reads on | exit 0, a row with `e3` | exit 1, the reference's wait | the same |
| `bestmove (none)\n`, reads on | exit 0, a row with `(none)` | exit 1, the reference's wait | the same |
| `bestmove e7e8`, no newline, exit | exit 0, a row with `e7e8` | **exit 0**, a row with `e7e8` | the same |
| `bestmove d2d4`, no newline, exit | exit 0, a row with `d2d4` | **exit 0**, a row with `d2d4` | the same |

The message: `RuntimeError: waiting for bestmove, engine sent 'bestmove e3', which
holds no whole move`. The stand-in that closes with no `bestmove` at all keeps S303's
`engine closed its output before bestmove`.

The sweep, `best_move()` called directly: every prefix of `bestmove d2d4 ponder d7d5`
and of `bestmove e7e8q`, with and without a newline, on `exit` and on `close`, 164
cases.

| | old | new |
|---|---|---|
| returned a move | 84, among them `d`, `d2`, `d2d`, `e`, `e7`, `e7e` | 30, every one with its newline: `d2d4` 26, `e7e8` 2, `e7e8q` 2 |
| `waiting for bestmove ...` | - | 70 |
| `closed its output before bestmove` | 64 | 64 |
| `IndexError` | 16 | 0 |

Its limit: only the form is checked. `bestmove e7e8\n` with its newline is not a cut
and passes, and a well-formed illegal move goes to the reference as before. See "Found,
not fixed", fourth item.

### `info ... nodes abc`: before and after

`probe_nodes()` called directly, a stand-in that prints the line and stays alive by
`exec sleep 8`:

| | old | new |
|---|---|---|
| `nodes abc` | bare `ValueError`, no kill: it sat 8.1 s, the stand-in's own bound, in the 30 s wait | `ConfigError` naming the line and `go depth 9`, 0.0 s, the engine gone |
| `nodes 12x4` | bare `ValueError` | the same `ConfigError` |
| `nodes -5` | **returned -5** | the same `ConfigError` |
| `nodes 1_000` | **returned 1000** | the same `ConfigError` |

### An engine outliving the tool: before and after

`depth_vs_eval.py`, then every pid the stand-ins logged, read with `ps`:

| stand-in | old | new |
|---|---|---|
| closes its output and stops reading at the deep `go` | exit 1, engine **alive** | exit 1, gone |
| answers whole and stops reading, the run succeeds | exit 0, engine **alive** | exit 0, the same row, gone |
| never reads, output closed at birth | exit 1, `BrokenProcessPool`, engine **alive** | exit 1, `... before uciok`, gone |
| reference answers `uci`, closes, stops reading | exit 1, reference **alive** | exit 1, gone |
| reference never reads, output closed at birth | not run | exit 1, `... before uciok`, gone, the engine beside it gone |
| three workers, the first's engine dies at birth, the others answer `uci` and stop reading | exit 1, one **alive** | 8 runs, exit 1, none alive |
| three workers, the first's engine dies at birth, the others silent with their output open | exit 1, 3 runs, two **alive** each | 8 runs, exit 1, none alive, after the silent engines' own bound (below) |
| six positions, three workers, each engine wedged at its second `go` | not run | exit 1, 0.2 s, all six gone |

The bound's cost and how an engine ends, a stand-in with an `EXIT` trap for its exit-time
work, which SIGKILL does not run:

| | old | the form the fast check read, a kill | now |
|---|---|---|---|
| healthy, the run succeeds, engine and reference | `saw-eof`, exit-time work | nothing logged | `saw-quit`, exit-time work |
| `--sample 1 --workers 3` | 3 and 3 started, each `saw-eof`, exit-time work | not run | 1 and 1 started, each `saw-quit`, exit-time work |
| six positions, three workers | not run | not run | 2 and 2 started on that run, each `saw-quit`, exit-time work |
| a reference beside a cut `bestmove` | not run | not run | exit 1, reference `saw-quit`, exit-time work |

The tool's exit carries the bound only for an engine that keeps its stdin and does not
read it: 5.2 s on the first, second and fourth rows of the table above, 5.7 s on the last.
A constructor's failure kills at once, 0.2 s and 0.3 s. An engine that closed its stdin
as well has nowhere to take `quit`, the write fails and it is killed at once, 0.3 s.

### `tools/spsa_s085.json`: the record of a finished run

The accepts' second branch, and the file does not change. The evidence, none of it from
the tune binary:

- `git log` holds one commit for the file, `2e8069c`, 2026-08-20, "Freeze S085's SPSA
  run configuration", never edited since. S085's stamp: 60000 games, H1, closed.
- DEC-200 rules it: "the record of what S085 ran, not a runnable config", "every new
  run writes its own config and passes `check` on the day", and rejects editing its
  bounds because "it would falsify the record of a completed run's inputs". S142, S251
  and S039 each left it alone for that reason.
- `src/search_params.hpp`: `X(RFP_MIN_PLY, "RfpMinPly", 3, 3, 63)`, the 3 from S251.
  Read against the table, `RfpMinPly` `[0, 63]` is the one axis of the twelve whose
  bounds differ. S303 saw the refusal on the tune build: `RfpMinPly: config bounds [0,
  63] against the binary's [3, 63]`, exit 1. The comparison below repeats it.

`DEV_MANUAL.md`'s sentence quoting 0.7 s stays true and takes a clause saying so.

### Callers and wrappers

- `best_move()`: `probe()` only. `probe_nodes()`: `bound_repeats()` and `probe_axis()`
  only, reached from `check()`, which catches nothing.
- `check()`: `main()`, and `tests/test_spsa_probe.py` directly. `main()`: the command
  line, and the test file. `adocs/data/S261_apply_fit.py` uses `Config` alone,
  `S261_split_sim.py` drives `run --sim`, `S231_apply_fit.py` names the driver in a
  docstring, and `S125_sprt.sh` names this step in a comment.
- The four wrappers, `adocs/data/S098_v1_spsa.sh`, `S222_spsa.sh`, `S231_spsa.sh`,
  `S261_spsa.sh`, all the same form: `if ! python3 tools/spsa_driver.py check ... >
  "$CHECK_LOG" 2>&1; then cat "$CHECK_LOG" >&2; echo "SPSA-FAILED: check refused the
  config" >&2; exit 1; fi; grep -v '^SPSA-DONE$' "$CHECK_LOG" || true`. They test the
  exit status, unchanged. The `grep` is on the success path, unchanged. On the failure
  path the log now carries the driver's `SPSA-FAILED` and then the wrapper's, which is
  what a `PROBLEM` has always done and what S222's comment calls two markers the
  watcher's grep reads as one; a failed check ends the run either way. None reads a
  traceback. Read, not run; they are frozen and need the machine.
- `Engine`'s constructor: `tools/error_profile.py`'s `_open_engine` and
  `tools/depth_vs_eval.py`, the only importers over `tools/`, `adocs/data/` and `tests/`
  until this step's test. `tools/analyse_game.py` has its own `Engine`. It acts only
  when the constructor raises, when no caller holds the process.
- `_init` and `_open_engines`: the pool and `probe()`. Nothing imports `depth_vs_eval`
  but the new test, and no reader of its columns exists outside it (S303).

`DEV_MANUAL.md`'s `check` and `depth_vs_eval` passages say what the failures now do.
The frozen wrappers' comment, "as its last line on every exit path", is now true of
`check` with the three exceptions under its table, and still not of `run`.

### Tests

`tests/test_spsa_probe.py`, 12 cases to 32, 14 s under `nice -n 19` at load 12, 5 s of it
one `QUIT_WAIT_S`; stdlib only. Green, and `tests/test_spsa_driver.py` green, 24 of 24. No
test weakened; the one existing `Cli` case moved onto a helper with its three assertions
as they were.

- `MisbehavingOutput`: closing with no `bestmove` on `exit`, `close` and `wedged`; a line
  cut before its number after a good count; `abc`, `-5`, `1_000`, `12x4` refused, killed
  inside a bound under the 30 s wait; `bestmov` refused; a `bestmove` cut after its word
  keeps the whole count, stated below.
- `BestMoveLine`: the pattern, 5 whole lines and 12 that are not, with no process.
- `DepthVsEvalCli`, the tool end to end: five lines with no whole move stop the run with
  no row and nothing handed to the reference; closing with no `bestmove` stops it; a
  promotion with a ponder word is a row, the control. A healthy engine and reference take
  `quit` and run their exit trap, on a run that succeeds and one that fails. An engine that
  stopped reading goes on the bounded wait, the reference beside it taking `quit`; one
  that closed its stdin goes without the wait; one that never read goes, on both sides.
  **Above one worker, `--workers 3`:** one position starts one engine and one reference;
  an engine dead at birth names `before uciok` and no `BrokenProcessPool`; six positions
  give the same row file at one worker and at three. No case drives the whole-run route
  at the bound, which would cost a second `QUIT_WAIT_S`: its wiring is the `quit` case's
  and its kill the stopped-reading case's, and it is in the table above.
- `Cli`: a live axis ends in `SPSA-DONE`; `check` ending in `ERROR:` and `SPSA-FAILED` on
  `closes`, `no number`, `no spin options`, a missing binary, and eight configs of the
  wrong shape, with no traceback.

Red first, the new file against `9bcafb9`'s tools: 44 subtests fail. `BestMoveLine` 17,
because no pattern exists, so those show the form and not the old behaviour; `Cli` 12;
`DepthVsEvalCli` 11, of which the closed-stdin case reds on the missing `QUIT_WAIT_S` and
not on behaviour; `MisbehavingOutput` 4. S303's own cases and the controls pass there, as
they should. **The first red run caught a false green in my own case.** The outliving cases
passed on the old tool, in a run that took 368 s: an engine left behind inherits stderr, so
`capture_output` waited out its 45 s and then found it gone. Now stderr goes to a file, and
on the old tool they fail with the engine alive.

Eight planted reverts, one edit each in a scratch copy, the staged tree untouched and green
after them, each killed:

| planted | red |
|---|---|
| **the open back in `_init`, the fast check's ask** | `test_a_worker_with_no_task_opens_no_engine`, `test_an_engine_dead_at_birth_fails_its_task_and_not_the_pool` |
| the kill without `quit`, the form the fast check read | the healthy `quit` case, the stopped-reading case's reference |
| no kill after the wait | stopped-reading, closed-stdin |
| `OSError` not caught | closed-stdin |
| the `Finalize` removed | healthy `quit`, stopped-reading, closed-stdin |
| the constructor's kill removed | never-read, dead-at-birth |
| the load's old tuple | wrong-shape configs, seven of eight subtests |
| `warnings()` outside the `try` | wrong-shape configs, the huge count |

Earlier, on the form before the fast check, three more were killed the same way: the
pattern's newline made optional, `int()` deciding the count, `probe_nodes`' kill removed.

### `bestmove` cut in `probe_nodes()`, left as it is

Cut before the word ends, `bestmov`, it raises `... before bestmove`. Cut after,
`bestmove`, `bestmove e3`, `bestmove e2e4 pon`, it returns the count of the whole `info`
line before it, a test says so. The count is the only thing the probe reads, the search
has answered and no wrong number can pass. On a stand-in that also exits, S303's race
on the `quit` write stays, and now ends in the marker.

### Pending: the real-binary comparison

To run after the match, on the workstation: a few minutes by estimate, not measured,
read-only on the builds, no timing read. `W` is the tree holding the new tools, the
worktree or, once landed, the main checkout. Its last line is
`PENDING-COMPARISON-DONE failures=<n>`, and 0 is the pass: every section checks its exit
statuses, so `IDENTICAL` cannot come from two failed runs or two empty files. Section by
section:

- 1, `--workers 1`: `--out` and stdout identical.
- 1b, `--workers 3`, the tool's default, the only one that exercises first-task opening.
  `workers.map` keeps input order, so no sort. Old, old again, new. If old against old
  differs it prints `NO BASELINE`, and old against new says nothing; otherwise old against
  new shows the late open moves no row. It cannot show the count.
- 1c, `--sample 1 --workers 3` through wrappers that log each pid and `exec` the real
  binary: old 3 and 3, new 1 and 1, the row equal. That is the departure above, measured.
- 2, `probe_nodes()`; 3, `check`, `spsa_dryrun` exit 0 and `spsa_s085` exit 1 naming
  `RfpMinPly`.
- 4, a cut `bestmove` on chesso and Stockfish, new exit 1, `waiting for bestmove`, no row.
- 5, `error_profile.py`'s CLI, because the step edits it, S301's form: `--max-games 2
  --workers 2`, chesso at `--depth 6` and Stockfish at `--nodes 5000`, report and raw.
- 6, the ruling: `CHESSO_LMP_CENSUS` on the tune build, a census written at exit under
  both tools and equal.
- 7, nothing left: every real binary by path against a snapshot taken at the start, and
  every logged pid. `pgrep -f "$T"` did not see a real binary. A planted lingering engine
  is reported by it.

The stand-ins' first `go` answers `e2e4` from the start position; that is a fixture for the
old tool's contrast rows and no chess conclusion is drawn from them. Its mechanics were
dry-run with stand-ins through `CH`, `SF`, `TU` and `CONV` in place of every real binary:
every section ran, and `failures=` counted the rows a stand-in cannot pass. That proves the
script and nothing else.

```bash
#!/bin/bash
# S304's pending comparison. Run after the match, on the workstation, from anywhere.
# Old tools (9bcafb9) against the new ones on the real binaries: nodes and moves
# only, read-only on the builds, no timing read. Its last line is
# PENDING-COMPARISON-DONE failures=<n>, and 0 is the pass; on a failure $T stays.
#   W=<tree holding the new tools; the worktree, or the main checkout once landed>
set -u
R=/home/max/ws/chesso
W=${W:-/home/max/ws/chesso-s304}
SF=${SF:-/usr/games/stockfish}
CH=${CH:-$R/build/src/chesso}
TU=${TU:-$R/build-tune/src/chesso}
CONV=${CONV:-$R/build/tools/pgn_to_positions}
T=$(mktemp -d)
mkdir "$T/old"
for f in depth_vs_eval error_profile spsa_driver; do
  git -C "$R" show "9bcafb9:tools/$f.py" > "$T/old/$f.py"
done
fails=0
bad() { echo "  FAIL: $*"; fails=$((fails + 1)); }
want() { if [ "$2" = "$3" ]; then echo "  $1 rc=$3"; else bad "$1 rc=$3, wanted $2"; fi; }
same() { if cmp -s "$2" "$3"; then echo "  $1 IDENTICAL"; else bad "$1 differs: $2 $3"; fi; }
dir_of() { if [ "$1" = old ]; then echo "$T/old"; else echo "$W/tools"; fi; }
real_pids() { pgrep -f -- "$CH|$TU|$SF" | sort; }
before=$(real_pids)

dve() { # old|new out-stem, then the tool's own arguments
  local d; d=$(dir_of "$1"); local stem=$2; shift 2
  python3 "$d/depth_vs_eval.py" "$@" --out "$T/$stem.tsv" > "$T/$stem.stdout" 2> "$T/$stem.stderr"
}
RAW=$R/adocs/data/S018_raw.tsv
LIMITS="--base-nodes 5000 --deep-nodes 20000 --reference-nodes 5000"

echo "== 1. depth_vs_eval, real engine and reference, 12 positions, one worker"
for w in old new; do
  dve "$w" "w1.$w" "$RAW" --engine "$CH" --reference "$SF" $LIMITS --sample 12 --workers 1
  want "$w" 0 $?
done
same "--out" "$T/w1.old.tsv" "$T/w1.new.tsv"
same "stdout" "$T/w1.old.stdout" "$T/w1.new.stdout"
[ "$(wc -l < "$T/w1.new.tsv")" = 13 ] || bad "wanted 12 rows"

# workers.map yields in input order and the seed fixes the sample, so a row file's
# order is deterministic and needs no sort. What this can show: at the tool's default
# worker count, an engine opened at its worker's first task and not at the worker's
# start moves no row. Its baseline is old against old: if scheduling alone moved a
# row, old against new says nothing, and the section says so instead of passing. It
# cannot show the engine count, since 12 positions normally reach all three workers;
# the wrappers below do that.
echo "== 1b. the same at --workers 3, the tool's default: old, old again, new"
for run in old:a old:b new:a; do
  w=${run%%:*}
  dve "$w" "w3.${run/:/.}" "$RAW" --engine "$CH" --reference "$SF" $LIMITS --sample 12 --workers 3
  want "$run" 0 $?
done
if cmp -s "$T/w3.old.a.tsv" "$T/w3.old.b.tsv"; then
  echo "  baseline, old against old: IDENTICAL, so scheduling moved no row"
  same "--out, old against new" "$T/w3.old.a.tsv" "$T/w3.new.a.tsv"
  same "stdout, old against new" "$T/w3.old.a.stdout" "$T/w3.new.a.stdout"
else
  echo "  NO BASELINE: old against old differs, so scheduling moves rows and old against new says nothing"
fi
cmp -s "$T/w1.new.tsv" "$T/w3.new.a.tsv" && echo "  new, one worker against three: IDENTICAL" ||
  echo "  note: new, one worker against three differs"

# Each wrapper logs its pid and execs the real binary, pid unchanged.
for name in ch sf; do
  real=$CH; [ "$name" = sf ] && real=$SF
  printf '#!/bin/sh\necho $$ >> %s\nexec %s\n' "$T/$name.pids" "$real" > "$T/$name"
  chmod +x "$T/$name"
done
echo "== 1c. --sample 1 --workers 3 through wrappers: old 3 and 3 engines, new 1 and 1, the row equal"
for w in old new; do
  : > "$T/ch.pids"; : > "$T/sf.pids"
  dve "$w" "one.$w" "$RAW" --engine "$T/ch" --reference "$T/sf" $LIMITS --sample 1 --workers 3
  want "$w" 0 $?
  counted="$(wc -l < "$T/ch.pids") $(wc -l < "$T/sf.pids")"
  if [ "$w" = old ]; then wanted="3 3"; else wanted="1 1"; fi
  if [ "$counted" = "$wanted" ]; then echo "  $w engines started: $counted"; else bad "$w started $counted, wanted $wanted"; fi
  cat "$T/ch.pids" "$T/sf.pids" >> "$T/all.pids"
done
same "the row" "$T/one.old.tsv" "$T/one.new.tsv"

echo "== 2. probe_nodes direct, both probe positions at depth 9"
cat > "$T/probe.py" <<PY
import sys
sys.path.insert(0, sys.argv[1])
import spsa_driver as sd
for engine, name, value in (("$CH", None, None), ("$CH", "Hash", 8),
                            ("$TU", "RfpMargin", 0), ("$TU", "RfpMargin", 2000)):
    for fen in (sd.PROBE_FEN, sd.PROBE_FEN_TACTICAL):
        print(engine, name, value, sd.probe_nodes(engine, name, value, fen, "go depth 9"))
PY
for w in old new; do
  python3 "$T/probe.py" "$(dir_of "$w")" > "$T/$w.probe"
  want "$w" 0 $?
done
[ "$(wc -l < "$T/new.probe")" = 8 ] || bad "wanted 8 counts"
same "counts" "$T/old.probe" "$T/new.probe"
cat "$T/new.probe"

echo "== 3. check on the tune build, elapsed stripped: dry run exit 0, S085's exit 1 naming RfpMinPly"
for cfg in spsa_dryrun:0:SPSA-DONE spsa_s085:1:SPSA-FAILED; do
  IFS=: read -r name code marker <<< "$cfg"
  for w in old new; do
    python3 "$(dir_of "$w")/spsa_driver.py" check "$R/tools/$name.json" --engine "$TU" 2>&1 |
      sed -E 's/ in [0-9.]+ s$//' > "$T/$w.$name"
    want "$name $w" "$code" "${PIPESTATUS[0]}"
    [ "$(tail -1 "$T/$w.$name")" = "$marker" ] || bad "$name $w does not end in $marker"
  done
  same "$name" "$T/old.$name" "$T/new.$name"
done
grep -h "RfpMinPly" "$T/new.spsa_s085" || bad "spsa_s085 does not name RfpMinPly"

echo "== 4. a cut bestmove, both real references: new exit 1, names the wait, no row"
printf 'game\tply\tphase\tcost\tref\town\tmate\tsan\tfen\n0\t16\t20\t150\t-5\t-8\t0\tQxf5\trnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\n' > "$T/raw1.tsv"
for name_payload in "cut:printf 'bestmove e3'" "bare:printf 'bestmove'" "promo:printf 'bestmove e7e8'"; do
  name=${name_payload%%:*}; payload=${name_payload#*:}
  cat > "$T/$name" <<SH
#!/bin/sh
echo \$\$ >> $T/all.pids
n=0
while read -r line; do
  case "\$line" in
    uci) printf 'uciok\n' ;;
    isready) printf 'readyok\n' ;;
    go*) n=\$((n+1))
      if [ "\$n" -ge 2 ]; then $payload; exit 0; fi
      printf 'info depth 1 score cp -20 nodes 1000 pv e2e4\nbestmove e2e4\n' ;;
    quit) exit 0 ;;
  esac
done
SH
  chmod +x "$T/$name"
  for ref in "$CH" "$SF"; do
    for w in old new; do
      rm -f "$T/row.tsv"
      python3 "$(dir_of "$w")/depth_vs_eval.py" "$T/raw1.tsv" --engine "$T/$name" --reference "$ref" \
        --base-nodes 1000 --deep-nodes 2000 --reference-nodes 2000 --workers 1 \
        --out "$T/row.tsv" > /dev/null 2> "$T/err"
      rc=$?
      row=none; [ -f "$T/row.tsv" ] && row=$(tail -1 "$T/row.tsv" | cut -f6-9)
      echo "  $name $(basename "$ref") $w rc=$rc row=[$row] said=[$(grep -v '^$' "$T/err" | tail -1 | cut -c1-90)]"
      if [ "$w" = new ]; then
        [ "$rc" = 1 ] && [ "$row" = none ] && grep -q "waiting for bestmove" "$T/err" ||
          bad "$name on $(basename "$ref"), new, did not refuse"
      fi
    done
  done
done

echo "== 5. error_profile.py's CLI, real binary, S301's form: report and --raw-out, old against new"
for run in "$CH:--depth 6" "$SF:--nodes 5000"; do
  engine=${run%%:*}; limit=${run#*:}
  for w in old new; do
    python3 "$(dir_of "$w")/error_profile.py" "$R/adocs/data/S018_match.pgn" --max-games 2 --workers 2 \
      --converter "$CONV" --engine "$engine" $limit --raw-out "$T/$w.ep.raw" > "$T/$w.ep.report" 2> "$T/$w.ep.err"
    want "$(basename "$engine") $w" 0 $?
  done
  same "$(basename "$engine") report" "$T/old.ep.report" "$T/new.ep.report"
  same "$(basename "$engine") raw" "$T/old.ep.raw" "$T/new.ep.raw"
  [ "$(wc -l < "$T/new.ep.raw")" -gt 1 ] || bad "$(basename "$engine") raw holds no move"
done

echo "== 6. the ruling on the real binary: the tune build's census, written at exit, old against new"
for w in old new; do
  CHESSO_LMP_CENSUS=$T/census.$w dve "$w" "census.$w" "$RAW" --engine "$TU" --reference "$SF" $LIMITS --sample 3 --workers 1
  want "$w" 0 $?
  [ -s "$T/census.$w" ] && echo "  $w census $(wc -l < "$T/census.$w") lines" || bad "$w wrote no census"
done
same "census" "$T/census.old" "$T/census.new"

echo "== 7. nothing left running: the real binaries by path, every logged pid"
for _ in 1 2 3 4 5; do
  left=""
  for p in $(comm -13 <(echo "$before") <(real_pids)) $(cat "$T/all.pids" 2>/dev/null); do
    st=$(ps -o stat= -p "$p" 2>/dev/null)
    case "$st" in ""|Z*) ;; *) left="$left $p" ;; esac
  done
  [ -z "$left" ] && break
  sleep 1
done
if [ -z "$left" ]; then
  echo "  none"
else
  for p in $left; do bad "left running: $(ps -o pid=,args= -p "$p")"; done
fi

if [ "$fails" = 0 ]; then rm -rf "$T"; else echo "kept $T"; fi
echo "PENDING-COMPARISON-DONE failures=$fails"
[ "$fails" = 0 ]
```

### Found, not fixed

- **The tool killed from outside still leaves its workers and engines.** SIGTERM to the
  main process, three seconds later, old and new alike, on healthy stand-ins: main exit
  143, both workers alive and blocked on their queue, all four engines alive, until those
  workers are killed by pid, when all four go. Python's pool does not watch for this. So
  "no engine outlives the tool" holds for every way the tool ends by itself and not for
  this one. A worker thread on `multiprocessing.parent_process().join()` would cover
  SIGTERM and SIGKILL both, not tried. Out of the accepts, reachable only by hand.
- **No read has a timeout.** `Engine.__init__`, `new_game`, `evaluate` and `best_move`
  block on a live engine that stays silent with its output open: old and new, one worker,
  the tool lasted that stand-in's 5 s bound and would last a real one's hang for ever.
  This step's cost on it: a silent engine beside a failed one used to be cut short by the
  broken pool's SIGTERM, with the survivors above, and is now waited for. 12.3 s on
  stand-ins of 6 s, a queued task opening a second one, with none left alive, measured
  before the bound was added. `Engine` is excluded here.
- **`run` has the same marker gap `check` had,** past its config load. Its `except` takes
  `ConfigError` and `RunError` alone. `run --sim /nonexistent`: exit 1, empty stdout,
  `FileNotFoundError` traceback, observed. `os.makedirs` and the other writes under it sit
  outside too, read, not run. Nothing hangs: the wrappers print a marker on any non-zero
  exit. `DEV_MANUAL`'s marker sentence is about `run`.
- **A well-formed illegal move still reaches the reference.** `best_move()` checks the
  form, it has no move generator. S303's table holds: chesso as a reference refuses it
  and searches another position, exit 0 and a wrong cost, and Stockfish stops. Only
  `--reference build/src/chesso` is exposed, and only on an engine answering an illegal
  move.
- **The reference's malformed lines still die unnamed.** `Engine.evaluate()`: a bare
  `bestmove` is `IndexError`, `score cp abc` is `ValueError`, read, not run. Excluded
  here. Neither passes a number.
- **An undecodable byte** from the engine in `best_move()`: exit 1, no row, a
  `UnicodeDecodeError` traceback naming the byte and not the wait, old and new. In `check`
  it now ends in the marker. `Engine` decides the decoding and is excluded.
- `tools/error_profile.py`'s own workers do not kill their engines either (`_open_engine`,
  read, not run): the same leak, in a tool the step excludes.
- A raise inside `check()` drops the `PROBLEM`s it had gathered, as the traceback did.

### Docs

`DEV_MANUAL.md`, four places: the 0.7 s sentence, the `check` paragraph, the
`depth_vs_eval` section, and the fast-suite paragraph, whose "12 cases and about a second"
was made false; `tests/CMakeLists.txt`'s comment with it. `MANUAL.md` and `TOOLCHAIN.md` name neither tool: no change. `specs.md`
cites `depth_vs_eval` for its evidence files alone, and neither is touched: 160 rows each
with no `(none)` or `0000`. `README.md` untouched.

### Accepts, clause by clause

- `check` prints `SPSA-FAILED` last on a `ConfigError` and exits non-zero, on a planted
  stand-in: **holds**, and on every other exception, config load included. The goal's
  "always" holds with the three exceptions under the table: a bad command line, a signal,
  an unwritable stdout.
- `best_move()` refuses a bare or cut `bestmove`, exit non-zero, naming the wait, on a
  planted stand-in with both references: **holds on two planted references and on the real two**,
  chesso and Stockfish, section 4 of the comparison below.
- `probe_nodes()` turns `info ... nodes abc` into a `ConfigError` with the process killed:
  **holds**.
- An engine that never reads stdin does not outlive `depth_vs_eval.py`: **holds**, through
  the kill after the bounded wait, for every way the tool ends by itself, on both sides of
  the pair, with a healthy engine seeing `quit` and ending by itself. Not when the tool is
  killed from outside, first item above.
- `spsa_s085.json` stated, with the reason, as a finished run's record, and `DEV_MANUAL`'s
  sentence says so: **holds**.
- Each tool's output on a healthy real binary unchanged, nodes and moves equal, timings
  stripped: **holds**, sections 1 to 3, 5 and 6 of the comparison below. The
  engine count and the end of a healthy engine change, the departures stated above.
- No timing is taken: **holds**.

## 2026-10-31: the comparison, run by the coordinator

The match over, the script above ran from `.tuning` with `W=../chesso-s304`, against
the main checkout's `build/` at `cf8661b` and `build-tune/`, read-only, no timing read:
**`PENDING-COMPARISON-DONE failures=0`**. Temporary paths written `$T`.

```
== 1. depth_vs_eval, real engine and reference, 12 positions, one worker
  old rc=0
  new rc=0
  --out IDENTICAL
  stdout IDENTICAL
== 1b. the same at --workers 3, the tool's default: old, old again, new
  old:a rc=0
  old:b rc=0
  new:a rc=0
  baseline, old against old: IDENTICAL, so scheduling moved no row
  --out, old against new IDENTICAL
  stdout, old against new IDENTICAL
  new, one worker against three: IDENTICAL
== 1c. --sample 1 --workers 3 through wrappers: old 3 and 3 engines, new 1 and 1, the row equal
  old rc=0
  old engines started: 3 3
  new rc=0
  new engines started: 1 1
  the row IDENTICAL
== 2. probe_nodes direct, both probe positions at depth 9
  old rc=0
  new rc=0
  counts IDENTICAL
/home/max/ws/chesso/build/src/chesso None None 37187
/home/max/ws/chesso/build/src/chesso None None 56111
/home/max/ws/chesso/build/src/chesso Hash 8 37187
/home/max/ws/chesso/build/src/chesso Hash 8 56111
/home/max/ws/chesso/build-tune/src/chesso RfpMargin 0 21117
/home/max/ws/chesso/build-tune/src/chesso RfpMargin 0 12136
/home/max/ws/chesso/build-tune/src/chesso RfpMargin 2000 51231
/home/max/ws/chesso/build-tune/src/chesso RfpMargin 2000 116342
== 3. check on the tune build, elapsed stripped: dry run exit 0, S085's exit 1 naming RfpMinPly
  spsa_dryrun old rc=0
  spsa_dryrun new rc=0
  spsa_dryrun IDENTICAL
  spsa_s085 old rc=1
  spsa_s085 new rc=1
  spsa_s085 IDENTICAL
PROBLEM: RfpMinPly: config bounds [0, 63] against the binary's [3, 63]; a value outside the binary's range is refused, not clamped, and the refusal is invisible
== 4. a cut bestmove, both real references: new exit 1, names the wait, no row
  cut chesso old rc=0 row=[e2e4	e3	-5	56] said=[per-position records in $T/row.tsv]
  cut chesso new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove e3', which holds no whole move]
  cut stockfish old rc=1 row=[none] said=[RuntimeError: engine closed its output before bestmove]
  cut stockfish new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove e3', which holds no whole move]
  bare chesso old rc=1 row=[none] said=[IndexError: list index out of range]
  bare chesso new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove', which holds no whole move]
  bare stockfish old rc=1 row=[none] said=[IndexError: list index out of range]
  bare stockfish new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove', which holds no whole move]
  promo chesso old rc=0 row=[e2e4	e7e8	-5	56] said=[per-position records in $T/row.tsv]
  promo chesso new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove e7e8', which holds no whole move]
  promo stockfish old rc=1 row=[none] said=[RuntimeError: engine closed its output before bestmove]
  promo stockfish new rc=1 row=[none] said=[RuntimeError: waiting for bestmove, engine sent 'bestmove e7e8', which holds no whole move]
== 5. error_profile.py's CLI, real binary, S301's form: report and --raw-out, old against new
  chesso old rc=0
  chesso new rc=0
  chesso report IDENTICAL
  chesso raw IDENTICAL
  stockfish old rc=0
  stockfish new rc=0
  stockfish report IDENTICAL
  stockfish raw IDENTICAL
== 6. the ruling on the real binary: the tune build's census, written at exit, old against new
  old rc=0
  old census 4184 lines
  new rc=0
  new census 4184 lines
  census IDENTICAL
== 7. nothing left running: the real binaries by path, every logged pid
  none
PENDING-COMPARISON-DONE failures=0
```
