id:         S109
goal:       late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
accepts:    all four rules land in one commit and are measured by **one** SPRT, whatever it returns, recorded as it comes (INV-6); every threshold and margin is a constant in src/search_params.hpp with a stated range and none of them is a number copied from anywhere (DEC-084); the four rules are gated on `depth - lmr_reduction(depth, move_number)` and not on raw depth; the late-move rule sets a skip-quiets flag the staged generator honours rather than `continue`-ing, so the quiet stage is abandoned and not merely skipped over; **a position with a forced mate inside the pruned depth is added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guards removed and the printout recorded**; no quiet is pruned while in check, at a PV node, on the first move, when the move gives check, or when alpha or beta is near mate, and the test asserts the precondition that would otherwise prune it; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   razoring, which is a node-level rule and is S116; SEE pruning of **captures** in the main search, which is S091; futility inside quiescence, which is S112; the improving flag, which S108 supplies and this step consumes
decisions:  DEC-071, DEC-082, DEC-084, DEC-087
closes:
blocks:
paused_by:
done:

## Why this is one step and not four

**Per-part attribution is deliberately forfeited here, and DEC-082 is the
decision that permits it.** The four rules prune overlapping sets of moves, so
each one measured against a tree the other three are absent from returns
nothing. Stockfish's own removal test is the evidence: move-count pruning
measures **~0 Elo alone** and the block it belongs to measures **~204**. Four
steps would spend four verdicts to record four zeros and leave this plan
believing a real +60 to +120 does not exist.

If the block fails, *then* it is bisected -- a failing block is evidence that
one part is wrong, which is the attribution question the block form was not
asked.

**The second review corrected the evidence without changing the shape,
DEC-087.** Lynx measured late move pruning and futility individually at
**+4.7 each** when added to a tree without the others, so "inert apart" is too
strong -- the parts are small-positive apart. The block is kept anyway, on
measurement-budget grounds: four verdicts on true effects near +5 sit at the
bounds and crawl (DEC-063), where one verdict on a +40 to +120 block resolves
in an hour. The bisect-on-failure clause is unchanged.

**This step absorbs S090 and S026, both retired.** Their ids are not reused.
S090 was late move pruning alone; S026 was "drop nodes near the horizon that
cannot reach alpha", whose razoring half is now S116 and whose futility half is
here. The mate clause below is S026's own, carried forward, and the retired
S060 existed to put it in S026's `accepts:` where it now sits in this step's.

## The four rules

For a quiet move, at `lmr_depth = depth - lmr_reduction(depth, move_number)`:

1. **Late move pruning.** Past a move count that grows with depth, stop
   generating quiets. The count is **doubled when improving** -- that is the
   single highest-leverage use of the flag S108 supplies.
2. **Futility.** Static score plus a base plus a slope times `lmr_depth` at or
   below alpha, skip the remaining quiets.
3. **History pruning.** History below a margin scaled by depth, skip.
4. **Quiet SEE pruning.** The exchange evaluation says the move loses more than
   a margin scaled by `lmr_depth` squared, skip.

The functional forms are read from the published record; **every constant is
ours and is fitted** -- a first setting from a sweep here, and SPSA at S127.
DEC-084.

## Hazard, three times observed and once more expected

Null move pruning hid a mate in two by reducing to depth 0. Late move reduction
reduced the mating move at the root. Reverse futility returns a static bound
and therefore cannot see a mate at all, which is why S033 bounded it to depth 6
and ply 3. All three were caught by a mate test rather than by a benchmark.
This is the fourth, fifth, sixth and seventh pruning rule in the engine and
they get the same treatment before any of them is called done -- the clause is
in the `accepts:` above rather than left for a later step to add.

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
below was a commit message or PR body, never a diff or source file (DEC-016,
DEC-084).

### 1. State of the art

**The block, traced.** The plan's ~204 is Stockfish PR #2401 (2019-11, removal
tests at 10+0.1, 20000 games): "step 14 (pruning at shallow depth) going from
~170 to ~204 Elo"; same message, "step 7 (futility pruning) going from ~30 to
~49" on code "actually identical" both times -- "the value of early futility
pruning increased significantly due to changes elsewhere in search." The
post-NNUE re-run (PR #3868, 2021-12) prices the block at +98.63 against its
in-block parts alone: FutilityMoveCount +6.91, futility parent +8.71, SEE
based pruning +9.10, cont-hist pruning +1.7 -- the block is ~5x its parts'
sum, DEC-082's whole case. The step's Lynx +4.7s: PR #512 "Add basic LMP",
merged 2023-11-24, **+4.7 +/-3.9**; PR #733 "Futility pruning", merged
2024-05-14, **+4.72 +/-3.97 STC / +2.07 +/-2.85 LTC** -- chesso's band.

**(a) Late move pruning.** Skip the remaining quiets past a move count that
grows with depth. Weiss PR #104 (2019-12, +20.17/+24.67): "after trying some
quiet moves we give up". The quadratic FORM `base + depth^2` is Stockfish's
shape (source, not citable) and in PR prose at ~2600 it **failed**: Lynx #512
measured `3 + depth*depth` at **-23.8 +/-15.0** while linear `moves >=
depth*10` capped at depth <= 3 passed +4.7; re-tries #783 (-0.9/-8.61) and
#1343/#1344/#1345 all closed; #1551 later removed the depth cap for
+0.87/+2.60. The aggressive count is a >3000 setting; the seed starts loose.
**Improving doubles the count**: Lynx #1129 "Improving: LMP, multiplying by
2" merged **+8.39 +/-4.48**; the halving direction (#1128) closed. Counter
semantics: Ethereal #160 (84ec1707, 2020-12) counted **all** moves toward the
threshold and set skipQuiets "at any point", +4.79/+6.31; Lynx #2492 (legal
instead of pseudo-legal count, +2.21) closed unmerged. The published mechanism
is a **skip-quiets flag** the move picker honours -- captures keep coming,
the quiet stage ends.
**(b) Futility pruning.** CPW: "adding a safety margin to the evaluation of
the current position. If this does not exceed alpha then futility pruning is
triggered to skip this move." Exempt per CPW: captures and moves that give
check; "not used when the side to move is in check, or when either alpha or
beta are close to the mate value"; requires "the existence of at least one
legal move to avoid returning erroneous stalemate scores". Margin scale per
CPW: a minor piece at depth 1, "more like the value of a rook" at depth 2.
The **lmrDepth** margin is Stockfish commit prose: dbd7f60 (2021) dropped the
second FP depth limit as "already capped by margin that is a function of
lmrDepth"; 910f779 (2020) raised the "LMRdepth threshold for futility pruning
at parent nodes". Improving in the move-loop margin is untraced in prose (its
futility role is RFP's margin, S108); it enters this block via LMP only.
**(c) History pruning.** CPW History Leaf Pruning (Fruit 05/11/03, Fabien
Letouzey): prune at "depth <= 0 after reductions" -- the lmrDepth idea in its
original form -- guarded !PV, not in check, at least 5 moves played, no
extension; two thresholds, reduce below the first, prune below the lower.
Modern records: Weiss #446 (2021-06) "Skip moves with bad history at low
depths" **+9.05/+10.50**, #483 lowered the margin, +6.33/+3.48; Lynx #972
(2024-09) **+2.06 +/-1.67**. Negative controls: captures included failed
(Lynx #1096, -7.23); `break` -> `continue` failed (Lynx #1828, -3.08 -- with
quiets ordered by history, everything after the first sub-threshold quiet is
sub-threshold too, so stopping is free); and the lmrDepth gate failed **twice
at Lynx** (#1303 -8.84, #1532 -0.94) where Stockfish prose keeps it (d37de3c
raises the "lmrDepth threshold for continuation history based pruning";
de2bf1a drops the depth limit as "naturally constrained by history
thresholds"). See Scope concerns.
**(d) Quiet SEE pruning.** Skip a quiet whose exchange evaluation loses more
than a margin scaled by lmrDepth. Stockfish prose: a834bfe (2018) allows "see
pruning on quiets to be done up to an lmrDepth of 9" -- the gate is lmrDepth
by record. The margin's power is source-only and both shapes appear in
surveyed descriptions (this file's goal says lmrDepth squared; S091's survey
says linear for quiets, squared for captures) -- ship the power as a
parameter, the fit decides. Weiss #742 (2024, ~3300) merged one shared
threshold for quiets and noisies, +1.44/+0.46, so the split is not
load-bearing either. No individual sub-3000 record for the quiet half was
found; SF #3868 prices "SEE based pruning" at +9.10.

**Exemptions every published form keeps**: nothing prunes while in check (the
list is evasions; pruning one risks a false mate) or near mate bounds;
futility exempts captures and checking moves; the TT move is structurally
exempt everywhere (searched first, before any count or flag trips); history
pruning exempts PV nodes (CPW). A killers exemption is untraced in prose.
LMP is the one rule with no per-move exemption list -- it ends a stage.

### 2. Shape for chesso

The move loop: `for (size_t i = 0;; ++i)` at src/search.cpp:635; staged quiet
generation inside it at :636-652 (`quiets_generated`, the branch the
skip-quiets flag must gate); `pick_next_move` :654; `make_move` :656;
`is_capture` :658; `is_check_move` :662 -- **computed only after make_move**,
from the child position; `legal_moves_counter++` :663; LMR eligibility and
`reduction = lmr_reduction(depth, legal_moves_counter)` at :693-698; the
fail-high update block :739-758; the no-legal-moves mate/stalemate return
:779-781. `is_pv` is a parameter (:408). RFP's guard row to copy the mate
clause from: :511-512 (`beta < MATE_MIN && beta > -MATE_MIN`).

- **lmrDepth before S098**: the reduction today is the static table
  `lmr_reduction(depth, move_number)` (src/search.cpp:42-76), `LMR_BASE 75` /
  `LMR_DIVISOR 225` (src/search_params.hpp:85-86), axes capped at 63. The
  accepts' `lmr_depth = depth - lmr_reduction(depth, move_number)`, clamped
  to >= 0, computed per candidate move with `move_number =
  legal_moves_counter + 1` when testing before the counter increments. When
  S098 rebuilds the reduction, this gate shifts with it by construction --
  see Interactions.
- **Exists after S108**: `static_evals[ply]` (TT_EVAL_NONE while in check --
  so futility is off in check by data as well as by guard) and the
  `improving_at()` helper whose first in-search call site is this step.
- **Exists after S093/S024**: signed quiet history `[-M, +M]` on butterfly
  indexing plus continuation entries, read through the single probe path
  those steps build; the sum's range is `[-3M, +3M]` after S024. The history
  threshold reads the **raw table sum**, never `score_move`'s return -- a
  killer's 900000 band value would silently exempt it (see Pitfalls).
- **Quiet classification in the loop**: `!MOVE_CAPTURE(m) &&
  !MOVE_PROMOTED(m)`, the same pair LMR uses (:693-694). En passant is
  capture-flagged. Non-capture promotions are not "quiet" here.
- **SEE for quiets exists today**: `see_ge(board, move, threshold)`
  (src/bitboard.cpp:1185) scores a quiet move -- `captured == EMPTY` gives
  gain 0 -- and takes a negative threshold, so quiet SEE pruning is
  `!see_ge(&game->board, moves[i], -margin)`. `see()` :1271 is the exact
  reference; `see_value` :1096; `capture_cannot_lose` :1160 is capture-only.
- **Does NOT exist**: a pre-make gives-check predicate. `is_check_move` is
  known only after make_move, so the per-move rules (futility, history, SEE)
  run **after** :662 and prune by `unmake_move + continue` -- the subtree
  saving dominates the wasted make. LMP's flag is pre-make by construction
  and cannot see gives-check (Scope concerns). Lynx #1520 measured moving
  rules before make at -0.6 +/-2.6, so nothing is lost by staying after.

### 3. Implementation sketch

One commit (accepts), assembled and tested in this order, each rule behind
its own constant whose declared range includes an **off value** -- that is
the kill-switch that keeps a failing block bisectable without four SPRTs and
without taking strength numbers on the tune build (forbidden, S073):

1. **Scaffold**: `lmr_depth` computation plus a `skip_quiets` flag wired into
   the generation branch (:636-652: when set, do not generate quiets, break
   instead) and into the staged-up-front corner -- when `tt_move_is_quiet`
   put both stages in the array (:619-623), already-generated quiets are
   filtered by the same flag. Flag never set yet: node counts identical,
   search_bench proves the scaffold inert before any rule lands.
2. **LMP**: `legal_moves_counter + 1 > lmp_threshold(depth, improving)` sets
   `skip_quiets` (never `continue`, per accepts). Threshold doubled when
   improving. Guards: `!is_pv`, `!is_in_check`, depth cap, `>= 1` legal move
   searched.
3. **Futility**: at `static_evals[ply] != TT_EVAL_NONE`, quiet, `!is_pv`,
   `!is_in_check`, `!is_check_move`, not first legal move, alpha and beta
   outside the mate band, `lmr_depth <= FUT_MAX_LMRDEPTH`: prune when
   `static_evals[ply] + FUT_BASE + FUT_SLOPE * lmr_depth <= alpha`. The
   margin shrinks as `lmr_depth` falls with move number, so later quiets
   prune more easily -- monotone at a fixed node.
4. **History pruning**: same guards, `depth <= HP_MAX_DEPTH` (raw depth --
   the lmrDepth gate is the accepts' rule; keep the raw-depth variant one
   constant-flip away, Scope concerns): prune when the S093/S024 history sum
   `< -HP_COEFF * depth`.
5. **Quiet SEE**: same guards, `lmr_depth <= SEE_QUIET_MAX_LMRDEPTH`: prune
   when `!see_ge(board, move, -(SEE_QUIET_COEFF * lmr_depth * lmr_depth))`
   (power parameterised; linear is the one-line alternative).
6. Tests red-first, per rule, before its rule lands:
   - Extend "pruning does not hide a forced mate" (tests/test_search.cpp:1274)
     with a position whose mating move is a **late, low-history, negative-SEE
     quiet** inside the pruned depth -- observed red with the in-check and
     near-mate guards removed, printout recorded (accepts). Built the S033
     way: python-chess enumeration plus Stockfish confirmation (DEC-023).
   - Stalemate edge: a position whose only legal moves are late quiets; the
     first-legal-move guard keeps :779-781 unreachable -- assert no false
     mate/stalemate score.
   - Pure-helper tests: `lmp_threshold` doubles exactly when improving says
     so; each formula at its off constant is provably unreachable.
   - Precondition tests (accepts, non-vacuous): a position where a rule fires
     (node counts move against the off value), then the exempt variants --
     in check, PV, first move, gives check, near-mate bounds -- search
     identically to the off build.
7. Fast suite green, one SPRT, verdict recorded whatever it is.

### 4. Constants and seeds

All in src/search_params.hpp with stated ranges; every number below is a
**seed -- must be fitted here (sweep) and SPSA'd at S127 (DEC-084)**. Off
values sit inside the declared ranges on purpose.

- `LMP_BASE`, `LMP_DEPTH_COEFF`, `LMP_MAX_DEPTH`: threshold `LMP_BASE +
  LMP_DEPTH_COEFF * depth` (linear seed: base 0, coeff 10, depth cap 3 --
  Lynx #512 PR prose, the only sub-3000 pass), quadratic `base 3 + depth^2`
  as the alternative FORM (failed at 2600, Lynx #512/#783 -- try it only
  above the first fit). Doubled when improving (Lynx #1129). Off: base at
  MAX_MOVES (270).
- `FUT_BASE`, `FUT_SLOPE`, `FUT_MAX_LMRDEPTH`: seed margin ~300 at depth 1,
  ~500 at depth 2 (CPW's minor/rook wording -> base 100, slope 200), cap 6-8.
  Off: base at the range top (>= 2 * queen value).
- `HP_COEFF`, `HP_MAX_DEPTH`: **no publishable numeric seed** (Weiss/Lynx
  constants live in source). Choose relative to S093's `HISTORY_MAX` M:
  first setting in the `M/64..M/8` per-depth-ply region, swept; depth cap
  seed 3-4 ("low depths", Weiss #446). Off: HP_COEFF at max (threshold below
  -3M).
- `SEE_QUIET_COEFF`, `SEE_QUIET_MAX_LMRDEPTH`: gate seed lmrDepth <= 9 (SF
  a834bfe prose); coefficient seeded so the bar at lmrDepth 1 is under a
  pawn's `see_value` (100, src/bitboard.cpp:1096) -- our own scale, not a
  copied margin. Off: coefficient at max.

### 5. Pitfalls

- **The recurring repo bug, now four ways.** Null move hid a mate in 2, LMR
  reduced the mating root move, RFP cannot see mates (S033). Every rule here
  carries the in-check + near-mate-bounds + PV + first-move guards, and the
  accepts' red-first mate test is the enforcement, not the comment.
- **Pruning the last legal move.** Rules skip moves without making them, so
  `legal_moves_counter` can end at 0 with legal moves on the board, and
  :779-781 would return a false mate/stalemate. The `>= 1 legal move
  searched` guard (CPW's own clause) is load-bearing; the stalemate edge
  test pins it. Zugzwang: no surveyed move-loop rule adds a phase guard
  (null move keeps that job); the depth caps bound the damage -- do not
  invent one silently.
- **Improving misread through S108's sentinel.** `static_evals[ply]` is
  TT_EVAL_NONE in check; `improving_at` falls back ply-2 -> ply-4 -> default
  true. A wrong-side default doubles the LMP count where it should not --
  Elo loss with no crash. The consumer-side test (LMP threshold doubles
  exactly when the helper says improving) is the guard; S108's own fallback
  tests cover the helper.
- **History sign and band errors.** After S093 history is signed; the
  threshold is negative (`< -HP_COEFF * depth`); a sign slip prunes the
  *good* quiets -- silent regression, the S093 band hazard's sibling. And the
  rule must read the raw table sum through the S093/S024 probe path: killers
  score 900000 and counters 700000 in `score_move` (src/evaluation.cpp:33-37,
  :1099-1105), so testing the ordering score instead of the table silently
  exempts killers/counters and nothing else -- decide the exemption, never
  inherit it from the wrong variable.
- **The skip-quiets flag kills killers and checking quiets in the tail.**
  Chesso's killers live inside the quiet stage (no separate emission stage),
  so an abandoned stage drops them; they order at the band top and are
  usually searched before the count trips, but the exposure exists. S107's
  newly-remembered checking quiets are what LMP would prune -- the mate test
  is the net. See Scope concerns.
- **Double-pruning with S091.** This step prunes quiets only; S091 prunes
  captures and adds an extra reduction for negative-SEE moves, after, with
  its own verdict. S091's accepts still names "separate margins for captures
  and for quiets" from before the re-scope -- the quiet margin lands **here**,
  not twice. When S091 arrives, one see_ge call per quiet, result shared.
- **Tune-build strength numbers.** The off values make per-rule ablation
  builds one-line release rebuilds; never SPRT the tune build (S073).

### 6. Measurement

- **One SPRT** at the S105 regime (8+0.08, Hash 16, UHO book), gainer bounds
  `elo0=0 elo1=5` (DEC-063: expected effect +40 to +120 self-play, far above
  the pair). Fast suite plus both mate cases green first, red observations
  recorded before the guards land. INV-6 takes the SPRT path -- node counts
  move by construction.
- **Bisection protocol on a failing block** (DEC-082/DEC-087): halves first,
  by evidence strength -- leg 1 disables {history, quiet SEE} (weakest
  sub-3000 records: +2.06, none) via their off constants, release rebuild,
  SPRT; leg 2 disables {LMP, futility} instead. One more run inside the
  failing half isolates the rule: three runs worst case against four
  one-at-a-time. First suspects within a half: LMP threshold too tight
  (Lynx's quadratic failed at -23.8) and the history threshold's sign/scale.
- **Node counts, recorded not argued**: tools/search_bench.py at depths 9
  and 12 before/after (expect large drops; the numbers go in the step
  stamp), and the DEC-081 probe re-run -- depth reached after `e4 e5 Nf3 Nc6
  Bb5 a6` at `go movetime 250` against the 12-on-1.45M baseline. Direction
  only; DEC-019 keeps expectations out of the verdict.

### 7. Interactions

- **S108 (required input)**: `static_evals[ply]`, `improving_at` -- this step
  is the helper's first in-search call site. S108's deferred layer (c),
  TT-score-adjusted eval, becomes "S109's first line" per that file: decide
  explicitly whether the futility comparison uses the adjusted value, and
  say so in the commit.
- **S093/S024 (history inputs)**: signed sum through one probe path;
  thresholds fitted against the **post-S024** shape (plan order guarantees
  it). Stockfish d37de3c records cont-hist pruning as strongly TC-sensitive
  -- the 8+0.08 verdict may not transfer upward; S128 eventually reads that.
- **S098 (after)**: rebuilds the reduction this gate is computed from.
  plan.md gives S098 three verdicts of its own, measured with the block
  live, so the block is **not** re-verdicted -- S098's SPRTs price the
  interaction and S127 refits the constants against the new lmrDepth.
- **S091 (after)**: capture SEE pruning + extra reduction; boundary above.
- **S112 (separate)**: per-move futility on **captures inside quiescence**;
  SF #3868 prices the quiescence pair separately (+4.95 futility+movecount,
  +5.14 negative-SEE).
- **S116 (separate)**: razoring is node-level, before the move loop, depth 1.
- **S107 (before)**: checking quiets now earn history; they are also what
  LMP's flag cannot exempt -- the mate test covers the join.
- **S127**: the eight-plus new constants enter with stated ranges; the
  quadratic LMP form and the SEE power are re-tried there, where the
  post-block tree can price them. Berserk 587fee4 (history in the futility
  margin, +4.34) and SF 2b62c44/93b14a1 are that phase's material.

### Scope concerns

1. **lmrDepth gating vs the sub-3000 record.** The accepts gates all four
   rules on `depth - lmr_reduction(...)`. Stockfish prose supports it
   (dbd7f60, a834bfe, d37de3c) -- at ~3600. The one engine that A/B'd the
   gate near this band measured it **negative twice for history pruning**
   (Lynx #1303 -8.84, #1532 -0.94; their merged rules gate on raw depth or
   move count). The accepts stands; but a failing block's bisection should
   include the gating axis, and the sketch's HP_MAX_DEPTH keeps raw depth
   one flip away.
2. **The accepts' gives-check exemption cannot bind LMP as written.** "No
   quiet is pruned ... when the move gives check" is testable for futility,
   history and SEE (post-make `is_check_move`), and structurally impossible
   for a skip-quiets flag honoured at the generation stage -- no pre-make
   gives-check predicate exists, and the published LMP form carries no such
   exemption. Read the clause as binding the three per-move rules; resolve
   the LMP reading with the owner at implementation -- recorded, not silent.
3. **"Quadratic in depth" is the >3000 form, not the entry form.** Both
   sub-3000 LMP passes traced (Weiss #104, formula unstated; Lynx #512,
   linear capped at depth 3) are conservative; every quadratic attempt at
   ~2600 failed. The FORM ships parameterised either way; the seed is loose.

### 8. References

- https://github.com/official-stockfish/Stockfish/pull/2401 -- removal tests 2019: step 14 pruning at shallow depth ~170 -> ~204; step 7 futility ~30 -> ~49; TC-sensitivity note. The plan's ~204.
- https://github.com/official-stockfish/Stockfish/pull/3868 -- 2021 re-run: block +98.63; FutilityMoveCount +6.91; futility parent +8.71; SEE based pruning +9.10; cont-hist pruning +1.7; qsearch pair +4.95/+5.14.
- https://github.com/official-stockfish/Stockfish/pull/4294 -- 2022 update (25k games, UHO): movecount pruning ~0 alone. The plan's "~0 alone".
- https://github.com/lynx-chess/Lynx/pull/512 -- basic LMP +4.7 +/-3.9; formula ablations in prose: 3+depth^2 -23.8, depth*10 cap-3 merged.
- https://github.com/lynx-chess/Lynx/pull/733 -- futility pruning +4.72 STC / +2.07 LTC.
- https://github.com/lynx-chess/Lynx/pull/1129 -- LMP count x2 when improving, +8.39 +/-4.48 (merged; #1128 divide-by-2 closed).
- https://github.com/lynx-chess/Lynx/pull/1551, /pull/783, /pull/2492 -- depth-cap removal +0.87/+2.60; depth^2 re-try failed; legal-count variant +2.21 closed.
- https://github.com/lynx-chess/Lynx/pull/972, /pull/1303, /pull/1532, /pull/1096, /pull/1828 -- history pruning +2.06; lmrDepth gate -8.84 and -0.94; captures -7.23; break->continue -3.08.
- https://github.com/TerjeKir/weiss/pull/104 -- LMP 2019: +20.17 STC / +24.67 LTC, "after trying some quiet moves we give up".
- https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22history+pruning%22 -- #446 +9.05/+10.50 "skip moves with bad history at low depths"; #483 margin -> +6.33/+3.48.
- https://github.com/TerjeKir/weiss/pull/742 -- one SEE threshold for quiets and noisies, +1.44/+0.46 at ~3300.
- https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+lmrDepth -- dbd7f60 FP margin is a function of lmrDepth; 910f779 FP lmrDepth threshold; a834bfe quiet SEE up to lmrDepth 9; de2bf1a quiet HP depth limit removed; d37de3c cont-hist pruning lmrDepth, TC-sensitive; 2b62c44 history sum in FP.
- https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+skipQuiets -- 84ec1707 (#160) count all moves toward LMP, set skipQuiets at any point, +4.79/+6.31; 85bbcd3 (2017) LMP added.
- https://www.chessprogramming.org/Futility_Pruning -- condition, exemptions (captures, checks, in-check, mate bounds, one-legal-move), minor/rook margin wording.
- https://www.chessprogramming.org/History_Leaf_Pruning -- Fruit origin; "depth <= 0 after reductions"; !PV / not in check / >= 5 moves / no extension; two-threshold reduce-then-prune cascade.
- https://www.chessprogramming.org/Late_Move_Reductions -- exemption list (tactical, in-check, gives-check, extensions, PV, killers, passers); published log-log reduction formulas (Obsidian, Weiss) as prose.
- https://github.com/jhonnold/berserk (commit 587fee4, message only) -- history value in futility pruning +4.34: S127-phase material.
- https://github.com/official-stockfish/Stockfish/commit/93b14a17d168e87e7f05fc09e3ba93e737b0757e -- title only ("Don't direct prune a move if it's a retake"): a later exemption refinement, not opened.
