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
author:    Maksym Bodnar
