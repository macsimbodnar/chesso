id:         S136
goal:       unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- at two divisions once S055 has landed, which the plan orders first -- refit and resolve it at bounds that can
accepts:    the truncation guard is **re-derived, never relaxed** -- and the count it is derived from is **read at this step's own HEAD, not taken from this line**: with N the number of taper divisions in `evaluate()` that can round while tempo ships at zero, unfreezing tempo makes it N+1, so the bound goes N x 23/24 to (N+1) x 23/24, `test_eval_model`'s tolerance goes N to N+1 **from that arithmetic**, and the non-vacuity threshold goes from "past (N-1) x 23/24" to "past N x 23/24", since that residual is what proves a pinned position exercised every division that can round; **the plan orders S055 first (`adocs/plan.md` "taper mobility and king safety through one division instead of two" against `adocs/plan.md` "unfreeze tempo, re-derive the truncation guard its zero weight holds") and S055 removes a division, so N is 2 and the concrete numbers are** bound 2 x 23/24 = 1.917 to 3 x 23/24 = 2.875, tolerance 2 to 3, per-position threshold `> 1.0` to `> 2.0 = 48/24` because two divisions cannot reach past 46/24 = 1.9167, and the worst-pin `> 1.9` to `> 2.8` -- DEC-092's literal 3.833, tolerance 4 and `> 2.875` are the **pre-S055** numbers and are what applies only if S055 has not landed; the four pinned FENs are re-measured under the new weights with `build/tools/truncation_scan`, DEC-057 having pre-authorised exactly this re-targeting; the fit is `--only tempo` so the SPRT measures the term; the label is **WDL-heavy** -- `--lambda 0`, stated rather than defaulted, because a score-blend label cannot teach a term the current evaluator scores at zero; the SPRT bounds resolve single digits and are stated in advance, S027's run having reached neither bound over 3000 games at `--fast` (DEC-063); a verdict of zero is recorded as zero and a second unresolved run is recorded as unresolved again, not converted; the fast suite green
touches:    src/evaluation.cpp tempo_mg/tempo_eg, tests/test_eval_model.cpp (the tolerance, the four pinned FENs and the non-vacuity threshold), the tuner's freeze list, adocs/specs.md
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

- `adocs/plan.md` "taper mobility and king safety through one division instead
  of two" is list entry 39, S055, *"taper mobility and king safety through one
  division instead of two, tightening the model guard's bound to 2"*;
  `adocs/plan.md` "unfreeze tempo, re-derive the truncation guard its zero
  weight holds" is entry 50, this step. First-in-order-not-in-`plan_done/` is
  how next is derived, so S055 precedes.
- `adocs/plan_todo/S055_taper_stage_two_once.md` "re-pinned to the post-merge
  bound of 2 x 23/24 = 1.917" re-pins both numbers in S055's own commit:
  *"re-pinned to the post-merge bound of 2 x 23/24 = 1.917, and the tempo
  precondition message's arithmetic becomes 3 x 23/24 = 2.875 with its
  tolerance of 4 becoming 3"*. Its Technical details give the analog thresholds
  it sets: per-pin `> 1.0`, worst `> 1.9`.
- What the guard holds today, for the before side: `tests/test_eval_model.cpp`
  "the model reproduces evaluate() on every phase" names the four divisions and
  says three of them round today, `:277` is `CHECK(std::abs(model -
  engine_white) <= 3.0)`, `:311-314` is the tempo precondition message, `:335`
  is `CHECK_MESSAGE(difference > 2.0, ...)` and `:343` is `CHECK_MESSAGE(worst
  > 2.8, ...)`.

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
`touches:` where S139 did not. This file's `goal:` and `adocs/plan.md`
"unfreeze tempo, re-derive the truncation guard its zero weight holds" now say
the guard is held one division down, at two divisions once S055 has landed.
Neither line is the gate: the `accepts:` field is, and it derives N at this
step's own HEAD rather than reading it off a goal.

## Measurement

`--only tempo` for the fit, `--lambda 0` for the label, and bounds that can
resolve a few Elo. DEC-063 is the whole lesson: the hypothesis pair and not the
hardware sets the cost of a verdict, and `--fast` at `elo0=0 elo1=10` is what
returned nothing last time over 3000 games. If the run reaches neither bound
again, that is recorded as unresolved a second time and the term stays at zero --
which is a legitimate outcome and the one S027's row is the template for.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki defines the term and states no figure.** CPW *Tempo*
(https://www.chessprogramming.org/Tempo, fetched 2026-09-13): "To avoid score
oscillations on the parity of the search depth, some programs give a small
bonus for having the right to move", and such a bonus is "useful mainly in the
opening and middle game positions, but can be counterproductive in the
endgame". **No centipawn value and no Elo figure anywhere on the page.**
chesso's form is exactly the page's -- a constant 1 for the side to move,
tapered -- so there is no departure to state, and the page's endgame clause is
the published reason to keep the middlegame and endgame halves separate rather
than fit one number, which chesso already does.

**One traced figure, and it is the right one.** Weiss pull request #241,
"Tempo", 2020-04-11: **+12.99 +/- 7.35** at 10.0+0.1s and **+6.67 +/- 4.67** at
40.0+0.4s (https://github.com/TerjeKir/weiss/pull/241). The 2026-09-04
literature check established that this is where the plan's "+12.99" comes
from and that the plan had it attached to the wrong feature -- it was written
as a rook-on-the-seventh figure (row A28). It is a *tempo* figure and it prices
adding the term from nothing at Weiss's band, which is not what this step does:
chesso already computes the feature and holds its weight at zero, so what is
being measured here is the weight, not the term.

**The published size and this engine's own measured signal disagree by a
factor of three, and the local number is the one that binds.** S100's corpus
measurement -- 5437085 White-to-move rows at mean result 0.556559 against
5358610 Black-to-move rows at 0.548682, a gap of 0.007878, which at the corpus
mean and `K` = 0.7624 is 7.26 cp of score and a tempo weight near 3.6 cp -- is
this project's own data and is confounded as that paragraph says. Nothing in
the published record contradicts it or supports it; the two are measurements of
different things (an Elo delta at one engine against a label-side gap here).

**Nothing published addresses the truncation interaction at all**, and it is
the reason this step is hard here. The guard, the four pinned FENs and the
non-vacuity threshold are chesso's own machinery from DEC-053 and DEC-057.
Searched this pass: CPW *Tapered Eval*
(https://www.chessprogramming.org/Tapered_Eval, fetched 2026-09-13) gives the
interpolation as "eval = ((opening * (256 - phase)) + (endgame * phase)) /
256" -- one division over the summed score, with the phase itself divided once
more -- and **says nothing about integer truncation, rounding or remainders**.
So the published form is the one-division shape S055 moves toward, and the
per-term divisions chesso still carries are this engine's own history, not a
published choice.

### 2. Shape for chesso

Nothing about the feature changes. `src/evaluation.cpp` `tempo_mg` and
`src/evaluation.cpp` `tempo_eg` are two constants; the work is the freeze list,
the label, the guard arithmetic and the bounds.

The guard arithmetic is derived in "One division fewer by the time this runs"
above and **is re-derived at this step's own HEAD**, from the number of taper
divisions that can round when the step starts. That derivation is the
accepts' and nothing in this enrichment moves it.

### 3. Implementation sketch

- `--only tempo` for the fit, `--lambda 0` for the label, both stated rather
  than defaulted.
- `build/tools/truncation_scan` re-drawn under the new weights; the four pins
  come out of its output, because a residual belongs to the weights and not to
  the position (DEC-057, DEC-053).
- `tests/test_eval_model.cpp` "the model reproduces evaluate() on every phase"
  is the test whose tolerance, pins and non-vacuity threshold move, and the
  arithmetic for each is written in the commit message.

### 4. Constants and seeds

**No seed, and this is the clearest case in block 3 for why.** Weiss's +12.99
is an Elo figure, not a constant; that engine's shipped tempo bonus is its
tuned output and is not a seed here wherever it is republished (DEC-105,
DEC-134). The two candidate values this project already has -- S027's
`--only` fit at mg 10 / eg 0 and the unfrozen S065 fit at mg 39 / eg 21 -- are
its own fits, and the fourfold disagreement between them is the reason the
step exists rather than a range to split.

The guard's three numbers are **(b) derived**, from the division count at this
step's HEAD, and the derivation is written out in the accepts. They are not
tuning constants and they are never chosen to make a test pass.

### 5. Pitfalls

- **The relaxation trap.** DEC-092's literals are the pre-S055 numbers. Taking
  them after S055 lands raises the tolerance a full unit above what the
  arithmetic supports and sets a non-vacuity threshold no position can reach,
  silently disarming the guard. AGENTS.md's TESTS rule forbids relaxing a test
  and this is what relaxing it would look like from inside.
- **It already broke once.** S065's first fit put tempo at 39 / 21 and `ctest
  -L fast` came back 9 of 12 on this guard. The guard change is planned and
  pre-authorised (DEC-057); an unplanned one is the failure.
- **A second unresolved run is unresolved, not zero.** S027's run reached
  neither bound over 3000 games. DEC-063 is the lesson: the pair sets the cost.
  If the properly-bounded run also fails to resolve, that is recorded again and
  the term stays at zero.
- **`--lambda` defaults matter.** A score-blend label cannot teach a term the
  current evaluator scores at zero; the accepts states `--lambda 0` for exactly
  that reason and the run records the flag it used.

### 6. Measurement

One SPRT at bounds chosen to resolve single digits, stated in advance with the
nElo worst case and the abort rule (DEC-143). The fit is minutes. The
truncation scan is seconds. The guard's red is observed before the
re-targeting lands, not assumed.

### 7. Interactions

- **S055 (before, by plan order)**: removes a taper division and re-pins both
  numbers. This step puts one division back with tempo in it, which is the
  cheap check on the derivation.
- **S135 (beside)**: the other half of the freeze. Separate fit, separate
  verdict.
- **S082 and S083 (before)**: the corpus. Tempo's label-side signal is measured
  on the corpus, so a corpus change changes the 3.6 cp figure and the step
  re-reads it rather than quoting S100's.
- **S126 (block end)**: refits everything including tempo, and its early
  stopping is on the held-out split.

### 8. References

- - https://github.com/TerjeKir/weiss/pull/241 -- "Tempo", 2020-04-11,
  +12.99 +/- 7.35 at 10.0+0.1s and +6.67 +/- 4.67 at 40.0+0.4s. Pull-request
  body only. This is the figure the plan had attached to rook-on-the-seventh.
- - https://www.chessprogramming.org/Tempo -- the side-to-move bonus as a small
  constant; **no Elo figure**.
- - https://www.chessprogramming.org/Tapered_Eval -- phase interpolation; says
  nothing about integer truncation or division count. Fetched 2026-09-13.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` row A28 -- where the
  +12.99 misattribution was found.
