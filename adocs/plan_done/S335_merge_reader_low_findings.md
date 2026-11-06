id:         S335
goal:       S332's three low findings in the corpus merge and its reader are answered: a part-1 game with no row is counted against `GAMES`, a malformed field is refused cleanly, and a stop before the merger's rename is described as one
accepts:    each finding has a fixture seen red at `S332`'s landing and green after: (1) `S122_corpus.sh merge` checks the merged pool's game count against `--games` counting a part-1 game that wrote no row, or states why the count is right as it is; (2) `S331_measure.py read` on a non-number field exits with a one-line refusal naming the file and line, not a traceback; (3) the interrupted-merge check tells a stop before the merger's rename from one after it, by the `.merging` files; `S332_merge_smoke.sh`'s 15 cases still pass and S331's 2400-game reading stays byte-equal
touches:    adocs/data/S331_measure.py, adocs/data/S122_corpus.sh, adocs/data/S332_merge_smoke.sh, adocs/data/S335_*, adocs/data/README.md
excludes:   `src/`; `tools/`; regenerating any corpus; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:       2026-11-06 -- `accepts` by clause, this commit. (1) The count stands, as the clause allows: the merger's `GAMES` is the top-up's booking and DEC-296's. A no-row game is in no view (`no_row_game`: views byte-equal to the plain merge's), the `merged games:` line counts it apart, and neither real run has one (0 of 150000, 0 of 2400). The comment above `merge()` says so. (2) `read` refuses a field not a number on one line naming file, line and field. Six cases: at `65463ae` and `d0cb418`, three end on a `ValueError` traceback and three refuse without a line; at the step's scripts all six refuse on one line naming `FILE:LINE`. (3) The interrupted-merge check tells the merger's `.merging` files alone from a renamed file. `stop_before`, a real SIGKILL at the merge's call of `S331_measure.py merge`, is refused as "stopped after" at both older scripts (red), and as "stopped before" at the step's; nothing is moved, and the named recovery gives the plain merge's pool. `stop_after` refuses as before. `S332_merge_smoke.sh`'s fifteen cases pass; `S122_corpus.sh smoke` passes 25 of 25 through `S333_guards.sh green`; S331's 2400-game reading is byte-equal (`27ec3b79…`). Gates: `bash -n`, `py_compile`, clang-format, the six ctest entries, and the five prose modes. No `src/` or `tools/` change. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Readings in `adocs/data/S335_findings.txt`. One fast check: nothing real, four lows; two fixed (the fixtures' line match made exact, `S331_measure.py`'s docstring narrowed to `read`), two listed as found (a stop inside the rename loop reads "after"; a stop between the view `.merging` removal and the log's rename reads "before" and its recovery loses only the diagnostic log). Also found, not fixed, all low: a non-UTF-8 byte ends `read` on a traceback; `merge` and `premise` still can on a game field; the stop cases read `/proc`; `merge` holds no lock.

## Why

Found 2026-11-05 by S332's implementer ("Findings beyond the step" in S332's done file), each rated low: none occurs
in either real pool, and none moves a label. (1) A merged pool's `GAMES` counts part 1's games only where they wrote a
row, so a part-1 game with no row would make the pool hold more games than `GAMES` says; S332's merge keeps such
games and counts them apart. (2) `read` ends on a traceback (exit 1) for a non-number field, as before S332. (3)
Before S332: the interrupted-merge check also matches the merger's `.merging` files, so a run stopped before the
rename is described as stopped after it; the manual recovery is the same. Filler by DEC-171.

## 2026-11-06: implementation (an Opus 5.5 subagent, briefed by the coordinator)

The main tree at `d0cb418`, beside S124's screen (its tuner at nice 10 on 12 threads). Scratch `.tuning/s335/`
(gitignored), holding a copy of the two small runs `S332_merge_smoke.sh runs` played on 2026-11-06 13:09 for S333's
regression, checked against their `pristine.sha256`. No datagen run, no match, fit or timing. Nothing under
`.tuning/s122/` was written; its pool was read once, by `cut` (finding 1, item 5).

### What changed

- **`adocs/data/S331_measure.py`, finding 2.** `read_games` checks that a games line's five fields read as numbers:
  the game and the three plies as integers, the result as a number. `stream_rows` checks a row's score, phase, ply
  and distance. A field that is not a number is refused by `not_a_number()` on one line:
  `refused: FILE:LINE: field N, the NAME, is 'x', not a number`. A row's game and result were already held to the
  games file, and refused without a line; that refusal now names the first such row:
  `...; the first, FILE:LINE: game 'x' has no games line`, or its result against its games line's. `premise` and
  `merge`'s read-back go through the same two readers. Valid files read as before: S331's reading is byte-equal.
- **`adocs/data/S122_corpus.sh` `merge`, finding 3.** The interrupted-merge test reads the names.
  - A merged file under its own name, a `pool.tsv.*` not ending `.merging` or `games.tsv`, means the run stopped after
    the rename began. It is refused with the message as before.
  - The merger's `.merging` files alone (`pool.tsv.*.merging`, `games.tsv.merging`) beside `generate.log.merging` mean
    the run stopped before the rename. It is refused with its own message: "so a run stopped before renaming its
    merged files: nothing of the merged pool was renamed", and "By hand, once no merge is running: remove
    POOL/*.merging, then re-run".
  - Both refuse and delete nothing. The comment above `merge()` describes the before state, why the script clears
    neither state, and, for finding 1, why `GAMES` counts part 1's games by their rows.
- **New:** `adocs/data/S335_findings.sh` (`red`, `green`, `reading`) and `adocs/data/S335_findings.txt`, its
  consoles. `adocs/data/README.md`: the `S122_corpus.sh` and `S331_measure.py` rows extended, two rows added.
- `S332_merge_smoke.sh` is unchanged. The new cases are in `S335_findings.sh`, which can also run them against an
  older pair of scripts; S332's smoke has no switch for that.

### Finding 1: the count stands, and why

The merger's `GAMES` is part 1's kept games with a row plus part 2's games played; `S082_pool_merge.py merge` refuses
unless they make `--games`. This step keeps that count and takes the clause's second branch. No code changed for it
but the comment above `merge()`.

1. **It is the count the top-up is booked by.** `adocs/data/S122_generate.sh` plays GAMES less `kept`, read from
   `S082_pool_merge.py part1`'s cut line, which counts games by their rows. A merge that counted a no-row game
   against GAMES would refuse every pool topped up so that has one, after its top-up has played, and the only remedy
   would be a second top-up booked by hand.
2. **It is DEC-296's.** The owner's top-up of S082 took part 1 as "the 122748 games before game 122926 ... complete
   in all three files", counted from the rows, and that merge's log reads "at least 3 of its games wrote no row"
   (`adocs/data/S082_pool.log`, line 68).
3. **No stage after the merge reads such a game.** It is in no view, and `verify`, `filter`, `split` (which pools
   the game numbers of the views' rows), `sample` and `k` read the views. Fixture `no_row_game`: the merge with one
   passes, its three views are the plain merge's bytes, and its games file is the plain one plus that game's line.
4. **The pool's log says it.** The `merged games:` line counts the game apart: "61 games written to ...; part 1's 21
   games with a row before its cut and 1 without, and part 2's 39 games played", beside the merger's "7909 rows from
   60 games".
5. **No real run has one.** S122's pool has 150000 games lines and 150000 games with a root row; S331's run 2400 and
   2400 (`cut -f5 pool.tsv.root | uniq | sort -u | wc -l`, read only).

Counting games played instead changes the top-up's booking (`S122_generate.sh`), the merger's check
(`S082_pool_merge.py`) and DEC-296's definition. All three are outside this step's touches; it is listed below as a
proposal.

### The fixtures, red and green

Every case by `adocs/data/S335_findings.sh` with `DIR=/home/max/ws/chesso/.tuning/s335/smoke`, on a fresh copy of
the runs under `DIR/s335/<stage>/<case>`; the consoles are verbatim in `adocs/data/S335_findings.txt`. Red is read at
S332's landing, `adocs/data/S335_findings.sh red` (`65463ae`'s two scripts written beside the tree's and removed after),
and again at HEAD as the step started, `BEFORE=d0cb418 adocs/data/S335_findings.sh red`. The two read the same: their
`merge()` and `check_merged()` are line for line the same, and `S331_measure.py` is byte-equal at both (`a58bd9f5…`).
The corpus scripts differ elsewhere, by S333 (`5a5faf7d…` and `bde5e485…`). Green is `adocs/data/S335_findings.sh
green`, at the step's scripts (`371a3035…`, `d8243326…` after the fast check's docstring fix).

| case | finding | setup | at `65463ae` and `d0cb418` | at the step's scripts |
|---|---|---|---|---|
| `plain` | reference | the runs merged as played | passes, 60 games | the same |
| `no_row_game` | 1, evidence | a line `99 0.5 8 -1 8` after the last kept game's in part 1's games file | passes: views byte-equal to `plain`'s; games file 61 lines, `plain`'s and game 99's; top-up booked 39 = 60 - 21 | the same |
| `read_games_id` | 2 | games file, line 1, field 1 set to x | one line, "66 rows of pool.tsv.root carry a result not their game's": no line named | `refused: games.tsv:1: field 1, the game, is 'x', not a number` |
| `read_games_result` | 2 | line 1, field 2 | the same 66-rows line | `games.tsv:1: field 2, the result, ...` |
| `read_games_ply` | 2 | line 2, field 3 | a 16-line traceback, `ValueError` | `games.tsv:2: field 3, the first ply, ...` |
| `read_view_score` | 2 | the qs view, line 5, field 3 | a 15-line traceback | `pool.tsv.qs:5: field 3, the score, ...` |
| `read_view_game` | 2 | the root view, line 7, field 5 | one line, "1 rows of pool.tsv.root carry ...": no line named | `...; the first, pool.tsv.root:7: game 'x' has no games line` |
| `run_view_ply` | 2 | the top-up read as one run, against its own log; the pv view, line 3, field 6 | a 15-line traceback | `pool.tsv.pv:3: field 6, the ply, ...` |
| `stop_before` | 3 | the merge killed at its call of `S331_measure.py merge`: three `pool.tsv.*.merging` and `generate.log.merging`, nothing renamed | refused as "stopped after renaming its merged files": **red** | refused as "stopped before renaming its merged files", nothing moved; `rm POOL/*.merging` and the merge passes, views and games file byte-equal to `plain`'s |
| `stop_after` | 3, control | the same at its call of `read`: views and games file renamed, `generate.log.merging`, no log | refused as "stopped after", nothing moved; the named recovery passes, byte-equal | the same |

A read case passes on exit 1 with one line, `refused: ...`, naming `FILE:LINE`. In a stop case, the merge runs with a
`python3` first on its PATH. That wrapper runs the real interpreter for every call but the named one. At the named
call it sends SIGKILL to each process up its parent chain that runs the corpus script's merge, as a power loss stops
it, so the case reads the state a real stop leaves rather than one built by hand. Both `red` runs pass, seven of seven
red and three of three passing; `green` passes ten of ten.

### Regressions

- `S332_merge_smoke.sh` `merge`, then `cases`, with DIR as above, at the step's scripts: both end `-DONE`. The fifteen
  cases read as S332's: `merge_no_row_game` passes with 61 lines, and `power_loss` merges and is refused by `read` as
  partial.
- `S333_guards.sh green` with `S=/home/max/ws/chesso/.tuning/s335/s333`, which runs `S122_corpus.sh smoke`: 25 of 25,
  `S122-CORPUS-SMOKE-DONE`.
- **S331's 2400-game reading, byte-equal**, by S332's check: `adocs/data/S335_findings.sh reading` runs
  `python3 adocs/data/S331_measure.py read /home/max/ws/chesso/.tuning/s331/measure` (the path string S331 read) and
  `cmp`s it with `.tuning/s331/measure/reading.txt` and with `adocs/data/S331_reading.txt` from its `S331 reading of`
  line on. All three hash to sha256 `27ec3b79fb637a085685be7c811552d1cdc4a6f2453df9e81d37cfa4509fb2b1`.

### Gates

- `bash -n` on `S122_corpus.sh`, `S332_merge_smoke.sh` and `S335_findings.sh`; `python3 -m py_compile` on
  `S331_measure.py`.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check`, exit 0.
- `ctest --test-dir build -R "test_s331_measure|plan_" -j6`: 6 of 6, `test_s331_measure` and the five plan tests.
- `tools/plan_prose_check.py` in its five modes, each exit 0 and the five together exit 0: `--prose` 0 sentences
  flagged; `--citations` 0 over 20 files, this file 0 code citations; `--touches` 0; `--params` silent; `--gate`
  the command agrees in both documents.
- No `src/` or `tools/` change, so no full ctest. `DEV_MANUAL.md` and `MANUAL.md` checked, no change: neither
  describes `read`'s refusals or the merge's interrupted states. DEV_MANUAL's datagen section names
  `S331_measure.sh` and the `--games-out` format only.

### Choices beyond the letter of `accepts`

1. **Finding 1 takes the second branch**: the count is stated right, not refused. A refusal would break S332's
   `merge_no_row_game`, which must keep passing, and would strand a topped-up pool.
2. **Finding 3 refuses the before state; it does not clear it.** The merge's own start removes stale `.merging`
   files, so a re-run could go on. But a refusal cannot tell a stopped run from one still running, and going on would
   delete the second's files under it.
3. **Finding 2 covers every field datagen writes as a number in the files `read` reads.** The game and result fields
   were refused already, but without a line; the first such row's line is now named.

### Findings beyond the step, not fixed

1. **`read` ends on a `UnicodeDecodeError` traceback, exit 1, on a byte that is not UTF-8.** Seen on a copy with
   `0xff` written into its qs view. The text reader decodes in chunks, so naming the exact line would need the bytes
   read a line at a time. Low: datagen writes ASCII.
2. **`S331_measure.py merge`'s `root_games` ends on a `ValueError` traceback** when part 1's root view has a game
   field that is not a number. Seen on a copy, called direct. Inside `S122_corpus.sh merge` it is never reached:
   `S082_pool_merge.py part1` refuses such a row first. Low.
3. **Proposal, should a merged pool's `GAMES` mean games played.** The top-up would be booked as GAMES less part 1's
   kept games counted from its games file, with a row or without. The merger's check would count the no-row games,
   and DEC-296 would be amended. No run needs it today: neither real run has such a game.
4. **`S335_findings.sh`'s two stop cases read `/proc`, so they run on Linux only.** The corpus script's own
   `stat -c` is Linux-only too.
5. **`merge` holds no lock.** Found by reading, not run. A second merge started while a first sits between composing
   its `generate.log.merging` and its merger's first write passes the interrupted-merge test. It then removes and
   rewrites that log under the first. This predates S335 and needs two merges started by hand at once. Low.
6. **Found by the coordinator's fast check, not fixed, both low:** a stop partway through the rename loop is reported
   as "after"; a stop between the removal of the views' `.merging` files and the log's rename is reported as "before",
   which loses only the diagnostic log.

### 2026-11-06: the coordinator's fast check, two trivial fixes

Nothing real found. Finding 1's second branch accepted.
- The read cases of `S335_findings.sh`: the line check was a substring match, so `:1` also accepted `:10` to `:19`. It now
  matches `FILE:LINE: ` exactly.
- `S331_measure.py`'s docstring claimed every non-number field is refused on one line. It now says that of READ, and
  that MERGE and PREMISE read a row's game as a number unchecked and can still end on a traceback there. The README
  row says the same.
- Re-run on the final scripts, consoles replaced in `adocs/data/S335_findings.txt`: both `red` runs and `green` 10 of
  10, `reading` byte-equal (`27ec3b79…`), `S332_merge_smoke.sh` merge and its fifteen cases, `S333_guards.sh green`
  25 of 25.

## Proposed done stamp (the coordinator writes the final one)

2026-11-06 -- `accepts` by clause, this commit.
(1) The count stands, as the clause allows: the merger's `GAMES` is the top-up's booking and DEC-296's. A no-row
game is in no view (`no_row_game`: views byte-equal to the plain merge's), the `merged games:` line counts it apart,
and neither real run has one (0 of 150000, 0 of 2400). The comment above `merge()` says so.
(2) `read` refuses a field not a number on one line naming file, line and field. Six cases: at `65463ae` and
`d0cb418`, three end on a `ValueError` traceback and three refuse without a line; at the step's scripts all six
refuse on one line naming `FILE:LINE`.
(3) The interrupted-merge check tells the merger's `.merging` files alone from a renamed file. `stop_before`, a real
SIGKILL at the merge's call of `S331_measure.py merge`, is refused as "stopped after" at both older scripts (red),
and as "stopped before" at the step's; nothing is moved, and the named recovery gives the plain merge's pool.
`stop_after` refuses as before.
`S332_merge_smoke.sh`'s fifteen cases pass; `S122_corpus.sh smoke` passes 25 of 25 through `S333_guards.sh green`;
S331's 2400-game reading is byte-equal (`27ec3b79…`). Gates: `bash -n`, `py_compile`, clang-format, the six ctest
entries, and the five prose modes. No `src/` or `tools/` change. `DEV_MANUAL.md` and `MANUAL.md` checked, no change.
Readings in `adocs/data/S335_findings.txt`.
