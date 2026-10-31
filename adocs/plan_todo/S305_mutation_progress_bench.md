id:         S305
goal:       `tools/mutation_check.py`'s progress line stops calling an unread bench `same`
accepts:    a mutant whose bench was never read prints `n/a` on the progress line, as the final table already does; a case in `tests/test_mutation_check.py` observed red on today's line before it goes green
touches:    tools/mutation_check.py, tests/test_mutation_check.py
excludes:   what a run kills or how it decides; the final table, which is right
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-10-31 reading S118's mutants. The progress line prints `bench same` whenever
`bench_moved` is false, and a mutant the suite kills before its bench is read leaves it false
with `bench_read` false too. The final table reads both and prints `n/a`. A run stopped before
its table, as S118's first was, leaves only the progress lines, so `same` there means "not seen
to move". S118 re-ran PT01 to PT13 rather than cite those rows.

Filler by DEC-171: it moves no score or line and no run's verdict.
