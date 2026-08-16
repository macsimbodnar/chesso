# Evaluation and Search Parameter Tuning: State of the Art

**Purpose.** Input document for a development plan. Describes the techniques with the highest
expected Elo return for a classical alpha-beta chess engine, ordered by return on effort, with
literature references and enough algorithmic detail to implement.

**Scope note.** This document makes no assumption about which of these are already present in the
engine. Any phase already implemented should be treated as a verification checkpoint rather than
new work.

---

## 0. Executive summary

Expected Elo, roughly, for a mature alpha-beta engine:

| Technique | Typical gain | Effort | Verdict |
|---|---|---|---|
| Gradient tuning of hand-crafted eval (Texel-style) | +100 to +300 | Low | **Do first** |
| NNUE evaluation | +400 to +700 over tuned HCE | High | **Highest ceiling** |
| Self-play data generation pipeline | Enables the above two | Medium | Prerequisite for scale |
| SPSA on search parameters | +10 to +40 cumulative | Medium, high compute | Do late, verify hard |
| TD / online learning during play (TDLeaf, TreeStrap) | Historically +300 from random init | Medium | Dominated by supervised methods |
| Book / position learning | +5 to +20 in repeated matches | Low | Optional, orthogonal |

**Read the gain column as reported figures, never as a target (DEC-019, added
2026-08-16 with DEC-062).** This engine has taken three published Elo figures at
face value and measured 0, 0 and *slower*: staged move generation quoted at
30-50, SEE pruning in quiescence, capture ordering reported around 150. A
technique's value depends on the search around it. What is in this table decides
what is tried; what an SPRT returns decides what is kept, and a verdict of zero
is recorded as zero.

The single most important fact: **the objective function is playing strength, not tuning loss.**
Every weight vector produced by any method below must pass SPRT against the incumbent before it
ships. Tuning loss is a proxy that correlates well but not perfectly.

The second most important fact: **eval parameters are only optimal relative to the search that uses
them.** Any material change to pruning, reductions, or quiescence invalidates the tuning. Retuning
after search work is normal and expected.

---

## 1. Why the naive online-learning idea underperforms

A common intuition is: perturb the eval weights slightly, play, observe whether the game was won,
push the delta back. Three problems:

1. **The search score is not an independent signal.** The search score is computed *from* the eval
   weights. Training the eval toward its own search score at the same depth is a fixed point that
   carries no information. A valid target must come from a strictly stronger estimator: a deeper
   search, the game result, or an external evaluator.
2. **Sample efficiency.** A game result is one noisy bit spread over ~80 positions. A gradient on a
   labeled position gives you a full vector in the parameter space per sample. The difference is
   three to four orders of magnitude in games required.
3. **Non-stationarity.** If weights drift while distributed clients are playing, each client is
   generating data under a different function. The dataset becomes incoherent and the gate is gone.

The correct decomposition of "an engine that improves as it plays" is:

> **collect while playing → label offline → optimize in batch → SPRT-gate → ship.**

Clients upload raw positions and outcomes, never gradients or weight deltas.

---

## 2. Technique 1: Gradient tuning of a hand-crafted evaluation

### 2.1 Origin and naming

Known in the engine community as **Texel tuning**, after Peter Österlund's Texel engine; described
by him on TalkChess in January 2014 and documented on the Chess Programming Wiki under
"Texel's Tuning Method". The underlying idea is older logistic regression of eval features against
game outcomes.

Key precursors:

- **Samuel, A. L. (1959)**, *Some Studies in Machine Learning Using the Game of Checkers*, IBM
  Journal of Research and Development 3(3). The origin of automatic parameter adjustment from play.
- **Campbell, Hoane, Hsu (2002)**, *Deep Blue*, Artificial Intelligence 134. Section on evaluation
  tuning from grandmaster games; the industrial precedent for supervised eval fitting.

### 2.2 The objective

Given a dataset of positions with game outcomes `r ∈ {0, 0.5, 1}` from the side-to-move
perspective:

```
E(w) = (1/N) * Σ_i ( r_i − σ(K · s_i(w)) )²

σ(x) = 1 / (1 + 10^(−x/400))     (or the logistic 1/(1+e^-x) with K absorbed)
s_i(w) = evaluation score of position i under weights w
```

`K` is a scaling constant fitted **once** by a 1-D search (golden section or simple ternary search)
against the initial weights, then frozen. Typical values land near 1.0 to 1.5 for centipawn scores
with the base-10 form above.

Mean squared error on the sigmoid is the classical choice. Binary cross-entropy is an equally valid
and slightly better-behaved alternative; the gradients differ only by a factor that cancels most of
the sigmoid derivative. Either works.

### 2.3 The critical design decision: what is `s_i`?

Three options, in ascending order of cost and quality:

- **Static eval at the position.** Fast, but noisy: positions with hanging pieces produce garbage
  labels and drag weights toward compensating for tactics.
- **Quiescence search score.** The standard choice. Cheap, and it evaluates the position at a quiet
  leaf where the eval is actually meaningful.
- **Fixed-depth search score.** More accurate, much slower, and introduces search-specific bias.

Use qsearch. This mirrors the insight from TDLeaf (Section 5): the eval function is only
well-defined on quiet positions, so the target must be anchored at a quiescent leaf.

### 2.4 Exploiting linearity (the big practical win)

A typical hand-crafted evaluation is **linear in its weights**:

```
eval(pos) = Σ_j w_j · f_j(pos)
```

where `f_j` are feature counts (piece counts, PST occupancy indicators, mobility counts, pawn
structure counts). Tapered evaluation does not break this: with phase `p ∈ [0,1]` and per-term
midgame/endgame pairs,

```
eval = Σ_j [ p · w_j^mg + (1−p) · w_j^eg ] · f_j
```

is still linear in the concatenated vector `(w^mg, w^eg)`. The gradient is therefore **closed form**:

```
∂E/∂w_j^mg = (2/N) Σ_i −(r_i − σ_i) · σ_i(1−σ_i) · ln(10)·K/400 · p_i · f_j(leaf_i)
```

Implementation requirement: an **eval trace mode** that, instead of returning a scalar, returns the
sparse feature vector `f(leaf)` and the phase `p` at the qsearch leaf. Build this. It converts
tuning from a 1000-dimensional finite-difference problem into a single sparse matrix-vector
operation per epoch.

**Non-linear terms need care.** King safety implemented as a table lookup indexed by accumulated
attack units, or any term with a `max`/`min`/threshold, is not linear in the parameters being
tuned. Options: (a) tune the table entries themselves as free parameters, which restores linearity;
(b) hold the non-linear terms fixed during the linear pass and tune them separately by SPSA;
(c) reformulate as a piecewise-linear basis. Option (a) is usually best and also removes hand-tuned
magic numbers.

If the feature vector is precomputed and cached, an epoch over 50M positions is seconds, not hours.

### 2.5 Optimizer

- Naive coordinate descent over integer weights (the original Texel formulation) works and is
  trivially correct, but is slow and gets stuck.
- **Adam** (Kingma & Ba, 2015, *Adam: A Method for Stochastic Optimization*, ICLR) on mini-batches
  is the modern default. Learning rate around 1.0 on centipawn-scale parameters, decayed; batch size
  16k to 64k positions.
- **AdaGrad** is also common in engine tuners and is more forgiving of the wildly different scales
  between, say, a pawn PST entry and a bishop pair bonus.

Quantize to integers only at the very end, and re-measure: rounding a few hundred parameters can
cost measurable Elo if any of them sit near a decision boundary.

### 2.6 Dataset construction (this dominates method choice)

Quality and size of data matter more than the choice of optimizer. Requirements:

- **Volume.** 10M positions is a working minimum for a few hundred parameters. 100M+ is where
  results stabilize. Serious HCE tuners use 100M to 1B.
- **Source.** Self-play games from the engine itself at fixed low nodes (e.g. 5000 nodes/move) with
  randomized opening plies, plus a smaller share of games against other engines for distribution
  coverage. Human game databases work but skew the position distribution away from what the engine
  actually reaches.
- **Filtering, all of which matter:**
  - Drop positions where the side to move is in check.
  - Drop positions where the qsearch leaf differs from the root (or rather: label the leaf, not the
    root).
  - Drop positions with mate scores or scores beyond ~|1500| cp.
  - Drop the first 8 to 12 plies (book noise, no signal).
  - Deduplicate by Zobrist key; heavily repeated positions bias the fit.
- **Opening diversity.** Use randomized or book-driven starts with `order=random`. A dataset from a
  narrow opening set produces weights that are excellent in that opening set and mediocre elsewhere.

### 2.7 Labeling: WDL, score, or both

Two label sources exist for a position: the eventual **game result** (WDL) and a **search score**
from a stronger evaluator or deeper search. Modern practice (from the Stockfish NNUE training
pipeline, `nnue-pytorch`) blends them:

```
target = λ · σ(score_from_deep_search) + (1 − λ) · WDL_result
```

with `λ` typically 0.7 to 1.0, sometimes annealed downward during training. Rationale: the search
score is low-variance but biased toward the current evaluator; the game result is unbiased but very
high-variance. The blend beats either alone. This is directly applicable to HCE tuning, not only to
NNUE.

### 2.8 Validation

- Hold out 5 to 10 percent of positions. Watch for the train/validation gap. With a few hundred
  parameters and tens of millions of positions, overfitting is unlikely but not impossible,
  especially for rare features (specific PST corners, obscure pawn formations).
- **Loss improvement is not Elo.** Always finish with SPRT. A loss improvement of 1e-5 that fails
  SPRT is a real and common outcome.

### 2.9 Realistic expectations

From hand-set "reasonable" values to fully tuned: +100 to +300 Elo is the usual reported range, with
the larger figures when PSTs are included in the tuning set. From an already-tuned set, re-tuning
after search changes typically yields +5 to +25.

---

## 3. Technique 2: NNUE (the actual ceiling)

If the goal is "highest Elo possible from evaluation work", the answer is not better HCE tuning. It
is replacing the HCE.

### 3.1 References

- **Nasu, Y. (2018)**, *NNUE: Efficiently Updatable Neural-Network-based Evaluation Functions for
  Computer Shogi*. The originating paper (Japanese; English translations circulate widely). Defines
  the HalfKP feature set and the incremental accumulator update.
- **Hoki, K. & Kaneko, T. (2014)**, *Large-Scale Optimization for Evaluation Functions with Minimax
  Search*, JAIR 49. The Bonanza method: comparison training against expert moves under minimax
  search. This is the intellectual ancestor of large-scale eval fitting in shogi and directly
  enabled NNUE.
- **Stockfish 12 (2020)** release notes and the `nnue-pytorch` repository documentation. The
  reference open-source implementation and training recipe.

### 3.2 Why it is fast enough

The architecture is deliberately shallow and the first layer is enormous but **incrementally
updatable**. Feature transformer: sparse input (~40k to 45k binary features under HalfKP/HalfKA) to
256 to 1024 hidden units. Because a move changes only a handful of active input features, the first
layer's accumulator is updated by adding and subtracting a few column vectors on make/unmake, not
recomputed. Remaining layers are small (e.g. 32 → 32 → 1), quantized to int8/int16, and evaluated
with SIMD. Net cost is on the order of a few hundred nanoseconds, which alpha-beta can afford.

### 3.3 The dependency structure that matters for planning

NNUE requires, in order:

1. A **data generation pipeline** at scale (the same one Section 2.6 requires).
2. A **training harness** in Python/PyTorch, separate from the engine.
3. **Quantization and SIMD inference** in the engine (AVX2 at minimum).
4. **Incremental accumulator** correctness, including handling king moves (which invalidate the
   whole accumulator under HalfKP) and null moves.

Item 1 is shared with Section 2. This is the strategic reason to build the data pipeline properly
even if the immediate goal is only HCE tuning: it is the prerequisite for the much larger gain
later.

A tuned HCE remains useful after NNUE lands: as a fallback for non-SIMD builds, as a sanity
reference, and in some engines as a fast filter in pruning decisions.

---

## 4. Technique 3: SPSA for search parameters

Search parameters (LMR coefficients, futility margins, null-move reduction formulas, aspiration
window widths, history bonus scaling, delta pruning margins) have **no differentiable objective**.
The only measurable objective is win rate, which is extremely noisy. This is where black-box
optimization belongs, and nowhere else.

### 4.1 References

- **Spall, J. C. (1992)**, *Multivariate Stochastic Approximation Using a Simultaneous Perturbation
  Gradient Approximation*, IEEE Transactions on Automatic Control 37(3). SPSA proper.
- **Spall, J. C. (1998)**, *Implementation of the Simultaneous Perturbation Algorithm for Stochastic
  Optimization*, IEEE Aerospace and Electronic Systems 34(3). The practical guide to choosing
  `a`, `c`, `A`, `α`, `γ`.
- **Coulom, R. (2011)**, *CLOP: Confident Local Optimization for Noisy Black-Box Parameter Tuning*,
  Advances in Computer Games 13. Fits a local quadratic model with a confidence region; more
  sample-efficient than SPSA in low dimensions, less used in practice today.
- **Hansen, N. (2006)**, *The CMA Evolution Strategy: A Tutorial*. CMA-ES; strong in moderate
  dimensions but needs more evaluations than the game budget usually allows.

### 4.2 Why SPSA and not something smarter

SPSA needs exactly **two objective evaluations per iteration regardless of dimensionality**. With
40 parameters, a coordinate method needs 80 evaluations per gradient estimate; SPSA needs 2. When
each "evaluation" costs thousands of games, this dominates every other consideration. Stockfish's
Fishtest uses SPSA for precisely this reason.

### 4.3 The algorithm

```
for k = 0, 1, 2, ...:
    a_k = a / (k + 1 + A)^α          # α = 0.602 standard
    c_k = c / (k + 1)^γ              # γ = 0.101 standard
    Δ   ~ Rademacher(±1) per dimension
    θ⁺  = θ + c_k · Δ
    θ⁻  = θ − c_k · Δ
    y⁺, y⁻ = play a game (or small batch) with θ⁺ and θ⁻   # paired, same opening, colors reversed
    ĝ   = (y⁺ − y⁻) / (2 c_k) · Δ⁻¹                        # elementwise
    θ   = θ + a_k · ĝ                                       # + because we maximize win rate
    clamp θ to legal ranges
```

Practical notes:

- Use **paired games**: same opening, both colors, θ⁺ vs θ⁻ directly. Variance reduction here is
  worth more than any tuning of `a` and `c`.
- Set `c_end` (the perturbation at the end of the run) to roughly the smallest change in each
  parameter you believe could matter, and `r_end = a_end / c_end²` to control final step size.
  Stockfish's tuning interface parameterizes it this way for good reason.
- Budget: 30k to 200k games at fast time control (e.g. 5+0.05 or 10+0.1) per run. Below ~30k games
  the result is indistinguishable from noise.
- Restrict to 10 to 30 parameters per run. SPSA's dimension-independence is about cost per
  iteration, not about the number of iterations needed to converge.

### 4.4 The verification requirement

SPSA output is a point estimate from a noisy process. A meaningful fraction of SPSA runs produce
parameter vectors that do **not** pass a subsequent SPRT. Treat SPSA as a hypothesis generator, not
a result. Always run a clean SPRT of the SPSA output against the incumbent at the intended time
control, with tight bounds.

---

## 5. Technique 4: Temporal-difference and online learning

Historically important, and the honest answer to the original "learn while playing" question, but
dominated by supervised methods when data is available.

### 5.1 TDLeaf(λ)

- **Sutton, R. (1988)**, *Learning to Predict by the Methods of Temporal Differences*, Machine
  Learning 3. The base algorithm.
- **Tesauro, G. (1994)**, *TD-Gammon*, Neural Computation 6(2). The proof that TD self-play reaches
  world-class play in a game without deep search.
- **Baxter, Tridgell, Weaver (2000)**, *Learning to Play Chess Using Temporal Differences*, Machine
  Learning 40(3). TDLeaf(λ) and the KnightCap result: roughly 1650 to 2150 Elo in about 300 games
  played online against humans.

The essential insight of TDLeaf: applying TD to the root evaluation is wrong, because the root
evaluation is a *search* score, not an *eval* score. The update must be applied to the weights
evaluated at the **leaf of the principal variation**, since that leaf is what actually produced the
score.

```
w ← w + η · Σ_{t=1}^{N−1} ∇_w eval(leaf_t) · Σ_{j=t}^{N−1} λ^(j−t) · d_j
d_t = score(leaf_{t+1}) − score(leaf_t)
```

The KnightCap result carries two caveats the paper is explicit about: it started from
hand-initialized material values (not random), and it learned against a *varied pool of stronger
human opponents*, which provided the exploration that self-play alone did not.

### 5.2 TreeStrap

- **Veness, Silver, Uther, Blair (2009)**, *Bootstrapping from Game Tree Search*, NIPS 22.

Strictly better than TDLeaf for this purpose. Instead of using only the PV leaf and successive
positions, TreeStrap updates the evaluation of **every node in the search tree** toward the value
the deeper search assigned to it:

```
Δw = η · Σ_{s ∈ tree} ∇_w eval(s) · [ search_value(s) − eval(s) ]
```

with updates clipped to respect alpha-beta bounds (a node cut off at a bound only provides a
one-sided constraint). Properties:

- Every search produces thousands of training pairs, not one.
- No need to wait for the game to end; the signal is available immediately.
- Their engine Meep learned from **random initial weights** to master level via self-play, which
  TDLeaf could not do, and TreeStrap-trained weights beat TDLeaf-trained weights head to head.

### 5.3 Why it is still not the recommendation

TreeStrap's target is "the value of a deeper search using the current eval". This is a genuine
improvement operator, but it converges to a fixed point of the engine's own search, and it is
sample-inefficient compared to fitting against millions of stored positions with known outcomes.
Supervised tuning on a large dataset (Section 2) reaches a better optimum faster, and NNUE
(Section 3) reaches a far better one.

TreeStrap remains the right choice in exactly one situation: when no dataset exists and none can be
generated, e.g. bootstrapping a novel variant.

### 5.4 If online learning is still desired: the safe architecture

1. Clients play and upload **`(FEN, qsearch_leaf_features, search_score, game_result)`** tuples.
   Never gradients, never weight deltas.
2. Server accumulates, dedupes by Zobrist, filters per Section 2.6.
3. Retrain in batch on a schedule (nightly, or per N million new positions).
4. SPRT-gate the candidate weight vector against the incumbent at a real time control.
5. Ship on H1 only. Clients pull the new weights on next start.

This preserves the "the engine gets better the more it plays" property while keeping the
statistical gate intact. It is a distributed data-generation system, which is what Fishtest and
OpenBench actually are.

---

## 6. Technique 5: Book and position learning (orthogonal, cheap)

Distinct from eval tuning: persist per-position or per-opening-line outcome statistics and use them
to steer opening choice away from lines the engine loses in. Present in commercial engines from the
1990s (Rebel, Crafty's "book learning" and "position learning").

- Key: Zobrist hash of the position, or the opening line prefix.
- Value: running win/loss/draw counts and an accumulated score adjustment.
- Effect: in repeated matches against the same opponent, worth +5 to +20 Elo. In one-off games,
  nothing.
- Risk: it is a form of overfitting to the opponent pool. Some tournaments prohibit persistent
  learning files. Keep it behind a UCI option, default off, and never let it influence the eval
  weights.

Low effort, and it delivers the *experience* of an engine that improves with use, independent of the
tuning work.

---

## 7. The gate: statistical infrastructure

Everything above is worthless without a correct gate.

- **Wald, A. (1945)**, *Sequential Tests of Statistical Hypotheses*, Annals of Mathematical
  Statistics 16(2). SPRT.
- Modern engine practice uses the pentanomial (5-outcome, game-pair) model rather than trinomial;
  it accounts for the correlation between the two games of a paired opening and reduces required
  games by roughly 20 to 30 percent.

Recommended discipline:

| Purpose | Bounds | α, β | Typical games |
|---|---|---|---|
| Quick sanity / non-regression | [-10, 0] or [-5, 5] | 0.05, 0.05 | 1k to 5k |
| Standard improvement | [0, 5] | 0.05, 0.05 | 10k to 40k |
| Confirmation at long TC | [0, 3] | 0.05, 0.05 | 40k to 100k+ |
| Simplification (accept no-loss) | [-3, 1] | 0.05, 0.05 | 20k to 60k |

Additional requirements:

- Pin `Hash` and `Threads` identically for both engines.
- Use a large, balanced opening book with `order=random` and paired colors.
- Enable draw and resign adjudication to save compute, but keep the thresholds conservative.
- Candidate engine listed first so positive Elo means improvement.
- Never tune and test on the same opening set.

**The trap specific to tuning work:** if you generate 20 candidate weight vectors and SPRT each at
α=0.05, you expect one false positive by chance. Either use tighter α for tuning candidates, or
confirm the winner with an independent second SPRT at a different time control.

---

## 8. Recommended ordering

Dependency-respecting, highest return first.

**Phase A. Instrumentation.**
Eval parameters exposed as a flat, addressable array. Eval trace mode returning the sparse feature
vector and phase at the qsearch leaf. UCI options for every tunable so external tools can set them.
Prerequisite for everything else; produces no Elo by itself.

**Phase B. Data pipeline.**
Self-play generation at fixed nodes with randomized openings, position filtering, Zobrist dedupe,
compact binary storage format. Target 50M+ positions. Shared prerequisite for Phase C and Phase E.

**Phase C. HCE gradient tuning.**
Fit `K`. Adam on the linear parameters using the closed-form gradient. Handle non-linear terms per
Section 2.4. Holdout validation, then SPRT. Expected +100 to +300 depending on the starting point.
Repeat after any significant search change.

**Phase D. Search parameter SPSA.**
Only after Phase C, and only after the search itself is stable. 10 to 30 parameters per run, 50k+
games, paired-game variance reduction, mandatory independent SPRT verification of the output.
Expected +10 to +40 cumulative across several runs.

**Phase E. NNUE.**
Reuses the Phase B pipeline. Trainer in PyTorch, quantization, SIMD inference, incremental
accumulator. Largest single gain available, and the largest single work item.

**Phase F (optional, any time). Book learning.**
Independent of A through E. Cheap.

**Explicitly deprioritized:** online TD/TreeStrap weight updates, and any scheme that mutates
shipped weights without an SPRT gate. Revisit only if the data pipeline proves infeasible.

---

## 9. Reference list

Ordered as cited.

1. Samuel, A. L. (1959). *Some Studies in Machine Learning Using the Game of Checkers*. IBM Journal
   of Research and Development 3(3), 210-229.
2. Campbell, M., Hoane, A. J., Hsu, F.-h. (2002). *Deep Blue*. Artificial Intelligence 134(1-2),
   57-83.
3. Österlund, P. (2014). Texel's tuning method. TalkChess forum; documented on the Chess Programming
   Wiki, "Texel's Tuning Method".
4. Kingma, D. P., Ba, J. (2015). *Adam: A Method for Stochastic Optimization*. ICLR.
5. Nasu, Y. (2018). *NNUE: Efficiently Updatable Neural-Network-based Evaluation Functions for
   Computer Shogi*. Ziosoft Computer Shogi Club.
6. Hoki, K., Kaneko, T. (2014). *Large-Scale Optimization for Evaluation Functions with Minimax
   Search*. Journal of Artificial Intelligence Research 49, 527-568.
7. Spall, J. C. (1992). *Multivariate Stochastic Approximation Using a Simultaneous Perturbation
   Gradient Approximation*. IEEE Transactions on Automatic Control 37(3), 332-341.
8. Spall, J. C. (1998). *Implementation of the Simultaneous Perturbation Algorithm for Stochastic
   Optimization*. IEEE Transactions on Aerospace and Electronic Systems 34(3), 817-823.
9. Coulom, R. (2011). *CLOP: Confident Local Optimization for Noisy Black-Box Parameter Tuning*.
   Advances in Computer Games 13, LNCS 7168, 146-157.
10. Hansen, N. (2006). *The CMA Evolution Strategy: A Tutorial*.
11. Sutton, R. S. (1988). *Learning to Predict by the Methods of Temporal Differences*. Machine
    Learning 3(1), 9-44.
12. Tesauro, G. (1994). *TD-Gammon, a Self-Teaching Backgammon Program, Achieves Master-Level Play*.
    Neural Computation 6(2), 215-219.
13. Baxter, J., Tridgell, A., Weaver, L. (2000). *Learning to Play Chess Using Temporal Differences*.
    Machine Learning 40(3), 243-263.
14. Veness, J., Silver, D., Uther, W., Blair, A. (2009). *Bootstrapping from Game Tree Search*.
    Advances in Neural Information Processing Systems 22.
15. Wald, A. (1945). *Sequential Tests of Statistical Hypotheses*. Annals of Mathematical Statistics
    16(2), 117-186.
16. Silver, D. et al. (2018). *A general reinforcement learning algorithm that masters chess, shogi,
    and Go through self-play*. Science 362(6419), 1140-1144. (Context for the MCTS/NN alternative
    paradigm; not applicable to alpha-beta parameter tuning.)
17. Stockfish `nnue-pytorch` repository documentation, and the Chess Programming Wiki entries for
    NNUE, SPSA, Texel's Tuning Method, and Automated Tuning.

---

## 10. Notes for the implementing agent

- Do not treat tuning loss as the success criterion. SPRT is the success criterion.
- Phase A is not optional and is not glamorous. Every later phase depends on the eval trace and the
  flat parameter array.
- Keep the tuner **out of the engine binary**. The engine should expose parameters and a trace mode;
  the optimizer is a separate program (Python is fine, and better for Phase E).
- Version every weight vector alongside the git commit of the engine and the dataset hash that
  produced it. Untraceable weights are unreproducible results.
- After any search change of consequence, re-run Phase C before evaluating whether the search change
  was actually good. A search change that looks neutral against stale eval weights may be positive
  against retuned ones.
