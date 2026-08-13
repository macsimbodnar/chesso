id:         S029
goal:       a perspective network evaluation trained on chesso's own self-play
accepts:    the accumulator is updated incrementally through the S008 primitives and asserted against a full refresh in the debug build; the training program and the self-play data pipeline delivered and the run stated for the owner; then an SPRT of the returned network against the tuned hand-crafted evaluation
touches:    src/, plus a separate training program outside the engine
excludes:   any training data derived from another engine's evaluation or search (DEC-016); **running the training** -- the agent delivers the program and the data, the owner runs it (DEC-015)
decisions:  DEC-016, DEC-015
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
- Inference is integer SIMD. On this machine (DEC-049) that is AVX2, with VNNI
  where it exists; the requirement is integer SIMD throughput, not x86 as such
  -- published NNUE engines run the same inference on Apple Silicon via NEON.
- Training is a separate program in Python or Rust, not part of the engine.

## Provenance constraint

Training data comes from chesso's own self-play. **No data derived from another
engine's evaluation or search, ever** -- the provenance of the network stays
clean, and reproducing someone else's network is the opposite of what this
branch is for. Running another engine's binary as a tool creates no derivative
work and is fine. DEC-016, DEC-013.

## Split of work

The agent builds the training program, the self-play data generation and the
inference code, and states exactly what the run should be. **The owner executes
the training.** The network comes back and is measured by SPRT like any other
change. DEC-015.
