id:         S323
goal:       the line-number citations in code comments point at what they name or cite it by a phrase, and the test list's clamp count says what the case reads
accepts:    each site below re-read against its target at landing, with what the check read, then true, cited by a phrase or symbol, or gone; any further line-number citation in `src/`, `tools/` and `tests/` comments found by a stated search and treated the same way, the search and its count in the step; comments only, code identical by S306's form (objects byte-equal or each difference explained, as S321 did), `No functional change`; a clean reader old against new
touches:    tests/CMakeLists.txt, tools/corpus_dedupe.hpp, tools/tuner_split.hpp, tools/mutants/time.py, tests/test_audit_fen_semantics.cpp, tests/test_helpers.hpp, tests/test_search.cpp, tests/test_evaluation.cpp, adocs/data/S323_*, adocs/data/README.md
excludes:   shortening comments or removing history (DEC-342); any change to what a test asserts; S322's changes to `tests/CMakeLists.txt` beyond the one comment
decisions:  DEC-171, DEC-342
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-02 by S321's implementer, beyond its rows. Filler by DEC-171: none moves a score, a move or a line.

| where | what it says | what was read |
|---|---|---|
| `tests/CMakeLists.txt`, the comment above `add_trace_doctest_target(test_eval_trace)` | "the 2696 positions ... the clamp binding on 478 of them" | false two ways, as `DEV_MANUAL.md`'s sentence S321 corrected: 478 was of the 2764 positions at `f57b92f`, and at `d0220d1` the 2696 JSON rows bind 246 high and 246 low |
| `tools/corpus_dedupe.hpp` (2), `tests/test_audit_fen_semantics.cpp` (3), `tools/tuner_split.hpp`, `tests/test_helpers.hpp`, `tests/test_search.cpp`, `tests/test_evaluation.cpp`, `tools/mutants/time.py` (inside a quoted compiler message) | a `file:line` citation each | S321 row 3's class, a citation by line number that a later edit moves; not checked whether each still points at what it names |
