id:         S242
goal:       test_engine's two timing-dependent CHECKs in "the first iteration honours stop and the hard timer" flake under load, and a doctest failure inside a stdout_capture_t scope is reported with no text -- make the capture keep doctest's report, and make the two CHECKs assert what holds under load without weakening what they guard
accepts:    a doctest assertion that fails inside a `stdout_capture_t` scope reaches the test's output with its text (a deliberately failing assertion planted inside a capture, observed red with its message, then removed); the two `deepest_completed_depth(capture) < 1` CHECKs either establish their precondition -- the stop landed inside the first iteration -- or assert the property that holds regardless of load, and the case still guards a first iteration that honours `stop` and the hard timer; the fast suite green in both builds; the flake's signature (1 assertion failed, no text) reproduced once under load before the fix, or the attempt recorded with its load
touches:    tests/test_helpers.hpp, tests/test_engine.cpp
excludes:   the engine's time management; every other test; any relaxation of a ceiling
decisions:  DEC-171, DEC-141, DEC-142
closes:
blocks:
paused_by:
done:

## Why this exists

Found by S131's implementing agent on 2026-09-28 during its mutation baseline
under S113's SPRT load: `test_engine` failed 1 assertion of 870240 with no
failure text, twice (the baseline run and mutant V01's row), and passed 8 of 8
direct runs and 4 of 4 suite runs of the same binary
(`.tuning/coord/S131_logs/engine_flake/`). S131's cold fast check read the
code: doctest's console reporter writes to `std::cout`; `stdout_capture_t`
(`tests/test_helpers.hpp`) swaps `std::cout`'s buffer for its own
stringstream, so a failure logged inside a capture scope is discarded with the
capture; the failed assertion was a CHECK, because the run's total assertion
count equalled every passing run's (a failed REQUIRE aborts its case and lowers
the total); and the only timing-dependent CHECKs inside a capture are the two
`deepest_completed_depth(capture) < 1` in "the first iteration honours stop and
the hard timer" -- after `go movetime 100000` followed by `stop`, and after
`go movetime 1` -- which need the stop to land inside a depth-1 iteration of
about 14 ms. Under a twelve-core match that is not guaranteed.

Not reachable in ordinary play, on the UCI surface as harnesses drive it, or
in a reported score, move or line: a filler behind the next strength step
under DEC-171, named by id in every pre-registration taken while it is open
(S131's is the first).

## Shape

1. **A capture that cannot swallow doctest.** Give doctest its own output
   stream -- the context's `out` option, or a reporter bound to a stream the
   capture never touches -- so a failure inside a `stdout_capture_t` scope
   prints. Prove it the S033 way: plant a failing assertion inside a capture,
   observe the red with its text, remove the plant, and record the log.
2. **The two CHECKs.** State the precondition and make it testable: read
   whether the stop was issued before the first iteration ended (the engine's
   own timestamps, or a node-limited first iteration) and assert the depth
   only under that precondition; or assert the property that holds under any
   load -- a stop is honoured before the next iteration begins, the hard timer
   fires. The case's name says what it guards; that does not weaken.
3. **Evidence first.** Reproduce the signature under load with the text now
   visible (a match or a deliberately loaded machine), or record the attempt.
   A defect in the engine found this way is a BUGS-rule fix, not this step.
