id:         S083
goal:       the corpus size and the generation node budget are decided by held-out error under a stated datagen budget, not by a volume target
accepts:    the datagen budget in nights is stated **before** generation starts; the fit is compared on held-out error across at least two corpus sizes at the S082 sampling density, so "more rows still helps" is a measured claim at this parameter count and not a quoted one; the generation command, seed, node budget, wall time and filter counts are recorded in the step file, not in a log that is gitignored; **one** candidate goes to an SPRT against the weights that ship, verdict recorded whatever it is -- the corpus built on S082's offline-chosen recipe at the size and node budget chosen here, and the only corpus SPRT the two steps owe (DEC-259); held-out loss is read on S082's fixed validation set; the node budget the corpus was generated at is stated as a decision with its reason, since it is the variable this step is trading
touches:    .tuning/, adocs/data/, src/eval_tables.hpp, src/evaluation.cpp, src/search_params.hpp (`CorrHistMax`'s declared top with `PAWN`, DEC-307 (2); the comments that stated a pawn as 94), tests/ (the goldens the paste moves, each by its script; test_tuner_gradient's clamp push; test_eval_trace's exact-reconstruction and within-bound cases reading `truncation_positions` with their mirrors; test_eval_positions' comment on S076's four; test_search_params' `CorrHistMax` row; DEC-313's five rulings: test_mate_breadth's `EXACT_FLOOR`, test_engine's `MATE_IN_THREE_FLOOR` comment and the ply-2 case's first-iteration clause, test_search's multicut mined row retired, test_tuner_gradient's gauge-walk floor), adocs/data/S192_anchors.py (its embedded anchors, the no-argument check, at S083's weights), adocs/data/S083_tuner_preconditions.cpp (`WALK_FLOOR`, DEC-313 (5)), adocs/data/S083s_rfp_ply2.py (added: the ply-2 golden's script, DEC-313 (3)), tools/mutants/S097_singular_extension.py (E21's description: the retired row, DEC-313 (4)), DEV_MANUAL.md, MANUAL.md, adocs/plan_done/ on completion
excludes:   what is labelled, which is S082's and should be settled first; dedupe, which is S076's; the blend, which is S075's; any change to the tuner
decisions:  DEC-041, DEC-055, DEC-087, DEC-259
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-25 -- curve 0 read stop at its top rung: S082's pool, every game of its training side at 46 qs rows a game, 6226944 rows, held-out L 0.071838 against the shipped 0.072864 (the last doubling +0.000067 inside m 0.000082; DEC-304); the queen re-anchored (DEC-059's form, DEC-307), re-landed on S292's and S293's guards; its one SPRT, the working tree against e972bd4 at 8+0.08, {0, 5}, read H1: Elo +116.07 +/- 23.26, nElo +153.75 +/- 28.57, 568 games, 0 forfeits (adocs/data/S083_sprt.log), all 396 adjudicated winners confirmed by Stockfish at depth 16; the owner's five rulings (DEC-313) applied, DEC-312 ratified; every golden by its script, fast suite 50/50 in both builds, mutation pass 224 of 225 with MW02 and H04 the shipped tree's (S294); bench 6042933 -> 3945487. Implemented by Opus 5.5 subagents, fast-checked at each phase; stamped by the coordinator.

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

## 2026-10-22: the phase 2 kit

Run by one Opus 5.5 subagent at `41e4140`, machine idle (load 0.25 on 12
cores), from `/home/max/ws/chesso`; scratch under `.tuning/coord/S083/kit/`
(K below), `.tuning/s082/` read only. **Every item passed; no defect found, no
script changed, no expectation in this file wrong.** No stage was run: no
smoke, curve, generate, apply, fit, datagen or match.

- **`verify_fit.py` and `apply_fit.py`**: run by the coordinator before this
  kit and passed as listed above (823 of 823 identical, exit 0, with `--root`
  and `--root=`; `--rot` and a second positional exit 1; the dry run 5
  defines, 2 tables, 10 arrays, 2 scalars, the worktree untouched).
- **`chunk-check`**, on `.tuning/s082/smoke/pool.tsv.qs` against K/smoke.log
  (`datagen.log` with `exit      0` and `elapsed   193 s` appended), 600 games:
  plain, exit 0, `51444 rows, 600 games with a row of 600 played, sha256
  2127da4f...`; with `--abort-share 1 --rate-floor 8333`, exit 0 and the
  rule's reading, `abort rule: 0 truncated, aborted or uncertified of 51459
  passed, terminal 15 (not gated)` and `rate: 11191 games an hour, floor
  8333`; with `--rate-floor 20000`, exit 2, `ABORT RULE: ... 11191 games an
  hour, under the 20000`; against the log without the two lines, exit 1,
  `exit lines [], not one 'exit 0'`. A relative FILE passes too (the log names
  the absolute path).
- **`merge --out K/merge_two.tsv FILE:0 FILE:1`**: exit 0, 102888 rows
  (51444 doubled), 1200 games, ids 2000000 to 2000599 and 2100000 to 2100599,
  every other field byte-identical to the source twice over. The code moves
  chunk k by 2000000 + 100000 k; S082's pool's largest id is 1027251 (the
  validation side; 1027250 on the training side, read with `awk`), so every
  chunk id sits above the pool's, DEC-304's "ids above the pool's", and twenty
  12000-game chunks never meet. Also: `--base` on a scratch base copies it
  byte for byte as the prefix (154332 rows, 1800 games); a wrong
  `--base-sha256`, an existing `--out` and an index named twice each exit 1.
- **`capacity --in K/merge_two.tsv`**, the three views, per-game 46, 8 and 0:
  exit 0, 49146, 8706 and 93668 rows from 1090 of 1200 games, 9220 rows dropped
  for a validation position (S082's smoke was played at part 1's seed,
  20261013, and loses 9 % of its rows to the fixed set against 0.03 % on the
  training side). Exact: `corpus_split.py sample --rows C --seed 1` writes C
  rows from 1090 games at each density, and `--rows C+1` refuses at each.
  The same pass on S082's training side, read only, 16 s and 0.28 GB: **C =
  6226944** at 46 (137994 of 137998 games, 3688 rows dropped), 1103627 at 8,
  11686496 at every row -- so `curve 0`'s rungs are 6226944, 3113472 and
  1556736, the seeds at 1556736, as the arithmetic above has them.
- **`read`**, on K/read_s082, whose `ladder.txt` names S082's 1 M-row qs-view
  fit logs as rungs (rung 4000000 = `fit_qs_46_s1_qs.log`, 2000000 =
  `fit_qs_8_s1_qs.log`, 1000000 with seeds 1 to 5 = `fit_root_0_s1..5_qs.log`,
  density 8 and 0 = `fit_qs_8`, `fit_qs_0`; the row labels are the ladder's,
  the logs say 1000000): `--mode s082 --stage 0 --last-stage 2`, exit 1, s =
  0.000094, m = 0.000218, `outcome (c), unreadable: rung 2000000 fits better
  than the top by more than m`, `READ-OUTCOME unreadable`, and the 8-a-game
  fit printed as a FINDING. The same logs with s3 removed, `--mode none
  --stage 0`: exit 0, `rung 1000000 s3 missing or unreadable`, `would be
  unreadable, reported only: missing ...fit_1000000_s3.log; rung 2000000 fits
  better ...`, `READ-OUTCOME continue`. The other outcomes, on reordered rungs:
  d +0.000250 > m at stage 0 of 2, `READ-OUTCOME continue`, exit 0; the same at
  stage 2 of 2, `stop -- still descending ... the budget's last stage`; d
  +0.000115 inside m, `stop`, the candidate `fit_4000000_s1.hpp`; a missing
  seed under s082, exit 1 (`refused: missing ...`, no READ-OUTCOME line).
  `generate 1`'s gate expression run over each `reading.txt`, written as
  `read_stage` writes it: opens on the s082 continue only, refuses the s082
  unreadable and the MODE=none continue.
- **`adocs/data/S083_corpus.sh verify`** (`OUT` under K): `fixed set: the
  three views' sha256 are S082's`, `S082's training side: sha256 as recorded`,
  `S083-VERIFY-DONE`, exit 0, 2 s. The five sha256 in the script are the ones
  `adocs/data/S082_pool.log` and `S082_compare.log` record.

**Read, not run: `smoke`, `curve 0` and `generate 1` under MODE=s082 find
every path they need.** `build/tools/datagen` hashes to `4b0e2848...`
(relinked 10:36 today, commits after the pool's `5b0e170`, the same bytes:
datagen carries no commit), `build/tools/tuner` and
`build/tools/corpus_dedupe` exist, the S082 files the script names exist, and
`engine_stamp` sees no untracked file under src/ or tools/ but `__pycache__`.
The frozen copies link only system libraries (`ldd`), so they run from
`$OUT`. Single-leaf datagen writes `--out` itself, which `chunk-check`
compares the log's path against. `smoke` records its pass under
`$OUT/smoke/<12 hex>/pass`, the directory and the `datagen` line `generate`
looks for; `generate 1` reads `$OUT/curve/s0/reading.txt`, where `curve 0`'s
`read_stage` writes it; the fit tags `<rows>_s<seed>` and `dens8`, `dens0`
are the names `read` opens; `corpus_dedupe` takes the seven-field rows (a
probe on K's sample read 49146 rows). Notes, none changed: (1) the tuner
embeds its commit (`engine f93a3b2-dirty` in S082's fit logs), so it hashes
`b7430c30...` today against S082's compare's `62204108...` -- src/ since
`f93a3b2` differs by a comment, and each stage freezes one tuner for every
fit, so a stage's curve is internally consistent; (2) `generate` freezes
`bin/datagen.stageN` and writes `chunks/datagen.sha256` before its smoke-pass
and reading gates, so a refused first attempt pins the run to that binary --
harmless under s082, where `require_mode` pins it to S082's anyway, and by
design ("an existing copy is reused"); (3) `filters` over a three-leaf log
reads the first coverage block, root's; the chunks are qs alone.

## 2026-10-22: smoke and curve 0, run by the coordinator -- stop at stage 0

**Smoke** (`MODE=s082 adocs/data/S083_corpus.sh smoke`, 10:52 to 10:55,
console `.tuning/s083/smoke.console`): the frozen datagen copy
`.tuning/s083/smoke/4b0e28483f64/datagen`, sha256 `4b0e2848...` (S082's pool's
binary), `--leaf qs --games 600 --nodes 100000 --threads 12 --seed
2026102100`: 600 games, 51234 qs rows (85.39 a game), filter 60798
considered, 51238 passed, skipped in-check 3847, mate 0, score past 1000
3998; qs not written: terminal 4, truncated 0, aborted 0, uncertified 0;
elapsed 190 s, **11368 games an hour** (a chunk 63 min, a night of ten
10.6 h); the abort rule 0 of 51238 against 1 %, the rate against the 8333
floor; pass record written for that sha.

**Curve 0** (`MODE=s082 adocs/data/S083_corpus.sh curve 0`, 10:55 to 11:49,
54 min, console `.tuning/s083/curve0.console`, outputs
`.tuning/s083/curve/s0/`): the fixed set's three views and S082's training
side checked against their sha256 first; capacity at 46 a game 6226944 rows,
at 8 a game 1103627, at every row 11686496 (137994 of 137998 games with a
row; 3688 rows dropped for a validation position). The reading, whole:

```
S083 reading, stage 0, mode s082, /home/max/ws/chesso/.tuning/s083/curve/s0
fit                         rows    games  L (qs view)      start       K  best at  improved
rung 6226944 s1          6226944   137978     0.071838   0.072864  0.7237     2400  yes
rung 3113472 s1          3113472    69010     0.071905   0.072862  0.7259     1100  yes
rung 1556736 s1          1556736    34514     0.071992   0.072860  0.7286     3500  yes
rung 1556736 s2          1556736    34507     0.072019   0.072863  0.7247      500  yes
rung 1556736 s3          1556736    34488     0.072072   0.072859  0.7313     2800  yes
rung 1556736 s4          1556736    34483     0.072000   0.072865  0.7226     1000  yes
rung 1556736 s5          1556736    34485     0.072057   0.072865  0.7225     2200  yes
every game, 8/game       1103627   137341     0.071897   0.072869  0.7197     1100  yes
every game, all/game    11686496   137994     0.071783   0.072860  0.7291     2700  yes
spread at 1556736 rows over 5 seeds: mean 0.072028, s = 0.000035; m = 1.645 * sqrt(2) * s = 0.000082
per doubling, seed 1, nested samples:
     3113472 ->    6226944: L 0.071905 -> 0.071838, gain +0.000067, inside m
     1556736 ->    3113472: L 0.071992 -> 0.071905, gain +0.000087, past m
second reading, every game: all a game, L 0.071783, -0.000055 against 46's 0.071838 -- within m of 46 or worse; decides nothing
second reading, every game: 8 a game, L 0.071897, +0.000059 against 46's 0.071838 -- within m of 46 or worse; decides nothing
top doubling: d = L(3113472) - L(6226944) = +0.000067 against m 0.000082
outcome: stop -- the last doubling gained +0.000067, inside m 0.000082. The candidate is the top: every game of this stage's pool at 46 rows a game, 6226944 rows, 137978 games
candidate /home/max/ws/chesso/.tuning/s083/curve/s0/fit_6226944_s1.hpp
READ-OUTCOME stop
```

The duplicate share at the top rung, printed and deciding nothing (DEC-304
(5)):

```
6226944 rows read, 5621365 distinct positions written, 605579 dropped (9.7251%)
repeats: 5137983 keys once, 384500 twice, 80415 three times, 18419 4-7, 48 8-15, 0 16 or more; most repeated 11
```

**By the pre-registered rule the outcome is stop at stage 0**: the top
doubling gained +0.000067 against m 0.000082, so no chunk is generated and
the candidate is `fit_6226944_s1.hpp`, every game of S082's training side at
46 rows a game. The every-row density read 0.000055 lower than 46's, inside
m; by DEC-304 (4) it decides nothing and is recorded. Two of DEC-304's
nights are unspent. Next: `apply` on that fit in a clean worktree, then the
landing's goldens by their scripts, then the commit and the SPRT.


## 2026-10-22: the landing -- stopped at the floors and a hidden mate (DEC-304 (11))

Run by one Opus 5.5 subagent in the worktree `/home/max/ws/chesso-s083l`
(branch `s083l` at `5a22a9c`), machine idle (load 0.54 on 12 cores at the
start); scripts and outputs under `.tuning/coord/S083/land/` (L below).
**The landing stopped where DEC-304 (11) says it stops: `EXACT_FLOOR` falls,
157 against 163.** A second stop beside it is not a golden at all: the
paste hides a mate in two the parent sees at depth 4. Nothing was changed for
either. `src/` holds the raw paste and nothing else; no test, comment or doc
outside this file and `adocs/data/` was edited; nothing is committed. Every
reading below is a measurement taken to report the stop, by the script each
golden names.

### The paste (command 6)

`MODE=s082 adocs/data/S083_corpus.sh apply .tuning/s083/curve/s0/fit_6226944_s1.hpp
/home/max/ws/chesso-s083l`, from the main checkout, 11:56 to 12:02, `S083-APPLY-DONE`;
outputs `.tuning/s083/apply/`, copied to L/`apply_raw/`; key lines in
`adocs/data/S083_apply.txt`. The fit's sha256 `f7ccdb7f50564cb8...`.
`apply_fit.py` 5 defines, 2 tables, 10 arrays, 2 scalars; `verify_fit.py`
**823 of 823 identical**; `diff_fit.py`: PAWN 94 -> 84, KNIGHT 327 -> 371,
BISHOP 308 -> 352, ROOK 487 -> 519, QUEEN 716 -> 710; every psqt table moved
(48 to 64 squares of 64 a piece, largest single move +241, `psqt_eg[king]`
g8); 54 constants outside `eval_tables.hpp` -- mobility, king safety, passed
pawns, pawn structure -- with `piece_placement` and `tempo` frozen at 0.
`src/eval_tables.hpp` 194 lines and `src/evaluation.cpp` 20, format pass
clean. Both builds without a warning. **Bench 6042933 -> 3945685.** The fast
suite **42 of 50 in both builds**, the same eight red in each:
`test_evaluation`, `test_search`, `test_engine`, `test_tuner_gradient`,
`test_eval_trace`, `test_search_params`, `test_mate_breadth`,
`test_mate_carry`. Format check clean.

What the paste does to the tree, for the record (L/`bench.sh`, frozen copies
in L/`bin/`; the parent is the main checkout's Release build, which benched
6042933): `bench 12` 2339074 -> 2104084; the tune build at its defaults reads
the Release candidate's stream, 0 lines apart. `tools/search_bench.py`, two
interleaved runs each, nodes and best moves identical within a build:

| depth | parent (nodes, best) | paste (nodes, best) |
|---|---|---|
| 9 | 62042 / 125888 / 34107, `c3d5` / `e2a6` / `d7c8q` | 42570 / 152468 / 27750, `c3d5` / `e2a6` / `d7c8q` |
| 12 | 214973 / 595000 / 217107, `c3d5` / `d5e6` / `d7c8q` | 250176 / 412312 / 157359, `c3d5` / `e2a6` / `d7c8q` |

### The goldens, each by its script, on fixture A (DEC-142)

Fixture A, `44ce663da3669da3a7c20eac6ee54c483c7fe196`: a commit object on no
branch, `5a22a9c` plus the raw paste, the parent's goldens (L/`fixture.sh`,
S111's form). The floors, first-mate and S192 by L/`floors.sh 163`, the S170
grid by L/`sweep.sh`, each S111's script with the paths moved; 444 s for the
chain (`CHAIN-A-DONE`).

| golden | before | the paste reads | script, and the reading's file | DEC-299's guard |
|---|---|---|---|---|
| `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 163 (S286: 164 shipping, 160 weakened) | **157** exact shipping, 161 at `RfpMinPly` 2, **154** weakened (`RfpMinPly` 1); 161, 165 and 158 right sign, 0 wrong; the gate at 1 red | `S156_mined_floor_sweep.py --ref <fixA> --floor 163`, `adocs/data/S083_mined_floor.log` | **outside: the floor falls.** The tree reads 6 under 163; a floor that separates would sit in 155..157, the convention 156 |
| `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 (S286: 12 shipping, 7 weakened) | **12** of 24 shipping, **11** weakened; `red` red on the mate-in-two exactness CHECK alone, the m3 REQUIRE passing at 11 >= 11 | `S154_floor_margin_sweep.py floor` and `red`, `adocs/data/S083_floor_sweep.log` | holds its value, **no longer separates**: DEC-116's floor sits strictly between the two ends and no integer does between 11 and 12 |
| `tests/test_mate_carry.cpp` `short_line_ceiling` | 2, 5, 4, 0, 1, 2 | **2, 5, 4, 0, 2, 2** (E +1); the grid alone 0, 1, 0, 0, 2, 2; 108 cells, 1373 mate lines, 18 short | `S203_case_sweep.sh --ceilings` over S202's, S261's, S286's and this grid, `adocs/data/S083_sweep_s170.txt` | inside (+1); **not applied** |
| `adocs/data/S170_cases.tsv` budgets | A 1000000, B 100000, C 1200000, D 500000, E 300000, F 300000 | A 500000, B 300000, C 1000000, D 1000000, E 500000, F 100000; none short at its pick | `.tuning/coord/S099/p2/pick_budgets.sh`, L/`picks.txt` | inside; **not applied** |
| `tests/test_engine.cpp` `first_mate_depth`, `mate_in` | 9, 9; 5 | **9, 9; 5** (`d9:mate5` on both) | `S188_repair_goldens.py first-mate`, `adocs/data/S083_first_mate.txt` | unmoved |
| `tests/test_search.cpp` "ordering keeps the tree small" | 65024, 3251 from 16256 (13446 read) | **7783**, inside the middle half [7270, 32512] | `S192_node_budget.py`, the same file | the pair stays |
| `tests/test_evaluation.cpp` six piece anchors | 135, 244, 325, 563, 787, 0 | 99, 276, 355, 585, 983, 0 | `S192_anchors.py <fit>` (10 of 10 reproduced first, at the shipped weights), `adocs/data/S083_anchors.txt` | -- ; not applied |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 383; 198; -505; -569 | 563, 567; 383; 198; -505; -569 | 585, 580; 396 (580 less 184); 209; -534 (the four leaves still the four king moves standing pat); -599 | the same | -- ; not applied |
| `tests/test_eval_trace.cpp` `truncation_positions` | the four | read 1.875, 1.708, 2.000, 1.500: all four stop qualifying; the scan over the candidate's corpus finds 68 rows past 2.8, 15 at 2.875, 61 distinct FENs | `build/tools/truncation_scan --data .tuning/s083/curve/s0/cand_6226944_s1.tsv --min 2.8`, `adocs/data/S083_truncation_scan.txt` | -- ; not re-picked |
| `tests/test_search.cpp` mined rows | multicut 10, ProbCut 10, extra ply 8, `capture_mates` 9, 8, 11, 12 | **the multicut row is red at 10** (`d9 d11 d12`); the other six still report their mates at their depths. The fast suite never ran them: the multicut's fatal REQUIRE comes first | `S230_mine_r01_row.py depths --lo 3 --hi 12` over the seven FENs, `adocs/data/S083_rows.txt`; separation by mutants not re-read | -- ; the re-mine (`S277_mine_mate_row.py`) not run |
| bench | 6042933 | **3945685** | `chesso bench` | -- |

### The stops, with figures

1. **`EXACT_FLOOR` falls** -- 157 shipping against 163, 154 weakened. By
   DEC-304 (11) and DEC-303's consequence the floor is not lowered and the
   owner rules before any commit. The paste loses 7 of the parent's 164 exact
   mined mates at depth 10, and the separation from the weakened build
   narrows from 4 to 3. Which seven, by tool, is not taken.
2. **A mate hidden that the parent finds: `tests/test_engine.cpp` "reverse
   futility at ply 2 does not hide a mate from the side to move"**, a
   property, `NOT A GOLDEN (DEC-142)` at its site, red in both builds at
   `REQUIRE( first_exact == 4 )`, 0. The case's premise passes (the block
   fires at the floor's ply; the PV drive is mated). At the root, through UCI as
   its `deepen_scores()` reads it (`adocs/data/S083_rfp_ply2.txt`), the
   parent reports `mate -2` from depth 4 to 8; **the paste reports cp 1168,
   1125, 1009, 1009, 1027, 996, 978, 913 at depths 1 to 8 and no mate.** For
   the record, one tune option at a time, deciding nothing: `RfpMinPly` 63
   brings `mate -2` back at depth 8 and `NullMoveBase` 16 at depth 6;
   `RfpMaxDepth` 0, `RfpMargin` 2000, `RazorDepth` 0 and `ProbCut` 0 do not.
   Not re-derivable: there is no number to move, and this file does not
   change the case.
3. **`MATE_IN_THREE_FLOOR` stops separating** (above): green at 12 >= 11, and
   the weakened guard reads 11, so it can no longer fail for the reason it
   exists -- DEC-116's 2026-09-01 shape. No placement satisfies DEC-116.
4. **The guard is otherwise satisfied**: the ceilings rise by at most 1 (E),
   first-mate and the S192 pair hold.

### Reds that are not goldens, and the brief's instruction one of them meets

- **`POSITIONAL_ROOM` fires** (`tests/test_evaluation.cpp` "removing a
  piece moves the score"): the white queen on d1 evaluates 983 against her
  fitted `QUEEN` 710, residual **273** against 150 (rook 66, pawn 15). The
  answer is DEC-059's, proposed below and **not pasted**: `.tuning/reanchor.py
  <fit> <out> queen -273`, `QUEEN` 710 -> 983 and -273 on all 128 queen
  squares, written to L/`reanchor/fit_6226944_s1_q273.hpp`. `S192_anchors.py`
  on it: nine of the ten cases identical to the raw fit's, the queen on d1
  984 against 983, residual 1. `piece_value` is read by the evaluation's
  material sum alone (`src/eval_tables.hpp`'s add and remove), not by
  ordering, SEE or any pruning margin, so the shift moves `evaluate()` only
  where a tapered numerator crosses zero. Its bench was not taken.
- **`tests/test_search_params.cpp` "the correction's ranges are the ones
  their purpose declares"**: `CHECK_EQ( max_param.max_value, PAWN )`, 94
  against 84. The case pins `CorrHistMax`'s top to `PAWN` and its comment says
  "a refit of the pawn ... has to move the range with it, in the same commit,
  or this goes red". The brief says the ranges are kept (S127 owns them). The
  two cannot both hold; not touched, a question below.
- **`tests/test_tuner_gradient.cpp`, two preconditions measured on the
  shipped constants**, neither a named golden nor scripted: "the clamp's
  derivative is exact where it binds" (`REQUIRE( !binding.empty() )`: the +40
  push measured at S262 binds the clamp on 0 of the 34 rows at the paste's
  constants) and "the fit holds every gauge still and no score sees the hold"
  (`psqt_eg[king]`'s plain-update shift 0.0587 against the 0.1 floor, the
  rook's next at 0.118; its comment says to re-read it with `-s` when the
  constants move). A DEC-142 finding: measured numbers with no script. In
  passing, that case's `CAPTURE( kind_name(kind) )` prints a pointer, not the
  optimiser's name.
- The `src/search_params.hpp` comments and `MANUAL.md` rows that state a pawn
  as 94: not restated, the landing having stopped; the sites are those
  comments' 94s and `MANUAL.md`'s `QsFutilityMargin`, `CorrHistMax`,
  `LmrDeeperMargin`, `SeeQuietCoeff`, `SeMarginPerDepth` and `ProbCutMargin`
  rows.

### `adocs/data/S083_sprt.sh`'s guards

Checked on fixture A as a stand-in for the landing commit: `fixA^` is
`5a22a9c`; both `src/eval_tables.hpp` and `src/evaluation.cpp` differ;
`fastchess.sh` reads `SPRT_BOUNDS` and `SPRT_ROUNDS`. So a landing commit made
on `5a22a9c` from this paste passes all three and the script prints its
header's banner -- `candidate <CAND>`, `reference 5a22a9c...`, `tc 8+0.08
hash 16 concurrency 12 of 12 cores`, `bounds elo0=0 elo1=5 alpha=0.05
beta=0.05`, `cap 20950 rounds, 41900 games`, fastchess's `Started game 1 of
41900` -- once the coordinator's sentence is filled. It plays commits only:
DEC-303's working-tree form would need S111_sprt.sh's mode written into it.

### The corpus's generation, copied from `adocs/data/S082_pool.log`

No chunk was generated (stop at stage 0), so the corpus is S082's pool and
its record is S082's, verbatim where quoted:

- **Part 1**: `datagen --out /home/max/ws/chesso/.tuning/s082/pool.tsv
  --leaf root,qs,pv --games 150000 --nodes 100000 --threads 12 --seed
  20261013` (workers 20261013 to 20261024), engine `5b0e170`, datagen sha256
  `4b0e2848...`, started 2026-10-16 01:36:14; stopped by a power cut 11 h 22 m
  in (the files' last write 12:58:47, 11 h 22 m 33 s), before datagen's
  closing lines. **Its filter counts are lost with those lines (DEC-296)**:
  the filter's considered positions and skips by clause, refusals by kind,
  the walks' and games' nodes, the elapsed time and the games played. What
  its rows still give: 122748 games kept before game 122926, qs 10392343 rows,
  84.66 a game, 2398 refused (0.0231 %, kind unknown).
- **Part 2**: `datagen --out /home/max/ws/chesso/.tuning/s082/topup/pool.tsv
  --leaf root,qs,pv --games 27252 --nodes 100000 --threads 12 --seed
  2026102000` (workers 2026102000 to 2026102011), the same binary, started
  2026-10-20 20:49:14, `exit 0`, **`elapsed 8420 s`**. Its filter: `2751655
  considered, 2313716 passed; skipped in-check 170952, mate 0, score past
  1000 185972; no completed iteration 0`; `leaf qs: 2313173 rows from 27252
  games ... 84.88 a game`; qs not written of 2313716 passed: terminal 543
  (0.0235 %), truncated 0, aborted 0, uncertified 0.
- Merged 2026-10-21 by `S082_pool_merge.py` (part 2's ids +1000000): qs
  12705516 rows from 150000 games, sha256 `f4535bd0...`; the training side
  137998 games and 11690184 qs rows (S082's compare); the candidate's sample,
  every game at 46 a game, 6226944 rows from 137978 games (curve 0 above).

### Proposed, for the coordinator

- **DEC text, the re-anchor** (only if the landing goes on): "S083's queen is
  re-anchored through the degeneracy, DEC-059's form. The fit's queen on d1
  against bare kings evaluates 983 against her fitted `QUEEN` 710, a residual
  of 273 against `POSITIONAL_ROOM` 150. `.tuning/reanchor.py` subtracts 273
  from all 128 queen squares and adds it to `QUEEN` (710 -> 983) in the emitted
  header before the paste, so `verify_fit.py` still compares one file:
  `S192_anchors.py` reads nine of ten cases identical and the queen on d1 984,
  residual 1. `POSITIONAL_ROOM` stays 150 and the test is not edited.
  Rejected: widening the room (DEC-059's reason); pasting the raw fit (the
  guard red)." It moves play by a centipawn where a tapered numerator crosses
  zero, so it precedes any SPRT.
- **A question for the owner (DEC-265 form).** *Why*: DEC-304 (11) stops the
  landing on a fallen floor; the hidden mate in two is new ground, DEC-303
  having covered a floor and not a property; the machine is idle meanwhile.
  *Options*: (a) DEC-303's form -- the re-anchor decided, the SPRT on the
  working tree (`S083_sprt.sh` given S111_sprt.sh's working-tree mode),
  the owner ruling on H1 on `EXACT_FLOOR` (156 would separate 157 from 154)
  and on the hidden mate; the goldens inside the guard written only on H1;
  about 20 h of machine; the risk an H1 ships a mate the parent finds. (b)
  rule now that a mate hidden from the parent's reach blocks the candidate:
  S083 closes on its curve with no SPRT, recorded as zero; no machine; a gain
  of up to 0.001026 held-out error goes unmeasured, and the scale the search's
  margins were fitted on (S261's lane, `S083_sprt.sh`'s "the scale") stays
  the likely cause, S127's to re-fit. (c) first find which seven mined mates
  and which node hides the mate in two, by tool; hours of agent work, little
  machine. Recommend (a): the verdict decides whether either ruling matters,
  and an H0 ends it without one.
- **The `CorrHistMax` range**: follow the case (the range to 0..84 with
  `PAWN`, `golden_defaults`' row and `MANUAL.md`'s row in the landing commit;
  a tune-build bound, no play change) or amend the case to "one pawn at the
  seed's weights" (a test edit, the owner's). Recommend the first: it is the
  rule the tree records, and S127 fits inside whatever range ships.
- **`specs.md` and `DEV_MANUAL.md`, on S083's H1 only**: the evaluation row's
  corpus sentence (S076's deduplicated root rows) becomes "fitted on S082's
  pool's training side -- 137998 games of chesso's own self-play at 100000
  nodes a move, leaf qs, 46 rows a game, undeduplicated, 6226944 rows -- held
  out on S082's fixed validation set, L 0.071838 against 0.072864 (S083,
  DEC-304)", with the verdict; DEV_MANUAL's "The corpus the shipping weights
  descend from is `.tuning/selfplay_v2.tsv`" becomes S082's pool, its two
  commands and seeds as above, and the sample's `corpus_split.py sample`
  line.

### Not done, the landing having stopped

No golden edited, the re-anchor not pasted, no ceiling, budget or comment
moved, no row re-mined or re-picked, no mutation pass, Debug self-play or
`gate_extra.sh`. `DEV_MANUAL.md` and `MANUAL.md` checked: nothing in either
is made untrue by an uncommitted paste; the edits a completed landing owes
are listed above. `touches:` already covers what changed here
(`src/eval_tables.hpp`, `src/evaluation.cpp`, `adocs/data/`); a completed
landing adds `src/search_params.hpp`'s comments. The staged diff is
`.tuning/coord/S083/s083_landing.diff` -- **red, not a commit**.

## 2026-10-22: paused by S289 (the coordinator, DEC-305)

The landing's proposal (a), DEC-303's working-tree SPRT, is not taken: DEC-303
covered a golden count, and here a mate-safety property the parent passes
fails; an H1 would still need the cause fixed, and the fix would change the
tree the verdict measured. S083 pauses for **S289**, which finds by tool the
node and the rule that hide the mate in two and attributes the seven lost
mined mates, and fixes the rule at fault as its own measured change. S083
then re-lands curve 0's candidate on top of it: the paste again, the queen's
re-anchor (DEC-059's form, proposed above), the `CorrHistMax` range as
`tests/test_search_params.cpp` records it, and every golden by its script.
The owner rules on `EXACT_FLOOR` only if a floor still falls then. The paste
stays in `../chesso-s083l` (`.tuning/coord/S083/s083_landing.diff`).

## 2026-10-22: the re-landing on S289's guard (DEC-307), on `cd50178`'s tree (S289's guard, reverted by DEC-309), never built after its fast check's fixes

*Recorded 2026-10-23 as written: S289's guard read H0 and was reverted
(DEC-309), so this worktree's diff is not what S083's SPRT plays, nothing owed
below was run, and S083 re-lands on the tree S293's verdict leaves (DEC-311,
DEC-312), its readings there `S083s_*`. Only the `S083r_*` readings are
committed; `S083_truncation_pick.py`, `S083_tuner_preconditions.cpp`,
`S083_sprt.sh`'s working-tree mode and the `src/`, `tests/`, DEV_MANUAL and
MANUAL edits described here exist only in
`.tuning/coord/S083/s083_reland.diff`.*

Run by one Opus 5.5 subagent in the worktree `/home/max/ws/chesso-s083r`
(branch `s083r` at `b860a8a`: S289's fix `cd50178`, its SPRT pin, DEC-307),
machine idle (load 1.2 on 12 cores at the start, nothing else running);
scripts and outputs under `.tuning/coord/S083/reland/` (R below), the first
landing's `fixture.sh`, `floors.sh`, `sweep.sh`, `bench.sh`, `gate.sh` and
`chain_a.sh` copied there with their paths moved. **Nothing is committed**:
the worktree's diff is the candidate S083's SPRT plays (DEC-307 (4)). Disk:
the worktree 433 MB with both builds, R 10 MB.

### The re-anchor and the paste

`python3 .tuning/reanchor.py .tuning/s083/curve/s0/fit_6226944_s1.hpp
R/reanchor/fit_6226944_s1_q273.hpp queen -273` (DEC-307 (1)): `QUEEN` 710 ->
983, `psqt_mg[queen]` mean +661.9 -> +388.9, `psqt_eg[queen]` +170.5 ->
-102.5, 129 numbers; byte for byte the first landing's file (`cmp`), sha256
`4cf27569...`. `S192_anchors.py` on it before the paste: ten of ten at the
shipped weights first, then the queen on d1 984 against `QUEEN` 983, residual
1 against `POSITIONAL_ROOM` 150 (273 on the raw fit), the other nine cases the
raw fit's (`adocs/data/S083r_anchors.txt`).

`MODE=s082 adocs/data/S083_corpus.sh apply R/reanchor/fit_6226944_s1_q273.hpp
/home/max/ws/chesso-s083r`, from the main checkout, `S083-APPLY-DONE`; it
refused nothing (it checks for a tuner header, which the re-anchored file keeps
under reanchor.py's note, not a sha256). `apply_fit.py` 5 defines, 2 tables, 10
arrays, 2 scalars; `verify_fit.py` **823 of 823 identical**; `diff_fit.py`
PAWN 94 -> 84, KNIGHT 327 -> 371, BISHOP 308 -> 352, ROOK 487 -> 519, QUEEN
716 -> 983, the queen's tables -333 and -213 on average, the 54 outside
`eval_tables.hpp` as the first landing's; `src/eval_tables.hpp` 194 lines and
`src/evaluation.cpp` 20. **Bench 6042928 -> 3945487.** The fast suite 42 of
50 in both builds, the first landing's eight reds less `POSITIONAL_ROOM`'s
(gone with the re-anchor) and with the same eight files
(`adocs/data/S083r_apply.txt`).

What the paste does to the tree (R/`bench.sh`, `adocs/data/S083r_bench.txt`;
the parent is the main checkout's Release build at `b860a8a`, which benched
6042928): `bench 12` 2339074 -> 2103576; the tune build at its defaults reads
the Release candidate's stream, 0 lines apart. `tools/search_bench.py`, two
interleaved runs each, nodes and best moves identical within a build:

| depth | parent (nodes, best) | re-landing (nodes, best) |
|---|---|---|
| 9 | 62042 / 125888 / 34107, `c3d5` / `e2a6` / `d7c8q` | 42570 / 152468 / 27750, `c3d5` / `e2a6` / `d7c8q` |
| 12 | 214973 / 595000 / 217107, `c3d5` / `d5e6` / `d7c8q` | 250176 / 412312 / 157359, `c3d5` / `e2a6` / `d7c8q` |

Both depths read what the first landing's paste read without the guard or the
re-anchor: neither touches these trees.

### Every golden by its script (DEC-142), on fixture A

Fixture A, `80d333a7f72edd879284de26e131fe752805bef4`: a commit object on no
branch, `b860a8a` plus the re-anchored paste and the parent's goldens
(R/`fixture.sh`). R/`chain_a.sh`: bench, then R/`floors.sh 163`, then
R/`sweep.sh`, 412 s, `CHAIN-A-DONE`.

| golden | before | the re-landing reads | script, and the reading's file | DEC-299's guard |
|---|---|---|---|---|
| `test_evaluation.cpp` six piece anchors | 135, 244, 325, 563, 787, 0 | **99, 276, 355, 585, 984, 0** | `S192_anchors.py <the pasted header>`, 10 of 10 at the shipped weights first; `adocs/data/S083r_anchors.txt` | applied |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 383; 198; -505; -569 | 563, 567; 383; 198; -505; -569 | **585, 580; 396** (580 less 184); **209; -534** (leaves -566, -534, -583, -545, each Black to move with no capture, so each its stand pat: the four king moves); **-599** | the same | applied |
| `test_search.cpp` "ordering keeps the tree small" | 65024, 3251 from 16256 (13446 read) | 7783, inside the middle half [7270, 32512] | `S192_node_budget.py`, `adocs/data/S083r_first_mate.txt` | the pair stays |
| `test_engine.cpp` `first_mate_depth`, `mate_in` | 9, 9; 5 | **9, 9; 5** | `S188_repair_goldens.py first-mate`, the same file | unmoved |
| `test_mate_carry.cpp` `short_line_ceiling` | 2, 5, 4, 0, 1, 2 | **2, 5, 4, 0, 1, 3** (F +1); the grid alone 0, 3, 2, 0, 0, 3; 108 cells, 1433 mate lines, 22 short | `S203_case_sweep.sh --ceilings` over S202's, S261's, S286's, S289's and this grid (`adocs/data/S083r_sweep_s170.txt`), `adocs/data/S083r_ceilings.txt` | inside (+1); **applied**, the grid added to the command |
| `adocs/data/S170_cases.tsv` budgets | A 500000, B 100000, C 1000000, D 1200000, E 300000, F 300000 | **A 300000, B 300000, C 1200000, D 1500000, E 500000, F 100000**; B's pick has 3 short of 19, under its ceiling 5 | `.tuning/coord/S099/p2/pick_budgets.sh`, the same file | inside; **applied** by the script's rewrite, the TSV's header paragraph beside it |
| `test_eval_trace.cpp` `truncation_positions` | S076's four, reading 1.875, 1.708, 2.000, 1.500 here | **the four at 2.875, phases 5, 11, 17, 23, sides b, w, b, w**; 64 rows past 2.8, 15 at 2.875, 14 distinct | `truncation_scan --data .tuning/s083/curve/s0/cand_6226944_s1.tsv --min 2.8`, then `adocs/data/S083_truncation_pick.py` (new, the site's sentence as a rule); `adocs/data/S083r_truncation_scan.txt` | applied, in a different place: see below |
| `test_search_params.cpp` `golden_defaults` `CorrHistMax` | 47, 0, 94 | **47, 0, 84** | `src/search_params.hpp` is the derivation; the purpose case holds the top at `PAWN` | DEC-307 (2) |
| `test_tuner_gradient.cpp` "the clamp's derivative is exact where it binds", the push (an input, not a golden) | +40 (0 of 34 rows bound at these weights) | **+100**: 8 rows bound, the farthest at 434.5, the nearest other at 161.0, every row 23 from the kink, finite differences 1.09e-7 | `adocs/data/S083_tuner_preconditions.cpp clamp` (new), which answers S262's +40 at `b860a8a`'s constants; `adocs/data/S083r_tuner_preconditions.txt` | applied |
| `test_tuner_gradient.cpp` "the fit holds every gauge still", the walk's floor 0.1 | -- | **psqt_eg[king] 0.0587 under AdaGrad**, the rook's 0.118 under Adam | the same program, `walk` | **red, not touched** (below) |
| `test_search.cpp` mined rows: `capture_mates` 9, 8, 11, 12; ProbCut 10; extra ply 8 | -- | each reports its mate at its depth | `S230_mine_r01_row.py depths --lo 3 --hi 12`, `adocs/data/S083r_rows.txt` | unmoved; mutant separation not re-read (below) |
| `test_search.cpp` the multicut row, 10 | -- | **no mate at 10** (`d9 d11 d12`) | the same; re-mined by `S097_mine_mate_row.py` stages 2 to 6 in guard mode, `adocs/data/S083r_remine_multicut.log` | **red, no row found** (below) |
| `test_mate_breadth.cpp` `EXACT_FLOOR` | 163 (S289: 164 shipping, 160 weakened) | **159** shipping, 159 at `RfpMinPly` 2, **154** weakened (`RfpMinPly` 1); 162, 162 and 158 right sign, 0 wrong; the gate at 1 red | `S156_mined_floor_sweep.py --ref <fixA> --floor 163`, `adocs/data/S083r_mined_floor.log` | **outside: the floor falls**; 163 left red, not lowered |
| `test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 (S289: 12 shipping, 7 weakened) | **12** of 24 shipping, **11** weakened; `red` red on the mate-in-two exactness CHECK alone | `S154_floor_margin_sweep.py floor` and `red`, `adocs/data/S083r_floor_sweep.log` | holds, **does not separate**; untouched |
| `test_engine.cpp` the ply-2 case's first-iteration clause | 4 | **`mate -2` first at 6** (to 8), S289's paste-with-guard reading unchanged by the re-anchor | `uci_probe.py` as `deepen_scores()` reads it, `adocs/data/S083r_rfp_ply2.txt` | red at `REQUIRE( first_exact == 4 )`, untouched |
| bench | 6042928 | **3945487** | `chesso bench` | -- |

The re-anchor's own effect, the paste with the guard against the re-landing:
`EXACT_FLOOR` 158 -> 159 shipping and 155 -> 154 weakened (`S289_paste.txt`
against this), bench 3945685 -> 3945487, the ceilings' F rising where the
first landing's E did; the S192 count, first-mate, the mined rows' distances
and search_bench 9 and 12 read the same.

### The reds left, both builds, after the last edit

R/`gate.sh gate4` (`CLANG_FORMAT_MAJOR=22`), after every edit but this
section: **46 of 50 in both builds**, the same four in each, format clean;
both builds bench 3945487 and `search_bench.py` 9 and 12 read the table above.
`tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, `--gate`
and `--prose` on this file flag nothing.

1. `test_mate_breadth` -- `EXACT_FLOOR` 159 against 163, 154 weakened. A
   floor that separates 159 from 154 would sit in 155..159, the convention
   158. The owner's on an H1 (DEC-307 (4)).
2. `test_engine` -- the ply-2 case at `REQUIRE( first_exact == 4 )`, 6. The
   mate holds from 6 to 8; what delays it is late move reduction of the
   late-ordered mating quiet at ply 1 (`S289_trace.txt`). The owner's on an
   H1. `MATE_IN_THREE_FLOOR` is green (12 >= 11) and does not separate, also
   the owner's.
3. `test_search` -- "pruning does not hide a forced mate", the multicut row at
   10, fatal, so the rows after it in that case (ProbCut's) are not executed
   by the suite; `S083r_rows.txt` reads them by script. **The re-mine found no
   row**: its site names it a golden re-derived by `S097_mine_mate_row.py`,
   so stages 2 to 6 ran in guard mode (DEC-238) over the range the script
   derives from `SeMinDepth`, 7..10, E21 applied by hand to two out-of-tree
   libraries and reverted before either sweep; **0 of the 269 candidates
   separate** the shipped build from E21's, so `pick` has nothing to take.
   The row itself reads no mate at 7, 8 and 10 and mate 5 at 9 in **both**
   builds: what moved it is the paste's tree, not the guard E21 drops. E21
   stays killed by S243's direct case. Left red, as the brief says.
4. `test_tuner_gradient` -- "the fit holds every gauge still and no score
   sees the hold", `CHECK( |level_shift(plain)| > 0.1 )` at 0.0587,
   `psqt_eg[king]` under AdaGrad. **Stopped here, item 5's clause**: the walk
   is a reading of the constants and the case's descent (300 epochs, Adam at
   1, AdaGrad at 10, the gauge fixture's four rows); none of those carries a
   derivation a script could re-pick, and the only re-derivation that turns it
   green is a floor under 0.0587, a threshold lowered. The program that reads
   it is written and named at the site; the floor is the coordinator's.

### Deviations, and what a reader would otherwise re-derive

- **The truncation rows live in `tests/test_eval_trace.cpp` alone.** Until
  S083 the four were also rows of `tests/test_eval_positions.hpp`, which is
  `test_tuner_gradient`'s fixture too. Measured, not argued: swapped in there
  (R/`gate1_*`), the gradient check went red at parameter 800,
  `passed_pawn_mg[1]` -- one of S076's four was the list's only middlegame
  row for it, which S100's count cases cannot see -- and "a nonlinear group
  stuck at zero is recovered" missed its planted numbers (endgame weights 4.7
  to 13.7 off at its step 0.2); appended instead (R/`gate2_*`), the recovery
  missed the same way and the walk read the rook's 0.0856 under Adam. So S076's
  four stay in the list as coverage rows, with a comment saying so, and "the
  smooth score is within the truncation bound" adds `truncation_positions` to
  its loop, so its `worst > 2.8` is reached by the four pinned rows directly.
  No threshold or assertion changed; the loop reads four positions more (eight
  since the fast check, the mirrors with them, below).
- **The truncation pick is a script now**, `adocs/data/S083_truncation_pick.py`:
  the lowest and highest phase at the maximum, the two nearest the thirds
  between, sides b, w, b, w (the four before S083's pattern), ties to the
  lower phase and then the corpus's order. Over S276's scan it answers the
  phases and sides that stood (5, 11, 19, 23), not the same FENs, which were a
  hand's pick among 33 tied.
- **The clamp's rule was changed once before the paste's constants were
  read**: written as "the smallest push", it answers +30 at `b860a8a`'s
  constants against S262's +40; "the push whose nearest row to the kink is
  farthest from it" answers +40 there and was the one applied.
- **Mutant separation of the six rows that still report was not re-read**:
  `tools/mutation_check.py` refuses a baseline that is red, and the floors
  leave it red by design until the owner rules. The brief asks it only of a
  re-mined row, and none was re-mined.
- The sentences stating a pawn as 94 are restated "at the seed's weights" in
  `src/search_params.hpp` (`QsFutilityMargin`, `RazorMargin`, `CorrHistMax`,
  `LmrDeeperMargin`, `FutBase`/`FutSlope` -- whose minor and rook too --
  `SeeQuietCoeff`, `SeeCaptureCoeff`, `SeMarginPerDepth`, `ProbCutMargin`) and
  in `MANUAL.md`'s rows for `QsFutilityMargin`, `CorrHistMax`,
  `LmrDeeperMargin`, `SeeQuietCoeff`, `SeMarginPerDepth` and `ProbCutMargin`;
  every range kept but `CorrHistMax`'s. Historical figures (S114's margin of
  94 in `search.cpp` and DEV_MANUAL, `score > best + 94` in LmrDeeperMargin's
  census) are left: they state what was measured then.
- `DEV_MANUAL.md`'s goldens table takes S083's readings row by row; its
  corpus paragraph ("the corpus the shipping weights descend from") is
  restated for the re-landing. `MANUAL.md` checked: the six rows above and
  `CorrHistMax`'s range; nothing else on the surface moves (`test_uci_surface`
  green).

### `adocs/data/S083_sprt.sh`, working-tree mode (DEC-307 (3))

S111_sprt.sh's form: `CAND` refused if set (it would turn the run into a
commit's); `REF` the coordinator's bracketed marker, the script refusing while
any marker remains; then, in `/home/max/ws/chesso-s083r`, refused unless HEAD
is REF, REF's src/ is `cd50178`'s (the src/ every reading here was taken on;
S289's H0 would change it, DEC-307's consequence), the tree is dirty, nothing
under src/ or tests/ is unstaged and nothing under src/ untracked (added at
the fast check, below), `src/eval_tables.hpp` and `src/evaluation.cpp` differ from REF's,
`cmake --build build --target chesso` succeeds and no file of src/, cmake/ or
a CMakeLists.txt is newer than the binary, `books/noob_3moves.epd` is there,
and fastchess.sh reads both SPRT names. Bounds, control and cap unchanged:
`{0, 5}`, 8+0.08, Hash 16, concurrency 12, 20950 rounds, abort at 40 h. Not
run; a dry copy with the marker removed and `exec` replaced by an echo passed
on this tree and refused, each with its own message, REF not HEAD, REF's src/
not `cd50178`'s, `CAND` set, `TC` set otherwise, the book moved away, and a
source newer than the binary (the staged-only guard was added after, blind,
and is not yet exercised). The banner it must print:

```
candidate  <REF>  <its committer date>  + uncommitted changes
diff       /home/max/ws/chesso/.tuning/sprt_s083_<stamp>/candidate.diff  blob <12 hex>
reference  <REF>  <its committer date>
config     arch native  tune off
padding    cand on  ref on
tc 8+0.08  hash 16  concurrency 12 of 12 cores
book       noob_3moves.epd
seed       <the run's stamp, digits only>
bounds     elo0=0 elo1=5 alpha=0.05 beta=0.05
cap        20950 rounds, 41900 games, from SPRT_ROUNDS; no bound by then is no verdict
out        /home/max/ws/chesso/.tuning/sprt_s083_<stamp>
```

with fastchess's names `cand-<REF>+<hex>` and `ref-<REF>`, the candidate's
`id name` carrying `-dirty`, and `Started game 1 of 41900`.

### Proposed, for the coordinator

- **No new DEC is needed for what was done**: DEC-307 (1) to (4) cover the
  re-anchor, the range, the goldens and the run. Two things a reader would
  re-derive are proposed for its record. (a) "S083's truncation rows live in
  `tests/test_eval_trace.cpp` alone and S076's four stay in
  `tests/test_eval_positions.hpp` as coverage rows: that list is
  `test_tuner_gradient`'s fixture, and moving rows there took
  `passed_pawn_mg[1]`'s only middlegame row away and moved the nonlinear
  group's recovery (S083's record)." (b) The walk's floor: "the gauge case's
  0.1 reads 0.0587 at S083's weights (`psqt_eg[king]`, AdaGrad); on an H1 the
  owner rules between a floor re-derived from the walk by a stated rule (a
  lowered threshold), a gauge fixture re-picked by one, or the case kept red
  until S127 or S135 moves the constants again."
- **For the owner, on an H1 only** (DEC-307 (4)), beside `EXACT_FLOOR`,
  `MATE_IN_THREE_FLOOR` and the ply-2 clause: the multicut row (no separating
  candidate at 7..10 on these weights) and the walk's floor above. Taken by
  DEC-308 (the coordinator's): the walk's floor joins the H1 rulings, and the
  multicut row first gets S291's diagnosis (below).
- **`specs.md`, on S083's H1 only**: the evaluation row's corpus sentence
  becomes "fitted on S082's pool's training side -- 137998 games of chesso's
  own self-play at 100000 nodes a move, leaf qs, 46 rows a game,
  undeduplicated, 6226944 rows -- held out on S082's fixed validation set, L
  0.071838 against 0.072864 (S083, DEC-304), the queen re-anchored through the
  tuner's degeneracy before the paste (DEC-059's form, DEC-307)", with the
  verdict, and the bench figure where specs quotes one (3945487).

### The fast check's fixes, written blind (2026-10-22)

S289's SPRT held every core from the fast check on, so these were made with
edits, git and reading only: nothing below was built, run or formatted.

1. Stale text: `adocs/data/S083r_truncation_scan.txt`'s header and
   `adocs/data/S083_truncation_pick.py`'s docstring said the four were
   appended to `tests/test_eval_positions.hpp`; both now say they live in
   `tests/test_eval_trace.cpp`'s `truncation_positions` alone, S076's four
   staying in the list as coverage rows.
2. Coverage: `tests/test_eval_trace.cpp` gains
   `positions_with_truncation_rows()` -- `all_positions()` plus the four
   pinned rows **and their mirrors**, as that list gives its own rows -- and
   both "the trace reconstructs evaluate() exactly, clamp included" and "the
   smooth score is within the truncation bound" loop over it, so
   `exact_score()` is tied to `evaluate()` on the rows at the truncation
   maximum as it was before S083. No assertion changed; the reconstruction
   case's `checked > 2000` and clamp counts only see eight rows more.
3. `adocs/data/S083_sprt.sh`: a guard refusing unstaged changes under src/ or
   tests/ (`git diff --quiet -- src tests`) and untracked files under src/
   (`git ls-files --others --exclude-standard -- src`), so only the staged
   landing plays; the header, the pin sentence and the H1 outcome name the
   five reds the owner rules on at H1 (DEC-307 (4), DEC-308): `EXACT_FLOOR`,
   `MATE_IN_THREE_FLOOR`'s separation, the ply-2 clause, the multicut mined
   row and the gauge-walk floor. The multicut row first gets S291, a blocking
   diagnosis after S289's SPRT and before S083's run, whose outcome the
   coordinator writes into the pin sentence. "The commit then lands" is gone:
   nothing is committed until the rulings, and every commit is green.
   `bash -n` only.
4. `src/eval_tables.hpp`: the header comment credited the tables to S028's
   fit; it now names curve 0's candidate (S082's pool's training side, 46 qs
   rows a game, K 0.7237, held-out 0.072864 -> 0.071838, the queen
   re-anchored, DEC-307) with its files, and keeps the history before it
   (S028's fit replacing the hand-written set, S065's and S076's refits,
   S134's fold). Comment only, inside the file's `clang-format off` block.
5. This section, `DEV_MANUAL.md`'s two truncation sentences and
   `adocs/data/README.md`'s rows for `S083_sprt.sh` and the pick script.

**Owed after S289's run releases the machine**, in the worktree, with
`CLANG_FORMAT_MAJOR=22`:

- `cmake --build build -j12 && cmake --build build-tune -j12`: both build,
  no warning.
- `ctest --test-dir build -L fast --output-on-failure` and the same in
  build-tune: 46 of 50 in each, the same four red as gate4 --
  `test_mate_breadth` (159 >= 163), `test_engine` (`first_exact == 4`, 6),
  `test_search` (`result.mate_found`, the multicut row), `test_tuner_gradient`
  (0.0587 > 0.1) -- and `test_eval_trace` **green** with its eight new rows.
- `./clang-format.sh --check`: clean (`tests/test_eval_trace.cpp` is the one
  hand-formatted code edit).
- `build/src/chesso bench` and the tune build's: 3945487 both (no play
  change since gate4); `tools/search_bench.py build/src/chesso 9` and `12`:
  the table above.
- `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`,
  `--gate`, and `--prose` on this file: nothing flagged.
- `adocs/data/S083_sprt.sh`'s guards, a dry copy with the marker removed and
  `exec` replaced by an echo: `DRY-RUN-WOULD-EXEC` on the staged tree; an
  unstaged edit under tests/ refuses with `unstaged changes under src/ or
  tests/`, and an untracked file under src/ with `untracked files under
  src/`. Then S291 before S083's run.

## 2026-10-23: the re-landing on S292's and S293's guards (DEC-307, DEC-309, DEC-312)

Run by one Opus 5.5 subagent after S293's SPRT closed (H1, `b1393cd`), in
the worktree `/home/max/ws/chesso-s083s` (branch `s083s` at `e972bd4`, the
first re-landing's record on `f964a0a`; src/ `ad0cfc7`'s: S292's narrowed
null-move guard and S293's reverse futility whole-search guard, bench
6042933). Scripts and outputs under `.tuning/coord/S083/reland2/` (R2 below),
the first re-landing's `fixture.sh`, `bench.sh`, `sweep.sh`, `gate.sh` and
`wait.sh` copied there with their paths moved and `chain.sh` written after
S292's `goldens.sh`; load 0.3 to 1.4 on 12 cores, nothing else running.
**Nothing is committed**: the worktree's staged diff
(`.tuning/coord/S083/s083_reland2.diff`) is what S083's SPRT plays (DEC-307
(4)). Disk: the worktree 441 MB with both builds, R2 20 MB.

### The apply, and its three conflicts

`git apply --3way --index` of `.tuning/coord/S083/s083_reland.diff` with
`adocs/data/S083r_*`, `adocs/data/README.md` and this file excluded (the
record is `e972bd4`'s; this file's `touches:` hunk applied alone). Clean:
`src/` (three files), the six `tests/` files but `test_mate_carry.cpp`, the
two tools, `S083_sprt.sh` and `MANUAL.md`. Three conflicts, where S292 and
S293 had moved the same lines, each resolved by keeping the shipped text and
re-deriving S083's side by its script on this tree, never by picking it:

- `adocs/data/S170_cases.tsv`: the header's S292 and S293 paragraphs and
  rows (C 1500000) against the first re-landing's S083 paragraph and budgets
  (A 300000, B 300000, C 1200000, D 1500000, E 500000, F 100000, read on
  `cd50178`'s tree). Kept the shipped header and rows; the budgets are
  `pick_budgets.sh`'s rewrite over this tree's grid, and S083's paragraph is
  written again from it.
- `tests/test_mate_carry.cpp`: three comment blocks (the ceilings' history
  and `Re-derive:` command, the rule's command, the grid list), S292's and
  S293's grids against S289's and S083r's. Kept the shipped command and
  added S083s's grid; the ceilings are the script's over the six grids. The
  auto-merged F 3 and its table row now cite the S083s grid.
- `DEV_MANUAL.md`'s goldens table, one block of 22 rows: kept the shipped
  rows and wrote each S083 clause again from this tree's readings.

The conflicts the coordinator expected from S293 in
`tests/test_mate_breadth.cpp`, `tools/mutants/S191_guards.py` and
`MANUAL.md`'s `RfpMinPly` row and Known-bugs entry did not arise: the diff
touches none of those lines. All 231 mutant anchors in `tools/mutants/` occur
exactly once in this tree (`tools/mutation_check.py`'s own `validate()`, no
build).

**src/**: `git diff 38a15f4 -- src/eval_tables.hpp src/evaluation.cpp
src/search_params.hpp` and `git diff e972bd4 -- src/search.cpp` are both
empty.

### Every golden by its script (DEC-142), on this tree

Fixture A, `7774e584cd7a52de507875c1e59154c749d69fcc`: `e972bd4` plus the
paste's three src/ files, the parent's tests (R2/`fixture.sh`, a commit
object on no branch). R2/`chain.sh`, 435 s, `CHAIN-DONE`. The parent is the
main checkout's Release and tune builds at `e972bd4` (bench 6042933;
byte-different from `.ref-builds/ad0cfc7`'s, another build directory).

| golden | shipped (`e972bd4`) | `cd50178`'s tree (S083r) | now | script, reading | DEC-299's guard |
|---|---|---|---|---|---|
| six piece anchors | 135, 244, 325, 563, 787, 0 | 99, 276, 355, 585, 984, 0 | **the same** | `S192_anchors.py` (10 of 10 on the shipped tree first), on the pasted header; its embedded anchors then set to these values, and 10 of 10 with no argument on this tree; `adocs/data/S083s_eval_only.txt` | evaluation only; confirmed |
| `QUIET_ROOK_EVAL`, `_CHEAP`; 383; 198; -505; -569 | 563, 567; 383; 198; -505; -569 | 585, 580; 396; 209; -534; -599 | **the same**, the four leaves still the king moves' stand pats | the same | confirmed |
| "ordering keeps the tree small" | 13446, the pair 65024 / 3251 | 7783 | **7783**, inside [7270, 32512] | `S192_node_budget.py`; `adocs/data/S083s_first_mate.txt` | the pair stays |
| `first_mate_depth`, `mate_in` | 9, 9; 5 | 9, 9; 5 | **9, 9; 5** | `S188_repair_goldens.py first-mate`; the same file | unmoved |
| `short_line_ceiling` | 2, 5, 4, 0, 1, 2 | 2, 5, 4, 0, 1, 3 (S289's grid in the command) | **2, 6, 4, 0, 1, 3** (B +1, F +1); the grid alone 0, 6, 1, 0, 0, 3; 108 cells, 1485 mate lines, 23 short | `S203_case_sweep.sh --ceilings` over S202's, S261's, S286's, S292's, S293's and `adocs/data/S083s_sweep_s170.txt`; `adocs/data/S083s_ceilings.txt` | inside (+1 each); **applied**, the grid added to the command |
| S170 budgets | A 1000000, B 100000, C 1500000, D 500000, E 300000, F 300000 | A 300000, B 300000, C 1200000, D 1500000, E 500000, F 100000 | **A 1000000, B 100000, C 1000000, D 4000000, E 500000, F 100000**; D's 4000000 is the grid's top and the only one of D's 18 cells with a mate line, C's 1000000 a single cell with one line (1500000 reports none) | `.tuning/coord/S099/p2/pick_budgets.sh` over that grid; the same file | **applied** by the script's rewrite, the TSV's header paragraph beside it |
| `truncation_positions` | S076's four | phases 5, 11, 17, 23, sides b, w, b, w | **the same**; the scan's hit lines equal S083r's | `truncation_scan --min 2.8` on the candidate's corpus, then `adocs/data/S083_truncation_pick.py`; `adocs/data/S083s_eval_only.txt` | confirmed |
| `golden_defaults` `CorrHistMax` | 47, 0, 94 | 47, 0, 84 | **47, 0, 84** | `src/search_params.hpp` | DEC-307 (2) |
| the clamp's push (an input) | +40 | +100 | **+100**, 8 rows bound, every row 23 from the kink | `adocs/data/S083_tuner_preconditions.cpp clamp`; `adocs/data/S083s_eval_only.txt` | confirmed |
| the gauge walk's floor 0.1 | walk 0.32 / 0.20 | 0.0587 | **0.0587** (`psqt_eg[king]`, AdaGrad) | the same program, `walk` | **red**, an H1 ruling (DEC-308 (1)) |
| mined rows: `capture_mates` 9, 8, 11, 12; ProbCut 10; extra ply 8 | each at its depth | each at its depth | **each at its depth** | `S230_mine_r01_row.py depths --lo 3 --hi 12`, candidate and parent; `adocs/data/S083s_rows.txt` | unmoved; mutant separation not re-read |
| the multicut row, 10 | mate 5 at 10 | no mate at 10 | **no mate at 10** (`d9 d11 d12`) | the same | **red**, S291's attribution, an H1 ruling; not re-mined (DEC-308 (2), DEC-311 (1)) |
| `EXACT_FLOOR` | 163 (163 shipping, 160 weakened) | 159 / 154 | **159** shipping, 158 at `RfpMinPly` 2, **154** weakened; 0 wrong sign; the gate at 1 red | `S156_mined_floor_sweep.py --ref <fixA> --floor 163`; `adocs/data/S083s_mined_floor.log` | **outside: the floor falls**; 163 left red, not lowered |
| `MATE_IN_THREE_FLOOR` | 11 (12 shipping, 7 weakened) | 12 / 11 | **12** of 24 shipping, **11** weakened; `red` red on mate-in-two exactness alone | `S154_floor_margin_sweep.py floor` and `red`; `adocs/data/S083s_floor_sweep.log` | holds, **does not separate**; untouched |
| the ply-2 case's first-iteration clause | 4 (the parent's `mate -2` from 4) | 6 | **`mate -2` first at 6**, to 8 | `uci_probe.py` as `deepen_scores()` reads it; `adocs/data/S083s_rfp_ply2.txt` | red at `REQUIRE( first_exact == 4 )`, untouched |
| bench; `bench 12` | 6042933; 2339074 | 3945487; 2103576 | **3945487; 2103576**, the tune build 0 lines apart | `chesso bench`; `adocs/data/S083s_bench.txt` | -- |
| `search_bench.py` 9 and 12 | 62042 / 125888 / 34107, 214973 / 595000 / 217107 | 42570 / 152468 / 27750, 250176 / 412312 / 157359 | **the same**, `c3d5` / `e2a6` / `d7c8q` at both | the same file | -- |

Every reading but the ceilings and the budgets equals the first re-landing's:
the evaluation-only ones by construction (the constants are byte for byte
the same), the search-dependent ones because neither tree's guard fires on
them at these weights. The S170 grid differs cell by cell from S083r's
(1485 mate lines against 1433), so B's ceiling rises here where it did not
there, and five of the six budgets land elsewhere.

### The reds left, both builds

R2/`gate.sh gate3` (`CLANG_FORMAT_MAJOR=22`), after the last code edit,
the coordinator's fast-check fixes included (`gate1` and `gate2`, before
them, read the same): both builds with no warning (0 in the fresh builds' logs and in the
incremental ones); **46 of 50 in both builds**, the same four in each, each
an H1 ruling DEC-307 (4) and DEC-308 name; format clean.

1. `test_mate_breadth` -- `REQUIRE( found_exact >= EXACT_FLOOR )`, 159
   against 163; 154 weakened. A floor that separates would sit in 155..159.
2. `test_engine` -- the ply-2 case at `REQUIRE( first_exact == 4 )`, 6.
   `MATE_IN_THREE_FLOOR` is green (12 >= 11) and does not separate, also
   the owner's.
3. `test_search` -- "pruning does not hide a forced mate" at
   `REQUIRE( result.mate_found )`, the multicut row at depth 10; fatal, so
   the ProbCut row after it is read by `S083s_rows.txt` instead.
4. `test_tuner_gradient` -- "the fit holds every gauge still and no score
   sees the hold" at `CHECK( |level_shift(plain)| > 0.1 )`, 0.0586689. The
   comment above that check still gives the walk at S076's weights (0.32
   under Adam, 0.20 under AdaGrad); at S083's it is 0.1178 and 0.0587. The
   stale sentence is left for the gauge-walk ruling, which decides what the
   case asserts.

The first re-landing's fast-check fixes, written blind, are built here:
`test_eval_trace` is green with its eight added rows (the four pinned rows
and their mirrors), and `test_mate_carry` is green at the re-derived budgets
and ceilings, 35.6 s alone and 41.6 s in the parallel suite (33.9 s in the
suite before them: D's budget is now 4000000 nodes a replayed position).
`tools/plan_prose_check.py` `--citations`, `--touches`, `--params` and
`--gate`: nothing flagged.

### `adocs/data/S083_sprt.sh`, moved to this tree

The working-tree mode as the first re-landing wrote it, with the worktree
`/home/max/ws/chesso-s083s`, `readings_src` `ad0cfc7` (the src/ every reading
here was taken on, `e972bd4`'s), REF described as `ad0cfc7`'s src/ (S292's
and S293's guards on the shipped constants, bench 6042933), the five H1
rulings named with this tree's figures, and the coordinator's bracketed
sentence kept unfilled and pointed at `.tuning/coord/S083/s083_reland2.diff`.
Bounds, control and cap unchanged: `{0, 5}`, 8+0.08, Hash 16, concurrency
12, 20950 rounds; 41861 games expected with the truth at the midpoint and
25591 on a bound (DEC-143), abort at 40 h. Not run. The script itself
refuses on its marker; a dry copy (R2/`dry/`, `ref` set to `e972bd4`, the
marker renamed, `exec` replaced by an echo) prints `DRY-RUN-WOULD-EXEC` on
the staged tree and refuses, each with its own message, `CAND` set, `TC` set
otherwise, an unstaged edit under tests/, an untracked file under src/, REF
not HEAD (`f964a0a`), `readings_src` not REF's src/ (`cd50178`) and the book
moved away. The banner it must print is the first re-landing's, with REF's
sha and date.

### What a reader would otherwise re-derive

- **The record and this landing.** The italic note under the first
  re-landing's heading says the two tools, `S083_sprt.sh`'s working-tree
  mode and the `src/`, `tests/`, DEV_MANUAL and MANUAL edits existed then
  only in `.tuning/coord/S083/s083_reland.diff`; this re-landing carries
  them, built and tested.
- **`adocs/data/S192_anchors.py` carries this tree's anchors.** Its
  no-argument check compares against values embedded in the script, the
  shipped ones until now, so on this tree it read 1 of 10 (the paste being
  there). After the coordinator's fast check the eight embedded values are
  the ones the script derived from the pasted header -- 99, 276, 355, 585
  and 580, 984, 209, -599 and `QUIESCE_IN_CHECK` -534 -- and it reads 10 of
  10 here (`S083s_eval_only.txt` part 5). On H0 they leave with the paste.
- **S083r's grid is not in the ceilings' command.** It was taken on
  `cd50178`'s reverted tree, as S289's was, and the command holds the grids
  of today's engine only.
- **`MANUAL.md`.** The pawn-94 rows and `CorrHistMax`'s range as the first
  re-landing wrote them, true here. Not moved: the `RfpMinPly` row's mined
  set figures (163, 166, 160 at the shipped weights) and `RfpMaxDepth`'s
  constructed-set figure, both measurements of the shipped constants; the
  paste reads 159, 158 and 154 (`S083s_mined_floor.log`), which lands with
  the owner's `EXACT_FLOOR` ruling on an H1.
- **`specs.md`, on S083's H1 only**: the first re-landing's proposal stands,
  with the bench figure 3945487 (6042933 before).

### Owed

Nothing on this tree before the run but the coordinator's fast check and
pin. On H1: the owner's five rulings, then the commit (green), the mutation
pass the red baseline refuses now, and `specs.md`. Tier 2 is not owed: no
search code moves.

## 2026-10-25: the owner's rulings (DEC-313)

Run by one Opus 5.5 subagent after S083's SPRT read H1 (Elo +116.07 +/-
23.26, nElo +153.75 +/- 28.57, 568 games), in the worktree
`/home/max/ws/chesso-s083s` on the staged re-landing the run played
(`git diff --cached --binary` sha256 `aecfed38...`,
`.tuning/coord/S083/s083_reland2_pinned.diff`). The owner ruled each of the
five reds the way the coordinator recommended, DEC-313 (the coordinator's text,
`.tuning/coord/S083/dec313.md`). Scripts and outputs under
`.tuning/coord/S083/rulings/` (R3 below): `fixture.sh` (fixture.sh of the
re-landing, generalised to name the paths it overlays), `floor158.sh`,
`gate.sh`. Two fixtures, commit objects on no branch: **F1** `352a4db`,
fixture A with `EXACT_FLOOR` at 158 and nothing else; **F2** `e8fe2c7`, the
whole working tree as the mutation pass took it (src/, tests/, tools/ final).
**`src/` did not move**: `git diff --cached --full-index --binary -- src/` is
byte for byte the src/ hunks of the run's `candidate.diff` (sha256
`8611af2f...` both), and nothing under src/ is unstaged.

### The five rulings, each by its script

| ruling | site | before | after | script, reading |
|---|---|---|---|---|
| (1) | `test_mate_breadth.cpp` `EXACT_FLOOR` | 163, red at 159 | **158**: 159 shipping, green; the gate built at `RfpMinPly` 1 with 158 compiled in **red at 154**; the GOLDEN block's ends, margin and band (155 to 159) moved, a dated paragraph added | `S156_mined_floor_sweep.py --ref 352a4db --floor 158 --jobs 12` on F1: 159, 158, 154, 154 at 3, 2, 1, 0, row for row `S083s_mined_floor.log`'s; `adocs/data/S083s_mined_floor_158.log` |
| (2) | `test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11, the GOLDEN block giving S250's ends 12 and 7 and a margin of 4 | **11**, unchanged; the GOLDEN block gives 12 and 11 and a margin of 0, and a dated paragraph says it no longer separates `RfpMinPly` 1 and that `EXACT_FLOOR` (158 against 159 and 154) carries it | `S154_floor_margin_sweep.py floor` and `red`, read 2026-10-23 on this src/; `adocs/data/S083s_floor_sweep.log`. Not re-run: the value holds and src/ is the same |
| (3) | `test_engine.cpp` "reverse futility at ply 2 does not hide a mate from the side to move" | `REQUIRE( first_exact == 4 )`, red at 6, marked NOT A GOLDEN (the oracle's Mate(-2) at twice its distance) | **`first_exact == 6`**, the rest of the case unchanged (`mate -2` still asserted at the last of 8); now a value read off the engine, so marked GOLDEN (DEC-142) with its script | `adocs/data/S083s_rfp_ply2.py` (added): the case's root at depth 8 through `S154_floor_margin_sweep.py`'s `engine_t`, `deepen_scores()`'s parse. 6 on this tree (4149 nodes), 4 on the parent's Release build at `e972bd4` (3617), both node for node the 2026-10-23 sections; appended to `adocs/data/S083s_rfp_ply2.txt` |
| (4) | `test_search.cpp` "pruning does not hide a forced mate", "mate the multicut hides, depth 10" | the row, red at its fatal `REQUIRE( result.mate_found )`, the rows after it never run | **retired**: the constant and its block removed; the row's history stays as comments, headed by a pointer, its GOLDEN line marked ended, and a dated note where the row stood (DEC-313 (4), S291's attribution, E21 held by S243's direct case) | No TSV or script emits the row: it was inline in the case. The case now runs whole, **54 of 54 assertions** (41 before, the 41st failing), the ProbCut row, `mate_probcut_hides`, green at both its requirements (`./test_search --test-case="pruning does not hide a forced mate" -s`, R3/`pruning_case_s.log`) |
| (5) | `test_tuner_gradient.cpp` "the fit holds every gauge still and no score sees the hold" | `> 0.1`, red at 0.0587; the comment at `f57b92f`'s 0.32 / 0.20 | **`> 0.01`**; the comment rewritten at S083's figures | `S083_tuner_preconditions.cpp walk`, its `WALK_FLOOR` moved to the case's 0.01 in the same edit: Adam 0.1178 (rook) to 4.0181 (knight), AdaGrad 0.0587 (`psqt_eg[king]`) to 4.5680 (pawn), every hold at most 1.13e-14, the runs 6.82e-13 apart; exit 0. `adocs/data/S083s_gauge_walk.txt`, figure for figure `S083s_eval_only.txt` part 4 |

### Deviations from the brief, and what a reader would otherwise re-derive

- **(1) read on F1, not fixture A.** Fixture A carries the parent's tests, so
  its gate has 163 compiled in and cannot show the red at 158 the ruling
  needs. F1 is A plus that one line; the sweep's counts are A's.
- **(3) a script added.** The clause's 6 is read off the engine, which makes
  it a golden, and DEC-142 asks for the script at its site; the probe that
  read it on 2026-10-23, `.tuning/coord/S083/land/probe/uci_probe.py`, is
  gitignored. `S083s_rfp_ply2.py` reuses S154's `engine_t` rather than a
  second parser, so `first_exact` is parsed as the case parses it.
- **(5) the walk program follows the case.** It printed "under the case's
  0.1" and exited 2 below it; reporting against a floor the case no longer
  asserts would be a wrong statement, so its threshold is `WALK_FLOOR` 0.01
  and its header says why.
- **(4), the row's other names.** E21's description in
  `tools/mutants/S097_singular_extension.py` said the row "stays as a second
  witness"; it now says the row was retired and why, and the file's header
  sentence "the one a mined row carries" is put in the past. Left as they are,
  being dated records of other trees: S131's pass table in
  `tools/mutants/S131_quiet_queen_promotions.py` (V04 "at its multicut row")
  and the ProbCut row's S113 history ("fails first at the multicut row
  above"). Every log under `adocs/data/` that names the row is a committed
  reading and is not touched.

### Gates, after the last test edit

R3/`gate.sh g1` (`CLANG_FORMAT_MAJOR=22`, `-j12`): both builds with **0
warnings**; **50 of 50 in both builds** (`build/` 112 s, `build-tune/`);
`./clang-format.sh --check` clean. Bench **3945487** in both builds,
unchanged. `tools/plan_prose_check.py` `--citations`, `--touches`,
`--params`, `--gate`: nothing flagged, after one line-form citation in this
section was put in a symbol's form (DEC-135); the two `MISSING` notes it
prints are S136's, not this step's.

### The mutation pass

`tools/mutation_check.py tools/mutants` over **every mutant file, 231
mutants**, on a clean detached worktree of F2 (`.ref-builds/mut83`, `--jobs
12`, `CLANG_FORMAT_MAJOR=22`), detached, watched by a poll loop with all four
exits; estimated 7.3 h from S286's and S293's 113 s a mutant, 28196 s (7 h 50
m) taken. Baseline green, 50 tests, bench 3945487. **224 of 225 scored
killed**, 5 equivalent as the list declares them (B15, RZ05, V05, W01, M26),
and two verdicts the list does not expect (`adocs/data/S083s_mutation.tsv`):

- `MW02_aborted_line_no_fallback`, **survived** (0 of 50 failed, bench the
  same), `tools/mutants/S202_mate_walk.py`; its file names
  `test_mate_carry`'s F cell as the killer.
- `H04_cont_hist_unread`, **stillborn**, `tools/mutants/S222_continuation_history.py`:
  the mutant leaves `prev_move` unused in `src/evaluation.hpp`'s
  `quiet_history_sum` and `-Werror=unused-parameter` refuses the build.

**Neither is S083's.** The same `--only MW02_aborted_line_no_fallback
H04_cont_hist_unread` on a clean worktree at the parent `e972bd4` (bench
6042933, 280 s) reads MW02 survived, 0 of 50, and H04 stillborn on the same
error; S083 does not touch `src/evaluation.hpp` or `src/chesso.cpp`. Both are
left as found, not fixed here, for the coordinator. **E21 is killed**, by
`test_search` at S243's direct case ("the multicut never ends a node on a mate
from its verification") and by `test_mate_breadth` at `EXACT_FLOOR` 158: the
retired row's kill gone, the direct case's left, as DEC-313 (4) expected.
**M06a is killed** by `test_engine` at the ply-2 case's `first_exact == 6`
and at S289's root case, and by `test_mate_carry`; C02, C05 and R02 are killed
by `test_search` and `test_mate_breadth` (R02 by `test_engine` too), per
binary, not per row. Disk: the fixture worktree 329 MB while it stood, the
logs 7 MB (R3/`mutation/`); the parent's fixture the same size, removed after.

### Documents

- `DEV_MANUAL.md`: the goldens table's `EXACT_FLOOR` row (158 since S083, its
  command `--floor 158`), `MATE_IN_THREE_FLOOR` row (held, no longer
  separating), a new row for the ply-2 golden (`grep -rn 'GOLDEN (DEC-142)'
  tests/` lists it now), the multicut row marked retired; the tuner section's
  gauge paragraph names the walk's floor, 0.01 since S083.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures at this tree's 159,
  158 and 154 against the floor 158 (`S083s_mined_floor.log`,
  `S083s_mined_floor_158.log`), the S293 figures kept as history. Nothing
  else on the surface moves.
- `adocs/data/README.md`: rows for `S083s_mined_floor_158.log`,
  `S083s_rfp_ply2.py`, `S083s_gauge_walk.txt` and `S083s_mutation.tsv`; the
  rows for `S083_tuner_preconditions.cpp`, `S083s_mined_floor.log`,
  `S083s_floor_sweep.log` and `S083s_rfp_ply2.txt` amended. `README.md` not
  touched.

## 2026-10-25: the SPRT's H1, and the landing (the coordinator)

**The run** (`adocs/data/S083_sprt.sh`, pinned at `e972bd4`, started
2026-10-23 21:31): the re-landing's working tree in `../chesso-s083s`
against `e972bd4`, `{0, 5}`, 8+0.08, Hash 16, `noob_3moves.epd`,
concurrency 12. **H1 in 16 m 18 s**: Elo +116.07 +/- 23.26, nElo +153.75
+/- 28.57, LLR 2.94 (-2.94, 2.94), 568 games, W 288 L 105 D 175, Ptnml
[7, 33, 78, 102, 64], 0 forfeits (`adocs/data/S083_sprt.log`; untrimmed as
`chesso_S083_sprt_2026-10-23.tar.xz` in the owner's archive). The played
diff is `adocs/data/S083_candidate.diff`, blob `c875e362fbb2` (DEC-254),
identical to `.tuning/coord/S083/s083_reland2_pinned.diff` but for
`--full-index`.

**Inspected**: the banner as pinned; the candidate at bench 3945487 and the
reference at 6042933, both Release, native, tune off; one "PV continues
after fifty-move rule" block, from the reference; no crash or disconnect;
quarters 65.1, 69.0, 64.1 and 66.9 % of the candidate's points over 142,
142, 142 and 145 games; kernel journal clean for the run's window. An SPRT
that stops this early biases its point estimate upward.

**Every decisive game was adjudicated.** All 396 ended by fastchess's
two-sided resignation rule (both engines at 400 cp or more for 3 moves);
the 114 normal terminations were all draws. So each decisive game's final
position went to Stockfish (`/usr/games/stockfish`, depth 16, Threads 4,
Hash 64, through python-chess, TOOLCHAIN.md's safe form;
`.tuning/coord/S083/verdict/adjud_check2.py`): it agrees with the
adjudicated winner in **291 of 291** candidate wins and **105 of 105**
candidate losses, the winner's margin under 200 cp in 7 and 0, medians 677
and 664 cp. One position (game 359) took the script's 60 s belt and still
reported depth 16. The first form of the script, without the belt or
per-game output, ran 37 h on that position before it was stopped
(`adjud_check.py`, kept beside it).

**The owner's rulings** (DEC-313, 2026-10-25), applied by the section above
and gated there: `EXACT_FLOOR` 158, `MATE_IN_THREE_FLOOR` 11 not separating,
the ply-2 clause 6, the multicut row retired, the gauge-walk floor 0.01.
DEC-313 also ratifies DEC-312.

**What landed**: `src/` byte for byte the candidate's; the tests, readings,
DEV_MANUAL and MANUAL of this tree; `specs.md`'s evaluation row on the
corpus the constants are now fitted to. Bench 3945487.

**Found beside it**: the mutation pass's two off-list verdicts, MW02
survived and H04 stillborn, read the same at `e972bd4`, so they are the
shipped tree's: S294, a DEC-171 filler.
