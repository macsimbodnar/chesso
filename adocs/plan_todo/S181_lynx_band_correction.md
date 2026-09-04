id:         S181
goal:       every Lynx figure the enriched steps and DEC-087 cite is banded by the CCRL Blitz rating of the Lynx release it was measured between, from a dated PR-to-release-to-rating table, and S098's "sub-3000 evidence" grouping is redrawn on the corrected band
accepts:    a table in `adocs/data/` maps every Lynx pull request the pending steps and DEC-087 cite -- at least #512, #613, #637, #733, #1203, #1206, #1233, #1662, #1663 and the four capture-history SPRTs DEC-087 places "at ~2600" -- to its merge date, the two releases it fell between, and the CCRL Blitz 1CPU rating of each release as read on a stated date, every row with its URL; every band word in S097, S098, S099, S109, S110 and S132 ("high-2800s", "~2850", "~2600" and the like) is replaced by the table's figure; S098's grouping under "Sub-3000 evidence" is redrawn -- an entry whose evidence is above 3000 leaves that heading -- and the file states whether the order of its three verdicts changes; S099's file records DEC-133's move to the reserve head and the condition under which S110 and S111 return; DEC-087's Amended line names the table; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/data/, adocs/plan_todo/S097_singular_extensions.md, adocs/plan_todo/S098_reduction_refinement.md, adocs/plan_todo/S099_correction_history.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S132_time_management_node_fraction.md, adocs/decisions.md
excludes:   any change to a technique or to the order beyond S099's move to the reserve head, which DEC-133 already made; sourcing figures for engines other than Lynx, which is S185; the Lynx figures' values themselves, which the pull requests state and which are not in question
decisions:  DEC-087, DEC-133
closes:     2026-09-04_plan_review-F02
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_plan_review-F02`. DEC-087 (b) uses one criterion for the
correction-history family: evidence below 3000 keeps a table in the main
order, evidence only above about 3100 sends it to the reserve. S099 stayed on
"+11.4 at ~2850 (Lynx)". Fetched during the review: Lynx pull request #1662,
"Pawn correction history", merged 2025-04-15, +11.35 +/- 5.16 at 8+0.08 over
7502 games -- between Lynx v1.9.0 (published 2025-03-11) and v1.10.0
(2025-06-29), which the CCRL Blitz list computed 2026-08-28 rates at **3226**
and **3293**. So the one "sub-3000" correction number was measured above the
band that demoted the other two, and the criterion separates nothing. The
owner's decision (DEC-133) moves S099 to the head of the reserve as the
family's probe; this step corrects the record the decision was taken on.

The same mis-banding recurs wherever DEC-087-era text places a Lynx version.
S098 files Lynx #1233 (+9.34, v1.8.0, 2024) under "high-2800s" and groups
cut-node, improving and PV scaling under "Sub-3000 evidence" on that band; the
list has v1.8.0 at 3139. S097 recorded the discrepancy as a scope concern on
2026-08-19 ("may sit ~300 low") and nothing moved since.

## The rows already fetched

From `https://computerchess.org.uk/ccrl/404/rating_list_all.html`, list
computed 2026-08-28, 1CPU entries:

| Lynx | CCRL Blitz |
|---|---|
| 1.5.0 | 2819 |
| 1.6.0 | 2926 |
| 1.7.0 | 3119 |
| 1.8.0 | 3139 |
| 1.9.0 | 3226 |
| 1.10.0 | 3293 |
| 1.11.0 | 3364 |

Release dates come from `https://api.github.com/repos/lynx-chess/Lynx/releases/tags/<tag>`
(v1.8.0 2024-12-20, v1.9.0 2025-03-11, v1.10.0 2025-06-29 fetched), and
merge dates from the pull request's API record. The Lynx README's own CCRL
table agrees with the list to within a few points and is the cross-check.
Every other row the step needs -- the 2023 and 2024 pull requests #512, #613,
#637, #733, #1203, #1206 and the capture-history attempts -- is fetched the
same way and dated the same way.

## What the corrected band decides, and what it does not

It decides the *record*: which heading each figure sits under and what the
plan says about where a technique's value has been shown. It does not decide
whether the technique helps chesso, which only a verdict does (DEC-019), and
S099's reserve position is a probe for exactly that question. S098 stays one
step with three verdicts; what may change is their order.

## Cost

Documents and a few API reads. An hour.
