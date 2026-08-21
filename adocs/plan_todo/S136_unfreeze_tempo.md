id:         S136
goal:       unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- at two divisions once S055 has landed, which the plan orders first -- refit and resolve it at bounds that can
accepts:    the truncation guard is **re-derived, never relaxed** -- and the count it is derived from is **read at this step's own HEAD, not taken from this line**: with N the number of taper divisions in `evaluate()` that can round while tempo ships at zero, unfreezing tempo makes it N+1, so the bound goes N x 23/24 to (N+1) x 23/24, `test_eval_model`'s tolerance goes N to N+1 **from that arithmetic**, and the non-vacuity threshold goes from "past (N-1) x 23/24" to "past N x 23/24", since that residual is what proves a pinned position exercised every division that can round; **the plan orders S055 first (`adocs/plan.md:359` against `adocs/plan.md:370`) and S055 removes a division, so N is 2 and the concrete numbers are** bound 2 x 23/24 = 1.917 to 3 x 23/24 = 2.875, tolerance 2 to 3, per-position threshold `> 1.0` to `> 2.0 = 48/24` because two divisions cannot reach past 46/24 = 1.9167, and the worst-pin `> 1.9` to `> 2.8` -- DEC-092's literal 3.833, tolerance 4 and `> 2.875` are the **pre-S055** numbers and are what applies only if S055 has not landed; the four pinned FENs are re-measured under the new weights with `build/tools/truncation_scan`, DEC-057 having pre-authorised exactly this re-targeting; the fit is `--only tempo` so the SPRT measures the term; the label is **WDL-heavy** -- `--lambda 0`, stated rather than defaulted, because a score-blend label cannot teach a term the current evaluator scores at zero; the SPRT bounds resolve single digits and are stated in advance, S027's run having reached neither bound over 3000 games at `--fast` (DEC-063); a verdict of zero is recorded as zero and a second unresolved run is recorded as unresolved again, not converted; the fast suite green
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
division truncates `0 / 24` exactly and contributes nothing. **As the tree
stands today** three divisions can round, the model-versus-engine bound is
3 x 23/24 = 2.875, and `test_eval_model`'s tolerance is 3; fit tempo and the
fourth division rounds too, bound 3.833, tolerance 4. That is the arithmetic
DEC-092 and the rest of this section were written against, and S055 changes it
before this step runs -- see "One division fewer by the time this runs" below.

It happened once already. S065's first fit put tempo at 39 / 21 and
`ctest -L fast` came back 9 of 12 on exactly this guard and its four pinned FENs.
The paste was reverted and the freeze chosen instead.

**So the guard change is planned, derived and pre-authorised, and it is not a
relaxation.** AGENTS.md section 6 forbids relaxing a test to get green and
DEC-057 records the owner permitting this specific re-targeting for a deliberate
behaviour change. The distinction that has to survive the edit is the
non-vacuity one: the per-position threshold exists so that a position in the
corpus genuinely exercises *every* division that can truncate. The rule is
"past one division short of the bound": at N divisions that can round the
threshold is past (N-1) x 23/24, because N-1 of them cannot reach further than
that between them. Today N is 3 and the threshold is `> 2.0 = 48/24` against
46/24 -- and the four pinned positions have to be re-drawn from
`truncation_scan`'s output under the new weights whatever N turns out to be,
since a residual belongs to the weights and not to the position (DEC-057,
DEC-053).

## One division fewer by the time this runs

S139 re-derived the `accepts:` field above because the plan runs **S055 first**
and S055 removes a taper division. The trace:

- `adocs/plan.md:359` is list entry 39, S055, *"taper mobility and king safety
  through one division instead of two, tightening the model guard's bound to
  2"*; `adocs/plan.md:370` is entry 50, this step.
  First-in-order-not-in-`plan_done/` is how next is derived, so S055 precedes.
- `adocs/plan_todo/S055_taper_stage_two_once.md:3` re-pins both numbers in
  S055's own commit: *"re-pinned to the post-merge bound of 2 x 23/24 = 1.917,
  and the tempo precondition message's arithmetic becomes 3 x 23/24 = 2.875
  with its tolerance of 4 becoming 3"*. Its Technical details give the analog
  thresholds it sets: per-pin `> 1.0`, worst `> 1.9`.
- What the guard holds today, for the before side:
  `tests/test_eval_model.cpp:242-249` names the four divisions and says three
  of them round today, `:277` is `CHECK(std::abs(model - engine_white) <= 3.0)`,
  `:311-314` is the tempo precondition message, `:335` is
  `CHECK_MESSAGE(difference > 2.0, ...)` and `:343` is
  `CHECK_MESSAGE(worst > 2.8, ...)`.

So at this step's HEAD three divisions exist and **two** can round, not three.
Unfreezing tempo makes it three: bound 2 x 23/24 = 1.917 to 3 x 23/24 = 2.875,
tolerance 2 to 3, per-position threshold `> 1.0` to `> 2.0 = 48/24`, worst-pin
`> 1.9` to `> 2.8`. The post-change numbers are today's numbers, which is the
cheap way to check the derivation: S055 lowers the guard by one division and
this step puts that division back with tempo in it.

Taking DEC-092's literals instead would raise the tolerance a full unit above
what the arithmetic supports -- the relaxation DEC-092 itself forbids -- and
set a non-vacuity threshold at 2.875 that no position could reach when the
bound is 2.875, silently disarming the guard. DEC-092 carries an unapplied
**agent proposal** recording this; the literals in its `Decision:` block are
the owner's to move.

**Both goal lines carried the pre-S055 count of three and both are corrected,
2026-08-21 by S140**, which had `adocs/plan.md` and `adocs/plan_todo/` in
`touches:` where S139 did not. This file's `goal:` and `adocs/plan.md:370` now
say the guard is held one division down, at two divisions once S055 has landed.
Neither line is the gate: the `accepts:` field is, and it derives N at this
step's own HEAD rather than reading it off a goal.

## Measurement

`--only tempo` for the fit, `--lambda 0` for the label, and bounds that can
resolve a few Elo. DEC-063 is the whole lesson: the hypothesis pair and not the
hardware sets the cost of a verdict, and `--fast` at `elo0=0 elo1=10` is what
returned nothing last time over 3000 games. If the run reaches neither bound
again, that is recorded as unresolved a second time and the term stays at zero --
which is a legitimate outcome and the one S027's row is the template for.
