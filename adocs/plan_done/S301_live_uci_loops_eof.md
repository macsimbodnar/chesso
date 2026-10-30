id:         S301
goal:       the live tools that drive the engine over UCI stop with an error, instead of spinning, when the engine closes its output during a handshake or a search, as `tools/search_bench.py` does since S300
accepts:    `build_release.sh`, `adocs/data/S135_cost.py` and `tools/error_profile.py` each raise at end of file in every loop that waits for `uciok`, `readyok` or `bestmove`, with a message naming what it waited for, shown on a planted stand-in engine that exits and one that closes stdout and stays alive; `tools/search_bench.py`'s `go` loop raises at end of file instead of dropping that position's row; each tool's output on the real binary is unchanged (nodes and moves equal, timings stripped); no timing is taken
touches:    build_release.sh, adocs/data/S135_cost.py, tools/error_profile.py, tools/search_bench.py
excludes:   the frozen scripts under `adocs/data/` that no live script calls (they are history, re-run only to reproduce their step); any change to a tool's protocol, output or workload; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30, Opus 5.5 subagent, coordinated: every `uciok`, `readyok` and `bestmove` loop in `build_release.sh`, `adocs/data/S135_cost.py` and `tools/error_profile.py` stops at end of file with `engine closed its output before <token>`, and `tools/search_bench.py`'s `go` loop raises where it dropped the row; each file keeps its own stop (`sys.exit` in the first two, `RuntimeError` in the other two), and the two `go` loops that already stopped now name `bestmove`; planted `/bin/sh` stand-ins that exit, or `exec 1>&-` and stay alive, at `uci`, `isready` and `go`: the old `uciok` and `readyok` loops spun to `timeout 3`'s exit 124, the new exit 1 with the message in under 0.4 s; the old `error_profile.py` exited 0 with a report costed from zeros on a reference that closed its output mid-search, against the Why's "none can pass a wrong number", and the old `search_bench.py` exited 0 with a row missing, both now exit 1; an engine that exits before reading stdin can raise `BrokenPipeError` on the `uci` write instead, old and new alike, no regression; on `build/src/chesso` at `733ba63` all four tools `cmp`-identical old against new, nodes and best moves (build_release's workload 29465966 nodes over 400 positions, S135's 37493477 over 300, search_bench S300's md5s at 9 and 12, error_profile with chesso and with Stockfish), no timing; `./build_release.sh portable` end to end printed the same total in pass 2 and failed pass 3 on `-Werror=missing-profile` for `src/leaf_walk.cpp`, which no engine binary links since `5b0e170`: pre-existing, outside touches, not fixed, filed as S302; every caller checked, none depends on the old behaviour; DEV_MANUAL.md, MANUAL.md and TOOLCHAIN.md checked, no change; both builds and fast suites green (50/50 each), `./clang-format.sh --check` clean under CLANG_FORMAT_MAJOR=22, py_compile and `bash -n` clean, the five plan_prose_check.py modes 0 flagged; no `src/` change, so no Tier 2

## Why

S300's subagent found the spinning loop it fixed in `tools/search_bench.py`
live in three more places. One is `build_release.sh`'s handshake. Another is
`adocs/data/S135_cost.py`'s, the workload that `S121_cost.sh`,
`S123_cost.sh`, `S125_cost.sh` and `S135_cost.sh` time. The third is
`tools/error_profile.py`'s `uciok` and `readyok` waits. Each is
`while "<token>" not in readline(): pass`. At end of file `readline()`
returns the empty string forever, so an engine that dies mid-handshake
spins the tool at full CPU until something kills it.

`search_bench.py`'s `go` loop breaks silently at end of file instead. It
drops that position's row, and only a caller that counts rows notices.
The cost scripts count rows, so they do.

None of these can pass a wrong number; each can only hang until a watcher's
ceiling or a timeout. Not reachable in ordinary play, on the UCI surface,
or in a reported score (DEC-171). Filler, behind the next strength step.
Filed by the coordinator, 2026-10-30.

**Corrected 2026-10-30, by S301's own evidence.** "None of these can pass a
wrong number" is wrong for `tools/error_profile.py`: its `evaluate()` broke
at end of file and scored the position from whatever it had read, 0 when
nothing, so a reference engine that closed its output and stayed alive
costed the rest of a worker's last game from zeros, and the tool exited 0
with a whole report. This step fixes it; the evidence is under the
2026-10-30 section's planted stand-ins.

## 2026-10-30: the fix and its evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s301`, branch `s301`, from `733ba63`. Every build and
run under `nice -n 10`, at most four cores, beside the coordinator's S125
screen fit. No timing, match or fit: the real binary was run only for node
counts and best moves. Scratch under `/tmp/claude-1000/`, deleted before
the report.

### What changed

Every loop that waits for `uciok`, `readyok` or `bestmove` in the four
tools now reads a line, stops with `engine closed its output before
<token>` on the empty string `readline()` returns at end of file, and
otherwise makes the test it made before. Each file keeps its own way of
stopping, as S300 took the `isready` loop's:

- `build_release.sh`, the pass-2 workload: the `uci` loop stops with
  `sys.exit("build_release.sh: engine closed its output before uciok")`,
  the `go` loop's own form. The `go` loop already stopped at end of file;
  its message, `engine died during the workload`, becomes `engine closed
  its output before bestmove`, so it names what it waited for.
- `adocs/data/S135_cost.py`, `workload()`: the same, without the prefix;
  the `go` loop's `engine died` becomes `engine closed its output before
  bestmove`.
- `tools/error_profile.py`, `Engine`: `__init__`'s `uciok` loop and
  `new_game()`'s `readyok` loop raise `RuntimeError`, the S300 form, and
  `evaluate()`'s `bestmove` loop raises where it used to `break`.
- `tools/search_bench.py`, `run()`: the `go` loop raises where it used to
  `break`.

Nothing else: the commands sent, what each tool prints on success and the
workloads are untouched.

### Evidence: planted stand-ins

The before side is `733ba63`'s copy of each tool (`build_release.sh`'s
workload extracted between its `WORKLOAD` markers, as bash feeds it to
`python3 -`; `S135_cost.py` in a scratch root with `S018_raw.tsv` linked,
since it finds the table from its own path). Each stand-in is a `/bin/sh`
script that answers `uci`, `isready` and `go` (one `info ... score cp 37
nodes 11 pv a2a3` line, then `bestmove a2a3`), and stops at its first
`uci`, `isready` or `go`: `*_exit` exits 0, `*_close` does `exec 1>&-` and
stays alive on `cat > /dev/null`. Each run is `nice -n 10 /usr/bin/time
timeout 3`; `error_profile.py` on `S018_match.pgn` with `--max-games 1
--depth 1 --workers 1`. No stand-in process was left after any run.

- **`uciok`, both stand-ins.** `build_release.sh`'s workload: old exit 124
  at 3.01 s and 3.00 s wall, 2.92 s and 2.77 s CPU (user plus sys); new
  exit 1 in 0.04 s and 0.16 s, `build_release.sh: engine closed its output
  before uciok`. `S135_cost.py workload`: old exit 124 at 3.00 s, 2.63 s
  and 2.62 s CPU; new exit 1 in 0.07 s and 0.06 s, `engine closed its
  output before uciok`. `error_profile.py`: old exit 124 at 3.00 s and
  3.01 s, the pool's worker spinning (`ps` at 2.5 s: 99.1 % CPU, 2 s CPU
  time) while the main process waits; new exit 1 in 0.31 s and 0.20 s, the
  worker's `Exception in initializer` traceback ending `RuntimeError:
  engine closed its output before uciok`, then `BrokenProcessPool` from the
  main process.
- **`readyok`, both stand-ins** (`error_profile.py` only; the other two send
  no `isready`, and `search_bench.py`'s already raised): old exit 124 at
  3.03 s and 3.00 s, the worker at 87.4 % CPU; new exit 1 in 0.21 s and
  0.20 s, `RuntimeError: engine closed its output before readyok`.
- **`bestmove`, both stand-ins.** `build_release.sh` and `S135_cost.py`
  already stopped: old exit 1 with `engine died during the workload` and
  `engine died`, new exit 1 with `engine closed its output before
  bestmove`. `error_profile.py`, `_exit`: old exit 1 on `BrokenPipeError`,
  from the next position's write; new exit 1, `RuntimeError: engine closed
  its output before bestmove`. **`error_profile.py`, `_close`: old exit 0
  with a whole report** -- `moves by the profiled engine: 38 over 1 games`,
  `total centipawns given away: 37` -- in which the first position took the
  37 from the one `info` line before the close and every later position a
  0 the stand-in never sent; new exit 1 with the message.
  `search_bench.py`, `_exit`: old exit 1 on `BrokenPipeError`; `_close`:
  old exit 1 with `engine closed its output before readyok`, from the next
  position's `isready` loop; new exit 1 with `engine closed its output
  before bestmove`, both.
- **The dropped row.** A stand-in that answers normally, closes stdout at
  its third `go` after one `info` line and exits on `quit`:
  `search_bench.py` old exit 0 printing midgame, kiwipete and the total,
  the tactical row gone; new exit 1, `engine closed its output before
  bestmove`.

**The Why's "None of these can pass a wrong number" is wrong for
`error_profile.py`.** Its `evaluate()` broke at end of file and scored the
position from whatever it had read, 0 when nothing, so a reference engine
that closes its output and stays alive costed the rest of a worker's last
game from zeros and exited 0. Every other case hung or exited 1. Fixed
here; `tools/depth_vs_eval.py` imports the same `Engine` and is fixed with
it.

**An engine that exits without reading stdin** is outside the stand-ins
above, which read `uci` first. As S300 found, `send("uci")` then raises
`BrokenPipeError` if the engine is gone before the write, and the loop is
reached only if the write wins the race. Three runs each of a `/bin/sh`
`exit 0` stand-in and a `sleep 0.3; exit 0` one, `timeout 3`:
`build_release.sh` old 124 twice and `BrokenPipeError` once, then 124
three times, new the `uciok` message six times; `S135_cost.py` old 124
six times, new the message five times and `BrokenPipeError` once;
`error_profile.py` old 124 six times, new exit 1 six times, the
initializer's line the message five times and `BrokenPipeError` once in a
second set of six. Exit 1 with no handshake message is the
`BrokenPipeError` case, in the old code and the new alike; the old code
spun when the write won the race. Not a regression.

### Evidence: the real binary

`build/src/chesso` built in the worktree at `733ba63` (sha256
`35a43ddbac50ea89...`). Old and new tool on the same input, every run
exit 0, `cmp` identical in every pair:

- `build_release.sh`'s workload against `build/src/chesso` at its defaults,
  depth 10 and the 400 FENs its own `awk` draws: `400 positions, 29465966
  nodes`. Through a `tee` wrapper the engine's whole output, `time` and
  `nps` masked, is identical (4408 lines; 400 `bestmove` lines, md5
  `f9c37cbc274fa8354a6456163c0652b6`). The tool prints no time.
- `S135_cost.py workload` at depth 11, the cost scripts' depth: `300
  37493477`. Engine output through `tee`, `time` and `nps` masked,
  identical (3608 lines; 300 `bestmove` lines, md5
  `b0401296ac354b77e9f7fc0c0a7108e0`). The tool prints no time.
- `search_bench.py` at depths 9 and 12 with `~/.venv/chess/bin/python`, as
  `streams()` runs it: the `streams()` projection md5
  `97b2ef5ec8e3228a05db37b8a65291c9` and `1f0c42a5555c395080a6fb3400f96d70`,
  S300's and `S123_close_inv6.txt`'s, and the whole output with seconds and
  knps masked identical. Depth 9: 26906 `c3d5`, 93894 `d5e6`, 35629
  `d7c8q`; depth 12: 317968, 341523, 205179, the same moves.
- `error_profile.py` over `S018_match.pgn`, `--max-games 2 --workers 2`, with
  `--engine build/src/chesso --depth 6` and with `--engine
  /usr/games/stockfish --nodes 5000` (`Stockfish dev-20260810-5062aee5`,
  which reads to `bestmove`, the safe form in TOOLCHAIN.md): report and
  `--raw-out` identical, 86 moves over 2 games each (report md5
  `2e43f9b6cc3c1871d5728136b994c6f4` and `2e1962a8e0c8a17af05b3e2653c2b0ab`,
  raw `32e5a4eed98915557bbf1eb78f9091b9` and
  `8b26069a34fb411fee2d701d11c05ef7`); stderr identical. The report prints
  no time.
- **`build_release.sh portable` end to end**, under `taskset -c 0-3` so its
  `nproc` is 4, with `TMPDIR` in scratch: everything it writes is
  `.pgo/portable` and `build-release-portable/`, both gitignored, deleted
  after. Pass 2, the changed workload on the instrumented binary, printed
  `400 positions, 29465966 nodes`, `build/src/chesso`'s total above, and 8
  profile files. **Pass 3 then failed, exit 2**:
  `src/leaf_walk.cpp ... profile count data file not found
  [-Werror=missing-profile]`. Not this change: `leaf_walk.cpp` is in
  `chesso_engine` by the `SRCS` glob since `5b0e170` (S082, 2026-10-16),
  no engine binary links it (`nm build/src/chesso` has no `leaf_walk`
  symbol), so no run writes its `.gcda`, whatever drives the workload.
  The last release build recorded, S274's `./build_release.sh bmi2`, exit
  0, is from 2026-10-12, before it. **`./build_release.sh portable` cannot
  build a release at `733ba63`**; bmi2 and avx2 were not run, and the cause,
  an object no engine binary links, does not depend on the arch. Outside
  `touches:`, not fixed: filed as S302, whose file is `cmake/pgo.cmake`,
  which already exempts `search_params.cpp` from `-Wmissing-profile` by name.

### The callers, and what a non-zero exit does in each

- `build_release.sh`, run by hand (`MANUAL.md`, `DEV_MANUAL.md`): under
  `set -euo pipefail` the workload's exit 1 stops the script at pass 2
  with the message; the `uciok` case spun there forever before.
  `tests/test_rating_script.sh` only parses it (`bash -n`, no bare
  `$(nproc)`) and runs nothing.
- `S135_cost.py workload`, in `pairs()` of `S121_cost.sh`, `S123_cost.sh`,
  `S125_cost.sh` and `S135_cost.sh`: the counter run's status is read, and
  a non-zero exit prints `$name pos11 $who $k failed` and returns 1; the
  wall pair is `hyperfine` without `-i`, which stops on a non-zero exit,
  and prints `$name wall pair $k failed`. Before, the `uciok` case hung the
  pair to the watcher's ceiling; the `bestmove` case was already exit 1.
  `S121_cost.py`, `S123_cost.py` and `S125_cost.py` import only
  `counters` and `wall` from it.
- `error_profile.py`, run by hand (DEV_MANUAL's "Profile the engine's own
  errors over many games"): a traceback and exit 1 where it hung or, in
  the `_close` case above, printed a report. `tools/depth_vs_eval.py`
  imports `Engine`: its initializer and `cost_of()` reach the three loops,
  and its `pool.map` carries the exception to exit 1.
- `search_bench.py`'s `go` loop: `streams()` in the four cost scripts reads
  the status and refuses with `search_bench.py at depth $d failed`; before,
  its three-row count refused the same run as `printed no three
  positions`. `tools/mutation_check.py`'s `bench()` finds no rows and
  answers `no search_bench rows`, where a dropped row before gave it two
  rows and a signature without the third. S033's sweeps and
  `2026-09-04_test_review/run.py`, historical, read no status: an empty
  result where a partial one was before.

None depends on the old behaviour, with one reading that changes: in
`tools/mutation_check.py`'s `search_bench` fallback a mutant that closes its
output at the third position read `moved`, two rows against the baseline's
three, and now reads `unmeasured`. That path runs only for an engine without
`bench`, one from before S189 (`detect_oracle()`). Everywhere else the old
behaviour ended in a hang, in the same refusal under another message, or in
a number that was wrong.

**Found, not fixed (outside `touches:`):** `tools/depth_vs_eval.py`'s own
`best_move()` returns `-` at end of file. Mid-run the next call's
`new_game()` now raises, but if the engine closes during a worker's last
deep search, `deep` is `-`, `cost_of()` hands the reference `moves -` and
costs the row from the wrong position, and the run exits 0 (S303 carries the
fix); `tools/spsa_driver.py`'s `probe_nodes()` breaks at end of file
and returns the last `info` line's node count, a partial search's, when the
engine closes its output and stays alive; and the `build_release.sh`
pass-3 failure above.

### Docs

`DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md` checked: none describes
these loops, the old messages or what a tool does when the engine closes
its output (DEV_MANUAL's "closes its output before `uciok` or before
`bestmove`" is `tools/analyse_game.py`'s), so none changes.

### Accepts, clause by clause

- `build_release.sh`, `S135_cost.py` and `error_profile.py` raise at end of
  file in every loop that waits for `uciok`, `readyok` or `bestmove`, with
  a message naming it, on a stand-in that exits and one that closes stdout
  and stays alive: **holds**, exit 1 with `engine closed its output before
  <token>` for every loop and both stand-ins.
- `search_bench.py`'s `go` loop raises at end of file instead of dropping
  the row: **holds**, exit 1 with the message where the old tool printed
  two rows and exit 0.
- Each tool's output on the real binary is unchanged, nodes and moves equal,
  timings stripped: **holds**, `cmp` identical for all four; through
  `build_release.sh` end to end pass 2 printed the same total, and pass 3
  failed for the pre-existing reason above.
- No timing is taken: **holds**; the wall and CPU figures above are a
  stand-in spinning to `timeout`, not a measurement of the engine.
