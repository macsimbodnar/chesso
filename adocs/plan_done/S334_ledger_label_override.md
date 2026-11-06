id:         S334
goal:       `tools/ledger.py` reads a verdict row's description from a recorded override where a closing commit's subject breaks its rule, the first being `1ffc96e`
accepts:    the ledger's S122 row reads "the king safety family" (or the override's text) while `git log` is unchanged; the override is keyed by full sha, refused when its sha closes no verdict or when the subject already reads out cleanly, each refusal seen red on a fixture; `--table` otherwise byte-equal to today's; plan.md's table regenerated in the same commit
touches:    tools/ledger.py, tools/ledger_overrides.tsv, tests/test_ledger.py, DEV_MANUAL.md, adocs/plan.md
excludes:   rewriting history; amending `1ffc96e`; changing the subject rule for later commits
decisions:  DEC-171, DEC-220
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:       2026-11-06 -- `accepts` by clause. `tools/ledger.py` reads `tools/ledger_overrides.tsv`, keyed by full sha, one row: `1ffc96e95d0e7e56f580f84a9c6084ec0baadc82` -> "the king safety family", by S334. `git log` is unchanged. It is `tools/`, not `adocs/data/`, because the file takes a row per later exception and `adocs/data/` is added-never-edited. Refused with exit 2, each seen red on a fixture: a sha that closes no verdict (no block, or a later carrier as `db1b6fa`), a subject that already reads out cleanly by the parser's own reading, a short, non-hex or repeated sha, an empty or `|`-carrying `what`, a `by` naming no step or decision, a comments-only or missing file. Red first: 16 errors with no mechanism, then 10 failures with the mechanism and no refusal, each `LedgerError not raised` or `SystemExit not raised`. Then the fast check's three probes, red under the first, regex test for "clean" (1 failure, 1 error) and accepted under the parser-backed one. The two file cases kill their mutants. Green 45 of 45. `--table` against `8cc9a38`'s output: the S122 row is the only diff; `--figures`, `--no-git` and `--audit-seed-class` byte-equal. `adocs/plan.md`'s table is from `python3 tools/ledger.py > .tuning/s334/plan_table.md`, pasted whole by the coordinator. `DEV_MANUAL.md` ledger section updated; `MANUAL.md` checked, nothing to change; no `adocs/data/` file. Gates first under S124's screen, `CLANG_FORMAT_MAJOR=22`, `-j6`: `build` fast 60 of 60, `build-tune` 59 of 60, the red being `plants_threats`' 60 s clock under load (taken before the fast check's fixes, which touch only `tools/ledger.py`, its test, its override file and the docs). That test passes unclocked in 80.07 s, and this step touches no C++. Format and the five `plan_prose_check.py` modes rc 0. `touches:` gained `tools/ledger_overrides.tsv` and `DEV_MANUAL.md` and lost `adocs/data/S334_*` and its README, which it does not use. Four other rows read out wrong (`e47eb4a`, `b9f5b96`, `775dafd`, `7f46c0c`): left for a later step.  **The coordinator's gate, on the final tree with S124's screen paused (SIGSTOP, 6 min, then SIGCONT): `tools/gate.sh` 60 of 60 in both builds, format clean.** One fast check, four lows: three fixed, the fourth (no gate check of override rows against history) kept by choice and said why. Found beyond the step, not fixed: four other rows read out wrong (`e47eb4a` and `b9f5b96`, which an override row can fix; `775dafd` and `7f46c0c`, whose run id is wrong too and which the file has no column for).

## Why

S122's H1 commit `1ffc96e` was subjected "Ship S122's fitted king safety family on its H1 (S122, DEC-353)"
instead of the form `ledger.py`'s header asks for ("Record S<nnn>'s H1 for <what>"). The rule strips the run id from
the middle and leaves "Ship 's fitted king safety family on its H1 (S122, DEC-353)" as the row's description,
read from `git log` at every regeneration. History is not rewritten, so the tool needs a recorded exception. The
mistake was the coordinator's (2026-11-05). Filler by DEC-171: it moves no play and no reported score.

## What was built (2026-11-06)

**Where the override lives: `tools/ledger_overrides.tsv`**, three columns `sha`, `what`, `by`, `#` comments on top as
in the seed. Not `adocs/data/`: that directory holds the evidence of runs, "added, never edited", and this file
takes a row at each later exception; a step-named `adocs/data/S334_*` would need a new file per exception or an edit
to this step's evidence. `tools/` already holds the inputs its scripts read (`spsa_*.json`). So `touches:` lost
`adocs/data/S334_*` and `adocs/data/README.md`, which hold nothing of this step, and gained the override file,
`tests/test_ledger.py` by name and `DEV_MANUAL.md`.

**The row.** `1ffc96e95d0e7e56f580f84a9c6084ec0baadc82	the king safety family	S334`. Its neighbours read "the
passed pawn family" (S123), "the pawn structure family" (S125), "the threat family" (S101).

**`tools/ledger.py`.** `read_commits(repo, overrides_path)` reads `%H` beside `%h`, a block row carries its commit's
full sha and subject, and after `drop_repeated_verdicts` `apply_overrides` replaces `what` and nothing else. Run,
numbers, class and every other field are the block's. `--no-git` and `--audit-seed-class` read no commit and so no
override. Refusals, each a `LedgerError` that `read_commits` turns into exit 2 with the file and line:

| refused | where |
|---|---|
| a key that is not 40 lowercase hex (short, upper case, 41 long, non-hex) | `read_overrides` |
| a key listed twice -- two descriptions for one commit | `read_overrides` |
| an empty `what`, or one carrying `\|` (it is a table cell) | `read_overrides` |
| a `by` that is no `S<nnn>` or `DEC-<nnn>`; a header other than `sha what by`; a row not of three cells | `read_overrides` |
| a file of comments alone (no header row); a file missing or unreadable | `read_overrides` |
| a sha that closes no verdict the ledger prints: no block, or a later carrier dropped as a repeat (db1b6fa's case) | `apply_overrides` |
| a sha whose subject already reads out cleanly | `apply_overrides` |

**"Reads out cleanly", coded** in `subject_reads_cleanly`, by asking the parser itself. Take the run and the description
`split_run_and_what` reads. Build the twelve forms `Record <run>'s <word><sep><what>`, with word one of H1, H0, no verdict
and verdict, and sep one of " for ", ": " and ", ". The subject is clean when it is one of those twelve and the parser
reads `(run, what)` back from all twelve. This replaces a first version, `CLEAN_SUBJECT_RE`, an outward regex for
the form. The fast check found it calling three subjects clean that the parser garbles: "Record S231's verdict: no
verdict at the cap" reads "at the cap", "Record S231's H1: verdict stands" reads "stands", and "Record S231's H1 for
for x" reads "for x", which the `: ` form would read as "x". An override on any of the three would have been refused,
so the row could never be fixed. "Record S024 v1's no verdict", whose description falls back to the whole subject,
is not clean. Over `git log` today, under both versions, 38 verdict-closing subjects are clean and 6 are not:
`1ffc96e`, `e47eb4a`, `7f46c0c`, `775dafd`, `b9f5b96`, `db1b6fa`.

## Readings

**Today's output saved before any change**, at `8cc9a38`, clean tree:

    python3 tools/ledger.py --table > .tuning/s334/table_before.md
    python3 tools/ledger.py         > .tuning/s334/full_before.md

`adocs/plan.md` lines 261-328 (table header to figures paragraph) are byte-equal to `full_before.md`.

**Red first, two stages.** The new cases written before any code (`OverrideFile` 8, `OverrideRule` 6,
`OverrideFromGitLog` 3; 42 tests in all):

1. Against `8cc9a38`'s `ledger.py`: `FAILED (errors=16)` -- `read_overrides` missing 8, `apply_overrides` 4,
   `subject_reads_cleanly` 1, `read_commits()` taking no arguments 3. The 17th,
   `test_the_rule_garbles_the_subject_s122_closed_on`, passed: it is the precondition, today's garbled reading of
   the S122 subject.
2. The mechanism with no refusal coded: `FAILED (failures=10)`, every one on its own assertion -- `LedgerError not
   raised` for `test_a_short_sha_is_refused`, `test_a_sha_that_is_not_forty_lowercase_hex_is_refused`,
   `test_a_sha_listed_twice_is_ambiguous_and_refused`, `test_an_empty_or_table_breaking_description_is_refused`,
   `test_a_row_naming_no_step_or_decision_is_refused`, `test_a_bad_header_or_row_shape_is_refused`,
   `test_an_override_whose_sha_closes_no_verdict_is_refused`, `test_an_override_on_a_later_carrier_of_a_verdict_is_refused`,
   `test_an_override_on_a_subject_that_reads_out_cleanly_is_refused`; `SystemExit not raised` for
   `test_a_commit_carrying_no_block_is_refused_loudly`. The override and full-sha cases were already green.
3. With the refusals: `Ran 42 tests`, `OK`.
4. After the fast check, three more cases. `test_a_subject_in_form_that_the_parser_garbles_takes_an_override` holds
   the three probes above, each override accepted. The probes are also added to
   `test_which_subjects_read_out_cleanly`. Under `CLEAN_SUBJECT_RE`: `FAILED (failures=1, errors=1)`. The first is
   `True is not false : Record S231's verdict: no verdict at the cap`, and the error is that probe's override
   refused, "already reads out under the rule". Green under the parser-backed version, 45 of 45.
   `test_a_file_of_comments_alone_is_refused` and `test_a_missing_file_is_refused` cover `read_overrides`'
   "no header row" and "cannot be read". Both checks already existed, so both cases passed at once. Each kills its
   mutant, run through the test module against a copy of `ledger.py` with the check removed. Without the header
   check: `LedgerError not raised`. Without the `open` guard: `FileNotFoundError` in place of a `LedgerError`.

Each `apply_overrides` refusal case first applies the same override where the refused condition is absent; the file
refusals rest on `test_a_well_formed_file_reads_by_full_sha` (TESTS rule). The git-backed cases build a throwaway
repository, as `WorkingTreeDiff` does; no test reads this repository's log.

**Byte equality.** After the change:

    python3 tools/ledger.py --table > .tuning/s334/table_after.md
    diff .tuning/s334/table_before.md .tuning/s334/table_after.md

    66c66
    < | S122 | Ship 's fitted king safety family on its H1 (S122, DEC-353) | 1 h 22 m | 2916 | `{0, 5}` | H1, +19.44 +/- 9.07 |
    ---
    > | S122 | the king safety family | 1 h 22 m | 2916 | `{0, 5}` | H1, +19.44 +/- 9.07 |

`8cc9a38`'s script against the new one in every mode (`git show HEAD:tools/ledger.py`, its `REPO` pointed at this
tree): `--figures`, `--no-git`, `--no-git --table`, `--audit-seed-class` 0 diff lines; `--table` and the bare
command 4, the hunk above. The figures paragraph does not move.

**Against the real log**, one scratch override row each through `read_commits(REPO, <file>)`: `1ffc96e` short, exit
2 "not a full sha"; `bc550ea`'s full sha (Record S101's H1 for the threat family), exit 2 "already reads out under
the rule, as [the threat family]"; `db1b6fa`'s, exit 2 "closes no verdict the ledger prints"; `8cc9a38`'s (no
block), the same; `1ffc96e`'s full sha, accepted.

**plan.md's table, for the closing commit:**

    python3 tools/ledger.py > .tuning/s334/plan_table.md

replaces `adocs/plan.md` from `| run | what it measured |` to the `**The ledger holds 64: ...` paragraph, whole; it
differs from what stands there in the S122 row only. Re-run it at the closing commit if a verdict lands first.

**Docs.** `DEV_MANUAL.md`'s ledger section gained one paragraph on the override file and its refusals, and its test
sentence names the override file's refusals. `MANUAL.md` makes no statement about the ledger: none needed. No
`adocs/data/` file, so no README row. `specs.md` does not name the ledger.

**Gates**, `CLANG_FORMAT_MAJOR=22`, `-j6`, S124's screen running (its `tuner` at nice 10, 12 threads, load 11.8 to
14.5 on 12):

- `cmake --build build -j6`, `cmake --build build-tune -j6`: both built.
- `ctest --test-dir build -L fast -j6`: 60 of 60 passed, `test_ledger` 42 tests `OK`.
- `ctest --test-dir build-tune -L fast -j6`: 59 of 60. The one red is `test_tuner_gradient_plants_threats`,
  Timeout at 60.01 s, the screen's load as `status.md` and S320, S321, S323 and S331 record it. Timed out again on a
  rerun at nice 10 and once more alone at nice 0. Run directly from `build-tune/tests` with no clock, 14357 of 14357
  assertions passed in 80.07 s; `build`'s binary the same in 81.03 s. This step changes no C++ and nothing that test
  reads. (A second rerun showed `test_clang_format_script` red as well: that rerun had not exported the override to
  ctest, and with it the test passes.) The coordinator's gate with the machine paused is the reading for that clock.
- `./clang-format.sh --check`: rc 0.
- `tools/plan_prose_check.py` `--touches`, `--citations`, `--params`, `--gate`, `--prose`: rc 0 each.

**Beyond the step.** Four other rows read out wrong today, and S334 does not touch them because `accepts` lets only
the S122 row move:

| commit | row reads | can an override fix it |
|---|---|---|
| `e47eb4a` | S289, "Revert 's null-move table guard on its H0 (DEC-309)" | yes, `what` only |
| `b9f5b96` | S112, "no-verdict at the cap: a zero, the code kept for S022" | yes, `what` only |
| `775dafd` | run "S022", "1's H0: the exchange gate stays" | `what` only; the run should match its sibling "S022 v2" |
| `7f46c0c` | run "S023", "1's H0: capture history in the margins reverts" | the same |

The two `verdict 1` subjects need a `run` correction too, which this file's columns do not carry. `db1b6fa`, the
sixth subject that does not read out cleanly, prints no row.

**Override rows are not checked by `tools/gate.sh`, by choice** (the fast check's finding 2). `ledger.py` checks every
row against `git log` at every regeneration, and refuses to print. A regeneration follows every verdict, so a bad row
fails loudly at the next one. A gate check would add a history walk to every commit for a file that changes once per
exception and that only the ledger reads.

## Proposed done stamp

done:       2026-11-06 -- `accepts` by clause. `tools/ledger.py` reads `tools/ledger_overrides.tsv`, keyed by full
sha, one row: `1ffc96e95d0e7e56f580f84a9c6084ec0baadc82` -> "the king safety family", by S334. `git log` is
unchanged. It is `tools/`, not `adocs/data/`, because the file takes a row per later exception and `adocs/data/` is
added-never-edited. Refused with exit 2, each seen red on a fixture: a sha that closes no verdict (no block, or a
later carrier as `db1b6fa`), a subject that already reads out cleanly by the parser's own reading, a short, non-hex
or repeated sha, an empty or `|`-carrying `what`, a `by` naming no step or decision, a comments-only or missing file.
Red first: 16 errors with no mechanism, then 10 failures with the mechanism and no refusal, each `LedgerError not
raised` or `SystemExit not raised`. Then the fast check's three probes, red under the first, regex test for "clean"
(1 failure, 1 error) and accepted under the parser-backed one. The two file cases kill their mutants. Green 45 of 45. `--table` against `8cc9a38`'s output: the S122 row is the only diff; `--figures`, `--no-git` and
`--audit-seed-class` byte-equal. `adocs/plan.md`'s table is from `python3 tools/ledger.py >
.tuning/s334/plan_table.md`, pasted whole by the coordinator. `DEV_MANUAL.md` ledger section updated; `MANUAL.md`
checked, nothing to change; no `adocs/data/` file. Gates under S124's screen, `CLANG_FORMAT_MAJOR=22`, `-j6`: `build`
fast 60 of 60, `build-tune` 59 of 60, the red being `plants_threats`' 60 s clock under load (taken before the fast check's fixes,
which touch only `tools/ledger.py`, its test, its override file and the docs). That test passes
unclocked in 80.07 s, and this step touches no C++. Format and the five `plan_prose_check.py` modes rc 0. `touches:`
gained `tools/ledger_overrides.tsv` and `DEV_MANUAL.md` and lost `adocs/data/S334_*` and its README, which it does not
use. Four other rows read out wrong (`e47eb4a`, `b9f5b96`, `775dafd`, `7f46c0c`): left for a later step.
