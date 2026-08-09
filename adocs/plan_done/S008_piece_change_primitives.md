id:         S008
goal:       every piece change in make_move goes through add_piece, remove_piece, move_piece
accepts:    no open-coded piece mutation left in make_move; debug build asserts squares[] against the bitboards on every make and unmake; perft node counts unchanged
touches:    src/bitboard.cpp
excludes:   unmake_move, which deliberately does not use them
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 8e46432. README and MANUAL checked at adoption, not when this shipped.

## Why this is not a refactor

These three functions are the alphabet an NNUE accumulator is updated from.
Six open-coded mutation sites in `make_move` meant six places to forget an
accumulator update later. Closing that door before S029 exists is the whole
point.

`unmake_move` deliberately does not use them: an accumulator is kept per ply in
the search stack and popped, never reverse-updated, so only the forward
direction needs the events.
