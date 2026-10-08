# S271 replay: what a new base FEN's table clear costs

The investigation the owner asked for on `2026-10-08_performance-F04`, run
2026-10-08 on the MacBook (M1) at `29348c5`. Node counts only, which carry
between machines; no time-based figure is taken here.

## What was run

200 games of `adocs/data/S198_calibration.pgn` (self-play at 8+0.08), the
side to move at the book position searched at every second ply of the first
120: **8653 positions**, each searched to a fixed depth in one engine process
per game sequence, `ucinewgame` at each game's start. Four send modes:

| tag | engine | `position` line sent |
|---|---|---|
| `h_base` | `29348c5` | book FEN + every move so far (what fastchess sends, logged 2026-10-08) |
| `h_bare` | `29348c5` | the current FEN alone |
| `h_irrev` | `29348c5` | the FEN after the last capture or pawn move + the moves since |
| `k_bare` | `29348c5` + `S271_keep_table.diff` | the current FEN alone |

```
python3 adocs/data/S271_replay.py ENGINE adocs/data/S198_calibration.pgn 200 DEPTH HASH MODE r_<h|k>_<mode>_d<DEPTH>_h<HASH>.json
python3 adocs/data/S271_replay_read.py <directory holding the json files>
```

Run with `~/.venv/chess/bin/python` (python-chess). The json outputs stay with
the run (DATA rule); this file is the reading.

## Reading

Total nodes against `h_base`, and the median per-position ratio. The Elo
column is a **conversion, not a verdict**: extra nodes to the same depth read
as the same share of thinking time lost, priced at S219's measured +174.85
Elo per doubling of time (self-play, 4+0.04 against 8+0.08).

| depth, Hash | `h_base` nodes | `h_bare` | `h_irrev` | `k_bare` |
|---|---|---|---|---|
| 12, 16 MB | 1088825787 | x1.218 (median x1.294), ~50 Elo | x1.119 (x1.123), ~28 | x1.001 (x1.000), ~0 |
| 12, 128 MB | 1072331563 | x1.241 (x1.313), ~54 | x1.128 (x1.133), ~30 | x1.000 (x1.000), ~0 |
| 14, 16 MB | 2463842776 | x1.236 (x1.335), ~53 | x1.136 (x1.148), ~32 | x1.000 (x1.000), ~0 |
| 14, 128 MB | 2404949104 | x1.268 (x1.361), ~60 | x1.144 (x1.154), ~34 | x1.000 (x1.000), ~0 |

Best move differing from `h_base`: `h_bare` 2626 to 2792 of 8653, `h_irrev`
2506 to 2728, `k_bare` **11 to 21** -- the residue of the repetition history a
bare FEN cannot carry, which no engine-side table rule recovers.

- The cost grows with depth and with hash: the longer the control, the more a
  cold table loses.
- Keeping the table on a new base recovers all of it: `k_bare` reads within
  0.1 % of `h_base` in every cell.
- Not in the node figures: the clear itself runs inside `position`, before
  `go`, so it is spent on the engine's clock -- milliseconds at 16 to 128 MB,
  about 0.6 s at 4096 MB on this 8 GB machine (the audit's F04 timing).

## Who sends a changing base

- **fastchess**: `ucinewgame`, then the book FEN + every move, the base fixed
  for the game (logged through a stdin-tee wrapper, two games, 2026-10-08).
- **python-chess** 1.11.2 (`chess.engine`, so lichess-bot): the root FEN +
  the move stack; a bare FEN only for a board built from a FEN with no stack,
  or a stack holding a null move. `ucinewgame` on the first game and on a
  changed `game` key.
- Graphical clients (Arena, ChessBase, Cute Chess's GUI): not verified.
  Forum reports describe UCI clients that send the FEN after the last
  irreversible move; none was observed here.

## One position, three engines

`S271_table_probe.py`: search
`r1bq1rk1/pp2bppp/2n2n2/3p4/3P4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 10` to depth 16
at Hash 64, then the board two plies down its PV as a bare FEN, with and
without `ucinewgame` between.

| engine | bare FEN after the first search | with `ucinewgame` between |
|---|---|---|
| `29348c5` | 1413044 | 1413044 |
| `29348c5` + `S271_keep_table.diff` | 520723 | 1413044 |
| Stockfish binary (tool use, DEC-016) | 136805 | 105738 |

Stockfish's two counts differ, so it carries state across a bare FEN; that
shows something persists, not that it is the table.
