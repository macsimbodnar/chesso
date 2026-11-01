id:         S312
goal:       the script's dead references go across all live files, and T1's log-shaped blocks lose their history by deletion and golden form, no paraphrase
accepts:    DEC-337's scope in `adocs/comment_strategy.md` section 4's order and form: part one, `--report`'s candidates across all live files, each read and none applied blind; part two, the calibration batch and then T1 by traffic, about 75 KB a batch, one writer, one reader, one commit; each block a register line, keep, delete or golden, deleted false sentences counted as dead claims; DEC-142's form at every golden with a value only where a test reads it; a removed reason held nowhere else is kept; `adocs/data/S312_local_gate.sh` green at every commit, each `src/` commit `No functional change`; seeds in the writer's and the reader's copies, none in the tree, recall recorded with its interval; a second reader over all of T1, its pooled error rate per 1000 changed comment lines recorded; the stops read as that section states them; cost read by `transcripts.py` with `ROOT` adapted in a scratch copy, or the agents' token counts as a stated proxy; the held blocks and goldens the machine could not re-derive are listed here, for after S101's close and for the workstation; T2's rule read and written here
touches:    src/, tests/, tools/, cmake/, rating.sh, build_release.sh, clang-format.sh, test_uci.sh, CMakeLists.txt, adocs/data/comment_study/cleanup/
excludes:   `adocs/plan_done/` and existing files under `adocs/data/`; `fastchess.sh` and `tests/test_fastchess_script.sh` until a machine runs that test; any hunk `adocs/data/S101_pause_check.sh` names and the blocks `DEV_MANUAL.md`'s goldens rows 2376, 2377 and 2385 say hold history, until S101 closes; T2 and T3 (DEC-337); any match, fit or timing; moving a number that decides something
decisions:  DEC-337, DEC-336, DEC-335, DEC-142, DEC-171
closes:     2026-11-01_comment_plan-F06, F09
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
- A golden's script may need `.tuning/` or the workstation. One that cannot run on the second machine keeps its value, and goes on this step's list.
