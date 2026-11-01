id:         S305
goal:       `tools/mutation_check.py`'s progress line stops calling an unread bench `same`
accepts:    a mutant whose bench was never read prints `n/a` on the progress line, as the final table already does; a case in `tests/test_mutation_check.py` observed red on today's line before it goes green
touches:    tools/mutation_check.py, tests/test_mutation_check.py, DEV_MANUAL.md
excludes:   what a run kills or how it decides; the final table, which is right
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01, a mutant whose bench was never read prints `n/a` on the progress line and a stillborn one `-`, each as the table does, through the table's own `bench_column`; `test_the_progress_line_reads_the_bench_as_the_table_does` observed red on today's line (B01, D01) then green; both builds 50 of 50 fast, `./clang-format.sh --check` and the five `plan_prose_check.py` modes clean; `DEV_MANUAL.md`'s case count corrected, `MANUAL.md` needs nothing; Tier 2 (DEC-141) does not bind, no `src/` file moved

## Why

Found 2026-10-31 reading S118's mutants. The progress line prints `bench same` whenever
`bench_moved` is false, and a mutant the suite kills before its bench is read leaves it false
with `bench_read` false too. The final table reads both and prints `n/a`. A run stopped before
its table, as S118's first was, leaves only the progress lines, so `same` there means "not seen
to move". S118 re-ran PT01 to PT13 rather than cite those rows.

Filler by DEC-171: it moves no score or line and no run's verdict.

## 2026-11-01: the fix (Opus 5.5 subagent, coordinated)

The progress line now prints `bench_column(row)`, the helper the table and `results.tsv`
already read, so the three have one source. The decision about a verdict is untouched: this
line is printed after `verdict_of` and nothing reads it back. Nothing in `tools/` or `tests/`
parses the line.

**The Why above has the mechanism wrong, and the code is what holds.** `main` reads the bench
before it runs ctest, so no suite kills a mutant before its bench is read. `bench_read` stays
false on three paths: the engine printed no bench line (PT07's table note says `[no bench
signature: no bench line]`), the mutant did not compile, or the build hit its ceiling. The
first prints `n/a`, the other two `-`, as the table does. So the defect reached stillborn rows
too, which the step did not name. The third path shares the second's branch in `bench_column`
and is not driven by a case: its ceiling is `BUILD_TIMEOUT_S`, 600 s.

Red, the new case against today's line, `test_the_progress_line_reads_the_bench_as_the_table_does`:

```
FAIL: ... (mutant='B01_bench_broken')
AssertionError: Regex didn't match: '(?m)^  B01_bench_broken\\s+\\S+\\s+fast \\S+\\s+bench n/a  ' not found in ...
FAIL: ... (mutant='D01_stillborn')
AssertionError: Regex didn't match: '(?m)^  D01_stillborn\\s+\\S+\\s+fast \\S+\\s+bench -  ' not found in ...
Ran 1 test in 0.174s
FAILED (failures=2)
```

Today's four lines in that run, the two read rows passing, which is what they are there for:

```
  B01_bench_broken                   killed      fast 1/1  bench same  0.0s
  D01_stillborn                      stillborn   fast -/-  bench same  0.0s
  S01_bench_moved                    survived    fast 0/1  bench moved  0.0s
  S02_bench_blind                    survived    fast 0/1  bench same  0.0s
```

Green after the one-line fix: `Ran 1 test in 0.170s`, `OK`; the whole file `Ran 31 tests in
3.785s`, `OK`.

Documents. `tools/mutation_check.py`'s module docstring says nothing about the progress line,
so nothing there went false. `DEV_MANUAL.md` said `Thirty cases`, which the new case made
false: now `Thirty-one`, so `touches:` names it. Its verdict table and prose never describe the
line. `MANUAL.md` does not mention the tool. `tests/CMakeLists.txt`'s comment says `24 cases,
~1.6 s`, S196's count, false before this step and left alone: not in `touches:`.

Tier 2 (DEC-141) does not bind: the diff touches no `make_move`, `unmake_move`, generator or
search, and adds no pruning, reduction or extension rule. No `src/` file moved, so `tools/gate.sh`
owes no `Bench:` line, and no engine node count can differ.
