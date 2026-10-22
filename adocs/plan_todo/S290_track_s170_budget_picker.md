id:         S290
goal:       the script that applies S203's budget rule to `adocs/data/S170_cases.tsv` is tracked in the tree and named where the budgets are, so the golden is re-derivable from a fresh clone
accepts:    `.tuning/coord/S099/p2/pick_budgets.sh` (gitignored, used by S099, S110, S111, S286, S083 and S289) is committed under `adocs/data/` or `tools/` byte for byte or with its changes stated; `adocs/data/S170_cases.tsv`'s header, `tests/test_mate_carry.cpp`'s golden site and `DEV_MANUAL.md` "Goldens: named, scripted, re-derived" name the tracked path; re-run on S289's grid (`adocs/data/S289_sweep_s170.txt`) it reproduces the committed budgets (A 500000, C 1000000, D 1200000, the rest unmoved) exactly; the fast suite green in both builds
touches:    adocs/data/ or tools/ (the picker), adocs/data/S170_cases.tsv header comments, tests/test_mate_carry.cpp comments, DEV_MANUAL.md, adocs/data/README.md
excludes:   any budget or ceiling value; the rule itself (DEC-156, DEC-162, DEC-299)
decisions:  DEC-142, DEC-171, DEC-299
closes:
blocks:
paused_by:
author:
done:

## Why this step exists (the coordinator, 2026-10-22)

S289's fast check: the S170 budgets are applied by
`.tuning/coord/S099/p2/pick_budgets.sh`, which lives under the gitignored
`.tuning/` and is not named in the TSV's header -- the same at S286 and S110.
DEC-142 wants every golden re-derived by a script named at its site; a script
outside the tree is not re-derivable from a clone. Not reachable in play or on
the UCI surface, so a DEC-171 filler behind the next strength step, named in
the pre-registration of every run taken while it is open.
