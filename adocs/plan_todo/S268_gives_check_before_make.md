id:         S268
goal:       a move a pruning rule has already discarded is discarded before make_move, by a gives-check test that reads the board without making the move, so the search stops making and unmaking moves it throws away
accepts:    (1) a predicate beside `src/bitboard.cpp` `is_check` that answers whether a move gives check without making it -- direct and discovered checks, the moving king's discovered check included -- with en passant, castling and promotion either handled or explicitly sent to the make-and-test fallback; (2) a test that the predicate agrees with `make_move` followed by `is_check` on every legal move of a corpus that contains each of those move kinds, and a mutant the test kills (`tools/mutation_check.py`); (3) used at the three sites the finding names: the quiet rules in `src/search.cpp` `negamax_at`, S091's capture rule there, and the futile captures in `src/search.cpp` `quiescence`; the PROBING build may keep its post-make record; (4) **node identity** (INV-6): `bench` and `tools/search_bench.py` node counts and best moves identical to the parent at two depths; (5) an interleaved timing on the workstation, noise floor read first (CLAUDE.md rules 4 and 5), with instruction and cycle counters (`perf stat -e instructions,cycles`) beside nps; kept only if faster; the audit's M1 figure is a pointer, not the claim; (6) the comment in `src/search.cpp` `negamax_at` that justifies the post-make test with another engine's figure is replaced by this step's measurement; (7) DEC-141's second tier: the Debug binary self-plays four rounds at 4+0.04 and its log is grepped for `Assertion`, and `tools/gate_extra.sh` runs, both named in the stamp
touches:    src/bitboard.cpp, src/bitboard.hpp, src/search.cpp, tests/test_movegen.cpp, tests/test_search.cpp
excludes:   any change to which moves a rule selects, or to its thresholds; move ordering and `pick_next_move`; the dead pruning work after late move pruning fires, which is S269
decisions:  DEC-083, DEC-141, DEC-260, DEC-263
closes:     2026-10-08_performance-F01
blocks:
paused_by:
author:
done:

## Why

At `bench`, 3313197 of the 5792360 moves `negamax_at` makes (57 %) are
unmade again without being searched; only 81133 of the moves a rule selected
survive by giving check. The audit's prototype (about 20 lines, quiet
non-castling moves only) was node-identical on three checks and measured
-7.6 to -9.0 % instructions and -5.0 to -7.2 % cycles on the M1. The post-make
order was chosen on another engine's Elo figure and never timed here
(CLAUDE.md rule 7). The report has the prototype and the commands that
re-derive every number: `adocs/audit/2026-10-08_performance.md`, F01.

## Lane

Agent work under DEC-260, written on either machine; it lands on the
workstation's timing. Behaviour-neutral, so no match (DEC-083).
