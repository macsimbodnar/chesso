id:         S098
goal:       the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
accepts:    an SPRT verdict per adjustment, measured separately -- history scaling, node type and the re-search rule are three changes and one at a time is the rule; every constant introduced goes into src/search_params.hpp with a stated range (S073), including the reduction table's own shape if it becomes a formula; the "pruning does not hide a forced mate" case re-run after each adjustment, since S013 shipped an LMR that reduced the mating move at the root; a mate found at the root is never reduced, asserted with the precondition that would otherwise reduce it; the fast suite green
touches:    src/search.cpp late move reduction, src/search_params.hpp, tests/test_search.cpp
excludes:   late move pruning, which is S109 -- S090 was retired into it by DEC-082, which measures the four shallow-depth rules as one step; the improving flag itself, which S108 supplies two entries earlier in the order (S092 retired into S108 by the 2026-08-19 review, `adocs/plan.md` "the improving flag, was first in the pending order"; no `decisions.md` entry records that merge) and which is an input here
decisions:  DEC-071, DEC-105, DEC-134, DEC-198
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator for verdict 1 (DEC-185, DEC-199); the SPRTs are the coordinator's; started 2026-09-15 08:03 on the idle machine
done:

## Why it comes after the history steps

The refinement's largest single input is the move's history score, and the
tables it reads are queued ahead of it: S093 malus and gravity, then S222's
fitted continuation history -- S024's unfitted table measured H0 and was
reverted (DEC-194), and S222 sits directly before this step since DEC-198.
Scaling a reduction by a table that is about to change
means measuring it twice. The improving flag is S108's, ahead of it too.
Capture history is **not** an input here -- this engine reduces only quiets --
and S023 sits in the reserve tail (DEC-087); if it ever lands, reducing
tacticals with bad capture history is its consumer, back in this file's scope
at that time (Ethereal measured that consumer at +7.2/+2.4).

## Amended 2026-09-12, DEC-198: read S222 where this file says S024

S024 built the one-ply table on plain history's scale, measured H0 and was
reverted the same day (DEC-194). The table returns as S222 with a scale of its
own, fitted in a narrow lane that also fits plain history's six coefficients
-- never fitted until then -- and S222 sits directly before this step, so
every "S093/S024" below reads S093/S222. Two figures move with it: the sum
this step reads saturates at plain history's bound plus S222's own weighted
bound, not at three times one bound as the two-table arithmetic below
assumes, and `LMR_HIST_DIV`'s natural seed is that saturated sum over
`LMR_HIST_CLAMP`; the two-ply table is S222's follow-up and lands after this
step, when the divisor is re-swept with it. The three verdicts and their
order are unchanged.

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
low: `r -= clamp(history / divisor, -k, +k)`. Sub-3000 record, and it is the layer that survives S181's
re-banding: **Lynx #613**
"reduce less if history value is high", merged 2024-01-15 between v1.2.0 (not
on the CCRL list) and v1.3.0, the nearest rated release before it being v1.1.0
— so the band is **2420 to 2653** and the release it shipped in rates
**2653** (`adocs/data/S181_lynx_bands.md`; DEC-087's "~2600" for this era holds):
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
- **Cutnode +1**: Lynx #1233 **+9.34 +/-4.75** (merged 2024-12-05, between
  v1.7.0 and v1.8.0, band **3119 to 3138** — *not* the "high-2800s" this
  file read until 2026-09-11, which was low by about 300; S181);
  Weiss #607 +2.27/+5.88 (2022-12, ~3300); a second ply on top failed at
  Lynx (**-17.62**, #1234); the prediction-plumbing fix alone was +1.77
  (#1304). Stash introduced cutNodes to *allow* LMR on them, +6.16 (bd9ecf5).
- **!improving +1**: Lynx #1135 merged 2024-10-31, band **3119 to 3138**
  (+4.64 per S108's trace); the
  reduce-less-when-improving direction failed first (#1134) — the asymmetry
  is the published shape (CPW Improving says the same).
- **TT-move-is-capture +1**: Weiss #536 +3.33 LTC (2021-08, ~3100), extended
  to all moves at #666 (2023); Lynx #1529 **+1.87 +/-1.52** (merged 2025-03-04, band
  **3138 to 3224**) after
  three wrong or failed attempts (#706 no-op, #1241 -3.99, #1243 -8.59).
  Small everywhere.
- **PV**: reduce less, or start later. Weiss #71 "LMR later in pv nodes"
  **+3.78 +/-2.98** (2019-11, sub-3000); Lynx #1230 "increase pv min moves"
  merged 2024-12-08, band **3119 to 3138**; Fruit Reloaded cuts the non-PV reduction by 2/3 at PV
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
through five years of churn. Not near the band after all: **Lynx #1535**, merged 2025-03-05, band
**3138 to 3224** (S181),
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

**The honest split, redrawn by S181 on 2026-09-11
(`2026-09-04_plan_review-F02`).** Every Lynx band here is now the CCRL Blitz
1CPU range its pull request merged between, from `adocs/data/S181_lynx_bands.md`, and one grouping
left this heading as a result.

**Sub-3000 evidence**: the log table (banked); history scaling (Lynx #613,
**2420-2653**, shipped at 2653); reducing later at PV nodes *as a direction*,
from Weiss #71 in 2019 when Weiss was sub-3000. **That is all of it.**

**Boundary and above, 3119 to 3224, where this file read high-2800s until
2026-09-11**: cutnode +1 (Lynx #1233, **3119-3138**), !improving +1 (#1135,
**3119-3138**), Lynx's own PV-min-moves patch (#1230, **3119-3138**),
TT-capture (#1529, **3138-3224**), deeper/shallower (#1535, **3138-3224**).

**Does the order of the three verdicts change? No — and the reason is not
the bands.** (b) history scaling stays first: it is the only layer with
evidence in this engine's own band, and it is the layer whose family the
Ethereal ledger prices highest by proxy (late move reduction, **-248.59** on
removal). (c) node type and (d) post-re-search follow in that order, unchanged,
because (d)'s device reads the re-search (c)'s reductions produce and cannot be
measured before it exists. What changes is what this file may *claim*: (c) and
(d) rest on 3100-band evidence, so a zero from either is an expected outcome
rather than a surprise, and neither may be argued for on "it worked below
3000". DEC-176 records that ruling and that the step stays whole in the main
order.

Defer as 3100+ refinements: fractional /
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
1. **History scaling** — the largest sub-3000 record (+11.40, at a banded
   2420-2653) and the direct
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
  +/-7.30 at 8+0.08, merged 2024-01-15, banded **2420-2653** and shipped in
  v1.3.0 at **2653** (`adocs/data/S181_lynx_bands.md`): verdict 1's
  sub-3000 record, and the only one this file has.
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

## Verdict 1 landed, 2026-09-15: the history scaling

Implemented by an Opus 5 subagent briefed by the coordinator (DEC-185,
DEC-199). **No `done:` stamp: the step completes after verdict 3.** The SPRT
is the coordinator's and is pre-registered as `adocs/data/S098_v1_sprt.sh`
before any game.

### The rule, and where it is clamped

`lmr_adjusted_reduction`, which lived in `src/search.cpp` until DEC-213
removed it, was the raw table plus one signed term:

```
r = lmr_reduction(depth, move_number) - clamp(hist_sum / LMR_HIST_DIV,
                                              +/-LMR_HIST_CLAMP)
```

`hist_sum` is `src/evaluation.hpp` `quiet_history_sum`, factored out of
`src/evaluation.cpp` `score_move` so the ordering and the reduction read one
number and cannot disagree about what a move's history is: the raw butterfly
entry plus S222's weighted continuation entry, **never** `score_move`'s return,
whose killer and countermove bands would saturate the clamp for a reason that
is not history at all.

The helper is **unclamped against the depth** and the call sites clamp, exactly
as `src/search.cpp` `negamax_at` clamped the raw table before it existed:

| site | clamp | history it reads |
|---|---|---|
| the reduction in `src/search.cpp` `negamax_at` | `[0, child_depth - 1]`, S091's extra ply added first | the move's own sum |
| the shallow-depth gate `src/search.cpp` `lmr_depth_of` | at 0 from below | the move's own sum at the three quiet rules; `NO_HISTORY_SUM` at late move pruning, which decides before a move is picked, and at S091's capture rule, where no history table has an entry for the move |

The sum is read **once per quiet, pre-make**, in `src/search.cpp` `negamax_at`:
the butterfly table is indexed by the side to move, so a read after
`make_move` would score the move against the opponent's half of the table.
`search_node_probe_t` in `src/data_structures.hpp` gains `hist_sum`, so a case
can assert the number the node decided on rather than the number the table
holds after the drive -- the node's own children write history as the loop
runs.

**S109's `lmr_depth` is re-pointed to the same helper** and S109 is not
re-verdicted: its file and `adocs/plan.md` both say S098's SPRT prices the
interaction and S127 refits its thresholds. With the term off the helper
returns the raw table, so one release rebuild separates the term from the
re-point -- and that is measured, not argued: at `LmrHistClamp 0` the depth-14
bench is `5685915`, the parent's signature exactly.

### The two constants, their seeds and their DEC-105 forms

The band both are derived from is `QuietHistoryMax + ContHistWeight *
CONT_HIST_BOUND / 100` = 8831 + 8519 = **17350** at the values S222 fitted.

| constant | default | range | form |
|---|---|---|---|
| `LMR_HIST_CLAMP` | 2 | 0 to 4 | **(b)** a stated fraction of chesso's own reduction table: `src/search.cpp` `search_lmr_reduction_probe` at the census median depth 11 and the last move index 63 is `0.52 + ln(11) * ln(63) / 1.82` = 5.98, floored to 5, and half of that rounded down is 2. Fallback **(c)**: the midpoint of 0 to 4 is the same integer. Off: 0, inside the range, which is what the bisection rebuild needs |
| `LMR_HIST_DIV` | 8675 | 1 to 34700 | **(b)** a derivation over that band: a saturated sum maps to exactly the full clamp, 17350 / 2. Floor arithmetic, a divisor. Range top twice the saturated sum, a second off value: no sum the band admits divides to anything there |

No engine's coefficient seeds either (DEC-084 as amended by DEC-105). The
records that say the technique is worth trying -- Lynx #613 +11.40 +/-7.30 in
this engine's own band, Weiss #451 +14.11 +/-7.57 above it -- are records and
not seeds, and section 5 above already lists them as anti-seeds.

**At the shipped divisor the clamp is reached and never exceeded**, by
construction. It is kept, and the reason is stated rather than assumed: the
tune build sweeps the divisor down, S127 refits it, and a two-ply continuation
table widens the band the saturated sum is computed from. The case "the
history term never moves the reduction by more than its clamp", in
`tests/test_search.cpp` until the removal took it with the term, fed the helper
sums from outside the band for exactly that reason.

### What the term actually does, measured before any game

The rule fires only when the sum crosses half its band, and a history entry
reaches half its band only after many cutoffs on the same move. So it is
**inert in a shallow search** and that is a property of the seed, not a wiring
fault. Measured on the tune build by the one ablation that build is allowed --
node counts, never strength (S073) -- `bench <depth>` at the shipped clamp
against the same binary at `LmrHistClamp 0`:

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 607842 | 935568 | 1634017 | 2364249 | 3798722 | 5968045 |
| off | 607842 | 935536 | 1634008 | 2364815 | 3849812 | 5685915 |
| delta | +0.00 % | +0.00 % | +0.00 % | -0.02 % | -1.33 % | +4.96 % |

The elbow is at depth 13. `bench` (depth 14) moves **5685915 -> 5968045,
+4.96 %**, and `tools/search_bench.py` at depth 9 does not move at all --
51189 / 146616 / 39389 on both trees, best moves `c3d5` / `e2a6` / `d7c8q` --
while at depth 12 only midgame moves, 143205 -> 141782, the other two and all
three best moves unchanged. The two instruments are complementary exactly as
DEC-140 says, and here the signature is the one that sees the change. INV-6
therefore takes the SPRT path.

### Tests, red first, with the printouts

Six cases in `tests/test_search.cpp` and one in `tests/test_search_params.cpp`.
The red observation was taken on the shipped code at `LmrHistClamp 0` -- its
off value -- which is both a red-first observation and the inert-at-off
property the bisection rests on. Verbatim, release build:

```
TEST CASE:  the history term never moves the reduction by more than its clamp
FATAL ERROR: REQUIRE( LMR_HIST_CLAMP > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

TEST CASE:  the shallow-depth gate reads the adjusted reduction
ERROR: CHECK( search_lmr_depth_probe(CONT_HIST_REF_DEPTH, 63, sat) > search_lmr_depth_probe(CONT_HIST_REF_DEPTH, 63, -sat) ) is NOT correct!
  values: CHECK( 6 >  6 )

TEST CASE:  a quiet the history tables like is reduced less
FATAL ERROR: REQUIRE_EQ( probe.reduction[k], raw - 1 ) is NOT correct!
  values: REQUIRE_EQ( 2, 1 )

TEST CASE:  a quiet the history tables have written off is reduced more
FATAL ERROR: REQUIRE_EQ( probe.reduction[k], raw + 1 ) is NOT correct!
  values: REQUIRE_EQ( 2, 3 )

TEST CASE:  the reduction reads the raw history and not the ordering band
FATAL ERROR: REQUIRE( LMR_HIST_CLAMP > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

[doctest] test cases:   6 |   1 passed | 5 failed | 119 skipped
```

The one that passes at the off value is "the history term is inert at a sum of
zero", which is the property case and is *supposed* to hold there. The root
case passes there too and is red under its own mutant instead.

### The mutants, and the case that killed each

`tools/mutants/S098_lmr_history.py`, prefix `L`, six mutants, **every one run
by hand against the working tree and observed** -- the pass could not go
through `tools/mutation_check.py`, which requires a linked worktree with a
clean `src/` and reverts with `git checkout --`, and this verdict is not
committed yet. Each was applied, built, run through the whole fast suite in
the release build and reverted from a byte snapshot whose sha256 was compared
after; `src/` is byte-identical to the landing, and the pass was run twice --
once when the cases were written and once on the tree that lands, after the
inline move below -- with the same kills both times. **A hand-driven pass
reverts the source and not the build**: the binary in `build/` is the last
mutant's until something rebuilds, which is a trap for any bench read by hand
afterwards and was caught here by re-reading the signature.

| mutant | what it breaks | killed by, with the values it printed |
|---|---|---|
| `L01_lmr_history_sign` | the term is added, not subtracted | "a quiet the history tables like is reduced less" `REQUIRE_EQ( 3, 1 )`; "... written off is reduced more" `REQUIRE_EQ( 1, 3 )`; "never moves the reduction by more than its clamp" `REQUIRE( 31752 == 0 )`; "the shallow-depth gate reads the adjusted reduction" `CHECK( 4 > 8 )` |
| `L02_lmr_history_no_clamp` | the clamp is dropped | "the history term never moves the reduction by more than its clamp" `REQUIRE( 7938 == 0 )` |
| `L03_lmr_gate_unscaled` | the S109 gate goes back to the raw table | "the shallow-depth gate reads the adjusted reduction" `REQUIRE( 15840 == 0 )` |
| `L04_lmr_reduction_unscaled` | the reduction goes back to the raw table | "a quiet the history tables like is reduced less" `REQUIRE_EQ( 2, 1 )`; "... written off is reduced more" `REQUIRE_EQ( 2, 3 )` |
| `L05_lmr_history_banded` | `score_move`'s band is divided instead of the raw sum | "the reduction reads the raw history and not the ordering band" `REQUIRE_EQ( 900000, 0 )` |
| `L06_lmr_root` | the root exemption is dropped | "a mate found at the root is never reduced" `REQUIRE( 1 == 0 )`, and seven more mate cases -- eight in all across `test_search`, `test_engine` and `test_mate_breadth`, "mate in two is found at the right distance" among them |

Each run reddened `test_search` and nothing else, except `L06`, which reddened
three binaries.

### S091's six mutants re-run, and the `capture_mates` golden re-derived

DEC-142: a golden is re-derived by its own script whenever **either** end
moves, and this verdict moves the engine end of one -- `tests/test_search.cpp`
`capture_mates`, whose four depths and four mutant labels are measurements
taken through the reduction. Seven sweeps of
`adocs/data/S230_mine_r01_row.py depths` over
`adocs/data/S230_table_fens.txt`, depths 3 to 12, once on this tree and once
per mutant of `tools/mutants/S091_capture_see.py` applied by hand and reverted
from a byte snapshot.

**The four depths survive; three of the four labels did not.** Shipped
profiles: `d7 d9 d10 d11 d12`, `d7 d8 d9 d10 d11 d12` (row 2 gained d8),
`d9 d10 d11 d12`, `d10 d11 d12` (row 4 lost d9). Every row still reports its
mate at the depth it claims, which is what the case asserts and what the green
suite says. Under them:

| row | depth | label at S230 | measured at S098 |
|---|---|---|---|
| 1 | 7 | C02 and C05 | unchanged |
| 2 | 7 | C02, C05 and R02 | **C02** -- C05 and R02 report the mate at 7 now |
| 3 | 9 | no S091 mutant, since S222 | **R02** -- it reports `d10 d11 d12` |
| 4 | 11 | C02 and R01 | **R02** -- C02 and R01 both report `#+5` at 11 now |

**R01's incidental kill is gone again**, which is the whole of what S230 went
mining for two days ago. Stated rather than papered over, and it is not a hole:
all six S091 mutants were then run through the **whole fast suite** on this
tree and every one died -- C02 at "a capture that gives check is not pruned",
C05 at "capture SEE pruning skips the captures that lose material and no
others", C06 at "capture SEE pruning stops at its depth cap", C07 at "a node
whose only legal move is a losing capture is never pruned", R01 at its own
direct guard "a capture that gives check is not reduced" and nowhere else, R02
at four cases including three of this step's. An incidental second kill is
measured in a tree that moves under every ordering change; the direct guards
are what the rules rest on.

### The speed cost, and the one thing done about it

Ten interleaved `bench` pairs against the parent binary built from `1db5b8e`
with the same compiler, arch and build type read **-4.81 % nodes per second**
(3815942 against 3632322, the two groups not overlapping) with
`quiet_history_sum` defined in `src/evaluation.cpp`. There is no LTO in this
build, so that definition is a call per quiet **scored** -- `score_move` calls
it for every quiet in every move list -- as well as a call per quiet searched.

**The definition moved into `src/evaluation.hpp` as an `inline`**, which is
behaviour-neutral by construction and proved by the signature rather than
argued: the bench total is `5968045` either way and `tools/search_bench.py`
reads the same counts and the same best moves at both depths. Eight interleaved
pairs then read **-2.80 %** (3813164 against 3706415), and the groups overlap,
which is where this machine stops calling a difference real (CLAUDE.md, rule 5).
What is left is not all the read either: the candidate searches a 4.96 % larger
and differently shaped tree, and nodes per second is a property of the shape.
No further guard was added -- an `is_in_check` conjunct on the read would be a
second copy of two consumers' preconditions for a saving this measurement
cannot see.

`tools/mutants/S222_continuation_history.py`'s `H04` anchor followed the
definition into the header, and its note says so.

### The root-mate case, and its oracle

The accepts' clause: "a mate found at the root is never reduced, asserted with
the precondition that would otherwise reduce it". `tests/test_search.cpp` "a
mate found at the root is never reduced" drives the root at depths 3 to 6 on
`6qk/7p/2p2p1B/4R2P/4P1Q1/1p4P1/5P2/6K1 w - - 1 43`, the position "pruning does
not hide a forced mate" already mines for the same class -- the key is a late,
quiet, hanging rook move.

Oracle re-run rather than quoted, through `chess.engine.SimpleEngine`
(TOOLCHAIN.md's safe form, never a printf pipe): stockfish depth 20 reports
**`#+2` in 1918 nodes, pv e5e8 g8e8 g4g7**. python-chess reports `is_valid()
True`, `is_check() False`, 36 legal moves of which 1 is a capture and none a
promotion, `is_attacked_by(BLACK, E8) True`, and the only two quiet moves that
give check are Bg7+ and Qg7+ -- not the key.

Every condition of `may_reduce` except `ply > 0` is asserted at each depth: the
key is quiet, gives no check, its history sum is 0 before the drive, the node
returns a mate score, its index is 10 and so past the move-number bound, and
`search_lmr_adjusted_reduction_probe` at that index and the sum the probe
recorded is positive. The reduction is 0.

### Two S109 cases moved, and neither was relaxed

`tests/test_search.cpp` "quiet SEE pruning skips the quiets that lose material
and no others" went red. Its plant was the history band's own edge, used only
to order two quiets to the top of their stage -- and since the gate reads
history, a saturated plant moved the rule's own margin as well as the order, so
the case was asserting a rule it had stopped isolating. The plant is now 2 and
1, which order the two moves identically (every other quiet at that node sits
at 0) and divide to nothing, so the case's own sentence -- "the plant moves
where the two sit in the order and nothing else" -- is true again. The case's
assertions are unchanged.

`tests/test_search.cpp` "history pruning skips the quiet the table has written
off" stayed green, and its local restatement of `lmr_depth_of` now carries the
term and is passed the value it planted, so its preconditions are stated at the
depth the engine uses. That restatement gained a defaulted `hist_sum` of 0,
which is what the five other call sites read.

### The suite, the signature and the second tier

- Fast suite, release: **39/39**. Fast suite, tune build: **39/39**.
  `./clang-format.sh --check` clean.
- `Bench: 5968045`.
- `tools/search_bench.py` depth 9 and 12 above; best moves unchanged at both.
- Debug self-play, DEC-141 clause 1, four rounds at 4+0.04 with the Debug
  binaries and `level=trace engine=true`: **8 games in 21 s, 0 `Assertion` in
  both the log and the tee'd stdout, 0 `disconnect`**.
- `tests/test_search_params.cpp`'s golden gains two rows and its count moves
  44 -> 46, re-derived the way its own GOLDEN note says -- by diffing it
  against `src/search_params.hpp`, which is its derivation.
- `MANUAL.md`'s tune-option table gains `LmrHistDiv` and `LmrHistClamp`;
  `DEV_MANUAL.md`'s bench ledger gains this verdict with the depth sweep and
  the off-value check, and its DEC-142 golden list moves `golden_defaults` from
  44 to 46. `tests/test_uci_surface.cpp` needed no edit: it generates the
  option lines from `search_param_info` and requires MANUAL.md to document each
  name, which it now does.
- `tools/gate_extra.sh`, DEC-141 clause 3, before this verdict completes:
  **GATE-EXTRA-DONE 5 stages 1075 s**, all five green -- prose, citations, the
  Debug binaries (325 s), the sanitizer build and deep perft.

### Proposed for `adocs/specs.md`, for the coordinator to apply

The search row's late-move-reduction sentence gains:

> The reduction is scaled by the move's raw history since S098: `r -=
> clamp(hist_sum / LmrHistDiv, +/-LmrHistClamp)`, where `hist_sum` is the
> butterfly entry plus `ContHistWeight` per cent of the continuation entry and
> never `score_move`'s banded return. The same adjusted reduction is what the
> four shallow-depth rules are gated on, so the gate and the reduction are one
> number; at `LmrHistClamp 0` the helper returns the raw table and the engine
> is the one before S098, bench signature included.

### Files

Changed: `src/search.cpp`, `src/search.hpp`, `src/search_params.hpp`,
`src/evaluation.cpp`, `src/evaluation.hpp`, `src/data_structures.hpp`,
`tests/test_search.cpp`, `tests/test_search_params.cpp`,
`tools/mutants/S222_continuation_history.py`, `MANUAL.md`, `DEV_MANUAL.md`,
this file. Created: `tools/mutants/S098_lmr_history.py`,
`adocs/data/S098_v1_sprt.sh`.

**For the coordinator.** The `adocs/specs.md` sentence above is proposed, not
applied. The pre-registration's open-findings paragraph first named
`2026-08-22_adversarial-F03` as in reach; that was read off a stale `Status:`
line. The defect is closed and has been since S162 (`ea9ba2c`) -- the
`halfmove_clock >= 100` return in `negamax_at` tests for a mated node first and
falls through, guarded by `tests/test_engine.cpp` "checkmate outranks the
hundredth halfmove", re-read in this tree -- so the paragraph now says no
finding is open.

### Re-seeded before the match, 2026-09-15

**The divisor was a statement about the band and not about the tree, and the
step's own ablation is what caught it.** At 8675 -- half the saturated sum --
the term moved +0.00 % of the bench nodes at depths 9, 10 and 11 and -0.02 %
at 12, so a verdict at `8+0.08`, where a game lives at depths 10 to 14, would
have priced the seed and not the technique. The paragraphs above are left as
the record of that first seed; this one replaces its number. The coordinator's
ruling, before any game: re-derive the divisor from this engine's own
distribution, still DEC-105 **(b)**, no other engine's number anywhere near it.

**The census**, `adocs/data/S098_v1_hist_census.py` and `.txt`. Every site the
rule reads history at -- one call of `lmr_adjusted_reduction`, that is a quiet
past the third legal move at depth 3 or more with neither side in check -- over
the 400 positions of `adocs/data/S024_census_positions.txt`, driven through
S024's own `Engine` at `go depth 10` and `go depth 12`. A detached worktree
carrying this working tree's `src/` with five throwaway counters patched in;
the instrumented binary's bench signature is the shipping build's, which is
what says the counters do not move the tree, and `src/` here was never touched.

| | depth 10 | depth 12 |
|---|---|---|
| sites | 2105964 | 5464717 |
| \|sum\| p50 / p75 / p90 / p99 | 107 / 258 / 689 / 4347 | 174 / **430** / 1442 / 5362 |
| signed sum p50 / p75 | -53 / 22 | -26 / 231 |
| zero sums | 17.72 % | 11.80 % |
| \|sum\| >= 8675, the old seed | **0.006 %** | **0.011 %** |

**`LMR_HIST_DIV` 8675 -> 430**, the 75th percentile of |sum| at depth 12, so
the term reaches one full ply at the quartile. **One pass, and that bounds what
the percentile means**: the census ran on the tree at 8675 -- its header's
bench signature, 5968045, is the proof -- and 430 grows that tree by 60 %, so
the distribution at reduction sites on the tree that ships is not the one 430
is the 75th percentile of. A fixed point would need the census and the re-seed
iterated to agreement; 430 is one step of that and not its limit, and DEC-212's
lane supersedes the iteration by fitting the scale against games instead. At 430 with the clamp at 2 the
shares at depth 12 are 74.96 % of sites moved by nothing, 10.38 % by one ply
and 14.66 % by two. **`LMR_HIST_CLAMP` stays 2** and the coordinator's
exception does not fire: the 99th percentile, 5362, is twelve times two
divisors, so the clamp binds on real sites rather than only at the band's edge
-- it is the safety and not the scale.

**What the new seed does to the tree**, `bench <depth>` on the tune build at
the shipped clamp against `LmrHistClamp 0`:

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 611512 | 1197881 | 2019830 | 3345622 | 5226045 | 9133516 |
| off | 607842 | 935536 | 1634008 | 2364815 | 3849812 | 5685915 |
| delta | +0.60 % | +28.04 % | +23.61 % | +41.47 % | +35.75 % | +60.63 % |

The off column is the parent's totals exactly at every depth, so the
inert-by-rebuild property is unchanged. The on column is the point of the
re-seed and also its price: **`Bench: 9133516`, +60.63 % on the parent**.
`tools/search_bench.py` depth 9: midgame 51189 -> 27434 with its best move
moving `c3d5` -> `g5f6`, kiwipete 146616 -> 148084 `e2a6`, tactical
39389 -> 30174 `d7c8q`; depth 12: 143205 -> 240137, 570238 -> 772719,
148060 -> 185931, all three best moves unchanged. A signed term does not cost
the same in both directions -- an un-reduced late quiet opens a subtree where
an extra ply on an already-reduced one saves little -- which is why a rule that
moves a quarter of its sites grows the tree by half. **That is the trade the
SPRT is being asked to price**, and it is a real one: 60 % more nodes at a
fixed depth is about two thirds of a ply given up elsewhere.

**Re-derived with it**, all of it measured and not argued:

- the driven cases now plant `LMR_HIST_DIV` and `-LMR_HIST_DIV` instead of the
  band's edge, so they read "one ply" at any divisor S127 lands on and would
  not have gone red at this re-seed for no defect;
- `tests/test_search_params.cpp`'s golden row and `MANUAL.md`'s option row;
- **`capture_mates` re-derived a second time**, seven more sweeps: rows 1 and 4
  moved depth, 1 to `d9` and 4 from `d11` to `d9`, and the labels with them --
  `no S091 mutant, since S098`, `C02 and C05`, `no S091 mutant, since S222`,
  `C02 and C07`. The rule that picks them is written at the table and applied
  uniformly: the lowest shipped depth that separates something, else the lowest
  shipped depth with the label saying nothing separates. Row 1 lost `d7`
  outright, which is what made the suite red and the re-derivation owed;
- the six mutants L01 to L06 re-observed by hand with the sha256 check, **every
  anchor unchanged** -- the re-seed moves a default and no line of
  `src/search.cpp` -- and every one killed by the same named case as before;
- Debug self-play again, **8 games in 20 s, 0 `Assertion`, 0 `disconnect`**;
- both fast suites **39/39**, `clang-format.sh --check` clean;
- `adocs/data/S098_v1_sprt.sh`'s seed paragraph, its node counts, its ablation
  table and its H0 leg 2, which now points the divisor **upward** and names the
  census's own p90 (1442) as the next value to try. `REF` is left at `1db5b8e`
  for the coordinator to re-pin; `0aa64ff`, the audit-status commit made while
  this ran, touches no `src/` file, so `.ref-builds/1db5b8e` is the right
  reference binary either way.

### The lane, DEC-212, 2026-09-15

**The two constants are fitted before the verdict is taken, and the reason is
the two paragraphs above.** One divisor, derived twice over this engine's own
data on the same day, produced two different searches: 8675 (half the saturated
band) is inert below depth 12 by the step's own ablation, and 430 (the census's
depth-12 p75) grows `bench` by 60.63 %. Both are DEC-105 (b); nothing but a run
can say which scale is the technique's, and an SPRT at either would price a
seed. DEC-212 is the ruling.

`tools/spsa_s098v1.json` (moved to `adocs/data/S098_v1_spsa_config.json` on 2026-09-16, when the removal left it naming parameters the tree no longer has) and `adocs/data/S098_v1_spsa.sh`. **Two axes and no
others**: `LmrHistDiv` 430 and `LmrHistClamp` 2, at S085's regime -- 1250
iterations x 24 pairs = 60000 games at `2+0.02` on `books/UHO_4060_v3.epd`,
which is not the harness book (DEC-209 clause 1). `LmrBase` and `LmrDivisor`
are not in it: the accepts prices one adjustment at a time and the table's own
coefficients are S127's. Neither is any history axis -- S222's lane fitted the
eleven that decide the sum this rule divides, and fitting the input beside the
consumer would leave neither readable.

Both axes carry the **declared** bounds from `src/search_params.hpp`,
unnarrowed, and that is forced rather than chosen: `tools/spsa_driver.py`'s
`check` refuses a config whose bounds are not equal to the binary's own `uci`
listing, because a value outside the binary's range is refused rather than
clamped and the refusal is invisible mid-run. The census's region is carried by
the seed and by `c_end` instead -- 128 for the divisor, half the gap between
the p50 (174) and the p75 (430), which is a quarter of the distribution. What
that resolution cannot reach in this budget is 8675, and the pre-registration
says so rather than implying otherwise; that end is the SPRT's own H0 leg, one
release rebuild.

`check` passed on the day, both axes reaching the search -- `LmrHistDiv` 1 ->
32857 nodes against 34700 -> 51189, `LmrHistClamp` 0 -> 51189 against
4 -> 27458, the off values being the pre-S098 count at that position -- with
one warning the runner records: the clamp's first iteration has `2c` = 4.11
against a range of 4, which is one iteration of 1250 (`c` is 2.0551 at k=0 and
1.9161 at k=1).

**Estimate 8 h 37 m**, from S222's measured 24.84 s an iteration, **ceiling
17 h 15 m**; a night run under DEC-155, on mains and on an idle machine. The
readings are fixed before the fit, including the two that end the step: a
rounded vector equal to the incumbent is a stuck run owing no SPRT (S085's
rule), and `LmrHistClamp` fitted to 0 is the term inert by the fit's own word,
where the technique leaves the plan with a decision instead of a match.

**Then the SPRT**, on the fitted defaults and not on either seed:
`adocs/data/S098_v1_sprt.sh`, `{0, 5}` nElo at the harness regime against the
commit before this landing, which the coordinator re-pins once the fitted
values land.

### Fitted before the match, 2026-09-15

The lane ran as pre-registered: **11:49:38 to 20:37:23, 8 h 47 m 45 s** for
1250 iterations and 60000 games at `2+0.02` on `books/UHO_4060_v3.epd`, against
the 8 h 37 m estimated from S222's measured 24.84 s an iteration -- 2 % over.
`SPSA-DONE` is the run log's only marker, which is the check-log plumbing doing
its job. Evidence: `adocs/data/S098_v1_spsa_trajectory.tsv`,
`S098_v1_spsa_run.json`, `S098_v1_spsa.log`, `S098_v1_spsa_check.log`.

**The vector: `LmrHistDiv` 430 -> 699, `LmrHistClamp` 2 -> 3**, the driver's own
rounding of theta `[698.6584395515816, 2.7319152681598724]`. Read by the
pre-registration's own rows, written before a game was played:

- **not stuck** -- both axes differ from their seeds, so the SPRT is owed
  (S085's rule would have excused it otherwise);
- **the clamp is not 0**, so the term is not inert by the fit's own word and
  the technique does not leave the plan on this reading;
- **the divisor is at or below the p90 of 1442**, so the fit agrees with the
  census seed's side of the question rather than the band seed's -- 699 is
  about 1.6 times the p75 it started from, a narrower class of moves, with the
  clamp widened from two plies to three.

The trajectory beside the endpoint: `LmrHistDiv` never touched a bound in 1250
iterations, ran 422 to 747 with a median of 648; `LmrHistClamp` sat at a bound
on 39 of 1250 -- five settings being what they are, and the pre-registration
expected that figure to be non-trivial; `y` is centred at -0.139 with a
standard deviation of 5.78 over -21 to 16, real spread rather than the
"barely changing" trajectory the fishtest wiki calls useless; `c_scale` decays
2.054887 -> 1.000000 as designed.

**What the fitted vector does to the tree.** A narrower class touched harder:
fewer sites clear 699 than cleared 430, but one that clears three divisors now
gets three plies back instead of two, and the second effect dominates.

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 706352 | 1185319 | 2153709 | 4036614 | 6296135 | 11046420 |
| off | 607842 | 935536 | 1634008 | 2364815 | 3849812 | 5685915 |
| delta | +16.21 % | +26.70 % | +31.81 % | +70.69 % | +63.54 % | +94.28 % |

The off column is the parent's totals exactly at every depth, at all three
settings this step has now measured, so the inert-by-rebuild property is
untouched by the fit. **`Bench: 11046420`**, against the parent's 5685915 and
the census seed's 9133516. `tools/search_bench.py`, parent -> 430 -> fitted:

| | depth 9 | depth 12 |
|---|---|---|
| midgame | 51189 -> 27434 -> 77969 | 143205 -> 240137 -> 231052 |
| kiwipete | 146616 -> 148084 -> 146770 | 570238 -> 772719 -> 618264 |
| tactical | 39389 -> 30174 -> 40190 | 148060 -> 185931 -> 221800 |

Best moves at the fitted vector are `c3d5` / `e2a6` / `d7c8q` at both depths --
the parent's, and midgame's depth-9 move comes back from the `g5f6` the census
seed read.

**Re-derived with the vector**: `tests/test_search_params.cpp`'s two golden rows
and `MANUAL.md`'s two option rows, both now saying the values are this
project's own SPSA fit. **The driven cases needed no edit and that was the
point of writing them against `LMR_HIST_DIV`** -- they plant the divisor rather
than a number, so they followed the re-seed and this fit without a line moving,
and `capture_mates` survived this change where it had not survived the last.
**No line of `src/search.cpp` moved**, so all six `L` anchors and every other
mutant anchor in the tree still resolve uniquely -- `git diff src/search.cpp`
is empty against the landing. Debug self-play again, four rounds at 4+0.04:
**8 games in 15 s, 0 `Assertion`, 0 `disconnect`**. Both fast suites **39/39**,
`clang-format.sh --check` clean.

**The SPRT prices 699 and 3**, not either seed: `adocs/data/S098_v1_sprt.sh`,
`{0, 5}` nElo at the harness regime, `REF` still `1db5b8e` and `CAND` still
`HEAD` for the coordinator to pin.

### Verdict 1's SPRT, 2026-09-16: H0

`adocs/data/S098_v1_sprt.sh` ran as pre-registered, 2026-09-15 21:19:10 to
2026-09-16 02:44:56 (**5 h 25 m 46 s**), candidate `0408447` -- the fitted
scale, `LmrHistDiv` 699 and `LmrHistClamp` 3 -- against `1db5b8e`, the tree
before verdict 1's landing, both identity lines printed before the first game.

**H0 accepted. LLR -2.96 against (-2.94, 2.94), `Elo -2.92 +/- 5.02`, `nElo
-3.70 +/- 6.34` over 11524 games**, W 3691 L 3788 D 4045, `Ptnml(0-2) [578,
1352, 1985, 1283, 564]`, LOS 12.65 %, 2124 games an hour. **0 time forfeits on
either side** over the 11526 games the PGN holds, so the abort rule never came
near firing; `Incomplete mating PV` 7 candidate and 3 reference, recorded and
not a stop. Pair score mean 0.9916, variance 0.3125 over 5762 pairs, beside
S219's 0.2905 and S212's 0.2939 on this book. Evidence
`adocs/data/S098_v1_sprt.log`, read in `adocs/data/S098_v1_sprt_pairs.txt`.

**The reading is the one written before the games**: the history-scaled
reduction at its own fitted scale does not gain 5 nElo over the tree before it,
and is recorded as a zero. The interval sits below zero rather than straddling
it, which is worth stating plainly: the point estimate is -2.92 Elo and the
upper edge is +2.10, so this is not "no gain found" but "no gain, and a small
loss is the better-supported reading". That the vector was a fit and not a seed
is what makes the zero worth something -- SPSA played 60000 games choosing the
scale, and the scale it chose does not pay for the tree it costs.

### Leg 2, 2026-09-16

The pre-registration's H0 bisection is two legs. **Leg 1, the sign, costs no
run**: it is pinned by "a quiet the history tables like is reduced less", by
"... written off is reduced more" and by mutant `L01_lmr_history_sign`, which
both cases kill. **Leg 2 is the divisor upward**, and this is it:
`LMR_HIST_DIV` **699 -> 1442**, `LMR_HIST_CLAMP` unchanged at 3, one value in
`src/search_params.hpp` and a Release rebuild, measured against the same
`1db5b8e`.

1442 is the census's own 90th percentile of |sum| at depth 12, so the term
reaches a full ply for a tenth of the sites the rule sees where 699 reached it
for under a quarter. **It is a pre-registered bisection point and not a fit**
-- the lane already fitted this axis over 60000 games and returned 699 -- and
it inherits the census's one-pass caveat: a percentile of the tree at 8675, not
of the tree it builds. `adocs/data/S098_v1_leg2_sprt.sh` prices it, and its H0
clause is written out in full: **the two legs are then spent, the term is
recorded as a zero and leaves the tree** in DEC-194's shape, down to what stays
(`quiet_history_sum`, behaviour-neutral and read by `score_move`) and what the
evidence keeps.

**Measured at the leg's value**: `Bench: 9268371`, against the reference's
5685915, the lane's seed 9133516 at 430 and the fitted 11046420 at 699 -- a
larger divisor touches fewer moves and the tree comes back toward the
reference's. `tools/search_bench.py`, reference -> leg: depth 9 midgame
51189 -> 60840, kiwipete 146616 -> 146616, tactical 39389 -> 42517; depth 12
143205 -> 243165, 570238 -> 641165, 148060 -> 199344. Best moves `c3d5` /
`e2a6` / `d7c8q` at both depths, the reference's.

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 632553 | 1074018 | 1867606 | 3023865 | 5018624 | 9268371 |
| off | 607842 | 935536 | 1634008 | 2364815 | 3849812 | 5685915 |
| delta | +4.07 % | +14.80 % | +14.30 % | +27.87 % | +30.36 % | +63.01 % |

The off column is the reference's totals exactly at every depth -- true now at
all four settings this step has measured, which is what makes the H0 removal a
revert of a known shape. Debug self-play four rounds at 4+0.04: **8 games in
22 s, 0 `Assertion`, 0 `disconnect`**. Both fast suites **39/39**,
`clang-format.sh --check` clean; no line of `src/search.cpp` moved, so every
mutant anchor still resolves.

### Leg 2's SPRT and the removal, 2026-09-16

`adocs/data/S098_v1_leg2_sprt.sh` ran 03:09:34 to 09:20:06 (**6 h 10 m 32 s**),
candidate `73fbf05` -- `LmrHistDiv` 1442 and `LmrHistClamp` 3 -- against the
same `1db5b8e`. **H0 accepted. LLR -2.95, `Elo -2.34 +/- 4.73`, `nElo -2.94
+/- 5.95` over 13078 games**, W 4187 L 4275 D 4616, `Ptnml(0-2) [667, 1518,
2240, 1464, 650]`, 0 forfeits either side, `Incomplete mating PV` 4 candidate
and 1 reference, pair variance 0.3154 over 6539 pairs, 2119 games an hour.
Evidence `adocs/data/S098_v1_leg2_sprt.log` and `_pairs.txt`.

**Both legs are spent and the term is recorded as a zero.** Three scales were
measured and none cleared the gainer pair: 8675, inert by node count before a
game was played; 699, chosen by SPSA over 60000 games and beaten at -3.70 nElo;
1442, the census's p90 and beaten at -2.94. Both runs put the interval below
zero rather than across it. The technique has a sub-3000 record (Lynx #613,
+11.40) and this engine does not reproduce it, which is DEC-019's seventh
instance and the fourth technique to arrive priced and measure nothing here.

**The removal, in the shape the leg's own header pre-registered it.**
`src/search.cpp`, `src/search.hpp`, `src/search_params.hpp` and
`src/data_structures.hpp` are restored to `1db5b8e` byte for byte -- the
helper, both probes, `NO_HISTORY_SUM`, the probe's `hist_sum` field and the two
parameters are gone, and S109's `lmr_depth_of` reads the raw table again. The
six cases that held the term, the two golden rows and the two `MANUAL.md` rows
went with it, and `tools/mutants/S098_lmr_history.py` is deleted. `git diff
1db5b8e -- src/` is **two files, `evaluation.cpp` -31 and `evaluation.hpp`
+66**, and that is the one difference the pre-registration allowed:
`quiet_history_sum` stays, `inline` in the header, with one reader in
`score_move`. It is behaviour-neutral and the bench proves it.

**Two things outlived the term and both earn their place.** The case "a mate
found at the root is never reduced" holds the root exemption -- `ply > 0` in
`may_reduce`, S013's own bug -- which predates the history term by the whole of
this search's history; it now reads `search_lmr_reduction_probe` and asserts
what it always asserted. Its mutant `L06_lmr_root` moved to
`tools/mutants/search.py`, where the other two `may_reduce` guards live,
**keeping its id** because ids are never reused; re-observed there, it reddens
eight cases across four binaries including its own guard.

**The tree is the reference's, and that is INV-6's proof rather than a claim.**
`Bench: 5685915`, exactly. `tools/search_bench.py` depth 9: 51189 / 146616 /
39389, `c3d5` / `e2a6` / `d7c8q`; depth 12: 143205 / 570238 / 148060. Identical
node counts and best moves, so no SPRT is owed for the removal. Both fast
suites **39/39**, `clang-format.sh --check` clean, Debug self-play **8 games in
14 s, 0 `Assertion`, 0 `disconnect`**, and every anchor in the eight remaining
mutant files resolves uniquely -- S222's `H04` included, which points at
`quiet_history_sum` where it now lives.

**`capture_mates` re-derived once more, and it found something that is not
S098's.** The depths return to S230's **7, 7, 9, 11** by the rule written at
the table -- the lowest shipped depth that separates a mutant. The **labels do
not**: measured here they are `C02 and C05`, `C02`, `R02`, `R02`, against
S230's `C02 and C05`, `C02, C05 and R02`, none and `C02 and R01`. The cause is
`d0a6667`, S222's fitted eleven-axis vector, which landed after S230 measured
and reordered every quiet with nothing re-deriving these labels; they were
already stale at the tree S098 started from. **R01's incidental kill is not
back** at the rule's depths -- and one measured fact is left at the table
rather than acted on: row 2 at depth 8 separates C02, C05, R01 and R02, four
mutants and the richest reading this table has ever had, where the rule takes 7
because 7 is lower. Moving it is a decision, not a re-derivation. The "quiet
SEE pruning" plant stays at 2 and 1: its premise -- the plant moves where the
two quiets sit and nothing else -- holds on this tree for any plant, and small
values keep it true by construction rather than by the current rule set.

### Proposed for verdict 2's seeding, for the coordinator to decide

Verdict 1 cost four settings, two SPRTs and a night's lane to learn that the
term does not pay here, and the expensive part was not the SPRTs: it was that
**the first seed was inert and nothing said so until a census was taken**. Two
readings of that, and they point different ways.

The step's own protocol already handles verdict 2 better than it handled
verdict 1: the four node-type terms each sit behind their own off-valued
constant, so a failing verdict bisects by release rebuild instead of four
SPRTs (DEC-063), and each is a **+/-1 ply count with no scale to get wrong** --
`LMR_CUTNODE`, `LMR_NOT_IMPROVING`, `LMR_TT_CAPTURE`, `LMR_PV` are 0, 1 or 2
and the midpoint is the seed. There is no 8675-against-430 question to have:
the entire range is three values wide.

So the proposal is **the off-value protocol as written, with one thing
borrowed from DEC-212**: before the SPRT, a cheap census of how often each of
the four conditions fires at a reduction site -- the same instrumented
throwaway worktree `adocs/data/S098_v1_hist_census.py` already builds, pointed
at `cut_node`, `!improving`, a capturing TT move and `is_pv` instead of the
history sum. It costs a build and 35 seconds, it is the measurement that would
have caught verdict 1's inert seed before the machine spent 20 hours, and for a
term with no scale it answers the only question a seed can get wrong here:
whether the condition is ever true. **A lane is not proposed**: DEC-212 exists
because one constant had two defensible derivations three orders apart, and a
0-to-2 ply count has neither the range nor the resolution to need SPSA -- it
would spend a night choosing between three integers. If the census shows a
condition firing at under a per cent of sites, that term ships at 0 and is said
to be inert rather than measured, which is the cheaper half of what verdict 1
learned the expensive way.
