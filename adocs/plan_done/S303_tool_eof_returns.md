id:         S303
goal:       `tools/depth_vs_eval.py` and `tools/spsa_driver.py` stop with an error when the engine closes its output, instead of returning a placeholder move or a partial node count
accepts:    `depth_vs_eval.py`'s `best_move()` raises at end of file instead of returning `-`, and `spsa_driver.py`'s `probe_nodes()` raises instead of returning a partial count, each with a message naming what it waited for, shown on a planted stand-in engine that exits and one that closes stdout and stays alive; each tool's output on the real binary is unchanged (nodes and moves equal, timings stripped); every caller of either function is checked and none depends on the old return; no timing is taken
touches:    tools/depth_vs_eval.py, tools/spsa_driver.py
excludes:   any change to a tool's protocol, output or workload; the frozen scripts under `adocs/data/`; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-31, both returns raise and name their wait on planted stand-ins, every caller checked, the real binary's output identical; both fast suites 50 of 50, format clean; fast-checked, its four fixed; filed S304; DEV_MANUAL and MANUAL checked, no change; tools only, tier two does not bind
## Why

S301's subagent found two more end-of-file returns in live tools.
`depth_vs_eval.py`'s `best_move()` returns `-` when the engine closes its
output, so a dead engine reads as a move. S301's fast check traced the
worst case: if the engine closes during a worker's last deep search,
`cost_of` sends `position fen ... moves -` to the reference, costs the row
from the wrong position, and the run exits 0. `spsa_driver.py`'s
`probe_nodes()` can return a partial node count, which an SPSA run would
take as a measurement. Both can pass a wrong number from a tool. Neither is reachable in ordinary play or on the UCI
surface, and neither moves the engine's own score, move or line (DEC-171).
Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.

## 2026-10-31: the fix and its evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s303`, branch `s303`, from `f08d9db`, beside the
coordinator's S125 screen fit. Nothing compiled and no test suite or gate run
here: the coordinator runs them after the fit. No timing, match or fit: the
real binaries were run read-only, for node counts and best moves, under
`nice -n 10`. The stand-ins and the old copies of the tools (`git show
f08d9db:...`) live in the session scratchpad, not in the tree.

### What changed

- `tools/depth_vs_eval.py`, `best_move()`: at end of file it raises
  `RuntimeError("engine closed its output before bestmove")`, the form and the
  words of `error_profile.py`'s `Engine`, which this tool imports, instead of
  returning `-`.
- `tools/spsa_driver.py`, `probe_nodes()`: at end of file it kills the process
  and raises `ConfigError(f"{engine} closed its output before bestmove under
  `{go}`")`, the class and the shape of the same function's `reported no node
  count` error, instead of breaking out with the last `info` line's count.
  The kill is there because the `finally` waits on the process before it closes
  stdin: an engine alive with its output closed would hold the error for the
  whole 30 s of that wait. `proc.kill()` on a process that has already exited
  does nothing.
- `tools/spsa_driver.py`, `probe_nodes()`'s parse, from the fast check: an `info`
  line that ends at ` nodes `, before its number, sets `nodes` to `None` instead
  of raising a bare `IndexError`, so one of the two errors above fires. Evidence
  under "A line cut off before its number".

A line that has its number parses as before, so a healthy engine's path through
both tools does what it did.

### Planted stand-ins, before and after

`/bin/sh` scripts that answer `uci` and `isready`, answer a `go` normally until
their Nth, then print one partial `info ... nodes` line and misbehave. `exit`
exits. `close` does `exec 1>&-`, keeps reading stdin and honours `quit`. `wedged`
does `exec 1>&-` and never reads again, for a bounded 45 s, so nothing leaks. The
reference beside them behaves and logs every `position` it is handed.

`depth_vs_eval.py`, one position, one worker, the engine failing at its second
`go`, the deep search (S301's worst case), with that planted reference:

| stand-in | old (`f08d9db`) | new |
|---|---|---|
| `exit` | exit 0, a row with `deep_move` `-`, reference handed `... moves -` | exit 1, `engine closed its output before bestmove`, no `--out` file, reference handed nothing |
| `close` | the same | the same |
| `wedged` | the same | the same |

**This corrects the Why, and this table's reach, found by the fast check.** The
old `exit 0` needs a reference that answers after `moves -`, and the installed one
does not. Observed, the stand-ins answering `e3d4` at their first `go`:

| reference | handed `position fen ... moves -`, then `go` | old tool |
|---|---|---|
| `/usr/games/stockfish`, `dev-20260810-5062aee5` | prints `info string CRITICAL ERROR: Command ... failed. Reason: Illegal move: -` and a blank line, no `bestmove`, exits rc 1 | exit 1, no row, `engine closed its output before bestmove` from `error_profile.py`'s `evaluate()`, on `exit`, `close` and `wedged` |
| `build/src/chesso` | prints `info string refused [position moves] -, does not parse` and answers | exit 0, a row with `deep_move` `-` and `deep_cost` -46, on `exit` and `close` |
| the planted reference | answers | exit 0, the table above |

So S301's worst case ("costs the row from the wrong position, and the run exits
0") does not happen on the installed Stockfish. There the old tool stopped, and
its message named a wait on the reference when the process that failed first was
the engine. It does happen on `--reference build/src/chesso`: after the refusal
chesso searched what `position startpos` searches (`bestmove d2d4`, last scoring
line `score cp 61 nodes 920`, at `go nodes 2000`, the same for both) and not the
fixture's FEN (`bestmove e3d4`, `score cp -20 nodes 2000`).

The new tool hands neither reference `moves -`: exit 1, `... before bestmove` from
`best_move()`, no row, on both references, on `exit` and `close`.

`e3d4` is chesso's own `bestmove` for the fixture, and Stockfish takes it, rc 0.
The stand-ins' fixed `e2e4` would not do for these runs: Stockfish stops on it the
same way, `Illegal move`, so the run would have ended at the base cost.

Failing at the first `go`, the base search: old exit 1, with `BrokenPipeError`
for `exit` and `engine closed its output before readyok` for `close`, both from
the second `best_move()`'s `new_game()` (S301's message), so the old tool stopped
there but named the wrong wait. New: exit 1, `... before bestmove`, for all three
stand-ins.

Six positions, three workers: every engine failing at its second `go`, and
separately one engine of three failing with the other two behaving. New: exit 1,
`... before bestmove`, no `--out` file, under 0.3 s each, so the pool does not
swallow the error. The old tool on the one-of-three case also exited 1, on that
worker's next position's `new_game()`, after handing the reference `moves -`.

`spsa_driver.py`'s `probe_nodes()`, called directly at option value 500:

| stand-in | old | new |
|---|---|---|
| `exit` | `BrokenPipeError` on the `quit` write | `ConfigError: <path> closed its output before bestmove under `go depth 9`` |
| `close` | **returned 1500**, the partial count | the same `ConfigError` |
| `wedged` | **returned 1500**, after the 30 s wait | the same `ConfigError`, 0.0 s |

`spsa_driver.py check`, end to end on a one-axis config:

| stand-in | old | new |
|---|---|---|
| `exit` | exit 1, a `BrokenPipeError` traceback | exit 1, the `ConfigError`'s traceback |
| `close` | **exit 0**, `setoption reaches the search: Axis 0 -> 1000 nodes, 1000 -> 2000 (depth 9, midgame)`, `SPSA-DONE` | exit 1, the `ConfigError`'s traceback |
| `wedged` | not run through `check`: 30 s per probe, and the direct call above shows the return | exit 1, the `ConfigError`'s traceback |

The old `close` row is the wrong number made visible: an engine that died at its
first `info` line passed the check that exists to prove an axis reaches the
search. **This corrects the Why:** `probe_nodes()` is reached from `check` only,
never from a `run`, so what the partial count could fool is the check's verdict
and not an SPSA run's measurement directly.

An engine that exits without reading stdin (`/bin/true`) raises `BrokenPipeError`
on the first write when the write loses the race, old and new alike: three of
three on the old, two of three on the new, the third the `ConfigError`. That is
S301's recorded case, it stops with an error, and it passes no number.

### A line cut off before its number (the fast check's corner)

An `info` line ending at ` nodes `, then end of file. The parse's `[0]` raised a
bare `IndexError` before the end-of-file branch could run, so no message named the
wait and the kill was skipped. Reproduced first, on a planted `/bin/sh` stand-in
that writes an exact payload at `go` and then does `exit`, `close` or `wedged`,
`probe_nodes()` called directly:

| payload, then | old, and this step before the change | now |
|---|---|---|
| `info depth 1 nodes \n`, `exit` | `IndexError`, 0.0 s | `ConfigError: <path> closed its output before bestmove under `go depth 9``, 0.0 s |
| the same, `close` | `IndexError`, 30.0 s | the same `ConfigError`, 0.0 s |
| the same, `wedged` | `IndexError`, 30.0 s | the same `ConfigError`, 0.0 s |
| no newline, `close` | `IndexError`, 30.0 s | the same `ConfigError`, 0.0 s |

Through `spsa_driver.py check`, `close`: exit 1 with `IndexError` before, exit 1
with the `ConfigError`'s traceback now.

The value is `None` and not the count before it, so the line cannot leave a stale
one behind. `info ... nodes 920`, the cut line, `bestmove` raises `reported no node
count` where keeping the count would have returned 920. The cut line, then `info
... nodes 920`, then `bestmove` returns 920, where it raised `IndexError` before.
That is the one input that now goes through: the count returned is the later
line's own.

The sweep, one process, every prefix of an `info` line the real binary printed,
with and without a newline, 156 cases, alone and after a good `info` line. Before
the change, on `exit` (`close` would wait 30 s on each hit): 154 raised the message
and 2 raised `IndexError`, the prefix ending ` nodes `, with and without its
newline. Now, on `exit` and on `close`: 156 of 156 raise the message, slowest 0.0 s.

Every prefix of `bestmove d2d4 ponder d7d5` after a good `info` line, 50 cases, the
same before and after: up to `bestmov`, 14 raise the message. From `bestmove` on
the search has answered, and on `close` all 36 return the good count; on `exit`, 19
raise `BrokenPipeError` on the `quit` write and 17 return it, by the race on that
write.

The figures in seconds in this section and the last are a stand-in failing, not a
measurement of the engine.

### Callers

`best_move()`: `probe()` only, twice (`tools/depth_vs_eval.py`), which feeds
`cost_of()` and `deep == base`. `probe()` runs only through `workers.map` in
`main()`, whose exception reaches `main()` and ends the run before `--out` is
written. Nothing imports `depth_vs_eval`; a grep over every `*.py`, `*.sh` and
CMake file finds the tool itself alone. No reader of a `base_move` or `deep_move`
column exists outside it. It has two committed outputs, found by that header over
the whole tree at `f08d9db`: `adocs/data/DEC033_depth_vs_eval.tsv` (DEC-033) and
`adocs/data/S028_depth_vs_eval.tsv` (DEC-035, quoted by `specs.md`). 160 rows each
and no `-` move in either, so neither decision's figures are touched by this.
`specs.md`, `DEV_MANUAL.md` and `adocs/data/README.md` cite them; no code reads
them.

`probe_nodes()`: `bound_repeats()` and `probe_axis()` only, both in
`tools/spsa_driver.py`, reached from `check()` alone. Both compare integers and
neither has a branch for a partial count; `check()` catches nothing, so the raise
goes up to `main()`. The rest of the reach:

- `tests/test_spsa_probe.py` calls `probe_axis()`, `check()` and the CLI against
  a stub that answers every `go` with `info` and `bestmove`, so no case reaches
  the end of file. Read, not run.
- `tests/test_spsa_driver.py` has no case that probes an engine: its
  `assertRaises(sd.ConfigError)` are on config loading. Read, not run.
- `adocs/data/S098_v1_spsa.sh`, `S222_spsa.sh`, `S231_spsa.sh`, `S261_spsa.sh`
  run `check` and test its exit status, so a raise reads as the refusal it is.
- `adocs/data/S261_apply_fit.py` uses `Config` only; `S261_split_sim.py` drives
  `run --sim`, which plays through no engine.

None depends on the old return.

### The real binary, old against new

Timings stripped, compared with `cmp`:

- `depth_vs_eval.py adocs/data/S018_raw.tsv`, `build/src/chesso` at 5000 and
  20000 nodes, `/usr/games/stockfish` reference at 5000, `--sample 12 --workers 1`:
  the 12-row `--out` file (md5 `37f0a70d83c7fd4572ece02cd97f7db5`) and stdout
  identical; stderr identical apart from its progress timings and the output path.
- `probe_nodes()` direct at depth 9 on both of the probe's positions:
  `build/src/chesso` with no option and with `Hash 8` (26906, 35629 both ways),
  `build-tune/src/chesso` at `RfpMargin` 0 (31933, 10416) and 2000 (93702, 72200):
  identical.
- `spsa_driver.py check` on `build-tune/src/chesso` with the elapsed figure
  stripped: `tools/spsa_dryrun.json` identical, exit 0, three axes reaching at
  depth 9, md5 `18eac0b0c57a`; `tools/spsa_s085.json` identical, exit 1 both ways,
  md5 `0a46902c365a` (see below).

`spsa_driver.py`'s two comparisons were repeated after the parse change, old from
`f08d9db` against the tree: the same eight `probe_nodes()` values, and `check` the
same exit status and the same two md5s. `depth_vs_eval.py` did not change after its
comparison.

**One departure from the brief, which named `build/`:** `check` needs the tune
build, because `build/src/chesso` lists only `OwnBook`, `Book`, `Best`, `Hash` and
`Threads`, so none of a config's axes reach a probe there. `build-tune/` was read
and executed only.

### Found, not fixed

- **A `bestmove` cut off in the middle of its move still reads as a move in
  `best_move()`.** The same family as the fast check's corner, inside `touches:`,
  outside the four things asked, so it is left for the coordinator. A stand-in that
  answers its first `go` whole, then writes `bestmove e3` with no newline at the
  deep search and exits, on the tool as staged: exit 0 and a row with `deep_move`
  `e3` on the planted reference and on `build/src/chesso`, `deep_cost` -46 on chesso
  as above; exit 1, no row, on `/usr/games/stockfish`. Bare `bestmove` then end of
  file raises `IndexError` with no message. Old and new alike. It needs a line cut
  mid-write, which a process that writes whole lines does not produce.
- `info ... nodes abc` raises a bare `ValueError` from the parse's `int()`, without
  the kill. No cut of a number produces it, so it is not this corner.
- `main()`'s `check` branch sits outside any `try`, so a `ConfigError` from inside
  `check()` ends in a traceback and exit 1 with no `SPSA-FAILED` line. This is
  pre-existing: the old tool does the same for `read_engine_options()`'s error
  (`--engine /bin/true`: exit 1, empty stdout, traceback) and for `reported no node
  count`. This step's new error takes the same exit. Nothing hangs today: all four
  wrappers test `check`'s exit status, and DEV_MANUAL's marker sentence is about
  `run`. Against DEC-061's every-exit-a-marker it is a gap, and closing it changes
  a tool's output, which this step excludes.
- `tools/spsa_s085.json` fails `check` on the current tune build: `RfpMinPly:
  config bounds [0, 63] against the binary's [3, 63]`. Old and new alike. It may
  be the history of a finished run; DEV_MANUAL still quotes its 0.7 s, from
  2026-09-12, beside the dry run's without saying so.
- A `wedged` engine outlives `depth_vs_eval.py`, whose workers exit without killing
  it. A closed stdin reaches anything that reads it, so only an engine that never
  reads stays, until it ends itself. `error_profile.py`'s `Engine` is the same.

### Docs

`DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md` checked. `MANUAL.md` and
`TOOLCHAIN.md` name neither tool. `DEV_MANUAL.md`'s `depth_vs_eval` section and
SPSA `check` paragraphs say nothing about an engine closing its output, and its
`SPSA-DONE`/`SPSA-FAILED` sentence is about `run`. Nothing is made false, so none
changes. `README.md` untouched.

### Accepts, clause by clause

- `best_move()` raises at end of file instead of returning `-`, with a message
  naming what it waited for, on a stand-in that exits and one that closes stdout
  and stays alive: **holds**, exit 1 with `engine closed its output before
  bestmove` on `exit`, `close` and `wedged`, with the planted reference, the
  installed Stockfish and chesso. Not for a `bestmove` cut mid-move, found above.
- `probe_nodes()` raises instead of returning a partial count, same conditions:
  **holds**, `ConfigError ... closed its output before bestmove under `go depth 9``
  on all three, where the old returned 1500 on two, and on a line cut before its
  number, where it raised `IndexError`.
- Each tool's output on the real binary is unchanged, nodes and moves equal,
  timings stripped: **holds**, `cmp` identical, on `build-tune/` for `check`,
  `spsa_driver.py` again after the parse change.
- Every caller checked, none depends on the old return: **holds**, listed above.
- No timing is taken: **holds**.
