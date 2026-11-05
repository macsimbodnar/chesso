id:         S332
goal:       the `repetition:` check reads a full pool, one run or merged, without holding its rows in memory
accepts:    `adocs/data/S331_measure.py` `read` streams the views, or reads `games.tsv` and the root view alone, and its peak resident memory on S122's regenerated 150000-game pool (`.tuning/s122/corpus/`, read in place) is stated from a run, under 2 GB; its reading on that pool is recorded and agrees with the log's `repetition:` line, or the disagreement is the finding; `S122_corpus.sh merge` writes the merged pool's games file and one combined `repetition:` line beside the two parts' (DEC-296), and a smoke merge of two small runs shows `read` accepting it; S331's 2400-game reading re-run byte-equal by the new reader
touches:    adocs/data/S331_measure.py, adocs/data/S331_measure.sh, adocs/data/S122_corpus.sh, adocs/data/S332_*, adocs/data/README.md
excludes:   `src/`; `tools/datagen.cpp`; regenerating any corpus; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator, 2026-11-05
done:       2026-11-05 -- `accepts` by clause, this commit. `read` streams all three views a row at a time and keeps only the games and per-game counts: on `.tuning/s122/corpus/` read in place, 75 s and **107628 kB peak resident** at the final reader (`/usr/bin/time -v`; 62 s and 108016 kB before the fast check's fix, the same bytes), against HEAD's reader stopping on `MemoryError` at 2052676 kB under a 2 GiB cap, the observed red. Its line recomputed from the files, [81702, 34164, 20646, 13518, 11707733, 6798228, 25], **equals the pool log's `repetition:` line in all seven figures**; the reading is `adocs/data/S332_pool_reading.txt`. `S122_corpus.sh merge` writes `POOL/games.tsv` and one combined `repetition:` line, by `S331_measure.py merge`, beside part 1's recomputed figures and part 2's own, holds the games file to its line in `check_merged` and reads the merged pool with `read` before the log is renamed; red first, HEAD's merge of the smoke's two runs wrote no games file and HEAD's reader refused its log; at the step's scripts the same runs merge to views byte-equal to HEAD's merge's, a 60-line games file, and `read` accepting the pool (`adocs/data/S332_merge_smoke.txt`, fifteen cases beside it). A power-loss stop, part 1's `games.tsv` ending before games the merger keeps, merges with the views as ever and a `repetition incomplete:` line in place of the combined one, which `read` refuses as a partial check: case C, red at the scripts as first delivered and green after the fast check's fix. S331's 2400-game reading re-run by the new reader is **byte-equal** to `.tuning/s331/measure/reading.txt` and to `S331_reading.txt`'s copy, sha256 `27ec3b79…`; the opening branch it never takes is read on 324 games of the pool, the 25 openings among them, HEAD's reader and the new byte-equal in file order and shuffled (`adocs/data/S332_opening_subset.txt`). `test_s331_measure` and the five `test_plan_*` ctest entries green, the five `tools/plan_prose_check.py` modes exit 0. No `src/` or `tools/` change; `S331_measure.sh` unchanged. `DEV_MANUAL.md` and `MANUAL.md` checked, no change: neither describes the reader's memory or the merge's files.

## Why

Found 2026-11-04 by S122's DEC-349 implementer ("Findings beyond the task, not fixed" in S122's step file).
`--games-out` exists so a pool's `repetition:` line can be checked against its files, but `read_rows()` keeps every
row of the three views: about 56 M tuples with their FENs on 150000 games, inferred at well over the machine's 15 GB.
And after a stop, `merge` leaves part 1's `games.tsv` and the top-up's apart, part 1's closing lines lost, so a merged
pool has no one file to check. Filler by DEC-171: no run's labels depend on it; it is the check that they are right.

## 2026-11-05: implementation (Opus 5.5 subagent, briefed by the coordinator)

Worktree `/home/max/ws/chesso-s332` at `bcc41f3`, the machine to this step alone. Peak disk about 50 MB: the scratch under
`.tuning/s332/` (38 MB, gitignored) and a Release `build/` of `datagen` alone (10 MB). No corpus copied, the pool read in
place and never written.

### What changed

- **`adocs/data/S331_measure.py` `read` streams.** `stream_rows()` yields a row at a time; `read_rows()`, which held them
  all, is kept for `premise` alone, whose two runs are 24 games. `check_run()` streams each view the log counts once,
  counting its rows and checking every row's result against `games.tsv`, as before, and on the root view adds each row to
  a `Tally`: per game one list of counts (old rows, differs, to win, to loss, for each of the eight keys; the new rows;
  all rows) and five totals. `read_command()` prints from those counts by the same format strings and in the same
  order, so the sums and the clustered errors run over the same lists as before. Every view is still read, not the
  root view alone, so the result check on `qs` and `pv` stays.
- **`read OUT [LOG]`**: the log may be named, so the merge can read a pool against the log it is composing.
- **A merged pool is read.** A log with `merged <view>:` lines is the merge's: the views' rows from those lines, the
  settings from the two runs' logs it holds indented (nodes, threads, quiet limit and leaves held equal), the games
  from one `merged games:` line, and the one `repetition:` line at a line's start. Its first line names both seeds.
  A merged log with no `merged games:` line is refused as merged before S332: the void pool's in
  `.tuning/s122/void_twofold/corpus/` reads so.
- **`S331_measure.py merge`**, new: the merged pool's games file, as `OUT.merging`, and the two lines. Part 1's
  `games.tsv` is cut where its rows were: kept up to the first line naming a game its root rows hold and the merged root
  does not, so the game the merger dropped goes, with a NUL tail. It refuses unless every merged root game has one
  line, in the root's order, with the rows' result; every kept part 1 id is under the merger's offset; part 2's lines
  are its booked games; and part 2's lines and rows give its log's `repetition:` line. Part 1's figures are recomputed
  from its kept lines and rows; the combined line is those plus part 2's, in datagen's words. A part 1 game with no row
  before the cut is kept and counted apart, since the merger counts part 1's games by their rows. The written file is
  read back and removed on any refusal.
- **`adocs/data/S122_corpus.sh merge`**: both parts must have a `games.tsv`, checked before anything moves; part 1's
  moves as before and is required in `originals.sha256` on a resumed run. After the merger, `S331_measure.py merge` runs on
  `pool.tsv.root.merging`; the four files are renamed; `check_merged` now also holds `POOL/games.tsv` to its line
  (named, sha256, newline, line count); then `read` on the pool against `generate.log.merging`, its output in the log
  prefixed `read`. Any refusal removes this run's files and keeps the log as `merge.failed.log`, as before. The
  interrupted state and "never overwritten" name `games.tsv` too, and the log records the reader's sha256. `verify` on a
  merged pool inherits the games check. The pre-registration block is unedited. `S331_measure.sh` needed no change.
- **New:** `adocs/data/S332_merge_smoke.sh`, `adocs/data/S332_merge_smoke.txt`, `adocs/data/S332_pool_reading.txt`;
  `adocs/data/README.md` has their rows, and the rows of `S331_measure.py` and `S122_corpus.sh` say what changed.

### The readings

All with `/usr/bin/time -v`, `adocs/data/S331_measure.py` at sha256 `b3cf1c2c…`; HEAD's reader is `bcc41f3`'s copied to
`.tuning/s332/S331_measure_head.py`. Verbatim in `adocs/data/S332_pool_reading.txt`.

| run | command | wall | peak resident | result |
|---|---|---|---|---|
| HEAD's reader, the pool, 2 GiB cap | `(ulimit -v 2097152; nice -n 10 python3 .tuning/s332/S331_measure_head.py read /home/max/ws/chesso/.tuning/s122/corpus)` | 6 s | 2052676 kB | `MemoryError` in `read_rows()` on the root view, exit 1: the red |
| the new reader, the pool | `nice -n 10 python3 adocs/data/S331_measure.py read /home/max/ws/chesso/.tuning/s122/corpus` | 62 s | **108016 kB** | exit 0, the line equal to the files |
| the new reader, S331's run | `python3 adocs/data/S331_measure.py read /home/max/ws/chesso/.tuning/s331/measure` | 1 s | 17848 kB | byte-equal |
| HEAD's reader, S331's run | `python3 .tuning/s332/S331_measure_head.py read /home/max/ws/chesso/.tuning/s331/measure` | 1.4 s | 294084 kB | the same bytes |

HEAD's reader was not run uncapped on the pool: the finding's inference, well over 15 GB, would have cost the machine to
show.

### The full pool against its log

The log's line, `.tuning/s122/corpus/generate.log`: 81702 games recurred, 34164 of them ended other than drawn, 20646 won
by White and 13518 lost, 11707733 positions passed in them, 6798228 before the first recurrence, 25 openings ended on
one. The files give `[81702, 34164, 20646, 13518, 11707733, 6798228, 25]`, **equal in all seven figures**; 150000
games, 18786043 root rows, every row of the three views carrying its game's result. The reading beside it: 22.78 %
+/- 0.11 of the games relabelled from a draw, 19.602 % +/- 0.103 of the old rule's root rows carrying a label it
changed, against S331's 22.79 % +/- 0.86 and 19.740 % +/- 0.820 on 2400 games. Those are datagen's counts read off its
own files; this step judged no position.

### Peak RSS

**108016 kB, 105 MiB**, on the full pool, under the accepts' 2 GB. The rows are never held; what is
held is `games.tsv` as a dict and a 34-count list per game.

### The merge smoke

`adocs/data/S332_merge_smoke.sh`, consoles in `adocs/data/S332_merge_smoke.txt`. `S122_generate.sh` itself refuses
under 6 GB free (`FREE_GB_FLOOR`, fixed), and the machine had 3, so `runs` frames the two runs as its `corpus` and
`topup` frame them: the worktree's `build/tools/datagen` (S331's, sha256 `3aff1001…`), the corpus's settings, part 1
60 games under seed 990332000 stopped by SIGTERM to datagen 15 s in, `exit 143`, `S122-CORPUS-FAILED`. Its files hold
22 games; `S082_pool_merge.py part1` keeps 21 and drops game 14, whose games line had been written. Then the top-up,
39 games under seed 990332100, `exit 0`. 40 s on 12 threads. The seeds meet no run the corpus log's `seeds` lines name.

- **Red, at HEAD's scripts:** `S122_corpus.sh merge` passed, `S122-CORPUS-MERGE-DONE`, with three merged views and
  no `games.tsv`; HEAD's reader on the merged pool: `refused: ... generate.log: 0 datagen settings lines, not one`.
- **Green, at the step's scripts**, a fresh copy of the same runs: `S122-CORPUS-MERGE-DONE`. The merged views are the
  same bytes HEAD's merge wrote (`3b3c8a82…`, `7f946966…`, `01244c40…`). `games.tsv` has 60 lines: part 1's 21 kept,
  its 22nd, game 14's, dropped, then the top-up's 39 with ids plus 100. Part 1 recomputed gives 16 recurred, 7 not
  drawn, 2014 rows, 1326 before; the top-up's own line gives 23, 12, 3624, 1961, and its files agree. The combined
  line reads 39, 19, 8 won and 11 lost, 5638, 3287, 0; `check_merged` passes on all four files, and `read` inside the
  merge and again on the finished pool reads the line equal to the files.
- **Cases**, `cases`, each on an edited copy. `S331_measure.py merge`: as merged, accepted and byte-equal to the merge's
  file; a 4-byte NUL tail, accepted; a no-row game kept before the cut, accepted, 61 lines, "1 without". Refused: a kept
  game's line removed (`has no games line`), a partial last line, one result flipped, the top-up one line short, its
  log's line changed, and a kept id at the offset. `S122_corpus.sh`: a full merge with that no-row game, 61 lines and
  `read` accepting it; a merge refused, nothing moved, for the top-up and for part 1 without `games.tsv`; `verify`
  refused on one byte of the merged games file and passing on the pool as merged. The cases' first run stopped
  without a marker: a call missing a space, then `$4: unbound variable` under `set -u`. The call was fixed and an
  EXIT trap now prints `-FAILED` in that case.

### The byte-equal re-run

`python3 adocs/data/S331_measure.py read /home/max/ws/chesso/.tuning/s331/measure`, its OUT the same path string S331
read: `cmp` byte-equal to `.tuning/s331/measure/reading.txt` and to `adocs/data/S331_reading.txt` from its line
`S331 reading of` to its end; sha256 `27ec3b79fb637a085685be7c811552d1cdc4a6f2453df9e81d37cfa4509fb2b1` for both.

### Checks

`ctest --test-dir build -R 'test_s331_measure|test_plan_'` with `CLANG_FORMAT_MAJOR=22`: 6 of 6 passed (the
`test_s331_measure` wrapper test over `S331_measure.sh` and the reader, and the five `test_plan_*`).
`tools/plan_prose_check.py`'s five modes, each exit 0: `--prose` 0 sentences flagged, `--citations` 0 flagged over
20 files (this file 0 code citations, 2 document ungated), `--touches` 0 flagged, `--params` silent, `--gate` the
command agrees in both documents; the five together, exit 0. `bash -n` on both shell scripts.

### Findings beyond the step, not fixed

1. **The merged pool's `GAMES` counts part 1's games by their rows.** `S082_pool_merge.py merge` holds part 1's games
   with a row plus the top-up's booked games to `--games`. A part 1 game that wrote no row was played but is not
   counted, so a merged pool would hold that many more games than `GAMES`. `games.tsv` now makes such games visible,
   and `S331_measure.py merge` keeps and counts them apart. None in S122's pool, nor in the void pool's part 1. Low.
2. **`read` lets a non-number field end on a traceback**, a `ValueError` and exit 1, not a refusal. It was so before
   S332 and is kept so; the wrapper reads exit 1 as failure either way. Low.
3. **The interrupted-merge test's `compgen -G "$POOL/pool.tsv.*"` also matches `.merging` files.** A run stopped after
   the merger wrote them and before the rename is named as stopped after the rename. The recovery by hand is the same.
   This predates S332. Low.

## 2026-11-05: the coordinator's fast check (Opus 5.5 subagent, briefed by the coordinator)

**A power-loss stop leads to a part 1 `games.tsv` that ends before games the merger keeps, and the merge as first
delivered refused that pool where HEAD's merged it.** Datagen writes a game's root, qs and pv rows and then its games
line under one lock, so after a SIGTERM `games.tsv` trails the views by at most the games the merger drops. A power
loss, DEC-296's own case, keeps in each file whatever the OS had flushed, so lines for kept games can be gone. The
checker built that case on the smoke, game 14's line and kept game 17's removed. The coordinator's ruling: the merge
does not refuse on that ground; the views merge as at HEAD; the merged games file holds what part 1's has; no combined
`repetition:` line, a line counting the kept games without one instead; and `read` refuses to report the check as
complete. The corpus stays usable, only the check is partial.

- **The fix**, `S331_measure.py merge`: a part 1 row whose game has no line is no longer refused. The lines part 1
  has must be the first kept games' in their order, the shape a file cut short takes. A line gone from the middle
  (`kept_game_missing`) or out of order is still refused, and the message now says why. Part 1's figures are not
  recomputed then. `merged games:` adds ", N of the first with no games line", and a `repetition incomplete:` line
  replaces the combined one. `read` refuses a merged log holding that line: "the merged pool's repetition: check is
  partial and is not reported as complete". `S122_corpus.sh merge` notes the partial check in the log and runs no
  `read`; `check_merged` holds the games file to its line as before. Part 2 finished with `exit 0` and must still have
  every line. The SIGTERM path and every earlier output are unchanged: the smoke's merge console is the same but for
  its date.
- **Case C**, `power_loss` in `S332_merge_smoke.sh cases`. Red at the scripts as first delivered: `refused: merged
  root: game 17 has no games line in part 1's file`, `S122-CORPUS-MERGE-FAILED`. Green after the fix: the merge
  passes; the three views are byte-equal to the merge's without the loss; `games.tsv` has 59 lines; the log reads
  `repetition incomplete: 1 of part 1's 21 kept games have no games line ...` and no `repetition:` line; `read`
  refuses as partial. The fourteen earlier cases pass as before.
- **The opening branch**, `adocs/data/S332_opening_subset.sh` with `POOL=/home/max/ws/chesso/.tuning/s122/corpus`, 4 s,
  output `adocs/data/S332_opening_subset.txt`. The subset is 324 games, the 25 whose opening ended on a recurrence
  and every 500th other, with their 42514 root rows, 3659 of them the openings'. It sits under
  `.tuning/s332/opening_subset`, a few MB, removed at the end. Its `repetition:` line is computed by awk,
  [163, 56, 32, 24, 25007, 13889, 25], and both readers recompute it equal. HEAD's reader and the new are byte-equal
  in file order (`11622cd9…`) and with `games.tsv` and the root rows shuffled (`19deec1b…`).
- **Re-run at the final reader**, sha256 `a58bd9f5…`, beside S124's rebase build, one-minute load 1.1. The pool:
  75 s, 107628 kB peak resident, its reading byte-equal to the first pass's. S331's 2400-game run: byte-equal again,
  17856 kB.
- **Checks**: `test_s331_measure` and the five `test_plan_*` entries pass with `CLANG_FORMAT_MAJOR=22`, and the five
  `tools/plan_prose_check.py` modes exit 0. `bash -n` passes on the three shell scripts.
