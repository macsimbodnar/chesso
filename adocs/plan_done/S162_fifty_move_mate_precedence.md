id:         S162
goal:       the 100-halfmove draw is not returned when the side to move is checkmated, matching the laws of the game
accepts:    at `halfmove_clock >= 100` the draw score is returned only after establishing the side to move is not checkmated — the standard form is `halfmove >= 100` and not (in check with no legal reply); a regression position whose forced line delivers mate exactly as the clock reaches 100 is constructed and tool-verified with `stockfish` (`go depth 20`) per DEC-023 before being pinned red-first in the fast suite; bench node counts are expected identical (no bench FEN approaches clock 100) and the correctness is decided by the test, with one `--nonreg` verdict as insurance since the change does alter play in its class — a null there is an expected and acceptable outcome
touches:    src/search.cpp, tests/test_engine.cpp, adocs/specs.md,
            adocs/data/S162_clock_census.py, adocs/decisions.md, adocs/plan.md
            -- the last three are the abandoned SPRT's record, DEC-107
excludes:   the 75-move rule, repetition detection, and any draw-adjudication interplay with the match harness — this is the search's own scoring of the boundary node
decisions:  DEC-023
closes:     2026-08-22_adversarial-F03
blocks:
paused_by:
done:      Checkmate outranks the 100-halfmove draw. negamax returned DRAW_SCORE at
               halfmove_clock >= 100 before generating a move; the clock is now a draw only
               once mate is ruled out. FIDE 5.1.1, and 9.6.2 states the same exception for
               the 75-move rule explicitly.
               --
               RED, observed and recorded, at depth 1 on the tree of 2026-08-22:
               7k/6pp/8/8/8/8/8/R6K w - - 99 60 reported 'cp 0' and
               7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60 reported 'cp 448' and played g2h3 --
               discarding a mate in one it had already generated, because one capture
               resets the clock and 448 beats a draw. GREEN: both report 'mate 1', both
               play a1a8. The four preconditions per case (reachable, clock exactly 100, in
               check, zero legal replies) pass on both builds, so the case is non-vacuous
               by construction.
               --
               Neither FEN was read off a board (DEC-023). python-chess: both VALID, one
               mate in one each, child clock exactly 100 with 0 legal moves. stockfish 'go
               depth 20': Mate(+1) on both. A mated node falls through to the two sites
               that already know the distance rather than to a third copy of that
               arithmetic; the insufficient-material test below it cannot intercept one,
               enumerated over all 1700560 legal positions it calls a draw -- 0 are
               checkmate.
               --
               NO SPRT, AND THAT IS THE STEP'S ONE DEVIATION FROM ITS OWN ACCEPTS. The
               insurance --nonreg was launched (ea9ba2c vs ecd735e) and killed at 3304
               games on a census of its own PGN, owner's decision, DEC-107. Both halves of
               that census matter: 102 of 3356 games peaked at a halfmove clock of 80 or
               more, so the new is_check() and move generation do run in about 3 % of games
               and this is not dead code -- and 0 positions were checkmate at a clock of
               100 or more, at 90 or more also 0, so the behaviour change never fired in a
               played game. The run agreed: LLR 0.09, 2.9 % of the way to a bound, Elo
               -1.37 +/- 8.67, its only reachable ending the null its accepts had
               pre-declared. adocs/data/S162_clock_census.py is the count and it is
               reusable over any run's PGN.
               --
               INV-6: bench identical to ecd735e, 639228 / 3430710 / 367858 at depth 12,
               c3d5 / e2a6 / d7c8q. Fast suite 20/20, clang-format clean. DEV_MANUAL.md and
               MANUAL.md checked -- neither documents the fifty-move rule, no change needed;
               README.md is owner-written and untouched.
               --
               Two things found by running this step, both filed rather than folded in:
               S166, the documented stockfish pipe invocation answers without searching
               ('quit' aborts the search, nodes 0 on the same line) and returned 'cp 0'
               with a non-mating move for one of these very positions; and the harness note
               that killing fastchess.sh leaves the fastchess process orphaned under init
               with no terminal marker written, which is what exited this run's watcher on
               pid death instead of on a marker.
               --
               touches amended mid-step to add the census script, decisions.md and plan.md
               -- the record of the abandoned run.

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
