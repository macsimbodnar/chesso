id:         S024
goal:       history indexed by the move played n plies ago and the current move
accepts:    two SPRT verdicts, one per table: the one-ply table first, the two-ply follow-up second, each against the commit before it
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Note

Chesso has the countermove *heuristic* and no continuation history at any
depth. `move_t counter_moves[12][64]` (`src/data_structures.hpp:434`, written
on beta cutoff at `src/search.cpp:507`, read as the fixed `ORDER_COUNTER` band
at `src/evaluation.cpp:1091-1094`) remembers a single refutation move per
(previous piece, target) and gives it a flat bonus — the countermove heuristic,
Uiterwijk 1992.

Continuation history is a different device: a score table indexed by
(previous move's piece, target) × (current move's piece, target), accumulating
graded bonuses and maluses for every quiet move, added into the ordering score
alongside plain history. One ply back is countermove history, two is follow-up
history; engines carry the heuristic and both tables at once, and the one-ply
table is consistently reported the stronger half of the pair.

Build the one-ply table first and take its verdict, then the two-ply table and
take its own. 2026-08-13_plan_review-F02 is why this note reads this way: the
previous version called the countermove table one-ply continuation history,
which would have scoped this step to the weaker half alone and recorded its
verdict against the wrong technique.
