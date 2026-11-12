id:         S342
goal:       the mutation tool refuses any malformed entry of a mutant file by name, before the baseline, whatever is missing or mistyped
accepts:    (1) an entry from a file's own `m` that lacks `file`, `id`, `klass` or `note`, one that is not a mapping, and a mutant whose `file` is not a string are each refused naming the entry and its file, with no traceback and no baseline run, where today each dies on a `KeyError`, `ValueError` or `AttributeError`, the last two cases after the baseline has run; each seen red first; (2) a row left `unmeasured` because its only listed row did not run or timed out says why in its note; (3) `DEV_MANUAL.md` says a baseline row that reads `(Not Run)` is refused as red, where it now says every did-not-run row is named after the green line; (4) every existing case of `tests/test_mutation_check.py` passes unchanged; no mutant's meaning moves
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   running any mutant on the real tree; re-reading any kill; the parked full mutation pass
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator, 2026-11-12
done:       2026-11-12 -- `accepts` by clause, each red seen first on the unchanged tool (15 of 15 new checks failing, the control passing). (1) `normalise` checks every entry's shape, the tool's own `m`'s too, and refuses before the baseline, naming `<id> in <file>` or `entry <n> in <file>` from 0 where no id is usable: an entry that is not a mapping, one lacking `file`, `id`, `klass`, `note` or `pairs`, and a non-string `id`, `file`, `klass`, `note` or `origin`, or `pairs` that are no list. The accept's "the last two cases after the baseline" was read as the step's Why has it: a missing `klass` or `note` died after the baseline, the not-a-mapping and non-string-`file` cases before it; six shapes in all died after it. (2) An `unmeasured` row's note ends with why: `[no test asserted: <test> (Timeout)]`, `(Not Run)`, or `[ctest exited <n> and listed no test]`. (3) `DEV_MANUAL.md` says the did-not-run line covers Skipped and Disabled rows and that a `(Not Run)` baseline is refused as red, naming the test. (4) No existing assertion edited, +113 lines and none removed; `tools/mutants/` untouched, and the 264 mutants and both evidence copies load field for field as under the parent's tool. `tests/CMakeLists.txt` changed beyond `touches:`, one comment's case count. Gate on a machine carrying another session's soak test: 61 of 61 in both builds, format clean, bench 4707943, no `src/`. `MutationCheckTest` 40 cases, 18.9 to 19.4 s inside `ctest -j12` against TIMEOUT 60. One fast check, an Opus subagent: nothing real, commit as is. Known and left, no step filed, neither reachable from the list: a file whose own `M` is not iterable still dies in `load_mutants` on a `TypeError`, and an empty-string `id` passes the check. The 3600 s ceiling's note is untested, the sandbox cannot reach it. `MANUAL.md` checked and unchanged. No Debug self-play: no `src/`.

## Why

Found by S339's implementer and its fast check, 2026-11-12, beyond that step and not fixed there. Probed on the fixed
tool (`.tuning/coord/S339/evidence/`), the `id` and the `klass`/`note` halves read from the code:

- An own-`m` entry without `file` dies on a `KeyError` in `validate`; one without `id` by the same path.
- An own-`m` entry that is not a mapping dies in `normalise` on `dict(entry)`.
- A mutant whose `file` is not a string dies on an `AttributeError` at `rel.startswith`, through the tool's own `m` too.
- An own-`m` entry without `klass` or `note` passes `validate` and dies on a `KeyError` in `main` when its row is
  built, after the baseline; the `finally` still reverts the mutant.
- From the fast check, cosmetic: an `unmeasured` row from a lone `(Not Run)` or a lone timeout carries no note; one
  sentence of `DEV_MANUAL.md` holds for Skipped and Disabled rows and not for `(Not Run)`.

None is reachable from `tools/mutants/` as it stands: 264 mutants, 276 pairs, all validating, no entry of these
shapes. Filler under DEC-171, the lowest in Open: a tool, no play. Agent-only, no `src/`, owns no run. It binds before
the parked full mutation pass.

## 2026-11-12: done

### What changed

**`tools/mutation_check.py`**, two fixes. No mutant's meaning moves and nothing under `tools/mutants/` changed.

- **One shape check for every entry.** `load_mutants` now passes every entry through `normalise(entry, path, index)`,
  the tool's own `m`'s entries as well as those of a file with its own `m`. `normalise` refuses, before the baseline:
  - an entry that is not a mapping: `entry <n> in <file>: a <type>, not a mapping of a mutant's fields`
  - an own-`m` entry that leaves out `id`, `file`, `klass`, `note` or `pairs` (a None `pairs` counts as left out, as
    S339 had it): ``<where>: an entry from the file's own `m` gives no <fields>``. The pairs case reads exactly as
    S339's message, so its test passes unchanged.
  - an `id`, `file`, `klass` or `note` that is not a string: `<where>: <field> is not a string: <value>`
  - an `origin` that is neither None nor a string, and `pairs` that are not a list or tuple. These two are beyond the
    shapes `accepts` lists. They are within the goal's "whatever is missing or mistyped", and each was a traceback too
    (`write_tsv` after the whole run; `list(5)` in `normalise`).
  - `<where>` is `<id> in <file>`, or `entry <n> in <file>` (index in the file's entries, from 0) when the id is
    missing or is not a string.
- **An `unmeasured` row from its suite says why.** `ctest()` returns `why`, and `main` appends it to the row's note on
  the inconclusive branch: `[no test asserted: <name> (<how>), ...]` for the rows listed, `[ctest exited <code> and
  listed no test]`, or `[ctest hit this tool's 3600s ceiling]`. The note shows in the "not what the list expects" lines
  and the TSV's `note` column.

**`tests/test_mutation_check.py`**, `MutationCheckTest` 38 → 40 cases. No existing line edited or removed
(`git diff` removes 0 lines).

- `test_a_malformed_entry_refused_by_name_before_the_baseline`: a control (a whole own-`m` entry loads and reads
  `killed`), then twelve subtests in `MALFORMED_ENTRIES`. Seven are own-`m` shapes: no file, no id, no klass, no note,
  not a mapping, a non-string file, non-list pairs. Five are the tool's `m`: a non-string file, id, klass, note and
  origin. A whole entry stands first in each file, so an index-named entry reads `entry 1`. Each asserts the refusal
  line, no traceback, no `baseline green`, **no ctest call at all** (the stub's argument log absent), a clean
  worktree, and the last line `MUTATION-RUN-FAILED`.
- `test_an_unmeasured_row_says_why_in_its_note`: subtests `T01` (lone Timeout), `N01` (lone Not Run) and `C01` (ctest
  exits 1 listing nothing). Each asserts the reason in the "not what the list expects" line and in the TSV note.

**`DEV_MANUAL.md`**, the mutation section:

- The sentence after the did-not-run paragraph now covers `(Skipped)` and `(Disabled)` only. A new sentence says a
  `(Not Run)` baseline is refused as red, naming the test, with the line the tool prints. This was verified on a
  sandbox first (`evidence/baseline_not_run.txt`: `refused: the unmutated worktree is red: 1 of 2 failed (test_gone)`)
  and read from `main`'s red-baseline branch.
- The unmeasured row's note, with its three texts.
- The anchors test's red list gains "an entry is malformed". The refusals paragraph gains S342's shapes and texts and
  the index naming.
- The driver's gate paragraph: forty cases, 14.1 s alone, and S342's reds.

**`tests/CMakeLists.txt`**: the comment above `test_mutation_check` gains the count, 40, and 14.1 s alone. This is a
comment only, outside `touches:`, as S339 did. `MANUAL.md` names neither the tool nor the mutants (0 hits), so it was
checked and is unchanged.

### Accepts, with evidence

Red first: the two new cases against the unchanged tool gave `Ran 2 tests`, `FAILED (failures=15)`, all 12 + 3
subtests red, with the control green (`.tuning/coord/S342/evidence/red_first_new.log`).

**(1) Each malformed entry is refused by name before the baseline.**
- Red, before the baseline:
  - no file: `KeyError: 'file'` at `rel = mut["file"]` in `validate`
  - no id: `KeyError: 'id'` at `mid = mut["id"]` in `validate`
  - not a mapping: `ValueError: dictionary update sequence element #0 has length 7; 2 is required` at `dict(entry)` in
    `normalise`
  - a non-string file, own `m` and tool's `m` alike: `AttributeError: 'int' object has no attribute 'startswith'` in
    `validate`
  - non-list pairs: `TypeError: 'int' object is not iterable` in `normalise`
- Red **after `baseline green`**:
  - no klass, no note: `KeyError` at the row built in `main`
  - a non-string id: `TypeError` at `mut["id"] + "_build.log"`
  - a non-string klass, note or origin: `TypeError: sequence item <n>: expected str instance, int found` in
    `write_tsv`, after the whole table
- Every one ended `MUTATION-RUN-FAILED: <Error>: ...` after a traceback. The `finally` left the worktree clean
  each time.
- Green: each reads its refusal line (all twelve in `evidence/refusal_lines.txt`), e.g.
  ``refused: entry 1 in <file>: an entry from the file's own `m` gives no id`` and
  `refused: Q02_bad in <file>: file is not a string: 5`. No traceback, no ctest call, last line
  `MUTATION-RUN-FAILED`.
- The accept's "the last two cases after the baseline has run" reads as the `klass`/`note` pair, as `## Why` has it.
  Observed: the non-mapping and non-string-file cases die before the baseline, and `klass`, `note` and the non-text
  `id`/`origin` after it.

**(2) An `unmeasured` row says why.**
- Red: `T01_timeout_only: unmeasured, expected killed -- a ceiling hit and nothing else` with nothing after it, and
  the same for `N01` and `C01`.
- Green: `... -- a ceiling hit and nothing else  [no test asserted: test_x (Timeout)]`,
  `... [no test asserted: test_gone (Not Run)]` and `... [ctest exited 1 and listed no test]`, and the same in the TSV.

**(3) `DEV_MANUAL.md`**: as above, the `(Not Run)` baseline claim read from the code and seen on a sandbox.

**(4) Every existing case unchanged, no mutant's meaning moves.**
- All 38 + 4 existing cases pass. The test diff adds 113 lines and removes none.
- `git diff --stat tools/mutants/` is empty.
- The read-only probe (`evidence/real_list_count.txt`; imports `load_mutants` and `validate`, runs no mutant):
  - the real list: 264 mutants, 34 files, 276 pairs, 259 `killed` and 5 `equivalent` expected, validating. This is
    S339's reading.
  - the evidence copies: `adocs/data/S191_mutants.py` loads 6 entries, `adocs/data/2026-09-04_test_review/mutants.py`
    loads 34.

### Gates

Load 3.4 at the start on 12 threads.

- The gate ran exactly as briefed, after the last edit to code, tests, `DEV_MANUAL.md` and `tests/CMakeLists.txt`:
  - rc 0
  - `ctest -L fast` on `build`: **61 of 61**, 52.1 s; on `build-tune`: **61 of 61**, 52.0 s
  - `./clang-format.sh --check`: rc 0
  - `test_mutation_check`: 18.9 s (Release) and 19.4 s (tune), against its TIMEOUT 60
  - `test_mutant_anchors`: 0.14 s and 0.18 s
  - logs: `.tuning/coord/S342/gate_*.log`
- This section was written after the gate.
- `python3 tests/test_mutation_check.py`: 44 cases (40 + 4), OK, 12.8 s. `MutationCheckTest` alone: 40 cases, 14.1 s
  wall at load 3.3.
- DEC-141's second tier does not apply: nothing in `src/`, no rule.

### Unmeasured

- No mutant was run on the real tree and no kill was re-read (excludes).
- The `[ctest hit this tool's 3600s ceiling]` note is untested: the sandbox cannot reach a 3600 s ceiling without
  patching the constant. It is one line beside the two tested ones.
- `tools/plan_prose_check.py` was not run on this file or on `DEV_MANUAL.md`.

### Found beyond the step (not fixed)

- An own-`m` file whose `M` is not iterable (`M = 5`) still dies in `load_mutants` on a `TypeError`, naming no file.
  This is a file-level shape, not an entry, so it is outside this step's goal. A dict `M` is iterated by its keys, and
  each key is then refused as not a mapping.
- An empty-string `id` passes the shape check and validates. It would name its rows and logs `""`.
