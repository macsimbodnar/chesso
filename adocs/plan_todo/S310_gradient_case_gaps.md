id:         S310
goal:       `test_tuner_gradient`'s clamp block names S101's cells, its AdaGrad hold reads a margin and not a chance, and it runs from any directory
accepts:    the clamp case asserts every stage-two weight that the clamp silences, S101's cells among them, whichever family is live, its failure seen on a planted cell; the AdaGrad hold case's step is chosen by a stated rule that keeps its reading clear of its bound on the shipped constants, with the reading across epochs shown; the binary run from the repository root and from `build/tests` both read the same tally; no bound moves and no plant is dropped; both builds green
touches:    tests/test_tuner_gradient.cpp
excludes:   any change to the tuner; the case's clock, which is S309's
decisions:  DEC-171, DEC-316, DEC-334
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-01 by S101's phase 2, on its paste, none reachable in play, so filler by DEC-171.

- The clamp case's block stops at king safety, so S101's cells are covered on a bound row only by the
  finite difference.
- AdaGrad at step 10 goes chaotic late on the paste: `apart` read 5e-9 at epoch 250 and 5e-6 at 300. S101's
  fixture pair moved it to 7.39e-12, which is a reading and not a margin, and a later fit can bring it back.
- Three of the 13 cases fail when the binary runs from the repository root and pass from `build/tests`, where
  ctest runs it. Cause not investigated.
  [2026-11-02, S309: the binary now holds 15 cases; how many of the 15 fail from the root is unmeasured, the three are of 13.]

Depends on S101's verdict, since which cells are live decides what the clamp case can plant.
