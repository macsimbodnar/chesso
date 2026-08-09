id:         S018
goal:       rank chesso's own errors by game phase over hundreds of games, from Stockfish
accepts:    a script that takes a PGN of many games, runs the S016 pipeline over all of them, and reports total centipawns lost bucketed by game phase and by error size; run against the fixed mailbox build in ~/.local/bin and the numbers recorded here
touches:    tools/
excludes:   fixing anything it finds -- that is S019 and after
decisions:
closes:
blocks:
paused_by:
done:

## Why this comes before any evaluation term

Every published Elo figure this project has borrowed has failed to transfer:
staged generation 30-50 predicted and 0 measured, SEE quiescence pruning 0,
move ordering reported around 150 and measured slower. See DEC-004. The
remaining way to choose the next evaluation term is to measure where this
engine actually loses centipawns, not where other engines report gains.

One game has already been analysed this way and produced S019. One game is an
anecdote; this step turns it into a distribution.
