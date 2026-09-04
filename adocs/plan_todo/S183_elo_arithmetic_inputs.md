id:         S183
goal:       the plan's Elo arithmetic is re-derived from recorded inputs -- per step the published figure, its source and the block sum -- with the ledger's measured published-to-measured transfer ratio applied as a third discount, and the reachability sentence kept only if the re-derived range supports it
accepts:    a table under `adocs/data/` lists, for every pending step that carries a published Elo figure, the figure, its source URL or the word unverified (taken from `adocs/data/2026-09-04_plan_review_literature_check.md` and from S185), the block it sums into, and the per-block sums; the ledger's published-to-measured ratios since 2026-08-19 are tabled with their stamps -- S093 verdict 1 0.29 to 0.38, S093 verdict 2 a wrong sign, S130 about 0.1, S149 negative, S108 no gain claimed -- and one stated rule turns them into a discount, the rule written before the number is computed; `adocs/plan.md`'s paragraph shows the three discounts (self-play to list, interaction, published to measured) and the low, mid and high landing points against the 2559 anchor; "The midpoint clears 3000" is deleted unless the re-derived midpoint clears it; if the re-derived high end does not clear 3000 the paragraph says so and names DEC-071 as the decision it puts to the owner; the rule "re-derived when S024 and S109 land, the two largest published inputs" is written beside it; `tools/plan_prose_check.py --params` and `--prose` report nothing new
touches:    adocs/plan.md, adocs/data/
excludes:   changing any step's order or content on the result -- what the number means is the owner's decision under DEC-071, not this step's; the cost line, which is S182
decisions:  DEC-019, DEC-071, DEC-136
closes:     2026-09-04_plan_review-F04
blocks:
paused_by:
author:
done:

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
