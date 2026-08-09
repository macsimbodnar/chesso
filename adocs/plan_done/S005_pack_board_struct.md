id:         S005
goal:       remove the padding in board_t and put hash next to the scalars make_move writes
accepts:    board_t is smaller; perft node counts unchanged; the result is recorded even if it is zero
touches:    src/data_structures.hpp -- color_t and promotion_t uint8_t-backed, hash moved
excludes:
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in 9c13e3a. README and MANUAL checked at adoption, not when this shipped.

## Measurement -- no measurable effect

`board_t` 208 bytes to 200. Best-of-N over five interleaved rounds put the two
builds within **0.3 % of each other, which is noise.** Likely reason:
`board_t` is touched on every move, so it is L1-resident either way and the
cache-line split never costs a miss.

**Kept regardless** -- strictly less padding, costs nothing, and the size
matters again the moment an NNUE accumulator lands beside it. Recorded so the
idea is not retried expecting a speed-up.
