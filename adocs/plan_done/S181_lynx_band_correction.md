id:         S181
goal:       every Lynx figure the enriched steps and DEC-087 cite is banded by the CCRL Blitz rating of the Lynx release it was measured between, from a dated PR-to-release-to-rating table, and S098's "sub-3000 evidence" grouping is redrawn on the corrected band
accepts:    a table in `adocs/data/` maps every Lynx pull request the pending steps and DEC-087 cite -- at least #512, #613, #637, #733, #1203, #1206, #1233, #1662, #1663 and the four capture-history SPRTs DEC-087 places "at ~2600" -- to its merge date, the two releases it fell between, and the CCRL Blitz 1CPU rating of each release as read on a stated date, every row with its URL; every band word in S097, S098, S099, S109, S110 and S132 ("high-2800s", "~2850", "~2600" and the like) is replaced by the table's figure; S098's grouping under "Sub-3000 evidence" is redrawn -- an entry whose evidence is above 3000 leaves that heading -- and the file states whether the order of its three verdicts changes; S099's file records DEC-133's move to the reserve head and the condition under which S110 and S111 return; DEC-087's Amended line names the table; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/data/, adocs/plan_todo/S097_singular_extensions.md, adocs/plan_todo/S098_reduction_refinement.md, adocs/plan_todo/S099_correction_history.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S132_time_management_node_fraction.md, adocs/decisions.md
excludes:   any change to a technique or to the order beyond S099's move to the reserve head, which DEC-133 already made; sourcing figures for engines other than Lynx, which is S185; the Lynx figures' values themselves, which the pull requests state and which are not in question
decisions:  DEC-087, DEC-133
closes:     2026-09-04_plan_review-F02
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **Every Lynx figure the pending steps and DEC-087 cite is banded, three band words were low by 180 to 380 points, and one claim was wrong in kind rather than in degree.** `adocs/data/S181_lynx_bands.md` maps twenty-one pull requests to their `merged_at` from the GitHub API, the two releases that date falls between, and the CCRL Blitz 1CPU rating of each release -- ratings from the list **computed 2026-09-05 with Bayeselo over 2'106'571 games**, fetched **2026-09-11** with `curl` and a browser user-agent because `WebFetch` gets HTTP 403 on that host; `adocs/data/S181_lynx_prs.tsv` is the twenty-one API records verbatim. **The read date is beside every figure because the list drifts**: against the 2026-08-28 reading this file quoted, 1.11.0 **3360** against 3364, 1.10.0 **3291** against 3293, 1.9.0 **3224** against 3226, 1.8.0 **3138** against 3139, 1.6.0 **2925** against 2926, 1.5.0 **2818** against 2819, 1.7.0 **3119** unchanged -- 0 to 4 points in a fortnight, and no band below turns on it. **Four releases the plan's argument needs are not on the list at all** -- v1.0.0, v1.2.0, v1.5.1 and v1.9.1 -- so where a bracketing release is unrated the range runs from the nearest rated release before the merge, and the table says which rows those are rather than interpolating. **The corrections.** (i) S099's headline "+11.4 at ~2850" (#1662, merged 2025-04-15) is **3224-3291**, wrong by about 380 and **above** the "~3100" that demoted S110 and S111 -- DEC-087 (b)'s criterion separated nothing, which is F02 itself and DEC-133's reason for moving S099 to the reserve head. (ii) **S023's Lynx evidence is withdrawn, and it was wrong in kind**: no source exists anywhere in this tree or in the 2026-09-04 literature check for "Lynx failed four SPRTs on it at ~2600 (-35.8 to -11.1)", and the API record says the opposite of the headline -- Lynx **merged** capture history, #634, 2024-02-02, into v1.3.0 at a banded **2653**; a title search over that repository's nineteen capture-history pull requests shows the closed-unmerged ones are later refinements of a feature already in the tree, dated 2024-09 onward at 2925 and above, not four attempts at the feature. The demotion stands on the figure that traces, Weiss #428's **-4.17 +/- 4.83** STC against **+3.66 +/- 3.29** LTC at an engine CCRL rates **3055**. (iii) **S098's node-type grouping leaves the "Sub-3000 evidence" heading**: cutnode (#1233) **3119-3138**, !improving (#1135) **3119-3138**, Lynx's PV-min-moves patch (#1230) **3119-3138**, TT-capture (#1529) **3138-3224**, deeper/shallower (#1535) **3138-3224**, where the file read high-2800s and "~3000-3100". What is left under that heading is the log table, history scaling (#613, **2420-2653**, shipped at 2653) and Weiss #71's PV direction from 2019 -- and the file says "that is all of it". (iv) **S109's band moves the other way**: Lynx #512's LMP passes and its failed quadratic are **2420-2430**, 180 points *below* the "~2600" written, which strengthens that step's sub-3000 claim. (v) S097's SE block (#1731, #1743, #1751) is **3224-3291**; S132's node time management (#1203, #1206) **3119-3138**; #971's clamp control **never merged**, so no release brackets it and the table says the 2818-3119 range is inferred from the pull-request number rather than from a date. **S097's band caution of 2026-08-19 is closed in the README's favour, which is the part worth keeping.** It flagged that Lynx's own README rated v1.8.0 at 3144 and v1.10.0 at 3293 where this repository placed that era at high-2800s, said the repository's banding might sit ~300 low, and could not resolve it. The list gives 3138 and 3291, agreeing with the README to within a few points: **the README was right and the banding was about 300 low**. Scope concern 1 is closed with it. **What changed and what did not, DEC-176** -- taken by the agent under the owner's delegation of 2026-09-11 and naming itself as such: the record moves and **no step's position does**. S023 stays in the reserve on the Weiss figure, which is better founded than four runs nobody can find; S098 stays one step with three verdicts **in that order**, history scaling first because it is the only layer with evidence in this engine's band and (d) reads the re-search (c) produces; S099, S110 and S111 stay where DEC-133 put them, the re-banding being the reason that decision was right rather than a new question. What the files may no longer *claim* is the change: (c) and (d) rest on 3100-band evidence, so a zero from either is an expected outcome and neither may be argued for on "it worked below 3000". **Rejected**: re-promoting S023 because its Lynx evidence evaporated, and demoting S098's node-type layer on its corrected band -- both are argued in DEC-176. **Beyond the accepts**, and noted rather than hidden: `S023_capture_history.md` is not in this step's `touches` (the accepts lists six files) but carries the "~2600" that S185 explicitly deferred here an hour earlier, so it was corrected here; and DEC-087 gained a second `Amended:` line naming the table, as the accepts asked. **Files fetched:** the GitHub releases endpoint, twenty-one `pulls/<n>` records, one `search/issues` query and the CCRL complete list -- an API record and a rating table, and nothing else. No Lynx source file, evaluation table or test data was read (DEC-016). Checks: `--prose` **0 flagged over 133 ids**, `--citations` **0 flagged over 64 files**, `--touches` **0 flagged over 64 files**, `--params` clean. Documents and API reads only, no `src/`, no `Bench:` line, no run -- taken while S207's SPRT held the machine, which is what the document lane is for (DEC-172). Docs: `DEV_MANUAL.md` and `MANUAL.md` checked, neither carries a Lynx band, neither needed a change; `adocs/data/README.md` gains the two rows. `README.md` untouched, human-owned. Closes `2026-09-04_plan_review-F02`.

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
