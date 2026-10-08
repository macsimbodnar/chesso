id:         S266
goal:       a complexity term moves the endgame score toward zero in structures the stronger side is unlikely to convert, and never flips the score's sign
accepts:    (1) complexity is a fitted linear sum of features `evaluate()` already holds -- pawn count, pawns on both flanks, a pawn-ending flag, a constant -- applied to the endgame half as `E + sign(E) * max(C, -|E|)`, the form the paper describes; (2) a test, with each precondition established first: the adjusted endgame score never has the opposite sign of `E`, it is zero where `E` is zero, and it **may** reach zero where `C <= -|E|` -- the 2026-10-07 proposal's "zero only where the unadjusted one is" contradicts its own formula and is not the contract; (3) the group is fitted through S262's nonlinear-group support with its gradient checked, every other constant frozen, from a nondegenerate seed (DEC-134 form b); (4) the term passes DEC-259's offline screen -- held-out loss on S082's fixed validation set below the model without it by the pre-registered margin -- before any match, and a term that fails the screen is recorded and not measured; (5) one `{0, 5}` nElo SPRT, worst case and abort rule pre-registered (DEC-143); (6) INV-4 and INV-5 hold: nothing recomputed from bitboards that the stage does not already have, and the mirror case in `tests/test_evaluation.cpp` extended; (7) `adocs/specs.md` gains the term
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, adocs/specs.md
excludes:   the endgame scale factor and mop-up, which are S124's family; any change to the tapered PSQT
decisions:  DEC-258, DEC-259, DEC-192, DEC-134, DEC-221
closes:
blocks:
paused_by:
author:
done:

## Why

S217 found complexity scaling in two of the five hand-crafted engines rated
3000 to 3344 (Ethereal 11.59 to 11.64; rofChade) and no step for it here;
DEC-192 filed it as a reserve candidate with no published figure. The
2026-10-08 ruling makes it a step (DEC-258): the premise that kept it out --
the network would supersede hand-crafted refinements (DEC-138) -- fell when
the owner put the network after the mark (DEC-179).

## Description it is implemented from

Grant, *Evaluation & Tuning in Chess Engines* (2020): the complexity
adjustment (remaining pawns, pawns on both flanks, pawn endgame; dampens
toward a draw and never flips the side to move's advantage) and its
gradient. Ethereal's 11.75 release notes name the term. No engine's
constants seed it (DEC-105).

## From the record

`adocs/data/S217_handcrafted_gap.md`, its outline for this term. Ported from
the 2026-10-07 proposal's P25 with its test contradiction corrected.
