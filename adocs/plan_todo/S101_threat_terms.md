id:         S101
goal:       evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the terms are fitted with every other constant frozen and the held-out error reported before and after, which is how every constant in this engine was fitted; the term is accumulated or computed in a stage that already has the attack bitboards it needs, never rebuilt from the bitboards a second time, since evaluate() runs at every quiescence node and INV-4 exists to keep that cost out; INV-5 holds -- mirroring a position agrees rather than negates, asserted in tests/test_evaluation.cpp; the lazy evaluation bound still holds by construction if the term lands in the expensive stage, or the step states why it belongs in the cheap one; tools/eval_model.hpp gains the feature and tools/tuner_groups.hpp a group whose partition properties still hold; the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp
excludes:   outposts and space, which are S102; king safety, which exists and is fitted
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The hazard is where it is computed, not what it scores

Every evaluation term must be accumulated or shared, never recomputed.
`evaluate()` runs at every quiescence node and rebuilding the tables from the
bitboards was 25 % of nodes per second before S014 removed it. S027's three
pawn terms landed in the cheap stage precisely because the lazy clamp would
truncate them; a threat term has the same question to answer and answers it in
the step, with the answer measured.
