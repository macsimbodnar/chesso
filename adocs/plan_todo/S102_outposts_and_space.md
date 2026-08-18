id:         S102
goal:       outpost and space terms in the evaluation, fitted like every other constant
accepts:    an SPRT verdict per term, measured separately -- outposts and space are two terms; each fitted with every other constant frozen, held-out error reported before and after; both share the pawn-derived bitboard fills the three S027 pawn terms already build, and the step states which fill each reuses rather than adding a pass; INV-5 holds, asserted by the mirror case in tests/test_evaluation.cpp; the taper is a single division if S055 has landed and the model guard's bound still holds; tools/eval_model.hpp gains each feature and tools/tuner_groups.hpp a group per term with the partition properties intact; the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp
excludes:   threat terms, which are S101; mobility, which exists; any term that needs a pass over the board that is not already being made
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## Why these two and not a longer list

They are the two terms on the standard hand-crafted list that reuse work
already being done: an outpost is a square no enemy pawn can attack -- which is
the pawn-attack span the passed pawn and pawn structure terms already fill --
and space is a count over safe squares behind one's own pawns in the centre,
from the same fills. Anything needing its own pass over the board is a
different trade and is not in this step.

## The measurement risk this step carries

Both terms are known to fit well and to be worth little on their own; the
literature's figures for them are small and this engine has measured 0, 0 and
*slower* on figures like that (DEC-019). A verdict of zero is the expected
outcome for at least one of the two and is recorded as zero.
