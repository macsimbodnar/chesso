# Proposed plan: 3000 CCRL Blitz without a network

Date 2026-10-07. Proposal by the coordinator agent; **not adopted**. Evidence
and sources are in `report.md` beside this file; one file per step is in
`steps/`. Step ids are `P01`…`P29`, local to this proposal: real `S<nnn>` ids
are allocated at adoption (§8), never before, so no id is consumed by a plan
that may not be taken.

## 1. Goal and scope

**Goal:** chesso at or above **3000 on the CCRL Blitz single-CPU scale**
(DEC-071, DEC-089, DEC-179), shown by `rating.sh` against anchors that bracket
3000 (P04, P29).

**Out of scope:** NNUE and any network (owner: after 3000; DEC-054); parallel
search (DEC-175); book learning (DEC-085); Syzygy unless the owner chooses a
route (O1).

**Rules every step carries** (from `AGENTS.md` and `CLAUDE.md`, restated so a
brief can quote them):

| rule | how the steps honour it |
|---|---|
| COPYING, DEC-016/104/105/134/221 | each step names a **description** (paper, wiki, release notes, PR prose) and is implemented from it; no engine's source is opened; **seeds** are a publication's own example, a derivation over chesso's data run at step start, or the range midpoint. No constant in this folder comes from an engine |
| MEASUREMENT, INV-6, DEC-063/143 | play-altering → one SPRT, bounds by DEC-063, pre-registration with worst-case games and abort rule; neutral → `bench` and `search_bench` identity, no match |
| one change at a time | multi-part steps run **numbered verdicts** (S098's form), each against the previous verdict's tree |
| TESTS, DEC-141/142 | both builds green; search-touching steps: Debug self-play, `gate_extra.sh`, a guard test and a killed mutant per new pruning/reduction/extension rule; goldens re-derived by script |
| mate safety | every step that changes a pruning input keeps the mate suites green (`test_mate_breadth`, `test_mate_pv`, `test_mate_carry`) |
| CHESS, DEC-023 | test positions for evaluation terms are labelled by Stockfish or a tablebase through `python-chess`, never by the agent |
| DATA, DEC-235 | corpora and run outputs stay under `.tuning/` and the owner's archive; the tree takes readings |
| RUNS, DEC-155 | estimate first; four hours or more goes to the night |
| DEC-175 | every new search table states at its declaration whether it is per-thread or shared |

## 2. Shape: two lanes, three blocks

- **Machine lane (M):** verdicts, SPSA, datagen, rating, in the order below.
- **Agent lane (A):** tooling and behaviour-neutral infrastructure, done while
  the machine lane is blocked on a match (AGENTS: "a second subagent starts
  only while the first is blocked on something long").

**Why search components go first on the machine although the evaluation is
the bigger lever:** the evaluation block cannot start before its tooling
(P02 trace tuner, P03 datagen, P15 attack maps) exists, and the search
components are ready to implement now. Running them first keeps the machine
busy while the tooling is built, and the corpus P03 generates then comes from
the stronger engine.

## 3. The ordered list

| # | id | step | lane | verdicts | depends on |
|---|---|---|---|---|---|
| 1 | P01 | incremental pawn key and per-side non-pawn keys | A | neutral | — |
| 2 | P04 | rating anchors above 3000 | A | none | — |
| 3 | P05 | pawn correction history | M | 1 | P01 |
| 4 | P02 | trace-based evaluation tuner | A | neutral | — |
| 5 | P06 | non-pawn and continuation correction history | M | 2 | P05 |
| 6 | P07 | capture history, then losing captures after quiets | M | 2 | — |
| 7 | P15 | shared attack maps and a pawn cache | A | neutral | P01 |
| 8 | P08 | transposition table: buckets, ageing, the whole `Hash` | M | 1 + a reading | — |
| 9 | P09 | threat-indexed, pawn and low-ply history | M | 3 | P01 |
| 10 | P10 | late captures reduced by LMR, scaled by capture history | M | 1 | P07 v1 |
| 11 | P11 | double and negative singular extensions | M | 2 | — |
| 12 | P12 | correction magnitude as uncertainty in LMR | M | 1 | P05/P06 H1 |
| 13 | P13 | fifty-move damping of the static evaluation | M | 1 | — |
| 14 | P14 | SPSA over every search parameter | M | run + 1 | block 1 |
| 15 | P03 | datagen v3 and a refit on its corpus | A, then M | night + 1 | P02, P14 |
| 16 | P16 | retire the lazy-evaluation clamp | M | 1 | P02 |
| 17 | P17 | king safety with a fitted nonlinear transform | M | 1 | P02, P15, P16 |
| 18 | P18 | mobility area and per-count mobility | M | 1 | P15, P16 |
| 19 | P19 | passed pawns: king, path, rook, candidates | M | 1 | P15 |
| 20 | P20 | connected and weak pawns | M | 1 | P15 |
| 21 | P21 | threats | M | 1 | P15 |
| 22 | P22 | outposts and bishop pawns, then the four zero-weight terms | M | 2 | P15 |
| 23 | P23 | space | M | 1 | P15 |
| 24 | P24 | endgame scale factor and mop-up | M | 1 | P02 |
| 25 | P25 | complexity (winnability) adjustment | M | 1 | P02, P24 |
| 26 | P26 | joint refit on fresh self-play | M | night + 1 | block 2 |
| 27 | P27 | king-relative piece-square tables | M | 1 | P26, O4 |
| 28 | P28 | SPSA over every search parameter, second pass | M | run + 1 | P26 |
| 29 | P29 | rating against the extended anchors | M | rating run | P04, owner |

Block 1 is #3–#14 (search components). Block 2 is #15–#27 (evaluation).
Block 3 is #28–#29. DEC-202's 32+0.32 readings for every verdict that moves a
pruning or reduction parameter (P10, P11, P12, P14) are taken once, at the
block 1 boundary.

## 4. Budget

| item | count | machine hours |
|---|---|---|
| SPRT verdicts | 30 (28 steps' verdicts + 2 SPSA verifications) | about 190 at the ledger mean of 6 h 19 m |
| SPSA runs | 2, about sixty parameters each | about 40 (S085 took 8 h 21 m for twelve) |
| datagen | 2 | about 20 |
| DEC-202 longer-control readings | about 4 | about 16 |
| rating run | 1 | 3 to 5 |
| **total** | | **about 270** |

About fifteen machine-days at eighteen hours a day; four to six weeks of
calendar with reverts, fixes and owner gates. Fast-class verdicts (effect
outside the bounds) average 2 h; slow-class 9 h. Large-effect steps are
front-loaded where the dependencies allow it.

## 5. Milestones and re-plan triggers

`Σ` is the sum of H1 point estimates since S240, in self-play Elo. The CCRL
projection is `2766 + 0.83 · Σ`, an estimate and never a measurement
(`specs.md`: an estimate does not belong there).

| trigger | when | rule |
|---|---|---|
| T1 | after P05–P08 (six verdicts) | Σ < +25: stop and re-plan block 1 — the missing-components thesis is failing |
| T2 | after P14 | record Σ and projection; no action unless T1 fired |
| T3 | after P16–P19 (four evaluation verdicts) | Σ for those four < +25: stop; profile evaluation cost against gain before adding more terms |
| T4 | after P26 | projection ≥ 2970: the owner decides on P29. Projection < 2900: re-plan (options: P27, O1, a second evaluation pass, or the owner reopens the network's timing) |
| T5 | P29 | ≥ 3000 with the anchor-dispersion interval stated: the goal is met; the phase-two transition gets its decision (DEC-014) |

## 6. Owner decisions

| id | question | recommendation |
|---|---|---|
| O1 | Syzygy: none, Fathom (MIT) as an optional build dependency (DEC-087's recorded route), or an own prober | decide before P26; Fathom optional, off by default, measured by its own SPRT with tablebases present. An own prober cannot be written without reading someone's prober |
| O2 | which machine runs runs over four hours (this M1 has failed twice after four to five hours of full-core play) | the workstation for slow-class SPRTs, SPSA and datagen; the M1 for fast-class SPRTs and agent work |
| O3 | adoption: replace the current plan, or merge | merge by the diff in §8; replace only what the diff shows superseded |
| O4 | P27 adds about 768 parameters per king bucket; still hand-crafted and linear, but a step toward network-like features | open only if P26's corpus clears the rows-per-parameter floor P27 states, and on the owner's yes |
| O5 | a rating checkpoint after P14, against DEC-108's once-near-the-goal rule | no; DEC-234's exception was for a re-anchoring need that P04 meets differently |

## 7. Deliberately not in this plan

Each was measured or decided; reopening one needs a new reason, recorded.

| item | record |
|---|---|
| check extensions, both forms | S188 H0, DEC-230 |
| two-ply continuation history alone | S231 H0, DEC-224 |
| history-scaled LMR on butterfly history | S098 v1, two legs H0, DEC-213; reopen only if P09 changes the history signal, as its own step with that reason |
| hindsight reductions; cutoff count | S237 H0; S238 H0 |
| null move: eval-scaled reduction; `static_eval ≥ beta` gate | S114 v1 H0; S114 v2 no verdict, DEC-243/244 |
| history persistence across `go`; killer dedupe | S093 v2 H0, DEC-101; S149 H0, DEC-098 |
| RFP return blended toward beta | S235 H0 |
| mate distance pruning; quiet checks in QS | DEC-087, about zero |
| evaluation cache | S120, DEC-257 |
| NNUE, SMP, book learning | DEC-054, DEC-175, DEC-085 |

## 8. Adoption

1. The owner reads `report.md` and this file and answers O1–O5.
2. **Diff against the current plan** (agent-only, first task): for each P, the
   pending S step that covers it, if any, and what differs; each pending S that
   no P covers, with keep or retire. Recorded as one decision entry.
3. Each adopted P gets the next `S<nnn>` (one more than the highest across
   `plan_todo/`, `plan_current/`, `plan_done/`), its file moves into
   `plan_todo/` with the same fields, `plan.md`'s Open list is rewritten by the
   coordinator, `status.md` is regenerated, one commit.
4. Each brief carries its step's description (DEC-221) and seed form
   (DEC-134), and the step's stamp says so.
