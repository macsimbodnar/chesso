id:         S147
goal:       a mate score is reported with a principal variation long enough to reach the mate it claims, so fastchess's -check-mate-pvs stops warning on a truncation
accepts:    an `info` line reporting `score mate N` carries a `pv` of at least `2N - 1` plies for a mate the engine delivers and `2|N|` plies for one it receives, and the last position of that line is checkmate; asserted over the constructed set in `adocs/data/S145_mate_set.tsv` and over the mined set in `adocs/data/S145_mined_set.tsv`, at every iteration from the first that reports a mate rather than at the last one only, because the truncation measured is a shallow-iteration effect; a run of `fastchess.sh --fast` produces no `Incomplete mating PV` line, which is the check `-check-mate-pvs` already performs on every SPRT since S145; behaviour-neutral for play, proven the S145 way -- identical node counts and identical best moves from `tools/search_bench.py`, since only what is printed changes; `tests/test_uci_surface.cpp` refreshed after MANUAL.md describes the new output; the known-bug entry S145 added to MANUAL.md removed in the same commit that removes the bug
touches:    src/chesso.cpp, src/search.cpp, tests/test_engine.cpp, tests/test_uci_surface.cpp, MANUAL.md
excludes:   changing any search behaviour, any default or any score -- the distances are already right and this step must not move them; extending the principal variation through quiescence as a search feature rather than as a reporting one, if the measurement says a table walk is enough; `go mate N`, which is a separate missing feature listed in MANUAL.md
decisions:  DEC-061
closes:
blocks:
paused_by:
author:     Maksym Bodnar

## What was measured, S145, 2026-08-21

Found by turning on `-check-mate-pvs` in `fastchess.sh`, which fired on the
first four-game smoke match. Then measured properly, over 400 late-game
positions drawn from `.spsa/S085/games.pgn` and driven through iterative
deepening to depth 8, with `stockfish` at 300000 nodes as the truth:

| what | count |
|---|---|
| positions where chesso reported a mate at all | 8 |
| `info` lines carrying a mate score | 31 |
| lines whose `pv` is shorter than the distance claimed | **2** |
| lines whose distance disagrees with stockfish | **0** |

**So the score is right and the line is short.** That is the whole finding, and
it is why this is a reporting step and not a search step. Both short lines are
the same shape: `1Q6/pp3pk1/5b2/2P3p1/8/P5Pb/2p2P1P/5NK1 b - - 0 35` reports
`mate 3` at depth 4 with a 4-ply line where 5 are needed, and its successor
reports `mate -3` at depth 5 with 5 plies where 6 are needed. In both the
principal variation is exactly as long as the iteration is deep.

**The mechanism follows from that.** `state->pv_table` is filled from the main
search, so it can hold at most one entry per main-search ply. A mate found
inside quiescence -- which happens, and `tests/test_search.cpp`'s "mate is
recognised at depth zero" is the case that pins it -- puts a correct mate score
at a node whose line continues below the deepest main-search ply, and the line
stops where the table stops.

**What it costs today: nothing in play, and one class of noise in every SPRT
log.** The move played is the mating move and the distance is right, so no
measurement is contaminated and this did not jump the queue. What it does is
put `Warning; Incomplete mating PV` lines into the output of every match run
since S145, at a rate of roughly 2 in 31 mate reports, and a warning that is
always there is a warning nobody reads.

## Why it is worth a step rather than an accepted limitation

Because the check is free and it is now armed. `-check-mate-pvs` runs on every
SPRT and it is the only mate check that sees the positions the engine actually
meets -- tens of thousands a night against the 32 a constructed set can hold.
Its value is entirely in the signal-to-noise: with a known constant warning it
reports nothing, and with the truncation fixed the next `Incomplete mating PV`
line is a real defect the moment it appears. That is the same argument S106 made
for clearing the bound-sign findings before trusting the bound assertions.

Two candidate fixes, and the step is to measure which is enough rather than to
assume. Walking the transposition table from the end of the stored line until
the position is checkmate is what most engines do and touches nothing in the
search. Extending the line out of quiescence is the complete answer and is more
work. The excludes above allow the first and do not require the second.
