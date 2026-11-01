id:         S312
goal:       all live comments read against the rule in DEC-336's three tiers, the log-shaped blocks made current, history and false claims out of the long ones, the short ones read and touched only where false
accepts:    DEC-336's tiers in its order, by traffic within a tier, S101's seven files last, each batch about 75 KB of comment, one writer, one reader, one commit: each block has a register line with its outcome, keep, delete, replace or golden, and a replacement names the source read; DEC-142's form at every golden (name, command, what moves it, margin), a value kept only where a test reads it; a rejected idea one line at its site; a removed reason that exists nowhere else is kept; `adocs/data/S312_local_gate.sh` green at every commit, each `src/` commit `No functional change`; **a clean reader on a fresh context reads old against new over every changed block, with three seeded errors**, its findings checked against the code and fixed, its recall and the error rate per 1000 comment lines recorded per batch; DEC-336 (7)'s stops read at every batch; what a removed log held that no step file holds goes into `DEV_MANUAL.md`'s goldens table, and its three rows that say history stays in a comment are corrected; the blocks S101's diff holds back are named here, for after its close; T3 may close this step's first part and run on as filler, the stamp saying where it stood
touches:    src/, tests/, tools/, cmake/, fastchess.sh, rating.sh, build_release.sh, clang-format.sh, test_uci.sh, CMakeLists.txt, DEV_MANUAL.md, adocs/data/comment_study/cleanup/
excludes:   `adocs/data/` and `adocs/plan_done/`, which are evidence; any hunk `adocs/data/S101_pause_check.sh` names, until S101 closes; any match, fit or timing; moving a number that decides something: a golden whose current value is false is re-derived by its script or the step stops and says why it cannot be on this machine
decisions:  DEC-336, DEC-335, DEC-142, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why

Long blocks are where false claims live: 9 of 36 blocks of 1200 bytes or more against 2 of 104 smaller, though per byte that excess reads 1.98x to 4.88x. In the selected blocks 4 of 10 sampled
hold one. Tokens do not justify this, by the study's own figures; defects and the 61 % of changed `src/` lines that are comment-only do.

## What the study found hard

- Its pilot's first draft stated two false things, a fabricated pointer and a wrong denominator, and S306's twelve rewrites held two more. Code
  identity sees neither, so the clean reader is not optional, and it has no stake in the rewrite.
- Short reasons are often held nowhere else, 7 of 25 in a hand test. Compress the logs, leave the reasons.
- Five of 244 mutant anchors overlapped a comment at the study's commit (M36, M03, TC04, K06, NB01), and nothing fails until the next mutation run. Re-count.
- `src/search_params.hpp`'s comments sit in a macro, each line ending in an aligned backslash. A comment wrap goes through the format gate.
- A golden's script may need `.tuning/` or the workstation. One that cannot run on the second machine keeps its value, and the step says so.
