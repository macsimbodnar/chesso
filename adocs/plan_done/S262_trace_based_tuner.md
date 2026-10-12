id:         S262
goal:       the tuner fits the evaluation from coefficient traces that evaluate() records in a trace build, so a new term costs its trace call and a parameter declaration, and nonlinear groups are fitted through their own analytic gradients
accepts:    (1) a trace build -- a compile-time switch compiled out of the release and tune builds -- records per position the sparse coefficient vector, **each side's** pre-transform sum for every nonlinear group (a per-side transform needs both sides' coefficients, not their difference), and the inputs of each endgame adjustment; (2) **exact reconstruction**: the score recomputed from a trace at the weights under test equals `evaluate()` after the engine's own integer tapers, rounding and clamp, on every row of a sample of at least one million corpus rows, and a fast-label test holds the identity on a small fixture -- the identity is the contract that replaces `tools/eval_model.hpp`'s hand-written mirror; (3) the analytic gradient is checked against finite differences of the **smooth** objective the optimiser minimises, over every parameter, with the treatment of truncation and clamp stated (S100 measured 4.9e-8 worst error for the current tuner); (4) S100's planted-vector recovery passes on the new tuner, and a recovery case covers each nonlinear group kind; (5) a nonlinear group whose transform has zero gradient at its seed -- `max(0, x)^2` at x = 0 is the case -- is seeded from a nondegenerate derivation over chesso's own data (DEC-134 form b), and recovery from that seed is shown before any long fit; (6) on the current corpus, same split by game (S066) and seed, the new tuner's held-out loss on the current model is no worse than the current tuner's by more than a tolerance stated before the run, recorded under `adocs/data/`; (7) output is `constexpr int` (DEC-255) with the provenance stamp (S077), and freeze groups by name survive (DEC-057); (8) trace extraction time and storage per million rows are measured and stated; (9) the src/ instrumentation is behaviour-neutral by INV-6 in full -- `bench` and `tools/search_bench.py` node counts and best moves identical at two depths -- and its release-build cost is a timing against the parent; (10) `DEV_MANUAL.md`'s tuner section describes the new path and retires the old one only after (6) holds
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tools/tuner_groups.hpp, tools/tuner_target.hpp, tools/eval_model.hpp, src/evaluation.cpp, src/evaluation.hpp, tests/test_tuner_gradient.cpp, tests/test_eval_model.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   any new evaluation term; any fit that ships weights (S083, S126 and the term steps own those); the corpus format and generator, which are S082's
decisions:  DEC-258, DEC-259, DEC-255, DEC-057, DEC-221
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-12 -- the tuner fits evaluate()'s own trace. A trace build (`CHESSO_TRACE`, on the new `chesso_engine_trace` library only; `chesso` never links it) records the coefficient of every weight evaluate() applies; the trace scored with the engine's integer tapers and clamp equals evaluate() on all 10795695 corpus rows (0 mismatches, clamp binding on 36503), checked by the tuner on every row it loads. INV-6: evaluation.cpp's object byte-identical in build and build-tune, `chesso`'s `.text` identical but for `.rodata` displacements, bench 3872032, bench 12 and search_bench 9/12 node- and move-identical; no release cost to time. The fit descends the untruncated score with the clamp differentiated exactly -- a reversal of the parent's gradient, which ignored it, taken because accepts (3) checks the true gradient, on 0.338 % of corpus rows, (6) measured under it, and S039 retires the clamp; gradient vs finite differences 4.87e-8 over all 823 (S100: 4.9e-8), 1.72e-7 where the clamp binds. Nonlinear groups: per-side records, one transform (`max(0,x)^2/d`), a synthetic group's gradient 2.80e-8, stuck at zero and recovered from a seed derived over the data (DEC-134 form b). Pre-registered: held-out 0.118449 old against 0.118446 new (tolerance 1e-5) -- the old path retired from the tuner; Adam stays default, AdaGrad at its swept best step 30 beating it by exactly the 1e-5 margin, not more. (4) end to end: **the pre-registered criterion, every planted term weight exact, missed** -- 17 of 18 at the defaults, `passed_pawn_eg[4]` 71 against 70, from fitting K (0.7557 against the labels' 0.7624); S100's own rule, within +/-1, holds, and the parent's tuner emits the identical 18 numbers; post hoc at the plant's K, 18 of 18. Extraction 0.8 s per million rows at 12 threads, 3.0 s at one; 119 MB per million rows. Tests: test_eval_trace (7), test_tuner_gradient re-targeted (9) plus 4 new, 5 mirror cases moved to test_eval_model; eleven mutants red. Fast suites 45/45 both, format clean, prose checks clean, bench 3872032. DEV_MANUAL and specs updated, MANUAL checked. Implemented from Grant (2020), CPW's Texel's Tuning Method, Kingma and Ba, Duchi et al. (DEC-221). The mirror stays for feature_audit and truncation_scan (follow-on). Fast check: no code defect; (4)'s miss stated plainly beside the pre-registration, the clamp reversal recorded at gradient() and in DEV_MANUAL, a truncation-identity sign, the fixture count (2764), stale src/ comments naming the mirror, and trace_board() leaving its buffer dirty on refusal (now cleared, overflow refusal tested, mutant M11), all fixed before commit. Follow-on filed as S276.

## Why

Every evaluation family in the new order needs fitting, three of them
through a nonlinear transform -- king safety (S122), the endgame scale
factor and mop-up (S124), complexity (S266). Today `tools/eval_model.hpp`
re-implements the evaluation term by term for the tuner, and
`tools/tuner_model.hpp`'s own header records three occasions when a term's
gradient block was forgotten and the fit silently returned its input. A trace
makes the evaluation its own feature extractor. It does **not** make a
missing gradient impossible: a nonlinear group still needs its model, its
derivative and a test, which is what (3) to (5) are for. DEC-044 kept king
safety linear only because the tuner could not fit through a transform.

## Description it is implemented from

Grant, *Evaluation & Tuning in Chess Engines* (2020), the paper that
describes the method: evaluation wrapping (every weight application also
records its coefficient), the sparse per-position layout, the derivatives for
linear terms, for a per-side king-safety transform and for complexity, and
the base-e sigmoid. CPW *Texel's Tuning Method*. The optimiser is Adam
(Kingma and Ba, 2014) or AdaGrad (Duchi, Hazan and Singer, 2011), chosen by a
comparison pre-registered in this step. Implemented from these descriptions
(DEC-221); no engine's tuner source is opened. Optimiser hyperparameters: the
papers' own published defaults (DEC-134 form a) or a stated sweep over
chesso's corpus (form b).

## Lane

Agent work under DEC-260: written in a separate worktree while matches hold
the workstation, compiled, tested and timed only between runs. It lands
before S134's fold is fitted on and before S082's corpus is selected, so
every later fit uses it.

## Measurement

Neutral for the engine; no match. (6) is an offline reading, not a verdict.

## From the record

S028, S065, S075 (the WDL/score blend, `--lambda`), S076 (dedupe), S077
(provenance), S100 (gradient and recovery checks), DEC-053 (truncation guard
slack), DEC-169 (truncation reading when the margin moves; S039 retires the
clamp). Ported from the 2026-10-07 proposal's P02 with the corrected
comparison's conditions (exact integer reconstruction, smooth-objective
gradients, per-side sums, nondegenerate seed) folded in.

## Pre-registration (written 2026-10-12, before any comparison run)

Machine: the workstation, i7-8700K, 12 threads, g++ 13.3, idle (load average
checked before each run). Corpus for (6) and the optimiser comparison:
`.tuning/selfplay_v2_dedup.tsv`, sha256 `0a6b59b6...`, 10795695 rows, read from
the main tree's `.tuning/`. "Old" is the parent's `tuner`, built at `f57b92f`
before any S262 edit and saved; "new" is this tree's.

**(6) old against new, held-out loss.** Both at `--threads 12` and every other
flag at its default (`--epochs 20000 --report 100 --patience 20 --lr 1
--validation 0.1 --seed 1 --lambda 0`, `--only all`, no `--freeze`), so both
cut the same S066 split by game at seed 1 over the same rows in the same
order, fit K the same way from the same start, and descend the same smooth
objective except where the clamp binds (the old gradient ignored it; the new
one differentiates it).
- Metric: the held-out error on each run's `best:` line (mean squared error
  against the game result over the held-out games).
- Tolerance: new <= old + 1.0e-5. Why: S076's whole refit of this corpus moved
  that number by 1.5e-4, so 1e-5 is under a tenth of a refit's gain; S275
  measured floating-point order alone parting two fits in the seventh decimal,
  so 1e-5 is far above the noise the two paths' summation orders add.
- Holds: the tuner's old path retires (the tuner, its tests and DEV_MANUAL
  move to the trace). Fails: both paths stay, the figures are recorded, and the
  step stops there.
- Reported beside it, not deciding: K, the start errors, every common report
  line's held-out difference, both best epochs, both wall times, and how far
  the two emitted integer vectors differ.

**Adam against AdaGrad.** On the same corpus and flags as (6)'s new run, which
is the Adam arm. AdaGrad's step (DEC-134 form b, a sweep over chesso's own
data): `--optimiser adagrad --lr X` for X in {1, 3, 10, 30} centipawns -- a
coordinate's first AdaGrad step is exactly X, so the range runs from Adam's
step size to the largest move S275's walk saw a weight make. Every arm at the
default epoch budget and patience, no cap.
- Metric: the `best:` held-out error, as for (6).
- Decision: the lowest of the four AdaGrad runs is AdaGrad's step, and the
  tuner's AdaGrad default. AdaGrad becomes the tuner's default optimiser only
  if that run's held-out error is below Adam's by more than 1.0e-5 (the same
  margin, for the same reasons); otherwise Adam, the incumbent, stays the
  default and AdaGrad stays selectable.
- Size: full corpus, because the shipped constants were fitted on this corpus
  at this split: on any slice, its held-out games were the fit's training games
  and no optimiser can beat the start there (S275's 1 M-row fits never did).

**(4) end to end.** S275's planted slice: the parent's `feature_audit --plant`
over the first 1000000 rows (labels sigmoid(0.7624 * score) at the shipped
constants with S100's term weights planted, S134's two folded columns
dropped), then the new `tuner` at the defaults above. Pass, S100's criterion
less the two columns S134 removed: the training error prints 0.000000 and all
18 planted term parameters -- passed pawns 10, piece placement 6, tempo 2 --
are emitted at their planted integers.

### Note on (4), added 2026-10-12 after the run; the text above is unchanged

- **The pre-registered criterion missed by one on one parameter.** At the
  defaults the planted slice gave 17 of the 18 term parameters at their planted
  integers; `passed_pawn_eg[4]` came out 71 against 70.
- **The pre-registration quoted S100's criterion as exact, and it is not.**
  S100's own pass rule (`adocs/plan_done/S100_zero_weight_terms_reexamined.md`,
  D3) is "identified parameters recover planted values within integer rounding
  (+/-1)". Under that rule (4) holds: 18 of 18 within one.
- **The parent's tuner emits the same 71** on the same slice -- the identical
  18 numbers -- so the miss is the method's, not the trace's: the tuner fits K
  from the shipped start (0.7557) and the labels were made at 0.7624, a scale of
  1.0089 that takes 70 to 70.6.
- **The `--k 0.7624` run is post hoc**, not pre-registered: 18 of 18 at the
  planted integers.

## As built (2026-10-12)

### Deviations

- **The switch is a library, not a build directory.** `CHESSO_TRACE` is a
  compile definition on one CMake target, `chesso_engine_trace`, built from the
  same sources as `chesso_engine` in every build directory. `chesso` never links
  it in any configuration, so it is compiled out of the release and tune
  binaries -- and `build_release.sh` and `fastchess.sh` build `--target chesso`
  alone -- while the tuner and the trace tests link it and run in both gated
  builds. A third build directory was the alternative; it would have kept the
  trace tests out of the gate.
- **The old path retired from the tuner, not from the tree.** (6) held, so the
  tuner, its tests and DEV_MANUAL moved to the trace, and the mirror's
  `error_range()`, `gradient()`, Adam and `descend()` left
  `tools/tuner_model.hpp` (Adam and `descend()` unchanged, into
  `tools/tuner_optimiser.hpp`). `tools/eval_model.hpp` and the mirror's
  dataset, `load()` and `evaluate_position()` stay, because `feature_audit`
  (identities, columns, `--plant`) and `truncation_scan` read them; so does
  `test_eval_model`, which gained the mirror loader's five cases from
  `test_tuner_gradient`. Re-pointing those two tools at the trace, and then
  deleting the mirror, is the proposed follow-on below.
- **No nonlinear group and no endgame adjustment exists in `evaluate()`.** The
  format carries per-side coefficients (`EVAL_TRACE_SIDE`, the dataset's
  per-side entries, `group_t` in the model) and one transform kind,
  `max(0, x)^2 / divisor`; the tested group is synthetic, its per-side inputs
  the engine's own per-colour king-attack counts (`king_safety_features()`,
  attackers by type and zone incidences) rather than a trace call, since no
  term records per side. There is no field for an endgame adjustment's inputs:
  there is no adjustment to record, and S124 adds its inputs with its model.
- **(4) end to end: 17 of the 18 planted term parameters at the defaults, the
  pre-registered criterion was 18.** The miss is `passed_pawn_eg[4]`, 71 against
  70, and it is K: the tuner fits K from the shipped start (0.7557) while the
  labels were made at 0.7624, so the whole fitted vector is the planted one
  scaled by 0.7624 / 0.7557 = 1.0089 -- 70.6 rounds to 71, and `QUEEN` comes out
  726 against 716. The parent tuner on the same slice emits the identical 18
  numbers, its own 71 included. S100's rule is within +/-1, under which (4)
  holds; the pre-registration quoted it as exact (dated note above).
  A run at `--k 0.7624`, not pre-registered and labelled so: all 18 at their
  planted integers, and the five piece values back at the shipped ones.
- **The S275 gauges are null for the score the fit descends, not exactly for
  the engine's.** Along a material gauge the shift moves from inside the
  positional taper's truncation to outside it, and truncation towards zero is
  not translation invariant where that sum crosses zero, so `evaluate()` itself
  moves by at most one centipawn (measured on the gauge fixture: 1 along the
  knight's and the rook's gauge, 0 along the other five). The re-targeted test asserts that bound for the exact score and
  < 1e-9 for the smooth one. DEC-059's hand re-anchor saw the same centipawn.
- **A nonlinear group's step is not the linear terms'.** From its derived seed
  the synthetic group came back exactly at `--lr` 0.2 and 0.1 and stalled at
  1.0 and 0.3 (errors 9.2e-8 and 8.8e-8 on the test's 2764 positions; on the
  2730 without the mirrored half, 7.2e-8 with two endgame weights negative,
  their sides on the transform's flat part). The tuner has one `--lr`; a
  per-group step is S122's to add.
- **The clamp's gradient is reversed from the parent's.** The parent's gradient
  ignored the clamp on purpose ("treating the boundary as flat would stop the
  fit moving a weight it should move") while its forward model clamped; the
  trace path differentiates it exactly, the true gradient of the objective the
  fit minimises, which accepts (3) checks against finite differences. It binds
  on 0.338 % of the dedup corpus's rows at the shipped weights, the held-out
  comparison (6) was taken under it, and it is short-lived: S039 retires the
  clamp and S122's king safety is not clamped. Recorded at the site
  (`gradient()` in `tools/tuner_trace.hpp`) and in DEV_MANUAL.
- **Adam stays the default; AdaGrad is selectable at a default step of 30.**
  AdaGrad at 30 read 0.118436 against Adam's 0.118446, exactly the
  pre-registered 1.0e-5 margin at the printed precision, and the rule needed
  more than the margin. 30 was the top of the swept range.
- **The gauge test runs AdaGrad at 10, not its default 30**: on its 38 rows a
  step of 30 makes the descent chaotic -- the held and plain runs part by 7e-13
  at epoch 25, 1e-7 at 75, 1.6 cp at 175, a hundredfold every 25 epochs, with
  every level held within 1e-9 throughout -- so the case would measure the
  chaos and not the hold. At 3 and 10 the runs stay within 6e-13.
- **`--lr` refuses a value that is not positive**; the old tuner took it.
- **Peak resident memory is higher**: 1.96 GB against 1.38 GB on the full
  corpus (vector growth during the load). Per epoch the trace path is faster.
- **`adocs/data/S275_walk.cpp` builds against `f57b92f` and earlier only**: it
  calls the mirror's `gradient()` and Adam, which this step retires.
- **Files outside `touches:`**: `src/eval_trace.hpp` (new), `src/CMakeLists.txt`,
  `tools/tuner_trace.hpp`, `tools/tuner_layout.hpp`, `tools/tuner_optimiser.hpp`
  (new), `tools/CMakeLists.txt`, `tools/feature_audit.cpp` (one comment),
  `tests/test_eval_trace.cpp` (new), `tests/CMakeLists.txt`, `adocs/specs.md`,
  `adocs/data/README.md` and the S262 data files.

### The trace format and the switch

- **`src/eval_trace.hpp`** (new). Compiled only where `CHESSO_TRACE` is defined:
  `term_t` -- the eight weight arrays, `TERM_MATERIAL` (untapered),
  `TERM_PSQT` (`type * 64 + square`, Black mirrored by `^ 56`),
  `TERM_MOBILITY`, `TERM_KING_SAFETY`, `TERM_PASSED_PAWN`,
  `TERM_PAWN_STRUCTURE`, `TERM_PIECE_PLACEMENT`, `TERM_TEMPO`; `side_t`
  (`SIDE_BOTH` for White's coefficient minus Black's, the form every linear
  term records; `SIDE_WHITE` / `SIDE_BLACK` for a per-side transform); a
  `record_t` of up to 1024 `(term, side, index, coefficient)` entries plus the
  phase, an overflow flag instead of a write past the end; a thread-local
  `sink`. Three macros: `EVAL_TRACE`, `EVAL_TRACE_SIDE`, `EVAL_TRACE_PHASE`.
  Without the define all three are `((void)0)` and their arguments are never
  evaluated.
- **`src/evaluation.cpp`**: one trace call beside each weight application --
  pawn structure, piece placement, each passer (+1 White, -1 Black), tempo
  (+1 or -1 by side to move, White relative), mobility per piece, king safety's
  attacker count, zone incidences and the four shelter features -- and the
  phase in `evaluate_cheap()`. Material and the tables are accumulated by
  `make_move`, so under `CHESSO_TRACE` alone `trace_accumulators()` walks the
  bitboards in `evaluate_cheap()` and writes one material and one table entry a
  piece (the king's value is 0 and not a parameter). Released code: no
  change, below.
- **`src/CMakeLists.txt`**: `chesso_engine_trace`, the same sources with
  `CHESSO_TRACE` PUBLIC (and `CHESSO_TUNE` when the engine has it). Linked by
  `tuner`, `test_eval_trace`, `test_tuner_gradient`.
- **`tools/tuner_trace.hpp`** (new): the term table -- each term's width, its
  parameters in the 823 layout and its **site**, the one thing the trace does
  not record: `MATERIAL` untapered, `POSITIONAL` (tables plus the three pawn
  terms, one taper), `TEMPO`, `MOBILITY`, `KING_SAFETY` (each its own taper, the
  last two summed and clamped). 414 features, each a parameter pair or the
  untapered piece value, covering the 823 exactly once (tested). A row: sparse
  `(uint16 feature, int16 coefficient)`, zeros dropped, plus per-side entries
  `(group, side, input, coefficient)`, the engine's phase, the side to move,
  result, score, ply. `sum_record()` (one record summed per feature, refusing
  one it cannot hold and leaving its thread-local buffer clear on every return),
  `trace_board()`, `load()` (blocks of 65536 lines traced on
  `--threads` threads, appended in file order), `exact_score()` (the engine's
  integer arithmetic), `smooth()` / `score()`, `error_range()`, `gradient()`,
  `derive_seed()`; `model_t` and `group_t` for nonlinear groups,
  `group_of_term()` mapping none today.
- **`tools/tuner_layout.hpp`** (new): the parameter layout and
  `starting_params()`, moved verbatim out of `tools/eval_model.hpp`, which
  re-exports them; `tools/tuner_groups.hpp` reads it now.
- **`tools/tuner_optimiser.hpp`** (new): Adam and `descend()` moved from
  `tools/tuner_model.hpp` with their arithmetic unchanged, sized by the model;
  AdaGrad beside it; `--optimiser adam|adagrad`.
- **`tools/tuner.cpp`**: loads through the trace, prints `trace: <rows> rows,
  <entries> entries (<n> a row), <s> s; <m> differ from evaluate(), the clamp
  binds on <c>`, refuses a corpus with a mismatch, and stamps
  `// model      evaluate()'s trace, <rows> rows; 0 differ ...` and
  `// optimiser  <name>`. `constexpr int` output (DEC-255), the S077 stamp,
  `--only` / `--freeze` by name (DEC-057) and S275's `gauges held` line and
  header line unchanged.

### INV-6, and the release cost

`adocs/data/S262_inv6.txt`, parent `f57b92f` built in the same worktree before
any edit.

- `src/evaluation.cpp`'s object is **byte-identical** to the parent's in
  `build/` (Release, native) and in `build-tune/` (`CHESSO_TUNE`), on the final
  tree. The only other `src/` edits are a comment in `src/evaluation.hpp`, the
  new header only `evaluation.cpp` includes, and `src/CMakeLists.txt`'s new
  target, which `chesso` does not link.
- The `chesso` binaries' `.text`, disassembled with addresses kept and
  RIP-relative displacements normalised: **no line differs**, 56277
  instructions at the same addresses in `build/`, 57650 in `build-tune/`. The
  raw listing differs on 344 lines, every one a displacement into `.rodata`,
  which moved because `id name` reads `f57b92f-dirty` against `f57b92f`. Every
  evaluation function -- `evaluate`, `evaluate_cheap`, `evaluate_lazy`,
  `evaluate_expensive`, `evaluate_expensive_terms`, the two collecting
  instantiations, `game_phase` -- has an identical listing (md5 over the
  normalised instructions).
- `bench` **3872032**, every info line identical to the parent's with time,
  nps and hashfull stripped (121 lines); `bench 12` 1730486, 105 lines likewise;
  `tools/search_bench.py` at depth 9 -- 32829 / 69892 / 25174, `c3d5` `e2a6`
  `d7c8q` -- and depth 12 -- 68090 / 246712 / 114555, `c3d5` `d5e6` `d7c8q` --
  identical. `build-tune` `bench` 3872032.
- **Release cost: none to measure.** The object code is the parent's, which
  subsumes a timing (the brief's first branch); no timing was taken.

### Exact reconstruction

- **Every row of the dedup corpus: 10795695 rows, 0 mismatches**, the clamp
  binding on 36503 of them (0.338 %), every one reproduced. The tuner checks it
  on every load and refuses a corpus with one mismatch; the line is in every
  log in `adocs/data/S262_fits.txt`. The first 1 M rows: 0 of 1000000, the
  clamp on 3315.
- **Early returns**: `evaluate()` has none. It is `evaluate_cheap() +
  evaluate_expensive()` on every call; the lazy shortcut is `evaluate_lazy()`'s,
  which the tuner does not fit (nor did the mirror); there is no draw or scale
  path inside `evaluate()` -- draws are the search's. The one nonlinearity, the
  clamp, is in the reconstruction and binds on those 36503 rows.
- **The fast-label fixture**, `test_eval_trace`: the identity on 2764 positions
  -- `test_eval_positions` both ways round (68) and the JSON suites (2696) --
  with the clamp binding on 239 from each side; the truncation bound reached
  (2.875) on the same positions.
- **Smooth against exact**: at most 2.875 over the 1 M-row scan and the fixture,
  3 x 23/24 with tempo at zero, the mirror's bound -- the smooth score is the
  mirror's function.

### The gradient check and the recovery cases

- **Gradient, every parameter**, `test_tuner_gradient` over its fixture,
  central differences, h = 0.01: worst **4.87e-8** (parameter 774, the bishop's
  middlegame mobility weight), against S100's 4.9e-8 on the mirror. Truncation
  is not in the objective (the fit descends the untruncated score), the clamp is
  and is differentiated exactly; the precondition that no fixture row sits
  within a centipawn of the kink is a case of its own.
- **Where the clamp binds**: +40 on the king safety weights binds it on one
  fixture row (263.7 against 184, the next row at 150); all 823 parameters
  within **1.72e-7** of finite differences at h = 0.005 (6.8e-7 at 0.01, too
  near the tolerance to survive a refit); on the binding row alone the clamped
  block's gradient is exactly 0 and the rest's is not (9.3e-4).
- **The synthetic group's gradient**: worst **2.80e-8** over its ten
  parameters, no side within 10 of the transform's kink.
- **(4) S100's planted recovery**, fast: per group with the rest frozen, over
  the 2764 positions, Adam at the tuner's step, 6000 epochs -- passed pawns,
  piece placement and tempo all within 0.05 of S100's numbers (the slowest,
  bucket 1's middlegame weight, 13.95 against 14). End to end: see the
  comparison below.
- **(4) and (5) the nonlinear group kind** (`max(0, x)^2 / 64`, per side, over
  the engine's per-colour attacker counts and zone incidences, subtracted from
  that side): from zero weights the gradient over the group is exactly 0 and 50
  epochs leave it at 0 with the labels unmet (error 3.9e-5); `derive_seed()`
  (DEC-134 form b: each side's mean pre-transform sum over the data put at the
  transform's one-centipawn point, inputs sharing it equally) seeds all ten
  positive, and from there 5000 epochs at step 0.2 recover the planted mg {14,
  9, 11, 18, 4} and eg {6, 4, 5, 8, 2} to within 2e-4, error 2.3e-18. At steps
  1.0 and 0.3 the same seed stalls at 9.2e-8 and 8.8e-8; 0.1 recovers too.

### The pre-registered comparisons

Pre-registration above, written before the first run. `adocs/data/S262_fits.txt`
has every log.

- **(6) holds.** Full dedup corpus, defaults, both: K 0.7795, start held-out
  0.119455, 1079625 rows held out (S066's split of 119978 games, seed 1). **Old 0.118449, new
  0.118446**, both best at epoch 100 and stopped at 2100; tolerance old + 1.0e-5
  = 0.118459. All 21 common reports within 4e-6 held out and 1e-6 train. Emitted
  vectors: 183 of 823 numbers differ, by 2 at most (172 table squares, 6 king
  safety, 2 placement, tempo, mobility, material one each). Wall 12 m 16 s old,
  9 m 03 s new; resident 1.38 GB old, 1.96 GB new.
- **Optimiser.** AdaGrad at 1, 3, 10, 30: 0.118448, 0.118446, 0.118445,
  **0.118436** (best epochs 1600, 300, 100, 5700; 14, 9, 9 and 34 minutes).
  Step 30 is AdaGrad's default. Against Adam's 0.118446 it is lower by exactly
  1.0e-5 at the printed precision, not by more, so **Adam stays the default**
  under the rule. 30 was the edge of the range.
- **(4) end to end**, S275's planted slice, defaults: training error 0.000000;
  **17 of 18** term parameters at their planted integers, `passed_pawn_eg[4]` 71
  against 70 -- K, as under Deviations. **The pre-registered exact criterion
  missed**; S100's own rule, within +/-1, holds (18 of 18). The parent's tuner,
  same slice: the identical 18 numbers, the same 71. Post hoc, `--k 0.7624`: 18
  of 18 exact.

### Extraction time and storage

Measured with nothing else running, `tuner --epochs 0`, three runs each:
**0.8 s per million rows at 12 threads, 3.0 s at one**; the whole corpus 9.1 s
and 32.3 s. **24.72 entries a row**, the dataset **119 MB per million rows** in
memory (4 bytes an entry, 8 for the two offsets, 12 for phase, side, result,
score and ply); nothing is written to disk -- every run re-traces its corpus,
which is what lets every run check the identity. Resident peak over the whole
corpus 1.96 GB.

### Tests

- **Added**: `test_eval_trace` (trace build), seven cases -- the features cover
  the 823 parameters exactly once; the identity on the 2764 positions, clamp
  binding from both sides as its precondition; tracing changes nothing
  `evaluate()` returns and leaves the sink cleared; every coefficient against a
  count the trace did not take (the engine's per-colour accessors, the
  bitboards, the mirror's ray-walked mobility); every non-table feature reached
  both ways (S100's both-directions case, for the trace); the smooth score within 3 x 23/24 of the exact one, and reaching
  it; and the refusals no legal board reaches -- a 1025th entry (the record
  keeps 1024 and flags the overflow), an index past its term, a per-side entry
  without a group, a coefficient past int16, no phase -- each refused through a
  synthetic record handed to `sum_record()`, and the valid record summed after
  each coming out exactly as it does alone. In `test_tuner_gradient`, four new cases: S100's planted recovery per
  group; the synthetic group's gradient; its zero seed stuck and its derived
  seed recovered; AdaGrad leaving frozen and unreached parameters
  bit-identical.
- **Re-targeted to the trace path**, `test_tuner_gradient`: loads through the
  trace (S100); the engine's phase, not the stored column (F36, S134); the
  one-king refusal (S275, through `load_FEN()` now); "every stored row is its
  FEN's trace, at any thread count" (was S100's stored-column case); "the
  fixture sits clear of the clamp" (was S100's "binds on no fixture row", now a
  centipawn of clearance); the gradient check (S100); "the clamp's derivative is
  exact where it binds" -- S100's "the clamp is where gradient() and the error
  part company" with its assertion inverted on purpose, since the treatment it
  pinned is the one this step replaces; the two S275 gauge cases, the second
  under both optimisers, the first also asserting the exact score moves at most
  one centipawn. `test_tuner_groups`: reads `tools/tuner_layout.hpp`, no
  assertion changed.
- **Moved, not retired**: the mirror loader's five cases (S100's three, F36's,
  S275's king refusal) to `test_eval_model`, where the loader's remaining
  reader, `feature_audit`, is covered. Nothing deleted.
- **Observed red**: eleven mutants, each alone, on the final tree, every one red
  (`adocs/data/S262_mutants.txt`): a term on the wrong taper (identity only);
  piece placement's or tempo's coefficient wrong (the meaning case only --
  both ship at zero, invisible to the identity); the clamp ignored in the
  gradient; a group's Black side added; the seed zeroed; the hold dropped;
  Black's squares unmirrored; rows out of order; a trace call dropped; a
  refusal in `sum_record()` that does not clear its buffer (the refusal case
  only, on the three refusals that sum something first).
- **Goldens**: one new site, `test_eval_trace`'s `worst > 2.8`, which reads the
  four truncation positions `test_eval_model` already pins -- named GOLDEN
  (DEC-142) at the site, re-derived by `build/tools/truncation_scan --data
  <corpus> --min 2.8`, row added to DEV_MANUAL's golden table. Everything else
  new is a precondition re-established at run time (clamp binding, the walk
  floor, kink clearance) or a planted input. No existing golden moved.

### Suites

`CLANG_FORMAT_MAJOR=22`, i7-8700K, g++ 13.3, `-j12`, final tree: build 45/45
fast (44 + `test_eval_trace`), build-tune 45/45, `./clang-format.sh --check`
clean; exit 0 (`.tuning/coord/S262/gate_final.log`). `tools/plan_prose_check.py`
`--citations`, `--touches`, `--params`, one mode per call: exit 0 each, 0 flagged. A Debug
configuration (scratch directory) builds the trace targets, and
`test_eval_trace`, `test_eval_model`, `test_tuner_groups` and
`test_tuner_gradient` pass with assertions on (the last 52 s, against the
600 s Debug timeout). No second tier: `make_move`, the generator and the search
are untouched. `bench` 3872032.

### Documents

`DEV_MANUAL.md`: a new "What the tuner fits: evaluate()'s own trace" under
the tuner (the switch, the identity line, the smooth objective with the clamp
and truncation stated, size and time, adding a term, nonlinear groups and the
two measured properties, `--optimiser`, the tests, the comparison, the mirror's
remaining readers); the build section names `chesso_engine_trace`; the test
section's `test_tuner_gradient` paragraphs, the tuner's model paragraph, the
`feature_audit` notes, the gauge paragraph (the engine's one centipawn), the
stride note, the base-chain and `descend()` paths and the golden table follow
it; the mirror's cost figures are marked as the mirror's. `adocs/specs.md`: one
sentence in the evaluation row (the trace build, compiled into no engine binary,
and the identity on every corpus row). `MANUAL.md` checked: no change -- its two
mentions of a tuner are the SPSA harness's.

### Proposed follow-on

**Retire the mirror.** `feature_audit` and `truncation_scan` are the last
readers of `tools/eval_model.hpp` and of the mirror's dataset in
`tools/tuner_model.hpp`.

- goal: `feature_audit` and `truncation_scan` read the engine's trace
  (`tools/tuner_trace.hpp`) instead of the hand-written mirror, and
  `tools/eval_model.hpp` with the mirror's dataset is deleted
- accepts: (1) `feature_audit`'s report re-taken over the dedup corpus reads
  `S275_feature_audit.txt`'s figures -- rank 784, the 39 null directions, 0
  column disagreements -- with its design rows the trace's
  coefficients times the taper weights and its `--plant` labels the trace's
  smooth score; (2) `truncation_scan` reads smooth against exact from the trace
  and reproduces the pinned four's 2.875; (3) `tools/eval_model.hpp`, the
  mirror dataset in `tools/tuner_model.hpp` and `test_eval_model`'s mirror
  cases are deleted, each deletion recorded, the engine-counting cases kept;
  (4) `adocs/data/S275_walk.cpp`'s build pin -- `f57b92f` or earlier -- is
  recorded where its row in `adocs/data/README.md` points
- touches: tools/feature_audit.cpp, tools/truncation_scan.cpp,
  tools/eval_model.hpp, tools/tuner_model.hpp, tools/tuner_trace.hpp,
  tests/test_eval_model.cpp, tests/test_eval_positions.hpp, tests/CMakeLists.txt,
  tools/CMakeLists.txt, DEV_MANUAL.md, adocs/data/README.md

Two notes for the steps that inherit this, not new steps: **S122** fits the
first real nonlinear group and will want a per-group step (the synthetic group
needed 0.2 where the linear terms use 1.0) and `derive_seed()` over the corpus;
**S082/S126**, whoever next compares optimisers on a new corpus, should extend
AdaGrad's sweep past 30, the edge where this one ended.

### Proposed `done:` stamp

2026-10-12 -- the tuner fits evaluate()'s own trace. A trace build
(`CHESSO_TRACE`, on the new `chesso_engine_trace` library only; `chesso` never
links it) records the coefficient of every weight evaluate() applies; the trace
scored with the engine's integer tapers and clamp equals evaluate() on all
10795695 corpus rows (0 mismatches, clamp binding on 36503), checked by the
tuner on every row it loads. INV-6: evaluation.cpp's object byte-identical in
build and build-tune, `chesso`'s `.text` identical but for `.rodata`
displacements, bench 3872032, bench 12 and search_bench 9/12 node- and
move-identical; no release cost to time. The fit descends the untruncated score
with the clamp differentiated exactly -- a reversal of the parent's gradient,
which ignored it, taken because accepts (3) checks the true gradient, on 0.338 %
of corpus rows, (6) measured under it, and S039 retires the clamp; gradient vs
finite differences 4.87e-8 over all 823 (S100: 4.9e-8), 1.72e-7 where the clamp
binds. Nonlinear groups: per-side records, one transform (`max(0,x)^2/d`), a
synthetic group's gradient 2.80e-8, stuck at zero and recovered from a seed
derived over the data (DEC-134 form b). Pre-registered: held-out 0.118449 old
against 0.118446 new (tolerance 1e-5) -- the old path retired from the tuner;
Adam stays default, AdaGrad at its swept best step 30 beating it by exactly the
1e-5 margin, not more. (4) end to end: **the pre-registered criterion, every
planted term weight exact, missed** -- 17 of 18 at the defaults,
`passed_pawn_eg[4]` 71 against 70, from fitting K (0.7557 against the labels'
0.7624); S100's own rule, within +/-1, holds, and the parent's tuner emits the
identical 18 numbers; post hoc at the plant's K, 18 of 18. Extraction 0.8 s per
million rows at 12 threads, 3.0 s at one; 119 MB per million rows. Tests:
test_eval_trace (7), test_tuner_gradient re-targeted (9) plus 4 new, 5 mirror
cases moved to test_eval_model; eleven mutants red. Fast suites 45/45 both,
format clean, prose checks clean, bench 3872032. DEV_MANUAL and specs updated,
MANUAL checked. Implemented from Grant (2020), CPW's Texel's Tuning Method,
Kingma and Ba, Duchi et al. (DEC-221). The mirror stays for feature_audit and
truncation_scan (follow-on).

### Commit text

```
Fit evaluate()'s own trace in the tuner (S262)

The tuner fitted tools/eval_model.hpp, a hand-written copy of the
evaluation held to the engine only within the truncation bound, with a
gradient whose per-term blocks had been forgotten three times. A new
trace build, CHESSO_TRACE on a library no engine binary links, has
evaluate() record the coefficient of every weight it applies. The
tuner loads every row through it, refuses a corpus where the trace
scored the engine's way differs from evaluate() -- 0 of 10795695 rows
do -- and fits the untruncated score, one loop for every term. The
clamp is differentiated exactly, reversing the old gradient, which
ignored it: the true gradient is what the finite-difference check
holds, and the clamp binds on 0.338 % of the rows.

evaluation.cpp's release and tune objects are byte-identical to the
parent's. On the corpus, same split and seed, the held-out error is
0.118446 against the mirror's 0.118449, inside the pre-registered 1e-5,
so the tuner's mirror path retires; feature_audit and truncation_scan
still read the mirror. The format carries per-side sums for nonlinear
groups, tested on a synthetic max(0,x)^2 group with a seed derived from
the data. AdaGrad is selectable; Adam stays the default.

No functional change
```
