id:         S091
goal:       skip captures the exchange evaluation says lose material, in the main search rather than in quiescence alone, and reduce a negative-SEE move by an extra ply
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); separate margins for captures and for quiets, both constants in src/search_params.hpp with stated ranges (S073), and the depth scaling stated as what it is rather than as a flag; a position with a forced mate inside the pruned depth added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guard removed; nothing is pruned at a PV node or while in check, with the precondition asserted; see() and see_ge() are unchanged and tests/test_search.cpp's exchange cases still pass unmodified; the fast suite green
touches:    src/search.cpp negamax move loop, src/search_params.hpp, tests/test_search.cpp
excludes:   any change to see() or see_ge() themselves; quiescence, where see_ge already declines losing captures at `src/search.cpp` `quiescence`; delta pruning, which is S022
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

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
