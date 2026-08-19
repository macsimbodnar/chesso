id:         S120
goal:       a small cache of full evaluations by position key, so the score behind the lazy shortcut can be paid for once
accepts:    an SPRT verdict, recorded whatever it is; the cache stores the **score and never a bound**, for the reason S094 records -- a bound is true on one side of one window and an entry outlives the window; the hit rate is measured over a real search and recorded; the step states whether the lazy shortcut is kept, narrowed or retired on the strength of the measured hit rate, and that statement is the input S039 and S122 read
touches:    src/evaluation.cpp, src/evaluation.hpp, src/data_structures.hpp
excludes:   choosing LAZY_EVAL_MARGIN, which is S039; the king safety rebuild, which is S122
decisions:  DEC-039
closes:
blocks:
paused_by:
done:

## The point is not speed, it is what it unblocks

The lazy shortcut exists because `evaluate()` costs 83 ns and quiescence calls
it at nearly every node. The shortcut's soundness rests on
`evaluate_expensive()` clamping mobility plus king safety to
**+/-LAZY_EVAL_MARGIN, 150 centipawns for the two of them together**. That
clamp is the ceiling on how much the evaluation is allowed to say, and a real
king-danger term needs to reach four to six hundred.

So the order is: cache the full score (here), size or retire the margin
(S039), then rebuild king safety without a clamp over it (S122). Measured cost
of paying full evaluation everywhere with no cache, 2026-08-19: **11.7 % of
nps**. S104 has already paid +16.7 % toward that, and this step and S118 are
what buy the rest back.
