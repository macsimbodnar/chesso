id:         S276
goal:       `feature_audit` and `truncation_scan` read the engine's trace (`tools/tuner_trace.hpp`) instead of the hand-written mirror, and `tools/eval_model.hpp` with the mirror's dataset is deleted
accepts:    (1) `feature_audit`'s report re-taken over the dedup corpus reads `S275_feature_audit.txt`'s figures -- rank 784, the 39 null directions, 0 column disagreements -- with its design rows the trace's coefficients times the taper weights and its `--plant` labels the trace's smooth score; (2) `truncation_scan` reads smooth against exact from the trace and reproduces the pinned four's 2.875; (3) `tools/eval_model.hpp`, the mirror dataset in `tools/tuner_model.hpp` and `test_eval_model`'s mirror cases are deleted, each deletion recorded, the engine-counting cases kept; (4) `adocs/data/S275_walk.cpp`'s build pin -- `f57b92f` or earlier -- is recorded where its row in `adocs/data/README.md` points
touches:    tools/feature_audit.cpp, tools/truncation_scan.cpp, tools/eval_model.hpp, tools/tuner_model.hpp, tools/tuner_trace.hpp, tests/test_eval_model.cpp, tests/test_eval_positions.hpp, tests/CMakeLists.txt, tools/CMakeLists.txt, DEV_MANUAL.md, adocs/data/README.md
excludes:   any change to src/ or to a shipped weight; any change to the tuner's fit path, which S262 settled
decisions:  DEC-142, DEC-170
closes:
blocks:
paused_by:
author:
done:

## Why

S262 moved the tuner onto `evaluate()`'s own trace and retired the mirror
from the fit, but `feature_audit` and `truncation_scan` still read
`tools/eval_model.hpp`, the hand-written copy of the evaluation. Two
descriptions of one function drift: every evaluation step after S262 would
otherwise have to update the mirror for those two tools alone, or let them
audit a model the engine no longer computes. Proposed in S262's As built.

## Lane

Agent work, tooling only; no match, no timing.
