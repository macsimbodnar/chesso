id:         S333
goal:       S122's corpus preparation refuses a pool written before S331, and every `.done` marker names the pool it was prepared from
accepts:    `S122_corpus.sh verify` refuses a one-run pool whose `generate.log` holds no `repetition:` line, as `merge` already refuses a top-up's, the refusal seen on a fixture log; each `prepared/` `.done` marker records the pool's `generate.log` sha256 beside its own file's, and `filter`, `split`, `sample` and `k` refuse a marker whose pool is not the one `POOL` names now, each refusal seen on a planted swap; the regenerated pool's preparation (`.tuning/s122/prepared/`) read by the new guards, or the step says why it is left as it stands; `bash -n` and `S122_corpus.sh smoke` green
touches:    adocs/data/S122_corpus.sh, adocs/data/S333_*, adocs/data/README.md
excludes:   `src/`; `tools/`; re-running the preparation over the 150000-game pool; any fit
decisions:  DEC-171, DEC-296, DEC-349
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:       2026-11-06 -- `accepts` by clause, this commit. `verify` refuses a one-run pool whose `generate.log` holds no `repetition:` line (case `verify_no_repetition`, on a fixture log), and an OUT holding another pool's preparation before it removes anything; every `.done` marker and `verify.txt` carry a `pool` line, `generate.log`'s sha256; `filter`, `split`, `sample`, `k` and `record` refuse a planted swap (`filter_swapped`, `split_swapped`, `sample_swapped`, `k_swapped`, `all_swapped`, `record_swapped`) and a missing predecessor (`sample_unmarked`); a new `adopt` stage stamps an existing preparation only after checking the pool's log, `verify.txt`'s views, each marker, the band filter recomputed, and `prepared.txt`'s corpus, held-out views and K (the training views, `split.txt` and the draws are not checked, and the docs say so). **The regenerated preparation is read by the guards**: refused by all five stages as made before S333 in a mirror, then stamped in place by the coordinator (`ADOPT_DRY=1 ... adopt` passed, `adopt` stamped five records, `k` read K 1.2302, the value on record; the old records backed up in `.tuning/s333/prepared_backup/`). `bash -n` both scripts; `smoke` 25 of 25, 23 of them red on `9daee21`'s script, both controls green; seven mutants each killed by its own case; S332's merge smoke green against the new script. Smoke now slices S331's measured run, since the old `.tuning/s122/smoke` source predates S331 and the new `verify` refuses it (its figures are not readings). One fast check, one real finding (no smoke stage read an adopted `verify.txt`) and five low, all fixed. Fast suites 59 of 59 in both builds, format clean, the five prose modes exit 0; no `src/` or `tools/`; `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Left for S336: `split.txt` and `k.txt` bound only through the records beside them, and `seeds` refusing because the corpus meets itself through `prepared/verify.txt` (before S333). Also found: plain `nohup setsid ... &` gives a dead `$!` under job control; `.moltke.local.md` now gives `setsid -w`.

## Why

Found 2026-11-04 by S122's DEC-349 implementer (S122's step file, "What still guards only by the move" and "Findings
beyond the task, not fixed"). The void two-fold pool was kept out of the regenerated preparation only by moving its
directories aside: `verify` takes a pool whose datagen predates S331, and `prepared/`'s markers hold their own file's
sha256, not the pool's. A later corpus built with these scripts has no guard against the same mix. Filler by DEC-171.

## 2026-11-06: implementation (an Opus 5.5 subagent, briefed by the coordinator)

The main tree at `9daee21`, the machine otherwise idle. Scratch `.tuning/s333/` (gitignored). Nothing under
`.tuning/s122/` was written; its preparation was read in place, and adopted only in a mirror (below).

### What changed

**`adocs/data/S122_corpus.sh`**, as first delivered, sha256 `f0073953…` (the fast check's fixes, below, follow it):

- **`verify`** keeps its log checks in `check_pool_log`, which `adopt` shares. New in it: a one-run pool's
  `generate.log` must hold a line starting `repetition: `, which only S331's datagen prints. Otherwise:
  "refused: ... has no 'repetition:' line: its datagen predates S331, whose two-fold draws DEC-349 voids". This is
  `merge`'s refusal of such a top-up, applied to a pool that finished in one run. A merged pool is read as before:
  `check_merged`, its parts held to S331 at the merge.
- **The `pool` line.** `verify.txt` now carries `pool      sha256 <generate.log's sha256>  <POOL>/generate.log` as
  its second line (`binary_line`'s form). Each `.done` marker, written by `write_marker` by way of a `.tmp`, holds
  its file's sha256 on line 1, as before, and that `pool` line on line 2.
- **`check_prepared`**, run by `filter` (after `verify.txt`'s presence), `split`, `sample`, `k` and `record`
  before any of them writes. Each of `verify.txt` and the four markers present must name, by sha256, the
  `generate.log` POOL holds now. A file that names another pool is refused: "was prepared from the pool whose
  generate.log has sha256 X, and ...'s is Y". A file with no `pool` line is refused too: "names no pool: written
  before S333; `adopt` stamps ...". All files are read, so one stale file is enough to refuse.
- **`adopt`**, a new stage, for a preparation made before S333. It writes nothing until every check passes:
  - POOL passes `check_pool_log`;
  - `verify.txt`'s engine, datagen and command lines are POOL's log's, and the sha256 it records of each view is the
    sha256 of POOL's view now;
  - each marker's line 1 is its file's sha256, and the file is no older than POOL's `generate.log`;
  - each filtered view equals the band filter run again over POOL's view, as a hash. That is `filter`'s own awk
    program, now `band_filter`, byte-equal to what it was, writing to `/dev/stdout` piped into `sha256sum`, with no
    file written;
  - if `prepared.txt` exists, it names POOL and `corpus.tsv`'s sha256.

  Only after that does each file take the `pool` line and an `adopted <date>` line. A file already naming this pool is
  left as it is, and one naming another is refused, never restamped. `ADOPT_DRY=1` makes every check and writes
  nothing.
- **`smoke`.** It now slices S331's measured run, `.tuning/s331/measure`: 2400 games at the corpus's settings by S331's
  datagen, seed 2026400331. The research pass's sample at `.tuning/s122/smoke/` came from a datagen before S331, so the
  new `verify` refuses it. A synthesized log carrying a `repetition:` line it never had would have been the only way to
  keep it.

  The slice's log is the source log's `engine|datagen|command|repetition` lines under a first line saying that they
  describe the whole run. The six stages pass on the slice: 6323 rows, 139 games, K 1.2376, against phase 1's 8333
  rows and K 0.8306 on the old source (neither is a reading). A second slice, the swap, is cut from rows (19000, 38000].

  After the stages, 18 cases run, each on its own copy under `OUT/cases/`, as a child of `SMOKE_AGAINST`, by default
  the script itself. 14 cases are refusals, 2 check `adopt` passing (dry and for real), and 2 are controls. A refusal
  must end `-FAILED` and leave its `prepared/` byte-equal, except `verify.txt` under `all`. `bash -n` passes.
- **The pre-registration** gets a dated amendment, "AMENDED 2026-11-06, S333", stating that the guards change and the
  recipe does not. THE SMOKE paragraph is kept, with a dated note beside it (DEC-348).

**New:** `adocs/data/S333_guards.sh` (`red`, `green`, `real`) and `adocs/data/S333_guards.txt`, its consoles.
**`adocs/data/README.md`**: the `S122_corpus.sh` row extended, and the two S333 rows added. **`DEV_MANUAL.md` and
`MANUAL.md` checked: no change.** Neither names the corpus script, `prepared/` or its markers.

### Three choices this step made, beyond the letter of `accepts`

1. **`verify.txt` names its pool, and `filter` refuses a `verify.txt` of another pool** (case `filter_unverified`).
   Without that, `filter` run alone after a swap would filter a pool `verify` never read, possibly one before S331. It
   would then stamp the markers with that pool's hash, and the markers would be true and the pool unverified.
2. **`record` is guarded too** (case `record_swapped`). It is the stage that writes `pool $POOL` into
   `prepared.txt`, which `S122_screen.sh` and the census scripts read.
3. **Clause 3: adopt, not "left as it stands".** `.tuning/s122/prepared/` is refused by every guarded stage as it is,
   which fails closed. Re-running the preparation is excluded. A file hash alone binds a marker to its file and not
   to the pool, so `adopt` recomputes the filter instead. This reads the 150000-game pool, well under a minute, and
   writes nothing; it is not the preparation run again. `corpus.tsv`'s marker rests on `prepared.txt` and the file's age,
   because proving its sample would be the preparation run again; `adopt` says so in its output.

### Clause 3: the regenerated pool's preparation, read by the guards

`adocs/data/S333_guards.sh real`, 91 s, writing nothing under `.tuning/s122`:

- **In place.** `filter`, `split`, `sample` and `k` on `.tuning/s122/prepared` each end `-FAILED`, naming
  `verify.txt` and all four markers as "names no pool: written before S333". The directory's manifest (39 lines: every
  file's name, size and mtime, and the sha256 of each file under 1 MB) is equal before and after, and again at the
  end of the stage. `record` was not run in place, because it rewrites `prepared.txt` on a pass.
- **In a mirror**, `.tuning/s333/real_mirror`: copies of the files under 1 MB, symlinks to the rest. No stage writes
  a large file while its marker is present.
  - `ADOPT_DRY=1 adopt` passes every check and leaves the mirror's manifest equal. The checks it passes: the three
    views are POOL's by sha256; each filtered view is the band filter of POOL's view, recomputed; and `corpus.tsv` is
    its marker's, with `prepared.txt` naming it and the pool.
  - `adopt` then stamps all five files. The pool's `generate.log` has sha256 `899cab6d…`.
  - After the stamp, `filter`, `split`, `sample`, `k` (`K before: 1.2302`) and `record` all pass on the mirror.
    `record`'s `prepared.txt` equals the real one, apart from its date and its paths.
- **`verify`** on the pool, written into `.tuning/s333/real_verify`, passes: the pool's log holds its `repetition:`
  line. Its three view lines equal the real `verify.txt`'s.

**For the coordinator: stamp the real preparation**, under a minute each (the `real` stage's two adopts and its
`verify` took 91 s together), no other file touched:

```
cd /home/max/ws/chesso
ADOPT_DRY=1 adocs/data/S122_corpus.sh adopt   # S122-CORPUS-ADOPT-DONE, nothing written
adocs/data/S122_corpus.sh adopt               # stamps verify.txt and the four markers in .tuning/s122/prepared
adocs/data/S122_corpus.sh k                   # K before: 1.2302, S122-CORPUS-K-DONE
```

Not stamping it is also safe: every stage refuses the preparation, and nothing but this script reads its markers or
`verify.txt` (the screen and census scripts read `prepared.txt`, which `adopt` leaves alone).

### Every refusal: its command and its reading

All 18 cases run together with `TUNING=/home/max/ws/chesso/.tuning/s333/tuning adocs/data/S122_corpus.sh smoke`, which
`adocs/data/S333_guards.sh green` wraps. Each one is
`env [ADOPT_DRY=1] POOL=<pool> OUT=$O/cases/<case>/prepared TUNING=... GAMES=2400 VALIDATION_GAMES=20 bash
adocs/data/S122_corpus.sh <stage>`, where `$O` is the smoke's OUT, `.tuning/s333/tuning/s122/phase1/corpus_smoke`. A
is `$O/pool`, the slice, whose log has sha256 `90d1bbca…`. B is `$O/swap`, whose log has sha256 `ee3b7c3e…`. The setups
are the script's own functions. Readings are from the green run, in `S333_guards.txt`:

| case | stage, POOL | setup (the plant) | reading |
|---|---|---|---|
| `control_all` | `all`, A | a copy of the prepared slice | passes: root filtered before, split before, sampled before, K before |
| `verify_no_repetition` | `verify`, its own | the slice's views, its log without the `repetition:` line | refused: `.../generate.log has no 'repetition:' line: its datagen predates S331` |
| `all_swapped` | `all`, B | a copy | verify passes on B, then refused: `pool.tsv.root.done was prepared from the pool whose generate.log has sha256 90d1bbca…, and $O/swap/generate.log's is ee3b7c3e…` |
| `filter_swapped` | `filter`, B | a copy, then `verify` on B | refused: `pool.tsv.root.done was prepared from ...` (and qs, pv, corpus) |
| `split_swapped` | `split`, B | the same | refused, the same four |
| `sample_swapped` | `sample`, B | the same | refused: `corpus.tsv.done was prepared from ...` |
| `k_swapped` | `k`, B | the same | refused, the same |
| `record_swapped` | `record`, B | the same | refused, the same; `prepared.txt` unchanged |
| `filter_unverified` | `filter`, B | A's `verify.txt` alone | refused: `verify.txt was prepared from the pool ...` |
| `old_marker` | `filter`, A | markers cut to line 1, `verify.txt`'s pool line removed | refused: `pool.tsv.root.done names no pool: written before S333` |
| `adopt_dry` | `adopt`, A, `ADOPT_DRY=1` | the old form | passes, `prepared/` unchanged: `would stamp verify.txt pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done corpus.tsv.done; nothing written` |
| `adopt` | `adopt`, A | the old form | passes: each view "the band filter of ..., recomputed", five files stamped |
| `adopted_reads` | `all`, A | the old form, then `adopt` | passes: root filtered before, K before |
| `adopt_swapped` | `adopt`, B | the old form | refused: `verify.txt's root view sha256 'ad938191…' is not $O/swap/pool.tsv.root's` |
| `adopt_tampered` | `adopt`, A | the old form, a newline appended to `pool.tsv.qs` | refused: `pool.tsv.qs is not the file its marker pool.tsv.qs.done records` |
| `adopt_forged` | `adopt`, A | the old form, `pool.tsv.root` short its last row and its marker rewritten to match | refused: `pool.tsv.root is not the band filter of $O/pool/pool.tsv.root, recomputed: sha256 591df5dc…` |
| `adopt_aged` | `adopt`, A | the old form, `corpus.tsv` touched to 2000-01-01 | refused: `corpus.tsv is older than $O/pool/generate.log, so it was not made from this pool` |
| `adopt_restamp` | `adopt`, B | a copy, stamped by A | refused: `verify.txt names the pool whose generate.log has sha256 90d1bbca…, not $O/swap's ee3b7c3e…: never restamped` |

### Red first

`adocs/data/S333_guards.sh red` writes `9daee21`'s `S122_corpus.sh` (sha256 `5a5faf7d…`) beside the script as
`.S333_before_corpus.sh`, runs the new smoke with `SMOKE_AGAINST` set to it, and removes it afterwards. All 16 cases
other than the controls fail on it:

- **Not refused.** `verify_no_repetition`, `all_swapped`, `filter_swapped`, `split_swapped`, `sample_swapped`,
  `k_swapped`, `record_swapped`, `filter_unverified` and `old_marker` each end `-DONE`: verify passes, "filtered
  before", "split before", "sampled before", "K before". `record_swapped` and `all_swapped` also rewrite
  `prepared.txt`, naming B.
- **No such stage.** The seven `adopt*` cases end on the usage line.

Both controls pass on it, the smoke reads "18 cases, 2 as they must be, 16 not", and it ends
`S122-CORPUS-SMOKE-FAILED`. The red stage itself ends `S333-GUARDS-RED-DONE`.

**Regression, the merged path.** `adocs/data/S332_merge_smoke.sh` `runs`, `merge` and `cases` were run against the new
script, with `DIR` in the session's scratchpad outside `.tuning` so that `seeds` never reads it. All three ended
`-DONE`. Among the cases, `verify_as_merged` passes and `verify_games_changed` is refused, as at S332.

### Gates

- `CLANG_FORMAT_MAJOR=22`: `cmake --build build -j12 && ctest --test-dir build -L fast -j10 && cmake --build
  build-tune -j12 && ctest --test-dir build-tune -L fast -j10 && ./clang-format.sh --check` exits 0, with 59 of 59
  passing in each build.
- `tools/plan_prose_check.py` `--touches`, `--citations`, `--params`, `--gate` and `--prose` each exit 0, run again
  once this section was written.
- `bash -n` passes on `S122_corpus.sh` and `S333_guards.sh`.
- No `src/` or `tools/` change.

### Findings beyond the step, not fixed

1. **A stage's own "before" outputs are bound to the pool only through the markers beside them.** These are
   `split.txt` with the views it split, and `k.txt`. If a marker is deleted by hand and the stages run again on a
   swapped pool, the result is a mixed preparation: `filter` filters the new pool and stamps fresh markers, then `split`
   reads "split before" from the old `split.txt`. It needs a hand edit, so it is low. A fix: a stage that runs removes
   the outputs of the stages after it, or `split.txt` and `k.log` carry the `pool` line too.
2. **`S122_corpus.sh seeds` refuses now: the corpus meets itself.** The corpus's own settings line (seed 2026500349)
   is in `.tuning/s122/prepared/verify.txt`, and `seeds` leaves out POOL, TOPUP and SAMPLE_DIR but not OUT. This
   predates S333: the line has been there since `all` ran on 2026-11-04. This step's `.tuning/s333/real_mirror/` and
   `real_verify/` add two more copies of the same run, but no new range. It bites only when the seeds of a run that has
   already played are checked again. Low.
3. **`nohup setsid ... & echo $!` does not give the run's PID in this harness.** The shell runs with job control, so
   the job leads its own process group, `setsid` forks, and `$!` names a parent that exits at once. Seen here: the
   `real` run's `$!` was gone within 10 s while its child ran on, so a watcher's `kill -0 "$PID"` exits 3, "process
   died", straight away. `setsid -w` kept `$!` alive in a test (checked at 0.5 s only). This affects the WATCHERS
   form and `.moltke.local.md`'s `nohup setsid nice -n 10`.
4. **`adopt`'s age rule refuses a genuine preparation if the pool was copied without `-p`.** The copy's
   `generate.log` is newer than the files. This fails closed. Moving the pool (`mv`) keeps the mtime and passes.
5. **`verify` checks that the `repetition:` line is present, not what it says.** `S331_measure.py read` checks the
   figures (75 s on the full pool, S332), and `verify` does not call it.

## 2026-11-06: the coordinator's fast check, six findings, fixed

The fast check found no blocker, one real finding and five low ones. All six are fixed in the same tree, uncommitted.
The section above describes the first delivery and is kept as written. `S333_guards.txt` now holds the readings
taken after the fixes.

- **Scripts.** `adocs/data/S122_corpus.sh` sha256 `bde5e485…`. `adocs/data/S333_guards.sh` sha256 `adadcdd2…`, which
  gains the `mutants` stage. In its `real` stage every stage now runs on the mirror alone.

### The findings and what changed

1. **(real) No smoke stage read an adopted `verify.txt`.**
   - `adopted_reads` ran `all`, and `all`'s `verify` deleted and rewrote `verify.txt` before `filter` read it.
     `adopted_reads` now runs `k`, which needs every earlier record and so reads the adopted `verify.txt`, with its pool
     line after the view lines and its `adopted` line.
   - Red: mutant `verify_stamp` makes `adopt` write `verify.txt`'s line as `pool      <sha>`, without `sha256`, which
     `named_pool` does not read. The smoke fails on `adopted_reads` alone: `k` ends `S122-CORPUS-K-FAILED`, 24 of 25 as
     they must be.
2. **(low) `verify` overwrote another pool's `verify.txt`.**
   - `verify` now runs `check_pool_log` and then `check_prepared` before it removes anything. An OUT that holds any
     file naming another pool, or none, is refused untouched. That includes a preparation made before S333, whose
     `verify.txt` is the one that `adopt` must read.
   - `all_swapped` now refuses at `verify`, so the smoke's `all` exemption for `verify.txt` is dropped: every refusal's
     `prepared/` is held byte-equal in full.
   - The plant for the four single-stage swap cases is now a `verify.txt` whose pool line is rewritten to B's. This
     isolates the markers' refusal, since `verify` no longer writes over A's preparation.
   - On the real preparation's mirror, `verify` refuses all five files as made before S333 and leaves the mirror
     unchanged.
3. **(low) `adopt`'s check of `corpus.tsv.done` could not tell two pools at one path apart.** It now has its own
   function, `adopt_corpus`:
   - the view markers, `split.txt`, `prepared.txt` and `k.txt` must be present; a missing `prepared.txt` is refused;
   - `prepared.txt` must name POOL and `corpus.tsv`'s sha256;
   - each `view <v> <path> <sha>` line must be that `.validation` file's sha256 now, and the file no older than POOL's
     `generate.log`;
   - its `k` must be `k.txt`'s.
   New cases: `adopt_validation` (a newline appended to `pool.tsv.qs.validation`), `adopt_no_prepared` and `adopt_k`.
   Each is killed by its own mutant (`prepared_view`, `no_prepared`, `k`).
4. **(low) The docs claimed more than the code checks.** The header, the comment above `adopt()`, the pre-registration
   amendment and the README row now list exactly what `adopt` checks and what it does not.
   - Not checked: the training views, `split.txt`'s contents, and whether the split and the sample were drawn from
     these files. Recomputing them would be the preparation run again.
   - Why that is enough: no stage after `sample` reads the training views. The screen reads `corpus.tsv`, the
     held-out views and K through `prepared.txt`, and all three are now held to it.
5. **(low) Three `adopt` refusals had no case to kill them.** One case each, each red first on a mutant that turns
   its refusal off, and each killed alone:
   - `adopt_pool_lines` (mutant `pool_lines`): a pool of A's views whose log's command line is edited, its mtime set to
     A's. It is refused with "verify.txt's pool lines are not ...". It is also refused on `prepared.txt`'s path, but the
     case asks for the pool-lines line.
   - `adopt_prepared_sha` (mutant `prepared_sha`): `prepared.txt`'s `sha256` line zeroed. Under the mutant, `adopt`
     passes and stamps.
   - `adopt_pre_s331` (mutant `pool_log`): A's views, with the log's `repetition:` line removed. It is refused with
     "has no 'repetition:' line".
6. **(low) Stages ran without their predecessors' records.** New `needs` checks each stage's earlier records:
   | stage | needs |
   |---|---|
   | `filter` | `verify.txt` |
   | `split` | + the three view markers |
   | `sample` | + `split.txt` |
   | `k` | + `corpus.tsv.done` |
   | `record` | + `k.txt` |

   Each stage refuses with "no X: run Y first", then runs `check_prepared`. Only `verify`, the first stage, runs on an
   empty OUT. `seeds` and `merge` do not read OUT. Case `sample_unmarked`: every marker and `verify.txt` removed by
   hand, `POOL=B sample`, refused with "no .../verify.txt: run verify first". At `9daee21` the same command samples and
   writes a marker.

### The readings (`adocs/data/S333_guards.txt`)

- **`green`**: the smoke passes 25 of 25: 21 refusals, `adopt` dry and for real, and 2 controls.
- **`red`**, against `9daee21`'s script:
  - 23 of 25 fail, and `control_all` and `adopted_reads` pass;
  - the 10 cases on stages that exist end `-DONE`, `sample_unmarked` among them;
  - the 13 `adopt*` cases end on the usage line.
- **`mutants`**: each of the seven is killed by its own case alone, every other case as it must be:

  | mutant | killed by |
  |---|---|
  | `verify_stamp` | `adopted_reads` |
  | `pool_lines` | `adopt_pool_lines` |
  | `prepared_sha` | `adopt_prepared_sha` |
  | `pool_log` | `adopt_pre_s331` |
  | `prepared_view` | `adopt_validation` |
  | `no_prepared` | `adopt_no_prepared` |
  | `k` | `adopt_k` |

  The copies are written beside the script one at a time and removed.
- **`real`**, 91 s, no stage on `.tuning/s122/prepared`, its 39-line manifest equal from start to end:
  - on the mirror, `verify`, `filter`, `split`, `sample` and `k` each refuse the five files as made before S333, and
    the mirror is unchanged;
  - `ADOPT_DRY=1 adopt` passes, the mirror unchanged. Its checks: the views by sha256, each filtered view the band
    filter recomputed, and `corpus.tsv` with the held-out views and K held to `prepared.txt`;
  - `adopt` stamps all five files, pool `899cab6d…`, and the five stages then read the mirror (`K before: 1.2302`);
  - `verify` on the pool into `.tuning/s333/real_verify` passes.
- **S332's merge smoke** against `bde5e485…`, with `DIR` in the session's scratchpad: `runs`, `merge` and `cases`
  each end `-DONE`. `verify_as_merged` passes and `verify_games_changed` is refused.

### The seven new cases

Each runs as in the table above. Their readings, from the green run:

| case | stage, POOL | plant | reading |
|---|---|---|---|
| `adopt_pool_lines` | `adopt`, its own | A's views, log's command line edited, A's mtime | refused: `verify.txt's pool lines are not .../adopt_pool_lines/pool/generate.log's` |
| `adopt_prepared_sha` | `adopt`, A | old form, `prepared.txt`'s `sha256` zeroed | refused: `prepared.txt does not name $O/pool and corpus.tsv's sha256` |
| `adopt_pre_s331` | `adopt`, its own | old form; A's views, log without its `repetition:` line, A's mtime | refused: `.../generate.log has no 'repetition:' line: its datagen predates S331` |
| `adopt_validation` | `adopt`, A | old form, a newline appended to `pool.tsv.qs.validation` | refused: `prepared.txt's qs view sha256 '5fc7e77b…' is not .../pool.tsv.qs.validation's` |
| `adopt_no_prepared` | `adopt`, A | old form, `prepared.txt` removed | refused: `no .../prepared.txt, so corpus.tsv's preparation cannot be read` |
| `adopt_k` | `adopt`, A | old form, `k.txt` set to 9.9999 | refused: `prepared.txt's k '1.2376' is not .../k.txt's '9.9999'` |
| `sample_unmarked` | `sample`, B | `verify.txt` and the four markers removed | refused: `no .../verify.txt: run verify first` |

`adopted_reads` now runs `k`, with the reading "K before: 1.2376".

### For the coordinator: stamp the real preparation

The coordinator has backed up the markers in `.tuning/s333/prepared_backup/`. Each command takes under a minute,
and none touches any file but `verify.txt` and the four markers in `.tuning/s122/prepared`:

```
cd /home/max/ws/chesso
ADOPT_DRY=1 adocs/data/S122_corpus.sh adopt   # S122-CORPUS-ADOPT-DONE, "would stamp verify.txt pool.tsv.root.done pool.tsv.qs.done pool.tsv.pv.done corpus.tsv.done; nothing written"
adocs/data/S122_corpus.sh adopt               # five "stamped" lines, S122-CORPUS-ADOPT-DONE
adocs/data/S122_corpus.sh k                   # K before: 1.2302, S122-CORPUS-K-DONE
```

Do not run `all` or `record` on it. A pass of either rewrites `verify.txt` or `prepared.txt`, and the screen and
census scripts hash or read `prepared.txt`.

### Gates, after the fixes

- `CLANG_FORMAT_MAJOR=22`: both builds and both fast suites, then `./clang-format.sh --check`, exit 0, 59 of 59 in
  each build. The first run failed `test_plan_citation_freshness` and `--citations` on one phrase in this file, read
  as a path and a symbol; it was rephrased, and the second run is green.
- `tools/plan_prose_check.py`, all five modes.
- `bash -n` on both scripts.

### Findings beyond the step, after the fixes

- **Finding 1 of the list above is narrowed, not closed.** With `needs`, a stage can no longer run on another pool
  after its predecessors' markers are deleted.
  - What is still open: `split.txt` and `k.txt` are bound only through the records beside them. Removing `verify.txt`
    and all four markers by hand, keeping `split.txt`, and running `all` on another pool would re-verify and
    re-filter, then read "split before" from the old `split.txt`.
  - A fix: a stage that runs removes the "before" records of the stages after it. Removing `split.txt` alone already
    makes `corpus_split.py split` refuse, since its outputs exist.
- **Findings 2 to 5 of the list above stand as written.**

## Proposed done stamp (the coordinator writes the final one)

2026-11-06 -- `accepts` by clause, after the coordinator's fast check (one real finding and five low, all fixed).

- **`verify`** refuses a one-run pool whose `generate.log` holds no `repetition:` line, on a fixture log (case
  `verify_no_repetition`). It now also refuses an OUT holding another pool's preparation, or one from before S333,
  before it removes anything.
- **Every `.done` marker, and `verify.txt`, name the pool** on a `pool` line by its `generate.log`'s sha256.
- **`filter`, `split`, `sample` and `k` each refuse a planted swap** naming the markers (`filter_swapped`,
  `split_swapped`, `sample_swapped`, `k_swapped`, `all_swapped`). `record` and a `verify.txt` of another pool are
  refused too. Each stage after `verify` refuses when an earlier stage's record is absent (`sample_unmarked`).
- **The regenerated preparation was read by the guards in a mirror.** Its records were copied and its large files
  linked; the five stages refuse it as made before S333; `adopt` passes dry and stamps; all five stages then read it.
  No stage ran on `.tuning/s122/prepared`, whose manifest is unchanged.
  - `adopt`'s checks, exactly: the pool's log; `verify.txt`'s pool lines and view sha256s; each marker its file's and
    no older than the pool's log; each filtered view the pool's band filter, recomputed; `prepared.txt` required and
    holding the corpus, the three held-out views and K as they are.
  - Not checked: the training views, `split.txt`, and the draws of the split and the sample.
  - The coordinator's three commands stamp the real preparation.
- **`bash -n`** passes on both scripts; **`smoke` passes 25 of 25**.
- **Red first.**
  - 23 of 25 cases fail against `9daee21`'s script, and both controls pass.
  - Seven mutants are each killed by their own case alone: finding 1's stamp break by `adopted_reads`, and the three
    refusals finding 5 named plus finding 3's three checks each by its own case.
- **Also passing:** S332's merge smoke against the new script; both fast suites, 59 of 59; `clang-format` clean; the
  five prose modes exit 0.
- No `src/` or `tools/` change. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Readings in
  `adocs/data/S333_guards.txt`.
