id:         S125
goal:       backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
accepts:    an SPRT verdict per group, recorded whatever it is; the terms are indexed by file or by rank where the surveyed record says the indexing is what pays, and the step states which indexing it chose and why; **an isolated pawn is never also counted backward**, which is a documented double-count worth +4.01 to fix; every constant is fitted (DEC-084); the terms live behind the pawn hash from S118 so the added cost is paid once per structure
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp
excludes:   passed pawns, which are S123
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there

Three terms -- isolated, doubled, backward -- each one flat weight, fitted to
`{-10, -9, -11}` middlegame and `{-12, -32, -8}` endgame. What is missing is
phalanx and connected pawns, supported pawns, and weak unopposed pawns; and
what the surveyed record says pays more than any of them is **conditioning**
rather than adding: the largest pure pawn-structure patch on record is "apply
the isolated penalty only when there is no pawn capture available", at +10.68.
Indexing the existing three by file and by rank is reported at +6.60 and +3.77.

So this step is as much about giving the three terms that exist more shape as
about adding four more.
