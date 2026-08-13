id:         S038
goal:       the tuner-model guard states a tolerance the truncation arithmetic actually supports
accepts:    test_eval_model passes over a corpus that includes the FENs recorded in the audit finding; the tolerance and its comment agree with the number of integer divisions evaluate() actually performs
touches:    tests/test_eval_model.cpp, and src/evaluation.cpp plus tools/eval_model.hpp if the third truncation is removed instead
excludes:   any change to what the evaluation computes -- only how many times it truncates, and only if that route is chosen
decisions:
closes:     2026-08-13_adversarial-F04
blocks:
paused_by:
done:

## What is broken

`tests/test_eval_model.cpp:313` asserts `std::abs(model - engine_white) <= 2.0`,
with a comment saying `evaluate()` truncates "twice, once for the tables and
once for mobility". It truncates three times: `src/evaluation.cpp:638`
(piece-square plus pawn terms), then `:949` and `:951`, where mobility and king
safety are tapered by two separate integer divisions before being summed. Three
truncations of a `/24` bound the disagreement at 3 x 23/24 = 2.875.

The test passes only because its corpus is 26 hand-picked FENs. Over the real
tuning corpus, 3446 of 200000 positions exceed 2 cp, worst exactly 2.875:

```
positions 200000 beyond 2cp slack 3446 worst 2.875
worst fen 2r1r1k1/4Q1p1/p1P1p1q1/3p3p/1P1PpP2/4P2P/PB4P1/R4RK1 w - - 5 29
```

## What is not broken

The model itself. The clamp sits in the same place in both
(`tools/eval_model.hpp:976` against `src/evaluation.cpp:982`), colour symmetry
holds over 300000 positions, and the worst observed difference is exactly the
truncation bound rather than anything larger. What is wrong is the guard.

## Why it matters at all

This is the only thing stopping `tools/eval_model.hpp` drifting from
`evaluate()`, and a fit is only as good as that correspondence. At 2 cp against
a legitimate 2.875 cp of truncation the tolerance cannot separate rounding from
a real model error of up to about 2.8 cp per position — which, over 1.49 M
positions, is a systematic bias the fit absorbs happily and silently.

It also trains the wrong reflex: extending the corpus toward real play makes the
test fail for a reason unconnected to any defect, and the obvious response is to
widen the tolerance rather than read it.

## Two routes

- Raise the slack to 3 and correct the comment to name the three divisions.
- Or taper mobility and king safety as one summed pair, which removes the third
  division, puts the bound back to 2, and is one integer division cheaper on the
  hot path. This changes evaluation output, so it is an SPRT.

The corpus this was measured on is gitignored, so whichever route is taken, pin
two or three of the audit's FENs into the test rather than depending on
`.tuning/`.
