id:         S278
goal:       the sanitizer build's `test_tuner_gradient` and `test_eval_trace` run clean: UBSan's "store to null pointer of type 'struct record_t *'" at `tools/tuner_trace.hpp` `trace_board`'s `eval_trace::sink = &record;` is explained and fixed, and `tools/gate_extra.sh`'s sanitize stage is green again
accepts:    (1) the failure reproduced first on `f6647dc` with a minimized failing case, observed, not assumed; (2) the cause stated -- the thread_local `eval_trace::sink` defined in `src/evaluation.cpp` under `CHESSO_TRACE` and written from the tools' translation units -- and whether any non-sanitizer build is affected (the trace reconstructed `evaluate()` on all 10795695 rows in S262, S263 and S276, so the Release path works; say why the sanitizer build differs); (3) the fix changes no `chesso` binary: `evaluation.cpp`'s object in the plain build byte-identical, bench 3872032; (4) `tools/gate_extra.sh` all five stages green, its sanitize stage's bench check included
touches:    src/eval_trace.hpp, src/evaluation.cpp (the trace-only definition, under `CHESSO_TRACE`), tools/tuner_trace.hpp, cmake/ or tools/CMakeLists.txt if the trace library's sanitizer flags are the cause, tests/ for the minimized case
excludes:   any change to `evaluate()`'s arithmetic or to the shipped binary; the tuner's fit path
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:
done:

## Why this exists (2026-10-13, the coordinator, from S099 (a)'s second tier)

S099 (a)'s `gate_extra.sh` run had its sanitize stage red: `test_tuner_gradient`
and `test_eval_trace` stop at UBSan "store to null pointer of type 'struct
record_t *'" on `eval_trace::sink = &record;` in `tools/tuner_trace.hpp`
(`trace_board`). The same two tests fail on the same line on a clean `5ac17bd`
worktree, 43 of 45 passing on both, so S099 did not cause it. The sink is
`thread_local` (`src/eval_trace.hpp`, defined in `src/evaluation.cpp`). The
last green `gate_extra` was S119's, on `298788b`, before S262 added the trace;
S262, S275 and S276 are tooling steps that ran no second tier, which is how it
went unseen. The sanitizer build's bench, run by hand, prints 3872032 with no
report.

## Lane

Agent work, tooling; minutes of machine. Filler under DEC-171: not reachable
in play, on the UCI surface, or by a reported score, move or line. Named as an
open finding in the pre-registration of every run taken while it is open.
