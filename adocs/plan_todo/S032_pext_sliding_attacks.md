id:         S032
goal:       use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
accepts:    perft node counts unchanged on both paths; measured on x86-64, since it cannot be measured here
touches:    src/bb_tables.hpp, src/bitboard.cpp sliding attack generation
excludes:   fancy or black magics -- see DEC-021
decisions:  DEC-021
closes:
blocks:
paused_by:
done:

## Blocked on hardware

**Zero on this machine.** ARM has no PEXT. Published comparisons put PEXT at
12.8 s against 13.5 s for magics on Kiwipete perft(6), so call it about 1 % of
perft. Its real hazard is that it is dead code on the development machine.

An x86-64 Linux box is needed for this and is useful well before S029.
