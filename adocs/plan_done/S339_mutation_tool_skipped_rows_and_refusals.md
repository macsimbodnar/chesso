id:         S339
goal:       the mutation tool never scores a test that did not run as a kill, and refuses a malformed mutant by name instead of crashing
accepts:    (1) a ctest row that reads skipped or not run is not a kill: on a fixture where one test skips with exit 77 an unmutated tree reads no kill and a mutant killed by a real failure still reads killed, the first seen red on the tool as it stands; a run whose baseline has a skipped test says so in its report, naming the test; (2) `validate` refuses a pair that is not an (old, new) pair of strings, naming the mutant and its file, where it now dies on a `ValueError`; (3) the loader refuses a file that brings its own `m` and gives no pairs, naming the file, where `normalise` now dies on a `KeyError`; (4) each seen red first; every existing case of `tests/test_mutation_check.py` passes unchanged; no mutant's meaning moves
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   running any mutant on the real tree; re-reading any kill; the loader's `*.py`-only, one-directory scan, which stays as it is and is stated in DEV_MANUAL; the parked full mutation pass
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator, 2026-11-12
done:       2026-11-12 -- `accepts` by clause, each red seen first on the unchanged tool (`Ran 38`, `FAILED (failures=10)`, the 33 existing cases passing). (1) A ctest row that did not run -- `Skipped`, `Disabled` or `Not Run`, the three shapes read from ctest 3.28.3 -- is no kill: the quiet mutant over a baseline with a skipping test read `killed` and reads `equivalent`, a mutant a real failure kills stays `killed` with only the failed row in its TSV, a lone `(Not Run)` reads `unmeasured`, and the run prints `baseline did not run: <test> (Skipped)`. (2) `validate` refuses a pair that is not an (old, new) pair of strings as `<id> in <file>: pair <n> is not an (old, new) pair of strings`, before the baseline, where it died on `ValueError` or `TypeError`. (3) The loader refuses an entry of a file's own `m` that gives no pairs, naming it, where `normalise` died on `KeyError`. (4) No existing assertion edited; `tools/mutants/` untouched, 264 mutants in 34 files still validating. `tests/CMakeLists.txt` changed beyond `touches:`, one comment's case count. Gate on a machine carrying another session's soak test: 61 of 61 in both builds, format clean, bench 4707943, no `src/`. `test_mutation_check` 38 cases, 17.5 to 18.3 s inside `ctest -j12` against TIMEOUT 60. One fast check, an Opus subagent: nothing real, commit as is. S342 filed for four further crashes on malformed entries and two cosmetic notes. `DEV_MANUAL.md` updated, `MANUAL.md` checked and unchanged. No Debug self-play: no `src/`.

## Why

Found by S338's implementer, 2026-11-11, beyond its step and not fixed there.

- **A skipped test is scored as a kill.** The tool's failed-row pattern also matches ctest's "did not run ... (Skipped)"
  rows. Seen on a two-test scratch project: an unmutated tree scored `killed`. `test_bare_fen_relay` skips with exit 77
  when no python imports python-chess, so on such a machine every mutant would read killed. Latent on the workstation,
  where the venv answers; the MacBook's state is not read.
- A pair that is not two strings crashes `validate` with a traceback naming neither mutant nor file.
- A file with its own `m` and no pairs crashes the loader the same way; only two evidence copies under `adocs/data/`
  use that form.

Filler under DEC-171: a tool, no play, no score, no UCI output. It binds before the next mutation reading taken on a
machine other than the workstation, and before the parked full pass. Agent-only, no `src/`, owns no run.

## 2026-11-12: done

### What changed

**`tools/mutation_check.py`**, three fixes. No mutant's meaning moves and nothing under `tools/mutants/` changed.

- **Rows for a test that did not run.** `CTEST_DID_NOT_RUN` is `Skipped`, `Disabled` and `Not Run`. Real ctest 3.28.3
  prints all three in the `N - name (how)` shape `CTEST_FAILED_RE` matches: the first two under "The following tests
  did not run:" with exit 0, `Not Run` (missing executable) under "FAILED" with exit 8. This was read on a scratch
  project, logs in `.tuning/coord/S339/evidence/real_ctest_{skip,skipfail,disabled,notrun}.log`.
  - `ctest()` now splits the rows. `tests` holds the rows of tests that ran and failed; `skipped` holds the rows of
    tests that did not run.
  - `asserted` and `extract_kills` read `tests` alone, so the TSV's `failing_tests` is `tests` alone too.
  - A run whose only listed row is `(Not Run)` exits non-zero with nothing asserted. That is the existing
    `inconclusive` branch, so it reads `unmeasured`.
  - After the `baseline green` line, a baseline with such a row prints
    `baseline did not run: <name> (<how>), ... -- a test that did not run kills no mutant`.
  - The red-baseline refusal still lists every row ctest listed. A `(Not Run)` row is the only one there when an
    executable is missing.
- **`validate`** checks each pair before unpacking or counting it. Anything but a tuple or list of two strings gets
  `<id> in <file>: pair <n> is not an (old, new) pair of strings: <repr>`. Lists of two strings are accepted as before.
- **`normalise`** refuses an entry from a file's own `m` whose `pairs` is missing or None, with
  ``<id> in <file>: an entry from the file's own `m` gives no pairs``. Like the loader's other refusals, it raises at
  once.
- `load_mutants`' docstring states the `*.py`-only, one-directory scan, which stays as it is.

**`tests/test_mutation_check.py`**, `MutationCheckTest` 33 → 38 cases. No existing case or assertion was edited. The
only removed line is one docstring line, rewrapped to name the new stub markers.

- The ctest stub gains four branches, each placed before `MUTANT_SEEN` and taken only on its own marker: `SKIP_ONE`,
  `DISABLE_ONE`, `NOT_RUN`, and `SKIP_ONE` beside `MUTANT_SEEN`. They copy the shapes of the real logs above.
- `Sandbox.commit_marker` commits a marker into the worktree so that the baseline carries it.
- The new cases:
  - `test_a_test_that_did_not_run_is_no_kill_and_the_run_names_it`: subtests Skipped and Disabled
  - `test_a_real_failure_beside_a_skipped_test_is_still_killed`
  - `test_a_test_whose_executable_is_missing_is_no_kill`
  - `test_a_pair_that_is_not_two_strings_refused_by_name`: five subtests (a bare string, one string, three strings, a
    number for the anchor, a number for the replacement)
  - `test_an_own_m_entry_without_pairs_refused_by_name`: with a positive control, where the same file giving its pairs
    loads and reads `killed`

**`DEV_MANUAL.md`**, the mutation section:

- a paragraph on rows for tests that did not run, after the verdict rules, with the line the run prints
- the anchors test's red list gains the malformed pair, and the two refusals' texts
- the loader's `*.py`-only, one-directory scan is stated, as this step's excludes asks
- the driver's gate paragraph: 38 cases, its time, and S339's reds

**`tests/CMakeLists.txt`**: the comment above `test_mutation_check` gains the new count and time. This is a deviation
from `touches:`, a comment only, made so that the count does not go stale. `MANUAL.md` names neither the tool nor the
mutants, so it was checked and is unchanged.

### Accepts, with evidence

Red first: `python3 tests/test_mutation_check.py MutationCheckTest` against the unchanged tool, with the new tests in
place, gave `Ran 38 tests`, `FAILED (failures=10)` (`.tuning/coord/S339/evidence/red_first_sandbox.log`). The
existing 33 passed in that run.

**(1) A row for a test that did not run is not a kill.**
- Red on Skipped: the quiet mutant `E01_equivalent` over a baseline with a skipped test read
  `E01_equivalent yes 0/2 same killed equivalent`, with `[test_skips] - | (no assertion text in the log)` as what
  caught it and `mutation score 1 of 1`.
- Red on Disabled: the same mutant read `yes 0/1 same killed` with `[test_off]`.
- Red on Not Run: `N01_not_run yes 1/2 same killed killed`, `MUTATION-RUN-DONE`
  (`red_first_not_run_output.log`).
- Red beside a real failure: `K01_killed` read killed as it should, but listed `[test_skips]` among what caught it.
- Green: `E01_equivalent ... equivalent` and `mutation score 0 of 0` for both shapes, and `N01_not_run ... unmeasured`.
  `K01_killed yes 1/2 ... killed` lists `[test_x]` alone, and its TSV `failing_tests` is `test_x(Failed)`.
- The baseline report: `baseline did not run: test_skips (Skipped) -- a test that did not run kills no mutant`, and
  the same for `test_off (Disabled)`, both asserted.

**(2) `validate` refuses a malformed pair by name.**
- Red: a bare string and three strings each gave `ValueError: too many values to unpack (expected 2)` at
  `for index, (old, new) in enumerate(mut["pairs"])`, and one string gave `not enough values to unpack`. A number for
  the anchor gave `TypeError: must be str, not int` at `text.count(old)`. A number for the replacement passed
  `validate`, ran the baseline, and died in `apply_mutant` with `TypeError: replace() argument 2 must be str, not int`.
  Every one ended `MUTATION-RUN-FAILED: <Error>: ...` after a traceback, naming no mutant.
- Green: each reads `Q01_malformed_pair in <file>: pair 0 is not an (old, new) pair of strings: <repr>`. None prints a
  traceback or a baseline line, the worktree stays clean, and the last line is `MUTATION-RUN-FAILED`.

**(3) The loader refuses an own-`m` entry without pairs by name.**
- Red: `KeyError: 'pairs'` at `out["pairs"] = list(out["pairs"])` in `normalise`, ending
  `MUTATION-RUN-FAILED: KeyError: 'pairs'`.
- Green: ``O02_own_m in <file>: an entry from the file's own `m` gives no pairs``, with no traceback and the last line
  `MUTATION-RUN-FAILED`. The control with pairs given reads `O02_own_m yes 1/1 ... killed`.

**(4) Red first, every existing case unchanged, no mutant's meaning moves.**
- The red run is above. `git diff tests/test_mutation_check.py` removes the one docstring line and nothing else.
- The real list read through the changed loader and validator gives 264 mutants in 34 files, 276 pairs, 259 `killed`
  and 5 `equivalent` expected, all validating. This is S338's reading (`real_list_count.txt`).
- `git diff --stat tools/mutants/` is empty. Both evidence copies under `adocs/data/` still load (6 and 34 entries).

### Gates

Load was 3.4 at the start on 12 threads, and a soak test that is not ours was running.

- The gate ran exactly as briefed, `CLANG_FORMAT_MAJOR=22`, `-j12`:
  - both builds: rc 0
  - `ctest -L fast` on `build`: **61 of 61**, 52.4 s
  - `ctest -L fast` on `build-tune`: **61 of 61**, 53.1 s
  - `./clang-format.sh --check`: rc 0
  - `test_mutation_check` took 17.2 s (Release) and 18.3 s (tune), under its TIMEOUT 60. `test_mutant_anchors` took
    0.14 s and 0.16 s.
  - Logs: `gate_*.log`
- No ctest entry was added.
- `python3 tests/test_mutation_check.py`: 42 cases (38 + 4), OK, 11.5 s.
- `MutationCheckTest` alone: 38 cases, 12.7 s wall at load 9.5.
- After the gate, `tests/CMakeLists.txt` changed by a comment, and `DEV_MANUAL.md` and this file changed. The same
  gate was run again after those edits, at load 4.5, and gave rc 0:
  - **61 of 61** on `build`, 51.5 s, and **61 of 61** on `build-tune`, 52.0 s
  - `test_mutation_check` took 17.5 s and 17.6 s; `test_mutant_anchors` took 0.14 s and 0.15 s
  - `./clang-format.sh --check`: rc 0
  - Logs: `gate2_*.log`
- `tools/plan_prose_check.py`:
  - `--prose`, `--touches` and `--gate` on this file: rc 0. The default mode: rc 0.
  - `--citations DEV_MANUAL.md`: 8 flagged, all at lines 1685 to 2514, outside the mutation section. `HEAD`'s
    `DEV_MANUAL.md` flags the same 8, so they predate this step.
- DEC-141's second tier does not apply: nothing in `src/`, no rule.

### Unmeasured

- No mutant was run on the real tree and no kill was re-read (excludes).
- Nothing was observed on a machine where `test_bare_fen_relay` skips. The fix is proved on the sandbox stub, which
  copies the real ctest 3.28.3 shapes, and on the scratch project's logs. It was not proved on a real mutation run.
- ctest versions other than 3.28.3 were not read.

### Found beyond the step (not fixed)

All are probed with the fixed tool on scratch files (`.tuning/coord/S339/evidence/further_defects.txt`). Each is
tool-only DEC-171 filler, of the same kind as (2) and (3).

1. An own-`m` entry without `file` dies on `KeyError: 'file'` in `validate`, and one without `id` dies the same way
   (that one is read from the code, not probed).
2. An own-`m` entry that is not a dict dies in `normalise` on `dict(entry)`. Probed: `ValueError: dictionary update
   sequence element #0 has length 3`.
3. A mutant whose `file` is not a string dies on `AttributeError` at `rel.startswith` in `validate`, through the
   tool's own `m` as well.
4. An own-`m` entry without `klass` or `note` passes `validate`. It then dies on a `KeyError` in `main` when its row is
   built, after the baseline has run. That last part is read from the code. The `finally` reverts, so the worktree is
   left clean.
