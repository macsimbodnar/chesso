id:         S136
goal:       unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- three divisions today, S055 having read H0 -- refit and resolve it at bounds that can
accepts:    the truncation guard is **re-derived, never relaxed** -- and the count it is derived from is **read at this step's own HEAD, not taken from this line**: with N the number of taper divisions in `evaluate()` that can round while tempo ships at zero, unfreezing tempo makes it N+1, so the bound goes N x 23/24 to (N+1) x 23/24, the bound `test_eval_trace` holds the smooth score to goes N to N+1 divisions **from that arithmetic** -- since S276 that case derives the bound from the tempo weights itself, so the step checks the derivation rather than editing a number, and moves the pinned case's tempo precondition, which fires once tempo is fitted (S276 deleted `test_eval_model`'s integer tolerance, which this clause named, with the mirror) -- and the non-vacuity threshold goes from "past (N-1) x 23/24" to "past N x 23/24", since that residual is what proves a pinned position exercised every division that can round; **S055 read H0 and was reverted (2026-10-05), so N is 3 at the time of writing and DEC-092's own numbers are the expected ones** -- bound 3 x 23/24 = 2.875 to 4 x 23/24 = 3.833, non-vacuity threshold past 2.875 -- and they are still re-read at this step's HEAD, never copied from this line and never relaxed to match a stale figure (corrected 2026-10-08, DEC-258; the single-division numbers this clause carried assumed an S055 that did not land); the four pinned FENs are re-measured under the new weights with `build/tools/truncation_scan`, DEC-057 having pre-authorised exactly this re-targeting; the fit is `--only tempo` so the SPRT measures the term; the label is **WDL-heavy** -- `--lambda 0`, stated rather than defaulted, because a score-blend label cannot teach a term the current evaluator scores at zero; the SPRT bounds resolve single digits and are stated in advance, S027's run having reached neither bound over 3000 games at `--fast` (DEC-063); a verdict of zero is recorded as zero and a second unresolved run is recorded as unresolved again, not converted; the fast suite green
touches:    phase 1 (2026-10-27): adocs/data/S136_screen.sh, adocs/data/S136_screen.py, adocs/data/S136_guard_forecast.cpp, adocs/data/S136_guard_forecast.txt, adocs/data/S136_sprt.sh, adocs/data/README.md, this file -- nothing under src/ or tools/, the freeze being a command-line convention no file holds (the 2026-10-27 section); phase 2, on a candidate: src/evaluation.cpp tempo_mg/tempo_eg (its tempo comments at the SPRT's closing commit), src/evaluation.hpp (the tempo comment, closing commit), tests/test_eval_trace.cpp (the bound's derivation, the tempo precondition, the per-pin and worst thresholds, the four pinned FENs and their comments), every golden the paste moves, each re-derived by its script (tests/test_evaluation.cpp, tests/test_search.cpp, tests/test_mate_carry.cpp, tests/test_mate_breadth.cpp, tests/test_engine.cpp, adocs/data/S170_cases.tsv, adocs/data/S192_anchors.py, MANUAL.md where it quotes a moved figure), tools/truncation_scan.cpp and tools/tuner_trace.hpp (the usage and comments that state 69/24 as the maximum, closing commit), DEV_MANUAL.md, adocs/data/S136_* (the readings; S136_sprt.log at the closing commit), adocs/specs.md (the coordinator's)
excludes:   piece placement, which is S135; any change to how the tempo feature is computed -- it is the constant 1 for the side to move and there is nothing to change
decisions:  DEC-092, DEC-057, DEC-053, DEC-063, DEC-258, DEC-259, DEC-304, DEC-315, DEC-321, DEC-322
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
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
- What the guard holds today, for the before side (in
  `tests/test_eval_model.cpp` against the mirror until S276, which moved it):
  `tests/test_eval_trace.cpp` "the smooth score is within the truncation bound
  of the exact one" takes its bound from the tempo weights -- 3 x 23/24 while
  both are zero, 4 x 23/24 once either is not -- and `tests/test_eval_trace.cpp`
  "the pinned positions reach the truncation bound" holds the tempo
  precondition message, `CHECK_MESSAGE(difference > 2.0, ...)` per pin and
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
- `tests/test_eval_trace.cpp` "the pinned positions reach the truncation
  bound" is the test whose pins, tempo precondition and non-vacuity threshold
  move, beside the bound "the smooth score is within the truncation bound of
  the exact one" derives from the tempo weights itself, and the arithmetic for
  each is written in the commit message.

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

## S055 read H0 (2026-10-05, the coordinator)

S055's single stage-two taper was reverted on its SPRT (`{-5, 0}` nElo,
nElo -8.78 +/- 6.39), and S117 packs the halves while keeping both stage-two
divisions. So `evaluate()` still tapers three times, and the passages above
that plan for "after S055 lands" describe a tree that does not exist. The
`goal:` line now says three. The `accepts:` field needs no change, because it
reads N at this step's own HEAD.

## 2026-10-27: phase 1

Written by one Opus 5.5 subagent in worktree `chesso-s136`, branch `s136` at
`80520d7` (bench 5069022). Built both trees, ran both fast suites, bench and
the scratch readings below; every stage of the new screen that does not fit
was dry-run into a scratch directory, nothing written under `.tuning/`. **No
fit, match, SPRT, datagen or timing ran.** The one tuner invocation was a
zero-epoch probe on 2000 rows.

### The rulings, as read

1. **DEC-259 governs.** Tempo is a family of one: fitted with every other
   constant frozen, `--only tempo --lambda 0` at DEC-304's settings as S135's
   screen ran them, on S083's corpus and five quarter samples, kept only if
   its gain on S082's fixed set clears DEC-315's m = max(1.645 s, 2e-6).
   Survives: one SPRT. Fails: zero with its figures, no paste, no match, the
   guard unchanged. No bisection exists. **The accepts' "the SPRT bounds
   resolve single digits and are stated in advance" is read as DEC-259 (b)'s
   `{0, 5}` nElo at alpha = beta = 0.05** (the coordinator's ruling,
   2026-10-27), stated with DEC-143's worst case in `adocs/data/S136_sprt.sh`.
   Four readings follow from it, each pre-registered there or in
   `adocs/data/S136_screen.sh`: the candidate is the full-corpus screen fit
   itself, since a family of one has no survivors to refit together and the
   joint fit's command would be the same one; a full fit that rounds to the
   shipped 0 / 0 owes no match (S076's rule 3); no identity stage, S135's
   `audit` having read this corpus at this layout (rank 792 of 831, S134's
   39 null directions, none of them tempo); and **no speed-cost run** --
   DEC-047's reason is a count the compiler drops at zero and keeps
   otherwise, and tempo is a constant whose whole cost is one taper, priced
   inside the verdict.
2. **N is read at this HEAD**, below.
3. **The corpus is S083's**, located by `adocs/data/S135_screen.py` `locate`
   reused by argument, sha256 `2e0e3de9...` pinned as `CORPUS_SHA` in
   `adocs/data/S136_screen.sh`.

### Unfreezing: nothing under src/ or tools/

The freeze is not in any file. DEC-057's `--freeze tempo,piece_placement` was
a command-line convention every fit script from S065 to S083 carried
(`adocs/data/S083_corpus.sh`'s `FIT` the last), and S135's screen held tempo
by `--only piece_placement.<term>`. The tuner refuses nothing:
`tools/tuner_groups.hpp` `free_mask` gives `tempo` its two columns, and a
zero-epoch probe at `80520d7` -- `build/tools/tuner --data <the corpus's first
2000 rows> --only tempo --lambda 0 --epochs 0` -- printed `831 parameters,
group tempo, freeze (nothing), 2 free`, its header `// only tempo` and both
weights 0. `tests/test_tuner_gradient.cpp` "a planted vector comes back
through the trace path" already recovers a planted tempo 23 / 12. So S136's
screen unfreezes tempo by not carrying the freeze, the weights stay 0 in
`src/evaluation.cpp`, and no line under `src/` or `tools/` moves: bench
5069022 in both builds, the parent's; INV-6 needs no `tools/search_bench.py`
run, none having been owed (no `src/` line).

### N, read at 80520d7

`evaluate()` is `evaluate_cheap()` plus `evaluate_expensive()`, and each
`src/evaluation.hpp` `taper` call is one truncating division by
`GAME_PHASE_MAX`. There are four: `src/evaluation.cpp` `evaluate_cheap`'s
`positional` (the tables and the pawn-stage terms summed into one pair) and
its `tempo`; `src/evaluation.cpp` `evaluate_mobility_and_king_safety`'s
`mobility` and `safety`, two because S055 read H0. Material is untapered; the
clamp is 1-Lipschitz and only narrows a gap. `tools/tuner_trace.hpp` `smooth`
drops all four truncations. At `tempo_mg == tempo_eg == 0` tempo's division is
0 / 24, exact, so **N = 3**: the step file's "S055 read H0" section holds and
DEC-092's own numbers are the expected ones. Measured, not assumed:
`adocs/data/S136_guard_forecast.cpp` reads the four pinned rows at +69, -69,
+69, +69 twenty-fourths (smooth minus exact, White relative) -- three
divisions at their largest remainder, the same sign -- and S083's corpus at
most 69/24 (16 rows, 13 positions; 65 rows past 2.8, S135's figure).

### The signal on this corpus, re-read (section 7 asks it)

S100's 3.6 cp was the old corpus's. S135's `audit` read S083's
(`.tuning/s135/audit/report.txt`): 3109435 rows White to move at mean result
0.554406, 3117509 Black to move at 0.572254, **gap -0.017848, White
relative** -- the opposite sign to S100's +0.007878. At this corpus's mean
0.5633 and S135's K = 0.7224 the sigmoid slope is 0.00102293 per centipawn, so
the gap is -17.4 cp between the groups and a tempo weight near **-8.7 cp**.
Confounded as S100's was (and the rows are quiescence leaves now), so it is
the size and sign of what the label offers, not a prediction (DEC-019,
DEC-084). It is why the forecast below reads both signs.

### The guard, before and after -- and what the arithmetic adds

Tempo's numerator `tempo_mg * phase + tempo_eg * (24 - phase)` is congruent
to `(tempo_mg - tempo_eg) * phase` modulo 24, so over the 25 phases its
remainders are exactly the multiples of **g = gcd(tempo_mg - tempo_eg, 24)**:
none but 0 when 24 divides the spread, at most 24 - g otherwise.

| | at `80520d7`, N = 3 | a fit whose tempo division rounds, N + 1 = 4 |
|---|---|---|
| bound, "the smooth score is within the truncation bound of the exact one" | 3 x 23/24 = 2.875 | (69 + 24 - g)/24: 4 x 23/24 = 3.833 at g = 1, 3.792 at g = 2, 3.750 at 3, down to 3.375 at 12 |
| each pin, "the pinned positions reach the truncation bound" | `> 2.0` (48/24, past (N - 1) x 23/24 = 1.917) | `> 69/24 + 1e-6` (past N x 23/24: three divisions cannot reach past it, so a pin past it proves tempo's rounded too) -- DEC-092's 2.875 with room, because rows sit exactly on 69/24 and ulp noise splits them, the 2.0 = 48/24 column's lesson in `DEV_MANUAL.md` (DEC-322) |
| the worst pin, and the bound case's own `worst > 2.8` | `> 2.8`, the maximum 2.875 less 0.075 | the bound less 0.075: 3.758 at g = 1 |
| the tempo precondition | `tempo_unfitted`: both weights zero | tempo's division rounds: `(tempo_mg - tempo_eg) % 24 != 0` |

If the fitted spread is a multiple of 24 -- `4 / 4` would be -- tempo's
division stays exact, N stays 3, the four pins keep their residuals at every
phase and no threshold moves; only the precondition's wording does.

Checked, ruling 2: the bound case does derive its bound from the tempo weights
itself -- `(tempo_unfitted ? 3.0 : 4.0) * 23.0 / 24.0` -- but on "both zero",
not on "the division rounds". That is never false (4 x 23/24 bounds any pair)
but it is exact only at g = 1, 8 spreads of 24, and a fit at `4 / 4` would
fire the precondition and loosen the bound with N still 3. Two things the
derivation in the accepts does not reach: (a) the worst-pin clause and the
bound case's own `worst > 2.8` take no weight, so both have to move by hand,
and moved to 3.833 - 0.075 = 3.758 they are **unreachable for every g >= 3**
(the reach is at most 3.750); (b) the pick, "The re-draw at four divisions"
below. Proposed for phase 2 (DEC draft below): compute g in the test, the
bound `(69 + largest) / 24` with `largest = (spread % 24 == 0) ? 0 : 24 -
std::gcd(spread, 24)` -- equal to today's at zero, never looser than the
literal -- and both worst clauses as `bound - 0.075`. **Nothing relaxes**:
every number is the arithmetic's at the fitted pair, and each is at least as
tight as the literal it replaces.

### What moves in phase 2, and to what

`tests/test_eval_trace.cpp`, on the fitted pair, after its red is observed:

- "the smooth score is within the truncation bound of the exact one": the
  `bound` line (the arithmetic above), `CHECK(worst > 2.8)` to the bound less
  0.075, the comment's "three while tempo ships at zero" and its GOLDEN
  paragraph (`--min 2.875`, the pick below).
- "the pinned positions reach the truncation bound": `CHECK_MESSAGE
  (tempo_unfitted, ...)` to the division-rounds precondition, its message
  naming the three-division numbers to fall back to; `difference > 2.0` to
  `> 69.0 / 24.0 + 1e-6` and its message; `worst > 2.8` to the bound less
  0.075 and its message; the comment block ("2.0 is the threshold the
  arithmetic gives").
- `truncation_positions`: the four re-drawn under the fitted weights, and its
  comment's derivation record (the scan's counts, the pick).

The red forecast, the scan and both picks are printed by
`adocs/data/S136_screen.sh` `apply` before any test line moves. At the H1
closing commit, the comments that state 69/24 as the maximum or tempo at zero
are made true: `src/evaluation.cpp` `evaluate_cheap` ("test_eval_trace's bound
is 3 x 23/24 with tempo at zero") and the block over `tempo_mg`;
`src/evaluation.hpp` `tempo_mg`'s comment; `tools/tuner_trace.hpp`'s header;
`tools/truncation_scan.cpp`'s header and `usage` (default `--min 2.8`, "the
maximum is 69/24 = 2.875"); `DEV_MANUAL.md`'s truncation_scan section, its
"at most 3 x 23/24 = 2.875 while tempo ships at zero" sentence and the two
goldens-table rows for the truncation cases. On H0 or no verdict all of it
reverts with the paste.

### The red expected first -- forecast, and observed on a dry run

**Certain at any non-zero pair: the precondition**, `CHECK_MESSAGE
(tempo_unfitted, ...)` in "the pinned positions reach the truncation bound".
The pins' own clauses depend on the pair: a pin's residual becomes |s3 + t|,
t tempo's signed remainder at its phase and side. With tempo positive the
pins at phases 5 (b), 11 (w) and 17 (b) lose t and the phase 19 (w) pin gains
it, so `> 2.0` goes red on a pin only where t >= 21 there and `worst > 2.8`
cannot (the phase 19 pin stays at 69/24 or more); tempo negative mirrors it.
`adocs/data/S136_screen.py` `guard` prints the exact red for any pair:
S027's 10 / 0 reads 67, 55, 67, 91 twenty-fourths -- only the precondition;
S065's 39 / 21 reads 51, 63, 51, 75 -- only the precondition; 5 / 2 reads 54,
60, 66, 78. For a same-sign pair the bound case stays green: its literal
bound follows the weights, and some pin or mirror stays past 2.8. **A
mixed-sign pair can take every pin under 68/24** -- `guard --pair -10 30`
reads all four at 53/24 -- and then the worst-pin clause goes red and the
bound case's own `worst > 2.8` rests on the shared list's other rows;
`guard` flags both. Beside it, every golden the paste
moves goes red as S135's did, each re-derived by its script (S135's landing
table, `adocs/data/S135_goldens.txt`).

**Observed at a hypothetical pair, 10 / 0**, in a scratch worktree at
`80520d7` (a probe header with `tempo_mg` set to 10, through
`adocs/data/S136_screen.sh` `apply` three times -- the second after its fix
below, the third with the views' scans, 5 m 53 s): the paste moved
`tempo_mg` alone, both builds clean, bench 5069022 -> 4683032, fast suite
**46 of 50 in both builds**: `test_eval_trace` one assertion, exactly the
forecast -- the precondition, 13 of the case's 14 assertions green, the bound
case's 8419 green -- and the goldens `test_evaluation` (the queen
anchor 985 against 984), `test_search` (`black_static` 202 against 199, the
depth-12 capture mate in "pruning does not hide a forced mate") and
`test_mate_carry` (3 of 6 guarded cases reporting, 4 needed: the S170
budgets). That pair is not the fit and those goldens are not this step's
numbers; it shows the forecast and the machinery agree.

### The re-draw at four divisions: S083's pick, and the set it scans

`adocs/data/S083_truncation_pick.py` keeps the scan's hits at its largest
difference and needs two positions of each side there. At three divisions the
maximum is common (13 positions at 69/24 on the corpus); at four it needs a
phase where tempo's remainder is at its largest and a row whose three
divisions agree with it in sign. `adocs/data/S136_guard_forecast.txt` reads
every spread modulo 24, both signs, at `80520d7`:

| set | rows | S083's pick refuses | short of the reach | past 69/24, a side |
|---|---|---|---|---|
| S083's corpus | 6226944 | **43 of 46** non-zero cases | 8 (spreads 1, 2, 11, 13, 22, 23 positive; 1, 23 negative) | 1528 to 15091 |
| S082's three training views | 34774692 | **1** (spread 5 negative) | 4 (spreads 1 and 23, both signs: 91 and 88 against 92) | 6989 to 63934 |

On the corpus the four-division maximum is held by one to five positions,
mostly of one side, at one to three phases. The dry run at 10 / 0 confirmed
the corpus row on the pasted build's own scan: 13995 rows past 2.875, worst
3.791667 (91/24, the reach at g = 2), S083's pick "no hit with b to move for
the target phase 7.00"; over the views the same build's scans read 79842 rows
past 2.875, worst 3.791667, and S083's rule picked four at 3.791667 (phases 7
b, 7 w, 19 b, 19 w, the last the pin S135 drew at phase 19). A residual
belongs to the weights and not to where a position came from, and
`tools/truncation_scan.cpp` reads any FEN file; the corpus is a sample of the
first view.

**Ruled, DEC-322** (the coordinator, 2026-10-27; (3) left open): (1) scan
S082's three training views (`apply` already does,
about 100 s), and pick by S083's rule unchanged -- four pins at the maximum,
DEC-057's "chosen from the hits at the maximum to span the taper with both
sides to move"; (2) where it refuses, `adocs/data/S136_screen.py` `pick`
over the same scan: the distinct positions past 69/24 + 1e-6, the per-pin
threshold's own room, the first at the largest
difference a pin, targets at the lowest phase, the highest and the two thirds
between with sides b, w, b, w, that pin taking its side's nearest target and
each other target the unpicked position of its side nearest in phase, then
largest, then first in scan order (at 10 / 0 over the corpus: 3.4167 at phase
2 b, 3.3750 at 9 w, 2.9583 at 17 b, 3.7917 at 19 w); (3) **not ruled**:
where the views' largest residual falls short of the reach (spreads 1 and 23
modulo 24, either sign), phase 2 stops before the re-targeting and the owner
rules; no worst-clause threshold is pre-chosen. `apply`'s guard reading ends
on `S136_screen.py` `short` over the views' scan, which prints REACHED or a
STOP line, and the stage repeats the STOP before its marker. The options
that ruling has, as phase 1 saw them: the largest found less 0.075 with the
gap recorded (a weaker claim than the reach, which no available row meets);
scanning the fixed validation views too (the set every fit is scored on).
Rejected: S083's rule on the corpus alone (refuses at 43 of 46); hand-chosen
rows.

### Built and run, with figures

- `adocs/data/S136_screen.sh`: the pre-registration and stages `locate`,
  `forecast`, `screen`, `read`, `apply FIT TREE`, each ending on
  `S136-<STAGE>-DONE|FAILED`, OUT `.tuning/s136`. **One stage more than the
  brief listed**: `forecast` (2 m 23 s), so `read` and `apply` print the
  guard's forecast for the fitted pair on both sets.
  `adocs/data/S136_screen.py`: `read`, `start-check`, `only-tempo`, `guard`,
  `forecast`, `pick`, importing
  `adocs/data/S135_screen.py` so the logs parse and the rule's constants are
  S135's. `adocs/data/S136_guard_forecast.cpp` and its reading
  `adocs/data/S136_guard_forecast.txt`. `adocs/data/S136_sprt.sh`.
  `adocs/data/README.md`: the S136 section.
- Dry runs into scratch: `locate` -- the corpus, sha256 `CORPUS_SHA`'s, 2.3 s;
  `forecast` -- 2 m 23 s, the pins as `adocs/data/S136_screen.py` `PINS`
  records them, 139403 corpus rows and 779957 view rows past 46/24; `read`
  over S135's bishop-pair logs renamed -- gain +0.000044, s 0.000007, m
  0.000011, kept, S135's own line
  for that term; `start-check` refusing a frozen value moved and passing tempo
  under `--free`; `apply` at 10 / 0, above (its first run ran the bound case
  from the wrong directory -- "Cannot open assets/test_jsons/castling.json" --
  fixed by running both cases from the test directory, as ctest does;
  re-run, then run a third time with the views' scans). `S136_sprt.sh`
  refuses unpinned, and a dry copy pinned at `56e7e1c` / `80520d7` refuses
  "not S136's paste"; its tempo-lines filter
  passes the dry paste's diff and refuses a synthetic extra line.
- Gates (the end of this section): both builds 0 warnings, both fast suites
  50 of 50, `./clang-format.sh --check` clean, `tools/plan_prose_check.py`
  `--citations`, `--touches`, `--params` and `--gate` each clean.

### Commands for the coordinator, in order, with expected times

From the main checkout once phase 1 has landed; each detached under a
four-exit watcher, its marker the last stdout line.

1. `adocs/data/S136_screen.sh locate` -- under a minute.
2. `adocs/data/S136_screen.sh forecast` -- about 2 min 30 s.
3. `adocs/data/S136_screen.sh screen` -- one full fit and five quarters:
   about 14 min typical, 23 min if every fit runs as long as S135's longest,
   31 min at the 5000-epoch cap (S135's measured 5.3 to 10.7 min a full fit,
   1.3 to 2.2 a quarter, `quarter_s4_trapped` 2 m 09 s), ceiling 1 h.
   Daytime. It ends by printing `read`.
4. `SCREEN-OUTCOME none`: record tempo as zero with its figures; S136
   completes with no paste and no match. `candidate tempo`: go on to 5.
5. A worktree for the paste at HEAD; `adocs/data/S136_screen.sh apply
   .tuning/s136/screen/full_tempo.hpp <worktree>` -- about 8 min (S135's
   5 m 36 s plus about 2 min of scans). Its `guard.txt` is the red forecast
   beside the red observed, then both scans, both picks over each and the
   reach check. A forecast, scan or S136 pick that exits non-zero fails the
   stage. **A STOP line (spreads 1 or 23) stops phase 2 for the owner's
   ruling on DEC-322 (3)** before step 6.
6. Phase 2's implementer: the guard re-targeted by the plan above, the pins
   by DEC-322 (1) and (2), every golden re-derived by its script (S135's
   landing table, about 2 to 3 h), the gates. Tier 2 does not bind: no search,
   generator or make/unmake change.
7. Pin and run `adocs/data/S136_sprt.sh` -- up to 19.9 h (41900 games at 2110
   an hour), 12.1 h on a bound; night by DEC-155.

### Decisions: DEC-321 and DEC-322 recorded, DEC-322 (3) open

Ruled by the coordinator 2026-10-27 and recorded by it in `adocs/decisions.md`;
the text below is the draft they were ruled on, with the per-pin threshold
set at 69/24 + 1e-6.

- **DEC-321** S136 measures tempo as DEC-259's family of one. Decision (the
  coordinator's rulings, 2026-10-27): `--only tempo --lambda 0` at DEC-304's
  settings on S083's corpus and five quarters, kept past DEC-315's m; the
  candidate is the full fit itself; one `{0, 5}` nElo SPRT at 8+0.08, Hash
  16, `noob_3moves.epd`, 20950 rounds; H0 recorded as zero, no verdict as
  unresolved, both reverted; no bisection, no identity stage, no cost run.
  Why: the accepts' "bounds that resolve single digits" is DEC-259 (b)'s
  pair; S135's audit already identifies tempo on this corpus; a constant has
  no count for DEC-047 to time.
- **DEC-322** At four divisions the truncation guard follows the fitted
  pair's arithmetic and picks its pins past 69/24. Decision: with g =
  gcd(tempo_mg - tempo_eg, 24), the bound (69 + 24 - g)/24 (69/24 when 24
  divides the spread), each pin past 69/24 + 1e-6, both worst clauses past the
  bound less 0.075, the precondition "tempo's division rounds"; (1) and (2)
  the pins re-drawn from S082's three training views by S083's pick, by
  `S136_screen.py pick` where that refuses; (3) short of the reach, **not
  ruled**: phase 2 stops before the re-targeting and the owner rules, no
  threshold pre-chosen. Why: DEC-092's 3.833 is exact only at g = 1 and its
  worst clause is unreachable at g >= 3; on S083's corpus S083's pick refuses at
  43 of 46 spreads, on the views at 1. Rejected: the 3.833 literal for every
  pair; S083's pick on the corpus alone; hand-chosen rows.

### Wording proposals

- `adocs/specs.md`: no change at phase 1 (no behaviour moves). On H1, the
  "absent, evaluation" row's opening becomes "S135's bad bishop and its
  rammed share -- present in the code, shipped at zero weight", its
  parameter count "their 4 parameters, making 831 fitted and 827 shipped",
  and the evaluation row gains "Tempo since S136: a bonus for the side to
  move, tapered on its own after the side-to-move sign, fitted `--only tempo`
  on S083's corpus." On H0 or no verdict, no change.
- `MANUAL.md`: checked, no change -- the UCI surface and play are unchanged.
- `DEV_MANUAL.md`: checked, no change at phase 1 -- the tuner's behaviour
  is unchanged and the scripts are documented in `adocs/data/README.md`; the
  phase 2 sentences are listed above.

### The fast check's five fixes (2026-10-27, the same day)

The coordinator accepted phase 1 with five fixes, made before the landing:
(1) the per-pin threshold is 69/24 + 1e-6, not the lattice point 2.875, in
the table, the phase 2 list, DEC-322 and `adocs/data/S136_screen.py`; (2)
the bound case's "stays green" is limited to same-sign pairs, and `guard`
flags the bound case's worst clause when every pin is under 68/24; (3) a
quarter fit is 1.3 to 2.2 min (`quarter_s4_trapped` 2 m 09 s), the totals
14 / 23 / 31 min; (4) `corpus_of`, and so `locate`'s reuse branch, refuses a
`corpus.txt` that does not name `CORPUS_SHA` -- checked on a scratch copy
naming another, `S136-LOCATE-FAILED`; (5) `apply`'s readings fail the stage
when the forecast, a scan or S136's pick exits non-zero, `guard` runs with
`2>&1`, the scans run only where tempo's division rounds, and the views'
reach is checked last (`short`, the STOP line for the unruled DEC-322 (3)).
The readings were re-run through a harness on a scratch tree pasted at
10 / 0 (1 m 54 s): the same pins, picks and thresholds as before, `REACHED`
at 91/24; `short` prints STOP for the same scan at 11 / 0, and compares on
the 1/24 lattice, since the scan's six decimals print 92/24 as 3.833333.
