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
- **No aspiration windows, futility pruning, razoring or singular extensions.**
  These are planned, not present; see `adocs/plan.md`.
- **The evaluation is optimistic in every phase**, not only the endgame.
  Measured over 13522 moves in 210 games, against Stockfish at 3000000 nodes:
  chesso's own score exceeds Stockfish's by +39 cp on average in the opening,
  +77 in the early middlegame, +100 in the late middlegame, +42 in the endgame
  and +30 in pawn endgames. The tail is what to watch — the 90th percentile is
  around +350 to +400 in every phase from the early middlegame on. Treat a
  reported advantage as an upper bound.
  An earlier note here named the endgame specifically, from one king-and-pawn
  game where chesso held +1.5 to +1.9 against Stockfish's +0.25. That game is
  real and such positions exist, but it is not where most of the error is.
- **The piece-square tables are hand-written and untuned.** Tuning them is S028.
- **`Hash` is not honoured exactly.** The value is clamped to 1–4096 MB, then the
  *entry count* is rounded down to a power of two so probing can mask instead of
  divide, so the table is usually smaller than asked for. If the allocation
  fails the engine halves the count and retries, and if every size fails it runs
  **without a transposition table** rather than refusing to play. Nothing is
  reported back over UCI in any of these cases; the log records them.
