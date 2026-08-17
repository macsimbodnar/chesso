# Chesso — user manual

For running the engine. To build or change it, see `DEV_MANUAL.md`.

Chesso is a UCI engine. It has no graphical interface of its own: point a UCI
GUI at the binary, or type the protocol at it directly.

## Install

Build it (see `DEV_MANUAL.md`) and use `build/src/chesso`. There is no installer
and no packaged release.

In a GUI, add a new engine and give it the path to that binary. In a shell:

```
$ ./build/src/chesso
uci
id name Chesso
id author MazerFaker
option name Use Book type check default false
option name Hash type spin default 16 min 1 max 4096
option name Threads type spin default 1 min 1 max 1
uciok
```

## Options

| name | type | default | range | effect |
|---|---|---|---|---|
| `Use Book` | check | `false` | — | play from the built-in opening book when the position is in it. Only `true` and `false` are recognised; any other value leaves the setting unchanged |
| `Hash` | spin | 16 | 1 to 4096 | transposition table size in MB, clamped into range. Not honoured exactly — see known bugs. A non-numeric value is ignored with a warning in the log |
| `Threads` | spin | 1 | 1 to 1 | present so GUIs that insist on setting it do not fail. **The search is single-threaded**; any value other than `1` is ignored with a warning in the log |

Set them the usual way:

```
setoption name Hash value 256
setoption name Use Book value true
```

Those three are the whole option surface of the engine you get from
`cmake --build build`. There is a second, separate build with more of them.

### The tune build, which is not the release binary

A build configured `-DCHESSO_TUNE=ON` exposes every search parameter as a spin
option so an external tuner can set it without recompiling. **It is a
measurement harness and not the engine you should play with.** In the release
binary each of these is a constant the compiler folds into the instruction that
uses it; in the tune build it is a variable the search has to load, and that
costs time no fixed-depth node count can show you. Every strength measurement
this project records is taken on the release build.

Build it beside the normal one and it will not disturb it:

```
cmake -S . -B build-tune -DCMAKE_BUILD_TYPE=Release -DCHESSO_TUNE=ON
cmake --build build-tune -j12
```

Set a parameter exactly like any other option, before or between searches:

```
setoption name RfpMargin value 120
setoption name LmrDivisor value 210
```

A name that is not in the table is ignored, like any unknown option. A name that
is, with a value outside the range below, is **refused and left unchanged** —
not clamped — with the reason written to the log. Sent with no `setoption` at
all, the tune build searches exactly what the release build searches.

| name | default | range | effect |
|---|---|---|---|
| `OrderHistoryMax` | 600000 | 0 to 899999 | ceiling on an accumulated history score. The upper bound keeps it under a killer move's 900000, which is what the ceiling is for |
| `MaxQsearchDepth` | 8 | 1 to 64 | how many plies quiescence may keep going on its own before it returns its static score |
| `RfpMargin` | 75 | 0 to 2000 | reverse futility pruning: centipawns per remaining ply the opponent is assumed able to claw back |
| `RfpMaxDepth` | 6 | 0 to 63 | the deepest node reverse futility pruning is applied at. 0 switches it off |
| `RfpMinPly` | 3 | 0 to 63 | the shallowest ply reverse futility pruning is applied at. The top of the tree is searched properly |
| `NullMoveBase` | 2 | 0 to 16 | the constant part of the null move reduction |
| `NullMoveDivisor` | 6 | 1 to 64 | the depth-dependent part: the reduction is `NullMoveBase + depth / NullMoveDivisor` |
| `LmrBase` | 75 | 0 to 400 | late move reduction, the constant term of the log fit, in hundredths. 75 is 0.75 |
| `LmrDivisor` | 225 | 1 to 2000 | late move reduction, the divisor of the log term, in hundredths. 225 is 2.25 |
| `LazyEvalMargin` | 150 | 0 to 2000 | the largest correction the lazy evaluation's expensive terms are allowed to apply |

## Commands

Standard UCI: `uci`, `debug`, `isready`, `setoption`, `register`, `ucinewgame`,
`position`, `go`, `stop`, `ponderhit`, `quit`.

`go` understands `depth`, `movetime`, `nodes`, `wtime`, `btime`, `winc`, `binc`,
`movestogo` and `infinite`.

`go mate`, `go searchmoves` and `go ponder` are **accepted and ignored**, with a
warning in the log. UCI says to ignore what is not implemented, and the rest of
the line still carries the time control. A GUI asking for a mate search gets a
normal search.

`position` takes `startpos` or `fen <six fields>`, either of them optionally
followed by `moves`. A move in the `moves` list that does not parse or is not
legal is skipped with a warning, and the rest of the list is still applied.

`ponderhit` is accepted; pondering itself is not implemented.

`debug on` and `debug off` are accepted. The flag is recorded and currently
changes nothing.

### What a search prints

One `info` line per finished iteration, then one `bestmove`. An iteration that
was cut short reports too, if it already has a line to play.

```
info score cp 80 time 10 depth 6 nodes 77104 nps 7165799 pv c3d5 e7d8 c2c3 f8e8 h2h3 g4f3
bestmove c3d5
```

| field | meaning |
|---|---|
| `score cp N` | centipawns, from the point of view of the side to move. `score mate N` instead when a mate is found, `N` in moves |
| `time` | milliseconds since this search started, not since this iteration started |
| `depth` | the deepest iteration that **finished**. An iteration cut short repeats the previous depth and the previous score, because neither of an unfinished iteration's own figures means anything |
| `nodes` | nodes searched in this search, counting every iteration. It never falls between lines. Subtract two successive lines for one iteration's own count |
| `nps` | `nodes` over `time`, both for the whole search |
| `pv` | the line the engine will play, and `bestmove` is its first move |

`bestmove 0000` means no legal move at all. A search cut off before it finished
even one move still answers with a legal one.

Until 2026-08-13 `nodes` was the current iteration's count and `time` the
current iteration's duration, so the count fell between depths and the
implied rate was several times too low. `nps` was not reported at all.

### Non-standard commands

Convenience only, not part of UCI. A GUI never sends these.

| command | what |
|---|---|
| `pb` | print the board |
| `fen` | print the current position as FEN |
| `help` | list commands |
| `test` | run the built-in self-test over seven positions. `test <n>` sets the depth, default 6 |
| `clean-tt` | clear the transposition table |

### Non-standard position shortcuts

`position` also accepts these names in place of `startpos` or `fen`. They are
the fixed positions the engine is developed and self-tested against, listed
here as the FEN each one loads. A GUI never sends them.

| shortcut | FEN |
|---|---|
| `empty` | `8/8/8/8/8/8/8/8 b - - 0 1` |
| `tricky` | `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1` |
| `killer` | `rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1` |
| `cmk` | `r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9` |
| `fine70` | `8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1` |
| `mate2w` | `4k3/Q7/8/4K3/8/8/8/8 w - - 0 1` |
| `mate2b` | `4K3/q7/8/4k3/8/8/8/8 b - - 0 1` |
| `3frep` | `2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 1` |

## Known bugs and limitations

- **Single-threaded.** `Threads` exists but cannot be set above 1.
- **No pondering.** `go ponder` is ignored and `ponderhit` does nothing useful.
- **No mate search.** `go mate N` is ignored and becomes a normal search.
- **No aspiration windows, forward futility pruning, razoring or singular
  extensions.** These are planned, not present; see `adocs/plan.md`. *Reverse*
  futility pruning is present since S033 (2026-08-16): a node whose static score
  is a margin clear of beta is not searched. It cannot see a mate — that is what
  a static score is — so it is bounded to depth 6 and to ply 3 and below.
- **The reported score is unreliable in both directions, and worst in pawn
  endgames.** Measured over 5582 moves in 98 games against Stockfish at 3000000
  nodes, after the evaluation constants were fitted (S028, 2026-08-11). Chesso's
  own score minus Stockfish's, mean and median: opening −1.5 / +6, early
  middlegame −24.3 / 0, late middlegame −40.3 / −4, endgame −3.5 / −1, pawn
  endgame **+204.3 / +227**. The typical move is close; the tail is not, and the
  90th percentile of the disagreement runs +111 to +423 depending on phase.
  Treat a reported score as an estimate with a wide tail, and treat one in a
  pawn endgame with real suspicion.
  Until 2026-08-11 this entry said the engine was optimistic in every phase, by
  +39 to +100 cp. That was measured on the hand-written constants and it no
  longer holds: fitting them removed the systematic optimism everywhere except
  pawn endgames, where it grew — on 22 moves, which is too few to be more than
  a warning. The constants moved again the same day, when mobility was added and
  all 781 of them were refitted (S034, +28.46 Elo), so even the figures above
  predate the current evaluation. They predate it by one more generation now:
  all 827 were refitted again on 2026-08-16 from a second corpus, generated by
  the stronger engine the earlier fits produced (S065, +21.10 Elo). The
  phase-by-phase disagreement above has **not** been re-measured against these
  constants. The shape of the warning is what to keep: the typical move is
  close, the tail is wide, and a pawn endgame score deserves suspicion.
- **`Hash` is not honoured exactly.** The value is clamped to 1–4096 MB, then the
  *entry count* is rounded down to a power of two so probing can mask instead of
  divide, so the table is usually smaller than asked for. If the allocation
  fails the engine halves the count and retries, and if every size fails it runs
  **without a transposition table** rather than refusing to play. Nothing is
  reported back over UCI in any of these cases; the log records them.
