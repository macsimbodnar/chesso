id:         S100
goal:       find out why five evaluation terms fit to exactly zero -- feature extraction, corpus composition or a real result -- before any further weight is fitted beside them
accepts:    the step's product is the **diagnosis** -- which of the three causes below, with the evidence stated -- not a set of shipped weights; the feature extraction is verified by an independent count test per term (hand-counted positions, not the circular guard the body names); an extraction bug found here is fixed in this step under the house rule, with its own verdict; per-term fits and SPRTs are otherwise **deferred into the evaluation block**, where the corpus S082 rebuilds is the one they are fitted on -- fitting them early on a corpus the diagnosis may indict is the waste this re-scope removes (DEC-087); a verdict of unresolved is recorded as unresolved and not as zero, which is the distinction S027's tempo comment draws and the ledger row must keep; any term that stays at zero keeps its weights at zero so the compiler still deletes it, and the reason is updated rather than replaced; the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg and tempo_mg/eg, tools/ for the fit, adocs/testing.md
excludes:   new terms, which are S101 and S102; any change to how the terms are computed, only to their weights
decisions:  DEC-071, DEC-063
closes:
blocks:
paused_by:
done:

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
