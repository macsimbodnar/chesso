id:         S300
goal:       `tools/search_bench.py` stops with an error, and does not spin forever, when the engine closes its output before `uciok`
accepts:    an engine that exits or closes stdout before `uciok` makes `search_bench.py` exit non-zero with a message naming the handshake, shown on a planted stand-in engine; the output on the real binary is byte-identical at depths 9 and 12 to the output before the change, nodes and best moves alike (INV-6 for the tool)
touches:    tools/search_bench.py
excludes:   any other change to the tool's protocol, output or positions; any timing run
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30, Opus 5.5 subagent, coordinated: `tools/search_bench.py`'s `uci` loop raises `engine closed its output before uciok` at end of file, the `isready` loop's shape, nothing else changed; two planted stand-ins (exit before `uciok`, `exec 1>&-` before `uciok`) spun the old tool to `timeout 3`'s exit 124 at 3.00 s and 3.01 s wall, 2.99 s and 2.56 s CPU (user plus sys), and make the new one exit 1 with the message in 0.14 s; an engine that exits without reading stdin is outside the handshake case: `send("uci")` raises `BrokenPipeError`, exit 1 with no handshake message, when the engine is gone before the write, in the old code and the new alike, so no regression (the old code spun when the write won the race); on `build/src/chesso` at `7145e45` old and new are `cmp`-identical at depths 9 and 12, on `streams()`'s projection (md5 `97b2ef5e...` and `1f0c42a5...`, S123_close_inv6.txt's) and on the whole output with seconds and knps masked; every caller checked, none depends on the spin; DEV_MANUAL.md, MANUAL.md and TOOLCHAIN.md checked, no change; both builds and fast suites green (50/50 each), `./clang-format.sh --check` clean under CLANG_FORMAT_MAJOR=22, the five plan_prose_check.py modes 0 flagged; no `src/` change, so no Tier 2. The same spin found live in build_release.sh, adocs/data/S135_cost.py and tools/error_profile.py, outside touches, not fixed

## Why

S299's subagent found it while planting failing engines into the cost
scripts' `streams()`. The `uci` handshake loop in `search_bench.py` reads
`while "uciok" not in p.stdout.readline(): pass`. At end of file `readline()`
returns the empty string forever, so an engine that dies or closes its output
before `uciok` spins the loop at full CPU until something kills it. The
`isready` loop below it already raises on end of file; the `uci` loop does not.

S299's `streams()` reaches `search_bench.py` only after both benches pass, so
a dead or junk engine stops at bench. An engine with a working bench and a
UCI that dies before `uciok` would hang the stream until the watcher's
ceiling. It would never pass. The defect is in a measurement tool, not
reachable in ordinary play, on the UCI surface, or in a reported score
(DEC-171). Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.

## 2026-10-30: the fix and its evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s300`, branch `s300`, from `7145e45`. Every build and
run under `nice -n 10`, at most `-j4`, beside the coordinator's S125 screen
fit. No timing, match or fit: the real binary was run only for node counts
and best moves. Scratch under `/tmp/claude-1000/`, deleted before the report.

### What changed

`tools/search_bench.py`'s `uci` handshake loop is now the `isready` loop's
shape: it reads a line, raises `RuntimeError("engine closed its output before
uciok")` on the empty string `readline()` returns at end of file, and stops
on a line containing `uciok`, the same test the old loop made. Nothing else
in the file: protocol, output and positions are untouched.

### Evidence

The before side is `7145e45`'s tool (a copy, sha256 `8bc20fc5...`, equal to
`git show 7145e45:tools/search_bench.py`). Two planted stand-in engines, each
a `/bin/sh` script that reads `uci` and prints `id name ...`:
`stub_exit` then exits 0; `stub_close` does `exec 1>&-` and stays alive on
`cat > /dev/null`.

```
nice -n 10 /usr/bin/time -f '...' timeout 3 python3 tools/search_bench.py <stub> 9
```

- **Before**, both stubs: no output, `exit 124`, `wall 3.00 s, user 2.46 s,
  sys 0.53 s` (`stub_exit`) and `wall 3.01 s, user 2.12 s, sys 0.44 s`
  (`stub_close`). The loop spun one core until `timeout` killed it.
- **After**, both stubs: `exit 1` in 0.14 s wall, the traceback ending
  `RuntimeError: engine closed its output before uciok`. No stub process
  left behind (`pgrep` empty) in either case.
- **An engine that exits without reading stdin** is outside the accepts'
  handshake case, and both stubs above read `uci` first. Whether
  `send("uci")` raises `BrokenPipeError` depends on a race: it does if the
  engine has already gone when the write is made. Three runs each of a
  `/bin/sh` `exit 0` stub, `timeout 3`: the old tool gave `BrokenPipeError`
  exit 1 once and spun to exit 124 twice; the new tool gave the handshake
  message twice and `BrokenPipeError` once, exit 1 every time. A `sleep 0.3;
  exit 0` stub, which loses the race every time: old exit 124 three times,
  new the message three times. Exit 1 with no handshake message is the
  `BrokenPipeError` case, in the old code and the new alike. Not a
  regression.

**The real binary**, `build/src/chesso` built in the worktree at `7145e45`
(sha256 `dccc52c2...`), old and new tool alternated, `~/.venv/chess/bin/python`
as `streams()` uses. All four runs exit 0.
- `streams()`'s projection (`awk '{print $1, $3, $4, $(NF)}' | grep -v
  total`, which drops seconds and knps): `cmp` identical at both depths.
  md5 `97b2ef5ec8e3228a05db37b8a65291c9` at depth 9 and
  `1f0c42a5555c395080a6fb3400f96d70` at depth 12, old and new -- the same
  two sums `adocs/data/S123_close_inv6.txt` records.
- The whole raw output with only the seconds and the knps figure masked
  (`sed -E 's/ +[0-9]+\.[0-9]+s( |$)/ Xs\1/; s/ +[0-9]+ knps/ X knps/'`):
  `cmp` identical at both depths, four lines each, total line included.
- depth 9: midgame 26906 `c3d5`, kiwipete 93894 `d5e6`, tactical 35629
  `d7c8q`. depth 12: 317968 `c3d5`, 341523 `d5e6`, 205179 `d7c8q`.

### The callers, and what a non-zero exit does in each

- `adocs/data/S121_cost.sh`, `S123_cost.sh`, `S125_cost.sh`,
  `S135_cost.sh`, `streams()`: the tool's status is checked, and a non-zero
  exit prints `$bin: search_bench.py at depth $d failed` and returns 1;
  `run` stops on it with `INV-6: no streams from $bin; no timing is taken`.
  The exit is refused, as S299 made it; before, the stream hung to the
  watcher's ceiling.
- `tools/mutation_check.py`, `bench()` under the `search_bench` oracle:
  reads the exit code only for a timeout (900 s), then parses rows; no rows
  is `no search_bench rows`, so the baseline is `Refused` and a mutant row
  `unmeasured`. Before, the same outcome came after the 900 s timeout.
- `adocs/data/S033_rfp_sweep.sh`, `S033_rfp_ply_sweep.sh`,
  `S033_rfp_guard_sweep.sh` (S033's, historical): `$(... 2>/dev/null | grep
  | awk)`, status unread; the row gets an empty node sum where it hung
  before.
- `adocs/data/2026-09-04_test_review/run.py` (historical): `rc` unread, no
  rows parsed, an empty dict; before, `subprocess.run`'s 600 s timeout
  raised and stopped the script.
- `DEV_MANUAL.md` line 3503, the hand command: a person sees the traceback
  and exit 1 where the terminal hung.

Every other mention -- `tools/gate.sh`, `tools/spsa_driver.py`,
`build_release.sh`, the step and data files, `src/` and `tests/` comments --
is prose and runs nothing. None depends on the spin: in each it ended only
in a hang or a timeout.

**Found, not fixed (outside `touches:`):** the same `while "uciok" not in
readline(): pass` form is live in `build_release.sh` line 123,
`adocs/data/S135_cost.py` line 93 (the workload `S123_cost.sh` and
`S125_cost.sh` time) and `tools/error_profile.py` lines 169 and 181 (its
`readyok` loop too), and in many frozen `adocs/data/` scripts.
`tools/timer_race_stress.py` and `tools/analyse_game.py` already raise. And
`search_bench.py`'s `go` loop still `break`s on end of file, dropping that
position's row silently; the cost scripts' three-row check catches it.

### Docs

`DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md` checked: none describes the
handshake loop or the spin (DEV_MANUAL's "closes its output before `uciok`"
at line 5232 is `tools/analyse_game.py`'s), so none changes.

### Accepts, clause by clause

- An engine that exits, or closes stdout, before `uciok` makes the tool exit
  non-zero with a message naming the handshake, on a planted stand-in:
  **holds**, `exit 1`, `engine closed its output before uciok`, both stubs.
- The output on the real binary is byte-identical at depths 9 and 12 to the
  output before the change, nodes and best moves alike: **holds**, `cmp`
  identical on `streams()`'s projection and on the whole output with only
  seconds and knps masked.
