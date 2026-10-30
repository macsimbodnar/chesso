id:         S299
goal:       `adocs/data/S121_cost.sh` and `adocs/data/S135_cost.sh` check every command their INV-6 streams run, as S123's does, so a failure common to every binary cannot pass the diff; and `S121_screen.sh` and `S123_screen.sh` refuse an empty or non-numeric K probe in `part()`
accepts:    each script's `streams()` checks the exit status of `bench`, `bench 12` and `tools/search_bench.py` at both depths, requires each bench stream to end on its total line and each search_bench stream to print its three positions, and keeps raw output out of the directory `run` compares; each script refuses a planted failing engine (exit 1, or junk output) and still passes its own zero against parent on real binaries, shown without a timing; `check` still prints its DONE marker at the shas each script pins; each screen script's `part()` refuses an empty or non-numeric `full.k` before any fit, shown on a planted empty and a planted junk file without a fit
touches:    adocs/data/S121_cost.sh, adocs/data/S135_cost.sh, adocs/data/S121_screen.sh, adocs/data/S123_screen.sh, adocs/data/README.md
excludes:   any timing run; any change to S123_cost.sh, which already has both fixes (S123's phase 1 fast check and `1a231dd`)
decisions:  DEC-047, DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-10-30, Opus 5.5 subagent, coordinated: `S121_cost.sh`'s and `S135_cost.sh`'s `streams()` are `S123_cost.sh`'s, and `run` checks it; `S121_screen.sh`'s and `S123_screen.sh`'s `part()` refuse a K that is not one positive decimal (`k_check`, `S125_screen.sh`'s), and `samples()` had the same hole and is fixed too; shown without a timing (planted engines refused, zero against parent identical on real binaries, `check` DONE at the pinned shas, an empty and a junk `full.k` refused before any fit), the 2026-10-30 section; both fast suites 50 of 50, the format check, `bash -n` and the five prose modes green; no `src/` change, so no tier two; found, out of scope: `tools/search_bench.py` hangs on an engine that closes its output before `uciok`

## Why

S123's phase 1 fast check found that `streams()` in its cost script did not
check the commands it ran. A `bench` or `search_bench.py` failing the same
way for every binary would write the same error text everywhere and pass
the INV-6 `diff -r`. S123 fixed its own copy, then found the fix's second
defect, raw timings in the compared directory, at its first run (`1a231dd`).
S121's and S135's copies have the first gap. They are pinned to their
steps' shas and are re-run only to re-time those steps, so the defect is
not reachable in ordinary play, on the UCI surface, or in a reported score
(DEC-171). Filler, behind the next strength step. Filed by the coordinator,
2026-10-30.

**Widened 2026-10-30 (S125's phase 1 fast check).** `part()` in
`S121_screen.sh` and `S123_screen.sh` writes the K probe's `full.k` by `sed`
and passes `--k "$(cat ...)"` unchecked. An empty file passes `--k ""`,
which `atof` reads as 0, and 0 means "fit K from the data": a bisection
part would silently leave the pre-registered K. Both steps closed at H1, so
neither `part()` has run or is due to run; S125's copy is fixed in its own
phase 1.

## 2026-10-30: the four fixes and their evidence (Opus 5.5 subagent, coordinated)

Worktree `../chesso-s299`, branch `s299`, from `b5f133c`. Every build and
script under `nice -n 10`, at most `-j4`, beside the coordinator's S125
screen fit. No timing, match, fit or tuner run over a corpus: every tuner
reached below is a stand-in shell script. Scratch under `.tuning/s299/` of
the worktree, deleted before the report.

### What changed

- **`S121_cost.sh`, `S135_cost.sh`**: `streams()` is `S123_cost.sh`'s, its
  body byte for byte (the two bodies `diff` empty against S123's), its
  comment naming the source. Both scripts already named the binary, the
  directory, `$python_chess` and `$root` as S123's does, so nothing needed
  adapting. `run`'s call to `streams()` now checks its status and stops,
  `INV-6: no streams from $bin; no timing is taken`, as S123's does; without
  that the function's refusals would be ignored. Nothing else in either.
- **`S121_screen.sh`, `S123_screen.sh`**: `k_check`, `S125_screen.sh`'s, its
  body byte for byte, its comment naming the source. `part()` calls it on
  `full.k` after the probe block, before the ties, `probe_tuner` and
  `fit_one`. **`samples()` had the same hole**: an existing K file was
  skipped unchecked, and a fresh one was checked by `-s` only, which `junk`
  passes. It now calls `k_check` on an existing file and in place of `-s`,
  as S125's does. Nothing else in either.
- **`adocs/data/README.md`**: one clause on each of the four rows.

### Evidence

Planted engines: `fail` (`exit 1`), `junk` (prints `junk`, exits 0). The
real binaries: Release, padded (`CHESSO_BRANCH_PAD:INTERNAL=ON`), `--target
chesso`, from detached worktrees of `12cab3c` and `6ff7485` (S121's pinned
pair) and `76c4ad8` and `b7be983` (S135's). Each `streams()` called alone
with the script's own definitions, the stage dispatch cut off by `sed`.

**The hole, observed first** (`b5f133c`'s `streams()`, both scripts, on two
binaries that fail the same way):
- `fail`, first attempt: both streams written, `INV-6 diff passes`,
  `bench.txt` empty.
- `fail` again, and `junk`: `search_bench.py` never returned. Killed at
  30 s, exit 124. Where both were killed, `junk` still read `INV-6 diff
  passes`. The old form never refused: it passed the diff or hung.

**The fixed `run` on a pre-seeded OUT** (every binary the planted engine,
stamped so `build_one` reuses it; S121 with `ROWS=parent`):
```
CAND_SHA=12cab3c PARENT_SHA=6ff7485 ROWS=parent OUT=<scratch> adocs/data/S121_cost.sh run
CAND_SHA=76c4ad8 PARENT_SHA=b7be983 OUT=<scratch> adocs/data/S135_cost.sh run
```
- `fail`: `.../bin/zero bench failed`, `INV-6: no streams from zero; no
  timing is taken`, `S121-COST-FAILED` (and `S135-COST-FAILED`), exit 1,
  no `pairs/`.
- `junk`: `.../inv6/zero/bench.txt does not end on a bench total`, then the
  same two lines, exit 1, no `pairs/`, for both scripts.

**Each check of the fixed `streams()`, on a plant of its own** (both
scripts, each line exit 1):
- a wrapper whose `bench 12` exits 1: `bench12_fails bench 12 failed`;
- a wrapper that passes bench through to S121's zero binary and on UCI
  prints `uciok` and exits 1: `search_bench.py at depth 9 failed`;
- `$python_chess` replaced by a stand-in printing one position, exit 0:
  `search_bench.py at depth 9 printed no three positions`.

**Zero against parent on real binaries, both pass** (the fixed
`streams()`, then `run`'s `diff -r -q`):
- S121: both exit 0, identical. bench `5069022 nodes`, bench 12 `2024016
  nodes`. `S121_cost_reading.txt` recorded 5069022.
- S135: both exit 0, identical. bench `3945487 nodes`, bench 12 `2103576
  nodes`. `S135_preconditions.txt` recorded both.
- Each compared directory holds `bench.txt bench12.txt sb9.txt sb12.txt`
  and nothing else. The raw `sb9.raw sb12.raw` are in `inv6_raw/<bin>`.
- S121 sb9: `midgame 48284 c3d5`, `kiwipete 126091 d5e6`, `tactical 62646
  d7c8q`.
- S135 sb9: `midgame 42570 c3d5`, `kiwipete 152468 e2a6`, `tactical 27750
  d7c8q`.

**`check` at the pinned shas**:
- `CAND_SHA=12cab3c PARENT_SHA=6ff7485 adocs/data/S121_cost.sh check`:
  `pair CAND carries the table, PARENT does not, PARENT is CAND^`, `patch
  applies to CAND, applied text sha256 d82d1fdc8ac5d4bd`, the rows line,
  `S121-COST-CHECK-DONE`, exit 0.
- `CAND_SHA=76c4ad8 PARENT_SHA=b7be983 adocs/data/S135_cost.sh check`:
  `pair CAND carries the four columns, PARENT does not, PARENT is CAND^`,
  `S135-COST-CHECK-DONE`, exit 0.

**`part()` on a planted `full.k`** (scratch OUT; the corpus record copied
from the coordinator's; the audit's `pass` file planted, so no audit ran;
the tuner a stand-in that prints no K; `S082_OUT` the main checkout's, read
for its sha256 only):
- S123: `adocs/data/S123_screen.sh part A`, root this worktree.
- S121: `adocs/data/S121_screen.sh part A 0 table`, root a detached
  worktree of `12cab3c` carrying this worktree's `S121_screen.sh`, which is
  how the script's header says it runs (`unfolded`). Its `S121_screen.py`
  is the same file at both shas.
- Empty: the probe re-ran on the stand-in (its log `stand-in tuner, no fit:
  ... --epochs 0 ...`), then `refused: .../k/full.k holds '', not a
  positive K`, `S123-PART-FAILED` / `S121-PART-FAILED`, exit 1, `fits/`
  empty.
- `junk`: no probe ran; `refused: .../k/full.k holds 'junk', not a positive
  K`, `-PART-FAILED`, exit 1, `fits/` empty.
- `0.6080`: passes the check and stops one stage on, at `probe_tuner`'s
  `start-check` (the stand-in writes no `probe.hpp`), no fit log.
- `b5f133c`'s `part()` on the same plants, empty and `junk`, both scripts:
  past the K to that same `start-check` refusal, `full.k` still `''` or
  `junk`. The hole, observed.

**`samples()` on planted K files** (five planted samples with their
`.done` hashes, so nothing is drawn; the stand-in tuner):
- Fixed, both scripts: `full.k` empty, `full.k` `junk` and `q3.k` `0.6x`
  each exit 1, `refused: ... holds '...', not a positive K`.
- Six valid K: exit 0.
- `b5f133c`'s: all three plants exit 0.

### Found, out of scope

`tools/search_bench.py` loops forever when the engine closes its output
before `uciok`: `while "uciok" not in p.stdout.readline(): pass`, and
readline returns `""` at EOF. That is the hang above. The fixed `streams()`
reaches `search_bench.py` only after both bench streams passed, so an engine
that is dead or prints junk stops at bench. An engine whose bench works but
whose UCI closes before `uciok` would still hang its stream. The run would
then stop at the watcher's ceiling, never pass. This is `tools/`, outside
`touches`, and not reachable in ordinary play (DEC-171): left for the
coordinator to file.

### Docs

- `adocs/data/README.md`: the four rows say it.
- `DEV_MANUAL.md` checked: it names `S121_screen.sh` and `S123_screen.sh`
  for their ties and audit only, neither the streams nor the K. No change.
- `MANUAL.md` checked: names none of the four. No change.

### Gates

- Both builds, `build` and `build-tune`, at `b5f133c` with this diff
  (`-j4`, `nice -n 10`): no warning.
- Both fast suites 50 of 50, with `CLANG_FORMAT_MAJOR=22`.
- `./clang-format.sh --check`: exit 0.
- `bash -n` on all four scripts: clean.
- `tools/plan_prose_check.py`, one mode per call, each exit 0:
  - `--touches`: 0 flagged over 19 files;
  - `--params`: exit 0;
  - `--citations`: 0 flagged over 19 files;
  - `--gate`: agrees;
  - `--prose`: 0 flagged.
- No `src/` change, so DEC-141's tier two is not run.

### Accepts, clause by clause

- `streams()` checks the exit status of `bench`, `bench 12` and
  `search_bench.py` at both depths, in both scripts: yes, the body is
  S123's.
- Each bench stream must end on its total line and each search_bench stream
  must print three positions: yes. Each check is shown refusing its own
  plant.
- Raw output kept out of the compared directory: yes. The directory is
  emptied first and holds the four stripped streams; the raw output is in
  `inv6_raw/`.
- Each script refuses a planted failing engine, exit 1 and junk: yes, via
  `run`, before the code check and any pair.
- Each still passes its own zero against parent on real binaries, without a
  timing: yes, both identical at the recorded totals.
- `check` prints its DONE marker at the shas each script pins: yes, both.
- Each screen script's `part()` refuses an empty and a junk `full.k` before
  any fit, without a fit: yes, both, with `fits/` empty. `samples()` is
  fixed too.
- `touches:` true as filed: the four scripts and `adocs/data/README.md`.
