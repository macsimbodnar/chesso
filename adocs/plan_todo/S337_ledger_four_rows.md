id:         S337
goal:       the ledger's four other rows whose closing subjects break the rule read out right: two by override rows, two by a run column the override file gains
accepts:    `python3 tools/ledger.py --table` reads `e47eb4a`'s and `b9f5b96`'s rows by override rows; `775dafd`'s and `7f46c0c`'s rows read their true run ids and descriptions through an optional `run` column of `tools/ledger_overrides.tsv`, refused like the other columns when malformed and seen red on a fixture; every other row byte-equal; `git log` unchanged; plan.md's table regenerated in the same commit
touches:    tools/ledger.py, tools/ledger_overrides.tsv, tests/test_ledger.py, DEV_MANUAL.md, adocs/plan.md
excludes:   rewriting history; the subject rule for later commits
decisions:  DEC-171, DEC-220
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-06 by S334's implementer ("Beyond the step" in S334's done file). `e47eb4a` (S289) reads "Revert 's
null-move table guard on its H0 (DEC-309)" and `b9f5b96` (S112) "no-verdict at the cap: ..."; an override row fixes
each. `775dafd` and `7f46c0c` say "S022 verdict 1" and "S023 verdict 1", so the run reads "S022"/"S023" and the
description starts "1's H0: ..."; the run id is wrong as well, and the override file has no column for it. Filler by
DEC-171: no play, no reported score.
