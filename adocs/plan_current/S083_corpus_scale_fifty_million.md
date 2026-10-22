id:         S083
goal:       the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
accepts:    the datagen budget in nights is stated **before** generation starts; the fit is compared on held-out error across at least two corpus sizes at the S082 sampling density, so "more rows still helps" is a measured claim at this parameter count and not a quoted one; the generation command, seed, node budget, wall time and filter counts are recorded in the step file, not in a log that is gitignored; **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is -- the corpus built on S082's offline-chosen recipe at the size and node budget chosen here, and the only corpus SPRT the two steps owe (DEC-259); held-out loss is read on S082's fixed validation set; the node budget the corpus was generated at is stated as a decision with its reason, since it is the variable this step is trading
touches:    .tuning/, adocs/data/, src/eval_tables.hpp, src/evaluation.cpp, tests/ (the goldens the paste moves, each by its script), DEV_MANUAL.md, MANUAL.md, adocs/plan_done/ on completion
excludes:   what is labelled, which is S082's and should be settled first; dedupe, which is S076's; the blend, which is S075's; any change to the tuner
decisions:  DEC-041, DEC-055, DEC-087, DEC-259
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
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

## From the 2026-09-27 tuning comparison (the coordinator, 2026-09-28, DEC-237)

`adocs/data/2026-09-27_article_finetuning_comparison.md` section 6 keeps one
idea for this step: **a bounded random-move injection inside the game** -- a
`--random-move-rate`, or a count within a ply window, in `tools/datagen`, the
row after a random move not recorded -- as one more setting this step's
held-out curve reads under its stated nights budget, its rate a range
declared by purpose with its midpoint (DEC-105 form (c)). The published
generators describe the same thing in prose (the report's references;
DEC-221), and S065's coverage gap is the symptom it might address. It is this
step's curve first and its one SPRT second; not a new step (PLAN rule).


## As designed (2026-10-22, phase 1 of 2, written blind)

Written in the worktree `/home/max/ws/chesso-s083` (branch `s083` at
`3918882`) while S111's SPRT held every core (DEC-260): nothing built, run,
generated or fitted. What ran: `git`, reading, and a parse of the new scripts
(`bash -n` on both shell files, Python's `ast` on the three Python files). The
diff is `.tuning/coord/S083/s083_phase1.diff`. Everything below that needs the
CPU is a command with its time from a measured throughput.

### The arithmetic, from this tree only

The step file's "S065 rate of 120 k games in eight hours" is stale and is not
used: S065 ran 4.669 games/s on its engine; today's 2.997 is 36 % slower a game.

| quantity | value | from |
|---|---|---|
| generation rate, the night run | **10790 games an hour** at 100000 nodes, 12 threads: 122749 games with a row between the log's start, 01:36:14, and the files' last write, 12:58:47 (11 h 22 m 33 s) | `adocs/data/S082_pool.log` (part 1's log); S082's record, "Dry run on the real part 1" |
| generation rate, the top-up | 11651 games an hour: 27252 games, `elapsed 8420 s` | `adocs/data/S082_pool.log` (part 2's log) |
| generation rate, the smokes | 11250 and 11191 games an hour, 600 games in 192 and 193 s | S082's record, "Readings" and "Shown" |
| the binary | datagen sha256 `4b0e2848...` played both parts; the main tree's `build/tools/datagen` hashes to it on 2026-10-22 (rebuilt 2026-10-21 23:29) | `S082_pool.log`; `sha256sum` today |
| rows a game written, qs | 84.66 (part 1), 84.88 (part 2); the training side 11690184 / 137998 = 84.71 | `S082_pool.log`, `adocs/data/S082_compare.log` |
| rows a game at the shipped density, 46 | **45.08**: 1000000 rows from 22182 games | `S082_compare.log`, `cand_qs_46_s1` |
| the pool | 150000 games, 149998 with a row; qs 12705516 rows | `S082_pool.log` |
| its training side | **137998 games, 11690184 qs rows**, sha256 `8e9645b3...` | `S082_compare.log` |
| the fixed validation set | 12000 games; qs view 1015332 rows, sha256 `2b508067...` | `S082_compare.log` |
| bytes | 6599 a game in a qs file (810032146 bytes over 122748 games); 79.3 MB a million sampled rows | `S082_pool.log`, the sample's size in `.tuning/s082/` |
| a fit | about a minute a million rows: S082's thirteen 1 M-row qs-view fits ran 48 to 93 s (file times), stopped by patience at epochs 2000 to 2700; S262's 10.8 M-row fit 2100 epochs in 9 m 03 s, 0.024 s an epoch a million rows; at most two minutes a million at 5000 epochs; about 0.18 GB resident a million rows (1.96 GB at 10.8 M) | `.tuning/s082/` file times and `fit_*_qs.log`; `DEV_MANUAL.md` "What the tuner fits" |

What the nights buy, a night being ten chunks of 12000 games (120000 games,
**11.1 h** at 10790 an hour, 10.3 h at 11651), at 45.08 rows a game:

| | games | rows at 46 a game | every row (84.71) | hours |
|---|---|---|---|---|
| S082's training side, no datagen | 137998 | 6.22 M | 11.69 M | 0 |
| + 1 night | 257998 | 11.63 M | 21.86 M | 11.1 |
| + 2 nights | 377998 | 17.04 M | 32.02 M | 22.2 |
| + 3 nights | 497998 | 22.45 M | 42.19 M | 33.4 |
| a new engine's pool, 12 chunks | 144000 | 6.49 M | 12.20 M | 13.3 |
| a new engine's pool, 2 nights | 240000 | 10.82 M | 20.33 M | 22.2 |
| a new engine's pool, 3 nights | 360000 | 16.23 M | 30.50 M | 33.4 |

For scale only, never a target: the three published corpora section 1 cites
are 4.5 to 10 M positions.

### The proposal, choice by choice (the coordinator decides; recommendation first)

**Accepted by the coordinator as recommended, all ten points, with the
landing's goldens under DEC-299's guard and a fallen mate floor stopping for
the owner before any commit (DEC-303's consequence): DEC-304 (pending, the
coordinator's).**

1. **The budget: 2 nights**, twenty chunks, 240000 games at most, 22.2 h.
   It takes the corpus to 17.0 M rows at 46 a game under S111's H0, and the
   stop rule (7) spends less when the curve flattens. *Alternatives*: 1 night (the
   corpus tops at 11.6 M, 11.1 h); 3 nights, the step's ceiling (22.5 M,
   33.4 h, a fit of the top about 22 to 45 min and 4 GB). Stated in
   `adocs/data/S083_corpus.sh` as `NIGHTS=2`, before any generation.
2. **The node budget: held at 100000, recorded as a decision.** Reason:
   S082's pool and the fixed validation set were played at it (DEC-275), so the
   curve's first points cost nothing and every fit is read on games of its own
   budget; one change at a time, so this step scales games and the budget
   stays; the shipping weights' corpus (S065, S076) was played at it too.
   *Alternative*: hold the games and drop to 5000 -- 20 times fewer nodes a
   move, so one night would buy far more games than the tuner can hold; its
   rate at 5000 is unmeasured here (a 3-minute smoke would measure it); it is
   another corpus and another verdict, which the step file says outright.
   Proposed decision text: "S083 generates at 100000 nodes a move: the pool,
   the validation set and the shipping corpus were played there, and the
   step's one verdict moves the corpus size only; the 5000-node corpus is a
   later question with its own verdict."
3. **The sizes the curve reads**: on each stage's pool, the rungs C, C/2, C/4,
   ... down to the last at or above 1 M rows, C being the pool's capacity at
   46 a game (every game). **S082's training side is stage 0 and its first
   points**: 6.22, 3.11 and 1.56 M rows, no datagen. More games join without
   changing the recipe (`MODE=s082`): the same datagen binary -- each stage
   runs a frozen copy under `.tuning/s083/bin/`, re-hashed before every chunk
   against the run's one recorded sha256, never `build/` in place -- chunk seeds 2026102200 + 100 k whose worker
   seeds meet none of the pool's, `--leaf qs`, every other flag S082's,
   game ids moved above the pool's, and S082's training file copied byte for
   byte -- DEC-296's top-up form.
4. **The second reading, rows a game at a fixed game count**: every game of
   S082's training side at 8, at 46 (stage 0's top rung) and at every row
   (about 1.10, 6.22 and 11.68 M rows). Reported, deciding nothing: the
   density is S082's choice and this verdict is about size; a density beating
   46 by more than m at the same games is printed as a finding for the
   coordinator.
5. **Dedupe: none**, S082's compare setting, since the candidate is "built on
   S082's offline-chosen recipe" and that recipe was chosen without one; the
   top rung's duplicate share is printed by `build/tools/corpus_dedupe`,
   deciding nothing. *Alternative*: dedupe every sample (S076's preparation,
   which its H1 said S082 and S083 inherit) -- a change to the recipe S082
   measured, and S076's own record says this step "re-asks it with the
   confounder gone", which a held setting does.
6. **The fit**: S082's compare's, the corpus aside -- `--holdout
   .tuning/s082/pool.tsv.qs.validation --freeze tempo,piece_placement --epochs
   5000 --report 100 --patience 20 --lambda 0 --seed 1 --threads 12`, K fitted
   per corpus, every view of the set named to `sample`. The qs view only: the
   root view decided nothing in S082 and mostly measured K (DEC-298).
7. **The rule, stated before any fit runs** (the script's header, applied by
   `adocs/data/S083_corpus.py` `read`): L is a fit's `best:` validation figure;
   s the spread of five seeds at the smallest rung, m = 1.645 x sqrt(2) x s;
   d = L(C/2) - L(C). Unreadable (outcome (c), nothing chosen) on s above
   1e-3, a refused or missing fit, or a smaller rung beating C by more than m
   -- except at stage 0 under `MODE=none`, below, which is informational.
   Else d > m with budget left: the next stage is generated. Else stop: **the
   candidate is the top rung, every game of the stage's pool at 46 a game**,
   its seed-1 fit. m is taken where the corpora overlap least, so it
   overstates the noise at the top and the rule errs toward spending no
   further night; more rows never hurt a fit in expectation, so the top is the
   candidate whenever the curve reads. If the candidate moves no constant, no
   SPRT is owed (S076's rule 3).
8. **The random-move injection: out of this step's verdict and its curve.**
   It is a second variable on what the corpus holds, beside the size; it needs
   a datagen flag written, tested and mutated; and a pool played with it
   cannot use S082's training side, so reading it at all costs a new pool and,
   if it won, a whole corpus regenerated with it -- for a symptom (S065's
   coverage gap) the coverage matrices S082 now prints can be read for first.
   This amends DEC-237's note, so it needs a decision: proposed, the option
   goes to the reserve with the coverage matrix as its trigger, or to the
   next corpus question (S126). *Alternative*: a third reading at 1 M rows on
   a 25000-game injected pool, about 2.3 h of datagen plus the flag, deciding
   only the next corpus's recipe.
9. **Which engine generates.** S111's SPRT is running; on its H1 the engine (and
   `tools/datagen.cpp`, which S111's tree gives two continuation tables per
   worker) changes. DEC-294 wants the corpus played by the engine that ships.
   - **S111's H0 or no verdict** (`MODE=s082`): nothing is committed, src/ is bench
     6042933 as at `5b0e170`, and the datagen binary is the pool's own, so
     S082's pool *is* the shipping engine's corpus. Stage 0 reads it; nights
     join it only if the rule says so. Best case: no datagen at all.
   - **S111's H1** (`MODE=none`): the pool is one search change behind the engine
     that ships. **Stage 0, the old engine's pool, is informational**: its
     figures are reported, readable or not, and decide nothing (the curve's
     shape at 823 parameters, free); stage 1 always runs -- twelve chunks of
     the new engine, 144000
     games, 13.3 h at the old rate (the smoke re-measures it), and the
     candidate comes from new-engine games only; stage 2 (eight chunks more,
     8.9 h) on stage 1's reading. The S082 pool and its validation set stay
     as they are (DEC-275 fixed the set whatever S083 chooses).
     *Alternative*: keep S082's pool on S111's H1, recording a decision that the
     corpus may lag the shipping search by one change -- saves 13.3 h; the
     cost is an unmeasured drift DEC-294 chose not to accept for S286.
   - Under `MODE=s082` stage 0 decides as every stage does.
   - Either way the binary decides and `MODE` must say the same, or the stage
     refuses; no pool mixes two engines.
10. **The SPRT** (`adocs/data/S083_sprt.sh`): CAND the landing commit, REF its
    parent; 8+0.08, Hash 16, `noob_3moves.epd` -- DEC-286 kept the harness's
    control for every verdict but the correction family's, and DEC-202 leaves
    evaluation weights out of the longer-control reading; `{0, 5}` nElo at
    0.05, plan.md's form for an evaluation family's survivors (DEC-259 (b)).
    Worst case 41861 games, 19.84 h at 2110 an hour; cap 20950 rounds, 41900
    games, 19.86 h; abort at 40 h. *Alternative*: `{-5, 5}` (S076's form),
    10465 games worst case, which would ship a zero half the time on the
    corpus every later family fits on.

### Deviations from the brief

- **The scripts are tracked under `adocs/data/`** (`S083_corpus.sh`,
  `S083_corpus.py`), where S082 kept its recipe script, and not in
  `.tuning/coord/S083/`: section 3 above wants the record in git, and the
  pre-registration must be committed before generation starts. They run from
  the main tree once committed (their paths and `OUT` derive from where they
  sit).
- **Two tracked fit scripts changed**, both trivial and in scope:
  `.tuning/apply_fit.py` and `.tuning/verify_fit.py` take `--root TREE` or
  `--root=TREE` and refuse any argument they do not know (the landing runs in
  a worktree; both wrote or read the main checkout only, and a misspelt
  option must not fall back to it).
  **A finding, fixed here**: `verify_fit.py` still listed S134's two deleted
  columns -- passer buckets six wide and placement four, 827 in all -- so it
  would have refused a correct paste of today's 823; the first fit pasted since
  S076 is this step's. Unreachable in play, so DEC-171 filler, closed in the
  same diff.
- `touches:` widened to `adocs/data/`, `tests/` (the goldens the paste moves),
  `DEV_MANUAL.md` and `MANUAL.md`.
- **The gating is the script's, not the coordinator's memory** (the fast
  check's findings, 2026-10-22): `smoke` writes a pass record naming the
  datagen sha256 it tested and `generate` refuses without one for the copy it
  runs; every chunk applies the abort rule to its own log and a firing writes
  `.tuning/s083/chunks/ABORTED`, which stops every later `generate` until the
  coordinator removes it; `generate N` refuses unless stage N - 1's reading
  under the same `MODE` printed `READ-OUTCOME continue`, stage 1 under
  `MODE=none` excepted; datagen and the tuner run only as frozen, re-hashed
  copies.
- Chunks are written `--leaf qs` alone: the qs rows are the ones a three-leaf
  run writes (S082 showed the walks leave the games alone), at a third of the
  disk; 20 GB are free.

### Commands in order, with times

All from `/home/max/ws/chesso` after this diff is committed, `OUT` the default
`.tuning/s083`, each launch prefixed `mkdir -p .tuning/s083 &&` so its console
has a directory before the script runs (the script makes its own beneath it).
Every stage prints `S083-<STAGE>-DONE` or `-FAILED` last; arm WATCHERS'
four-exit poll on the console and the pid with the ceiling given, at least
twice the worst estimate. Curve times include the capacity pass and every
sample pass, scaled by pool size from S082's compare (the script's MACHINE
HOURS).

1. After S111's verdict (and on its H1 its landing and `cmake --build build -j12`):
   `sha256sum build/tools/datagen` -- `4b0e2848...` means `MODE=s082`, anything
   else `MODE=none`. Seconds.
2. `mkdir -p .tuning/s083 && nohup adocs/data/S083_corpus.sh smoke >
   .tuning/s083/smoke.console 2>&1 &` -- 600 games, about 3 min, ceiling
   15 min; the day's rate and the abort rule, and the pass record `generate`
   needs. Re-estimate every night from its rate.
3. `mkdir -p .tuning/s083 && nohup adocs/data/S083_corpus.sh curve 0 >
   .tuning/s083/curve0.console 2>&1 &` -- about 36 min, at most 66; ceiling
   2 h 15 m. Its reading says `continue` or `stop` and prints the candidate on
   a stop; under `MODE=none` it is informational and says `continue`.
4. On `continue` (or under `MODE=none` whatever stage 0 read), at night
   (DEC-155): `mkdir -p .tuning/s083 && MODE=<m> nohup
   adocs/data/S083_corpus.sh generate 1 > .tuning/s083/generate1.console 2>&1
   &` -- 11.1 h (s082) or 13.3 h (none); ceiling 30 h. After a power cut or a
   failed chunk the same command resumes; after the abort rule fires it
   refuses until the coordinator removes `chunks/ABORTED`. Then
   `MODE=<m> ... merge 1`, about 2 to 5 min, ceiling 30 min; then
   `MODE=<m> ... curve 1`, about 36 min, at most 63, ceiling 2 h 10 m (none:
   23, at most 41, ceiling 1 h 30 m).
5. On `continue` again: `generate 2` (11.1 h, or 8.9 h under none; ceiling
   30 h), `merge 2`, `curve 2` (about 50 min, at most 87, ceiling 3 h; none
   34, at most 60, ceiling 2 h). Stage 2 always stops: the budget's end.
6. `adocs/data/S083_corpus.sh apply <the candidate's fit .hpp> <a clean
   worktree at HEAD>` -- the paste, `verify_fit`, `diff_fit`, both builds,
   bench, the fast suite in both builds: about 15 min. Its reds are the
   goldens the paste moved.
7. The landing, below: about 2 to 3 h, the re-mines' share unmeasured.
8. The commit (green, `Bench:` from step 6), `S083_sprt.sh` pinned and its
   bracketed sentence filled, the A/A rule checked: `nohup
   adocs/data/S083_sprt.sh > .tuning/s083_sprt.log 2>&1 &`, a night, at most
   19.86 h, ceiling 144000 s.

**What phase 2 copies into this file, as the accepts want it tracked**: per
stage the `filters` totals (`.tuning/s083/chunks/stageN.filters` -- the
filter's clauses, refusals by kind, rows a game, wall time and games an
hour), each chunk's command, seed and elapsed time (`cKK.done`, `cKK.log`), the
merged pool's rows and sha256, and every reading whole. If no chunk is
generated, the corpus's generation is S082's pool and its record is copied
from `adocs/data/S082_pool.log`: both commands, both seeds, 11 h 22 m and
8420 s, and part 2's filter counts -- part 1's were lost with its closing
lines (DEC-296), which this file then says.

Machine time: best case (S111's H0, the curve flat on S082's pool) about 4 h
before the SPRT; worst case (two nights) about 28 h, 30 h with every fit at
its 5000-epoch worst.

### The landing: what the paste moves, each re-derived by its script

An evaluation refit moves every golden that reads `evaluate()`, which is most
of them, as S261's vector moved the search's (its record's goldens table).
From `DEV_MANUAL.md` "Goldens: named, scripted, re-derived":

| golden | script |
|---|---|
| the six piece anchors, `QUIET_ROOK_EVAL` and `_CHEAP`, 383, 198, -505, -569 | `python3 adocs/data/S192_anchors.py` |
| `test_eval_trace` `truncation_positions`, four positions | `build/tools/truncation_scan --data <the candidate's corpus> --min 2.8` (DEC-057), re-picked across the taper with both sides to move |
| "ordering keeps the tree small" | `python3 adocs/data/S192_node_budget.py` |
| `short_line_ceiling` and `S170_cases.tsv`'s budgets | the S170 sweep (S110's `p2/sweep.sh` form), `adocs/data/S203_case_sweep.sh --ceilings`, the budget rule |
| `EXACT_FLOOR` | `S156_mined_floor_sweep.py --floor <the floor>` |
| `MATE_IN_THREE_FLOOR` | `S154_floor_margin_sweep.py floor` and `red` |
| `first_mate_depth`, `mate_in`; `DRIVE_DEPTH` | `S188_repair_goldens.py first-mate`; `s207` |
| `capture_mates` rows | the seven `S230_mine_r01_row.py depths` sweeps |
| the ProbCut, extra-ply and multicut mate rows | `S277_mine_mate_row.py`'s six commands in guard mode, and the rows' own miners |
| the late-capture cases | `adocs/data/S261_mine_late_capture.py` |
| bench | `chesso bench`, the commit's `Bench:` |

Times measured elsewhere: build 2 min, gate 4 min, bench 5 min, S170 sweep
5 min, the floors and first-mate 15 min (S110's phase 2); a mutant pass over
the mutants a re-mined row kills, 10 in 1288 s (S261); Debug self-play 10 min
and `tools/gate_extra.sh` 17 min (S110). The mined rows' re-mines are not timed
in any record read here. Debug self-play is not owed by DEC-141's letter (no
make, unmake, generator or search code moves) and is proposed anyway, the whole
tree's play moving.

**A question to settle before the sweep (DEC-265).** *Why*: the paste will
move carry ceilings and may move a floor; DEC-295's and DEC-299's guards were
each one step's and set no allowance for another, and DEC-303 makes a fallen
floor the owner's. *Options*: (a) record now that S083 applies the ceilings
and budgets the script answers provided both mate floors hold or rise and no
ceiling rises by more than 2, outside that the owner -- no stop inside the
guard, the risk being a looser mate test carried on the SPRT's verdict; (b)
stop for the owner at any ceiling or floor move -- an idle machine for up to a
day. Recommend (a), DEC-299's words, recorded before the sweep. **Settled
by DEC-304 (pending, the coordinator's)**: (a), and a mate floor that falls
stops for the owner before any commit, per DEC-303's consequence.

Also moved by the paste, for the landing commit: the `POSITIONAL_ROOM`
property in `tests/test_evaluation.cpp` may fire (S065's did); its answer is
DEC-059's re-anchoring of the emitted header (`.tuning/reanchor.py`), a
decision, never a wider room. And every sentence that states a pawn's value
as 94 -- `src/search_params.hpp` comments, `MANUAL.md`'s option rows, the
declared range of the pawn correction's cap -- states S076's scale: restate as
"at the seed's weights", the ranges kept (S127 owns them).

### What must be built and tested later (phase 2, before `curve 0`)

- `.tuning/verify_fit.py .tuning/s082/fit_root_0_s1_qs.hpp` must print 823
  compared, 823 identical, exit 0: that fit never left the shipped constants
  (DEC-298). The same with `--root TREE` and with `--root=TREE` at a clean
  worktree; `--rot TREE` and a second positional must be refused. Seconds.
- `.tuning/apply_fit.py <any S082 fit> --dry-run --root <worktree>`: lists 5
  defines, 2 tables, 10 arrays, 2 scalars and writes nothing. Seconds.
- `adocs/data/S083_corpus.py` over S082's smoke files: `chunk-check` on
  `.tuning/s082/smoke/pool.tsv.qs` against a scratch copy of its
  `datagen.log` with `exit      0` and `elapsed   193 s` appended (passes, and
  with `--abort-share 1 --rate-floor 8333` prints the rule's reading; with
  `--rate-floor 20000` exits 2; without the exit line, refuses with 1);
  `merge` of that file as two chunks into a scratch file (rows doubled, ids
  2000000+ and 2100000+); `capacity` of it against the three views; `read` on
  a scratch directory whose `ladder.txt` names S082's 1 M-row fit logs as
  rungs, once with `--mode s082` (a rule outcome) and once `--mode none
  --stage 0` with one log removed (reports it and prints `READ-OUTCOME
  continue`, exit 0). About 2 min.
- `adocs/data/S083_corpus.sh verify` (the fixed set and S082's training side
  against their sha256): seconds. Then `smoke` is the first live run.
- Nothing in src/ or tools/ changed, so no build or bench is owed for this
  diff; the gate's prose checks run over this file at the commit.

### Proposed for the documents, at completion

- `DEV_MANUAL.md` "Generate self-play data and fit the evaluation": the
  chunked generation, S082's measured pool rates beside the smoke figure, and
  on S083's H1 "the corpus the shipping weights descend from" becomes S083's, with
  its commands, seeds and rows; `apply_fit.py` and `verify_fit.py`'s `--root`.
- `adocs/specs.md`'s evaluation row, on S083's H1 only: the corpus sentence (S076's
  deduplicated root rows) becomes S083's qs leaves at 46 a game, undeduped,
  with the verdict.
- `MANUAL.md`: the "a pawn is 94" rows, on S083's H1 only, as above.

## 2026-10-22: S111 read H0, so MODE s082 (the coordinator)

S111's SPRT read H0 at 10:22 (record `7b6f951`, completed as zero by
`8555e6d`) and nothing of it was committed, so the engine that ships is the
one that played S082's pool. Command 1, after the gate rebuilt `build/` at
`8555e6d`: `sha256sum build/tools/datagen` reads `4b0e2848...`, S082's
pool's binary bit for bit, so **`MODE=s082`**. DEC-304 records the
pre-registration with that outcome; this file, the scripts and the two
`.tuning/` tools are committed with it before any stage runs, then the phase
2 kit and commands 2 and 3.

