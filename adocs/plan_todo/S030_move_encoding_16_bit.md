id:         S030
goal:       move_t drops the moving piece and becomes 16 bits
accepts:    perft node counts unchanged; identical tools/search_bench.py node counts and best moves against the preceding commit, or an SPRT verdict where the tree changes (INV-6); measured gain larger than the benchmark's own resolution; the opening book and UCI layer still round-trip every move
touches:    the encoding macros, src/bitboard.cpp, move ordering, the opening book, the UCI layer
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Estimate and risk

1-3 %. The move list is `MAX_MOVES` entries touched at every node, so this
halves the traffic through it. Risk is medium and **wide rather than deep** --
it touches six areas. Do it when the surrounding code has stopped moving, which
is why it sits below the search work rather than above it.

The neutrality hazard is move ordering, which reads the field being removed:
`MOVE_PIECE` (`src/data_structures.hpp:86`) indexes `history_moves` and
`counter_moves` in `score_move` (`src/evaluation.cpp:1092,1097`). Every such
site must be re-pointed at `squares[from]`, and a slip there changes ordering
-- and so the tree -- with perft still green. That is why the accepts carries
the search_bench identity clause and not perft alone.
