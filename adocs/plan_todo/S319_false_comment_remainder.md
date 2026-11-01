id:         S319
goal:       the false comments S308 found and held to its rows are made true or removed, and `depth_1_floor_ms` takes DEC-142's form
accepts:    each row below re-checked against the code at landing, with what the check read, then true or gone; `depth_1_floor_ms` named a golden at its site with what re-derives it and listed with `DEV_MANUAL.md`'s goldens, or a stated reason it is not one; `tests/test_search.cpp`'s GOLDEN block and `DEV_MANUAL.md`'s row agree; comments only but for that, the code identical by S306's form, `No functional change`; a clean reader old against new
touches:    src/chesso.cpp, src/evaluation.cpp, src/search_params.hpp, src/search.cpp, tests/test_engine.cpp, tests/test_search.cpp, fastchess.sh, DEV_MANUAL.md
excludes:   shortening comments or removing history, which is parked (DEC-342); `adocs/data/` and `adocs/plan_done/`
decisions:  DEC-171, DEC-142, DEC-342
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-01 by S308's implementer and its clean reader, and by neither changed, that step being bound to its rows. Filler by DEC-171: none moves a score, a move or a line.

| block | what it says | what was read |
|---|---|---|
| `src/chesso.cpp`, at the first board | "reports 36165 nodes now" | 48840 |
| `src/evaluation.cpp`, `neighbour_files` | "invisible while the weights are zero" | the groups are live |
| `src/evaluation.cpp`, `king_shelter_features` | "`inline` is load-bearing" | rests on a clang timing at zero weights; a timing would answer, else say what it rests on |
| `src/search_params.hpp` and `src/search.cpp`, the history band | "twice the band's own edge" | `QuietHistoryMax` ships 8831, its range top 16384; true of 8192 |
| `fastchess.sh`, the watcher line | "AGENTS.md 12" | no numbered paragraphs since DEC-109 |
| `tests/test_engine.cpp`, `depth_1_floor_ms` | an asserted floor | a golden by `DEV_MANUAL.md`'s definition, no DEC-142 form or row |
| `tests/test_search.cpp`, the GOLDEN block | its readings stop at 7366 | `DEV_MANUAL.md`'s row has 10467 |
