id:         S136
goal:       unfreeze tempo, re-derive the truncation guard it was holding at three divisions, refit and resolve it at bounds that can
accepts:    the truncation guard is **re-derived, never relaxed** -- the bound moves from 3 x 23/24 = 2.875 to 4 x 23/24 = 3.833 the moment tempo is non-zero, so `test_eval_model`'s tolerance goes 3 to 4 **from that arithmetic** and the four pinned FENs are re-measured under the new weights with `build/tools/truncation_scan`, DEC-057 having pre-authorised exactly this re-targeting; the non-vacuity clause moves with it -- the per-position assertion must still be a threshold only a position exercising all four divisions can pass, which is `> 2.875` once four can truncate, not the old `> 2.0`; the fit is `--only tempo` so the SPRT measures the term; the label is **WDL-heavy** -- `--lambda 0`, stated rather than defaulted, because a score-blend label cannot teach a term the current evaluator scores at zero; the SPRT bounds resolve single digits and are stated in advance, S027's run having reached neither bound over 3000 games at `--fast` (DEC-063); a verdict of zero is recorded as zero and a second unresolved run is recorded as unresolved again, not converted; the fast suite green
touches:    src/evaluation.cpp tempo_mg/tempo_eg, tests/test_eval_model.cpp (the tolerance, the four pinned FENs and the non-vacuity threshold), the tuner's freeze list, adocs/testing.md, adocs/specs.md
excludes:   piece placement, which is S135; any change to how the tempo feature is computed -- it is the constant 1 for the side to move and there is nothing to change
decisions:  DEC-092, DEC-057, DEC-053, DEC-063
closes:
blocks:
paused_by:
done:

## The state this inherits

**Unresolved, and recorded as unresolved twice now.** S027's SPRT ran the full
3000 games and reached neither bound -- LLR -1.46 against a -2.20 boundary, for
-0.69 +/- 9.64 Elo. What that establishes is that the term is smaller than that
instrument could see, not that it is harmful. S100 confirmed the machinery is
clean: `--only tempo` on a synthetic corpus recovers a planted 23 / 12 exactly,
and the corpus carries both sides to move.

**And S100 measured how much signal there is to fit.** Tempo's entire
contribution to a WDL label is the gap between the mean outcome of
White-to-move and Black-to-move rows. Over 10795695 rows:

```
5437085 rows White to move, mean result 0.556559
5358610 rows Black to move, mean result 0.548682
gap 0.007878, White relative
```

At the corpus mean 0.552649 and K = 0.7624 the sigmoid slope is 0.00108502 per
centipawn, so that gap is a **7.26 cp** score difference between the groups and a
tempo weight near **3.6 cp**. Confounded -- the groups differ in more than whose
move it is, and `datagen` skips in-check positions, which removes rows correlated
with the mover standing worse -- so it is the size of what is available, not a
prediction and not an Elo figure (DEC-019, DEC-084).

Against that, the two existing fits disagree fourfold: S027's `--only` fit gave
mg 10 / eg 0, the unfrozen S065 fit gave mg 39 / eg 21. A term with a few
centipawns of label-side signal and a fourfold disagreement between its own fits
is one to resolve with a better instrument, not a bigger sample of the old one.

## The price, which was derived before it was ever paid

This is the half of `--freeze tempo,piece_placement` whose reason S100 did
**not** void. DEC-057 froze tempo not on a verdict but on a mechanical
consequence DEC-053 had already stated: `evaluate()` taper-divides in integers
and truncates toward zero, and while `tempo_mg == tempo_eg == 0` the tempo
division truncates `0 / 24` exactly and contributes nothing. Three divisions can
round, the model-versus-engine bound is 3 x 23/24 = 2.875, and
`test_eval_model`'s tolerance is 3. Fit tempo and the fourth division rounds
too: bound 3.833, tolerance 4.

It happened once already. S065's first fit put tempo at 39 / 21 and
`ctest -L fast` came back 9 of 12 on exactly this guard and its four pinned FENs.
The paste was reverted and the freeze chosen instead.

**So the guard change is planned, derived and pre-authorised, and it is not a
relaxation.** AGENTS.md section 6 forbids relaxing a test to get green and
DEC-057 records the owner permitting this specific re-targeting for a deliberate
behaviour change. The distinction that has to survive the edit is the
non-vacuity one: the per-position threshold exists so that a position in the
corpus genuinely exercises *every* division that can truncate. At three
divisions that threshold was `> 2.0 = 48/24`, because two divisions cannot reach
past 46/24. At four it becomes `> 2.875 = 69/24`, because three cannot reach past
69/24 -- and the four pinned positions have to be re-drawn from
`truncation_scan`'s output under the new weights, since a residual belongs to the
weights and not to the position (DEC-057, DEC-053).

## Measurement

`--only tempo` for the fit, `--lambda 0` for the label, and bounds that can
resolve a few Elo. DEC-063 is the whole lesson: the hypothesis pair and not the
hardware sets the cost of a verdict, and `--fast` at `elo0=0 elo1=10` is what
returned nothing last time over 3000 games. If the run reaches neither bound
again, that is recorded as unresolved a second time and the term stays at zero --
which is a legitimate outcome and the one S027's row is the template for.
