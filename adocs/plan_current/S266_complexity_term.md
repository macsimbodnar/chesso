id:         S266
goal:       a complexity term moves the endgame score toward or away from zero, never past it, by the structures the stronger side is or is not likely to convert (amended 2026-11-05 from "toward zero", DEC-352: the paper's C is free in sign), and never flips the score's sign
accepts:    (1) complexity is a fitted linear sum of features `evaluate()` already holds -- pawn count, pawns on both flanks, a pawn-ending flag, a constant -- applied to the endgame half as `E + sign(E) * max(C, -|E|)`, the form the paper describes, except where either king stands alone (amended 2026-11-05, DEC-352: E is E1, and a lone king is mop-up's domain); (2) a test, with each precondition established first: the adjusted endgame score never has the opposite sign of `E`, it is zero where `E` is zero, and it **may** reach zero where `C <= -|E|` -- the 2026-10-07 proposal's "zero only where the unadjusted one is" contradicts its own formula and is not the contract; (3) the group is fitted through S262's nonlinear-group support with its gradient checked, every other constant frozen, from a nondegenerate seed (DEC-134 form b); (4) the term passes DEC-259's offline screen -- held-out loss on S122's regenerated corpus's own held-out qs view (amended 2026-11-05, DEC-352, as DEC-350 (7) for S124: DEC-349 made S082's set history) below the model without it by the pre-registered margin -- before any match, and a term that fails the screen is recorded and not measured; (5) one `{0, 5}` nElo SPRT, worst case and abort rule pre-registered (DEC-143); (6) INV-4 and INV-5 hold: nothing recomputed from bitboards that the stage does not already have, and the mirror case in `tests/test_evaluation.cpp` extended; (7) `adocs/specs.md` gains the term
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md, src/eval_trace.hpp, tools/tuner_trace.hpp, tools/tuner_layout.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tools/truncation_scan.cpp, tools/eval_spread.cpp, tests/test_eval_trace.cpp, tests/test_eval_model.cpp, tests/test_walked_counts.hpp, tests/test_tuner_groups.cpp, tests/test_tuner_gradient.cpp, tests/test_eval_positions.hpp (widened 2026-11-05, DEC-352)
excludes:   the endgame scale factor and mop-up, which are S124's family; any change to the tapered PSQT
decisions:  DEC-258, DEC-259, DEC-192, DEC-134, DEC-221, DEC-349, DEC-350, DEC-352
closes:
blocks:
paused_by:
author:     the coordinator; phase 0 (research pass) by an Opus 5.5 subagent, briefed by the coordinator, 2026-11-05
done:

## Why

S217 found complexity scaling in two of the five hand-crafted engines rated
3000 to 3344 (Ethereal 11.59 to 11.64; rofChade) and no step for it here;
DEC-192 filed it as a reserve candidate with no published figure. The
2026-10-08 ruling makes it a step (DEC-258): the premise that kept it out --
the network would supersede hand-crafted refinements (DEC-138) -- fell when
the owner put the network after the mark (DEC-179).

## Description it is implemented from

Grant, *Evaluation & Tuning in Chess Engines* (2020): the complexity
adjustment (remaining pawns, pawns on both flanks, pawn endgame; dampens
toward a draw and never flips the side to move's advantage) and its
gradient. Ethereal's 11.75 release notes name the term. No engine's
constants seed it (DEC-105).

## From the record

`adocs/data/S217_handcrafted_gap.md`, its outline for this term. Ported from
the 2026-10-07 proposal's P25 with its test contradiction corrected.

## 2026-11-05: the research pass

Read at `2bf3ad4`, bench 5482277 reproduced in the pass's own build: counts, label readings and instruction counts
only, no fit of the engine, no match, no timing, nothing under `src/` or `tools/` changed. Every figure's source is
adocs/data/S266_census.txt, by adocs/data/S266_census.sh (adocs/data/S266_census.cpp), and adocs/data/S266_cost.txt, by
adocs/data/S266_cost.sh (S266_cost_sink.patch). The proposal is `.tuning/coord/S266/decision_draft.md`. Revised the
same day after the coordinator's fast check, which reproduced the census byte for byte:
- every label reading is now taken at two K, the corpus's and K refitted at S124's cells, and finding (c) is rewritten
  on that;
- findings (a) and (b), the other order's integer form and the plant's description are corrected.

**What the census reads.** The FEN and game columns of every corpus. The result column of one: S122's regenerated
prepared training corpus (`.tuning/s122/prepared/corpus.tsv`, sha256 `04c596de...`, 6307294 rows, 137991 games,
written by S331's `datagen`, DEC-349), at its own K 1.2302. Its three held-out views (`pool.tsv.{qs,root,pv}.validation`,
12000 games) and S083's corpus (the old two-fold rule's, DEC-349) are read for counts; their labels are unread. On
the views the census also counts the rows the corpus's fitted C would take to zero: E1 and the features are the
engine's, not labels.

**How it is checked.**
- Every feature is held to a square-by-square walk of its definition on every row of all five corpora: 0 differ.
- A planted flank, file d counted on neither side, turns the walk red: 534 rows of 20000.
- Every traced row's integer score equals `evaluate()`: 0 differ on all five.
- E1 is an integer at the shipped weights on every row (the census refuses one that is not).

No position was judged: every feature is a count of pawns, files and men (DEC-023).

**The description, read** (Grant, *Evaluation & Tuning in Chess Engines*, 2020, the PDF in the Ethereal repository,
sha256 `7436892a...`, fetched 2026-11-05; its prose and equations, DEC-221).
- Section 1.5 names the features and the contract: "In Ethereal we look at remaining pawns, whether there are pawns
  on both flanks, and whether we are in a pawn endgame. In a position in which White is deemed winning, complexity
  may not revise the evaluation to have Black winning. At most, the position may be dampened to a draw."
- The form: Γ = L_c · C, "the linear weights for Complexity dotted with their coefficients", and E'_eg = E_eg +
  sign(E_eg) max(-|E_eg|, Γ). "Usually an engine will only adjust the evaluation for the endgame component."
- **The order.** Section 1.3's taper is ((256 - ρ) E_mg + ρ ξ E_eg) / 256, ξ the scale factor, and sections 3.1,
  3.4 and 3.7 carry ξ inside ρ_eg on the adjusted endgame half: complexity first, then the scale factor.
- **Nothing is defined.** No feature has a definition and no weight a value. Section 1.4's two divisors are an
  engine's king-safety constants and are no seed (DEC-105). Section 4's listings are an engine's tuner; they were
  not used (a search of the text for "complex" showed two of their lines).
- **Section 3.7 as printed is not the derivative of section 1.5.** It reads ∂E/∂L_c = sign(Γ) C_i ρ_eg where
  |E_eg| + Γ >= 0, else 0. From 1.5 the derivative is sign(E_eg) C_i ρ_eg on that branch. The two differ wherever
  sign(Γ) and sign(E_eg) differ: damped rows with E_eg > 0 and widened rows with E_eg < 0, White relative. Nothing in
  `src/` or `tools/` implements any complexity derivative today. Section 3.4's E_eg = 0 case takes the unadjusted
  derivative of the linear weights; that is the derivative only at Γ = 0. Inside the dead band, Γ < 0, it is 0, and
  at Γ > 0 the function jumps by 2Γ there. The paper says the E_eg = 0 case is "undefined" and passes over it. This pass's
  gradient is derived from 1.5 and is checked by finite differences in phase 1 (below).
- The paper allows Γ > 0, which moves the endgame half away from zero. The step's goal says "toward zero";
  adocs/data/S217_handcrafted_gap.md's outline says the term "widens or narrows". Q1 is that question.

**What exists.**
- **Stage one**, `src/evaluation.cpp` `cheap_score`: the pair `board->psqt + pawns`, one `taper`, `board->material`
  added untapered, tempo apart. E1 = `board->material` + `eg_value` of the pair is the endgame half S124's factor
  multiplies (DEC-350 (1)). It holds the material, so the seven gauges `tools/tuner_groups.hpp` holds stay null under
  any function of E1 (S124's argument).
- **Every feature is already held**, and INV-4 is untouched.
  - P: one popcount of the two pawn bitboards the board keeps. Not stored, and nothing would gain from storing it.
  - flank: `src/data_structures.hpp` `pawn_entry_t`'s `white_files` and `black_files`, the bytes `pawn_pass` writes on
    a miss and a hit reads for free; two masks on their OR. A zeroed slot under a pawnless key reads no file, which
    is the pawnless truth, so DEC-351's zero-slot rule holds.
  - pe: `board->phase` is 0. `make_move` accumulates it, and INV-4 names it.
  - Nothing missing.
- **Stage two** (`src/evaluation.cpp` `expensive_score`) clamps the tapered sum of mobility, threats and king safety
  to `LAZY_EVAL_MARGIN` and holds no endgame half apart after the clamp.
- **S124's phase 1** (worktree `chesso-s124p1`, staged, not landed) puts its factor inside the one positional
  division: stage one's integer is material + (F X + (f_c - F) E1 (24 - phase)) / (24 F), X `taper`'s numerator, F
  128, one truncation. Its trace records both candidate cells, and the tuner picks by E1's sign at the weights it
  holds.

**The cost** (adocs/data/S266_cost.txt). The term in stage one at every call, forced non-zero weights into an asm
barrier, the score the landing's: the popcount, the two file bytes, the phase test, C, adj(E1, C) and its share of
the taper's numerator. Node counts 5482277 and 14931387 on all four builds before any count. Five interleaved
`tools/perf_counters` runs, instructions only, spreads 59 to 199, under S122's screen's load:

| sink build, `S266_SINK`, instructions over base | `bench` | `bench 16` |
|---|---|---|
| `=1`: the term on E1 at every stage-one call | +1.325 % | +1.348 % |
| `=3`: the same, skipped at phase 24, where its share is 0 | +1.241 % | +1.305 % |
| `=5`: `=1` with C as min(C, 0) | +1.415 % | +1.436 % |

No cycle, wall or nps was read. LONE's exclusion (below) reads the two occupancy compares S124's mop-up already
makes; it is not in the sink and is not priced. Frames: S124's factor and mop-up +1.906 %, S102's family +6.737 %.

**What the corpus holds.** Every definition is chesso's own and is said to be (DEC-221):
- P is the pawns on the board, both colours.
- flank is a pawn of either colour on files a to d and one on files e to h.
- pe is no knight, bishop, rook or queen on the board.
- C = c0 + cP P + cF flank + cE pe, one endgame weight each.
- adj(E1, C) = E1 + sign(E1) max(-|E1|, C).

The term moves nothing at phase 24 (w_eg = 0), and nothing where E1 = 0. S122's regenerated corpus, rows by phase:

| phase | rows % | mean P | flank % | pe % | E1 = 0 % | \|E1\| <= 25 % | \|E1\| <= 100 % |
|---|---|---|---|---|---|---|---|
| 0-4 | 16.191 | 4.26 | 57.63 | 2.364 | 0.171 | 8.128 | 27.129 |
| 5-8 | 19.902 | 6.12 | 78.99 | 0 | 0.136 | 6.703 | 24.493 |
| 9-12 | 12.224 | 8.85 | 95.52 | 0 | 0.177 | 8.885 | 32.425 |
| 13-16 | 9.306 | 9.99 | 98.75 | 0 | 0.197 | 9.684 | 35.393 |
| 17-20 | 13.651 | 11.22 | 99.73 | 0 | 0.223 | 10.894 | 39.268 |
| 21-23 | 13.673 | 13.02 | 99.97 | 0 | 0.237 | 12.113 | 41.952 |
| 24 | 15.054 | 14.28 | 100 | 0 | 0.386 | 19.429 | 64.214 |
| all | 100 | 9.38 | 88.26 | 0.383 | 0.216 | 10.705 | 37.287 |

The views and S083's corpus, all rows:

| corpus | rows | mean P | flank % | pe %, rows, games | E1 = 0 % | \|E1\| <= 25 % |
|---|---|---|---|---|---|---|
| S122's corpus | 6307294 | 9.38 | 88.26 | 0.383, 24137, 6460 | 0.216 | 10.705 |
| qs view | 1384182 | 8.53 | 81.62 | 0.376, 5199, 904 | 0.218 | 10.562 |
| root view | 1385493 | 8.75 | 82.22 | 0.292, 4048, 371 | 0.209 | 10.310 |
| pv view | 1316166 | 7.46 | 77.83 | 0.999, 13149, 3543 | 0.183 | 9.228 |
| S083's corpus | 6226944 | 10.43 | 94.47 | 0.268, 16709, 3915 | 0.217 | 10.971 |

The P histogram by phase is in adocs/data/S266_census.txt.

**The case the corpus barely holds is pe.** It holds 24137 rows in 6460 games, past DEC-326 (5)'s 1000, so no tie.
- **The screen's instrument, the qs view, holds 904 games of it**, under the 1000.
- 6592 of the corpus's pe rows sit in S124's PAWNS cells (a strong side with at most two pawns), 17545 in no cell.
- What cE adds over the other three is -16.36 +/- 2.28 across halves (below).

DEC-259's amendment, S124's accepts, screens such a case on a tool-labelled set. Here the tablebase guard below reads
its rows of five men or fewer. Its rows of six men or more are screened on the loss alone, and the stamp says so.

**How many rows an adjustment can take to zero.** Rows with 0 < |E1| <= -C below phase 24, as a share of all rows:
-25: 7.623 % (97663 games); -50: 14.847 %; -100: 27.463 %; -200: 46.522 % (by band in the census). At the proposal's
fit, C4x | F12 below, at K0 (C from -46.32 to 84.23) and on the corpus also at K* (C from -50.69 to 67.72), rows with
E1 != 0 below phase 24 and no king alone:

| corpus | bound to 0 | damped (C < 0, not bound) | widened (C > 0) |
|---|---|---|---|
| S122's corpus | 1.178 % | 9.831 % | 73.138 % |
| S122's corpus, K*'s fit | 1.368 % | 16.242 % | 66.536 % |
| qs view | 1.790 % | 14.592 % | 68.721 % |
| root view | 1.739 % | 14.145 % | 68.087 % |
| pv view | 1.744 % | 17.303 % | 72.919 % |

By band on the corpus at K0's fit, 5.276 % of phase 0-4's rows are bound, 1.390 % of 5-8's, 0.294 % of 9-12's, and
under 0.1 % past phase 12.

**What the labels read beside the shipped score.** S122's corpus, at two K. Revised 2026-11-05 after the
coordinator's fast check, which reproduced the census and found that the recalibration reading below was mostly K.
- **K0 = 1.2302, the corpus's K.** It is the in-sample optimum at the shipped constants (the census refits it there
  and reads 1.2302).
- **K* = 1.3578, K refitted at S124's cells**, to a fixed point. F12 is fitted at K, K is refitted at that fit by
  golden section on the in-sample MSE, and the loop repeats until K moves by under 1e-4.
  - **Four rounds:** 1.3474, 1.3570, 1.3578, 1.3578.
  - The tuner fits K at its start constants, so with S124's cells at their fit S266's screen reads at about this K.
    Round 1, the cells fitted at K0 and K refitted once, is 1.3474.
  - K0 balances the cells' over-confident drawish rows against the rest; once the cells take those rows, K rises.
- **Every reading below is taken at both**, every model fitted afresh at each.
- **The method.** Each model is fitted to the labels by damped Gauss-Newton on the squared error, every engine weight
  at the shipped constants. A model with S124's cells and complexity starts from the identity and from the
  cells' own fit with C at 0 and at the complexity-only fit's, and keeps the best; the loss is not convex in a
  product with a kink.
- **The readings.** In sample, at the fit rounded (cells to 128ths, weights to integers), and across halves: the
  even games' fit read on the odd and back. dMSE is in 1e-6 against the shipped score at the same K, with
  whole-game errors. These are readings and never fits of the engine. Every reading is in the census.
- **The models.** Each is a smooth score, every taper exact, w_eg = (24 - phase) / 24.
  - **C4**: the term on E1, the paper's form.
  - **W4**: on E1 + E2, the paper's whole endgame half, the change put outside the clamp: a reading of the form, not
    a design.
  - **R4**: C replaced by min(C, 0).
  - **F12**: S124's thirteen cells as a ratio r_c in its box [0, 1], OCB_PURE P >= 5 tied to P = 3-4.
  - **G1, G6**: one unboxed ratio on every row's E1, or one per phase band. A recalibration of the endgame half with
    no cell and no feature: the control.
  - **F12G6**: the cells where one is picked, the band's ratio on every other row but a lone king's.
  - **X | Y**: X's weights fitted with Y's ratios held at Y's fit of the same rows, the screen's setting
    (DEC-259 (a)).
  - **x**: no adjustment on a row where either king stands alone (LONE, mop-up's domain).
  - **C4x'**: C4x in the other order, adj(r E1, C).

| model, across halves | dMSE at K0 | at K* | fitted at K0 (c0, cP, cF, cE) | at K* |
|---|---|---|---|---|
| C4 | -2108.68 +/- 37.52 | -2370.01 +/- 41.86 | -228.72, 11.61, 149.45, 65.13 | -243.73, 11.00, 154.95, 61.90 |
| W4 | -2241.23 +/- 38.99 | -2546.96 +/- 43.75 | -241.96, 11.54, 162.26, 63.98 | -259.80, 10.81, 171.19, 60.12 |
| R4 | -2935.47 +/- 41.20 | -3310.87 +/- 44.60 | -452.79, 131.23, 23.00, 142.33 | -471.38, 132.61, 21.68, 144.88 |
| F12 | -2932.26 +/- 39.10 | -3352.45 +/- 42.80 | | |
| G1 | -78.98 +/- 8.52 | -351.80 +/- 17.84 | ratio 0.905 | 0.810 |
| G6 | -587.60 +/- 18.71 | -803.59 +/- 22.93 | ratios 0.694, 0.979, 1.202, 1.216, 1.256, 1.870 | 0.628, 0.884, 1.087, 1.088, 1.087, 1.424 |
| F12G6 | -3254.32 +/- 39.26 | -3432.51 +/- 42.07 | band ratios 1.221 to 1.260 below phase 21, 1.874 at 21-23 | 1.091 to 1.136, 1.428 |
| C4x \| F12 | -3179.16 +/- 39.94 | -3473.30 +/- 43.99 | -46.32, 3.46, 42.71, 39.44 | -50.69, 2.80, 39.26, 39.93 |
| R4x \| F12 | -2968.09 +/- 40.52 | -3400.08 +/- 44.44 | -59.51, 6.55, 73.18, 97.51 | -54.09, 3.39, 40.54, 128.16 |

At K* a model with no cell reads more against the shipped score than at K0, because the shipped score is off K*'s
optimum: compare models with each other at one K, never across the two. In sample and across halves agree within 10
everywhere but in fits of the hinge form: F12 + R4 by 13 at K0; R4 | G6, whose full fit reads 46 above its
across-halves figure at K0, a local optimum of that form's loss, and 24 below it at K*. Rounding to integers moves no
reading by more than 11.1 (C4 | G6 at K*), and the screen's candidate, C4x | F12, by at most 3.4.

The increments, paired by game across halves:

| what | over | at K0 | at K* |
|---|---|---|---|
| C4 | C1 (c0 alone) | -2055.68 +/- 36.06 | -2142.76 +/- 37.77 |
| C4 | C4 without cP / cF / cE | -352.10 / -522.18 / -16.36 +/- 2.28 | -345.52 / -565.54 / -17.78 +/- 2.34 |
| W4 | C4 | -132.55 +/- 7.37 | -176.95 +/- 7.18 |
| F12 | C4 | -823.58 +/- 26.96 | -982.44 +/- 26.85 |
| C4 \| F12 | F12 | -260.49 +/- 11.23 | -132.87 +/- 7.62 |
| C4x \| F12 | F12 | **-246.90 +/- 10.99** | **-120.84 +/- 7.23** |
| C4x \| F12 | C4 \| F12 | +13.59 +/- 0.98 | +12.03 +/- 0.89 |
| R4 \| F12 | C4 \| F12 | -124.47 +/- 18.88 | -250.17 +/- 16.54 |
| R4 \| F12 | R4x \| F12 | -349.12 +/- 14.96 | -335.42 +/- 15.18 |
| R4x \| F12 | F12 | -35.83 +/- 3.61 | -47.63 +/- 4.23 |
| R4x \| F12 | C4x \| F12 | +211.07 +/- 10.94 | **+73.22 +/- 6.38** |
| W4x \| F12 | C4x \| F12 | -107.72 +/- 6.01 | -143.09 +/- 5.77 |
| C4x' \| F12 | C4x \| F12 | +12.52 +/- 2.31 | +18.45 +/- 2.46 |
| F12 + C4 joint, paper's order | the other order, joint | +19.89 +/- 1.86 | +21.90 +/- 1.98 |
| F12G6 | F12 | -322.06 +/- 14.83 | **-80.06 +/- 7.53** |
| C4x \| F12G6 | F12G6 | **-82.82 +/- 5.65** | **-85.70 +/- 5.78** |
| R4x \| F12G6 | F12G6 | -71.34 +/- 5.47 | -74.76 +/- 5.59 |
| R4x \| F12G6 | C4x \| F12G6 | +11.48 +/- 2.79 | +10.94 +/- 2.80 |

(The cP and cF increments' errors are 12.43 and 15.72 at K0, 12.46 and 17.03 at K*.)

**What C adds over S124's thirteen cells, and what it is.** At K*, the screen's K, unless a K0 figure is given beside
it.
- **Alone, the term is mostly S124's cells.** C4's in-sample gain sits -1823 of -2111 in S124's PAWNS cells and -271
  in its OCB cells at K0. F12 alone reads -982.44 +/- 26.85 below C4 alone (-823.58 at K0).
- **The two do overlap.** The cells partition by the strong side's pawns and by men; C's pawn count and flank read the
  same structures colour-blind.
- **Over the cells frozen at their fit, C adds -132.87 +/- 7.62**, and **-120.84 +/- 7.23 with LONE excluded**. At
  K0 the same read -260.49 and -246.90: half of the K0 increment was K. Excluding LONE costs 12.03 +/- 0.89.
- **The increment is mostly widening.** At C4x | F12's fit C > 0 on 66.5 % of rows (73.1 % at K0). In sample it takes
  -9 and -20 from the PAWNS and OCB cells' rows, -54, -21 and -10 from the rows in no cell at phases 0-8, 9-16 and
  17-23, and -7 from pawn endings in no cell.
- **Little of it is a recalibration of the shipped endgame half.**
  - At K*, F12G6, a ratio per phase band on the rows in no cell, reads -80.06 +/- 7.53 over F12. Its ratios are 1.09
    to 1.14 below phase 21 and 1.43 at 21-23, and w_eg is under 0.13 there.
  - At K0 the same control read -322.06, its ratios 1.22 to 1.26 and 1.87; most of that was K.
  - Without the cells the endgame half reads over-scaled at low phase, not under-scaled: G6's ratio at phase 0-4 is
    0.694 at K0 and 0.628 at K*, and G1 reads 0.905 and 0.810.
  - Over F12G6 the term adds -85.70 +/- 5.78 and min(C, 0) -74.76 +/- 5.59, 10.94 +/- 2.80 apart (-82.82 and -71.34
    at K0).
  - **So at K*, about -86 of C4x's -121 over the cells survives a phase recalibration of the other rows, and about
    -35 does not (inferred: the control is a ratio, not a refit).** At K0 the split read -83 and -164; the
    "two thirds" the first draft of this section gave was K's.
- **LONE is a trap for a label fit.** With LONE included, min(C, 0) takes -335.42 +/- 15.18 more than without it,
  nearly all on LONE rows: -319 in sample at K*, where the labels read 0.698 against 0.978 predicted (strong side's
  view; 0.973 at K0). That is mop-up's domain, which S124 fits on conversion because these labels are the draws mop-up
  is for (DEC-350 (4)). There the fitted R4 | F12 takes the strong side's prediction to 0.755, with C at -583 where no
  pawn stands. The free form takes -14 there (C -57). With LONE out, min(C, 0) adds -47.63 +/- 4.23 over the cells.
- **Which E.** On E1 + E2, the paper's whole endgame half, the term reads 143.09 +/- 5.77 better than on E1 over the
  cells (107.72 at K0), and 176.95 +/- 7.18 better alone.
- **The order.** In the screen's setting, the cells held, the paper's order reads **18.45 +/- 2.46 better** than
  adjusting after the factor (12.52 at K0). In joint fits, where the cells move with C, it reads 21.90 +/- 1.98
  worse. The first draft of this section read only the joint figure.
- **The jump.** Where C > 0, a change of E1's sign carries the endgame half from -C to +C. At C4x | F12's fit, 407453
  of the 480757 rows with 0 < |E1| <= 25 below phase 24 have C > 0, and their jump on the score, 2 C w_eg, reads
  p50 9.3, p90 24.9, max 135.4 cp (p50 18.9, p90 44.2, max 168.5 at K0). Under min(C, 0) there is none.

**The rank against what exists** (157683 rows, every 40th). R^2 of each complexity column at the seed, sign(E1) x_i
w_eg, the score's derivative in its weight, on every existing parameter's tapered column (1101 of 1172 live once the
dependent ones drop, the intercept included), then with S124's cells' columns at F (E1 w_eg on the rows E1 picks),
then with the other three complexity columns too:

| column | existing | + S124's cells | + the other three |
|---|---|---|---|
| c0 | 0.621 | 0.648 | 0.918 |
| cP | 0.431 | 0.458 | 0.887 |
| cF | 0.459 | 0.488 | 0.866 |
| cE | 0.033 | 0.034 | 0.053 |

- The existing design, the cells and the four together are 1118 live columns of 1189: full rank, the cells adding 13
  and the term 4.
- `feature_audit`'s identities report is expected to read the rank up by 4 at the landing (inferred; the run
  decides). The S124 cells' own R^2 on the existing design is 0.028 to 0.181, and the four columns add at most 0.09
  (P=1 d<=0).
- No exact dependency is expected. The three non-pe columns overlap one another strongly, and cP and cF are about
  half spanned by the existing design.

**The form, as this pass proposes it.** Every definition is chesso's own and is said to be (DEC-221). The form is
section 1.5's; the order is the paper's; the features are the step's.
- **Where:** stage one, on E1, inside the positional taper's one division, outside the clamp (reading A, as S124's).
  - On HEAD the integer is material + (X + (adj(E1, C) - E1)(24 - phase)) / 24.
  - With S124 landed, material + (F X + (f_c adj(E1, C) - F E1)(24 - phase)) / (24 F), f_c = F where no cell is
    picked.
  - One division, so the truncation bound is S124's, per row, and unmoved by this term. adj(E1, C) - E1 is an integer
    at integer weights, so the smooth score's residual is still one truncation.
  - At C = 0 it is S124's integer exactly; at f_c = F, (X + (adj - E1)(24 - phase)) / 24 exactly, F dividing both.
- **C** = c0 + cP P + cF flank + cE pe: four integer weights, endgame half only, no middlegame half (section 1.5's
  "only ... the endgame component"). `PARAM_COUNT` gains 4: 1214 to 1218 on S102's landed tree.
- **adj**, the published form: C free in sign (Q1). Never E1's opposite sign; 0 at E1 = 0; 0 wherever C <= -|E1|.
- **LONE excluded** (Q5): on a row where either king stands alone, nothing. That is the occupancy test S124's mop-up
  already makes, so no count is read twice. LONE is mop-up's domain, fitted on conversion.
- **The order** (Q3): complexity first, then S124's factor, the paper's.
- **The other order** is integer-exact too: F adj(f E1 / F, C) = f E1 + sign(f E1) max(-f |E1|, F C), the sign
  taken of f E1 so that f = 0 gives 0 and not F C. Its numerator is F X + (f E1 + sign(f E1) max(-f |E1|, F C) - F
  E1)(24 - phase), one division.
- **Headroom:** |adj(E1, C)| <= |E1| + max(C, 0), so S124's int32 `static_assert` on the numerator gains C's positive
  reach (the sum of the positive weights times each input's maximum, 16 for P).
- **The seed: zero, the identity, DEC-134 form (b)**, as S121's, S124's and S102's.
  - adj(E1, 0) = E1 exactly, so the landing reads `No functional change`. A liveness flag compiles the term out.
  - **And zero is nondegenerate.** A zero C is a kink of max only where E1 = 0: the kink of max(-|E1|, C) is at
    C = -|E1|, which C = 0 meets only there, and there sign(E1) = 0 makes the term identically zero in C.
  - On every row with E1 != 0, C = 0 is strictly inside the branch C > -|E1|. The derivative sign(E1) x_i w_eg is
    defined there, and non-zero wherever x_i is.
  - So the fit starts on the shipped evaluation with every input's gradient live. S122's curve needed c = 45 for
    the opposite reason.
  - Under min(C, 0) (Q1 (b)) zero is a kink of min: the derivative at the seed is one-sided. A fit needs the left
    derivative by convention, or a seed off zero that moves the landing.
- **The fit.** S124's product in full, extended. With r = f_c / F, live = (C > -|E1|), and at E1 = 0 live = (C >= 0):
  - the score's share is (r adj(E1, C) - E1) w_eg;
  - d/dc_i = r sign(E1) x_i w_eg where live, else 0;
  - d/df_c = adj(E1, C) w_eg / F;
  - a piece value's coefficient is 1 + (r live - 1) w_eg;
  - a positional endgame weight's taper weight is r live w_eg.
  - The kinks: E1 = 0, a jump of 2C when C > 0 and the edge of a flat band when C < 0; and |E1| = -C, a corner.
  - The finite-difference check reads rows whose E1 and |E1| + C both clear the perturbation's reach at forced C.
    Both branches are represented, a bound row and a live one each way round.
- **Variants:**
  - none is a part beyond the one.
  - min(C, 0) is Q1's alternative.
  - E1 + E2 and the other order are recorded (Q2, Q3).

**What S262's machinery carries and what it lacks.**
- **Carried, with S124's phase 1 landed:**
  - E1 at the weights read, in `exact_parts`, `smooth` and `gradient`;
  - the ratio r and the cell's pick by E1's sign;
  - the chain rule into the material and the positional endgame weights through r;
  - the box, `--only`, ties, the gauges, `feature_audit`'s cell columns.
- **Lacking:**
  1. **A colour-free input.** Every trace coefficient is White's less Black's, and these are counts. A new term,
     `TERM_COMPLEXITY`, with its own site that no linear sum reads, as `SITE_MATERIAL` is special-cased. Its
     parameters are one endgame half each, as a piece value's single half is.
  2. **`group_t` cannot express it.** Its transform reads its own inputs' per-side sums, White's less Black's, and
     this reads E1, another site's sum, and E1's sign: the same lack S124 found for the product.
  3. `exact_parts`: C in integers and adj(E1, C) inside S124's numerator, the no-cell branch included. That branch
     is `exact_taper` today, and must take the adjustment too.
  4. `smooth`: (r adj(E1, C) - E1) w_eg in place of (r - 1) E1 w_eg.
  5. `gradient`: the five derivatives above, with `live` at E1 = 0 as stated.
  6. `tools/feature_audit.cpp`: the four columns at C = 0, sign(E1) x_i (24 - phase), integers. Off zero its
     identities report says "not computed", S124's form. Support lines per input (rows and games with x_i != 0,
     E1 != 0, below phase 24, no king alone).
  7. `tools/tuner_groups.hpp`: a group and part `complexity`; the header emits `complexity_eg`.
  8. `tests/test_eval_trace.cpp` "the positions move every term both ways" cannot hold for counts, which are never
     negative. It skips the term, as it skips `TERM_PSQT`. A new case reaches each input on rows with E1 > 0 and with
     E1 < 0.
- **Not lacking: a box.** C needs none; min(C, 0) is a form, not a box.

**The identity plan.**
- **`tools/truncation_scan` is the identity.** It holds `exact_score` to `evaluate()` on every row.
  - At the seed C = 0 the term is exactly zero and no plant shows. So the plants are read on a build with C forced
    non-zero, S124's `forced` form, and with a cell forced off F for the order plant.
  - Each of these turns it red:
    - the trace's P input one off;
    - `exact_parts` adjusting the endgame sum without the material;
    - the sign taken from E1 + E2: the split rows, 9.308 % of the corpus's;
    - the order swapped in `exact_parts` alone;
    - the adjustment given a taper of its own, a second truncation;
    - LONE not excluded in the trace alone.
- **`tools/eval_spread` reads stage two alone.** It is blind to the term and stays green, and its header says so.
- **A wrong count in the engine passes both**, the trace reading the engine's own variable. So the walk comes first:
  `tests/test_walked_counts.hpp` gains the three features from their definitions, held by `tests/test_eval_model.cpp`
  "the walk counts what the engine counts". Each plant turns it red:
  - flank from one colour's file byte;
  - file d counted on neither flank (this pass's plant);
  - pe from `board->phase` <= 1.
- **A stale sign in the tuner is invisible to the scan.** S124's stale-pick test, a material weight moved until some
  row's E1 changes sign, is extended: `exact_parts` and `smooth` must take sign(E1) from the moved weights. Red on a
  plant that uses the engine's stored sign.
- **`tests/test_tuner_gradient.cpp`** gains the term's derivative and its chain rule against finite differences, on
  rows off both kinks. It is red on two plants:
  - section 3.7's printed sign(C) in place of sign(E1);
  - the derivative kept on bound rows.
- **INV-5:** "the complexity inputs mirror exactly" (counts equal under `mirror_fen`). And at forced C over every test
  position, adj is odd: the mirrored position's evaluation agrees.
  - `tests/test_evaluation.cpp` "colour symmetry over every test position" is blind at the seed, so it also runs on
    the `forced` build.
- **The accepts' test (2)**, preconditions first, at forced C over the test positions: rows with E1 = 0, rows with
  C <= -|E1| and rows live each way round each exist. Then: never the opposite sign of E1, 0 at E1 = 0, 0 where C <=
  -|E1|.
- **INV-6 at the landing:** `tools/search_bench.py` identical, bench unchanged, and S124_objdiff.py's form showing no
  function changed.

**The screen, as proposed: a pre-registration draft** (S122's, S124's and S102's form). Nothing here is fixed until the
coordinator writes the screen's script and this becomes its header.
- **The corpus.** S122's regenerated prepared corpus at the landed tree's K, re-read after S122, S124 and S102 land:
  about 1.36 with S124's cells at their fit (this pass's K*), not the 1.2302 the first readings here were taken at.
  The instrument is its own held-out qs view (DEC-350 (7)), and the labels are the laws' (DEC-349).
- **The identity rule before any fit.** `feature_audit --free complexity`: every new column non-zero, the rank up by
  4, expected and decided by the run.
- **One part, P1 `complexity`**, the four weights, `--only complexity`. Every other constant is frozen, S124's
  fitted cells and mop-up and S102's survivors among them. It is paired against the landed tree.
  - **Kept iff d > m, where m = max(1.645 sd(d_s), 2e-6)** over five quarter samples: DEC-331 (6)'s rule as DEC-344
    (3), DEC-350 and DEC-351 apply it. The margin is the five samples' own spread, with a floor at 2e-6.
  - Every fit runs `--epochs 20000 --report 100 --patience 20` (DEC-345 (1)).
  - Each d is read at the fit's weights and again at the integers the engine ships. A part kept at real weights alone
    stops for the coordinator.
- **The guards**, each a refusal or a stop and never a reason to keep:
  - **The tablebase guard, S124's.** The part's fitted C read on the training side's rows of five men or fewer
    against the tables' verdict at each row's own clock (adocs/data/S124_tb_probe.py's labels). A part that raises
    that loss by more than 1.645 whole-game errors is refused, whatever its d. pe's rows of six men or more have no
    such reading, and the stamp says so.
  - **S124's conversion test, P0's starts, on the candidate.** LONE is excluded, so nothing is expected. A signature
    converting fewer than on the landed tree by McNemar's one-sided z > 1.645 stops for the coordinator.
- **The cost.** S266_cost.sh's sink at the candidate. It decides nothing; the verdict carries it.
- **The SPRT.** One `{0, 5}` nElo of the candidate at 8+0.08, Hash 16, `books/noob_3moves.epd`, concurrency 12,
  against the landed tree (DEC-259 (b)).
  - DEC-143's worst case is 41861 games, 19.84 h at 2110 games an hour, at the midpoint; 25591 games, 12.13 h, on a
    bound. The cap is 20950 rounds, 41900 games, 19.86 h.
  - The abort rule is S101's: a time-forfeit rate past 1.0 % on either side, a crash or a disconnect, mains power
    lost, another load on the machine, a banner not the pinned one, or 40 h with neither bound.
  - An A/A of 1000 games comes first if the harness moved since the last.
  - The open DEC-171 findings at the run's HEAD are named by id.
- **On H0 or no verdict: no bisection.** A family of one, DEC-321 (4)'s precedent. H0 is recorded as zero and no
  verdict as unresolved; both revert to zero.
- **Machine hours, inferred and not timed.**
  - The identity rule and K are minutes.
  - P1's six fits: S122's screen's P1, eight linear weights, ran its six on 2026-11-04 in 28 m 31 s, the full fit
    stopping on patience at epoch 2400. About 30 min at patience is expected here, 3 h 45 m at the cap (2.25
    full-corpus fits of 20000 epochs at 0.30 s, S102's reading of that screen's log).
  - The guards are about 20 min, the cost about 30 min.
  - The SPRT is 12.1 to 19.9 h. So about 13 to 21 h in all, and 24 h at worst.

**Interactions.**
- **S124 (the central one).** Both act on E1 inside one division, and the census reads the overlap above.
  - Alone the term is mostly the cells; over them it adds -120.84 +/- 7.23 across halves at K*, about -86 of it
    surviving a phase recalibration of the other rows (-246.90 at K0, most of the difference K's).
  - **Neither undoes the other's sign**: f is boxed to [0, F] and adj never flips.
  - **The factor cannot undo complexity**: f scales toward zero, and a zero stays zero; at f_c = 0 complexity has no
    effect and no gradient in that cell.
  - **Complexity can undo the factor**: where C > (F / f_c - 1) |E1|, f_c adj(E1, C) exceeds |E1|. Under min(C, 0)
    neither undoes the other.
  - In the screen the cells are frozen at S124's fit, so C takes only what they left.
  - The tuner's product reaches C through r. The cell's pick and C's sign both follow E1's sign at the weights read,
    and adj preserves it, so they never disagree.
- **Mop-up.** Its terms sit in E1's pair, and LONE is excluded, so complexity never reads mop-up's domain. Its weights'
  gradients are unchanged.
- **S102's terms** sit inside E1. They move E1's magnitude and sign, and with them C's binding set. Its fits come
  before, frozen in this screen. The product reaches their endgame weights with r live w_eg, and space fires in
  every pawn structure P counts.
- **The clamp, the lazy margin, the truncation bound.**
  - Stage one, outside the clamp: |`evaluate` - cheap| <= the margin holds by construction, and DEC-324's
    re-derivation does not bind.
  - The cheap score moves, so which nodes take the shortcut moves: a play change the SPRT reads.
  - No new division, so the truncation bound is S124's.
  - Under Q2 (b) the term would enter stage two's clamped sum and DEC-324 would bind.
- **The gauges.** The seven held gauges stay null, E1 holding the material. The eight mobility levels (S121, c on a
  type's mobility row, -c on its table) move E1 by -c n_t and E2 by +c n_t. E1 alone is not invariant along their
  four endgame halves, so where C != 0 or a cell is off F those levels stop being null.
  - Every `--only` fit freezes one side, so the screen is untouched. With a cell off F or C != 0 no hold of a level is
    null, so S126's joint refit must freeze one side of each (finding (b)).
- **S129, tablebases.** A probe answers a five-man node exactly and supersedes the term there. It still acts at six
  men and more, and on a machine without the files.
- **The fifty-move decay**, a reserve candidate: a factor on the halfmove clock, which no key carries. This term reads
  the board alone, so a stored evaluation stays a function of the position; the decay, applied after, composes
  with it and cannot flip a sign either.
- **S099's pawn correction** learns the gap between search and raw evaluation in-game. A raw score moved in pawn-light
  endings leaves it a different gap: read by the SPRT, not argued.
- **S126** refits everything, this term included. It inherits the recalibration reading (finding (c)) and the mobility
  levels (finding (b)).

**Questions**, each in DEC-265's form; the draft carries them as proposals. **None is the owner's.** Each amends this
step's text or a coordinator's ruling, and no owner's decision. Reserved to the owner at the landing, as DEC-344 (9)
lists them for every evaluation family: `EXACT_FLOOR` under 157, `MATE_IN_THREE_FLOOR` under 11, or a mate-carry
ceiling rising by more than 2 (DEC-299, DEC-303, DEC-330, DEC-343). These are read only if the candidate moves them.

1. **The form: C free in sign, or min(C, 0). The coordinator's.** *Why:* the accepts says "the form the paper
   describes", which lets C > 0 widen. The goal says "toward zero". At K*, the screen's K, over the cells the free
   form reads 73.22 +/- 6.38 more (211.07 at K0); over the cells and a phase recalibration the two are 10.94 +/- 2.80
   apart.
   - *(a)* The paper's form. It reads -120.84 +/- 7.23 over S124's cells at K* (-246.90 at K0), about -35 of it a
     phase recalibration of the other rows. It widens on 66.5 % of rows. A sign change of E1 jumps the score by p50 9,
     p90 25, max 135 cp on the 7.6 % of rows with 0 < |E1| <= 25 (p50 19, p90 44, max 169 at K0). The identity seed
     is nondegenerate. The goal's "toward zero" is amended to "toward or away from zero, never past it".
   - *(b)* min(C, 0). It reads -47.63 +/- 4.23 over the cells at K* (-35.83 at K0), and -74.76 +/- 5.59 over cells and
     recalibration. It is continuous, as the goal says, and costs one more instruction (+1.415 % against +1.325 %).
     The seed sits on min's kink, so the fit needs the left derivative. The accepts' formula is amended to
     max(min(C, 0), -|E|): a stated departure from the paper.

   **Recommended (a), unmoved by the refit K:** the described form and the larger reading in the screen's setting,
   now mostly the term's own signal rather than recalibration. The jump is a property of the published technique,
   which the SPRT judges; at K* it is about half the K0 figure.
2. **Which E: E1, or E1 + E2. The coordinator's.** *Why:* the paper adjusts the whole endgame half, and chesso's has
   two stages, the second clamped.
   - *(a)* E1, stage one, S124's E. No clamp, every node sees it.
   - *(b)* E1 + E2. Over the cells it reads 143.09 +/- 5.77 better at K* (107.72 at K0), and it is invariant along
     the mobility levels. It needs stage
     two's endgame half before the clamp, the adjustment's correction inside the clamped sum, and DEC-324's margin
     re-derived. It is skipped at shortcut nodes.

   **Recommended (a), unmoved:** the price, larger at K*, buys a stage-two term under the clamp. It is recorded as a
   lead.
3. **The order with S124's factor. The coordinator's.** *Why:* the paper's order is complexity first. In joint fits,
   the cells moving with C, the other reads 21.90 +/- 1.98 better at K* (19.89 at K0); in the screen's setting, the
   cells held, the paper's reads 18.45 +/- 2.46 better (12.52 at K0).
   - *(a)* The paper's: f_c adj(E1, C).
   - *(b)* adj(f_c E1 / F, C): integer-exact too, F adj(f E1 / F, C) = f E1 + sign(f E1) max(-f |E1|, F C), a
     departure.

   **Recommended (a), strengthened:** the described form, and the better reading where the screen reads it.
4. **The accepts (4)'s instrument. The coordinator's.** *Why:* it names S082's fixed validation set, which DEC-349 (6)
   made history. *Amend* to "held-out loss on S122's regenerated corpus's own held-out qs view", DEC-350 (7)'s
   wording for S124. *Keep*: no new fit may read S082's view. **Recommended: amend.**
5. **LONE. The coordinator's.** *Why:* a lone king's rows are mop-up's, fitted on conversion. Their labels read 0.698
   against 0.978 predicted at K* (0.973 at K0), and a label fit learns to shrink them: R4 | F12 took -319 there at
   K* with C -583 (-318 and -569 at K0), and min(C, 0) with LONE reads -335.42 +/- 15.18 more than without it.
   - *(a)* No adjustment where a king stands alone. A departure from the paper, chesso's own. It costs 12.03 +/- 0.89
     of loss at K* (13.59 at K0) and two compares mop-up already makes.
   - *(b)* Include them, and let the tablebase guard refuse a part that learns those draws. The whole term then falls
     for one domain.

   **Recommended (a), unmoved.**
6. **The guards. The coordinator's.** *Why:* S124's accepts screens a barely-held case on tool labels, and pe is
   barely held in the instrument (904 games).
   - *(a)* The tablebase guard as a refusal, and the conversion test as a stop. About 20 min.
   - *(b)* The loss alone. 0 h, with pe's and LONE's behaviour unread.

   **Recommended (a).**
7. **On H0. The coordinator's.** *Why:* DEC-259 (c) asks for a partition, and this is a family of one.
   - *(a)* No bisection, DEC-321 (4)'s precedent.
   - *(b)* A = c0 and cP, B = cF and cE: about 20 h more for a split no reading asks for.

   **Recommended (a).**
8. **The tuner's chain rule. The coordinator's.** *Why:* with the old constants frozen only d/dc_i is read; S126 refits
   everything.
   - *All of it now*, the five derivatives and one finite-difference check over them.
   - *Frozen form*: d/dc_i alone, the tuner refusing a free old constant while C is free.

   **Recommended all of it now**, as DEC-350 (5) did.

**The accepts' amendment, drafted for the ruling.** (4) becomes: "the term passes DEC-259's offline screen --
held-out loss on S122's regenerated corpus's own held-out qs view below the model without it by the pre-registered
margin (DEC-349, DEC-350 (7)) -- before any match, and a term that fails the screen is recorded and not measured".
Under Q1 (a) the goal's "moves the endgame score toward zero" becomes "moves the endgame score toward or away from
zero, never past it"; under Q1 (b) (1)'s formula becomes "E + sign(E) * max(min(C, 0), -|E|)". Under Q5 (a) (1) gains
"except where either king stands alone".

**Findings beyond the step, not fixed here.**
- (a) **Grant 2020's section 3.7 as printed is not the derivative of its section 1.5**: sign(Γ) where the derivative
  is sign(E_eg), which differ on damped rows with E_eg > 0 and widened rows with E_eg < 0, White relative. And 3.4's
  E_eg = 0 case is the derivative only at Γ = 0. Nothing in `src/` or `tools/` implements any complexity derivative
  today. S262's step file cites the paper for "the derivatives ... for complexity". An implementer following the
  print would ship a wrong gradient that only the finite-difference check catches.
- (b) **E1 is not invariant along S121's four endgame mobility levels.** S124's research pass says "E1 is
  invariant" (its "What exists" and its draft's Context) of the material and table gauges, and that holds. Along a
  mobility level, c on a type's mobility row and -c on its table, c n_t moves between E1 and E2. So S124's factor
  makes those levels non-null wherever a cell is off F, and this term wherever C != 0. Nothing in S124's phase 1 is
  wrong, and `--only` fits freeze a side. What goes stale is the comment on `tools/tuner_groups.hpp`
  `mobility_levels_held` that a fit freeing both sides must first give the levels a hold of their own: with a cell
  off F or C != 0 no hold is null, so S126 must freeze one side.
- (c) **K moves when S124's cells land, and a reading at the corpus's K overstates any term on E1 that can widen.**
  - K0 = 1.2302 is the in-sample optimum at the shipped constants. It balances the cells' over-confident drawish rows
    against under-confident rows elsewhere.
  - Refitted at S124's cells, K reads 1.3474 after one round and 1.3578 at its fixed point, four rounds.
  - At K0 a phase-band recalibration of the rows outside the cells read -322.06 over F12, ratios 1.22 to 1.26 and 1.87.
    At K* it reads -80.06 +/- 7.53, ratios 1.09 to 1.14 below phase 21 and 1.43 at 21-23, where w_eg is at most
    0.125.
  - Without the cells the endgame half reads over-scaled at low phase (G6 0.694 at K0, 0.628 at K*; G1 0.905 and
    0.810), not under-scaled. This pass's first draft read the K0 figure as an under-scaled endgame half; that was
    wrong.
  - **What stays:** every later family screened after S124's cells land is read at a K well above 1.2302. A reading
    taken at the corpus's K may move when re-read there, as this term's did (-247 to -121). S102's research pass
    took its readings at 1.2302 too, not re-read here; DEC-351 already re-reads them on the landed tree, and this is
    a reason to expect them to move.
  - The residual recalibration, about -80 at K*, is S126's to take.
- (d) **The step's `touches:` lacks** what this design needs: `src/eval_trace.hpp`, `tools/tuner_trace.hpp`,
  `tools/tuner_layout.hpp`, `tools/tuner.cpp`, `tools/feature_audit.cpp`, the headers of `tools/truncation_scan.cpp`
  and `tools/eval_spread.cpp`, `tests/test_eval_trace.cpp`, `tests/test_eval_model.cpp`, `tests/test_walked_counts.hpp`,
  `tests/test_tuner_groups.cpp`, `tests/test_tuner_gradient.cpp`, `tests/test_eval_positions.hpp`. Its
  `decisions:` lacks DEC-349 and DEC-350.
- (e) The hinge form's loss has local optima: R4 | G6's full fit reads 46 above its own across-halves figure at K0.
  Adam from the seed may stop in one (inferred). The free form's fits agreed within 10 everywhere, at both K.
- (f) adocs/data/README.md's S102 rows sit in the main tree's index and this pass's rows were written at `2bf3ad4`
  without them. Both append at the file's end, so the landing has to order the two sections.

**Unmeasured or inferred, flagged.**
- No fit of the engine ran: every "the fit would" is inferred.
- The readings fit the term to the labels against the shipped score, in sample and across halves, on the training
  corpus at K 1.2302 and at K 1.3578. Held-out loss is the screen's, at the K its tuner fits.
- K* is one K for the whole corpus, refitted in sample; the across-halves readings fit no K per half.
- The recalibration's share is read against G6 and F12G6, controls, not a refit.
- The cost rows are a sink's instructions; LONE's exclusion is not in them.
- The search's distribution of the features and of E1 near zero is unmeasured, and so is the jump's effect on play.
- No part's Elo, no nps.

## 2026-11-05: the ruling, DEC-352

The research pass, revised after the coordinator's fast check, is ruled in DEC-352. Every item of the decision draft
is taken as written, (1) to (13), and every question takes the pass's recommendation:
- Q1 (a), the paper's form with C free in sign; the goal is amended to "toward or away from zero, never past it";
- Q2 (a), the term on E1, in stage one, inside S124's one division and outside the clamp;
- Q3 (a), the paper's order: complexity first, S124's factor after;
- Q4 (a), the screen's instrument is S122's regenerated corpus's own held-out qs view;
- Q5 (a), no adjustment where either king stands alone;
- Q6 (a), S124's tablebase guard a refusal, its conversion test a stop;
- Q7 (a), on H0 no bisection (DEC-321 (4));
- Q8 (a), the tuner's chain rule in full at phase 1.

**A lead, not built: the term on the whole endgame half, E1 + E2** (Q2 (b)). It reads 143.09 +/- 5.77 better over
S124's cells at K 1.3578 than the term on E1, more than the E1 term's own -120.84. It is the paper's own E. It is
not taken now because it sits in stage two, inside the clamp, unseen at the lazy shortcut's nodes, and DEC-324's
margin would have to be re-derived for it. It is read again after S266's SPRT, on the tree that verdict leaves.

**K.** Every reading the ruling rests on is at K 1.3578, refit at S124's twelve cells to convergence, with K 1.2302 in
brackets. The screen runs at the landed tree's own K.

**Order.** Phase 1 is built on S102's landed tree, the screen after S102's. Every reading here is re-read there.

**Unmeasured at this ruling:**
- every fit and held-out loss of the engine's tuner: the readings are the census's own fits, on the training corpus;
- the search's distribution of the features;
- cycles and nps: the cost rows are a sink's instructions;
- the term's strength.
