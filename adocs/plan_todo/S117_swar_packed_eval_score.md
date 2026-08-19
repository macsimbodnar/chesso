id:         S117
goal:       the middlegame and endgame halves of every evaluation term travel in one integer instead of two
accepts:    identical scores from `bench_eval`'s checksum over its ten positions and identical node counts and best moves from tools/search_bench.py against the preceding commit -- **behaviour-neutral, so no SPRT is owed** (INV-6, DEC-083); the nps change measured by interleaved runs with the spread recorded and converted at the published rate; the truncation behaviour is unchanged or the change is stated and test_eval_model's tolerance is re-pinned rather than relaxed; the packing survives a negative endgame half, which is the classic sign-extension bug, and a test covers it
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/eval_model.hpp, tests/
excludes:   any change to a weight or a term
decisions:  DEC-083
closes:
blocks:
paused_by:
done:

## Why this is on the plan at all

Reported at **+25.41 Elo** in one engine -- the largest single evaluation-speed
number in the surveyed record. Chesso carries `psqt_mg` and `psqt_eg` as
separate accumulators and every term as a separate mg/eg pair, so the whole
evaluation does twice the adds it needs to.

It is behaviour-neutral by construction and therefore costs no verdict, which
makes it one of the cheapest items here. The risk is entirely in the packing
arithmetic: the endgame half must survive being negative, and the single
tapering division at the end has to produce the same truncation the two
divisions produce today or `test_eval_model`'s pinned tolerance moves.
Interacts with S055, which removes a division from the same expression -- do
S055 first or fold it in, and say which.
