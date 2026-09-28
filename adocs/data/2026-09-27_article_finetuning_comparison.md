# Parameter tuning: the Medium article against chesso's record — 2026-09-27

What one published account of Texel tuning says, what chesso has done and
measured under the same heading, what it still plans, and what — if anything
— the article adds. Written as an analysis; it changes no step, no decision
and no document. Companion: `2026-09-27_nnue_training_preparation.md` covers
the article's network training and the literature around it.

---

## 0. Provenance and scope

Read on 2026-09-27: the Medium article at
https://medium.com/@alan0408yuan/from-texel-tuning-to-nnue-the-steep-learning-curve-of-building-a-3200-elo-chess-ai-d842393b5955
(title on the page: *The Steep Learning Curve of Training a Dual-Perspective
HalfKA NNuE for a 3000+ ELO Chess AI*, by Alanyuan, 20 min read, shown as
published six days before the read, so about 2026-09-21; the URL slug still
says "3200-elo"). Medium answers every fetch tool with HTTP 403; it was read in
the browser pane, whole, 34620 characters, and the last paragraph on the page
is the one on king bucketing. Also fetched: the project's `README.md` and its
repository landing page (licence line only). **No source file, no table and
no data of that project was opened** (DEC-016). Every quotation below is from
the article or the README as read that day.

Nothing here is a chess judgement. It is a comparison of methods and of
recorded measurements.

---

## 1. What the article is

- **The engine.** "AlanBot Chess AI": a Rust compute core behind a Python
  (PySide6) interface, repository `alanyuan08/Chess-App`, MIT licence, 336
  commits, 4 stars at the read. The README claims "17+ million nodes per
  second on Apple M4 Pro" and "14+ depth on 15 second search".
- **The strength claim.** The README says the engine was "tested against ELO
  3200+ Chess.com bots" with two documented wins and that it "has not been
  officially ratified by Computer Chess Rating Lists". **No SPRT, no rating
  list entry and no self-play match appears anywhere in the article or the
  README.** The 3000+/3200 in the title is a claim, and under DEC-019 it
  decides nothing.
- **Structure**, in page order: Background; Summary; Hand-Crafted
  Evaluation; Texel Tuning; Self-Play Mechanics; Texel Era Discoveries; NNuE
  Revolution; Optimizing Neural Network Compute; Efficiently Updatable;
  Dual-Perspective HalfKA Model; Implementation; NNuE Training; Lichess Data
  Preparation; NNuE Knowledge Distillation; Dual-Perspective HalfKA Model
  Improvements. The summary promises "a roadmap for future NNuE models"; the
  page ends at king bucketing, and the "Reinforcement Learning" phase is
  described as a method, never reported as run.
- **What it measures.** One number family: a validation loss of "35.3
  Centipawn error on positions between +/- 350 Centipawn Score and 46.8
  Centipawn Error" for the distilled network. Nothing about the tuned
  hand-crafted evaluation is measured at all.

---

## 2. The tuning method the article describes, as printed

The section is short and is quoted rather than paraphrased, because the
comparison in section 4 rests on its exact wording.

- **Data.** "The Chess Engine would play 100k+ games against itself and would
  generate roughly 1.5 to 3 million unique positions." Generation runs "at
  low-depths (e.g., Depth 10)"; "the algorithm randomly forces 10% or more of
  the searches to operate at an even lower depth (e.g., Depth 5) — These
  poisoned positions are excluded from Gradient Training." Positions must
  have "no active checks, no pending profitable captures and no known forced
  mate sequence".
- **Label.** A per-position win rate: "Number of Wins * (1.0) + Number of
  Draws * (0.5) * Number of Losses * (0.0) / Total Number of Games" (the
  formula as printed multiplies where it should add). Texel's method as
  published labels each position with the result of *the one game it came
  from* (CPW *Texel's Tuning Method*); a per-position win rate over several
  games exists only for positions repeated across games.
- **Score to probability.** The CPW pawn-units-to-win-percentage curve, and
  the statement that "a coefficient of 0.67 (or its scaled equivalent
  depending on the internal pawn unit normalization) is utilized" in
  Stockfish. **Unverified**: no fetched source names 0.67; the constants the
  Stockfish training documentation states are a sigmoid scaling of about 400
  in centipawn space (section 8, nnue-pytorch `docs/nnue.md`).
- **Update rule.** "New Weight = Old Weight + (LEARNING_RATE *
  Abs(Sigmoid(Score) — WinRate)", called "conceptional Linear Gradient of
  Descent", run "several thousands of times with a decreasing learning rate".
  As printed the rule has no gradient direction and no feature term: it moves
  every active weight up by the absolute error.
- **Where the objective should concentrate.** An "irreducible baseline error
  of roughly 2–3%", so the "Target Capacity Optimization Window" is "-2.0 to
  +2.0 Pawns".
- **Depth against accuracy.** A table: "+1 Depth Value in Centipawn" of
  "+50cp to +100cp" at depth 10, "+20cp" at 15, "+5cp" at 20, "~0cp" at 25+,
  offered as "rough assumptions" and "an ideal starting heuristics".
- **The seven "Texel era discoveries".** (1) a piece's value depends on both
  kings' squares; (2) the tapered evaluation is a proxy — "correlation is not
  causation"; (3) the depth-versus-accuracy trade above; (4) the pool need not
  contain extreme scores; (5) one evaluation from the side to move's
  perspective with the board rotated; (6) mirroring the board "to produce a
  second data set"; (7) keep "quiet, tactically stable positions".
- **Verdict on the method.** "This method is obsolete in the NNuE era".

---

## 3. chesso's tuning record

### 3.1 Done, with verdicts

| step | date | what | verdict |
|---|---|---|---|
| S028 | 2026-08-11 | first Texel fit: 773 constants over 1490839 self-play positions (20000 games, 100000 nodes a move, 8 random opening plies), `K` 1.1141 fitted then held, full-batch Adam at learning rate 1.0, 10 % held out | **+188.74 +/- 32.21** over 438 games at 10+0.2, H1 |
| S065 | 2026-08-14 | corpus regenerated by the S028 engine: 120000 games, 100000 nodes a move, 12 threads, about 8 hours, 11.0 M rows, the tactical clause loosened (`--allow-tactical 1`, DEC-055); 827 constants refit | **+21.10 +/- 10.47** over 3396 games, H1 |
| S066 | 2026-08-14 | held-out set is whole games, the boundary reconstructed from the FEN (DEC-056) | held-out error differs by 2.8e-05, recorded as **zero**; kept because a row-level split shares games by construction |
| S075 | 2026-08-17 | `--lambda`: fit against `lambda * sigmoid(K * score) + (1 - lambda) * result` | **every non-zero lambda is worse** on held-out game-result error, 0.118530 to 0.119213 against 0.118457 at a 4e-05 noise floor; ships at 0; two rules recorded as DEC-064 |
| S076 | 2026-08-17 | dedupe by the engine's zobrist key: 207998 of 11003693 rows, 1.89 % | refit **+26.68 +/- 16.40** over 1044 games, H1 (DEC-065) |
| S077 | 2026-08-17 | every emitted header names the tuner's commit, the corpus SHA-256, its rows and bytes | no verdict owed (DEC-066) |
| S084 / S085 | 2026-08-20 / 21 | chesso's own SPSA driver; first run over 12 search parameters, 60000 games in 8 h 21 m | independent SPRT **+21.02 +/- 9.86**, H1 in 2946 games at 8+0.08 on a book and a control the run never saw; `RfpMinPly` came back 0 and did not ship, 3 of 18 mate cases red |
| S100 | 2026-08-20 | the five zero-weight terms re-examined: extraction, gradient and pipeline excluded as causes | diagnosis only; two exact degeneracies found, folded by S134 |
| S222 | 2026-09-15 | one-ply continuation history with its scale fitted in a narrow SPSA lane, plain history's coefficients beside it | **+11.13 +/- 6.90**, H1 in 6278 games |
| S231 | 2026-09-20 | two-ply table, its own lane | H0, **-2.65 +/- 4.82**; reverted (DEC-224) |

Elo figures are self-play at the harness's regime, not list Elo. S240
(2026-09-27) puts the whole engine at about 2766 on the CCRL Blitz scale,
against 2559 at S088.

### 3.2 What the tuner is

From `tools/tuner.cpp`, `tools/eval_model.hpp`, `tools/tuner_target.hpp`,
`tools/datagen.cpp` and `adocs/eval_tuning_strategy.md`:

- **Objective.** Mean squared error of `sigmoid(K * evaluate())` against the
  game result of the row's own game, sigmoid in the base-10 Elo form over
  400. `K` is fitted once against the outcome and held for the run
  (DEC-064), refitted per corpus: 1.1141 at S028, 0.7595 to 0.7801 on the
  S065 corpus and its dedupe.
- **Model.** The evaluation is linear in its 827 constants (823 once S134
  folds the two degeneracies), so the gradient is closed form. The tuner
  never calls the engine: `tools/eval_model.hpp` is a second, floating-point
  implementation of `evaluate()` that emits the sparse feature vector, and
  `tests/test_eval_model.cpp` holds it against the engine on every phase
  (S186 records this as a decided departure from the strategy document's
  in-engine trace mode).
- **Optimiser.** Full-batch Adam, learning rate 1.0, early stopping on the
  held-out game-result error with patience; deterministic under `--seed`, so
  an emitted vector is re-derivable from its own stamp. Full-batch is a
  recorded departure from the mini-batch recommendation: with cached
  features an epoch over 11 M rows is 0.40 s (S075), so the reason for
  mini-batching does not apply.
- **Groups.** `--only` and `--freeze` over named parameter groups, which is
  how tempo and piece placement were held at zero (DEC-057) and how a single
  term gets its own verdict (S135, S136).
- **Data.** `tools/datagen`: self-play at a fixed node budget from 8 random
  opening plies, openings discarded past 400 cp, adjudication at 2000 cp
  held for 6 plies, positions skipped when the side to move is in check, when
  the search found a mate, when the score is beyond `--quiet-limit` 1000 and,
  unless `--allow-tactical`, when the chosen move is a capture or a
  promotion; every clause's cost is printed. Columns `fen result score
  phase`; the score column is the engine's own search score, written for
  re-filtering and for the blend. Dedupe by zobrist key (S076); the corpus
  gitignored and archived with its recipe and digests (DEC-183).
- **What decides.** Held-out error ranks candidates; exactly one goes to an
  SPRT; a verdict of zero is recorded as zero (DEC-019, DEC-064). Search
  parameters are not fitted on a corpus at all: they go through SPSA on
  games with an independent SPRT of the returned vector (S085's shape),
  now one lane per completed block (DEC-222).

### 3.3 Planned, in plan order

- **S134** folds the two degenerate columns, parameter count 823.
- **S082** labels the position quiescence resolves to instead of the root,
  retires the tactical clause (the published method's author withdrew the
  same filter), and samples few rows a game as chesso's own hypothesis.
- **S083** decides corpus size and the generation node budget by a held-out
  curve under a budget stated in nights, holding one of the two fixed; the
  sourced corpora for hand-crafted evaluations span 4.5 to 10 M positions.
- **S135 / S136** unfreeze piece placement and tempo, one verdict each at
  bounds that resolve single digits.
- **S121 to S133** rebuild terms and fit every constant of each; S133 makes
  the piece-square tables king-relative with a small bucket scheme chosen by
  held-out error.
- **S126** refits everything once the search stops moving: the sourced
  ledger for a full retune is seven of Texel's, 2.8 to 39.4 Elo, median
  10.2.
- **S127** the full SPSA run after the search block, last of the per-block
  cadence.

---

## 4. Side by side

| dimension | the article | chesso, done | chesso, planned |
|---|---|---|---|
| data source | own self-play | own self-play only (DEC-002, DEC-016) | same |
| volume | "100k+ games", "1.5 to 3 million unique positions" | 120000 games, 11.0 M rows, about 92 a game; 1.49 M at S028 | S083 decides on a held-out curve; density is S082's hypothesis |
| search budget per move | "Depth 10" | fixed **nodes**: 100000 used, 5000 the default | S083 trades nodes against games at fixed compute |
| opening diversity | none stated | 8 uniform random plies, discarded past 400 cp | unchanged |
| mid-game randomisation | 10 % of moves at a lower depth, those positions dropped | none | not planned; see section 6 |
| filters | no checks, no profitable captures, no forced mate | not in check, no mate score, |score| <= 1000, tactical clause optional since DEC-055 | S082 records the quiet leaf instead of filtering the root |
| label | a per-position "WinRate" | the result of the row's own game | unchanged; blend measured worse (S075) |
| score to probability | CPW curve; "0.67" | base-10 sigmoid over 400 with `K` fitted per corpus on the outcome and held | unchanged |
| objective | absolute error, as printed | mean squared error on the sigmoid | unchanged |
| optimiser | "several thousands" of passes, decaying rate | full-batch Adam, learning rate 1.0, early stopping on held-out error | unchanged (recorded departure) |
| parameters | "8000+" rules in the Deep Blue framing; own count not stated | 827 named constants in groups | 823 after S134, growing per term step |
| validation | not described for the tuned evaluation | 10 % held out **by game** | same splitter for every fit |
| dedupe | not described | zobrist dedupe, 1.89 % | S076's setting stated with each curve |
| augmentation | horizontal mirroring "when data is sparse" | none | none for the hand-crafted evaluation; see section 6 |
| what decides | validation loss only | one candidate per SPRT; zero recorded as zero | unchanged |
| search parameters | not discussed | SPSA on games, independent SPRT, driver of chesso's own | one lane per block, S127 last |
| provenance | not discussed | stamp in every header; corpus archived with digests | same for every new corpus (DEC-183) |
| honest speed / depth trade | a table of assumed centipawns per ply | measured: sixteen times the nodes removes 24.1 % of the error, 29.8 % after S028; 10.4 cp per effective doubling (DEC-033, DEC-081) | re-measured at block boundaries |

---

## 5. Where the article's description departs from the published method

Listed so nobody implements the article's version by mistake.

1. **The label.** A per-position win rate across games is not Texel's method;
   the published objective is over each position's own game result, and
   repeated positions are the exception (S076 measured them at 1.89 % of
   chesso's corpus). Averaging them was considered here and refused
   (DEC-065: the first row of a repeated position survives, no label is
   averaged).
2. **The update rule** as printed has no sign and no feature multiplier. The
   published gradient of the squared error is proportional to
   `(sigmoid - result) * sigmoid * (1 - sigmoid) * feature`, per feature;
   `adocs/eval_tuning_strategy.md` section 2.4 writes it out.
3. **"0.67"** is not traceable to any fetched source (section 2).
4. **"Obsolete in the NNuE era"** is contradicted by the public record:
   every engine in DEC-071's table reached 3130 to 3565 CCRL Blitz on a
   hand-crafted evaluation one version before its first network, and Leorik
   went from 2917 to 3276 by putting a net on top of a tuned hand-crafted
   engine, not instead of tuning it. It is also contradicted here: S028 and
   S065 are +188.74 and +21.10 measured.
5. **The speed claim.** The article says NNUE ran "~26% faster than the
   hand-crafted code it replaced"; CPW *Stockfish NNUE* records Stockfish
   12's network as gaining "at least 80 Elo" despite "approximately halved
   search speed". The two statements cannot both describe Stockfish 12.
6. **Mirroring as augmentation** is presented as a general Texel-era
   discovery. For a piece-square table it assumes the evaluation is
   left-right symmetric, which a castled king is not; for networks the
   published form is a mirroring built into the feature index (HalfKAv2_hm)
   rather than a doubled data set, and it is a design choice, not a free
   augmentation.
7. **The depth table** is offered as assumption. chesso's measured version
   (DEC-033, DEC-081) says the opposite of the article's ordering claim: the
   engine was tree-shape-limited, not evaluation-limited, which is why the
   search block precedes the evaluation block in `adocs/plan.md`.

---

## 6. What chesso can take from the article

Ideas only, each in DEC-105's form (c) — a range declared by purpose — and
each a measurement, never a constant.

1. **Random moves inside the game, not only in the opening.** The article
   drops 10 % of moves to a lower depth and excludes those rows; Stockfish's
   generator does the same thing as a bounded count of random moves early in
   the game and keeps writing rows only after a minimum ply. chesso's
   `tools/datagen` randomises the first 8 plies and nothing after. The
   coverage numbers S065 measured — a queen at `phase <= 4` on 0.0889 % of
   rows — are the symptom that more in-game diversity might address. **Shape
   for chesso**: a `--random-move-rate` (or a bounded count with a ply window)
   in `tools/datagen`, the row after a random move not recorded, the rate a
   declared range with its midpoint, and its effect read on S083's held-out
   curve and then one SPRT. It is S083's night, not a new step, until the
   curve says otherwise.
2. **A coverage report by phase and score band, printed by `datagen` or the
   tuner.** The article's stratification matrix is the right *report*; its
   resampling is not (below). S065 measured one cell by hand; making the
   matrix a standard print costs nothing and makes the next corpus decision
   readable. A tools-only change, no verdict owed.

Not taken, with the reason:

- **Score-stratified resampling.** It changes the label distribution the
  objective is defined on, no fetched source supports it for a hand-crafted
  evaluation, and S066's zero says 827 parameters over 9.9 M rows are not
  data-starved.
- **Horizontal mirroring of the corpus.** Wrong for chesso's asymmetric
  tables (section 5, item 6). If S133's bucket scheme mirrors by king side it
  does so in the feature index, and that is S133's own held-out comparison.
- **Depth-gated filters per phase.** They belong to a corpus labelled by
  another engine at unknown depth; chesso generates at fixed nodes and the
  budget is S083's single variable.
- **The "-2 to +2 pawns" optimisation window.** The sigmoid's slope already
  weights rows this way (`sigmoid * (1 - sigmoid)` is the gradient factor;
  DEC-055 priced the rows past 1000 cp at 0.206 % of the gradient mass), so
  the window is what the objective does by construction.

---

## 7. What chesso does that the article does not

For the record, because "compare to what was done" cuts both ways.

- **A gate.** Every fitted vector plays an SPRT against the incumbent; loss
  ranks, games decide (DEC-019, DEC-064). The article decides on validation
  loss and reports no game.
- **A held-out set that cannot leak** (S066), and a dedupe whose collision
  count is verified (S076).
- **A second implementation of the evaluation held by a test**, so the fit
  cannot drift from the engine unnoticed (S028, S186).
- **Provenance in the artefact**: commit, corpus digest, rows, bytes, lambda,
  `K` (S077, DEC-064); the corpus archived with its recipe (DEC-183).
- **A measured answer to the blend question** (S075): monotone harm at every
  non-zero lambda on this corpus, with the two rules that make lambdas
  comparable recorded before the numbers.
- **Search parameters tuned on games, not on a corpus**, with the SPSA
  vector treated as a hypothesis until an independent SPRT on an unseen book
  and control confirms it (S085), and one axis refused because it broke mate
  tests.
- **A rule that published figures do not transfer** (DEC-019), with the
  project's own five transfers recorded at 0, 0, wrong sign, 0.10 and 0.33.

---

## 8. References

Fetched 2026-09-27 unless stated.

- The article: https://medium.com/@alan0408yuan/from-texel-tuning-to-nnue-the-steep-learning-curve-of-building-a-3200-elo-chess-ai-d842393b5955 — read in the browser pane (HTTP 403 to every fetch tool).
- Its README: https://raw.githubusercontent.com/alanyuan08/Chess-App/main/README.md — strength claim, nps claim, architecture line, data counts; licence from the repository page https://github.com/alanyuan08/Chess-App. No source file opened.
- CPW *Texel's Tuning Method*: https://www.chessprogramming.org/Texel%27s_Tuning_Method — the published objective, the per-game label, the withdrawn deviation filter, the 64000-game / 8.8 M corpus, the seven-retune ledger. (Quoted from S082, S083 and S126, which fetched it 2026-09-13.)
- CPW *Stockfish NNUE*: https://www.chessprogramming.org/Stockfish_NNUE — "at least 80 Elo" and "approximately halved search speed" for Stockfish 12.
- Stockfish `nnue-pytorch` `docs/nnue.md` (raw): https://raw.githubusercontent.com/official-stockfish/nnue-pytorch/master/docs/nnue.md — the sigmoid scaling of about 400 in centipawn space; the lambda blend. Documentation read; no code reproduced.
- nodchip `docs/gensfen.md` (raw): https://raw.githubusercontent.com/nodchip/Stockfish/master/docs/gensfen.md — `random_move_count` 5 between plies 1 and 24, `write_minply` 16, `eval_limit` 3000, `ensure_quiet`. Documentation only.
- Local: `adocs/eval_tuning_strategy.md`; `adocs/plan_done/S028_texel_tuning.md`, `S065_*`, `S066_*`, `S075_*`, `S076_*`, `S077_*`, `S084_*`, `S085_*`, `S100_*`, `S222_*`, `S231_*`; `adocs/plan_todo/S082_*`, `S083_*`, `S126_*`, `S127_*`, `S133_*`, `S135_*`, `S136_*`; `adocs/decisions.md` DEC-002, DEC-016, DEC-019, DEC-033, DEC-055, DEC-056, DEC-057, DEC-064, DEC-065, DEC-066, DEC-071, DEC-081, DEC-105, DEC-183, DEC-222, DEC-224; `tools/tuner.cpp`, `tools/datagen.cpp` header comments.
