id:         S083
goal:       the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
accepts:    the datagen budget in nights is stated **before** generation starts; the fit is compared on held-out error across at least two corpus sizes at the S082 sampling density, so "more rows still helps" is a measured claim at this parameter count and not a quoted one; the generation command, seed, node budget, wall time and filter counts are recorded in the step file, not in a log that is gitignored; **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is; the node budget the corpus was generated at is stated as a decision with its reason, since it is the variable this step is trading
touches:    .tuning/, src/eval_tables.hpp, src/evaluation.cpp, adocs/plan_done/ on completion
excludes:   what is labelled, which is S082's and should be settled first; dedupe, which is S076's; the blend, which is S075's; any change to the tuner
decisions:  DEC-041, DEC-055, DEC-087
closes:
blocks:
paused_by:
done:

## Re-scoped 2026-08-19 by the second review, DEC-087

This step said "a corpus past 50 M positions". The 50 M floor is retired for
arithmetic and for evidence. Arithmetic: a row target converts into a datagen
cost only through a positions-per-game density, and that density is S082's
open question -- chesso's own choice of few rows a game is a hypothesis this
step measures, not a published practice (DEC-203). So the budget is stated in
nights instead, on the S065 rate of 120 k games in eight hours, on the machine
whose time is the plan's binding constraint. Evidence: the published corpora
located for hand-crafted evaluations sit at **4.5 to 10 M resolved positions**
-- Stash's v32 retune used 4.5 M self-play positions and its changelog states
the evaluation "doesn't overfit for datasets > 500k positions", Ethereal's
published dumps are three sets of ~10 M, and the original method's own corpus
is 8.8 M (section 1 has all three with their URLs) -- against 827 shipped
constants here.
`adocs/eval_tuning_strategy.md` section 2.6 says "100M+ is where results
stabilize"; that sentence describes NNUE-scale parameter counts, and this step
exists to answer it with a held-out curve at this engine's own parameter count
rather than adopt either number.

## The trade this step is actually making

The same document suggests generating "at fixed low nodes (e.g. 5000
nodes/move)". chesso generated at **100000** -- twenty times that -- and the
default in `tools/datagen` is 5000, so the S065 run overrode it deliberately.

At fixed compute those two settings are the same night spent differently: a
larger corpus of noisier labels, or a smaller one of better labels. Nobody here
has measured which side of that trade is better, and the published advice and
this project's own practice disagree by a factor of twenty. So the node budget is
a recorded decision in this step, with its reason, rather than a number carried
over.

**One change at a time still applies.** If both the size and the node budget
move, the verdict says nothing about either. The honest form is: hold the node
budget where S065 had it and scale the games, or hold the games and drop the node
budget, and if both are wanted that is two corpora and two verdicts.

## Cost

One to three nights of generation inside the stated budget, held-out fits in
minutes each, one SPRT.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**Three published corpus sizes carry URLs and they span 4.5 to 10 M.**

- **The original method's is 8.8 M.** CPW *Texel's Tuning Method*
  (https://www.chessprogramming.org/Texel%27s_Tuning_Method, fetched
  2026-09-13): "Take 64000 games played at a fast time control", "This
  typically gives about 8.8 million positions", "about 140 positions per game
  on average". One engine's working corpus for a hand-crafted evaluation, an
  order of magnitude below the retired 50 M floor.
- **Stash's is 4.5 M, and its author states an overfitting threshold.**
  `mhouppin/stash-bot`'s `CHANGELOG.md`, entry **v32.0 (2021-12-02)**
  (https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md, re-read
  2026-09-13 after the fast check): "Retuned eval for testing, using a dataset
  of 4.5M positions coming from selfplay games at 1+0.01 time control", and in
  the same entry "Removed the validation loss from the tuning code, since the
  eval doesn't overfit for datasets > 500k positions". Both are the author's
  own statements about his own fit and neither is a constant of any kind -- a
  dataset size and an overfitting claim are not seeds and DEC-105 does not
  reach them. The second is the more useful of the two: it says the
  overfitting risk this step's held-out split guards against was, for one
  hand-crafted evaluation, already gone two orders of magnitude below the
  retired floor. It is one author's report on one engine's parameter count and
  is quoted for direction only (DEC-019); this step measures its own curve.
- **Ethereal's published dumps are three sets of ~10 M.** TalkChess "Ethereal
  Tuning - Data Dump" (https://talkchess.com/viewtopic.php?t=75350,
  AndrewGrant, 2020-10-10, fetched 2026-09-13 after the fast check): "3x Sets
  of ~10M positions of <fen> <result>" and "1x Sets of ~12.5M postitions from
  FRC of <fen> <result>". **The figure is read; the data is not touched** --
  those rows are another engine's labelled output and COPYING forbids training
  on them here, ever (DEC-016).

**One figure this file used to quote stays deleted (DEC-203).** The "per
generation" reading of Ethereal's dump size has no source: the post states
three sets of a size, not a per-generation rate, and nothing located says how
many generations produced them. The size is restored above in the form its
source actually states.

**The 2026-09-04 check and S186's first pass recorded these three as not
located; that was wrong for all three and the error is this file's.** The
Stash changelog does state dataset sizes and an overfitting threshold, in the
v32.0 entry, and the Ethereal set size is in the announcement thread. What is
genuinely absent: CPW *Texel's Tuning Method* and *Automated Tuning* give no
dataset-size guidance beyond the 8.8 M figure above (*Automated Tuning*:
dataset size absent). The argument the "Re-scoped 2026-08-19" paragraph makes
is unchanged and now rests on three sourced corpora instead of one -- the
parameter count is 823 after S134 and the held-out curve is what decides.

**`adocs/eval_tuning_strategy.md`'s "100M+ is where results stabilize" has no
citation in that document either**, and the one published figure located
(8.8 M) is an order of magnitude below it. The strategy document's own
sentence is about NNUE-scale parameter counts, which this step already says;
the enrichment adds that the 100 M number is unsourced where it is written and
that no fetched source supports it for a hand-crafted evaluation.

**The node budget has one published statement and it is a recommendation, not
a measurement.** TalkChess "Generating original training data"
(https://talkchess.com/viewtopic.php?t=84811, Sapling, 2025-02-07): "Set a
hard node limit for your search e.g 5k nodes". `tools/datagen`'s own default
is 5000 and the S065 run overrode it to 100000, so the published
recommendation and this project's practice differ by twenty times -- which is
the trade this step exists to decide and not evidence for either side.

### 2. Shape for chesso

The decision is a curve, not a number. At a stated datagen budget in nights:

- Generate at **one** sampling density (S082's) and at **one** node budget,
  varying only the number of games, and read held-out error at two or more
  corpus sizes.
- **The density is a hypothesis and this step is where it is measured**
  (DEC-203). S082 samples few rows a game because positions inside one game
  are correlated; nothing published supports the number. So this step reads
  held-out error a second way -- against **rows per game at a fixed game
  count** -- and reports both curves, rows per game and games needed. That is
  two readings of one corpus family and not two moving variables in one
  verdict: the SPRT still takes **one** candidate, per the accepts.
- Held-out error is computed on the by-game splitter S066 fixed -- a
  position-wise split leaks, because positions from one game are correlated by
  construction and the original method's 140-per-game density is exactly what
  makes that leak large.
- "More rows still helps" becomes a measured claim **at 823 parameters**,
  which is the only scale this engine can act on.

### 3. Implementation sketch

The generation command, the seed, the node budget, the wall time and the
filter counts go in this step's file, not in a gitignored log -- `.tuning/` is
not tracked and a machine move loses it, which is the same hazard S039 records
for `.tuning/selfplay_v2.tsv`.

### 4. Constants and seeds

Two constants and both are **(c) declared by purpose, not seeded from any
published value**:

- **Corpus size.** No seed. The step reports a curve and the chosen point is
  the curve's, stated with its held-out figures. The 50 M floor is retired
  above; the 4.5-to-10 M band of section 1 is three other projects' corpora at
  three other parameter counts, recorded as context and never as a target.
- **Node budget.** The two candidates are this engine's own: `tools/datagen`'s
  shipping default and the 100000 the S065 run used. The decision is recorded
  with its reason, per the accepts. A number taken from another engine's
  datagen configuration would be that engine's constant (DEC-105, DEC-134) and
  is not used; the TalkChess "e.g 5k nodes" is a forum author's illustration
  and is quoted for direction only (DEC-019).

### 5. Pitfalls

- **Two variables, one verdict.** If size and node budget both move, the
  verdict prices neither. Hold one.
- **The arithmetic depends on S082's density and that density is this
  project's own hypothesis.** At 140 rows a game -- the one sourced figure --
  50 M rows is 357 k games; at the few-rows-a-game density S082 argues for it
  is orders of magnitude more, which is the whole reason the row target was
  retired in favour of a nights budget. **This step re-derives its own
  arithmetic from the density S082 actually ships**, at this step's own HEAD,
  and from nothing written in either file before that.
- **Held-out error is not Elo.** DEC-019's rule applies to fit quality as much
  as to published figures: a corpus that fits better and plays worse is the
  documented overfitting signature and is why the SPRT is in the accepts.
- **The budget is stated before generation starts**, because a run that
  overruns is an argument for keeping what it produced.

### 6. Measurement

Held-out error across at least two sizes, then **one** candidate to an SPRT
against the shipping weights. The generation itself is one to three nights and
is DEC-155 night work; the fits are minutes. A verdict of zero is recorded as
zero and the corpus may still be kept with the reason stated.

### 7. Interactions

- **S082 (before)**: decides what is labelled and at what density. Settled
  first, by the accepts.
- **S076 (dedupe)**: changes the effective row count, so the curve is read at
  a stated dedupe setting.
- **S126 (block end)**: fits on whatever this step lands; its early stopping is
  on the held-out split, which is the same splitter.
- **S029 (parked, DEC-054)**: NNUE is where the 100 M sentence belongs; this
  step does not answer it and does not pretend to.

### 8. References

- - https://www.chessprogramming.org/Texel%27s_Tuning_Method -- "64000 games",
  "about 8.8 million positions", "about 140 positions per game on average".
  Fetched 2026-09-13.
- - https://www.chessprogramming.org/Automated_Tuning -- no dataset-size
  guidance; regularisation named only for Medvedev's logistic regression.
  Fetched 2026-09-13.
- - https://talkchess.com/viewtopic.php?t=84811 -- Sapling, 2025-02-07, "Set a
  hard node limit for your search e.g 5k nodes". Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- entry
  v32.0 (2021-12-02): "using a dataset of 4.5M positions coming from selfplay
  games at 1+0.01 time control" and "Removed the validation loss from the
  tuning code, since the eval doesn't overfit for datasets > 500k positions".
  Re-read 2026-09-13 after the fast check, which is where both were found;
  S186's first scan of this file reported it as carrying neither and was
  wrong. Changelog entries only.
- - https://talkchess.com/viewtopic.php?t=75350 -- "Ethereal Tuning - Data
  Dump", AndrewGrant, 2020-10-10: "3x Sets of ~10M positions of <fen>
  <result>", "1x Sets of ~12.5M postitions from FRC". Forum post only; the
  sizes are read, the data is never used (DEC-016). Fetched 2026-09-13 after
  the fast check.
