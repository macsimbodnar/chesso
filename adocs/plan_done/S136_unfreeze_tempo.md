id:         S136
goal:       unfreeze tempo, re-derive the truncation guard its zero weight holds one division down -- three divisions today, S055 having read H0 -- refit and resolve it at bounds that can
accepts:    the truncation guard is **re-derived, never relaxed** -- and the count it is derived from is **read at this step's own HEAD, not taken from this line**: with N the number of taper divisions in `evaluate()` that can round while tempo ships at zero, unfreezing tempo makes it N+1, so the bound goes N x 23/24 to (N+1) x 23/24, the bound `test_eval_trace` holds the smooth score to goes N to N+1 divisions **from that arithmetic** -- since S276 that case derives the bound from the tempo weights itself, so the step checks the derivation rather than editing a number, and moves the pinned case's tempo precondition, which fires once tempo is fitted (S276 deleted `test_eval_model`'s integer tolerance, which this clause named, with the mirror) -- and the non-vacuity threshold goes from "past (N-1) x 23/24" to "past N x 23/24", since that residual is what proves a pinned position exercised every division that can round; **S055 read H0 and was reverted (2026-10-05), so N is 3 at the time of writing and DEC-092's own numbers are the expected ones** -- bound 3 x 23/24 = 2.875 to 4 x 23/24 = 3.833, non-vacuity threshold past 2.875 -- and they are still re-read at this step's HEAD, never copied from this line and never relaxed to match a stale figure (corrected 2026-10-08, DEC-258; the single-division numbers this clause carried assumed an S055 that did not land); the four pinned FENs are re-measured under the new weights with `build/tools/truncation_scan`, DEC-057 having pre-authorised exactly this re-targeting; the fit is `--only tempo` so the SPRT measures the term; the label is **WDL-heavy** -- `--lambda 0`, stated rather than defaulted, because a score-blend label cannot teach a term the current evaluator scores at zero; the SPRT bounds resolve single digits and are stated in advance, S027's run having reached neither bound over 3000 games at `--fast` (DEC-063); a verdict of zero is recorded as zero and a second unresolved run is recorded as unresolved again, not converted; the fast suite green
touches:    phase 1 (2026-10-27, landed): adocs/data/S136_screen.sh, adocs/data/S136_screen.py, adocs/data/S136_guard_forecast.cpp, adocs/data/S136_guard_forecast.txt, adocs/data/S136_sprt.sh, adocs/data/README.md, this file -- nothing under src/ or tools/, the freeze being a command-line convention no file holds (the 2026-10-27 section); phases 2 to 2c and the close (2026-10-27 to 2026-10-28, landed by the close's record commit, documents only): adocs/data/S136_sprt.sh (pinned and re-launched), adocs/data/S136_floor.txt, adocs/data/S136_overlay_run.py, adocs/data/S136_mined_rows.py, adocs/data/S136_lost_mates.txt, adocs/data/S136_lost_mates.py, adocs/data/S136_uci_go.py, adocs/data/S136_trace_patch.py, adocs/data/S136_trace_line.py, adocs/data/S136_line_cut.py, adocs/data/S136_goldens.txt, adocs/data/S136_sweep_s170.txt, adocs/data/S136_candidate.diff, adocs/data/S136_sprt.log, adocs/data/S136_sprt_mate_pvs.txt, adocs/data/README.md, this file; adocs/specs.md (the coordinator's) -- **nothing under src/, tests/ or tools/: the candidate read H0 and reverted whole**; measured on that candidate and never landed, recorded in adocs/data/S136_candidate.diff: src/evaluation.cpp (tempo_mg / tempo_eg 0 / 0 -> 10 / 1, whole-line comments), tests/test_eval_trace.cpp (the guard by DEC-322), tests/test_evaluation.cpp, tests/test_search.cpp, tests/test_mate_carry.cpp, tests/test_eval_model.cpp (a comment), tools/tuner_trace.hpp, tools/truncation_scan.cpp, adocs/data/S170_cases.tsv, adocs/data/S192_anchors.py, DEV_MANUAL.md, MANUAL.md
excludes:   piece placement, which is S135; any change to how the tempo feature is computed -- it is the constant 1 for the side to move and there is nothing to change
decisions:  DEC-092, DEC-057, DEC-053, DEC-063, DEC-258, DEC-259, DEC-304, DEC-315, DEC-321, DEC-322, DEC-323, DEC-325, DEC-254, DEC-319, DEC-320, DEC-122, DEC-127, DEC-162, DEC-220
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-28 10:23 -- H0 at {0, 5} nElo, 8+0.08, Hash 16, CAND ba65b86+ea1eb6cfad0b (the working tree of ../chesso-s136b: tempo fitted alone, mg 10 / eg 1) vs REF ba65b86 (tempo 0 / 0): Elo -0.66 +/- 3.44, nElo -0.90 +/- 4.70, LLR -2.96, 21014 games, 9 h 46 m, 0 forfeits (adocs/data/S136_sprt.log, archived with the void first launch as chesso_S136_sprt_2026-10-28.tar.xz); the first launch (2026-10-27 13:15, 6520 games, SIGINT for a shutdown) void, never pooled; the banner as pinned, both binaries at their bench (6007564, the candidate rebuilt byte-identical; 5069022), no crash or disconnect, quarters +0.5, +0.7, -0.4, -3.5, kernel journal clean; five incomplete mating PVs, three the candidate's in two games and two the reference's in one, each a line shorter than its distance (DEC-122's class, DEC-162) and none a distance shorter than Stockfish's at 5000000 nodes, none reproduced cold or warm (adocs/data/S136_sprt_mate_pvs.txt); so by the pre-registered H0 row (DEC-321 (3), DEC-323 (4)) tempo is recorded as zero, the paste reverts whole and is kept as adocs/data/S136_candidate.diff (blob ea1eb6cfad0b, DEC-254), EXACT_FLOOR stays 159 (DEC-325 moot) and the guard keeps its tempo-zero form; unmeasured: tempo at any other pair, and the -8.7 cp side-to-move gap on S083's corpus, confounded. Implemented by one Opus 5.5 subagent per phase (phase 1 pre-registered, DEC-321 and DEC-322; phase 2 the screen kept tempo, gain +0.000031 against m 0.000002, the paste stopped at EXACT_FLOOR 156; phase 2b the lost mates attributed, none unpriced; phase 2c the guard and every golden re-derived, DEC-323), fast-checked; the run the coordinator's; inspected, recorded and stamped by one Opus 5.5 subagent (both fast suites 50/50, bench 5069022, no src/ change).

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

## 2026-10-27: phase 2 -- stopped at EXACT_FLOOR

One Opus 5.5 subagent, worktree `chesso-s136b`, branch `s136b` at `ba65b86`,
on the coordinator's paste. **Stopped before any test line moves**: the
candidate's mined-set count falls under `EXACT_FLOOR`, and no floor that
DEC-299's guard allows passes it (the brief's stop rule). Under src/ and
tests/ nothing is edited past the paste. Written: this section,
`adocs/data/S136_floor.txt` (each command beside its output),
`adocs/data/S136_overlay_run.py`, `adocs/data/S136_mined_rows.py`, their rows
in `adocs/data/README.md`. No fit, match, SPRT, datagen or timing ran.

### The screen and the paste, as read

`.tuning/s136/screen/reading.txt`: baseline held-out 0.071732 at the shipped
constants, K 0.7171, 6226944 rows; tempo gain **+0.000031**, s 0.000001, m
0.000002, kept; quarters +0.000031, +0.000031, +0.000031, +0.000033,
+0.000030; fitted pairs full 10 / 1, quarters 9 / 2, 10 / 1, 10 / 2, 9 / 1,
10 / 1. The candidate `.tuning/s136/screen/full_tempo.hpp`: **tempo_mg 10,
tempo_eg 1**, error 0.069179 train and 0.071701 validation, engine stamp
`80520d7-dirty` (src/ is `ba65b86`'s). The guard at the pair: spread 9, g 3,
largest remainder 21/24, bound (69 + 21)/24 = 3.75. The views reach it
(`REACHED` at 90/24), so DEC-322 (3) does not arise. The apply
(`.tuning/s136_apply.console`, `.tuning/s136/apply_full/`) moved the two tempo
definitions alone. Bench went 5069022 -> **6007564**, read again here in
both builds.

### The red, observed

Both builds, `ctest -L fast` with `CLANG_FORMAT_MAJOR=22`: **46 of 50, the
same four in each**.

- `test_eval_trace`, "the pinned positions reach the truncation bound": 2 of
  its 14 assertions red, exactly `guard.txt`'s forecast. One is
  `CHECK_MESSAGE(tempo_unfitted)`. The other is `difference > 2.0` on the
  phase 5 b pin (`2R5/8/3r3p/6p1/1k2B1P1/4K2P/5P2/8 b - - 16 52`, 2.000000 =
  48/24). The other three pins read 66, 60 and 72 twenty-fourths, and the
  worst, 72/24, passes 2.8. "The smooth score is within the truncation bound"
  is green: 8419 assertions at its literal 4 x 23/24. Whole file: 1214578 of
  1214580.
- `test_evaluation`, "each piece is worth what the tables say": `pawn on e2`
  reads 100 against 99. It is a REQUIRE, so the first anchor ends the case.
- `test_search`, 12 cases: `QUIET_ROOK_EVAL` 584 against 583 (eight cases),
  `QUIET_ROOK_EVAL_CHEAP` 579 against 578, `black_static` 203 against 199,
  the quiet evasion's `quiesce` -534 against -533, and the repetition case's
  `evaluate` -596 against -597.
- `test_mate_breadth`: `found_exact >= EXACT_FLOOR`, **156 >= 159**.

Green at this pair, though red at the phase 1 dry run's 10 / 0:
`test_mate_carry` (96 of 96). `test_engine` (`MATE_IN_THREE_FLOOR`, the ply-2
clause, first-mate) is green.

### The two floors, read by their scripts

The parent is `ba65b86` and the candidate is the paste. The sweeps measure
commits, so the candidate's run through `adocs/data/S136_overlay_run.py`. It
copies the working tree's `src/evaluation.cpp` into the throwaway worktree
right after `git worktree add`, which is S154's own `overlay` argument
(`adocs/data/S136_floor.txt` has every command).

| | parent: shipping, 2, weakened | candidate: shipping, 2, weakened | gate built at RfpMinPly 1 |
|---|---|---|---|
| `EXACT_FLOOR` 159, `S156_mined_floor_sweep.py --floor 159` | 160, 161, 159, as S296's | **156, 157, 156**: gap 0 | candidate red at 156, its shipping build red too; parent green at 159, not separating since DEC-319 (5) |
| `MATE_IN_THREE_FLOOR` 11, `S154_floor_margin_sweep.py floor` and `red` | 13, 13, 8 | **13, 12, 10** | candidate red at 10 >= 11 |

The wrong-sign column is 0 at every setting in both trees.

**Against DEC-299** ("both mate floors hold or rise"):
`MATE_IN_THREE_FLOOR` holds at 11 and still separates (13 against 10).
`EXACT_FLOOR` does not hold. The candidate's shipping build finds 156, so a
floor that passes it is 156 or lower, a fall of at least 3. With both ends at
156, S156's band (weakened, shipping] is empty, so no value separates. A
floor at 155 would be DEC-313 (1)'s one-below rule and 156 would have no
margin; both fall, so both are the owner's.

**The churn behind the count** (`adocs/data/S136_mined_rows.py`, one line a
row, the parent read on run B's kept tune build): at depth 10, **16 of the
318 rows differ**. The candidate loses 10 of the parent's exact rows (89, 105,
107, 158, 182, 185, 210, 225, 247, 256; row 185 reads mate 7 for a labelled
6, the other nine a cp score) and gains 6 (86, 116, 117, 155, 163, 216):
160 - 10 + 6 = 156. On those sixteen at depth 12 the parent reads 15 exact
and the candidate 14: 8 of the 10 come back and 158 and 182 do not, and the
parent still misses its 86. At depth 14 both read 15. The candidate misses
only row 158 (`6k1/4rp2/8/p1Q3K1/7P/8/8/8 w - - 7 53`, labelled mate in 5, cp
1966) and the parent only row 86 (labelled mate in 4, cp 1613). These are
the engine's own reported scores, no chess judgement. On these sixteen rows
the depth-10 difference is gone by depth 14 (15 against 15). The full set was
read at depth 10 only.

### For the coordinator's question (DEC-265 form; the owner rules)

Precedent: S083's refit read `EXACT_FLOOR` red. Its SPRT ran as a
working-tree candidate with the reds left in (DEC-307 (4), DEC-308 (3)), and
on H1 the owner lowered the floor to one below the shipping count (DEC-313
(1)). Here, `adocs/data/S136_sprt.sh` pins a committed CAND whose parent is
REF, and every commit is green. So the ruling comes before the candidate
commit, or the run moves to DEC-254's working-tree form. The options as read:

(a) `EXACT_FLOOR` 159 -> 155 with the paste: DEC-313 (1)'s rule, recorded as
not separating. `MATE_IN_THREE_FLOOR` alone separates, 13 against 10, as it
already does since DEC-319 (5). It leaves with the paste on H0 or no verdict.
(b) S083's form: the working-tree SPRT with the red, and the owner rules on
H1.
(c) The floor binds: no match, against DEC-321's "a survivor plays one SPRT".

### Not done, the step having stopped

Everything after the red: the guard re-targeting by DEC-322 (1) and (2);
every other golden (the anchors by `S192_anchors.py`, the `test_search`
scores, both floors' landing, the S170 ceilings and budgets by
`S203_case_sweep.sh --ceilings` and `S170_pick_budgets.sh`, first-mate, the
ply-2 clause, the mined rows, the node budget, the clamp and the walk, per
S135's table); the comments; the documents; the gates over a re-targeted
tree; the proposals. Ready for a resume:

- the guard by the arithmetic: the bound (69 + 21)/24 = 3.75, from 3.833;
  both worst clauses 2.8 -> 3.675; the per-pin `> 2.0` -> `> 69.0 / 24.0 +
  1e-6`; the precondition `(tempo_mg - tempo_eg) % 24 != 0`;
- the pins, by S083's pick over the views as `guard.txt` prints it (S083's
  rule does not refuse there, so DEC-322 (1) applies): all four at 3.750000,
  phases 5 b `8/6pp/1p1bk3/2p4P/P1R5/4P1P1/r4PK1/8 b - - 0 37`, 5 w
  `8/8/1k6/7r/8/R7/B4PK1/8 w - - 0 60`, 13 b
  `4r1k1/2b2pp1/5n2/2Np4/1P1Pp2p/2P3P1/rB2R1KP/2N4R b - - 1 32`, 13 w
  `2r2b1r/4nk2/6p1/p3RnB1/2PN1R1P/Pp6/1P3PK1/8 w - - 2 33`.

### Phase 2b (2026-10-27): the lost mates attributed by tool

The coordinator accepted the stop. Before any SPRT, the lost mates get DEC-305
and DEC-308 (2)'s treatment: attributed by tool, because a rule defect's fix
would change the tree the verdict measures. The guard and the goldens stay
untouched. Same worktree; parent and traced builds in throwaway worktrees of
`ba65b86`, removed after. Every reading with its command is in
`adocs/data/S136_lost_mates.txt`. No fit, match, SPRT, datagen or timing ran.

**First exact iteration** (one `go depth 16`, 21 for a row still missing; iteration
d equals a fresh `go depth d`, 96 of 96 against the phase 2 readings):

| row | label | parent | candidate | |
|---|---|---|---|---|
| 89, 105, 107 | m4 | 10, 10, 10 | 12, 11, 12 | lost |
| 158 | m5 | 10 | **17** | lost |
| 182 | m6 | 10 | **13** | lost |
| 185, 210, 225 | m6 | 10, 10, 10 | 11, 11, 12 | lost |
| 247, 256 | m7 | 10, 10 | 12, 11 | lost |
| 86 | m4 | **19** | 8 | gained |
| 116, 117, 155, 163, 216 | m4, m4, m5, m5, m6 | 11, 11, 11, 12, 12 | 10, 10, 10, 10, 9 | gained |

**The oracle** (Stockfish at 5000000 nodes, DEC-319 (3)): all 16 rows read
their label's mate, 158 `#5` and 86 `#4` among them. DEC-320's deeper reading
did not arise.

**The traces.** Row 158 and row 182 are lost the same way, the shape DEC-319
(2) recorded for S145 root 76.
- The paste reorders a PV node: at 158's ply 2 e7e5 comes first and h4h5 third;
  at 182's ply 4 c3d4 comes first and c3b3 second.
- So the mating move is scouted. One ply down, the mated side's non-PV cut node
  cuts it.
- Row 158: by the null move at depths 10 to 13, then by reverse futility at 14
  to 16.
- Row 182: by the null move in iteration 9, which moves the ordering, then by
  reverse futility at 9 to 12.
- The table entry at each node is exact and outside the mate band (-1228 and
  -1380, depth 4, from a whole search), so nothing there proves mated and
  neither S289's nor S293's guard applies.
- With the null move off at that one key, both rows report the label from the
  parent's depth 10. With reverse futility off there alone, 158 stays lost and
  182 comes at 13.

**Reach at the shipped weights** (DEC-308 (2)): over the mined set at depth 10,
the parent with reverse futility off reads 199 against 160 (+42 -3), and with
the null move off 161 (+8 -7). S296 recorded the same two rules at root 76 on
the constructed set. So the shipped tree loses mates to both rules already,
which is DEC-158's and DEC-087 (e)'s price.

**Classes**:
- (i) depth churn: 89, 105, 107, 185, 210, 225, 247, 256. The candidate reports
  the label 1 or 2 plies after the parent. The walks of the parent's lines
  (`S136_line_cut.py`) point at reverse futility for 89 and 105; they are
  pointers, not traces.
- (ii) priced: 158 and 182, DEC-087 (e) and DEC-158 as DEC-319 (2) reads them.
- (iii): **none**.

Written: `adocs/data/S136_lost_mates.txt` and five scripts.
- `S136_lost_mates.py`: `depths`, `oracle`.
- `S136_uci_go.py`.
- `S136_trace_patch.py`: S295's patch at S296's loop return, and `--keys`.
- `S136_trace_line.py`.
- `S136_line_cut.py`.

Their rows are in `adocs/data/README.md`.

### Phase 2c (2026-10-27): the guard re-targeted and the goldens re-derived, DEC-323

The coordinator accepted phase 2b and ruled DEC-323, S083's form:
- every golden inside DEC-299's guard is re-derived by its own script;
- `EXACT_FLOOR` stays at 159 and red, for the owner, asked only on H1 (option
  (a) recommended);
- the SPRT plays this worktree's uncommitted tree against its HEAD.

Same worktree. Every reading with its command is in
`adocs/data/S136_goldens.txt`. No fit, match, SPRT, datagen or timing ran.

**The guard** (`tests/test_eval_trace.cpp`, DEC-322 (1) and (2) at 10 / 1:
spread 9, g 3):

| line | old | new |
|---|---|---|
| the bound | `(tempo_unfitted ? 3.0 : 4.0) * 23.0 / 24.0`, 3.833 here | `truncation_bound()`, `(69 + 24 - g)/24`, 69/24 when 24 divides the spread: **3.75** |
| the bound case's `worst` | `> 2.8` | `> bound - 0.075` = **3.675** |
| each pin | `> 2.0` | `> 69.0 / 24.0 + 1e-6` |
| the worst pin | `> 2.8` | `> bound - 0.075` = **3.675** |
| the precondition | `tempo_unfitted` (both zero) | `(tempo_mg - tempo_eg) % 24 != 0`; its message names the three-division numbers (69/24, 48/24, 2.8) |
| the four pins | S135's: phases 5 b, 11 w, 17 b, 19 w (now 48, 66, 60, 72 /24) | S083's pick over S082's three training views, all at 3.75: phases 5 b, 5 w, 13 b, 13 w |

Notes on the guard:
- The views' scan was re-run on this build, and its output is byte-identical
  to the apply stage's: 80801 hits past 2.875, 16 distinct at 3.75.
- On the corpus alone S083's rule refuses: both rows at 3.75 are Black to move.
- Every comment and GOLDEN paragraph that stated the old numbers is rewritten,
  and the header's "tempo ... ships at zero" is made true.
- `truncation_bound()` is one function shared by both cases.

**The goldens**:

| golden | old | new | script |
|---|---|---|---|
| six piece anchors (`test_evaluation`) | 99, 276, 355, 583, 984, 0 | **100, 277, 356, 584, 986, 1** (the case "bare kings cancel" is renamed "bare kings": tempo alone) | `python3 adocs/data/S192_anchors.py <fit header>`, then 10 of 10 on this tree with its embedded values set |
| `QUIET_ROOK_EVAL`, `_CHEAP` | 583, 578 | **584, 579** | the same, "rook on d1" |
| the written-out lazy bound | 394 | **395** (579 - 184) | the same and `src/search_params.hpp` |
| `black_static` | 199 | **203** | the same, "black in check, Re8" |
| the quiet evasion | -533 | **-534**; leaves -565, -534, -583, -544, still the four king moves standing pat | the same, `LEAVES` |
| the repetition case | -597 | **-596** | the same, "black a rook down, Kh7" |
| `truncation_positions` | above | above | `truncation_scan --min 2.875` over the views, `S083_truncation_pick.py` |
| `short_line_ceiling` | 2, 6, 4, 1, 2, 3 | **unmoved** over nine grids (S136's alone: 0, 2, 1, 0, 0, 1) | `S203_case_sweep.sh --ceilings` with `adocs/data/S136_sweep_s170.txt` added (108 cells, 1403 mate lines, 19 short) |
| S170 budgets | A 500000, B 100000, C 1500000, D 4000000, E 300000, F 300000 | **C 500000, D 1500000, E 100000**; A, B, F unmoved | `adocs/data/S170_pick_budgets.sh`, applied with a header paragraph |
| `EXACT_FLOOR` | 159 | **159, red** (156 shipping, 156 weakened; DEC-323) | `S156_mined_floor_sweep.py`, `adocs/data/S136_floor.txt` |
| `MATE_IN_THREE_FLOOR` | 11 | **11, holds and separates** (13 against 10; the weakened gate red at 10) | `S154_floor_margin_sweep.py floor`, `red` |
| node budget | 65024 / 3251 | **unmoved**: 7386, 0.45 of c, inside [7270, 32512] | `S192_node_budget.py` |
| first-mate, `DRIVE_DEPTH`, ply-2 `first_exact` | 9, 9, 5; 4; 6 | **unmoved** | `S188_repair_goldens.py first-mate`, `s207`; `S083s_rfp_ply2.py` |
| `capture_mates`, ProbCut, extra ply | at their depths | **unmoved** | `S230_mine_r01_row.py depths` |
| `LATE_CAPTURE_*` | LQ02 at 7, LQ03 at 3 | **unmoved** | `S261_mine_late_capture.py sweep`, `pick` |
| clamp push; walk floor | +100; 0.01 | **unmoved**: +100, 23.00 from the kink; the AdaGrad walk 0.0116 | `S083_tuner_preconditions.cpp both` |

Notes on the goldens:
- `capture_mates`' first three rows now first report at exactly their asserted
  9, 8 and 11, where the parent's started one ply earlier, so none has a spare
  ply.
- The walk was 0.0135 at S135; the clamp push was not re-picked.
- **Readings only, none asserted**: `test_mate_pv`'s lines are 189 + 513 = 702,
  against 707 on the parent. The defender sweep reads 83/104 against the
  parent's 85, with m2 31/31.
- **Unmoved by construction**: `golden_defaults`, the option count, the census
  floors and the book.
- **Against DEC-299**: `MATE_IN_THREE_FLOOR` holds, no ceiling rises, and
  `EXACT_FLOOR` is DEC-323's one exception. No other golden is outside the
  guard.

**Comments the paste made false, made true**:
- `src/evaluation.cpp`, whole-line only: the comment block over `tempo_mg`
  (now "Fitted since S136" with S027's history in the past tense, and the
  S100 paragraph's last sentence) and `evaluate_cheap`'s tempo comment (the
  bound as `truncation_bound()`'s arithmetic). `git diff -U0 ba65b86 -- src/evaluation.cpp` outside
  whole-line comments is the two tempo definitions alone, so `S136_sprt.sh`'s
  filter passes.
- `tools/tuner_trace.hpp`'s header, the smooth score's paragraph.
- `tools/truncation_scan.cpp`'s header and `usage` text: the maximum is now
  stated as `truncation_bound()`; the default `--min 2.8` is unchanged.
- `tests/test_eval_model.cpp`, the comment over "the move is worth the same
  to either side".
- `DEV_MANUAL.md`: the goldens table's rows, the truncation-scan section,
  the ulp paragraph, the "at most 3 x 23/24" sentence, the gauge walk and
  `test_mate_pv`'s count.
- `MANUAL.md`: the `RfpMinPly` row's mined-set figures.
- **Left for the closing commit**, because `S136_sprt.sh` allows only
  `src/evaluation.cpp` under src/: three comments in `src/evaluation.hpp`,
  over `piece_placement_mg` ("tempo is zero for its own reason"), over
  `tempo_mg` ("Zero, and not because the tuner has not reached them") and
  over `mobility_score` ("invisible ... for tempo").

**`adocs/data/S136_sprt.sh`, working-tree mode** in `S083_sprt.sh`'s form
(DEC-307 (3), DEC-254). CAND is this worktree's uncommitted tree and REF its
HEAD. The guards:
- HEAD is REF, and REF's src/ is `80520d7`'s;
- the tree is dirty;
- no unstaged change under src/ or tests/, and no untracked file under src/;
- `src/eval_tables.hpp` is unchanged, and `src/evaluation.cpp` is the only
  src/ file that differs, by the two tempo definitions outside whole-line
  comments;
- `cmake --build` runs first, then no source may be newer than build/'s
  binary;
- the book is present.

Its outcomes:
- H1: the owner rules on `EXACT_FLOOR` (DEC-323), then the tree lands.
- H0: zero, nothing committed.
- No verdict: unresolved, nothing committed.

The script keeps `{0, 5}` nElo, 20950 rounds, the abort rule and the
banner, which now has the `diff` line and the `cand-<REF>+<hex>` names. The
console goes to `/home/max/ws/chesso/.tuning/s136_sprt.console` and OUT to
`/home/max/ws/chesso/.tuning/sprt_s136_<stamp>`. The pin sentence and the
`[COORDINATOR ...]` marker are left for the coordinator. Unpinned, the
script refuses: "still holds the coordinator's unfilled marker".

**Bench** 6007564 in both builds (the parent 5069022).

**Gates**:
- Both builds: 0 warnings.
- Both fast suites: **49 of 50**. The one red is `test_mate_breadth`'s
  `REQUIRE( found_exact >= EXACT_FLOOR )`, which prints `values: REQUIRE( 156
  >= 159 )`, with `wrong_sign == 0` passing before it.
- `./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`).
- `tools/plan_prose_check.py`: `--citations`, `--touches`, `--params` and
  `--gate` each rc 0.
- Tier 2 (DEC-141) does not bind: `git diff --cached --stat -- src/` names
  `src/evaluation.cpp` alone, the weights and comments. There is no search,
  generator or make/unmake change, and no new pruning rule, so no mutant is
  owed.

## 2026-10-28: the SPRT read H0; tempo stays zero (one Opus 5.5 subagent, closing)

Worktree `../chesso-s136c`, branch `s136c` at `5a9b931`. The close carries
the evidence of phases 2 to 2c and the run from `../chesso-s136b`, and none
of the paste. A shutdown killed this session once (2026-10-28 into
2026-10-29). Every figure below was read again after it and came out the
same: the trim, the blob, the oracle, cold and warm replays, the rates, the
quarters and the journal.

**The void run.** The first launch (2026-10-27 13:15, out
`.tuning/sprt_s136_20261027_131508`, console
`.tuning/s136_sprt_void_20261027.console`, candidate
`cand-ba65b86+75c04550c0f1`) was stopped by SIGINT at 16:17 for the owner's
shutdown: 6520 games in 3 h 02 m, LLR 1.10, no bound,
`SPRT-RUN-FAILED: exited 1`. A killed run is void by precedent: never pooled,
never resumed, re-launched from the pre-registration with a fresh seed. Its
figures are not read here. Both launches are archived in
`chesso_S136_sprt_2026-10-28.tar.xz` (sha256 `b0a0777d...`).

**The verdict.** `adocs/data/S136_sprt.sh` as pinned 2026-10-27 13:15 and
re-launched 2026-10-28 00:35:59. The one change is a 7-line re-launch
paragraph appended to the pin sentence, which is why the blob moved from
`75c04550c0f1` to `ea1eb6cfad0b`. That paragraph says "00:40": it was written
just before the launch, and the out directory and the seed date the launch to
00:35:59. The blob stays as played. The working tree of
`../chesso-s136b` (tempo 10 / 1, bench 6007564) played its HEAD `ba65b86`
(tempo 0 / 0, bench 5069022) and ended `SPRT-RUN-DONE`:

```
SPRT | cand ba65b86+ea1eb6cfad0b vs ref ba65b86, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | -0.66 +/- 3.44, nElo -0.90 +/- 4.70
LLR | -2.96 (-2.94, 2.94) -> H0
Games | N: 21014 W: 5637 L: 5677 D: 9700, Ptnml [774, 2583, 3805, 2599, 746]
Wall | 9 h 46 m, 2148.2 games/h, forfeits 0
Log | adocs/data/S136_sprt.log
```

`Wall`'s rate is N over fastchess's `Total Time` 09:46:55, S296's rule.

### The inspection, before the verdict was read as the record

- **Banner** (the console's): candidate `ba65b86 2026-10-27 + uncommitted
  changes`, `diff .../sprt_s136_20261028_003559/candidate.diff blob
  ea1eb6cfad0b`, reference `ba65b86 2026-10-27` (the commit's date, 11:17),
  `arch native tune off`, padding on both, 8+0.08, hash 16, 12 of 12 cores,
  `noob_3moves.epd`, seed 20261028003559, `elo0=0 elo1=5 alpha=0.05
  beta=0.05`, cap 20950 rounds and 41900 games from `SPRT_ROUNDS`, out
  `.tuning/sprt_s136_20261028_003559`. There is no `relay` or `A/A
  CALIBRATION` line. The `id name`s are `cand-ba65b86+ea1eb6cfad0b Chesso
  ba65b86-dirty native` and `ref-ba65b86 Chesso ba65b86 native`, and the
  first game line reads `Started game ... of 41900`. All of it is as the
  pin and the script's banner sentence say.
- **Binaries.**
  - The reference `../chesso-s136b/.ref-builds/ba65b86/build/src/chesso`
    benches **5069022**. Its worktree is clean at `ba65b86`. The binary was
    written 2026-10-27 13:15 and not since.
  - The candidate played from `fastchess.sh`'s snapshot (`cp` of
    `../chesso-s136b/build/src/chesso`), deleted at the end. That file was
    written 2026-10-27 12:57, not since, and benches **6007564**.
  - The tree, rebuilt into a scratch directory there
    (`build-s136close/`, Release, native, tune off, ccache), benches
    6007564 and is byte-identical to it (sha256 `1407f16bc07f7a25...`).
  - `git -C ../chesso-s136b diff --cached --binary --full-index` equals
    the run's `candidate.diff` byte for byte. Without `--full-index` the
    index lines are abbreviated, so the brief's form differs only there
    (`fastchess.sh` writes `git diff-tree -p --binary --full-index`).
  - Nothing is unstaged and nothing untracked. That worktree's files are
    unchanged.
- **Crashes, disconnects, illegal moves**: none in the console or in
  `fastchess.log`, whose 10 entries are the warnings below.
  - The PGN holds 21017 games. fastchess's game numbers (`2 * Round - 1`
    with the candidate White, `2 * Round` with it Black) run from 1 to 21020,
    each at most once. 21014, 21017 and 21019 are the ones unfinished when
    the match stopped.
  - Terminations: 13590 adjudication (11312 decisive, 2278 drawn) and 7427
    normal (7424 drawn, 3 decisive).
- **Forfeits**: `tools/forfeit_report.py` reads 0 of 21017 for each engine.
- **Console warnings**: 5 PV-past-threefold blocks (4 the reference's, 1
  the candidate's; `trim_console.py`'s count) and **5 `Incomplete mating
  PV`** (2 the reference's, 3 the candidate's), read in the next section.
- **Quarters.** Taken in game-number order, 5254 or 5255 games each
  (boundaries `i * 21017 // 4`), as Elo from the candidate's score with its
  95 % per-game interval, over the PGN's games. The method reproduces S296's
  -0.7, -3.3, +4.4, +3.3 (chi-square 2.73) and S135's +5.6, -2.5, +13.8, +6.6
  (6.56) from their PGNs exactly. Here: **+0.5, +0.7, -0.4, -3.5**, each
  +/- 6.9 to 7.0. Their spread about the run's mean is chi-square 0.90 on 3
  degrees of freedom, p 0.82.
- **Kernel journal**, 00:35 to 10:24 (boot `3ac9e3c8`, from 00:31:58, no
  suspend). It holds one kernel entry, 00:35:02 `usb 3-2.4: USB disconnect`,
  which is the monitor's control device, before the launch, as on S296's
  night. From the launch to 10:24 there is no kernel entry and no entry at
  error level. Before the launch, the panel applets restarted at 00:35:03 to
  00:35:06 (exit 137, user space) and bluetoothd logged errors. There is no
  machine check, OOM or thermal event.

The run stands.

### The five `Incomplete mating PV` warnings (`adocs/data/S136_sprt_mate_pvs.txt`)

The method is S296's, by its script `adocs/data/S296_sprt_mate_pvs.py`, run
from the main checkout. Each position is put on a board by python-chess and
its game found in the PGN. The oracle is DEC-319 (3)'s, Stockfish at
5000000 nodes through python-chess in `TOOLCHAIN.md`'s safe form, from the
bare FEN and with the game's moves. It reads a mate on all three positions,
so DEC-320's 10^9-node escalation does not arise. Console line numbers are
the untrimmed console's; the trimmed log's are the same for the first two
and 12, 16 and 16 lower for the last three.

| console line | engine | game, ply | printed | line | Stockfish 5000000 | printed against it | reproduced |
|---|---|---|---|---|---|---|---|
| 12253 | ref | 5096, 98 | mate 8, depth 5, 3185 nodes | 5 of 15 plies | mate 8 | equal | no |
| 12257 | ref | 5096, 98 | mate 8, depth 6, 6029 nodes | 6 of 15 | mate 8 | equal | no |
| 14853 | cand | 6174, 61 | mate -14, depth 14, 1649044 nodes | 21 of 28 | mate -10 | longer | no |
| 35997 | cand | 14980, 91 | mate 11, depth 3, 733 nodes | 3 of 21 | mate 8 | longer | no |
| 36001 | cand | 14980, 91 | mate 13, depth 4, 1387 nodes | 4 of 25 | mate 8 | longer | no |

**No printed distance is shorter than the oracle's, and the claimed side is
the oracle's in every one.** In game 5096 the side to move mates, in game
6174 it is mated, and in game 14980 it mates. That covers the reference,
the shipped engine, and the brief's stop does not arise. Three distinct
positions, each line legal throughout and shorter than the distance it
claims. All are **DEC-122's short-line class**, which `test_mate_carry`
counts against per-case ceilings (DEC-162) and S202 owns. None is the other
half, a line at least as long as its distance that is not mate there. Every
cause is a recorded one, S296's two shapes:

- **12253, 12257, 35997, 36001**: a shallow iteration (depth 3 to 6, 733 to
  6029 nodes) printing a mate too shallow to build its line, as DEC-150's
  case was. The distance is the position's (game 5096) or longer (game
  14980).
  - Game 5096's later iterations printed the complete distance: the PGN's
    `+M15/13`, mate 8, with e4e7 played.
  - Game 14980's search ended at `+M17/13`, mate 9, still one move longer
    than the oracle's. It played the oracle's first move h3h1 where the
    short lines began g3d3.
- **14853**: the search's last line, the PGN's `-M28/14`. It names a mate
  four moves longer than the oracle's against the side to move: DEC-127's
  "sound but not optimal", and "A depth-limited search naming a longer mate
  than the game value is the search, not the report". The walk leaves the
  line 7 plies short. The oracle's line opens with its six first plies.
  The game was adjudicated two plies later.

That the mates came off entries the games' earlier searches wrote is
inferred from the depths and node counts, as S296 inferred it for its
candidate's five.

**Reproduction.**
- **Cold table**, on both binaries: one fresh process each, `Hash 16`,
  `ucinewgame`, the console's `position fen ... moves ...`, and `go depth N`
  at the warning's depth. None reproduces. Both binaries print cp lines and
  no mate line, and both play the move the game played.
- **Warm replay**, through one process of the warning side's binary,
  `Hash 16`, at each of its turns `go nodes n` with the PGN's per-move node
  count. **None reproduces, the reference's two included.**
  - Game 5096's replay ends at cp +544, depth 14, where the game printed
    `+M15/13`.
  - Game 6174's ends at cp -2463, depth 14.
  - Game 14980's prints the complete mate 9 from depth 1.
  - No earlier turn of the three replays prints an incomplete line.
- So the replay's table diverges from the game's, and no entry-by-entry
  trace (`mate_trace`) of any of the five exists.

**The rates** (`S296_sprt_mate_pvs.py rates`), as lines and distinct
searches per 10000 games a side:

| run | candidate | reference |
|---|---|---|
| this run | **1.43 and 0.95** (3 and 2) | 0.95 and 0.48 (2 and 1) |
| S296 | 2.70 / 1.62 | 1.08 / 0.54 |
| S293 | 0 / 0 | 0.91 / 0.91 |
| S292 | 0 | 0 |
| S135 | 4.48 / 1.49 | 0 |
| S289 | 0 | 4.59 / 2.29 |
| S283 | 0 | 0 |

- Pooled over those six runs: 18 lines and 9 searches in 160274
  side-games, 0.56 searches per 10000.
- At that rate each side's 21017 games expect 1.18 searches. The
  candidate's 2 or more has Poisson probability 0.33, and the reference's 1
  is at the rate.
- Both sides read under S296's figures.

A count, recorded, not a finding. What S296 forbids, a distance shorter than
the position's, is in none of them.

### The verdict, by the pre-registration

**The H0 row executes.** At 8+0.08 the fitted pair is not shown to gain:
H0's 0 nElo is preferred to H1's 5 at alpha = beta = 0.05 (DEC-321 (3),
DEC-323 (4)).

- **Tempo is recorded as zero**, the screen's and the match's figures
  together:
  - the screen kept it, held-out gain +0.000031 against m 0.000002, fit
    mg 10 / eg 1;
  - the match read H0.
  A verdict of zero is recorded as zero (`CLAUDE.md` rule 8).
- **The paste reverts whole.**
  - Nothing under `src/`, `tests/` or `tools/` lands.
  - None of the paste's changes to `DEV_MANUAL.md`, `MANUAL.md`,
    `adocs/data/S170_cases.tsv` or `adocs/data/S192_anchors.py` lands.
  - REF's tempo-zero tree stays, and bench stays 5069022.
  - The measured paste stays reproducible as
    `adocs/data/S136_candidate.diff` (blob `ea1eb6cfad0b`, DEC-254).
- **`EXACT_FLOOR` never moves**: it stays 159, and DEC-325's 159 -> 155
  is moot. The truncation guard keeps its tempo-zero form: N = 3, bound
  2.875, the four pins S135 drew.
- **The stopping run's Elo is biased (DEC-063) and is not the effect
  size.** The run stopped at a bound in 21014 games, under the 25591 the
  pre-registration expected with the truth on a bound.
- No bisection follows (a family of one), and no question is asked.

**What H0 leaves unmeasured.**
- Tempo at any pair other than 10 / 1. The screen's quarters read 9 to 10
  mg and 1 to 2 eg, and nothing was fitted or played at another value or
  sign. Nor was any other label, corpus or control.
- The side-to-move gap on S083's corpus, -0.017848 White relative, about
  **-8.7 cp** of tempo weight at K 0.7224. It is the opposite sign to the
  fitted pair and to S100's +3.6 cp, and confounded as S100's was:
  quiescence leaves, in-check rows skipped, and the two groups differing in
  more than whose move it is. H0 says nothing about it.
- S027's run stays recorded as unresolved, as it was. This run is a second
  measurement at a different pair, corpus and control, and converts
  neither.

### The trim and the archive

`tools/trim_console.py --summary` from `.tuning/s136_sprt.console` (50521
lines, 4041589 bytes) into **`adocs/data/S136_sprt.log`**, 50507 lines,
4037692 bytes. The 5 PV-past-threefold blocks are dropped (20 lines) and
counted at the foot. The five mating-PV warnings are kept. After the
shutdown the trim was re-run into a scratch file and compared: `cmp` equal.

The run's `candidate.diff` is **`adocs/data/S136_candidate.diff`**, 278127
bytes. `git hash-object` gives `ea1eb6cfad0b71762da06938c1b7d0afecee4189`,
the console's blob, and the file is `cmp` equal to the run's.

Both launches' untrimmed consoles and out directories (PGN, `fastchess.log`,
`candidate.diff`) are **`chesso_S136_sprt_2026-10-28.tar.xz`** in the owner's
archive (DEC-235), sha256 `b0a0777d...4fda4d0ac5`. The coordinator built,
verified and moved it in; a copy is in `.tuning/coord/S136/archive/`.

### The evidence carried, not the paste

From `../chesso-s136b`'s index, by `git diff --cached --binary --full-index`
limited to these paths and applied here. The paths are unchanged on the main
line since `ba65b86`, and each file's blob equals the index's:

- this file (the phase 2, 2b and 2c sections);
- `adocs/data/S136_sprt.sh` as pinned and re-launched;
- `S136_floor.txt`, `S136_overlay_run.py`, `S136_mined_rows.py`,
  `S136_lost_mates.txt`, `S136_lost_mates.py`, `S136_uci_go.py`,
  `S136_trace_patch.py`, `S136_trace_line.py`, `S136_line_cut.py`,
  `S136_goldens.txt` and `S136_sweep_s170.txt`;
- their `adocs/data/README.md` rows.

**Labels.** Each carried row is labelled *On the H0 candidate (tempo 10 /
1), never shipped*, and the phase 2 table's intro says the three tables were
measured on the H0 candidate and describe nothing that ships. The files'
own bytes are left as the candidate diff recorded them. The
`S136_sprt.sh` row gains the pin, the void launch and the H0.

**New rows** in a "The close" table: `S136_candidate.diff`,
`S136_sprt.log` and `S136_sprt_mate_pvs.txt`. No new script:
`S296_sprt_mate_pvs.py` is used as committed.

**Not carried**: anything under `src/`, `tests/` or `tools/`;
`DEV_MANUAL.md`, `MANUAL.md`, `adocs/data/S170_cases.tsv` and
`adocs/data/S192_anchors.py`.

### Comments and documents the H0 makes false

**None, so none changed.**

A grep of `src/`, `tests/`, `tools/`, `DEV_MANUAL.md`, `MANUAL.md` and the
S136 data files for "pending SPRT", "if H1", "on H1", "until the SPRT",
"tempo is fitted", "S136 will" and their kin finds nothing the H0 makes
false. "Tempo is fitted" occurs once, in `tests/test_eval_trace.cpp`'s
`CHECK_MESSAGE`, a message for a fitted pair, which stays conditional. The
"fitted since S136" wording existed only in the paste, and it reverts with
it. No file under `src/`, `tests/` or `tools/` names S136.
`adocs/data/S136_sprt.sh` keeps the future tense of a pre-registration, as
S296's does.

**Stale, not false, left for the coordinator** (comments in `src/` would
make this a source commit):
- `src/evaluation.cpp`'s tempo block still says "If this is ever revisited
  it needs tighter bounds than `--fast` gives" and "Re-measuring this needs
  a WDL-heavy label and bounds that can resolve single digits".
- `src/evaluation.hpp`'s "Zero, and not because the tuner has not reached
  them" names S027 alone.
Both are true and silent on S136, which did exactly that and read H0.

**Pre-existing, not the H0's.** `tests/test_eval_trace.cpp`'s pinned-case
comment ("Fit tempo to anything non-zero ... the bound becomes 4 x 23/24 =
3.833"), its bound case's `(tempo_unfitted ? 3.0 : 4.0)` and
`tools/tuner_trace.hpp`'s "4 x 23/24" fitted figure are exact only at
g = 1 (DEC-322). The paste re-derived them and reverts. The tempo-zero
branch, the one that runs, is exact. A later tempo fit starts from
`S136_candidate.diff`'s guard.

**Manuals.** `DEV_MANUAL.md` was checked: the goldens table, the
truncation-scan section, the "at most 3 x 23/24" sentence, the ulp
paragraph, `--freeze` and `--only`, the gauge walk, `test_mate_pv`'s count
and the `-check-mate-pvs` standing figure. `MANUAL.md` was checked: the
`RfpMinPly` row's mined-set figures and the Known bugs. Every statement
there is the tempo-zero tree's, which is the shipped tree again, so neither
changes. `specs.md`'s "absent, evaluation" row is the coordinator's: its
tempo sentences are incomplete after the H0.

### `accepts`, clause by clause

1. **The truncation guard re-derived, never relaxed, from N read at this
   step's HEAD.**
   - N = 3 at `80520d7`, measured: the four pins +69, -69, +69, +69
     twenty-fourths, the corpus at most 69/24 (phase 1, "N, read at
     80520d7").
   - Re-derived on the candidate by DEC-322 (1) at the fitted pair's
     g = 3: bound 90/24 = 3.75, both worst clauses past 3.675, each pin past
     69/24 + 1e-6, the precondition `(tempo_mg - tempo_eg) % 24 != 0`. Its
     red was observed first (phase 2, "The red, observed").
   - Applied in phase 2c, in `S136_candidate.diff`.
   - DEC-092's literal 4 x 23/24 is exact only at g = 1. That is a
     re-derivation at the exact g, not a relaxation (DEC-322).
   - On H0 it reverts with the paste, and the shipped guard is N = 3's,
     exact for the shipped zero pair.
2. **The four pinned FENs re-measured under the new weights by
   `truncation_scan`.** The candidate's pins were drawn by
   `truncation_scan --min 2.875` over S082's three training views and
   S083's pick, all four at 3.75: phases 5 b, 5 w, 13 b, 13 w
   (`adocs/data/S136_goldens.txt`; DEC-057's pre-authorised re-targeting,
   DEC-322 (2)). They revert on H0.
3. **The fit is `--only tempo`**: `adocs/data/S136_screen.sh`, `--only tempo
   --lambda 0`, fitted mg 10 / eg 1. The candidate differs from REF by the
   two tempo definitions alone outside whole-line comments, which
   `S136_sprt.sh`'s guards checked before the run.
4. **The label WDL-heavy, `--lambda 0` stated**: the same command (DEC-321
   (1)).
5. **SPRT bounds that resolve single digits, stated in advance**: `{0, 5}`
   nElo at alpha = beta = 0.05 (DEC-321 (3), DEC-259 (b)), pre-registered
   in `adocs/data/S136_sprt.sh` at `ba65b86` with DEC-143's worst case
   before any game. It resolved, H0 in 21014 games, where S027's `{0, 10}`
   at `--fast` resolved nothing.
6. **A verdict of zero recorded as zero; a second unresolved run recorded
   as unresolved, not converted.** H0, recorded as zero (above). No verdict
   did not arise. S027's stays unresolved.
7. **The fast suite green**, on the tree that lands, in both builds (Gates).
   The candidate's 49 of 50, `EXACT_FLOOR` red by DEC-323, was never
   committed.

### Gates

On this close's worktree (`../chesso-s136c`, branch `s136c` at `5a9b931`),
with `CLANG_FORMAT_MAJOR=22` and builds at `-j12`:

- `build/` and `build-tune/` with `-Werror`: no warning.
- The fast label, **50 of 50 in both** (119 s and 117 s, sequential,
  AGENTS.md's TESTS command).
- `./clang-format.sh --check` clean.
- `tools/plan_prose_check.py` `--citations`, `--touches`, `--params` and
  `--gate`, one mode per call: each exit 0. The two pre-existing notes on
  this file's lines 80 and 86 are ungated.
- `chesso bench` **5069022** in both builds.
- `tools/gate.sh --message` with the proposed record message over the
  staged tree: both suites again 50 of 50, `SPRT block checked: cand
  ba65b86+ea1eb6cfad0b vs ref ba65b86 against adocs/data/S136_sprt.log, diff
  adocs/data/S136_candidate.diff`, `GATE-DONE 5069022 (no src/ change, no
  signature owed)`.

Tier 2 (DEC-141) does not bind. The close changes no file under `src/`,
`tests/` or `tools/`, and `git diff --cached --stat -- src/` is empty. So
neither `tools/gate_extra.sh` nor the Debug self-play was run. The H0
candidate's own Tier 2 was not owed either: no search, generator or
make/unmake change, no pruning rule (phase 2c).
