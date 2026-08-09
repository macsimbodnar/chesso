id:         S031
goal:       one unconditional xor for the side-to-move zobrist key instead of two
accepts:    perft node counts unchanged; compute_full_hash() and swap_side() agree
touches:    src/bitboard.cpp
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Estimate

**Under 1 %**, which is below anything this machine can resolve. Listed only
because it is a ten-minute change with no risk attached, and it should be
carried in alongside other hash work rather than measured on its own.
