id:         S122
goal:       king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
accepts:    an SPRT verdict, recorded whatever it is; the term can return several hundred centipawns and **no clamp truncates it** -- S039 and S120 have settled the lazy shortcut before this runs, and this step states which of the three outcomes it inherited; everything feeding the accumulator is a fitted weight and only the finalizer is non-linear, so the tuner's gradient still exists and tools/eval_model.hpp carries the same finalizer; safe checks per piece type and weak squares in the king zone are counted; shelter and storm come from the pawn hash (S118); the corpus this is fitted on **contains attacking positions**, and the step says how that was ensured; **the move-ordering band clearance is re-checked**, because this term's magnitudes are about to grow by an order of magnitude
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_eval_model.cpp
excludes:   mobility, which shares the loop but is S121; the lazy margin, which is S039
decisions:  DEC-071, DEC-084, DEC-039
closes:
blocks:
paused_by:
done:

## The clamp is the whole problem

`evaluate_expensive()` clamps mobility **plus** king safety to
+/-LAZY_EVAL_MARGIN -- 150 centipawns for both together -- and `evaluate()` is
`evaluate_cheap() + evaluate_expensive()`, so the clamp is on the real score and
not only on the shortcut's. **The architecture forbids a king-safety term
strong enough to matter.** A mating attack is worth four to six hundred
centipawns and this evaluation cannot say more than one hundred and fifty about
the king and the mobility combined.

That is why S039 was moved from the end of the plan to just before this step,
and why S120 is ordered ahead of both: retiring the clamp costs 11.7 % of nps
measured, and the cache and S104 are what pay for it.

## The formulation, and the one that failed

Four formulations exist and they differ in how accumulated danger becomes
centipawns. The attack-unit table indexed by a hand-written S-curve -- **which
is what chesso approximates today, badly, with a linear model** -- is the
weakest and the hardest to tune: its entries are sparsely represented in any
corpus. One documented from-scratch implementation of it **regressed 8 to 10
Elo**, and the diagnosis was the corpus, not the code: the tuner drove the
attack bonus to +7 centipawns because it never saw a mating attack.

The form to reproduce is the tuned linear accumulator with a quadratic
finalizer, because it is the one that stays differentiable -- every input is an
ordinary weight the existing Adam fit handles and only the finalizer is
non-linear. **The finalizer's own constants are ours and are fitted at S127**,
not taken (DEC-084).

The corpus warning is the part to act on rather than read: this term is fitted
on chesso's own self-play, and if chesso plays positionally tame chess the fit
will find nothing to fit. Say in the step how the corpus was made to contain
sharp positions.
