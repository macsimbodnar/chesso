id:         S131
goal:       quiescence searches non-capture queen promotions instead of filtering them out
accepts:    an SPRT verdict, recorded whatever it is; queen promotions pass the filter, underpromotions stay out unless a node-count sweep says otherwise, and the step states which was shipped and why; the filter comment in `quiescence()` is rewritten to describe what is searched now rather than what used to be; the mate-in-quiescence case in the fast suite still passes; the fast suite green
touches:    src/search.cpp quiescence, tests/test_search.cpp
excludes:   futility exemptions for promotions, which are S112's; the SEE treatment of promotions, unchanged here
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

## Created by the second review, DEC-087 -- the engine's own recorded TODO

The filter in `quiescence()` keeps non-capture promotions out, and its own
comment, written at S094, says the quiet part out loud: "Letting them through
is very likely an improvement, but it is a search change and needs to be
measured in games." This step is that measurement and nothing else.

A pawn reaching the eighth is a capture-magnitude material event that the
capture-only stage cannot see: a horizon node with an unstoppable promotion
stands pat on a score that is about to be wrong by a queen. Every surveyed
engine's noisy stage includes queen promotions. No published isolated number
exists -- the seed here is this engine's own comment, which is why the verdict
is the whole content of the step.
