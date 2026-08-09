id:         S024
goal:       history indexed by the move played n plies ago and the current move
accepts:    an SPRT returns a verdict
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Note

One-ply continuation history is the countermove table already present. Two-ply
is the usual next step.
