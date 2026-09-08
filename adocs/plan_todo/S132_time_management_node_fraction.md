id:         S132
goal:       the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
accepts:    an SPRT verdict at an increment control, recorded whatever it is, with the time-forfeit count read from the PGN (the S089 lesson); per-root-move node counting happens at the root loop only, with no per-node cost added to the tree, and a test asserts the per-move counts sum to the iteration's total; the scale formula's constants are in src/search_params.hpp with stated ranges, seeded from the published form and fitted here (DEC-084, S127); a time the GUI named with `go movetime` is still never scaled, and the S089 stability and falling-score scalers are untouched -- this multiplies them, stated in the code where the three meet
touches:    src/chesso.cpp iterative_deepening_search and the root move loop, src/search_params.hpp, tests/
excludes:   the S089 base allocation and its two scalers; the hard limit
decisions:  DEC-071, DEC-084, DEC-087, DEC-105, DEC-134
closes:
blocks:
paused_by:
done:

## Created by the second review, DEC-087

The third soft-limit scaler the surveyed set carries, and the one S089 did not
build: when one root move has eaten most of the iteration's nodes, the search
already knows the answer, and when the nodes are spread the position is
genuinely unclear and worth more clock. Ethereal measured it at **+9.9 / +9.7**
(+20.9 at a cyclic control, 60f4d5c5, crediting the idea to Koivisto), Lynx at
+3.6, and Stormphrax adopted it. Zero search risk: it touches when iterations
start, never what they search. The published shape is a linear scale on
`1 - bestmove_nodes/total_nodes` clamped to a band; the constants ship from
our own sweep and S127, per DEC-084.

## Technical details (SOTA research, 2026-08-19)

Every engine record below was read as commit-message or PR-body prose over the
GitHub API; no source file of any engine was opened (DEC-016, DEC-084).

### 1. State of the art

**The form, in published prose.** Lynx PR #1203 states it exactly: "scale soft
limit time bound by the proportion of nodes spent searching the best move at
root level vs the total nodes searched", `scale = nodeTmScale * (nodeTmBase -
bestMoveFraction)`, with worked examples at base 2, scale 1: fraction 0.50 →
1.5, 0.25 → 1.75, 1.00 → 1.0. More nodes on the best move → more certainty →
less time. The fraction's denominator is the whole search's nodes so far, not
one iteration's ("total nodes spent", Ethereal; "vs the total nodes searched",
Lynx), and Stormphrax's later fix "count the root node for every root call --
previously AW widenings were not counted" (ca2ed2b217) shows aspiration
re-searches are inside the accumulation.

**The spread, traced.** Berserk PR #324 "Utilize node count statistics with
move stability", 2021-12-29: +5.13 ±3.69 at 8+0.08, +6.68 ±4.34 at 40+0.4 —
the earliest record found. Koivisto is where Ethereal and Stockfish both say
the idea came from; its own TM PRs (#173, #180, +4 to +6 at 8+0.08) carry
records but not the mechanism in prose. Ethereal 60f4d5c5, 2022-06-03 (a TM
rewrite whose "new one" is this factor): **+9.85 ±6.19 at 10+0.1, +9.66 ±6.02
at 60+0.6, +20.90 ±9.67 at 40/10 cyclic, +8.33 ±5.52 at 40/40**, all [0, 5] —
the plan's +9.9/+9.7. Stormphrax 4f14671c8d, 2023-06-25: "adjust soft timeout
based on the percentage of nodes that were spent on the best move". Stockfish
bf2c7306, 2024-02: a *stop* variant — "stops the search when almost all nodes
are searched on a single move... Koivisto scales the optimal time by the nodes
effort; we just scale down the totalTime" — STC <0,2> over 88672 games and LTC
<0.5,2.5> over 170856, both passed. Weiss PR #752, 2024-12: "Adjust time spent
based on how much of the search effort goes into the top move", **+11.37 ±4.93
at 8+0.08 and +11.76 ±4.85 at 40+0.4**, both [0, 3]. Lynx #1203: +3.59 at
8+0.08, +9.40 at 16+0.16, +10.18 at 40+0.4; #1206 retuned (base 2.4, scale
1.65) for +13.33 more at 40+0.4. CPW's Time Management page names "the ratio
of the size of the subtree under the best move versus the size of the whole
search tree" as a consideration and gives no method — the engines' records are
the literature here.

**Composition with stability/score scaling is multiplicative and published.**
Lynx landed node TM first (#1203), then best-move stability (#1211) and score
stability (#1223) as separate scalers on the same soft bound; Berserk #324
combined node counts *with* move stability in one patch; Ethereal's rewrite
carries the node factor beside its older factors. Chesso has the other two
already (S089), so this multiplies them — the order the scalers landed in
differs across engines and nothing in the record says it matters.

### 2. Shape for chesso

**The base this scales** (all S089): `compute_search_time_budget()`
src/chesso.cpp:454-507 — base = remaining/movestogo, or 5 % of remaining + 50 %
of increment at sudden death (src/chesso.cpp:471-479); hard = 300 % clamped to
`remaining - MOVE_OVERHEAD_MS` and floored (src/chesso.cpp:481,
src/chesso.cpp:488-499); soft = 60 % clamped to hard (src/chesso.cpp:482,
src/chesso.cpp:504). `search_time_scale_percent()` src/chesso.cpp:510-534 is
the existing scaler — `100 - 4*stability (cap 8) + falling grant (≤50)`,
floored at `TM_SCALE_MIN_PERCENT` 30. Applied after each completed iteration at
src/chesso.cpp:864-879: `soft = base * scale / 100`, clamped to hard at
src/chesso.cpp:873-875; consulted between iterations only,
src/chesso.cpp:920-927. `scale_time` is true only on the clock path
(src/chesso.cpp:1402); `go movetime` sets both limits to the named time and
never scales (src/chesso.cpp:1380-1384); `go ponder` is ignored
(src/chesso.cpp:1346-1350). The nine `Tm*` constants:
src/search_params.hpp:262-312.

**What per-root-move attribution needs.** The node counter is one global:
`search_state_t::explored_nodes` (src/data_structures.hpp:453), incremented at
negamax entry (src/search.cpp:608) and quiescence (src/search.cpp:302), reset
per depth iteration (chesso.cpp:726), summed into `result.total_node_explored`
(src/chesso.cpp:804). There is no root loop of its own — the root is `ply == 0`
inside negamax's shared move loop (src/search.cpp:896-1063; root-only branches
src/search.cpp:1041, src/search.cpp:1074). The counter arithmetic: at `ply ==
0` only, snapshot `explored_nodes` before `make_move` (src/search.cpp:917),
take `after - before` past `unmake_move` (src/search.cpp:994), add the delta to
a bucket **keyed by the move** — `pick_next_move` (src/search.cpp:915) reorders
in place, so index i is not stable across iterations. Buckets live in
`search_state_t`, which is constructed per `go` (chesso.cpp:674) and survives
iterations and aspiration re-searches (src/chesso.cpp:774-795) — so they
accumulate across both, the published reading. Residual per `search()` call:
exactly the root's own +1 at src/search.cpp:608 (NMP is gated `ply > 0`,
src/search.cpp:822; nothing else at the root counts nodes before the loop) —
the accepts' sum test pins `1 + sum(deltas) ==` the call's counter growth, so a
later root-level feature breaks it loudly.

**Where the multiplier applies:** the src/chesso.cpp:864-879 block.
`fraction_pct = 100 * bucket[search_result.best_move] / max(1, sum(buckets))`,
integer like the rest of the TM code; multiply the node factor onto `scale`;
the existing clamp to hard (src/chesso.cpp:873-875) already bounds the top.
Expose `uci_last_bestmove_node_percent` beside the S089 accessors
(chesso.cpp:71-93, uci.hpp:144-153) for the probe.

### 3. Implementation sketch

Two increments, one verdict.

1. **Counting.** Buckets + accessor + the sum-identity test. The counter is
   only read, so node counts and best moves are byte-identical —
   `tools/search_bench.py` discharges INV-6. Not free by assumption: the
   `if (ply == 0)` guard sits in the move loop every node runs, so DEC-083's
   proof is an interleaved timing (expect noise; the branch predicts to
   not-taken everywhere but the root).
2. 2. **The multiplier.** New constants (par.4), factor into the
   src/chesso.cpp:864 block, clamp the combined product (par.5), tests, then
   the SPRT.

Tests, all deterministic (fixed depth, no clock, the S089 probe pattern at
tests/test_engine.cpp:866-981): the pure node-scale function is monotone
decreasing in the fraction and matches the Lynx worked examples at the seed
constants; the sum identity above; a stalemate-adjacent one-legal-move
position drives the fraction to 100 % and the probe's reported scale below
100; the product clamp — soft never exceeds hard (test_engine.cpp:449-533
already asserts the budget side); `go movetime` unscaled (accepts).

### 4. Constants and seeds

All **seed — must be fitted/SPSA'd here** (S127), X-macro rows of
`CHESSO_SEARCH_PARAMS` in `src/search_params.hpp`. Under DEC-105 each is one
of three forms and says which: **(a)** a value from a publication about the
technique, with its URL; **(b)** a derivation over chesso's own data or
scale; **(c)** the range midpoint or off value, stated as such. **No (a)
exists for this technique**: the wiki's Time Management page names "the ratio
of the size of the subtree under the best move versus the size of the whole
search tree" as a consideration and states no method and no number
(https://www.chessprogramming.org/Time_Management, fetched 2026-09-05). So
the pair is derived over chesso's own tree, and the engines' shipped pairs
are records only — section 1 and, as anti-seeds, section 5. Ranges: base
[100, 400], scale [0, 300], gate [0, 64].

- `TM_NODE_BASE_PCT` and `TM_NODE_SCALE_PCT` — **(b)**, a census over
  chesso's own tree, **P3**, run by this step once its counting half has
  landed (behaviour-neutral, and first in section 3's sketch). Release build.
  Over the 300-position stratified pick
  (`adocs/data/S021_aspiration_sweep.py`) at `go depth 12`, record the best
  move's share `f` of root-child nodes. In the X-macro's percent units
  section 3's factor is `factor_pct = (TM_NODE_BASE_PCT - 100 f) *
  TM_NODE_SCALE_PCT / 100`. **Two constraints, both chesso's own.**
  `factor_pct` = 100 at the census median `f_med`, so the seed spends today's
  allocation in expectation and the SPRT measures redistribution rather than
  a longer clock; and `factor_pct` = `TM_SCALE_MIN_PERCENT` (30 today) at
  `f` = 1, chesso's own floor for the scaled soft limit. Solving:
  `TM_NODE_SCALE_PCT = 7000 / (100 - 100 f_med)` and `TM_NODE_BASE_PCT = 100
  + 3000 / TM_NODE_SCALE_PCT`, rounded to integers — **140 and 121** at
  `f_med` = 0.5, which is the illustration and not the seed; the census
  supplies `f_med`. Record `f_med` and the quartiles in this step's stamp.
  Fallback **(c)** if the census cannot run before the multiplier is written,
  said so in the stamp: midpoints **250** of [100, 400] and **150** of
  [0, 300].
- `TM_NODE_MIN_DEPTH` — **(b)**, equal to `ASPIRATION_MIN_DEPTH` **as
  compiled at this step's HEAD** (2 today, not the 5 the 2026-08-19 pass
  wrote from its pre-S085 value). It is chesso's own guard against a
  depth-1..4 fraction being noise, and 0 — no gate — is a valid swept
  outcome. **(c) is refused here and the refusal is the point**: the midpoint
  32 of [0, 64] is above every depth this engine reaches, so it would seed
  the feature switched off and the sweep would start from a rule that never
  fires.

seeds re-derived 2026-09-04 under DEC-105 (DEC-134)

### 5. Pitfalls

- **Anti-seeds — records, not seeds.** DEC-019 lets a record say which
  direction is worth trying; DEC-105 forbids any of these numbers starting
  the fit, which is why section 4 names no engine. The only pair traced as
  prose is Lynx #1203's (base 2.0, scale 1.0), and its retune to (240, 165)
  in #1206 is the useful part of the record: the pair co-moves with the base
  allocation, which is exactly why a number fitted against another engine's
  allocation cannot seed chesso's. P3's two constraints are the same
  observation written as arithmetic over chesso's own clock.
- **TM constants are the most TC-sensitive family.** S085 traced an SPSA'd
  time manager at **+23.8 at 20+0.2 that measured -22.9 at 10+0.1**. Every
  node-TM record above was verified at two to four controls (Ethereal even at
  cyclic, +20.9). Fitting at 8+0.08 and playing CCRL at ~2'+1" is exactly the
  hazard's shape — see par.6.
- - **Multiplicative stacking underspends.** Stability floor 30 % times a node
  factor at fraction → 1 craters the product; apply `TM_SCALE_MIN_PERCENT` to
  the *combined* scale, and state in the src/chesso.cpp:864 block that three
  scalers meet there (the accepts require it). The top is already the hard
  clamp (src/chesso.cpp:873-875) — never touched, per excludes.
- **The counting is easy to get subtly wrong.** Weiss shipped a follow-up
  "Fix node counts for root moves" (#753, ~neutral at 51k games); Stormphrax
  found AW widenings uncounted (ca2ed2b217); Stockfish moved the effort
  snapshot "back to its original place right before making the move"
  (944bee7117). The sum-identity test is the local answer to all three.
- - **Edge cases chesso actually has:** fastchess sends `wtime/btime winc/binc`
  (sudden death — the src/chesso.cpp:471-479 path); `movestogo` arrives only at
  cyclic controls (rating.sh-style), and the factor applies downstream of the
  budget so both paths get it; `go ponder` is ignored
  (src/chesso.cpp:1346-1350) so there is no ponderhit accounting; the first
  iteration is not abortable (S089 finding), unchanged here; a `go movetime`
  time is never scaled (src/chesso.cpp:1380-1384, accepts).
- - **Bucket the move, not the index** (src/search.cpp:915 reorders), and mind
  promotions — Stormphrax's "store move node counts directly in the root move"
  was "functional in the case that the best move is a promotion" (e3c86966b1):
  key on the full move encoding, not from/to.

### 6. Measurement

Increment 1 owes a timing, not a match (DEC-083): interleaved, identical node
counts stated. Increment 2 owes **one SPRT at the S105 regime** — 8+0.08,
Hash=16, UHO book, `elo0=0 elo1=5` — verdict recorded whatever it is, with the
time-forfeit count read from the run-filtered PGN (accepts; the S089 lesson).
**Recommendation, owner decides:** before calling the constants shipped, one
confirmation at a second control (40+0.4-class, or fold into the rated run of
S152, which absorbed S128's question by DEC-108, at the list's own control) — the S085 +23.8/-22.9 record and the multi-TC
verification every published node-TM patch ran are the reasons. Published
figures decided what to try here, never what to conclude (DEC-019).

### 7. Interactions

- **S089 (done)** is the base: this multiplies its two scalers and changes
  neither, per excludes; the three meet at chesso.cpp:864-879.
- **S115 (order 28, before this at 30):** re-sweeping the widening schedule
  changes how often the root is re-searched, which moves both bucket
  composition and denominator — land S115 first (plan order already does),
  and re-run the sum-identity and fraction-distribution measurements after.
- **S127:** the (base, scale, gate) triple joins the SPSA set, with S085's
  caveat doubled — the TM family is the one S085 recommends excluding from a
  tune at a TC the verification does not share; a fit of these three wants
  the playing control or a second-TC verification.
- **S152** (which absorbed S128 by DEC-108)**:** the rated run near the list's
  control is where a TM fit overfitted to 8+0.08 would show as
  underperformance against the anchor.

### 8. References

- - https://github.com/AndyGrant/Ethereal/commit/60f4d5c5 — commit message via
  API: the factor, Koivisto credit, four SPRT records
  (+9.85/+9.66/+20.90/+8.33).
- https://github.com/lynx-chess/Lynx/pull/1203 — the form in prose with worked
  examples (base 2, scale 1); three-TC record. #1206 — retune (2.4, 1.65),
  +13.33 at 40+0.4. #1211 — stability added on top afterwards.
- https://github.com/TerjeKir/weiss/pull/752 — +11.37 at 8+0.08 / +11.76 at
  40+0.4; #753 — the root-node-count fix shipped after it, ~neutral.
- https://github.com/jhonnold/berserk/pull/324 — node counts with move
  stability, 2021-12-29, +5.13/+6.68.
- https://github.com/official-stockfish/Stockfish/commit/bf2c7306 — the stop
  variant, Koivisto mechanism described in prose, STC/LTC records; fix commit
  944bee7117 (snapshot placement, "only functional with active TM").
- Stormphrax commit messages via search API: 4f14671c8d (adoption),
  ca2ed2b217 (AW widenings counted), e3c86966b1 (per-root-move storage),
  b02cce57a3 (counter-increment overhead, SMP context).
- https://github.com/Luecx/Koivisto — PRs #173/#180 read for records; the
  mechanism prose lives in the engines that credit it, not there.
- https://www.chessprogramming.org/Time_Management — names the subtree ratio
  as a consideration, no method; stability and score heuristics documented.
