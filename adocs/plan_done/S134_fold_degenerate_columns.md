id:         S134
goal:       delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
accepts:    the engine-side identity is measured before anything is deleted -- `passed_pawn_counts()` and `piece_placement_counts()` read out of the engine, not out of `eval_model`, and compared against the signed piece-square occupancy of squares 8..15 over the whole corpus, 0 violations required and the non-zero row count reported so the check is not vacuous; the fold is **bit-exact and shown to be**, by identical scores on the seven pinned anchor positions and by `tools/search_bench.py` returning identical node counts and best moves at two depths (INV-6 discharged on node counts, no SPRT owed -- DEC-090); `PIECE_PLACEMENT_COUNT` 4 to 3 and `PASSED_PAWN_COUNT` 6 to 5, `PARAM_COUNT` 827 to 823, with `test_tuner_groups`' three partition properties green **and observed red** under a base left unshifted; `test_eval_model`'s hand cases, differential sweep and non-vacuity lists re-targeted to the narrowed features rather than deleted; `tools/feature_audit` still runs and its identity report states the rank/identity result for the **remaining** parameterisation, including the method, corpus rows and every exact dependency found -- no unreported exact degeneracy remains after the two folds, or a new finding is filed before a fit; that measurement is the recorded basis for DEC-170's decision against global regularisation, and S126 reopens it if its fit shows another ridge; the fast suite green
touches:    src/evaluation.cpp (the two terms and their accumulation), src/eval_tables.hpp (the sixteen folded entries), tools/eval_model.hpp (widths and bases), tools/tuner_model.hpp (the phase column), tools/tuner_groups.hpp, tools/feature_audit.cpp, tests/test_eval_model.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the other three placement features, which is S135; tempo, which is S136; any refit -- this step moves numbers between two places that add up to the same score and fits nothing
decisions:  DEC-090, DEC-258
closes:     2026-09-10_adversarial-F34, 2026-09-10_adversarial-F36, 2026-09-12_adversarial-F02
blocks:     S135
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-12 -- passer bucket 5 and rook on the seventh folded into the tables and deleted, bit-exact (DEC-090). Engine identity first, at `1472c96`: `passed_pawn_counts()` bucket 5 and `piece_placement_counts()` rook seventh equal the signed occupancy of squares 8..15 on all 10795695 rows, 550880 and 1264773 non-zero, 0 violations (`adocs/data/S134_engine_identity.*`). Pawn `psqt[8..15]` mg -17 / eg +42 each, rook +0; `evaluate()` identical on every corpus row (546224 rows move without the fold); `search_bench.py` d9 and d12 node counts and best moves identical; bench 3872032 and `bench 12` streams identical; ten anchors identical. Counts 6 to 5, 4 to 3, `PARAM_COUNT` 827 to 823; `test_tuner_groups` green and observed red under an unshifted base (property 1 and the chain precondition; disjointness and union cannot fire here). `test_eval_model` re-targeted; a guard row catches a bucket written past Black's row, observed red with each colour's seventh-rank mask removed alone. F36: `load()` recomputes the phase, observed red first, precondition read from the file. `feature_audit` identity report: rank exactly 784 of 823 -- modular lower bound, 39 exact independent null vectors for the upper -- nullity 39 accounted for, no further dependency; 4 found on the 827 layout at the same rank. The king-table gauge pair and the 32 zero pawn columns it found are S275. Fast suites 44/44 both, format clean, prose checks clean. No timing. DEV_MANUAL (tuner first lines requoted from a re-run) and specs updated, MANUAL checked (no change). Fast check: Black's seventh-rank mask had no clean guard (a sentinel row now catches it), stale tuner quotes and counts, the rank argument's upper bound and a vacuous F36 precondition, all fixed before commit.

## Ordered 2026-10-08, DEC-258

Lands before S262's trace tuner is fitted on, so the trace is built on the
823-parameter layout and no tuner is updated twice.

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

## As built (2026-10-12)

### Deviations

- **Files outside `touches:`, each because the narrowing broke or staled it.**
  `tools/tuner.cpp`: it emitted `passed_pawn_%s[6]` and `piece_placement_%s[4]`
  as literals, which would have padded a fitted header with a zero and compiled;
  it now prints the model's widths. `tests/test_evaluation.cpp`: the packed-halves
  case passed literal widths 6 and 4 and would have read past the narrowed
  arrays without a compiler word; it reads the packed arrays' `size()`.
  `adocs/data/S192_anchors.py`: its `placement` tuples were four wide and the
  script died on `IndexError`; narrowed to three, every dropped entry zero.
  `src/bitboard.cpp`: comment only, in `load_FEN`'s back-rank refusal, which
  described a six-entry table. `tools/CMakeLists.txt`: the `feature_audit`
  comment. `adocs/data/README.md`: index rows for the four new data files.
  `tools/tuner_groups.hpp` is in `touches:` and is unchanged: its ranges follow
  `eval_model.hpp`'s base chain.
- **The engine-side identity check is not in the final tree.** The two counts
  it reads are what this step deletes, so it is kept as a patch against
  `1472c96`, `adocs/data/S134_engine_identity.diff`, beside its output.
- **`test_tuner_groups`' red is not where the Hazards section expects it.**
  Every group ends at the next group's start and `tempo` runs to `PARAM_COUNT`,
  so a base left at its 827-layout value (too high) can empty `tempo` but cannot
  make two groups overlap or leave a hole: property 1 and the chain
  precondition fire, disjointness and union never do. Observed below.
- **F36: the tuner recomputes**, it does not assert. The step's amendment says
  this step moves `phase_value`; it does not -- `phase_value` is untouched --
  but `closes:` names F36 and the fix is the step's text, so it was done.
- **"Seven pinned anchors"**: the anchors script pins ten, all ten identical
  before and after. None of the ten has a pawn or a rook on its own seventh, so
  they cannot see this fold; the corpus-wide `evaluate()` comparison below is the
  check that can.
- **2026-09-10 F34 and F36 had no `Status:` line** (Part E's lows never got
  one). One line each was added, reading closed, "Previously: open (this
  report's header: open on all of them)". F02 of 2026-09-12 had one and it moved.
- **Not fixed, filed as S275**: the identity report's 39 null directions
  include **two that were on no record** -- one per king table (`psqt_mg[king]`
  and `psqt_eg[king]` each sum to zero on every row, one king a side) -- and
  **32 pawn columns zero on every row** (ranks 1 and 8). Both are structural and
  do not move the evaluation; the king pair is the same class as the five
  documented material-against-table directions, a split Adam's per-coordinate
  steps can drift along and no fit can identify. The coordinator filed it as
  S275, filler before S262's fit.
- **The fast check's three fixes**, made in this step before it closed:
  Black's seventh-rank mask had no clean guard (a bucket 5 written past Black's
  row was undefined behaviour, a stack smash in a later case, not a failed
  check) -- `test_eval_model` now reads the engine's rows through a guard row;
  `DEV_MANUAL.md` quoted the tuner's 827-era first lines and `tools/tuner_groups.hpp`
  and `tests/test_tuner_groups.cpp` carried 817/827/825 -- re-run and requoted;
  the identity report stated only the lower bound of the rank -- it now pins
  the rank from both sides; and the F36 test's precondition was vacuous -- it
  now reads the column back from the file.

### The identity, measured from the engine before anything was deleted

`feature_audit` at `1472c96` plus `adocs/data/S134_engine_identity.diff`: every
row of `.tuning/selfplay_v2_dedup.tsv` through `load_FEN`, then
`passed_pawn_counts()` and `piece_placement_counts()` against the signed
occupancy of squares 8..15 of the piece list the fit reads.
`adocs/data/S134_engine_identity.txt`:

- 10795695 rows read, 0 refused by `load_FEN`.
- bucket 5, White minus Black, against signed pawn occupancy: **550880 rows
  non-zero, 0 violations**.
- rook seventh, White minus Black, against signed rook occupancy: **1264773 rows
  non-zero, 0 violations**.
- per colour, against the board's own bitboards: 569739 rows with a pawn on its
  own seventh, 1380393 with a rook, 0 violations each.
- Not vacuous: the same check reading bucket 4 instead of 5 reports 2872
  violations on a 20000-row slice.

### The fold, and that it is bit-exact

- `src/eval_tables.hpp` `psqt_mg[PAWN][8..15]` each -17 (216 118 96 206 172 43
  12 112 to 199 101 79 189 155 26 -5 95) and `psqt_eg[PAWN][8..15]` each +42
  (187 228 199 156 137 204 248 203 to 229 270 241 198 179 246 290 245). The rook
  entries take +0 and do not change. Read from `passed_pawn_mg` /
  `passed_pawn_eg` and `piece_placement_mg` / `piece_placement_eg` at `1472c96`;
  nothing fitted, no seed.
- `src/evaluation.cpp`: `passed_pawn_mg` / `passed_pawn_eg` five wide,
  `PL_ROOK_SEVENTH` gone, `piece_placement_mg` / `piece_placement_eg` three wide.
  `evaluate_pawns()` masks each side's own seventh out of the passer set
  explicitly (`rank_masks[a7]` for White, `rank_masks[a2]` for Black) before any
  bucket is computed; the debug asserts read `bucket < 5`.
- **`evaluate()` and `evaluate_cheap()` identical on all 10795695 corpus rows**,
  parent library against this one, byte-compared dumps (569739 of the rows carry
  a pawn on its own seventh, 1380393 a rook). Not vacuous: the deletion without
  the fold, the tables put back, changes `evaluate()` on 546224 rows.
- `tools/search_bench.py`, parent binary against this one, identical node counts
  and best moves: depth 9 midgame 32829 `c3d5`, kiwipete 69892 `e2a6`, tactical
  25174 `d7c8q`; depth 12 midgame 68090 `c3d5`, kiwipete 246712 `d5e6`, tactical
  114555 `d7c8q`. INV-6 discharged on node counts; no SPRT owed (DEC-090).
- `bench` **3872032**, the parent's, and its whole stream identical with `time`
  and `nps` stripped; `bench 12` 1730486, stream identical.
- Anchors: `python3 adocs/data/S192_anchors.py` prints "10 of 10 reproduced",
  output byte-identical to the same script over the parent's sources.
- `tests/test_evaluation.cpp`, `test_eval_model` and `test_tuner_gradient` also
  pass in a Debug build, with the bucket asserts live.
- No timing taken; none is claimed.

### Counts and the partition

- `PASSED_PAWN_COUNT` 6 to 5 (now taken from the engine array's width, as the
  other two counts are), `PIECE_PLACEMENT_COUNT` 4 to 3, `PARAM_COUNT` 827 to
  823; bases from `PP_EG_BASE` on shift by 1, from `PS_MG_BASE` on by 2, from
  `PL_EG_BASE` on by 3, from `TEMPO_MG_BASE` on by 4. Groups: 5, 768, 8, 18, 10,
  6, 6, 2. The tuner's first line reads "823 parameters, group all, freeze
  (nothing), 823 free" and its emitted header `passed_pawn_mg[5]` /
  `piece_placement_mg[3]`.
- `test_tuner_groups` green, 4/4, 1749 assertions. **Observed red** with one
  base pinned at its 827-layout value (`.tuning/coord/S134/groups_red.log`):
  `PS_MG_BASE` at 811 and `PL_MG_BASE` at 817 each turn all four cases red --
  `REQUIRE( 825 == 823 )` on the chain precondition, "group frees no parameter
  at all: tempo" on property 1, and both `--freeze` cases on their `tempo`
  preconditions; `PP_EG_BASE` at 805 turns the chain precondition alone red,
  `REQUIRE( 824 == 823 )`. Disjointness and union stay green in all three, for
  the reason under Deviations.

### Tests re-targeted, none deleted

- `test_eval_model`: names three wide, both `static_assert`s re-pinned (5, 3),
  every engine row sized by the model's counts. Passed pawn hand cases: the two
  "one square from promotion" cases now expect every bucket zero -- "a white
  (black) pawn on its own seventh is in no bucket". All three callers read the
  engine's rows through `engine_passed_counts()`, which hands
  `passed_pawn_counts()` a third guard row filled with -12345: White's bucket 5
  lands in Black's bucket 0, which the per-colour rows catch, and Black's lands
  in the guard row instead of past the caller's array. **Observed red with each
  mask removed alone** (`.tuning/coord/S134/seventh_mask_red.log`): Black's
  alone, the guard fires in all three passed pawn cases, `CHECK( -12344 ==
  -12345 )` "passed_pawn_counts() wrote past Black's row", on the hand case
  `4k3/8/8/8/8/8/p7/4K3` and on four curated positions; White's alone, the
  per-colour and difference checks fire, `CHECK( 1 == 0 )` on Black's row for
  `4k3/P7/8/8/8/8/8/4K3`, no crash. Run whole under Black's mutant the binary
  still aborts ("stack smashing detected") in the pawn structure case: that is
  the engine's own `pawn_structure_counts()` scratch array overflowing, inside
  `src/`, after the guard has already failed. "f6 stops e4"
  had its black passer on a2 (bucket 5); it moved to a3 (bucket 4) so the
  answer stays non-zero. The two ladders lost their trailing 0. Piece placement
  hand cases: the fourth column dropped; the four seventh-rank cases now pin
  that a rook on either seventh or its own second counts for its file alone.
  Non-vacuity lists unchanged in form; every bucket 0..4 and every feature 0..2
  is still claimed positive, negative and per colour.
- `test_tuner_gradient`: new case "load() tapers on the placement's phase, not
  the stored column" (F36), **observed red** before the fix, 34 of 34 rows
  (`CHECK( 11 == 4 )` first); `write_fixture` takes a phase shift for it. Its
  precondition reads the phase column back from the written file and requires
  it wrong on every row; observed red at shift 0, `REQUIRE( 0 == 34 )`, and the
  check red again with `load()` mutated to trust the column, 34 failed
  (`.tuning/coord/S134/f36_red.log`).
- `test_evaluation`: the packed-halves widths read from the arrays.
- No golden moved, so none was re-derived: bench, the ten anchors (re-derived
  by their script anyway, identical) and the four truncation positions (engine
  scores unchanged, the case green) all hold.

### `tools/feature_audit`

`adocs/data/S134_feature_audit.txt`, all 10795695 rows, 1 m 9 s:

- columns: 0 disagreements; stored phase column against the placement's phase,
  0 disagreements.
- identities, the new report: method -- each row is 24 x the derivative of
  `eval_model::evaluate()` with respect to every parameter, clamp ignored as
  `gradient()` ignores it; `X'X` summed exactly in 64-bit integers over every
  row; rank by Gauss-Jordan modulo 2^61-1, a lower bound on the rational rank;
  each dependency checked exactly as `v'X'Xv == 0`. Design check: `x.params/24`
  against `evaluate_position()` on 202966 unclamped rows, worst 1.36e-12.
  **Rank exactly 784 of 823, nullity 39**: 32 pawn columns zero on every row
  (ranks 1 and 8), five material-against-table directions and one per king
  table, all seven holding on every row; the 784 columns left are full rank
  modulo the prime, **no further exact dependency**. Exact from both sides: the
  modular rank, 784, is a lower bound on the rational rank; the 39 null vectors
  hold exactly, have pairwise disjoint supports and are independent (rank 39
  stacked, modulo the prime), so the rational rank is at most 823 - 39 = 784.
  The same report on the 827 layout (`adocs/data/S134_identity_before.txt`,
  re-run with the bounds): rank exactly 784, and exactly four further
  dependencies, each `+ passed_pawn_{mg,eg}[5]` or `+
  piece_placement_{mg,eg}[rook seventh]` minus its eight seventh-rank columns,
  holding on every row; 43 independent null vectors there (supports not
  disjoint, independence from the stacked rank). Same rank: the fold removed
  four dependent columns and nothing the tables could not express. This is the measurement DEC-170's ruling
  against global regularisation rests on.
- occurrence and collinearity: the eight term columns left read exactly S100's
  figures (passer buckets 0-4 R^2 0.216 to 0.620, pair 0.487, rook open 0.265,
  half-open 0.168).
- `--plant` writes five passer and three placement weights (S100's numbers less
  the two dropped columns).

### Suites

`CLANG_FORMAT_MAJOR=22`, i7-8700K, g++ 13.3, `-j12`: build 44/44 fast, build-tune
44/44 fast, `./clang-format.sh --check` clean, exit 0
(`.tuning/coord/S134/gate3.log`, after the fast check's fixes).
`tools/plan_prose_check.py` `--citations` 0 flagged over 36 files, `--touches`
0 flagged, `--params` silent, exit 0 each. `bench` 3872032 on the final
binary. `DEV_MANUAL.md`'s two tuner first lines are quoted from a re-run
(`.tuning/coord/S134/tuner_quotes.log`): `823 parameters, group all, freeze
(nothing), 823 free` and `freeze tempo,piece_placement, 815 free`.
No second tier: `make_move`, the generator and the search are untouched;
`src/bitboard.cpp`'s change is a comment in `load_FEN`.

### Proposed `done:` stamp

2026-10-12 -- passer bucket 5 and rook on the seventh folded into the tables
and deleted, bit-exact (DEC-090). Engine identity first, at `1472c96`:
`passed_pawn_counts()` bucket 5 and `piece_placement_counts()` rook seventh
equal the signed occupancy of squares 8..15 on all 10795695 rows, 550880 and
1264773 non-zero, 0 violations (`adocs/data/S134_engine_identity.*`). Pawn
`psqt[8..15]` mg -17 / eg +42 each, rook +0; `evaluate()` identical on every
corpus row (546224 rows move without the fold); `search_bench.py` d9 and d12
node counts and best moves identical; bench 3872032 and `bench 12` streams
identical; ten anchors identical. Counts 6 to 5, 4 to 3, `PARAM_COUNT` 827 to
823; `test_tuner_groups` green and observed red under an unshifted base
(property 1 and the chain precondition; disjointness and union cannot fire
here). `test_eval_model` re-targeted; a guard row catches a bucket written past
Black's row, observed red with each colour's seventh-rank mask removed alone.
F36: `load()` recomputes the phase, observed red first, precondition read from
the file. `feature_audit` identity report: rank exactly 784 of 823 -- modular
lower bound, 39 exact independent null vectors for the upper -- nullity 39
accounted for, no further dependency; 4 found on the 827 layout at the same
rank. The king-table gauge pair and the 32 zero pawn columns it found are
S275. Fast suites 44/44 both, format clean, prose checks clean. No timing.
DEV_MANUAL (tuner first lines requoted from a re-run) and specs updated,
MANUAL checked (no change).

### Commit text

```
Fold passer bucket 5 and rook seventh into the tables (S134)

Both features were exactly the signed sum of eight piece-square columns
(S100, DEC-090), so no fit could identify their split from the tables.
Measured from the engine first: passed_pawn_counts() bucket 5 and
piece_placement_counts() rook seventh equal the occupancy of squares
8..15 on every corpus row, 0 violations. Bucket 5's mg -17 / eg +42
move into psqt[PAWN][8..15], rook seventh's 0 into psqt[ROOK][8..15],
and both are deleted: PARAM_COUNT 827 to 823. The pawn terms join the
piece-square pair before the one taper, so evaluate() is identical on
all 10795695 corpus rows and search_bench node counts match at depths
9 and 12 (INV-6); no SPRT is owed.

feature_audit's identity report now measures the whole design: rank
exactly 784 of 823, bounded below modulo a prime and above by 39 exact,
independent null vectors, all by construction, no further exact
dependency -- the basis DEC-170's ruling against regularisation rests
on. The king-table pair among them is S275. The tuner recomputes the
phase from the placement instead of trusting the corpus column. Closes
2026-09-10_adversarial-F34, F36 and 2026-09-12_adversarial-F02.

No functional change
```
