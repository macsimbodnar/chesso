id:         S242
goal:       test_engine's two timing-dependent CHECKs in "the first iteration honours stop and the hard timer" flake under load, and a doctest failure inside a stdout_capture_t scope is reported with no text -- make the capture keep doctest's report, and make the two CHECKs assert what holds under load without weakening what they guard
accepts:    a doctest assertion that fails inside a `stdout_capture_t` scope reaches the test's output with its text (a deliberately failing assertion planted inside a capture, observed red with its message, then removed); the two `deepest_completed_depth(capture) < 1` CHECKs either establish their precondition -- the stop landed inside the first iteration -- or assert the property that holds regardless of load, and the case still guards a first iteration that honours `stop` and the hard timer; the fast suite green in both builds; the flake's signature (1 assertion failed, no text) reproduced once under load before the fix, or the attempt recorded with its load
touches:    tests/test_helpers.hpp, tests/test_engine.cpp
excludes:   the engine's time management; every other test; any relaxation of a ceiling
decisions:  DEC-171, DEC-141, DEC-142
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-28 23:32 CEST
done:       2026-09-29 -- a doctest failure inside a `stdout_capture_t` scope reaches the output with its text: `capture_report_listener_t` in `tests/test_helpers.hpp` repeats on std::cerr what the console prints while a capture is alive, observed the S033 way (planted CHECKs red with no text before, with their text after, each reported once through ctest, the plants removed). The flake's signature reproduced under load, 1 of 500 runs of the case; with the text visible it was the hard timer's depth claim, 4 of 2000, a correct engine finishing depth 1 20 to 27 ms after a 1 ms limit. The first redesign retried it, which the cold fast check showed to be a relaxation; the coordinator ruled DEC-240 and the case lands in that form, renamed "a stop inside the first iteration cuts it and the hard timer ends the search within its bound": the stop half keeps its claim and its clock precondition and retries only a failed precondition; the hard-timer half asserts `bestmove` within 402 ms of `go movetime 1` (the timer's 1 ms, 1 ms of check granularity, 400 ms of scheduler allowance, ten times the largest latency measured under a match), once and never retried, on a board of its own, because under `go movetime` a timer that never fires stops at the first completed iteration (the soft limit), not at the depth cap -- 8.3 to 9.0 ms on the eight-queens board, about 5.9 s on the new one, measured with the timer disarmed. 500 runs, 0 failed, no retry; red on both halves with F21 restored, and on the timer half alone with the timer disarmed. No ceiling moved, `src/` untouched, so no `Bench:` line and no second tier; the one new golden, the board's depth-1 time, is named with its command. `DEV_MANUAL.md` and `MANUAL.md` checked, no change; `specs.md` untouched. Both fast suites, the format check and the prose checks green. Test-side filler under DEC-171, named in the pre-registrations taken while it was open (S131's). Written by an Opus subagent briefed by the coordinator.

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

## What was found, 2026-09-28 and 29

Measured in the linked worktree `chesso-fill` at `13caf43`, every binary run
at `nice -n 19` beside S131's SPRT on every core (one-minute load 14.2 to
17.1 on twelve hardware threads). Logs under `.tuning/coord/S242_logs/`.

**The capture.** `tests/test_helpers.hpp` `stdout_capture_t` swaps
`std::cout`'s buffer for its own, and doctest's console reporter writes to
the stream in its context options, which is `std::cout` unless `--out` names
a file or `--quiet` discards it. So a failure logged inside a capture scope
went into the capture's buffer and out of the run with it. A planted
`CHECK(false)` and a `CHECK_MESSAGE` inside the capture of "position and
moves are applied" reproduce it exactly: exit 1, `2 failed`, no text
(`plant_before_fix.log`).

**The flake.** Thirty whole runs of the unmodified binary: 30 passed
(`loop1_before/`). Five hundred runs of the case alone: 1 failed, with no
text -- `assertions: 6 | 5 passed | 1 failed`, the signature S131 met twice
(`loop1b_before_focused/`, run 171). With the capture fixed and the CHECKs
still as they were, 2000 runs of the case alone: 4 failed, all four with
their text, and all four were the hard timer's -- after `go movetime 1` the
engine reported `depth 1 nodes 36165` at `time` 20 to 27 ms
(`loop2_capture_fix_focused/`, runs 0061, 0078, 1745, 1844). The `stop`
CHECK failed in none of the 2000.

**Why, and why it is not an engine defect.** The hard timer is a detached
thread that sleeps and then sets the flag (`src/chesso.cpp`
`stop_search_after_ms`), `stop` is set by the caller's thread
(`src/chesso.cpp` `command_stop`), and the search reads the flag every 2048
nodes (`src/search.cpp` `check_limits`). Nothing orders either sender
against the search thread, and with every core busy a sender can be held off
the CPU for the whole depth-1 iteration. A correct engine then finishes an
iteration whose stop came after its last poll. No BUGS-rule fix is owed.

## What changed

`tests/test_helpers.hpp`. `stdout_capture_t` counts the captures alive
(`stdout_capture_t` `any_alive`). A doctest listener,
`capture_report_listener_t`, registered from the header, repeats on
`std::cerr` what the console prints -- a failed assertion with its values and
its INFO or message contexts, a MESSAGE, WARN or FAIL, a crash -- while a
capture is alive and the console writes to `std::cout`, and prints nothing
otherwise, so no report appears twice. It flushes C's stdout first, so in a
log that takes both streams the report lands where it happened. It uses the
reporter interface `tests/doctest/doc/markdown/reporters.md` documents and
nothing inside doctest's implementation. Every binary that includes the
header has it without asking: the eight that construct a capture benefit --
`test_engine`, `test_uci_surface`, `test_audit_go_infinite`,
`test_audit_fen_semantics`, `test_search_params`, `test_mate_pv`,
`test_mate_breadth`, `test_mate_carry` -- and the five that include it
without one (`test_invariants`, `test_evaluation`, `test_movegen`,
`test_search`, `test_en_passant_key`) carry it inert.

`tests/test_engine.cpp` "the first iteration honours stop and the hard
timer". The precondition block, the position, the golden and its floor are
unchanged, and so are the case's name and both claims. Each claim's
precondition -- the stop landed inside the first iteration -- is now read per
attempt instead of assumed, and a claim is attempted up to five times on a
fresh engine:

- a cut first iteration (no info line reports depth 1 or more) establishes
  the precondition and the claim at once;
- a finished one establishes nothing, is kept for the message, and the
  attempt is repeated;
- for `stop` only, this thread times the command: back within
  `prompt_stop_us`, half of `depth_1_floor_ms`, after `go` was sent, the
  flag was set before the search thread can have done half of the
  iteration's work on any machine the floor admits, so a finished iteration
  there fails the attempt on the spot, as the single attempt did;
- the claim fails when no attempt of five established it, with every miss's
  engine output in the message.

The unfixed engine finishes every attempt, so it fails both halves as it
always did. No ceiling moved and no golden was added: `attempts` is a retry
budget and `prompt_stop_us` is derived from the existing floor, not measured.

**What the redesign costs in sensitivity.** A defect that honoured the stop
only some of the time would be caught per attempt on the `stop` half
whenever the clock settles the precondition, which it did in 299 of 300
attempts under the match (below); on the timer's half it is caught only when
five attempts in a row miss. The two halves share the pointer the defect
lived in (S210's F21), so the `stop` half carries that coverage for both.

## Evidence

- **The plant, the S033 way.** After the fix the same two plants print with
  their text, values and the logged message (`plant_after_fix.log`); through
  ctest over the whole binary, with a third plant outside any capture, each
  of the three is reported exactly once (`plant_after_fix_ctest.log`). The
  plants were removed and `tests/test_engine.cpp` was back to `13caf43`
  byte for byte before the redesign (`plant_removed.log`).
- **Under the match, after both changes.** 2000 runs of the case alone: 0
  failed; 9 runs took a second attempt (an assertion total of 8 instead of
  7), none a third (`loop3_final_focused/`).
- **How `stop` behaves under the match.** A diagnostic build with one
  temporary MESSAGE, 300 runs of the case: `stop` sent and back median 63 us,
  p90 71, p99 102, max 36941; 299 of 300 under `prompt_stop_us`; all 300 cut
  (`diag_stop_us.txt`). Measured under load to see the mechanism, not a
  timing claim about the engine. The line was removed.
- **Red against the unfixed engine.** A fixture at `13caf43` with both test
  files and S210's F21 defect restored in `src/chesso.cpp`
  (`apply_unfixed_mutant.py`: the first search of each iteration on a local
  flag nobody sets): 3 of 3 runs of the case red on both halves -- `stop`
  settled by the clock at 61, 62 and 95 us and the iteration finished; the
  timer finished all five attempts -- and the whole binary 2 failed, both
  this case (`unfixed_case_run{1,2,3}.log`, `unfixed_whole_binary.log`).
  Run 3 took the retry path: attempt 1 a miss, attempt 2 the failure.
