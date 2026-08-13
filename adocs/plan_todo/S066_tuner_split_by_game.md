id:         S066
goal:       hold out whole games from the fit, not rows, so the validation error is not shared with training
accepts:    a fit over a corpus `datagen` produced puts no game on both sides of the validation split, asserted by a test over the splitter rather than by inspection; the split is still randomised and still lands within a stated tolerance of `--validation`; the S065 corpus is fitted under both splitters at the same seed and the two held-out figures are recorded side by side, with zero difference recorded as zero
touches:    tools/tuner.cpp, and either tools/datagen.cpp's output columns or the tuner's grouping of contiguous rows
excludes:   any change to what `evaluate()` computes; any change to the objective, the optimiser or K; re-deciding anything S027 or S028 concluded, since those verdicts are SPRTs and this touches a diagnostic only
decisions:  DEC-019
closes:
blocks:
paused_by:
done:

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
