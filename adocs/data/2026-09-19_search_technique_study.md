# Literature and open-source resources, read for ideas — 2026-09-19

What the open-source record contains that chesso's plan does not, what the
published measurement ledgers say each of those things was worth, and what was
added and then thrown away. Written to improve chesso's plan, not to import
anyone's code.

---

## 0. Provenance, first, because it governs everything below

This is an analysis of literature and open-source resources: published test
ledgers whose commit messages carry SPRT result blocks, public wikis,
published articles. **No source, no table and no constant from any project is
reproduced anywhere below** (DEC-016, DEC-084 as amended by DEC-105), and
every technique taken from it is implemented here from its published
description (DEC-221).

**What this report contains and does not contain.** Technique names,
mechanisms described in prose, and Elo measurements read out of published
commit messages. Every tuned integer in the record was deliberately left out:
under DEC-084 as amended by DEC-105 another engine's tuned output is never a
seed, and those constants are SPSA output written straight into published
source. Where a mechanism needs a shape, the shape is named ("linear in depth
with a cap") and the number is not given. Anything chesso builds from this
report gets its constants from this project's own tuner or SPSA, starting from
a value this project picks.

**Nothing below is a chess judgement.** It is a comparison of technique
inventories and of published numbers.

---

## 1. Method, and what the evidence actually is

Read on 2026-09-19.

1. **The published mechanism descriptions.** The techniques of §3.2 were read
   out of open-source resources and are given here in prose only; an
   implementer works from the description (DEC-221).
2. **The commit log as a measurement ledger.** The SPRT result is committed
   into the commit message: Elo with a 95 % interval, time control,
   threads, hash, LLR with its bounds, game count, pentanomial, and a link to
   the run on the testing instance. **796 commits of the record carry at least
   one such result, about 44 % of them; 794 of those are dated 2025-02 or
   later.** They were parsed into `2026-09-19_technique_ledger.py`, committed
   beside this file after the 2026-09-19 review; every figure quoted below came
   out of that parse and is re-derived by running it.

Also read: published release notes, and the published description of the
testing regime (the STC/LTC split).

### 1.1 What the ledger is and is not

- **It is passes only.** OpenBench keeps failed tests on the instance, not in
  git. The real test count is several times what is here; the usual OpenBench
  pass rate is well under half. **Every cost figure below is therefore a
  floor.**
- **The record's bounds are stated in logistic Elo; chesso's are in nElo.** One
  commit in the whole record mentions nElo at all. chesso's DEC-143
  pre-registers with `model=normalized`, so the pairs are on different scales —
  but `README.md` in this directory already records the conversion this project
  measured: **5 nElo was 3.54 logistic Elo at S165's 44.75 % draws, and 3.87 to
  3.99 at the lower draw rates of S021, S076 and S107.** So chesso's `{0, 5}` is
  roughly the record's `[0, 3.5]` to `[0, 4]`, which is where most of the
  published testing sits. **The two regimes are close enough that the record's
  run lengths are informative here**, and the reported Elo figures are the same
  kind of number as chesso's ledger reports. Bounds used in the record, by
  frequency: `[0, 3]` 372 times, `[-2.75, 0.25]` 215, `[0, 4]` 169, `[-3, 0]`
  103, `[0, 5]` 89. The 2025-02/03 rebuild era — the part most relevant to
  chesso — ran mostly at `[0, 5]` logistic, which is *looser* than chesso's
  gainer pair, so those run lengths are if anything optimistic.
- **The rebuild base already had an NNUE.** The February 2025 sequence is a
  from-scratch rewrite of the *search* on top of an existing network
  evaluation, not a rewrite of a whole engine. Every figure in that ladder was
  measured with a network eval and a bare search. chesso is the opposite mix.
  §5 says where that matters.
- **Time controls.** 8.0+0.08 appears in 682 runs, 40.0+0.40 in 243,
  4.0+0.04 in 72. STC is the same control chesso adopted at S105. 206 of the 796
  tested commits carry both the 8+0.08 and the 40+0.40 control (218 carry two
  distinct controls, 252 two runs; corrected 2026-09-19, review F10).

---

## 2. The scale of the programme in the record, and what chesso can afford of it

The single most important number in this report is not an Elo figure.

**The 794 commits' recorded runs since 2025-02 total 41,964,262 games**
(31,493,134 counting only the first run of each; the first figure stood here
until the 2026-09-19 review, F10). At the throughput chesso's own ledger
measures — 2,200 games an hour on the workstation, `plan.md` "The ledger" —
that is **≈19,100 machine-hours, about 2.2 years of a dedicated machine running
24/7**, for the passes alone. That testing was spread across a distributed
instance of many machines. chesso has one machine and the MACHINE rule.

So the record cannot be replayed. It can only be **mined for ordering**,
and the mining rule falls straight out of the record's own data.

### 2.1 Effect size is the price of the verdict, measured over 642 runs

Every 8.0+0.08 run in the ledger, bucketed by the Elo it reported, against the
games it took to report it:

| STC Elo band | runs | median games | p90 games | median hours at 2200 g/h |
|---|---|---|---|---|
| ≥ 20 | 19 | 1,310 | 2,174 | **0.6 h** |
| 10 – 20 | 17 | 3,432 | 4,930 | **1.6 h** |
| 5 – 10 | 67 | 8,528 | 11,344 | **3.9 h** |
| 3 – 5 | 95 | 16,584 | 24,026 | **7.5 h** |
| 2 – 3 | 128 | 31,369 | 45,146 | **14.3 h** |
| < 2 | 316 | 55,881 | 112,018 | **25.4 h** |

Caveat: the band and the bound pair are confounded — the big early effects ran
at `[0, 5]` or `[0, 10]` logistic and the 2026 work at `[0, 3]`, which
lengthens runs independently of the truth, and `[0, 5]` logistic is looser
than chesso's `{0, 5}` nElo (§1.1), so the top two rows are optimistic for
chesso by some amount this study did not compute. The curve is still the right
shape, and it is
**chesso's own ledger's fast/slow split confirmed on 642 runs instead of
twenty**: `plan.md` prices the fast class at 2 h 31 m and the slow class at
6 h 17 m; the table says 0.6–1.6 h for a large effect and 7.5 h upward for a
small one.

**The operating conclusion.** chesso's pending list is "almost all slow class"
by `plan.md`'s own admission. The record is the best available instrument for
finding the remaining items that are *not* slow class. Every recommendation in
§6 is ranked by the STC Elo it measured, ~~because that figure is a prediction
of **what the verdict will cost**, which transfers far better than the Elo
itself (DEC-019 is about the Elo, not about the variance)~~.
**Withdrawn 2026-09-19 (review F01): games-to-verdict is a function of the true
effect *here*, which is exactly what DEC-019 says does not transfer — chesso's
five two-ended published-to-measured transfers read 0, 0, wrong sign, 0.10 and
0.33 (`plan.md` "The third discount"), and expected games scale with 1/Elo².
The ranking is a guide to which effects were large *there*. The cost of a
chesso verdict is DEC-143's — 25,591 games on a bound, 41,861 at the midpoint,
12 to 20 hours — and every hour figure in §2.1's last column and §6 is read
as the record's, never as this project's.**

### 2.2 Gains per month in the record, which is the shape of the wall

Sum of reported STC/first-run Elo per month, passes only:

| period | tests | Σ reported Elo |
|---|---|---|
| 2025-02 (rebuild) | 39 | 1621 |
| 2025-03 | 80 | 500 |
| 2025-04 | 71 | 305 |
| 2025-05 – 2025-08 | 168 | 965 |
| 2025-09 – 2026-02 | 211 | 603 |
| 2026-03 – 2026-09 | 225 | 617 |

The sums double-count (release commits restate the total) and cannot be added
to a rating. The shape is the point: the first 120 tests bought most of it.

---

## 3. What the record has that chesso does not

The 2026-09-04 literature check already inventories six engines against the
plan. This section only records what that pass does **not** cover, or covers
without a number. Three columns: the mechanism, what the record measured for
it, and where chesso stands.

### 3.1 In chesso's plan already — the record's number is ordering evidence

| Technique | record STC Elo (games) | chesso |
|---|---|---|
| Singular extensions | **+10.40** (4276); +26.99 at 20+0.20 (1522) | S097, open, position 3 |
| Multi-cut off the singular search | **+14.17** (2992) | S097 |
| Razoring | **+8.94** (5132) | S116, open, position 11 |
| Internal iterative reduction | **+5.11** (10202) — **replaced 2026-02-15 by a reduction term inside LMR**, a simplification at −0.32 ± 0.78 over 195,882 games; the node-level cut does not exist in the record's current form (review F05) | S095, re-formed as that LMR term, DEC-222 |
| ProbCut | +4.54 (12100), then +7.22 (6598) for the modern form | S113, open, position 8 |
| NMP restricted to cut nodes | +9.19 (3934, at 6+0.06, on a `[-5, 0]` non-regression pair — a point estimate, not a gainer verdict; review F15) | S114 |
| NMP reduction up when the TT move is a capture | +5.34 (9694) | S114 |
| Static-eval condition in NMP | +7.68 (6240) | S114 (partly) |
| Pawn correction history | **+9.18** (5036) | S099, **reserve head**, position 40 |
| Non-pawn correction history | **+11.81** (3678) | S110, **reserve**, position 43 |
| Continuation ("last move") correction history | **+6.90** (6902) | S111, **reserve**, position 44 |
| Correction history used in quiescence | **+13.85** (3062) | no step |
| Capture (noisy) history | +5.97 (10008) replacing LVA | S023, **reserve**, position 41 |
| SEE threshold from the move's own ordering score | +3.96 (15082) | no step; near S025 |
| Bad captures deferred behind the quiets | +37.65 (1158) for the qsearch half | S025, **reserve**, position 42 |
| Time management by node distribution | **+17.59** (2352) | S132, open, position 12 |
| Aspiration: average score as the centre | +4.80 (11290) | S115 |
| Aspiration: root depth reduced on repeated fail-high | +4.64 (12964) | S115 |
| TT clusters | +2.03 (47436) | S119, open, position 18 |
| Quiescence per-move futility | +2.15 (45454) | S112, open, position 5 |
| Quiescence SEE pruning | +5.08 (11424) | present (S015, measured 0 here) |
| Quiescence late move pruning | +5.52 (10770) | no step |

**The largest single reordering signal in this report.** The correction-history
family — pawn, non-pawn, continuation, used in quiescence, and the
`corrplexity` consumers of §3.2 — measured **+9.18, +11.81, +6.90, +13.85,
+9.17, +3.77 and +6.15** in the record, at **3,000 to 8,300 games each**. That is
six to eight verdicts in chesso's 1.6-to-4-hour class, for a block the plan
currently holds at positions **40, 43 and 44 as reserves**, behind the whole
evaluation block. The 2026-09-04 literature check already flagged the band
claim behind that placement as `NOT FOUND` (row A22: "no source supports 'only
above ~3100'"). **The record does not settle that band either, and this study
did not try to**: the absolute strength of the rebuild in March 2025 is not
derivable from the record — the rebuild was a bare search on top of an
existing network, and no rating was published between the releases either side
of it. What the ledger does establish is the effect size inside an engine
whose evaluation was a **network**, which is the harder case for a technique
whose whole job is to correct static-evaluation error: a hand-crafted
evaluation has more error to correct, not less. That is an argument and not a
measurement, and it is the argument S099 exists to settle. The point stands
either way on **cost**: the family is six to eight verdicts in the cheapest
class this project has, and it sits behind forty steps.

### 3.2 Not in chesso's plan at all — the new material

Ranked by the STC Elo the record measured, which is also the inverse of what the
verdict costs.

| # | Technique, in prose | record STC Elo (games) |
|---|---|---|
| N1 | **Static eval tightened by the TT score.** Where the table holds a score whose bound points the same way as the difference between that score and the static eval, the *table's* score is used as the node's estimate for every margin test — reverse futility, null move, razoring — while the raw static eval is still what the correction tables learn from. One value for pruning decisions, another for bookkeeping. | **+6.07** (8414) |
| N2 | **Fail-middle returns.** A node that prunes on a margin does not return the margin's own score; it returns a point **interpolated between that score and beta**. Applied at the reverse-futility return, the ProbCut return, the multi-cut return and the quiescence stand-pat. Each site got its own interpolation weight and each was tested separately. | **+9.71** (4654) for the RFP site alone; four further sites later, +1.4 to +4.5 each |
| N3 | **Corrplexity.** The *magnitude* of the correction the correction tables are applying is read as a measure of how unreliable the static eval is here, and used to widen the reverse-futility margin, widen the futility margin, and **reduce less** in LMR. | **+9.17** (4962) in LMR; +1.81 STC and +3.77 LTC in futility; +2.56 in RFP (labels corrected 2026-09-19, review F15) |
| N4 | **Cutoff count.** Each node counts how many beta cutoffs happened at it; the *child's* count, seen from the parent, increases the parent's reduction of later moves. A child that keeps failing high is a node whose siblings are not worth depth. | **+6.00** (8688), later refined +3.52 |
| N5 | **Hindsight reductions.** At the child, the *parent's* reduction of the move that got here is read together with how the static eval moved across that move. A heavily reduced move whose eval got worse gets a ply back; a lightly reduced move whose eval got better gives one up. Depth is corrected after the fact, one ply down from the decision. | **+6.03** (8590) |
| N6 | **Reductions on the non-LMR path.** Moves that LMR does not touch — the first move, and every move at depth below the LMR threshold — get their own reduction formula, accumulated the same way and converted to whole plies by thresholds. | **+3.92** (17740, at LTC) |
| N7 | **Threat dimensions in the quiet history index.** The butterfly table is split by whether the move's **from** square and its **to** square are attacked by the opponent. A quiet that runs from a threat and a quiet that walks into one are different moves and get different history. | +2.99 unfinished at STC; **+8.81** (6512) at 20+0.20 |
| N8 | **Bad-noisy futility pruning.** A separate futility rule for losing captures, with its own margin and its own depth ceiling, firing in the bad-noisy stage of the picker and breaking the loop rather than skipping the move. | **+5.30** (11082) |
| N9 | **Decaying malus.** The malus a quiet gets for having been tried and not cut off **shrinks with its position in the tried list**. An early quiet is punished nearly in full, a late one barely. | +4.67 (15778), +5.18 at LTC |
| N10 | **Prior-countermove history updates.** When a node fails **low**, the opponent's previous quiet move is *rewarded* — it refuted this node. The bonus is scaled by how late that move was tried, whether it was the TT move, and how far below the parent's own eval the node came back. | +3.87 (16414); a later refinement limiting it to unexpected all-nodes, +5.22 |
| N11 | **History written on a TT cutoff.** A node that cuts off straight from the table still credits the table's move, when that move is quiet and the parent tried few moves before reaching here. | +3.51 (18786) |
| N12 | **History bonuses scaled by node type.** The bonus and malus magnitudes differ at a cut node from elsewhere. | +4.35 (16996) |
| N13 | **Pawn history.** A quiet-move history table indexed by a **bucket of the pawn-structure key** crossed with (piece, destination). Distinct from a pawn hash table for evaluation (chesso's S118) — this is move ordering. | +4.43 (17334) |
| N14 | **Upcoming-repetition detection.** Before searching, a node detects that the side to move *can force* a repetition and raises alpha to the draw score, cutting when that already beats beta. Implemented with a cuckoo table of reversible-move hash differences. | +3.46 (18498); a related GHI fix +9.74 (3496) |
| N15 | **Move ordering by board geometry, not only by history.** Four separate terms on quiets, each tested on its own: a bonus for a move whose destination gives check; a bonus for moving a piece **off** a square attacked by a lesser piece; a penalty for moving it **onto** one; a bonus for a safe destination from which the piece would attack something more valuable; a penalty for pushing a pawn out of one's own king's shelter. | check squares +3.13 / +5.09 LTC; offense squares +2.45 / +3.66 LTC; further terms +3.4 to +7.2 |
| N16 | **Low-depth singular extension.** At a shallow cut node whose static eval is below alpha by a margin, extend by a ply without running a verification search at all. **Read the numbers before wanting this**: it *failed* at STC and shipped on LTC alone. | **-0.63 over 34032 at STC (H0)**; +1.09 over **157,644** games at 40+0.40. Out of reach here |
| N17 | **Negative extensions.** When the verification search says the TT move is *not* singular and the node still looks like a cut node, the whole node is searched **shallower**. | +1.85 (70894) |
| N18 | **Secondary reverse futility.** A second RFP rule with its own gate and margin beside the first. | +4.41 (16154) |
| N19 | **Material scaling in place of game phase.** The phase interpolation is driven by a material count rather than by a piece-count phase. | +2.52 (31680) |
| N20 | **Optimism.** A small side-dependent offset applied to the evaluation, derived from the best score the search itself has been seeing, shared across threads. Contempt without a constant. | +7.64 (6000, at 25+0.25) |
| N21 | **Laterality and critical-chain length in LMR.** How far "off the main line" the current path is — accumulated from the move indices along it, and separately the plies since the last node where something interesting happened — both move the reduction. | laterality +6.13 (11624); critical chain +2.03 (51114) |
| N22 | **Reduction jitter.** A small pseudo-random offset, derived from the node counter, added to the reduction. Sold as thread divergence; it is also stochastic rounding of a fixed-point reduction. | −0.10 at STC, **+3.19** at 5+0.05 SMP |
| N23 | **Store the raw static eval in the TT entry**, and write a shallow "eval-only" entry the first time a node is evaluated, so the next visit at any depth reuses it. This is chesso's S120 evaluation cache **done inside the existing table** instead of beside it. | bundled with clusters; +20.29 in an early 2486-game run, +2.03 over 47436 later |
| N24 | **Time management: hard bound by a constant scale; soft bound ramping with the move number; best-move instability as a factor; score trend as a factor.** The soft limit in the record is a *product of five* multipliers — node share, score trend, PV stability, eval stability, best-move changes — not a sum of adjustments. | max time by move number **+24.01** (1870); constant hard scale +8.19 (7808); eval stability +6.15 (7744); PV stability +3.75; best-move instability +4.00 LTC; score trend +4.34 LTC |
| N25 | **Quiescence: late move pruning after a small, fitted move count unless the move gives check**, and a SEE threshold that moves with the distance from alpha, the corrplexity and the move's history. | qsearch LMP +5.52 (10770); qsearch SEE margin by history +4.53 |

### 3.3 The architectural difference, which is not a feature

**In the record the reduction is accumulated in fixed point — at sub-ply
resolution — and divided once at the end. chesso accumulates it in whole
plies.**

`build_lmr_table()` in `src/search.cpp:175` truncates a `double` into a
`uint8_t`, and `lmr_node_adjustment()` adds whole plies to it. Every term that
wants to say "reduce a third of a ply less here" can only say nothing or say a
whole ply.

This is a hypothesis about a **recorded chesso failure**, not a general
argument. S098 verdict 1 measured late-move reduction scaled by history at
**−2.92 ± 5.02** and its bisection leg at **−2.34 ± 4.73**, and the term left
the tree (DEC-213). The record measures the same idea at **+9.09** (4970 games).
The three differences that could explain a sign flip are: the record's history
sum is richer (quiet + two continuation tables, threat-indexed); its reduction
is fixed point, so the history contribution is a fraction of a ply; and its
rounding is jittered rather than truncated. **The second is testable cheaply
and independently of the first**, and it would be tested as a behaviour change
with an SPRT, not as a refactor: moving the accumulator to fixed point changes
which moves get which depth.

Two smaller architectural notes:

- **The record's continuation tables are bucketed by `(in check, move was
  noisy)`** as well as by (piece, to) × (piece, to). chesso's `cont_hist` and
  `cont_hist2` have the two piece-square axes only. Two extra bits on an
  existing table.
- **A pointer to the continuation sub-table is carried in the search stack**,
  set once per make-move, so the read at the child is one indirection and not
  four index multiplications. chesso reached the same conclusion from the other
  side at S231 — no per-ply stack, pass the continuation key down — for the
  same reason (S191 measured 1.49 % of nps for one extra read per node). Worth
  knowing the alternative exists if a third continuation table is ever wanted.
- **Neither killer moves nor a countermove table survives in the record.** Both
  were removed (§4). Ordering is history-only, plus the geometry terms of N15.

### 3.4 Not applicable to chesso, recorded so nobody re-derives it

- **The NNUE.** The record's eval is a network and nothing else — there is no
  hand-crafted evaluation there at all, and **no commit in 794 prices a
  hand-crafted evaluation term**. The handful of commits that read like
  evaluation — king proximity to passed pawns, queens penalised for stepping
  into a threat — all belong to the move picker: they are move-ordering terms,
  not score terms. Everything in chesso's evaluation block
  (S101, S102, S121–S126, S133) has no counterpart and no number here. Under
  DEC-054/DEC-071 chesso pursues 3000 without a network, so the evaluation
  block stands on its own evidence; this study simply has nothing to say about
  it, which is itself worth recording.
- **SMP, NUMA, thread voting, MultiPV, large pages.** chesso is
  single-threaded by DEC-089 and the target is the CCRL 1CPU list. The record's
  +14.36 for a NUMA thread pool and +33.95 for MultiPV are not available and
  not wanted.
- **SIMD work.** +35.97 for manual AVX2 SIMD, several more for AVX-512, all of
  it inside the network.
- **DFRC / Chess960.** Not a chesso goal.

### 3.5 What the SPSA cadence in the record says

**Fourteen SPSA sessions with a measured result, totalling ≈+73 Elo**, at
28k to 200k games each, spread from 2025-05 to 2026-06 — roughly one every six
weeks, never once and for all. The tuning machinery there has the same shape as
chesso's `CHESSO_TUNE=ON` dual build: a macro in a parameter module that
compiles the declared set either as constants or as settable statics, gated on
a build feature — independent confirmation that chesso's dual-build design
(DEC-118) is the normal answer. In the record's current form the declared set
is *empty*: the tuned values have been folded back into the search source as
literals, and a session re-declares whatever it wants to move.
chesso's S127 is a single SPSA run at position 37. The evidence says that is
under-scheduled by an order of magnitude: on this record, **SPSA is not a step,
it is a recurring night**, and it is the single largest line item in the
ledger after the network.

**Corrected 2026-09-19 (review F07): the ≈+73 sums each session's LTC pass;
six of the fourteen are SMP sessions verified at several threads, outside
chesso's arena (§3.4); five of the seven that also ran STC read negative there
(−1.64, −1.49, −6.87, −6.52, −1.43); and 28k to 200k tuning games is 13 to 91
hours of this machine per session, not a night. Chesso's own lanes (S222,
S231, eight to nine hours) are the right unit; the cadence is DEC-222's.**

---

## 4. The traps: what was added, measured, and later removed

241 commits begin with Revert, Simplify, Remove or Drop. Each removal that
carries a number is an SPRT showing play was **no worse without the
thing**. These are the most valuable rows in the whole study, because they are
the cases where chesso would otherwise pay for a verdict twice.

| Removed | When | Verdict on removal | Read |
|---|---|---|---|
| **Killer moves** | 2025-06-19 | **+0.50 ± 1.61 over 47676 games**, bounds `[-3, 0]` | Added at +2.14 in March; a reduction discount for killers landed at +7.64 over 7,054 games in April and a separate killer stage at +2.50 in May; the heuristic was gone in June at +0.50 — the value migrated into the history sum and the reduction terms (review F16). Once quiet history, two continuation histories and a pawn history are all in the ordering sum, the killer slot adds nothing. chesso has killers, two per ply, and has already spent two steps on them: **S149** (CPW's distinctness guard) measured **−11.02 ± 10.53 over 2522 games** and was reverted, and **S159** refuted the ageing follow-up on a census before a game was played. They are not worthless here today — **S107** (killers for checking quiets) was H1. The reading is forward-looking: do not invest further in the slots, and once the history stack of §6 is built out, the right experiment is *removing* killers as a `{-5, 0}` non-regression. |
| **Countermove table** | 2024-03-14 | no number | Subsumed by 1-ply continuation history. chesso has both a `counter_moves` table and `cont_hist`. |
| **Check extensions** | 2025-04-17 | **−0.26 ± 1.49 over 58032 games**, bounds `[-4, 0]` | Added at +2.33 a month earlier and removed as free. This is a **third** independent datum against check extensions, beside Ethereal's pre-move-loop removal at +4.14/+4.54 and Stormphrax's removal (2026-09-04 literature check rows A6, A7). **S188 is open at position 4 of the plan.** Three engines have now measured this at or below zero. It should be re-priced before it is run, or run with the expectation of a zero and a `{-5, 0}` pair. |
| **Recapture extensions** | 2026-04-02 | +0.33 | Added at +6.41 five months earlier. |
| **Minor/major correction histories** | major 2026-01-05, minor 2026-04-17 | +1.38 and +2.24 | **Added together at +17.00**, the third-largest gain of the rebuild, and both later removed as free once pawn and non-pawn correction histories existed. **The correction-history axes overlap.** The lesson for S099/S110/S111: build the family incrementally and re-test each older axis after a newer one lands; the plan's ordering (pawn first, then non-pawn, then continuation) is the right one, and a fourth axis should be viewed with suspicion. |
| **Factorized quiet history**, then **factorized noisy history** | 2026-05-22 and 2026-05-12 | −0.01 and +0.75 | Both were *added* with positive verdicts (+3.05, +3.32) a year earlier, and both later removed as free. A shared "factor" term across history buckets decays to noise as the buckets fill. |
| **Move count in LMR** | 2026-05-14 | +3.23 STC at `[0, 3]`; **+4.15** at LTC on `[-2.75, 0.25]` | The reduction there stopped depending on the move index at all — it is depth, history, node type, and the path terms. chesso's LMR is a log(depth)·log(move number) table, which is the *only* thing it depends on before the node adjustment. This does not say chesso should drop the move number; it says the move number's role shrinks to nothing once enough better signals are in the sum. |
| **Secondary TT aging** | 2025-10-03 | **+3.94** to revert | Landed and reverted inside three weeks. Relevant to **S119**. |
| **Null-move history**, **insufficient-material detection**, **evaluation grain**, **random component in eval**, **alpha-raise counter**, **root-depth condition** | various | +0.5 to +2 each | The general shape: a rule that once paid stops paying as the search around it gets better, and is worth **re-measuring as a removal**, ~~which is cheaper than it sounds because a `[-2.75, 0.25]`-style non-regression on a truly free change stops fast~~ **Corrected 2026-09-19 (review F08): it is not cheap. The record's 151 STC runs at `[-2.75, 0.25]` have a median of 34,548 games and its 408 non-regression runs average 40,347; chesso's `{-5, 0}` pair prices a truth on the bound — which is what a free change is — at 25,591 expected games (DEC-143), a night per removal.** |

**A fourth observation about the method in the record, which is the
trap-avoidance machinery itself.** 215 of the runs are simplification bounds:
about a quarter of that testing budget goes on deleting things. chesso's plan
has exactly one step whose goal admits deletion as a valid outcome — S022,
"deleting delta pruning is a valid recorded outcome" — plus S134, which is a
bit-exact fold and not a strength question. Against that sits an accumulating
stock of rules that have never been re-measured since the search around them
changed.

---

## 5. Where the record's numbers disagree with chesso's own measurements

DEC-019 says a published figure decides what to try and never what to conclude.
Three of the disagreements are sharp enough to be worth writing down, because
each one has a candidate explanation that is itself testable.

| chesso measured | the record measured | Candidate explanation |
|---|---|---|
| **Staged move generation: 0** (S006, DEC-004 calls it "the largest wrong estimate in the project") | **Fully staged move picker: +29.79** (1356 games) | The staging gain there is not the movegen saving. ~~It is that a fully staged picker **defers losing captures behind the quiets**~~ **Struck 2026-09-19 (review F04): the SEE split of noisy moves had existed for a month when the staged picker landed, so the deferral is not what it bought, and its message says nothing.** The candidate that survives is that a staged picker never generates quiets at a node that cuts off on the TT move or a good capture. chesso is staged but does not defer bad captures (S025, reserve). The saving chesso measured and whatever the record measured are different changes wearing one name. |
| **SEE pruning in quiescence: 0** (S015) | **+5.08** (11424 games) | The threshold in the record is not zero and not fixed: it moves with the distance from alpha, the corrplexity, and the move's own history. chesso's is a fixed threshold. A zero on the fixed form does not price the adaptive form. |
| **Bad-capture ordering: slower, three ways** (S025, DEC-004) | **Skip bad noisy in qsearch: +37.65**; **split noisy by SEE: +14.08** | ~~Both were measured *without capture history existing*~~ **Corrected 2026-09-19 (review F04): both were measured *with* capture history present — noisy history replaced LVA on 2025-02-22, the SEE split landed on 2025-02-23 and the qsearch skip on 2025-02-25** — and with a SEE call inside the picker's own loop rather than a pre-pass. That is direct support for S025's gate on S023 (capture history first), not a disagreement to explain. The +37.65 rule was itself removed as free on 2026-01-30, +0.52 ± 1.51 over 51,834 STC and −0.00 ± 1.16 over 77,828 LTC. |

A fourth, smaller one worth flagging: chesso's **S130 measured the TT score as
quiescence stand-pat at +1.14 ± 4.04 over 16784 games — no verdict, recorded as
zero and kept (DEC-103)**, where Weiss measured +10.78/+12.09 (literature
check A12). The record has the same mechanism and a **generalised** version of it
— N1, the same tightening applied at *every* node's margin tests, not only at
the quiescence stand-pat — measured at +6.07. So chesso's zero on the narrow
form does not close the wide one.

---

## 6. What this recommends, in priority order

Nothing below changes the plan. It is a proposal to the owner, and every item
would be a step with its own pre-registration and its own SPRT.

### Tier 1 — large effect, cheap verdict, and the plan already owns the step

These are re-orderings, not new work.

1. **Promote the correction-history family out of reserve.** S099 → S110 →
   S111, plus a new step for "correction applied in quiescence" (+13.85 in the
   record) and one for corrplexity as a margin and reduction input (+9.17,
   +3.77). Six verdicts, every one of them in the 3,000–8,300-game class,
   ~~which is 1.5 to 4 hours each at chesso's throughput. This is the highest
   measured Elo per machine-hour anywhere in this report.~~ **Struck 2026-09-19
   (review F01, F02): a chesso verdict costs what DEC-143 says, 12 to 20 hours
   worst case, whatever the effect measured elsewhere; and the two best
   Elo-per-game rows in this whole ledger are time management's, S132.** The
   plan's stated reason for holding them back — a band claim of "only above
   ~3100" — is already recorded as `NOT FOUND` by the 2026-09-04 literature
   check, and the record does not settle the band either way (§3.1). ~~What it
   adds is the cost: **the cheapest verdicts available are sitting at positions
   40 to 44.**~~ **Corrected 2026-09-19 (review F02): the placement's operative
   reason is DEC-133's — a technique whose only evidence is above the band goes
   behind steps with sub-3000 records — and DEC-176 (c) confirmed it on
   2026-09-11 after S181's re-banding. A network engine rated far above
   chesso's band is more above-band evidence. What DEC-133 already permits is
   the path taken: S099 runs as the probe on a spare night, after S232 reseeds
   it (DEC-222).**
2. **Keep S097 where it is.** Singular extensions at +10.40 STC / +26.99 at
   20+0.20, with multi-cut at +14.17 falling out of the same verification
   search, is the best-priced item in the open order and it is already at
   position 3.
3. **Re-price S188 (check extensions) before running it.** Three engines have
   measured this at or below zero on removal. ~~If it runs, it should run as a
   `{-5, 0}` non-regression against a stated prior of zero, not as a gainer.~~
   **Corrected 2026-09-19 (review F11): a `{-5, 0}` pair accepts H1 on a truth
   of zero and would ship the extension after the full walk; S188 keeps `{0, 5}`
   with the prior written down (DEC-222). And the third datum does not state
   its form: the removal message in the record names neither, and the 2024
   history there moved the extension inside the loop and back out within a
   fortnight.**

### Tier 2 — new steps, cheap verdicts, no plan step exists

Ranked by the record's STC Elo. Each is one step.

4. **N1, static eval tightened by the TT score at every margin site** (+6.07,
   8414 games). Small, local, and it subsumes S130's narrow form.
5. **N2, fail-middle returns** (+9.71 for the first site, 4654 games). One
   interpolation weight, fitted here; four further sites tested one at a time
   afterwards.
6. **N5, hindsight reductions** (+6.03, 8590 games). Needs the parent's
   reduction in the search stack, which S098 has already made a first-class
   quantity.
7. **N4, cutoff count** (+6.00, 8688 games).
8. **N8, bad-noisy futility pruning** (+5.30, 11082 games). Sits beside S109's
   block -- and after S023 and S025: chesso has no bad-noisy picker stage for
   it to fire in (review F17).
9. **N7, threat dimensions in the quiet history index** (+8.81 at 20+0.20).
   Two bits on an existing table; the threat bitboard has to be computed, which
   chesso does not currently do and which S101 would want anyway.
10. **The history-update family as one block: N10 prior-countermove bonus on a
    fail-low, N11 update on a TT cutoff, N12 bonuses by node type, N9 decaying
    malus** (+3.87, +3.51, +4.35, +4.67 separately). Under DEC-082's
    block-form reasoning these are four verdicts near +4 that crawl at the
    bounds individually and resolve as one block.
    **Corrected 2026-09-19 (review F12): DEC-082 requires parts that are inert
    in isolation; these four measured positive separately, so they are four
    candidates, or a block whose `accepts` names the forfeited attribution.**

### Tier 3 — architecture, worth a step of its own

11. **Move the LMR reduction to fixed point** (§3.3). Not a refactor: it alters
    play and takes an SPRT. It is the one candidate explanation for S098
    verdict 1's recorded −2.92 that can be tested without also changing the
    history sum, and if it is right it unlocks every later term that wants to
    contribute a fraction of a ply — which is most of §3.2.
12. **N6, reductions on the non-LMR path**, once 11 exists.

### Tier 4 — methodology

13. **Schedule SPSA as a recurring night, not as S127.** Fourteen sessions,
    ≈+73 Elo, one every six weeks. On chesso's single machine the cadence
    would be slower, but "once, at position 37" is not what the evidence
    supports.
14. **Budget a simplification lane.** A quarter of the record's budget goes to
    removing things that stopped paying, and four of chesso's shipped rules
    are candidates on the record's own removal register: the killer slots, the
    countermove table, check extensions if S188 ever lands, and whatever of
    S109's block the later pruning work makes redundant.
15. **Take an LTC reading on the items where the record's STC and LTC disagree.**
    The split is informative: correction history, singular extensions and
    threat-indexed history all read higher at the longer control, and DEC-202
    already gives chesso a block-boundary longer-control reading to hang this
    on.

### Explicitly not recommended

- Anything NNUE — DEC-054 holds.
- Anything SMP, NUMA, MultiPV or SIMD — out of the arena, out of the goal.
- Reduction jitter (N22) — it measured −0.10 at single-thread STC over 111828
  games and only paid under SMP. It is an SMP feature wearing a search-quality
  hat.
- Laterality and critical chain (N21) — real, but they are 2026 refinements on
  a reduction formula that is already fixed-point and already carries a dozen
  terms. They have nothing to attach to here yet.
- Low-depth singular extensions (N16) — **failed at STC** and shipped on a
  157,644-game LTC run. That is 70 hours of this machine for one verdict.
- Negative extensions (N17, +1.85 over 70894) and material scaling (N19,
  +2.52 over 31680) — right ideas, wrong price: 32 to 52 hours each here.

---

## 7. Decisions this study proposes

Proposed, not taken. Agents propose; decisions belong to the owner.

1. **The open-source record is read for technique only.** No source, no table
   and no constant from it enters this repository, and any technique taken from
   this report is implemented from first principles or from its published
   description, with its constants fitted here. (§0)
   **Taken 2026-09-19 (DEC-221).**
2. **The correction-history family leaves the reserve** and is ordered ahead of
   the evaluation block, on the strength of two independent sources against the
   band claim that put it there. (§3.1, §6 item 1)
   **Not taken (review F02): the placement's reason is DEC-133's and DEC-176 (c)
   confirmed it; S099 runs as DEC-133's probe on the next idle night after S232's
   reseed, the reserve otherwise untouched (DEC-222).**
3. **S188 is re-priced against three engines' removal results** before it is
   run. (§4, §6 item 3)
   **Taken 2026-09-19 as: pair unchanged at `{0, 5}`, prior recorded in the
   pre-registration (DEC-222, review F11).**
4. **SPSA becomes a recurring scheduled night** rather than one step. (§3.5,
   §6 item 13)
   **Taken 2026-09-19 as a per-block cadence in chesso's own lane size, verified
   by an independent `{0, 5}` SPRT at 8+0.08 (DEC-222, review F07).**

---

## Appendix A — the rebuild ladder, 2025-02-21 to 2025-04-05

Every feature commit of the record's 2025 search rewrite, in the order it
landed, with the Elo it reported at the control stated. This is the closest
thing in public to a controlled ordering of what each classical search
technique is worth when added one at a time to a bare search. Read it as
**ordering and as a cost estimate**, never as a prediction of a chesso verdict
(DEC-019). All figures 8.0+0.08 unless noted.

| Elo | Technique |
|---|---|
| 615.41 | MVV-LVA ordering of captures |
| 204.48 | TT move ordered first |
| 127.73 | Late move reductions |
| 72.88 | Reverse futility pruning |
| 69.70 | TT cutoffs restricted to non-PV nodes |
| 58.23 | TT cutoffs |
| 38.88 | Preserve the existing TT move on a write that has none |
| 37.65 | Skip losing captures in quiescence |
| 31.22 | Do not drop into quiescence while in check |
| 29.79 | Fully staged move picker |
| 24.17 | Null move pruning |
| 24.01 | Maximum time scaled by move number |
| 23.47 | Quiet butterfly history |
| 18.16 | SEE pruning |
| 17.59 | Time management by the best move's node share |
| 17.00 | Minor and major correction histories *(both later removed as free)* |
| 14.17 | Multi-cut |
| 14.08 | Captures split good/bad by SEE |
| 13.85 | Correction history used in quiescence |
| 13.74 | Late move pruning |
| 11.81 | Non-pawn correction histories |
| 11.51 | Principal variation search |
| 10.91 | Reduce less for checking moves |
| 10.40 | Singular extensions *(+26.99 at 20+0.20)* |
| 9.71 | Fail-middle in reverse futility |
| 9.41 | Double extensions *(at 12+0.12)* |
| 9.19 | Null move only at cut nodes *(at 6+0.06)* |
| 9.18 | Pawn correction history |
| 9.17 | Corrplexity in LMR |
| 9.09 | LMR adjusted by history |
| 9.09 | Aspiration windows |
| 8.94 | Razoring |
| 8.63 | Dedicated noisy move generator |
| 7.76 | A move list type |
| 7.70 | TT-move stage in the picker |
| 7.68 | Static eval condition in null move |
| 7.55 | Futility pruning |
| 7.54 | TT used in quiescence |
| 7.40 | Reduce cut nodes more |
| 7.22 | Modernised ProbCut |
| 6.93 | 1-ply continuation history |
| 6.90 | Last-move correction history |
| 6.74 | Larger base null-move reduction |
| 6.66 | History bonus formula |
| 6.15 | Time management by eval stability |
| 6.15 | Static eval as the correction-history target |
| 6.07 | Static eval tightened by TT information |
| 6.03 | Hindsight LMR decrease |
| 6.03 | Re-search depth from the reduced search's result |
| 6.00 | Cutoff count |
| 5.97 | Noisy history replacing LVA |
| 5.95 | Reduce more when not improving |
| 5.63 | Futility on quiet moves only |
| 5.52 | Late move pruning in quiescence |
| 5.34 | Larger null-move reduction when the TT move is a capture |
| 5.26 | Reduce PV nodes less on a wider window |
| 5.24 | Optimal time scaled by move number |
| 5.12 | 2-ply continuation history |
| 5.11 | Internal iterative reduction |
| 5.02 | Improving flag in reverse futility |
| 4.89 | Aspiration fix |
| 4.80 | Average score as the aspiration centre |
| 4.74 | Guard quiescence pruning on recaptures |
| 4.64 | Root depth reduced on aspiration fail-highs |
| 4.62 | Triple extensions |
| 4.59 | Aspiration delta from the previous score |
| 4.54 | ProbCut |
| 4.51 | One reduction call per node |
| 4.47 | Correction-history key fix |
| 4.27 | Easier reverse futility at expected cut nodes |
| 4.18 | Improving flag in late move pruning |
| 3.96 | SEE threshold from the move's ordering score |
| 3.87 | Prior-countermove history updates |
| 3.75 | Time management by PV stability |
| 3.67 | Post-LMR continuation-history updates |
| 3.64 | Static evaluation history |
| 3.61 | TT replacement policy |
| 3.52 | LMR refined by cutoff count |
| 3.48 | Fail-soft multi-cut |
| 3.46 | Upcoming-repetition detection |
| 3.40 | Less reverse futility at TT-PV nodes |
| 3.32 | Factorized noisy history with threats *(later removed as free)* |
| 3.13 | SEE threshold by move history |
| 3.05 | Factorized quiet history *(later removed as free)* |
| 2.86 | Mix MVV with noisy history |
| 2.74 | Correction updates inside the singular search |
| 2.58 | LMR allowed to reduce into quiescence |
| 2.52 | Material scaling in place of game phase |
| 2.33 | Check extensions *(removed at −0.26 a month later)* |
| 2.26 | TT prefetch |
| 2.15 | Futility pruning in quiescence |
| 2.14 | Killer moves *(removed at +0.50 three months later)* |
| 1.98 | Skip history updates when there was one quiet move |
| 1.92 | Always penalise bad captures |
| 1.85 | Negative extensions |
| 1.83 | Limit evasions in quiescence |
| 1.45 | LMR for some noisy moves |
| 1.43 | Extend all moves at low depth when the extension is large |

---

## Appendix B — how to re-derive every number here

```
git -C <repository> log --format='%H%x01%ad%x01%s%x01%b%x02' --date=short
```

Each record's body holds `Elo | <x> +- <y> (95%)`, `SPRT | <tc>s Threads=<n>
Hash=<n>MB`, `LLR | <l> (<a>, <b>) [<lower>, <upper>]`, `Games | N: <n> ...`
and `Penta | [...]`. The parse used here kept, per commit, every `Elo` figure,
every time control and every game count in document order, and paired them
positionally where the counts matched. The bucket table of §2.1 uses only runs
whose time control parsed as `8.0+0.08`. The parse is
`2026-09-19_technique_ledger.py` beside this file; run it as
`python3 adocs/data/2026-09-19_technique_ledger.py <repository>`.


---

## Corrections after the 2026-09-19 adversarial review

Reviewed the day it was written: `adocs/audit/2026-09-19_study_review.md`, 17
findings, two reviewers (the coordinator re-deriving every figure, a cold
adversarial reviewer on the reasoning), the owner ruling item by item
(DEC-220, DEC-221, DEC-222). Corrections are made in place above — struck
through where a claim is withdrawn, marked "Corrected 2026-09-19" where a fact
is replaced — and listed here so the document reads the same from either end.
Dates and commit subjects identify each commit of the record;
`2026-09-19_technique_ledger.py` re-derives every aggregate.

- **F01, high — the cost rule is withdrawn.** §2.1's operating conclusion and
  every hour figure in §6 read the record's game counts as chesso's verdict
  cost. Games-to-verdict depends on the true effect here, which DEC-019 says
  does not transfer, and chesso's own record of transfers is 0, 0, wrong
  sign, 0.10, 0.33. Ranking by measured Elo stays as a guide to which effects
  were large there; the price is DEC-143's.
- **F02, high — §6 item 1 and §7 item 2 misread the reserve.** DEC-133's
  reason is above-band-only evidence, DEC-176 (c) confirmed it on 2026-09-11,
  and a network engine rated far above chesso's band is more of the same. Not
  taken; S099 runs as DEC-133's probe after S232's reseed. Time management
  (S132), not the correction family, has the best Elo-per-game rows here, and
  moves up.
- **F03, medium — provenance.** §1 now says the rows of §3.2 are prose
  descriptions taken from open-source resources, and that an implementer works
  from the description (DEC-221).
- **F04, medium — §5 chronology.** Capture history existed before the SEE
  split and the qsearch skip; the deferral cannot explain the staged picker's
  gain; the +37.65 rule was removed as free in 2026. Both rows corrected.
- **F05, medium — §4 omitted the IIR replacement.** The §3.1 row now carries
  it; S095 is re-formed (DEC-222).
- **F06, medium — S099's seeds** quote Stockfish commit prose, against
  DEC-134; S232 reseeds S099, S110 and S111 before any of them runs.
- **F07, medium — §3.5's SPSA total** sums LTC passes, counts six SMP
  sessions, and each session is 13 to 91 hours here. Corrected in place; the
  cadence adopted (DEC-222) is in chesso's lane size, verified at 8+0.08.
- **F08, medium — "stops fast" (§4, Tier 4 item 14)** is false in both
  records; corrected in place with the medians. Removals are folded into the
  landing step that makes them redundant, a night each (DEC-222).
- **F09, medium — later simplifications** on N5, N8, N10, N11 and quiescence
  pruning are not listed in §4; the step files S234 to S238 build the record's
  current form.
- **F10, medium — line 93 and the headline game count** corrected; the
  parser committed.
- **F11, medium — S188's pair** in Tier 1 item 3 corrected; the third datum's
  form is unknown; `{0, 5}` stays with the prior recorded (DEC-222).
- **F12, medium — DEC-082** does not reach Tier 2 item 10; corrected in place.
- **F13, low — Tier 3 item 11** has no price in the ledger (the fractional LMR
  there predates it); it is S236, a hypothesis test in one block.
- **F14, low — four phrasings** that hinted at scales ("thousandths of a
  ply", "the twentieth", "a couple of moves", "0, 1 or 2 plies") are reworded
  to shape only.
- **F15, low — three mislabelled controls** (§3.1 NMP row, N3, §4 move-count
  row) corrected.
- **F16, low — the killers row** now carries the two intermediate commits.
- **F17, low — N8** waits for S023 and S025; said at Tier 2 item 8.
