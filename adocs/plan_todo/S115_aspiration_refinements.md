id:         S115
goal:       the widening schedule is re-swept fail-soft, a fail-low halves beta toward alpha, and a repeated fail-high costs the root a ply
accepts:    an SPRT verdict, recorded whatever it is; the changes are measured together only if a sweep shows them inert apart, and otherwise separately -- DEC-082 is a condition to be met, not a convenience; the node-count sweep is run over the 300 stratified positions adocs/data/S021_aspiration_sweep.py picks out of adocs/data/S018_raw.tsv -- PER_PHASE 4 over the 25 phase values that file carries is 100 positions, at each of the script's three offsets -- and not over three, because S021 recorded that one sample chooses the wrong setting; the mate-appears-mid-search case S074 put in the gate still passes
touches:    src/chesso.cpp iterative_deepening_search, src/search_params.hpp
excludes:   a window width seeded from the score's own volatility -- dropped by DEC-087, no evidence at this band; the initial delta, which S127 fits
decisions:  DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## What is there

`ASPIRATION_DELTA` is 50 and widening doubles the failing side alone. What the
band's engines do that this does not: keep the search fail-soft through the
window plumbing (fail-soft in the pruning returns measured +2.6/+5.1 at
Ethereal), halve beta toward alpha on a fail-low (one line), and reduce the
root depth on a repeated fail-high so an unstable root does not burn a whole
iteration (Lynx carries it). The volatility-seeded width the strong engines
run has no measured gain below ~3100 and left this step at DEC-087; S085 and
S127 own the delta itself.

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
  Chesso's gate of 5 was measured here (S021) and stands.
- **The dropped branch, confirmed dropped.** Eval-scaled width exists at the
  top (SF 0150da5c2b, 2019); Weiss *removed* its extreme-score adjustment as a
  passed simplification, -0.72 +/- 1.13 (#668, 2023) -- band-level support for
  DEC-087's exclusion.

### Shape for chesso -- today's loop

The plumbing is **already fail-soft end to end**: negamax returns best_so_far
(src/search.cpp:1103), RFP returns `static_score - margin`
(src/search.cpp:771), null move returns `null_score` (src/search.cpp:839), TT
cutoffs return the stored score, not the bound (src/search.cpp:236-249),
quiescence returns best_value (src/search.cpp:580). Ethereal's +2.60/+5.07 for
fail-soft pruning returns (a0d84b633e) is already banked here.

The loop (src/chesso.cpp): init `aspiration_score +/- ASPIRATION_DELTA` from
depth `ASPIRATION_MIN_DEPTH` (src/chesso.cpp:765-772); re-search while the
score is a bound (src/chesso.cpp:776-795) -- fail-low `alpha = max(score -
delta, -SEARCH_SCORE_INF)` (src/chesso.cpp:787), fail-high `beta = min(score +
delta, SEARCH_SCORE_INF)` (src/chesso.cpp:789), so the failing side is already
re-centered on the returned score; `delta += delta` (src/chesso.cpp:792);
re-search at the **same depth** (src/chesso.cpp:794); mate or `delta >
ASPIRATION_MAX_DELTA` jumps to the full window (src/chesso.cpp:783-785). The
resolved score becomes the next centre and a mate disarms the window
(src/chesso.cpp:831-842). `last_aspiration_failures` is test-only
(src/chesso.cpp:60-64, src/chesso.cpp:88-89). **Missing: the opposite bound is
never touched, and the root is never reduced.** The changes: (a) fail-low
additionally sets `beta = (alpha + beta) / 2` before pushing alpha; (b) a
consecutive-fail-high counter makes the re-search run at `max(1, current_depth
- count)`, reset on fail-low; (c) the widening schedule is re-swept under
(a)+(b).

### Implementation sketch

1. (a) then (b), each a few lines in iterative_deepening_search; keep the
   existing clamps and the mate/max escape around both.
2. Re-sweep with adocs/data/S021_aspiration_sweep.py (tune build, depth 11,
   the three stratified offsets -- 300 positions, never the three-position
   bench), with (a) and (b) toggled apart and together, multiplier variants
   x1.5/x2/x3 in the same table.
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
     move -- assert bestmove/PV agree (search.cpp:1074-1096 is the recorded
     failure shape);
   - loop termination: bounded number of fails to the full window, with (a)
     active.

### Constants and seeds

| quantity | today | seed (prose source) -- fit here |
|---|---|---|
| midpoint-pull weight | none | 1/2 of the interval (SF prose in 57b32f3e60); SF 2025 runs 3/4 toward alpha -- seed 1/2 |
| fail-high reduction | none | 1 ply per consecutive root fail-high, floor depth 1, reset on fail-low (SF 3a572ffb48); uncapped -- Lynx caps measured negative |
| widening multiplier | x2 (src/chesso.cpp:792) | x2 "exponential" (CPW); linear +delta/fail (SF 49dfc50b12, 2010); "reduce the rate" (Weiss #183) -- sweep x1.5/x2/x3 |
| depth gate | 5, measured (S021) | Weiss >6, Althoff/Buijs 4 -- keep 5 unless the sweep says otherwise |
| initial delta | 50 (S021) | excluded: S085/S127 own it (for the record: 50 cp Althoff, 15 cp Buijs, "21 internal units" SF 2019 prose) |
| max delta escape | 400 then full | keep; re-sweep confirms |

### Pitfalls

- **Bound arithmetic near infinity.** SEARCH_SCORE_INF is 2000000000
  (src/search.hpp:8): never compute `(alpha+beta)/2` while either bound is
  infinite -- armed bounds are finite, but the full-window escape must bypass
  the pull. Lynx #1275 ("windows outside [MinEval, MaxEval] after overflow")
  is the published instance of getting this wrong.
- - **Mate-band windows.** MATE_MAX 49000 (search.cpp:16). Keep the existing
  guards -- mate ends the schedule (src/chesso.cpp:783) and disarms the next
  window (src/chesso.cpp:842), the published form (SF 1f73a9ed63: mate scores
  made "aspiration blow up in a series of researches loops"; 8acb1d7e4d) -- and
  do not reduce the root when the fail-high score is a mate: SF's
  opposite-bound patch was reverted for a near-mate bug (fc54d87301), and Lynx
  #2560 added mate-range guards after false mate reports "specially after being
  saved in TT".
- **Fail-soft scores as re-centres.** SF 57b32f3e60's caution verbatim
  applies: the fail-low score is an untrusted upper bound (at this root it is
  the max over null-window children), so `beta = alpha` overshoots -- keep a
  buffer, which is what the 1/2 weight is.
- **Reduction reaching depth 0.** Berserk shipped "Fix aspiration window from
  entering QSearch" (0645ca1079) -- floor the adjusted depth at 1.
- **Alternation loops.** The pull shrinks beta, making fail-low -> fail-high
  cycles likelier (Madsen's pathological cycles, t=76115). The delta escape
  must still bound the loop, and a fail-low resets the fail-high counter.
- **TT on re-searches.** A re-search at the same or reduced depth reads
  entries the failed attempt just wrote; the root never takes a TT cutoff
  (is_pv) but everything below does. Mate scores written during a fail are
  the recorded hazard (8acb1d7e4d, Lynx #2560). tt_new_search runs once per
  go (chesso.cpp:678) -- do not age per re-search.
- **Root ordering across fails.** A root fail-high publishes the cutoff move
  and a one-move PV (search.cpp:1074-1096, S021's bug fix); the reduced
  re-search must find that move first via the TT root entry -- assert, not
  assume.
- **Time management.** S089's scaler reads completed in-window iterations
  only (chesso.cpp:847-879); fail events feed nothing. Keep it that way:
  Lynx measured soft-limit checks inside the window loop at -7.4 to -94.5
  Elo, all rejected (#2212-#2214). A fail-low near the soft limit is stopped
  by the hard timer alone, today and after this step.

### Measurement

S105 regime (8+0.08, Hash 16, UHO book), `elo0=0 elo1=5`. Band evidence:
+6.78/+8.19 (Weiss, the full bundle), +2.20/+4.82 (Lynx, reduction alone) --
the expected effect sits near elo1, so state the expectation and each
outcome's reading before launching (DEC-063). Verdict count per the accepts:
1 if the sweep shows the parts inert apart, else up to 3. Gate: fast suite
and the S074 mate cases green at the shipping schedule; the sweep over the
300 stratified positions.

### Interactions

- **S089 (done).** Verified: the budget does *not* react to fails -- its
  inputs are best-move stability and the completed iteration's score drop
  (chesso.cpp:847-879). This step adds no mid-loop time checks (Lynx
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
not: every return path is already fail-soft (search.cpp:1103,
src/search.cpp:771, src/search.cpp:839, src/search.cpp:236-249,
src/search.cpp:580) and the failing bound has re-centered on the returned score
since S021 (chesso.cpp:787, src/chesso.cpp:789) -- Ethereal's +2.6/+5.1
rewarded fail-soft *pruning returns*, which RFP and null move here already do.
What remains of the goal's first clause is the re-sweep itself; the new
behaviour is the midpoint pull and the root reduction. The goal's direction is
the published one -- fail-low pulls **beta** toward alpha, `(alpha+beta)/2` --
confirmed by SF prose (57b32f3e60) and Weiss #183 ("lower beta when resolving
fail lows"). No change to goal or accepts is needed.

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
