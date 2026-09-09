id:         S098
goal:       the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
accepts:    an SPRT verdict per adjustment, measured separately -- history scaling, node type and the re-search rule are three changes and one at a time is the rule; every constant introduced goes into src/search_params.hpp with a stated range (S073), including the reduction table's own shape if it becomes a formula; the "pruning does not hide a forced mate" case re-run after each adjustment, since S013 shipped an LMR that reduced the mating move at the root; a mate found at the root is never reduced, asserted with the precondition that would otherwise reduce it; the fast suite green
touches:    src/search.cpp late move reduction, src/search_params.hpp, tests/test_search.cpp
excludes:   late move pruning, which is S109 -- S090 was retired into it by DEC-082, which measures the four shallow-depth rules as one step; the improving flag itself, which S108 supplies two entries earlier in the order (S092 retired into S108 by the 2026-08-19 review, `adocs/plan.md` "the improving flag, was first in the pending order"; no `decisions.md` entry records that merge) and which is an input here
decisions:  DEC-071, DEC-105, DEC-134
closes:
blocks:
paused_by:
done:

## Why it comes after the history steps

The refinement's largest single input is the move's history score, and the
tables it reads are queued ahead of it: S093 malus and gravity, then S024
continuation history. Scaling a reduction by a table that is about to change
means measuring it twice. The improving flag is S108's, ahead of it too.
Capture history is **not** an input here -- this engine reduces only quiets --
and S023 sits in the reserve tail (DEC-087); if it ever lands, reducing
tacticals with bad capture history is its consumer, back in this file's scope
at that time (Ethereal measured that consumer at +7.2/+2.4).

## Technical details (SOTA research, 2026-08-19)

Line numbers at `cf89e22`; re-locate by symbol if drifted. Every GitHub read
below was a PR body, release note or commit message, never a diff or source
file (DEC-016, DEC-084).

### 1. State of the art

**Value at stake**: Ethereal's removal ledger prices its LMR at **-249**,
second only to history (-759) — repo-recorded via DEC-087; the primary URL is
still untraced publicly (S091 hit the same wall). The modern architecture is
four layers on one log-log table.

**(a) The base table.** CPW LMR carries the form with named constants as
prose: Obsidian `0.99 + ln(depth)*ln(moves)/3.14`; Weiss `1.35 + lnln/2.75`
for quiets (`0.20 + lnln/3.35` captures); Ethereal `0.7844 + lnln/2.4696`
quiets; Senpai 1 ply for six moves then depth/3. The record for moving TO
this from a static reduction: Weiss #73 "Aggressive lmr" (fixed 1 ply → grows
with depth and moves tried) **+48.44 +/-15.81**, then #76 (adopt Ethereal's
log formula) **+20.28 +/-9.75** — both 2019-11, Weiss sub-3000 (1.2 = 3055
came in 2021). **Chesso already ships this layer** — build_lmr_table at
`src/search.cpp` `build_lmr_table` is exactly this form (S013, +129.2 +/-33.8
killed at
96 % LLR) — so layer (a) owes nothing here; its constants are S085/S127
material.
**(b) History scaling.** Reduce less for a quiet with high history, more with
low: `r -= clamp(history / divisor, -k, +k)`. Sub-3000 record: **Lynx #613**
"reduce less if history value is high", merged 2024-01-15 into v1.3.0 (Lynx
1.0.1 = 2432 on CCRL's 2024-01 recalc; DEC-087 pegs this era ~2600):
**+11.40 +/-7.30** at 8+0.08 — the one clean record in chesso's own band.
Above it: Weiss #451 "adjust LMR reduction by between +2 and -2 based on move
history" **+14.11 +/-7.57** (2021-06, ~3050), #452 more-aggressive +4.94,
#482 tweak +2.62/+4.79; Berserk f35db3a +3.11 (2021-05); Stash ae5c295
(2021-02, bench-only message). The mature input is the movepicker's sum —
Stockfish 37c2b56 scales by the "sum of first continuation history and main
history (similar to movepicker)". Negative control: clamping Lynx's history
term to [-1,1] resolved nothing (#971, +0.63 unresolved — the clamp is not
where the Elo is).
**(c) Node-type adjustments.** CPW's modern set: "Reduce less in PV-nodes",
"Reduce less when improving", "Reduce more in an expected Cut-node", "Reduce
more when hash move is a capture", less on killers/checks. Per adjustment:
- **Cutnode +1**: Lynx #1233 **+9.34 +/-4.75** (v1.8.0, 2024, high-2800s);
  Weiss #607 +2.27/+5.88 (2022-12, ~3300); a second ply on top failed at
  Lynx (**-17.62**, #1234); the prediction-plumbing fix alone was +1.77
  (#1304). Stash introduced cutNodes to *allow* LMR on them, +6.16 (bd9ecf5).
- **!improving +1**: Lynx #1135 merged (v1.8.0; +4.64 per S108's trace); the
  reduce-less-when-improving direction failed first (#1134) — the asymmetry
  is the published shape (CPW Improving says the same).
- **TT-move-is-capture +1**: Weiss #536 +3.33 LTC (2021-08, ~3100), extended
  to all moves at #666 (2023); Lynx #1529 **+1.87 +/-1.52** (v1.9.0) after
  three wrong or failed attempts (#706 no-op, #1241 -3.99, #1243 -8.59).
  Small everywhere.
- **PV**: reduce less, or start later. Weiss #71 "LMR later in pv nodes"
  **+3.78 +/-2.98** (2019-11, sub-3000); Lynx #1230 "increase pv min moves"
  merged (v1.8.0); Fruit Reloaded cuts the non-PV reduction by 2/3 at PV
  (CPW); removing Lynx's PV decrement failed (#779).
- **Not worth a term by the record**: killers (Weiss #665 simplified its
  killer adjustment away at ~3300; Lynx #2006/#2070/#2071/#2072 all
  neutral-negative), a per-move no-TT-move term (Lynx #2253 **-8.08** with
  IIR already present — S095 owns that signal at node level), "previous
  moves raised alpha" (Lynx #2089/#2105/#2106/#2121/#2399, ~0 or negative).
**(d) Post-re-search behaviour.** Two devices, and only the first is this
step's. **Do deeper / do shallower** — the verification depth responds to the
reduced search's score. Stockfish prose: 061f98a (2021-12) "with more
reduction, bigger fail-high is required"; 219fa2f (2022-11) do-shallower when
the score is "not too far from the current best search result"; 98965c1
doEvenDeeperSearch (+2 when "really really good"); 65e2150 (2023) re-bases
the margin on best value rather than alpha; a37b38b (2025) adds a
`d < newDepth` guard; 4d4c6eb/1047f84 (2025-12) simplify — alive at ~3600
through five years of churn. Near the band: **Lynx #1535** (v1.9.0, 2025-03)
measured the bare form at **-7.07 +/-7.65** and the guarded form at
**+3.11 +/-2.35** — the guard is load-bearing. Weiss #675 passed both
controls (+2.36/+3.65, 2023, ~3300) yet closed unmerged, no landed commit
traced. Negative-adjacent: Ethereal 12.29 "Use knowledge of
fail-highs/fail-lows to tweak LMR" was reverted wholesale as 12.30 (~3300).
**Post-LMR history updates** — bonus/malus to the reduced move's history from
the re-search outcome: Weiss #662 +2.61/+8.92, Stash 2138db2 +2.20, Berserk
4d94973 +1.40, SF 389e607 half bonus, Lynx #1758 near-zero at 100 k games.
All 3300+, and a history change rather than a reduction change — routed
onward, see Scope concerns.

**The honest split.** Sub-3000 evidence: the log table (banked), history
scaling (Lynx #613, ~2600), cutnode/improving/PV-min-moves (Lynx v1.8.0
high-2800s; Weiss 2019). Boundary (~3000-3100): TT-capture, deeper/shallower
(one guarded pass at Lynx v1.9.0). Defer as 3100+ refinements: fractional /
quantised reductions (Lynx #1512/#1514 +5.13), ttPv terms (Lynx #1476 +9.39 —
needs a TT PV-flag bit the entry does not carry), post-LMR history updates,
bad-re-search malus (Weiss #701), capture LMR (Weiss #356 +11.55 at ~3050,
Lynx #2256 +3.50, Stash 663ddbc +2.95 — this engine reduces only quiets, and
S023 sits in reserve), cutnode-with-no-TT-move (SF prose only; ~0 at Lynx).

### 2. Shape for chesso

The whole feature is `src/search.cpp` `build_lmr_table` and `src/search.cpp`
`lmr_reduction` and `src/search.cpp` `negamax`:

- - Table: build_lmr_table `src/search.cpp` `build_lmr_table`, `r =
  LMR_BASE/100 + ln(depth) * ln(move_number) / (LMR_DIVISOR/100)`, uint8_t,
  axes clamped 1..63, row 0 zero-initialised; `LMR_BASE 52` / `LMR_DIVISOR 182`
  (`src/search_params.hpp` `LMR_BASE` and `src/search_params.hpp`
  `LMR_DIVISOR`, ranges stated, both already in S085's SPSA set). CHESSO_TUNE
  rebuilds it per setoption (`src/search.cpp` `search_params_rebuild_derived`);
  a test probe exists (search_lmr_reduction_probe, `src/search.cpp`
  `search_lmr_reduction_probe` and `src/search.cpp` `lmr_reduction`).
- - Eligibility `src/search.cpp` `negamax`: `ply > 0 && depth >= 3 &&
  legal_moves_counter > 3 && !is_capture && !MOVE_PROMOTED && !is_in_check &&
  !is_check_move`. Root exempt (S013's mate bug), first three moves exempt,
  **PV not exempt** — chesso reduces at PV nodes from the same table.
  is_check_move (`src/search.cpp` `negamax`) is post-make and survives S107 for
  exactly this guard. The 3 and the 3 are hardcoded, not yet parameters.
- - Clamps `src/search.cpp` `negamax`: `int reduction` clamped to [0,
  child_depth - 1] — the child keeps one real ply; new signed terms ride the
  same variable and the same clamps.
- - Re-search structure `src/search.cpp` `negamax` (PVS): first legal move
  full-window; others zero-window at `child_depth - reduction`
  (`src/search.cpp` `negamax`); **reduced fail-high → zero-window re-search at
  child_depth** (`src/search.cpp` `negamax`), verdict 3's site; alpha < score <
  beta → full-window re-search, is_pv passed (`src/search.cpp` `negamax`).
  `best_so_far` (`src/search.cpp` `negamax`) is the fail-soft base the deeper
  margin compares to.
- - Node type today: the `is_pv` parameter alone (`src/search.cpp` `negamax`).
  No cutnode, no ttPv. `tt_move` is copied out at `src/search.cpp` `negamax` —
  the TT-capture term's input exists.
- History after S093/S024: signed butterfly + continuation sum in [-3M, +3M]
  through one probe path, all pre-make inputs. Read the **raw sum**, never
  score_move's banded return (S109's trap). Gravity's fixed range is what
  makes one divisor meaningful (S093 names this).
- improving after S108: `improving_at(state, ply, in_check)`; S109 is its
  first in-search call site, verdict 2 here its second.
- S109 consumes `lmr_depth = depth - lmr_reduction(depth, move_number)`
  pre-make. Every S098 term is pre-make computable (history by
  [colour][from][to] and moves_played; node type at node entry) — only the
  gives-check *exemption* is post-make — so one shared helper serves both
  sites.

**The three verdicts**, grouped by input and evidence:
1. **History scaling** — the largest sub-3000 record (+11.40) and the direct
   consumer of S093/S024; first, so verdicts 2 and 3 measure on the shipping
   history term.
2. **Node type** — cutnode + improving + TT-capture + the PV term as one
   verdict: all are per-node inputs to +/-1-class adjustments, the plumbing
   (a cut_node parameter) is shared, and each term sits behind its own
   off-valued constant so a failing verdict bisects by release rebuild (the
   S109 protocol) instead of spending four SPRTs on +1-class effects
   (DEC-063).
3. **Re-search rule** — deeper/shallower, last: the thinnest record below
   3100, and it composes with whatever 1 and 2 shipped. A zero is a live
   outcome, recorded as zero.

### 3. Implementation sketch

Verdict 1 — history:
1. 1. Factor `lmr_adjusted_reduction(...)`: raw table plus signed terms,
   clamped at the call sites exactly as `src/search.cpp` `negamax` today. With
   every new constant at its off value it returns the raw table — the property
   that makes the whole step inert-by-rebuild, S109's re-pointed gate included.
2. The term: `r -= clamp(hist_sum / LMR_HIST_DIV, +/-LMR_HIST_CLAMP)`,
   hist_sum through the S093/S024 probe path.
3. Re-point S109's `lmr_depth` to the helper, stated in the commit; S109 is
   **not** re-verdicted — its file and plan.md both say S098's SPRTs price
   the interaction and S127 refits the thresholds.
4. Tests red-first: helper unit test — driven table state, the high-history
   quiet reduced strictly less than the low-history one (direction pinned; a
   sign slip reduces the good quiets, silent); the accepts' root-mate case — a
   mate at the root that is late, low-history and quiet, precondition asserted
   (it would be reduced but for `ply > 0`), built the S033 way (python-chess
   enumeration + Stockfish confirmation, DEC-023); both mate cases re-run --
   "pruning does not hide a forced mate", `tests/test_search.cpp` "pruning does
   not hide a forced mate" and "pruning does not hide a mate against the
   material leader", `tests/test_search.cpp` "pruning does not hide a mate
   against the material leader"; fast suite. SPRT.

Verdict 2 — node type:
1. Thread `bool cut_node` through negamax per CPW Node Types (Garms's
   rules): root PV; first child of a PV node is PV, further children are
   scouted as CUT; first child of a CUT node is ALL, further children CUT;
   children of ALL nodes are CUT; the re-searched child of a PV node is PV
   (is_pv already carries that). The null-move child's label: CPW's Kannan
   paragraph covers it — re-read it at implementation rather than trusting a
   summary. Debug-assert `!(is_pv && cut_node)`.
2. Terms, each its own constant, off value 0: `+LMR_CUTNODE` when cut_node;
   `+LMR_NOT_IMPROVING` when not improving; `+LMR_TT_CAPTURE` when
   `tt_move != 0 && MOVE_CAPTURE(tt_move)` (the move itself is quiet by
   eligibility); PV either as `-LMR_PV` when is_pv or as a separate
   `LMR_MIN_MOVES_PV` threshold against the parameterised existing 3 —
   decide at implementation and record which (both forms published; the
   subtraction composes more simply).
3. Tests: a pure-function test of the child-type alternation on a fixed
   walk; per-term precondition tests — node counts move against the off
   value, exempt variants search identically; mate suites re-run. SPRT.

Verdict 3 — re-search rule:
1. 1. At `src/search.cpp` `negamax` the re-search depth becomes `child_depth +
   1` when the reduced score clears `best_so_far + LMR_DEEPER_MARGIN` (the
   65e2150 re-basing), `child_depth - 1` when it beat alpha by under
   LMR_SHALLOWER_MARGIN; gate the deeper path on a real reduction (`reduction
   >= 2` seed) — Lynx's bare form measured -7.07 and the guarded one +3.11, and
   SF scales the bar with the reduction (061f98a, a37b38b). Cap at child_depth
   + 1, floor at 1; the `src/search.cpp` `negamax` full-window re-search stays
   at child_depth — state that choice in the commit.
2. Tests: precondition test that the deeper path fires (node counts move
   against off = margin at range top) and never exceeds its cap or floor;
   mate suites re-run per the accepts. SPRT; a zero recorded as zero.

Consequences to carry: new search_params entries add UCI options to the tune
build — MANUAL.md's tune table and test_uci_surface / test_search_params
follow (S073). The table stays unsigned and untouched; terms apply at the
node, so search_params_rebuild_derived still covers only LMR_BASE/DIVISOR.

### 4. Constants and seeds

All in the `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with
stated ranges; every number below is a **seed — must be fitted/SPSA'd here
(S127)**; off values sit inside the declared ranges. Under DEC-105 each seed
is one of three forms and says which: **(a)** a value from a publication
about the technique, with its URL; **(b)** a derivation over chesso's own
data or scale; **(c)** the range midpoint or off value, stated as such. Where
a midpoint is not an integer this step takes the integer below it and says
so. Another engine's shipped coefficient is never a seed, wherever it is
republished — the wiki's Late Move Reductions page proposes no formula of its
own, only named engines' (fetched 2026-09-05,
https://www.chessprogramming.org/Late_Move_Reductions), which is DEC-105's
own PeSTO case. Those records are in section 1 and, as anti-seeds, in
section 5.

**Units, once for this file (P6).** A margin compared against `evaluate()` is
in chesso's material scale, `piece_value` in `src/eval_tables.hpp`: `PAWN`
94, `KNIGHT` 327, `BISHOP` 308, `ROOK` 487, `QUEEN` 716. The header's own
comment says the split between `piece_value` and `psqt_mg` / `psqt_eg` is
degenerate, so the material term alone is the unit. Plies have no unit.

- `LMR_BASE 52` / `LMR_DIVISOR 182` ship already, and they are **(b)** — the
  output of S085's SPSA run over chesso's own games. That is the only seed a
  re-sweep needs and no other is admissible. Section 1's record that the pair
  goes stale the moment verdict 1 ships still holds: that is S127's refit,
  not a mid-step re-sweep.
- `LMR_HIST_DIV` — **(b)**, chesso's own scale. A saturated history sum
  mapping to the full clamp gives `3M / LMR_HIST_CLAMP`, where `M` is
  `QUIET_HISTORY_MAX` (8192) — ~12288 at `LMR_HIST_CLAMP` 2. The 3 counts
  S024's two continuation tables alongside the quiet table: **before S024
  lands the sum is `[-M, +M]` and the divisor is `M / LMR_HIST_CLAMP`**,
  ~4096. Sweep. Off: range top.
- `LMR_HIST_CLAMP` **2** — **(b)**, a stated fraction of chesso's own
  reduction table. Read `search_lmr_reduction_probe(depth, move)` in
  `src/search.cpp` at the census median depth (11 at the S085 control, per
  the `RFP_MAX_DEPTH` comment in `src/search_params.hpp`; re-read at this
  step's HEAD) and the table's last move index 63: `0.52 + ln(11) * ln(63) /
  1.82` = 5.98, stored floored as 5. Half of that, rounded down, is **2** —
  history may move a late reduction by at most half of what depth and move
  number gave it. Re-derive the arithmetic when `LMR_BASE` / `LMR_DIVISOR`
  move. Fallback **(c)**: midpoint of 0..4 is 2, the same integer. Off: 0.
- `LMR_CUTNODE`, `LMR_NOT_IMPROVING`, `LMR_TT_CAPTURE`, `LMR_PV` **1** each —
  **(c) midpoint** of a 0..2 ply term, off 0. A ply count has no unit to
  derive from and no publication states one.
- `LMR_MIN_MOVES` **4** — **(b)**, chesso's own: the parameterised form of
  the `legal_moves_counter > 3` guard in `negamax` in `src/search.cpp`.
  `LMR_MIN_MOVES_PV` **6** — **(c) midpoint** of 4..8; off: equal to
  `LMR_MIN_MOVES`.
- `LMR_DEEPER_MARGIN`, `LMR_SHALLOWER_MARGIN` **47** each — **(c) midpoint**
  of a range stated by purpose, 0..`PAWN` (0..94): the top is one pawn, the
  point past which the re-search decision would be made on more material than
  a pawn of window. Midpoint **47**. Both off values sit inside it and they
  are opposite ends — `LMR_DEEPER_MARGIN` is off at the range top (the
  re-search never returns far enough above to search deeper) and
  `LMR_SHALLOWER_MARGIN` is off at 0. Compared against `evaluate()`, so the
  units above apply.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** DEC-019 lets a record say which
  direction is worth trying; DEC-105 forbids any of these numbers starting a
  sweep, which is why section 4 names no engine. Measured negative
  elsewhere: per-move no-TT-move (-8.08, Lynx #2253), a second cutnode ply
  (-17.62, #1234), killer terms (four neutral-negative Lynx PRs; Weiss
  removed its own), the reduced child reaching depth 0 (-25.77/-25.92, Lynx
  #2332/#2338). The coefficient pairs section 1 quotes from the wiki's LMR
  page — Obsidian 0.99/3.14, Weiss 1.35/2.75, Ethereal 0.7844/2.4696 — are
  those engines' tuned output republished, DEC-105's own PeSTO case: they are
  records of what a curve can look like, never a seed for chesso's.
- **The repo's own bug class.** S013's LMR reduced the mating move at the
  root; null move hid a mate in 2 (`tests/test_search.cpp` "search: draws", the
  "pruning does not hide a forced mate" case). The
  accepts re-runs the mate case per adjustment and adds the root assertion
  with its precondition. The root exemption is not up for relaxation —
  Lynx measured LMR on the root's first move at **-35.47** (#1771).
- **Reduction below zero / into quiescence.** Terms are signed now: history
  subtracts, node-type adds. Adjust in int, clamp to [0, child_depth - 1] —
  Lynx let the reduced child hit depth 0 and measured -25.77/-25.92; Stash's
  "don't reach depth 0" passed +3.14 (e35289f); clamp against the **child**
  depth (Lynx #2378 fixed an overflow from using depth). The deeper path
  adds the upper edge: cap at child_depth + 1.
- **Sign and band of the history input.** Positive history must shrink the
  reduction; read the raw S093/S024 sum, never score_move's banded value — a
  killer's 900000 would zero its reduction silently (S093's band hazard,
  S109's sibling trap).
- **Double-counting S091's SEE extra ply.** It **survives as an additive
  term with its own off constant** — the statement S091's file demands from
  this step; folding it into the formula would reopen S091's verdict. One
  see_ge result per quiet, shared (S091 and S109 both name it).
- **Double-counting S095's IIR.** S095 (after this step) reduces the node
  when its entry carries no move; a per-move no-TT-move term here duplicates
  that signal — Lynx measured the duplicate at -8.08 with IIR present.
  Verdict 2 carries no such term; S095's own SPRT prices the composition.
- **lmrDepth under S109 changes here.** With all constants off the helper
  equals the raw table, so the off-rebuild restores S109's gate exactly —
  one protocol covers "gate moved" and "term wrong". S109 is not
  re-verdicted; S127 refits its thresholds against the new distribution.
- **cut_node prediction bugs are silent** — Elo noise, no crash; Lynx's fix
  of one ZWS-prediction wrinkle was +1.77 (#1304). The debug assert and the
  alternation unit test are the guard.
- **Extensions later (S097).** The clamp target is child_depth; when S097
  extends the child, the clamp and the deeper/shallower cap must follow the
  extended depth — S097's edit, noted there. SF's e4e61cd records the
  doDeeper-times-extension coupling as real machinery.
- **Tune-build strength numbers are forbidden** (S073); ablation is off
  constants plus a release rebuild.

### 6. Measurement

Three SPRTs at the S105 regime (8+0.08, Hash 16, UHO book), gainer bounds
`elo0=0 elo1=5`, in order V1 → V2 → V3, each against the commit before it.
Fast suite plus both mate cases green first per verdict, new tests observed
red first with printouts recorded. Node counts move by construction — INV-6
takes the SPRT path all three times; search_bench depths 9/12 recorded per
verdict in the stamp.

- Expectations (DEC-019: direction only): V1 +5..+15 (seeds +11.4, +14.1);
  V2 +3..+10 (+9.3 cutnode, +4.6 improving, +1.9..+3.3 ttcapture); V3 0..+3
  — the {0,5} pair does not straddle a +3 truth (DEC-063), so a
  random-walked V3 is terminated and recorded as zero, and dropping the
  feature is the default at zero (keeping it needs a stated reason —
  S005/S006/S015 precedent).
- Failure bisection: **V1** → sign first, then divisor scale (the clamp is
  not the suspect: Lynx #971 ~0); the off rebuild separates the term from
  the S109 gate re-point. **V2** → off-value halves {ttcapture, PV} before
  {cutnode, improving} (weakest records first), and re-check the cut_node
  alternation before blaming any delta — a wrong prediction is noise.
  **V3** → the guard (bare deeper/shallower measured -7.07 at Lynx; the
  guarded form +3.11), then the margin.

### 7. Interactions

- **S093/S024 (inputs, before)**: the signed sum through one probe path;
  gravity's fixed range is the stable denominator; verdict 1 is measured
  against the post-S024 sum by plan order.
- **S109 (consumer, before)**: gate re-pointed to the shared helper in V1;
  not re-verdicted; off values restore its exact today-gate; S127 refits.
- **S091 (before)**: the SEE extra ply survives additive under its own off
  constant; one see_ge per quiet, result shared.
- **S095 (after)**: node-level no-TT-move reduction; no per-move duplicate
  here (Lynx -8.08 is the duplicate, measured).
- **S097 (later)**: extension-vs-clamp note above; the doDeeper/extension
  coupling is singular-extension machinery, not this step's.
- **S107 (before)**: is_check_move survives for the LMR guard alone; verdict
  2 does not change the exemption set (Scope concerns carries the one
  candidate).
- **S127**: LMR_BASE/LMR_DIVISOR refit against the adjusted reduction
  (Weiss #481 +7.37 is the record that they go stale), plus the ~8 new
  constants; re-tries priced there: fractional LMR (Lynx #1512/#1514
  +5.13), post-LMR history updates (Weiss #662 +2.61/+8.92, Stash 2138db2
  +2.20, Berserk 4d94973 +1.40, SF 389e607), bad-re-search malus (Weiss
  #701), capture LMR (Weiss #356 +11.55, Lynx #2256 +3.50, Stash 663ddbc
  +2.95 — re-decides the quiets-only premise), the in-check allowance below.

### Scope concerns

1. 1. **The in-check/gives-check exemptions have a contrary sub-3000 record.**
   Lynx allowed LMR while in check at ~2700 (#702, v1.5.0) and re-forbidding it
   later failed at -5.60 (#1800); Stash reduces all moves (663ddbc). Chesso
   exempts both (`src/search.cpp` `negamax`). The goal names three scalings and
   no eligibility change, so this step keeps the exemptions — the repo's mate
   history argues the same — but the record is on file: relaxation is a
   candidate fourth verdict or S127-era step, and it would change S107's
   "needed for the LMR guard alone" statement.
2. **Post-LMR history updates are routed onward, not smuggled in.** S093 and
   S024 both defer them "to S098", but they are a history-update change
   triggered by the re-search, not a reduction change, and every record is
   3300+ (+1.4..+2.6 STC class). Bundling them into verdict 3 would put two
   changes under one SPRT. They are S127-era material or their own later
   step; records in section 1(d).
3. **Verdict 3's evidence below 3100 is one guarded pass at the boundary**
   (Lynx #1535, v1.9.0) plus an unmerged Weiss pass, with Ethereal's revert
   (12.29/12.30) on the negative side. The three-verdict structure stands;
   V3's expectation is set accordingly.

### 8. References

- - https://www.chessprogramming.org/Late_Move_Reductions — formula forms with
  named constants (Obsidian, Weiss, Ethereal, Halogen, Senpai, Fruit Reloaded),
  exemption list, the modern adjustment set, re-search depth adjustment noted
  as recent Stockfish practice.
- - https://www.chessprogramming.org/Node_Types — Knuth types; Garms's
  prediction rules (root PV; first child of PV is PV, others CUT; first child
  of CUT is ALL, others CUT; children of ALL are CUT); Kannan on re-search and
  null-move labels.
- - https://github.com/lynx-chess/Lynx/pull/613 — history in LMR, +11.40
  +/-7.30 at 8+0.08, merged 2024-01-15 (v1.3.0, ~2600 era): verdict 1's
  sub-3000 record.
- -
  https://api.github.com/search/issues?q=repo:lynx-chess/Lynx+LMR+in:title+type:pr
  — #1233 cutnode +9.34 / #1234 -17.62; #1135 !improving (#1134 inverse
  failed); #1529 TT-capture +1.87 (#706 no-op, #1241/#1243 failed); #1230 PV
  min moves; #1304 prediction fix +1.77; #1535 deeper/shallower -7.07 bare /
  +3.11 guarded; #1512/#1514 fractional +5.13; #1476 ttPv +9.39; #2256
  not-only-quiets +3.50; #971 clamp ~0; #2253 no-TT-move -8.08; #2332/#2338
  depth-0 -25.77/-25.92; #2378 newDepth overflow fix; #702/#1800 in-check;
  #1771 root -35.47; #2006/#2070/#2071/#2072 killers ~0;
  #2089/#2105/#2106/#2121/#2399 alpha-raises ~0; #1758/#2346/#2347 post-LMR
  conthist.
- -
  https://api.github.com/search/issues?q=repo:TerjeKir/weiss+LMR+in:title+type:pr
  — #73 +48.44 static→scaled (2019-11); #76 +20.28 Ethereal log formula
  (2019-11); #71 PV later +3.78; #451 history +/-2 +14.11 (2021-06); #452
  +4.94; #482 +2.62/+4.79; #481 steeper post-history +7.37; #536/#666 ttcapture
  +3.33/+2.87; #607/#686 cutnode +2.27/+5.88; #675 do-deeper +2.36/+3.65 passed
  both, closed unmerged; #662 post-LMR conthist +2.61/+8.92; #701 bad-re-search
  malus; #665 killers simplified away; #540 root later +3.88.
- - https://api.github.com/search/commits?q=repo:mhouppin/stash-bot+LMR —
  ae5c295 history in LMR (2021-02); bd9ecf5 introduce cutNodes +6.16; e35289f
  no depth 0 +3.14; 663ddbc LMR on all moves +2.95; 3e38519/1c8d87e/fecff93
  log-formula tunes; 2138db2 post-LMR conthist +2.20; ad17dbd smaller-on-PV
  retune; 27d338f/3801c54 root LMR.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+doDeeperSearch
  — 061f98a threshold scales with reduction; 98965c1 doEvenDeeper; e4e61cd
  doShallower/extension interplay; f17db46, 4d4c6eb, 1047f84 simplifications;
  a37b38b d<newDepth guard.
- -
  https://api.github.com/search/commits?q=repo:official-stockfish/Stockfish+%22do+shallower%22
  — 219fa2f do-shallower, "not too far from the current best search result".
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+LMR+history —
  f35db3a history LMR +3.11 (2021-05); 4f353c3 tactical-history LMR +2.26 (S023
  consumer); f783f92 tweak +2.51 LTC; 4d94973 post-LMR conthist +1.40; c6e4d55
  early bundled history reduction.
- - https://api.github.com/repos/AndyGrant/Ethereal/releases — V12.50 notes:
  "12.29: Use knowledge of fail-highs/fail-lows to tweak LMR" / "12.30: Revert
  all of the changes from V12.29"; V10.55 "10.06: Rewrite the Late Move
  Reductions from scratch"; V9.65 "9.33: Break LMR researches into 3 steps for
  PV nodes". The -249 removal ledger stays repo-recorded via DEC-087; primary
  URL untraced in this pass too.
- - https://api.github.com/repos/lynx-chess/Lynx/releases — v1.3.0 (2024-02,
  #613), v1.5.0 (2024-06, #702/#706), v1.8.0 (2024-12, #1135/#1233/#1230),
  v1.9.0 (2025-03, #1476/#1529/#1535/#1514/#1512/#1304), v0.7.0 (2021-11, #96
  LMR added).
- - https://kirill-kryukov.com/chess/discussion-board/viewtopic.php?t=13343 —
  Lynx on CCRL; 1.0.1 recalculated to 2432 (2024-01): the band anchor for #613.
- - Stockfish 37c2b56 (statScore sum in LMR), 389e607 (post-LMR updates, half
  bonus), d37de3c (TC-sensitivity) — commit messages re-cited from S024's
  research.
