# chesso on the CCRL Blitz scale, five engines and three families

S088. Measured 2026-08-18. DEC-072, DEC-075, DEC-076, DEC-077.
Supersedes `rating_2026-08-18_ccrl_blitz.md` (S087) as the current figure; that
file is not edited and remains the record of the four-engine measurement.

## The number

**chesso ~= 2559 on the CCRL Blitz scale, 95 % +/-25, and the result is SOFT.**

Soft is the procedure's own verdict, not a hedge: the rating must be stable to
within ~30 Elo under the choice of anchor, and here it moves **121.8**.

| anchor | its CCRL Blitz | chesso solves to | 95 % |
|---|---|---|---|
| Blunder 7.1.0 | 2389 +/-18 | 2533.8 | +/-24.8 |
| Leorik 2.1 | 2568 +/-18 | **2476.7** | +/-24.0 |
| Blunder 8.5.5 | 2664 +/-11 | 2586.0 | +/-23.7 |
| Stash v21.0 | 2713 +/-14 | 2597.6 | +/-24.4 |
| Leorik 2.4 | 2829 +/-11 | 2598.5 | +/-27.7 |

**All five: mean 2558.5, spread 121.8. Without Leorik 2.1: mean 2579.0, spread
64.7.** The quoted figure is the five-anchor one -- the lower number carrying the
larger spread. Dropping an inconvenient reference is how a measurement gets
talked into a nicer answer, and 64.7 Elo means the other four do not agree
either. DEC-077 records the choice and what was rejected.

## S088's premise was wrong, and that is the finding

The step existed to arbitrate S087's 83.1 Elo spread: "two families cannot
arbitrate a disagreement between two families; a third can." A third family did
not close it -- the spread went **up**, from 83.1 over four anchors to 121.8 over
five.

What it bought instead is the *shape*. Set Leorik 2.1 aside and the solved rating
rises monotonically with the anchor's own rating and flattens at the top:

| anchor CCRL | 2389 | 2664 | 2713 | 2829 |
|---|---|---|---|---|
| chesso solves to | 2533.8 | 2586.0 | 2597.6 | 2598.5 |

That is **compression, not scatter**. Across a CCRL span of 440 Elo the measured
differences span **375.3**, a ratio of **0.853**. S087 excluded a scale artifact
on the grounds that its two extreme anchors agreed to 16 Elo; with 668 games a
pairing and a fifth rung they disagree by 64.7, so that argument no longer holds.

**No reference set fixes this.** The largest uncounted term is the time control:
CCRL Blitz is stated on the list as "equivalent to 2'+1" on an Intel i7-4770K"
and this ran at `10+0.2`. A step that wants the spread inside 30 has to attack
that, not add a sixth rung.

## chesso has not moved

`src/`, `tests/`, `CMakeLists.txt` and `cmake/` are **byte-identical** between
`a9f2b33` -- S087's rated commit -- and `c2f1c43`. `git diff --stat` empty over
all four. S087's **2570** sits between this run's 2559 and 2579.

**The instrument changed; the engine did not.** That is the correct outcome for a
step whose only product was a reference engine.

## Conditions

| | |
|---|---|
| games | **3340**, one run, 334 rounds x 5 pairings x 2 |
| time control | **`10+0.2`** -- *not* CCRL's, see the caveats |
| hash | 64 MB, every engine |
| threads | 1, every engine, by construction rather than by option |
| book | `books/8moves_v3.pgn`, `order=random`, colours reversed per opening |
| **concurrency** | **6**, DEC-075 decision 3 as amended by DEC-076 |
| adjudication | `-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400` |
| machine | i7-8700K, 6 cores / 12 threads, `g++ 13.3` (DEC-049) |
| solver | `ordo 1.2.6`, `-s 1000 -F 95 -W -D` |
| wall time | **5 h 02 m 49 s** |
| commit | `c2f1c43`, `src/` last touched at `764c3d8` |

**Concurrency 6 is this figure only.** DEC-075 puts every later run at 12. The
two are not expected to differ -- see the open risk below -- but that is an
expectation, not a measurement.

## Terminations, and a tolerated forfeit

2807 `adjudication`, 532 `normal`, **1 `time forfeit`**.

| engine | games | forfeits | rate |
|---|---|---|---|
| Blunder 7.1.0 | 668 | 0 | 0.00 % |
| Blunder 8.5.5 | 668 | 0 | 0.00 % |
| Leorik 2.1 | 668 | 0 | 0.00 % |
| Leorik 2.4 | 668 | 0 | 0.00 % |
| **Stash v21.0** | 668 | **1** | **0.15 %** |
| chesso | 3340 | 0 | 0.00 % |

Inside the 1 %-per-engine threshold DEC-075 sets, so the run stands. chesso
gained **1 point** from it.

**That forfeit is a hang, not a thin time margin, and the distinction is the
reason the overruns are printed.** Round 624, Stash as White, **274 plies**,
1 m 35 s of normal play at ~0.2 s a move, and then at move 138 -- in a repetition
dance, `135. Ke4 Rh4+ 136. Ke3 Rh3+ 137. Ke4 Rh4+`, both engines reporting `0.00`
at depth 39 and 47 -- Stash **hung for 25360 ms**. It is not the DEC-072 startup
deadlock, which fires at the first `go` and never plays a move.

**Concurrency was never the cause of Stash's forfeits.** 3 of 668 at concurrency
12 (0.45 %) against 1 of 668 at concurrency 6 (0.15 %). DEC-073 dropped the
concurrency to fix this; DEC-075 put it back.

## The reference set

| engine | `id name` | CCRL entry | tag | origin | md5 |
|---|---|---|---|---|---|
| Blunder 7.1.0 | `Blunder 7.1.0` | Blunder 7.1.0 | `v7.1.0` | built from source | `294d3252a337c7b3814d169227de7082` |
| Leorik 2.1 | `Leorik 2.1` | Leorik 2.1 | `2.1` | official `linux-x64` release | `8300612936009ae7298aa1e59e6c3c7a` |
| Blunder 8.5.5 | `Blunder 8.5.5` | Blunder 8.5.5 | `v8.5.5` | built from source | `e354234b0929cf9d8f2b42b149509065` |
| **Stash v21.0** | **`Stash v21.0`** | **Stash 21.0** | **`v21.0`** | **built from source, PGO** | **`5117fbc26f9eb0b91480b8328985f435`** |
| Leorik 2.4 | `Leorik 2.4` | Leorik 2.4 | `2.4` | official `linux-x64` release | `8e53f8903322383a831e1e0a8a8630bc` |

**Five rungs, three families, three authors, three languages** (Go, C#, C),
spanning 2389 to 2829 with chesso near the middle. Every anchor read from
`rating_list_all.html` on 2026-08-18 by `tools/ccrl_rating.py` at run time,
nothing hardcoded. List header: "Computed on August 15, 2026 with Bayeselo based
on 2'074'932 games".

Three things are unlike the other rows and are recorded at the row in
`references.tsv`: Stash's `uci_name` and `ccrl_name` differ (`Stash v21.0` vs
`Stash 21.0`); its md5 is an identity record only, because `utils/unix_build.sh`
is a two-pass PGO build whose counters vary run to run; and it carries a startup
deadlock that fastchess's handshake cannot reach (0 hangs in 240 GUI-paced
starts at 12-way concurrency, against 5/5 from a shell pipe).

## chesso's score against each reference

668 games against each, colours reversed:

| reference | CCRL Blitz | chesso score | implied diff |
|---|---|---|---|
| Leorik 2.4 | 2829 | 21.2 % | -230.5 |
| Stash v21.0 | 2713 | 34.1 % | -115.4 |
| Blunder 8.5.5 | 2664 | 39.1 % | -78.0 |
| Leorik 2.1 | 2568 | 37.3 % | -91.3 |
| Blunder 7.1.0 | 2389 | 69.5 % | +144.8 |

Overall 40.2 % over 3340 games, +993 =702 -1645.

## An open risk, with numbers, for the next run

| opponent | CCRL | S087, 4 engines, c12 | S088 voided, 5 engines, c12 | S088 valid, 5 engines, c6 |
|---|---|---|---|---|
| Leorik 2.4 | 2829 | 19.2 % | 22.1 % | 21.2 % |
| Blunder 8.5.5 | 2664 | **37.6 %** | **44.3 %** | **39.1 %** |
| Leorik 2.1 | 2568 | 39.4 % | 38.5 % | 37.3 % |
| Blunder 7.1.0 | 2389 | 72.5 % | 71.0 % | 69.5 % |

**The five-engine concurrency-12 run is the outlier of the three**, by 5 to 7
points on Blunder 8.5.5 alone. Stash is a PGO C engine at 3.76 Mnps sharing a
12-way pool with a .NET engine and two Go engines, so more contention at 12 is a
plausible mechanism, and three single runs cannot establish it.

DEC-075 puts every later run at concurrency 12. **The next rating run is the
test**: if it reproduces ~44 % against Blunder 8.5.5, the pool composition is
real and DEC-075 needs revisiting.

## Caveats, in order of how much they could move the number

1. **The time control is not CCRL's.** "Equivalent to 2'+1" on an i7-4770K"
   against `10+0.2` here. Engines do not scale identically with time, and this
   is the leading candidate for the 0.853 compression above.
2. **The anchor spread is 121.8 Elo**, four times the ~30 the procedure allows.
3. **The +/-25 does not include the anchors' own uncertainty** (+/-11 to +/-18 on
   the list), nor anything for items 1 and 2. The true uncertainty is wider.
4. **Ordo's intervals are trinomial** and are never comparable with the nElo
   figures the SPRT verdicts here report under `model=normalized`.
5. **CCRL plays with a general book to 12 moves and up to 6-piece endgame
   tables.** This ran on `8moves_v3.pgn` with no tablebases for anybody.
6. **Concurrency 6 for this figure, 12 for every later one.** See the open risk.

## Against the criteria as set

| criterion | verdict |
|---|---|
| >= 3 engine families and >= 5 rungs | **met** -- 3 families, 5 rungs |
| anchors read from the list at run time | **met** -- `tools/ccrl_rating.py`, nothing hardcoded |
| added engine built and recorded with tag, source, md5 | **met** -- DEC-069, `references.tsv` |
| added engine inside the 10-90 % band | **met** -- 30.9 % bracketing, 34.1 % rated |
| 95 % interval +/-30 or tighter | **met** -- +/-23.7 to +/-27.7 |
| forfeits at or below 1 % of an engine's own games | **met** -- worst 0.15 % |
| spread reported, dissenter named, both figures given, choice recorded | **met** -- and the spread criterion itself **FAILED at 121.8 Elo**, reported soft |

## Reproduce

```bash
./rating.sh --bracket                                   # 340 games, brackets the set
./rating.sh                                             # 3340 games, ~2 h 30 m at concurrency 12
adocs/data/S088_solve.sh <games.pgn> [outdir]           # the sweep, if rating.sh voided the run
tools/forfeit_report.py <games.pgn> [--max-pct 1.0]     # the per-engine forfeit rate alone
```

Evidence in this directory: `S088_bracket.pgn`, `S088_rated_c6.pgn` (this
figure), `S088_rated_c12_INVALID.pgn` with its `_summary.txt` (the voided run,
kept because it is the only measurement of what the pool composition does), and
`S088_solve.sh`.
