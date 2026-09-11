id:         S183
goal:       the plan's Elo arithmetic is re-derived from recorded inputs -- per step the published figure, its source and the block sum -- with the ledger's measured published-to-measured transfer ratio applied as a third discount, and the reachability sentence kept only if the re-derived range supports it
accepts:    a table under `adocs/data/` lists, for every pending step that carries a published Elo figure, the figure, its source URL or the word unverified (taken from `adocs/data/2026-09-04_plan_review_literature_check.md` and from S185), the block it sums into, and the per-block sums; the ledger's published-to-measured ratios since 2026-08-19 are tabled with their stamps -- S093 verdict 1 0.29 to 0.38, S093 verdict 2 a wrong sign, S130 about 0.1, S149 negative, S108 no gain claimed -- and one stated rule turns them into a discount, the rule written before the number is computed; `adocs/plan.md`'s paragraph shows the three discounts (self-play to list, interaction, published to measured) and the low, mid and high landing points against the 2559 anchor; "The midpoint clears 3000" is deleted unless the re-derived midpoint clears it; if the re-derived high end does not clear 3000 the paragraph says so and names DEC-071 as the decision it puts to the owner; the rule "re-derived when S024 and S109 land, the two largest published inputs" is written beside it; `tools/plan_prose_check.py --params` and `--prose` report nothing new
touches:    adocs/plan.md, adocs/data/
excludes:   changing any step's order or content on the result -- what the number means is the owner's decision under DEC-071, not this step's; the cost line, which is S182
decisions:  DEC-019, DEC-071, DEC-136
closes:     2026-09-04_plan_review-F04
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **The Elo arithmetic is checkable for the first time, and it does not reach 3000 -- not at the midpoint, and not at the high end.** `adocs/data/S183_elo_inputs.md` records what the paragraph could never be checked against: every pending step's published figure, its source URL or the word unverified, the block it sums into, and the per-block sums, with **both rules fixed in writing before any sum was computed** -- the selection rule (one headline figure per step, the largest sourced figure for the change the step actually makes, excluding what the engine already ships) and the discount rule. **The discount rule, and the record behind it:** this project has **eight** published-to-measured transfers, five with a figure on both ends, and of those five **two are exactly zero, one is the wrong sign, one is 0.10 and one is 0.33** -- mean **0.061**, median **0.00**, taking the capture-ordering row charitably at 0.00 rather than at its true negative. The headline discount is **0.38**, S093 v1's +10.73 against Lynx's +28.0, chosen because it is the most generous reading of the record that is still a measurement; the mean and median are reported beside it rather than averaged in, because a mean over a set containing wrong signs is not a ratio. **Two findings, and the second is the one that matters.** *One:* the plan's five quoted ranges are **not reproducible** from the figures its own files carry. Reconstructed, block 1 reads **+184.6** after the two old discounts against the quoted +180 to +280 -- close enough that the original arithmetic was plainly the same shape -- but evaluation reads **+54.3** against +90 to +160, speed **+22.7** against +40 to +90, and **tuning has no published input at all** behind its +50 to +90: its only evidence is S028's **+188.74**, a one-time move from hand-picked constants to fitted ones which by construction cannot happen twice. Raw published total **+544.9**, **+261.6** after two discounts, **+99.4** after three. *Two:* **nothing clears 3000.** The plan's own +390 to +680 with the third discount is +148 to +258 and lands at **2707 to 2817**; the reconstruction lands at **2658**; at the mean ratio 2575 and at the median 2559. **2817 is 183 short, and adding the anchor's whole 121.8 Elo of internal disagreement in the favourable direction reaches 2939 and is still short.** So "The midpoint clears 3000" is **deleted**, struck and dated at the site, and the paragraph says the high end does not clear it either and **names DEC-071 as the decision it puts to the owner**: whether the goal is met by this list, by a longer one, or only with the network premise DEC-054 parked. **This step changed no step's order and no step's content on that result**, which its `excludes:` forbids for exactly this reason, and the arithmetic is labelled as arithmetic -- DEC-019 is in the paragraph's first sentence and stays there. **A convergence worth recording**: the 2026-08-23 estimate reached 2600 to 2650 by summing what had already been kept and discounting it; this reconstruction reaches 2658 by summing what is still owed and discounting it. Two independent arithmetics, one backward and one forward, landing 8 to 58 Elo apart -- and the paragraph now says so. **The re-derivation rule (DEC-136)** is written into both the data file and the paragraph: S024 (+44.68) and S109 (+204) are 46 % of the raw sum between them, so each of their completing commits re-derives this arithmetic with the measured figure replacing the published one and the transfer ratio re-taken over the enlarged ledger. **What is unverified is marked and separated**: +21.4 of block 3's +113.1 rests on figures S185 could not source (S124's +11.3, S118's +10.11), and the sum is given with and without them; S133's +65 and +88 are release bundles and are **not summed** (DEC-133); S126's +188.74 is a local measurement and is not summed either. Checks: `--prose` **0 flagged over 133 ids**, `--params` clean, `--citations` and `--touches` unaffected and clean. Documents only, no `src/`, no `Bench:` line, no run -- written while S207's SPRT held the machine. Docs: `adocs/data/README.md` gains the row; `DEV_MANUAL.md` and `MANUAL.md` checked, neither carries the Elo projection, so neither needed a change; `specs.md` untouched -- the 2559 +/- 25 soft anchor it records is unchanged by this step and is what the arithmetic is measured against. `README.md` untouched, human-owned. Closes `2026-09-04_plan_review-F04`. **For the owner:** this is the one of the night's four document steps whose result is a decision rather than a correction. Nothing was reordered and nothing was dropped.

## Why this exists

`2026-09-04_plan_review-F04`. `adocs/plan.md`'s Elo paragraph discounts
self-play to list Elo "at the ratio the published per-release records support
(~60 % sticks)", takes "a fifth off for interaction", sums four blocks and a
margin, and says "The midpoint clears 3000". The per-step published figures it
was summed from, and the sums, are recorded in no tracked document -- a grep of
`adocs/decisions.md` and `adocs/plan_done/` for the block ranges returns
nothing -- so the arithmetic cannot be checked or re-derived. And it applies no
published-to-measured discount, although DEC-019 says a published figure
decides what to try and never what to conclude, and the ledger since the
paragraph was written has measured that ratio five times:

| step | published | measured here |
|---|---|---|
| S093 verdict 1, malus and gravity | +37.49 (Weiss), +28.0 (Lynx) | +10.73 +/- 6.70 |
| S093 verdict 2, persistence across `go` | +12.5 (Lynx) | -1.65 +/- 4.22, reverted |
| S130, table score as stand pat | +10.8 / +12.1 (Weiss) | +1.14 +/- 4.04, no verdict, kept at zero |
| S149, the wiki's killer replacement rule | published practice | -11.02 +/- 10.53 |
| S108, static evaluation at every node | +4.6-class inputs | +2.28 +/- 4.40, no gain claimed |

The review's sensitivity check, arithmetic and not a measurement: the plan's
own low end sums to +390 and lands at 2949; the midpoint +535 lands at 3094;
the largest transfer ratio measured since the paragraph was written, 0.38,
applied to the high end +680 gives +258 and 2817.

## What the rewrite does

Makes the claim checkable and applies the discount the project has measured.
The inputs go into a table so the next reader can redo the sum; the
transfer-ratio rule is written first and the number second, which is the
pre-registration discipline every SPRT here already follows; and the sentence
about 3000 says whatever the re-derived range says. If that is "does not
clear", the paragraph names DEC-071 and stops -- more steps, a longer road, or
the network premise are the owner's to weigh, and this step measures nothing
that could decide them.

## Cost

Documents only. Two hours, most of them collecting the inputs from the step
files and the literature check.
