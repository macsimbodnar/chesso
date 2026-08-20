id:         S100
goal:       find out why five evaluation terms fit to exactly zero -- feature extraction, corpus composition or a real result -- before any further weight is fitted beside them
accepts:    the step's product is the **diagnosis** -- which of the three causes below, with the evidence stated -- not a set of shipped weights; the feature extraction is verified by an independent count test per term (hand-counted positions, not the circular guard the body names); an extraction bug found here is fixed in this step under the house rule, with its own verdict; per-term fits and SPRTs are otherwise **deferred into the evaluation block**, where the corpus S082 rebuilds is the one they are fitted on -- fitting them early on a corpus the diagnosis may indict is the waste this re-scope removes (DEC-087); a verdict of unresolved is recorded as unresolved and not as zero, which is the distinction S027's tempo comment draws and the ledger row must keep; any term that stays at zero keeps its weights at zero so the compiler still deletes it, and the reason is updated rather than replaced; the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg and tempo_mg/eg, tools/ for the fit, adocs/testing.md
excludes:   new terms, which are S101 and S102; any change to how the terms are computed, only to their weights
decisions:  DEC-071, DEC-063
closes:
blocks:
paused_by:
done:      2026-08-20. Diagnosis, no weight shipped. Extraction, gradient and pipeline excluded for all six symptoms: 10795695 rows re-extracted for 0 disagreements, worst finite-difference error 4.9e-8 over all 827 parameters, a planted vector recovered at train error 0.000000 with 18 of 22 identified term parameters exact. Two symptoms are exact degeneracies, R^2 1.000000 with 0 violations in 1264773 and 550880 rows, and their splits are unidentified; the other four were never separately measured, three sharing one bundled SPRT at bounds that cannot resolve them and tempo reaching neither bound. Coverage refuted at 5.10 to 31.38 %, the correlation form recorded unresolved. One real defect found and fixed: three passed pawn middlegame parameters had no gradient from the test corpus at all. Fast suite 17 of 17 green, format clean, evaluation.cpp object code bit-identical.

## Why they are worth re-asking

Bishop pair, rook on an open file, rook on a half-open file, rook on the
seventh and tempo are in every hand-crafted engine in the 3000-plus band on the
CCRL list. Here all five ship at zero because they measured zero at S027, and
three things about that measurement have since changed:

1. **The corpus is not the same corpus.** `src/evaluation.cpp:194-198` says so
   itself -- the fit was on self-play by an engine predating all of S027. The
   corpus has since been regenerated (S065), deduplicated by zobrist key
   (S076), and S082 and S083 will move the label to the quiescence leaf and take
   it past 50 M positions.
2. **The search around them is about to change eleven times** (DEC-071). Eval
   parameters are only optimal relative to the search that uses them.
3. **Four of the five were measured as one block.** The comment at
   `evaluation.cpp:186-191` states the case for splitting the pair -- which is
   free and carried the largest fitted weight -- from three rook features that
   cost 3.1 to 4.0 % of a search between them, and then does not split it.

Tempo is the clearest: its SPRT reached neither bound, `-0.69 +/- 9.64` over the
full 3000 games. That is unresolved, not zero, and the code says so.


## Re-targeted 2026-08-19, and moved to the front

This was "re-examine the five terms against the current fit". It is now a
**diagnostic**, and it goes early rather than at position 84, because five
terms fitting to exactly zero is not a plausible outcome of a correct fit and
whatever causes it is also acting on the other 817 constants.

What ships at zero: bishop pair, rook on an open file, rook on a half-open
file, rook on the seventh, and tempo. Published fits of the same terms measure
bishop pair **+16.7**, rook and queen on the seventh **+9.2**, rook on an open
file **+4.2**, tempo **+12.99**. Zero for all five at once is a pattern, not
five results.

Three candidate causes, and the step's job is to tell them apart rather than to
pick one:

1. **Feature extraction.** The counts are wrong, so the fit is regressing on a
   column that does not mean what its name says. `piece_placement_counts()` is
   exposed for the test and the test can only check counts while the weights
   are zero -- which is a circular guard.
2. **Corpus composition.** The corpus is chesso's own self-play, and chesso
   values none of these terms at zero weight. An engine that never keeps the
   bishop pair on purpose generates few positions where keeping it
   discriminates. This is the documented failure mode for king safety and the
   same mechanism applies here.
3. **A real result** -- the tapered piece-square tables already absorb them.
   Possible, and it is the answer only after the first two are excluded.

`passed_pawn_mg` is the fourth data point: `{0, -6, -4, 19, 59, -17}`, with the
seventh-rank bucket fitted **below** the sixth and negative. Whatever explains
that probably explains the five.


## Technical details (SOTA research, 2026-08-19)

### Scope concern: no fit ever returned zero, and two symptoms are degeneracies by construction

**The zeros are a verdict plus a freeze, not a fit output**, and the diagnosis
must report against that. In-tree record: S027 fitted piece placement to mg
`{2, 32, 8, -26}` / eg `{55, -8, 12, 11}` and tempo to mg 10 / eg 0
(`src/evaluation.cpp:168-170,564-566`); the four placement features got **one
bundled SPRT** at `--fast` bounds (`elo0=0 elo1=10`), -5.48 +/- 11.46, H0, and
were reverted to zero **by hand** (plan_done/S027 §4-5); tempo's run reached
neither bound. Every fit since S065 carries `--freeze tempo,piece_placement`
(DEC-057; S076 rule 5), so the ten parameters were **held**, never fitted, at
zero. The one unfrozen fit on the current corpus — `.tuning/tuned_v2.hpp:185-190`,
never played, reverted for unrelated guards — returned mg `{25, 72, 25, -6}` /
eg `{93, -19, 35, 22}`, tempo 39/21. So a fourth candidate cause joins the
three: **measurement procedure** — a 4-feature bundle whose rook half cost
3.1-4.0 % of a search (~4.4-5.7 Elo at the published 1.43 Elo/%), judged under
bounds that cannot resolve +5 (DEC-063), plus a freeze nobody re-decided.
Goal/accepts unchanged; the verdict ledger should name this per term.

Second, provable without a fit: **two symptom columns are exactly redundant
with the PSQT.** Index 0 = a8 and black mirrors by xor 56, so (a)
`rook_seventh`'s differential equals the sum of the 8 rook-PSQT occupancy
columns for squares 8..15 (black rooks on 48..55 mirror to 8..15, sign -1);
(b) a pawn on its 7th rank is a passer **by definition** — "ahead" is the
enemy back rank, where the extraction refuses pawns (`tools/eval_model.hpp:597`)
— so passer bucket 5 equals the sum of the 8 pawn-PSQT columns 8..15.
Corpus-confirmed: 5923 of 5923 sampled 7th-rank pawns are passers. In a
**joint** fit those two weights are unidentified directions — the class the
tuner already documents for material/PSQT (`tools/tuner.cpp:36-40`) — so
`passed_pawn_mg[5] = -17` "below the sixth" is where Adam left an
unconstrained direction, not a statement about pawns. In a **frozen-base**
fit (`--only`) they are identified but fit only the residual the frozen PSQT
row leaves (`src/evaluation.cpp:50-59` already says so). Only the sums
`psqt[pawn][8..15] + pp[5]` and `psqt[rook][8..15] + placement[seventh]` are
identified; ledger rows and future SPRTs should be phrased on sums or
frozen-base residuals, never on the split.

### 1. State of the art: published causes of zero and degenerate weights

- **Extraction/label bugs.** Blunder's extractor "caused my tuner to load in
  every position as white having won the game" (algerbrex, t=78536); the
  checklist for a broken pipeline: "bug in the K calculation, or the MSE, or
  you parse the game result wrong, or you use non-quiet positions with static
  eval" (Ras, t=83196). Symptom: absurd K (0.05-0.1 against sane ~0.5-1.5),
  regressing tuned build. Chesso's K = 0.7624/0.7801, moving 2.7 % on a 1.9 %
  row change (S076), argues against a gross label bug, not a per-term one.
- **Harnesses.** "calculate the gradient for each term by finite differences
  and compare to the gradient calculated by the tuner" (Jon Dart, t=74877 p2).
  Recovery of planted weights from synthetic labels is the generic pipeline
  proof; no chess-specific write-up found — standard practice by construction,
  not a citation. Grant's tuner stores only non-zero coefficients (§4), so
  occurrence counts fall out of his design; chesso's dense arrays never
  produced them, and S027's v1 corpus had two king-safety features zero in
  **every** position — the in-repo precedent for counting first.
- **Corpus composition.** CPW's self-play caution: the engine "would learn to
  appreciate positional features that are correlated to increased winning
  chances, even though actively striving to reach such positions does not
  increase winning chances"; features the engine cannot use fit wrong (KBNK).
  Blunder saw mobility "driven from the current values to one or zero" and
  -153.6 Elo from human-game labels (t=78536). Antidotes, both S082's design:
  zurichess sampled 20 positions/game, each relabelled by a Stockfish playout
  (brtzsnr, t=61427); Grant resolves positions down a deep PV to a quiet leaf,
  trading label precision for diversity (§2.2).
- **Label quality.** Österlund's largest gain was *admitting* positions he had
  filtered (CPW); Grant filters only mate scores. A score-blend label (S075)
  is biased toward the current evaluator: a term the engine scores at zero
  gets no signal from its own search score, only from WDL — new-term refits
  want WDL-heavy labels.
- **Regularization.** Texel-family tuners descend unregularized MSE (Grant
  §2.4; Medvedev's regularized cross-entropy the noted exception, CPW
  Automated Tuning). Explicit L1/L2 pulls rare-feature weights to zero;
  without one the same look arises implicitly — chesso's list is in §5.

Hypothesis mapping, to test, not conclude: bishop pair / rook open / half-open
— measurement procedure first, composition second, extraction last (hand-count
tests exist); rook seventh and passer bucket 5 — exact PSQT redundancy, "real
zero of the residual" is the well-posed question; tempo — unresolved verdict
plus weak label-side identification (its WDL signal is the mean outcome gap
between white-to-move and black-to-move rows; datagen's in-check skip removes
rows correlated with the mover standing worse — a selection bias to measure);
passed-pawn shape — bucket-5 degeneracy plus documented frozen-base residual.

### 2. Shape for chesso

- Fit: `tools/tuner.cpp` — pure-MSE gradient, no weight decay (`gradient()`
  :424-561; placement :523-532, tempo :534-541); Adam from shipped constants
  (:1008); patience early-stop (:1133-1141); K fitted once vs WDL and held
  (:571-601, DEC-064); freeze mask (:913-946, `tools/tuner_groups.hpp:125`).
- Features, extracted once at load from FEN text, independent of the engine:
  `tools/eval_model.hpp` — `passed_pawn_features` :552-625,
  `piece_placement_features` :794-871, `tempo_feature` :883-897; row parse
  `tools/tuner.cpp:206-340` (side-to-move :227-235).
- Engine counts the model must agree with: `piece_placement_counts`,
  `passed_pawn_counts` (`src/evaluation.hpp:86,196`).
- Existing guards: `tests/test_eval_model.cpp` already holds per-colour
  hand-computed placement cases (:1015-1130), an engine-vs-model differential
  sweep (:933-958) and non-vacuity counts (:966-1000) — the body's "circular
  guard" sentence undersells what exists; the accepts' hand-count is partly
  done and needs auditing, not inventing. **Nothing tests the gradient or
  end-to-end recovery.**
- Corpus: `.tuning/selfplay_v2_dedup.tsv`, 10795695 rows (S065 + S076); fit
  logs `.tuning/tuner_v2*.log` for D0.

Occurrence counts, measured 2026-08-19, every 100th row (107956 rows, scratch
reimplementation from the `evaluation.hpp` definitions, self-checked on hand
positions). Differential nonzero: bishop pair **15.84 %**, rook open
**31.10 %**, half-open **31.37 %**, rook seventh **11.72 %**; passer buckets
0..5: 11.95 / 11.59 / 16.88 / 14.44 / 9.62 / **5.07 %**; white to move
50.26 %. No coverage desert — every column sits 5-31 % of rows, three orders
above starvation — so the coverage form of the corpus hypothesis is already
disfavoured for all six symptoms; the correlation-not-causation form remains,
and counts cannot decide it.

### 3. Implementation sketch: ordered protocol, pass/fail stated in advance

1. **D0 record audit** (no compute): per term, what "zero" is — S027 fitted
   values, freeze since S065, unfrozen v2 values. Pass: ledger distinguishes
   verdict-zero / frozen / unresolved per term.
2. **D1 extraction verification** (the accepts' independent count test): audit
   existing hand cases per term per colour; add tempo's corpus-path check and
   a dataset-path check — recompute features independently for >=10k corpus
   rows against what `load()` stored, catching column misalignment end to end.
   Pass: 0 disagreements. Fail: extraction bug — red test, fix here, own
   verdict (house rule).
3. **D2 finite-difference gradient check** (Dart): on a ~10k-row slice compare
   `gradient()` against `(E(w+h)-E(w-h))/2h` for all 22 term parameters plus
   psqt/material spot checks. Linear model in doubles: pass at relative error
   < 1e-6.
4. **D3 synthetic-label recovery**: copy a >=100k-row slice, overwrite the
   result column with `sigmoid(K * model_score)` at planted non-zero term
   weights (self-generated, no provenance issue), fit with the real tuner from
   the shipped start, freeze off. Pass: identified parameters recover planted
   values within integer rounding (+/-1); the two degenerate directions
   recover the **sums**; a frozen-base rerun recovers planted residuals
   exactly. Fail: pipeline defect — red test, fix, jumps the queue.
5. **D4 collinearity**: verify both exact identities on every corpus row
   (bucket-5 count == 7th-rank pawn count; seventh count == rook rank-row
   occupancy sum); regress each remaining term column on its overlapping PSQT
   columns, sampled. R^2 > 0.99: redundant on this corpus — a zero SPRT is
   then the expected output of a *correct* pipeline. R^2 < 0.9: independent
   signal worth a refit.
6. **D5 composition audit**: full-corpus occurrence counts (sampled table
   above re-run whole, minutes), reported per phase band, not only
   whole-corpus (S065's `phase <= 4` queen story is the precedent); tempo bias
   check — mean result of white-to-move vs black-to-move rows, beside S065's
   in-check skip counts. Data for S082's filter decisions.
7. **D6 verdict ledger**: one verdict per hypothesis per symptom, evidence
   cited, unresolved recorded as unresolved. No SPRT, no refit, no weight
   ships; the compiler-deletion property stays checked.

### 4. Constants and seeds (order-of-magnitude references only, DEC-084/DEC-019)

Confirmed this session (Elo at each engine's own STC; seeds for nothing —
they calibrate the evaluation-block refits): tempo **+12.99 +/- 7.35** (Weiss
commit 8bf33fde97, "Tempo (#241)"); bishop pair **+8.2 +/- 6.1** STC / +9.5
LTC (Lynx PR #390); rook+queen file bonuses retune **+9.86 +/- 5.74** (Weiss
"Tune Rook+Queen (#231)"). The body's +16.7 / +4.2 / +9.2 came from the
2026-08-19 review's ledger read; their commit URLs were **not re-located**
this session — re-verify before citing further. Stash's changelog confirmed
the adjacent plan figures (+22.27 passers with king proximity, +25.38
connected). Centipawn seeds, if wanted later, come from open literature only
(e.g. Kaufman's imbalance article) and are refit here (DEC-084).

### 5. Pitfalls

- **The corpus is about to be replaced** (S082 relabels at the resolved leaf,
  S083 re-sizes). D1-D3 are corpus-independent and survive; D4's exact
  identities are definitional and survive; D4's R^2 and all of D5 describe
  `selfplay_v2_dedup.tsv` only and are re-run inside S082/S083 — build them
  as tracked tools, not scratchpad scripts (the S072 lesson).
- **Do not fix the tuner and refit in one step.** Refits and SPRTs live in the
  evaluation block (DEC-087(g)); the one exception is an extraction bug under
  the house rule. A refit on the outgoing corpus is the waste the re-scope
  removed.
- **Implicit regularization masquerading as collinearity.** No explicit
  regularizer here, but three implicit ones: Adam starts at the shipped value
  and patience stops early, so a weight whose gradient is absorbed by
  faster-moving collinear PSQT columns stays near its start; `--freeze` is an
  infinite regularizer and is ON for exactly these ten parameters in every
  shipped fit; `write_tables` rounds via `lround`, so |w| < 0.5 ships as 0.
  D3 with freeze off, run past patience, separates optimizer inertia from
  true redundancy.
- **Reading fitted signs as chess** (DEC-023): bucket-5's -17 is an
  unidentified direction; report identified sums, and published figures decide
  what to try, never what to conclude (DEC-019).

### 6. Measurement

This step owes: a written verdict per hypothesis per symptom with D0-D6
evidence, recorded in this file; the fast suite green; any extraction or
gradient defect gets a red test before its fix and its own verdict — a defect
here contaminates every fit taken after it. **No SPRT**: nothing play-altering
ships; terms stay at zero weight, the compiler still deletes them
(`evaluate_cheap()` instruction-count check exists). Unresolved is recorded as
unresolved, distinct from zero — tempo's S027 row is the template.

### 7. Interactions

- **S082/S083 consume D4/D5**: occurrence and collinearity tooling become
  corpus-preparation gates; the label note (score-blend labels cannot teach a
  zero-weighted term) feeds S082's lambda choice.
- **S121-S126 are gated on the diagnosis**: per-term refits and SPRTs happen
  there, on the S082 corpus; the S027 candidate split — bishop pair alone,
  compile-free, largest fitted weight — is the first refit shape. S133
  (king-relative PSQT) re-shapes but does not remove the rank-row
  degeneracies: any PSQT with per-square pawn/rook entries keeps them.
- **S084/S085 (SPSA) are adjacent, not coupled**: they sweep search
  parameters; nothing here blocks them, nothing there re-decides a weight.
- S105's cheaper harness is what makes deferring the per-term SPRTs
  affordable.

### 8. References (read 2026-08-19)

- https://www.chessprogramming.org/Texel%27s_Tuning_Method — objective, K, filter reversal, correlation caution, KBNK, degenerate values.
- https://raw.githubusercontent.com/AndyGrant/Ethereal/master/Tuning.pdf — Grant's published paper (no repo code read): model, K absorption, §2.2 PV-resolved dataset, §2.4 L2, §4 sparse coefficient tracing.
- https://talkchess.com/forum3/viewtopic.php?f=7&t=74877 (+ &start=10) — paper thread; Jon Dart's finite-difference debugging quote.
- https://talkchess.com/viewtopic.php?t=83196 — near-zero K as pipeline-bug symptom; Ras's diagnosis checklist.
- https://talkchess.com/viewtopic.php?t=78536 — Blunder datagen experiments: white-won label bug, mobility to zero/one, human-label noise, zurichess comparison.
- https://www.talkchess.com/forum3/viewtopic.php?t=61427 — zurichess quiet-labeled.epd generation (20 positions/game, Stockfish playout labels).
- https://www.chessprogramming.org/Automated_Tuning — Medvedev regularization mention, Althöfer small-set quote.
- https://github.com/TerjeKir/weiss — commit messages 8bf33fde97 (Tempo #241, +12.99), b95caf73d1 (Tune Rook+Queen #231, +9.86); no source read.
- https://github.com/lynx-chess/Lynx — PR #390 body (bishop pair +8.2/+9.5); no source read.
- https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md — the Stash ledger entries the plan cites; prose only.
author:    Maksym Bodnar


## The diagnosis, 2026-08-20

The step's product. Six symptoms, four candidate causes, one verdict per pair,
with the evidence beside it. Unresolved is recorded as unresolved. **Nothing
play-altering ships: all ten frozen parameters stay at zero and the compiler
still deletes both terms.** No SPRT was run and none is owed.

Headline: **the pipeline is exonerated and the pattern has two different
explanations, not one.** Two of the six symptoms are exact algebraic
degeneracies and their zeros carry no information at all. The other four were
never separately measured — three of them shared one bundled SPRT under bounds
that cannot resolve their published effect size, and the fifth reached neither
bound. One real defect was found, in the test corpus rather than in the fit, and
fixed here.

### D0. What "zero" is, per term

Not one thing. Three different states wear the same value.

| symptom | S027 fit | S027 verdict | since S065 | unfrozen on the current corpus |
|---|---|---|---|---|
| bishop pair | mg 2, eg 55 | one bundled SPRT, −5.48 +/− 11.46, H0 | frozen | mg 25, eg 93 |
| rook open | mg 32, eg −8 | same bundle | frozen | mg 72, eg −19 |
| rook half-open | mg 8, eg 12 | same bundle | frozen | mg 25, eg 35 |
| rook seventh | mg −26, eg 11 | same bundle | frozen | mg −6, eg 22 |
| tempo | mg 10, eg 0 | **unresolved**, −0.69 +/− 9.64, LLR −1.46 vs −2.20 | frozen | mg 39, eg 21 |
| passer bucket 5 | — | never separately measured | **not frozen, fitted** | see below |

So four of the six carry a bundled H0, one carries no verdict at all, and all
five have been **held** by `--freeze tempo,piece_placement` in every fit since
S065 (DEC-057) rather than fitted to zero. The zeros were never a fit output.

**A record correction.** The body reads `passed_pawn_mg[5] = −17` as a fit
result. It is one of three, and the three do not agree:

| bucket | S065 unfrozen | S065 frozen, re-anchored | S076, shipped | spread |
|---|---|---|---|---|
| 0 | −4 | −2 | 0 | 4 |
| 1 | −8 | −6 | −6 | 2 |
| 2 | −1 | −1 | −4 | 3 |
| 3 | 22 | 22 | 19 | 3 |
| 4 | 58 | 61 | 59 | 3 |
| **5** | **+22** | **−1** | **−17** | **39** |

Five buckets stable to within 4 across three fits; the sixth moves by 39 and
changes sign twice. That is a parameter nothing constrains, and D4 says why.
The shipped vector is S076's fit on the deduplicated corpus (commit `77d7450`),
not S065's — `.tuning/` holds S065's emitted headers and S076's is gone, which
is the `.tuning/` durability item already parked in `status.md`.

### D1. Feature extraction — **refuted for all six**

- **Corpus path.** `build/tools/feature_audit` re-extracts all 27 feature
  columns from the FEN text of every row and compares against what `load()`
  stored: **10795695 rows, 0 disagreements** (`adocs/data/S100_feature_audit.txt`).
- **Dataset path.** `tests/test_tuner_gradient.cpp` holds the same comparison
  per column, per row, plus the piece list, the offsets and the phase, as a fast
  test. This is the guard that did not exist; the whole loader was unreachable
  from `tests/`.
- **Hand cases.** The accepts asked for an independent count test per term. The
  audit found the existing ones adequate rather than circular: per-colour
  hand-computed placement cases at `tests/test_eval_model.cpp:1015-1130`, an
  engine differential sweep at `:933-958`, and non-vacuity counts at `:966-1000`.
  The body's "circular guard" sentence undersold what was there. Tempo has no
  count to check and never did — what stands in for it is the corpus carrying
  both sides to move, which is asserted.

None of this checks what a feature *means*: the re-extraction calls the same
extractors, so a wrong definition agrees with itself. The engine differential and
the hand cases are what hold the definitions, and they are green.

### D2. The gradient — **refuted, and it found a defect**

`gradient()` against central differences of `error_range()` for **all 827
parameters**: worst relative error **4.9e-8** against a 1e-6 tolerance. `h = 0.01`
was measured rather than picked — the sweep traces the O(h²) truncation law down
to the cancellation floor near 0.005, and the first draft at h = 0.5 failed at
5.7e-4 on `mobility_eg[2]` for exactly that reason.

The one place `gradient()` is deliberately not the derivative is the lazy clamp,
and that is now pinned from both sides: at the shipped constants the clamp binds
on no fixture row, and with the king safety weights pushed +40 it binds on 3 of
34, where the clamped block disagrees by 1.61 while everything outside it stays
at 1.5e-7. The six symptoms all live outside the clamp.

**The defect.** Non-vacuity — a parameter with no gradient is confirmed for
free — turned up red on `PP_MG[2]`, `PP_MG[4]` and `PP_MG[5]`. Every position in
the curated test corpus reaching passer buckets 2, 4 or 5 is a bare-pawn endgame
at phase 0, so `mg_weight` is exactly 0 there and three of the twelve passed pawn
parameters had no gradient at all — while every *count* column was covered, so no
count test could see it. Four positions were added to
`tests/test_eval_positions.hpp`, taking the term parameters to 54 of 54 and the
occupied piece-square columns from 149 to 158. It is a test-corpus defect, not a
fit defect; the real corpus reaches those buckets at every phase (D5).

### D3. End-to-end recovery — **refuted, on every clause**

Labels replaced by `sigmoid(K * model_score)` at planted non-zero weights over a
203693-row strided slice, then fitted by the real `tuner` from the shipped start
with freeze off.

The joint fit reached **train error 0.000000** and recovered **18 of the 22
identified term parameters exactly**: `passed_pawn_mg` planted
`{7, 14, 21, 33, 48, 66}` came back `{7, 14, 21, 33, 48, 24}`,
`piece_placement_mg` `{28, 17, 9, 13}` came back `{28, 17, 9, 5}`, tempo 23/12
came back exactly.

**Every single deviation is a documented null direction.** The four that miss are
the two seventh-rank ones D4 proves. The tables came back uniformly shifted —
pawn −5 per square against `PAWN +5`, rook −3 against `ROOK +3`, the
material↔table degeneracy `tools/tuner.cpp:36-40` already names — so the
seventh-rank **sums** are exact to the unit once that shift is added back, which
is precisely the pass criterion this step set in advance.

Frozen-base runs confirm the mechanism: `--only passed_pawns` recovers all twelve
**exactly, bucket 5 included**, because holding the table resolves the
degeneracy. `--only tempo` exact. `--only piece_placement` within ±1 on 3 of 8 at
residual 0.000003, patience stopping the run one unit short.

So a zero this pipeline returns is a result about the data, not an artefact.

### D4. Collinearity — **confirmed exactly for two symptoms, refuted for four**

Two exact identities, proven from the indexing and then checked on every row
rather than argued. Index 0 is a8 and a black piece mirrors by `^56`, so a rook
or pawn on its own seventh rank always lands on squares 8..15:

| identity | rows non-zero | violations |
|---|---|---|
| rook seventh == signed rook occupancy of 8..15 | 1264773 | **0** |
| passer bucket 5 == signed pawn occupancy of 8..15 | 550880 | **0** |

R² of each term column on the tables that could absorb it, 203693 rows. The rook
file features are regressed on **rook + pawn** columns, not rook alone, because
an open file is a statement about the pawns as much as the rook:

| column | R² | on |
|---|---|---|
| **rook seventh** | **1.000000** | rook, 64 columns |
| **passer bucket 5** | **1.000000** | pawn, 64 columns |
| passer bucket 4 | 0.620476 | pawn, 64 |
| bishop pair | 0.486761 | bishop, 64 |
| passer bucket 2 | 0.290187 | pawn, 64 |
| passer bucket 3 | 0.270118 | pawn, 64 |
| rook open | 0.265313 | rook+pawn, 128 |
| passer bucket 0 | 0.215745 | pawn, 64 |
| passer bucket 1 | 0.201678 | pawn, 64 |
| rook half-open | 0.168362 | rook+pawn, 128 |

The two 1.000000 rows are the regression validating itself against algebra
already proven. Everything else is far under the 0.99 the step set as the
redundancy threshold, on the *widest* right-hand side the model can offer.

**Consequence, and it is the operative one.** For rook-on-the-seventh and passer
bucket 5 only the **sums** `psqt[rook][8..15] + placement[seventh]` and
`psqt[pawn][8..15] + pp[5]` are identified. A joint fit's value for either is a
position on a ridge, chosen by the optimiser's path — which is exactly the 39-point
spread in D0's table, and why `passed_pawn_mg[5] = −17` is not a statement about
pawns and must not be read as one (DEC-023). Reading it as one is the mistake the
body was about to make.

### D5. Corpus composition

**Coverage form — refuted for all six.** Every column is non-zero on 5.10 % to
31.38 % of rows, three orders above starvation, and the per-phase-band table in
`adocs/data/S100_feature_audit.txt` has no desert either: the thinnest live cell
is bishop pair at 2.73 % in band 1–4, and the zeros in the phase-0 column are
structural — a board with a bishop or a rook on it is not at phase 0.

**Correlation form — unresolved, and recorded as unresolved.** The corpus is
chesso's own self-play and chesso values four of these terms at zero weight; a
feature the engine never plays for generates positions where it does not
discriminate. Neither counting nor collinearity can decide that, and nothing here
does. It is the documented failure mode (CPW's self-play caution, Blunder's
mobility) and it is what S082's relabel at the resolved leaf and S083's rescale
are for.

**Tempo's label-side signal, measured.** Tempo's whole WDL signal is the gap
between the mean outcome of white-to-move and black-to-move rows:

```
5437085 rows White to move, mean result 0.556559
5358610 rows Black to move, mean result 0.548682
gap 0.007878, White relative
```

At the corpus mean (0.552649) and K = 0.7624 the sigmoid slope is 0.00108502 per
centipawn, so that gap corresponds to a **7.26 cp** score difference between the
two groups and a tempo weight near **3.6 cp**. Confounded — the groups differ in
more than whose move it is, and `datagen` skips in-check positions, which removes
rows correlated with the mover standing worse — so this is the size of the signal
available, not a prediction. Against it, S027's `--only tempo` fit produced mg 10
and the unfrozen S065 fit mg 39: a fourfold disagreement on a term whose label-side
signal is a few centipawns.

### D6. The verdict ledger

| symptom | extraction | gradient / pipeline | coverage | collinearity | measurement procedure | verdict |
|---|---|---|---|---|---|---|
| bishop pair | refuted | refuted | refuted, 15.81 % | refuted, R² 0.487 | **confirmed** — one bundled SPRT at `--fast` bounds against a published +8.2 | **never measured.** The strongest refit candidate: identified, not redundant, compile-free, largest fitted weight |
| rook open | refuted | refuted | refuted, 31.38 % | refuted, R² 0.265 | **confirmed** — same bundle | **never measured**, and its speed cost is real (3.1–4.0 % shared with the two below) |
| rook half-open | refuted | refuted | refuted, 31.26 % | refuted, R² 0.168 | **confirmed** — same bundle | **never measured** |
| rook seventh | refuted | refuted | refuted, 11.72 % | **confirmed exactly**, R² 1.000000, 0 violations in 1264773 rows | confirmed — same bundle | **not identified.** Its split from the rook table is a ridge position. Only the sum means anything |
| tempo | refuted | refuted | n/a, both sides present | n/a, not a board feature | **confirmed** — reached neither bound | **unresolved, not zero**, as S027 recorded. New: the label-side signal is ~3.6 cp, and two fits of it disagree fourfold |
| passer bucket 5 | refuted | refuted | refuted, 5.10 % | **confirmed exactly**, R² 1.000000, 0 violations in 550880 rows | never measured alone | **not identified.** −17 is a ridge position, not a valuation. Three fits span 39 points where every other bucket spans ≤ 4 |

**Cause 1 (extraction) and the pipeline are excluded for all six.** Cause 2's
coverage form is excluded for all six; its correlation form is open for all six
and is S082/S083's to answer. Cause 3 (a real zero) is the answer for **none** of
them yet, because for four the measurement never happened and for two the
quantity measured was not identified. Cause 4, measurement procedure — the fourth
candidate the technical section added — is **the operative cause of the pattern**.

### What follows, for the steps that consume this

1. **The freeze has to be re-decided.** `--freeze tempo,piece_placement`
   (DEC-057) rests on S027's verdict, and that verdict is procedural. A decision
   is owed; this step does not take it and ships nothing.
2. **Two parameters may never be fitted jointly and read as values.** Any refit
   of rook-on-the-seventh or passer bucket 5 is frozen-base (`--only`), and any
   ledger row about them names the sum or the frozen-base residual. S133's
   king-relative tables re-shape the degeneracy without removing it: any table
   with per-square pawn and rook entries keeps it.
3. **The first refit is bishop pair alone**, which is S027's own untested
   candidate and now has the evidence behind it. Then the two rook file features,
   whose speed cost has to be inside their own verdict.
4. **Bounds, not effort.** Published seeds for these are +8.2 and +9.86 (DEC-084,
   order of magnitude only). `--fast` at `elo0=0 elo1=10` cannot resolve that, and
   DEC-063 is the measurement: the same constant took 6 h 36 m for no verdict at
   one bound pair and 1 h 41 m for H1 at another.
5. **S082 gets two inputs.** The per-phase occurrence table and the tempo gap are
   corpus-preparation evidence, and both re-run on the new corpus with the same
   tool. And the label note stands: a score-blend label cannot teach a term the
   current evaluator scores at zero, so a refit of these six wants WDL-heavy
   labels.
6. **`feature_audit` and `test_tuner_gradient` are the durable products.** The
   corpus is about to be replaced; D1–D3 are corpus-independent, D4's identities
   are definitional, and D4's R² and all of D5 are one command on whatever corpus
   S082 and S083 produce.
