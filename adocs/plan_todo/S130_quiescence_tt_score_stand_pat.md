id:         S130
goal:       quiescence takes the table score as its stand-pat where the stored bound allows it, instead of the static evaluation alone
accepts:    an SPRT verdict, recorded whatever it is; the table score is used only where its bound makes it valid -- a lower bound may only raise the stand-pat, an upper bound may only lower it, an exact score replaces it -- with a unit test per bound type; a score in the mate band is never used as a stand-pat, tested, because a stand-pat must not become a mate claim; the entry's eval field keeps its S094 semantics -- what is stored there is still the static score and never the improved stand-pat, for the same reason a bound is never stored (an improved stand-pat is window-relative through its bound test); the fast suite green
touches:    src/search.cpp quiescence, tests/test_search.cpp
excludes:   per-move futility, which is S112; delta pruning, which is S022; any change to what is stored
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

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

- Probe already paid: `tt_get_entry` at src/search.cpp:216, outright answer
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
- Entry fields (src/data_structures.hpp:386-412): `int32_t score`
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
