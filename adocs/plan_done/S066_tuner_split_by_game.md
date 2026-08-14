id:         S066
goal:       hold out whole games from the fit, not rows, so the validation error is not shared with training
accepts:    a fit over a corpus `datagen` produced puts no game on both sides of the validation split, asserted by a test over the splitter rather than by inspection; the split is still randomised and still lands within a stated tolerance of `--validation`; the S065 corpus is fitted under both splitters at the same seed and the two held-out figures are recorded side by side, with zero difference recorded as zero
touches:    tools/tuner.cpp, tools/tuner_split.hpp, tests/test_tuner_split.cpp, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   any change to what `evaluate()` computes; any change to the objective, the optimiser or K; re-deciding anything S027 or S028 concluded, since those verdicts are SPRTs and this touches a diagnostic only
decisions:  DEC-019, DEC-056
closes:
blocks:
paused_by:
done:      2026-08-14. The tuner holds out whole games. Boundary detection over .tuning/selfplay_v2.tsv, 11003693 rows: the FEN ply finds 119998 blocks against the 120000 games datagen reported, so at most 2 missed boundaries, 0.0017 % -- the move number alone misses 148, 0.1233 %, which is why the detector is the ply and not the descent this file proposed (DEC-056). Games straddling the split: 119360 of 119999, 99.467 %, before; 0 of 119999 after, and zero by construction because the predicate only fires where a within-game invariant is violated, so it can never cut a game in half. Held-out rows 1100388 against the 1100369 asked for, 10.0002 %.
            
                        The same corpus fitted under both splitters at the same pinned K = 0.7615, seed 1 and 3000 epochs: held out 0.117352 row level against 0.117380 game level. That difference is 2.8e-05 against the 3.7e-04 the two held-out sets differ by before a single gradient step, so it is dominated by which rows each split took and is recorded as **zero**. 827 parameters over 9.9 M rows cannot memorise a game. The splitter is kept because the old one shared games by construction, not because a number moved. Side effect, measured: the block-ordered index runs the identical fit in 1090.87 s against 2333.91 s, 2.14x.
            
                        Training behaviour unchanged, two ways. evaluate_position, sigmoid, error_range, gradient, fit_k and the Adam loop are byte-identical to dd57c3c. End to end, on 200000 rows whose FEN move number was rewritten to descend so that every row is its own game and the new index is provably the old permutation: every epoch report identical to six digits and the 827 emitted constants byte-identical at sha256 f710eaff...5399.
            
                        One defect introduced here and fixed here, before the commit, by this step's own test: a one-game corpus held all of itself out and gradient() divided by a count of zero, printing validation -nan. The last block is never held out now and main() says when nothing could be. tests/test_tuner_split.cpp, 10 cases, 6513 assertions, label fast, observed red three ways.

## What is there

`tools/tuner.cpp:791-805` builds an index over **positions** and shuffles it:

```
  // Positions from one game are consecutive and share a label, so a split that
  // cuts the file in two puts whole games on one side and correlates the
  // validation set with nothing. Shuffling first is what makes the held-out
  // error mean something.
  std::vector<uint32_t> index(data.size());
  ...
  std::shuffle(index.begin(), index.end(), rng);

  const size_t validation_count =
      static_cast<size_t>(static_cast<double>(data.size()) * opts.validation);
  const size_t train_count = data.size() - validation_count;
```

The comment is right about what it fixes and wrong about what is left. What
shuffling removes is the block structure. What it does not remove is the
dependence: a game contributes about 94 rows, all sharing one label, and after
a row-level shuffle those rows land on both sides of the cut.

`tools/datagen.cpp` writes a game's rows consecutively -- the whole `samples`
loop runs inside one `std::lock_guard` on `output_lock`, so no other worker can
interleave a row into it.

## Verified here, two ways

**From the file.** Over the 56304-row S065 smoke corpus, counting descents in
the FEN's fullmove number: **600 contiguous runs over 56304 rows, for 600
games** -- one run per game exactly, so a game's rows really are a contiguous
block.

**By simulating the split.** Marking each row into validation with probability
0.1 over the real per-game row counts, three trials:

```
games 600  rows 56304  mean rows/game 93.84
trial 0: games straddling the split 599 (99.833%), train-only 1, val-only 0
trial 1: games straddling the split 599 (99.833%), train-only 1, val-only 0
trial 2: games straddling the split 597 (99.500%), train-only 3, val-only 0
```

**99.5 to 99.8 % of games contribute rows to both sides.** Analytically the
same figure: a game escapes the validation set with probability `0.9^94`, which
is 5.1e-05 here and 3.9e-04 at S028's 74.5 rows per game. The held-out set is
not held out.

## Why it is worth a step

Not because any verdict is in doubt -- every S027 and S028 verdict is an SPRT
and this touches no game. Because the project uses held-out error to **choose**
what to run, and S027's own carried-forward lesson is that held-out error does
not predict Elo in either direction. A split that shares games with the
training set is one documented reason a held-out figure would be optimistic,
which makes a weak instrument weaker.

The literature raises the same violation from the other end: Österlund states
that many positions per game "violates least-squares assumptions of independent
data points" and argues the games are the independent units; Grant §2.2 says
using many positions from the same game produces a lower quality dataset. CPW,
Texel's Tuning Method. Reported by the 2026-08-14 evaluation survey as its C13
and re-verified above against the code before being written down. No audit
finding covers it.

## Why it sits ahead of S065

S065 is the next step that reads a held-out figure -- its accepts asks the fit
to beat its own starting error with no refusal warning, and that gate is
computed on this split. This step costs minutes and changes no game; S065's
datagen run does not depend on it and can generate while this lands. What must
not happen is S065's *fit* being read through a splitter already known to be
optimistic.

## Two shapes, and the choice is not made here

Either `datagen` emits a game id as a fifth column and the tuner groups on it,
or the tuner reconstructs game boundaries from the contiguity the file already
has and shuffles blocks instead of rows. The first is explicit and costs a
column and a corpus regeneration to apply to old data; the second reads a
corpus written before the change, S065's included, and rests on contiguity
being a property `datagen` guarantees rather than merely exhibits. Decide it in
the step, and record it.

A fifth column would also change what `tools/eval_spread` and any other reader
of the four-column format see, so whichever shape is chosen, the readers are
part of the step.

**Decided: reconstruct, DEC-056.** The corpus that exists cost seven hours and
11003693 rows and cannot take a column retroactively, and no reader of the
four-column format changes. Nothing in `tools/` other than `tuner.cpp` was
touched, so `eval_spread` sees the same four columns it always did.

## What was built, and the two corrections to the text above

`tools/tuner_split.hpp`, on the S041 precedent that put `free_mask` where a test
could reach it. Three functions: `row_ply` parses one row's ply, `game_starts`
finds the block boundaries, `split` shuffles the blocks and holds whole ones
out. `tools/tuner.cpp` keeps a `ply` per row in `dataset_t` and calls all three.

**The detector is the ply, not the descent in the move number this file
proposed.** `2 * (fullmove - 1) + (black to move)`. Both are sound for the same
reason -- neither can go backwards inside a game -- but on the real corpus the
move number alone misses sixty times as many boundaries, and the extra bit is
the side to move, which `load()` already parses for the tempo term:

```
rows 11003693, unparsed 0                       datagen reported 120000 games
move number descends            119852 blocks   at most 148 missed, 0.1233 %
ply does not advance            119998 blocks   at most   2 missed, 0.0017 %
... or a piece appeared         119998 blocks   at most   2 missed, 0.0017 %
... or castling back or label   119999 blocks   at most   1 missed, 0.0008 %
```

**The failure mode is not contamination.** This file called the no-descent case
a failure and left its cost unstated. It costs granularity: a missed boundary
merges two adjacent games into one block, and a block is indivisible, so both
games still land on the same side. What the detector can never do is *split* a
game, because it only fires where a within-game invariant is violated. Zero
straddling games is therefore a property of the construction. Measured anyway,
using the finest sound partition above as the truth the file does not carry:

```
old row-level split, seed 1, 0.10:  119360 of 119999 games straddle, 99.467 %
new block split,     seed 1, 0.10:       0 of 119999 games straddle
                                    1100431 held out under the move number
                                    1100388 held out under the ply, 10.0002 %
```

The 148 the move number misses are all single merges -- 147 of 119852 blocks
hold exactly two of the finer blocks and none holds three -- so even the weaker
detector never straddles. It was still dropped: a coarser split for no reason is
not worth keeping.

## The defect this step introduced, and the behaviour chosen for it

A block is indivisible in **both** directions. Holding out a fraction of a
one-game corpus is impossible, and giving up all of it left `gradient()`
dividing by a count of zero. Measured, on a two-row corpus at
`--validation 0.5`:

```
2 positions, 1 games, 0 train, 2 validation (100.0000%), 827 parameters, ...
start: train 0.000000  validation 0.195436
epoch      1  train 0.000000  validation -nan
```

The row-level split could always cut one row off a game, so no earlier version of
the tuner could reach this. It is reachable now, and not hypothetically: S040 and
S041 both measured the `--only` groups over a two-row TSV.

**Clamped, not refused.** Two defensible answers and the reasoning for the one
taken:

- **Clamp so one block always survives.** Chosen. Every invocation that worked
  before this change still works, which matters because the two-row invocation is
  a real workflow here and refusing it would make `--validation 0` mandatory for
  it. The clamp is `first_held > 1` in `split()`.
- **Refuse the fraction up front.** Rejected. It turns a working command into an
  error for a corpus the tuner can still fit perfectly well, and it puts the
  decision at the CLI where the caller cannot know how many games their file
  holds until the tuner has parsed it.

Clamping silently would be the wrong half of the choice, so `main()` says it:

```
2 positions, 1 games, 2 train, 0 validation (0.0000%), 827 parameters, ...
WARNING: 1 game(s) in this corpus, so nothing could be held out. Every
validation figure below is over an empty set.
```

That line exists because an empty held-out set reports an error of 0.000000,
which reads as a perfect fit. The end-of-run refusal then fires as well, since
0.000000 does not improve on 0.000000.

Found by this step's own test, before the commit, which is the only reason it is
a paragraph here rather than a finding in the next audit.

## The two splitters over the same corpus

`--k 0.7615` pinned on both sides -- the whole-corpus fit, so neither split's own
K biases the comparison -- and the same seed, epoch budget and thread count.
Patience disabled so both run exactly 3000 epochs.

| | row level | game level |
|---|---|---|
| train rows | 9903324 | 9903305 |
| held-out rows | 1100369 | 1100388 |
| games held out whole | 0 of 119999 | all of them |
| start train / held out | 0.122699 / 0.122933 | 0.122741 / 0.122560 |
| best train / held out | 0.117053 / **0.117352** | 0.117059 / **0.117380** |
| held out minus train | +0.000299 | +0.000321 |
| wall, 3000 epochs | 2333.91 s | 1090.87 s |

**Recorded as zero.** The two held-out figures differ by 2.8e-05. The two
held-out *sets* differ by 3.7e-04 at the untuned starting constants, before a
single gradient step, which is thirteen times as much -- so the endpoint
difference is dominated by which rows each split happened to take and cannot be
attributed to the contamination. 827 parameters over 9.9 M rows have no capacity
to memorise a game.

That is the expected result and it is not a reason to doubt the defect. The old
split shared games with the training set by construction; the figure it produced
was uninterpretable whatever its value, and the fix is kept for that reason
rather than for a number. The gap widens with more parameters or fewer rows,
which is the direction the plan goes.

The wall times are a side effect and not why the change was made: a fully
shuffled row index random-walks a 1.2 GB dataset every epoch, blocks walk it in
near-file order, and the identical fit runs **2.14x faster**. Peak resident
1227876 KB against 1140956 KB, +7.6 %, which is the 44 MB `ply` vector and the
index built by `reserve`.
author:    Maksym Bodnar
