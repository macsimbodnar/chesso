id:         S098
goal:       the late move reduction is scaled by history, by node type and by what the re-search returned, instead of by depth and move number alone
accepts:    an SPRT verdict per adjustment, measured separately -- history scaling, node type and the re-search rule are three changes and one at a time is the rule; every constant introduced goes into src/search_params.hpp with a stated range (S073), including the reduction table's own shape if it becomes a formula; the "pruning does not hide a forced mate" case re-run after each adjustment, since S013 shipped an LMR that reduced the mating move at the root; a mate found at the root is never reduced, asserted with the precondition that would otherwise reduce it; the fast suite green
touches:    src/search.cpp late move reduction, src/search.hpp, src/data_structures.hpp, src/search_params.hpp, tests/test_search.cpp, tests/test_search_params.cpp, tools/mutants/, adocs/data/, MANUAL.md, DEV_MANUAL.md
excludes:   late move pruning, which is S109 -- S090 was retired into it by DEC-082, which measures the four shallow-depth rules as one step; the improving flag itself, which S108 supplies two entries earlier in the order (S092 retired into S108 by the 2026-08-19 review, `adocs/plan.md` "the improving flag, was first in the pending order"; no `decisions.md` entry records that merge) and which is an input here
decisions:  DEC-019, DEC-063, DEC-071, DEC-084, DEC-105, DEC-134, DEC-141, DEC-142, DEC-143, DEC-176, DEC-185, DEC-194, DEC-198, DEC-199, DEC-202, DEC-209, DEC-212, DEC-213, DEC-214
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

## Verdict 2 landed, 2026-09-16: the node-type adjustments

Implemented by an Opus 5 subagent briefed by the coordinator (DEC-185,
DEC-199). **No `done:` stamp: the step completes after verdict 3.** The SPRT is
the coordinator's and is pre-registered as `adocs/data/S098_v2_sprt.sh` before
any game.

### The rule, and where it is clamped

`src/search.cpp` `lmr_adjusted_reduction` is the raw table plus one integer the
node computes once:

```
r = lmr_reduction(depth, move_number)
    + LMR_CUTNODE        at a node predicted to fail high
    + LMR_NOT_IMPROVING  where improving_at() is false
    + LMR_TT_CAPTURE     where the entry's own move is a capture
    - LMR_PV             at a principal variation node
```

`src/search.cpp` `lmr_node_adjustment` is that sum, and every input is a
property of the **node** rather than of the move -- which is why it is computed
once above the move loop and read by both consumers instead of recomputed per
move.

The helper is **unclamped**, and the two call sites clamp exactly as they
clamped the raw table before it existed:

| site | clamp | what it reads |
|---|---|---|
| the reduction in `src/search.cpp` `negamax_at` | `[0, child_depth - 1]`, S091's extra ply added first | the node's own adjustment |
| the shallow-depth gate `src/search.cpp` `lmr_depth_of` | at 0 from below | the same adjustment, so the gate and the reduction are one number |

**S109's `lmr_depth` reads the adjusted value and S109 is not re-verdicted**:
its file and `adocs/plan.md` both say S098's SPRT prices the interaction and
S127 refits its thresholds. The PV term never reaches that gate -- a PV node is
not a pruning node -- so of the four only three can move what is pruned, and
`lmr_depth_of` therefore never sees a negative adjustment in the engine.

**The PV term is the subtraction and not a second move-number bound.** Section 3
left the choice open and named both published forms; the subtraction is taken
because it composes with the other three inside one integer, where a separate
`LMR_MIN_MOVES_PV` threshold would be a second mechanism with an interaction of
its own to price. Recorded here as the decision section 3 asked for.

### The prediction, and the one place the two published lists disagree

`negamax_at` carries `bool cut_node` beside `is_pv`, so the pair names three
labels and only three -- PV `(true, false)`, CUT `(false, true)`, ALL
`(false, false)` -- and `assert(!(is_pv && cut_node))` at the top of the node
says the fourth pair is not one of them. The rules are five named functions
rather than expressions inlined at their recursions, because a wrong prediction
is silent and a named function is something a case can walk
(`src/search.cpp` `search_child_label_probe`):

| function | the rule, and whose |
|---|---|
| `first_child` | "The first child of a PV-node is a PV-node"; "The first child of a CUT-node is an ALL-node"; "Children of ALL-nodes are CUT-nodes" (Garms) |
| `scouted_child` | "The further children are searched by a scout search as CUT-nodes" (Garms); "Children of PV-nodes that are searched with a zero-window scout search are Cut-nodes" (Kannan) |
| `zero_window_research` | the same, for the full-depth repeat a reduced move that beat alpha is owed: it is still a zero window and so still a scout |
| `full_window_research` | "PVS re-search is done as PV-node" (Garms); Kannan's "re-searched because the scout search failed high, are PV-nodes" |
| `null_move_child` | **Kannan**, and this is the disagreement |

The root is a PV node, which is both lists' first rule and what `is_pv` already
carried.

**The null-move child, re-read at implementation rather than taken from a
summary, as section 3 asked.** The page carries two lists and they do not agree.
Garms: "The node after a null move is a CUT-node", unconditionally. Kannan: "The
first child of a Cut-node, and other candidate cutoff moves (nullmove, killers,
captures, checks, ...) is an All-node", with "Children of All-nodes are
Cut-nodes" covering the other parent. **This engine follows Kannan**, so the
label is the parent's opposite -- ALL under a CUT parent, CUT under an ALL
parent -- and the window is the second argument for it: the parent passes
`(-beta, -beta + 1)` hoping the child comes back at or below `-beta`, which is
the child failing low, which is an All-node. The block never runs at a PV node,
so no third case arises.

### The firing census, taken before anything was seeded, DEC-214

`adocs/data/S098_v2_node_census.py` and `.txt`. The method is verdict 1's, down
to the positions and the driver: a detached worktree carrying this working
tree's `src/` with write-only counters patched in at the one site the reduction
is consulted, driven over the 400 positions of
`adocs/data/S024_census_positions.txt` through S024's own `Engine` at
`go depth 10` and `go depth 12`. One site is the reduction call site in
`negamax_at`, the late-quiet reduction read -- not every call of
`lmr_adjusted_reduction`, which the three `lmr_depth_of` gates also make -- a
quiet past the third legal move at depth 3 or more with neither side in check.

**Two differences from verdict 1's, and the second is worth more than the
first.** It counts four booleans rather than a distribution, because a 0-to-2
ply count has no scale to get wrong. And **the patch also sets the four
constants to 0**, so the shares belong to the tree the SPRT's reference searches
-- which gives the signature check an exact number to hold rather than a
comparison against whatever `build/` happens to contain: the instrumented
Release binary must print **5685915**, the total the commit before this landing
prints. It does, which says both that the counters do not move the tree and that
the whole of verdict 2's plumbing is inert at its off values in the Release
build.

| | depth 10 | depth 12 |
|---|---|---|
| sites | 2103446 | 5478549 |
| `cut_node` | 17.74 % | **21.12 %** |
| `!improving` | 52.73 % | **52.75 %** |
| a capturing TT move | 25.37 % | **24.83 %** |
| `is_pv` | 36.19 % | **26.72 %** |

**No term ships at 0.** DEC-214's inert threshold is one per cent and the lowest
share is twenty times it, so all four are measured and none is recorded as
inert. The joint distribution is the other half of the reading: at the midpoint
seeds the four sum to nothing on 27.85 % of depth-12 sites, +1 on 41.95 %, +2 on
18.15 %, +3 on 1.45 % and **-1 on 10.60 %**, which is the PV term's own share.
So this is a rule that moves about seven reduction sites in ten -- the opposite
of what verdict 1's first seed turned out to be, and the whole reason DEC-214
put the census in front of the match.

### The four constants, their seeds and their DEC-105 forms

| constant | default | range | form |
|---|---|---|---|
| `LMR_CUTNODE` | 1 | 0 to 2 | **(c)** the midpoint of the declared range |
| `LMR_NOT_IMPROVING` | 1 | 0 to 2 | **(c)** the same |
| `LMR_TT_CAPTURE` | 1 | 0 to 2 | **(c)** the same |
| `LMR_PV` | 1 | 0 to 2 | **(c)** the same, subtracted |

The range is 0 to 2 by **stated purpose** and not by a guess at where the good
values are: 0 is off and inside the range, which is what the bisection needs; 1
is the published class of adjustment; and at 2 the term alone already equals
what the table returns for the first reducible move at the median depth --
`search_lmr_reduction_probe(11, 4)` is 2 -- past which the term replaces the
ordering's own estimate instead of adjusting it, which is a different rule and
not a setting of this one. A ply count has no unit to derive a **(b)** from and
no publication about the technique states one, so **(c)** is the only form
available and the midpoint of 0 to 2 is the integer 1. Section 4 wrote these
four as **(c) midpoint** before the step began and nothing here changes that.

No engine's ply count seeds any of them, wherever it is republished (DEC-084 as
amended by DEC-105). The records that say the direction is worth trying --
cutnode +1 at +9.34 with a second ply at -17.62, `!improving` +1 at +4.64, a
capturing hash move at +1.87 to +3.33, the PV decrement at +3.78 -- are records
and not seeds, section 5 already lists them as anti-seeds, and all but the last
sit at or above the 3119 to 3138 band this engine is below (DEC-176).

### What the terms do to the tree, measured before any game

`Bench: 5469072`, against the parent's 5685915: **-3.81 %**. The by-depth
ablation on the tune build, the shipped seeds against all four at 0 -- node
counts, never strength (S073):

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 475717 | 841594 | 1344741 | 2453847 | 3447081 | 5469072 |
| off | 607842 | 935536 | 1634008 | 2364815 | 3849812 | 5685915 |
| delta | -21.74 % | -10.04 % | -17.70 % | **+3.77 %** | -10.46 % | -3.81 % |

**The off column is the parent's totals exactly at every depth**, which is the
property the bisection rests on, measured rather than argued -- and asserted a
second way by the census script, whose instrumented Release binary is built at
the off values and refuses to take a census unless it prints 5685915.

**The on row is not monotone and that is the finding, not a wobble.** Three
terms lengthen the reduction and one shortens it, so which effect wins at a
given depth is a property of that depth's tree rather than of the rule: the tree
is 10 to 22 per cent smaller at depths 9, 10, 11 and 13, and 3.8 % **larger** at
12. `tools/search_bench.py` says the same from the other side.

| | depth 9, parent -> this | depth 12, parent -> this |
|---|---|---|
| midgame | 51189 -> 22078 | 143205 -> 205096 |
| kiwipete | 146616 -> 104682 | 570238 -> 646466 |
| tactical | 39389 -> 29842 | 148060 -> 149330 |

All three smaller at depth 9, two of three larger at depth 12. Best moves are
`c3d5` / `e2a6` / `d7c8q` at depth 12 -- the parent's -- and at depth 9 midgame
moves `c3d5` -> `g5f6` while the other two do not. Recorded and not explained: a
move at a fixed depth is a chess judgement no agent here makes (CHESS). Node
counts move by construction, so INV-6 takes the SPRT path.

### Tests, red first, with the printouts

Eight cases in `tests/test_search.cpp`, in the "search: pruning and reduction
guards" suite. The red observation was taken on the shipped code with the four
constants at **0** -- their off values -- which is both a red-first observation
and the inert-at-off property the bisection rests on. Verbatim, Release build:

```
TEST CASE:  a node expected to fail high reduces its late quiets by LmrCutNode more
FATAL ERROR: REQUIRE( LMR_CUTNODE > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

TEST CASE:  a node that is not improving reduces its late quiets by LmrNotImproving more
FATAL ERROR: REQUIRE( LMR_NOT_IMPROVING > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

TEST CASE:  a node whose table move is a capture reduces its late quiets by LmrTtCapture more
FATAL ERROR: REQUIRE( LMR_TT_CAPTURE > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

TEST CASE:  a principal variation node reduces its late quiets by LmrPv less
FATAL ERROR: REQUIRE( LMR_PV > 0 ) is NOT correct!
  values: REQUIRE( 0 >  0 )

[doctest] test cases:   40 |   36 passed | 4 failed | 87 skipped
```

That guard is a real precondition and not a formality: at the off value the two
drives agree by construction, so a wiring that read no condition at all would
pass the comparison below it.

The other four cases -- the walk of the prediction rules, the sum of the four
terms, the labels the sites hand their children, and the null-move child's label
-- are **property cases about the prediction, which is live at any constant**,
so they hold at the off values and are red under their own mutants instead. That
is the shape verdict 1's root case took, and it is stated here rather than left
to be noticed.

**How the four term cases are built, because the construction is the evidence.**
Each drives one position twice and compares the reduction of one late quiet. Two
properties make the two drives comparable and both are asserted rather than
assumed:

- the position is `3r2k1/5pb1/7p/p4B1P/2r3P1/8/1P1n1B2/1R2R1K1 w - - 3 36`, row
  1 of `adocs/data/S024_census_positions.txt` -- this project's own self-play --
  and it has **no captures and no promotions**, so `negamax_at` generates and
  scores the whole quiet stage before it searches anything and the move order
  cannot depend on what a child wrote into the history table;
- the window is inside the mate band, `[MATE_MIN, MATE_MIN + 1)`, which switches
  the shallow-depth block, null move pruning and reverse futility off at the
  node and through its whole subtree. A pruned quiet is not counted as a legal
  move, so a rule that fired in one drive and not the other would move every
  index after it.

`aligned_reduced_index` then requires the two drives to have searched the same
moves in the same order before it returns an index, so a difference read there
is the term and not the ordering. The transposition-table case needs one more
thing and it is that case's own precondition: the entry it plants carries a
capture **the position does not contain**, because a table move the list holds
is ordered first and would move every index after it, and the term asks what
class the entry's move is and nothing else.

The accepts' root case, "a mate found at the root is never reduced", is green
unchanged, and so are both mate cases -- "pruning does not hide a forced mate"
and "pruning does not hide a mate against the material leader".

### The mutants, and the case that killed each

`tools/mutants/S098_node_type.py`, prefix `T` -- the next free one, since `N` is
S191's -- nine mutants, **every one run by hand against the working tree and
observed**. The pass could not go through `tools/mutation_check.py`, which
requires a linked worktree with a clean `src/` at `HEAD` and reverts with
`git checkout --`, and this verdict is not committed yet. Each was applied,
built, run through the **whole fast suite** in the Release build and reverted
from a byte snapshot whose sha256 was compared after; a hand-driven pass reverts
the source and not the build, so each revert is followed by a `touch` and a
rebuild before anything reads the binary again.

| mutant | what it breaks | killed by |
|---|---|---|
| `T01_first_child_label` | the first child of an ALL node is labelled ALL, so the alternation stops alternating | "the node type of every child is the one the published rules predict" `CHECK( same_type(child_of(CHILD_FIRST, CUT_NODE), ALL_NODE) )` false, and "pruning does not hide a forced mate" at `capture_mates` row 4 |
| `T02_cutnode_inverted` | the ply is added at every node that is **not** a cut node | "a node expected to fail high ..." `REQUIRE_EQ( 2, 4 )`, the LmrNotImproving and LmrTtCapture cases on their own clamp preconditions, and "the node-type adjustment is the sum of its four terms" -- the LmrPv case stays green |
| `T03_improving_inverted` | the ply is added when the side to move **is** improving | "a node that is not improving ..." `REQUIRE_EQ( 2, 4 )`, the LmrCutNode and LmrTtCapture cases on their own clamp preconditions, "the node-type adjustment ...", and "a mate found at the root is never reduced" on its own `k >= 3` precondition |
| `T04_ttcapture_inverted` | the ply is added where the table move is **not** a capture | "a node whose table move is a capture ..." `REQUIRE_EQ( 2, 4 )`, the LmrCutNode and LmrNotImproving cases on their own clamp preconditions, and "the node-type adjustment ..." |
| `T05_pv_added` | the PV term is added instead of subtracted | "a principal variation node reduces its late quiets by LmrPv less" `REQUIRE_EQ( 3, 1 )`, "the node-type adjustment ..." `CHECK_EQ( 1, -1 )`, and `test_mate_carry` "a mate score carried across searches keeps a line that reaches it" `CHECK( 13 <= 9 )` |
| `T06_adjusted_reduction_ignores_node` | the shared helper drops the adjustment, so both consumers read the raw table | all four term cases and "the node-type adjustment ..." over its whole grid |
| `T07_site_first_child` | the recursion labels its first child by the scout rule, so a PV node scouts its own line | "the node labels its first child by the first-child rule and the rest by the scout rule" `CHECK_EQ( false, true )`, and three binaries: `test_search` "the reported line runs the full depth" `REQUIRE( 1 == 4 )`, `test_mate_breadth` `REQUIRE( 140 >= 143 )`, `test_mate_carry` `CHECK( 27 <= 9 )` |
| `T08_null_child_label` | the null-move child keeps the parent's own type | "the child after a null move is labelled the type the parent is not" `CHECK_EQ( true, false )`, and the walk's two null-move rows |
| `T09_pv_and_cut_together` | the full-window re-search labels its child both PV and CUT, the pair the Debug assert forbids | "the node type of every child ..." `CHECK_FALSE( is_the_fourth_pair )` true, and its three `CHILD_FULL_RESEARCH` rows |

Each reddened `test_search` and nothing else, except `T05` (two binaries) and
`T07` (three). **`assert` is dead in both gated builds**, which is why `T09`'s
Release kill is the walk case and the assert is the second net; the Debug
self-play below is where it would fire.

Eight of the nine were observed on the tree before `clang-format.sh` ran and the
formatting moved no anchor of theirs; `T06`'s anchor is the one the formatter
collapsed onto one line, so it was **re-observed on the tree that lands** and
killed by the same six cases.

**One mutant of another step's file moved and was re-observed too.**
`tools/mutants/S222_continuation_history.py` `H03_null_child_keeps_prev` anchors
on the null-move recursion, which gained two arguments here; the anchor is
updated, the mutation itself is unchanged, and it still dies at "the node after
a null move has no previous move to index" and nowhere else. A note at the
mutant says so. Every anchor in all ten registry files resolves exactly once on
the tree that lands, checked by script.

### `capture_mates` re-derived, and R01's kill is back

DEC-142: a golden is re-derived by its own script whenever **either** end moves,
and adjusting the reduction by the node's type is the same clause of the same
rule verdict 1 moved. Seven sweeps of `adocs/data/S230_mine_r01_row.py depths`
over `adocs/data/S230_table_fens.txt`, depths 3 to 12, once on this tree and
once per mutant of `tools/mutants/S091_capture_see.py` applied by hand and
reverted from a byte snapshot.

Shipped profiles: `d9 d10 d11 d12`, `d8 d9 d10 d11 d12`, `d11 d12`,
`d9 d10 d11 d12`. The rule written at the table -- the lowest shipped depth that
separates a mutant, else the lowest shipped depth with the label saying nothing
separates -- then gives:

| row | depth, was -> is | label, was -> is |
|---|---|---|
| 1 | 7 -> **9** | `C02 and C05` -> **no S091 mutant, since S098 verdict 2** |
| 2 | 7 -> **8** | `C02` -> **C02, C05, C07 and R01** |
| 3 | 9 -> **11** | `R02` -> **C07 and R02** |
| 4 | 11 -> **9** | `R02` -> `R02` |

Three depths moved and no mate distance did. That is the expected shape rather
than the hazard: three of the four terms lengthen the reduction, so a mate the
ordering does not put first arrives an iteration or two later, and the hazard is
a mate that never arrives. Every row still reports its own inside the swept
range, and so do both dedicated mate cases, `test_mate_carry` and
`test_mate_breadth`.

**R01's incidental kill is back**, which is what S230 went mining for and what
the tree verdict 1 left had lost: row 2 at depth 8 separates C02, C05, C07 and
R01 at once. The previous pass recorded that this row *would* say that at 8 and
took 7 because 7 was lower and separated something; on this tree 7 reports no
mate at all, so the rule itself takes 8 and the decision it left open never has
to be made.

### The suite, the signature and the second tier

- Fast suite, Release: **39/39**. Fast suite, tune build: **39/39**.
  `./clang-format.sh --check` clean.
- `Bench: 5469072`.
- `tools/search_bench.py` depths 9 and 12 above.
- Debug self-play, DEC-141 clause 1, four rounds at 4+0.04 with the Debug
  binaries and `level=trace engine=true`: **8 games in 23 s, 0 `Assertion` in
  both the log and the tee'd stdout, 0 `disconnect`**. The new
  `assert(!(is_pv && cut_node))` never fired.
- `tests/test_search_params.cpp`'s golden gains four rows and its count moves
  44 -> 48, re-derived the way its own GOLDEN note says -- by diffing it against
  `src/search_params.hpp`, which is its derivation.
- `MANUAL.md`'s tune-option table gains `LmrCutNode`, `LmrNotImproving`,
  `LmrTtCapture` and `LmrPv`; `DEV_MANUAL.md`'s bench ledger gains this verdict
  with the non-monotone ablation, and its DEC-142 golden list moves
  `golden_defaults` 44 -> 48 and carries the new `capture_mates` row.
  `tests/test_uci_surface.cpp` needed no edit: it generates the option lines
  from `search_param_info` and requires `MANUAL.md` to document each name, which
  it now does -- and it was observed red in the tune build before those rows were
  written.
- `tools/gate_extra.sh`, DEC-141 clause 3, before this verdict completes:
  **GATE-EXTRA-DONE 5 stages 1076 s**, all five green -- prose, citations, the
  Debug binaries (317 s), the sanitizer build (700 s) and deep perft (58 s). The
  prose and citation stages were re-run after this section was written, since
  they had been taken before it existed.

### Proposed for `adocs/specs.md`, for the coordinator to apply

The search row's late-move-reduction sentence gains, beside the verdict 1
sentence already there:

> **Late move reduction is adjusted by the node's type since S098 verdict 2**:
> `negamax_at` carries `cut_node` beside `is_pv`, so the pair names CPW's three
> node types by Garms's prediction rules, with Kannan's reading of the null-move
> child where the two published lists disagree; the reduction becomes
> `r + LmrCutNode + LmrNotImproving + LmrTtCapture - LmrPv`, each behind its own
> constant with 0 as an off value inside its range, clamped where the raw table
> was clamped, and **S109's shallow-depth gate reads the same adjusted number**,
> so the gate and the reduction are one value. At all four off values the helper
> returns the raw table and the engine is the one before the step, bench
> signature included.

### Files

Changed: `src/search.cpp`, `src/search.hpp`, `src/search_params.hpp`,
`src/data_structures.hpp`, `tests/test_search.cpp`,
`tests/test_search_params.cpp`, `tools/mutants/S222_continuation_history.py`,
`MANUAL.md`, `DEV_MANUAL.md`, `adocs/data/README.md`, this file. Created:
`adocs/data/S098_v2_node_census.py`, `adocs/data/S098_v2_node_census.txt`,
`adocs/data/S098_v2_sprt.sh`, `tools/mutants/S098_node_type.py`.

**For the coordinator.** The `adocs/specs.md` sentence above is proposed, not
applied. `adocs/data/S098_v2_sprt.sh` has `REF` pinned at `50fd965`, the commit
before this landing, and `CAND` at `HEAD` to pin once the landing commit exists;
its open-findings paragraph says no finding is open, re-read on this tree rather
than copied from verdict 1's.

### Verdict 2's SPRT, 2026-09-16 (the coordinator's)

The landing committed as `a771260` after the fast check and its repair: six of
the nine `T` evidence blocks quoted red values never observed and were re-quoted
verbatim from the captured ctest logs, three cells of the mutant table above
named the wrong set of killers and were corrected, the census sentence now says
the counted site is the late-quiet reduction read and not every call of
`lmr_adjusted_reduction`, and the header's `decisions:` and `touches:` were
completed. Gate `GATE-DONE 5469072`, 39 of 39 in both builds. `CAND` pinned as
`a771260` in `8be3f13`.

`adocs/data/S098_v2_sprt.sh` ran 11:59:32 to 12:55:41 (`adocs/data/S098_v2_sprt.log`,
`adocs/data/S098_v2_sprt_pairs.txt`): banner `candidate a771260`, `reference
50fd965`, both identity lines right, seed `20260916115932`, `8+0.08`, Hash 16,
concurrency 12, `noob_3moves.epd`, bounds `{0, 5}`.

**H1 accepted: `Elo 29.05 +/- 11.52`, `nElo 38.70 +/- 15.26`, LLR 2.95 against
(-2.94, 2.94), W 712 L 546 D 732 over 1990 games, `Ptnml(0-2) [60, 200, 358,
268, 109]`, LOS 100.00 %, 56 m 09 s at 2126 games an hour.** 0 time forfeits on
either side over the 1991 games the PGN holds (1397 adjudications, 594 natural
ends); `Incomplete mating PV` 14 candidate against 9 reference, recorded and
not a stop; 995 complete pairs at pair-score variance 0.2805, beside S219's
0.2905 and S212's 0.2939 on this book.

Read against the pre-registration's H1 line: **all four terms stay at 1 ply
each**, the claim written is at least 5 nElo and not the stopping estimate
(DEC-063), S127 refits the four beside `LmrBase`, `LmrDivisor` and S109's
thresholds after the block, and **verdict 3 (the re-search rule, section 3)
opens against `a771260`**. The specs search row carries the passage proposed
above with the verdict appended; the ledger holds the run as its eighteenth
row. Nothing of the bisection was needed. No `done:` stamp: the step completes
after verdict 3.

## Verdict 3 landed, 2026-09-16: the re-search depth

Implemented by an Opus 5 subagent briefed by the coordinator (DEC-185,
DEC-199). **No `done:` stamp here either: the coordinator writes it when this
verdict's SPRT is read.** The SPRT is the coordinator's and is pre-registered as
`adocs/data/S098_v3_sprt.sh` before any game.

### The rule, its precedence and the inequality it keeps

`src/search.cpp` `lmr_research_depth` is the whole of it, and it is consulted at
exactly one site -- the reduced fail-high re-search in `src/search.cpp`
`negamax_at`, the block guarded by
`!state->aborted && reduction > 0 && score > alpha`:

```
child_depth - 1   reduction >= 2 and score < alpha + LMR_SHALLOWER_MARGIN
child_depth + 1   reduction >= LMR_DEEPER_MIN_REDUCTION
                  and score > best_so_far + LMR_DEEPER_MARGIN
child_depth       otherwise
```

`best` is `best_so_far` **before this move updates it** -- the fail-soft base
and not alpha, which is the re-basing the published record made and kept. The
full-window re-search below it still runs at `child_depth`, the alpha-raising
path is untouched, and no history is updated from the re-search's outcome
(routed onward, Scope concerns 2).

**Precedence: the shallower path wins, and the reason is stated rather than
picked.** Both conditions can hold, and only where `best` sits more than a
margin below `alpha` -- which at a scout node is every node that has not yet
found a move, since a zero-window node that raises alpha cuts off instead of
continuing. In exactly that region `score > best + LMR_DEEPER_MARGIN` is
satisfied by `best` being low and not by `score` being high, so the deeper
condition's premise is degenerate precisely where the two meet, while
`score < alpha + LMR_SHALLOWER_MARGIN` is a statement about the score against
the node's own bound and is never degenerate. The degenerate one yields.
The case that held it carried the title "where both re-search paths could fire
the shallower one wins" here, was renamed at leg 1 to name the constant that
decides the region, and is now `tests/test_search.cpp`
"a score that barely beat alpha goes a ply deeper when the fail-soft best is far below it"
-- the same region under the description it has once the shallower path is
gone. `D05_precedence_swapped` was the mutant it killed on this tree and it
left the registry with the branch it reordered.

**The re-search is never the reduced search repeated.** The returned depth is
always strictly greater than `child_depth - reduction`, asserted in the Debug
build and held over the rule's whole declared domain by
`tests/test_search.cpp` "the re-search depth stays
inside its cap, floor and inequality". The shallower path's `reduction >= 2` is what keeps it
and it is **arithmetic, not a setting**: at a reduction of 1 the shallower depth
*is* the reduced depth. The deeper path's guard is the tunable one and it is the
published one.

The cap at `child_depth + 1` and the floor at 1 are written and asserted, and
what they are worth is recorded rather than implied: **neither binds at any
input the engine itself produces**, since the three branches give `child_depth`
and its two neighbours and the reduction is already clamped to
`[0, child_depth - 1]` before it arrives. They are kept because S127 tunes the
branch set and S097 extends `child_depth`, and because the floor **is**
reachable from the wider domain the function declares -- a reduction of 2 at a
child depth of 1 -- which is what makes `D10_floor_dropped` an ordinary mutant
with an ordinary kill. The cap has no such input, which is why a mutant that
only deletes it is declared equivalent below.

### The firing census, taken before anything was seeded, DEC-214

`adocs/data/S098_v3_research_census.py` and `.txt`. The method is verdict 2's
down to the positions and the driver: a detached worktree carrying this working
tree's `src/` with write-only counters patched in at the one site, driven over
the 400 positions of `adocs/data/S024_census_positions.txt` through S024's own
`Engine` at `go depth 10` and `go depth 12`. **It also patches the two off
values in**, so the distribution is the tree the SPRT's reference searches and
the signature check has an exact number to hold: the instrumented Release binary
must print **5469072**, which it does.

Verdict 2's census counted four booleans because a ply count has no scale.
**These two margins are scales**, so this one keeps distributions: four
histograms of one-unit bins over -8192 to 8192, two of them conditioned on
`r >= 2`, which makes the firing share readable at any margin without re-running
and makes a quantile inside the range exact.

| | depth 10 | depth 12 |
|---|---|---|
| re-search sites | 44871 | 106610 |
| with `r >= 2` | 48.38 % | **52.41 %** |
| `score - best` p10/p25/p50/p75/p90 | 2 / 6 / 14 / 30 / 67 | 2 / 6 / 14 / 32 / 73 |
| the same where `r >= 2` | 2 / 5 / 12 / 26 / 54 | 2 / 5 / 12 / 27 / 59 |
| `score - alpha` p10/p25/p50/p75/p90 | 2 / 4 / 9 / 21 / 45 | 1 / 3 / 9 / 22 / 47 |
| the same where `r >= 2` | 1 / 3 / 9 / 20 / 41 | 1 / 3 / 8 / 20 / 43 |
| the deeper **condition** at the seed 47 | 5.85 % | 6.89 % |
| the shallower **condition** at the seed 47 | 44.36 % | 47.77 % |
| both conditions at once | 1.88 % | 2.29 % |
| **the deeper path fires** (after precedence) | 3.97 % | **4.60 %** |
| **the shallower path fires** | 44.36 % | **47.77 %** |
| the re-search is unchanged | 51.66 % | 47.64 % |

The reduction taken at a re-search site never exceeds 6 and is 1 on about half
of them (51.62 % at depth 10, 47.59 % at depth 12).

**A condition's share is not a path's share, and DEC-214's test is about the
path.** The shallower path is tested first, so a site where both conditions hold
belongs to it; the deeper path fires only on what is left. The three outcomes
after precedence are counted **in the engine**, at the seeds the script reads out
of `src/search_params.hpp` so they cannot drift, because the histograms are
one-dimensional and cannot see the overlap. The first pass of this census had no
joint counters and published the deeper *condition's* 6.89 % as the path's share
in four documents; the Tier-1 fast check caught it and the census was re-run.

**Both paths are in the seed rule's first case.** The rule was written before the
run: the (c) seed 47 stands for a path firing at one per cent of sites or more;
a path under one per cent is re-seeded by DEC-105 **(b)** at the quantile that
makes it fire at about a tenth of sites; a path that cannot reach one per cent
anywhere inside 0 to `PAWN` ships inert. The smaller **path** share is over four
times the threshold, so **neither margin was re-seeded** and neither ships at its
off value. What a (b) re-seed would have been is printed anyway, so the number is
on record rather than reconstructed later: margin 34 for the deeper condition and
3 for the shallower one at depth 12 -- conditions and not paths, since a re-seed
moves the overlap too and the path share at a new margin needs another run.

The tree says the same from the other side and independently of the census: a
release rebuild at each off value benches **4646334** with the deeper path alone
(`LmrShallowerMargin` 0, against the off tree's 5469072) and **4794294** with the
shallower path alone (`LmrDeeperMinReduction` 126, against the shipped 4025871).
Both paths move the tree on their own, which is what the H0 bisection needs of
them. Measured by the coordinator's fast check and quoted here.

**One thing the census corrected before a game was played, and it is a
correction to this file's own section 4.** Section 4 says `LMR_DEEPER_MARGIN` is
off at its range top. **It is not.** The fail-soft best sits at or below alpha at
every site, so `score > best + 94` is still true on **2.86 %** of depth-12 sites
(2.34 % at depth 10) -- a margin bounded by a pawn cannot switch off a condition
whose base is unbounded below. The deeper path's off value is
`LMR_DEEPER_MIN_REDUCTION` at **its** range top, 126, which is above every
reduction the clamp to `[0, child_depth - 1]` admits. That is what the H0
bisection's second leg uses and what the census script patches in.

### The three constants, their seeds and their DEC-105 forms

| constant | default | range | form |
|---|---|---|---|
| `LMR_DEEPER_MARGIN` | 47 | 0 to 94 | **(c)** the midpoint of a range stated by purpose: 0 to `PAWN`, whose top is the point past which the re-search depth would be decided on more material than a pawn of window. Compared against search scores, so the unit is `piece_value` in `src/eval_tables.hpp` (section 4's P6 paragraph) |
| `LMR_SHALLOWER_MARGIN` | 47 | 0 to 94 | **(c)** the same. Off: 0, by arithmetic -- the site requires `score > alpha`, so no score is under `alpha + 0` |
| `LMR_DEEPER_MIN_REDUCTION` | 2 | 1 to 126 | **(b)** a derivation over this engine's own site: the re-search exists only where `reduction > 0`, so 1 is a guard that says nothing and 2 is the smallest value at which it does -- the same integer the shallower path's strict inequality forces, so one threshold serves both. Floor 1 arithmetic, ceiling `MAX_DEPTH` and above every reachable reduction, which makes it the deeper path's off value |

No engine's coefficient seeds any of them, wherever it is republished (DEC-084
as amended by DEC-105). The records that say the direction is worth trying --
the guarded form at +3.11 +/-2.35 and the bare form at -7.07 +/-7.65 in the
3138-3224 band, one unmerged pass above it, one wholesale revert on the negative
side -- are records and not seeds, and section 5 already lists them as
anti-seeds. All three sit above this engine's band, so a zero is an expected
outcome (DEC-176).

### What the rule does to the tree, measured before any game

`Bench: 4025871`, against the parent's 5469072: **-26.39 %**. The by-depth
ablation on the tune build, the shipped seeds against `LmrShallowerMargin` 0 and
`LmrDeeperMinReduction` 126 -- node counts, never strength (S073):

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 535688 | 836148 | 1399862 | 2002440 | 2746511 | 4025871 |
| off | 475717 | 841594 | 1344741 | 2453847 | 3447081 | 5469072 |
| delta | +12.61 % | -0.65 % | +4.10 % | -18.40 % | -20.32 % | -26.39 % |

**The off column is the parent's totals exactly at every depth**, which is the
property the bisection rests on, measured rather than argued -- and asserted a
second way by the census script, whose instrumented Release binary is built at
the off values and refuses to take a census unless it prints 5469072.

**The row changes sign and that is the finding.** Verdict 2's ablation was
non-monotone; this one is *larger* at 9 and 11, flat at 10, and 18 to 26 per
cent smaller at 12, 13 and 14. Both directions are the same two paths: the
shallower path fires on nearly half of all re-search sites and saves a ply at
each, while the deeper path spends one on 7 % and every ply it spends opens a
subtree that grows with depth. So the rule costs where the tree is shallow and
pays where a game at the harness control actually lives.

`tools/search_bench.py` says the same from the other side and disagrees with
`bench` about the sign at depth 9:

| | depth 9, parent -> this | depth 12, parent -> this |
|---|---|---|
| midgame | 22078 -> 51048 | 205096 -> 115959 |
| kiwipete | 104682 -> 104849 | 646466 -> 471175 |
| tactical | 29842 -> 29842 | 149330 -> 134331 |

The parent's numbers are `DEV_MANUAL.md`'s ledger row for verdict 2 and not a
re-run. Best moves are `c3d5` / `e2a6` / `d7c8q` at **both** depths, and
midgame's depth-9 move therefore comes back from the `g5f6` verdict 2 read.
Recorded and not explained: a move at a fixed depth is a chess judgement no
agent here makes (CHESS). Node counts move by construction, so INV-6 takes the
SPRT path.

### Tests, red first, with the printouts

Nine cases in `tests/test_search.cpp`, in the "search: pruning and reduction
guards" suite: two per path, the precedence, the re-basing, the cap/floor/
inequality grid, the site replay, the site's table-depth witness and the site's
base -- the last added by the Tier-1 repair below. The red
observation was taken on the shipped code at `LmrShallowerMargin` 0 and
`LmrDeeperMinReduction` 126 -- their off values -- which is both a red-first
observation and the inert-at-off property the bisection rests on. Verbatim,
Release build, `.tuning/coord/S098v3_mutlogs/red_at_off_values.log`:

```
===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7182:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  a re-search whose score clears the fail-soft best goes a ply deeper

/home/max/ws/chesso/tests/test_search.cpp:7193: FATAL ERROR: REQUIRE( LMR_DEEPER_MIN_REDUCTION <= 3 ) is NOT correct!
  values: REQUIRE( 126 <= 3 )

===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7253:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  a re-search that only just beat alpha goes a ply shallower

/home/max/ws/chesso/tests/test_search.cpp:7259: FATAL ERROR: REQUIRE( LMR_SHALLOWER_MARGIN > 1 ) is NOT correct!
  values: REQUIRE( 0 >  1 )

===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7307:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  the deeper margin is measured from the fail-soft best and not from the window

/home/max/ws/chesso/tests/test_search.cpp:7311: FATAL ERROR: REQUIRE( LMR_DEEPER_MIN_REDUCTION <= 3 ) is NOT correct!
  values: REQUIRE( 126 <= 3 )

===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7356:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  where both re-search paths could fire the shallower one wins

/home/max/ws/chesso/tests/test_search.cpp:7358: FATAL ERROR: REQUIRE( LMR_SHALLOWER_MARGIN > 1 ) is NOT correct!
  values: REQUIRE( 0 >  1 )

===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7702:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  a re-search that went a ply deeper left a table entry a ply deeper

/home/max/ws/chesso/tests/test_search.cpp:7714: FATAL ERROR: REQUIRE( LMR_DEEPER_MIN_REDUCTION <= 6 ) is NOT correct!
  values: REQUIRE( 126 <= 6 )

===============================================================================
/home/max/ws/chesso/tests/test_search.cpp:7761:
TEST SUITE: search: pruning and reduction guards
TEST CASE:  both re-search depths are reached in a real search

/home/max/ws/chesso/tests/test_search.cpp:7769: FATAL ERROR: REQUIRE( LMR_SHALLOWER_MARGIN > 1 ) is NOT correct!
  values: REQUIRE( 0 >  1 )

===============================================================================
[doctest] test cases:     9 |     3 passed | 6 failed | 127 skipped
[doctest] assertions: 28655 | 28649 passed | 6 failed |
[doctest] Status: FAILURE!
```

**Six of the nine are red there and the three that are not are the property
cases**: "the re-search depth stays inside its cap, floor and inequality", which
is a statement about the rule at any constants including the off ones, "the node
re-searches at the depth the rule returns", which compares the site against the
rule and is satisfied when both say `child_depth`, and "the node measures the
deeper margin from its own fail-soft best", which is about which variable the
call hands over and is true whatever the margins hold. Those two are red under
their own mutants instead, which is the shape verdict 1's root case and verdict
2's four prediction cases took, and it is stated here rather than left to be
noticed. The other six carry a `REQUIRE` naming the off value they need, and
that guard is a real precondition and not a formality: at the off values every
re-search runs at `child_depth`, so a wiring that read no condition at all would
satisfy every comparison below it.

**The two site cases are self-deriving and that is deliberate.** What they need
is a node where a *reduced* move beat alpha, and whether a given window produces
one is a property of the tree rather than something to assert by hand. They
sweep alpha from -500 to +500 in steps of 10 over the first 30 positions of
`adocs/data/S024_census_positions.txt`, driving each as a scout node at depth 8
and collecting every re-search the probed node made -- so no number in either
case is read off a run (DEC-142), and a rule that stops firing fails loudly
rather than passing over an empty set. The firing case stops as soon as it has
three of each. 0.7 s for both.

**The probe carries what the site decided and the table carries what the tree
did, and the second is the load-bearing one.** `search_node_probe_t` in
`src/data_structures.hpp` gains `research_depth` and the three numbers that
decided it (`research_score`, `research_alpha`, `research_best`), so a case can
replay `lmr_research_depth` on the node's own inputs. That alone is not enough
and the first mutant pass proved it: a recursion that computes the answer and
then searches at `child_depth` anyway agrees with every replay of itself, and
`D08_site_ignores_the_rule` **survived** the whole fast suite. The case "a
re-search that went a ply deeper left a table entry a ply deeper" is what
answers it -- after each drive it makes the move and reads the child's own
transposition entry, which is stored at the depth the child ran at and which the
table keeps at the deeper value within one search. D08 dies there, and 48 of 48
sites in the sweep carry an entry.

### The mutants, and the case that killed each

`tools/mutants/S098_research_rule.py`, prefix `D` -- the next free one, since
`T` is verdict 2's and `L` was verdict 1's, whose mutants left with the term --
**twelve mutants, every one run by hand against the working tree and
observed**.
The pass could not go through `tools/mutation_check.py`, which requires a linked
worktree with a clean `src/` at `HEAD` and reverts with `git checkout --`, and
this verdict is not committed yet. Each was applied, built, run through the
**whole fast suite** in the Release build and reverted from a byte snapshot
whose sha256 was compared after; a hand-driven pass reverts the source and not
the build, so each revert is followed by a `touch` and a rebuild before anything
reads the binary again. The driver is `.tuning/coord/run_mutants_v3.py` and
every ctest log is kept under `.tuning/coord/S098v3_mutlogs/`. **Every value quoted in the
test file's evidence blocks is checked back against those logs by script** --
21 `values:` lines carrying 28 mutant-id references, all observed. The first
version of that script matched only lines with a trailing mutant id and missed
a fabricated one; it now matches every `values:` line in the block and fails one
that names no mutant. That is the repair verdict 2 needed, and this verdict
needed half of it after all.

| mutant | what it breaks | killed by, with the values it printed |
|---|---|---|
| `D01_deeper_inverted` | the deeper path fires where the score is *below* the best by the margin | "a re-search whose score clears the fail-soft best goes a ply deeper" `CHECK_EQ( 6, 7 )`; "a re-search that went a ply deeper left a table entry a ply deeper" `CHECK( 4 >= 6 )` and `REQUIRE( 0 >  0 )`; "both re-search depths are reached in a real search" `REQUIRE( 0 >  0 )`; "pruning does not hide a forced mate" `REQUIRE( false )`; "a reduced move that beats alpha is searched again" `REQUIRE( 0 >  0 )` |
| `D02_shallower_inverted` | the shallower path fires where the score beat alpha by *more* than the margin | "a re-search that only just beat alpha goes a ply shallower" `CHECK_EQ( 6, 5 )`; "... clears the fail-soft best ..." `CHECK_EQ( 5, 7 )`; "where both re-search paths could fire the shallower one wins" `CHECK_EQ( 7, 5 )`; "... a table entry a ply deeper" `CHECK( 4 >= 7 )`; "pruning does not hide a forced mate" |
| `D03_deeper_guard_dropped` | the deeper path fires at any reduction -- the bare form | "a re-search whose score clears the fail-soft best goes a ply deeper", on the guard's own sub-assertion, `CHECK_EQ( 7, 6 )` |
| `D04_shallower_guard_dropped` | the shallower path fires at a reduction of 1, repeating the reduced search | "a re-search that only just beat alpha goes a ply shallower" `CHECK_EQ( 5, 6 )`; "the re-search depth stays inside its cap, floor and inequality" `CHECK( 1 >  1 )` |
| `D05_precedence_swapped` | the deeper path is tested first and wins the region where both hold | "where both re-search paths could fire the shallower one wins" `CHECK_EQ( 7, 5 )`; "pruning does not hide a forced mate" |
| `D06_deeper_two_plies` | the cap is raised and the deeper path becomes an even-deeper search | "... clears the fail-soft best ..." `CHECK_EQ( 8, 7 )`; "the re-search depth stays inside its cap, floor and inequality" `CHECK( 3 <= 2 )`; "pruning does not hide a forced mate" |
| `D07_shallower_two_plies` | the shallower path drops two plies | "a re-search that only just beat alpha ..." `CHECK_EQ( 4, 5 )`; "where both re-search paths could fire ..." `CHECK_EQ( 4, 5 )`; "the re-search depth stays inside its cap, floor and inequality" `CHECK( 1 >  1 )`; "pruning does not hide a forced mate" |
| `D08_site_ignores_the_rule` | the recursion re-searches at `child_depth` and the rule is thrown away | "a re-search that went a ply deeper left a table entry a ply deeper" `CHECK( 7 >= 8 )` and `REQUIRE( 0 >  0 )`; "pruning does not hide a forced mate". **It survived the first pass**, which is why that case exists |
| `D12_site_rebases_on_alpha` | the **call** hands the rule the window where the fail-soft best belongs | "the node measures the deeper margin from its own fail-soft best" `CHECK_EQ( -500, -504 )`, and `test_mate_carry` `CHECK( 11 <= 9 )`. **It was green on the whole suite until the Tier-1 check found it**, which is why `research_base` and that case exist |
| `D09_deeper_margin_off_alpha` | the deeper margin is measured from the window instead of the fail-soft best | "the deeper margin is measured from the fail-soft best and not from the window" `CHECK_EQ( 6, 7 )`, and `test_mate_carry` `CHECK( 11 <= 9 )` |
| `D10_floor_dropped` | the floor is written at 0, so a shallower re-search can reach quiescence | "the re-search depth stays inside its cap, floor and inequality" `CHECK( 0 >= 1 )` and `CHECK_EQ( 0, 1 )` |
| `D11_cap_one_ply_low` | the cap is a ply low and clamps the deeper path away | the same five cases D01 dies at, at `CHECK_EQ( 6, 7 )`, `CHECK( 4 >= 6 )` and `REQUIRE( 0 >  0 )` |

**One mutant is declared equivalent and is not in the list.** Deleting
`if (depth > child_depth + 1) { depth = child_depth + 1; }` on its own changes
nothing: the three branches produce `child_depth` and its two neighbours, so
nothing the rule can build reaches the cap. That is a clamp doing its job, and
`D06` and `D11` are the killable forms of the same bug -- the bound moved up
together with the branch that would then reach it, and the bound moved down
alone. Declared here and in the registry's header, never inferred from a green
suite (`tools/mutation_check.py`'s own rule).

**The first pass is recorded because it changed the step.** Ten mutants, one
survivor (`D08`) and one weak kill (`D09`, then a site-level mutant, dying only
at `test_mate_carry`'s incidental golden). The survivor produced the
table-depth case above; the weak kill produced the re-basing case and moved that
mutant from the site into the rule, where a direct guard can reach it. Both are
the second tier working as DEC-141 intends, and both would have shipped
unnoticed on the mutant list alone.

### `capture_mates` re-derived, and R01's kill is gone again

DEC-142: a golden is re-derived by its own script whenever **either** end moves,
and changing the depth a reduced move's re-search runs at is the same clause of
the same rule verdicts 1 and 2 both moved. Seven sweeps of
`adocs/data/S230_mine_r01_row.py depths` over `adocs/data/S230_table_fens.txt`,
depths 3 to 12, once on this tree and once per mutant of
`tools/mutants/S091_capture_see.py` applied by hand and reverted from a byte
snapshot; the raw profiles are in `.tuning/coord/S230_v3/`.

Shipped profiles: `d9 d10 d11 d12`, `d9 d10 d11 d12`, `d10 d12`,
`d9 d10 d11 d12`. The rule written at the table -- the lowest shipped depth that
separates a mutant, else the lowest shipped depth with the label saying nothing
separates -- then gives:

| row | depth, was -> is | label, was -> is |
|---|---|---|
| 1 | 9 -> 9 | `no S091 mutant` -> `no S091 mutant` |
| 2 | 8 -> **9** | `C02, C05, C07 and R01` -> **`no S091 mutant`** |
| 3 | 11 -> **10** | `C07 and R02` -> **`C02, C07 and R02`** |
| 4 | 9 -> 9 | `R02` -> `R02` |

Two depths moved and no mate distance did. **R01's incidental kill is gone
again**, stated rather than papered over: it lived at row 2's depth 8, and on
this tree the shipped build reports no mate there at all, so the rule takes 9
and nothing separates. What replaces it is row 3, which separates three mutants
at its new depth where it separated two before. The direct guards are what the
rules rest on -- all six S091 mutants were run through the whole fast suite at
verdict 1 with every one killed by its own named case -- and every row still
reports its own mate inside the swept range, as do both dedicated mate cases,
`test_mate_carry` and `test_mate_breadth`.

### One existing case changed, and it was not relaxed

`tests/test_search.cpp` "a mate found at the root is never reduced" went red on
its own precondition, `REQUIRE( k >= 3 )` reading `REQUIRE( 2 >= 3 )` at depth 6
and 10 at depths 3, 4 and 5. That is the case's own comment doing what it was
written to do: the key's index at the root is not a constant of the position but
of the tree, because the quiet stage is scored only once the captures run out,
so the order depends on what the first capture's subtree wrote into the history
table -- and this verdict changes every such subtree.

The conclusion is now asserted at **every** depth and the precondition is
counted: a drive whose key is not both past the move-number bound and reduced by
the table proves nothing about the root exemption, so it is reported by name and
not asserted through, and `REQUIRE(meaningful > 0)` refuses to let the case go
vacuous at every depth at once. Coverage at depths 3, 4 and 5 is exactly what it
was; depth 6 stops claiming something it can no longer establish. `L06_lmr_root`
in `tools/mutants/search.py` is still killed by it.

One title described what the engine did until this verdict: "a reduced move that
beats alpha is searched again **at full depth**". It is now
"a reduced move that beats alpha is searched again", renamed on the
coordinator's decision at the Tier-1 check, in the title and in the
`M09_lmr_no_research` block above it, with the long comment explaining the old
one cut to a sentence. `adocs/specs.md` quotes the title and the coordinator
moves it in the same commit. What the case asserts -- that the re-search happens
at all -- is unchanged and still what `M09_lmr_no_research` breaks.

### The suite, the signature and the second tier

- Fast suite, Release: **39/39**. Fast suite, tune build: **39/39**.
  `./clang-format.sh --check` clean.
- `Bench: 4025871`.
- `tools/search_bench.py` depths 9 and 12 above; every best move the parent's.
- Debug self-play, DEC-141 clause 1, four rounds at 4+0.04 with the Debug
  binaries and `level=trace engine=true`: **8 games in 17 s, 0 `Assertion` in
  both the log and the tee'd stdout, 0 `disconnect`** on the repaired tree (8
  games in 21 s and the same zeros before it). The five `assert`s in
  `lmr_research_depth` -- the two preconditions and the three bounds -- never
  fired.
- `tests/test_search_params.cpp`'s golden gains three rows and its count moves
  48 -> 51, re-derived the way its own GOLDEN note says, by diffing it against
  `src/search_params.hpp`.
- `MANUAL.md`'s tune-option table gains `LmrDeeperMargin`,
  `LmrShallowerMargin` and `LmrDeeperMinReduction`; `DEV_MANUAL.md`'s bench
  ledger gains this verdict with the sign-changing ablation, and its DEC-142
  golden list moves `golden_defaults` 48 -> 51 and carries the new
  `capture_mates` row. `tests/test_uci_surface.cpp` needed no edit: it generates
  the option lines from `search_param_info` and requires `MANUAL.md` to document
  each name, which it now does.
- `tools/gate_extra.sh`, DEC-141 clause 3, before this verdict completes:
  **GATE-EXTRA-DONE 5 stages 1313 s** on the tree that lands, all five green --
  prose, citations, the Debug binaries (393 s), the sanitizer build (849 s) and
  deep perft (70 s). It has run three times and the first is worth the sentence:
  it went **red** on `test_plan_citation_freshness`, because this section cited a
  test title the source wraps across two lines and the checker looks for the
  phrase literally. The title was shortened to fit one source line -- "the
  re-search depth stays inside its cap, floor and inequality" -- and the gate
  re-run; the third run is the Tier-1 repair's, since it moved `src/`.
- `tools/mutation_check.py` over every registry file: **89 mutants across 10
  files, every anchor resolving exactly once** on the tree that lands, S222's
  `H03` and `H04` and verdict 2's nine `T` anchors included. The tool's own pass
  needs a linked worktree with a clean `src/` at `HEAD` and this verdict is not
  committed, so what ran is the anchor half of its check, over every file rather
  than only this step's.

### The Tier-1 fast check and its repair, 2026-09-17

The coordinator's fast check over this verdict's diff found two real problems
and one stale artefact, and carried one decision of its own. All four were
repaired in the same working tree.

**1. A site-level bug the whole suite could not see, and a fabricated evidence
line that hid it.** The `// Mutation:` block above "the node re-searches at the
depth the rule returns" named `D09_site_rebases_on_alpha` -- a mutant the
registry does not contain, since D09 had been rewritten into the rule -- and
quoted `values: CHECK_EQ( 7, 8 )`, which appears in no log. **The value-checking
script that reported "26 quoted values, 0 not observed" did not catch it**: it
matched only `values:` lines carrying a trailing mutant id, and that line had
none. It now matches **every** `values:` line in the verdict-3 block and fails a
line that names no mutant at all.

The block was not only mislabelled. The checker applied the bug it describes --
the call in `negamax_at` passing `alpha` where `best_so_far` belongs -- and the
fast suite **stayed green** while `bench` moved 4025871 to 4025922, so the
mutant is not equivalent and the green suite was a proved gap. The cause is
structural and is the one that made `D08` survive its own first pass: a case
that replays the rule on the numbers the node recorded moves both sides of its
comparison together, so no replay can see a call site handing over the wrong
variable.

The fix is in the rule. `lmr_research_depth` now returns `research_decision_t`
-- the depth **and the base it measured the deeper margin from**, echoed back --
and the probe records that echo in `research_base` beside `best_so_far` in
`research_best`, written from two different places. The new case
`tests/test_search.cpp` "the node measures the deeper margin from its own
fail-soft best" compares them at every site and counts the sites where the two
candidate bases differ, refusing to pass if none does. The bug is now
`D12_site_rebases_on_alpha` in the registry, anchored on the call, and it dies
there:

```
TEST CASE:  the node measures the deeper margin from its own fail-soft best
ERROR: CHECK_EQ( site.base, site.best ) is NOT correct!
  values: CHECK_EQ( -500, -504 )
```

The echo is behaviour-neutral by construction -- nothing outside the probe block
reads it, so it folds away in `negamax_at<false>` -- and by measurement:
`Bench: 4025871`, unchanged.

**2. The census counted the deeper condition and four documents published it as
the deeper path.** The rule tests deeper in the `else`, so the shallower path
takes precedence, and at depth 12 the two conditions overlap on 2.29 % of sites.
The script kept five one-dimensional histograms and no joint distribution, so it
could not say what the path's share was -- and DEC-214's inert test is about the
path. `adocs/data/S098_v3_research_census.py` now counts the three outcomes
**after precedence** in the engine, at the seeds it reads out of
`src/search_params.hpp` so they cannot drift, and the census was re-run on the
off tree with the bench-equality guard intact (`5469072`). The true path shares
are **4.60 % deeper and 47.77 % shallower at depth 12**, 3.97 % and 44.36 % at
depth 10, with 47.64 % of sites unchanged; the condition shares stay printed
beside them, labelled as conditions. The census file, this one,
`adocs/data/README.md`, `adocs/data/S098_v3_sprt.sh` and `DEV_MANUAL.md` all
carry the path shares now. The census file's claim that either path's share was
readable at any margin without re-running is corrected: that is true of a
condition and false of a path. Both paths clear the one-per-cent threshold, so
the reading is unchanged and neither ships inert.

**3. A stale snapshot.** `.tuning/coord/S230_v3/search.cpp.sha256` recorded the
tree before this section's own comment rewrite -- nothing functional, and the
checker re-ran the miner and got the recorded profiles byte for byte. Refreshed,
so the `capture_mates` evidence can be audited without seven rebuilds, and
`.tuning/coord/S230_v3/PROVENANCE.txt` beside it says what the refreshed tree
differs from the swept one by -- that comment, and the repair's struct return,
both behaviour-neutral and both benching 4025871 -- rather than letting the
checksum imply the sweeps were taken on it.

**4. The coordinator's edit, carried out here.** The case
"a reduced move that beats alpha is searched again at full depth" is renamed to
"a reduced move that beats alpha is searched again", in the title and in the
`M09_lmr_no_research` block above it, with the explanation cut to one sentence.
`adocs/specs.md`'s quotation of the title is the coordinator's, in the same
commit.

**What was re-run.** `src/search.cpp` changed, so the whole second tier was
taken again on the tree that lands: the twelve-mutant pass, `tools/gate_extra.sh`,
both fast suites, `clang-format.sh --check` and both prose checks. The list below
is re-stated for the repaired tree rather than left at the first landing's.

### Proposed for `adocs/specs.md`, for the coordinator to apply

The search row's late-move-reduction sentence gains, beside the verdict 2
sentence already there:

> **The depth a reduced move's re-search runs at answers that search since S098
> verdict 3**: `lmr_research_depth` takes `child_depth - 1` where the reduced
> score beat alpha by less than `LmrShallowerMargin` with a reduction of at
> least 2, `child_depth + 1` where it cleared the node's own fail-soft best by
> `LmrDeeperMargin` with a reduction of at least `LmrDeeperMinReduction`, and
> `child_depth` otherwise; the shallower path wins where both hold, the returned
> depth is always strictly greater than the reduced depth, and the full-window
> re-search is untouched. At `LmrShallowerMargin` 0 and `LmrDeeperMinReduction`
> at its range top the re-search runs at `child_depth` everywhere and the engine
> is the one before the verdict, bench signature included. `LmrDeeperMargin` at
> its range top is **not** an off value: the fail-soft best sits below alpha at
> every scout node, so the condition still fires there.

### Files

Changed: `src/search.cpp`, `src/search.hpp`, `src/search_params.hpp`,
`src/data_structures.hpp`, `tests/test_search.cpp`,
`tests/test_search_params.cpp`, `MANUAL.md`, `DEV_MANUAL.md`,
`adocs/data/README.md`, this file. Created:
`adocs/data/S098_v3_research_census.py`,
`adocs/data/S098_v3_research_census.txt`, `adocs/data/S098_v3_sprt.sh`,
`tools/mutants/S098_research_rule.py`.

**For the coordinator.** The `adocs/specs.md` sentence above is proposed, not
applied. `adocs/data/S098_v3_sprt.sh` has `REF` pinned at `efdbc9b`, `HEAD` at
this landing -- whose `src/` is `a771260`'s byte for byte, so it is the tree
verdict 2's H1 approved -- and `CAND` at `HEAD` to pin once the landing commit
exists; its open-findings paragraph says no finding is open, re-read on this
tree. Two items need a decision that is not this file's: the wrong test title
above, and whether section 4's off-value sentence for `LMR_DEEPER_MARGIN` is
amended in place or left with this section's correction beside it.

### Verdict 3's SPRT, 2026-09-17 (the coordinator's)

The landing committed as `cb40afd` after the fast check and its repair. The
check found two real problems and both were fixed before the commit. **One was
a test gap wearing a label's clothes**: an evidence block named a mutant that
does not exist and quoted values found in no log, and behind it the site
passing `alpha` where the rule measures from the fail-soft best was killed by
nothing in `test_search` -- a case that replays the rule on the site's own
recorded inputs moves both sides together and cannot see a site-level fault.
The rule now returns the base it measured from, a case compares that echo
against the node's own `best_so_far`, and `D12_site_rebases_on_alpha` is the
twelfth mutant. **The other was in the numbers four documents published**: the
census counted the deeper *condition*, not the deeper *path*, and the two
conditions overlap on 2.29 % of depth-12 sites, so the path's share after
precedence is 4.60 % and not 6.89 %; the script gained joint counters, was
re-run on the off tree at the same bench signature, and the census, the data
README, this SPRT's pre-registration and the `DEV_MANUAL.md` entry now separate
condition from path. All 21 quoted evidence lines and their 28 mutant
references were then re-verified against the kept logs, by the agent and by the
coordinator independently. `gate_extra` re-ran because `src/` changed:
`GATE-EXTRA-DONE 5 stages 1313 s`. Gate `GATE-DONE 4025871`, 39 of 39 in both
builds. `CAND` pinned as `cb40afd` in `ffa1f68`. DEC-215 records what the
census caught about the off value.

`adocs/data/S098_v3_sprt.sh` ran 13:48:41 to 15:55:26
(`adocs/data/S098_v3_sprt.log`, `adocs/data/S098_v3_sprt_pairs.txt`): banner
`candidate cb40afd`, `reference efdbc9b`, both identity lines right, seed
`20260917134841`, `8+0.08`, Hash 16, concurrency 12, `noob_3moves.epd`, bounds
`{0, 5}`.

**H0 accepted: `Elo -9.97 +/- 7.56`, `nElo -13.40 +/- 10.16`, LLR -2.96 against
(-2.94, 2.94), W 1282 L 1411 D 1803 over 4496 games, `Ptnml(0-2) [185, 595,
794, 512, 162]`, LOS 0.48 %, 2 h 06 m 45 s at 2128 games an hour.** The whole
interval sits below zero, so this is not a null but a measured loss. 0 time
forfeits on either side over the 4497 games the PGN holds (3104 adjudications,
1393 natural ends); 2248 complete pairs at pair-score variance 0.2766, beside
verdict 2's 0.2805. **One asymmetry is on the record**: `Incomplete mating PV`
8 on the candidate against 0 on the reference, where verdict 2's run split 14
against 9 and verdict 1's 7 against 3. A re-search a ply shallower than the
move's own depth is the obvious suspect for a mate line that arrives one ply
short, and it is recorded as an observation and not as a diagnosis -- what the
line is worth is a chess judgement and not this file's (CHESS).

### The bisection, leg 1, as pre-registered

H0 opens the two-leg bisection the pre-registration wrote before the games, one
path at a time and never the tune build (S073). **Leg 1 is
`LmrShallowerMargin` to 0, keeping the deeper path**, because the record's
argument is about the guard: the guarded form is what measured positive
elsewhere, the shallower path is the unguarded half by volume -- 47.77 % of
re-search sites against the deeper path's 4.60 % -- and it is what the -26.39 %
tree is mostly made of.

**The firing census is re-read first, as the pre-registration demands** (a path
that stopped firing is not a path that was measured, DEC-212). With the
shallower path off, precedence no longer takes anything from the deeper path,
so the deeper path fires at exactly its condition's share:
**6.89 % of re-search sites at depth 12 and 5.85 % at depth 10**
(`adocs/data/S098_v3_research_census.txt`), which is above DEC-214's one per
cent by a factor of six. The leg measures a path that fires, and the tree it
measures is the one the coordinator's fast check already benched at 4646334
against the off tree's 5469072.

## Verdict 3, bisection leg 1, 2026-09-17: the shallower path off

`LmrShallowerMargin` 47 -> **0**, with `LmrDeeperMargin` 47 and
`LmrDeeperMinReduction` 2 untouched. One default in `src/search_params.hpp`,
one release rebuild, and **no code path removed**: the shallower branch stays
in `lmr_research_depth` and is simply never taken, because the site requires
`score > alpha` and the branch asks for `score < alpha + 0`. That is what makes
the leg reversible in one more rebuild, which is what leg 2 needs of it. The
pre-registration is `adocs/data/S098_v3_leg1_sprt.sh`, written before a game is
played; `REF` is `efdbc9b`, the same reference verdict 3 used, and `CAND` is
`HEAD` for the coordinator to pin.

**The firing census was re-read first, as the pre-registration demands** -- a
path that stopped firing is not a path that was measured, which is what
verdict 1 cost the plan (DEC-212). With the shallower path off, precedence
takes nothing from the deeper path any more, so the deeper path fires at
exactly its condition's share, **6.89 % of re-search sites at depth 12 and
5.85 % at depth 10** (`adocs/data/S098_v3_research_census.txt`), against the
4.60 % / 3.97 % the same census measured for the path after precedence. Six
times DEC-214's one per cent, and **more** sites than the path had in the tree
that lost.

### The tree, measured before the games

Node counts at a fixed depth are not Elo and nothing here reads them as Elo
(DEC-019).

- `chesso bench` (depth 14): **4646334**, against the shipped verdict-3 tree's
  4025871 (+15.41 %) and the reference `efdbc9b`'s 5469072 (-15.04 %). It is the
  total the coordinator's own fast check predicted for this default, so the leg
  is the tree that was already benched and not a new one.
- `tools/search_bench.py`, reference -> candidate. Depth 9: midgame
  22078 -> 21995, kiwipete 104682 -> 104682, tactical 29842 -> 29842 -- two of
  the three node-identical to the reference. Depth 12: midgame
  205096 -> 155612, kiwipete 646466 -> 683624, tactical 149330 -> 152138, so
  the three disagree in direction, one a quarter smaller and two a few per cent
  larger. **Every best move is the reference's at both depths**, `g5f6` /
  `e2a6` / `d7c8q` at 9 and `c3d5` / `e2a6` / `d7c8q` at 12: verdict 3 moved
  midgame's depth-9 answer `g5f6` -> `c3d5` and this leg puts it back.
- The by-depth ablation on the tune build, the shipped seeds against
  `LmrDeeperMinReduction` 126 -- with the shallower path already off, that is
  the whole rule's off value:

| depth | 9 | 10 | 11 | 12 | 13 | 14 |
|---|---|---|---|---|---|---|
| on | 475656 | 812822 | 1294648 | 2406743 | 3238952 | 4646334 |
| off | 475717 | 841594 | 1344741 | 2453847 | 3447081 | 5469072 |
| delta | -0.01 % | -3.42 % | -3.73 % | -1.92 % | -6.04 % | -15.04 % |

**The off column is the reference's totals exactly at every depth**, which is
the property the bisection rests on and is measured rather than argued.

**The sign change is gone with the shallower path, and that attributes it.**
Verdict 3's row read +12.61, -0.65, +4.10, -18.40, -20.32, -26.39 per cent; the
deeper path alone is smaller at every depth, flat at 9, with no crossover to
read. The only thing that moved between the two rows is the shallower path, so
the shallow-depth growth verdict 3 measured was that path's. What is left is
what the census predicts of a path firing on 7 % of re-search sites and
spending a ply at each: the ply buys a table entry a ply deeper, and what that
saves grows with the depth it is re-used at.

### The tests: one arm per configuration, and both arms assert

Nothing was relaxed, skipped or deleted. Four cases carried a precondition that
is false at a margin of 0 -- three `REQUIRE(LMR_SHALLOWER_MARGIN > 1)` and one
`REQUIRE(LMR_SHALLOWER_MARGIN > 0)` -- and each was re-derived so that the case
asserts the consequence of the configuration it is compiled in, with both arms
counting what they examined the way the drive cases count their sites.

**Renamed, and the renames are the honest half of the work.** Three titles
stated behaviour the shipped tree does not have at a margin of 0, so each names
the constant that decides it instead and is true at either setting -- which is
also what leg 2 needs, since a title that flips with the configuration would be
renamed twice:

| was | is |
|---|---|
| "a re-search that only just beat alpha goes a ply shallower" | "LmrShallowerMargin decides whether a re-search that only just beat alpha goes a ply shallower" |
| "where both re-search paths could fire the shallower one wins" | "LmrShallowerMargin decides the region where both re-search paths could fire" |
| "both re-search depths are reached in a real search" | "every re-search depth the margins admit is reached in a real search" |

**Four cases changed.**

1. **The shallower path's own case.** At a margin above 1 it is verdict 3's
   case exactly. At 0 it asserts what the constant implies: the same input that
   used to read `child_depth - 1` reads `child_depth`, and then a sweep over
   reductions 2 to 6 -- the range the census saw at real sites -- and scores
   one point, two points and a whole deeper margin above alpha, skipping the
   ones the deeper path takes so that what is read is the absence of the
   shallower path and not the precedence. Fifteen checks, counted, with
   `REQUIRE(examined > 0)` beside them.
2. **The precedence case.** With the shallower path off, "both could fire"
   cannot arise, and the case is kept honest by asserting exactly that rather
   than by standing down: it walks the region the precedence was written for --
   the fail-soft best more than a deeper margin below alpha, at three depths of
   `best` and five reductions -- asserts the shallower condition **false** at
   each point, and asserts the deeper path takes every one of them at
   `child_depth + 1`. Fifteen points, counted, `REQUIRE(in_the_region > 0)`.
   The region that belonged to the shallower path now belongs to the deeper
   one, which is a statement about this tree and not about the rule.
3. **"every re-search depth the margins admit is reached in a real search".**
   `REQUIRE(deeper > 0)` holds at either setting; the shallower count is
   `REQUIRE(shallower > 0)` where the margin admits the path and
   `CHECK_EQ(shallower, 0)` where it does not, which is the same rule read from
   its other end. The per-site assertions -- a deeper site's reduction is at
   least `LmrDeeperMinReduction`, a shallower site's at least 2, everything
   else at `child_depth` -- are untouched, and the case now reports its three
   counts.
4. **"the deeper margin is measured from the fail-soft best and not from the
   window".** Its `REQUIRE(LMR_SHALLOWER_MARGIN > 0)` is gone and **nothing
   replaces it**, which is a removal and not a relaxation: it asserted that the
   shallower path was switched on, which this case never needed. What it needs
   is that the shallower path does not take *this* score, and the case already
   states that directly, at any margin, with `REQUIRE_FALSE` on the shallower
   condition itself. The rest of the case is verdict 3's.

What the drive reports on this tree, from the two cases' own `MESSAGE` lines:
**305 re-search sites, 303 with a child entry at the rule's depth or deeper, 2
shallower and none missing**, and by outcome **39 deeper, 266 unchanged, 0
shallower** -- which is the leg stated as counts rather than as an argument.

**The drive fixture's stop rule is unchanged from verdict 3**, and the first
draft of this leg changing it is the fast check's finding 1 below.

**What the leg does not change, and each is still non-vacuous.** "a re-search
whose score clears the fail-soft best goes a ply deeper" picks its score as
`max(LmrDeeperMargin + 1, LmrShallowerMargin)`, which is 48 at either setting,
and its guard sub-assertion at `LmrDeeperMinReduction - 1` is untouched. "the
re-search depth stays inside its cap, floor and inequality" sweeps the rule's
whole declared domain and counts it; its floor corner was already behind
`if (LMR_SHALLOWER_MARGIN > 1)` and its own comment already said why, and at a
margin of 0 the sweep's `LMR_SHALLOWER_MARGIN + 1` score collapses onto its
`1`, so the domain is three distinct scores rather than four and every point is
still checked. "the node re-searches at the depth the rule returns" and "the
node measures the deeper margin from its own fail-soft best" are untouched, and
the second still separates its two bases: `separating` counts the sites whose
fail-soft best is strictly below their window, which a scout node produces at
any margin.

### One case lost an assertion the engine never promised, and the leg is how it was found

"a re-search that went a ply deeper left a table entry a ply deeper" asserted
`CHECK(site.table_depth >= site.depth)` at **every** site. On this tree that
went red twice, `CHECK( 4 >= 7 )`, at 2 sites of the 305 the old sweep
collected.

It is not the site disobeying the rule and it is not a defect: **a node can
return without storing**. `negamax_at` reaches its one store past its move
loop, and a null-move cutoff returns `null_score`, a reverse-futility cutoff
returns `static_eval - margin`, a draw returns `DRAW_SCORE` and a table answer
returns `tt_score`, all of them before it. A child re-searched at depth d that
returns on one of those paths leaves the slot as it was -- empty, which the
case already counted as `entryless`, or holding an earlier and shallower
visit's entry for the same position, which reads as `table_depth < d`. The
table is depth-preferred inside one search, so a later shallower visit cannot
have overwritten the deeper entry; the only reading left is that the deeper
search stored nothing. The two classes cannot be told apart from here, and a
per-site `table_depth >= depth` over all sites is therefore a claim the engine
does not make -- it held on verdict 3's tree and stopped holding on this one.

What replaces it is what the table can carry: `REQUIRE(witnessed > 0)`, the
kill this case was written for and the one thing only the table can say -- an
entry at `child_depth + 1` exists exactly where the rule asked for one, and the
fast check confirmed `D08_site_ignores_the_rule` still dies there and nowhere
else in the whole fast suite -- plus
`CHECK(deep_enough > short_entry + entryless)` over the **whole** file, which
reads `CHECK( 303 > 2 )`, and a `MESSAGE` carrying all three counts. This is
recorded as a finding of the leg rather than as a tidy-up: the assertion was
over-strong from the day it was written and no tree had exposed it.

**What the two exempt sites are, instrumented by the fast check rather than
inferred**: both read `child_depth 7, reduction 3, depth 7, table_depth 4`, so
both are *unchanged* sites -- the rule asked for `child_depth`, the re-search
ran there and returned through one of `negamax_at`'s early exits without
storing, and the reduced search's own depth-4 entry stayed in the slot because
`tt_store_entry` is depth-preferred inside one search. The engine is not
failing to store a deeper entry where it should, and no finding was raised
against it.

### The mutants: eight killed, four equivalent on this configuration

The same twelve of `tools/mutants/S098_research_rule.py`, every one applied to
the working tree, built, run through the **whole fast suite** in the Release
build and reverted from a byte snapshot whose sha256 was compared after. The
driver is `.tuning/coord/run_mutants_v3_leg1.py` and every ctest log is kept
under `.tuning/coord/S098v3_leg1_mutlogs/`. The pass could not go through
`tools/mutation_check.py` for the reason verdict 3's could not: that tool wants
a linked worktree whose `src/` is clean at `HEAD` and reverts with
`git checkout --`, and this leg is not committed. What the hand-driven pass does
carry is the tool's **second oracle** -- the bench total and the eight best
moves, read after every mutant -- because an equivalence claim needs a still
signature and a moved one refutes it whatever a registry declares.

| mutant | verdict | bench | killed by, with the values it printed |
|---|---|---|---|
| `D01_deeper_inverted` | KILLED | 4397777 | "a re-search whose score clears the fail-soft best goes a ply deeper" `CHECK_EQ( 6, 7 )`; "LmrShallowerMargin decides whether a re-search that only just beat alpha goes a ply shallower" `CHECK_EQ( 7, 6 )`; "LmrShallowerMargin decides the region where both re-search paths could fire" `CHECK_EQ( 6, 7 )`; "the deeper margin is measured from the fail-soft best and not from the window" `CHECK_EQ( 6, 7 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D02_shallower_inverted` | KILLED | 4262065 | "LmrShallowerMargin decides whether a re-search that only just beat alpha goes a ply shallower" `CHECK_EQ( 5, 6 )` -- **the off arm, and it is a stronger kill than the one at 47**; "... clears the fail-soft best ..." `CHECK_EQ( 5, 7 )` and `CHECK_EQ( 5, 6 )`; "... the region where both re-search paths could fire" `CHECK_EQ( 5, 7 )`; "... from the fail-soft best ..." `CHECK_EQ( 5, 7 )`; "a re-search that went a ply deeper left a table entry a ply deeper" `REQUIRE( 0 > 0 )`; "every re-search depth the margins admit is reached in a real search" `REQUIRE( 0 > 0 )`; "a reduced move that beats alpha is searched again" `REQUIRE( 0 > 0 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D03_deeper_guard_dropped` | KILLED | 5520483 | "a re-search whose score clears the fail-soft best goes a ply deeper", on the guard's own sub-assertion, `CHECK_EQ( 7, 6 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D04_shallower_guard_dropped` | **EQUIVALENT** | same | nothing, and that is the declaration: the condition the guard sits on is false at every site, so dropping the guard admits nothing. 39 of 39 green, signature still |
| `D05_precedence_swapped` | **EQUIVALENT** | same | nothing: the region where both conditions hold is empty, so the two orders decide every site alike. 39 of 39 green, signature still |
| `D06_deeper_two_plies` | KILLED | 4466060 | "the re-search depth stays inside its cap, floor and inequality" `CHECK( 3 <= 2 )`; "... clears the fail-soft best ..." `CHECK_EQ( 8, 7 )`; "the node re-searches at the depth the rule returns" `CHECK( 9 <= 8 )`; "every re-search depth ..." `CHECK_EQ( 9, 7 )` and `REQUIRE( 0 > 0 )`; "... a table entry a ply deeper" `REQUIRE( 0 > 0 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D07_shallower_two_plies` | **EQUIVALENT** | same | nothing: the branch it rewrites is never taken. 39 of 39 green, signature still |
| `D08_site_ignores_the_rule` | KILLED | 5469072 | "a re-search that went a ply deeper left a table entry a ply deeper" `REQUIRE( 0 > 0 )`, **and nothing else in the suite** -- the same one case that was written for it at verdict 3. Its bench is the off tree's exactly, which is what the site ignoring the rule means |
| `D09_deeper_margin_off_alpha` | KILLED | 4940530 | "the deeper margin is measured from the fail-soft best and not from the window" `CHECK_EQ( 6, 7 )`; "... the region where both re-search paths could fire" `CHECK_EQ( 6, 7 )`; `test_mate_carry` "a mate score carried across searches keeps a line that reaches it" `CHECK( 12 <= 9 )` |
| `D10_floor_dropped` | **EQUIVALENT** | same | nothing: the rule returns `child_depth` or `child_depth + 1` here and the site's own reduction of at least 1 forces `child_depth >= 2`, so the floor is unreachable. 39 of 39 green, signature still |
| `D11_cap_one_ply_low` | KILLED | 5469072 | "a re-search whose score clears the fail-soft best goes a ply deeper" `CHECK_EQ( 6, 7 )`; "... from the fail-soft best ..." `CHECK_EQ( 6, 7 )`; "... the region where both re-search paths could fire" `CHECK_EQ( 6, 7 )`; "every re-search depth ..." `REQUIRE( 0 > 0 )`; "... a table entry a ply deeper" `REQUIRE( 0 > 0 )` |
| `D12_site_rebases_on_alpha` | KILLED | 4940530 | "the node measures the deeper margin from its own fail-soft best" `CHECK_EQ( -500, -504 )`; `test_mate_carry` `CHECK( 12 <= 9 )` |

**The four equivalences are a fact about the configuration and not about the
bugs**, and they are declared in the registry with that qualification and never
deleted: at `LmrShallowerMargin` 0 the shallower branch is unreachable, so a
mutant that only changes it (`D04`, `D07`), only reorders it against the deeper
branch (`D05`) or moves a floor only it could reach (`D10`) changes nothing any
input reaches. Each declaration names the case that kills it again at leg 2's
margin of 47, with the values verdict 3 observed, so restoring the margin
restores four kills without anyone having to rediscover them.

**Two kills changed hands and both are recorded rather than smoothed over.**
`D01` and `D11` used to die in "a re-search that went a ply deeper left a table
entry a ply deeper"; on this tree `D01` still produces deeper re-searches -- at
a scout node `score < best + margin` is common -- so that case no longer sees
it, and `D01` dies in four other cases instead. `D02`, by contrast, gained a
kill: at a margin of 0 inverting the shallower condition turns "fires nowhere"
into "fires at every site with a reduction of at least 2", which is exactly what
the off arm asserts against.

**Every `// Mutation: D...` evidence block in `tests/test_search.cpp` was
rewritten from these logs and then checked back against them by script**
(`.tuning/coord/S098v3_leg1_evidence_check.py`, which is stricter than verdict
3's: it requires each `values:` line to be in the named mutant's own log **and**
that mutant's log to hold a failure in the case the block names). **9 blocks, 21
`values:` lines, 31 mutant references, all observed.** The four quotes of
verdict 3's own observations that the equivalence notes carry were checked the
same way against `.tuning/coord/S098v3_mutlogs/`.

### `capture_mates` re-derived, and three of the four rows moved

DEC-142: a golden is re-derived by its own script whenever **either** end
moves, and switching the shallower re-search path off moves the same rule the
three verdicts moved. Seven sweeps of `adocs/data/S230_mine_r01_row.py depths`
over `adocs/data/S230_table_fens.txt`, depths 3 to 12, once on this tree and
once per mutant of `tools/mutants/S091_capture_see.py` applied by hand and
reverted from a byte snapshot; the raw profiles are in
`.tuning/coord/S230_v3_leg1/` and the rule is applied by script
(`.tuning/coord/S230_leg1_rows.py`) rather than by eye.

Shipped profiles: `d9 d10 d11 d12`, `d8 d9 d10 d11 d12`, `d11 d12`,
`d9 d10 d11 d12`. The rule written at the table -- the lowest depth in the
shipped profile at which some mutant loses the mate, else the lowest shipped
depth with the label saying nothing separates -- then gives:

| row | depth, was -> is | label, was -> is |
|---|---|---|
| 1 | 9 -> 9 | `no S091 mutant` -> `no S091 mutant` |
| 2 | 9 -> **8** | `no S091 mutant` -> **`C02, C05, C07 and R02`** |
| 3 | 10 -> **11** | `C02, C07 and R02` -> **`C07 and R02`** |
| 4 | 9 -> **10** | `R02` -> `R02` |

Three depths moved and no mate distance did, and the case went red on row 3
before any of them was touched -- `REQUIRE( result.mate_found )` at depth 10,
which is the golden's own end moving and is why DEC-142 asks for the script.
Row 2 comes back to 8 because the shipped build reports that mate at 8 again,
where verdict 3's tree did not, and four mutants lose it there. **R01 is
separated by no row at any depth its profile covers**, as at verdict 3; the
direct guards are what the S091 rules rest on and a label is an incidental
second kill in a tree that moves under every ordering change.

### The suite, the signature and the second tier

- Fast suite, Release: **39/39**, `test_search` 4.44 s. Fast suite, tune build:
  **39/39**. `./clang-format.sh --check` clean.
- `Bench: 4646334`.
- The red was observed before anything was touched and is kept in
  `.tuning/coord/S098v3_leg1_red_first.log`: three
  `REQUIRE( LMR_SHALLOWER_MARGIN > 1 )`, one `REQUIRE( LMR_SHALLOWER_MARGIN >
  0 )`, two `CHECK( 4 >= 7 )` in the table-entry case, the `capture_mates` row
  at `REQUIRE( result.mate_found )`, the golden's
  `CHECK( param.default_value == golden_defaults[i].value )` and
  `test_plan_params` on `MANUAL.md`'s row. Every one of them is a thing this
  section had to answer.
- Debug self-play, DEC-141 clause 1, four rounds at 4+0.04 with the Debug
  binaries and `level=trace engine=true`: **8 games in 17 s, 0 `Assertion` in
  both the log and the tee'd stdout, 0 `disconnect`**. The five `assert`s in
  `lmr_research_depth` -- the two preconditions and the three bounds -- never
  fired.
- `tools/mutation_check.py`'s anchor half over every registry file: **89
  mutants across 10 files, every anchor resolving exactly once** on this tree,
  and the tool now reads five `expected="equivalent"` declarations, `M26` and
  this leg's four. Its own pass needs a linked worktree with a clean `src/` at
  `HEAD` and this leg is not committed, so the hand-driven pass above is what
  ran the mutants.
- `tools/gate_extra.sh` was **not** run and is not owed: this leg changes
  `src/` by one default and the comment that records it, and nothing else. Its
  two prose stages were run by hand anyway -- `plan_prose_check.py --citations`
  is 0 flagged over 46 files and `--params` is silent -- and the four prose
  checks in the fast suite are green in both builds.
- `tests/test_search_params.cpp`'s golden row moves 47 -> 0, re-derived the way
  its own GOLDEN note says: there is no script and none is owed, because
  `src/search_params.hpp` is the derivation and a diff of the two is the
  re-derivation. Its count does not move.
- `MANUAL.md`'s tune-option row carries the default and now says the shipped
  value **is** the off value and why the range keeps its 47.
  `DEV_MANUAL.md`'s bench ledger gains this leg, and its DEC-142 golden list
  carries the new `capture_mates` depths and labels.
  `tests/test_uci_surface.cpp` needed no edit: it generates the option lines
  from `search_param_info`, so the default it compares is the one it prints.

### The Tier-1 fast check and its two repairs, 2026-09-17

The check cleared the judgement call the table case rests on and found two
things to repair. **The judgement call stands**: it reached both short-entry
sites and instrumented them -- `child_depth 7, reduction 3, depth 7,
table_depth 4` at both, so both are unchanged sites holding the reduced
search's own entry, left there by a re-search that returned through an early
exit without storing -- so no defect was raised against the engine and the
per-site assertion really was a claim the engine does not make.

**Finding 1: the case was green for the wrong reason.** This leg's first draft
also taught the drive fixture's stop rule to skip an outcome the constants had
switched off (`wanted_shallower`), which ended `scan(3)` at the third deeper
site: the case then read **24 sites and `CHECK( 24 > 0 )`** and never reached
the class its own comment documents. Two changes had been made where one was
needed, and the weaker one is what leg 2 would have inherited. The stop rule is
reverted to verdict 3's, and the case asks for the whole file with `scan(0)`
instead, so its reach is the same at a live margin and at 0: **305 sites, 303
at the rule's depth or deeper, 2 shallower, 0 missing, `CHECK( 303 > 2 )`**.
`REQUIRE(witnessed > 0)` is untouched, since it is `D08`'s only killer in the
whole fast suite.

**Finding 2: a stale label.** `capture_mates` row 1 still read
`no S091 mutant, since S098 verdict 3` although the whole table was re-derived
here, and `DEV_MANUAL.md`'s golden row repeated it. Both now say
`no S091 mutant, since S098 verdict 3's bisection leg 1`; the depths and the
other three labels are the script's and did not move.

Neither repair is in `src/`, and `bench` still prints **4646334**.

### One thing this leg breaks and does not fix

`adocs/data/S098_v3_research_census.py` will not run on this tree. Its
`OFF_PATCH` rewrites the literal line `X(LMR_SHALLOWER_MARGIN, ..., 47, 0, 94)`
to the same line with 0, and that line no longer exists, so `apply_patch` exits
with "census patch anchor is not unique" before it builds anything. **It fails
loudly and it cannot produce a wrong census**, which is why it is recorded here
rather than fixed inside a leg that is meant to move one default: the fix is to
let that patch tolerate a worktree that already carries the off value, and it
belongs in whichever step next needs a census. Leg 2 restores the margin to 47
and the script works again unchanged.

### Proposed for `adocs/specs.md`, for the coordinator to apply

The search row's late-move-reduction sentence, which verdict 3 added, gains a
clause:

> **Leg 1 of verdict 3's bisection ships `LmrShallowerMargin` at 0 since
> 2026-09-17**, so the shallower path is switched off: the re-search runs at
> `child_depth + 1` where the deeper path takes it and at `child_depth`
> everywhere else. The branch and its arithmetic `reduction >= 2` guard stay in
> `lmr_research_depth`, unreachable at that default, because leg 2 restores the
> margin to 47 if this leg reads H0.

### Files

- `src/search_params.hpp` -- one default, 47 -> 0, and the paragraph above the
  three constants that records the leg and why the range does not move.
- `tests/test_search.cpp` -- four cases re-derived and three of them renamed,
  the drive fixture's stop rule, the table-depth case's assertion, and
  `capture_mates`'s four rows with the paragraphs that carry their
  re-derivation.
- `tests/test_search_params.cpp` -- the golden's `LmrShallowerMargin` row,
  re-derived the way its own GOLDEN note says, by diffing the table against
  `src/search_params.hpp`.
- `tools/mutants/S098_research_rule.py` -- four `expected="equivalent"`
  declarations, each qualified by the configuration and each naming the case
  that kills it again at leg 2, and the header paragraph that states the class.
- `MANUAL.md` -- the tune-option row's default and the sentence that says what
  0 means here.
- `DEV_MANUAL.md` -- the bench ledger's leg entry and the DEC-142 golden list's
  `capture_mates` row.
- `adocs/data/S098_v3_leg1_sprt.sh` -- new, the leg's pre-registration,
  and its row in `adocs/data/README.md`, which is the index every file
  there is registered in.
- This file -- this section, and one citation in the verdict-3 section updated
  to the renamed case title.

Evidence kept outside the repository, under `.tuning/coord/`:
`S098v3_leg1_mutlogs/` (one ctest log per mutant), `S230_v3_leg1/` (the seven
sweeps), `S098v3_leg1_red_first.log` (the red observed before any test was
touched), and the three drivers -- `run_mutants_v3_leg1.py`,
`S230_leg1_sweeps.py` and `S098v3_leg1_evidence_check.py`.

### Leg 1's SPRT, 2026-09-18 (the coordinator's)

`adocs/data/S098_v3_leg1_sprt.sh` ran 2026-09-17 17:34:22 to 2026-09-18
00:23:23 (`adocs/data/S098_v3_leg1_sprt.log`,
`adocs/data/S098_v3_leg1_sprt_pairs.txt`): candidate `8d60551`, reference
`efdbc9b` -- the same reference verdict 3 measured against, which is what makes
the two runs comparable -- seed `20260917173422`, `8+0.08`, Hash 16,
concurrency 12, `noob_3moves.epd`, bounds `{0, 5}`.

**H1 accepted: `Elo 5.75 +/- 4.37`, `nElo 7.44 +/- 5.65`, LLR 2.97 against
(-2.94, 2.94), W 4559 L 4319 D 5632 over 14510 games, `Ptnml(0-2) [603, 1695,
2522, 1729, 706]`, LOS 99.51 %, 6 h 49 m 01 s at 2128 games an hour.** 0 time
forfeits on either side over the 14510 games the PGN holds (10057
adjudications, 4453 natural ends); 7255 complete pairs at pair-score variance
0.2981, the highest of the S098 family and beside verdict 3's 0.2766. The
`Incomplete mating PV` asymmetry verdict 3's run showed -- 8 candidate against
0 reference -- is gone here at 28 against 21 over three times the games, which
is consistent with the shallower re-search having been its cause and is
recorded as an observation and not a diagnosis (CHESS).

**Read against the pre-registration's H1 line: the deeper path alone gains at
least 5 nElo and the shallower path was the loss, so the shallower path leaves
and S098 completes on the deeper path.** The claim written is at least 5 nElo
and not the stopping estimate (DEC-063). No second leg is taken: leg 2 exists
only for the case where leg 1 also reads H0.

**What the pair of runs says, and it is worth stating once.** The rule measured
whole read `Elo -9.97 +/- 7.56` and one of its halves reads `+5.75 +/- 4.37`
against the same reference. A rule can be a loss whose parts are not, and the
two runs cost 8 h 56 m together -- less than the single longest verdict in the
ledger. The bisection was pre-registered before either number existed, which is
the only reason the second run reads as a measurement rather than as a search
for a better answer (DEC-063).

## Verdict 3, the shallower path removed, 2026-09-18

Leg 1 read **H1**: `Elo 5.75 +/- 4.37`, `nElo 7.44 +/- 5.65`, LLR 2.97 over
14510 games in 6 h 49 m against `efdbc9b`, the same reference the verdict used.
`adocs/data/S098_v3_leg1_sprt.sh` pre-registered that reading before a game was
played, and pre-registered what follows from it: **the shallower path leaves and
S098 completes on the deeper path.** This section is that removal, in DEC-194's
shape -- the surviving half stays exactly as it was measured, and nothing of the
removed half is left behind as dead code or a dead constant.

Implemented by an Opus 5 subagent briefed by the coordinator (DEC-185,
DEC-199). **No `done:` stamp here: the coordinator writes it.**

### Behaviour-neutral, and that is what makes it free of an SPRT

The removal deletes a branch **no input reached**. `LmrShallowerMargin` already
shipped at 0 and the site requires `score > alpha`, so `score < alpha + 0` was
false everywhere and the branch was unreachable before the code went. INV-6 asks
for that to be proved rather than argued, and all three instruments say the same
thing:

| instrument | leg 1 | this tree |
|---|---|---|
| `chesso bench` (depth 14) | 4646334 | **4646334** |
| `tools/search_bench.py` 9 | 21995 / 104682 / 29842, `g5f6` / `e2a6` / `d7c8q` | **identical** |
| `tools/search_bench.py` 12 | 155612 / 683624 / 152138, `c3d5` / `e2a6` / `d7c8q` | **identical** |
| tune build at `LmrDeeperMinReduction` 126 | 5469072 | **5469072** |

Every count and every best move, both depths, both builds. The fourth row is
the one that says the *rule's* off value still restores the reference exactly:
with the shallower path gone, `LmrDeeperMinReduction` at its range top is the
whole rule's off switch, and the tree it leaves is `efdbc9b`'s to the node. A
change that plays identical games is decided by INV-6 and not by an SPRT, and
DEC-213's removal is the precedent for the shape.

### What left the engine

- **`src/search.cpp` `lmr_research_depth`, the shallower branch.** What remains
  is `child_depth + 1` where the reduction is at least `LmrDeeperMinReduction`
  and the score clears the fail-soft best by `LmrDeeperMargin`, and
  `child_depth` otherwise.
- **The `reduction >= 2` guard**, which existed only to keep that branch off a
  depth the reduced search had already run.
- **The floor at 1.** It could only bind below `child_depth`, and nothing goes
  there now. The bound is still declared and asserted: it is a consequence of
  the preconditions instead of something a clamp produces, and
  `assert(child_depth >= 1)` is written beside the other two preconditions so
  the consequence has a premise. The site satisfies it by construction --
  `child_depth` is `depth - 1` and the reduction is eligible only from depth 3.
  **The cap stays** and the asymmetry is the point: S097 extends `child_depth`
  and S127 tunes the branch set, and the cap still has two killable mutants
  where the floor now has none.
- **The constant itself**, `LmrShallowerMargin`, out of the
  `CHESSO_SEARCH_PARAMS` X-macro in `src/search_params.hpp` with its half of the
  comment. The count drops 51 -> 50.
- **`alpha` stays a parameter and that is deliberate**, stated because it is the
  one thing the removal leaves looking unused. It carries the function's own
  precondition, `score > alpha`, asserted in the Debug build; and it is the
  wrong variable the deeper margin could be measured from, which is exactly what
  `D09_deeper_margin_off_alpha` mutates. A signature without it is a signature
  that bug cannot be written against. `[[maybe_unused]]` is what keeps the
  Release build's `-Werror=unused-parameter` quiet, since the assert compiles
  out there.

### The tests: no case deleted, the arms are what went

TESTS makes a deletion a recorded decision. **Nothing was deleted**: all nine of
verdict 3's cases are still in `tests/test_search.cpp`. What went is the arm
each carried for the configuration that no longer exists -- the
`if (LMR_SHALLOWER_MARGIN > 1)` half -- and in every case the arm that stayed is
the arm this tree was already compiling, so no case changed what it asserts
about the engine that ships.

**Two were re-derived and renamed**, because their titles named the removed
constant and would otherwise have named a thing the tree does not have:

| was | is | what it holds now, and what kills it |
|---|---|---|
| "LmrShallowerMargin decides whether a re-search that only just beat alpha goes a ply shallower" | "a re-search that did not clear the fail-soft best is left at its own depth" | the **otherwise** branch: over reductions 2 to 6 and four score classes under the margin, counted with `REQUIRE(examined > 0)`, the rule answers `child_depth`. `D01_deeper_inverted` deepens exactly those inputs and dies here at `CHECK_EQ( 7, 6 )` |
| "LmrShallowerMargin decides the region where both re-search paths could fire" | "a score that barely beat alpha goes a ply deeper when the fail-soft best is far below it" | the region the surviving path actually fires in -- a score one point over the window, a fail-soft best a margin or more below it, three depths of `best` and five reductions, counted with `REQUIRE(in_the_region > 0)`. That is what 6.89 % of real re-search sites look like, and it is why the deeper path is reachable at a scout node at all. `D01`, `D06`, `D09` and `D11` die here |

**Two were re-derived without a rename**, because their subject survived:

- "a re-search whose score clears the fail-soft best goes a ply deeper" picks
  its score as `LmrDeeperMargin + 1` instead of the maximum of the two margins
  -- the same number at either setting -- and its `on_the_margin` check is now
  guarded by the rule's own precondition alone. Its sub-assertion at
  `LmrDeeperMinReduction - 1` is untouched and is `D03`'s only killer.
- "the deeper margin is measured from the fail-soft best and not from the
  window" loses two preconditions that spoke about the shallower condition and
  keeps every assertion it made. `REQUIRE(LMR_DEEPER_MARGIN > 0)` stays, which
  is what makes the two bases separable at all.

**Three mentioned the path in a sweep and were re-derived, not deleted:**

- "the re-search depth stays inside its cap, floor and inequality" sweeps
  `LmrDeeperMargin + 1` where it swept `LmrShallowerMargin + 1`, which keeps four
  distinct score classes over the rule's whole declared domain, and its floor
  corner went with the floor. The title keeps its three bounds because the rule
  still declares and asserts all three; what the case's own comment now says
  plainly is that the lower one is a bound **no mutant on the registry can
  move**, so a green suite is not what that claim rests on. `D06` dies here at
  `CHECK( 3 <= 2 )`.
- `research_drive_t`'s stop rule waited for **both** outcomes, which at a margin
  of 0 meant it never fired and the sweep ran the whole file. It now waits for
  the one outcome the rule has, and the single caller that passed `scan(3)`
  passes `scan(0)` -- so all four drive cases read the same 305 sites they
  already read and no case's reach moves with a constant. That is leg 1's own
  finding 1 kept rather than undone.
- "every re-search depth the margins admit is reached in a real search" is
  renamed to "every re-search depth the rule admits is reached in a real
  search", since one margin decides it now, and its two arms collapse to
  `REQUIRE(deeper > 0)` and `CHECK_EQ(shallower, 0)`. **The `shallower` counter
  is kept on purpose**: it counts sites the rule sent below `child_depth`, which
  is the removal's own invariant, and a path that comes back is a path a count
  catches.

`capture_mates`'s paragraph gained the reason it was not re-swept, and one
citation in verdict 3's section above was updated to a renamed title, the way
leg 1 updated one for the same reason. Nothing else in the earlier sections was
touched.

### The mutants: five left with the branch they broke, seven were re-run

`tools/mutants/S098_research_rule.py` drops to **seven**. The five that go are
the four leg 1 had to declare `equivalent` plus `D02`, and each goes because its
anchor no longer exists -- not because it stopped being observable:

| mutant | why it goes |
|---|---|
| `D02_shallower_inverted` | inverts the shallower condition; there is no condition |
| `D04_shallower_guard_dropped` | drops the `reduction >= 2` guard; there is no guard |
| `D07_shallower_two_plies` | rewrites `depth = child_depth - 1`; there is no such line |
| `D05_precedence_swapped` | reorders two branches; there is one branch |
| `D10_floor_dropped` | moves the floor; the floor went with the branch that could reach it |

Ids are never reused, so the next mutant of this rule is `D13`. Three anchors of
the survivors moved with the code and were re-pointed -- `D01` and `D09` onto
the deeper condition's new indentation, `D03` onto the `if` that was an
`else if`. Two descriptions were corrected while they were being read: `D03`
**gains** the `(void)` its now-orphaned `reduction` needs, which the first pass
caught as a build failure under `-Werror`; `D09` and `D08` **lose** a `(void)`
clause their prose promised and their pairs never carried, and neither needs one
-- the rule echoes `best` out in its return and the probe block reads the local
in the probing instantiation.

**All seven were run by hand against the tree that lands**, applied, built, put
through the **whole fast suite** in the Release build and reverted from a byte
snapshot whose sha256 was compared after; the driver is
`.tuning/coord/run_mutants_v3_removal.py` and every ctest log is kept under
`.tuning/coord/S098v3_removal_mutlogs/`. The pass carries
`tools/mutation_check.py`'s second oracle -- the bench total and the eight best
moves after every mutant -- and **every one of the seven moved the signature**,
so none of them is equivalent by accident. **All seven KILLED, none unexpected.**

| mutant | bench | killed by, with the values it printed |
|---|---|---|
| `D01_deeper_inverted` | 4397777 | "a re-search whose score clears the fail-soft best goes a ply deeper" `CHECK_EQ( 6, 7 )`; "a re-search that did not clear the fail-soft best is left at its own depth" `CHECK_EQ( 7, 6 )`; "a score that barely beat alpha goes a ply deeper when the fail-soft best is far below it" `CHECK_EQ( 6, 7 )`; "the deeper margin is measured from the fail-soft best and not from the window" `CHECK_EQ( 6, 7 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D03_deeper_guard_dropped` | 5520483 | "a re-search whose score clears the fail-soft best goes a ply deeper", on the guard's own sub-assertion, `CHECK_EQ( 7, 6 )`; "pruning does not hide a forced mate" `REQUIRE( false )` |
| `D06_deeper_two_plies` | 4466060 | "the re-search depth stays inside its cap, floor and inequality" `CHECK( 3 <= 2 )`; "... clears the fail-soft best ..." `CHECK_EQ( 8, 7 )`; "... barely beat alpha ..." `CHECK_EQ( 8, 7 )`; "the node re-searches at the depth the rule returns" `CHECK( 9 <= 8 )`; "every re-search depth the rule admits is reached in a real search" `CHECK_EQ( 9, 7 )` and `REQUIRE( 0 >  0 )`; "... a table entry a ply deeper" `REQUIRE( 0 >  0 )`; "pruning does not hide a forced mate" |
| `D08_site_ignores_the_rule` | 5469072 | "a re-search that went a ply deeper left a table entry a ply deeper" `REQUIRE( 0 >  0 )`, **and nothing else in the whole fast suite** -- the one case written for it, as at verdict 3 and at leg 1. Its bench is the off tree's exactly, which is what the site ignoring the rule means |
| `D09_deeper_margin_off_alpha` | 4940530 | "the deeper margin is measured from the fail-soft best and not from the window" `CHECK_EQ( 6, 7 )`; "... barely beat alpha ..." `CHECK_EQ( 6, 7 )`; `test_mate_carry` "a mate score carried across searches keeps a line that reaches it" `CHECK( 12 <= 9 )` |
| `D11_cap_one_ply_low` | 5469072 | "... clears the fail-soft best ..." `CHECK_EQ( 6, 7 )`; "... barely beat alpha ..." `CHECK_EQ( 6, 7 )`; "... from the fail-soft best ..." `CHECK_EQ( 6, 7 )`; "every re-search depth ..." `REQUIRE( 0 >  0 )`; "... a table entry a ply deeper" `REQUIRE( 0 >  0 )` |
| `D12_site_rebases_on_alpha` | 4940530 | "the node measures the deeper margin from its own fail-soft best" `CHECK_EQ( -500, -504 )`; `test_mate_carry` `CHECK( 12 <= 9 )` |

Every `// Mutation: D...` evidence block in `tests/test_search.cpp` was rewritten
from these logs and then checked back against them by script
(`.tuning/coord/S098v3_removal_evidence_check.py`, leg 1's, which requires each
`values:` line to be in the named mutant's own log **and** that log to hold a
failure in the case the block names): **9 blocks, 15 `values:` lines, 23 mutant
references, all observed.**

**One thing the first pass caught and it is worth the sentence.** `D03` failed
to *build*: with the shallower path gone, its own guard is the only thing in the
rule that reads `reduction` outside an assert, so dropping the guard orphans the
parameter and the Release build refuses it under `-Werror`. A registry entry
that cannot be applied is a mutant nobody is running, and it looked like a kill
in the summary line until the log was read. The pass was fixed and re-run whole
on the tree that lands rather than patched in place.

### The census that no longer runs, and what was done about it

`adocs/data/S098_v3_research_census.py` measures a **two-path** rule: at every
re-search site it counts the shallower condition, the deeper condition, their
overlap and the three outcomes after the precedence between them, on an off tree
it builds by patching both off values into `src/search_params.hpp`. Four of those
six quantities have no referent on this tree, and the off patch's anchor -- the
literal `LMR_SHALLOWER_MARGIN` line -- does not exist.

It was **not repointed**. Repointing means rewriting the instrumentation, the
summary and the off patch of a script this step is told not to re-run, which
would leave a census nobody has seen claiming to be reproducible from the
repository. What it got instead is `refuse_if_the_rule_moved()`, checked before a
worktree is created, and a docstring paragraph naming which quantities went and
why. **The refusal was observed and not assumed**: `census` exits 1 with a page
naming the removal, both SPRT readings and what a later step's census would have
to drop. Leg 1 recorded this breakage as the one thing it did not fix; this is
the fix it named, and it is a loud refusal rather than a repair.

**The guard keys on the patch and not on the macro's name**, which the Tier-1
check corrected. Its first form asked whether `LMR_SHALLOWER_MARGIN` appeared
anywhere in `src/search_params.hpp` -- a literal-name test that any future
comment recalling the removed constant would satisfy, silently re-arming a
census whose four shallower quantities point at nothing. It now applies
`apply_patch`'s own rule to every `OFF_PATCH` anchor: each `old` must occur
exactly once. The guard therefore fails exactly when the patch would fail and
for the same reason, before a worktree exists, and the refusal prints the
anchor it looked for with the count it found.

**One repair that is not this step's and is named anyway.** `DEV_MANUAL.md`'s
general rule about bench totals began `Quote it with its commit...` as the tail
of the sentence before it, and **S098 verdict 2's landing inserted its
bench-ledger paragraph into that break**, stranding the pronoun with no
antecedent three insertions ago; every landing since, this one included, added
after the same break and carried it forward. It reads `A bench total is quoted
with its commit...` now. Recorded with the landing that broke it rather than
left anonymous, and nothing around it was reflowed.

`adocs/data/S098_v3_research_census.txt` stays as the evidence it is -- taken on
the tree that shipped both paths, and what DEC-214 was answered with. One
reading of it transfers unchanged and is written down here so it is not
re-derived later: with the shallower path gone nothing takes sites from the
deeper path, so the **path's** share is its condition's share, 6.89 % at depth
12 and 5.85 % at depth 10, which is six times DEC-214's one-per-cent threshold.

### `capture_mates` was not re-swept, and that is the DEC-142 call

The tree does not move, so the golden's engine end does not move: its four
depths, its four mutant labels and its four mate distances are leg 1's,
unchanged. The labels keep leg 1's provenance because leg 1's sweep is where
they were measured, and claiming the removal's date for them would name a sweep
nobody ran. Seven rebuilds that cannot change an answer are not evidence, and
what shows they cannot is the three signatures above and not an argument.
`.tuning/coord/S230_v3_leg1/PROVENANCE.txt` now says what the tree those sweeps
were taken on differs from this one by.

### The suite, the signature and the second tier

- Fast suite, Release: **39/39**. Fast suite, tune build: **39/39**.
  `./clang-format.sh --check` clean.
- `Bench: 4646334`, leg 1's exactly. `tools/search_bench.py` at depths 9 and 12
  identical in every count and every best move; the tune build reads 4646334 at
  the shipped defaults and 5469072 at `LmrDeeperMinReduction` 126.
- No test was relaxed, skipped or deleted, and no red was owed: a removal that
  leaves the tree still has nothing to observe red first. What the arms lost is
  recorded case by case above.
- Debug self-play, DEC-141 clause 1, four rounds at 4+0.04 with the Debug
  binaries and `level=trace engine=true`: **8 games in 17 s, 0 `Assertion` in
  both the log and the tee'd stdout, 0 `disconnect`**. The six `assert`s in
  `lmr_research_depth` -- three preconditions now, and three bounds -- never
  fired, and `assert(child_depth >= 1)` is new in this section.
- `tools/mutation_check.py`'s anchor half over every registry file: **84 mutants
  across 10 files, every anchor resolving exactly once** -- 89 before the five
  left. One `expected="equivalent"` declaration remains in the whole tree,
  `M26`, since leg 1's four went with their anchors. Its own pass needs a linked
  worktree with a clean `src/` at `HEAD` and this removal is not committed, so
  the hand-driven pass above is what ran the mutants.
- `tools/gate_extra.sh`, DEC-141 clause 3, on the tree that lands:
  **GATE-EXTRA-DONE 5 stages 1108 s**, all five green -- prose, citations, the
  Debug binaries (362 s), the sanitizer build (685 s) and deep perft (60 s). The
  two document stages were then re-run on the final tree, after this section was
  written into this file: **GATE-EXTRA-DONE 2 stages 0 s**, both green. Nothing
  under `src/` or `tests/` moved between the two runs, which is why only those
  two were owed a second pass.
- `tests/test_search_params.cpp`'s golden loses its `LmrShallowerMargin` row and
  its count moves **51 -> 50**, re-derived the way its own GOLDEN note says --
  there is no script and none is owed, because `src/search_params.hpp` is the
  derivation and a diff of the two is the re-derivation. It was diffed, name,
  default and both bounds, row for row: 50 against 50, identical.
- `MANUAL.md`'s tune-option table loses the `LmrShallowerMargin` row, and the
  two rows that stay say what the rule is now: `LmrDeeperMinReduction` at 126
  switches the **whole** rule off, and a sentence records that a shallower path
  shipped beside this one until it was measured. `DEV_MANUAL.md`'s bench ledger
  gains the removal with the three proofs and no ablation row -- the tree does
  not move, so the by-depth row is still leg 1's -- and its DEC-142 golden list
  moves `golden_defaults` 51 -> 50 and notes on the `capture_mates` row why it
  was not re-derived. `adocs/data/README.md`'s two census rows say the script
  refuses and the output is kept as evidence. `tests/test_uci_surface.cpp`
  needed no edit: it generates the option lines from `search_param_info`, so a
  parameter that left the table leaves its line.

### Proposed for `adocs/specs.md`, for the coordinator to apply

**This supersedes the two passages verdict 3 and leg 1 proposed** and that the
coordinator has not applied; both describe a two-path rule the tree no longer
has, and one passage replaces both.

> **The depth a reduced move's re-search runs at answers that search since S098
> verdict 3, and it answers it only upward (DEC-215)**: `lmr_research_depth`
> returns `child_depth + 1` where the reduced score cleared the node's own
> fail-soft best by `LmrDeeperMargin` with a reduction of at least
> `LmrDeeperMinReduction`, and `child_depth` otherwise; the base is the node's
> `best_so_far` before the move and never alpha, the returned depth is always
> strictly greater than the reduced depth, and the full-window re-search is
> untouched. `LmrDeeperMargin` at its range top is **not** an off value -- the
> fail-soft best sits below alpha at every scout node, so the condition still
> fires there -- and the rule's off value is `LmrDeeperMinReduction` at its own
> range top, where the re-search runs at `child_depth` everywhere and the engine
> is the one before the verdict, bench signature included. **A second path ran
> the re-search one ply shallower until 2026-09-18 and was removed after it was
> measured**: the pair read `Elo -9.97 +/- 7.56` over 4496 games, the whole
> interval below zero, and the pre-registered bisection's leg 1 switched the
> shallower path off and read H1, `Elo 5.75 +/- 4.37` and `nElo 7.44 +/- 5.65`
> over 14510 games against the same reference, so the loss was that path's. It
> left with `LmrShallowerMargin`, with the `reduction >= 2` guard written only
> for it and with the floor at 1 that only it could reach; the removal is
> behaviour-neutral at the shipped configuration and INV-6 proves it rather than
> asserting it -- `bench` 4646334 unchanged, `tools/search_bench.py` identical in
> every count and best move at depths 9 and 12, and the tune build at the rule's
> off value back at the reference's 5469072.

### Files

- `src/search.cpp` -- `lmr_research_depth`: the shallower branch, its guard and
  the floor clamp out; `assert(child_depth >= 1)` in; `alpha` marked
  `[[maybe_unused]]`; the comment rewritten around one path.
- `src/search.hpp` -- `search_lmr_research_depth_probe`'s comment: the precedence
  it no longer holds, and the third precondition.
- `src/search_params.hpp` -- the removed constant's X-macro row and its half of
  the comment out; the count 51 -> 50.
- `src/data_structures.hpp` -- the probe block's comment: `researched` is no
  longer a full-depth repeat, three numbers rather than four, and why
  `research_alpha` is kept.
- `tests/test_search.cpp` -- four cases re-derived and two of them renamed, one
  renamed for its plural, the drive fixture's stop rule and its counter, the
  cap/floor case's sweep and corner, every `// Mutation:` block rewritten from
  the new logs, and `capture_mates`'s paragraph on why it was not re-swept.
- `tests/test_search_params.cpp` -- the golden's `LmrShallowerMargin` row and its
  count.
- `tools/mutants/S098_research_rule.py` -- five mutants out, three anchors
  re-pointed, `D03`'s `(void)`, `D09` and `D08`'s corrected descriptions, and the
  header rewritten.
- `adocs/data/S098_v3_research_census.py` -- the refusal and its docstring.
- `MANUAL.md` -- the tune-option row out, the two that stay reworded.
- `DEV_MANUAL.md` -- the bench ledger's entry, the DEC-142 golden list's count
  and its `capture_mates` note.
- `adocs/data/README.md` -- the two census rows.
- This file -- this section, and one citation in the verdict-3 section.

Evidence kept outside the repository, under `.tuning/coord/`:
`S098v3_removal_mutlogs/` (one ctest log per mutant),
`S098v3_removal/debug_selfplay.{log,out,pgn}`, `run_mutants_v3_removal.py`,
`S098v3_removal_evidence_check.py` and `S098v3_removal_anchor_check.py`.

**For the coordinator.** The `adocs/specs.md` passage above is proposed, not
applied, and it replaces verdict 3's and leg 1's proposals rather than adding to
them. No `done:` stamp is written here. The step's three verdicts are spent: S127
refits `LmrDeeperMargin` and `LmrDeeperMinReduction` beside `LmrBase`,
`LmrDivisor`, the four node-type terms and S109's thresholds after the block.
