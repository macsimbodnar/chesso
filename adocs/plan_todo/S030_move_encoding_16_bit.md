id:         S030
goal:       move_t drops the moving piece and becomes 16 bits
accepts:    perft node counts unchanged; measured gain larger than the benchmark's own resolution; the opening book and UCI layer still round-trip every move
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
