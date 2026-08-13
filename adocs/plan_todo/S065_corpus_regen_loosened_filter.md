id:         S065
goal:       regenerate the tuning corpus from today's engine with the tactical-move filter loosened, and fit it
accepts:    `.tuning/selfplay_v2.tsv` exists, generated at this tree's HEAD by `--games 120000 --nodes 100000 --threads 12 --seed 20260814 --allow-tactical 1` with `--quiet-limit` left at 1000, which the run's own first log line states; it holds at least 10 M positions from at least 100000 games, every row four tab-separated fields whose first is a six-field FEN, `result` in {0.0, 0.5, 1.0} and `phase` in [0, 24]; the run's closing `filter:` line is recorded with the per-clause counts; a fit over it reports a held-out error below its own starting figure and prints no refusal warning; the returned constants are measured by one SPRT against the constants shipping today and **the verdict is recorded, with zero recorded as zero** and a negative verdict reverting the paste rather than retrying the corpus
touches:    tools/datagen.cpp, DEV_MANUAL.md, adocs/testing.md, then src/eval_tables.hpp and src/evaluation.cpp values only for the paste
excludes:   changing which evaluation terms exist; any change to what `evaluate()` computes; the validation splitter, which is S066; loosening a second filter clause or changing `--nodes` in the same corpus, either of which would put two changes behind one verdict
decisions:  DEC-055, DEC-016, DEC-019, DEC-041, DEC-050
closes:
blocks:
paused_by:
done:

## Why this cuts the queue

Two facts, neither of them a preference.

**There is no corpus on this machine.** `2026-08-13_plan_review.2-F02` recorded
it: `.tuning` is gitignored (`.gitignore:11`), the work moved machines at
DEC-049, and `.tuning/selfplay_v1.tsv` did not come along. Every fit the plan
still wants -- S027's carried-forward terms, any refit after a model change --
needs a corpus that does not exist here, and F02 also established that the old
one is not regenerable identically, because S027 and S028 changed the engine
that generates it.

**The corpus is stale by construction anyway.** `selfplay_v1.tsv` was produced
by a build that predates mobility, king safety, passed pawns, pawn structure
and the S028 fit itself. `plan_done/S028_texel_tuning.md:177-179` closes on
exactly this: "The data is one generation of self-play at 100000 nodes a move.
Nothing here says a second generation, sampled from the stronger engine this
produced, would not do better; nothing here says it would either."

DEC-055 is the decision to run it tonight, ahead of S033. It is tuning rather
than a match, it needs no engine code, and it fills a long night that would
otherwise hold one SPRT.

## The filter change, and what decided it

### What the filter was

`tools/datagen.cpp` recorded a position only when four clauses all held: the
side to move is not in check, the search's chosen move is neither a capture nor
a promotion, no mate was found, and `|score| < --quiet-limit` (1000). The file
justified the second clause in its own header:

> A tuned evaluation is fitted to what evaluate() returns, and evaluate() is
> only asked about a position that quiescence has already resolved, so training
> it on positions in the middle of an exchange fits it to noise it never sees in
> play.

### That justification is false, and the code says so

`src/search.cpp:112-138`:

```
  state->explored_nodes++;

  // Lazy: quiescence is where the evaluation is called most, and most of those
  // nodes are nowhere near the window. S034.
  const int stand_pat = evaluate_lazy(&game->board, alpha, beta);
  ...
  if (!in_check) {
    if (stand_pat >= beta) { return stand_pat; }
    if (stand_pat > alpha) { alpha = stand_pat; }
  }
```

The static evaluation is called at the **top of every quiescence node**, at
line 125, before `generate_captures` at `:152` has produced a single move. A
node with a winning capture available gets its static score asked for and used
as the stand-pat bound at `:136-137`. Positions whose best move is a capture
are not positions `evaluate()` is spared -- they are among the positions it is
asked about most.

### What the literature says

The survey's C2(ii). Österlund's largest single published methodology gain came
from *removing* an exclusion of this family -- he had been dropping positions
where the quiescence score deviated from the search score, and admitting them
helped, on the argument that the evaluation has to handle those positions
anyway (CPW, Texel's Tuning Method). Grant filters only mate scores
(*Evaluation & Tuning in Chess Engines*, 2020, §2.2). Christian Dean's Blunder
experiments converged on excluding only check, mate and a 25 cp
quiescence-versus-evaluation disagreement (TalkChess t=78536). All three admit
positions with captures available; chesso alone excluded them.

Reported figures decide what to try, never what to conclude. DEC-019.

### What each clause actually costs, measured

`build/tools/datagen --games 240 --nodes 100000 --threads 12 --seed 20260814`
at HEAD, with the per-clause counters this step added. Each figure is the
number of positions that would have been recorded but for that one clause:

```
filter: 27839 considered, 17593 recorded; skipped in-check 1698,
        tactical best move 4814, mate 0, score past 1000 1824
```

| clause | marginal cost | decision |
|---|---|---|
| in check | 1698, 6.1 % of considered | **kept.** All three surveyed engines keep it, and the stand-pat score is not used as a bound while in check (`src/search.cpp:135`) |
| tactical best move | 4814, 17.3 % of considered, **+27.4 % on top of what is recorded** | **loosened.** The clause the engine's own hot path contradicts |
| mate found | 0 | **kept.** It is shadowed by the score cap, not redundant: with the cap removed it rejects 649 of the same 27839 |
| score past 1000 | 1824, 6.6 % of considered | **kept.** See below |

### Why the score cap stays at 1000

The cap is the other clause with no support in any surveyed engine, so it was
measured rather than argued. Same 240 games with the cap removed
(`--quiet-limit 1000000 --allow-tactical 1`): 24603 rows recorded, of which
2196 (8.93 %) lie past 1000. The tuner weights a row by `sig * (1 - sig)`
(`tools/tuner.cpp:380-384`), and at the K this corpus fits, 0.7472, that factor
is 0.01319 at a score of 1000 against 0.25 at an equal position -- 5.28 %, and
falling. Summed over the corpus, **the rows past 1000 carry 0.206 % of the
total gradient mass.**

Uncapping is therefore measurably close to inert, and keeping it means the new
corpus differs from S028's on exactly one filter clause, which is the only
shape in which the SPRT that follows attributes anything.

### How it is expressed

`--allow-tactical N`, default 0. Zero is what `selfplay_v1.tsv` was generated
under, so the old behaviour stays reachable and the change is a run parameter
rather than a rewrite. The flag was proved inert at its default: 240 games at
seed 20260814 through the pre-change binary and the post-change binary produce
byte-identical output once sorted, md5 `2a13141f0b86f333684df5b2be333104` both
ways, over the same 240 games and 17593 positions.

## The run

```bash
nohup build/tools/datagen --out .tuning/selfplay_v2.tsv \
    --games 120000 --nodes 100000 --threads 12 --seed 20260814 \
    --allow-tactical 1 > .tuning/datagen_v2.log 2>&1 &
```

Detached, with a watcher that outlives the turn, AGENTS.md section 0. Progress
without disturbing it is `wc -l .tuning/selfplay_v2.tsv`. `.tuning/` is
gitignored (`.gitignore:11`), confirmed by
`git check-ignore -v .tuning/selfplay_v2.tsv`.

Datagen `fflush`es under the output lock once per finished game, so a run
stopped early leaves a complete, parseable file of whole games -- and the
tuner's `load()` skips a short line rather than failing on one
(`tools/tuner.cpp:182-185`).

### Where 120000 comes from

Smoke run at the exact settings above, `--games 600`, on this machine:

```
600 games, 56304 positions written to .tuning/smoke_v2.tsv
filter: 69579 considered, 56304 recorded; skipped in-check 4962,
        tactical best move 0, mate 0, score past 1000 5407
wall 143.93 s
```

- 600 / 143.93 s = **4.169 games/s**; 56304 / 143.93 s = **391.2 positions/s**
- 56304 / 600 = **93.84 positions per game**, against 73.3 under the old filter
  at the same seed -- +28.0 %, which is the 4814/17593 the counters predicted
- 8 h = 28800 s. 4.169 x 28800 = **120067 games**; 391.2 x 28800 =
  **11.27 M positions**. Cross-check: 120000 x 93.84 = 11.26 M
- 3611822 bytes / 56304 rows = 64.15 B/row, so **723 MB** on disk. 34 G free

**S028's corpus was 1490839 positions from 20000 games.** This is 7.6 times the
positions and 6.0 times the independent games, which is also the direction
Grant §2.2 and Österlund argue for -- the independent unit is the game, and
S028 had 20000 of them.

### The fit afterwards

```bash
nohup build/tools/tuner --data .tuning/selfplay_v2.tsv \
    --out .tuning/tuned_v2.hpp --threads 12 > .tuning/tuner_v2.log 2>&1 &
```

Measured on the smoke corpus at 12 threads: load 0.29 s, and 500 epochs in
1.11 s, so **2.22 ms per epoch at 56304 rows**. Scaling by rows: about 58 s to
load and **0.44 s per epoch** at 11.27 M, so S028's 11200 epochs come to
roughly **1 h 23 m**. Resident set was 10.9 MB on the smoke corpus, so about
**2.2 GB** at full size against 9 GB available -- it fits, and it has to,
because the tuner holds the whole corpus in memory.

Held-out error across corpora is **not comparable**: a different corpus is a
different objective, and the K that scales it is refitted per corpus (0.7472
here against S028's 1.1141). The held-out figure is the tuner's own refusal
gate and nothing more. The verdict is the SPRT.

Then paste over **both** files -- `src/eval_tables.hpp` and
`src/evaluation.cpp`, all 54 parameters below the two tables included, which
`DEV_MANUAL.md` warns is the half that gets left behind -- and run
`REF=<sha> ./fastchess.sh`.

## What this produces, and what it does not

**Candidate weights, not a verdict.** 827 constants move at once, which is one
change by SPRT's standard and 827 numbers by any other. Nothing is kept without
the match, and a verdict of zero is recorded as zero. DEC-055, INV-6, DEC-019.

The corpus is also the input S039 has been missing: `eval_spread` reads the FEN
as the first tab-separated field (`tools/eval_spread.cpp:163-165`), which is
what `selfplay_v2.tsv` writes. Re-pointing S039 is S057's business, not this
step's.
