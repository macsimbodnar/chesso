id:         S134
goal:       delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
accepts:    the engine-side identity is measured before anything is deleted -- `passed_pawn_counts()` and `piece_placement_counts()` read out of the engine, not out of `eval_model`, and compared against the signed piece-square occupancy of squares 8..15 over the whole corpus, 0 violations required and the non-zero row count reported so the check is not vacuous; the fold is **bit-exact and shown to be**, by identical scores on the seven pinned anchor positions and by `tools/search_bench.py` returning identical node counts and best moves at two depths (INV-6 discharged on node counts, no SPRT owed -- DEC-090); `PIECE_PLACEMENT_COUNT` 4 to 3 and `PASSED_PAWN_COUNT` 6 to 5, `PARAM_COUNT` 827 to 823, with `test_tuner_groups`' three partition properties green **and observed red** under a base left unshifted; `test_eval_model`'s hand cases, differential sweep and non-vacuity lists re-targeted to the narrowed features rather than deleted; `tools/feature_audit` still runs and its identity report states the rank/identity result for the **remaining** parameterisation, including the method, corpus rows and every exact dependency found -- no unreported exact degeneracy remains after the two folds, or a new finding is filed before a fit; that measurement is the recorded basis for DEC-170's decision against global regularisation, and S126 reopens it if its fit shows another ridge; the fast suite green
touches:    src/evaluation.cpp (the two terms and their accumulation), src/eval_tables.hpp (the sixteen folded entries), tools/eval_model.hpp (widths and bases), tools/tuner_model.hpp (the phase column), tools/tuner_groups.hpp, tools/feature_audit.cpp, tests/test_eval_model.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the other three placement features, which is S135; tempo, which is S136; any refit -- this step moves numbers between two places that add up to the same score and fits nothing
decisions:  DEC-090
closes:     2026-09-10_adversarial-F34, 2026-09-10_adversarial-F36, 2026-09-12_adversarial-F02
blocks:     S135
paused_by:
done:

## Why this is worth a step of its own

S100 proved two exact linear dependencies, from the indexing rather than from a
correlation. Index 0 is a8 and a black piece mirrors by `^56`, so a rook or a
pawn on its own seventh rank always occupies one of squares 8..15. Therefore:

- `PL_ROOK_SEVENTH`'s differential **is** the signed sum of eight
  `psqt[ROOK][8..15]` occupancy columns.
- passer bucket 5 **is** the signed sum of eight `psqt[PAWN][8..15]` columns,
  because a pawn on its own seventh is a passer by definition -- "ahead" is the
  enemy back rank and the extractor refuses a pawn standing there.

Measured: R^2 exactly 1.000000 for both, and 0 violations over 1264773 and
550880 non-zero rows of `.tuning/selfplay_v2_dedup.tsv`
(`adocs/data/S100_feature_audit.txt`).

**So there is no weight either feature could carry that the tables cannot
already express**, and a joint fit's value for the split is a position on a
ridge. That is not a theory: across three real fits passer buckets 0 to 4 move
by at most 4 while bucket 5 reads +22, -1 and -17. Deleting them loses nothing
and removes two columns no future fit can read correctly and no future SPRT can
attribute.

## The fold is bit-exact, and that is the whole design

`evaluate_pawns()` sums all three pawn terms into `pawn_mg` / `pawn_eg`, and
`src/evaluation.cpp` `evaluate_cheap` adds those to the piece-square
accumulator **before a single tapered division**:

```
const int positional =
    (((board->psqt_mg + pawn_mg) * phase) +
     ((board->psqt_eg + pawn_eg) * (GAME_PHASE_MAX - phase))) /
    GAME_PHASE_MAX;
```

So moving a weight from `pawn_mg` into `board->psqt_mg` moves it inside the same
summand of the same division. No extra truncation, no rounding: the score is
**identical**, not merely within a centipawn. This is the reason the step is
behaviour-neutral rather than DEC-059's near-neutral re-anchoring, and it is why
node counts discharge INV-6 here.

What gets folded:

| from | value | onto |
|---|---|---|
| `passed_pawn_mg[5]` | **-17** | each of `psqt_mg[PAWN][8..15]` |
| `passed_pawn_eg[5]` | **+42** | each of `psqt_eg[PAWN][8..15]` |
| `piece_placement_mg[PL_ROOK_SEVENTH]` | 0 | each of `psqt_mg[ROOK][8..15]` |
| `piece_placement_eg[PL_ROOK_SEVENTH]` | 0 | each of `psqt_eg[ROOK][8..15]` |

**The rook half is a no-op arithmetically and is not therefore free**: the code
still has to go, or the next fit puts a weight back on an unidentified column.

**The passer half is not zero and this is the trap the step exists to avoid.**
S100's own first framing said "the weights are already zero so the compiler
deletes both terms" -- true of `piece_placement`, false of passer bucket 5,
which ships mg -17 and eg +42 (`src/evaluation.cpp` `passed_pawn_mg` and
`src/evaluation.cpp` `passed_pawn_eg`). Deleting bucket 5 without folding is a
play-altering change wearing a behaviour-neutral label.

## Hazards

- **Every base after the two shrinks moves.** `eval_model.hpp` chains
  `PP_MG_BASE` through `TEMPO_EG_BASE` off the widths, so both drop by 1 and 2
  respectively and `PARAM_COUNT` goes 827 to 823. `tools/tuner_groups.hpp`'s
  ranges are the thing that has silently swallowed an appended block four times;
  the same arithmetic run backwards is the same hazard, and
  `test_tuner_groups`' disjointness and union properties are what catch it. Get
  the red first.
- **The engine's own definition has not been checked against the identity.**
  S100 proved it against `tools/eval_model.hpp` and the stored columns; the
  engine's `evaluate_pawns()` computes passers from bitboard fills and its
  agreement is only tested over the curated positions. The argument carries over
  -- rank 8 holds no pawn, so a 7th-rank pawn has no possible blocker -- but that
  is reasoning, and the accepts requires the measurement instead. **This runs
  before any deletion.**
- **The narrowed arrays must not silently re-index.** A 5-bucket passer array
  with a 7th-rank pawn still computing `bucket = 5` reads out of bounds. The
  7th-rank pawn contributes nothing to the term now and the accumulation has to
  say so explicitly.
- **S123 and S133 inherit whatever this leaves.** S123 rebuilds the passer suite
  and S133 re-shapes the tables; a per-square table keeps the degeneracy for any
  feature defined on a single rank, so S123's new terms need the same test S100
  built. `tools/feature_audit`'s identity report is where that check lives.

## Amended 2026-09-11, DEC-170: the tuner recomputes the phase (F36)

`2026-09-10_adversarial-F36`: `tools/tuner_model.hpp` trusts the corpus's
`phase` column, range-checked only, instead of recomputing it with
`eval_model::phase_of()` from the same header. Verified equal today over
456304 rows, 0 disagreements -- and latent, because this step and S117 both
move `phase_value`, after which every existing corpus tapers on the wrong
phase while the range check still passes. The audit names both steps; S117's
own file changes how the two halves travel and not the phase weights, so this
step is the one that moves `phase_value` by its own text and carries the
obligation -- and S117's implementer re-checks the column if the packing turns
out to touch the phase. This step makes the tuner recompute the phase from the placement (or assert
equality with the column and refuse on a mismatch, naming the row) before any
fit is taken on the new value, and states which.

## Amended 2026-09-12, DEC-197: F34 and the 2026-09-12 audit's F02

DEC-170 decided `2026-09-10_adversarial-F34` against regularisation on
2026-09-11 -- the two exact degeneracies are removed structurally here, and the
question reopens only if S126's fit shows another ridge -- but wrote the
finding by its short id, so a search for the verbatim id found no home and the
2026-09-12 audit reported that as `2026-09-12_adversarial-F02`. Both ids now
sit in `closes:`, and the accepts asks `tools/feature_audit` for more than a
report with nothing left to check: the rank and exact-dependency result over
the **remaining** 823 columns, method and non-zero row count stated, so
DEC-170's ruling rests on a measurement rather than on the two proofs S100
gave. A dependency found is a new finding filed before any fit, not a silent
fold. The accepts wording is the audit's own sharpening, kept.


## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The published method does not describe this problem, and that is the
finding.** CPW *Texel's Tuning Method*
(https://www.chessprogramming.org/Texel%27s_Tuning_Method, fetched 2026-09-13)
states the objective -- positions labelled 0, 0.5 and 1 from the game result,
the quiescence score through a sigmoid with a scaling constant `K` fitted once
and "never changed again by the algorithm", parameters moved until `E` is "a
local minimum in parameter space". It says **nothing** about linearly
dependent parameters, nothing about redundant features, nothing about
regularisation and nothing about overfitting. CPW *Automated Tuning*
(https://www.chessprogramming.org/Automated_Tuning, fetched 2026-09-13) is the
same: the only regularisation mention on the page is a note that Vladimir
Medvedev "further used cross-entropy and regularization" in a logistic
regression, and redundancy, holdout splits and dataset size are all absent.
Its one sentence about parameter choice is Ingo Althofer's, quoted there: "It
is one of the best arts to find the right SMALL set of parameters and to tune
them."

So the technique this step performs -- proving an exact linear dependency
between two features and folding one into the other -- **is not on the wiki**,
and the form check owes that statement rather than a departure list. Where it
does appear in the published record is as the shape of the fix rather than as
a named method: Ethereal's "Remove SCALE_OCB_TWO_KNIGHTS (#61)"
(https://github.com/AndyGrant/Ethereal/commit/8db27ad52de3e0d029d3a122b4f22e17d099ce37,
2018-07-18, **+0.94 +/- 2.36** at 10.0+0.1s over 35725 games) and "Remove
scaling for OCB with 2 Rooks"
(https://github.com/AndyGrant/Ethereal/commit/874e509ea284520c450159d3924ce09e2cc453a2,
2018-08-26, **+0.18 +/- 1.71** short and **+0.11 +/- 1.39** long) are the
published form of deleting a term the rest of the evaluation already
expresses, and both were run as full SPRTs because in those cases the deletion
was *not* bit-exact. This step's is, which is why it owes node counts and no
match (DEC-090).

The one quantitative statement in the published record that bears on the
degeneracy at all is `src/eval_tables.hpp`'s own comment, not a citation: the
piece-square tables are "degenerate by five dimensions" against
`piece_value`, which is the same class of defect one level up and was
accepted rather than folded.

### 2. Shape for chesso

The identity is `S100`'s and is restated in "Why this is worth a step of its
own" above; nothing in this section re-derives it. What the enrichment adds is
where the published method's silence bites:

- A Texel fit minimises `E` over the parameter vector. On an exactly
  rank-deficient design matrix the minimiser is a **set**, not a point, and
  Adam's trajectory picks one member of it by construction -- from the
  initialisation and the step sequence, not from the data. The three fits
  quoted above (bucket 5 reading +22, -1 and -17 while buckets 0 to 4 move by
  at most 4) are that set being sampled three times.
- The fold is a change of basis inside one summand of one division:
  `src/evaluation.cpp` `evaluate_cheap` adds `pawn_mg` to `board->psqt_mg`
  before the taper divides, so moving a weight between them is exact.
- After the fold the column is gone rather than pinned, so no later fit can
  re-enter the ridge. Pinning it at zero would leave the column in
  `tools/eval_model.hpp` and in `tools/tuner_groups.hpp`, where the next
  `--only` fit would free it again.

### 3. Implementation sketch

Order matters and the accepts fixes it: **measure the identity out of the
engine first**, then delete. The engine-side check reads
`src/evaluation.cpp` `evaluate_pawns` through the counts the accepts names and
compares them against the signed piece-square occupancy of squares 8 to 15;
`tools/feature_audit.cpp` is where the report lands. Only then do the widths
in `tools/eval_model.hpp` move, and every base after them with the widths.

### 4. Constants and seeds

**None, and the step must not acquire any.** Nothing here is fitted: the two
folded values are read out of `src/evaluation.cpp` `passed_pawn_mg` and
`src/evaluation.cpp` `passed_pawn_eg` at this step's own HEAD and written into
sixteen table entries, and the arithmetic is exact. DEC-105's three seed forms
do not apply because there is no seed. A "seed" appearing in this step's diff
is a scope breach, not a tuning choice -- the refit is S126's.

### 5. Pitfalls

- **The rook half looks free and is not.** Zero times anything is zero, so the
  fold is arithmetically a no-op; the column still has to leave
  `tools/eval_model.hpp`, or the next fit puts a weight on an unidentified
  feature and DEC-170's ruling against regularisation loses its basis.
- **Off-by-one on the narrowed arrays.** A 7th-rank pawn that still computes
  bucket 5 reads past the end of a five-entry array. The accumulation states
  the exclusion explicitly rather than relying on the fold.
- **The bases are chained.** Every base from `PP_MG_BASE` onward moves; the
  partition properties in `tests/test_tuner_groups.cpp` are what catch a miss,
  and the accepts requires the red observed first.
- **A published fit's value for a degenerate column is not a seed and not a
  check.** If a later reader finds another engine's number for a
  rook-on-seventh bonus, it is that engine's constant wherever it is
  republished (DEC-105, DEC-134) and it says nothing about whether this fold
  is exact.

### 6. Measurement

Bit-exactness, not Elo. `tools/search_bench.py` at two depths returning
identical node counts and identical best moves discharges INV-6 (DEC-090); the
seven pinned anchor positions scoring identically is the direct check. No SPRT
is owed and one would be a category error -- the engines play identical games.
The identity report over the remaining 823 columns is the measurement DEC-170's
ruling against global regularisation now rests on, and it states method, corpus
rows and every exact dependency found.

### 7. Interactions

- **S135 (blocked by this)**: the placement group becomes three identified
  features, which is what makes its bundled verdict attributable.
- **S123 and S133 (after)**: a per-square table keeps this degeneracy for any
  feature defined on a single rank. S123's new passer terms and S133's buckets
  each need the same identity check, run through `tools/feature_audit.cpp`.
- **S126 (after)**: DEC-170 reopens the regularisation question only if S126's
  fit shows a second ridge, and this step's report is the baseline it is read
  against.
- **S117 (phase column)**: the F36 amendment above is this step's because this
  step moves `phase_value` by its own text.

### 8. References

- - https://www.chessprogramming.org/Texel%27s_Tuning_Method -- objective,
  sigmoid, `K` fitted once, "local minimum in parameter space"; **no**
  treatment of dependent parameters, redundancy, regularisation or
  overfitting. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Automated_Tuning -- supervised learning of
  evaluation weights; regularisation named only for Medvedev's logistic
  regression; redundancy, validation splits and dataset size absent; Althofer
  on small parameter sets. Fetched 2026-09-13.
- - https://github.com/AndyGrant/Ethereal/commit/8db27ad52de3e0d029d3a122b4f22e17d099ce37
  -- "Remove SCALE_OCB_TWO_KNIGHTS (#61)", +0.94 +/- 2.36, 10.0+0.1s, 35725
  games. Commit message only.
- - https://github.com/AndyGrant/Ethereal/commit/874e509ea284520c450159d3924ce09e2cc453a2
  -- "Remove scaling for OCB with 2 Rooks", +0.18 +/- 1.71 short, +0.11 +/-
  1.39 long. Commit message only.
- - `adocs/data/S100_feature_audit.txt` -- the two R^2 = 1.000000 results and
  the violation counts this step acts on. Local evidence, not literature.
