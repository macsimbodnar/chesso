id:         S065
goal:       regenerate the tuning corpus from today's engine with the tactical-move filter loosened, and fit it
accepts:    `.tuning/selfplay_v2.tsv` exists, generated at this tree's HEAD by `--games 120000 --nodes 100000 --threads 12 --seed 20260814 --allow-tactical 1` with `--quiet-limit` left at 1000, which the run's own first log line states; it holds at least 10 M positions from at least 100000 games, every row four tab-separated fields whose first is a six-field FEN, `result` in {0.0, 0.5, 1.0} and `phase` in [0, 24]; the run's closing `filter:` line is recorded with the per-clause counts; a fit over it reports a held-out error below its own starting figure and prints no refusal warning; the returned constants are measured by one SPRT against the constants shipping today and **the verdict is recorded, with zero recorded as zero** and a negative verdict reverting the paste rather than retrying the corpus
touches:    tools/datagen.cpp, DEV_MANUAL.md, adocs/testing.md, then src/eval_tables.hpp and src/evaluation.cpp values only for the paste
excludes:   changing which evaluation terms exist; any change to what `evaluate()` computes; the validation splitter, which is S066; loosening a second filter clause or changing `--nodes` in the same corpus, either of which would put two changes behind one verdict
decisions:  DEC-055, DEC-016, DEC-019, DEC-041, DEC-050
closes:
blocks:
paused_by: S067  # 2026-08-14
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

## The corpus, as generated

```
datagen: 120000 games, 100000 nodes per move, 12 threads, seed 20260814, quiet-limit 1000, allow-tactical 1
120000 games, 11003693 positions written to .tuning/selfplay_v2.tsv
filter: 13759085 considered, 11003693 recorded; skipped in-check 957698, tactical best move 0, mate 0, score past 1000 1205452
```

11003693 positions from 120000 games, 715409623 bytes, 2026-08-14 01:05:11 to
08:13:32 — **7 h 08 m**, 4.669 games/s, 428.2 positions/s, 91.70 positions per
game, 65.0 bytes per row. The 600-game smoke run predicted 120067 games and
11.27 M positions in eight hours; the 120000 games it was configured for finished
in 7 h 08 m and produced 11.00 M -- faster per game, thinner per game.
`tactical best move 0` is the loosened clause switched off, so it rejects nothing
and has nothing to count.

## The fit, as run

```bash
build/tools/tuner --data .tuning/selfplay_v2.tsv --out .tuning/tuned_v2.hpp \
    --threads 12 --epochs 5000
```

Everything else default: `--lr 1.0 --report 100 --patience 20 --validation 0.1
--seed 1`, and `--k` left at 0 so K is fitted from the train rows.

```
11003693 positions, 119998 games, 9903305 train, 1100388 validation (10.0002%), 827 parameters, group all, 827 free
fitted K = 0.7624
start: train 0.122741  validation 0.122560
best: train 0.117043  validation 0.117359  (start 0.122741 / 0.122560)
```

**K = 0.7624**, fitted per corpus and not carried: S028's was 1.1141 over
`selfplay_v1.tsv`. Best held-out error at **epoch 4800**, no refusal warning,
2082 s of wall on 12 threads. Emitted header sha256
`b60e71341ac42a1a71e683f9c74d87b56dafb329a8403ec2e2534dffb6440bc4`.

The curve, held out: 0.118022 at 100 epochs, 0.117532 at 1000, 0.117424 at 2000,
0.117380 at 3000, 0.117366 at 4000, 0.117359 at 4800. Against a total gain of
0.005201, **87.3 % of it is inside the first 100 epochs and 96.7 % inside the
first 1000**; the sixth decimal is where the last two thousand live.

**Why 5000 epochs.** The first attempt ran at the default `--epochs 20000` and
was killed by the harness at epoch 8500, before the header was written. It had
reached held out **0.117357**, which is 2e-06 below the 4800 best over 3700
further epochs. The re-run's log is identical to it line for line through epoch
5000, so the fit is deterministic at a given seed and thread count and the budget
does not change the curve, only where it stops. The tuner keeps the best held-out
checkpoint, so a longer budget could only have bought that 2e-06.

**The held-out figure cannot rank this fit against S028's** and is not offered as
one: different corpus, different objective, K refitted per corpus, and S028's
0.113852 → 0.108043 was measured under the row-level split S066 replaced. It is
the tuner's own refusal gate and nothing more. The verdict is the SPRT.

## The paste, and the three guards that stopped it

Applied mechanically and verified rather than by eye: **827 parameters compared,
827 identical** to the emitted header after the paste — the five defines and both
tables into `src/eval_tables.hpp`, then `mobility_mg/eg`, `king_safety_mg/eg`,
`passed_pawn_mg/eg`, `pawn_structure_mg/eg`, `piece_placement_mg/eg` and the two
tempo scalars into `src/evaluation.cpp`, which is the 54 `DEV_MANUAL.md` warns are
the half that gets left behind. `git diff` was values only, 111 lines each way.

`ctest -L fast` then came back **9 of 12**, and each of the three is a guard
placed by an earlier step firing on exactly what it was built to catch:

| guard | what it says |
|---|---|
| `test_eval_model`, `CHECK(tempo_unfitted)` | `tempo_mg = 39`, `tempo_eg = 21`. All four taperings can now truncate, so the bound is 4 x 23/24 = **3.833** and the tolerance has to be 4. This is DEC-053's own stated consequence and the first fit to price tempo at all |
| `test_eval_model`, the four pinned FENs | their residuals are of the **old** weights. Now 2.541667 / 1.250000 / 1.125000 / 2.416667 against 2.875 / 2.333333 / 2.25 / 2.125, so two fall under the `> 2.0` threshold and the worst under `> 2.8` |
| `test_evaluation`, "removing a piece moves the score" | a white queen on d1 against bare kings is worth **713** where her own fitted material value is 1152 — **439** off, against `POSITIONAL_ROOM` 150. -354 of it is `psqt_eg[queen][d1]` at phase 4, -116 is `mobility_eg[queen]` over 17 squares, +28 is king safety |

Seven value anchors moved with them, which is what anchors are for, and all seven
were reproduced by a second implementation of `evaluate()` written from the
specification: 88 → 148, 240 → 237, 325 → 283, 530 → 515 (cheap 524 → 536),
1101 → 734, 0 → **21** (bare kings, which is tempo at phase 0), 266 → 251. That
implementation reproduces 8 of 8 of the shipped values on the shipped weights,
which is what says the new numbers are derived and not read off the engine.
`test_search`'s two -491 cases moved to -454 and -503 and were not re-derived.

**The paste is reverted and the tree is green.** The first guard is arithmetic
DEC-053 already decided. The other two hold numbers that are evidence of the old
weights, and re-targeting either is a decision rather than a paste:

- the four pinned FENs were the worst residuals the 2026-08-13 audit found over
  200000 positions **under the old weights**. Re-pinning them means measuring the
  worst residuals under the new ones and deriving new thresholds from the 3.833
  bound — S038's procedure, run again
- `POSITIONAL_ROOM` is a property and not an anchor: it asserts a piece is worth
  its material to within what one square's positional terms can say. Widening it
  is weakening it, and what fires it is a real property of these constants

**S056 does not unblock this.** Its `touches:` is
`adocs/plan_todo/S055_taper_stage_two_once.md` and its `excludes:` forbids
editing `tests/test_eval_model.cpp`, so it re-targets S055's acceptance text and
changes no assertion in the suite.

## One thing the guards found, as data

The queen's two tables moved in opposite directions — `psqt_mg[queen]` by a mean
of **+270** and `psqt_eg[queen]` by **-324**, with `QUEEN` 1067 → 1152 and
`mobility_eg[queen]` 3 → **-9**. That is not the five-dimensional degeneracy the
tables document, which needs the *same* constant added to both; it is a change in
the shape of the taper, and it is the largest single movement in the fit.

Measured coverage, one pass over all 11003693 rows: mean phase 13.06,
`phase <= 4` on 19.264 % of rows, a queen anywhere on 50.1 % — and **a queen at
`phase <= 4` on 9786 rows, 0.0889 %**. `phase <= 4` with a queen is one queen and
no other piece, so the endgame end of that table is fitted from about ten thousand
rows in eleven million. Data, not a chess judgement, and recorded because it is
what `POSITIONAL_ROOM` fired on.

## The second fit: tempo and piece_placement frozen, DEC-057

The owner's decision on the three guards above. `tempo` is held so the
model-versus-engine bound stays at three effective divisions and guard 1 cannot
fire; `piece_placement` is held because S027 measured it at **-5.48 +/- 11.46
Elo over 2284 games, H0 accepted** and zeroed it so the compiler would delete the
term, and re-applying it re-pays a measured 3.1-4.0 % search cost for a term
measured at zero. Held **during** the fit, not zeroed after it: with a parameter
held the remaining 817 absorb what it would have taken.

### How they were frozen

`--only` frees one group and cannot say "everything but tempo", so
`tools/tuner.cpp` gained `--freeze LIST`, a comma-separated subset of the same
group names, held at what the engine ships. It clears bits from the mask
`free_mask` already produced and therefore changes no group range, which is what
keeps `test_tuner_groups`' three partition properties untouched — they still pass
unchanged, 4 cases and 1757 assertions.

**Proved inert at its default.** The pre-change binary — `HEAD:tools/tuner.cpp`
compiled against the same engine library — and the post-change binary over the
smoke corpus at `--threads 4 --epochs 300 --report 100`: every epoch report
identical, `best:` line identical, and the emitted headers differing in exactly
one added line, `// freeze     (nothing)`. All 827 constants identical.

Three refusals, each `rc=1` before the corpus is read: an unknown name
(`--freeze not_a_group`), `--freeze all`, and an `--only`/`--freeze` pair leaving
nothing free (`--only tempo --freeze tempo`).

### The run

```bash
build/tools/tuner --data .tuning/selfplay_v2.tsv --out .tuning/tuned_v2_frozen.hpp \
    --threads 12 --epochs 5000 --freeze tempo,piece_placement
```

```
11003693 positions, 119998 games, 9903305 train, 1100388 validation (10.0002%), 827 parameters, group all, freeze tempo,piece_placement, 817 free
fitted K = 0.7624
start: train 0.122741  validation 0.122560
best: train 0.118204  validation 0.118460  (start 0.122741 / 0.122560)
```

**817 free of 827.** K = 0.7624, identical to the unfrozen fit — K is fitted on
the *starting* parameters over the train rows, and those are the same rows and the
same starting point, so the agreement is a consistency check and not a
coincidence. Best held out **0.118460 at epoch 5000**, the last one; no refusal
warning. Emitted header sha256
`bb6c68b5e5dbb81868a64117201494a305d4a949add412c9c254d6645700ce54`. Wall
14:34:37 to 15:13:26, **2329 s**, against the unfrozen fit's 2082 s on the same
threads — this run shared the machine with the compiles and corpus scans below
for its first quarter, so the difference is contention and not the freeze.

The curve, held out: 0.119156 at 100 epochs, 0.118638 at 1000, 0.118526 at 2000,
0.118479 at 3000, 0.118464 at 4000, 0.118460 at 4900 and unchanged at 5000. The
last 1000 epochs bought **4e-06**, so the budget is at the flat end of the curve
exactly as the unfrozen fit's was, with the difference that the best checkpoint
here is the final epoch rather than 4800. Against a total gain of 0.004100 —
**3.35 % of the starting held-out error**, against the unfrozen fit's 4.24 % —
83.0 % of it is inside the first 100 epochs and 95.7 % inside the first 1000.

**`tempo_mg`, `tempo_eg` and all eight `piece_placement` weights are zero in the
emitted header and zero in the applied source**, confirmed by reading
`src/evaluation.cpp:199-200,582-583` after the paste. The verifier that says so is
non-vacuous: with `tempo_mg` set to 7 and `piece_placement_eg[2]` to 5 in a copy of
the header it reported 825 of 827 identical and named both by name.

**This held-out figure ranks nothing.** 0.118460 against the unfrozen fit's
0.117359 is a comparison between different free-parameter sets, and against S028's
0.108043 a comparison across corpora with K refitted per corpus. Only the SPRT
decides.

### The paste, and the three guards this time

Applied mechanically: **827 parameters compared, 827 identical** to the emitted
header, `git diff` values only, 97 lines each way in `src/eval_tables.hpp` and
8/10 in `src/evaluation.cpp`. `ctest --test-dir build -L fast` then **9 of 12**,
and the three are not the same three.

| guard | outcome |
|---|---|
| `test_eval_model`, `CHECK(tempo_unfitted)` | **did not fire.** `tempo_mg == tempo_eg == 0`, so the tempo division truncates `0 / 24` exactly, three divisions can round, the bound stays 3 x 23/24 = **2.875** and the tolerance stays 3. This is what freezing tempo was for |
| `test_eval_model`, the four pinned FENs | **fired**, harder than before: 0.916667 / 1.250000 / 0.250000 / 0.666667, all four under the `> 2.0` threshold and the worst 1.250000 short of 2.8. Re-targeted, below |
| `test_evaluation`, `POSITIONAL_ROOM` | **fired.** `REQUIRE( 432 <= 150 )`, white queen. **Not touched, and this is where the step stops** |

### Guard 2, re-measured and re-targeted

A residual belongs to the weights and not to the position, which is the whole
reason the guard had to be re-pinned rather than carried forward. Measured with a
scratch program of the same construction as the test case — same
`starting_params`, same `model_score_white`, engine against model — over **all
11003693 rows** of `.tuning/selfplay_v2.tsv` at the new constants:

```
rows 11003693  past 2.0: 175415 (1.594%)  past 2.875: 0  worst 2.875000
```

Validated before it was trusted: at the constants shipping today the same program
returns the four residuals the test currently asserts, **2.875 / 2.333333 / 2.25 /
2.125**, and 3296 of the first 200000 rows past 2.0 against the 2026-08-13 audit's
3446 of 200000 on the older corpus.

**32 positions sit exactly at the arithmetic maximum**, 69/24 = 2.875, spanning
phase 5 to phase 23 with both sides to move. Four were pinned, all four at that
maximum, chosen to span the taper rather than to cluster at one end:

| residual | x24 | phase | position |
|---|---|---|---|
| 2.875000 | 69 | 5 | `8/5R2/2nk2K1/8/1r3P2/8/8/8 b - - 3 54` |
| 2.875000 | 69 | 11 | `r3r1k1/p5p1/1p1Pb2p/5P2/8/7P/P2N1B2/bN3RK1 w - - 0 24` |
| 2.875000 | 69 | 17 | `5rk1/5ppp/p1b4q/8/2QP2P1/5N1n/PP3P2/4RR1K w - - 2 26` |
| 2.875000 | 69 | 23 | `r1bq1rk1/1ppp1p1p/n4np1/pNP3N1/4p2P/4P3/PBPP1PP1/R2QKB1R b KQ - 5 9` |

**The thresholds are unchanged at `> 2.0` and `worst > 2.8`, and the arithmetic is
now written into the test.** Each of the three divisions loses at most 23/24, so
two of them together cannot reach past 46/24 = 1.9167: a residual over
2.0 = 48/24 is *proof* that all three truncated on that position, which is what
"exercises the bound" means. That is why 2.0 is the right threshold and not a
round number, and it is why a position agreeing for free cannot pad the set. All
four pinned positions sit at 69/24, all three divisions losing their maximum at
once.

Two floating-point notes, recorded rather than smoothed over. The scan counts 10
rows of 11003693 whose residual exceeds 2.875 by about 1e-15; that is the double
accumulation in the model, not a violation of the integer bound, and every one of
the 10 prints as 2.875000. And the exact-arithmetic bound held over the whole
corpus: no row reached 2.9.

**Verified in both directions.** With the paste applied, `test_eval_model` is
**17 of 17, 2604 assertions**. With the paste reverted and the re-target left in
place, the same four fall to 1.625 / 0.375 / 1.458333 / 0.166667 and the case
fails with `worst pinned difference 1.625000, short of 2.875` — the mirror image
of what the old four did under the new weights. The re-target is therefore pinned
to these constants and cannot be committed without them.

The prepared edit is `.tuning/s065_guard2_retarget.patch`, 92 lines, and the four
FENs above are the durable record of it.

### Guard 3, in full — this is where the step stops

`test_evaluation:183`, "removing a piece moves the score", white queen:

```
REQUIRE( std::abs((with - without) - test.material) <= POSITIONAL_ROOM )
values: REQUIRE( 432 <= 150 )
```

A white queen on d1 against bare kings evaluates **716**. Her own fitted material
value is `QUEEN` = **1148**. The gap is **432**, against `POSITIONAL_ROOM` = 150.
The other two cases pass: rook 509 against `ROOK` 487, gap 22; pawn 125 against
`PAWN` 95, gap 30.

Decomposed at phase 4, mg weight 4/24 and eg weight 20/24, every term computed by
a second implementation of `evaluate()` and then confirmed against the engine:

| term | weights | contribution |
|---|---|---|
| material | `QUEEN` 1148 | +1148 |
| piece-square, tapered | `psqt_mg[queen][d1]` **+326**, `psqt_eg[queen][d1]` **-485**; (326 x 4 + -485 x 20) / 24 = -8396 / 24 | **-349** |
| mobility, tapered | `mobility_mg[queen]` 4, `mobility_eg[queen]` **-9**, over 17 squares; (4 x 17 x 4 + -9 x 17 x 20) / 24 = -2788 / 24 | **-116** |
| king safety, tapered | the queen bears on d7 and d8 in Black's zone, so `KS_QUEEN_ATTACKERS` 35 / -57 and 2 x `KS_ZONE_ATTACKS` -30 / 11 enter with Black's sign; 800 / 24 | **+33** |
| `LAZY_EVAL_MARGIN` clamp | mobility + safety = -83, inside +/-150 | not binding |

1148 - 349 - 83 = **716**. The dominant term is `psqt_eg[queen][d1]` = -485 at
endgame weight 20/24.

**What constrains that square.** One pass over all 11003693 rows: mean phase
13.0564, `phase <= 4` on 2119786 rows (19.2643 %), `phase <= 8` on 4341585
(39.4557 %), a queen anywhere on 5512057 (50.0928 %) — and **a queen at
`phase <= 4` on 9786 rows, 0.0889 %**. Identical to the unfrozen fit's figures,
because coverage is a property of the corpus and not of the weights. `phase <= 4`
with a queen is one queen and no other piece.

**One measured fact about what the guard constrains, offered as data.**
`tools/tuner.cpp:26-30` documents the parameterisation as degenerate: adding a
constant to every square of `psqt_mg[t]` and `psqt_eg[t]` and subtracting it from
`piece_value[t]` is the same evaluation. Measured on this weight set with 432
added to `QUEEN` and 432 subtracted from all 128 queen squares: **all seven anchor
positions evaluate to exactly the same numbers** — 125, 211, 279, 509 (cheap 524),
716, 0, 224 — while `QUEEN` reads 1580 and the guard's residual becomes 864. In
the other direction, `QUEEN` 716 with 432 added to the queen's squares, six of the
seven are unchanged and the queen on d1 moves by **1**, 716 to 715, because that
position's tapered numerator crosses zero and truncation towards zero is not
translation-invariant across it; the guard's residual becomes 1. So a change worth
at most one centipawn of evaluation moves this residual from 432 to 1. The guard
therefore constrains a direction the fit is documented as not determining.

That is a design question about how much positional room the evaluation
legitimately needs and it is the owner's, so nothing here acts on it.
`POSITIONAL_ROOM` is untouched, the property is not widened, and the paste is
reverted for the second time. The tree is green at the old constants.

### The value anchors, re-derived

Same procedure as the unfrozen fit: a second implementation of `evaluate()` in
`.tuning/anchors.py`, written from `src/evaluation.cpp`'s specification with every
feature count hand-derived from the placement. It reproduces **8 of 8** of the
pinned values on the weights the engine ships — 88, 240, 325, 530 with its cheap
524, 1101, 0, 266 — which is what makes it independent of the thing it is
anchoring.

Against the frozen fit it gives 125, 211, 279, 509 (cheap 524), 716, **0**, 224.
Every one of the seven is exactly what the engine printed with the paste applied,
including the four `test_search` failures it did not derive: 530 -> 509 at
`:511`, 266 -> 224 at `:568`, -491 -> -446 at `:603` and -491 -> **-537** at
`:971`. The bare-kings case is **0** rather than the unfrozen fit's 21, because
tempo is frozen and there is nothing left once the board terms cancel.

### Largest weight movements, against a2f0065

`QUEEN` 1067 -> 1148, `PAWN` 87 -> 95, `KNIGHT` 366 -> 327, `BISHOP` 339 -> 306,
`ROOK` 505 -> 487. The queen's two tables again move in opposite directions,
`psqt_mg[queen]` by a mean of **+264.9** and `psqt_eg[queen]` by **-321.7**, with
`mobility_eg[queen]` 3 -> **-9**. The largest single squares are the king's
middlegame table — h7 -314 -> **+1152**, h8 55 -> 888, c8 608 -> -202 — where 64
of 64 squares moved; `psqt_eg[queen] g2` -120 -> -591 is the largest outside it.
`passed_pawn_eg` moved furthest of the small arrays, `-15 -4 18 28 25 8` ->
`19 22 46 69 93 25`.

## The SPRT, when there is something to run

```bash
REF=a2f0065 CONCURRENCY=12 nohup ./fastchess.sh > .tuning/sprt_s065_fit.log 2>&1 &
```

`a2f0065` is the constants shipping today, verified: `git diff a2f0065 HEAD -- src/`
is empty, and the two commits since it touch documents only. Detached, with a
watcher that outlives the turn (AGENTS.md section 0). Nothing is kept without it
and a verdict of zero is recorded as zero.

**There is still nothing to run it on.** Both fits are reverted — the first
because three guards fired, the second because one did, and that one is
`POSITIONAL_ROOM`, which is not the agent's to re-derive.
author:    Maksym Bodnar
