id:         S002
goal:       make_move, unmake_move and generate_moves take colour as a template parameter
accepts:    perft node counts unchanged; measured gain tracks the profile; no instruction-cache regression at the resulting object size
touches:    src/bitboard.cpp
excludes:   any logic change -- the ternaries stay verbatim and fold on their own
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in a25237f. README and MANUAL checked at adoption, not when this shipped.

## Measurement

Applied one function at a time, each measured against the previous, three
interleaved rounds each:

| step | before | after | gain |
|---|---|---|---|
| `make_move` | 552.5 ms | 517.1 ms | -6.4 % |
| `unmake_move` | 517.1 ms | 491.9 ms | -4.7 % |
| `generate_moves` | 491.9 ms | 485.5 ms | -1.3 % |
| **total** | **552.5 ms** | **485.5 ms** | **-12.1 %** |

75.5 to 86.0 Mnps. `generate_moves` alone 754 ms to 674 ms, -10.6 %. The gains
track the profile almost exactly -- the first time in this project an estimate
landed where it was aimed. Object code 61 KB with both instantiations.
