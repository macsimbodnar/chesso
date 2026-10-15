id:         S039
goal:       the lazy shortcut and the clamp on mobility plus king safety retire together, so evaluate() returns the exact sum it computes everywhere, and one non-regression SPRT decides it
accepts:    (1) the shortcut and the clamp go in one change: the expensive stage is always computed and its sum is never truncated, and no weight moves (weights frozen); (2) every reader of the margin is resolved, the list read at this step's HEAD with `git grep -E 'LAZY_EVAL_MARGIN|LazyEvalMargin|evaluate_lazy'` and recorded -- each removed, re-pointed or retired with its reason, including the tune-build option, the truncation tests and guards (DEC-053, DEC-169: the taper divisions stay, S055 having read H0, and the truncation guard is re-derived at this step's HEAD, never relaxed), `adocs/data/S192_anchors.py`, `tools/eval_spread.cpp`, the mutants in `tools/mutants/eval.py` that target the clamp or the shortcut (re-anchored, or retired with the reason recorded, DEC-141), and the prose checker's phrase table; (3) INV-6 does not apply -- the tree changes -- so `bench` moves and the commit carries the new total; the nps and the nodes to depth 11 over S103's 300 positions are measured against the parent and recorded beside the verdict, as S120 measured the shortcut alone; (4) one `{-5, 0}` nElo non-regression SPRT at 8+0.08 with DEC-143's worst case (25591 games on a bound, 41861 at the midpoint) and the H1, H0 and no-verdict actions written in the pre-registration before the first game; H1 lands it; H0 keeps both, and S122's design is amended by an owner decision before S121 starts, never by S122 claiming unclamped scores under a kept clamp; (5) the surface golden (`tests/test_uci_surface.cpp`) is refreshed only after `adocs/specs.md` and `MANUAL.md` describe the removed tune option (SURFACE rule); (6) the second tier (Debug self-play, `tools/gate_extra.sh`) before completion, because the step changes what the search reads at every node
            (Re-formed 2026-10-08 by DEC-258, from "re-decide LAZY_EVAL_MARGIN
            from measured spread". The margin re-decision below is history.)
touches:    src/evaluation.cpp, src/evaluation.hpp, src/search.cpp, src/search_params.hpp,
            tests/test_eval_trace.cpp, tests/test_evaluation.cpp, tests/test_search.cpp,
            tests/test_search_params.cpp, tests/test_tuner_gradient.cpp,
            tests/test_uci_surface.cpp, tools/tuner_trace.hpp, tools/eval_spread.cpp,
            tools/datagen.cpp, tools/mutants/eval.py, tools/plan_prose_check.py,
            adocs/data/S192_anchors.py, MANUAL.md, DEV_MANUAL.md, adocs/specs.md
            (re-pointed 2026-10-13: S276 deleted tools/eval_model.hpp and the
            test_eval_model cases that read the clamp; the trace that replaced
            them is tools/tuner_trace.hpp and tests/test_eval_trace.cpp)
excludes:   any weight change or new term -- the fits that use the unclamped sum are S121 onward; another lazy shortcut with a different bound, which would need its own demonstrated bound and scope; the taper divisions, which S055 measured (H0)
decisions:  DEC-039, DEC-257, DEC-258, DEC-143, DEC-274
closes:     2026-08-13_adversarial-F05, 2026-09-10_adversarial-F16, 2026-09-10_adversarial-F25
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:

## Re-formed 2026-10-08, DEC-258: both off, one verdict

The owner chose, on 2026-10-08, to retire the shortcut and the clamp
together under one non-regression verdict, over two separate verdicts and
over folding the removal into the mobility step. The reasons:

- **The two are one mechanism.** The clamp is what makes the shortcut's
  bound hold by construction ("Not an unsoundness" below). With the clamp
  gone the shortcut has no bound, so measuring the shortcut alone answers a
  question that stops mattering the moment the clamp goes.
- **Each half alone is near zero.** S120 measured the shortcut switched off
  with the clamp kept: -5.01 % nps, -7.5 % nodes, -2.64 % wall to depth 11
  (`adocs/data/S120_measurements.txt`, "Exact everywhere"). The clamp binds
  rarely at today's weights (F16: p99 153, max 401 over 1.5 M rows, against
  184). Two near-zero `{-5, 0}` runs cost 24 to 38 hours for about nothing.
- **The fits need it first.** S121 fits mobility, which sits inside the
  clamped sum, and S122 needs king safety unclamped. So this step moves from
  directly before S122 to directly before S121.

F16's concern -- the clamp sized before the terms that change its sum --
dissolves with the clamp. F25's missing candidate value is resolved with the
tool in (2). F05 closes with the comment the clamp's removal deletes or
rewrites.

## Before 2026-10-08: the margin re-decision this step replaced


## What is stale

`src/evaluation.hpp` `evaluate_expensive_terms` says 150 is above the largest
correction observed, and adds that the figure "is still the whole correction
only because king safety ships at zero weight; the margin is re-decided from
measured data once it is fitted, and tools/eval_spread is what measures it".

King safety has not shipped at zero weight since S027 (`src/evaluation.cpp`
`king_safety_mg`). The margin is 184, moved by S085's SPSA and not by a spread
measurement. The re-decision the comment promises has not happened.

## What it costs now

`eval_spread` over 50000 corpus positions at the current weights:

```
  150            66 0.132%      0 0.000%    182 0.364%
  200             3 0.006%      0 0.000%     24 0.048%
  250             0 0.000%      0 0.000%      9 0.018%
worst position per term
  mobility      201  r1b2qk1/3p2p1/p2p4/1p1P1B2/5p1p/1QB4P/PP3P1K/4R1R1 b - - 0 27
  king safety   134  r1b4r/1p1k1pp1/1Qn2q1p/p2N4/3p4/5B2/P2B1PPP/4RRK1 b - - 0 21
  combined      279  r1b2q1r/ppppk3/5n1p/2P3p1/8/1P5N/PBQ3BK/3R1R2 b - - 5 21
```

The combined correction exceeds the margin on 0.364 % of positions and reaches
279, so `evaluate()` discards up to 129 cp of a term it has already computed on
roughly one position in 275.

## Not an unsoundness

The clamp is applied inside `evaluate_expensive()`, so `evaluate()` *is* the
clamped function and the shortcut's bound holds by construction — an audit check
over 100000 positions found the gap never above 150. What is lost is
expressiveness, and it lands exactly where king safety was added for: 134 in a
position with a king on d7 under fire, where the clamp is what stops mobility
and king safety both being paid.

## Why it is a step and not an edit

Raising the margin costs search time in `evaluate_lazy` — fewer positions take
the shortcut. That trade is an SPRT question, not an argument. Re-run
`eval_spread` over the full corpus first, pick the margin from that
distribution, then measure.

## Amended 2026-09-11, DEC-170 and DEC-172: moved beside S122, and two audit findings folded in

`2026-09-10_adversarial-F16` re-measured the spread at the shipping weights
over 1.5 M rows: p99 **153**, max **401**, against the `src/evaluation.cpp`
comment's "p99 128 and max 330" -- the figures in "What it costs now" above
are older still. And it observed that this step sized the margin "at the
weights that ship at this step's own HEAD" while the Open order placed S121,
S123, S125 and S101 -- four steps that change the very sum being clamped --
between it and S122. **So this step now sits directly before S122**, its
consumer, and the `eval_spread` run is taken at the weights those four steps
leave behind. DEC-169 already binds the other side: a move of
`LAZY_EVAL_MARGIN` owes a fresh `truncation_scan` reading, recorded beside the
new margin.

`2026-09-10_adversarial-F25`: `tools/eval_spread.cpp`'s candidate margins are
`{150, 200, 250, 300, 400}` and do not include the 184 that has shipped since
S085 -- in the tool whose whole job is this re-decision. The candidate list
gains the shipping value, read from the binary or from `search_param_info()`
rather than typed, before the run is taken.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki defines the technique and gives no number for anything this step
decides.** CPW *Lazy Evaluation*
(https://www.chessprogramming.org/Lazy_Evaluation, fetched 2026-09-13): "If
after performing all the tasks for a given stage score exceeds beta by a
certain margin or if it falls below alpha by the same margin, the score is
returned." The page states **no margin value, no method for choosing one, no
warning about the bound, and no Elo figure**. So there is nothing published to
seed the margin from and nothing to check the choice against -- the number can
only come from this engine's own spread, which is precisely what the accepts
says and why this step exists.

**Two departures from the wiki's form, both deliberate and both already in the
tree.** First, the page's condition is a *stage* test that returns early;
chesso's clamp is applied inside `src/evaluation.cpp` `evaluate_expensive`, so
`evaluate()` **is** the clamped function and the shortcut's bound holds by
construction. That is stronger than the published form and it is why "Not an
unsoundness" above is correct. Second, the published form's margin guards a
return; chesso's also bounds the *expressiveness* of the two terms inside it,
which is the cost S122 cannot pay. Neither departure is on the wiki and both
are stated here so the implementer does not read the page and expect the
simpler shape.

**No located source prices a lazy-evaluation margin in Elo, at any band.** The
2026-09-04 literature check recorded lazy evaluation as present in Berserk 4.3
and in chesso and found no figure; this pass fetched the CPW page and confirms
it carries none. Searched: CPW *Lazy Evaluation*; the review's part B3 row.
The step does not need one -- its verdict is its own SPRT -- and no ordering
argument here rests on a published number.

### 2. Shape for chesso

The number lives in `src/search_params.hpp` `CHESSO_SEARCH_PARAMS` as
`LAZY_EVAL_MARGIN` and has since S073; `src/evaluation.hpp`'s comment is what
it *means* and is the sentence the accepts asks to re-measure. The distribution
that decides it is `tools/eval_spread.cpp`'s, run at the weights that ship at
this step's own HEAD -- which, after the DEC-172 move, is the weights S121,
S123, S125 and S101 leave behind.

### 3. Implementation sketch

- `tools/eval_spread.cpp`'s candidate list gains the shipping value, read from
  `search_param_info()` rather than typed (F25). A tool whose job is this
  re-decision that cannot report the incumbent cannot answer the question.
- The run goes in `adocs/data/S039_eval_spread.log` with the corpus it read
  named, because `.tuning/selfplay_v2.tsv` is gitignored and a machine move
  loses it.
- The comment above `LAZY_EVAL_MARGIN` is rewritten from the new run's
  figures, both sentences -- mobility and king safety -- and cited by symbol.
- DEC-169 binds the other side: a move of the margin owes a fresh
  `truncation_scan` reading recorded beside the new value.

### 4. Constants and seeds

One constant, and it is **(b) a derivation over chesso's own data, run by this
step at its start**. The procedure, stated so the implementer does not
improvise:

1. Build `tools/eval_spread` at this step's HEAD and run it over the largest
   corpus on disk, with the candidate list including the shipping value.
2. Report, per candidate margin, the share of positions whose combined
   mobility-plus-king-safety correction exceeds it, and the worst position per
   term with its FEN.
3. Choose the candidate from that distribution with the rule stated in the
   step -- the share it is willing to clamp -- and record the share, not only
   the margin.
4. SPRT the change against the preceding commit if the margin moves.

**No published value seeds this and none exists** (section 1). Another
engine's lazy margin would be that engine's tuned constant wherever it is
quoted (DEC-105, DEC-134) and is not used. The figures inside "What it costs
now" above are this project's own older measurements and are superseded by the
new run, not averaged with it.

### 5. Pitfalls

- **Three generations of figures now sit in this file and they disagree.** "What
  it costs now" gives 0.364 % past 150 and a worst of 279; the
  `src/evaluation.cpp` comment says p99 128 and max 330; F16's re-measurement
  over 1.5 M rows says p99 **153** and max **401**. Only the run this step
  takes counts, and the older three are kept as history, not as inputs.
- **Raising the margin costs search time.** Fewer positions take the shortcut
  in `evaluate_lazy`, which this step deletes. That is the trade and it is an
  SPRT question, never an argument.
- **The measurement moves under the step.** Four terms-changing steps now sit
  between this file's original writing and its run. Taking the spread at the
  old weights would size a clamp for a sum that no longer exists, which is
  exactly what F16 found and DEC-172 moved the step to fix.
- **The clamp is on the real score, not only on the shortcut.** Anyone reading
  this as a pure speed knob will under-weight what S122 loses.

### 6. Measurement

An `eval_spread` run recorded in `adocs/data/`, then one SPRT against the
preceding commit **if the margin changes** -- and no SPRT if it does not,
because nothing moved. A `truncation_scan` reading beside the new margin
(DEC-169). The spread run is minutes; the SPRT is the S105 regime and is
daytime work under DEC-155.

### 7. Interactions

- **S122 (immediately after, its consumer)**: the clamp is what caps king
  safety, and S122 states which of the three outcomes it inherited.
- **S121, S123, S125, S101 (before, by DEC-172)**: each changes the sum being
  clamped, so the spread is taken behind all four.
- **S120 (ordered ahead)**: retiring the clamp costs a measured 11.7 % of nps;
  the evaluation cache and S104 are what pay for it. [2026-10-06, coordinator:
  S120 retired the cache (DEC-257), so nothing but S104 pays. Its statement (c):
  exact everywhere re-measures at 5.0 % nps for a tree 7.5 % smaller and 2.6 %
  less wall to depth 11 (`adocs/data/S120_measurements.txt`). Read that, not
  11.7 %.]
- **DEC-169 (truncation)**: a margin move owes a fresh scan.

### 8. References

- - https://www.chessprogramming.org/Lazy_Evaluation -- the condition quoted
  above; **no margin value, no selection method, no Elo figure**. Fetched
  2026-09-13.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- lazy
  evaluation present in Berserk 4.3 and chesso, no figure located.
- - `adocs/audit/2026-09-10_adversarial.md` findings F16 and F25 -- the p99 153
  / max 401 re-measurement and the missing candidate value. Local evidence.

## As built (2026-10-13, phase 1)

Phase 1 under DEC-260, in the worktree `/home/max/ws/chesso-s039` on
`c88a961`, while S261's SPSA lane held every core: the change, its tests, its
documents and the pre-registration written blind. **Nothing was compiled,
built, tested or run.** Every "green" and every number below that is not a
line count is a prediction for phase 2, labelled as one.

### Deviations

- **Nothing measured yet** (DEC-260): accepts (3), (4) and (6), the DEC-169
  scan, the anchors run and the suites are phase 2's, listed below.
  `adocs/data/S039_sprt.sh` refuses to run while `PIN_ME` or `P2_PENDING` is
  in it.
- **The no-verdict outcome was not stated by this file or by DEC-258**, and
  accepts (4) wants it written before the first game. Phase 1 put the
  question; **DEC-274** (2026-10-13) answers it: no verdict at the cap lands
  the change as H1 does, recorded as a zero with the interval stated, and no
  follow-up run (DEC-063). The pre-registration states the three outcomes
  with it.
- **`touches:` re-pointed**: S276 deleted `tools/eval_model.hpp` and the
  `test_eval_model` cases that read the clamp; the trace that replaced them
  is `tools/tuner_trace.hpp` and `tests/test_eval_trace.cpp`.
  `tests/test_uci_surface.cpp` stays named and is **not edited**: the tune
  build's option lines are generated from `search_param_info()` in the test,
  and the release build's golden, 5 lines, does not move -- `LazyEvalMargin`
  was never a release option. Accepts (5) holds by `adocs/specs.md` and
  `MANUAL.md` describing the removal in this same change.
- **Files outside `touches:`**: `src/data_structures.hpp`,
  `src/eval_trace.hpp`, `src/transposition_table.hpp` (comments that described
  the clamp or the shortcut as present; no code); `tools/tuner.cpp` and
  `tools/feature_audit.cpp` (they printed `load_report_t::clamped`, which goes,
  and `feature_audit`'s design check skipped the rows past the margin);
  `adocs/data/README.md` (the S206 script's `HEAD` rows pinned to this step's
  parent; an S039 section); `adocs/plan_todo/S122_king_safety_rebuild.md` (its
  citation of the renamed trace case, DEC-159: a rename fixes every pending
  file citing it); new: `adocs/data/S039_sprt.sh`, `adocs/data/S039_readers.txt`.
- **`tools/eval_spread.cpp` re-pointed, not retired.** Its candidate-margin
  table is gone (F25 closes with the list it was missing a value from); what is
  left is the stage-two spread per term and its self-check, now exact, which
  S121 and S122 change and can read before and after. Retiring it instead
  deletes `evaluate_expensive_terms()`'s only tool caller; the accessor would
  stay for the test.
- **One assertion added, not asked for**: "the static score never reaches the
  mate band" also asserts `|evaluate()| <= INT16_MAX` over both populations.
  The transposition table keeps the static score in 16 bits and S213's 1.4x
  headroom was measured with stage two clamped; nothing else holds that
  ceiling now (`tt_eval_to_store()` clamps silently in Release).
- **The smooth score keeps its summation order** (`tools/tuner_trace.hpp`
  `smooth`: stage two summed apart and added last, as when it was clamped), so
  every row the clamp never bound scores to the bit what it scored before and
  `truncation_scan`'s past-2.0 column and `feature_audit`'s design check move
  on the clamp's rows only.
- **`evaluate_cheap()` stays public** and `QUIET_ROOK_EVAL_CHEAP` stays a
  golden: two cases still pin the cheap stage, and `tools/eval_spread` reads it.

### Accepts (2): the readers and what became of each

`git grep -nE 'LAZY_EVAL_MARGIN|LazyEvalMargin|evaluate_lazy' c88a961`: **398
lines in 91 files**, recorded in `adocs/data/S039_readers.txt`.

| reader | resolution |
|---|---|
| `src/evaluation.cpp` (8) | `evaluate_lazy()` deleted; `std::clamp` deleted from `evaluate_expensive()`, which returns `stage_two` whole; the stage-one, tempo, stage-two and `evaluate_expensive_terms()` comments rewritten |
| `src/evaluation.hpp` (4) | `evaluate_lazy()`'s declaration and the margin's comment block deleted; `evaluate()`'s comment carries the history; the include's note re-pointed to `CONT_HIST_WEIGHT` |
| `src/search.cpp` (5) | quiescence calls `evaluate()` (or reads the entry's evaluation) and `stored_eval` is the static score at every node; `static_eval_is_exact` gone; three comments rewritten |
| `src/search_params.hpp` (1) | the `LAZY_EVAL_MARGIN` / `LazyEvalMargin` row deleted: **the tune option retires** (73 parameters to 72); the RFP block's comment says what bounds the static score now |
| `tests/test_evaluation.cpp` (25) | three cases retired, one re-pointed (Tests below) |
| `tests/test_search.cpp` (29) | re-pointed: eleven cases (two stand-pat cases, S164, S112's seven, S131), the S112 helper, the file-scope golden note, two comments |
| `tests/test_search_params.cpp` (1) | the row removed, 73 to 72 |
| `tests/test_eval_trace.cpp` (3) | the identity case renamed and re-pointed; the truncation goldens' "moves on" text |
| `tests/test_tuner_gradient.cpp` (3) | one case retired, one re-pointed |
| `tools/tuner_trace.hpp` (5) | clamp gone from `exact_score()`, `smooth()`, `load()`'s report and `gradient()`; `clamped_site()` deleted |
| `tools/eval_spread.cpp` (3) | re-pointed (Deviations); `MARGINS` and `count_above()` deleted |
| `tools/feature_audit.cpp` (1) | the design check's skip past the margin removed |
| `tools/datagen.cpp` (1) | comment: quiescence calls `evaluate()` since S039 |
| `tools/mutants/eval.py` (2) | **M31_lazy_bound_no_margin retired** (its function is gone; the id is not reused); **M29 re-anchored** onto `stage_two`, same bug; **M41_eval_clamp_restored added**, the clamp put back at 184 (DEC-141) |
| `tools/plan_prose_check.py` (7) | **the phrase table `PARAM_PHRASES` holds no `LazyEvalMargin` rule**, so nothing there retires; two docstring examples that become false re-pointed (`holds_phrase`, `_strip_c`); five left as history (the NEAR sweep example quotes a sentence `specs.md` keeps; the citation-form example, which never resolved after S073; the S039 / F06 narrative twice; the `SELF` comment, still true) |
| `tools/spsa_s085.json` (1) | kept: the record of S085's run, not a runnable config (DEC-200) |
| `adocs/data/S192_anchors.py` (3) | the margin read and the clamp removed; stage two added whole |
| `adocs/data/S206_truncation_drift.sh` (3) | evidence, untouched; its two `HEAD` readings pinned to this step's parent in `adocs/data/README.md` (the margin counterfactual now exits 2 at its own vacuity check) |
| `MANUAL.md` (1) | the option row removed; a paragraph says it is gone and what a tuner naming it now gets |
| `DEV_MANUAL.md` (10) | eight passages re-pointed; the `--touches` S039 narrative and the NEAR example kept as history |
| `adocs/specs.md` (4) | the evaluation row, the mate-band sentence, the machinery row's four present-tense claims and the clamp open item rewritten |
| this step file (7) | history; its one code citation into `evaluate_lazy` de-cited, since `--citations` reads `plan_current/` |
| `adocs/plan_todo/S122_*` (2), `S082_*` (1), `S099_*` (1); `adocs/plan_current/S261_*` (1) | prose, owned by those steps, not edited: their citations still resolve (`evaluate_expensive`, `quiescence`). S122's "Today:" line and S082's quote of the datagen comment go stale with this landing (report) |
| `adocs/plan.md` (1), `adocs/status.md` (8), `adocs/decisions.md` (12) | the coordinator's; not edited (brief). `plan.md`'s writing-rule example cites `LAZY_EVAL_MARGIN` in `src/evaluation.hpp`, a citation that has not resolved since S073 |
| `adocs/plan_done/` (35 files), `adocs/audit/` (9), `adocs/worklog.md`, the other 17 `adocs/data/` files | history and evidence, never edited |

### Design

- **One change, weights frozen** (accepts (1)). `evaluate()` is
  `evaluate_cheap() + evaluate_expensive()` as before; `evaluate_expensive()`
  no longer clamps; nothing else in the evaluation moved -- no weight, no
  taper, no stage, no term. The two-stage split stays: the trace's sites and
  `tools/eval_spread` read it, and moving the pawn terms out of stage one
  would re-taper every score.
- **Quiescence**: the static score is the entry's evaluation where the probe
  carries one, else `evaluate()`, at every window; every store carries it.
  S130's substitution, S112's futility and every store are unchanged and now
  always start from an exact score. What changes in play: (a) the stand pat
  where the old shortcut fired is the score, not `cheap -/+ 184` -- a lower
  stand pat on fail-high nodes, a higher one on fail-low nodes -- which feeds
  S112's futility base and the fail-soft value; (b) quiescence entries carry
  an evaluation at nodes that stored none, which later probes read; (c) the
  0.338 % of positions past 184 score their whole stage two in both searches;
  (d) every quiescence node pays stage two (S120: about 5 % nps, a smaller
  tree).
- **The main search**: no code change; `negamax_at` already called
  `evaluate()`.
- **The trace and the tuner** (S262): `exact_score()` is the plain sum of the
  five parts, so the per-row identity holds against the unclamped engine;
  `smooth()` has no clamp and `gradient()` no skip; the objective has no kink.
- **No new constant, no seed.** The retired one, 184, was S085's SPSA output.
  The technique removed is the CPW's lazy evaluation; DEC-221's
  from-the-description rule has nothing to apply to.

### Tests

Retired, each with what holds its property now:

- `test_evaluation` "the lazy shortcut cannot change a decision", "the
  shortcut says when it took one", "a shortcut return is never better than
  the truth": `evaluate_lazy()` is gone. That the search reads the exact score
  at the windows the shortcut answered is asserted where it reads it:
  `test_search` "a quiet position stands pat" (exact at (0, 100), (-10000,
  10000) and (1000, 2000) -- the two that read 383 and 751 before) and "a
  quiescence entry carries the static score, never a bound" (the evaluation
  stored at all three windows). The widest-correction report moved to the
  case below.
- `test_tuner_gradient` "the fixture sits clear of the clamp at the shipped
  constants": a precondition for the clamp's kink, which no longer exists.

Re-pointed, never weaker:

- `test_evaluation` "the unclamped terms are the engine's own" -> **"the
  expensive stage is applied whole"**: `mobility + safety == evaluate() -
  evaluate_cheap()` exactly, where it was `clamp(...) ==`; non-vacuity on
  **both** sides past the retired 184 (was: past it on either side).
- `test_evaluation` "the static score never reaches the mate band": `+
  |evaluate()| <= INT16_MAX`, both populations.
- `test_search` "a quiet position stands pat": three exact equalities and the
  precondition `static != cheap`, where it asserted a bound below the score.
- `test_search` "a quiescence entry carries the static score, never a bound":
  the narrow half asserts the score where it asserted `TT_EVAL_NONE`; a high
  window half added.
- `test_search` S164's "a stand pat that is itself a bound is still capped" ->
  **"a stand pat at a narrow window is the score, still capped"**: S164's
  window, the cap and the fail-high store kept; the stand pat is the score and
  the entry carries it. **Its golden, 383, retires**: one of its two ends,
  `LAZY_EVAL_MARGIN`, no longer exists.
- `test_search` S112's seven futility cases and S131's queen-promotion case:
  `lazy_stand_pat()` becomes `empty_table_stand_pat()`, `evaluate()`; every
  window keeps its place relative to the stand pat, so each mutant in
  `tools/mutants/S112_qs_futility.py` and `S131_*` meets the same thresholds.
- `test_eval_trace` "the trace reconstructs evaluate() exactly, clamp
  included" -> **"the trace reconstructs evaluate() exactly"**: the identity
  unchanged, its precondition "stage two past the retired 184, both sides" on
  the same rows the clamp bound (478 of 2764 at `f57b92f`).
- `test_tuner_gradient` "the clamp's derivative is exact where it binds" ->
  **"the stage-two weights move the rows past the old margin"**: the same +40
  push and finite-difference check on all 823; on the rows past 184 the block
  a clamp would silence is asserted non-zero, where it was asserted exactly
  zero.
- `test_search_params`: 72 rows.
- **Goldens (DEC-142)**: 383 retired (above); `QUIET_ROOK_EVAL(_CHEAP)` and the
  ten anchors do not move (no anchor case reaches 184 -- the script prints
  each stage; prediction); `truncation_positions` is re-derived in phase 2 by
  its scan (DEC-169).

### Documents

`adocs/specs.md`, `MANUAL.md` and `DEV_MANUAL.md` updated (the table above);
`adocs/data/README.md` pinned and indexed. Checked: `MANUAL.md`'s option
surface and nothing else in it named the margin.

### Phase 2, pending (in order)

1. **Rebase** onto S261's landing, then S099 and S023 (both land before this
   SPRT). Expected conflicts: `adocs/specs.md` line 658, the search row, which
   all three edit (merge by hand); `tests/test_search_params.cpp`'s count lines
   (S099 +2, S023 +4: 79 before this, **78** after); `src/search.cpp`
   `negamax_at`'s static-evaluation comment beside S099's correction hunk.
   Re-grep the readers on the rebased tree.
   - **S023's two quiescence-futility cases are converted at this rebase**
     ("a capture's worst record makes a capture that could reach alpha
     futile", "a capture's best record searches a capture that could not
     reach alpha"). They are built on `lazy_stand_pat()`, `evaluate_cheap()`
     and `LAZY_EVAL_MARGIN`, none of which exists after this change, so they
     do not compile on the rebased tree until converted the way S112's are
     here: their `load_FEN` / `evaluate_cheap()` pair and `lazy_stand_pat()`
     call become `const int stand_pat = empty_table_stand_pat(PAWN_TAKES_PAWN);`
     and alpha becomes `stand_pat + margin + victim - 1` and `+ 1`
     respectively, every other line and threshold unchanged, so each keeps its
     place relative to the stand pat and its CX mutants meet the same
     thresholds.
2. **Build** `build` and `build-tune`; `./clang-format.sh` (two `MESSAGE` and
   one `printf` may reflow).
3. **Gate** (TESTS rule, both builds). Predicted green: the cases above. At
   risk, because the tree moves and each is re-derived by the script at its
   site, never relaxed: "ordering keeps the tree small" (`S192_node_budget.py`),
   `capture_mates` depths, the multicut and ProbCut mate rows (E21, B04),
   `DRIVE_DEPTH`, `first_mate_depth` / `mate_in`, `MATE_IN_THREE_FLOOR`,
   `EXACT_FLOOR`, `short_line_ceiling`, the census floors.
4. **Anchors**: `python3 adocs/data/S192_anchors.py`, predicted `10 of 10
   reproduced`.
5. **DEC-169**: `build/tools/truncation_scan --data
   .tuning/selfplay_v2_dedup.tsv --min 2.8` into
   `adocs/data/S039_truncation_scan.txt`. Predicted: the past-2.8 and
   at-2.875 counts rise (the clamp's rows now carry a residual), the four pins
   stay among the rows at 2.875 (none can have been clamped there). Record
   the reading at the GOLDEN block in `tests/test_eval_trace.cpp` and in
   `DEV_MANUAL.md`'s truncation section.
6. **Identity on the corpus**: `build/tools/tuner` loading the dedup corpus,
   predicted `0 differ from evaluate()`.
7. **Accepts (3)**: `bench` (new total), `tools/search_bench.py` at 9 and 12,
   and nodes, nps and wall to depth 11 over S103's 300 positions against the
   parent by S120's `nps_rotate.py` (8 rounds); fill `P2_PENDING` in
   `adocs/data/S039_sprt.sh`.
8. **Mutants**: `tools/mutation_check.py tools/mutants <wt> --only M29` and
   `--only M41`, predicted killed (M41 by "the expensive stage is applied
   whole", "the trace reconstructs evaluate() exactly" and the bench).
9. **Second tier** (accepts (6)): the Debug binary self-played, four rounds of
   fastchess at 4+0.04, log grepped for `Assertion`; `tools/gate_extra.sh`.
10. `tools/plan_prose_check.py --citations`, `--touches`, `--params`: clean.
11. CAND and REF pinned in `S039_sprt.sh` (the no-verdict action is
    DEC-274's, written); the SPRT, a night run (DEC-155).
12. Then the verdict's commit, this file's stamp, `plan.md` and `status.md`
    (coordinator).

### Proposed `done:` stamp

2026-10-?? -- the lazy shortcut and the clamp retired together, weights
frozen: evaluate() is the exact sum of both stages at every call, quiescence
stands pat on it and stores it at every window, LazyEvalMargin left the tune
build (<n> parameters at landing: 74 on 8ef820f, 78 with S023 verdict 1's
four), the trace and tuner carry no clamp. SPRT {-5, 0} at
8+0.08: <verdict, games, Elo, nElo> (a no verdict lands as H1, a zero with its interval, DEC-274); bench <n> (was <n>); nodes / nps / wall
to depth 11 over S103's 300: <...>. 398 readers at c88a961 and 400 at
8ef820f resolved (adocs/data/S039_readers.txt): 3 test_evaluation cases and 1
test_tuner_gradient case retired, 15 re-pointed (4 renamed), the 383 golden retired; M31
retired, M29 re-anchored, M41 added and killed; truncation pins re-derived
(DEC-169): <reading>; anchors 10 of 10. Second tier green: <...>. Fast suites
both builds, format, prose checks clean.

### Commit text (the candidate; the verdict's commit carries DEC-220's block)

```
Retire the lazy shortcut and the clamp together (S039)

evaluate_lazy() skipped mobility and king safety wherever the cheap
stage cleared the window by LAZY_EVAL_MARGIN, and evaluate_expensive()
clamped the two to the same margin so that bound held by construction.
The clamp also capped the real score: no king safety term could say
more than 184 centipawns, which S122's rebuild cannot live with, and
S121 fits mobility inside the same sum. DEC-258 retires both under one
non-regression verdict.

quiescence() now stands pat on evaluate(), exact at every window, and
stores it in every entry; evaluate_expensive() adds the stage whole;
LazyEvalMargin leaves the tune build. No weight moves. The trace's
exact score and the tuner's smooth score and gradient drop the clamp
with the engine, so the per-row identity still holds. The tests that
held the shortcut's bound now hold the exact score at the windows it
used to answer; the clamp's tests assert the stage applied whole.

Bench: <pending>
```

## Rebased onto `8ef820f` (2026-10-14, phase 1, blind)

Phase 1 again under DEC-260, in the worktree `/home/max/ws/chesso-s039c`
(branch `s039c` at `8ef820f`: S279, S261's vector, S099's candidate and their
documents on top of `c88a961`), while S099's SPRT holds every core.
`git apply --3way` of the phase-1 diff, conflicts resolved by hand. **Nothing
compiled, built, tested or run**; every outcome below is a prediction. No
commit, no ref moved. The phase-1 text above stands as the record on
`c88a961`; where it and this section disagree, this section is the tree.

### Deviations

- **Two readers the accepts' grep cannot see**, both present at `c88a961` and
  missed by phase 1, found by grepping the clamp's own words over `src/`,
  `tests/` and `tools/`: the GOLDEN block over `tests/test_evaluation.cpp`
  "each piece is worth what the tables say" named "the lazy shortcut cannot
  change a decision", a retired case, as its property -- re-pointed to "the
  expensive stage is applied whole"; and `tools/CMakeLists.txt`'s comment over
  `eval_spread`, which still said the tool sizes the lazy margin -- rewritten.
  Both are recorded at the foot of `adocs/data/S039_readers.txt`.
- **A phase-1 sentence claimed a reading not yet taken**: the truncation
  golden in `tests/test_eval_trace.cpp` said S039 "re-derived the four by the
  same scan". It now says the four are owed that scan, marked PENDING.
- **The count is 74, not 72**: S099 added two rows. Where the phase-1 text
  above says 73 to 72, read 75 to 74 on this base, and 79 to 78 once S023
  verdict 1's four are in. The proposed stamp says so.
- **Files outside `touches:`, PENDING marks only**: `tests/test_engine.cpp`,
  `tests/test_mate_breadth.cpp`, `tests/test_mate_carry.cpp`,
  `tests/test_eval_positions.hpp`; and `tools/CMakeLists.txt` (above).
- **The mutant-anchor check is by intersection, not anchor by anchor.**
  Extracting every anchor from `tools/mutants/` needs the Python that execs
  them, a tool run DEC-260 forbids. Instead every code line S039 removes or
  adds in `src/` was grepped against every file in `tools/mutants/`: only
  `tools/mutants/eval.py`'s M29 and M41 contain one, and each anchor occurs
  exactly once in `src/evaluation.cpp`; the anchors that carry comment lines
  (S099's CH03 and PK01 to PK04, S119's, `tools/mutants/search.py`'s null-move
  one) sit outside every comment S039 rewrites. An anchor S039 did not touch
  keeps the count it had on `8ef820f`, which S099's phase 2 validated.
  `tools/mutation_check.py` validates them all in phase 2.
- **More goldens marked PENDING than the brief listed**: the step's own
  at-risk list (the ordering budget, the six mate rows). The census floors in
  `tests/test_invariants.cpp` are not marked: they count a walk of
  `make_move`, which reads no evaluation and no search.

### Conflicts and resolutions

Five files conflicted; the rest merged clean in three ways.

- `tests/test_search_params.cpp`: S261's ProbCut rows (87, 3, 6) and S099's
  two rows kept, `LazyEvalMargin`'s row out; the count line "75 to 74" after
  S099's; GOLDEN 74.
- `MANUAL.md`: S261's three ProbCut rows kept, the `LazyEvalMargin` row out;
  S039's paragraph on the retired option merged clean.
- `DEV_MANUAL.md`: the `golden_defaults` row reads 74, S039's sentence first,
  S099's kept; the multicut and ProbCut rows, which S039 never changed, take
  S261's text (the conflict was adjacency).
- `adocs/specs.md`: the evaluation row is S039's (S261 and S099 did not touch
  it). The search row, which both rewrote, was merged word by word -- one
  word a line, `git merge-file` over the three versions, no word in conflict:
  S039's one edit there, the reverse-futility sentence's reason the positive
  mate edge is unreachable, lands inside S261's and S099's row unchanged.
  S099's "Quiescence is untouched: its stand pat reads the raw evaluation"
  stays true.
- `adocs/data/README.md`: both sections kept, S099's then S039's.
- **Clean, though the brief expected conflicts**: `src/search.cpp` (S279's
  `check_limits` above the probe, S099's raw and corrected split, S261's one
  line; S039's hunks are quiescence's static score and three comments, none
  adjacent), `src/search_params.hpp`, `src/data_structures.hpp` (S039 states
  no `board_t` size), `src/transposition_table.hpp`, `tools/datagen.cpp`
  (S099's table and S039's comment are apart), `tests/test_search.cpp`,
  `tests/test_evaluation.cpp`, `tools/mutants/eval.py` (no S099 or S261
  anchor moved into it). `src/evaluation.cpp` and `src/evaluation.hpp` merged
  over `e7b1d04`'s re-pointed comments without overlap.

### S099's raw evaluation

- **The correction reads and writes the raw full score.** `src/search.cpp`
  `negamax_at` computes `raw_eval` as the entry's evaluation or `evaluate()`;
  S039 changes no line there, only what `evaluate()` returns, now the
  unclamped sum. `corrected_static_eval` adds the slot to it, clamped inside
  the mate band's edge; the update learns `best_so_far - raw_eval`, gated by
  `correction_may_learn` on `raw_eval`. `corrected_static_eval` has one
  caller, `negamax_at`; quiescence never corrects.
- **The table stores raw at every store.** `negamax_at`'s node store and
  ProbCut's store pass `raw_eval`. Quiescence's four stores pass
  `stored_eval`, which since S039 is its `static_eval`: the entry's
  evaluation, written raw by every writer, or `evaluate()`. What S039 adds:
  quiescence entries now carry an evaluation at every node, so the read of
  the entry's evaluation in `negamax_at` hits more often -- the same number,
  `evaluate()` being a function of the position (S103 counted 0
  disagreements).
- **INV-5 holds.** `tests/test_evaluation.cpp`'s "a mirrored position reads the
  same corrected score" reads `evaluate()` on both boards and requires them
  equal, which "colour symmetry over every test position" asserts outright.
  The clamp S039 removes is odd-symmetric on a stage that is equal under the
  mirror, so equality holds with it or without it. Predicted green.

### New readers (accepts (2), at `8ef820f`)

400 lines in 91 files against 398 at `c88a961`. No new reader in `src/`,
`tests/` or `tools/`: S261 and S099 brought none. Two moved with their step
files (S099's to `plan_current/`, S261's to `plan_done/`); two are new prose
-- S099's step file ("stand pat reads the raw evaluation, from the entry or
`evaluate_lazy()`", true of S099's tree, history once S099 completes) and the
coordinator's `adocs/status.md` line describing this step. Neither is edited
here. Plus the two outside the names, under Deviations.

### Goldens marked PENDING phase 2

Each marked at its site with its script; values left at the parent's until
the script's reading replaces them:

- `tests/test_eval_trace.cpp` `truncation_positions` and the bound case's
  `worst > 2.8`, and `tests/test_eval_positions.hpp`'s four pins (DEC-053,
  DEC-169: `build/tools/truncation_scan`).
- `tests/test_mate_carry.cpp`'s ceilings 2, 5, 4, 0, 1, 2 and the budgets in
  `adocs/data/S170_cases.tsv` (S203's sweep, S039's grid added to
  `--ceilings`); a rise stops for a ruling, DEC-272 and DEC-284 being S099's.
- `tests/test_mate_breadth.cpp` `EXACT_FLOOR` 163 and `tests/test_engine.cpp`
  `MATE_IN_THREE_FLOOR` 11 (S156, S154); a drop stops for a ruling, DEC-283
  being S099's.
- `tests/test_engine.cpp` `first_mate_depth` and `mate_in` (S188's
  `first-mate`).
- `adocs/data/S192_anchors.py`'s ten: the six one-piece values over
  `tests/test_evaluation.cpp` "each piece is worth what the tables say",
  `tests/test_search.cpp` `QUIET_ROOK_EVAL` and `QUIET_ROOK_EVAL_CHEAP`, 198,
  -505 and -569. Predicted unchanged: no anchor's stage two nears 184.
- `tests/test_search.cpp` "ordering keeps the tree small" (S192's node
  budget), and the six mate rows: `mate_the_extra_ply_hides` (R01),
  `mate_the_multicut_hides` (E21), `mate_probcut_hides` (B04), the
  `capture_mates` depths and labels, `DRIVE_DEPTH`, `LATE_CAPTURE_POS`.
- Bench and the accepts (3) readings stay `P2_PENDING` in
  `adocs/data/S039_sprt.sh`.

### Where S023 verdict 1 meets this diff

S023's blind diff (`.tuning/coord/S023/s023_on_c3ed1ef.diff`, on `c3ed1ef`,
whose files equal `8ef820f`'s but for `adocs/status.md`) against this one,
hunk by hunk on the shared base:

- **Converts, `tests/test_search.cpp` "search: quiescence futility"**, the
  two cases S023 appends with its helper `qs_run_with_record`: "a capture's
  worst record makes a capture that could reach alpha futile" and "a capture's
  best record searches a capture that could not reach alpha". Each loses
  `REQUIRE(load_FEN(PAWN_TAKES_PAWN, &game));`, `const int cheap =
  evaluate_cheap(&game.board);` and `const int stand_pat =
  lazy_stand_pat(PAWN_TAKES_PAWN, alpha, beta);`, gains `const int stand_pat
  = empty_table_stand_pat(PAWN_TAKES_PAWN);` above alpha, and its alpha
  becomes `stand_pat + margin + victim - 1` (worst record) and `+ 1` (best
  record) in place of `cheap + LAZY_EVAL_MARGIN + margin + victim - 1` and
  `+ 1`. Every other line stays, `qs_run_with_record` included: the record
  moves no stand pat, so each window keeps its place relative to it and the
  CX mutants meet the same thresholds. No textual overlap with S039's hunks;
  a compile error until converted.
- **Conflict, mechanical**: `tests/test_search_params.cpp`, the count
  paragraph and the GOLDEN line (S023's 75 to 79, then S039's 79 to 78; 78);
  `DEV_MANUAL.md`'s `golden_defaults` row (78, both sentences);
  `adocs/specs.md`'s search row (by word, as here); `adocs/data/README.md`
  (both sections, S023's then S039's).
- **Conflict, mechanical, the PENDING marks**: both diffs insert a mark above
  the same GOLDEN lines -- `tests/test_engine.cpp` (first-mate,
  `MATE_IN_THREE_FLOOR`), `tests/test_mate_breadth.cpp`,
  `tests/test_mate_carry.cpp`, and in `tests/test_search.cpp` the R01, E21,
  B04, `capture_mates` and `LATE_CAPTURE_POS` blocks. By S039's rebase S023's
  marks are its phase-2 readings: keep S023's text, put S039's mark back
  above it, and read "the parent's value" as S023's.
- **Clean**: `src/search.cpp` (S023's quiescence hunks replace
  `futility_base` with a per-move margin below the stand pat; they read the
  stand pat, exact since S039, and no line S039 changes), `MANUAL.md`,
  `src/search_params.hpp`, `src/data_structures.hpp`, the rest of the tests.
- **A new reader on landing**: S023's step file says quiescence's static
  score is "the entry's `eval` or `evaluate_lazy()`" -- history once S023 is
  in `plan_done/`, one more line for this step's grep.

### On S099's H0

`8758622` reverted. These S039 hunks meet S099's lines and re-resolve: the
`adocs/data/README.md` section (after S278's), the evaluation and search rows
of `adocs/specs.md` (the row's S099 sentences go; S039's sentence is outside
them), `DEV_MANUAL.md`'s `QUIET_ROOK_EVAL` and `golden_defaults` rows (73 to
72), `src/search.cpp` `negamax_at`'s static-score comment (S099's "Raw, and
what the table stores" paragraph and the `raw_eval` split go; S039's
paragraph above them reads true on either tree), `tests/test_search_params.cpp`
(73 to 72, GOLDEN 72), and the PENDING marks in `tests/test_mate_breadth.cpp`,
`tests/test_mate_carry.cpp` and over `QUIET_ROOK_EVAL`, whose neighbouring
lines S099 wrote; their DEC-272, DEC-283 and DEC-284 clauses go with S099.
Nothing else S039 changes touches a line S099 added.

### The SPRT script

`adocs/data/S039_sprt.sh`: CAND and REF stay `PIN_ME`; REF is now S023
verdict 1's outcome on achesso (its H1 closing commit or its revert), CAND
S039 committed on REF or a docs-only descendant, and S099's guard is added --
it refuses unless `git diff REF CAND^ -- src/` is empty. The open-findings
line was re-read against `adocs/plan.md`'s Open list at `8ef820f`: none; S281
is a study conditional on S099's H1. A paragraph states that S039 changes the
raw score S099's correction is added to and learns against, and no line of
the correction. Bounds `{-5, 0}`, DEC-143's 25591 and 41861 games, the cap,
the abort rule and DEC-274's three outcomes unchanged.

### Phase 2, pending (supersedes the list above)

On S023's outcome, at 2110 games an hour (`.moltke.local.md`); the other times
are S099's recorded ones unless marked *est.*

1. Rebase onto S023's outcome by the meeting points above; convert its two
   cases; re-grep the readers. No machine.
2. Configure and build `build`, `build-tune`, `build-debug`: 2 min (S099's
   118 s), 15 min if configured fresh.
3. The TESTS gate, both builds and the format check: 5 min. A red outside
   the PENDING goldens stops phase 2.
4. `bench`, `bench 12`, `tools/search_bench.py` at 9 and 12 on both sides;
   `adocs/data/S192_anchors.py`; `adocs/data/S192_node_budget.py`: 10 min.
5. Accepts (3): nodes, nps and wall to depth 11 over S103's 300 by S120's
   `nps_rotate.py`, 8 rounds, REF, CAND and a REF copy: *est.* 10 min.
6. DEC-169: `build/tools/truncation_scan --data .tuning/selfplay_v2_dedup.tsv
   --min 2.8`, and `build/tools/tuner` loading the same corpus for the
   identity: *est.* 10 min.
7. The S170 grid by S203's script and `--ceilings` with S039's grid added:
   5 min (279 s split six ways at S099).
8. The floors (S154 `floor` and `red`, S156) and S188's `first-mate`: 15 min.
9. Mutants, S280's recipe: M29, M41, S112's F and S131's mutants the
   re-pointed cases kill, and the mate-row guards (E21, B04, S091's C and R
   mutants, S269's LQ02 and LQ03): *est.* 45 min (S099: 15 in 1916 s).
   A row whose guard survives, or that the gate turned red, is re-mined by
   the script at its site: 0 to 60 min.
10. The gate again with every golden in: 5 min.
11. Debug self-play, four rounds at 4+0.04, grepped for `Assertion`: 10 min.
12. `tools/gate_extra.sh`, five stages: 17 min (1019 s at S099).
13. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`:
    seconds.

**About 2.5 h, 3.5 h with every mate row re-mined**: under DEC-155's four,
a day's work. Then the SPRT, a night: 12.1 h on a bound (25591 games), the
40000-game cap at 19.0 h, the midpoint's 41861 lying past it.

### What phase 2's compiler and tests are expected to say

- **Compiler**, Release, tune and Debug at -Wall -Wextra -Werror: nothing.
  No caller of `evaluate_lazy`, `LAZY_EVAL_MARGIN`, `static_eval_is_exact`,
  `clamped_site` or the load report's clamp count is left in `src/`,
  `tests/` or `tools/` (grep); quiescence reads both `static_eval` and
  `stored_eval`; the tune build's X-list loses one row and nothing reads it.
- `test_search_params`: 74 rows, green in both builds.
- `test_evaluation`: "the expensive stage is applied whole" green, both
  sides past 184 reached; "the static score never reaches the mate band"
  green with its 16-bit ceiling; S099's mirror case green; the anchors green.
- `test_eval_trace`, `test_tuner_gradient`: green; the four pins still at
  2.875 (prediction, the scan decides).
- `test_search`: the re-pointed cases, S279's three and S099's correction
  suite green; "ordering keeps the tree small" green inside its 4x and 5x
  band; the six mate rows green unless the tree moved them.
- `test_mate_carry`: **the likeliest red**. Its ceilings have margin 0 and
  its budgets are a knife edge (DEC-154); any tree change can move a short
  line. Re-swept; a rise stops for a ruling.
- `test_mate_breadth` and `test_engine`'s floors and first-mate depths: at
  risk, margin 1; re-read by their scripts.
- `test_uci_surface`: the release golden's 5 lines unmoved; the tune build's
  lines follow `search_param_info()`.
- The rest of the fast suite unaffected.

## Rebased onto `4e6ca63` (S099 reverted, S023 v1 in) (2026-10-15, phase 1, blind)

Phase 1 a third time under DEC-260, in the worktree
`/home/max/ws/chesso-s039d` (branch `s039d` at `4e6ca63`: S099's revert
`5a04ffa` and its completion `7e36271`, S023 verdict 1's candidate `adb5d78`
and the commit pinning it), while S023 verdict 1's SPRT holds every core.
`git apply --3way` of `s039_on_8ef820f.diff`, conflicts resolved by hand,
S023's two quiescence cases converted. **Nothing compiled, built, tested or
run**; every outcome below is a prediction. No commit, no ref moved. Main has
since moved to `bb20f35`, documents only (DEC-289, S284), which changes
nothing in this base. Where this section and the two above disagree, this
section is the tree. Line numbers below are this tree's, written in words:
the `path:line` form fails `--citations` in a pending file (DEC-135).

### Deviations

- **The ordering budget's PENDING mark is re-worded** (`tests/test_search.cpp`
  "ordering keeps the tree small"). It said the pair is re-derived "only if it
  leaves the band's middle half". That verdict is S282's open defect -- the
  script reads every freshly derived count as outside its own middle half --
  and S023 kept the pair on that account. The mark now keeps the pair while
  the count is inside the band and sends a count outside it to the owner
  before the pair moves.
- **The marks sit below S023's "Re-read at S023" paragraphs, directly above
  each GOLDEN line**, where the 8ef820f plan said "above S023's text", so that
  "the script named below" stays beside the block naming it: first-mate and
  `MATE_IN_THREE_FLOOR` in `tests/test_engine.cpp`, B04, `capture_mates` and
  the LQ block in `tests/test_search.cpp`. R01's and E21's GOLDEN lines are
  S023's own, with no paragraph between.
- **The marks' S099 clauses are replaced, not dropped.** The two floors'
  "DEC-283's allowance is S099's alone" reads "S039 holds no allowance to
  lower it"; the carry ceilings' "DEC-272 and DEC-284 are S099's alone" reads
  "A rise, or a budget the rule loosens, goes to the owner for a ruling:
  DEC-287 is S023's alone" -- the brief's stop list, budgets included.
- **REF in the SPRT script is wider than the brief's.** S023 verdict 1's
  outcome as it lands, or, if a step ahead of S039 in the Open list lands an
  engine change first -- S283's H1 relanding S099, S023 verdict 2, S265 --
  that landing, with S039 rebased onto it. The `git diff REF CAND^ -- src/`
  guard holds the pair to one change either way. At `4e6ca63` the Open list
  reads S023, S282, S283, S025, S265, then S039.
- **The anchor check is by awk.** No `gawk` here and no Python (DEC-260):
  `mawk` tokenised every file of `tools/mutants/` -- string literals with
  implicit concatenation and escapes -- took the first string of every
  `(str, str)` tuple and every module-level `NAME = (str)` (S113's and S116's
  `GATE`, S131's `FILTER`), and counted each in the file its `m()` names: 208
  strings over 27 files and 199 mutants. All 182 literal anchors and the three
  helper anchors occur **exactly once** on the merged tree, as on `4e6ca63`;
  the other 23 strings are the arguments of the 23 `gate_with()` and
  `filter_with()` calls, substrings of `GATE` and `FILTER` and not anchors, 9
  of them reading 2 to 7. Only `tools/mutants/eval.py` differs
  from `4e6ca63`: M29 re-anchored onto `stage_two`, M31 gone, M41 new, each
  once. `tools/mutation_check.py` `validate` repeats it over the whole
  directory before `--only` selects, in phase 2.
- **The H0 differences were taken by applying, not by reading alone.**
  `7e36271`'s tree exported into the scratchpad, `S023_h0_band_case.diff`
  applied, then this step's diff with `git apply --reject` outside any
  repository; no build. The rejects are listed under "On S023's H0".
- `adocs/data/S039_readers.txt` gains a third reading, and
  `adocs/data/S039_sprt.sh` its S023 paragraph, the open-findings line and a
  sentence on S284.

### Conflicts and resolutions

Eight files, fourteen hunks; every other file merged clean in three ways,
all of `src/` among them. The eight as `git apply` left them are
`.tuning/coord/S039/*.conflict2.bak`.

- `tests/test_search.cpp`, five: the PENDING marks over R01
  (`mate_the_extra_ply_hides`), E21 (`mate_the_multicut_hides`), B04
  (`mate_probcut_hides`), `capture_mates` and S269's LQ pair. S023's text
  kept -- its re-mined R01 row (`k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80` at
  8, mate in 4), its E21 row (depth 12, mined 11..14 by DEC-288) and its "Read
  at S023" paragraphs -- with S039's mark directly above each GOLDEN line.
- `tests/test_engine.cpp`, two: first-mate and `MATE_IN_THREE_FLOOR`, S023's
  paragraphs kept, the marks below them.
- `tests/test_mate_breadth.cpp`: GOLDEN `EXACT_FLOOR` **160**, S023's (S099's
  163 gone with it).
- `tests/test_mate_carry.cpp`: ceilings **0, 5, 5, 0, 1, 1**, S023's under
  DEC-287 (S099's 2, 5, 4, 0, 1, 2 gone).
- `tests/test_search_params.cpp`, two: S023's count paragraph (73 to 77), then
  S039's sentence, **77 to 76**; GOLDEN **76**. The 76 rows of
  `golden_defaults` and the 76 `X(...)` rows of `src/search_params.hpp` agree
  in name, default, minimum and maximum, in order (compared as text).
- `DEV_MANUAL.md`: the `golden_defaults` row is S023's ("make 77") with
  S039's sentence first: **76** since S039, 77 to 76.
- `adocs/specs.md`: the evaluation row is S039's, unchanged by the other
  side (the conflict was adjacency); the search row merged word by word,
  `git merge-file` over `8ef820f`, `4e6ca63` and S039, one word a line: no
  word in conflict, S039's one edit (the positive mate edge "the evaluation's
  own arithmetic makes unreachable") inside S023's row, S099's sentences gone.
- `adocs/data/README.md`: S099's section (its H0 logs) and S023's kept,
  S039's after them.

### S023's two quiescence cases, converted

`tests/test_search.cpp`'s "a capture's worst record makes a capture that could
reach alpha futile" (line 2442) and "a capture's best record searches a
capture that could not reach alpha" (line 2483), as planned at `8ef820f`:
`REQUIRE(load_FEN(...))`, `evaluate_cheap()` and the stand pat read from the
lazy shortcut out; `const int stand_pat =
empty_table_stand_pat(PAWN_TAKES_PAWN);` above alpha (lines 2456 and 2495);
alpha `stand_pat + margin + victim - 1` and `+ 1`. Every other line,
`qs_run_with_record` and every `REQUIRE` and `CHECK` unchanged.

### S023's quiescence: what the margin reads on S039's tree

- **The stand pat**, `src/search.cpp` `quiescence`: `static_eval` is the
  entry's evaluation or `evaluate()` at every window (lines 1140 to 1143),
  where `4e6ca63` called the lazy form with the node's alpha and beta;
  `stand_pat = static_eval` (line 1163); S130's substitution unchanged (1184
  to 1200); the stand-pat cutoff and alpha raise (1217 to 1236). **The
  record**: `src/data_structures.hpp`'s `capture_history_entry` (line 1157),
  read at line 1334. **The margin**: `src/search.cpp`'s
  `capture_futility_margin` (lines 222 to 227), `max(0, QsFutilityMargin + 94
  * entry / 32767)`, read at line 1335. **The test**: `stand_pat + margin +
  qs_futility_value[victim] <= alpha` (1336 to 1338). S039 changes no line
  from the S112 comment at 1245 to the end of the loop; it changes what
  `stand_pat` holds at the windows the shortcut answered: the cheap stage
  plus 184 on the fail-low side, less 184 on the fail-high side, the score
  now.
- **What S023's cases assume, and it holds.** A cold table (`tt_reset` in
  `tests/test_search.cpp`'s `qs_run_with_record`), so no entry evaluation and no
  substitution: the stand pat is `evaluate()` of the position, which
  `tests/test_search.cpp` `empty_table_stand_pat` reads. The record is planted
  in every cell and quiescence reads it and writes nothing, so the margin is
  `capture_futility_margin` of the planted value, which
  `search_capture_futility_margin_probe` reads: 132 for the worst, 320 for the
  best, 226 for an empty cell at the defaults.
- **Which change their precondition: those two, and only their stand pat.**
  Each alpha is rebuilt one off the empty cell's best case from the new stand
  pat, so each inequality keeps its value: the worst case's `futility_value <=
  alpha` holds iff `worst < margin` (132 < 226), the best case's `stand_pat +
  best + victim > alpha` iff `best > margin + 1` (320 > 227) -- the same
  parameter inequalities as before, which both cases already `REQUIRE`.
  CX04 and CX06 meet the same thresholds.
- **Unchanged**: "the capture margins move with the record and never go below
  zero" (arithmetic) and `test_search_params`' bound case (no stand pat); the
  table cases ("the capture table is indexed by mover, target and victim", the
  credit and charge cases, "a search fills the capture table both ways");
  S023's band case in `test_evaluation`. S112's seven and S131's one were
  converted at `c88a961` and read the empty cell's margin, `QsFutilityMargin`
  exactly, as S023's fast check found.
- **Tree-dependent, at risk, nothing from the stand pat in their
  preconditions**: the worst-record mate window (`tests/test_search.cpp`'s
  `WORST_RECORD_WINDOW`, DEC-285) and the probe-driven capture-rule cases ("a
  capture's record moves the capture SEE margin at a pruning node", "no
  capture that holds its material is skipped whatever its record", C02's,
  C05's, LQ02's, LQ03's), whose drives write the records as they go. S039
  moves leaf values, so the records a drive leaves can move. A red there is
  not re-pointed by S039: the window is S023's (DEC-285) and S039 holds no
  allowance.

### Readers (accepts (2), at `4e6ca63`)

402 lines in 92 files, against 400 in 91 at `8ef820f`; recorded as the third
reading in `adocs/data/S039_readers.txt`. **New in code**: S023's two alphas
in `tests/test_search.cpp`, beside two calls of the file's lazy stand-pat
helper that the names miss -- converted above. **New prose**: one line of
S023's step file, history once S023 completes, before S039 lands. **Moved**:
S099's two prose lines, to `plan_done/`. **Left**: one `adocs/status.md`
line. **Outside the names** (lazy, clamp over `src/`, `tests/`, `tools/`):
every clamp S099 brought left with `5a04ffa`, none of them the stage-two
clamp or a reader of it; S023's two "clamp" words name capture history's own
bonus clamp. On the merged tree no reader remains in code; comments aside,
grep finds none.

### What left with S099

- From this diff: the marks' DEC-272, DEC-283 and DEC-284 clauses; the count
  line 75 to 74 (now 77 to 76) and DEV_MANUAL's S099 sentence; the SPRT
  script's paragraph on the correction; the README section's place after
  S099's alone.
- From this file's reasoning: "S099's raw evaluation" above is history -- no
  raw split, no corrected score, no pawn table; `src/search.cpp` `negamax_at`
  takes the entry's evaluation or `evaluate()` (lines 1757 to 1759) and stores
  it, as before S099. S099's mirrored corrected case left with it;
  `tests/test_evaluation.cpp` "colour symmetry over every test position"
  holds INV-5. S099's mutant files left `tools/mutants/`. S281 is void.
- Nothing else: no S039 hunk in `src/`, `tests/` or `tools/` touched a line
  S099 wrote (every added line grepped for S099's names: none).

### Goldens PENDING phase 2

Nineteen marks, each at `4e6ca63`'s value with its script named at the site:

- `tests/test_eval_trace.cpp` `truncation_positions` and the bound case's
  `worst > 2.8`, `tests/test_eval_positions.hpp`'s four pins:
  `build/tools/truncation_scan` (DEC-169).
- `tests/test_mate_carry.cpp` ceilings 0, 5, 5, 0, 1, 1 and the S170 budgets
  A 500000, C 500000, D 1500000, E 100000 (S023's, DEC-287): S203's sweep,
  with S039's grid added to `--ceilings`.
- `tests/test_mate_breadth.cpp` `EXACT_FLOOR` 160 (S156's script);
  `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` 11 (S154's `floor` and `red`)
  and first-mate 9 and 9, mate 5 (S188's `first-mate`).
- `adocs/data/S192_anchors.py`'s ten: the six piece values, 563 and 567, 198,
  -505, -569. Predicted unchanged.
- The mined rows in `tests/test_search.cpp`: R01 at 8, mate in 4 (S095's and
  S230's scripts); E21 at 12, mate in 4 (S097's six, guard mode, 11..14 by
  DEC-288); B04 at 10, mate in 5 (S277's six); `capture_mates` at 9, 8, 11 and
  12 with their labels (S230's seven sweeps), the worst-record window moving
  with them; `DRIVE_DEPTH` 4 (S188's `s207`); the LQ pair (S269's commands,
  DEC-281).
- `tests/test_search.cpp` "ordering keeps the tree small", 65024 and 3251
  (`S192_node_budget.py`), kept while the count is inside the band (S282).
- Bench and the accepts (3) readings: `P2_PENDING` in
  `adocs/data/S039_sprt.sh`.

The rule for each, from the brief: a carry ceiling that rises, a budget or a
floor loosened, or a mate floor dropped stops phase 2 for the owner; a mined
row that stops separating is re-mined by its script (DEC-288), and only "no
separator one band deeper" stops.

### On S023's H0 or no verdict, per hunk

That tree is `7e36271`'s `src/`, `tests/` and `tools/` (bench 5636835) with
S023's band case kept without its record loop. Applied there: every `src/`
and `tools/` hunk, `tools/mutants/eval.py` included, and `MANUAL.md`,
`S192_anchors.py`, S122's file, `tests/test_eval_positions.hpp`,
`test_eval_trace.cpp`, `test_evaluation.cpp` (beside the band case) and
`test_tuner_gradient.cpp`, all clean; no reader left in code, every mutant
anchor once. The rejects, each mechanical:

- `tests/test_search.cpp`: the two conversions -- **dropped**, the cases leave
  with the revert. The R01, E21, B04, `capture_mates` and LQ marks -- put back
  above `7e36271`'s GOLDEN lines, with no S023 paragraph: R01 back to
  `4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1` at 8, mate in 2, a row
  S023 found no longer separating; E21 back to S261's at 10, mate in 5. The
  ordering mark applies but says "as S023 kept it": read "the pair stays while
  the count is inside the band until S282 restates the trigger". The
  capture-rule cases lose their probe reads with the revert; S039 touches
  none of them.
- `tests/test_search_params.cpp`: no S023 paragraph; S039's sentence "The
  count goes 73 to 72"; GOLDEN 72; no `CaptHist` rows.
- `tests/test_engine.cpp`: both marks above `7e36271`'s GOLDEN blocks.
- `tests/test_mate_breadth.cpp`: the mark above `EXACT_FLOOR` 159 (S261's,
  ends 160 and 156).
- `tests/test_mate_carry.cpp`: the mark above S261's 0, 5, 4, 0, 0, 1 and its
  budgets; the DEC-287 clause goes, it never lands.
- `DEV_MANUAL.md`'s `golden_defaults` row: `7e36271`'s with "**72** ... since
  S039, ... 73 to 72" first. `adocs/specs.md`'s search row: merged by word
  again. `adocs/data/README.md`: S039's section after what the revert leaves.
- `adocs/data/S039_sprt.sh`: the S023 paragraph goes; REF is the revert.
- **The extra-ply row comes back stale** (DEC-288's consequences): a filler
  step re-mines it on the reverted tree; if S039's phase 2 comes first, its own
  re-mine of the row on S039's tree covers it.
- The mutant list loses CX01 to CX12 and gains nothing.

### The SPRT script

`adocs/data/S039_sprt.sh`: CAND and REF stay `PIN_ME`; REF is S023 verdict
1's outcome as it lands -- its H1 closing commit, documents only on
`adb5d78`, or the revert with the band case -- or the landing of a step ahead
of S039 if one comes first; the guard that `git diff REF CAND^ -- src/` is
empty stays. S099's paragraph is replaced by one on S023: both sides carry
the table, and S039 changes only the stand pat its futility margin is added
to. The open-findings line, re-read against `adocs/plan.md`'s Open list at
`4e6ca63`, names **S282**, DEC-171 filler, with its one bearing on this step;
S281 is void. One sentence on S284 (DEC-289): `--nonreg`'s default path and
its 20000-round cap are the same before and after it. Bounds `{-5, 0}`,
DEC-143's 25591 and 41861 games, 2110 games an hour, the cap, the abort rule
and DEC-274's three outcomes unchanged.

### Phase 2, pending (supersedes both lists above)

On S023's outcome; times are S023's recorded ones unless marked *est.*; 2110
games an hour (`.moltke.local.md`).

1. Rebase onto the outcome: on H1 nothing beyond the documents commit; on H0
   the list above. Re-grep the readers. No machine.
2. Configure and build `build`, `build-tune`, `build-debug`: 2 min (S099's
   118 s), 15 min configured fresh.
3. The TESTS gate, both builds and the format check
   (`CLANG_FORMAT_MAJOR=22`): 4 min (S023's gates, 180 to 270 s). A red
   outside the PENDING goldens and the at-risk cases stops phase 2.
4. `bench`, `bench 12`, `tools/search_bench.py` at 9 and 12 on both sides;
   `S192_anchors.py`; `S192_node_budget.py`: *est.* 10 min.
5. Accepts (3): `nps_rotate.py`, 8 rounds, REF, CAND and a REF copy: *est.*
   10 min.
6. DEC-169: `build/tools/truncation_scan --data
   .tuning/selfplay_v2_dedup.tsv --min 2.8`, and the tuner's load of the same
   corpus for the identity: *est.* 10 min.
7. The S170 grid by S203's script and `--ceilings` with S039's grid added:
   5 min (S099's 279 s).
8. The floors (S154's `floor` and `red`, S156's) and S188's `first-mate`:
   *est.* 15 min.
9. Mutants, DEV_MANUAL.md's recipe with S280's line: M29 and M41; S112's F01
   to F06; S131's V06, V09 and V10; S023's CX01 to CX12 (CX05 equivalent,
   eleven runs); S091's C02, C05, C06, C07, R01, R02; S269's LQ01 to LQ03; E21
   and B04 -- 33 runs at S023's 112 s a mutant (2474 s for 22): **62 min**. A
   row whose guard survives, or that the gate turned red, re-mined by its
   script: 0 to 60 min.
10. The gate again with every golden in: 4 min.
11. Debug self-play, four rounds at 4+0.04, grepped for `Assertion`: *est.*
    10 min.
12. `tools/gate_extra.sh`, five stages: 16 min (929 s at S023).
13. `tools/plan_prose_check.py`, one mode a call: `--prose`, `--citations`,
    `--touches`, `--params`, `--gate`: seconds.

**About 2.5 h, 3.5 h with every mate row re-mined**: under DEC-155's four,
daytime. Then the SPRT, a night: 12.1 h on a bound (25591 games), the
40000-game cap at 19.0 h, the midpoint's 41861 past it.

### What phase 2's compiler and tests are expected to say

- **Compiler**, Release, tune and Debug at `-Werror`: nothing. No caller of
  the lazy form, the margin, the lazy stand-pat helper,
  `static_eval_is_exact`, the trace's clamped site or the load report's clamp
  count in `src/`, `tests/` or `tools/` (grep, comments aside); S023's
  converted cases use every local they declare; `src/evaluation.hpp` still
  needs `search_params.hpp` for `CONT_HIST_WEIGHT`.
- `test_search_params`: 76 rows in both builds; S023's capture-weight cases
  unchanged.
- `test_search`: S023's two converted cases green by the arithmetic above;
  S112's seven, S131's one, S164's, the stand-pat and the entry cases green;
  S023's table cases green. At risk with the tree, each a stop or a re-mine by
  its own rule and never relaxed: the five mined rows, the worst-record
  window, the ordering band, `DRIVE_DEPTH`, the LQ pair, and the probe-driven
  capture-rule cases.
- `test_evaluation`: "the expensive stage is applied whole" and the 16-bit
  ceiling green; S023's band case green, `score_move` reading no evaluation;
  the anchors green.
- `test_eval_trace`, `test_tuner_gradient`: green; the four pins predicted
  still at 2.875, the scan decides.
- `test_mate_carry`: the likeliest red, its ceilings at margin 0 (DEC-154);
  a rise stops.
- `test_mate_breadth`, `test_engine`: at risk, margin 1; re-read by their
  scripts.
- `test_uci_surface`: the release golden's 5 lines unmoved; the tune build's
  lines follow `search_param_info()`.
- The rest of the fast suite unaffected.

### Proposed `done:` stamp: the clauses that change

"LazyEvalMargin left the tune build (76 parameters at landing on S023 verdict
1's H1, 72 on its H0)"; "398 readers at c88a961, 400 at 8ef820f and 402 at
4e6ca63, resolved (adocs/data/S039_readers.txt), S023's two quiescence cases
converted with S112's"; every other clause as proposed above.

## Rebased onto `769ce7a` (S023 reverted) (2026-10-15, phase 1, blind)

S023 verdict 1 read H0 and was reverted (`0cdd39a`); DEC-290 makes S039 the
next engine landing, its phase 2 and candidate before S283's run and its
SPRT after it. Phase 1 a fourth time under DEC-260, in the worktree
`/home/max/ws/chesso-s039e` (branch `s039e` at `769ce7a`: S023's H0 record,
its revert, its completion, the ledger and S284), while S285's agent mines on
the machine. `git apply --3way` of `s039_on_4e6ca63.diff`, conflicts resolved
from the per-hunk H0 list in the section above. **Nothing compiled, built,
tested or run**; every outcome below is a prediction. No commit, no ref
moved; the resolved paths were added to the index (the coordinator's
permission), so nothing is left unmerged. On this tree `src/`, `tools/`,
`MANUAL.md`, `adocs/specs.md` and `adocs/data/S170_cases.tsv` are `7e36271`'s
byte for byte, and `tests/` is too but for S023's band case without its record
loop and S284's script test: bench 5636835. Where this section and those above
disagree, this section is the tree; the H1 analysis above is history. Line
numbers are this tree's, in words (DEC-135).

### Deviations

- **The count is 73 to 72, not 72 to 71.** `src/search_params.hpp` holds 73
  `X(...)` rows at `769ce7a` and `tests/test_search_params.cpp`'s
  `golden_defaults` 73; S039 takes one from each, and the two tables, 72 rows,
  agree in name, default, minimum and maximum, in order (compared as text).
  The "72 to 71" the brief floated is the stale figure `DEV_MANUAL.md`'s
  goldens row carried.
- **Two stale golden statements on this tree corrected**, both in blocks this
  step edits and both put back by S023's revert, which had corrected them:
  `tests/test_mate_carry.cpp`'s GOLDEN line read "6, 15, 5, 2, 11 and 5" --
  S116's six, pre-S202 -- against the 0, 5, 4, 0, 0, 1 that
  `short_line_ceiling` ships; it now states the six it ships, with the history
  S023 had written (S202's walk, DEC-251; S261's F, DEC-282) and one sentence
  saying the stale six were back from S023's revert until S039. And
  `DEV_MANUAL.md`'s `golden_defaults` row read **71** against a table of 73
  (stale since S116); it reads **72** since S039, 73 to 72, with S116's two
  rows making 73 in its history. No number in a test moves.
- **The ordering budget's mark is the coordinator's text** (S282's phase 2):
  the middle half `[7270, 32512]` it names is S282's rule, which is not in
  this tree yet; S282 lands before phase 2.
- **The extra-ply row's mark names S285's row**, which is not in this tree yet:
  the GOLDEN block below the mark is still `7e36271`'s stale row
  (`4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1` at 8, mate in 2), which
  S285 replaces before phase 2.
- **The SPRT script's refusal message is re-worded** (an `echo` string, no
  behaviour): REF is the last commit before the candidate, on the revert.

### Conflicts and resolutions

The eight files predicted, fifteen hunks; every other hunk merged clean, all
of `src/` and `tools/` among them. As `git apply` left them:
`.tuning/coord/S039/*.conflict3.bak`.

- `tests/test_search.cpp`, six. **The S023 block, dropped whole**: a
  modify/delete over `qs_run_with_record` and the two cases this step had
  converted, which the revert deleted. R01 and E21: `7e36271`'s GOLDEN lines
  (R01 at 8, mate in 2; E21 S261's, depth 10) with the marks above them, R01's
  naming S285. B04, `capture_mates` and the LQ pair: S023's "Re-read at S023"
  paragraphs dropped, the marks alone above the GOLDEN lines.
- `tests/test_engine.cpp`, two: S023's paragraphs dropped, the marks kept.
- `tests/test_mate_breadth.cpp`: the mark above `EXACT_FLOOR` **159** (S261's;
  ends 160 and 156).
- `tests/test_mate_carry.cpp`: the mark above S261's ceilings, its clause
  "S039 holds no allowance" in place of DEC-287's; the stale GOLDEN line
  corrected (Deviations).
- `tests/test_search_params.cpp`, two: S039's sentence after S261's paragraph,
  "The count goes 73 to 72"; GOLDEN 72.
- `DEV_MANUAL.md`: the `golden_defaults` row, 72 (Deviations).
- `adocs/specs.md`: the evaluation row S039's; the search row merged by word
  (`4e6ca63`, `769ce7a`, S039), no word in conflict, S039's one sentence in
  `7e36271`'s row and no S023 sentence left.
- `adocs/data/README.md`: S023's section with its SPRT log row, then S039's.

### Readers (accepts (2), at `769ce7a`)

400 lines in 92 files, the fourth reading in `adocs/data/S039_readers.txt`.
Left since `4e6ca63`: S023's two alphas, with their cases. Moved: S023's step
file line, to `plan_done/`. Nothing new; no reader in code on the merged tree,
comments aside.

### Mutant anchors (the mawk check, again)

26 files, 188 mutants, 197 strings: all 171 literal anchors and the three
helper anchors (`GATE` twice, `FILTER`) occur **exactly once** on the merged
tree; the other 23 are the arguments of the 23 `gate_with()` and
`filter_with()` calls, as before. Against `769ce7a` only `tools/mutants/eval.py`
moves: M29 re-anchored, M31 gone, M41 new, each once.

### Quiescence on this tree

`src/search.cpp` `quiescence`: the static score is the entry's or `evaluate()`
at every window (line 1045 on), `stand_pat = static_eval` (line 1068),
S112's `futility_base = stand_pat + QS_FUTILITY_MARGIN` (line 1166), each
capture's `futility_base + qs_futility_value[victim]` (line 1235). No capture
history, so S112's seven and S131's one, converted at `c88a961`, are the whole
of what reads the stand pat against the futility margin.

### Goldens PENDING phase 2

Nineteen marks, each at `769ce7a`'s value (`7e36271`'s) with its script named
at the site, and S282's and S285's changes read when phase 2 fast-forwards
onto them:

- `truncation_positions`, the bound case's `worst > 2.8` and the four pins
  (`build/tools/truncation_scan`, DEC-169).
- `tests/test_mate_carry.cpp` ceilings 0, 5, 4, 0, 0, 1 and S261's budgets in
  `adocs/data/S170_cases.tsv` (S203's sweep, S039's grid added to
  `--ceilings`).
- `EXACT_FLOOR` 159 (S156's script), `MATE_IN_THREE_FLOOR` 11 (S154's
  `floor` and `red`), first-mate 9 and 9, mate 5 (S188's `first-mate`).
- `adocs/data/S192_anchors.py`'s ten: predicted unchanged.
- The mined rows: R01 (S285's row, checked by S095's and S230's scripts on
  S039's tree), E21 at 10, mate in 5 (S097's six, guard mode, in its range
  7..10; DEC-288 one band deeper if it finds no separator), B04 at 10, mate in
  5 (S277's six), `capture_mates` 9, 8, 11, 12 and labels (S230's sweeps),
  `DRIVE_DEPTH` 4 (S188's `s207`), the LQ pair (S269's commands).
- "ordering keeps the tree small", 65024 and 3251, by S282's middle half
  `[7270, 32512]` (the coordinator's mark text); 13446 on this tree today.
- Bench and accepts (3): `P2_PENDING` in `adocs/data/S039_sprt.sh`.

A carry ceiling that rises, a budget or a floor loosened, or a mate floor
dropped stops phase 2 for the owner; a mined row that stops separating is
re-mined by its script (DEC-288), and only "no separator one band deeper"
stops.

### The SPRT script

`adocs/data/S039_sprt.sh`: CAND and REF stay `PIN_ME`. REF is the last commit
before the candidate, documents and tests only on the revert `0cdd39a`
(S282's and S285's landings among them, neither touching `src/`), and the
guard that `git diff REF CAND^ -- src/` is empty stays. The S023 paragraph
now says the table is in neither side, and S099 relands after this run if
S283 reads H1 (DEC-290). Open findings at `769ce7a`: **S282** and **S285**,
DEC-171 filler, each named with its bearing on this step; the coordinator
re-reads the list on the day it pins. S284 landed: `--nonreg` is still
`{-5, 0}` at 0.05 with its 20000-round cap, and a stray `SPRT_BOUNDS` or
`SPRT_ROUNDS` in the shell makes fastchess.sh refuse the mode, a failed run
before any game. Bounds, DEC-143's 25591 and 41861 games, 2110 games an hour,
the abort rule and DEC-274's three outcomes unchanged.

### Phase 2, pending (supersedes every list above)

After S282 and S285 land; times are S023's recorded ones unless marked
*est.*; 2110 games an hour (`.moltke.local.md`).

1. Fast-forward onto S282 and S285: the two marks they meet ("ordering keeps
   the tree small", the extra-ply row) re-read against their landed text.
   Re-grep the readers. No machine.
2. Configure and build `build`, `build-tune`, `build-debug`: 2 min, 15 min
   configured fresh.
3. The TESTS gate, both builds and the format check
   (`CLANG_FORMAT_MAJOR=22`): *est.* 5 min (S023's gates read 180 to 270 s
   with its shorter carry budgets; S261's are back). A red outside the PENDING
   goldens and the at-risk cases stops phase 2.
4. `bench`, `bench 12`, `tools/search_bench.py` at 9 and 12 on both sides;
   `S192_anchors.py`; `S192_node_budget.py`: *est.* 10 min.
5. Accepts (3): `nps_rotate.py`, 8 rounds, REF, CAND and a REF copy: *est.*
   10 min.
6. DEC-169: `build/tools/truncation_scan --data
   .tuning/selfplay_v2_dedup.tsv --min 2.8`, and the tuner's load for the
   identity: *est.* 10 min.
7. The S170 grid by S203's script and `--ceilings` with S039's grid added:
   5 min (S099's 279 s).
8. The floors (S154's `floor` and `red`, S156's) and S188's `first-mate`:
   *est.* 15 min.
9. Mutants, DEV_MANUAL.md's recipe with S280's line: M29 and M41; S112's F01
   to F06; S131's V06, V09, V10; S091's C02, C05, C06, C07, R01, R02; S269's
   LQ01 to LQ03; E21 and B04 -- 22 runs at S023's 112 s a mutant: **41 min**.
   A row whose guard survives, or that the gate turned red, re-mined by its
   script: 0 to 60 min.
10. The gate again with every golden in: *est.* 5 min.
11. Debug self-play, four rounds at 4+0.04, grepped for `Assertion`: *est.*
    10 min.
12. `tools/gate_extra.sh`, five stages: 16 min (929 s at S023).
13. `tools/plan_prose_check.py`, one mode a call: `--prose`, `--citations`,
    `--touches`, `--params`, `--gate`: seconds.

**About 2.2 h, 3.2 h with every mate row re-mined**: daytime under DEC-155.
Then the candidate is committed, S283's run plays (DEC-290), and S039's SPRT
follows it: a night, 12.1 h on a bound (25591 games), the 40000-game cap at
19.0 h.

### What phase 2's compiler and tests are expected to say

- **Compiler**, Release, tune and Debug at `-Werror`: nothing; no caller of a
  removed symbol in `src/`, `tests/` or `tools/` (grep, comments aside).
- `test_search_params`: 72 rows in both builds.
- `test_search`: the stand-pat, entry, S112, S131 and S164 cases green. At
  risk with the tree, each a stop or a re-mine by its own rule: the mined rows
  (E21 and R01 most, both recently stale on neighbouring trees), the ordering
  band, `DRIVE_DEPTH`, the LQ pair, S091's capture-rule drives.
- `test_evaluation`: "the expensive stage is applied whole", the 16-bit
  ceiling, the anchors and S023's band case green.
- `test_eval_trace`, `test_tuner_gradient`: green; the pins' fate is the
  scan's.
- `test_mate_carry`: the likeliest red, margin 0; `test_mate_breadth`,
  `test_engine`: at risk, margin 1.
- `test_uci_surface`: release 5 lines unmoved; `test_fastchess_script`
  untouched by this step. The rest unaffected.

### Proposed `done:` stamp: the clauses that change

"LazyEvalMargin left the tune build (73 to 72 parameters)"; "400 readers at
769ce7a resolved (398 at c88a961, 400 at 8ef820f, 402 at 4e6ca63;
adocs/data/S039_readers.txt)"; "two stale golden statements the S023 revert
restored corrected in passing (test_mate_carry's GOLDEN line, DEV_MANUAL's
golden_defaults count)"; every other clause as proposed above.

## Phase 2 on `be48e02` (2026-10-15, Opus subagent) -- STOPPED for two rulings

The machine held alone (DEC-290's slot). The worktree fast-forwarded from
`769ce7a` to `be48e02` (S282, S285) with the phase-1 diff stashed and put
back; built, gated, every PENDING golden re-read or re-derived by its script,
the three mined rows that stopped separating re-mined, the Debug self-play
and `tools/gate_extra.sh` run. **Two reds are not goldens S039 may re-derive,
and phase 2 stops on them** (the brief): `test_mate_carry`'s ceilings rise,
and `test_tuner_gradient`'s gauge-walk precondition fails. Neither is
touched. The mutation pass is not run: its baseline must be green. No match,
SPRT or SPSA was started; no commit, no ref moved but `s039e`'s
fast-forward. Fixtures are commit objects on no branch (`e65a02a`, the tree
as phase 1 left it; `b551f08`, with the goldens re-derived).

### Deviations, first

1. **The stash did not pop with `--index`** (conflicts in the index after the
   fast-forward): `git stash pop` without it put the tree back with two
   conflicts, resolved by keeping both sides (`adocs/data/README.md`, S285's
   section then S039's; `tests/test_search.cpp`, S285's row under S039's
   mark), and the stash entry, which a clean pop would have dropped, was
   dropped by hand once the tree matched `s039_pre_ff2.diff` plus main's two
   commits. `refs/stash` is as it was before the stash.
2. **Seven of this file's own citations went MISSING on the reverted tree**
   (`test_plan_citation_freshness` red in gate 1): history sections cited
   S023's symbols, gone with its revert, and one old line cited a comment-only
   symbol. De-cited in place (`path`'s `symbol`), no text lost.
3. **Two mate floors rose, by the margin their blocks state** (1 below the
   shipping count): `MATE_IN_THREE_FLOOR` 11 -> 12 (13 shipping, 7 weakened),
   `EXACT_FLOOR` 159 -> 161 (162 shipping, 152 weakened). Tightenings, taken
   by the scripts; S250 kept 11 at an unmoved shipping end, S023 raised the
   breadth floor the same way. A ruling may prefer to keep them.
4. **The E21 row's re-mine takes the script's in-range pick, which asserts a
   distance the oracle calls one too long** -- see "The E21 row" below; a
   ruling is asked.
5. **The LQ pair was read, not re-mined**: none of its stated movers moves,
   both cases are green, and its mutants go to the mutation pass.
6. Scripts that build throwaway worktrees (S154, S156) put them under `/tmp`
   and removed them, as written; every other scratch tree is under
   `s039e/.ref-builds/`.

### Readings (`adocs/data/S039_measurements.txt`)

- `bench` 5636835 -> **6100949**, the eight replies unchanged; the sanitizer
  build's `bench` the same 6100949. `search_bench` 9: 234765 -> 192267 nodes,
  best moves unchanged; 12: 837949 -> 873087, kiwipete's d5e6 -> e2a6.
- Accepts (3), S120's `nps_rotate.py`, 8 rounds over S103's 300 at depth 11:
  nodes 38381506 -> 36684997 (**-4.42 %**), nps **-2.79 %** (CI -3.20 ..
  -2.38), wall **-1.68 %** (CI -2.09 .. -1.26); A/A copy +0.01 % (CI -0.41 ..
  +0.43). S120 had read the shortcut alone off at -7.5 % nodes, -5.01 % nps.
- The trace identity: the tuner's load over the dedup corpus, 10795695 rows,
  **0 differ from `evaluate()`**.
- INV-6 does not apply (the tree moves); DEC-215 has no off value to prove
  (S039 removes a parameter and adds none); INV-4: `make_move` untouched, and
  `gate_extra`'s debug stage (every `assert` over six binaries) is green.
- Debug self-play (DEC-141): 8 games at 4+0.04, **0 `Assertion`**, 0
  disconnects, 8 normal terminations, 1076 `bestmove` lines.
- `tools/gate_extra.sh`: prose, citations, debug (330 s), perft (51 s) ok;
  sanitize exit 8, its only failures the two open reds, 0 sanitizer reports.
- `tools/plan_prose_check.py`: `--prose`, `--citations`, `--touches`,
  `--params`, `--gate` all clean; the TESTS gate 44 of 46 in each build,
  format clean, the two reds the open rulings.

### Goldens, each by its script (shipping / weakened / gate / value)

| golden | reading | value |
|---|---|---|
| six piece anchors, `QUIET_ROOK_EVAL(_CHEAP)`, 198, -505, -569 | `S192_anchors.py`, 10 of 10 | unchanged |
| truncation pins (DEC-169) | 106 past 2.8, 34 at 2.875, the four among them | stand (`S039_truncation_scan.txt`) |
| "ordering keeps the tree small" | 13702, inside [7270, 32512] | 65024 / 3251 stand |
| `first_mate_depth`, `mate_in` | 9 and 9, mate 5 | unchanged |
| `DRIVE_DEPTH` | separating depth 4 | unchanged |
| `MATE_IN_THREE_FLOOR` | 13 / 7 / gate red at 7 | **11 -> 12** |
| `EXACT_FLOOR` | 162 / 152 / gate red at 152 | **159 -> 161** |
| extra-ply row | S285's read `d7-d12` both builds; re-mined | **`4brbr/...` depth 9, mate 2**; red with the guard opened |
| E21 row | shipped `#6` at 10, E21 none; one separator in 7..10 | **same position, depth 10, mate 6** (ruling asked) |
| B04 row | S261's read mate 5 at 9..12 both builds; re-mined | **`1R6/5k2/...` depth 9, mate 5**; red with B04 (below) |
| `capture_mates` | seven sweeps | 9, 8, 11, **11**; labels `C07`, none, `R02`, `C07, R02` |
| LQ pair | movers unmoved, cases green | kept |
| carry ceilings, S170 budgets | grid: 2, 5, **6**, 0, 0, 1 vs 0, 5, 4, 0, 0, 1; budgets A 500000, D 500000 | **not applied: ruling** |
| `golden_defaults` | 72 rows, table and X-list agree | 72 |

Observed red under each weakened build (`adocs/data/S039_remine.log`): the
guard-opened build at the extra-ply row; E21's at the E21 row; B04's at the
E21 row first (it reads that row's distance as 5), its own row's red being
the sweep (no mate at 7..10 under B04 against shipped `#5` at 9).

### The two stops, and one ruling asked (DEC-265's form)

**1. `test_mate_carry`: case A's ceiling 0 -> 2 and C's 4 -> 6.** *Why:*
S203's sweep of S039's tree (`adocs/data/S039_sweep_s170.txt`) with
`--ceilings` over S202's, S261's and it answers 2, 5, 6, 0, 0, 1; A's 2 is
its own 1000000 cell (7 lines, 2 short), so the case is red as shipped; C's 6
is its 4000000 cell (22, 6 short). The budget rule on the same grid gives A
1000000 -> 500000 (6 lines, 0 short) and D 2000000 -> 500000. S039 holds no
allowance. *Options:* **(a)** raise A and C as the script answers, the
budgets as the rule gives, on S039's H1 only (DEC-250, DEC-272, DEC-284,
DEC-287's form) -- costs two ceilings' worth of tolerated short lines, which
are DEC-122's reporting residue and not mates missed: every mate floor rose
on this tree; **(b)** keep 0 and 4 and study the short lines first (S202's
class) -- delays the slot DEC-290 booked, effort unknown, and S039 changes
no line of the reporting layer. *Recommend (a).*

**2. `test_tuner_gradient` "the fit holds every gauge still and no score
sees the hold": the plain update's walk along `psqt_eg[king]` reads 0.094
under Adam and 0.039 under AdaGrad, under the case's 0.1 floor.** *Why:* the
floor is a non-vacuity precondition, written "far above the hold's 1e-9 and
below the walk", to be re-read when constants or the fixture move; S039 took
the clamp out of the trace's gradient, so the fixture's descent moves. The
hold itself is intact (held shifts about 2.5e-15). The other six walks read
0.46 to 3.9; REF's eg-king walks were 0.41 and 0.89. *Options:* **(a)** set
the floor between the hold and the walk as its own comment says, e.g. 0.01,
with the new readings written beside it -- the precondition still separates
a working hold from a vacuous one by seven orders of magnitude; **(b)** keep
0.1 and lengthen the descent or raise AdaGrad's step until the eg-king walk
clears it -- changes the procedure the hold is tested under, and S262
measured AdaGrad at 30 as chaotic on this fixture; **(c)** keep 0.1 and add
fixture rows that load the eg-king gauge -- a fixture change for a
precondition. *Recommend (a).*

**3. (asked, not a stop) The E21 row's distance.** *Why:* the in-range
re-mine (DEC-288) finds one separator, the same position at the same depth,
and the script takes it with the engine's distance at that depth, `#6`;
stockfish reads `#+5`, the shipped build's distance moves with depth (6, 5,
6, 5, 5 at 10 to 14), and B04's build reads it as 5 -- S095's miner rejected
a row "whose distance moves" on a ground the rule does not cover. One band
deeper (11..14), twelve separate and the pick is
`r6k/3b1pqp/p2p1p2/2pP1P2/4P2Q/P1P3RP/2P4K/R7 w - - 3 36` at depth 11, mate 5
at every depth 11 to 14, the guard-dropped build losing 11; stockfish reads
`#+5` at depth 30 (and `#+6` at depth 20, the candidates file's label).
*Options:* **(a)** keep the in-range pick, in the tree now -- the letter of
DEC-288; **(b)** take the deeper band's row by S095's precedent -- a stable
distance, at the cost of reading "no separator" for a range that has one.
*Recommend (b)*; either way the row is observed red under E21 before the
candidate is committed.

### What remains after the rulings (about 1.2 h)

The rulings written into the tree (minutes); the TESTS gate, 5 min; the
mutation pass -- M29, M41, F01-F06, V06/V09/V10, S091's six, LQ01-LQ03, E21,
B04, J01-J03, 25 runs at about 112 s: **47 min**; `tools/gate_extra.sh` again,
16 min; the prose checks; this file's stamp text and the commit.

### Proposed `done:` stamp (HH:MM and the SPRT line the coordinator's)

    2026-10-15 HH:MM -- <SPRT line: H1 | H0 | no verdict at {-5, 0} nElo,
    8+0.08, Hash 16, CAND <sha> vs REF be48e02: Elo, nElo, LLR, games,
    forfeits (adocs/data/S039_sprt.log)>. The lazy shortcut and the clamp on
    mobility plus king safety retired together, weights frozen: evaluate() is
    the exact sum of both stages at every call, quiescence stands pat on it
    and stores it at every window, LazyEvalMargin left the tune build (73 to
    72 parameters), the trace and tuner carry no clamp. bench 5636835 ->
    6100949; nodes / nps / wall to depth 11 over S103's 300: -4.42 % / -2.79 %
    / -1.68 % (S120's nps_rotate.py, A/A +0.01 %). Readers resolved: 398 at
    c88a961, 400 at 8ef820f, 402 at 4e6ca63, 400 at 769ce7a
    (adocs/data/S039_readers.txt); 3 test_evaluation cases and 1
    test_tuner_gradient case retired, 15 re-pointed, the 383 golden retired;
    M31 retired, M29 re-anchored, M41 added; <mutation pass>. Goldens by their
    scripts: truncation pins stand (106 past 2.8, 34 at 2.875); anchors 10 of
    10; MATE_IN_THREE_FLOOR 11 -> 12, EXACT_FLOOR 159 -> 161; the extra-ply,
    E21 and B04 rows re-mined (DEC-288) and capture_mates re-derived, each
    observed red under its weakened build; ordering band, first-mate and
    DRIVE_DEPTH unchanged; <carry ceilings and budgets as ruled>; <gauge-walk
    precondition as ruled>. Second tier: Debug self-play 8 games, 0
    Assertion; gate_extra <all five stages>, the sanitizer bench equal to
    Release. Two stale statements S023's revert restored corrected in passing
    (DEV_MANUAL's golden_defaults count, test_mate_carry's GOLDEN line).
    Implemented by Opus subagents in phases; run, read and stamped by the
    coordinator.

### The candidate's commit text (the verdict's commit carries DEC-220's block)

    Retire the lazy shortcut and the clamp together (S039)

    evaluate_lazy() skipped mobility and king safety wherever the cheap
    stage cleared the window by LAZY_EVAL_MARGIN, and evaluate_expensive()
    clamped the two to the same margin so that bound held by construction.
    The clamp also capped the real score: no king safety term could say
    more than 184 centipawns, which S122's rebuild cannot live with, and
    S121 fits mobility inside the same sum. DEC-258 retires both under one
    non-regression verdict, and DEC-290 lands it next.

    quiescence() now stands pat on evaluate(), exact at every window, and
    stores it in every entry; evaluate_expensive() adds the stage whole;
    LazyEvalMargin leaves the tune build. No weight moves. The trace's
    exact score and the tuner's smooth score and gradient drop the clamp
    with the engine, so the per-row identity still holds (0 of 10795695
    rows differ). The tests that held the shortcut's bound now hold the
    exact score at the windows it used to answer.

    The tree moves, so its goldens were re-derived by their scripts: both
    mate floors rise (12, 161), the extra-ply, multicut and ProbCut mate
    rows are re-mined (DEC-288) and the capture-mate rows re-derived, each
    observed red under its weakened build; <the rulings>. nps -2.79 %,
    nodes -4.42 % to depth 11 over S103's 300 positions.

    Bench: 6100949

## Phase 2 completed under DEC-291 to DEC-293 (2026-10-15, Opus subagent)

The rulings, recorded in main as `2833636` (documents only; REF stays
`be48e02`, whose `src/` it shares), written into the tree; the gate, the
full mutation pass, `tools/gate_extra.sh` and the prose checks run. The
worktree stays on `be48e02`: `2833636` touches `adocs/decisions.md` alone,
which this diff does not, so the diff applies to either. No `git stash`
(the coordinator's instruction). No match, SPRT or SPSA; no commit; no ref
moved.

### Deviations, first

1. **J01 and J02 no longer stop first at the extra-ply row.** S285's row was
   their first failing assertion; on S039's tree the re-mined row is
   separated by S095's re-derivation edit (`= true`, the GOLDEN block's
   weakened build), which neither mutant is, and both now fail first at the
   E21 row (DEC-293's) inside the same case. Each is still killed by the
   case its `Mutation:` line names ("the no-table-move term is one ply of its
   own on top of the other four"), so nothing stops; the extra-ply row
   witnesses the guard-opened build, as its block says.
2. **B04's row was observed red in the case with the E21 row's block
   commented out in a scratch copy, never committed**: B04's build loses the
   new E21 row's mate too, and that row comes first. The scratch edit is in
   `adocs/data/S039_remine.log`.
3. **Documents beyond the brief's list, each a current-state sentence the
   rulings or the floors made false**: `adocs/specs.md`'s search row
   (`MATE_IN_THREE_FLOOR` 12 and the breadth floor 161, the latter stated as
   143 there since S156), `DEV_MANUAL.md`'s mate-safety paragraphs (the floor
   of 12, the red at `7 >= 12`, `--floor 161`).
4. `adocs/data/S039_mutation_pass.tsv` is the pass's own `results.tsv`,
   tracked as the reading (DATA).

### The rulings, as written

- **DEC-291**: `tests/test_mate_carry.cpp` `short_line_ceiling` A 0 -> 2 and
  C 4 -> 6, each line citing DEC-291; the GOLDEN line, the ceiling table, a
  paragraph and both `--ceilings` commands carry S039's grid
  (`adocs/data/S039_sweep_s170.txt`); `adocs/data/S170_cases.tsv` A
  1000000 -> 500000 and D 2000000 -> 500000 with its paragraph;
  `DEV_MANUAL.md`'s row and command.
- **DEC-292**: `tests/test_tuner_gradient.cpp` the gauge-walk floor 0.1 ->
  0.01, the readings (0.094 Adam, 0.039 AdaGrad, 0.41 and 0.89 before, hold
  about 2.5e-15) and DEC-292 beside it.
- **DEC-293**: the E21 row is `r6k/3b1pqp/p2p1p2/2pP1P2/4P2Q/P1P3RP/2P4K/R7 w
  - - 3 36` at depth 11, mate in 5; its GOLDEN block cites DEC-293 and both
  bands' readings (7..10: one separator, S261's position at the engine's
  `#6`; 11..14: twelve, this pick). Observed green shipped (the gate) and
  red with E21 applied, `REQUIRE( result.mate_found )` at "mate the
  multicut hides, depth 11".
- The SPRT script's H0 row lists DEC-291's and DEC-292's values as leaving
  with the revert, and DEC-293's row with S039's other re-mined rows.

### Readings

- **The TESTS gate**: **46 of 46 in both builds**, the format check clean.
- **Mutation pass** (DEV_MANUAL.md's recipe with S280's line, fixture
  `f4c3076`, a commit object on no branch, the fixture's own
  `tools/mutants/`): **25 of 25 killed**, 2786 s, each by the case its entry
  names -- M29 (seven binaries, the anchors and the expensive-stage case
  among them), M41 ("the expensive stage is applied whole", "the trace
  reconstructs evaluate() exactly", `test_mate_carry`), F01-F06 by their
  six, V06 by "futility does not skip a queen promotion that takes nothing",
  V09 and V10 by S112's capturing-promotion case and the exchange-gate case,
  C02, C05, C06, C07, R01, R02 by S091's six, LQ01-LQ03 by their own, E21 by
  "the multicut never ends a node on a mate from its verification" and the
  mate row, B04 by "probcut does not run with beta in the mate band" and the
  mate row, J01 and J02 by the no-table-move term's case, J03 by "a node
  whose table entry carries no move reduces its late quiets by LmrNoTtMove
  more". Bench moved on 20, held on F05, F06, B04, LQ01, LQ02, each killed by
  the suite. `adocs/data/S039_mutation_pass.tsv`.
- **`tools/gate_extra.sh`** (`OUT` under `.tuning/coord/S039/p2/gate_extra_final/`):
  **all five stages ok in 871 s** -- prose, citations, debug (326 s), sanitize
  (494 s, 46 of 46, 0 sanitizer reports, "INV-6 across builds: both 6100949
  nodes"), perft (50 s).
- **Prose checks**, one mode a call: `--prose`, `--citations`, `--touches`,
  `--params`, `--gate` clean.

### On S039's H0 or no verdict: what the revert keeps

The revert takes everything this step's commit carries, DEC-291's and
DEC-292's values and DEC-293's row included, and leaves the evidence under
`adocs/data/`. **Three edits it keeps, written into the revert commit**,
because the tree before this step carried three stale statements this step
corrected in passing: `DEV_MANUAL.md`'s `golden_defaults` row at **73**,
the reverted tree's count, not 71; `tests/test_mate_carry.cpp`'s GOLDEN line
naming the six `short_line_ceiling` ships there, **0, 5, 4, 0, 0 and 1**,
not S116's 6, 15, 5, 2, 11 and 5; and `adocs/specs.md`'s search row stating
both mate floors at the reverted tree's readings, not `be48e02`'s "a floor of
143 exact" and mate-in-three weakened end of 10 -- `EXACT_FLOOR` **159**,
ends 160 shipping and 156 weakened, and `MATE_IN_THREE_FLOOR` **11**, ends
12 and **6**, each re-read on `be48e02` by its script in this phase
(`adocs/data/S039_floors.log`, the reverted tree's section; S154's `red`
overlays the working tree's floor of 12, so it reads `6 >= 12`, and 6 is
under 11 too). `adocs/data/S039_sprt.sh`'s H0 row says the same.

### Fast check (2026-10-15): three findings, fixed

1. **The H0 row kept two stale-statement corrections and missed a third**:
   `adocs/specs.md`'s search row, "a floor of 143 exact" and a mate-in-three
   weakened end of 10 at `be48e02`. Added to the H0 row and the section
   above, every number re-read on `be48e02` by its script, not by hand.
2. **M41 had no `Mutation:` line at its killer.** Added above
   `tests/test_evaluation.cpp` "the expensive stage is applied whole", in
   the form the recent mutants use, with the values the pass observed
   (`REQUIRE( -585 == -184 )`). M41 re-run alone through the recipe on the
   fixed tree (fixture `8f95b53`): killed, 3 of 46, by that case first,
   256 s.
3. **`src/data_structures.hpp` still quoted the clamped maximum.** The case
   run on S039's tree prints a maximum |evaluate()| of **23891** over the
   same 66430 placements (15353 over the 2696 corpus positions): **1.37x**
   headroom under 32767, where S213's clamped 22945 was 1.43x -- lower than
   the 1.4x the comment stated, reported and not fixed; the case asserts
   the 16 bits on every board. The comment (comment only; `bench` still
   6100949) and the case's own comment state the reading, recorded in
   `adocs/data/S039_measurements.txt`.

After the fixes: the TESTS gate 46 of 46 in both builds, format clean;
`bench` 6100949; the five prose modes clean.

### Proposed `done:` stamp (supersedes the one above; HH:MM and the SPRT line the coordinator's)

    2026-10-15 HH:MM -- <SPRT line: H1 | H0 | no verdict at {-5, 0} nElo,
    8+0.08, Hash 16, CAND <sha> vs REF be48e02: Elo, nElo, LLR, games,
    forfeits (adocs/data/S039_sprt.log)>. The lazy shortcut and the clamp on
    mobility plus king safety retired together, weights frozen: evaluate() is
    the exact sum of both stages at every call, quiescence stands pat on it
    and stores it at every window, LazyEvalMargin left the tune build (73 to
    72 parameters), the trace and tuner carry no clamp (0 of 10795695 rows
    differ). bench 5636835 -> 6100949; nodes / nps / wall to depth 11 over
    S103's 300: -4.42 % / -2.79 % / -1.68 % (S120's nps_rotate.py, A/A
    +0.01 %). Readers resolved: 398 at c88a961, 400 at 8ef820f, 402 at
    4e6ca63, 400 at 769ce7a (adocs/data/S039_readers.txt); 3 test_evaluation
    cases and 1 test_tuner_gradient case retired, 15 re-pointed, the 383
    golden retired; M31 retired, M29 re-anchored, M41 added; mutation pass 25
    of 25 killed, each by its named case. Goldens by their scripts:
    truncation pins stand (106 past 2.8, 34 at 2.875); anchors 10 of 10;
    MATE_IN_THREE_FLOOR 11 -> 12, EXACT_FLOOR 159 -> 161; the extra-ply and
    B04 rows re-mined (DEC-288), the E21 row one band deeper (DEC-293), the
    capture_mates rows re-derived, each observed red under its weakened
    build; carry ceilings A 0 -> 2 and C 4 -> 6 with budgets A 500000 and D
    500000 (DEC-291); the gauge-walk floor 0.1 -> 0.01 (DEC-292); ordering
    band, first-mate and DRIVE_DEPTH unchanged. Second tier: Debug self-play
    8 games, 0 Assertion; gate_extra all five stages (871 s), the sanitizer
    bench equal to Release. Three stale statements corrected in passing
    and kept by any revert (DEV_MANUAL's golden_defaults count,
    test_mate_carry's GOLDEN line, specs.md's two mate floors). Fast check:
    3 findings, fixed -- the third H0 correction, M41's Mutation: line, the
    unclamped 16-bit maximum (23891, 1.37x headroom). Implemented by
    Opus subagents in phases; run, read and stamped by the coordinator.

### The candidate's commit text (supersedes the one above)

    Retire the lazy shortcut and the clamp together (S039)

    evaluate_lazy() skipped mobility and king safety wherever the cheap
    stage cleared the window by LAZY_EVAL_MARGIN, and evaluate_expensive()
    clamped the two to the same margin so that bound held by construction.
    The clamp also capped the real score: no king safety term could say
    more than 184 centipawns, which S122's rebuild cannot live with, and
    S121 fits mobility inside the same sum. DEC-258 retires both under one
    non-regression verdict, and DEC-290 lands it next.

    quiescence() now stands pat on evaluate(), exact at every window, and
    stores it in every entry; evaluate_expensive() adds the stage whole;
    LazyEvalMargin leaves the tune build. No weight moves. The trace's
    exact score and the tuner's smooth score and gradient drop the clamp
    with the engine, so the per-row identity still holds (0 of 10795695
    rows differ). The tests that held the shortcut's bound now hold the
    exact score at the windows it used to answer.

    The tree moves, so its goldens were re-derived by their scripts: both
    mate floors rise (12, 161), the extra-ply and ProbCut mate rows are
    re-mined (DEC-288), the multicut row one band deeper (DEC-293), and the
    capture-mate rows re-derived, each observed red under its weakened
    build. Two test numbers moved by the owner's rulings: the carry
    ceilings A 0 -> 2 and C 4 -> 6 with their budgets (DEC-291), and the
    tuner's gauge-walk floor 0.1 -> 0.01 (DEC-292), both leaving with any
    revert. nps -2.79 %,
    nodes -4.42 % to depth 11 over S103's 300 positions.

    Fast check: 3 findings, fixed. The H0 row keeps a third stale
    statement corrected (specs.md's two mate floors, re-read on the
    reverted tree); M41 gains its Mutation: line at its killer; the
    16-bit headroom comment states the unclamped maximum, 23891.

    Bench: 6100949
