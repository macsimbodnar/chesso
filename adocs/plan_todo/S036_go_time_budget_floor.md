id:         S036
goal:       a go command with a 1 ms clock returns a bestmove instead of searching forever
accepts:    compute_search_time_ms returns a positive budget for every remaining_ms >= 1; go wtime 1 answers with a bestmove without a stop; the new cases are in tests/test_engine.cpp and were observed failing first
touches:    src/chesso.cpp compute_search_time_ms and command_go, tests/test_engine.cpp
excludes:   the time management policy itself -- how much of the clock a move gets is not being retuned here
decisions:
closes:     2026-08-13_adversarial-F02
blocks:
paused_by:
done:

## What is broken

`compute_search_time_ms()` (`src/chesso.cpp:393-405`) returns 0 for
`remaining_ms == 1` at every increment and every `movestogo`:
`budget = std::min(budget, remaining_ms - MOVE_OVERHEAD_MS)` drives it to -49,
and the floor `std::max(budget, std::min(50, remaining_ms / 2))` evaluates to
`min(50, 0) = 0`.

Nothing then bounds the search. `command_go`'s zero-clock fallback at
`src/chesso.cpp:1047` is guarded by `else if (!depth_given && ...)` and is
unreachable, because `remaining_ms > 0` already took the branch above it. The
timer is armed at `src/chesso.cpp:1056` only `if (search_time_ms > 0)`. Depth is
`MAX_DEPTH`, `nodes` is 0, `infinite` is false.

Measured: 30 s, depth 19, 137396943 nodes, no `bestmove`. An explicit `stop`
answers in 0.11 s.

## Impact

A protocol hang reachable at flag fall in any timed match, which is every match
this project runs. The process pins a core until killed, and at the concurrency
of DEC-048 it shares the machine with the remaining games of the same SPRT — so
it distorts the timed games running beside it, which is precisely what
`fastchess.sh`'s load warning exists to prevent.

## Why the existing test misses it

`tests/test_engine.cpp:400` asserts `budget > 0`, exactly the property that
fails. Its smallest case is `remaining = 51`
(`tests/test_engine.cpp:409-418`). `fastchess --compliance` passes too; its
smallest clock is `wtime 100`.

## Shape

Either give `compute_search_time_ms()` a floor that does not collapse when
`remaining_ms <= 2 * MOVE_OVERHEAD_MS`, or make `command_go` treat a
non-positive budget the way it treats a clock of zero and fall back to
`FALLBACK_SEARCH_TIME_MS`. The first keeps the decision in one function.

Red first: add `{1, 0, 20}` and `{1, 100, 1}` to the case list at
`tests/test_engine.cpp:409` and observe `REQUIRE_MESSAGE(budget > 0, title)`
fail before the fix.
