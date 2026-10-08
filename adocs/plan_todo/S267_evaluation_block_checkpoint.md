id:         S267
goal:       before the final refit and tuning, a fixed drift match against the build S240 rated says whether the plan is on course for 3000, against a threshold written down on 2026-10-08, and a shortfall stops the order for a re-plan with the owner
accepts:    (1) the reference is `1680439`, the build S240 rated at about 2766 (`adocs/data/rating_2026-09-27_S240_ccrl_blitz.md`), built from its commit and pinned; (2) the reading is one fixed 1000-pair match, 2000 games, at the harness's regime -- 8+0.08, Hash=16, `noob_3moves.epd` -- in S199's form, appended to a drift series of its own with `ref` and `regime` columns, the Elo point estimate and its 95 % half-width read from fastchess; (3) **the gate, fixed here before any evaluation family lands (DEC-258)**: G is that point estimate, HEAD against `1680439`, taken after S133 and before S126; G >= 232 continues the order; G < 232 stops it -- S126 does not start until the coordinator has put to the owner the shortfall in self-play Elo, the reserve candidates with their evidence and price, and a recommendation, and the owner has ruled; the network's timing is not among the options the coordinator offers (DEC-179); (4) one DEC-202 reading at 32+0.32, Hash=64 beside it, an estimate and never a verdict; (5) the boundary readings before it -- after S261's lane (S261's accepts) and after the capture-history family (the coordinator's block-boundary reading, `plan.md` "At each block boundary") -- use the same reference, regime and series and are recorded without action; (6) nothing here is a rating (DEC-108): no figure from it enters `adocs/specs.md`'s measured strength
touches:    adocs/data/ for the drift series and its readings, adocs/plan.md, adocs/status.md
excludes:   any rating run, which is S152's; any change to the harness or its book; reading a drift point as a verdict on any single change inside it
decisions:  DEC-258, DEC-108, DEC-143, DEC-179, DEC-202
closes:
blocks:
paused_by:
author:
done:

## The threshold, and where 232 comes from

Written 2026-10-08 with the plan, so no evaluation result can move it:

- **The mark needs about +234 on the list** from S240's 2766 (DEC-258 reads
  the claim as the gauntlet's point estimate, not its lower bound).
- **Self-play to list: 0.83**, the one transfer on record -- S240 measured
  +207 on the list where the kept verdicts' point estimates summed to about
  +250. Those were early-stopped SPRT estimates, which run high; a fixed drift
  match has no stopping bias, so its true transfer is probably higher than
  0.83 and this threshold errs toward stopping. 234 / 0.83 = **282** self-play
  Elo needed in total against `1680439`.
- **What comes after the gate is credited at 50**: a refit measured +21.10
  (S065) and +26.68 (S076) here, the first SPSA lane +21.02 (S085), and the
  speed steps a few Elo between them; the first fit's +188.74 (S028) was a
  one-time move and is not counted.
- **282 - 50 = 232.** Below it, the steps left cannot close the gap on the
  record's own numbers, and continuing would spend the refit and the final
  lane on a plan already short.

The figure is re-derived only by a decision, before the reading, never after.

## Why

The plan's sourced items do not reach the mark under the transfer the
record measured (DEC-258's arithmetic), and DEC-108 leaves no absolute
rating before S152. A drift match costs about an hour (2000 games at 2110 an
hour) and is the cheapest instrument that sees the sum. The owner chose this
form on 2026-10-08 over a mid-course gauntlet and over no checkpoint.
