id:         S003
goal:       drop repetition_t and walk history.entries[].hash instead
accepts:    perft node counts unchanged; test_engine repetition cases still pass; history_entry_t is 16 bytes
touches:    src/data_structures.hpp, src/bitboard.cpp make_move/unmake_move, is_position_repeated
excludes:
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in b770043. README and MANUAL checked at adoption, not when this shipped.

## Why

`repetition_t` was 39 KB of hashes already stored in the history.
`repetitions.size` moved in lockstep with `history.size`, so
`history_entry_t::repetition_size` was a third copy of a known number.

## Measurement

`hyperfine`, 12 interleaved runs, sigma 0.013 s: **2.036 s to 1.925 s, 1.06x.**
Bench perft total 608 ms to 554 ms, 69.0 to 75.5 Mnps. `history_entry_t` 24 to
16 bytes.

The estimate said 1-3 %. It was low. **Standing prior: this codebase is more
sensitive to the size of what make/unmake touches than any estimate assumes.**
