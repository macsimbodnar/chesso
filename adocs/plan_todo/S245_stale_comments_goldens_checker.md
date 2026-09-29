id:         S245
goal:       five stale or wrong things the fillers of 2026-09-28 and 2026-09-29 noted and did not touch -- two comments, one timing golden, one checker defect, one budget table -- each verified against the tree and corrected or re-derived by its own script
accepts:    (1) the comment in `iterative_deepening_search` quoting 42371 nodes for the depth-1 iteration of `test_engine`'s eight-queens board reads the number the engine reports (36165 on 2026-09-29), re-taken with `go depth 1`; (2) the eight-queens depth-1 golden of "a stop inside the first iteration cuts it and the hard timer ends the search within its bound" (13.8 ms recorded, 6 to 7 ms today) is re-derived by the command its GOLDEN comment names, the asserted floor unchanged unless the re-derivation moves it and then only by that rule; (3) `tools/plan_prose_check.py`'s `holds_phrase` finds a quoted title that wraps across two string literals in the source (a test in `tests/` that plants such a citation and is red before the fix, green after); (4) the "needs fractional reductions" comments naming S237 and S238 in `src/search.cpp`, `src/search_params.hpp`, `tests/test_search.cpp` and `DEV_MANUAL.md` say what is true after both left; (5) `S170_cases.tsv`'s budgets are re-derived by `adocs/data/S203_case_sweep.sh` under the DEC-156 rule with DEC-162's stride, the old rows quoted; the fast suite green in both builds, and `bench` unchanged (comments and tests only) or the change's reach counted and decided the way its reach says
touches:    src/search.cpp (comments), src/search_params.hpp (comments), src/chesso.cpp (a comment), tests/test_engine.cpp, tests/test_search.cpp, tools/plan_prose_check.py, tests/ (a checker test), adocs/data/S170_cases.tsv, DEV_MANUAL.md
excludes:   any change to the engine's behaviour; any ceiling; the mined mate rows
decisions:  DEC-171, DEC-142, DEC-156, DEC-162
closes:
blocks:
paused_by:
done:

## Why this exists

Five things noted in passing by the agents of S242, S243 and the 2026-09-28
handover, none of them a defect reachable in ordinary play, on the UCI
surface or in a reported score (DEC-171): a source comment quoting a node
count the engine no longer reports; a timing golden recorded at 13.8 ms that
reads 6 to 7 ms today (its floor holds); `tools/plan_prose_check.py`
reporting a citation MISSING when its quoted title wraps across two string
literals in the source, which refused S243's first mutation fixture; the
"needs fractional reductions" comments that still name S237 and S238 after
both left; and `S170_cases.tsv`'s budgets, which S238's pre-registration
already recorded as not the DEC-156 rule's answer. Bundled as one filler
because each is an afternoon and none needs the machine; scheduled behind
the strength steps and named by id in every pre-registration taken while it
is open. Comments and tests only, so no verdict is owed unless (5) or (2)
moves a number the search reads, which they do not.
