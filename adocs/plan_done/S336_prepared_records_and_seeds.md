id:         S336
goal:       S122's corpus preparation drops a later stage's "before" record when an earlier stage runs, and `seeds` does not read the corpus's own preparation as a meeting
accepts:    a stage that runs removes the records of every stage after it (`split.txt`, `k.txt` and their markers), so that with `verify.txt` and the four markers removed by hand and `split.txt` left, `all` on another pool no longer reads "split before", seen red on a fixture at S333's landing; `S122_corpus.sh seeds` passes on the current tree with the stamped `.tuning/s122/prepared/` in place (today it refuses, "the corpus meets itself", reading the corpus's settings line from `prepared/verify.txt`), while still refusing a planted range that meets the corpus; `bash -n`, `smoke` and `adocs/data/S333_guards.sh green` green
touches:    adocs/data/S122_corpus.sh, adocs/data/S333_guards.sh, adocs/data/S336_*, adocs/data/README.md
excludes:   `src/`; `tools/`; re-running the preparation over the 150000-game pool; any fit
decisions:  DEC-171, DEC-349
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:       2026-11-06 -- `accepts` by clause. (1) `filter`, `split`, `sample` and `k`, when they make their output, first remove the records of every later stage (`split.txt`, `corpus.tsv.done`, `k.txt`, `prepared.txt`). Data files stay. `split` refuses a fixed set no record names before it removes anything, and `verify` removes nothing. With `verify.txt` and the four markers removed by hand and `split.txt` left, `all` on another pool (`all_unmarked`) reads no "split before": at `6444f6e` it ended `-DONE` on a mixed preparation, and now it removes the stale records and refuses at `split`. Five more cases each red at `6444f6e` (`filter_again`, `split_again`, `split_unrecorded`, `sample_again`, `k_again`). For these cases `6444f6e`'s script is S333's landing's, `0bb182d`'s: the diff between them is two hunks inside `merge()` and its comment, which no case reaches. S333's guards are kept: `S333_guards.sh` `green` 35 of 35; `red` 33 of 35 at `9daee21`, `seeds_copies_only` among them, and both controls passing; before the fast check, the seven mutants each killed alone. The stamped preparation passes all eight stages in a mirror, reading every stage as made before and removing nothing, its records byte-equal and `.tuning/s122/prepared` unchanged. (2) `seeds` reads a line that is, whole, a settings line of `POOL`, `TOPUP` or `SAMPLE_DIR`, found elsewhere, as that run's own record. On the real tree it passes (refused at `6444f6e`, the corpus meeting its own line from `prepared/verify.txt` and three copies under `.tuning/s333`). It still refuses a planted run at seed 2026600000, naming the plant alone, and the corpus's line with another quiet limit. It writes nothing. It refuses when every line it finds is such a copy, since then no other run is checked (`seeds_copies_only`: it passed before the fast check's fix). Four smoke cases (`seeds_self`, `seeds_planted`, `seeds_near_copy`, `seeds_copies_only`) are red at `6444f6e`. `seeds_near_copy` differs from the corpus in the quiet limit only, and a mutant keyed on the settings line's prefix passes it. (3) `bash -n` on the three scripts. `smoke` 35 of 35. `S333_guards.sh green`, `S335_findings.sh green`, and S332's merge smoke `merge` and `cases` green. Format clean; the five prose modes and the plan ctests pass. No `src/` or `tools/`. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Readings in `adocs/data/S336_records.txt`. One fast check: nothing real, four lows. Three fixed: `seeds` refusing when no other run is checked, the near copy differing in the quiet limit only, and the red's commit stated. One accepted: a run with the corpus's identical settings is read as a copy once the corpus has logged it. The coordinator kept a copy of the stamped preparation's 13 small records with their sha256 in `.tuning/s336/prepared_stamped_backup/` (the step's beyond-step finding 1).

## Why

Found 2026-11-06 by S333's implementer ("Findings beyond the step" in S333's done file). (1) `split.txt` and `k.txt` are
bound to their pool only through the records beside them: removing `verify.txt` and the markers by hand and running
`all` on another pool still reads the old split. Low, fails only after a manual deletion. (2) `seeds` skips `POOL`,
`TOPUP` and `SAMPLE_DIR` but not `OUT`, so the corpus's own settings line in `prepared/verify.txt` is read as another
run meeting the corpus, and `seeds` refuses. Predates S333; a top-up or a fresh side sample for S124's or S126's fits
would meet it first. Filler by DEC-171: no play, no reported score.

## 2026-11-06: implementation (an Opus 5.5 subagent, briefed by the coordinator)

The main tree at `6444f6e`, beside S124's screen (its tuner at nice 10 on 12 threads, reading
`.tuning/s122/prepared/corpus.tsv`). Scratch `.tuning/s336/` (gitignored). No datagen run, match, fit or timing.
Nothing under `.tuning/s122/` was written: its preparation was read in a mirror, and `seeds` read the tree.

### What changed

- **`adocs/data/S122_corpus.sh`**, sha256 `c46e7f93…`.
  - Finding 1. New `later_records` and `drop_later`. `filter`, `split`, `sample` and `k`, at the point where each makes
    its output rather than reading it as made before, first remove the records of every later stage and print
    `<stage> runs: removed ...`:

    | stage that runs | removes, where present |
    |---|---|
    | `filter` (the first view it filters) | `split.txt`, `corpus.tsv.done`, `k.txt`, `prepared.txt` |
    | `split` | `corpus.tsv.done`, `k.txt`, `prepared.txt` |
    | `sample` | `k.txt`, `prepared.txt` |
    | `k` (after its tuner is found) | `prepared.txt` |

    `prepared.txt` is not a stage's "before", but the screen reads it as the preparation, so it goes with the rest.
    `verify` removes nothing. Only these small records go: the filtered views, the split's six files and
    `corpus.tsv` stay.
  - `split` gains one refusal, made before it removes anything. A `.train` or `.validation` file in OUT without
    `split.txt` and `pool.tsv.qs.validation` both is a fixed set no record names, which `tools/corpus_split.py` refuses
    to write over (after reading the three views). It is refused with "remove them by hand, then run split".
  - Finding 2. `seeds` collects whole settings lines, from `datagen: ` to the end of the line. A line found under
    `SEED_LOGS` that is, whole, a settings line of a file in `POOL`, `TOPUP` or `SAMPLE_DIR` is that run's own record. It
    is listed apart, with the files holding it, and is not read as another run. A line with the same seed and any
    other setting is still another run. The refusal for an empty search now means no settings line at all. The copy is
    printed last on its listing line, so a stored console reads as one more copy (see choice 4).
  - `smoke` plays 34 cases, S336's nine after S333's 25. `smoke_case` gains the expectation `fails` (`-FAILED`,
    `prepared/` free to change), and two argument forms: `!RE` (must not be printed) and `gone:NAME` (no
    `prepared/NAME` after the stage). A failing case now names what it lacked.
  - Header: a "Since S336" paragraph, and an "AMENDED 2026-11-06, S336" note in the pre-registration. The guards
    change and the recipe does not.
- **`adocs/data/S333_guards.sh`**, sha256 `a196c9df…`. `CASES` is now 34, and the header notes that its `real` reads the
  preparation as it was before the coordinator's stamp. Since the stamp the five stages pass where that stage asks for
  refusals, so it was not run. `S336_records.sh real` reads the stamped preparation.
- **New:** `adocs/data/S336_records.sh` (`red`, `green`, `real`, `seeds`), sha256 `21431a47…`, and
  `adocs/data/S336_records.txt`, its consoles and the regressions'.
- **`adocs/data/README.md`**: the `S122_corpus.sh` and `S333_guards.sh` rows extended; two rows added.
- **`DEV_MANUAL.md` and `MANUAL.md` checked: no change.** Neither names the corpus script's stages, its records or
  `seeds`. DEV_MANUAL's corpus section is S082's `corpus_split.py` recipe.

### Choices beyond the letter of `accepts`

1. **Records, not data.** The stages remove the records only. The split's own files are kept, because
   `corpus_split.py` never overwrites a fixed set, and deleting one would remove that guard. So after a re-filter,
   `split` refuses until a person removes the six files. The scenario in `accepts` therefore ends on a refusal at
   `split`, not on a new preparation. The alternative, removing the split's files too, would make that `all` pass and
   would silently delete the real preparation's 3.8 GB fixed set after one marker was lost.
2. **`split` refuses before it removes anything** (case `split_unrecorded`). Without that, a `split.txt` lost by hand
   would cost `corpus.tsv.done`, `k.txt` and `prepared.txt` too, and then `corpus_split.py` would refuse anyway. With
   it, nothing else goes, and restoring `split.txt` restores the preparation.
3. **`verify` removes nothing.** It runs on every `all`. If it removed records, `all` would stop being resumable, and
   `control_all` and the real mirror's `all` would fail. Only `filter` reads `verify`'s output as made before, its
   markers name their pool, and `verify` holds OUT to POOL before it writes.
4. **`seeds`: identity by the whole line, not by pruning OUT.** Pruning OUT alone does not pass on this tree. The
   corpus's line is also in `.tuning/s333/prepared_backup/verify.txt`, `real_mirror/verify.txt` and
   `real_verify/verify.txt`, and every mirror or backup adds one. The whole line is compared, not the triple of seed,
   games and threads, so a run at the corpus's seed and games with another quiet limit is still a meeting (the
   fourth `seeds` run below).
   - The cost: a separate run with the corpus's exact settings line, found after the corpus has run, reads as a copy.
     Before the corpus runs, POOL holds no log and such a line is refused as before. The same seed with identical
     settings is a real meeting, masked only in a re-check made after our own run logged it; the coordinator's fast
     check (its finding 3) accepted this.
   - Found while building it: the listing first printed `datagen: ...: in FILES`. A console holding that line read as a
     run with the corpus's triple and other trailing text. The first `seeds` stage run, whose console was being
     written under `.tuning/s336`, listed the corpus's range as met in both plant runs. The line is now printed last.

### The fixtures, red and green

The red is `adocs/data/S336_records.sh red`. It writes `6444f6e`'s `S122_corpus.sh` (sha256 `371a3035…`) beside the
tree's as `.S336_before_corpus.sh`, runs the tree's smoke with `SMOKE_AGAINST` set to it, and removes it afterwards.
The green is `adocs/data/S336_records.sh green`.

Each case runs as `env [EXTRA] POOL=<pool> OUT=$O/cases/<case>/prepared TUNING=... GAMES=2400 VALIDATION_GAMES=20 bash
<script> <stage>`, as in S333's table:
- `$O` is `.tuning/s336/tuning/s122/phase1/corpus_smoke`;
- A is `$O/pool` and B is `$O/swap`;
- the seeds cases add `EXTRA` = `SEED_LOGS=$O/cases/<case> CORPUS_SEED=990336000 TOPUP_SEED=990339000`, with POOL
  `$O/cases/<case>/pool`.

| case | stage, POOL | setup | at `6444f6e` (red) | at the step's (green) |
|---|---|---|---|---|
| `all_unmarked` | `all`, B | a copy of A's preparation; `verify.txt` and the four markers removed by hand, `split.txt` left: the case in `accepts` | `-DONE`: verify and filter on B, "split before" from A's `split.txt`, `sample` draws a corpus from A's training view and stamps it with B's pool, "K before", and `prepared.txt` names B | `-FAILED`: `filter runs: removed split.txt k.txt prepared.txt`, then `split` refuses on the six files; no "split before" or "K before"; the four records gone |
| `filter_again` | `filter`, A | a copy, `pool.tsv.qs.done` removed | `-DONE`, the four records left | `-DONE`: `filter runs: removed split.txt corpus.tsv.done k.txt prepared.txt`, "root filtered before", qs filtered again |
| `split_again` | `split`, A | a copy, `split.txt` and the six split files removed | `-DONE`, three records left | `-DONE`: `split runs: removed corpus.tsv.done k.txt prepared.txt` |
| `split_unrecorded` | `split`, A | a copy, `split.txt` removed | `-FAILED` on `corpus_split.py`'s own refusal, after it read the views | refused before it reads the views or removes anything, `prepared/` byte-equal |
| `sample_again` | `sample`, A | a copy, `corpus.tsv.done` removed | `-DONE`, `k.txt` and `prepared.txt` left | `-DONE`: `sample runs: removed k.txt prepared.txt` |
| `k_again` | `k`, A | a copy, `k.txt` removed | `-DONE`, `prepared.txt` left | `-DONE`: `k runs: removed prepared.txt` |
| `seeds_self` | `seeds`, its own | the case's pool log, its settings line copied into `prepared/verify.txt` and `mirror/verify.txt`, and one other run of 10 games at seed 990335000 | refused: the corpus meets its own `[990336000, 990338400)` | passes: "meets none of the 1"; the copy listed in both `verify.txt` files; `prepared/` unchanged |
| `seeds_planted` | `seeds`, its own | the same, plus a plant of 10 games at seed 990337000 | refused, but naming the corpus's own range beside the plant's | refused, naming `[990337000, 990337010)` alone |
| `seeds_near_copy` | `seeds`, its own | the same, plus the corpus's line with 2399 games | refused, naming its own range too | refused, naming `[990336000, 990338399)` alone |

- **Red:** "34 cases, 25 as they must be, 9 not". Exactly the nine above fail, each naming what it lacked, and S333's
  25 pass. The stage ends `S336-RECORDS-RED-DONE`.
- **Green:** "34 cases, 34 as they must be, 0 not", `S122-CORPUS-SMOKE-DONE`.
- **No mutant stage.** Each of the nine cases runs one stage and is red at `6444f6e` on that stage's own missing
  behaviour. So each case kills its stage's change alone.

### S333's guards, kept

Run with `S=.tuning/s336/s333`:
- `adocs/data/S333_guards.sh green`: 34 of 34.
- `S333_guards.sh red`, against `9daee21`'s script: "34 cases, 2 as they must be, 32 not". The two controls pass and
  all nine new cases are red there too.
- `S333_guards.sh mutants`: each of the seven is killed by its own case alone ("34 cases, 33 as they must be, 1 not").

### The stamped preparation, read by every stage in a mirror

`adocs/data/S336_records.sh real`, 68 s. The mirror is `.tuning/s336/real_mirror`: 13 copies of the files under 1 MB and
13 links to the rest. Before each stage, `verify.txt`, the four markers and `split.txt` are checked present. These are
the records whose absence would let a stage write a large file through a link.
- `verify`, `filter`, `split`, `sample`, `k` and `record`, then `ADOPT_DRY=1 adopt` ("names this pool already" five
  times, "nothing to stamp") and `all`, each end `-DONE`. None prints a removal.
- Every stage reads as made before: "root/qs/pv filtered before", "split before", "sampled before", "K before:
  1.2302".
- After the eight runs, the three view markers, `corpus.tsv.done`, `split.txt` and `k.txt` are the real ones' bytes.
  `verify.txt`'s view and pool lines are the real one's, compared as a set: `adopt` appended the real one's pool line,
  and `verify` writes it second. `prepared.txt` is the real one's but for its date and paths.
- `.tuning/s122/prepared`'s 39-line manifest is the same at the end as at the start.

### `seeds` on the real tree

`adocs/data/S336_records.sh seeds`. A check by reading came first: `seeds()` has no redirection to a file, and the
script skips its `mkdir -p "$OUT"` for `seeds`. The stage also checks it: nothing under `.tuning` (but the scratch) or
`adocs/data` is newer than a stamp taken before the first run. The four runs:

1. **`6444f6e`'s script, the defaults: refused.** "the corpus's [2026500349, 2026650349) ... meet [2026500349,
   2026650349)". The line is read from `.tuning/s122/prepared/verify.txt` and the three copies under `.tuning/s333`.
2. **The tree's, the defaults: passes**, `S122-CORPUS-SEEDS-DONE`. 22 other runs; the corpus's own line is listed
   apart with the files holding it. The corpus, top-up and sample ranges each meet none.
3. **The tree's, with a plant: refused, naming the plant alone.** The plant is 600 games at seed 2026600000, in a
   `mktemp` directory under the session's scratchpad, added to `SEED_LOGS`. The reading is "meet [2026600000,
   2026600600)".
4. **The tree's, with the corpus's line, quiet limit changed to 1000: refused.** It is another run, not a copy.

The plants' lines are written by `printf` and never printed. The directories are removed on exit. After
`adocs/data/S336_records.txt` was written, a plain `adocs/data/S122_corpus.sh seeds` passes again and lists that file
among the copies.

### Regressions

- `adocs/data/S335_findings.sh green`: ten of ten.
- `adocs/data/S332_merge_smoke.sh` `merge`, then `cases`: both `-DONE`. `verify_as_merged` passes and
  `verify_games_changed` is refused.
- Both ran with `DIR=.tuning/s336/merge_smoke`, a copy of `.tuning/s335/smoke/pristine` and its `pristine.sha256`,
  checked first. No datagen ran.

### Gates

- `bash -n` on `S122_corpus.sh`, `S333_guards.sh` and `S336_records.sh`.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check`, exit 0.
- `tools/plan_prose_check.py` in its five modes, each exit 0, and the five together exit 0. `--prose` flags 0 sentences;
  `--citations` flags 0 over 19 files, 0 code citations in this file; `--touches` flags 0; `--params` is silent; and
  `--gate` reports the command agreeing in both documents.
- `ctest --test-dir build -R "test_plan_" -j4`: 5 of 5.
- No `src/` or `tools/` change, so no build and no full ctest.

### Findings beyond the step, not fixed

1. **One lost view marker now costs the split, the sample and K** on the real preparation. A view marker removed by hand,
   then `filter`, re-filters that view and removes `split.txt`, `corpus.tsv.done`, `k.txt` and `prepared.txt`. `split`
   then refuses until the six fixed-set files are removed by hand. This is what `accepts` asks for, and it fails
   closed. The cheap recovery would be a copy of the removed records, but none exists for the stamped preparation:
   `.tuning/s333/prepared_backup` holds the pre-stamp markers, `verify.txt` and `prepared.txt`, and no `split.txt` or
   `k.txt`. A copy of the stamped preparation's small files, kept beside it, would make the recovery a `cp`. Low.
2. **`check_prepared` binds OUT to POOL by `generate.log`'s sha256 alone.** A pool's views replaced in place under the
   same log would be re-verified by `verify`, and `filter` would still read "filtered before". It needs a pool edited
   by hand, and it predates S336. Low.
3. **`seeds` reads every text file under `.tuning` and `adocs/data`.** So any fixture holding a settings line that meets
   a real range makes `seeds` refuse, wherever it is written. This step's real-tree plants are kept outside the
   repository for that reason. The smoke's seeds fixtures (seeds 990335000 to 990337010) and S332's runs appear in the
   real listing as runs that meet nothing. A hazard, not a defect.
4. **`S333_guards.sh real` no longer passes**, since the coordinator stamped the preparation (by reading: its first five
   stages now pass where it expects refusals). Its header says so; it was not run.

## 2026-11-06: the coordinator's fast check, three lows fixed, one accepted

Nothing real, four lows. All fixed in the same tree, uncommitted, except finding 3, which choice 4 now names. Scripts:
`S122_corpus.sh` sha256 `7e5a47a9…`, `S333_guards.sh` `33973bec…`, `S336_records.sh` `0b3b162d…`.
Readings are at the end of `adocs/data/S336_records.txt`. The sections above describe the first delivery and are kept as
written. The smoke now has 35 cases.

1. **`seeds` passed when every line it found was a copy.** It now also refuses when no other run is left to check, so a
   `SEED_LOGS` that misses the real logs fails closed: "every datagen settings line under ... is one in POOL, TOPUP or
   SAMPLE_DIR, so no other run was checked". Fixture `.tuning/s336/fc/copies_only`: the case's pool log and
   `prepared/verify.txt`'s copy of it, no other run. Each script ran `seeds` there with `SEED_LOGS`, `POOL`, `OUT` and
   `TUNING` in the fixture and `CORPUS_SEED=990336000`:
   - the script before the fix (sha256 `c46e7f93…`, copied beside the tree's and removed): passes, "meets none of the
     0". This is the red;
   - `6444f6e`'s script: refused, but as the corpus meeting its own line;
   - the fixed script: refused, with the new line.

   New smoke case `seeds_copies_only`.
2. **`seeds_near_copy` now differs in the quiet limit only** (1000 for 30000). Seed, games and threads are the corpus's,
   so the case kills a `seeds` keyed on them. Its fixture `.tuning/s336/fc/near` was run by a mutant whose settings regex
   lacks its trailing `.*`, so copies are compared by the line's prefix of games, nodes, threads and seed (sha256
   `e94039d4…`, written beside the tree's and removed). The mutant passes: "meets none of the 1". This is the red. The
   fixed script refuses, naming `[990336000, 990338400)`. The case asks for the corpus's range met and for the copies
   listed apart, since the plant's range is the corpus's own. At `6444f6e` the same fixture is refused without a copies
   listing, so the case is red there too.
3. **Accepted, choice 4:** the masked re-check of a run with identical settings.
4. **The red's commit.** `accepts` names S333's landing, `0bb182d`. The red was taken at `6444f6e`, and for these
   cases the two are the same script. `git diff 0bb182d 6444f6e -- adocs/data/S122_corpus.sh` is two hunks, both
   S335's: `merge()`'s interrupted-merge test and the comment above `merge()`. No case here calls `merge`.

Also: `all_unmarked`'s `gone:corpus.tsv.done` is kept, with a comment. The setup removes the file, but the check can
fail: at `6444f6e`, `sample` drew a new corpus and wrote `corpus.tsv.done` again. The red reading says "left
corpus.tsv.done". So the check asks that no corpus is drawn.

Re-run once each, under S124's screen: `S336_records.sh green` gave 35 of 35, and `S333_guards.sh green` 35 of 35. A
plain `S122_corpus.sh seeds` on the real tree passes, with 21 other runs; the old 2399-game fixture is gone. `S333_guards.sh red`, re-run at the coordinator's request with
35 cases, reads "35 cases, 2 as they must be, 33 not", and both controls pass. `seeds_copies_only` fails at `9daee21`:
`seeds` ends `-FAILED` without the new refusal line, as at `6444f6e`. Not re-run after the fix: `S336_records.sh red`,
`real` and `seeds`, and `S333_guards.sh mutants`; their readings above are at 34 cases.

## Proposed done stamp (the coordinator writes the final one)

2026-11-06 -- `accepts` by clause.
(1) `filter`, `split`, `sample` and `k`, when they make their output, first remove the records of every later stage
(`split.txt`, `corpus.tsv.done`, `k.txt`, `prepared.txt`). Data files stay. `split` refuses a fixed set no record names
before it removes anything, and `verify` removes nothing. With `verify.txt` and the four markers removed by hand and
`split.txt` left, `all` on another pool (`all_unmarked`) reads no "split before": at `6444f6e` it ended `-DONE` on a mixed
preparation, and now it removes the stale records and refuses at `split`. Five more cases each red at `6444f6e`
(`filter_again`, `split_again`, `split_unrecorded`, `sample_again`, `k_again`). For these cases `6444f6e`'s script is
S333's landing's, `0bb182d`'s: the diff between them is two hunks inside `merge()` and its comment, which no case reaches. S333's guards are kept: `S333_guards.sh`
`green` 35 of 35; `red` 33 of 35 at `9daee21`, `seeds_copies_only` among them, and both controls passing; before the
fast check, the seven mutants each killed alone. The stamped preparation passes all
eight stages in a mirror, reading every stage as made before and removing nothing, its records byte-equal and
`.tuning/s122/prepared` unchanged.
(2) `seeds` reads a line that is, whole, a settings line of `POOL`, `TOPUP` or `SAMPLE_DIR`, found elsewhere, as that
run's own record. On the real tree it passes (refused at `6444f6e`, the corpus meeting its own line from
`prepared/verify.txt` and three copies under `.tuning/s333`). It still refuses a planted run at seed 2026600000, naming
the plant alone, and the corpus's line with another quiet limit. It writes nothing. It refuses when every line it finds is such a copy, since then no other run is checked
(`seeds_copies_only`: it passed before the fast check's fix). Four smoke cases (`seeds_self`, `seeds_planted`,
`seeds_near_copy`, `seeds_copies_only`) are red at `6444f6e`. `seeds_near_copy` differs from the corpus in the quiet
limit only, and a mutant keyed on the settings line's prefix passes it.
(3) `bash -n` on the three scripts. `smoke` 35 of 35. `S333_guards.sh green`, `S335_findings.sh green`, and S332's
merge smoke `merge` and `cases` green. Format clean; the five prose modes and the plan ctests pass. No `src/` or
`tools/`. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Readings in `adocs/data/S336_records.txt`. One fast check: nothing real, four lows. Three fixed: `seeds` refusing when no
other run is checked, the near copy differing in the quiet limit only, and the red's commit stated. One accepted: a
run with the corpus's identical settings is read as a copy once the corpus has logged it.
