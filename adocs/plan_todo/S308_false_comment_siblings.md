id:         S308
goal:       the ten comments S306 and its reader found beside its twelve are made true or removed
accepts:    each row below is re-checked against the code at landing, with what the check read, and then says what the code does or is gone; the first board's figure in `tests/test_engine.cpp` is treated as S306 treated the timer board's (DEC-142); commits touching `src/` read `No functional change`; code identity shown per file as `adocs/data/S306_identity.txt` shows it; every anchor in `tools/mutants/` still found once; a clean reader compares old against new
touches:    src/data_structures.hpp, src/chesso.cpp, src/search.cpp, src/evaluation.cpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp, DEV_MANUAL.md, rating.sh
excludes:   shortening comments or removing history, which waits for the owner's decision on the comment study (`adocs/status.md`); `adocs/data/` and `adocs/plan_done/`
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Why

Reported on 2026-11-01 by S306's implementer and its clean reader, and by neither of them changed,
that step being bound to its twelve. Each row's quoted words find its block.

| block | what it says | what was read |
|---|---|---|
| `src/data_structures.hpp`, the `se_multicut` field | "false in every release build" | the switch ships on |
| `src/search.cpp`, at `PRUNE_HISTORY` | "the sum belongs with S098" | S098 closed without the move and no open step holds it, as S306's `HP_COEFF` row says |
| `tests/test_engine.cpp`, its first board | "**Golden**: 36165 nodes" | 49231 nodes, 9 ms, one run; its 3 ms floor still clears |
| `src/evaluation.cpp` | "while the weights are zero no test can say so" | the groups are no longer all zero |
| `src/evaluation.cpp` | "the zero weights downstream fold" | the same; only `doubled_file` and `backward_rank` are all zero |
| `DEV_MANUAL.md` | "AGENTS.md par.12" | no numbered paragraphs since DEC-109 |
| `rating.sh` | cites lines 71 to 84 of `fastchess.sh` by number | line numbers decay; name the symbol |
| `tests/test_uci_surface.cpp` | `surface_guard` | a key `AGENTS.md` no longer has |
| `src/chesso.cpp`, the board at `go depth 1` | "reports 36165 nodes now" | 49231 nodes in the audit's scratch build, F09; the same board S306 read false, and no list named this site |
| `tests/test_search.cpp`, the budget beside S306's S065 row | "the search costs 109575", "The budget is **4x** that" | the pair is 65024 from 16256, as the GOLDEN block below it says |

Two errors in S306's twelve rewrites survived its writer's own check, so the reader is not optional.
Filler by DEC-171: none moves a score, a move or a line. If the owner takes the comment study's cleanup,
this folds into its step that covers `src/`.
