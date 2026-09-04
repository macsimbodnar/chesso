id:         S197
goal:       `tools/gate_extra.sh` runs what the fast label cannot hold -- the Debug binaries, a sanitizer build, deep perft, the prose and citation checks -- and prints a terminal marker; the coverage recipe is documented beside it
accepts:    the script builds `build-debug` and runs its six invariant-carrying binaries from `tests/` (INV-2, INV-4), configures and builds a sanitizer directory with the existing `SANITIZER` option and runs the fast label and `bench` under it, runs `ctest -L slow`, runs `tools/plan_prose_check.py --prose` and `--citations`, and ends with `GATE-EXTRA-DONE` or `GATE-EXTRA-FAILED` on every exit path (WATCHERS rule); every stage's exit status reaches the shell; its wall time on this machine is measured and recorded in this file; `DEV_MANUAL.md` "Test" documents the script, the DEC-141 cadence, and the `llvm-cov` coverage recipe of the 2026-09-04 test review as an on-demand command whose output is compared with `adocs/data/2026-09-04_test_review/coverage_unexecuted.txt`; `tests/test_gate_extra_script.sh` smoke-tests both markers in a sandbox like the other script tests; fast suite green in both builds
touches:    tools/gate_extra.sh, tests/, tests/CMakeLists.txt, DEV_MANUAL.md, CMakeLists.txt
excludes:   putting any of it in the automatic TESTS gate (DEC-025); a remote CI
decisions:  DEC-139, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why this exists

The 2026-08-14 test review listed sanitizer runs and the Debug assertions as
"absent from the gate" and the 2026-09-04 review found them still by hand;
Stockfish runs the equivalent in CI (`sanitizers.yml`, `games.yml`,
`adocs/testing_strategy.md` section 3.1). DEC-025 keeps the automatic gate to
the fast suite; DEC-141 fixes when this second tier runs. R13 of the strategy
document -- coverage as a periodic report, never a gate -- folds in here as a
documented command.

## Cost

Machine-free to write; about 20 minutes to run, measured when it exists.
