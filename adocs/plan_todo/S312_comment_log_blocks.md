id:         S312
goal:       the log-shaped comment blocks become what the code cannot say, with current values, read old against new
accepts:    the scope the owner set under S311, by default the log-shaped blocks `tools/comment_budget.py` lists, each now holding contracts, hazards, reasons and deciding numbers in the present tense, DEC-142's form and current value at every golden (name, command, what moves it, margin), and a rejected idea one line at its site; `--identical` green at every commit, bench and `tools/search_bench.py` identical, each `src/` commit `No functional change`; every `tools/mutants/` anchor still found once, those overlapping a comment moved in the same commit; `plan_prose_check.py --citations` 0 flagged; `adocs/data/S101_pause_check.sh` green at every commit; **a clean reader, on a fresh context, reads old against new over every rewritten block**, its findings fixed and its error rate per 1000 comment lines recorded, more than one meaning-changing error per 1000 after that review stopping the step for the owner; a removed reason that exists nowhere else is kept; what a removed log held that no step file holds goes into `DEV_MANUAL.md`'s goldens table; both builds green at every commit; the hunks S101's diff held back are named here, for after its close
touches:    src/, tests/, tools/, cmake/, fastchess.sh, rating.sh, DEV_MANUAL.md
excludes:   `adocs/data/` and `adocs/plan_done/`, which are evidence; any hunk `adocs/data/S101_pause_check.sh` names, until S101 closes; any match, fit or timing; moving a number that decides something: a golden whose current value is false is re-derived by its script or the step stops and says why it cannot be on this machine
decisions:  DEC-335, DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Long blocks are where false claims live: 9 of 36 blocks of 1200 bytes or more against 2 of 104 smaller. In the selected blocks 4 of 10 sampled
hold one. Tokens do not justify this, by the study's own figures; defects and the 61 % of changed `src/` lines that are comment-only do.

## What the study found hard

- Its pilot's first draft stated two false things, a fabricated pointer and a wrong denominator, and S306's twelve rewrites held two more. Code
  identity sees neither, so the clean reader is not optional, and it has no stake in the rewrite.
- Short reasons are often held nowhere else, 7 of 25 in a hand test. Compress the logs, leave the reasons.
- Five of 244 mutant anchors overlapped a comment at the study's commit (M36, M03, TC04, K06, NB01), and nothing fails until the next mutation run. Re-count.
- `src/search_params.hpp`'s comments sit in a macro, each line ending in an aligned backslash. A comment wrap goes through the format gate.
- A golden's script may need `.tuning/` or the workstation. One that cannot run on the second machine keeps its value, and the step says so.
