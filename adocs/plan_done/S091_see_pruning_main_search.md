id:         S091
goal:       skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); separate margins for captures and for quiets, both constants in src/search_params.hpp with stated ranges (S073), and the depth scaling stated as what it is rather than as a flag; a position with a forced mate inside the pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; nothing is pruned at a PV node or while in check, with the precondition asserted; see() and see_ge() are unchanged and tests/test_search.cpp's exchange cases still pass unmodified; the fast suite green
touches:    src/search.cpp negamax move loop, src/search_params.hpp, tests/test_search.cpp
excludes:   any change to see() or see_ge() themselves; quiescence, where see_ge already declines losing captures at `src/search.cpp` `quiescence`; delta pruning, which is S022
decisions:  DEC-071
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); the SPRT is the coordinator's; started 2026-09-14 10:40 on the idle machine
done:       2026-09-14 18:05. **H1: the pair is worth +46.52 +/- 15.43 Elo at `8+0.08`, decided in 1360 games.** `adocs/data/S091_sprt.sh` ran as pre-registered -- candidate `d785b89` (this step's landing, pinned by `b0df255`) against the pre-step `08461e0`, both built fresh into `.ref-builds/` and both identity lines printed (`Chesso d785b89 native`, `Chesso 08461e0 native`), `{0, 5}` nElo, `8+0.08`, `Hash=16`, `noob_3moves.epd`, seed `20260914172203`, 12 cores -- from 17:22:03 to 18:00:14: **LLR 2.96, `Elo 46.52 +/- 15.43`, `nElo 56.38 +/- 18.47`, LOS 100 %, `Ptnml(0-2) [52, 119, 221, 172, 116]`, W 545 L 364 D 451, 1360 games in 38 m 11 s at 2137 an hour**, 0 time forfeits either side (the abort rule's 1.0 % never approached), 1015 adjudications and 348 natural ends over the 1363 games the PGN holds, `Incomplete mating PV` 1 from the candidate and 0 from the reference (recorded, not a stop), pair-score mean 1.1331 and variance 0.3363 over 680 pairs (`adocs/data/S091_sprt_pairs.txt`). Read as pre-registered: H1 at the gainer pair, **both rules kept**, the two-leg bisection not needed; the stopping run's Elo is upward-biased and what may be written is at least 5 nElo (DEC-063). The figure sits far above the +9.14 +/- 4.55 the published record priced for the skip alone at this regime, and DEC-019 keeps it from meaning more than it says: a rule's value depends on the search around it, and this pair landed on S109's block. The pre-registration named S225, S228, S229 and S213 as findings open while the run was taken; all four had closed before the first game (S213 on 2026-09-13, the other three earlier on 2026-09-14), so the run was taken with none open. The block-boundary readings (DEC-202's longer control, S199's next drift point) fall at the next boundary, not here. What the two rules are, their constants, tests, mutants and node counts are the "Landed" sections above; evidence `adocs/data/S091_sprt.log`. Implemented by an Opus 5 subagent and a repair agent; run, read and stamped by the coordinator (DEC-185, DEC-199).

## The precedent that decides how this is read

S015 measured quiescence SEE pruning at **0 Elo** and it was kept anyway with
the reason recorded. That verdict was taken when `see()` cost 12.1 % more than
it does today, and the main search is a different caller with a different
alternative -- a move pruned here is not searched at all, where in quiescence
it was only declined. So this is a new measurement and not a re-run of that
one, and a verdict of zero here is recorded as zero as well.


## Re-scoped 2026-08-19

Quiet-move SEE pruning moved into S109, where it is one of four rules measured
as a block (DEC-082). What is left here is the capture side plus the extra
reduction for a negative-SEE move, and it is not a leftover: the +151 release
this step's ordering rests on is Leorik 2.4. **The second review tempers the
attribution (DEC-087): that release bundled SEE in the main search with a
reverse-futility cutoff inside null move and with drawn-material recognition,
so +151 is the bundle's number and not SEE's alone.** The step keeps its
position -- Ethereal's removal ledger prices its SEE pruning at -41.5 on its
own -- and DEC-019 keeps the expectation honest.

The margins differ by move type in every implementation surveyed -- a margin
scaled by depth for quiets, by depth squared for captures -- and both are ours
to fit (DEC-084).

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
below was a release note, PR body or commit message, never a diff or source
file (DEC-016, DEC-084).

### 1. State of the art

**The FORM, traced.** CPW's SEE page: "In Minimax, SEE pruning is performed
on both captures and quiets. This is usually done with a **linear depth
margin for captures, and a quadratic depth margin for quiets**, though such
details may vary" -- note this is the *reverse* of this file's re-scope
paragraph; see Scope concerns. Stockfish prose agrees on shape: 714329d
(2016) allows "SEE pruning at higher depths ... using a threshold increasing
with depth" (STC+LTC passes), after 10429dd (2013) "Increase see prune
depth", **+9.56 +/-6.8**. Lineage: Ethereal's 10.00 version ledger has 9.70
"Implement SEE(). Use to prune in QSearch", 9.71 "Use SEE() to prune captures
and quiets in Search", 9.73 "different SEE() pruning values for improving
nodes" -- skip first landed as one rule for both move classes there.
**Band records for the capture skip**: Lynx #1521 "PVS SEE pruning", merged
2025-03-01, **+9.14 +/-4.55 at 8+0.08** and +14.32 +/-5.57 LTC -- measured at
exactly the S105 regime. It was Lynx's *sixth* attempt: #565 (-42.84/-31.3),
#1077 (threshold 0 everywhere, -219 to -23.58), #1144, #1066 (threshold +10,
-28.74), #987 (static -109, -11.10) all failed -- the technique is
detail-sensitive, not free. Weiss: #590 "Tweak SEE pruning" +2.55/+2.45,
#742 one shared threshold for quiets and noisies +1.44/+0.46, #589 separate
SEE piece values +2.42/+3.93. SF #3868 prices "SEE based pruning" at +9.10
in the 2021 removal re-run. Ethereal's removal ledger (-41.5 for SEE pruning
alone) is repo-recorded via DEC-087; its primary URL was not re-traced here.
**Exemptions, as published**: the PV exemption is *absent* from Stockfish's
record -- 335b57b5 (2013) "Prune negative SEE moves also in PV nodes"
measured **+5.18/+4.31**, and Lynx #1921 (skip pruning on ttPv) failed -0.99
-- the accepts' PV guard is chesso's own conservatism, kept. Gives-check:
SF 6ed81f0 (2019) stops pruning "check-extending moves except for discovered
checks" -- an exemption tied to check *extensions*, which chesso does not
have (S096 retired), so it does not transfer; the accepts carries no
gives-check clause for captures and the mate test is the net instead.
In-check and near-mate guards are universal, and the accepts asserts both.
The lmrDepth gate for SEE pruning failed at Lynx (#1531, -0.36) as it did
for history pruning; the traceable capture gate is raw depth.
**The reduction half, as published.** CPW LMR, Uncommon Conditions:
"Allowing reductions of 'bad' captures (SEE < 0)". Leorik 2.4's own release
notes: SEE is used "to skip bad captures in Quiescence search and to
**search moves with a bad SEE score at a reduced depth in the main search**"
-- so the +151 bundle's main-search SEE mechanism was the *reduction*, not
the skip (Scope concerns). Negative record: Lynx #2254 "LMR: Reduce more if
quiet moves return negative SEE scores" closed at **-5.04 +/-4.33** -- the
quiet side of the extra ply is the suspect half, keep it one flip from off.
**Why quiescence SEE measured 0 here and this is still expected to pay.**
S015's zero (kept, see() then 12.1 % dearer) priced *declining* a capture at
a node that could stand pat; a main-search skip removes a full negamax
subtree instead, and it is measured against a post-S109 tree that did not
exist at S015. The published statement of context-dependence is SF PR #2401:
"the value of early futility pruning increased significantly due to changes
elsewhere in search" on identical code. And the transfer failure is already
on record from the other side: Weiss #355 measured qsearch SEE pruning at
+35.73/+24.89 where chesso measured 0 -- DEC-019 both ways.

### 2. Shape for chesso

- - **SEE API, frozen by the accepts**: `see()` exact at `src/bitboard.cpp`
  `see`, `see_ge(board, move, threshold)` at `src/bitboard.cpp` `see_ge`, which
  handles en passant, promotions and quiets (`captured == EMPTY`, gain 0) in
  branches of its own; `src/bitboard.cpp` `capture_cannot_lose` is the
  capture-only fast path quiescence tries first; the scale is `see_value[6] =
  {100,300,300,500,900,10000}`, `src/bitboard.cpp` `see_value`.
- **Test coverage**: suite "search: static exchange evaluation"
  `tests/test_search.cpp` "search: static exchange evaluation" (six hand-valued
  cases, a quiet into an attack); "search: see_ge agrees with see"
  `tests/test_search.cpp` "search: see_ge agrees with see" sweeps every legal
  move of `all_test_fens()` against 8 thresholds, >10000 assertions -- the
  corpus carries EP-capture FENs (assets/test_jsons/pawns.json) and promotions
  (promotions.json), so both special branches are exercised against exact
  `see()`. No hand-valued EP or promotion case exists: the sweep catches the
  two implementations diverging, not a shared model error (Pitfalls).
- - **Quiescence site today**: `src/search.cpp` `quiescence`
  (`capture_cannot_lose` then `see_ge(..., 0)`), which is where the excludes
  line now points; it named a line five hundred lines short of the function
  until S138 re-anchored it, and the excludes' meaning is unchanged either way
  -- S094 grew the function under the old number.
- - **The skip site**: the move loop `for (size_t i = 0;; ++i)` in
  `src/search.cpp` `negamax`, where captures arrive from the first generation
  stage and each iteration runs `pick_next_move`, then `make_move`, then
  computes `is_capture`. `see_ge` reads the *parent* board, so the capture skip
  runs **pre-make**, between `pick_next_move` and `make_move` -- the make is
  saved, unlike S109's post-make quiet rules. Guards: `!is_pv`, `!is_in_check`
  (`src/search.cpp` `negamax`), `legal_moves_counter >= 1` (the move ordered
  first -- ORDER_TT_MOVE, `evaluation.cpp` `score_move` -- is then structurally
  exempt, and `src/search.cpp` `negamax`'s false-mate return stays
  unreachable), alpha and beta outside the mate band (copy the guard row at
  `src/search.cpp` `negamax`), `depth <=` the gate.
- - **The reduction site**: `src/search.cpp` `negamax`. Today captures and
  promotions are never reduced (`!is_capture && !MOVE_PROMOTED`,
  `src/search.cpp` `negamax`). The extra ply is two edits: quiets, `reduction
  += 1` after `src/search.cpp` `negamax` when the shared see_ge result is
  negative; captures, `reduction = 1` for a negative-SEE capture that survived
  the skip (deeper than the gate), clamped by `src/search.cpp` `negamax` as
  everything is. `is_check_move` (`src/search.cpp` `negamax`) is hardcoded
  false for captures -- never reuse it for capture logic; it exists for the
  killer path only.
- **Boundary with S109, exactly.** S109 owns pruning of **quiets**
  (`!MOVE_CAPTURE && !MOVE_PROMOTED`): LMP, futility, history pruning, and
  quiet SEE pruning at a margin scaled by lmr_depth -- the quiet SEE margin
  constant lands **there**. S091 owns pruning of **captures**
  (`MOVE_CAPTURE`, en passant included) at a depth-scaled capture margin,
  plus the SEE-sign reduction adjustment -- the only reduction change either
  step makes. Non-capture promotions are pruned by **neither** (outside
  S109's quiet class, outside this skip's capture class) -- deliberate:
  promotion mates are the mate hazard's favourite shape, and S131 is busy
  un-filtering them in quiescence. No move class is prunable twice; where
  both steps want SEE on the same quiet (S109's prune, S091's reduction),
  one `see_ge` call is made and the result shared (S109 Pitfalls names it).
  The accepts' "separate margins for captures and for quiets, both
  constants" predates the re-scope and is satisfied **jointly**: the quiet
  constant exists in src/search_params.hpp from S109 by the time this step
  completes; this step adds the capture constants only.

### 3. Implementation sketch

**One SPRT for both halves.** The accepts says "an SPRT verdict", singular,
and plan.md's multi-verdict list (S093, S024, S098, S097, S022) does not
name S091 -- the budget prices one. Each half sits behind its own constant
with an off value inside the declared range, so a failing step bisects by
release rebuild without new code (the S109 protocol), and the tune build is
never SPRT'd (S073).

1. Capture skip, pre-make, guards above: prune when
   `!see_ge(&game->board, moves[i], -(SEE_CAPT_COEFF * depth))` at
   `depth <= SEE_CAPT_MAX_DEPTH` (power parameterised; linear seed, CPW).
2. Extra-ply reduction from the shared per-move SEE result, both move
   classes, behind `SEE_LMR_EXTRA` (off value 0) -- Lynx #2254 says the
   quiet half is the first suspect in a bisect.
3. Tests red-first, before the guards land, printouts recorded (accepts):
   - a forced mate inside the pruned depth whose line runs through a
     negative-SEE capture, added to "pruning does not hide a forced mate"
     (`tests/test_search.cpp` "pruning does not hide a forced mate"), observed
     red with the in-check/near-mate guards removed; built the S033 way --
     python-chess enumeration plus Stockfish confirmation, never own judgement
     (DEC-023).
   - precondition tests (non-vacuous): a position where the skip fires (node
     counts move against the off value), then the exempt variants -- in
     check, PV node -- search identically to the off build.
   - - a position whose only legal moves are losing captures while not in
     check: the `legal_moves_counter >= 1` guard keeps `src/search.cpp`
     `negamax` unreachable, asserted.
   - `see()`/`see_ge()` untouched; both exchange suites pass unmodified
     (accepts) -- nothing in src/bitboard.cpp changes.
4. tools/search_bench.py at depths 9 and 12 before/after, numbers in the
   stamp; fast suite green; one SPRT; verdict recorded whatever it is.

### 4. Constants and seeds

All in src/search_params.hpp with stated ranges (S073); every number below
is a **seed -- must be fitted here (sweep) and SPSA'd at S127 (DEC-084)**.

- `SEE_CAPT_COEFF`: **no publishable numeric seed** -- SF/Lynx/Weiss margins
  live in source only. Seed from chesso's own scale: the bar at depth 1 under
  one pawn of `see_value` (100, `src/bitboard.cpp` `see_value`). Off: max.
- `SEE_CAPT_MAX_DEPTH`: no publishable value either (Ethereal 9.71 and SF
  prose name none); sweep 4..10. Off: 0.
- `SEE_LMR_EXTRA`: 1 ply, the narrowest published form (CPW "reductions of
  'bad' captures (SEE < 0)"; Leorik 2.4 "reduced depth"). Off: 0. The
  "negative" bar is `!see_ge(move, 0)`; a margin there is S127 material.
- Anti-seeds, measured negative at ~2600-2900: threshold 0 with no depth
  gate (Lynx #1077, -219), static -109 (#987), +10 (#1066), lmrDepth gate
  (#1531), ttPv exemption (#1921).

### 5. Pitfalls

- - **The repo's own SEE bug, S015-era** (CLAUDE.md's "shipped and pruned
  quiescence on wrong values for two commits"): a speculative cutoff in `see()`
  returned 400 for a 500 exchange on `3k4/8/1K6/8/8/8/1ppppppp/RqRRRRRR`; fixed
  in 63c9378, and the refusal to reinstate it is written at `src/bitboard.cpp`
  `see`. What catches its class today is the `tests/test_search.cpp` "search:
  see_ge agrees with see" agreement sweep plus the hand-valued cases -- the
  accepts freezes both. S015 also logged an x-ray king bug and an inverted
  ternary in see_ge; all three lived in code this step must not touch.
- - **A shared model error is invisible to the agreement sweep**: see() and
  see_ge() mishandling en passant *identically* would still pass
  `tests/test_search.cpp` "search: see_ge agrees with see". If any doubt is
  raised, add one hand-valued EP case and one promotion case red-first --
  cheap, and the suite has none today.
- **Sacrifices pruned**: the mate case in the accepts is the enforcement;
  the near-mate-bounds guard protects the defender's nodes on a mating
  line, the depth gate bounds what an iteration can miss -- the S033
  lesson that guards, not hopes, contain a static rule.
- **Scale confusion**: the skip's predicate is `see_ge`, never
  `capture_score`/MVV-LVA -- `piece_values_abs` (`evaluation.cpp`
  `piece_values_abs`) is an ordering scale whose bands clear each other by 100
  (CLAUDE.md hazard); `see_value` is the exchange scale. They share numbers
  today by coincidence, not by contract.
- **Ordering interaction**: captures are MVV-LVA (`evaluation.cpp`
  `capture_score`) inside the ORDER_CAPTURE band, so losing captures are
  *searched early* today (QxP defended orders above every quiet); the skip
  removes them at shallow depth regardless of order. S025 (retry losing
  captures after quiets, reserve) loses most of its premise once this lands --
  note it in the stamp if the verdict passes.
- **Double-prune with S109**: disjoint move classes by construction; a
  quiet S109 pruned never reaches this step's reduction. Verify with the
  off-value ablation: S109 off + S091 on must move only capture nodes.

### 6. Measurement

- **One SPRT** at the S105 regime (8+0.08, Hash 16, UHO book), gainer
  bounds `elo0=0 elo1=5` -- the stated S105 default; the band expectation
  is +5 to +15 self-play (Lynx #1521 +9.14 at this exact control), so the
  pair sits under the expected effect and terminates (DEC-063).
- Fast suite and both mate cases green first, red observations recorded.
- Node counts recorded, not argued: search_bench depths 9/12 against the
  current baseline (164123/670488/84351 and 1162576/5167100/683367). No
  published node figure for capture SEE alone was traced; record ours.
- A verdict of zero is recorded as zero and the keep/delete argument made
  in the stamp -- the S015 precedent this file opens with.

### 7. Interactions

- **S109 (before)**: boundary in section 2; quiet SEE margin lands there;
  one shared see_ge result per move where both steps want it.
- **S098 (after)**: rebuilds the reduction with history/node-type/re-search
  terms, three verdicts of its own; the extra ply must either survive as an
  additive term or be folded into the new formula -- state which in S098,
  and S098's accepts re-runs the mate case per adjustment, re-covering this
  step's clause.
- - **S112/S022 (quiescence siblings)**: S112 puts per-move capture futility
  *before* the SEE call at `src/search.cpp` `quiescence`; S022 owes the S015
  rerun and the delta decision. SF da8513f0 (2023, "do less SEE pruning in
  qsearch", constant negative threshold) and Lynx #1580/#1318/#1598 (qsearch
  SEE refinements, all negative) are their material, not this step's.
- **S023 (reserve)**: the S023 order note already says capture history's
  value is "in the capture futility and capture SEE margins" -- when it
  lands, this step's threshold gains a history term; SF d3860f8 (2023,
  history in SEE pruning thresholds) is the quiet-side precedent.
- **S127**: SEE_CAPT_* and SEE_LMR_EXTRA enter the SPSA set; the capture
  power (linear vs squared) and a nonzero reduction bar are re-tried there.

### Scope concerns

1. **The re-scope paragraph's margin shapes are backwards against CPW.**
   This file says depth-scaled for quiets, depth-squared for captures; CPW's
   SEE page says linear for captures, quadratic for quiets. S109 hit the
   same discrepancy from the quiet side and ships the power as a parameter;
   this step does the same with the **linear** capture seed. Goal and
   accepts name no power, so neither changes.
2. **The Leorik attribution sharpens further than DEC-087 stated.** Leorik
   2.4's release notes describe its main-search SEE as searching bad-SEE
   moves "at a reduced depth" -- the reduction half of this step -- with the
   skip confined to quiescence. So the +151 bundle contained no main-search
   SEE *skip* at all; the skip's evidence is Lynx #1521 and Ethereal 9.71.
   The step's position stands on those; the expectation stays DEC-019-sized.

### 8. References

- - https://www.chessprogramming.org/Static_Exchange_Evaluation -- "linear
  depth margin for captures, and a quadratic depth margin for quiets"; SEE to
  reduce bad captures and checks.
- - https://www.chessprogramming.org/Late_Move_Reductions -- Uncommon
  Conditions: "Allowing reductions of 'bad' captures (SEE < 0)"; Weiss/Ethereal
  capture-reduction prose.
- - https://api.github.com/repos/lithander/Leorik/releases -- 2.4 notes:
  qsearch skip + "moves with a bad SEE score at a reduced depth in the main
  search"; RFP-inside-null; drawn-material recognition; ~2800 estimate.
- - https://api.github.com/repos/lynx-chess/Lynx/pulls/1521 -- PVS SEE pruning,
  merged 2025-03-01, +9.14 +/-4.55 STC (8+0.08) / +14.32 +/-5.57 LTC; body
  carries no mechanics.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+SEE+pruning+in:title+type:pr
  -- #565 -42.84/-31.3; #1077 -219..-23.58; #1144; #1066 +10 threshold -28.74;
  #987 -109 threshold -11.10; #1531 lmrDepth gate -0.36; #1921 ttPv -0.99;
  #1318 qs in-check -0.77; #1598 qs quiets -4.00; #1580 recaptures -8.7.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+%22negative+SEE%22+type:pr
  -- #2254 reduce-more for negative-SEE quiets, closed -5.04 +/-4.33.
- -
  https://api.github.com/search/issues?q=repo:TerjeKir/weiss+SEE+in:title+type:pr
  -- #355 qsearch SEE +35.73/+24.89; #590 tweak +2.55/+2.45; #589 separate SEE
  values +2.42/+3.93; #742 shared quiet/noisy threshold +1.44/+0.46; #658 SEE
  in ProbCut +5.37/+2.05.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22SEE+pruning%22
  -- 10429dd 2013 depth increase +9.56; 714329d 2016 "threshold increasing with
  depth"; 57b6df4/335b57b5 2013 PV-node pruning +5.18/+4.31; 6ed81f0 2019
  check-extension exemption; 46ce245 givesCheck speedup; b61759e promotions
  redundant in SF's see_ge; a834bfe quiet lmrDepth 9; d3860f8 2023 history in
  SEE thresholds.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22negative+SEE%22
  -- 78eeba29 2019 simplify negative-SEE pruning; da8513f0 2023 qsearch
  constant negative threshold; a989aa18 2023 simplify.
- - https://talkchess.com/forum3/viewtopic.php?t=67602 -- Ethereal 10.00
  version ledger: 9.70 SEE in QSearch; 9.71 prune captures and quiets in
  Search; 9.73 improving-node values; 9.74 ProbCut.
- - https://talkchess.com/forum3/viewtopic.php?t=75335 -- Ethereal 12.75 final
  thread: no removal ledger published there; capture-history-in-LMR line (S023
  material).
- - https://github.com/official-stockfish/Stockfish/pull/2401 -- "value of
  early futility pruning increased significantly due to changes elsewhere in
  search" (re-cited from S109's research).
- - https://github.com/official-stockfish/Stockfish/pull/3868 -- SEE based
  pruning +9.10 in the 2021 removal re-run (re-cited from S109's research).
- - Ethereal removal ledger, SEE pruning -41.5 -- repo-recorded via DEC-087;
  primary URL untraced publicly in this pass.

## Landed 2026-09-14 11:25

Everything in `accepts:` except the SPRT, which is the coordinator's. The
`done:` stamp is theirs to write with the verdict.

### Reconciled with S109 before a line was written

This file predates S109 (landed 2026-09-13) and its own "Re-scoped 2026-08-19"
paragraph anticipated the split without knowing what would land. What S109
actually shipped is **quiet SEE pruning** -- `!see_ge(move, -SeeQuietCoeff *
lmr_depth^2)` in `negamax_at`'s move loop, gated on `lmr_depth`, off at a PV
node, in check, on the first legal move and in the mate band, with the
gives-check exemption -- plus its `SeeQuietCoeff` and `SeeQuietMaxLmrDepth`.

So the accepts' "separate margins for captures and for quiets, both constants
in src/search_params.hpp with stated ranges" is satisfied **jointly**, exactly
as section 2's boundary paragraph says it would be: the quiet constant was
already there when this step started, and what this step adds is

- **(a) capture SEE pruning** in the main search, its own margin and its own
  depth cap, over `MOVE_CAPTURE` only; and
- **(b) the extra reduction** of a negative-SEE move, capture or quiet, in the
  late move reduction path.

S109's quiet rule is untouched, and so are `see()`, `see_ge()` and quiescence.
Nothing in `src/bitboard.cpp` changed.

### Per accepts clause

| clause | where |
|---|---|
| an SPRT verdict against a named commit | `adocs/data/S091_sprt.sh`, pre-registration written, **not run** |
| separate margins for captures and for quiets, both constants with stated ranges | `SeeCaptureCoeff` here, `SeeQuietCoeff` from S109; `src/search_params.hpp` |
| the depth scaling stated as what it is | linear in `lmr_depth` for captures against S109's quadratic for quiets, said in the parameter's own comment and in `MANUAL.md` |
| a forced mate inside the pruned depth, red with the guard removed | three rows added to "pruning does not hide a forced mate", each red under a named mutant, below |
| nothing pruned at a PV node or in check, precondition asserted | `pruning_node`, reused unchanged; the PV half is the second drive of "capture SEE pruning skips the captures that lose material and no others" |
| `see()` and `see_ge()` unchanged, both exchange suites pass unmodified | `src/bitboard.cpp` is not in the diff; "search: static exchange evaluation" and "search: see_ge agrees with see" pass as written |
| the fast suite green | 38 of 38 in both builds |

### What landed, by file and symbol

- `src/search.cpp` `negamax_at`: the capture skip, between `pick_next_move` and
  `make_move`, `if (may_prune && is_capture)` -- **`may_prune` reused and not
  restated**, so the five node guards and the first-move guard are S109's own
  and there is one list, not two. `prune_rule = PRUNE_SEE_CAPTURE` when
  `lmr_depth < SEE_CAPT_MAX_LMRDEPTH && !see_ge(board, move, -(SEE_CAPT_COEFF *
  lmr_depth))`.
- `src/search.cpp` `negamax_at`: `see_loses_material`, the extra ply's own
  question, asked pre-make because `see_ge` reads the parent board. It cannot
  share an answer with either skip -- those ask whether a move loses *more than
  a margin* and this asks whether it loses anything -- so it is a second
  `see_ge` call, kept off the hot path by late move reduction's own eligibility
  in front of it and by `prune_rule == PRUNE_NONE`.
- `src/search.cpp` `negamax_at`: `capture_gives_check`, an `is_check()` paid
  only by the captures one of the two rules is about to act on. `is_check_move`
  is hardcoded false on a capture (S107) and reusing it for capture logic is
  what its own comment warns against.
- `src/search.cpp` `negamax_at`: `may_reduce`, the class-free half of the late
  move reduction condition, so the extra ply reads the same eligibility without
  inheriting `!is_capture`, which is about the ordering's guess and not about
  safety. The base reduction keeps `!is_capture && !MOVE_PROMOTED`.
- `src/data_structures.hpp` `prune_rule_t`: `PRUNE_SEE_CAPTURE = 5`, so a probe
  can tell the two SEE rules apart.
- `src/search_params.hpp`: three parameters, below.

**Two deliberate scope calls, both stated rather than assumed.** A capture that
promotes is inside the skip's class, because the class is `MOVE_CAPTURE` and
`see_ge` models the promotion gain in a branch of its own; a promotion that
captures nothing is outside every rule in the loop, which is where S109 left it
and why. And the extra ply exempts promotions, because that exemption is late
move reduction's own and the brief for this step said the existing exemptions
are kept.

### Files beyond `touches:`

The header's `touches:` line was written before S109 landed and names
`src/search.cpp`, `src/search_params.hpp` and `tests/test_search.cpp`. Four
more were needed and none of them is scope creep:

- `src/data_structures.hpp` -- one enumerator, `PRUNE_SEE_CAPTURE`, so the
  probe can tell the two SEE rules apart.
- `tests/test_search_params.cpp` -- the `golden_defaults` table, which every
  step that adds a parameter re-derives (DEC-142).
- `tools/mutants/search.py` and `tools/mutants/S109_shallow_pruning.py` -- three
  anchors that this step's restructuring invalidated. An anchor that no longer
  occurs makes `tools/mutation_check.py` refuse the whole run, so leaving them
  would have broken the tool for every mutant, not only for these three.
- `MANUAL.md` and `DEV_MANUAL.md` -- the DOCS rule.

Amending the field is the coordinator's call; recording the list is this
section's job.

### The three constants

| name | default | range | off | form |
|---|---|---|---|---|
| `SeeCaptureCoeff` | 50 | 0..10000 | its cap at 0 | (b) chesso's own exchange scale: the bar at lmr depth 1 stays under one pawn of `see_value`, so the declared interval is 0 to 100 and 50 is its midpoint -- the same derivation `SeeQuietCoeff` took |
| `SeeCaptureMaxLmrDepth` | 8 | 0..16 | 0 | (c) midpoint of the range the four S109 caps declare by purpose |
| `SeeLmrExtra` | 1 | 0..3 | 0 | (a) literature as a form with no number in it -- CPW's late move reduction page lists "allowing reductions of 'bad' captures (SEE < 0)" and Leorik 2.4's notes say "a reduced depth"; neither publishes a ply count, so the default is the smallest value that is not the off value |

`SeeCaptureMaxLmrDepth` reads `lmr_depth < CAP`, so **0 is an exact off value**
and the bisection leg needs one release rebuild. `SeeCaptureCoeff` has no off
value of its own for the reason S109 records for its two coefficients: the bar
is `-(COEFF * lmr_depth)` and at lmr depth 0 it is 0 whatever the coefficient
holds. `SeeLmrExtra` is an additive ply and 0 is off outright.

**The cap's axis is a choice against this file's own section 1**, which says the
traceable capture gate is raw depth because one engine A/B'd an lmr-depth gate
for SEE pruning and measured it negative. Gating every rule of the block on one
axis is worth more here than a second axis nothing in this tree has measured,
and S127 re-tries the axis with the power. Said in the parameter's comment too.

**No sweep set a default.** `adocs/data/S091_rule_sweep.txt` is what the two
rules do to the three `search_bench` positions at depths 9 and 12, taken
through the tune build, and it is characterisation and not a fit: a node count
at fixed depth is not Elo (DEC-019) and picking a cap by it would be picking by
a proxy. S127 is where these three enter SPSA.

### What the two rules do to the tree, measured

`tools/search_bench.py`, before -> after, `HEAD` = `08461e0`:

| position | depth 9 | depth 12 |
|---|---|---|
| midgame | 47635 -> 74327 | 141455 -> 172303 |
| kiwipete | 213916 -> 146873 | 1038779 -> 596588 |
| tactical | 26130 -> 27855 | 175684 -> 190823 |

Best moves at depth 9 unchanged (`c3d5` / `e2a6` / `d7c8q`); at depth 12
kiwipete moves `d5e6` -> `e2a6` and the other two are unchanged.
`chesso bench` **7105111 -> 6267842**, -11.8 %, which is the `Bench:` line the
commit carries (DEC-140). Nodes per second are flat, 3.83 M -> 3.80 M on the
bench, which is the second `see_ge` per late move against the subtrees the skip
removes.

**A skip does not always shrink the tree**, and the per-rule sweep is what says
so rather than an argument. Each rule alone against both off, depth 12,
`adocs/data/S091_rule_sweep.txt`:

    both off    141455 / 1038779 / 175684
    skip only   187956 /  666527 / 204486
    extra only  133938 /  714841 / 128080
    both on     172303 /  596588 / 190823

The skip takes 36 % off kiwipete and *adds* a third to midgame -- a node that
skips a capture has lost a move that might have cut it off and searches more of
what is left -- and the extra ply takes 31 % off kiwipete and 27 % off tactical
while taking 5 % off midgame. Neither rule is inert, neither dominates, and
only `both off` reports a different best move (`d5e6` against `e2a6` on
kiwipete). Regenerate with the driver quoted at the end of this section.

### Tests, and the red each was observed at

New and re-stated cases in `tests/test_search.cpp`, every one in the S191 suite
"search: pruning and reduction guards" except the mate rows. Each line below is
the printout doctest produced, copied from `.tuning/coord/S091_mutant_kills.txt`
and never paraphrased; the mutants are `tools/mutants/S091_capture_see.py`, six,
**6 of 6 killed**.

| case | mutant | observed red |
|---|---|---|
| capture SEE pruning skips the captures that lose material and no others | C05 | `REQUIRE_NE( rule_that_pruned(probe, safe), PRUNE_SEE_CAPTURE )` values `( 5, 5 )` |
| a capture that gives check is not pruned | C02 | `REQUIRE( k >= 1 )` values `REQUIRE( -1 >= 1 )` |
| a node whose only legal move is a losing capture is never pruned | C07 | `REQUIRE_EQ( probe.move_count, 1 )` values `( 0, 1 )` |
| capture SEE pruning stops at its depth cap | C06 | `REQUIRE( k >= 1 )` values `REQUIRE( -1 >= 1 )` |
| a capture that loses material is reduced by the extra ply | R02 | `REQUIRE_EQ( probe.reduction[losing], SEE_LMR_EXTRA )` values `( 0, 1 )` |
| a capture that gives check is not reduced | R01 | `REQUIRE_EQ( probe.reduction[k], 0 )` values `( 1, 0 )` |

**Two of those reds are the precondition and not the assertion the case is
named for, and that is a property of the case rather than an accident** -- the
same thing S109 recorded for P02 and P0A. A move the rule skips is a move the
node never searches, so `searched_index` reads -1 before the rule that skipped
it can be read at all.

**The mate case.** "pruning does not hide a forced mate" gains a table of three
rows, every position a row of `adocs/data/S145_mined_set.tsv` -- one per game
from this engine's own self-play -- and every one a forced mate whose line runs
through a capture that loses material. Oracles, and not a reading of the board:
stockfish at depth 20 through python-chess, with python-chess's own reading of
the root.

- `3krb1r/Np2pppp/3q1n2/8/Q4Bb1/2P3P1/P3NPBP/3RR1K1 w - - 3 18`, depth 7, mate
  in 5. `#+5` in 17073 nodes, pv `a4a5 d8d7 a5b5 d7d8 b5b6 d8d7 b6b7 d7e6 e2d4`
  -- `Qxb7+` is the capture on the line. **Red under C02, C05 and R02**:

      TEST CASE:  pruning does not hide a forced mate
      FATAL ERROR: REQUIRE( result.mate_found ) is NOT correct!
        values: REQUIRE( false )
        logged: capture mate, 3krb1r/Np2pppp/3q1n2/8/Q4Bb1/2P3P1/P3NPBP/3RR1K1 w - - 3 18, depth 7, red under C02, C05 and R02

- `2b5/4k2P/2Bp1r2/Q3p3/ppp4q/P1P5/1P4P1/3R2K1 w - - 2 55`, depth 7, mate in 5.
  `#+5` in 7205 nodes, pv `a5c7 c8d7 c7d7 e7f8 d7e8 f8g7 e8g8 g7h6 h7h8q` --
  `Qxd7+` is the capture. **Red under R01**, same assertion, its own row logged.
- `3N1bk1/3Q3p/6p1/p3Bp1n/1p6/3P1P1P/1q5K/8 w - - 0 33`, depths 8 and 9, mate
  in 4. `#+4` in 8868 nodes, pv `e5b2 f8d6 d7d6 h5f4 d6d7 g8f8 d7f7` -- the key
  `Bxb2` and `Qxd6` are both captures, and python-chess reports the root **in
  check**, so the block is off at the root and live in every child. Red under
  R02 at both depths, measured in `.tuning/coord/S091_inproc_R02.txt`; in a
  whole-suite run the first row above fires first and aborts the case, which is
  what `REQUIRE` does.

**Each row is read at one depth and that is the position's own profile, not a
depth chosen to pass.** Under the case's own search -- `search_fen()` calls
`search()`, one fixed-depth `negamax_at` from a cold table, not iterative
deepening -- the mate distance is not monotone in depth: the first two are
reported at 7, not at 8, and again at 9, 10 and 11, which is what reverse
futility's ceiling does to a deep mate class (S148, DEC-158). The shipped
engine's `go depth N` reports these positions as centipawn scores at those
depths (the fast check drove it: `cp 1620`, `cp 1244`, `cp 1282` / `cp 1349`),
so the sentence is about the guard's search path, not the UCI reply; S098's
re-runs of this case inherit that reading. The sweep behind that statement is `.tuning/coord/S091_inproc_*.txt`,
400 positions of the two committed mate sets at depths 3 to 11, shipped and
under each mutant.

**One case was re-stated, not relaxed**, the way S109 re-stated three of the same suite's. "a capture is not reduced"
selected the first capture past the third move at perft position 2 and asserted
a reduction of zero; every capture past the third there is one the exchange
evaluation writes off, and those are reduced now. It selects the first capture
past the third **that the exchange evaluation clears**, asserts that
precondition, and drives `CAPTURE_POS` -- a row of `adocs/data/S018_raw.tsv`
found by scanning that corpus for a node with such a capture where the
reduction table would have reduced it. The assertion is unchanged and M07 still
kills it.

**Three mutants of other files were re-pointed, and each was re-observed.**
`M07_lmr_captures` and `M08_lmr_checks` in `tools/mutants/search.py` and
`P05_prune_gives_check` in `tools/mutants/S109_shallow_pruning.py` anchored
lines this step restructured, and an anchor that no longer occurs makes
`tools/mutation_check.py` refuse the **whole** run before it writes a byte. All
three now anchor the new shape and all three were re-run by hand: M07 red at
`a capture is not reduced` `REQUIRE_EQ( probe.reduction[k], 0 )` values
`( 1, 0 )`; M08 red at `a quiet move that gives check is not reduced`, same
assertion and values; P05 red at `a quiet move that gives check is not pruned`
`REQUIRE_EQ( rule_that_pruned(probe, checking), PRUNE_NONE )` values `( 1, 0 )`.
`validate()` over `tools/mutants/` reports **OK over 63 mutants** against this
tree.

`tools/mutation_check.py` wants a worktree at a commit holding the rule, so the
tool's own pass is the coordinator's after the commit --
`python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only
C02_capture_gives_check C05_capture_threshold_sign C06_capture_no_cap
C07_capture_first_move R01_extra_reduction_gives_check R02_extra_reduction_sign
M07_lmr_captures M08_lmr_checks P05_prune_gives_check`.

**What carries no mutant, and why it is said rather than left.** The extra ply's
`!MOVE_PROMOTED` clause mirrors late move reduction's own exemption. A scan of
45245 positions -- the S018 and S024 corpora plus one ply of children -- found
**no** node where a promotion the exchange evaluation writes off sits past the
third move at a reducible node, so a mutant dropping that clause would be one
nothing in the suite can kill. The clause stays because removing it is a silent
widening; it ships without a mutant and this paragraph is the record.

### Goldens re-derived (DEC-142)

- `tests/test_search_params.cpp` `golden_defaults`: 38 rows -> **41**, the three
  new parameters with their ranges. The derivation is `src/search_params.hpp`
  and the diff of the two is the re-derivation; `DEV_MANUAL.md`'s golden table
  row moved with it.
- `tests/test_search.cpp` "ordering keeps the tree small": the pair **440000 and
  20000 -> 69804 and 3490**, re-derived by `python3
  adocs/data/S192_node_budget.py`, which reports `count 17451 nodes`,
  `budget 69804 (4x the count)`, `floor 3490 (the count over 5)`. The count left
  the band outright -- below the floor -- which is the condition the case's own
  comment sets for re-deriving both ends. The ratios are unchanged; only the
  count moved. Readings so far: 109575 (2026-08-14), 179851 (S192), 17451 here.

  **An observation for the coordinator, not fixed here.** That script's "middle
  half" advisory can never be satisfied by a freshly derived band: with the band
  `[c/5, 4c]` the middle half starts at `c/5 + (4c - c/5)/4`, which is above `c`
  for every `c`. It read OUTSIDE at 109575 in 2026-08-14's band as well. The
  numbers it derives are right; the sentence it prints under them is not a
  criterion anything can meet.

### Debug self-play (DEC-141 clause 1)

`cmake --build build-debug -j12`, then four rounds at 4+0.04 on
`noob_3moves.epd` with `-log ... level=trace engine=true`: **8 games in 19 s, 0
`Assertion`, 0 `disconnect`**, in both the trace log and the tee'd stdout.
`.tuning/coord/S091_debug_selfplay.{log,out,pgn}`.

### Gate

`cmake --build build -j12 && ctest --test-dir build -L fast` **38 of 38**;
`cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast` **38 of
38**; `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` clean. Run again after
this section was written. `tools/gate_extra.sh` is the coordinator's.

### Documents

- `MANUAL.md`: three rows in the tune build's option table.
- `DEV_MANUAL.md`: the golden table's `golden_defaults` row 38 -> 41, the node
  budget row and the one prose sentence that quoted 440000.
- `specs.md` is the coordinator's; the wording this step proposes is in the
  report.

### The pre-registration

`adocs/data/S091_sprt.sh`, written and **not run**. `./fastchess.sh` at its own
default bounds, `elo0=0 elo1=5`, alpha = beta = 0.05, nElo, against `08461e0`,
the commit before this change; 8+0.08, Hash 16, `noob_3moves.epd`, concurrency
12, and `OUT` under `.tuning/` rather than the `/tmp` default this machine wipes
at boot. Worst case **41861 games / 19.8 h** with the truth at the interval's
midpoint and **25591 / 12.1 h** with it on a bound, at the 2110 games an hour
`.moltke.local.md` says to budget from. Abort at a time-forfeit rate over 1.0 %
a side; a crash voids. The three outcomes, the two-leg bisection on H0 with its
first suspect named -- `SeeLmrExtra`, the half whose published record is
negative -- and the open findings the run is taken while open (S225, S228, S229,
S213) are all in the header.

### The driver behind the per-rule sweep

`adocs/data/S091_rule_sweep.py` is what produced `S091_rule_sweep.txt`: four
`setoption` pairs -- `SeeCaptureMaxLmrDepth` 0 or 8 against `SeeLmrExtra` 0 or 1
-- over the tune build, the three `search_bench` positions at depths 9 and 12,
waiting for `bestmove` between them (`TOOLCHAIN.md`'s oracle section; the race
is chesso's too). Committed rather than left in scratch so the table above is
re-derivable when either end of it moves.
