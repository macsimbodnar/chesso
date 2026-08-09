id:         S029
goal:       a perspective network evaluation trained on chesso's own self-play
accepts:    the accumulator is updated incrementally through the S008 primitives and asserted against a full refresh in the debug build; an SPRT against the tuned hand-crafted evaluation
touches:    src/, plus a separate training program outside the engine
excludes:   any Stockfish-derived training data -- see DEC-002
decisions:  DEC-002
closes:
blocks:
paused_by:
done:

## Shape

- Architecture: the standard `(768 -> N) x 2 -> 1` perspective network, `N` of
  256 to 512.
- The accumulator is updated incrementally in `make_move`, popped per ply in the
  search stack rather than reverse-updated -- which is exactly what S008 and
  S014 already put in place.
- Inference is integer SIMD. **This is where x86 stops being optional**: AVX2 and
  VNNI are where the performance is and Apple Silicon has no equivalent.
- Training is a separate program in Python or Rust, not part of the engine.

## Provenance constraint

Training data comes from self-play. No Stockfish-derived data, ever: the licence
provenance of the network stays clean. Running the Stockfish binary as a tool
creates no derivative work and is fine. DEC-002.
