id:         S076
goal:       the fit runs on a corpus deduplicated by zobrist key, so a repeated position stops carrying repeated weight
accepts:    a tracked tool reduces an existing corpus to one row per distinct position without replaying anything, keyed on the engine's own zobrist key rather than on the FEN text, and prints how many rows it dropped; the duplicate count on `selfplay_v2.tsv` is recorded in the step, since nobody has measured it; the deduplicated corpus is refitted and **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the dedupe pass is deterministic -- run twice, same output file byte for byte
touches:    tools/, .tuning/, src/eval_tables.hpp, src/evaluation.cpp
excludes:   regenerating any corpus, which is S082's and S083's; changing datagen's filters; the label blend, which is S075's; the held-out split, which S066 fixed
decisions:  DEC-056
closes:
blocks:
paused_by:
done:

## Why this exists

`adocs/eval_tuning_strategy.md` section 2.6 lists dedupe among the filters "all
of which matter": "Deduplicate by Zobrist key; heavily repeated positions bias
the fit."

`tools/datagen` does not dedupe. Every game starts from 8 uniform random plies
and every position after a capture-free opening repeats across games; the
resulting weight on early and drawish positions is whatever the opening
randomiser happened to produce, which is not a modelling choice anyone made.

**How big the effect is here is unknown**, and that is the first thing this step
produces. 11003693 rows from 120000 games at 8 random opening plies could hold
almost no duplicates or a great many; the step records the number before it
records a verdict, because a dedupe that drops 0.1 % of rows is not worth an
SPRT and one that drops 20 % changes what the fit is fitting.

## Why key on the engine's zobrist and not on the FEN string

Two FENs that differ only in halfmove clock or fullmove number are the same
position for evaluation purposes, and the en passant field is written by
`make_move` whether or not a capture is available -- which is
`2026-08-13_adversarial-F08` and S042, still open. Keying on the text would treat
those as distinct and would inherit the same defect the plan is already fixing
elsewhere. The engine's key is one call and is the definition the rest of the
project uses.

## Interaction with the steps after it

S082 changes which position is labelled, and S083 changes how many there are.
Both make this pass part of corpus preparation rather than a one-off, which is
why it is a tracked tool and not a shell pipeline in a scratchpad -- the shape
`2026-08-16_plan_review-F04` found and S072 is cleaning up.

## Cost

A pass over 715 MB, minutes. One fit, minutes. One SPRT if the duplicate count
justifies it, three to four and a half hours -- and if it does not, that is
recorded as the result and no match is spent.

## Pre-registered, before the count is known

Written and committed before the tool was run on `selfplay_v2.tsv`, for the
reason S075's sweep script states its rules before its first fit: the step's own
body says a 0.1 % drop is not worth a match and a 20 % drop changes what the fit
is fitting, and a threshold chosen after seeing the number is not a threshold.
DEC-065 records the two design choices; these are the rules of the run.

1. **The first row of a repeated position survives, in file order, and no label
   is averaged.** DEC-065.
2. **The drop rate decides whether a match is spent.** Under **1 %** of rows
   dropped, no SPRT: the fit would be over essentially the same data and a
   verdict would be measuring the noise floor S075 put at 4e-05 of held-out
   error. At or above 1 %, exactly one candidate -- the vector the refit emits
   -- plays one SPRT against the weights that ship.
3. **A candidate that rounds to the incumbent does not play either.** If
   `.tuning/diff_fit.py` reports no constant changed between the emitted table
   and `src/eval_tables.hpp` plus `src/evaluation.cpp`, there is nothing for a
   match to measure and the recorded result is the dedupe count.
4. **No sweep.** One dedupe rule, one fit, at most one verdict. There is no
   bake-off between keep-first and label-averaging, and no second fit at another
   setting: that is the multiple-comparison shape S075's step file names, and
   held-out error is not Elo (DEC-019).
5. **The refit uses S065's and S075's settings**, so the only thing that moved
   is the corpus: `--freeze tempo,piece_placement --seed 1 --validation 0.1
   --threads 12`, K fitted from the data, `--lambda 0`.

## The count, which nobody had measured

`.tuning/selfplay_v2.tsv`, 11003693 rows from 120000 games, through
`build/tools/corpus_dedupe` in **19 s** and 1.0 GB resident:

```
11003693 rows read, 10795695 distinct positions written, 207998 dropped (1.8903%)
repeats: 10733523 keys once, 35124 twice, 9585 three times, 10547 4-7, 4284 8-15, 2632 16 or more; most repeated 120
```

**1.8903 %**, which clears the pre-registered 1 % and buys the match. The step
file's two illustrative extremes were 0.1 % and 20 %; the answer is an order of
magnitude above the first and an order below the second.

**The repeats are concentrated, not spread.** 10733523 of the 10795695 distinct
positions occur exactly once, so 99.42 % of the corpus's positions are already
unique and the whole 1.89 % comes from **62172** positions — 0.58 % of them —
that repeat. The tail is what carries it: 2632 positions appear 16 times or
more, one appears **120** times, and those 2632 alone account for at least 42112
rows and so for at least 39480 of the drops — 19 % of everything dropped, from
0.02 % of the positions. That is the shape the eval_tuning_strategy sentence
describes -- "heavily repeated positions bias the fit" -- rather than a
uniform thinning.

## The pass is deterministic, and no collision hid inside the count

Two runs over the same input, byte for byte:

```
0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f  .tuning/selfplay_v2_dedup.tsv
0a6b59b6af9f50c4360cac7c87c33250318e712250f2653eb09bcb9f0b64b69f  .tuning/selfplay_v2_dedup_run2.tsv
```

Which is what the accepts asks for, and it is deterministic by construction
rather than by luck: one thread, one pass, output in input order, and
`init_zobrist()` is fixed-seed (`src/bitboard.cpp:2597`) so the keys are the
same in every process.

A third pass with `--verify`, 26.8 s and 1.9 GB resident, holds the four hashed
FEN fields per key and compares them on every hit: **0 of the 207998 key-equal
rows disagreed**. So the 1.8903 % is 207998 genuine repeats and not a birthday
collision inflating the count. The exposure it rules out is small — about 3e-06
over 11.0 M keys — and it is now measured instead of bounded.

## What it cost the game-level split: 20 blocks of 119998

DEC-065 predicted the direction and the tuner's own first line measures it.
`tuner_split` reconstructs a game boundary from a ply that does not advance, so
removing rows can never split a game and can only merge two — and the merged
block still lands wholly on one side of the cut, which is S066's asymmetry.

| corpus | rows | blocks | held out |
|---|---|---|---|
| `selfplay_v2.tsv` | 11003693 | 119998 | 1100388, 10.0002 % |
| `selfplay_v2_dedup.tsv` | 10795695 | **119978** | 1079625, 10.0005 % |

20 boundaries lost, 0.017 % of the blocks, against 120000 games datagen
reported. The split is 20 games coarser and no more contaminated than it was.

## The refit, and the one number in it that is not about weights

`adocs/data/S076_dedupe_fit.sh`, 1378 s, best at epoch 1600 and stopped by
patience at 3600 of the 5000 allowed. Held-out error against the game result,
over the deduplicated corpus's own held-out rows: **0.119608 at the shipping
constants → 0.119458**, a move of 1.5e-04 against the 4e-05 noise floor S075
measured. Not comparable with the 0.118457 the same constants score on the full
corpus — a different row set is a different number, which the script says before
the run rather than after it.

**K fitted to 0.7801.** The comparison that makes this a measurement rather than
a curiosity: S075's lambda 0 control fitted K on the *full* corpus from the
*same* shipping constants and got **0.7595**. K is fitted at the starting
parameters against the game result, so with the starting parameters held the
only input that changed is the corpus. Removing 1.9 % of the rows moved the
score-to-outcome scale by 2.7 %, which is what says the dropped rows were not a
random 1.9 % — they are repetitions, and a position repeated on the way to a
draw carries the label the search's score least agrees with.

## The weights moved ten times further than the epochs alone move them

Same budget, same seed, same freeze, same starting constants, and the only
difference is the corpus. Against `git HEAD` before the paste:

| | S075 lambda 0, full corpus, 5000 epochs | S076, deduplicated, 3600 epochs |
|---|---|---|
| material defines | `QUEEN` +15, rest ≤ +3 | `PAWN` -1, `BISHOP` +2, rest 0 |
| psqt rook mg, mean | +2.3 | **-51.2** |
| psqt rook eg, mean | -1.8 | **+59.9** |
| largest single square | 74 | **761** |

That is the attribution the SPRT script pre-registers, carried as loss and as
weights rather than as a second verdict: more training on the same corpus does
not do this, so the corpus did.

The material defines barely moving while the tables move by tens is the
degeneracy `tools/tuner.cpp:26-30` documents, read from the other side. What
the fit did to the rook is a taper change and not a degeneracy shuffle: mg down
51 and eg up 60 cannot be absorbed into a material constant, which only moves
both together.

## Re-anchoring, and the two guards the paste fired

Pasting 827 constants fires the same two guards S065 met, and both are answered
by re-derivation rather than by relaxing a threshold.

**The ten pinned absolute anchors.** `.tuning/anchors.py` is a second
implementation of `evaluate()` for these positions, written from the
specification so that an anchor is never copied from the thing it anchors. Run
against the pasted weights it derives 135, 244, 325, 563 (with `evaluate_cheap`
567), 787, 0, 198, -569 and the -505 quiescence composite, and the suite's ten
assertions now hold exactly those values: **10 of 10 reproduced**. Five of the
six piece anchors went up while `PAWN` went *down* by one, which is the
degeneracy again — the tables under those pieces moved, not what a piece is
worth.

**The truncation-bound guard.** `test_eval_model`'s pinned four have to
disagree with the float model by more than 2.0, and one of them by more than
2.8, or the tolerance in "the model reproduces evaluate() on every phase" is
asserted against a corpus that no longer exercises it. S065's four fell to
0.083, 0.958, 1.833 and 1.000 under the new weights — exactly the failure its
own comment predicts, since **a residual belongs to the weights and not to the
position** (DEC-057).

Re-measured with `build/tools/truncation_scan`, new and tracked at this step
because the same scan had been done by hand twice, at S038 and S065, from
scripts in session scratchpads that no longer exist —
`2026-08-16_plan_review-F04`'s shape. Over all 10795695 rows: 135399 past 2.0,
99 past 2.8, and **30 at the arithmetic maximum 69/24 = 2.875**. The four pinned
are chosen from those 30 to span the taper with both sides to move; phase 17 has
no maximal position under these weights, so the middle two are phases 11 and 19
where S065's were 11 and 17. The thresholds are untouched.

The tool is checked against the test rather than trusted: on S065's four
positions it prints 0.083333, 0.958333, 1.833333 and 1.000000, which is what
`test_eval_model` printed when it went red, to six digits.

**`piece_values_abs` is not affected.** The move-ordering bands clear each other
by 100 points and a fitted material value can invert that silently — `CLAUDE.md`
names it as a one-way door. It reads `MVV_*` and not the fitted defines
(`src/evaluation.cpp:41-43`), so the paste cannot reach it. Checked, not
assumed.
author:    Maksym Bodnar
