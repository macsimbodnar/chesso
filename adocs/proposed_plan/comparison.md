# Adversarial comparison: the current plan against the proposed plan

Date 2026-10-07. Coordinator agent (Claude Opus 5.5), at the owner's request.
**A recommendation. It amends nothing**: `adocs/plan.md`, `plan_todo/`,
`status.md`, `specs.md` and `decisions.md` are untouched until the owner
rules. "Current" is `adocs/plan.md` at `e98419b` with its 29 pending step
files. "Proposed" is `adocs/proposed_plan/` (P01 to P29) from the same commit.

Goal both are judged against: 3000 CCRL Blitz 1CPU without a network
(DEC-071, DEC-179), under COPYING, MEASUREMENT and the other project rules.

**Machine, per the owner today: the Linux workstation** (i7-8700K, 6 cores /
12 threads, DEC-049; 2110 games/h at 8+0.08 on `noob_3moves.epd`, the ledger's
figure; `rating.sh` ran there for S240). Consequences: the M1's four-to-five
hour failure (proposed O2) is moot; BMI2 is fast there, so the PEXT step is
measurable; Linux huge pages are available to the table step.

## 1. Verdict

**Merge, on the current plan as the base.** Keep its step files, ids,
enrichment and the owner's rulings behind them. Import seven things from the
proposal, reject six of its search steps as unsupported or already ruled on,
and fix four defects of the current plan that the proposal exposed.

Neither plan should be adopted as written:

- The current plan is **stale and mis-ordered at the head**: it does not know
  the engine is 2766, it fits new terms under a clamp it removes later, it
  leaves an SPSA lane the owner already ordered unlisted, and its mechanism for
  running the correction-history probe never fires.
- The proposal **reopens owner decisions without citing them**, drops open
  items silently, and front-loads six search refinements of exactly the kind
  its own §1.3 shows measuring zero here.

## 2. Findings against the current plan

| # | finding | evidence |
|---|---|---|
| C1 | **Stale baseline.** `plan.md` never mentions S240 or 2766. Its distance ("about 440 Elo"), its arithmetic (lands 2658, or 2707 to 2817) and its discounts are computed from S088's 2559. S240 measured 2766 against a forecast of 2600 to 2720: the plan's pessimistic arithmetic was wrong once, by 46 to 166 | `grep -c 2766 adocs/plan.md` = 0; S240 stamp |
| C2 | **New terms fitted under a clamp removed later.** Mobility plus king safety is clamped to ±184. Mobility curves (S121), passers (S123), pawn structure (S125) and threats (S101) are fitted before the clamp is re-decided (S039, Open #15). Mobility is inside the clamped sum, so its fit models a truncation that is about to go | Open list order; `specs.md` clamp paragraph |
| C3 | **The clamp's speed case is gone.** S120 (2026-10-06) measured exact evaluation everywhere at −5.0 % nps for a 7.5 % smaller tree, **2.6 % less wall time to depth**. S039's task, "re-decide the margin from measured spread", is overtaken: the question is now retire or keep, and the data says retire | S120 stamp, DEC-257, status 2026-10-06 |
| C4 | **An owed SPSA lane is not listed.** DEC-222 (8): "one lane per completed block over the axes that block added". The search block is complete. At least eight parameters were parked "for S127 to fit" (razoring margin, fail-low pull, ProbCut, no-TT-move reduction, delta phase, others). S127 is Open #21, behind about twenty evaluation verdicts, each measured on a search running on seeds | DEC-222; `status.md` lines naming "for S127" |
| C5 | **The correction-history probe never runs.** DEC-222 (1), the owner's ruling: S099 "runs as the probe ... on the next idle night". The reading rule keeps the machine always busy, so no night is idle. S099 is fully enriched and reseeded (S232, 424 lines) and has waited 18 days | DEC-222 (1); Open #24 "reserve head" |
| C6 | **No tuner step, ten term steps.** Every evaluation step re-implements its term by hand in `tools/eval_model.hpp` (1097 lines, a second evaluation beside `src/evaluation.cpp`'s 1244). `tools/tuner_model.hpp`'s header records three occasions when a term's gradient block was forgotten and the fit silently returned its input. King safety's quadratic finaliser (S122) and endgame scaling (S124) need gradients through a transform, also by hand | `tools/tuner_model.hpp` header; S121, S122, S101, S102 `touches:` |
| C7 | **Low-value head.** PEXT (S032) is priced by its own file at about 1 % of perft; the 16-bit move (S030) is unpriced. Both sit ahead of every Elo-bearing evaluation step | S032 file |
| C8 | **Two corpus verdicts where one serves.** Leaf labels (S082) and corpus size (S083) each regenerate, refit and SPRT. The recipe can be chosen offline by held-out error; one SPRT then measures the corpus that ships | S082, S083 `accepts:` |
| C9 | **No anchor can show 3000, and no step fixes it.** S240: Leorik 2.4 (2830) still scores 54.7 % against chesso; "the next read wants an anchor above ~2850". The near-goal rating (S152) has no step that installs one. No re-plan trigger exists between now and S152 | S240 stamp; S152 file |

## 3. Findings against the proposed plan

| # | finding | evidence |
|---|---|---|
| X1 | **Reopens owner rulings without citing them.** DEC-138 (owner, 2026-09-04) recorded double and negative extensions, pawn history, threat and further correction tables, and complexity scaling as considered and given no step in phase one. P09, P11 and P25 reopen them; §7 "Deliberately not in this plan" omits DEC-138. DEC-192 says complexity becomes a step "when a figure appears or the owner says so"; none has | DEC-138, DEC-192; proposed `plan.md` §7 |
| X2 | **Capture history up front against the sourced evidence.** S023 sits in reserve on Weiss PR #428: **−4.17 ± 4.83 at short control**, +3.66 at long (DEC-176 a). This harness runs 8+0.08, short control. P07 v1 cites only Stash's +4.6 and does not engage the negative | S023 file; DEC-176 |
| X3 | **Losing captures after quiets was measured slower here three ways** (+17.6 %, +13 %, +3 % time to depth, DEC-022). P07 v2 notes it but offers no change to the cost, which S025 names as the only thing that would make it pay | S025 file |
| X4 | **Contradicts its own thesis.** §1.3: of 23 verdicts since 2026-09-20, 14 are H0 or no verdict; "refining existing search rules is close to harvested". Then P09 to P13 add five refinements, four with no figure (P09, P11, P12, P13) and one at +4.9 (P10). The singular extension they build on (P11) measured −0.81 here; the check extension −13.84 | ledger; S097 v1, S188 |
| X5 | **Silent drops.** Not mentioned anywhere: the multicut/ProbCut mutant rows (S259, an open finding every pre-registration must name under DEC-171), the degenerate-column fold (S134, which any tuner needs: R² = 1.000000), PEXT (S032), the 16-bit move (S030), the own Syzygy prober (S129, replaced by owner question O1) | Open list against `steps/` |
| X6 | **Budget understates the slow class.** 30 verdicts at the ledger mean of 6 h 19 m. The six search refinements are small or unpriced effects, which the ledger's own rule puts in the slow class at 8 h 52 m | `plan.md` "Priced by class" |
| X7 | **Two lanes conflict with the reading rule.** `plan.md`: while a run plays, "a change to `src/` waits". P01, P02 and P15 touch `src/`. The lane model needs a rule amendment, not an assumption | `plan.md` "How to read the list with one machine" |
| X8 | **Pawn cache before the terms that fill it.** P15 builds it before P19 and P20 and ships it switched off if it pays nothing. DEC-046 measured a pawn hash at zero when the pawn stage was small; DEC-087 moved S118 after the pawn terms for exactly that reason | DEC-046, DEC-087 |
| X9 | **King safety first in the evaluation block.** Stash's king-safety rewrite came as three changes, +4.2, +9.7 and +10.9 (+24.8 together); mobility area (+20.0), passer king proximity (+22.3) and connected pawns (+25.4) each came as one. The current plan orders by that ledger; the proposal does not, and king safety is the documented failure magnet | Stash changelog as quoted in both plans |
| X10 | **Thin step files.** 14 to 48 lines each, against 54 to 424 (most over 150) for the current files they overlap (sources, seeds in DEC-134 form, tests, priced pairs). Adopting P files wholesale re-pays the 2026-09-05 enrichment | `wc -l` |

What the proposal gets right and the current plan lacks: the trace tuner
(C6), early clamp retirement (C3), a scheduled correction-history probe (C5),
an SPSA lane now (C4), one corpus verdict with a fixed validation set (C8),
anchors above 3000 (C9), re-plan triggers (C9), and two technical catches in
its table step: the table uses only 75 % of `Hash` (`bit_floor` of 24-byte
entries) and partial keys make every unverified reader of the stored move a
false-hit risk.

## 4. Disposition, item by item

| item | disposition | why |
|---|---|---|
| S119 table clusters (≈ P08) | **keep first**; add P08's legality check for every reader of a stored move, "keep the move on a moveless store", and state that 32- or 64-byte clusters fill `Hash` | both plans agree; P08 adds two real checks |
| S259 mutant rows | keep, agent lane during S119's match | open finding, DEC-171 |
| P01 pawn and non-pawn keys | **adopt**, neutral, before S099 | S099, S110, S118 all need them; measured alone so the key's nps cost is not hidden in a verdict |
| P16 retire the clamp | **adopt, replaces S039**, right after S119 | C2, C3 |
| S099 pawn correction history (≈ P05) | **promote to the machine lane now** | executes DEC-222 (1); no reversal |
| S110, S111 non-pawn and continuation correction (≈ P06) | keep, **gated on S099's H1** | DEC-222 (1); same gate as the proposal |
| P12 correction magnitude in reductions | conditional step **on S099's H1** | DEC-222 (1) already promises it a step then |
| SPSA lane, search block (≈ P14) | **adopt now**, sized as DEC-222 says (the block's axes, 8 to 9 h), verified by `{0, 5}` | C4 |
| P02 trace tuner | **adopt**, agent lane, starts at once | C6; behaviour-neutral, own code, no dependency |
| P04 anchors 2850 to 3150 | **adopt**, agent lane; the agent builds them (DEC-069) | C9 |
| P15 shared attack sets | **adopt the attack-set half** (neutral); the pawn cache stays S118, after the pawn terms | X8 |
| S134 fold degenerate columns | keep, agent lane, before the first fit | X5 |
| S082 + S083 corpus (≈ P03) | **merge into one step**: per-game sampling, resolved-leaf labels, a fixed validation set from independent games, recipe chosen offline by held-out error, one night, one SPRT | C8 |
| S121 mobility, S123 passers, S125 pawn structure, S122 king safety, S101 threats | keep, **in that order** (Stash's ledger), now fitted with the trace tuner and no clamp; fold in P18's pin rule, P19's tablebase-labelled sign check, P21's pawn-push threats as pre-registered legs | X9 |
| S118 pawn hash | keep, after the pawn terms | DEC-046, DEC-087 |
| S135 placement, S136 tempo | keep, moved **after** the big terms | single-digit effects, slow class |
| S124 endgame scaling | keep; P24's mop-up as a second pre-registered leg | |
| S102 outposts and space | keep | |
| S133 king-relative tables | keep (already owner-approved, DEC-087) | |
| S126 full refit | keep | |
| S032 PEXT, S030 16-bit move | **move to after S126** | C7; timings only, need an idle machine |
| S127 final SPSA | keep, last run of the cadence | |
| S129 Syzygy | keep, last, optional; route is the owner's (see §8) | |
| S152 rating | keep, closes the order, **on P04's anchors** | |
| S023, S025, P07 capture history and losing captures | **reserve** | X2, X3 |
| P10 captures in late move reduction | reserve, **gated on S023's H1** | built on capture history |
| P09 threat, pawn, low-ply histories; P11 double/negative extensions | **stay under DEC-138** | X1, X4 |
| P13 fifty-move damping | reserve; reach census first if ever opened | no figure |
| P25 complexity | **owner question** at T2 | DEC-192 |
| S029 network | parked, DEC-054 | |

## 5. The merged order

M = machine lane (holds the workstation). A = agent lane (no run; `src/`
work happens in a separate worktree while a match plays and **lands between
verdicts**, timed then; needs the rule amendment in §9).

| # | lane | step | verdicts |
|---|---|---|---|
| 1 | M | S119 table clusters, aging, prefetch, huge pages, full `Hash` | 1 |
| | A | S259 mutant rows; P01 keys; P04 anchors (smoke matches between runs) | 0 |
| 2 | M | P16 retire the clamp, `{-5, 5}` | 1 |
| 3 | M | S099 pawn correction history | 1 |
| | A | P02 trace tuner (long); S134 fold | 0 |
| 4 | M | S110 non-pawn correction, if S099 H1 | 0–1 |
| 5 | M | S111 continuation correction, if S099 H1 | 0–1 |
| 6 | M | P12 correction in reductions, if S099 H1 | 0–1 |
| 7 | M | SPSA lane over the search block's axes, then `{0, 5}`; block-boundary drift point and 32+0.32 reading | lane + 1 |
| | A | P15 shared attack sets | 0 |
| 8 | M | corpus (S082 + S083 merged): recipe offline, one night, refit SPRT | night + 1 |
| 9 | M | S121 mobility | 1–2 |
| 10 | M | S123 passed pawns | 2–3 |
| 11 | M | S125 pawn structure | 2 |
| 12 | M | S122 king safety, nonlinear | 1 |
| 13 | M | S101 threats | 1 |
| 14 | M | S118 pawn hash | 0–1 |
| 15 | M | S135 placement, S136 tempo | 2 |
| 16 | M | S124 endgame scaling (+ mop-up leg) | 2 |
| 17 | M | S102 outposts, space | 2 |
| 18 | M | S133 king-relative tables | 1 |
| 19 | M | S126 full refit on fresh self-play | night + 1 |
| 20 | M | S032 PEXT, S030 16-bit move | timings |
| 21 | M | S127 final SPSA | lane + 1 |
| 22 | M | S129 Syzygy, if the owner picks a route | 0–1 |
| 23 | M | S152 rating against P04's anchors | rating run |

## 6. Budget on the workstation

At the ledger's figures (mean 6 h 19 m a verdict, fast class 2 h 02 m, slow
class 8 h 52 m, 2110 games/h):

| block | verdicts | machine hours |
|---|---|---|
| #1–#7 search mechanisms and SPSA | 4–7 + one lane | 30–75 |
| #8–#19 corpus and evaluation | 16–19 + two nights | 130–200 |
| #20–#23 speed, final SPSA, Syzygy, rating | 1–2 + one lane + timings + rating | 25–50 |
| **total** | **about 21–28** | **about 185–325** |

Against the proposal's "about 270" and the current plan's "275 to 351, plus
nights and gauntlets". At 18 h a day: **10 to 18 machine-days**, four to six
weeks of calendar with reverts and owner gates. Dropping P07 and P09 to P13
saves about nine slow-class verdicts, **about 80 machine-hours**. The only
published figures behind those items sum to about +27, and +17.9 of that is
Stash's bundle with staged generation, which measured 0 here.

**Elo, an ordering aid only (DEC-019).** On the proposal's bands minus the
dropped items, +190 to +360 self-play; at its 0.83 factor (one observation)
2925 to 3065. The current plan's 0.38 discount under-forecast S240; the 0.83
factor is one point. Neither is a forecast. **3000 needs most blocks near the
middle of their bands**, which is why the triggers below exist.

## 7. Re-plan triggers

`Σ` = sum of H1 point estimates since S240. Projection = `2766 + 0.83 · Σ`, an
estimate, never written into `specs.md`.

| trigger | when | rule |
|---|---|---|
| T1 | after #9–#11 (first four or five evaluation verdicts) | their Σ < +25: stop; profile evaluation cost against gain before adding terms |
| T2 | after #19 (full refit) | projection ≥ 2970: proceed to #20–#23. Below 2900: re-plan with the owner (complexity, reserve search items, Syzygy, or the network's timing under DEC-179) |
| T3 | #23 | ≥ 3000 with the anchor-dispersion interval: the phase-two transition gets its decision (DEC-014); the 0.83 factor is re-measured |

## 8. Owner decisions this needs

1. **Adopt the merge** (or keep the current plan, or switch to the proposal).
2. **Retire the clamp** (P16) in place of re-deciding its margin (S039).
3. **Schedule the correction-history probe now** rather than "on an idle
   night". This executes DEC-222 (1); it reverses nothing.
4. **The lane rule**: agent-lane `src/` work in a separate worktree during a
   match, landing and timing between verdicts. Today's reading rule forbids
   starting it.
5. **Syzygy route**, unchanged as last and optional: own prober (S129 says
   "written from the format description"; the proposal says no independent
   description exists — checked at step start, not assumed), or Fathom (MIT,
   DEC-087's recorded route), which is a dependency under DEPS.
6. **Complexity term**: decided at T2, not now.

## 9. Adoption, on the owner's yes

1. One decision entry recording this merge, its rejections and the triggers.
2. New ids, one more than the highest in any plan directory: P01 keys, P02
   trace tuner, P04 anchors, P15 attack sets, P16 clamp retirement, the
   search-block SPSA lane; S039 retired into P16's step; S082 and S083 merged
   into one (the other id retired, never reused).
3. `plan.md`: a dated section for this merge, the Open list rewritten to §5,
   the distance and arithmetic restated from 2766, the triggers added.
   `status.md` regenerated. One commit, no `src/`.
4. If the workstation's harness changed since its last A/A, DEC-143 owes a
   1000-game A/A before the first verdict.
