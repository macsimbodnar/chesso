id:         S317
goal:       the T1 blocks held for S101 and for `test_fastchess_script` get S312's treatment, and `DEV_MANUAL.md`'s three goldens rows stop saying history stays in a comment
accepts:    after S101's close: S312's form for the 6 blocks its diff overlapped, `tests/test_search.cpp`'s at `4395` and `4815`, and the three rows 2376, 2377 and 2385, located by their words; once a machine runs `test_fastchess_script`: the 3 blocks in `fastchess.sh` and that test, with `--identical` and the test green together; a clean reader as S312; the goldens S312 listed as this machine's to re-derive, re-derived on the workstation
touches:    tests/test_eval_trace.cpp, tests/test_mate_pv.cpp, tests/test_search.cpp, tests/test_tuner_gradient.cpp, fastchess.sh, tests/test_fastchess_script.sh, DEV_MANUAL.md
excludes:   T2 and T3; anything before S101 closes that its pause check would refuse
decisions:  DEC-338, DEC-337, DEC-335, DEC-142
closes:     2026-11-01_comment_plan-F06; 2026-11-01_comment_plan.2-F07
blocks:
paused_by:
author:
done:

## Why

11 of T1's 76 blocks, 53717 B, 17.5 % of its bytes, cannot be touched on the second machine today, and S312's list would otherwise sit in an immutable file nothing reads.

Also here from S308's list. `tests/test_engine.cpp`'s `depth_1_floor_ms` is an asserted floor read from a measurement, a golden by DEC-142's "value or floor", with no `GOLDEN (DEC-142)` form and no table row; the row sits in S101's `DEV_MANUAL.md` hunk. `fastchess.sh`'s "(AGENTS.md 12, DEC-061)" is a dead citation, in a held file.

