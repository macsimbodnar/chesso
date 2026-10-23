# Plan

The strongest CPU chess engine in the world — MIT-licensed, nothing
copy-pasted, every change proved by SPRT — in C++20, bitboard based, built on
the `achesso` branch to find out what AI-driven development can produce
(DEC-013, DEC-104). See `specs.md` for what it must do and what it is today.

**This whole list is phase one:** reach the level the published literature
already describes, by reading documented technique and implementing it here —
never by copying it (DEC-016, DEC-014). Phase two is experimentation and has no
steps yet, and should not get any until the engine is strong enough for an
experiment to mean something. S029, the network, is parked (DEC-054): the owner
put it after the mark (DEC-179), and it is un-parked by a decision taken when
S152 reads the mark, not before. When it resumes, the agent builds the trainer
and the data and the owner runs the training (DEC-015 as amended by DEC-041).

## The goal, and how it is read

**At least 3000 on the CCRL Blitz scale, 1CPU entry, without a network**
(DEC-071, DEC-089, DEC-179). The owner fixed how the claim is read on
2026-10-08, before any result exists (DEC-258): the mark is met when S152's
gauntlet reads an **anchor-mean point estimate of 3000 or more at `rating.sh`'s
standing control, 10+0.2**. The interval -- no narrower than about +/-60 --
the anchor spread and the second control are reported beside it and qualify
the claim; they do not move the bar. A shortfall at any point leads to more
hand-crafted work, never to the network early (DEC-179).

## Where chesso stands, 2026-10-08

- **About 2766** on the list (S240, 2026-09-27, build `1680439`, five
  anchors, spread 94.1; `adocs/data/rating_2026-09-27_S240_ccrl_blitz.md`).
  234 short. Leorik 2.4, the top anchor at 2830, still scores above chesso,
  so the reference set barely brackets it.
- **Since that build**, four H1s: S113 ProbCut +7.70, S131 quiet queen
  promotions in quiescence +17.84, S115 the fail-low pull +4.24, S116
  razoring at depth one +8.36 -- point estimates, which early stopping biases
  upward (DEC-063). Speed: S020 +1.63 %, S117 +3.47 %. S055 read H0 and was
  reverted; S120's cache was retired (DEC-257).
- **The search block is complete, and its refinements are nearly harvested.**
  Of the 23 verdicts from S231 to S055, 14 were H0 or no verdict.
- **The evaluation is the thin part**: mobility linear in raw counts, king
  safety linear in nine counts, the two clamped together at +/-184, five terms
  held at zero weight, and no threats, outposts, space or endgame scaling.

## What the gap needs: arithmetic, not a forecast

DEC-019: published figures decide what to try, never what to conclude. The
table below ranks the risk; it predicts nothing. Its two ratios are the only
transfers on record. **Published to self-play: 0.54** -- the kept search
verdicts measured +188 against +345 published (0.52 counting S114's and S188's
zeros; S183's per-step figures). **Self-play to list: 0.83** -- S240 measured
+207 on the list where the kept point estimates summed to about +250.

| piece | self-play | list |
|---|---|---|
| banked since `1680439`: S113, S131, S115, S116 | +38 | +32 |
| the evaluation block's sourced figures, +91.7 verified to +113.1 raw (`adocs/data/S183_elo_inputs.md`) | +48 to +61 | +40 to +51 |
| new scope with a figure: S099 +11.35, S265 +4.9 to +7.22, pins +6.15, S023 0 to +4.6, endgame cases about +10 | +17 to +21 | +14 to +17 |
| after the gate: refits measured +21.10 (S065) and +26.68 (S076), a lane +21.02 (S085), speed | about +50 | about +41 |
| **total** | **+153 to +170** | **+127 to +141** |

**On the record's own ratios the plan lands near 2900, about 100 short.** What
could close the rest: the terms with no published figure (threat extras,
complexity, trapped pieces, king defenders, the king-relative tables); an
evaluation that transfers better than search did -- the Stash figures behind
S121, S123 and S125 were measured at 8+0.08 and 10+0.1 by an engine near this
band; and the reserve. The opposite risk is on record too: three published
figures measured 0, 0 and slower here. **S267 exists because of this table**:
a drift match before the refit says whether the plan is on course, and a
shortfall stops the order for the owner to re-plan.

## How this plan was made

The order before 2026-10-08 grew out of four plan reviews and five audits: the
2026-08-19 review and its second pass (DEC-081 to DEC-089), the 2026-08-22,
2026-09-03, 2026-09-10 and 2026-09-12 audits, the 2026-09-04 plan and test
reviews (DEC-133 to DEC-144), the 2026-09-11 reorder for the goal and the
owner's rulings on the parked list (DEC-170 to DEC-184), and the 2026-09-19
study (DEC-220 to DEC-222). Their reasoning lives in `decisions.md` and in this
file as it stood before the rewrite: `git show a43ef3e:adocs/plan.md`.

On 2026-10-07 the coordinator wrote a plan from scratch without reading this
one -- a proposal, its research report and 29 step files -- and compared the
two; Codex corrected the comparison at the owner's request. On 2026-10-08 an
adversarial review of that comparison found the gaps the table above shows,
and the owner answered twelve questions: DEC-258 to DEC-261. The proposal and
the comparison are in history at `a43ef3e`; what was taken from them lives in
S261 to S267 and in the dated sections of the amended step files.

What carried over unchanged: the search block before the evaluation block;
the corpus before the fits it feeds; the seed rule (DEC-084 as made
provenance-based by DEC-105 and DEC-134); every step's tests, mate guards and
second tier (DEC-141, DEC-142); one verdict per search change.

## The order, in five parts

**First, the 2026-10-08 performance audit (S268 to S272).** Its three
behaviour-neutral speed-ups go before S119 on the owner's ruling (DEC-263):
the pre-make gives-check test, the dead rule tests after late move pruning,
and link-time optimization. Each owes an interleaved timing on the workstation
and no match, and every verdict after them is played by the faster engine.
Then the table survives a new base FEN (S271, DEC-264), which leaves harness
play unchanged and removes a cold table for a client that sends the board,
and the match that validates it through a bare-FEN relay if it is still
needed when the workstation is free (S272).

**A. Finish the search, on the workstation (S119 to S265).** The table's
cache-line clusters, aged replacement, prefetch and huge pages (S119; its
verdict also prices a third more entries at the harness's Hash=16, since
24-byte entries left a quarter of the allocation unused). Then the search
block's own SPSA lane, owed by DEC-222 (8) and never scheduled (S261), so
every later verdict, and the corpus, are played by a tuned search. Then pawn
correction history in a fixed slot (S099) -- its idle-night probe never ran --
with S110 and S111 (both H0 at 32+0.32, neither kept) and the two consumers
DEC-222 named only on its H1. Then
the capture-history family: capture history (S023), losing captures after the
quiets (S025) and late captures reduced by it (S265), the last two only if
S023 is kept.

**B. Foundations (S260 to S039, S082, S083).** Mostly the agent lane, written
while part A's matches play (DEC-260): the Apple clang build (S260), the
mutant rows (S259), the bit-exact fold of the two degenerate columns (S134),
the trace-based tuner (S262), the attack-set sharing (S263) and anchors above
2850 (S264). Then on the machine: retiring the lazy shortcut and the clamp
together was measured by one non-regression verdict (S039), which read H0 and
kept both (record `ca578de`), so the corpus and the fits still see the clamped
evaluation; the leaf-resolved corpus whose recipe is chosen offline on a fixed
validation set (S082); and its size and node budget, with the one corpus SPRT
(S083).

**C. The evaluation families (S135 to S133).** Each family screened offline
and measured by one SPRT, bisected on H0 (DEC-259). The zero-weight placement
group with bad bishop and trapped pieces added (S135) and tempo (S136) first,
because S134 unblocks them and they are cheap; then the families the Stash
record prices highest -- mobility with its area and pins (S121, v27 +19.95),
passed pawns (S123, v32 +22.27), pawn structure (S125, v31 +25.38); the pawn
cache once the pawn terms are worth caching (S118, DEC-087); threats with
hanging pieces and pawn-push threats (S101) before king safety, which reads
the cache and must not double-count attacks (S122); endgame scaling with
mop-up and specialised endgames (S124); outposts and space (S102);
complexity (S266); and the king-relative tables last, the largest parameter
count (S133).

**The gate (S267).** A fixed drift match against `1680439`, the build S240
rated, read against 232 self-play Elo, a threshold derived in S267 before any
family lands. Below it the order stops and the owner re-plans before S126.

**D. Close (S126 to S152).** The full refit on the trace tuner (S126); the
two speed steps, which owe timings rather than matches (S032, S030); the last
SPSA lane over the whole search set (S127); tablebase probing from a format
description, if an independent description exists (S129); and the rating,
both controls, on the new anchors (S152).

**At each block boundary** -- after S261, after the capture-history family,
and at S267 -- the coordinator takes one drift point against `1680439` in
S267's series and, where the block moved pruning or reduction parameters, one
DEC-202 reading at 32+0.32 and Hash=64. Both are estimates, never verdicts,
and neither is a rating (DEC-108).

## How a change is measured

- **A search change**: one verdict per change, `{0, 5}` nElo at 8+0.08,
  Hash=16, `noob_3moves.epd`, DEC-143's worst case and abort rule written
  before the first game.
- **An evaluation family** (DEC-259): each term fitted with every other
  constant frozen and kept only if it lowers held-out loss on S082's fixed
  validation set by more than the pre-registered margin; the survivors take
  one `{0, 5}` SPRT; an H0 or no verdict is bisected along a pre-registered
  partition. DEC-082's block rule for pruning is the precedent.
- **A behaviour-neutral change**: identical node counts and best moves from
  `tools/search_bench.py` (INV-6) and an interleaved timing (DEC-083); no
  match.
- **A retirement that must not lose**: `{-5, 0}` non-regression (S039).
- **Every run**: on mains power, detached, with a terminal marker and a
  watcher with four exits; its pre-registration names the open DEC-171
  findings (S259 until it closes).

## How to read the list with one machine

The coordinator holds the workstation (DEC-113) and takes the first Open entry
that owns a run. While that run plays, the next entry that owns no run -- a
document, a tool, a test -- may start, in list order. **Since DEC-260 that
includes src/ work written in a separate git worktree**: nothing is compiled,
tested or timed while a run holds the machine; the change lands between runs,
gated and timed then; and a play-altering change still waits its turn for its
own verdict, one at a time. An entry's dependencies are the entries above it,
so a step is never started out of order to fill the machine.

A run that ends while the owner is away is completed and the next run-owning
entry taken without waiting; the list is the authority. Between the verdicts
of a multi-verdict step, a src/ entry that is behaviour-neutral on node counts
may land, since it cannot contaminate the next verdict's attribution, and a
play-altering one may not (DEC-172).

An entry marked conditional runs only on the outcome it names; otherwise it
returns to the reserve, and the list moves on.

## Reserve and parked

**Conditional in the Open list:** none.

**Reserve with a step file:** S025 and S265, returned on S023 verdict 1's
H0 (2026-10-15, DEC-258 (8)). The two correction consumers DEC-222 named --
the correction in quiescence and its magnitude as a reduction and margin
input -- have no step file (DEC-222 (1)).

**Reserve candidates without a step**, first in line at S267's re-plan:
threat-indexed, pawn-structure and low-ply quiet histories; double and
negative singular extensions; fifty-move damping of the static evaluation
(DEC-258); fortress detection and a castling-ability term (S217, DEC-192); an
evaluation cache kept across moves (DEC-257). Mate distance pruning and
quiescence checks stay refused (DEC-087).

**Parked:** S029, the network (DEC-054, DEC-179).

## What this costs

On the workstation, at the 2110 games an hour DEC-190 measured on
`noob_3moves.epd`.

| part | verdicts | other machine work |
|---|---|---|
| A, search | S119, S261's verification, S099, S023: 4; conditional S110, S111, two correction consumers, S025, S265: up to 6 | S261's lane, about 9 h; two boundary readings, about 4 h 40 m each |
| B, foundations | S039, S083: 2 | S082's and S083's generation, two to three nights; S264's smoke matches |
| C, evaluation | eleven families; S118 0 to 1; bisections on H0 | |
| gate | none | a drift point, about 1 h, and a 32+0.32 reading, about 3 h 40 m |
| D, close | S126, S127's verification: 2; S129 0 to 1 | S127's lane, about 9 h; S032 and S030 timings; S152's two gauntlets with the larger manifest |

**19 verdicts on the main path, up to 27 with the conditional ones**, before
bisections. At the ledger's slow-class mean -- where single-digit effects run
-- that is 168 to 239 hours; the flat mean gives 120 to 171 as the floor. With
about 80 hours of lanes, datagen, readings and gauntlets: **roughly 250 to 320
machine-hours, 14 to 18 days at 18 hours a day.** An estimate from the ledger,
re-derived as verdicts land, never a schedule.

### What the formula says before a run, DEC-143

The ledger prices the *expected* run; the nElo run-length formula
(`adocs/testing_strategy.md` section 1.1) prices the worst one, and every
pre-registration states it before the first game.

| pair | truth at the midpoint | truth on a bound |
|---|---|---|
| `{-5, 5}`, alpha=beta=0.05 | 10465 games, 4.96 h | 6398 games, 3.03 h |
| `{0, 5}` or `{-5, 0}`, alpha=beta=0.05 | 41861 games, 19.84 h | 25591 games, 12.13 h |
| `{0, 10}`, alpha=beta=0.10 (`--fast`) | 5828 games worst case, 2.76 h | |

The harness stops at 40000 games, 18.96 h, which can end a run with no
verdict: a truth near the midpoint of `{0, 5}` meets the cap before it meets a
bound.

### The ledger: every SPRT verdict since the S105 regime

**Generated, not typed (S233, DEC-220).** The table and the figures paragraph
after it are the output of `python3 tools/ledger.py`, which reads every commit
whose message carries DEC-220's result block and appends
`adocs/data/ledger_seed.tsv`, the twenty verdicts taken before that decision.
Re-run it at every verdict and replace both, whole: no row and no figure here
is typed by hand.

| run | what it measured | wall | games | bounds | verdict |
|---|---|---|---|---|---|
| S093 v1 | history malus and gravity | 2 h 44 m | 6412 | `{0, 5}` | H1 |
| S093 v2 | history persistence across `go` | 6 h 35 m | 15398 | `{0, 5}` | H0 |
| S107 | killers for checking quiets | 1 h 37 m 52 s | 3812 | `{-5, 0}` | H1 |
| S149 | killer slot dedupe | 1 h 05 m | 2522 | `{-5, 5}` | H0 |
| S108 | static evaluation at every node | 5 h 26 m 38 s | 12774 | `{-5, 0}` | H1 |
| S165 | null-move mate-band guard | 7 h 58 m 07 s | 18598 | `{-5, 0}` | H1 |
| S130 | table score as stand pat | 7 h 12 m | 16784 | `{0, 5}` | **no verdict** |
| S148 | reverse futility depth ceiling | 6 h 19 m 35 s | 14808 | `{-5, 0}` | H0 |
| S207 | a repetition before the root is not a draw | 4 h 26 m 56 s | 10258 | `{-5, 0}` | H1 |
| S042 | en passant key only when capturable, `noob_3moves.epd` | 2 h 40 m 45 s | 5741 | `{-5, 0}` | H1 |
| S024 v1 | one-ply continuation history, unfitted, `noob_3moves.epd` | 4 h 10 m 11 s | 8954 | `{0, 5}` | H0 |
| S109 | late move pruning, futility, history pruning and quiet SEE pruning as one block, `noob_3moves.epd` | 37 m 51 s | 1364 | `{0, 5}` | **H1**, +46.90 +/- 15.43 |
| S210 F22 | quiescence scores a dead position as a draw, `noob_3moves.epd` | 13 h 16 m 30 s | 28598 | `{-5, 0}` | H1, +0.01 +/- 3.17 |
| S091 | capture SEE pruning in the main search and an extra reduction ply for a losing capture, `noob_3moves.epd` | 38 m 11 s | 1360 | `{0, 5}` | **H1**, +46.52 +/- 15.43 |
| S222 | one-ply continuation history on its own fitted scale, with plain history's coefficients fitted beside it, one vector, `noob_3moves.epd` | 2 h 55 m 08 s | 6278 | `{0, 5}` | **H1**, +11.13 +/- 6.90 |
| S098 v1 | late move reduction scaled by history at its fitted scale (divisor 699, clamp 3), `noob_3moves.epd` | 5 h 25 m 46 s | 11524 | `{0, 5}` | H0, -2.92 +/- 5.02 |
| S098 v1 leg 2 | the same term at the census p90 (divisor 1442, clamp 3), the pre-registered bisection, `noob_3moves.epd` | 6 h 10 m 32 s | 13078 | `{0, 5}` | H0, -2.34 +/- 4.73 |
| S098 v2 | late move reduction adjusted by node type: one ply more at a cut node, when not improving and when the table move is a capture, one less at a PV node, `noob_3moves.epd` | 56 m 09 s | 1990 | `{0, 5}` | **H1**, +29.05 +/- 11.52 |
| S098 v3 | the re-search depth after a reduced fail-high: a ply shallower where the score barely beat alpha, a ply deeper where it cleared the node's own best by a margin, `noob_3moves.epd` | 2 h 06 m 45 s | 4496 | `{0, 5}` | H0, -9.97 +/- 7.56 |
| S098 v3 leg 1 | the same rule with the shallower path switched off, the deeper path alone, the pre-registered bisection, `noob_3moves.epd` | 6 h 49 m 01 s | 14510 | `{0, 5}` | **H1**, +5.75 +/- 4.37 |
| S055 | mobility and king safety tapered through one division instead of two, measured from the working tree, `noob_3moves.epd` | 5 h 26 m 16 s | 11372 | `{-5, 0}` | H0, -6.72 +/- 4.89 |
| S231 | the two-ply continuation history | 5 h 37 m | 12070 | `{0, 5}` | H0, -2.65 +/- 4.82 |
| S095 | the no-table-move reduction term | 6 h 39 m | 14252 | `{0, 5}` | H1, +5.92 +/- 4.50 |
| S097 v1 | the singular extension | 9 h 27 m | 20080 | `{0, 5}` | H0, -0.81 +/- 3.70 |
| S132 | the node-fraction time manager | 1 h 48 m | 3822 | `{0, 5}` | H1, +16.28 +/- 8.47 |
| S097 v2 | the multicut | 4 h 57 m | 10470 | `{0, 5}` | H1, +7.17 +/- 5.13 |
| S188 | the safe check extension | 1 h 42 m | 3542 | `{0, 5}` | H0, -13.84 +/- 8.95 |
| S132 v2 | the node-fraction time manager at 32+0.32 | 6 h 8 m | 3208 | `{0, 5}` | H1, +17.99 +/- 8.70 |
| S236 | the fractional history reduction | 7 h 31 m | 15658 | `{0, 5}` | H0, -1.60 +/- 4.25 |
| S236 v2 | the history term at one ply | 19 h 8 m | 40000 | `{0, 5}` | **no verdict** |
| S234 | reverse futility on the table-tightened estimate | 9 h 32 m | 20062 | `{0, 5}` | H1, +4.73 +/- 3.74 |
| S235 | the reverse-futility return blended toward beta | 5 h 42 m | 11978 | `{0, 5}` | H0, -2.64 +/- 4.76 |
| S237 | hindsight reductions | 3 h 50 m | 8072 | `{0, 5}` | H0, -5.08 +/- 6.07 |
| S238 | the cutoff count | 15 h 29 m | 32574 | `{0, 5}` | H0, +0.25 +/- 2.96 |
| S112 | no-verdict at the cap: a zero, the code kept for S022 | 19 h 2 m | 40000 | `{0, 5}` | **no verdict** |
| S113 | ProbCut stays at its seeds | 4 h 34 m | 9654 | `{0, 5}` | H1, +7.70 +/- 5.41 |
| S131 | quiet queen promotions stay | 1 h 37 m | 3412 | `{0, 5}` | H1, +17.84 +/- 8.83 |
| S022 | 1's H0: the exchange gate stays | 0 h 25 m | 900 | `{-5, 0}` | H0, -65.22 +/- 17.66 |
| S022 v2 | the delta early-out leaves | 19 h 3 m | 40000 | `{0, 5}` | **no verdict** |
| S114 v1 | the static-score term leaves | 1 h 17 m | 2720 | `{0, 5}` | H0, -18.54 +/- 10.19 |
| S115 | the fail-low pull stays | 11 h 7 m | 23276 | `{0, 5}` | H1, +4.24 +/- 3.39 |
| S114 v2 | the null move's entry gate leaves | 19 h 7 m | 40000 | `{0, 5}` | **no verdict** |
| S116 | razoring at depth one stays | 3 h 55 m | 8234 | `{0, 5}` | H1, +8.36 +/- 5.62 |
| S272 | a bare-FEN client gains from S271's table | 0 h 18 m | 716 | `{0, 5}` | H1, +75.91 +/- 16.87 |
| S119 | the cluster table gains at the harness setting | 7 h 43 m | 16128 | `{0, 5}` | H1, +5.24 +/- 4.06 |
| S261 | the search block's fitted vector ships | 0 h 28 m | 986 | `{0, 5}` | H1, +59.78 +/- 16.52 |
| S099 | the pawn key and the correction are reverted | 4 h 36 m | 9916 | `{0, 5}` | H0, -3.47 +/- 5.03 |
| S023 | 1's H0: capture history in the margins reverts | 4 h 21 m | 9330 | `{0, 5}` | H0, -3.80 +/- 5.18 |
| S283 | S099's pair gains at 32+0.32, S099 relands | 8 h 48 m | 4738 | `{0, 10}` | H1, +8.51 +/- 6.70 |
| S039 | the lazy shortcut and the clamp are kept | 2 h 23 m | 5116 | `{-5, 0}` | H0, -12.23 +/- 7.08 |
| S110 | the non-pawn correction tables are reverted | 6 h 44 m | 3644 | `{0, 10}` | H0, -3.34 +/- 7.71 |
| S111 | the continuation correction tables | 10 h 11 m | 5484 | `{0, 10}` | H0, -1.01 +/- 6.15 |
| S289 | Revert 's null-move table guard on its H0 (DEC-309) | 4 h 3 m | 8716 | `{-5, 0}` | H0, -7.42 +/- 5.07 |
| S292 | the narrowed null-move guard | 11 h 7 m | 23802 | `{-5, 0}` | H1, +0.35 +/- 3.08 |

**The ledger holds 54: mean 6 h 09 m, median 5 h 26 m, 663191 games in 332.95 hours, 1991.8 an hour across the set.** **Fast class**, an effect outside the bounds interval -- nineteen runs, mean **1 h 53 m**. **Slow class**, inside it, on a bound or a true zero -- 35 runs, mean **8 h 29 m**.

### Priced by class

The class of a row is `tools/ledger.py`'s (DEC-223): the nElo estimate's
interval either misses the bounds pair, which is fast class, or reaches it,
which is slow class -- an effect inside the interval or a true zero runs to
the wall. Two rows are classed against the rule and stay that way: **S149 and
S207** carry the hand classification the eleven pre-`noob_3moves.epd` rows were
given when the class means were first computed, where the rule reads both as
slow -- S149 at -14.21 +/- 13.56 against `{-5, 5}` and S207 at +4.47 +/- 6.72
against `{-5, 0}`. `python3 tools/ledger.py --audit-seed-class` prints the
comparison and `adocs/data/ledger_seed.tsv`'s header records it.

**Re-derived, not restated (DEC-136).** A step that lands a verdict re-runs
the ledger in its completing commit: the table grows by one row and the
figures paragraph moves with it. A verdict that lands without moving them is a
missed edit. The cost estimate above is re-derived at each block boundary.

## How this file works

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered or reused -- including for the retired S019, S026,
S031, S056 to S061, S063, S079 to S081, S086, S090, S092 and S096, and for
S128, folded into S152 by DEC-108 -- so reordering is a one-line edit to the
Open list. The 2026-10-07 proposal's P01 to P29 were never ids; what was
adopted from them got S260 to S267 (DEC-258).

Order is read from the numbered entries under `## Open`, and the next step is
the first of them. An id named in a sentence anywhere else in this file is
prose: it does not change the order. Every pending step file appears as an
Open entry and every Open entry names an existing step file.

**A citation names a file and a symbol, and repeats the path for each one.**
`src/search.cpp` `negamax`, `src/evaluation.hpp` `LAZY_EVAL_MARGIN`,
`tests/test_search.cpp` "pruning does not hide a forced mate" -- and, where
there is no definition to name, a phrase quoted out of the file. Never a line
number, and never `:917` after a sentence that named the file. Backtick the
symbol: the checker fires on a backticked identifier or a double-quoted phrase
directly after the path and on nothing else, so a path followed by an ordinary
word is prose and goes unchecked. The path is repeated because the checker
cannot decide which file an inherited path meant (DEC-120); the symbol replaced
the line because line citations drifted faster than they could be repaired
(DEC-135, gated in the fast suite since DEC-159). The check is existence, not
relevance.

**Both lists are maintained by hand** (moltke v1, DEC-109): completing a step
means moving its file, moving its entry from Open into `## Done recently`,
dropping the oldest of the five, re-running the ledger if it landed a verdict,
and rewriting `status.md` -- deliberately, in the completing commit.

## Done recently

- S291  **the multicut row S083's re-landing loses: reverse futility's known price, no rule at fault** -- at S083's constants on S292's tree, reverse futility cuts at ply 3 over a lower bound (`RfpMaxDepth` 4 restores the mate), so the row joins S083's H1 rulings (DEC-308 (2), DEC-311); found beside it, reverse futility failing high against a whole search's mated bound (90 returns over four mate sets, 0 in 11503 ordinary-play positions), ruled BUGS-binding under DEC-171 (c) and taken by S293. 2026-10-23.
- S289  **the mate S083's paste hid: the null move against the table's mated bound, closed by S292's narrowed guard** -- traced from the engine's output (the reduced search stores the defender's mated upper bound, the re-search's null move fails high against it), reached at the shipped weights (six of S165's 31 roots mated in two), fixed in `cd50178`, whose guard read H0 (nElo -10.68 +/- 7.29) and was reverted (DEC-309); completes on S292's H1 (DEC-310): its cases red first and green at the paste's constants; the ply-2 case's `first_exact` (late move reduction, 6 against 4), `EXACT_FLOOR` 158 against 163 and the mate-in-three floor's separation are the owner's on S083's H1 (DEC-307 (4)). 2026-10-23.
- S292  **the narrowed null-move guard: H1 at 8+0.08, `{-5, 0}`, it stays** -- no null move where the entry is a mated upper bound or exact score written by a node that skipped no move (the entry's `no_move_skipped`, DEC-310, after David-Tabibi and Netanyahu 2002); 98 to 99 % of S289's reverted guard's firings were on bounds of nodes that skipped moves; `e27d3e5` against `10f1e42`: LLR 2.96 over 23802 games, nElo +0.50 +/- 4.41, 0 forfeits; the six roots mated in two report `mate -2` (31 of 31), bench 6042933 unmoved, S170 C 1200000 -> 500000 by the rule. 2026-10-23.
- S111  **the continuation correction tables: H0 at 32+0.32, `{0, 10}`, never committed** -- two tables keyed on (2 plies ago, 1 ply ago) and (4, 1) (DEC-302), averaged with the pawn read (DEC-300), played from the worktree's tree against `03fa3ec` (DEC-303): LLR -2.96 over 5484 games, nElo -1.52 +/- 9.20; recorded as zero with the played diff (record `7b6f951`), bench 6042933 unmoved; `EXACT_FLOOR` stays 163, no owner ruling needed. 2026-10-22.
- S110  **the non-pawn correction tables: H0 at 32+0.32, `{0, 10}`, reverted** -- two non-pawn keys per colour and two tables averaged with the pawn read (DEC-300), `39194e4` against `bb3d31d`: LLR -2.98 over 3644 games, nElo -4.88 +/- 11.28; reverted whole, bench 6570375 -> 6042933, DEC-301 lapsed; 46 mutants killed before the run. 2026-10-21.

## Open

1. S293  **BUGS, DEC-171 (c), DEC-311: first; blocks S083** -- close the defect S291 found -- reverse futility failing high at a node whose entry is a mated upper bound or exact score from a node that skipped no move, DEC-306's shape -- as its own measured change, with S292's condition reused, or show by measurement that no guard of that kind is correct
2. S083  **paused by S293 (DEC-311); S291 attributed the multicut row to reverse futility's price, one of the five H1 rulings (DEC-308 (2)); S289 completed on S292's guard (DEC-310); its re-landing was made on S289's guard, since reverted, so it is re-read on the tree S293 leaves (DEC-307, DEC-309, DEC-310): curve 0 read stop, the candidate is S082's pool at 46 rows a game** -- the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, and one SPRT measures the corpus that ships
3. S290  **filler, DEC-171: behind S083; its budgets moved again with S292 (C 500000, DEC-310), so its diff is rebased on that tree** -- the script that applies S203's budget rule to `adocs/data/S170_cases.tsv` is tracked in the tree and named where the budgets are
4. S135  unfreeze the piece placement group, add bad-bishop and trapped-piece features, and refit it, one SPRT over the family
5. S136  unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- three divisions today, S055 having read H0 -- refit and resolve it at bounds that can
6. S121  **S122's design amended by the owner first (S039's H0)** -- mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on, and a pinned piece counts only the moves along its pin
7. S123  passed pawns are scored by rank crossed with whether the push is available and safe, by both kings' distance, and candidates are scored too
8. S125  phalanx, supported and weak unopposed pawns join the isolated, doubled and backward pawns that exist, and each is fitted
9. S118  the pawn terms and the king shelter are computed once per pawn structure and cached, instead of at every evaluation call
10. S101  threat terms: a piece attacked by a lesser piece, an attacked piece nobody defends, and a safe pawn push that would attack a piece
11. S122  king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks, weak squares and the king zone's defenders, and it is no longer clamped
12. S124  the endgame half of the score is scaled toward a draw by what is on the board, and won lone-king and specialised endgames are scored so they convert
13. S102  outpost and space terms in the evaluation, one family
14. S266  a complexity term moves the endgame score toward zero where the stronger side is unlikely to convert, never flipping its sign
15. S133  the piece-square tables become king-relative -- indexed by a king bucket as well as piece and square -- and every entry is fitted
16. S267  **the gate** -- a drift match against `1680439`, the build S240 rated, read against 232 self-play Elo; below it the order stops for the owner to re-plan
17. S126  every constant in the evaluation is refitted once the search that consumes them has stopped moving
18. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
19. S030  move_t drops the moving piece and becomes 16 bits
20. S127  the last SPSA lane, over the whole search parameter set, and an independent SPRT of what it returns
21. S129  three, four and five man tablebase probing, written from the format description, if an independent description exists
22. S152  the rating, once, near the mark: both controls, the new anchors, the claim read as the point estimate at 10+0.2 (DEC-258)
23. S029  **parked, DEC-054 and DEC-179** — a perspective network evaluation trained on chesso's own self-play
