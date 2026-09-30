id:         S115
goal:       the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply [the third clause, the root fail-high reduction, was built and refused, DEC-245 -- an annotation of 2026-09-30, the goal as written stands; "halves" is the pull at its seed, the midpoint of its range, DEC-134 form (c)]
accepts:    an SPRT verdict, recorded whatever it is; the changes are measured together only if a sweep shows them inert apart, and otherwise separately -- DEC-082 is a condition to be met, not a convenience; the node-count sweep is run over the 300 stratified positions adocs/data/S021_aspiration_sweep.py picks out of adocs/data/S018_raw.tsv -- PER_PHASE 4 over the 25 phase values that file carries is 100 positions, at each of the script's three offsets -- and not over three, because S021 recorded that one sample chooses the wrong setting; the mate-appears-mid-search case S074 put in the gate still passes. **Read with DEC-245 (2026-09-30)**: the root fail-high reduction -- the goal's third clause -- is refused on this engine's mate guards and does not ship; the step's verdict is the fail-low pull alone, one SPRT, `adocs/data/S115_sprt.sh`, with the widening ratio a parameter re-swept under it
touches:    src/chesso.cpp iterative_deepening_search, src/search_params.hpp; as built also src/chesso.cpp aspiration_after_fail and uci_last_aspiration_searches, src/uci.hpp (their declarations, the S089/S132 pattern), tests/test_engine.cpp (the pull's cases), tests/test_search_params.cpp (two golden rows), tools/mutants/S115_aspiration_pull.py
excludes:   a window width seeded from the score's own volatility -- dropped by DEC-087, no evidence at this band; the initial delta, which S127 fits
decisions:  DEC-063, DEC-082, DEC-084, DEC-087, DEC-105, DEC-134, DEC-141, DEC-142, DEC-143, DEC-156, DEC-162, DEC-171, DEC-215, DEC-221, DEC-239, DEC-244, DEC-245
closes:
blocks:
paused_by:
author:     an Opus subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-30 06:32 CEST
done:

## What is there

The triple this step re-sweeps is **S085's, not S021's**: `ASPIRATION_MIN_DEPTH`,
2 as shipped since S085; `ASPIRATION_DELTA`, 21 as shipped since S085;
`ASPIRATION_MAX_DELTA`, 437 as shipped since S085. S021 measured and shipped
5 / 50 / 400 and the SPSA vector of 2026-08-21 replaced all three
(+21.02 +/- 9.86, verified; `adocs/plan_done/S085_spsa_first_run.md`). Every
number below is read against those, and the sweep's own off row is taken at
them -- taken at S021's triple it would measure S085's axes and this step's
together, and neither number would mean anything.

Widening doubles the failing side alone. **The plumbing is already fail-soft
end to end** -- "Shape for chesso" walks the five return paths and the "Scope
concern" section is the record of this paragraph having once said otherwise, so
Ethereal's +2.6/+5.1 for fail-soft pruning returns is banked here and is not
this step's to win. What the band's engines do that this does not: halve beta
toward alpha on a fail-low (one line), and reduce the root depth on a repeated
fail-high so an unstable root does not burn a whole iteration (Lynx carries
it). The volatility-seeded width the strong engines run has no measured gain
below ~3100 and left this step at DEC-087; S127 owns the delta itself.

## Technical details (SOTA research, 2026-08-19)

Sources are prose only -- commit messages via the GitHub API, PR bodies, forum
threads, CPW. No engine source file was opened (DEC-084). Every constant below
is a seed and must be swept or SPSA'd here.

### State of the art

- **Fail-soft re-centering.** The re-search window is built from the score the
  failed search returned, not stepped blindly from the old centre. Stockfish's
  one-line record: "Center aspiration window on last returned score"
  (b50eb6bea8, 2013). CPW: start small, widen "the bound that fails in an
  exponential fashion", and "the bound that didn't fail is unchanged".
- **The midpoint pull on the opposite bound.** On a fail-low, alpha is pushed
  down *and beta is pulled toward alpha*. The long-standing Stockfish form was
  `beta = (alpha+beta)/2`, stated in prose by commit 57b32f3e60 (2025-07-30),
  which moved it to `(3*alpha+beta)/4`: "Before it was beta=(alpha+beta)/2 ...
  we can't trust the score that caused the failLow. Therefore beta=alpha
  doesn't work." So the goal's direction is the published one: fail-low moves
  **beta** toward alpha; the classical fail-high leaves alpha alone. Weiss
  #183 (b086028c3d, 2020) is the band-level record and is this step's exact
  bundle: "Reduce depth when resolving fail highs, lower beta when resolving
  fail lows, and reduce the rate at which the aspiration window grows" --
  **+6.78 +/- 4.87 at 10+0.1 and +8.19 +/- 5.31 at 60+0.6**, both H1 on
  [0, 5]. A 2025 Stockfish generalisation (weighted averages on both fail
  directions, bfc7000597) was **reverted three weeks later** for a bug "when
  the position is close to mate or being mated" (fc54d87301) -- do not import
  it.
- **Root fail-high depth reduction.** Stockfish 3a572ffb48 (2019), prose:
  increment failedHighCnt on fail highs, which "makes the search try again at
  lower depth", and reset the counter on a fail-low. talkchess t=83781: the
  point is to have time to complete the failed-high move with an open window,
  and the reduced re-search still scores it fully because the prior
  iteration's LMR no longer applies to it (Uri Blass). Lynx PR #800 (merged
  2024-10-07) is the band-level measurement: **+2.20 +/- 1.78 at 40+0.4** (H1
  on [0, 3]), re-measured +4.82 +/- 3.11; capping the reduction at 2-3 or
  gating it on alpha measured -1 to -2 and was rejected (#1102, #1103, #1141).
- **Depth gating.** Weiss enabled windows "for depth > 6" (#30, 2019);
  Althoff +/-50 cp from depth 4, Buijs +/-15 cp from depth 4 (t=76115).
  S021 measured 5 here; S085 moved it to 2, the arithmetic floor its own
  `min` allows, inside a +21.02 +/- 9.86 verified vector.
- **The dropped branch, confirmed dropped.** Eval-scaled width exists at the
  top (SF 0150da5c2b, 2019); Weiss *removed* its extreme-score adjustment as a
  passed simplification, -0.72 +/- 1.13 (#668, 2023) -- band-level support for
  DEC-087's exclusion.

### Shape for chesso -- today's loop

The plumbing is **already fail-soft end to end**: in `src/search.cpp`
`negamax`, the ordinary return hands back `best_so_far`, RFP returns
`static_score - margin` and the null move returns `null_score`; TT cutoffs
return the stored score and not the bound (`src/search.cpp`
`tt_entry_answers`); and `src/search.cpp` `quiescence` returns `best_value`.
Ethereal's +2.60/+5.07 for fail-soft pruning returns
(a0d84b633e) is already banked here.

The loop is all of `src/chesso.cpp` `iterative_deepening_search`: init
`aspiration_score +/- ASPIRATION_DELTA` from depth `ASPIRATION_MIN_DEPTH`;
re-search while the score is a bound -- fail-low `alpha = max(score - delta,
-SEARCH_SCORE_INF)`, fail-high `beta = min(score + delta, SEARCH_SCORE_INF)`,
so the failing side is already re-centered on the returned score; `delta +=
delta`; re-search at the **same depth**; mate or `delta >
ASPIRATION_MAX_DELTA` jumps to the full window. The resolved score becomes the
next centre and a mate disarms the window. The counter `src/chesso.cpp`
`last_aspiration_failures` is test-only, declared once and read through its
accessor.
**Missing: the opposite bound is never touched, and the root is never
reduced.** The changes: (a) fail-low additionally sets `beta = (alpha + beta) /
2` before pushing alpha; (b) a consecutive-fail-high counter makes the
re-search run at `max(1, current_depth - count)`, reset on fail-low; (c) the
widening schedule is re-swept under (a)+(b).

### Implementation sketch

1. (a) then (b), each a few lines in iterative_deepening_search; keep the
   existing clamps and the mate/max escape around both.
2. Re-sweep with adocs/data/S021_aspiration_sweep.py (tune build, depth 11,
   the three stratified offsets -- 300 positions, never the three-position
   bench), with (a) and (b) toggled apart and together, multiplier variants
   x1.5/x2/x3 in the same table. **The off row is taken at the shipped triple,
   2 / 21 / 437, and re-read from `src/search_params.hpp` before the run**:
   S021's script was written when 5 / 50 / 400 compiled, and an off row at
   those values would put S085's verified vector inside this step's own
   comparison.
3. Verdicts per the accepts: **one SPRT if the sweep shows (a) and (b) inert
   apart, otherwise one each -- 1 to 3, expected 1** (Weiss shipped the whole
   bundle as one patch). The schedule choice itself is a node-count decision
   (the S021 pattern) and folds into whichever SPRT ships.
4. Tests before any verdict is read:
   - the S074 gate case ("engine: aspiration windows", tests/test_engine.cpp)
     stays green: mate appears mid-search above the gate, failures > 0, every
     later iteration reports the same mate distance;
   - a Release-visible engine-loop test in the S021 "windowed root" style: a
     root fail-high resolved by a *reduced* re-search must not lose the best
     move -- assert bestmove/PV agree (`search.cpp` `negamax` is the recorded
     failure shape);
   - loop termination: bounded number of fails to the full window, with (a)
     active.

### Constants and seeds

**Rewritten 2026-09-30 in DEC-134's three forms, by the coordinator's ruling 1
for this step.** The table this replaces predated DEC-134 and seeded the pull,
the reduction and the ratio from numbers quoted in engine commit prose; under
DEC-105 as amended by DEC-134 a number quoted in another engine's commit message
is that engine's constant and is never a seed. The published record above stays
as the record of what the technique is and what it measured elsewhere.

| quantity | parameter, range | seed | form |
|---|---|---|---|
| the fail-low pull's weight | `AspirationFailLowPull`, quarters of the window, 0 to 4 | 2, one half | **(c)**, the range midpoint, stated as such. That one half is also a form the record carries is a record, not the number's source. 0 is the off value; 4 puts beta on the old alpha |
| the root fail-high reduction | **refused, DEC-245; not in the tree** (as built: `AspirationFailHighReduce`, a switch, 0 to 1) | (1, as built) | as built, the step was **one ply per consecutive root fail-high, the smallest unit a depth reduction has, stated as such**; floored at depth 1; the count put back to zero by a fail-low and, by ruling 5, by a mate score; no cap -- that caps measured negative in the published record is a record, not a reason. It turned three mate guards red and its code left before landing (`adocs/data/S115_reduction_as_built.diff`) |
| the widening ratio | `AspirationWidenPct`, percent, 100 to 400 | 200, the doubling S021 shipped | **(b)**, a derivation over chesso's own positions: `adocs/data/S115_aspiration_sweep.py`, 150 / 200 / 300 over S021's 300 positions, the ratio moving only on a lead held on every sample (DEC-244). Under the pull alone (`--rows pull`, the tree that ships) 150 led 200 on two samples of three and 300 on one; with both rules (`--rows first`) each led on one; so 200 stays, and is S127's to fit with this table as input |
| depth gate | `AspirationMinDepth` | 2 | S085's SPSA fit, (b); not touched -- a sweep does not re-decide a fitted value (DEC-244) |
| initial delta | `AspirationDelta` | 21 | S085's fit; excluded, S127 owns it |
| max delta escape | `AspirationMaxDelta` | 437 | S085's fit; not touched |

### Pitfalls

- **Bound arithmetic near infinity.** SEARCH_SCORE_INF is 2000000000
  (`src/search.hpp` `SEARCH_SCORE_INF`): never compute `(alpha+beta)/2` while
  either bound is infinite -- armed bounds are finite, but the full-window
  escape must bypass the pull. Lynx #1275 ("windows outside [MinEval, MaxEval]
  after overflow") is the published instance of getting this wrong.
- - **Mate-band windows.** MATE_MAX 49000 (`search.cpp` `MATE_MAX`). Keep the
  existing guards -- in `src/chesso.cpp` `iterative_deepening_search` mate ends
  the schedule and disarms the next window, the published form (SF 1f73a9ed63:
  mate scores made "aspiration blow up in a series of researches loops";
  8acb1d7e4d) -- and do not reduce the root when the fail-high score is a mate:
  SF's opposite-bound patch was reverted for a near-mate bug (fc54d87301), and
  Lynx
  #2560 added mate-range guards after false mate reports "specially after being
  saved in TT".
- **Fail-soft scores as re-centres.** SF 57b32f3e60's caution verbatim
  applies: the fail-low score is an untrusted upper bound (at this root it is
  the max over null-window children), so `beta = alpha` overshoots -- keep a
  buffer. That is the published reason a weight below the whole window exists
  and **not where this step's weight comes from**: `AspirationFailLowPull` 2
  is the midpoint of its range 0 to 4, DEC-134 form (c), as the seeds table
  above states it.
- **Reduction reaching depth 0.** Berserk shipped "Fix aspiration window from
  entering QSearch" (0645ca1079) -- floor the adjusted depth at 1.
- **Alternation loops.** The pull shrinks beta, making fail-low -> fail-high
  cycles likelier (Madsen's pathological cycles, t=76115). The delta escape
  must still bound the loop, and a fail-low resets the fail-high counter.
- **TT on re-searches.** A re-search at the same or reduced depth reads
  entries the failed attempt just wrote; the root never takes a TT cutoff
  (is_pv) but everything below does. Mate scores written during a fail are
  the recorded hazard (8acb1d7e4d, Lynx #2560). tt_new_search runs once per
  go (`chesso.cpp` `search_book_move`) -- do not age per re-search.
- **Root ordering across fails.** A root fail-high publishes the cutoff move
  and a one-move PV (`search.cpp` `negamax`, S021's bug fix); the reduced
  re-search must find that move first via the TT root entry -- assert, not
  assume.
- **Time management.** S089's scaler reads completed in-window iterations only
  (`chesso.cpp` `iterative_deepening_search`); fail events feed nothing. Keep
  it that way: Lynx measured soft-limit checks inside the window loop at -7.4
  to -94.5 Elo, all rejected (#2212-#2214). A fail-low near the soft limit is
  stopped by the hard timer alone, today and after this step.

### Measurement

S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`. Band evidence:
+6.78/+8.19 (Weiss, the full bundle), +2.20/+4.82 (Lynx, reduction alone) --
the expected effect sits near elo1, so state the expectation and each
outcome's reading before launching (DEC-063). Verdict count per the accepts:
1 if the sweep shows the parts inert apart, else up to 3. Gate: fast suite
and the S074 mate cases green at the shipping schedule; the sweep over the
300 stratified positions.

### Interactions

- **S089 (done).** Verified: the budget does *not* react to fails -- its inputs
  are best-move stability and the completed iteration's score drop
  (`chesso.cpp` `iterative_deepening_search`). This step adds no mid-loop time
  checks (Lynx
  #2212-#2214 measured them negative).
- **S132 (later).** Node-fraction soft-limit scaler, same function -- land
  S115 first as ordered; nothing here reads node shares.
- **S097 (before this, position 22).** Extensions change score stability, so
  the re-sweep runs against the post-S097 tree: re-run the off row, do not
  reuse S021's numbers.
- **S127 (after).** SPSA may re-fit the pull weight and multiplier; ship them
  as S073-set parameters as S021 did with its three.

### Scope concern

The "What is there" paragraph implies the fail-soft plumbing is missing. It is
not: every return path is already fail-soft -- the three in `src/search.cpp`
`negamax`, plus `src/search.cpp` `tt_entry_answers` and `src/search.cpp`
`quiescence` -- and the failing bound has re-centered on the returned score
since S021, on both sides, in `src/chesso.cpp` `iterative_deepening_search` --
Ethereal's +2.6/+5.1 rewarded fail-soft *pruning returns*, which RFP and null
move here already do. What remains of the goal's first clause is the re-sweep
itself; the new behaviour is the midpoint pull and the root reduction. The
goal's direction is the published one -- fail-low pulls **beta** toward alpha,
`(alpha+beta)/2` -- confirmed by SF prose (57b32f3e60) and Weiss #183 ("lower
beta when resolving fail lows"). No change to goal or accepts is needed.

### References (all read 2026-08-19, as prose)

- - https://www.chessprogramming.org/Aspiration_Windows -- sizes, exponential
  widening, the unchanged bound.
- - https://github.com/official-stockfish/Stockfish/commit/57b32f3e60 --
  fail-low pull was (alpha+beta)/2, now (3a+b)/4; the distrust caution.
- - https://github.com/official-stockfish/Stockfish/commit/3a572ffb48 --
  failedHighCnt: retry lower, reset on fail-low.
- - https://github.com/official-stockfish/Stockfish/commit/bfc7000597 and
  /commit/fc54d87301 -- both-sides weighted form, and its near-mate revert.
- - https://github.com/official-stockfish/Stockfish/commit/5c93616a3f -- 2025:
  narrow the window after fail-high.
- - https://github.com/official-stockfish/Stockfish/commit/b50eb6bea8 -- centre
  on the last returned score.
- - https://github.com/official-stockfish/Stockfish/commit/49dfc50b12 -- 2010
  widening progression.
- - https://github.com/official-stockfish/Stockfish/commit/1f73a9ed63 and
  /commit/8acb1d7e4d -- mate scores versus the window loop.
- - https://github.com/official-stockfish/Stockfish/commit/0150da5c2b --
  eval-scaled width (the DEC-087-dropped branch).
- - https://github.com/official-stockfish/Stockfish/issues/2169 -- repeated
  fail-high node waste in won positions.
- - https://talkchess.com/viewtopic.php?t=83781 -- why reduced-depth re-search
  after a root fail-high works.
- - https://talkchess.com/viewtopic.php?t=76115 -- 15-50 cp windows from depth
  4; instability and cycle cautions.
- - https://github.com/lynx-chess/Lynx/pull/800 -- fail-high reduction
  +2.20/+4.82; /pull/1102 /pull/1103 /pull/1141 -- caps and conditions
  rejected.
- - https://github.com/lynx-chess/Lynx/pull/1275 -- overflow clamp; /pull/2560
  -- mate-range guards and the TT interplay.
- - https://github.com/lynx-chess/Lynx/pull/2212 /pull/2213 /pull/2214 --
  soft-time checks inside the loop, all negative.
- - https://github.com/TerjeKir/weiss/commit/b086028c3d -- Weiss #183, this
  step's bundle, +6.78/+8.19.
- - https://github.com/TerjeKir/weiss/commit/47896263c5 -- gate at depth > 6;
  /commit/3b553e9047 -- extreme-score adjustment removed.
- - https://github.com/AndyGrant/Ethereal/commit/a0d84b633e -- fail-soft
  pruning returns +2.60/+5.07; /commit/46d90fe649 -- cap reported bounds to
  [alpha, beta].
- - https://github.com/jhonnold/berserk/commit/0645ca1079 -- window entering
  qsearch fixed; /commit/99e178f97e -- windows off at |score| > 1000.

## What the tree already did, 2026-09-30 (checked at `cd50c7a`)

The step file was written at the 2026-08-19 tree; each assumption was checked
on this one by symbol before any code was written. **This tree's engine is
`794e4c3`'s: S114 verdict 1's candidate, the eval-scaled null-move reduction,
whose SPRT against `d946b6f` was running while every number below was taken.**
If that run reads H0 or a zero the term leaves, and this step is rebased onto
the tree without it with every number re-taken -- S114 itself went through
exactly that on 2026-09-29.

- **The loop** (`src/chesso.cpp` `iterative_deepening_search`) was as "Shape
  for chesso" walks it: the band `aspiration_score +/- ASPIRATION_DELTA` from
  `ASPIRATION_MIN_DEPTH`; a fail-low set `alpha = max(score - delta,
  -SEARCH_SCORE_INF)` and a fail-high `beta = min(score + delta,
  SEARCH_SCORE_INF)`, so the failing side was already re-centred on the
  returned score; `delta += delta`; every re-search at the iteration's own
  depth; a mate score or `delta > ASPIRATION_MAX_DELTA` went to the full
  window; a mate disarmed the next iteration's band. The opposite bound was
  never touched and the root never reduced. `last_aspiration_failures` is
  test-only, read through `uci_last_aspiration_failures`.
- **Its comment's schedule was stale**: "25, 50, 100, 200, 400, full ... six
  searches at one depth" is S021's delta 25 and cap 400 counted one short. At
  S085's 21 / 437 a failure moves its bound 21, 42, 84, 168 and 336 past the
  score and the sixth goes to the full window: seven searches at worst. The
  rewrite says so.
- **The fail-soft plumbing, re-read**: `negamax_at` returns `best_so_far`;
  reverse futility returns `rfp_eval - margin` (S234's estimate at
  `RfpTtEstimate` 1, where the step file says `static_score - margin`); the
  null move returns `null_score`, or beta inside the mate band;
  `tt_entry_answers` hands back the stored score; `quiescence` returns
  `best_value`. Ruling 4 accepts the scope concern as written.
- **The triple** is S085's: `AspirationMinDepth` 2, `AspirationDelta` 21,
  `AspirationMaxDelta` 437 in `src/search_params.hpp`.
- **The parent's numbers**, a Release build of `cd50c7a` saved before any edit
  (`.ref-builds/parent/chesso_release`): `bench` **4192793** with c3d5 d5e6
  d7c8q g7h8q d8e7 a1b2 e5e6 e5e6; `bench 12` 1792474 with d8e7 -> a7a6 in the
  fifth; `tools/search_bench.py` 23875 / 69842 / 21491 at 9, best g5f6 e2a6
  d7c8q, and 106886 / 237812 / 86217 at 12, best c3d5 d5e6 d7c8q -- S114
  verdict 1's candidate, node for node
  (`.tuning/coord/S115_logs/parent_*`).

## The sweep, 2026-09-30: neither part is inert apart, and the ratio stays 200

`adocs/data/S115_aspiration_sweep.py` on the tune build of this working tree
(copied aside to `.ref-builds/sweep/chesso_tune` so no rebuild could replace it
mid-run), `go depth 11`, cold table per position, niced beside S114 verdict 1's
SPRT, 06:44 to 06:47 (`.tuning/coord/S115_logs/sweep_d11.tsv`). The positions
are S021's own pick executed rather than copied, at **S021's three offsets 0,
37 and 71** -- the ones its TSV and `adocs/data/README.md` record; S113, S114
and S132 took 0, 1 and 2, and S132's docstring says 0, 1 and 2 are the
aspiration rows' 300, which they are not. The off row is the shipped triple
re-read from `src/search_params.hpp`, 2 / 21 / 437, at both switches off --
the parent's tree, proved separately below. Node counts, not Elo (DEC-019).

| row | pull | reduce | ratio | pooled nodes | rel | per sample (0 / 37 / 71) | best moves changed of 300 |
|---|---|---|---|---|---|---|---|
| off | 0 | 0 | 200 | 40704024 | 1.0000 | 1.0000 / 1.0000 / 1.0000 | 0 |
| pull | 2 | 0 | 200 | 41721551 | **1.0250** | 1.0267 / 1.0350 / 1.0192 | 44 |
| reduce | 0 | 1 | 200 | 37385683 | **0.9185** | 0.9241 / 0.8554 / 0.9472 | 62 |
| both | 2 | 1 | 200 | 37419505 | 0.9193 | 0.9257 / 0.8796 / 0.9361 | 72 |
| both150 | 2 | 1 | 150 | 37727971 | 0.9269 | 0.9678 / 0.8533 / 0.9441 | 75 |
| both300 | 2 | 1 | 300 | 37743206 | 0.9273 | 0.9288 / 0.8752 / 0.9525 | 69 |

**The readings, stated in the script before the run and applied by it.** A
part is inert apart when alone it moves the pooled count by less than 1 % of
the off row and changes at most 3 of the 300 best moves. The pull moves it
**+2.50 %** -- more nodes, on every sample -- and changes 44; the reduction
moves it **-8.15 %** and changes 62. **Neither is inert apart**, so DEC-082's
condition does not hold and the ruling's alternative applies: **two SPRTs, the
reduction first**, each against the commit before it. The ratio moves off 200
only for a ratio whose `both` row leads the 200 row on every sample: 150 and
300 each lead on sample 37 alone, so **`AspirationWidenPct` stays 200**
(DEC-244's test), and 200 is the parameter's (b) value by this derivation.

## Stopped, 2026-09-30 07:05: the root fail-high reduction turns three mate guards red, in every form tried; the pull alone keeps them green

**The brief's rule applied: a red that is not a mined golden re-derived by its
own script stops the step.** Every log named here is under
`.tuning/coord/S115_logs/` in the main checkout. Built so far, not committed: the code (both rules,
the ratio as a parameter, the two switches, the schedule as two pure functions
and a trace of every root search for the tests -- below), the off-value
identity proved, the sweep above. No test, mutant or pre-registration is
written yet; they wait on the ruling.

**The reds**, the Release fast suite on the shipped seeds (pull 2, reduce 1,
ratio 200), niced beside the SPRT: 38 of 41, three red
(`.tuning/coord/S115_logs/fast_release_first.log`).

1. **`test_engine`, "a narrowed window still finds a mate that appears
   mid-search"** -- the S074 gate case -- `REQUIRE( iterations[i].kind ==
   "mate" )`, cp against mate, first position at depth 9. Its golden's own
   script, `adocs/data/S188_repair_goldens.py first-mate`, reads the first
   position's first mate at iteration **12** where the parent reads 9, past
   the case's ten iterations, and reports "no mate inside the swept depth"
   at the case's depth 10 (`s074_first_mate.log`): not re-derivable inside
   the case.
2. **`test_engine`, "a proved mate is never mis-scored, and every mate in two
   is found on time"** -- S145's constructed set -- `REQUIRE( first_exact ==
   minimum )`, rook0_black first reported at iteration 4 and not 3. Over
   UCI, **21 of the 26 mates in two** are first reported at iteration 4 or 5
   (`mate2_configs.out`). The minimum `2m - 1` is not a golden; it is the
   first iteration that can hold the mate.
3. **`test_mate_breadth`**: **102** exact at depth 10 against the floor 143
   (the parent 147).
4. `test_search_params`: 75 parameters against the table's 72 -- the three
   rows the step owes, not a finding.

**Attribution, measured, never argued.** The same three instruments over UCI on
every configuration (`s074_first_mate.log`, `mate2_configs.out`,
`breadth_configs.out`, `variant_C_guards.out`; the variants are throwaway
builds of a copy of this tree, their one-line diffs `variant_A.diff`,
`variant_B.diff`, `variant_C.diff`):

| configuration | S074 first mate, positions 1 / 2 (the case holds 10 iterations) | mates in two first reported at iteration 3, of 26 | mined set exact at depth 10 (floor 143) |
|---|---|---|---|
| parent `cd50c7a` | 9 / 9 | 26 | 147 |
| **pull alone** (reduce 0) | 9 / 9 | **26** | **148** |
| reduction alone (pull 0), as described | 12 / 9 | 5 | 103 |
| both, as described (the seeds) | 12 / 9 | 5 | 102 |
| A: the escape to the full window puts the count to zero | 10 / 9 | 5 | 109 |
| B: the count capped at 1 | 10 / 9 | 5 | 144 |
| C: the first fail-high re-searched unreduced, each repeat a ply (the goal's "repeated") | 10 / 9 | 20 | 124 |

**The mechanism, from the trace** (the instrument below, driven outside the
suite). rook0_black at iteration 3: the depth-3 search in `[-990, -948]`
fails high at -948, the re-search runs at depth 2 and lands inside
`[-990, -927]` at -948, and that settles the iteration -- a mate in two needs
three plies; the parent re-searched at depth 3 and found it there. At
iteration 4 the depth-3 re-search finds the mate and the mate guard puts the
count to zero, so the full-window search confirms it at depth 4
(`trace_rook0_black_d5.txt`). The S074 position does the same at iterations
9 to 11, and at 10 and 11 six fail-highs in a row put the full-window escape
at depths 4 and 5. **The reduction settles iterations with shallower trees by
design** -- over the sweep's 300 positions at depth 11, 848 of 3300 iterations
were settled by a search 1 to 5 plies shallower (670, 141, 26, 9, 2), and none
of the 8 full-window escapes ran shallower (`census_both_d11.txt`) -- so every
guard that asserts a mate by a nominal iteration reads it later. The pull
alone moves none of them.

**Options for the ruling**, none taken here:

1. **Measure the pull alone** as the step's first verdict, the reduction's
   code kept behind `AspirationFailHighReduce` 0 as the second verdict's
   switch (the DEC-243 pattern) or taken out; the ruling's order "the
   reduction first" was written before this.
2. Measure the reduction by SPRT anyway: the three guards would have to
   change what they assert, which is a test decision and not this step's to
   take (never relax a test).
3. Record the reduction as refused on this engine's mate guards and close the
   step on the pull.

Every variant's sweep is recorded beside the main one (`sweep_d11_variant_A.tsv`,
`_B.tsv`): A is node for node the described rule at ratio 200 over the 300
(no escape ran shallower there) and B reads 0.9381 alone and 0.9307 with the
pull.

## The ruling, DEC-245 (the coordinator, 2026-09-30)

**The root fail-high depth reduction is not adopted on this engine as its mate
guards stand**, on the table above: its guards assert that iteration d
searches depth d, the property the technique trades away by design, and
relaxing them is a test decision reserved to the owner, parked as the owner's
question with this record. **Its code left the tree entirely** -- no switch at
0 (DEC-215 keeps a switch only for a planned second verdict; DEC-141 lets no
reduction rule ship without a guard test that passes). `aspiration_search_depth`,
the `AspirationFailHighReduce` row, the counter in the window, its mate guard
and the trace fields only it read are gone; the record is the stop section
above, the trace and census logs, and under `adocs/data/`
`S115_reduction_as_built.diff` (the code as built, `git diff cd50c7a -- src`
taken before the removal) and `S115_reduction_variant_A.diff`, `_B.diff`,
`_C.diff` (the three one-line variants, applied on top of it), with README
rows, so the owner can re-open it from the tree. **S115 ships the fail-low pull
alone, one `{0, 5}` SPRT**, the widening ratio a parameter at 200 for S127.
The sweep's first run above, with both rules, stays as what it was: the
record that decided the verdict count before the reduction left.

## As built: the fail-low pull alone (DEC-245), 2026-09-30

Implemented from the step file's description ("Technical details", the loop
walk and the sketch), written from the wiki and published prose (DEC-221); no
other project's code was opened and no constant of one is behind any value
here. **Every number in this section is from the tree at `cd50c7a`, S114
verdict 1's candidate** -- whose SPRT ended at 07:31 while this was being
built, **H0** at 2720 games (`Elo -18.54 +/- 10.19`, `.tuning/coord/S114_sprt.log`
as read, not recorded by me). Its pre-registration takes the term out on that
reading, and this step is then rebased onto the tree that leaves, every number
re-taken there -- the coordinator's call, not made here.

**What lands.**

- `src/chesso.cpp`: `aspiration_after_fail`, the loop's window update as a pure
  function -- the escape (a mate score, or a band past `AspirationMaxDelta`,
  to the full window), **the pull** on a fail-low, only between two finite
  bounds, the width in 64 bits, then alpha below the score; the fail-high
  unchanged; the widening `max(delta + 1, delta * AspirationWidenPct / 100)`,
  at 200 the parent's `delta += delta` exactly. `iterative_deepening_search`
  calls it, every re-search at the iteration's own depth, and records every
  root search into `uci_last_aspiration_searches` through the file-local
  `aspiration_root_search`, which nothing in the engine reads. The loop's
  comment says what the schedule is now; its old "25, 50, 100, 200, 400 ...
  six searches" was S021's delta and one short.
- `src/search_params.hpp`: `AspirationWidenPct` 200 (100..400) and
  `AspirationFailLowPull` 2 (0..4), each form at its site, the refusal named.
- `src/uci.hpp`: the declarations -- `aspiration_window_t`,
  `aspiration_after_fail`, `aspiration_search_t`,
  `uci_last_aspiration_searches` -- a deviation from the brief's two files,
  accepted by the coordinator as the S089/S132 pattern.
- `tests/test_engine.cpp`, "engine: aspiration windows": five cases.
  "a fail-low pulls beta toward alpha and pushes alpha below the score" (the
  arithmetic, both bounds, and that a fail-high leaves alpha alone); "the pull
  is never computed while either bound is infinite" (the guard through the
  pure function, since no search can put the loop there, and the escape);
  "each failure widens the band by AspirationWidenPct and the schedule ends
  whichever way it fails" (the width rule; fail-lows, fail-highs and the
  alternation the pull makes likelier reach the full window in the same number
  of failures, six at the shipped values counted by doubling; in the tune
  build the same at the ratio's floor of 100, `AspirationMaxDelta -
  AspirationDelta + 2` failures, finite); the tune-only "at
  AspirationFailLowPull 0 a fail-low moves alpha alone"; and "the loop
  re-searches a failed root with the window the schedule gives", S021's four
  windowed-root positions at `go depth 8` held to the function search by
  search -- the band each iteration starts with, every repeated search a
  failure, the next window exactly the function's, beta pulled and alpha
  pushed after a fail-low (16 such re-searches over the four as built, the
  precondition asking for one), no iteration past the schedule's bound, and
  S021's postcondition that a published line starts with the best move, on
  every re-search. `#include "search.hpp"` for `SEARCH_SCORE_INF`.
- `tests/test_search_params.cpp`: the two golden rows, 72 -> 74, its history
  comment naming the third row that left with its rule.
- `tools/mutants/S115_aspiration_pull.py`, AW01 to AW04.
- `adocs/data/`: `S115_aspiration_sweep.py` and its two readings,
  `S115_reduction_as_built.diff` and the three variants, `S115_sprt.sh`, and
  their README rows. `MANUAL.md`'s two option rows, the `AspirationMaxDelta`
  row's "stops doubling" and the aspiration sentence under "Known bugs and
  limitations"; `DEV_MANUAL.md`'s bench ledger and two golden rows.

**The sweep on the tree that ships**, `--rows pull`
(`adocs/data/S115_aspiration_sweep_pull_d11.tsv`, 07:13 to 07:15, niced):

| row | pull | ratio | pooled nodes | rel | per sample (0 / 37 / 71) | best moves changed of 300 |
|---|---|---|---|---|---|---|
| off | 0 | 200 | 40704024 | 1.0000 | 1.0000 / 1.0000 / 1.0000 | 0 |
| pull | 2 | 200 | 41721551 | **1.0250** | 1.0267 / 1.0350 / 1.0192 | 44 |
| pull150 | 2 | 150 | 41389747 | 1.0168 | 1.0544 / 1.0123 / 1.0013 | 58 |
| pull300 | 2 | 300 | 41623111 | 1.0226 | 1.0354 / 1.0085 / 1.0235 | 56 |

The pull row is the first run's node for node. 150 leads 200 on samples 37
and 71 and not 0, 300 on 37 alone: **the ratio stays 200** (DEC-244), S127's
input. The pull costs more nodes on every sample -- reach, not a forecast
(DEC-239).

**The off value, proved on the tree (DEC-215).** The tune build at
`AspirationFailLowPull` 0, `AspirationWidenPct` at its 200, against the
parent's Release build saved before any edit: `bench` **4192793** with all
eight replies and the whole 121-line stream identical, time and nps stripped;
`bench 12` 1792474, its 105-line stream identical; `tools/search_bench.py`
23875 / 69842 / 21491 at 9 and 106886 / 237812 / 86217 at 12, best moves g5f6
e2a6 d7c8q and c3d5 d5e6 d7c8q -- the parent node for node, re-proved after
the reduction's code left (`.tuning/coord/S115_logs/counts_pull.out`). The tune
build at its defaults prints the Release candidate's bench stream line for
line.

**The counts, parent -> candidate** (counts only, niced):

| instrument | parent (`cd50c7a`) | candidate |
|---|---|---|
| `bench` | 4192793 | **4054253** (-3.30 %) |
| `bench` replies | c3d5 d5e6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | c3d5 d5e6 **d7c8r** g7h8q d8e7 a1b2 e5e6 e5e6 |
| `bench 12` | 1792474 | 1797180 (+0.26 %), replies unchanged |
| `search_bench` 9 | 23875 / 69842 / 21491, g5f6 e2a6 d7c8q | 23875 / 69814 / 23286, same moves |
| `search_bench` 12 | 106886 / 237812 / 86217, c3d5 d5e6 d7c8q | 105253 / 237784 / 85846, same moves |
| fixed-node depth, `go nodes 1000000`, Hash 16 | 18 / 14 / 16 = 48, c3d5 d5e6 d7c8q | 17 / 14 / 15 = **46**, c3d5 **e2a6** **d7c8r** |

The fixed-node depths are `adocs/data/S097_fixed_node_depth.py`, stated as
reach and not as a forecast (DEC-239). INV-6 does not discharge the change; the
SPRT does.

**The mate instruments.** Both fast suites green. The three guards that
refused the reduction, each on its own over UCI as well: the S074 case's
first mates 9 and 9, `adocs/data/S188_repair_goldens.py first-mate`
(`s074_first_mate.log`); all 26 mates in two first reported at iteration 3
(`mate2_configs.out`); the mined set 148 exact at depth 10 against its floor of
143, the parent 147 (`breadth_configs.out`). The constructed set's
`MATE_IN_THREE_FLOOR` count, 13 of 24 on both, and mates in five 0 -> 1 of 16
(`mate_set_counts.out`). **A finding older than this step**: the two floors'
golden comments table shipping ends the parent no longer reads -- 145 where it
reads 147, 12 where it reads 13; both floors still separate on the shipping
side, and neither the floors nor their weakened ends were re-derived here. A
test's comment, green, no play reached: for the next filler batch (DEC-171).

**Mutation (DEC-141 clause 2).** `tools/mutation_check.py` over the whole
`tools/mutants/` directory with `--only AW01 AW02 AW03 AW04`, so every anchor
of the 166 validates, on a clean detached fixture, `.ref-builds/mut` at
`d9c9eda` -- a commit of this working tree on no branch -- `--jobs 4`, niced:
header `baseline green, 41 tests, bench 4054253 nodes via engine`; **mutation
score 4 of 4 (100 %)**, wall 678 s (`.tuning/coord/S115_logs/mutation.log`,
`mutation_s115/results.tsv`). AW01 and AW02 at "a fail-low pulls beta toward
alpha and pushes alpha below the score" and at the loop case's point 3 (14 and
16 of its checks); AW03 at "the pull is never computed while either bound
is infinite", **bench signature unmoved** -- no search reaches the guard, which
is why it is held on the function; AW04 at "each failure widens the band by
AspirationWidenPct and the schedule ends whichever way it fails" and at
`test_mate_carry`'s majority. The worktree differs from the fixture since by
three comments (the trace's storage, two in the infinity case), nothing an
assertion or a mutant reads.

**Suites and checks** (niced, the SPRT's last minutes and after it). Release
`build` 41 of 41 (359 s), tune `build-tune` 41 of 41 (125 s)
(`fast_release_pull.log`, `fast_tune_pull.log`); `./clang-format.sh --check`
clean with `CLANG_FORMAT_MAJOR=22`; `tools/plan_prose_check.py` `--citations`,
`--touches` and `--params`, one mode per call, 0 flagged. **Debug**:
`build-debug`'s whole `test_engine`, 80 of 80 cases, 870826 assertions, no
`assert` fired -- `aspiration_after_fail`'s three live there
(`debug_test_engine.log`). **Not run here, by the brief**: DEC-141's second
tier -- the Debug self-play, which is a match, and `tools/gate_extra.sh` -- and
the SPRT.

**S170's budgets, re-swept on this tree (DEC-156), not adopted.**
`adocs/data/S203_case_sweep.sh`, the full grid on a copy of the candidate's
Release build, 07:25 to 07:45 (`s170_grid.txt`). The rule, the script's own
(DEC-162): the cheapest cell at the row's stride that reports a mate line. At
the budgets as they stand -- S114's, `ad18661` -- `test_mate_carry` is green
with its majority at exactly three of five, A (300000) and D (1200000)
reporting no mate line; the rule would move five: A 300000 -> 500000, C
1200000 -> 1000000, D 1200000 -> 1000000, E 300000 -> 100000, F 100000 ->
500000, B staying at 100000. `--ceilings` over the four recorded grids reads 5,
15, 0, 2, 11, 5; with this grid added C reads 1 (its 4000000 cell, one short
line); this grid alone 0, 7, 1, 0, 3, 3 (`s170_ceilings.txt`). **Not adopted
here**: S114 verdict 1 read H0, so this tree does not land; the budgets are
re-derived by the same script on the tree S115 is rebased onto, and land as
their own commit after the landing, the S114 precedent.

**The pre-registration**, `adocs/data/S115_sprt.sh`, in `S114_sprt.sh`'s
shape: the candidate and the refusal (DEC-245) named; the seeds in their forms;
both sweeps; the prior as a bundle's record and the pull alone as unmeasured,
direction only; DEC-063's expectation "small, of either sign" with the gainer
pair; the off value, the counts, the fixed-node depths as reach and the mate
instruments; **the warning that this is S114 verdict 1's tree**, whose SPRT has
since read H0; `{0, 5}` nElo at DEC-143's 41861 / 25591 games, 19.8 h / 12.1 h
at 2110 an hour; the regime, `OUT` under `.tuning/`, the abort rule read per
side; the open findings carried from S114's block by id, with item 18 (S114 v1
on this tree), 19 (this verdict's goldens) and 20 (S132's docstring offsets)
new; and the three readings -- H1 keeps pull 2 and the ratio for S127; H0 below
zero returns the pull to 0 and its code leaves, **the ratio, the pure function
and the record staying with the reason stated** (the ratio is the schedule's
axis, unswept until this step and S127's to fit, and the function and the
record are what hold the loop to a test at all); no verdict or an interval
reaching above zero a zero read the same way. `REF` and `CAND` `PIN_ME` with
the refusal; "CAND = the landing commit's sha, REF = CAND^".

## Proposed `specs.md` sentence (the coordinator edits specs.md)

In the search row, the aspiration sentence becomes: Aspiration windows search
the root of each iteration from depth 2 in a band **21 centipawns** either
side of the previous iteration's score; a failure moves the failing bound past
the returned score by the band's current half-width and widens the band by
`AspirationWidenPct` (200, the doubling S021 shipped), going to the full window
past 437 or on a mate score; **since S115 a fail-low first brings beta down
toward alpha by `AspirationFailLowPull` (2) quarters of the window**, never on
an infinite bound, and every re-search runs at the iteration's own depth -- the
published root fail-high reduction was built and refused on the fast suite's
mate guards (DEC-245). S085 retuned the triple from the 5 / 50 / 400 S021
shipped, and the values here are `AspirationMinDepth`, `AspirationDelta`,
`AspirationMaxDelta`, `AspirationWidenPct` and `AspirationFailLowPull` as they
compile today; `AspirationFailLowPull` 0 is the tree before S115, node for node
(DEC-215).

## Proposed commit (not verdict-closing)

```
Pull beta toward alpha on a root fail-low (S115)

When a root search fails low, aspiration_after_fail now brings beta
down toward alpha by AspirationFailLowPull quarters of the window (2, a
half: the range midpoint, DEC-134 form c) before alpha is pushed below
the returned score, never on an infinite bound; the widening is the
parameter AspirationWidenPct, 200 being the doubling S021 shipped, which
a sweep over S021's 300 positions with the pull on did not move
(DEC-244). Every re-search still runs at the iteration's own depth: the
published root fail-high reduction was built beside it and refused on
three mate guards of the fast suite (DEC-245); its code and variants are
adocs/data/S115_reduction_*.diff. Implemented from the step file's
description (DEC-221).

AspirationFailLowPull 0 is the tree before S115 node for node
(DEC-215): the tune build there benches the parent's 4192793 with all
eight replies and the whole stream, and reproduces search_bench at
depths 9 and 12. The pull: bench 4192793 -> 4054253 (-3.3 %), the
tactical position's reply d7c8q -> d7c8r; fixed-node depths 48 -> 46
over the three positions, stated as reach (DEC-239); +2.5 % nodes at
depth 11 pooled over the 300. The counts move, so INV-6 does not
discharge it: adocs/data/S115_sprt.sh measures it at {0, 5} nElo
against this commit's parent.

The loop's window update is a pure function the tests hold directly,
and every root search is recorded so a test can see the loop use it;
five cases, AW01 to AW04 killed 4 of 4. The S074 case, the mates in two
and the mined set's floor stay green unchanged.

Bench: 4054253
```

**What a reviewer should read first**: `aspiration_after_fail` in
`src/chesso.cpp` (the guard, the 64-bit width, the centipawn floor on the
widening) against the loop's old body in the diff; the loop case "the loop
re-searches a failed root with the window the schedule gives"; the stop
section's table, which is DEC-245's record.

## Rebased onto the tree without the static-score term, 2026-09-30

S114 verdict 1 read H0 (`22e00c3`), its term left as `f3868fb` and the S170
budgets were re-derived as `f5eaa99`. This section is the step on that tree:
the worktree `chesso-s115b`, base `f5eaa99`, the fast-checked patch applied
three-way. **Every number here is this tree's and replaces the one the
sections above took on `cd50c7a`**; those sections stay as the record of the
first build. Two agents did it: the first was stopped mid-task with no report,
the second finished from its worktree and logs. Logs are under
`.tuning/coord/S115b_logs/` in the main checkout. Nothing is committed.

**Deviations and findings, first.**

1. **The brief names S248 as an open filler; it is done** (`3f9ffa5`, which
   sits above this worktree's base and moves documents only). The
   pre-registration says S247 and S249 are the open fillers and S248 is done.
2. **Half of S249 is not stale on this tree.** S249 was written on the first
   tree's readings: the mined set's comment 145 against 147 read, and
   `MATE_IN_THREE_FLOOR`'s comment 12 against 13 read. On `f5eaa99` and on the
   candidate the mined set reads **146** (still not the comment's 145) and the
   mates in three read **12 of 24**, what the comment says. S249's accepts
   quote `cd50c7a`'s figures; the coordinator's to amend, not touched here.
3. **The ratio sentence in `src/search_params.hpp` was re-read and not
   rewritten**: "150 led 200 on two of the three samples and 300 on one" is
   this tree's sweep as well as the first's, word for word, so no source file
   changed after the fast check and the binaries are the ones the patch
   builds. `MANUAL.md`'s row is reworded (fast-check fix 2).
4. **The mutation fixture is a commit object on no branch**, `13bea70`, made
   through a temporary index so the branch and the index were not touched --
   the tool refuses anything but a clean linked worktree, and the first build
   did the same (`d9c9eda`). Its worktree is removed; the object is
   unreachable and left for `git gc`.
5. **`--ceilings` with this tree's S170 grid added reads C 6 where the test
   holds 0** (C's 4000000 cell, 15 lines, 6 short). Not acted on, by the S245
   and S114 precedent that leaves re-sweep grids out of that command; the new
   budget's own cell, C at 1000000, has 0 short. A finding for whoever next
   owns the ceilings.
6. **The new budgets cost `test_mate_carry` about 12 s**: 22 s and 23 s
   (Release, tune) at the removal's budgets, inside the suite, against 34 s
   and 35 s at the re-derived ones, run alone; C and D are the rows whose
   budgets grew.
7. The first table of the ratio sweep on `cd50c7a` is kept under a new name,
   `S115_aspiration_sweep_pull_d11_cd50c7a.tsv`, and
   `S115_aspiration_sweep_pull_d11.tsv` is this tree's; the brief allowed
   either name.
8. **Not run, by the brief**: a match of any kind, so DEC-141's Debug
   self-play is not done; `tools/gate_extra.sh`; the SPRT. No Stockfish or
   other chess oracle was needed: no position was assessed.

**The conflicts (item 1).** `git apply -3` left three files unmerged:
`tests/test_search_params.cpp`, `DEV_MANUAL.md`, `adocs/data/README.md`. After
resolution the diff of `src/chesso.cpp`, `src/search_params.hpp`, `src/uci.hpp`,
`tests/test_engine.cpp`, `tools/mutants/S115_aspiration_pull.py` against
`f5eaa99` is line for line the fast-checked patch's against `cd50c7a` (142, 48,
40, 361 and 88 changed lines, compared mechanically): the pull, the two
parameters, the pure function and the trace, nothing else.
`tests/test_search_params.cpp` differs from the patch only where it must: the
history comment and the GOLDEN line say 70 to 72, and the two rows are
re-aligned to the narrower table.

**Goldens (item 4).** One moved, and it is the deliberate one:

| golden | old | new |
|---|---|---|
| `test_search_params.cpp` `golden_defaults` | 70 rows | **72**: `{"AspirationWidenPct", 200, 100, 400}` and `{"AspirationFailLowPull", 2, 0, 4}` after `AspirationMaxDelta`, equal to the X-macro's two rows |

No other golden went red in either build. Re-read by their own scripts on
**this tree's pull-only Release build** -- `build/src/chesso` copied aside as
`.ref-builds/bin/cand_release`, sha256 `e85baab1...` -- and on the parent's
(`parent_release`, `f5eaa99`; `binaries.sha256`), fast-check fix 3:

| instrument | parent `f5eaa99` | candidate | log |
|---|---|---|---|
| S074 case, first mate, positions 1 / 2 (`adocs/data/S188_repair_goldens.py first-mate`) | 9 / 9, mate in 5 from there on | **9 / 9**, the same | `s074_first_mate.log` |
| mates in two first reported at iteration 3, of 26 | 26 | **26** | `mate2.out` |
| mined set exact at depth 10, of 318 (floor 143) | 146 | **146** | `breadth.out` |
| constructed set: mates in three / four / five exact | 12 of 24 / 2 of 16 / 0 of 16 | the same | `mate_set_counts.out` |

On the first tree the mined set read 147 and 148 and the mates in three 13;
on this one the pull moves none of the four.

**The off value (item 2, DEC-215).** The tune build at `AspirationFailLowPull`
0, `AspirationWidenPct` at its 200, against a Release build of `f5eaa99` made
in a throwaway worktree, since removed: `bench` **3429473**, the whole
121-line stream identical with time and nps stripped; `bench 12` **1694808**,
its 105-line stream identical; `tools/search_bench.py` 48304 / 71580 / 25413
at 9 and 104784 / 244824 / 117798 at 12, best moves c3d5 e2a6 d7c8q at both --
the parent node for node. The tune build at its defaults prints the Release
candidate's bench stream line for line (`counts.out`).

**The counts, parent -> candidate (item 3).**

| instrument | parent (`f5eaa99`) | candidate |
|---|---|---|
| `bench` | 3429473 | **3513310** (+2.44 %) |
| `bench` replies | c3d5 e2a6 d7c8q g7h8q d8e7 a1b2 e5e6 e5e6 | the same eight |
| `bench 12` | 1694808 | 1619863 (-4.42 %), replies unchanged |
| `search_bench` 9 | 48304 / 71580 / 25413, c3d5 e2a6 d7c8q | 34236 / 71552 / 25351, same moves |
| `search_bench` 12 | 104784 / 244824 / 117798, c3d5 e2a6 d7c8q | 70283 / 240680 / 80264, same moves |
| fixed-node depth, `go nodes 1000000`, Hash 16 | 17 / 15 / 15 = 47 | 17 / 15 / 16 = **48**, no move changing |

Reach, not a forecast (DEC-239): the two bench depths move in opposite
directions and the fixed-node depth gains a ply on one position where the
first tree lost one on two. INV-6 does not discharge the change; the SPRT
does.

**The ratio re-swept (item 6).** `adocs/data/S115_aspiration_sweep.py --rows
pull` on a copy of this tree's tune build, depth 11, the off row at the
shipped triple re-read from `src/search_params.hpp` (2 / 21 / 437)
(`adocs/data/S115_aspiration_sweep_pull_d11.tsv`):

| row | pull | ratio | pooled nodes | rel | per sample (0 / 37 / 71) | best moves changed of 300 |
|---|---|---|---|---|---|---|
| off | 0 | 200 | 40114760 | 1.0000 | 1.0000 / 1.0000 / 1.0000 | 0 |
| pull | 2 | 200 | 40892425 | **1.0194** | 1.0144 / 1.0430 / 1.0104 | 33 |
| pull150 | 2 | 150 | 40098837 | 0.9996 | 1.0245 / 1.0024 / 0.9860 | 48 |
| pull300 | 2 | 300 | 40957623 | 1.0210 | 0.9905 / 1.0594 / 1.0174 | 45 |

DEC-244's reading, printed by the script: **no ratio leads 200 on every
sample, `AspirationWidenPct` stays 200.** 150 leads on samples 37 and 71 and
loses on 0; 300 leads on 0 alone. Pooled, 150 is below 200 by 1.9 % and level
with the off row, which is S127's input and re-decides nothing here. The pull
costs +1.94 % nodes pooled and more on every sample (the first tree: +2.50 %).

**Mutation (item 5, DEC-141 clause 2).** `tools/mutation_check.py tools/mutants
.ref-builds/mut --only AW01 AW02 AW03 AW04 --jobs 8`, the whole directory so
every anchor of its 160 validates, fixture `13bea70` clean: header `baseline
green, 41 tests, bench 3513310 nodes via engine`; **mutation score 4 of 4
(100 %)**, wall 569 s (`mutation.log`, `mutation_s115b/results.tsv`). AW01 at
"a fail-low pulls beta toward alpha and pushes alpha below the score" and at
the loop case (15 of its checks); AW02 at the same two (16) and at
`test_mate_carry`'s majority; AW03 at "the pull is never computed while
either bound is infinite", **bench signature unmoved**; AW04 at "each failure
widens the band by AspirationWidenPct and the schedule ends whichever way it
fails" and at `test_mate_carry`'s majority.

**S170's budgets (item 7, DEC-156 as amended by DEC-162), a separate patch.**
The rule, stated in `s170/run_sweep.sh` before the grid was read: each row
takes the cheapest budget at its own stride whose cell reports a mate line,
on the mate count alone. `adocs/data/S203_case_sweep.sh`, the full 108-cell
grid on a copy of the candidate's Release build (`s170/grid.txt`):

| row | the removal's budget, lines on this tree | re-derived | lines, short |
|---|---|---|---|
| A_mate8_shallow | 500000, 4 | 500000 | 4, 0 |
| B_mate6_shallow | 100000, 32 | 100000 | 32, 1 |
| C_mate7_depth11 | 500000, **0** | **1000000** | 11, 0 |
| D_mate_minus6_depth10 | 1200000, **0** | **3000000** | 4, 0 |
| E_mate_minus9 | 300000, 3 | 300000 | 3, 0 |
| F_mate6_inherited_no_line (stride 2, unguarded) | 100000, **0** | **500000** | 17, 0 |

Three of six move. At the removal's budgets the test is green with its
majority at exactly three of five (`s170/at_standing.txt`); at the new ones
`--at` reproduces the six cells (`s170/at_new.txt`) and `test_mate_carry` is
green in both builds. The change is
`.tuning/coord/S115_landing/s170_budgets.patch` -- the TSV's three rows and
its note, `adocs/data/S115_sweep_s170.txt`, its README row and
`DEV_MANUAL.md`'s list of re-sweeps -- applying on top of this working tree;
the TSV in the worktree is unchanged. No comment in `tests/test_mate_carry.cpp`
is made false by it. Ceilings: finding 5 (`s170/ceilings.txt`).

**Suites (item 8).** On the final tree: Release `build` 41 of 41 and tune
`build-tune` 41 of 41; `./clang-format.sh --check` clean with
`CLANG_FORMAT_MAJOR=22`; `tools/plan_prose_check.py` `--citations`,
`--touches` and `--params`, one mode per call, exit 0 each
(`final_suites.out`). Debug: `build-debug`'s whole `test_engine`, 80 of 80
cases, 870802 assertions, no `Assertion` line (`debug_test_engine.log`).

**The six fast-check fixes.** (1) 70 -> 72 in the test's history comment and
GOLDEN line, `DEV_MANUAL.md`'s `golden_defaults` row and the
pre-registration's item 19; the ledger entry, the README rows, the mutation
header's bench and the `Bench:` line are this tree's; the pre-registration's
reference paragraph and items 1, 3, 4, 16, 18 and 19 rewritten and no longer
contradicting each other. (2) The H0 row keeps a precondition on the loop
case -- a count of failures in place of the count of pulls -- and rewords the
kept `MANUAL.md` row; that row now says "with `AspirationFailLowPull` at 2".
(3) The three mate numbers re-taken on the pull-only Release build, named in
this section, the pre-registration and `DEV_MANUAL.md`'s `first_mate_depth`
row. (4) The Pitfalls bullet points at the seeds table and the goal line
carries an annotation. (5) `S115_aspiration_sweep.py` is 100755 in the index;
the README row for `S115_aspiration_sweep_d11.tsv` says an earlier revision of
the script wrote it. (6) The refusal's trace and census are
`adocs/data/S115_reduction_trace.txt` and `S115_reduction_census.txt`, copied
byte for byte, with README rows.

**The pre-registration (item 9).** Expected games, the regime, the abort rule,
the bounds and the H1 and no-verdict readings are the first build's, unchanged;
the H0 row changes only by fix 2.

**Which readings are whose.** The first agent built the three binaries and
took the counts, the three mate instruments, the constructed set's counts, the
ratio sweep and the S170 grid. **Reproduced by re-running on the same
binaries, byte-identical outputs but for nps**: the off-value identity, all
counts, the fixed-node depths, `s074_first_mate.log`, `mate2.out`,
`breadth.out`, `mate_set_counts.out` and the sweep table; the two build
directories' binaries hash to the copies the readings were taken on. **Taken on trust of its exit marker and
cross-checked, not re-run**: the 108-cell S170 grid (its binary's hash is the
candidate's; its nine cells at the old and new budgets were re-read by two
`--at` runs and agree). **Taken fresh**: the conflict review, mutation, both
suites, format and prose checks, the Debug `test_engine`, `--ceilings`, the
budgets patch and its test runs.

**The `specs.md` sentence** proposed above stands unchanged; it quotes no
count.

## Proposed commit on this tree (not verdict-closing)

```
Pull beta toward alpha on a root fail-low (S115)

When a root search fails low, aspiration_after_fail now brings beta
down toward alpha by AspirationFailLowPull quarters of the window (2, a
half: the range midpoint, DEC-134 form c) before alpha is pushed below
the returned score, never on an infinite bound; the widening is the
parameter AspirationWidenPct, 200 being the doubling S021 shipped, which
a sweep over S021's 300 positions with the pull on did not move
(DEC-244). Every re-search still runs at the iteration's own depth: the
published root fail-high reduction was built beside it and refused on
three mate guards of the fast suite (DEC-245); its code, variants, trace
and census are adocs/data/S115_reduction_*. Implemented from the step
file's description (DEC-221).

Built first on S114 verdict 1's tree and rebased onto the tree that
verdict's H0 leaves, every number re-taken there. AspirationFailLowPull
0 is the tree before S115 node for node (DEC-215): the tune build there
benches the parent's 3429473 with all eight replies and the whole
stream, and reproduces search_bench at depths 9 and 12. The pull: bench
3429473 -> 3513310 (+2.4 %), bench 12 1694808 -> 1619863, no reply
changing; fixed-node depths 47 -> 48 over the three positions, stated
as reach (DEC-239); +1.9 % nodes at depth 11 pooled over the 300. The
counts move, so INV-6 does not discharge it: adocs/data/S115_sprt.sh
measures it at {0, 5} nElo against this commit's parent.

The loop's window update is a pure function the tests hold directly,
and every root search is recorded so a test can see the loop use it;
five cases, AW01 to AW04 killed 4 of 4. golden_defaults goes 70 -> 72.
The S074 case, the mates in two and the mined set's floor stay green
unchanged, read 9 and 9, 26 of 26 and 146 as on the parent. The S170
budgets move on this tree and land as their own commit.

Bench: 3513310
```

### Landed and pinned (the coordinator, 2026-09-30)

Landed as `eb334e3` on `3f9ffa5` from the rebase worktree's working tree, two cold fast checks first (on `cd50c7a` and on this tree). `bench` 3429473 -> 3513310; the off value re-proved on the landed tree before pinning: the tune build at `AspirationFailLowPull` 0 benches 3429473 with the parent's whole 121-line stream (`.tuning/coord/S115_landing/offvalue_on_landing_bench.txt`), and `3f9ffa5`'s `src/` tree is `f5eaa99`'s, object for object. The S170 budgets re-derived on this tree as `001aae4` (DEC-162). At the landing `specs.md`'s aspiration sentence was rewritten and `tools/plan_prose_check.py`'s `AspirationMaxDelta` phrase re-anchored to it (`test_plan_params` red on the stale phrase first). **Second tier on the landing** (DEC-141): Debug self-play of 8 games at 4+0.04 on `001aae4`, 0 `Assertion`, 0 `disconnect` (`.tuning/coord/S115_debug_selfplay/`); `tools/gate_extra.sh` 5 stages green in 1027 s (`.tuning/gate_extra_2026-09-30_S115.log`). SPRT pair pinned: `REF` `3f9ffa5` (the commit the landing sits on), `CAND` `eb334e3`; open findings re-read at pinning: as the pre-registration states them. S114 verdict 1 read H0 and its term left first (`f3868fb`); S248 completed (`3f9ffa5`); owner question 3 (DEC-245) open.
