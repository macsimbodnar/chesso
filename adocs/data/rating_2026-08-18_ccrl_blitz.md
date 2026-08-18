# chesso, absolute rating on the CCRL Blitz scale

S087. Measured 2026-08-18. DEC-067, DEC-068, DEC-069.

## The number

**chesso ≈ 2570 on the CCRL Blitz scale, 95 % interval ±25, and the result is
SOFT.**

Soft is not a hedge, it is the specification's own verdict: the rating must be
stable to within ~30 Elo under the choice of which reference is anchored, and
here it moves **83.1 Elo**. The full sweep, so nothing is hidden behind the
headline:

| anchor | its CCRL Blitz | chesso solves to | 95 % |
|---|---|---|---|
| Blunder 7.1.0 | 2389 ±18 | 2559.3 | ±25.5 |
| Leorik 2.1 | 2568 ±18 | **2492.5** | ±24.1 |
| Blunder 8.5.5 | 2664 ±11 | 2574.6 | ±24.4 |
| Leorik 2.4 | 2829 ±11 | 2575.6 | ±28.3 |

Three of the four agree to within **16.3 Elo** — 2559.3, 2574.6, 2575.6, whose
mean is **2569.8**. Leorik 2.1 alone dissents by about 80. The headline is the
three-anchor cluster; including Leorik 2.1 pulls the four-anchor mean to 2550.5.
Both figures are stated because dropping an inconvenient reference is how a
measurement gets talked into a nicer answer.

## What the engine was

| | |
|---|---|
| commit | `a9f2b33`, `src/` last touched at `77d7450` |
| binary | `build/src/chesso`, snapshotted before the first game |
| `id name` | `Chesso` |

## Conditions

| | |
|---|---|
| games | **2672**, two runs of 1336 combined |
| time control | **`10+0.2`** — *not* CCRL's, see the caveats |
| hash | 64 MB, every engine |
| threads | 1, every engine, by construction rather than by option |
| book | `books/8moves_v3.pgn`, `order=random`, colours reversed per opening |
| concurrency | 12, DEC-050 |
| adjudication | `-draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400` |
| machine | i7-8700K, 6 cores / 12 threads, `g++ 13.3` (DEC-049) |
| solver | `ordo 1.2.6`, `-s 1000 -F 95 -W -D` |
| wall time | 1 h 00 m + 1 h 00 m, plus two bracketing runs |

**Zero time forfeits in 2672 games**: 2258 `adjudication`, 414 `normal`, nothing
else, and no disconnect or time-loss line in either fastchess log. Any forfeit
would have invalidated the run.

## The reference engines, exactly

Neither the binaries nor their sources are in this repository; `references.tsv`
is the record and this table is a copy of it as it stood for this measurement.
Every one was identified by asking it `uci` and reading `id name`, not by its
filename — `rating.sh` refuses the run on a mismatch.

| engine | `id name` | CCRL entry | tag | origin | md5 |
|---|---|---|---|---|---|
| Blunder 7.1.0 | `Blunder 7.1.0` | Blunder 7.1.0 64-bit | `v7.1.0` | built from source, worktree off `/home/max/ws/blunder` | `294d3252a337c7b3814d169227de7082` |
| Leorik 2.1 | `Leorik 2.1` | Leorik 2.1 64-bit | `2.1` | official `linux-x64` release | `8300612936009ae7298aa1e59e6c3c7a` |
| Blunder 8.5.5 | `Blunder 8.5.5` | Blunder 8.5.5 64-bit | `v8.5.5` | built from source, worktree off `/home/max/ws/blunder` | `e354234b0929cf9d8f2b42b149509065` |
| Leorik 2.4 | `Leorik 2.4` | Leorik 2.4 64-bit | `2.4` | official `linux-x64` release | `8e53f8903322383a831e1e0a8a8630bc` |

Blunder is built here; Leorik is not, because this machine has no .NET SDK and
the agent may not add one. DEC-069.

**Their CCRL rows as read**, so the anchors are auditable without re-fetching —
rating, error, and the game count the list computed it from:

| engine | rating | error | CCRL games |
|---|---|---|---|
| Leorik 2.4 | 2829 | ±11 | 2437 |
| Blunder 8.5.5 | 2664 | ±11 | 2502 |
| Leorik 2.1 | 2568 | ±18 | **990** |
| Blunder 7.1.0 | 2389 | ±18 | 1049 |

List header: "Ponder off, General book up to 12 moves, up to 6 piece EGTB",
"Time control: Equivalent to 2'+1" on an Intel i7-4770K", "Computed on August
15, 2026 with Bayeselo based on 2'074'932 games". Read 2026-08-18.

## Tooling versions

| | |
|---|---|
| `fastchess` | `alpha 1.8.1 20260720-daa3ea2` |
| `ordo` | `1.2.6` |
| compiler | `g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`, DEC-049 |

The match, as issued by `rating.sh`:

```
fastchess -engine cmd=<snapshot> name=chesso \
  -engine cmd=... name="Blunder 7.1.0" ... \
  -tournament gauntlet -seeds 1 \
  -openings file=books/8moves_v3.pgn format=pgn order=random \
  -each tc=10+0.2 option.Hash=64 \
  -draw movenumber=40 movecount=8 score=10 -resign movecount=3 score=400 \
  -rounds 167 -games 2 -repeat -concurrency 12 -recover
```

`option.Threads` is deliberately absent: no reference engine exposes it, all
four are single-threaded by construction, and chesso's is `min 1 max 1`.
DEC-068.

## chesso's score against each reference

668 games against each, colours reversed:

| reference | CCRL Blitz | chesso score | implied diff |
|---|---|---|---|
| Leorik 2.4 | 2829 | 19.2 % | −253.4 |
| Blunder 8.5.5 | 2664 | 37.6 % | −89.4 |
| Leorik 2.1 | 2568 | 39.4 % | −75.5 |
| Blunder 7.1.0 | 2389 | 72.5 % | +170.3 |

Overall 42.2 %, +869 =515 −1288.

## The disagreement, isolated

Each solve also states what it thinks the *other* references are worth. Against
their CCRL ratings:

| reference | CCRL | this gauntlet says | difference |
|---|---|---|---|
| Leorik 2.4 | 2829 | 2828.0 (anchored on Blunder 8.5.5) | −1 |
| Blunder 8.5.5 | 2664 | 2665.0 (anchored on Leorik 2.4) | +1 |
| Blunder 7.1.0 | 2389 | 2404.3 (anchored on Blunder 8.5.5) | +15 |
| **Leorik 2.1** | **2568** | **2650.1** (anchored on Blunder 8.5.5) | **+82** |

Blunder 7.1.0, Blunder 8.5.5 and Leorik 2.4 reproduce each other's CCRL ratings
to within 16 Elo across a 440-point span. Leorik 2.1 comes out about 82 Elo
above its listed rating, consistently, whichever of the others is anchored.

**It is not sampling noise.** 668 games per pairing; the standard error on a
38 % score is about 1.9 points, and CCRL's 96-Elo gap between Leorik 2.1 and
Blunder 8.5.5 predicts a score gap of roughly 13 points where the measured gap
is 1.8 — about six standard errors.

**It is not an ordo-versus-bayeselo scale artifact either.** CCRL solves with
Bayeselo and this solves with Ordo, and the two do not share a scale
convention, so a mismatch would show up as an anchor-dependent answer. It would
show up *monotonically* across the rating range, and it does not: anchoring the
weakest reference (2389) and the strongest (2829) gives 2559.3 and 2575.6, 16
apart across 440 Elo. The dissent sits in the middle of the range, not at an
end.

Candidate explanations, none of them tested here: CCRL rates Leorik 2.1 on
**990 games** against 2502 for Blunder 8.5.5 and 2437 for Leorik 2.4; the time
controls differ (below); the books and endgame tables differ. Which of these it
is, this run cannot say.

## Caveats, in order of how much they could move the number

1. **The time control is not CCRL's.** CCRL Blitz is stated on the list as
   "Equivalent to 2'+1" on an Intel i7-4770K". This ran at `10+0.2`, which is
   far faster. Engines do not scale identically with time, so the anchors are
   imported across a time-control gap of unknown size. This is the largest
   uncounted term and it is why the result is called approximate.
2. **The anchor spread is 83.1 Elo**, above the ~30 the specification allows
   before a result is soft.
3. **The ±25 does not include the anchors' own uncertainty** — ±11 to ±18 on
   the CCRL list — nor anything for items 1 and 2. The true uncertainty is
   wider than the interval printed.
4. **Ordo's intervals are trinomial** and are never comparable with the nElo
   figures the SPRT verdicts in this repository report under
   `model=normalized`.
5. **CCRL plays with a general book to 12 moves and up to 6-piece endgame
   tables.** This ran on `8moves_v3.pgn` with no tablebases for anybody.
6. **Four references from two families.** Three authors' worth of independence
   would be better; if chesso has a property both families are weak to, no
   game count here detects it.

## Against the criteria as set

| criterion | verdict |
|---|---|
| 1. gauntlet against ≥3 rated references | **met** — four |
| 2. 95 % interval ±30 or tighter | **met** — ±24.1 to ±28.3, after doubling from 1336 to 2672 games; at 1336 the best was ±34.0 |
| 3. stable under anchor choice, <~30 Elo | **FAILED** — 83.1 Elo, so the result is reported soft, as that criterion requires |
| 4. anchors read from the list at run time | **met** — `tools/ccrl_rating.py`, list computed 2026-08-15, read 2026-08-18, nothing hardcoded |
| 5. reproducible from a script | **met** — `./rating.sh` |

## The prior was wrong, and cheaply

The starting guess was ~2000. The first reference set was chosen to bracket it —
Leorik 1.0 (2102), Blunder 5.0.0 (2017), Rustic Alpha 3.0.6 — and chesso scored
**90.4 %** against the strongest of them in 204 games and 7 m 30 s. The
bracketing gate caught it before any long run was booked, which is exactly what
it is for: at `2+1` that set would have cost a night and returned a number from
the tail of the logistic curve.

## Reproduce

```bash
./rating.sh --bracket     # ~272 games, checks the set still brackets
./rating.sh               # ~1336 games, solves and sweeps the anchors
```

Evidence in this directory: `S087_bracket1.pgn` (failed set),
`S087_bracket2.pgn` (passing set), `S087_rated1.pgn`, `S087_rated2.pgn` and the
`_h2h` and `_report` files beside them. The combined solve is the two rated PGNs
concatenated.
