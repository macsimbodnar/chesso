id:         S030
goal:       move_t drops the moving piece and becomes 16 bits
accepts:    perft node counts unchanged; identical tools/search_bench.py node counts and best moves against the preceding commit, or an SPRT verdict where the tree changes (INV-6); the speed change measured against the benchmark's own reported resolution and the keep-or-revert call made from that number — a zero is recorded as zero and does not block completion; the opening book and UCI layer still round-trip every move
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
`MOVE_PIECE` indexes `history_moves` and `counter_moves` in `score_move`, and
the killer, history and countermove writes in `negamax` index the same way.
**The sites are not all the same site and they do not share a remedy** -- this
is what the retired S058 and S079 corrected, folded in here by DEC-086.

- **Sites keyed on the move being scored** take `board->squares[MOVE_FROM(move)]`.
  The piece is still on the from-square at scoring time, so the substitution is
  exact.
- **Sites keyed on `prev_move`** cannot. `prev_move` has already been played, so
  its from-square is empty and its to-square holds the piece -- these take
  `board->squares[MOVE_TO(prev_move)]`, **and the promotion case is the trap**:
  after a promotion the piece standing on the to-square is not the piece that
  moved, so a countermove table keyed that way indexes a different row on the
  read than it did on the write. Either exclude promotions from the countermove
  key or key it on the promoted piece consistently on both sides.
- **The write site** is the countermove store in `negamax`'s fail-high block,
  which is keyed on `prev_move`; the read is in `score_move`. Both are named by
  symbol rather than by line, because S023 and S024 add to this set.

The site set is whatever `grep -n MOVE_PIECE src/` returns when the step
starts, not the list above. A slip changes ordering -- and so the tree -- with
perft still green, which is why the accepts carries the search_bench identity
clause and not perft alone.
