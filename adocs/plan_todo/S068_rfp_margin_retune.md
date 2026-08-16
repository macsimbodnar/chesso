id:         S068
goal:       re-decide the reverse futility margin against a verdict, starting from the 75 that S033 left on the table
accepts:    an SPRT against the commit shipping margin 100 returns a verdict, recorded whatever it is; every mate case in the fast suite stays green at the setting that ships, the material-leader case included
touches:    src/search.cpp RFP_MARGIN
excludes:   the ply bound and the depth bound, which S033 set from measurement and DEC-060 records; any second pruning rule
decisions:  DEC-060
closes:
blocks:
paused_by:
done:

## Why this exists

S033 shipped `RFP_MARGIN` 100 and said so: a first setting, one pawn per
remaining ply, fitted to nothing. It bought +59.98 +/- 17.24 Elo. The sweep that
chose it also measured margin 75 at the same ply and depth bounds, and 75 is
green on every mate case and visits **1216123 nodes against 1422053**, 14.5 %
fewer, on `tools/search_bench.py` at depth 9.

That is not a reason to ship it. Fewer nodes is not more Elo -- DEC-019 is three
techniques that were quoted a gain and measured zero or worse -- and S033 shipped
the more conservative of two greens deliberately, one change at a time. This step
is where the other one gets its verdict.

## What the sweep already knows

All at ply bound 3, depth bound 6, from `.../scratchpad/rfp_ply_sweep.tsv`:

| margin | nodes at depth 9 | fast suite |
|---|---|---|
| 75 | 1216123 | green |
| 100 | 1422053 | green, shipping |

Margins 150, 200 and 300 were measured too and prune less, so they are the wrong
direction unless 75 loses. `.../scratchpad/rfp_sweep.tsv` has them.

## The trap this step walks into

**A margin that prunes harder makes the rule wronger, not just faster.** RFP
returns a static lower bound and a mate is what that bound cannot respect
(DEC-060). The mate cases are green at 75 today, and that is one position's worth
of evidence, not a proof. Any margin below 75 needs the same treatment: the fast
suite green first, the SPRT second, and a margin that goes green only by
arithmetic is what the material-leader case exists to catch.

## Cost

One verdict. S033's took 44 m 10 s at 12 cores for 1012 games; a smaller effect
takes longer, and this one is expected to be small either way.
