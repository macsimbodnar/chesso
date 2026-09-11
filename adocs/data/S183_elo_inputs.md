# The Elo arithmetic, re-derived from recorded inputs — S183, 2026-09-11

`adocs/plan.md` discounted self-play to list Elo at "~60 % sticks", took "a
fifth off for interaction", summed four blocks and a margin, and concluded
"the midpoint clears 3000". **The per-step figures it was summed from were
recorded nowhere**, so the claim could not be checked:
`2026-09-04_plan_review-F04`. This file records inputs, states the rule for the
third discount **before** computing it, and lands wherever the arithmetic
lands.

Nothing here is a measurement. It is arithmetic over other engines' published
figures, and DEC-019 is the standing warning that such a figure decides what to
try and never what to conclude.

## The rule for the third discount, written before the number

The project has published-to-measured transfers on its own record. All of them,
with the stamp each is read from:

| what | published | measured here | ratio |
|---|---|---|---|
| staged move generation (S006) | 30 to 50 | **0** | 0.00 |
| capture ordering (S005) | ~150 | **slower** | negative |
| SEE pruning in quiescence (S015) | no figure quoted | **0** | n/a |
| history malus and gravity (S093 v1) | +37.49 Weiss, +28.0 Lynx | **+10.73 +/- 6.70** | 0.29 to 0.38 |
| history persistence across `go` (S093 v2) | +12.5 Lynx | **-1.65 +/- 4.22**, reverted | **wrong sign** |
| quiescence stand-pat from the table (S130) | +10.78 / +12.09 Weiss | **+1.14 +/- 4.04**, no verdict | 0.09 to 0.11 |
| the wiki's killer replacement rule (S149) | published practice, no figure | **-11.02 +/- 10.53** | **wrong sign** |
| static evaluation at every node (S108) | +4.6-class inputs | **+2.28 +/- 4.40**, no gain claimed | ~0.5, not a verdict |

Five have a published figure and a measured one on both ends. Of those five:
**two are exactly zero, one is the wrong sign, one is 0.10, and one is 0.33**.
Taking the capture-ordering row charitably at 0.00 rather than at its true
negative, the **mean is 0.061 and the median is 0.00.**

**The rule, fixed here before any sum is computed: the headline discount is
0.38, the largest ratio this project has ever measured.** It is chosen because
it is the most generous reading of the record that is still a measurement, so a
range built on it cannot be dismissed as pessimism; the mean and the median are
reported beside it so the shape of the record is visible rather than averaged
away. Applying a mean over a set containing wrong signs would be arithmetic on
a quantity that is not a ratio.

## The inputs, per block

**Selection rule, also fixed before the sums:** one headline figure per pending
step, the **largest sourced** figure for the change the step actually makes,
excluding any part the engine already ships. Where a step's only figures are
unverified (S185's marking), the figure is listed and the sum is given with and
without it. Where a step carries no published figure at all, that is stated
rather than filled in.

Sources are the URLs in `adocs/data/2026-09-04_plan_review_literature_check.md`
and in each step file after S185; **unverified** means no source was located by
either pass.

### Block 1, the search

| step | what | published | source |
|---|---|---|---|
| S024 | continuation history, one-ply table | **+44.68** / +33.95 | Weiss #477 (A23) |
| S109 | the shallow-depth pruning block | **+204** | Stockfish #2401 (A17) |
| S091 | SEE pruning of captures in the main search | **+9.56** | Ethereal 10.00 ledger; the ledger prices removal at -41.54 (A4) |
| S098 | reduction refinements only, not the log table it already ships | **+30.4** = 11.40 + 9.34 + 4.64 + 1.87 + 3.11 | Lynx #613, #1233, #1135, #1529, #1535 (S181) |
| S095 | internal iterative reduction | **+17** | engine write-up, unverified magnitude |
| S099 | pawn correction history | **+11.35** | Lynx #1662 (A21), banded 3224-3291 |
| S097 | singular extensions plus multicut | **+26.7** = 20.53 + 6.21 | Lynx #1731, #1751 (S181), banded 3224-3291 |
| S113 | ProbCut | **+6.36** | introduction figure at ~2950 |
| S114 | eval-scaled null-move reduction | **+12.3** | Berserk |
| S115 | aspiration refinements | **+2.20** | Lynx |
| S116 | razoring at depth one | **+6.02** | Berserk #339; CPW puts Stockfish's reintroduction at ~1 |
| S132 | node-fraction time management | **+9.85** | Ethereal 60f4d5c5 (A20) |
| S188 | check extension in the move loop | **+4.14** | Ethereal 3f4ef537 (A6) |
| S112, S131, S022, S202 | quiescence futility, promotions, the delta decision, the mate-score class | **no published figure** | — |

**Block 1 raw sum: +384.6.**

### Block 2, speed

| step | what | published | source |
|---|---|---|---|
| S117 | packed middlegame/endgame score | **+25.41** | Berserk #65 (A26) -- prices a 17-commit bundle with tuning, not the packing |
| S120 | evaluation cache | **+11.86** | engine write-up; Halogen #92 +7.37 |
| S119 | table clusters, ageing, prefetch, huge pages | **+10** (5 to 15) | **unverified** (S185) |
| S020, S055, S042, S032, S030 | in-check once, merged taper, en passant, PEXT, 16-bit moves | **no Elo figure** -- each owes a timing, converted at DEC-083's 1.43 Elo per % nps at LTC and 2.10 at STC | Stockfish wiki (A31) |

**Block 2 raw sum: +47.3**, plus whatever the five timing steps return through
the nps conversion, which is not a published figure and is not summed here.

### Block 3, the corpus and the evaluation

| step | what | published | source |
|---|---|---|---|
| S121 | mobility area and curves | **+19.95** | Stash v27 (A27) |
| S123 | passed pawns, king proximity | **+22.27** | Stash v32 (A27); the group's +36.1 is **unverified** |
| S125 | connected and phalanx pawns | **+25.38** | Stash v31 (A27); the +10.68 conditioning figure is **unverified** |
| S122 | king safety rebuild | **+13.93** = 4.24 + 9.69 | Stash v31 (S185) |
| S101 | threats | **+10.13** | Stash v26 (A27) -- Stash's term is *initiative*, wider than S101's goal |
| S124 | endgame scaling | **+11.3** | **unverified** (S185) |
| S118 | pawn hash | **+10.11** | **unverified** (S185); only the >95 % hit rate is sourced |
| S133 | king-relative piece-square tables | **+65** / **+88** are *release bundles*, not the tables | Berserk 4.3.0, Leorik 2.5 (A18, A19; DEC-133) |
| S134 | fold the two degeneracies | **0 by construction** -- bit-exact | S100 |
| S135, S136 | the zero-weight groups | +16.10 bishop pair is the nearest sourced figure; the +16.7 / +4.2 / +9.2 the plan quoted are **unverified** | Weiss #95 (S185) |
| S082, S083, S039, S102, S126 | corpus, clamp, outposts, the full refit | **no published figure**; S126's local evidence is S028's **+188.74** measured here, a one-time move from hand-picked to fitted that cannot repeat | S028 |

**Block 3 raw sum: +113.1** counting the seven term figures, of which **+21.4
is unverified**; +91.7 without them. S133's bundles and S126's local figure are
deliberately not summed.

### Block 4, close

| step | what | published | source |
|---|---|---|---|
| S127 | SPSA over the whole search parameter set | **no published figure** | S085 is the local precedent |
| S129 | Syzygy, three to five men | **+13 at six men**, and the only three-man figure found is **0** | CPW, talkchess (A29; S185) |

**Block 4 raw sum: 0** that this file is willing to claim.

## The sums, and the three discounts

| | raw published | x 0.60 list | x 0.80 interaction | x 0.38 transfer |
|---|---|---|---|---|
| block 1, search | +384.6 | +230.8 | +184.6 | **+70.1** |
| block 2, speed | +47.3 | +28.4 | +22.7 | **+8.6** |
| block 3, evaluation | +113.1 | +67.8 | +54.3 | **+20.6** |
| block 4, close | 0 | 0 | 0 | **0** |
| **total** | **+544.9** | **+327.0** | **+261.6** | **+99.4** |

## What that lands on, against the 2559 anchor

The anchor is **2559 +/- 25, soft**, and `plan.md` records that it carries
**121.8 Elo of internal disagreement** across five references until S152 runs.

| reading | gain | lands at |
|---|---|---|
| this file's reconstruction, two discounts | +261.6 | 2821 |
| this file's reconstruction, all three (rule above, 0.38) | **+99.4** | **2658** |
| the same at the mean ratio 0.061 | +16.0 | 2575 |
| the same at the median ratio 0.00 | 0 | 2559 |
| `plan.md`'s own quoted range, two discounts | +390 to +680 | 2949 to 3239 |
| `plan.md`'s own range with the third discount | +148 to +258 | **2707 to 2817** |

**Two things fall out, and the second is the finding.**

**One: the plan's own sums are not reproducible from the figures its files
carry.** Its four blocks after two discounts read +180 to +280, +90 to +160,
+40 to +90 and +50 to +90; the reconstruction above reads +184.6, +54.3, +22.7
and 0. Search agrees almost exactly with the plan's low end, which suggests the
original arithmetic was the same shape. Evaluation, speed and tuning do not:
the plan claims two to four times what the recorded figures support, and
tuning's +50 to +90 has **no published input at all** behind it -- its only
evidence is S028's +188.74, a one-time move from hand-picked constants to
fitted ones, which by construction cannot happen twice.

**Two: nothing here clears 3000, and the high end does not either.** The most
generous reading that is still a measurement -- the plan's own range, its two
discounts, and the largest transfer ratio the project has measured -- lands at
**2817**, and that is **183 short**. Adding the anchor's whole 121.8 Elo of
disagreement in the favourable direction reaches 2939 and is still short. The
reconstruction from recorded inputs lands at **2658**.

**This is arithmetic and not a measurement, and it decides nothing on its
own.** What it does is put the question DEC-071 exists for: whether the goal
is reached by this list, by a longer one, or only with the network premise
DEC-054 parked. That is the owner's to weigh. S183 changes no step's order and
no step's content on this result, which its `excludes:` forbids for exactly
this reason.

## Re-derived when the two largest inputs land

S024 (+44.68) and S109 (+204) are 65 % of block 1's raw sum and 46 % of the
whole. **When each of them lands, this file and `plan.md`'s paragraph are
re-derived in the completing commit**, with the measured figure replacing the
published one and the transfer ratio re-taken over the enlarged ledger. Two
measurements will then be worth more than every number above.
