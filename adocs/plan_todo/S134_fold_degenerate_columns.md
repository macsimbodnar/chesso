id:         S134
goal:       delete rook-on-the-seventh and passer bucket 5 by folding their weights into the piece-square tables, which is bit-exact, and shrink the parameter vector to 823
accepts:    the engine-side identity is measured before anything is deleted -- `passed_pawn_counts()` and `piece_placement_counts()` read out of the engine, not out of `eval_model`, and compared against the signed piece-square occupancy of squares 8..15 over the whole corpus, 0 violations required and the non-zero row count reported so the check is not vacuous; the fold is **bit-exact and shown to be**, by identical scores on the seven pinned anchor positions and by `tools/search_bench.py` returning identical node counts and best moves at two depths (INV-6 discharged on node counts, no SPRT owed -- DEC-090); `PIECE_PLACEMENT_COUNT` 4 to 3 and `PASSED_PAWN_COUNT` 6 to 5, `PARAM_COUNT` 827 to 823, with `test_tuner_groups`' three partition properties green **and observed red** under a base left unshifted; `test_eval_model`'s hand cases, differential sweep and non-vacuity lists re-targeted to the narrowed features rather than deleted; `tools/feature_audit` still runs and its identity report now has nothing left to check, which is stated rather than silently dropped; the fast suite green
touches:    src/evaluation.cpp (the two terms and their accumulation), src/eval_tables.hpp (the sixteen folded entries), tools/eval_model.hpp (widths and bases), tools/tuner_groups.hpp, tools/feature_audit.cpp, tests/test_eval_model.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md, adocs/specs.md
excludes:   any change to the other three placement features, which is S135; tempo, which is S136; any refit -- this step moves numbers between two places that add up to the same score and fits nothing
decisions:  DEC-090
closes:
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
