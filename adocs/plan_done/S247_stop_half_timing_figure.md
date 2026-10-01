id:         S247
goal:       the eight-queens stop half's recorded timing figure in `tests/test_engine.cpp`'s first-iteration case reads what the idle machine reports, or is named as a record and not a golden
accepts:    the GOLDEN paragraph of the stop half either quotes a figure re-taken on the idle machine with the command that took it, or says the number is the landing-day record and the floor of 3 ms is the only assertion (DEC-240); whichever is chosen is stated with the reason; the fast suite is green in both builds
touches:    tests/test_engine.cpp (a comment), DEV_MANUAL.md if its golden row quotes the figure
excludes:   the floor itself, the board, the timer half; any change to what the case asserts
decisions:  DEC-142, DEC-240, DEC-171
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-10-01 21:50 CEST
done:       2026-10-01 -- re-taken, not ruled a landing-day record (that option, the 3 ms floor as the only assertion under DEC-240, is for a figure that cannot be re-taken cleanly). The stop half's GOLDEN paragraph in `tests/test_engine.cpp` quotes 36165 nodes and 6 to 9 ms, median 6.8 ms, on the idle machine, with the command: `position fen <the board>` then `go depth 1` in a fresh `chesso` process, the info line's `nodes` and `time` (`nodes * 1e6 / nps` for microseconds), 400 runs in two sessions interleaved with the start position as control, governor `performance`, load 3.0 to 3.4 (the build's tail), session medians 6.81 and 6.77 ms, range 6.39 to 9.46 ms; 13.8 ms reproduced by no run and named as the figure the case was written at. The margin sentence follows the figure ("four times faster" becomes "twice faster", 6.4 / 3); the floor, board, assertion and timer half unmoved; comments only, `src/` untouched. Fast suite 41 of 41 in both builds, `clang-format.sh --check` clean, `plan_prose_check.py` `--citations`, `--touches`, `--params` clean; cold fast check FIX-FIRST on text only (an unlogged tune-build row dropped, unlogged load observations marked), fixed. `DEV_MANUAL.md` and `MANUAL.md` checked: neither quotes the figure, no change; `README.md` human-owned, no change.

## Why this exists (2026-09-30)

S022 verdict 2's landing moved the stop half to a heavier board because the
early-out had cut the eight-queens board under the case's 3 ms floor, and
S245's item 2 closed the stale "13.8 ms" figure as moot on that board. The
verdict read as a zero and the removal restored the eight-queens board with
its numbers byte for byte, as the pre-registration's H0 row ordered, so the
"13.8 ms" figure is live again while the board ran 6 to 7 ms on the idle
machine at the landing's ruling. The removal's fast check (2026-09-29)
asked for a filler to re-take the time on the idle machine rather than bend
the byte-for-byte restore. A filler behind S114 (DEC-171): it reaches no
play and moves no reported score.

## As built (2026-10-01)

**The choice: re-taken, not ruled a record.** The measurement is clean and
repeatable -- 400 runs in two sessions, every one at 36165 nodes and between
6.39 and 9.46 ms, the two sessions' medians 6.81 and 6.77 ms, the per-round
medians of twenty within 6.50 to 7.11 ms -- so the GOLDEN paragraph quotes the
figure with the command that took it. The other option -- a landing-day record
with the 3 ms floor as the only assertion (DEC-240) -- is for a figure that
cannot be re-taken cleanly; this one can. 13.8 ms is reproduced by no run
(the slowest of 400 is 9.46 ms) and stays named at the site only as the figure
the case was written at.

**The measurement.** `.tuning/coord/S247/depth1_time.py` (gitignored, beside
its logs): each run is a fresh `build/src/chesso` process -- the test's
`uci_init()` is a first search in a fresh state too -- sent `uci`, `isready`,
`position fen q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1`,
`go depth 1`, and `quit` only after `bestmove` (TOOLCHAIN.md's trap). It reads
the info line's `time` (integer ms) and recovers microseconds as `nodes * 1e6 /
nps`, the engine's own microsecond clock (`src/chesso.cpp`, the info line).
Interleaved: each round alternates the eight-queens board with the start
position at `go depth 1` as a control, 20 of each, 10 rounds.

```
~/.venv/chess/bin/python .tuning/coord/S247/depth1_time.py build/src/chesso 10 20
```

| run | load before | load after | n | nodes | min | p10 | median | p90 | max | `time` ms histogram |
|---|---|---|---|---|---|---|---|---|---|---|
| 1, 21:57:04 | 3.35 3.08 2.43 | 3.06 3.03 2.42 | 200 | 36165 | 6393 us | 6429 | 6806 | 7715 | 8957 | 6: 114, 7: 82, 8: 4 |
| 2, 21:57:18 | 3.29 3.08 2.44 | 3.11 3.04 2.44 | 200 | 36165 | 6388 us | 6437 | 6772 | 7820 | 9458 | 6: 114, 7: 78, 8: 6, 9: 2 |

Control (start position, depth 1): 49 nodes, median 67 to 68 us, every run
`time 0`, in both sessions. Governor `performance` on all 12 CPUs. Logs:
`.tuning/coord/S247/depth1_time.log`, `depth1_time_2.log`.

**The load, stated.** The load figures in the table are logged; the rest of this paragraph was observed at the terminal and not logged. The one-minute load average read 2.0 to 2.2 before the
build and 3.0 to 3.4 during the runs (the build's tail decaying); `vmstat`
read 93 to 94 % idle, `ps` showed only the desktop (cosmic-comp, a terminal,
btop) running, nothing CPU-bound. The baseline of about 2 with nothing
runnable is the machine's resting figure here (kernel threads counted in
the average), not a competing process. The spread -- min 6.4, p90 7.7 to
7.8 ms, about 20 % -- is the run-to-run variation of a single-threaded 6 ms
search in a fresh process, the round medians agreeing within 9 %; the golden
is quoted as the range "6 to 9 ms, median 6.8 ms" rather than one number for
that reason. `hyperfine` does not fit the shape: it times the whole process,
start-up and `quit` included, and a pipe cannot hold `quit` until `bestmove`;
the engine's own microsecond figure measures exactly what the golden names.

**What changed.** `tests/test_engine.cpp`, comments only, the stop half's
GOLDEN paragraph: "13.8 ms on this machine" becomes "36165 nodes and 6 to 9
ms on this machine, median 6.8 ms", with the command and run shape above and
a sentence that the case was written at 13.8 ms, which no run here
reproduces. The record paragraph beneath it loses its parenthesis "the 13.8
ms above is the case's figure from when it was written", now said once above.
**One sentence of rationale moved with the figure**: "a four times faster
machine would still leave the case separating" was 13.8 / 3; at 6.4 ms
minimum it is about 2.1, so it reads "a twice faster machine". The floor
(`depth_1_floor_ms` 3), the board, the assertion and the timer half do not
move; the timer half's own "four times faster" sentence is about its 402 ms
bound against 5.9 s and is untouched. `DEV_MANUAL.md` quotes no stop-half
figure (its S022 paragraphs name the board and node counts only), so it does
not change; `MANUAL.md` does not touch this case, no change. `README.md`
human-owned, no change. `git diff 10efd2b -- src` is empty.

**Gates.** Release `build` and tune `build-tune` built `-j8`, `ctest -L fast`
41 of 41 in each (103.4 s, 104.4 s); `./clang-format.sh --check` with
`CLANG_FORMAT_MAJOR=22` exits 0; `tools/plan_prose_check.py --citations`,
`--touches`, `--params`, one mode per call, each exit 0 with nothing
flagged (`--params` prints nothing). Logs under `.tuning/coord/S247/`.

**Worktree note.** The worktree had no `tests/doctest` submodule checkout, so
the first build failed on `doctest.h`; it was cloned from the main checkout's
`.git/modules/tests/doctest` with a one-shot `-c submodule.tests/doctest.url=`
override (no shared config written), at the pinned `ae7a135` (v2.4.11).

**Proposed `done:` stamp.**

2026-10-01 -- re-taken, not ruled a landing-day record (that option, the
3 ms floor as the only assertion under DEC-240, is for a figure that cannot
be re-taken cleanly). The stop half's GOLDEN paragraph in
`tests/test_engine.cpp` quotes 36165 nodes and 6 to 9 ms, median 6.8 ms, on
the idle i7-8700K, with the command: `position fen <the board>` then `go depth
1` in a fresh `chesso` process, the info line's `nodes` and `time`
(`nodes * 1e6 / nps` for microseconds), 400 runs in two sessions interleaved
with the start position as control, governor `performance`, load 3.0 to 3.4
(the build's tail, logged; 93 to 94 % idle, observed and not logged), session medians 6.81 and 6.77 ms, range
6.39 to 9.46 ms; 13.8 ms reproduced by no run and named as the figure the case
was written at. The rationale sentence follows the figure ("four times
faster" becomes "twice faster", 6.4 / 3); the floor, board, assertion and
timer half unmoved; comments only, `src/` untouched. Fast suite 41 of 41 in
both builds, `clang-format.sh --check` clean, `plan_prose_check.py`
`--citations`, `--touches`, `--params` clean. `DEV_MANUAL.md` and
`MANUAL.md` checked: neither quotes the figure, no change; `README.md`
human-owned, no change.

**Proposed commit.**

```
Re-take the eight-queens stop half's depth-1 time

The stop half's GOLDEN paragraph in test_engine's first-iteration case
quoted 13.8 ms, the figure the case was written at, restored byte for
byte when S022 verdict 2's early-out left. On the idle machine the board
runs 6 to 9 ms at depth 1, median 6.8 ms over 400 fresh-process runs, so
the paragraph now quotes that with its command, and the margin sentence
follows it: the 3 ms floor separates on a machine twice as fast, not four
times. Comments only; the floor, board, assertion and timer half do not
move (DEC-142, DEC-240). S247, a filler behind S114 (DEC-171).
```

**Fast check (the coordinator, 2026-10-01):** FIX-FIRST on text only. A tune-build row of 40 runs had no saved log and was dropped (the slowest-of count is 400 now), and the load paragraph's unlogged observations are marked as such. The comment in `tests/test_engine.cpp` did not change.
