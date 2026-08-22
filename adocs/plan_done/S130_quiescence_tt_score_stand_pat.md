id:         S130
goal:       quiescence takes the table score as its stand-pat where the stored bound allows it, instead of the static evaluation alone
accepts:    an SPRT verdict, recorded whatever it is; the table score is used only where its bound makes it valid -- a lower bound may only raise the stand-pat, an upper bound may only lower it, an exact score replaces it -- with a unit test per bound type; a score in the mate band is never used as a stand-pat, tested, because a stand-pat must not become a mate claim; the entry's eval field keeps its S094 semantics -- what is stored there is still the static score and never the improved stand-pat, for the same reason a bound is never stored (an improved stand-pat is window-relative through its bound test); the fast suite green
touches:    src/search.cpp quiescence, tests/test_search.cpp, adocs/data/S130_sprt.sh, adocs/data/S130_sprt.log, adocs/specs.md
excludes:   per-move futility, which is S112; delta pruning, which is S022; ~~any change to what is stored~~ **amended by the orchestrator, 2026-08-22, DEC-102: the store's node *type* is in scope, because a step that consumes a table bound must also correct what it propagates -- the original excludes made this step unsatisfiable as written.** What is still excluded is the stored *score* and the entry's `eval` field, both unchanged
decisions:  DEC-019, DEC-063, DEC-071, DEC-087, DEC-102, DEC-103, INV-6
closes:
blocks:
paused_by:
done:      2026-08-22: substitution and degraded store landed; per-bound unit tests written red-first (three stages, two perturbations recorded in this file); fast suite 19/19 in build and build-tune, clang-format and plan_prose_check green.
              SPRT vs 293a45b at elo0=0 elo1=5: NO VERDICT over 16784 games in 7h12m — Elo 1.14 +/- 4.04, nElo 1.48 +/- 5.26, LLR -0.71 in (-2.94, 2.94), 0 forfeits; the pre-registered no-verdict clause fired at twice its 8000-game number. Recorded as zero and KEPT, DEC-103 (agent-proposed under DEC-041); no two-sided re-run, pre-registered. Run 1 killed at 103 games for measuring the unsound exact-store; fix and 69 percent laundered-store count under DEC-102. adocs/data/S130_sprt.sh, S130_sprt.log and S130_sprt_run1.log tracked.
              specs.md absent-machinery row carries the layer, its numbers and the degraded store in the same commit. README checked, owner-written, no change. MANUAL checked: one line corrected — MaxQsearchDepth cap returns the stand-pat score, no longer described as the static score. DEV_MANUAL checked, no change owed. No watcher armed.

## Created by the second review, DEC-087

S094 put the probe in quiescence and measured the probe alone at zero; S103
found the stored evaluation's consumer in the main search. This is the
consumer inside quiescence itself, and it is the one with the band's best
numbers: Weiss measured "use ttScore instead of static eval when the bound
permits" at **+10.8 / +12.1 / +21.4** across its runs (cca90ea7), and
Ethereal's equivalent probe-and-use family at +11.0/+3.1/+2.6 (25e56feb).

The probe is already paid for at the top of `quiescence()` and the bound
logic already exists in `tt_entry_answers()` -- what is missing is three
lines between them: where the entry did not answer the node outright, its
score is still a bound on this position, and a stand-pat tightened by a valid
bound prunes more and prunes honestly. `de_normalize_score()` applies on the
way out, as everywhere.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

Two published layers, distinct in every record that separates them:

- **Layer (b), landed here at S094**: the entry's *static eval field* replaces
  a recomputation of `evaluate()`. Published as "save static eval in TT".
  Chesso reads it since S094 (measured zero); S103 is its main-search consumer.
- **Layer (a), this step**: the entry's *search-backed score* substitutes for
  the static number where the stored bound certifies the direction. Weiss
  cca90ea7 states the rule and the rationale in one line: "A score gotten from
  the TT will almost certainly be a better evaluation of the position than the
  static eval. If the score+bounds suggest a better evaluation then use that
  instead for pruning heuristics."

The bound-conditioned rule itself (derived from CPW's bound definitions; the
derivation is arithmetic, not chess judgement): a **lower bound** (fail-high
entry) certifies `value >= s`, so `s` may only *raise* the stand-pat (take the
max); an **upper bound** (fail-low entry) certifies `value <= s`, so `s` may
only *lower* it (take the min); an **exact** score *is* the value and replaces
it. Any other use consumes a claim the entry never made. Under fail-soft the
substituted value flows into the returned/stored best value, which stays sound
because the certified direction is exactly the direction fail-soft propagates.

Traced records (URLs in section 8):

- **Weiss cca90ea7, 2020-09-08, "Use score from TT instead of static eval if
  possible (#336)"**: +10.78 +/-6.91 at 10+0.1 Hash=32; +12.09 +/-6.83 at
  60+0.6 Hash=128; +21.44 +/-9.52 at 20+0.2 8 threads. This is the plan's
  +10.8/+12.1/+21.4, confirmed verbatim. **The message scopes it to "pruning
  heuristics", not to quiescence** -- which sites it covered is not
  establishable from the record without reading the diff, which DEC-016
  forbids. Weiss sat in the ~3050 CCRL band then.
- **Ethereal 25e56feb, 2019-01-13, "Probe the TT during qsearch for cutoffs
  and an eval"**: +11.04/+3.12/+2.55 -- probe and use landed as one patch, so
  it prices S094's ground plus this step's together.
- **Lynx PR #1319, 2025-01-18, "QSearch: reuse TT score instead of TT static
  eval in QSearch when possible"** -- the exact increment this step is (score
  over eval-field, quiescence only, on top of an existing probe and eval
  reuse): first attempt **-1.70 +/-3.02** over 22236 games at 8+0.08; revised
  version **-0.01 +/-1.85** over 60152 games STC and **+1.07 +/-2.45** at
  40+0.4. Merged at ~0, in an engine far above 3000 by then.
- Lynx's wider family shows the technique is per-site and churns: #692 "Use TT
  score as positional eval for pruning" (2024-03), #1084 "reuse TT scores over
  real static eval" (2024-10), #1971 removes the lot (2025-08), #1973 and
  #2055 re-add it for RFP and NMP alone (2025-08/09).
- talkchess t=47373: +25 over 4000 games for storing+using the TT in qsearch
  in one engine -- probe and use combined; chesso's probe alone was zero.

CPW is silent on this idiom: the Quiescence Search page gives stand pat as the
lower bound and its beta cutoff, the Transposition Table page gives the three
bound/cutoff conditions, and neither describes score-as-stand-pat. The record
for this technique is engine commit logs, not the wiki.

### Scope concern

The step's premise calls this "the one with the band's best numbers" on Weiss
+10.8/+12.1. Traced: those numbers are real, but the commit message does not
scope them to quiescence -- "pruning heuristics" plausibly includes the
main-search eval sites (S108/S109 territory here). The nearest exactly-shaped
record, Lynx #1319 (qsearch-only, on top of an existing eval-field read --
chesso's precise situation post-S094), measured **~0**. Goal and accepts are
unchanged -- the change is ~6 lines, the accepts already say "recorded
whatever it is", and DEC-019 says the figure decides what to try -- but the
expectation should be a verdict anywhere from 0 to +10, and part of the Weiss
gain likely belongs to S108's consumer, not this one.

### 2. Shape for chesso

Line numbers at `cf89e22`; re-locate by symbol if drifted.

- Probe already paid: `tt_get_entry` at src/search.cpp:226, outright answer
  via `tt_entry_answers` at :221-223. `tt_entry_answers` (:155-188):
  de-normalisation *before* comparison (:170), EXACT answers unconditionally
  (:172-175), `TT_BETA_NODE && s >= beta` (:177), `TT_ALPHA_NODE && s <=
  alpha` (:182).
- **Therefore only two entry kinds reach the stand-pat site**: a lower bound
  with `s < beta`, and an upper bound with `s > alpha`. An EXACT entry never
  reaches it -- the accepts' "exact replaces it" is discharged by the existing
  :172 cutoff, and its unit test asserts the returned value without pinning
  which path serves it (so it survives any future PV-guard on the probe).
- Two structural consequences, worth knowing before measuring: a LOWER raise
  can never itself trip the :268 `stand_pat >= beta` return (`s < beta` on
  this path) -- its effect is the alpha raise at :279 (tighter child windows)
  and the fail-soft floor `best_value` at :332. An UPPER cap *can* suppress a
  static stand-pat cutoff at :268 (static >= beta, s < beta): the node then
  searches captures the static guess would have skipped -- pruning less there,
  honestly.
- Stand-pat today: :241-247 -- `tt_entry->eval` where present (layer (b)),
  else `evaluate_lazy()`; `stored_eval` fixed at :253 from the pre-substitution
  value. `stand_pat` is `const` (:244) and stops being so, or the substitution
  writes a successor variable.
- Entry fields (src/data_structures.hpp:388-414): `int32_t score`
  (search-backed, **ply-normalised on store** -- all four quiescence stores go
  through `normalize_score()`), `int16_t eval` (static only, never a bound or
  mate; assert at src/transposition_table.cpp:134), `uint8_t type`.
  `TT_DEPTH_QS = -1` (src/transposition_table.hpp:12).
- **The score field is NOT ply-adjusted at rest**: the consumer must read it
  through `de_normalize_score(entry->score, ply)` (:139-144) exactly as :170
  does. S106 -- ordered before this step, not yet done -- is what red-tests
  that round trip in both searches; do not start this step ahead of it. After
  the mate-band exclusion below, `de_normalize_score` is an identity (:122-124
  touch only band scores), so calling it costs nothing and keeps "no raw table
  score is ever consumed" uniform.
- Mate band: `MATE_MAX` 49000, `MATE_MIN` 48000 (src/search.cpp:16-17); band
  width 1000 > 2 x `MAX_PLY` 128 (src/data_structures.hpp:42), so a mate score
  sits in the band both raw and de-normalised -- `|entry->score| > MATE_MIN`
  on the raw score is ply-independent and equivalent.
- What changes: ~6 lines between :253 (after `stored_eval` is fixed) and :268
  (before the stand-pat cutoff), plus tests. Nothing stored changes
  (`excludes:`).

### 3. Implementation sketch

One commit, one SPRT; tests first within it.

1. **Unit tests, one per bound case, non-vacuous by construction.** The
   :554-568 `quiesce()` helper resets the table before the call, so these
   inline its body: `load_FEN`, `tt_reset`, `tt_store_entry(&tt, &game.board,
   TT_DEPTH_QS, <score>, <type>, 0, TT_EVAL_NONE)`, then call `quiescence()`
   at ply 0. The quiet captureless FEN already pinned in the suite
   (`4k3/8/8/8/8/8/8/3RK3 w`, `evaluate()` == 563 at :583-594) serves all
   cases; at ply 0 normalise/de-normalise are identities for non-mate scores.
   Each case first asserts the empty-table result (563) so the entry provably
   changes behaviour:
   - LOWER raises: `TT_BETA_NODE` 600, window (-10000, 10000) -- 600 < beta so
     no outright answer; expect 600 (pre-change: 563).
   - LOWER never lowers: `TT_BETA_NODE` 500 -- expect 563.
   - UPPER caps: `TT_ALPHA_NODE` 500, window (0, 10000) -- 500 > alpha so no
     outright answer; expect 500 (pre-change: 563).
   - UPPER never raises: `TT_ALPHA_NODE` 600 -- expect 563. This is the S106
     inversion asserted from the consumer side.
   - EXACT substitutes: `TT_PV_NODE` 600 -- expect 600 (today served by the
     :172 cutoff; the assertion holds whichever path answers).
   - Mate band never: `TT_BETA_NODE` at `MATE_MAX - 5`, window
     (-100000, 100000) so it does not answer outright -- expect 563 and a
     returned score outside the band. Non-vacuous against the *naive*
     implementation: without the band exclusion this returns 48995, so the
     test is red against exactly the bug it guards. Mirror with
     `TT_ALPHA_NODE` at `-(MATE_MAX - 5)`.
   - eval-field semantics (accepts): after a run whose stand-pat was raised,
     re-read the entry -- `eval` is still `TT_EVAL_NONE`/the static score,
     never the raised value.
2. **The change**, between :253 and :262: where `tt_entry != nullptr` and
   `|entry->score| <= MATE_MIN`, let `s = de_normalize_score(entry->score,
   ply)`; then `TT_BETA_NODE && s > stand_pat` or `TT_ALPHA_NODE && s <
   stand_pat` or `TT_PV_NODE` substitutes `s` (the last arm unreachable under
   today's probe -- keep or drop with a comment, the tests hold either way).
   `stored_eval` stays computed from the pre-substitution value.
3. Fast suite plus the whole-search mate cases green (pruning that hides a
   mate is the recurring bug), then the SPRT.

### 4. Constants and seeds

None. The rule is semantic -- bound directions, not margins -- so there is
nothing to fit and no seed to declare (DEC-084 trivially satisfied). A "trust
the entry only within N of the window" refinement exists in later engines; if
ever wanted it is a different step and needs its own seed source then.

### 5. Pitfalls

- **The inversion**: an UPPER-bounded score raising the stand-pat (or LOWER
  capping it) is the bound-sign failure class S106 exists for -- suites and
  node counts stay green, only strength leaks. The per-bound unit tests above
  guard the consumer side; S106 guards `tt_entry_answers` itself.
- **Mate contamination**: a raised stand-pat in the mate band would be stored
  back by the :273 stand-pat store or returned at the :287 qply-cap as a
  positional score claiming mate. The band exclusion is load-bearing; it is
  tested, and the accepts name it.
- **`stored_eval` ordering**: the substitution must land after :253 or use a
  separate variable -- otherwise the improved stand-pat leaks into the entry's
  eval field, which the accepts forbid (S094 semantics: static only; the
  improved value is window-relative through its bound test's reachability).
- **The final store's exactness claim (watched, out of scope)**: :387-390
  stores `TT_PV_NODE` when `best_value > alpha0`. A substituted stand-pat that
  lands in (alpha0, beta) with no capture beating it is stored EXACT while
  only a one-sided bound is known -- for a raise it even replaces the entry
  that certified `>= s` with a weaker claim. The published qsearch shape
  sidesteps this by never storing exact in qsearch; chesso's PV-store predates
  this step and `excludes:` forbids touching it here. If the SPRT fails, this
  corner is the first suspect, and a store-side guard is its own step.
- **Lazy-eval clamp**: the TT score is a search value built from leaf
  `evaluate()` calls that each carry the +/-150 clamp on mobility+king safety
  -- no single evaluation bypasses the clamp; the substitution replaces one
  clamped static guess with a search-backed bound over many clamped leaves.
  That is the point, and it is intended. Separately: when the lazy shortcut
  fired, `stand_pat` is itself only a window-side bound
  (src/evaluation.hpp:20-46); max/min against a table bound tightens on the
  same side, so the :268/:279 decisions stay sound, and the :287 qply-cap
  return hands the composite upward no less validly than today's.
- **In-check placement**: `stand_pat` is computed before `in_check` (:244 vs
  :262) and is unused for pruning in check (`best_value = MIN` at :332), but
  it *is* returned at :259/:260/:287. Substituting unguarded before :262 is
  the minimal diff and harmless-to-better on those returns; a `!in_check`
  guard is the literature-faithful alternative. Either way, before :268.

### 6. Measurement

Not behaviour-neutral -- it changes which nodes stand pat and every window
below them -- so no node-identity claim; one SPRT decides it (INV-6). S105
regime: 8+0.08, Hash=16, UHO book, gainer bounds `elo0=0 elo1=5` (S105 lands
well before this step in plan order; if somehow it has not, run current
`fastchess.sh` defaults and say so in the verdict). Fast suite and mate cases
green first. Record the verdict whatever it is -- S094's two zeros are the
local precedent. Cheap and optional before booking the match: count over one
depth-12 kiwipete search how often each arm fires (S094 measured the probe
finding any entry on 0.79 % of quiescence nodes on that workload -- main-search
entries, which the qs probe accepts, are the likelier suppliers). A near-zero
fire rate predicts the zero before it is bought; DEC-079 is the precedent for
buying it anyway or amending.

### 7. Interactions

- **S094 (done)** supplies everything consumed: the probe (:216), the eval
  field, the layer-(b) stand-pat read (:241-247) this step tightens, and the
  `de_normalize_score` endpoint fix this step's mate-band test leans on.
- **S106 (ordered before, not yet done)** red-tests the bound conditions and
  the mate round trip this step consumes. Keep the order: a swapped condition
  here is stand-pat-on-the-wrong-side-bound, invisible to everything but an
  SPRT.
- **S108 (ordered after)** widens `eval`-field writers to every non-check
  main-search node in the same shared entry format and fixes the
  TT_EVAL_NONE-overwrite bug its file names -- both raise layer (b)'s hit
  rate, not this layer's. The main-search version of *this* substitution
  (eval-for-pruning-margins = tt score where bound permits, Weiss's likely
  actual site, Lynx #1973/#2055's per-site record) belongs to S108/S109's
  territory, not here.
- **S112/S022 (after)**: quiescence per-move futility and delta pruning take
  the stand-pat as their base, so landing S130 first means their margins are
  measured against the tightened base -- the right order, and one more reason
  each owes its own verdict.
- **S131** also edits quiescence's move handling; one change at a time,
  whatever the commit order.

### 8. References

Every URL read for this section; GitHub API URLs return commit messages only,
no diffs (DEC-016 observed).

- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+hash:cca90ea7 --
  Weiss "Use score from TT instead of static eval if possible (#336)",
  2020-09-08: +10.78/+12.09/+21.44, rule and rationale quoted above.
- https://github.com/TerjeKir/weiss/pull/336 -- PR conversation: "pruning
  heuristics", no quiescence scoping stated.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+hash:25e56feb
  -- Ethereal "Probe the TT during qsearch for cutoffs and an eval",
  2019-01-13: +11.04/+3.12/+2.55.
- https://github.com/lynx-chess/Lynx/pull/1319 -- Lynx qsearch
  score-over-eval-field, 2025-01: -1.70 first cut, then -0.01 STC / +1.07 LTC,
  merged at ~0. The nearest-shaped record to this step.
- https://api.github.com/search/commits?q=repo:lynx-chess/Lynx+%22tt+score%22
  -- the Lynx per-site family: #692, #1084/#1085, #1971 (removal), #1973
  (RFP), #2055 (NMP).
- https://www.chessprogramming.org/Quiescence_Search -- stand pat as the lower
  bound and its beta cutoff; silent on TT use inside quiescence.
- https://www.chessprogramming.org/Transposition_Table -- the three bound
  types and cutoff conditions; silent on the substitution idiom.
- https://talkchess.com/viewtopic.php?t=47373 -- +25/4000 games for qsearch TT
  probe+use in one engine (context; also cited by S106's enrichment).
author:    Maksym Bodnar

## What landed, 2026-08-22

Phase one of two. The change is in the tree, the gates are green and the SPRT
is launched. The verdict, the `done:` stamp and the `specs.md` wording are
phase two.

### Where the line numbers went

The step's section 2 is written at `cf89e22` and every anchor in it has moved;
S093 (`40f5b56`) rewrote most of quiescence's neighbourhood. Re-derived at
`293a45b`, and these are the ones the change sits between:

| section 2 says | actually at `293a45b` |
|---|---|
| probe :226 | `tt_get_entry` at :280 |
| outright answer :221-223 | :282-288 |
| `tt_entry_answers` :155-188 | :219-252, de-normalisation at :234 |
| stand-pat :241-247 | :305-311 |
| `stored_eval` :253 | :317 |
| `stand_pat >= beta` :268 | :332 |
| alpha raise :279 | :343 |
| qply cap :287 | :351 |
| fail-soft floor :332 | :396 |
| final store :387-390 | :451-454 |

Section 2's two structural claims survive the move intact: only a lower bound
with `s < beta` and an upper bound with `s > alpha` reach the stand-pat site,
and a raise therefore cannot itself trip the `stand_pat >= beta` return.

### The shape, as built

`stand_pat` was `const` and is not any more. The static number keeps the old
name reversed into `static_eval` / `static_eval_is_exact`, `stored_eval` is
fixed from it as before, and `stand_pat` is a successor variable initialised
from it — so the eval field cannot be reached by the substitution even by
accident. The substitution is one `if`, placed immediately after `stored_eval`
and before the `check_limits` return, which is section 5's minimal-diff option
rather than the `!in_check` variant.

The bound test is written as one predicate, so the three arms are read
together and an inverted comparison is visible on one screen:

```cpp
if (tt_entry != nullptr && std::abs(tt_entry->score) <= MATE_MIN) {
  const int tt_score = de_normalize_score(tt_entry->score, ply);

  const bool substitutes =
      tt_entry->type == TT_PV_NODE ||
      (tt_entry->type == TT_BETA_NODE && tt_score > stand_pat) ||
      (tt_entry->type == TT_ALPHA_NODE && tt_score < stand_pat);

  if (substitutes) { stand_pat = tt_score; }
}
```

The mate test is on the **raw** score and the exclusion is `<= MATE_MIN`, so
anything outside the band on either side — including a stray value above
`MATE_MAX` — is refused rather than substituted. It is ply-independent because
the band is 1000 wide against a `MAX_PLY` of 128, which is exactly what S106's
`static_assert` above the probe holds; `de_normalize_score` is then an identity
on everything that gets through, and is called anyway so that "no raw table
score is ever consumed" stays uniform.

### Red first, in three stages and two perturbations

Every output below is verbatim from `./build/tests/test_search`.

**Stage A — tests present, no implementation.** The two directional cases are
red against the tree as it stood:

```
/home/max/ws/chesso/tests/test_search.cpp:1361: ERROR: CHECK_EQ( run(-10000, 10000), 600 ) is NOT correct!
  values: CHECK_EQ( 563, 600 )

/home/max/ws/chesso/tests/test_search.cpp:1369: ERROR: CHECK_EQ( raised->score, 600 ) is NOT correct!
  values: CHECK_EQ( 563, 600 )

/home/max/ws/chesso/tests/test_search.cpp:1380: ERROR: CHECK_EQ( run(0, 10000), 500 ) is NOT correct!
  values: CHECK_EQ( 563, 500 )
```

:1361 is the lower bound raising, :1380 the upper bound capping, :1369 the
entry's score field following the raise. The mate-band case is green here and
that is expected: with no substitution there is nothing to exclude.

**Stage B — the bound directions implemented, the mate-band guard deliberately
omitted.** The mate case is red against exactly the bug the clause guards, and
an existing S094 case falls over with it:

```
/home/max/ws/chesso/tests/test_search.cpp:1303: FATAL ERROR: REQUIRE_EQ( run(), planted ) is NOT correct!
  values: REQUIRE_EQ( -9999, 363 )

/home/max/ws/chesso/tests/test_search.cpp:1437: ERROR: CHECK_EQ( mating, static_score ) is NOT correct!
  values: CHECK_EQ( 48995, 563 )
/home/max/ws/chesso/tests/test_search.cpp:1438: ERROR: CHECK( mating < MATE_MIN_LOCAL ) is NOT correct!
  values: CHECK( 48995 <  48000 )
/home/max/ws/chesso/tests/test_search.cpp:1443: ERROR: CHECK_EQ( mated, static_score ) is NOT correct!
  values: CHECK_EQ( -48995, 563 )
/home/max/ws/chesso/tests/test_search.cpp:1444: ERROR: CHECK( mated > -MATE_MIN_LOCAL ) is NOT correct!
  values: CHECK( -48995 >  -48000 )
```

**The S094 case at :1303 is the one that matters, and it is re-targeted rather
than relaxed.** `quiescence stands pat on the stored static score` planted an
upper bound of -9999 to prove the entry's score could not answer the node, so
that anything but 563 had to have come from the eval field. Against a beta of
10000 that entry is still inert; against a stand pat of 563 it is not — an
upper bound below the static score is precisely what this step consumes, and
the case answered -9999. It now plants the same -9999 as a **lower** bound,
which is inert on both counts, and the assertion is unchanged. The failure was
observed before the edit and is the reason for it.

**Stage C — the guard added.** All 19 fast tests green in both builds.

**Perturbation 1, the inversion.** The two "never the other way" cases exist
because a swapped comparison leaves node counts and every other suite green
and leaks only strength (section 5). Comparisons inverted in place:

```
/home/max/ws/chesso/tests/test_search.cpp:1382: ERROR: CHECK_EQ( run(-10000, 10000), static_score ) is NOT correct!
  values: CHECK_EQ( 500, 563 )

/home/max/ws/chesso/tests/test_search.cpp:1391: ERROR: CHECK_EQ( run(0, 10000), static_score ) is NOT correct!
  values: CHECK_EQ( 600, 563 )
```

:1382 is a lower bound lowering the stand pat, :1391 an upper bound raising it.
Reverted immediately.

**Perturbation 2, the exact case.** The accepts' "an exact score replaces it"
cannot be seen red against the unmodified tree — `tt_entry_answers()` returns
an exact entry whatever the window is, so the assertion is already satisfied by
the probe, which section 2 predicted and the fire count below confirms (the
exact arm fires 0 times). Its non-vacuity is shown by removing the exact path
from **both** places at once, the probe's `TT_PV_NODE` arm and the
substitution's:

```
/home/max/ws/chesso/tests/test_search.cpp:1398: ERROR: CHECK_EQ( run(-10000, 10000), 600 ) is NOT correct!
  values: CHECK_EQ( 563, 600 )
```

So the assertion is about the outcome and not about which path serves it, which
is what makes it survive a future PV guard on the probe. Reverted immediately.

### The eval field keeps S094's semantics, asserted from the consumer side

The raise case re-reads the entry the run wrote:

```cpp
const tt_entry_t* raised = tt_get_entry(&tt, &game.board);
CHECK_EQ(raised->score, 600);          // the raised stand pat, fail-soft
CHECK_EQ(raised->eval, static_score);  // still 563, never 600
```

Green. The mechanism is that `stored_eval` is computed from `static_eval`
before `stand_pat` exists, so there is no ordering to get wrong later; the code
comment says so at the site.

### The mate suite did not move

`test_engine`'s 48-position S145 set, at `293a45b` and with the change, in the
same build directory and read from the same `MESSAGE` line:

```
before: mate in 4: 0 of 8 exact, mate in 5: 0 of 8   49 cases, 866668 assertions
after:  mate in 4: 0 of 8 exact, mate in 5: 0 of 8   49 cases, 866668 assertions
```

Identical, assertion count included.

### Node effect

INV-6 is not available — the change alters play by construction — so these are
the mechanism beside the verdict and not a discharge. `tools/search_bench.py`,
interleaved, two runs each, machine idle:

| depth | position | `293a45b` | S130 | delta |
|---|---|---|---|---|
| 9 | midgame | 121496 | 121512 | +16 |
| 9 | kiwipete | 800792 | 800768 | -24 |
| 9 | tactical | 62910 | 62911 | +1 |
| 12 | midgame | 639205 | 639221 | +16 |
| 12 | kiwipete | 3425236 | 3425199 | -37 |
| 12 | tactical | 367858 | 367859 | +1 |

Best move unchanged at all six: `c3d5` / `e2a6` / `d7c8q`. Every delta is under
0.01 %, so the timings are not separable from noise and are not quoted.

### The fire rate, counted before booking the match

Section 6 calls this cheap and optional; it was bought, because a near-zero
fire rate predicts the zero before the four hours are spent. A throwaway
instrumented build, three `search_bench` positions at depth 12, one process and
one table across all three:

```
S130 qs stand-pat sites 1818978  entry 20483 (1.13%)  mate-skipped 0
                        raised 4110 (0.226%)  lowered 149 (0.008%)  exact 0
```

Read: the probe finds an entry on **1.13 %** of quiescence stand-pat sites, up
from the 0.79 % S094 measured on kiwipete alone. Of those entries **20.1 %**
supply a usable raise and **0.7 %** a usable cap, so the substitution fires on
**0.23 %** of sites in total. The exact arm fires **0** times, as section 2
predicted. **0** entries were rejected for a mate score on this workload —
which is why the band exclusion is held by a unit test and not by a counter.

That is a thin mechanism, and it is consistent with Lynx #1319, the
nearest-shaped published record, merging at about zero. It is written into the
SPRT script's header as the standing candidate explanation for a no-verdict
before the first game.

### Gates

| gate | result |
|---|---|
| `cmake --build build -j12 && ctest -L fast` | 19 of 19 pass |
| `cmake --build build-tune -j12 && ctest -L fast` | 19 of 19 pass |
| `./clang-format.sh --check` | exit 0 |
| `python3 tools/plan_prose_check.py --touches` | 0 flagged over 64 files |

### The SPRT

`adocs/data/S130_sprt.sh`, launched detached against `293a45b`, bounds
`elo0=0 elo1=5 alpha=0.05 beta=0.05` — the coordinator's decision, the default
gainer pair, on the same reasoning as S093 verdict 1. All three readings are
pre-registered in the script header before the first game, including the
no-verdict clause with a number on it: **if the LLR is still inside the bounds
at about 8000 games, that is itself evidence the effect is small**, and
re-running at two-sided bounds is a recorded decision rather than an automatic
action (DEC-063, S021's precedent). S093 verdict 2 is the cost being priced —
an H0 to the wall against this exact pair took 6 h 35 m and 15398 games.

### Left for phase two

- The verdict, the `done:` stamp, the commit.
- `adocs/specs.md`'s "absent, machinery" row carries the S094/S103 stand-pat
  story and has to gain this layer's sentence and its number, in the same
  commit as the behaviour change.
- `README.md` is the owner's and `MANUAL.md` describes no part of this; both
  still to be checked at completion.
- The watched corner, if the verdict is negative: the final store marks the
  entry `TT_PV_NODE` when `best_value > alpha0`, so a raised stand pat no
  capture beats can be stored exact where only a one-sided bound is known. It
  predates this step, `excludes:` forbids touching it here, and a store-side
  guard is its own step.

### One citation flag stopped being reported, and it was correct

`--citations` read 84 flags at `293a45b` and reads 83 now. The one that
vanished is this file's own, and it disappeared when `--step start` moved the
file into `plan_current/`, not when the prose below was written — `baseline()`
returns `None` for a step file that is new or dirty in the working tree, and an
untracked path is both. S139's hazard, exactly as described.

```
DRIFT  adocs/plan_todo/S130_quiescence_tt_score_stand_pat.md:100  src/search.cpp:226
       -- held 'const tt_entry_t* tt_entry = tt_get_entry(state->tt, &game' at 7b54f18
```

**It was correct.** Section 2 line 100 puts `tt_get_entry` at
`src/search.cpp:226` and at `293a45b` the call is at :280. It is corrected by
the anchor table above rather than by editing section 2, which is research
written at `cf89e22` and is left as the record of what was true then. No other
flag in the file moved: it held 7 code citations and 1 flag before, and the
other six were and remain unflagged.

## The first SPRT was killed at 103 games, 2026-08-22

`.tuning/sprt_s130.log` is the record and it stays. 12 cores, tc 8+0.08,
reference `293a45b`, out dir `/tmp/chesso_sprt_s130_20260822_074255`. It ran
for about nine minutes and reached game 117 started, 103 finished, before the
orchestrator killed it deliberately. No verdict was reached and none is
claimed. The reason is below and it is the right one: a known defect in the
tree contaminates every measurement taken after it (AGENTS.md §0), and the
defect was in the change being measured.

### The corner the last section called "watched, out of scope" was neither

Phase one's closing note said the exact-store corner predated S130 and was
excluded. **The shape predates S130. The unsoundness does not, and the
orchestrator was right to stop the run.** Verified by test rather than by
reading, which is the only reason it is stated as fact here.

Before S130, `best_value` at the final store was `max(static_eval, captures)`
when nothing beat the stand pat, and storing that `TT_PV_NODE` is *correct by
quiescence's own definition*: the value of a node no capture improves is what
standing pat is worth. After S130, `best_value` can be a table bound, and
storing a bound as exact claims `value == s` on evidence that says only
`value >= s` — at the same key and the same depth, over the entry that
certified it, where every later quiescence probe answers from an exact entry
unconditionally.

The mirror is worse than incidental, it is **unconditional**. An upper-bound
entry only reaches the stand-pat site when `tt_score > alpha` — otherwise
`tt_entry_answers()` would have answered the node — and `alpha` at the probe is
still `alpha0`. So a cap always leaves `best_value = tt_score > alpha0`, and
the store's `best_value > alpha0` test therefore *always* chose exact.

**Red, verbatim, before the fix** (`1` is `TT_PV_NODE`, `2` `TT_ALPHA_NODE`,
`3` `TT_BETA_NODE`):

```
tests/test_search.cpp:1517: ERROR: CHECK_EQ( raised->type, TT_BETA_NODE ) is NOT correct!
  values: CHECK_EQ( 1, 3 )

tests/test_search.cpp:1528: ERROR: CHECK_EQ( capped->type, TT_ALPHA_NODE ) is NOT correct!
  values: CHECK_EQ( 1, 2 )

tests/test_search.cpp:1580: ERROR: CHECK_EQ( entry->type, TT_BETA_NODE ) is NOT correct!
  values: CHECK_EQ( 1, 3 )
```

Non-vacuous by construction: the same case first asserts that with an *empty*
table this node still stores `TT_PV_NODE` with 563, so what is being tested is
the substitution and not the site having stopped storing exact scores at all.

### Where the orchestrator's specification needed one correction

The brief said "a value a real capture search beat is as exact as it was
before". That is true for a **raise** and false for a **cap**, and :1580 above
is the case that separates them.

A stand pat raised by a lower bound sat *above* the static score, so a line
that beats it beats the static score too and the maximum is the one quiescence
defines — exact, nothing owed. A stand pat lowered by an upper bound sat
*below* the static score, so the static score it displaced may beat the winning
line as well; the node's maximum can then be smaller than the value quiescence
would have reported, and all that is established is `value >= best_value`.

Held by a perturbation rather than an argument. With the loop written the way
the brief specified — `value_type = TT_PV_NODE` on every improvement — the two
symmetric cases stay green and the third goes red on its own:

```
tests/test_search.cpp:1580: ERROR: CHECK_EQ( entry->type, TT_BETA_NODE ) is NOT correct!
  values: CHECK_EQ( 1, 3 )
[doctest] test cases: 10 | 9 passed | 1 failed
```

### The fix

The property, stated once: **a node never stores a claim stronger than the
weakest thing that produced its value.** Implemented by carrying the stand
pat's bound kind alongside its number, in `node_type_t` so it maps onto the
store with no translation:

- `stand_pat_type` starts `TT_PV_NODE` — the static score is exact by
  quiescence's definition — and the substitution sets it to the entry's own
  type, which is where the weakness comes from and what direction it runs in.
- `floor_type` is `stand_pat_type`, or `TT_PV_NODE` in check, where the stand
  pat is not on offer and so is not an input to the maximum.
- `value_type` starts at `floor_type` and, when a searched move takes over the
  maximum, becomes `TT_BETA_NODE` if the floor was capped and `TT_PV_NODE`
  otherwise. **Computed from `floor_type` and never from itself**, so a second
  improvement cannot launder the first.
- The final store degrades: `value_type == TT_PV_NODE` keeps the existing
  `best_value > alpha0 ? PV : ALPHA` window test; anything else stores
  `value_type` and the window test cannot raise it back.

Two properties fall out and both are worth naming. With no entry, `floor_type`
is `TT_PV_NODE` and every branch reduces to the pre-S130 code exactly. And the
substitution becomes **fixpoint-stable**: a raise that nothing beats now
re-stores the same `TT_BETA_NODE(s)` it read, instead of upgrading its own
evidence to exact on every revisit.

Paths checked and unaffected, each for a stated reason:

| path | why it is untouched |
|---|---|
| `stand_pat >= beta` early store | stores `TT_BETA_NODE`, a lower bound, which is what it has. A raise is below beta by construction and never arrives; a cap sits below the static score, so `value >= stand_pat` follows from `value >= static_eval`, the claim the node was already making |
| in-loop `score >= beta` store | stores the child's searched score as `TT_BETA_NODE`; no stand pat in it |
| `in_check && legal_moves == 0` mate store | `floor_type` is `TT_PV_NODE` in check, and the branch returns before the final store. The mate store is unreachable from the substitution |
| `check_limits` and ply-cap returns | store nothing at all |
| the entry's `eval` field | still fixed from `static_eval` before `stand_pat` exists. S094 semantics, unchanged twice over |

### The mate suite still did not move

Same 48-position S145 set, same build directory, third reading:

```
293a45b   mate in 4: 0 of 8 exact, mate in 5: 0 of 8   49 cases, 866668 assertions
S130 v1   mate in 4: 0 of 8 exact, mate in 5: 0 of 8   49 cases, 866668 assertions
S130 v2   mate in 4: 0 of 8 exact, mate in 5: 0 of 8   49 cases, 866668 assertions
```

### Fire rate, re-taken, and what the fix actually corrects

Same instrumentation as before, three `search_bench` positions at depth 12, one
process:

```
S130 sites 1821940  entry 20432 (1.12%)  mate-skipped 0
           raised 4161 (0.228%)  lowered 151 (0.008%)  exact 0
final store  PV 175  ALPHA 318250  BETA 386  degraded-from-PV 386
```

The substitution side is unchanged to within the workload's own jitter — 1.12 %
against 1.13 %, 0.228 % raises against 0.226 % — so the SPRT header's standing
explanation for a no-verdict still holds. The store side is the new number and
it is not small in relative terms: **386 of the 561 exact stores this site
would have written were laundered bounds, 69 %.** They are now `TT_BETA_NODE`,
and PV stores at this site drop from 561 to 175.

### Node effect, re-measured with the fix

`tools/search_bench.py` at depth 12, interleaved, two runs each:

| position | `293a45b` | S130 + fix | delta |
|---|---|---|---|
| midgame | 639205 | 639228 | +23 |
| kiwipete | 3425236 | 3430710 | +5474 (+0.16 %) |
| tactical | 367858 | 367858 | 0 |

Best move unchanged: `c3d5` / `e2a6` / `d7c8q`. kiwipete moves more than v1's
-37 did, in the expected direction: fewer exact entries means fewer nodes
answered outright, so slightly more search. Still under 0.2 %, and still
mechanism rather than evidence — INV-6 is not available.

### Gates, re-run after the fix

| gate | result |
|---|---|
| `cmake --build build -j12 && ctest -L fast` | 19 of 19 pass |
| `cmake --build build-tune -j12 && ctest -L fast` | 19 of 19 pass |
| `./clang-format.sh --check` | exit 0 |
| `python3 tools/plan_prose_check.py --touches` | 0 flagged over 64 files |

## Run 2: no verdict over 16784 games, 2026-08-22

Stopped by the orchestrator at 7 h 12 m. **No verdict is the outcome, not an
interruption** — the pre-registered clause named 8000 games as the point at
which staying inside the bounds is itself evidence, and this ran to more than
twice that.

```
Elo: 1.14 +/- 4.04, nElo: 1.48 +/- 5.26
LOS: 70.95 %, DrawRatio: 38.80 %, PairsRatio: 1.01
Games: 16784, Wins: 5739, Losses: 5684, Draws: 5361, Points: 8419.5 (50.16 %)
Ptnml(0-2): [790, 1759, 3256, 1780, 807], WL/DD Ratio: 2.57
LLR: -0.71 (-24.1%) (-2.94, 2.94) [0.00, 5.00]
```

0 time forfeits in 16788 games written to the PGN; 12545 adjudications, 4243
normal terminations. `adocs/data/S130_sprt.log` is the run's own output,
tracked because `.tuning/` is gitignored and the number has to survive the
machine.

**It oscillated, it did not travel.** Over the 839 result blocks the log
printed, the LLR stayed inside `[-1.71, +0.69]` against bounds of `+/-2.94` —
never past 58 % of the way to H0 or 23 % to H1. Sampled every ~2000 games from
the tracked log:

```
    16  -0.09     6002  +0.28    12000  -1.09
  2002  -0.52     8000  +0.21    14002  -0.81
  4000  -0.07    10000  -0.36    16002  -0.93   final 16784  -0.71
```

DEC-063's failure mode, and worse than its precedent: S068 spent 6 h 36 m over
9036 games to reach the same nothing, and this spent 7 h 12 m over 16784.

**The pre-registered explanation was the right one.** The substitution fires on
0.23 % of quiescence stand-pat sites. There was never enough mechanism for
`+/-5` bounds to resolve, and that was written into the script header before
the first game rather than reached for afterwards.

**No re-run.** The pre-registration makes that a recorded decision and not an
automatic action (DEC-063, S021's precedent), and the orchestrator decided
against it: `+/- 4.04` over 16784 games is tighter than most verdicts this
project produces, and the interval `[-2.9, +5.2]` already excludes the 5-Elo
loss a two-sided re-run would be asked to exclude. Another two hours buys a
label for a number that is already in hand.

### Kept, and the four grounds

Recorded as zero and **kept**. DEC-103, agent-proposed under DEC-041 and
annotated so the owner can reverse it.

1. **The point estimate is positive**, +1.14, where S093 verdict 2's was -1.65
   and was reverted. Nothing here has the wrong sign.
2. **AGENTS.md permits keeping a measured zero with the reason stated.** S005,
   S006 and S015 are the precedent and all three were kept.
3. **The thin mechanism is explained and is itself a target — forward-looking
   and unmeasured, and labelled as such.** The substitution can only fire where
   the probe hits, and the probe hits 1.12 % of quiescence nodes. S119's table
   layout and S120's evaluation cache both move that number, so a consumer that
   fires more often later is worth having in place already. **This is an
   argument about steps that have not run.** It is the S015 shape exactly — a
   feature kept on what it should enable rather than on what it measured — and
   if S119 and S120 land without moving the hit rate, this ground expires and
   nothing else here depends on it.
4. **The bound-type rule is durable infrastructure, not S130's overhead.** This
   is the load-bearing ground and it is the one that survives S130 itself.

   What is encoded is one property: *a node never stores a claim stronger than
   the weakest thing that produced its value.* It is not a fact about taking a
   table score as a stand pat. It is a fact about **any** consumer that writes
   a value derived from something weaker than the claim the store is about to
   make, and quiescence's final store makes an exactness claim on every node
   that beats `alpha0`.

   Three pending steps take the stand pat as their base and each one is a new
   way to put something weaker into it. **S112**, per-move futility, prunes
   moves against `stand_pat + margin` and returns a value built on that
   comparison. **S022**, delta pruning, does the same at the capture level.
   **S116** touches the same floor again. Every one of them, written the
   natural way, feeds a value into `best_value` that the final store will mark
   exact — and if the stand pat underneath is a substituted bound, that is the
   laundering we already found, re-introduced by a step whose author had no
   reason to look for it. The failure is invisible: node counts stay green,
   every suite stays green, only strength leaks. That is S106's class, and
   S106's sweep found the eight sites correct precisely because no path from a
   stored bound into a stand pat existed yet.

   Reverting S130 deletes `stand_pat_type`, `floor_type`, `value_type`, the
   degrading store and the three tests that hold them, and leaves the next step
   to rediscover the rule — after its own SPRT has already measured an unsound
   engine, which is what happened here and cost a killed run. **The rule stays
   even if a later step removes the substitution**: with no entry every branch
   reduces to the pre-S130 code exactly, so the machinery costs nothing when
   unused, and the tests keep failing loudly for anyone who removes it while a
   consumer still exists.

### What the published record was worth here

Third transfer failure of this session, after S093's two. Weiss cca90ea7
measured +10.78 / +12.09 / +21.44; Ethereal 25e56feb +11.04 / +3.12 / +2.55.
Neither transferred. What did transfer is the record this step's own research
called the nearest-shaped and the least flattering: **Lynx PR #1319**, qsearch
only, on top of an existing probe and an existing eval-field read — chesso's
exact situation post-S094 — measured -1.70, then -0.01 over 60152 games STC and
+1.07 at LTC, and merged at about zero. **+1.14 +/- 4.04 is Lynx's number.**
The scope concern written into this file's section on 2026-08-19, before any
code existed, called this outcome and gave the reason: Weiss's commit message
scopes its gain to "pruning heuristics" and never to quiescence, so most of it
plausibly belongs to the main-search sites, which here are S108 and S109.
DEC-019, and this is the form where the research was right and the headline
number was not.
