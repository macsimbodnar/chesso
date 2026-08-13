id:         S032
goal:       use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
accepts:    perft node counts unchanged on both paths; identical tools/search_bench.py node counts and best moves with PEXT on and off (INV-6); measured here, with the magics fallback still built and perft-verified
touches:    src/bb_tables.hpp, src/bitboard.cpp sliding attack generation
excludes:   fancy or black magics -- see DEC-021
decisions:  DEC-021
closes:
blocks:
paused_by:
done:

## Measurable here since DEC-049

The machine this tree builds on has BMI2 (`grep -o bmi2 /proc/cpuinfo`), so the
step stopped being blocked when the work moved to it (DEC-049; the old text
predates the move and priced this as zero on ARM). Published comparisons put
PEXT at 12.8 s against 13.5 s for magics on Kiwipete perft(6), so call it about
1 % of perft. The dead-code hazard has inverted with the machine: the magics
path is now the one nothing exercises locally, and it stays built and
perft-verified because BMI2 is not universal and pre-Zen 3 AMD runs PEXT in
microcode, slower than magics.
