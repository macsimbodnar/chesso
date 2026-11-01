id:         S315
goal:       `tools/mutation_check.py --help` exits as help and not as a failed run
accepts:    `--help` prints the usage, exits 0 and prints no `MUTATION-RUN-FAILED`; a failing test observed first; both builds green
touches:    tools/mutation_check.py, tests/test_mutation_check.py
excludes:   anything else in the driver
decisions:  DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Read 2026-11-01 at `55d7e75`: `tools/mutation_check.py --help` prints the usage and then `MUTATION-RUN-FAILED: SystemExit: 0` with a traceback, so a
watcher reading that marker takes help for a failure. Nothing reaches play, so it is filler by DEC-171.
