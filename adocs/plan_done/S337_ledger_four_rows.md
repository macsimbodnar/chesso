id:         S337
goal:       the ledger's four other rows whose closing subjects break the rule read out right: two by override rows, two by a run column the override file gains
accepts:    `python3 tools/ledger.py --table` reads `e47eb4a`'s and `b9f5b96`'s rows by override rows; `775dafd`'s and `7f46c0c`'s rows read their true run ids and descriptions through an optional `run` column of `tools/ledger_overrides.tsv`, refused like the other columns when malformed and seen red on a fixture; every other row byte-equal; `git log` unchanged; plan.md's table regenerated in the same commit
touches:    tools/ledger.py, tools/ledger_overrides.tsv, tests/test_ledger.py, DEV_MANUAL.md, adocs/plan.md
excludes:   rewriting history; the subject rule for later commits
decisions:  DEC-171, DEC-220
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-06
done:       2026-11-06 -- `accepts` by clause. `tools/ledger_overrides.tsv` gains an optional `run` column (header `sha what by run`; a row of three cells keeps the subject's run) and four rows by S337: `e47eb4a` -> S289 "the null-move table guard", `b9f5b96` -> S112 "per-move futility in quiescence at its seeds", `775dafd` -> **S022 v1** "the exchange gate stays", `7f46c0c` -> **S023 v1** "capture history in the margins reverts"; run ids from each step's verdict-1 candidate commit ("(S022 v1)", "(S023 v1)"), S022's `S022_v1_sprt.log` and sibling S022 v2, and S023's done stamp "verdict 1 of two". `git log` unchanged. Refused with exit 2, each seen red on a fixture: a `run` that `RUN_RE` does not match whole, a header other than the three or four columns, a row longer than its header, and a row that changes nothing the rule reads (run and `what` both the rule's; this also refuses a `what`-only no-op, which S334 accepted), and a `run` naming another step than the subject's (the fast check's second low). S334's refusals kept. Red first: 5 failures and 4 errors with no mechanism, then 2 `LedgerError not raised` with the mechanism and no refusal; green 59 of 59; seven mutants of the new checks all killed. `--table` against `e03366a`'s: the four rows are the only diff; `--figures`, `--no-git`, `--no-git --table` and `--audit-seed-class` byte-equal. `adocs/plan.md`'s table is `python3 tools/ledger.py > .tuning/s337/plan_table.md`, pasted whole by the coordinator. `DEV_MANUAL.md` ledger section updated; `MANUAL.md` checked, nothing to change; no `adocs/data/` file. Gates under S124's screen: `test_ledger` and the five `test_plan_*` 6 of 6, format and the five `plan_prose_check.py` modes rc 0; no C++ touched, no build. One fast check, two lows fixed (the module docstring; a run cell naming another step is now refused, red first). The coordinator's gate with S124's screen paused: `tools/gate.sh` 60 of 60 in both builds; plan.md's table pasted from `python3 tools/ledger.py`, four rows moved.

## Why

Found 2026-11-06 by S334's implementer ("Beyond the step" in S334's done file). `e47eb4a` (S289) reads "Revert 's
null-move table guard on its H0 (DEC-309)" and `b9f5b96` (S112) "no-verdict at the cap: ..."; an override row fixes
each. `775dafd` and `7f46c0c` say "S022 verdict 1" and "S023 verdict 1", so the run reads "S022"/"S023" and the
description starts "1's H0: ..."; the run id is wrong as well, and the override file has no column for it. Filler by
DEC-171: no play, no reported score.

## What was built (2026-11-06)

**The four rows**, in `tools/ledger_overrides.tsv`, `by` S337, each read from its commit and its step's done file:

| commit | run before -> after | what before -> after | evidence |
|---|---|---|---|
| `e47eb4a` | S289 | "Revert 's null-move table guard on its H0 (DEC-309)" -> "the null-move table guard" | the subject's own phrase; cd50178's guard, S289's done stamp; S292's row beside it reads "the narrowed null-move guard" |
| `b9f5b96` | S112 | "no-verdict at the cap: a zero, the code kept for S022" -> "per-move futility in quiescence at its seeds" | the body's first words; the subject named the outcome, not what was measured; S113's row reads "ProbCut stays at its seeds" |
| `775dafd` | S022 -> **S022 v1** | "1's H0: the exchange gate stays" -> "the exchange gate stays" | log `adocs/data/S022_v1_sprt.log` in its own block; candidate `a97bc1a` "(S022 v1)", pin `05ab85d` "S022 v1's SPRT pair", revert `39841ed` "Remove S022 v1's QsSeeGate switch"; S022's done stamp "Verdict 1 ... Verdict 2"; sibling row S022 v2 (`858a0ac`) |
| `7f46c0c` | S023 -> **S023 v1** | "1's H0: capture history in the margins reverts" -> "capture history in the margins reverts" | candidate `adb5d78` "(S023 v1)"; S023's done stamp "verdict 1 of two (DEC-273) ... verdict 2 not run"; S039's done file "S023 v1 in". Against: its log and script are `S023_sprt.*`, no `v1` |

The descriptions keep each subject's own words after the garbled prefix; none is invented.

**The `run` column.** `OVERRIDE_COLUMNS` is `sha what by run`; a header is either the first three or all four. Under
four, a row has three cells (the subject's run kept) or four (the run replaced). `read_overrides` returns `"run": None`
for a row without the cell; `apply_overrides` sets `run` and `what`, nothing else.

**New refusals**, exit 2 naming file and line, each seen red on a fixture:

| refused | where |
|---|---|
| a `run` cell that `RUN_RE` does not match whole: empty, blank, `S98`, `S0981`, `s098 v1`, `S098v1`, `S098 v`, a leading or trailing space, `S098 verdict 1`, `S098 V1`, `v1`, prose | `read_overrides` |
| a header other than `sha what by` or `sha what by run` (`run` before `by`, a fifth column, `Run`); a row of fewer than three cells or more than the header's | `read_overrides` |
| a row that changes nothing: its run (the subject's when it has none) and its `what` are both what the rule reads | `apply_overrides` |

S334's refusals are kept, unchanged in code and in their tests. The clean-subject refusal is checked before the new
one, so a run cell on a clean subject still reads "already reads out".

**Beyond the brief, said here:** "changes nothing" also refuses a `what`-only row whose `what` equals the rule's
reading. S334 accepted that row silently. The brief's "a `run` equal to the parser's reading is refused unless the
`what` also changes" applied to a row without the cell gives the same rule. A `run` equal to the parser's reading with
a new `what` is accepted, as the brief says.

## Readings

**Before any change**, at `e03366a`, clean tree, into `.tuning/s337/`: `--table` > `table_before.md`, bare >
`full_before.md`, `--figures`, `--no-git`, `--no-git --table`, `--audit-seed-class`, and `git log --format='%H %s'`.
`adocs/plan.md` lines 261-328 are byte-equal to `full_before.md`.

**Red first, two stages** (13 new cases; 58 in all):

1. Against `e03366a`'s `ledger.py`: `FAILED (failures=5, errors=4)` (`red_stage1.log`). Errors: the four-column
   header refused (3), `KeyError: 'run'` (1). Failures: the malformed-run and row-shape cases' needles absent (the
   header refusal fired instead), the run not applied (2), the changes-nothing case `LedgerError not raised`. Four
   passed at once: the precondition (`split_run_and_what` on "verdict 1" subjects, fixture and 775dafd's real one),
   a run cell under the three-column header (S334's column count already refuses it), a run equal to the rule's with
   a new `what` (accepted), a run cell on a clean subject (S334's refusal).
2. The column read and applied, neither refusal coded: `FAILED (failures=2)` (`red_stage2.log`), both `LedgerError
   not raised`: `test_a_run_that_is_not_a_run_id_is_refused`, `test_an_override_that_changes_nothing_is_refused`.
3. With the refusals: `Ran 58 tests`, `OK` (`green.log`).

**Mutants**, each through the test module against a scratch copy of `tools/ledger.py` (`mutants.log`), all killed:
M1 row length checked against four columns, not the header (kills the three-column-header case); M2 run checked by
`search`, not `fullmatch`; M3 no run check; M4 no changes-nothing check; M5 header checked on its first three cells;
M6 no clean-subject check (kills S334's case and the run one); M7 run not applied (3 cases).

**Byte equality**, after:

    python3 tools/ledger.py --table > .tuning/s337/table_after.md
    diff .tuning/s337/table_before.md .tuning/s337/table_after.md

    37c37
    < | S112 | no-verdict at the cap: a zero, the code kept for S022 | 19 h 2 m | 40000 | `{0, 5}` | **no verdict** |
    ---
    > | S112 | per-move futility in quiescence at its seeds | 19 h 2 m | 40000 | `{0, 5}` | **no verdict** |
    40c40
    < | S022 | 1's H0: the exchange gate stays | 0 h 25 m | 900 | `{-5, 0}` | H0, -65.22 +/- 17.66 |
    ---
    > | S022 v1 | the exchange gate stays | 0 h 25 m | 900 | `{-5, 0}` | H0, -65.22 +/- 17.66 |
    50c50
    < | S023 | 1's H0: capture history in the margins reverts | 4 h 21 m | 9330 | `{0, 5}` | H0, -3.80 +/- 5.18 |
    ---
    > | S023 v1 | capture history in the margins reverts | 4 h 21 m | 9330 | `{0, 5}` | H0, -3.80 +/- 5.18 |
    55c55
    < | S289 | Revert 's null-move table guard on its H0 (DEC-309) | 4 h 3 m | 8716 | `{-5, 0}` | H0, -7.42 +/- 5.07 |
    ---
    > | S289 | the null-move table guard | 4 h 3 m | 8716 | `{-5, 0}` | H0, -7.42 +/- 5.07 |

`--figures`, `--no-git`, `--no-git --table`, `--audit-seed-class`: 0 diff lines each. The bare command: the same 16
lines as `--table`, the figures paragraph unmoved. The new code with S334's override file alone (before the four
rows): `--table` byte-equal to `table_before.md`. `git log --format='%H %s'` before and after: 0 diff lines.

**Against the real log**, one scratch row each through `read_commits(REPO, <file>)` under the four-column header:
775dafd with run `S022 verdict 1`, exit 2 "not a run id"; with run `S022` and the rule's own description, exit 2
"changes nothing"; the same without the run cell, the same; bc550ea (Record S101's H1 for the threat family) with
run `S101 v2`, exit 2 "already reads out"; 775dafd with run `S022 v1` and the rule's description, accepted.

**plan.md's table, for the closing commit** (not edited here):

    python3 tools/ledger.py > .tuning/s337/plan_table.md

replaces `adocs/plan.md` from `| run | what it measured |` to the `**The ledger holds 64: ...` paragraph, whole; it
differs from what stands there (lines 261-328) in the four rows only. Re-run it at the closing commit if a verdict
lands first.

**Docs.** `DEV_MANUAL.md`'s ledger section: the override paragraph names the `run` cell, its header, the two new
refusals and the five rows. `tools/ledger.py`'s header and the override file's comments say the same. `MANUAL.md`
names neither the ledger nor the override file: nothing to change. `specs.md` does not name the ledger. No
`adocs/data/` file.

**Gates** (no C++ touched; no build; S124's screen running, load 11.2 on 12):

- `python3 tests/test_ledger.py`: 58 tests, OK.
- `ctest --test-dir build -R "ledger|test_plan_" -j4`: 6 of 6 passed.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check`: rc 0.
- `tools/plan_prose_check.py` `--touches`, `--citations`, `--params`, `--gate`, `--prose`: rc 0 each.

**Found beyond the step.** None of the four is open after this. A run cell may name a step other than the subject's
(`S032 v1` for an S022 subject would be accepted); not refused, because no row needs it and a subject naming the wrong
step is conceivable. `db1b6fa` still prints no row, as S334 recorded.

## The fast check's two lows (2026-11-06)

1. `tools/ledger.py`'s header said an override "replaces the description and nothing else: run, numbers and class
   still come from the block", and the next sentence contradicted it. The run comes from the subject, never the
   block. The paragraph is rewritten to say what is replaced.
2. **A run cell naming another step than the subject's is refused** in `apply_overrides`: the cell qualifies the
   subject's step and never replaces it, so a typo cannot move a verdict to another step's row. Red first:
   `test_a_run_naming_another_step_is_refused` (`S032 v1`, `S023 v1`, `S220` on an S022 subject) `FAILED
   (failures=1)`, `LedgerError not raised` (`red_step_id.log`); green after, 59 of 59. The override file's comments
   and `DEV_MANUAL.md` name the refusal. The four rows pass it. `--table` byte-equal to `table_after.md`; bare
   command 16 diff lines against `full_before.md`, as before; `--figures`, `--no-git`, `--no-git --table`,
   `--audit-seed-class` and `git log` 0. `plan_table.md` regenerated. `ctest -R "ledger|test_plan_"` 6 of 6, the
   five prose modes and format rc 0. This retires "Found beyond the step"'s first item.

## Proposed done stamp

done:       2026-11-06 -- `accepts` by clause. `tools/ledger_overrides.tsv` gains an optional `run` column (header
`sha what by run`; a row of three cells keeps the subject's run) and four rows by S337: `e47eb4a` -> S289 "the
null-move table guard", `b9f5b96` -> S112 "per-move futility in quiescence at its seeds", `775dafd` -> **S022 v1**
"the exchange gate stays", `7f46c0c` -> **S023 v1** "capture history in the margins reverts"; run ids from each
step's verdict-1 candidate commit ("(S022 v1)", "(S023 v1)"), S022's `S022_v1_sprt.log` and sibling S022 v2, and
S023's done stamp "verdict 1 of two". `git log` unchanged. Refused with exit 2, each seen red on a fixture: a `run`
that `RUN_RE` does not match whole, a header other than the three or four columns, a row longer than its header, and
a row that changes nothing the rule reads (run and `what` both the rule's; this also refuses a `what`-only no-op,
which S334 accepted), and a `run` naming another step than the subject's (the fast check's second low). S334's refusals kept. Red first: 5 failures and 4 errors with no mechanism, then 2 `LedgerError
not raised` with the mechanism and no refusal; green 59 of 59; seven mutants of the new checks all killed. `--table`
against `e03366a`'s: the four rows are the only diff; `--figures`, `--no-git`, `--no-git --table` and
`--audit-seed-class` byte-equal. `adocs/plan.md`'s table is `python3 tools/ledger.py > .tuning/s337/plan_table.md`,
pasted whole by the coordinator. `DEV_MANUAL.md` ledger section updated; `MANUAL.md` checked, nothing to change; no
`adocs/data/` file. Gates under S124's screen: `test_ledger` and the five `test_plan_*` 6 of 6, format and the five
`plan_prose_check.py` modes rc 0; no C++ touched, no build.
