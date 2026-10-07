# Road to 3000 without a network: research report

Date 2026-10-07. Written by the coordinator agent (Claude Opus 5.5) at the owner's
request. **A proposal. It amends nothing**: `adocs/plan.md`, `plan_todo/`,
`plan_current/`, `status.md`, `specs.md` and `decisions.md` are untouched.
`plan.md` in this folder is the proposed plan; `steps/` holds one file per
proposed step.

## 0. How this was produced

**Read:** `adocs/specs.md`; the titles of all 257 decisions and the text of
DEC-033, 036, 037, 044, 046, 054, 063, 081, 085, 087, 105, 134, 143, 155, 192,
221; every `plan_done/` file name and the stamps of S013, S021, S027, S028,
S033, S068, S076, S085, S089, S093, S104; the SPRT ledger regenerated from
`git log` by `tools/ledger.py`; `src/search.cpp`, `src/evaluation.cpp`,
`src/evaluation.hpp`, `src/transposition_table.cpp`, `src/search_params.hpp`'s
parameter list; `DEV_MANUAL.md`'s datagen, tuner and verdict-cost sections;
`.moltke.local.md`; the S240 rating record.

**Not read, on the owner's instruction (avoid bias):** `adocs/plan.md`,
`adocs/plan_todo/`, `adocs/plan_current/`, `adocs/status.md`. Also not read,
because they were inputs to the current plan: `adocs/eval_tuning_strategy.md`,
`adocs/data/2026-09-19_search_technique_study.md` and its ledger script.

**Leakage, stated:** `specs.md` and decision titles name some pending step ids
and their topics in passing (correction history, the TT row, the absent
evaluation terms, Syzygy). Nothing was taken from those steps' files, but the
overlap on obvious items is expected and is not copying. The first task at
adoption is a diff against the current plan (see `plan.md`, "Adoption").

**Measured today**, HEAD `32a8a68`, M1 MacBook, load average 2.4 on 8 cores,
`go movetime 3000`, one run each (a reading, not a benchmark):

| position | depth | nodes | nps |
|---|---|---|---|
| start position | 20 | 13079178 | 4.37 M |
| Italian, move 4 | 21 | 12746755 | 4.44 M |
| "kiwipete" | 22 | 10363002 | 4.62 M |
| rook ending (perft pos 3) | 31 | 20093346 | 7.05 M |
| queen's gambit, move 10 | 25 | 11902564 | 4.08 M |

## 1. Where chesso stands

### 1.1 Strength

- **2766 CCRL Blitz** (S240, 2026-09-27, five-anchor mean, interval no
  narrower than about ±60). **234 short of 3000.**
- **Self-play to gauntlet factor ≈ 0.83, one observation.** Between S088 and
  S240 the kept verdicts' point estimates sum to about +250 (the S240 record's
  own figure); the gauntlet moved +207. Point estimates of early-stopped SPRTs
  are biased upward, so the factor is soft. At that factor 234 CCRL points need
  roughly **+280 summed self-play Elo**.
- **The anchor set tops out at Leorik 2.4 (2830)**, which still outscores
  chesso. A 3000 claim needs anchors from about 2850 to 3200 (step P04).

### 1.2 What the engine has, from the code at `32a8a68`

| area | present |
|---|---|
| search frame | PVS, aspiration windows with fail-low pull, iterative deepening, node-fraction time manager (S132) |
| table | direct-mapped TT, 24-byte entries, generation, static eval stored, QS probes and stores |
| pruning | RFP (TT-tightened estimate), razoring at depth 1, null move (`base + depth/divisor`, no eval term), ProbCut, LMP, futility, history pruning, quiet and capture SEE pruning, multicut |
| reductions | log-based LMR in fixed point with node-type terms (cut, not improving, TT capture, PV, no TT move), deeper re-search on a clear fail-high, extra ply for SEE-losing moves |
| extensions | singular +1 (S097 v1 measured −0.81, kept as the multicut's carrier) |
| ordering | TT move, captures by MVV-LVA (**all captures, losing ones included, above killers**: `src/evaluation.cpp:1222` before `:1227`), 2 killers, countermove, butterfly history with gravity and malus, one-ply continuation history |
| quiescence | TT probe, stand pat with TT substitution, per-move futility, SEE filter, quiet queen promotions, evasions in check, dead-draw scoring |
| evaluation | material, tapered PSQT (6×64 mg/eg), passed pawn by rank (6), isolated/doubled/backward, mobility **linear** in raw attack count (4 weights per phase), king safety **linear** in 9 counts, 4 placement terms and tempo **at zero**; mobility + king safety **clamped to ±184** (`src/evaluation.cpp:1112`) |
| tuning | own Texel-style tuner (term-specific model in `tools/eval_model.hpp`), own datagen, own SPSA driver, Zobrist-deduplicated corpus, WDL/score blend |

**Absent:** correction history of any kind, capture history, pawn or threat
history, low-ply history, reduction of captures by LMR, double or negative
singular extensions, TT buckets/prefetch, a pawn key, a pawn hash, and every
evaluation term in §3.2 below.

**Hash is under-used by construction.** `tt_resize()` takes
`bit_floor(MB · 2^20 / 24)` entries (`src/transposition_table.cpp:22`):
`Hash=16` uses 12 MiB, `Hash=128` uses 96 MiB. A quarter of the table the
harness and the rating list grant is never touched.

### 1.3 What the record says paid, and what did not

The large verdicts were **new mechanisms or a fit**, not variants:

| verdict | Elo (point estimate) |
|---|---|
| S028 Texel fit of all constants | +188.7 |
| S033 reverse futility | +60.0 |
| S109 LMP + futility + history + quiet SEE pruning | +46.9 |
| S091 capture SEE pruning in main search | +46.5 |
| S089 time split | +45.4 |
| S021 aspiration windows | +35.1 |
| S098 v2 node-type LMR terms | +29.1 |
| S076 deduplicated corpus refit | +26.7 |
| S085 SPSA, 12 parameters | +21.0 |
| S065 refit | +21.1 |
| S027 three eval terms | +20.9, +17.3, +13.1 |
| S131, S132, S222, S093 v1 | +17.8, +16.3, +11.1, +10.7 |

**Since 2026-09-20 the verdicts are mostly zeros.** Of 23 (S231 to S055),
14 are H0 or no verdict: S231, S097 v1, S188, S236 (both), S235, S237, S238,
S112, S022 (both), S114 (both), S055. Of the nine H1s one re-reads S132 at
32+0.32; the others are S131 +17.8, S132 +16.3, and six between +4 and +8
(S095, S097 v2, S234, S113, S115, S116). Refining existing search rules is
close to harvested at this evaluation; what is left in search is **missing
components**.

### 1.4 Measured facts that bind any plan

1. **Evaluation-limited** (DEC-033, re-run DEC-035): 70 % of the cost of
   expensive moves survives a sixteen-fold search; 10.4 cp per doubling.
   DEC-081 later put the tree shape first and the search block that followed
   delivered (§1.3); the tree now reaches depth 20–25 in three seconds (§0).
   The evaluation is again the thin part.
2. **The clamp is a ceiling on what the evaluation can say.** Mobility plus
   king safety cannot exceed ±184. S120 (2026-10-06, clamp kept): paying the
   full evaluation everywhere instead of the lazy shortcut costs 5.0 % nps and
   **saves 2.6 % wall time** to depth, the tree being 7.5 % smaller.
3. **Evaluation cost is real.** Recomputed mobility once cost a third of the
   nps and measured −14.93 (DEC-036/037). A richer evaluation needs shared
   attack maps and a pawn cache first.
4. **A pawn hash recovered nothing in 2026-08** (DEC-046), when the pawn stage
   was four bitboard fills. It is re-measured when the pawn stage grows, not
   assumed either way.
5. **Machine time is the binding constraint.** 43 ledger runs: mean 6 h 19 m,
   fast class (effect outside bounds) 2 h 02 m, slow class 8 h 52 m;
   2110 games/h on the workstation. `.moltke.local.md` says this M1 does
   2700 games/h but has gone down twice after four to five hours of full-core
   match.

## 2. What top engines and the literature do

### 2.1 The arena

CCRL Blitz (`computerchess.org.uk/ccrl/404/`, read 2026-10-07): "equivalent to
2'+1" on an Intel i7-4770K", **up to 6-piece EGTB**, general book to 12 moves,
ponder off. Network-free engines at and above 3000 are many: on today's page
CuckooChess 1.13 (3071), Gaviota 1.0 (2986), Crafty 25.3 (2975) near it;
Stash v29 3137, v30 3166, v33 3286, v35 3358 (search summary; re-read on the
day); the single-CPU list `specs.md` recorded on 2026-08-18 has Stockfish 11 at
3565, Komodo 14.1 3482, Xiphos 0.6 3356, Ethereal 11.75 3346. **3000 without a
network is well-trodden ground.**

### 2.2 Today's top engines: what is search-side and therefore transferable

Top engines are NNUE. Their evaluation does not transfer here; their search
does, and since 2023 it has grown mainly in **learned corrections and
histories**:

- **Static evaluation correction history** (CPW): a table indexed by a board
  feature stores a running estimate of `search score − static eval` and
  corrects later static evals with the same feature. Introduced in Caissa,
  October 2023; Stockfish PR #4950 (2023-12-31, pawn-structure index). Index
  families now in use: pawn, material, non-pawn per side, minor, major,
  threats, continuation (previous two plies). Updated only out of check, when
  the best move is absent or quiet, and when the bound does not contradict the
  direction. CPW notes **larger gains at longer controls**. Direction only:
  Avalanche PR #75 (2026-10-01) reports +29.6 nElo at 8+0.08 and +47.7 at
  40+0.4 for the pawn table alone.
- **History family** (CPW History Heuristic): butterfly with gravity and malus;
  continuation histories at 1 and 2 plies most common, more plies in
  Stockfish; **capture history** indexed by moved piece, target square and
  captured type (Geschwentner, 2016) replacing MVV-LVA as the capture order;
  **low-ply history** for plies 0–3 (Stockfish PR #2557); pawn-structure
  history.
- **LMR** (CPW): log(depth)·log(moves) base; reduce less for history, PV,
  killers, checks, improving; more at cut nodes and when the TT move is a
  capture; **captures and promotions reduced on a separate, smaller base** in
  several engines.
- **Singular extensions** (CPW): the relaxed modern form also accepts exact TT
  entries. Double and negative extensions are common practice; CPW does not
  describe them, so their description is pinned at step start (P11).
- **Transposition table** (CPW): buckets sized to a cache line, depth-preferred
  replacement with ageing, partial keys.
- **Stockfish 18** (2026-01-31 release notes): "heavily refined search,
  utilizing Correction History"; +46 over 17.

### 2.3 How network-free engines went from ~2900 to ~3350

**Stash** changelog (GPL-3; ideas only, own measurements, LTC/STC as stated).
Its v26–v32 window spans roughly this project's gap:

| change | Stash's measured Elo |
|---|---|
| stand pat after the TT probe in QS | +35.7 STC, +28.5 LTC (chesso has it) |
| time manager on move type, eval direction, stability | +31.0 (chesso has one) |
| negative-SEE pruning in QS | +67.4 (chesso has it) |
| LMP by move count; more aggressive; non-improving | +18.8/+24.5, +24.8, +16.2 (chesso has S109) |
| staged generation with bad captures last | +17.9 |
| capture history | +4.6 |
| mobility zone excluding rammed and low-rank pawns | +20.0 |
| mobility respecting pins | +6.2 |
| king safety rewrite: attack count, safe checks, quadratic mg / linear eg | +4.2, then +9.7 (weak/safe squares), then +10.9 |
| passed pawns: king proximity | +22.3 |
| candidate passers | +6.9 |
| connected pawns (phalanx, defender) | +25.4 STC, +18.6 LTC |
| knight outposts, shielded minors | +10.2 |
| initiative from threatened pieces | +10.1 STC, +7.8 LTC |
| endgame scaling and specialised endgames | +3.9, +3.8, +9.1, +3.3 |
| singular extensions | +5.9 STC, +13.5 LTC |
| captures reduced by LMR by one ply | +4.9 |
| TT: lower-depth entries may replace | +7.5 |

**Ethereal**, the strongest network-free engine of its era (GPL-3; release
notes and the 2020 paper, ideas only). 11.00–12.75 added threat evaluation,
pawn-push threats, passed-pawn safe paths, candidate passers, king-area tables,
complexity, outposts by openness, multicut, an evaluation cache and
asymmetric PSQTs. The paper *Evaluation & Tuning in Chess Engines* (Grant,
2020) is the tuning method this plan adopts:

- **Trace-based tuning**: every weight application in `evaluate()` also records
  its coefficient; the tuner recomputes any position's score from the sparse
  coefficient vector without calling the engine, so thousands of weights are
  tuned together and a new term costs only its trace call.
- **Nonlinear groups with analytic gradients**: king safety as a linear sum per
  side passed through `max(0,x)²`-style mg and linear eg transforms;
  **complexity** adjusting the endgame score toward zero without flipping its
  sign; a scale factor ξ for drawish material.
- **Data**: about one million self-play games at hyper-bullet controls,
  positions sampled, each **replaced by the end of a high-depth PV** so the
  stored position is quiet. Ethereal's early data also used other engines'
  positions; **that part is excluded here** (DEC-016).

**Leorik 2.x** (release notes) is the lesson in the other direction: material,
PSQT, a few pawn terms with a pawn hash (+50), mobility with a TT and
time-manager rewrite (+100), a self-play refit and SEE in QS reached about 2800;
2.5 added king-position-dependent piece-square functions with PEXT and SMP
(+100 together). An engine with far less search than chesso's sits above it;
the difference is not search.

### 2.4 Tablebases

CCRL allows 6-piece Syzygy. Published gains vary from under 10 to 35–40
(TalkChess thread 70110: Komodo 35–40, Topple about 25); DEC-085 recorded about
13 for a hand-crafted engine. No independent format specification exists: an
own prober means reading someone's prober. Fathom is MIT (jdart1/Fathom), the
route DEC-087 put on record. This is an owner decision (O1 in `plan.md`).

### 2.5 Testing practice elsewhere

Fishtest gainers: STC `{0, 2}`, LTC `{0.5, 2.5}` nElo (PR #4950). OpenBench
engines: 8+0.08 STC and 40+0.4 LTC at `{0, 3}` (Avalanche PR #75). chesso's
`{0, 5}` nElo at 8+0.08 fits an engine whose changes are still large. SPSA
(Spall; CPW; `zamar/spsa`) tunes search constants jointly; S085 measured +21
here on twelve of twenty-two parameters, and about sixty exist now.

## 3. Gap analysis

### 3.1 Search

| technique | chesso | literature direction | step |
|---|---|---|---|
| pawn correction history | absent | +30 to +48 nElo in one engine; larger at long TC | P05 |
| non-pawn and continuation correction | absent | adopted widely | P06 |
| capture history | absent; MVV-LVA | Stash +4.6 | P07 v1 |
| losing captures after quiets | absent; above killers | Stash +17.9 (with staging) | P07 v2 |
| TT buckets, ageing, full Hash | direct-mapped, 75 % of Hash used | Stash +7.5 for replacement alone | P08 |
| threat-indexed, pawn, low-ply history | absent | no figure read | P09 |
| captures reduced by LMR | only the SEE-losing extra | Stash +4.9 | P10 |
| double and negative SE | +1 only | no figure read | P11 |
| correction as uncertainty in LMR | absent | CPW names the use | P12 |
| fifty-move damping | absent | no figure read | P13 |
| joint SPSA over every parameter | twelve tuned in S085; the rest seeded | S085 +21 | P14, P28 |

### 3.2 Evaluation

| term | chesso | literature direction | step |
|---|---|---|---|
| clamp-free full evaluation | clamped at ±184 | S120: −2.6 % wall time | P16 |
| nonlinear king safety with safe checks | linear, 9 counts | Stash +4.2, +9.7, +10.9 | P17 |
| mobility area and per-count tables | linear raw counts | Stash +20.0, pins +6.2 | P18 |
| passer king proximity, path, rook behind, candidates | rank only | Stash +22.3, +6.9 | P19 |
| connected and weak-unopposed pawns | absent | Stash +25.4/+18.6; Leorik +50 with a pawn hash | P20 |
| threats | absent | Stash +10.1; Ethereal 11.00 | P21 |
| outposts, bad bishop, rook files, bishop pair | absent or zero | Stash +10.2 | P22 |
| space | absent | Ethereal-era standard | P23 |
| endgame scale factor, mop-up | absent | Stash +3 to +9 each | P24 |
| complexity | absent | Grant 2020 §1.5 | P25 |
| king-relative PSQT | absent | Leorik 2.5, bundled | P27 |

## 4. Strategy

1. **The evaluation is the main lever.** Chesso's search is broad and deep;
   its evaluation has about ten term families where engines at 3000–3350 had
   dozens. The last twenty search refinements returned mostly zeros.
2. **Search components second, refinements last.** What is missing in search
   is mechanisms (correction and capture history, TT), which is where the
   record says Elo lives.
3. **Tooling first where it multiplies.** A trace tuner turns each evaluation
   term into "write the term, emit its trace, fit, SPRT". Without it every term
   re-plumbs `tools/eval_model.hpp` by hand.
4. **Two lanes.** The machine lane runs verdicts in order. The agent lane
   builds tooling and neutral infrastructure while a match holds the machine,
   as AGENTS' one-task rule allows when the first task is blocked on a match.
5. **Re-tune at block boundaries.** SPSA after the search block, a joint refit
   after the evaluation block, SPSA again at the end. Each is its own verdict.
6. **One change per verdict.** A new term is fitted with every other constant
   frozen (S027's rule); a joint refit is a separate verdict.
7. **Stop and re-plan on triggers**, not on drift (`plan.md` §5).

## 5. Elo arithmetic: an ordering aid, not a forecast

DEC-019: published figures have failed to transfer here three times. The
numbers below rank work; they predict nothing.

| block | published-direction band, self-play Elo |
|---|---|
| search components P05–P13 | +60 to +110 |
| SPSA P14 | +10 to +25 |
| evaluation P16–P25 | +120 to +200 |
| joint refit P26 | +15 to +30 |
| SPSA P28 | +5 to +15 |
| **total** | **+210 to +380** |

At the 0.83 factor that is +175 to +315 on CCRL: **2940 to 3080**. Reaching
3000 needs most blocks to land near the middle of their band. The re-plan
triggers in `plan.md` §5 exist because of that.

## 6. Risks

- **Evaluation cost.** Each term is paid at every node. Shared attack maps and
  a pawn cache come first (P15), and every term's SPRT carries its own cost.
- **Mate safety.** Correction history changes the input of RFP, razoring,
  futility and null move. The mate suite, a guard test and a killed mutant are
  in every such step's `accepts` (DEC-141).
- **Tuner overfit.** Held-out split by game (S066), dedup (S076), a fixed
  validation set from independent games (P03), and an SPRT on every fit.
- **Transfer.** The 0.83 factor is one observation. P29's rating run measures
  it again.
- **Machine.** This M1 falls over after four to five hours of full-core match.
  Slow-class SPRTs, SPSA and datagen nights need the workstation (O2).
- **Partial TT keys** (P08) raise the false-hit rate. Every reader of
  `best_move` that can play it without generating moves (the root hint, the
  mate-line walk) must check legality.

## 7. Sources

- CCRL Blitz list and conditions: https://computerchess.org.uk/ccrl/404/
- Static Evaluation Correction History, CPW: https://www.chessprogramming.org/Static_Evaluation_Correction_History
- Stockfish PR #4950, correction history: https://github.com/official-stockfish/Stockfish/pull/4950
- Avalanche PR #75, pawn correction history: https://github.com/SnowballSH/Avalanche/pull/75
- Stockfish PRs on non-pawn and minor correction indexing (topic only; their constants are not used): https://github.com/official-stockfish/Stockfish/pull/5816, https://github.com/official-stockfish/Stockfish/pull/5841
- Stockfish PR #2557, low-ply history: https://github.com/official-stockfish/Stockfish/pull/2557
- History Heuristic, CPW: https://www.chessprogramming.org/History_Heuristic
- Late Move Reductions, CPW: https://www.chessprogramming.org/Late_Move_Reductions
- Singular Extensions, CPW: https://www.chessprogramming.org/Singular_Extensions
- Transposition Table, CPW: https://www.chessprogramming.org/Transposition_Table
- King Safety, CPW: https://www.chessprogramming.org/King_Safety
- Stash changelog: https://raw.githubusercontent.com/mhouppin/stash-bot/master/CHANGELOG.md
- Grant, *Evaluation & Tuning in Chess Engines* (2020): https://github.com/AndyGrant/Ethereal/blob/master/Tuning.pdf
- Ethereal release notes: https://github.com/AndyGrant/Ethereal/releases
- Leorik release notes: https://github.com/lithander/Leorik/releases
- Stockfish 18 release notes: https://stockfishchess.org/blog/2026/stockfish-18/
- Determining engine strength (Stash as a gauntlet family): https://dannyhammer.github.io/engine-testing-guide/determining-strength.html
- Syzygy Elo discussion: https://www.talkchess.com/forum3/viewtopic.php?t=70110
- Fathom (MIT): https://github.com/jdart1/Fathom
- SPSA, CPW: https://www.chessprogramming.org/SPSA ; https://github.com/zamar/spsa
