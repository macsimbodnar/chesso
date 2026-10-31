id:         S306
goal:       the eleven comments the 2026-10-31 study read as false are made true or removed
accepts:    each of the eleven below is re-checked against the code at landing, with what the check read, and then says what the code does or is gone; `tests/test_engine.cpp`'s figure is re-taken by its command (DEC-142), or its "Golden" mark goes with the reason stated; every commit touching `src/` reads `No functional change` and the gate passes it; every anchor in `tools/mutants/` is still found once; a clean reader compares old against new
touches:    src/search.cpp, src/search_params.hpp, src/evaluation.cpp, src/data_structures.hpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp, tools/timer_race_stress.py, fastchess.sh
excludes:   shortening comments or removing history, which waits for the owner's decision on the study (`adocs/status.md`); `adocs/data/` and `adocs/plan_done/`, which are evidence
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Why

A comment study on `5954c12` read a stratified sample of 140 live blocks against the code, and 11
held a claim the code no longer bears out, so this list is a floor. One agent read them, and the
coordinator confirmed only the first row's switch and that the texts exist, so each is checked
again here. Each row's quoted words find its block.

| block | what it says | what the study read |
|---|---|---|
| `src/search.cpp` | "the second of them ships switched off" | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)` ships on |
| `tests/test_engine.cpp` | "Golden (DEC-142)": its depth-1 iteration is 25933707 nodes, about 5.9 s | one run answered `nodes 35516352`, 8962 ms; no test holds the figure; the bound still separates |
| `src/search_params.hpp` | "S098 owns moving the rule onto the sum" | S098 is closed and the rule still reads the raw entry |
| `src/search_params.hpp` | "the build prints 4493659" | bench is 5707449 |
| `src/search_params.hpp` | cites `evaluation.cpp` lines 33 to 37 | `ORDER_COUNTER` is on line 39 |
| `src/evaluation.cpp` | "clang still inlines it", "the zero weights still fold" | g++ is the reference since DEC-049; `passed_pawn_mg` is no longer zero |
| `src/data_structures.hpp` | "evaluate() is material plus tapered tables" | mobility, king safety and pawn terms too |
| `tests/test_search.cpp` | "S065 is a pending 827-constant paste" | S065 is done |
| `tests/test_search.cpp` | "an H0 on S234 flips `RfpTtEstimate`" | S234 is closed and it ships at 1 |
| `tools/timer_race_stress.py` | cites a numbered paragraph of `AGENTS.md` | none since DEC-109; `tests/test_uci_surface.cpp`'s header is the same class |
| `fastchess.sh` | cites lines 72 to 77 of `rating.sh` as doing the same | those lines count cores |

Filler by DEC-171: none moves a score, a move or a line. It runs in the gap after S118's landing
or refusal and before S101, because S118 rewrites `src/evaluation.cpp` and nine open steps touch
it. If the owner takes the study's cleanup, this folds into its step that covers `src/`. A comment
edit in `fastchess.sh` moves no harness behaviour, so DEC-143's A/A does not read it as a change.
