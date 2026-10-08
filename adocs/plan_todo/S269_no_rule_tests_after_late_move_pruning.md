id:         S269
goal:       once late move pruning has fired, a quiet move skips futility, history pruning, quiet SEE and S091's exchange test, none of whose answers can change what happens to it
accepts:    (1) in `src/search.cpp` `negamax_at`, the three per-move quiet rules are not evaluated for a quiet once `skip_quiets` is set, and `see_loses_material` is not computed for such a quiet; (2) **node identity** (INV-6): `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths; (3) an interleaved timing on the workstation, noise floor read first (CLAUDE.md rules 4 and 5), with instruction and cycle counters beside nps; kept only if faster, and timed against the parent alone, not stacked on S268; (4) under PROBING a skipped quiet records `PRUNE_LATE_MOVE`; a test case states which rule the probe records for it; (5) DEC-141's second tier: Debug self-play, four rounds at 4+0.04, the log grepped for `Assertion`, and `tools/gate_extra.sh`, both named in the stamp
touches:    src/search.cpp, tests/test_search.cpp
excludes:   any change to which moves are pruned, reduced or searched; the late move pruning threshold; the pre-make gives-check test, which is S268
decisions:  DEC-083, DEC-141, DEC-260, DEC-263
closes:     2026-10-08_performance-F02
blocks:
paused_by:
author:
done:

## Why

Once `skip_quiets` is set, a quiet's fate is fixed: any rule's verdict and
the late move rule's verdict reach the same branch, and the exchange value is
read only behind a reduction guard such a move cannot pass. At `bench`, 46 %
of the quiet-SEE calls and 28 % of S091's calls are made in that state. The
audit's two-line prototype was node-identical on three checks and measured
-1.6 to -2.4 % instructions and -2.7 to -3.9 % cycles on the M1.
`adocs/audit/2026-10-08_performance.md`, F02, has the diff and the counts.

## Lane

Agent work under DEC-260; it lands on the workstation's timing.
Behaviour-neutral, so no match (DEC-083).
