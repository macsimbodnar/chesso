id:         S016
goal:       turn a game into per-move cost from Stockfish instead of reading the move list
accepts:    SAN in, per-move cost out, using the engine's own parser rather than a new dependency; a 158-ply game analysed in about 45 seconds at depth 18
touches:    tools/pgn_to_positions.cpp, tools/analyse_game.py, tools/CMakeLists.txt
excludes:   any dependency on python-chess
decisions:  DEC-008
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 66979cb, rule written in 9e4126a. README and MANUAL checked at adoption, not when this shipped.

## Why this exists

A drawn game was analysed by reading the move list. The analysis claimed the
evaluation was two pawns too optimistic before move 62; Stockfish put the
position at +196 against the engine's +1.95, **agreeing to within five
centipawns.** The real defect was the opposite one, in the ten moves after that
trade. The same analysis missed the four other moves that each cost more than a
pawn, including the largest error in the game, 12.c3 at -213.

Reading a game finds the move you were already looking for. See DEC-008 and the
CLAUDE.md section it produced.

## Usage

```bash
build/tools/pgn_to_positions < moves.txt > positions.tsv
tools/analyse_game.py positions.tsv --engine ~/.local/bin/stockfish --depth 18
```

Cost of move i is `eval[i] + eval[i+1]`, both side-to-move relative.

## The finding it produced, which is now S019

Chesso held +1.5 to +1.9 for ten moves in a position Stockfish valued at +0.25.
King-and-pawn endgame evaluation is a confirmed defect, measured by a tool.
