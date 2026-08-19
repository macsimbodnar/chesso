id:         S121
goal:       mobility becomes a fitted curve per piece over a mobility area that excludes what a piece cannot safely stand on
accepts:    an SPRT verdict, recorded whatever it is; mobility is a table indexed by piece type and by count -- knight 0 to 8, bishop 0 to 13, rook 0 to 14, queen 0 to 27 -- and **every entry is fitted by our own tuner on our own corpus** (DEC-084); the mobility area excludes squares attacked by enemy pawns and the side's own blocked and low-rank pawns, and each exclusion is a separate measured decision rather than one bundle; the tuner's model in tools/eval_model.hpp is updated in the same commit and test_eval_model holds the two against each other; the nps cost is recorded next to the verdict
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_eval_model.cpp
excludes:   king safety, which shares the loop but is S122
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there, and why the fit could not save it

Mobility is **linear**: one weight per piece type times a raw count, over
`attacks & ~own` with no exclusions at all. `src/evaluation.cpp`:

    const int mobility_mg[4] = {-1, 5, 8, 3};   // knight bishop rook queen
    const int mobility_eg[4] = {0, 5, 0, -6};

Knight middlegame **-1**, knight endgame **0**, rook endgame **0**, queen
endgame **-6**. Those are what a correct fit returns when the model cannot
express the shape: mobility is not linear in the count -- the first few squares
are worth far more than the twentieth -- and a single coefficient fitted across
that curve lands near zero. DEC-040 already noticed the symptom and read it as
a finding about knights; it is a finding about the model.

The exclusions are the other half and are separately reported large: excluding
rammed and low-rank own pawns **+19.95**, excluding enemy pawn attacks from
knight mobility **+10.5**, removing the king square **+10.9**. Note what is
*not* done anywhere: full "safe mobility" excluding every attacked square. The
exclusion is enemy **pawn** attacks, and that distinction is worth stating
because it is the cheap half.
