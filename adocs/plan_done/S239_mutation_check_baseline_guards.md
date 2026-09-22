id:         S239
goal:       tools/mutation_check.py refuses a baseline that ran no tests and refuses a fixture whose tests/ or adocs/ differ from the tree it claims to measure
accepts:    a run whose ctest label selects zero tests stops at the baseline with a failure marker and a message naming the label, never a score (a fixture with `--label` set to a ctest label that matches nothing reproduces today's "baseline green, ? tests" and must fail); `require_clean_src` (or its successor) covers `tests/` and `adocs/` as well as `src/` in the fixture worktree, and the run's log header names the sha the fixture is actually at plus "dirty" with the paths when it is not clean; each refusal has a case in `tests/test_mutation_check.py`'s fast suite that fails when the guard is removed; `DEV_MANUAL.md`'s mutation section states both refusals; no change to any mutant file, to the kill logic, or to `src/`
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   any mutant, any engine code, the mutation score's arithmetic; a re-run of past mutation results (they stand as recorded, with S097's two-run history as the example of what the gap cost)
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-22
done:       2026-09-22 21:45 -- `tools/mutation_check.py` refuses two things it accepted before, and the fast suite proves each refusal. `require_baseline_tests` reads the test count out of ctest's own last summary line, after the exit code has been read, and stops the run with `MUTATION-RUN-FAILED` naming the label when no test ran -- the `--label` matching nothing that scored twenty survivors over an empty suite on 2026-09-21 now stops at the baseline; a ctest that timed out or never started is named by its exit code and not blamed on the label. `require_clean_fixture` replaces the `src/`-only check with `git status --porcelain` over the whole fixture worktree (build directories are ignored, so a healthy fixture is clean), the run's header names the fixture's sha with `dirty:` and the paths when there are any, and a further header line names where the mutant list was actually read from -- the argument resolves against the process cwd, so the documented invocation reads the main tree's list and the fixture's copy is never opened, which is why the cold fast check had the `tools/mutants` guard entry dropped. Six cases in `tests/test_mutation_check.py` (24 to 30), each observed red under the cut it guards against (the baseline guard deleted: 3; its exit-code branch removed: 1; the status narrowed to `src/`: 2; the list line dropped: 1; the first summary read instead of the last: 3); the suite 30 of 30 under the match's load; two defects of the step's own code found and fixed on the way (porcelain sliced after `git()` stripped it; the first summary line taken where a failing test's output can embed one). `DEV_MANUAL.md`'s mutation section states both refusals and the header format; `MANUAL.md` has no mutation content. No mutant, no kill logic, no score arithmetic and no `src/` file changed, so no `Bench:` line was owed and no second tier (DEC-141 binds steps touching the search). Cold fast check before the landing: two findings of record and three trivial, all closed. Landed as `4c583ce` after the gate (both fast suites, the format check) -- twice: the first landing commit was lost to the 14:04 power cut with six unflushed objects, and the intact working tree was re-committed after the repository was repaired to `9d21ada`. Test-side filler under DEC-171, named by id in the pre-registrations taken while it was open (S188's, S132's confirmation). Written by an Opus 5 subagent from the step file's description of the two gaps.

## Why this exists

S097 verdict 2's agent (2026-09-21) found both gaps by walking into them.
Its first mutation launch passed `--label S097_v2`, a ctest label no test
carries; the tool printed `baseline green, ? tests`, ran every mutant against
an empty suite in 154 s and scored all twenty as survivors -- a score that
would have read as twenty gaps in coverage had the agent not noticed the
wall time. Its second run measured a fixture whose `src/` was clean but whose
`tests/test_search.cpp` and `adocs/specs.md` had been edited in place, so the
log header named a commit whose tests did not contain the mined mate row the
run's E21 kill depended on; `require_clean_src` guards `src/` alone and said
nothing. The kills were valid for the landing tree -- the cold fast check
diffed the fixture to prove it -- but the evidence had to be reconstructed
by hand, and a tool that accepts a zero-test baseline will one day print a
clean 100 % over nothing.

Test-side under DEC-171: neither gap can move a reported score, move or
line, so it is filler behind the next strength step and is named by id in
the pre-registration of every run taken while it is open. Created
2026-09-22 by the coordinator from the agent's Report 4 and the fast check's
finding 2.

## Cost

A few dozen lines of Python and two cases; no machine beyond the fast suite.

## What landed, 2026-09-22

Two refusals in `tools/mutation_check.py`, each printing `refused: ...` and
`MUTATION-RUN-FAILED` with no table, no score and no mutant row:

- `require_baseline_tests`, called on the baseline result before the red check,
  reads the count out of ctest's own summary and refuses at zero:
  `the baseline ran no tests: ctest printed no summary line for label
  'S097_v2' in <build dir>; every mutant would score a survivor over an empty
  suite`, and `ctest ran 0 tests` for the other shape, a summary counting to
  zero. Before the red check on purpose -- a label that selects nothing exits
  0, so the red check passes it, and when ctest does exit non-zero its
  `? of ? failed ()` names nothing about the label that is the real fault.
- `require_clean_fixture`, the widened `require_clean_src`, over the **whole**
  worktree with no pathspec. The header is printed before it refuses, so a
  refused run still records its fixture: `worktree <path> at <sha> clean`, or
  `worktree <path> at <sha> dirty: CMakeLists.txt adocs/specs.md`.
  `fixture_state` reads sha and status once and hands both to the header and
  the guard.

The header also names the mutant list, which is the one thing a run reads that
the fixture's sha does not describe: `mutants_state` reports the path
`load_mutants` opened -- resolved against the process's cwd, so the documented
invocation reads the main tree's list while the fixture's copy stays shut --
and that path's status in whatever tree holds it, `list <path>   clean`,
`dirty: <paths>` or `outside any git tree`.

Six cases in `tests/test_mutation_check.py`, 24 to 30, on a sandbox that now
commits a `tests/`, an `adocs/`, a root `CMakeLists.txt` and a `.gitignore`
covering `build/` -- the last because the tool checks the whole tree and the
run writes its logs inside it, which is what makes the real fixture clean
whole -- and a `ctest` stub that knows one label and answers any other as the
real one answers a label no test carries:
`test_a_label_that_selects_no_test_stops_the_run`,
`test_a_baseline_summary_counting_to_zero_stops_the_run`,
`test_a_baseline_whose_ctest_never_ran_is_not_blamed_on_the_label`,
`test_a_fixture_dirty_outside_src_refused`,
`test_the_header_names_the_fixture_sha_and_what_dirties_it` (dirtying the root
`CMakeLists.txt`, which an allowlist would not have looked at) and
`test_the_header_names_where_the_mutant_list_was_read_from`. The suite's
`worktree_dirty` spans the whole tree now, which strengthens the existing
clean-after-every-mutant assertions.

Five cuts, each red observed in a scratch copy of the tool and the suite, not
argued. The baseline count guard deleted: both count cases fail and the run
reproduces 2026-09-21 exactly -- `baseline green, ? tests` and `K01_killed ...
survived ... fast ?/? bench same`. Its exit-code branch removed: the
never-ran case fails, the label blamed for a ctest that did not start. The
status narrowed back to `src/`: the dirty-fixture and header cases fail on a
run that exits 0 and scores the mutant against a tree whose `tests/` is not the
one its header names. The `list` line dropped: the mutant-list case fails. The
count taken from the log's first summary instead of its last: three cases fail,
including `test_red_baseline_stops_the_run`.

Two defects in this step's own code, both found by a case and fixed before the
case was kept (BUGS rule). `dirty_paths` sliced porcelain at a fixed column
while `git` strips its whole output, so the first row of an unstaged change
lost its leading space and the header printed `ools/mutants/...`; it splits the
status off now. And `ctest` read the log's first summary line, which
`--output-on-failure` lets a failing binary's own output supply -- this suite
is such a binary -- so a nested `out of 0` would have turned the new guard into
a false refusal; the stub prints one inside its failing test's output and the
last match is what is read.

Test evidence, 2026-09-22 with the S132 confirmation match on every core
(loadavg 14.61, twelve cores): 30 cases green, `python3
tests/test_mutation_check.py` 6.9 s and `ctest --test-dir build -R
test_mutation_check` 7.1 s; `python3 -m py_compile` on both files; `sh -n` and
`bash -n` on the extracted `ctest` stub; `plan_prose_check.py --citations`,
`--touches` and `--gate` clean; the documented header format checked against a
real sandbox run's own output rather than transcribed. **Not run, machine
held: the two-build gate** (`ctest -L fast` in `build` and `build-tune`,
`clang-format.sh --check`) and any real mutation pass. No `src/`, no mutant
file, no kill logic and no score arithmetic was touched.

The fast check over the first draft found two real and three trivial, all five
fixed here: the `tools/mutants` entry in the allowlist guarded a copy no run
reads (it became the `list` line), the allowlist itself left the root
`CMakeLists.txt`, `cmake/` and `clang-format.sh` unguarded (it became a
whole-tree check), the first-summary read, the exit-code blame, and a
117-character line in `DEV_MANUAL.md` whose "Three of them" had come to read as
three of this step's cases.
