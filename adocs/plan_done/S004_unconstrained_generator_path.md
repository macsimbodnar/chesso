id:         S004
goal:       specialise generate_moves on whether checkers or pins constrain the move list
accepts:    perft node counts unchanged; the unconstrained instantiation contains no check-mask or pin test after optimisation; gain consistent across every interleaved round
touches:    src/bitboard.cpp -- body templated on <Color, Constrained>, front end computes checkers and pinned and picks the instantiation
excludes:   a hand-written second copy of the emit loops
decisions:
closes:
blocks:
paused_by:
done:       2026-08-09  retro-stamped at moltke adoption (DEC-001), shipped in d453da2. README and MANUAL checked at adoption, not when this shipped.

## Measurement

Three interleaved rounds:

```
perft total          475.0 -> 466.9 ms   -1.8 %   88.0 -> 89.6 Mnps
generate_moves alone 659.2 -> 582.1 ms  -11.7 %  540 -> 612 Mmoves/s
```

The generator got 11.7 % faster but is only a fifth of perft, so the workload
moved 1.8 %. **Below this project's 3 % bar**, kept only because the direction
is consistent across every round and the mechanism is understood.

Writing the masks as `Constrained ? mask : ~BB_0` is enough: the condition is
compile-time, so the optimiser removes every `& check_mask`, every pin test and
the whole pinned-pawn loop as dead code. Object code 61 to 68 KB. This is the
last template parameter that pays; a third doubles the code for less.
