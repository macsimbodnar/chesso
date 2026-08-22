id:         S162
goal:       the 100-halfmove draw is not returned when the side to move is checkmated, matching the laws of the game
accepts:    at `halfmove_clock >= 100` the draw score is returned only after establishing the side to move is not checkmated — the standard form is `halfmove >= 100` and not (in check with no legal reply); a regression position whose forced line delivers mate exactly as the clock reaches 100 is constructed and tool-verified with `stockfish` (`go depth 20`) per DEC-023 before being pinned red-first in the fast suite; bench node counts are expected identical (no bench FEN approaches clock 100) and the correctness is decided by the test, with one `--nonreg` verdict as insurance since the change does alter play in its class — a null there is an expected and acceptable outcome
touches:    src/search.cpp, tests/test_engine.cpp, adocs/specs.md
excludes:   the 75-move rule, repetition detection, and any draw-adjudication interplay with the match harness — this is the search's own scoring of the boundary node
decisions:  DEC-023
closes:     2026-08-22_adversarial-F03
blocks:
paused_by:
done:

## Evidence

2026-08-22_adversarial-F03. `src/search.cpp:587` returns `DRAW_SCORE` at
`halfmove_clock >= 100` before move generation runs, so a node whose side to
move is checkmated with the clock at 100 scores as a draw instead of a mate.
FIDE Laws 5.1.1: checkmate ends the game immediately and takes precedence
over a 50-move claim; 9.6.2 states the same exception for the 75-move rule
explicitly. As the winner the engine can discard a win it evaluates as a
draw; as the loser it can steer into a "draw" that is a loss. Vanishingly
rare, but it is a wrong game-theoretic value in the class the prime
directive ranks first, and the fix is a few characters of missing condition.

## Cost

Tiny search change, one tool-verified regression position, one insurance
`--nonreg` run.
author:    Maksym Bodnar
