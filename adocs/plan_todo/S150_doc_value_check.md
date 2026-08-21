id:         S150
goal:       a number stated in a plan or manual document about a search parameter is checked against search_param_info(), and the three values S085 left stale in specs.md and MANUAL.md are corrected
accepts:    a tracked check reads `search_param_info()` and flags a plan or manual document sentence that states a different number for the same parameter, and it is observed red on each of the three known cases before it is trusted; the three are corrected -- `adocs/specs.md:171` aspiration 5 / 50 / 400 to 2 / 21 / 437, `adocs/specs.md:178` quiescence "capped at 8 plies" to 19, `MANUAL.md:250` "from depth 5" to 2; the check's coverage limit is stated rather than implied, since the two specs.md sentences never name the parameter and a name-adjacency scan misses them, so the phrase set it keys on is written down as the maintenance cost; nothing in `src/` changes
touches:    tools/, adocs/specs.md, MANUAL.md, DEV_MANUAL.md
excludes:   the evaluation constants, which are not in `search_param_info()`; re-checking numbers in `adocs/plan_done/`, which is history and is never rewritten; the citation checker's own two modes, which S138 and S144 own
decisions:  
closes:     2026-08-21_adversarial-F02
blocks:
paused_by:
done:

## Why the existing checks cannot catch this

`tools/plan_prose_check.py` has `--prose` and `--citations` and neither compares
a number to code. `tests/test_uci_surface.cpp:88-91` builds its expected option
lines *from* `search_param_info()`, so it restates the code rather than checking
prose against it. `tests/test_search_params.cpp`'s golden list guards the code
against itself. S139, S140, S141 and S144 check accepts fields, retired ids,
touches fields and citation paths -- none checks a value.
