id:         S132
goal:       the soft time limit scales with the share of the root's nodes the best move consumed, spending less when the choice is not in doubt
accepts:    an SPRT verdict at an increment control, recorded whatever it is, with the time-forfeit count read from the PGN (the S089 lesson); per-root-move node counting happens at the root loop only, with no per-node cost added to the tree, and a test asserts the per-move counts sum to the iteration's total; the scale formula's constants are in src/search_params.hpp with stated ranges, seeded from the published form and fitted here (DEC-084, S127); a time the GUI named with `go movetime` is still never scaled, and the S089 stability and falling-score scalers are untouched -- this multiplies them, stated in the code where the three meet
touches:    src/chesso.cpp iterative_deepening_search and search_time_node_factor_percent, src/search.cpp negamax at the root, src/data_structures.hpp search_state_t, src/uci.hpp, src/search_params.hpp, tests/, tools/node_share_census.cpp, tools/mutants/, adocs/data/
excludes:   the S089 base allocation and its two scalers; the hard limit
decisions:  DEC-071, DEC-083, DEC-084, DEC-087, DEC-105, DEC-134, DEC-141, DEC-142, DEC-143, DEC-215, DEC-221
closes:
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-21
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
at 8+0.08 and +11.76 ±4.85 at 40+0.4**, both [0, 3]. Lynx #1203, merged 2024-11-27, band **3119 to 3138**
(`adocs/data/S181_lynx_bands.md`; #1206's retune is the same band): +3.59 at
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
`src/chesso.cpp` `compute_search_time_budget` — base = remaining/movestogo, or
5 % of remaining + 50 % of increment at sudden death (`src/chesso.cpp`
`tokenize_input`); hard = 300 % clamped to `remaining - MOVE_OVERHEAD_MS` and
floored (`src/chesso.cpp` `tokenize_input`, `src/chesso.cpp`
`stop_search_after_ms`); soft = 60 % clamped to hard, both in
`src/chesso.cpp` `compute_search_time_budget`.
`search_time_scale_percent()` `src/chesso.cpp` `search_time_scale_percent` is
the existing scaler — `100 - 4*stability (cap 8) + falling grant (≤50)`,
floored at `TM_SCALE_MIN_PERCENT` 30. Applied after each completed iteration in
`src/chesso.cpp` `iterative_deepening_search`: `soft = base * scale / 100`,
clamped to hard in the same place; consulted
between iterations only, `src/chesso.cpp` `iterative_deepening_search`.
`scale_time` is true only on the clock path (`src/chesso.cpp`
`command_position`); `go movetime` sets both limits to the named time and never
scales (`src/chesso.cpp` `command_position`); `go ponder` is ignored
(`src/chesso.cpp` `command_position`). The nine `Tm*` constants:
`src/search_params.hpp` `TM_SOFT_PERCENT` to `src/search_params.hpp`
`TM_SCALE_MIN_PERCENT`.

**What per-root-move attribution needs.** The node counter is one global:
`search_state_t::explored_nodes` (`src/data_structures.hpp` `explored_nodes`),
incremented at negamax entry (`src/search.cpp` `negamax`) and quiescence
(`src/search.cpp` `quiescence`), reset per depth iteration and summed into
`result.total_node_explored`, both in `src/chesso.cpp`
`iterative_deepening_search`. There is no root loop of its own — the root is
`ply == 0` inside the shared move loop of `src/search.cpp` `negamax`, whose
root-only branches are the only place the distinction shows. The
counter arithmetic: at `ply == 0` only, snapshot `explored_nodes` before
`make_move`, take `after - before` past `unmake_move`, add the delta to a
bucket **keyed by the move** — `pick_next_move` reorders in
place, so index i is not stable across iterations. Buckets live in
`search_state_t`, which `src/chesso.cpp` `iterative_deepening_search`
constructs per `go` and which survives iterations and aspiration
re-searches — so they
accumulate across both, the published reading. Residual per `search()` call:
exactly the root's own +1 at the top of `src/search.cpp` `negamax` (NMP is
gated `ply > 0` further down; nothing else at the root counts nodes before
the loop) — the accepts' sum test pins `1 + sum(deltas) ==` the call's counter
growth, so a later root-level feature breaks it loudly.

**Where the multiplier applies:** the `src/chesso.cpp`
`iterative_deepening_search` block. `fraction_pct = 100 *
bucket[search_result.best_move] / max(1, sum(buckets))`, integer like the rest
of the TM code; multiply the node factor onto `scale`; the existing clamp to
hard (`src/chesso.cpp` `iterative_deepening_search`) already bounds the top.
Expose `uci_last_bestmove_node_percent` beside the S089 accessors
(`src/chesso.cpp` `last_aspiration_failures` to `src/chesso.cpp`
`last_time_scale_percent`, `src/uci.hpp` `uci_last_aspiration_failures` to
`src/uci.hpp` `uci_last_time_scale_percent`) for the probe.

### 3. Implementation sketch

Two increments, one verdict.

1. **Counting.** Buckets + accessor + the sum-identity test. The counter is
   only read, so node counts and best moves are byte-identical —
   `tools/search_bench.py` discharges INV-6. Not free by assumption: the
   `if (ply == 0)` guard sits in the move loop every node runs, so DEC-083's
   proof is an interleaved timing (expect noise; the branch predicts to
   not-taken everywhere but the root).
2. 2. **The multiplier.** New constants (par.4), factor into the
   `src/chesso.cpp` `iterative_deepening_search` block, clamp the combined
   product (par.5), tests, then the SPRT.

Tests, all deterministic (fixed depth, no clock, the S089 probe pattern at
`tests/test_engine.cpp` "uci reports the options the GUI needs"): the pure
node-scale function is monotone decreasing in the fraction and matches the Lynx
worked examples at the seed constants; the sum identity above; a
stalemate-adjacent one-legal-move position drives the fraction to 100 % and the
probe's reported scale below 100; the product clamp — soft never exceeds hard
(`test_engine.cpp` "generation never lands on the reserved 0" already asserts
the budget side); `go movetime` unscaled (accepts).

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
  the *combined* scale, and state in the `src/chesso.cpp`
  `iterative_deepening_search` block that three scalers meet there (the accepts
  require it). The top is already the hard clamp (`src/chesso.cpp`
  `iterative_deepening_search`) — never touched, per excludes.
- **The counting is easy to get subtly wrong.** Weiss shipped a follow-up
  "Fix node counts for root moves" (#753, ~neutral at 51k games); Stormphrax
  found AW widenings uncounted (ca2ed2b217); Stockfish moved the effort
  snapshot "back to its original place right before making the move"
  (944bee7117). The sum-identity test is the local answer to all three.
- - **Edge cases chesso actually has:** fastchess sends `wtime/btime winc/binc`
  (sudden death — the `src/chesso.cpp` `tokenize_input` path); `movestogo`
  arrives only at cyclic controls (rating.sh-style), and the factor applies
  downstream of the budget so both paths get it; `go ponder` is ignored
  (`src/chesso.cpp` `command_position`) so there is no ponderhit accounting;
  the first iteration is not abortable (S089 finding), unchanged here; a `go
  movetime` time is never scaled (`src/chesso.cpp` `command_position`,
  accepts).
- - **Bucket the move, not the index** (`src/search.cpp` `negamax` reorders),
  and mind promotions — Stormphrax's "store move node counts directly in the
  root move" was "functional in the case that the best move is a promotion"
  (e3c86966b1): key on the full move encoding, not from/to.

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
  neither, per excludes; the three meet at `chesso.cpp`
  `iterative_deepening_search`.
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

---

## 9. What landed, as coded (2026-09-21)

Written by an Opus 5 subagent from the description in sections 1 to 8 and from
the coordinator's brief, with no other project's source opened (DEC-221).
Citations name symbols and carry no line numbers (DEC-135).

**Increment 1, the counting.** `src/data_structures.hpp` `search_state_t`
gains `root_move_keys`, `root_move_nodes` and `root_move_count` -- two
parallel arrays of `MAX_MOVES` and their length -- with `root_nodes_add`,
`root_nodes_of` and `root_nodes_total` beside the struct, for the reason
`continuation_entry` is one function: the write and the two reads cannot
disagree about what identifies a root move if one place decides it. The key is
the whole move encoding, so the four promotions of one pawn push are four
buckets. `src/search.cpp` `negamax` snapshots `explored_nodes` into
`nodes_before_move` immediately before `make_move` -- zero below the root --
and adds the delta to the move's bucket immediately after `unmake_move`,
**before** the abort check, so an iteration the timer cut still attributes
what it spent. The struct is built per `go`, so the buckets accumulate across
iterations and across aspiration re-searches, which is the published reading.

**Increment 2, the multiplier.** `src/search_params.hpp` gains `TmNodeBasePct`
120 [100, 400], `TmNodeScalePct` 151 [0, 300] and `TmNodeMinDepth` 2 [0, 64]
with the comment block the house uses; the first two are the census's answer
and section 14 records it. `src/chesso.cpp` `search_time_node_factor_percent`
is the pure factor, `(TmNodeBasePct - share) * TmNodeScalePct / 100`, with the
off value in front of it. `src/chesso.cpp` `iterative_deepening_search`
computes the share from the buckets, gates the factor on
`current_depth >= TM_NODE_MIN_DEPTH` and on `conf.scale_time`, multiplies it
onto S089's scale and floors **the product** at `TM_SCALE_MIN_PERCENT`; the
block says in so many words that three scalers meet there. The existing clamp
to the hard limit is untouched and `go movetime` still reaches none of it.

**Five test-only accessors**, beside S089's three and for the same stated
reason -- a pure function can be right and never called:
`uci_last_bestmove_node_percent`, `uci_last_node_factor_percent`,
`uci_last_soft_scale_percent`, `uci_last_soft_limit_ms` and
`uci_last_root_nodes_total` (`src/uci.hpp`). The first is the one section 2
asks for; the last two are what let a case hold the hard clamp and the
accumulation to numbers instead of to a stopwatch and an argument.
`uci_last_time_scale_percent` keeps its old meaning -- S089's two scalers
alone -- so every case written against it still asserts what it asserted.

## 10. Where this departs from sections 2 to 7, and why

- **The off value, DEC-215.** Section 4 declares no off value for the pair and
  the brief requires one. `TmNodeScalePct` 0 is it, and it is a **switch**:
  the published formula alone answers 0 at a scale of 0, which floors every
  soft limit and is the opposite of off, so the factor returns 100 at 0 before
  it computes anything. The cost is an axis that is discontinuous at its own
  floor, which `src/search_params.hpp` states beside the row: a tuning lane
  that wants a continuous axis declares its floor at 1.
- **The seed pair is (scale, base) in that order.** Section 4's "**140 and
  121** at `f_med` = 0.5" reads in the order its two formulas are given:
  `TmNodeScalePct` 140, `TmNodeBasePct` 121. Checked against both constraints
  -- at base 140 with scale 121 the factor at the median is 108 and not 100.
  `adocs/data/S132_node_share_census.py` reproduces 121 and 140 at a median of
  0.5 and derives the pair from the census's own median at the landing.
- **A range can be too small for the census.** The second constraint's slope
  is `7000 / (100 - 100 f_med)`, which passes `TmNodeScalePct`'s declared top
  of 300 once the median share passes about 0.77. The census says so in its
  own output rather than clamping. If it happens, the decision is the range's
  purpose re-derived from chesso's own census -- the slope needed to reach the
  floor at a share of 100 % -- recorded as a (b) bound, and not a seed quietly
  trimmed to fit. **It did not happen**: the median is 0.5350 and the slope is
  151, half the declared top, so both seeds are inside their ranges and no
  range decision is owed.
- **The book in section 6 is older than the harness.** Section 6 names the
  "UHO book"; DEC-189 moved `fastchess.sh` to `books/noob_3moves.epd` after
  S219 measured it, and every verdict in the ledger since is at that book. The
  run takes the harness's own regime and `adocs/data/S132_sprt.sh` says so.
  Section 6 keeps its sentence with this correction beside it, in DEC-215
  clause 3's style.
- **The census needs an instrument, `tools/node_share_census.cpp`.** Section 4
  asks for the census on the release build and the share is not observable
  from outside the process: the buckets live in `search_state_t`, which is
  built and destroyed inside one `go`. Publishing them on the UCI channel
  would be a surface change for a calibration, so the census links the engine
  instead, as `mate_trace` and `feature_audit` do. `adocs/data/S132_node_share_census.py`
  drives it and owns the pick and the arithmetic.

## 11. What holds it

Cases, all deterministic and none of them a golden (DEC-142) -- every number
asserted is read off the same run as the number it is compared with:

- `tests/test_search.cpp` "the root's per-move buckets account for every node"
  -- the sum identity `1 + sum(buckets) == the call's node growth` on three
  positions, one bucket per legal root move (which is the keying: a position
  whose moves differ only in the promotion is in the set and asserted to be),
  every key a legal move, no empty bucket, and the same identity after a
  second call on the same state with a residual of two, which is the
  accumulation an aspiration re-search needs.
- `tests/test_engine.cpp` "the node factor falls as the best move takes more
  of the tree" -- monotone over every share, never negative, a swing of
  exactly `TmNodeScalePct` between the ends, the neutral share reported and
  asserted where the constants keep it inside the range of shares that exist,
  and, in the tune build alone, the off value: at `TmNodeScalePct` 0 the
  factor is 100 at every share, restored and the restoration checked.
- `tests/test_engine.cpp` "the iteration loop scales its soft limit by the
  share it measured" -- four subcases on a root of two bare kings with exactly
  one legal move, chosen because every precondition it needs is then asserted
  rather than described: the share is 100 %, the score cannot move so the fall
  is 0 and the stability is exact, and S089's discount times a factor at a
  share of 100 % lands **under** the floor, which is the multiplicative
  stacking hazard arriving on the first position anyone would try. The four:
  the floor on the product; the depth gate, where the share is measured and
  the factor is neutral below `TmNodeMinDepth` and not at it; the hard clamp,
  with both limits handed to the loop so neither the reading nor the
  precondition depends on the tree; and the whole-search identity
  `nodes == buckets + iterations + widenings`, which is what catches a
  denominator taken over one iteration.
- `tests/test_engine.cpp` "a time the GUI named with movetime is not scaled by
  any of it" -- through the real command path, with the share still measured.

Mutants, `tools/mutants/S132_node_time.py`, `K01` to `K07`, one per rule and
each with its intended killer named in this file's report: the key collided on
squares, the snapshot lost, the factor inverted, the floor left on S089's two
scalers, the depth gate dropped, the buckets cleared between iterations, and
the denominator taken from the iteration's counter. One mutant is declared
absent rather than inferred: moving the snapshot to after `make_move` is
**equivalent**, because `make_move` counts no node.

## 12. Proposed `adocs/specs.md` amendment, for the coordinator

**Applied by the coordinator in the landing commit, 2026-09-21**, with the
seeds' derivation, the INV-6 figures and a `<verdict>` placeholder added to
the passage; the wording below is the proposal as the agent wrote it.

The time management row's scaling sentence, today

> The soft limit is then scaled after each completed iteration: **4 % off per
> consecutive iteration with an unchanged best move, up to 8**, and up to
> **+50 % for a score that has fallen 100 centipawns or more since the
> previous one**, floored at 30 %.

becomes

> The soft limit is then scaled after each completed iteration by three
> factors multiplied together: **4 % off per consecutive iteration with an
> unchanged best move, up to 8**; up to **+50 % for a score that has fallen
> 100 centipawns or more since the previous one**; and, from the iteration at
> `TmNodeMinDepth` on, `(TmNodeBasePct - share) * TmNodeScalePct / 100`, where
> the share is the percentage of the root's own nodes spent under the move
> about to be played, accumulated over every iteration and every aspiration
> re-search of this `go`. **The 30 % floor is on the product**, and
> `TmNodeScalePct` 0 makes the third factor 100 at every share, which is the
> rule's off value. S132.

and "The nine constants are in `src/search_params.hpp`" becomes "The twelve
constants". The verdict sentence is the coordinator's to write from the run.

## 13. What was written before the machine was free (2026-09-21)

Kept as written, with **section 14 below as the answer to every item**:
the machine came free the same day and the whole list was measured.

S097 verdict 1's SPRT held all twelve threads while this was written, and its
pre-registration aborts on a second load, so **nothing here has been built,
run or measured**: no `cmake`, no `ctest`, no `bench`, no `search_bench`, no
`hyperfine`, no census, no mutation run. What was run is what costs the
machine nothing -- `bash -n`, `python3 -m py_compile`, `clang-format-22` over
the changed files, `tools/plan_prose_check.py --citations --touches --params`,
the mutant anchors checked against the tree by a reader, and the census
script's picker exercised offline, which reproduces the 300 positions and the
(121, 140) illustration at a median of 0.5.

Owed at "machine free", in order:

1. Both builds green, both labels, and `./clang-format.sh --check`.
2. Increment 1's INV-6 discharge: `tools/search_bench.py` at depths 9 and 12,
   six counts and six best moves identical to the parent's, and `chesso bench`
   equal to the parent's total.
3. The interleaved timing DEC-083 asks for: `hyperfine` over `chesso bench`,
   at least ten pairs, load stated.
4. The census, then the seeds it gives, then `src/search_params.hpp`,
   `tests/test_search_params.cpp`, `MANUAL.md` and this file updated to them
   -- **the 121 and 140 in the tree today are the f_med = 0.5 illustration and
   not a census result**.
5. The off value on the tree, DEC-215: the tune build at `TmNodeScalePct` 0
   against the parent's bench signature and the probe's own numbers.
6. `tools/mutation_check.py --only` over `K01` to `K07`, each killed by the
   case named in section 11.
7. `tools/gate_extra.sh` and the Debug self-play, DEC-141: this step touches
   the search's move loop.

## 14. What the machine said (2026-09-21, after S097 verdict 1's SPRT)

Measured on the workstation with the machine to itself, one-minute load
average 1.1 to 1.5 on twelve threads throughout, nothing running but the
desktop. Reference is `9fdd9fb`, whose `src/` and `tests/` are `88ec74f`'s to
the byte, so the comparison is against S097 verdict 1's landing. HEAD has
moved since, by documents-only commits that leave that tree untouched, so the
reference sha the SPRT is pinned to at landing may differ from the one named
here while the binary behind it does not.

**Tier 1, both builds green.** `cmake --build build -j8` and
`cmake --build build-tune -j8`, `ctest -L fast` 40 of 40 in each, and
`./clang-format.sh --check` clean -- run twice, once at the illustration
seeds and again at the census's, logs in `.tuning/coord/S132_ctest_*.log`.

**INV-6, the counting half.** `tools/search_bench.py` parent against
candidate: depth 9 is 21479 / 102462 / 33148 with `g5f6` / `e2a6` / `d7c8q`,
depth 12 is 154388 / 459115 / 239314 with `c3d5` / `e2a6` / `d7c8q` --
**identical on both sides, counts and best moves**. `chesso bench` is
**5066204 on both**, with all eight `bestmove` replies identical; the only
line that differs between the two transcripts is the nps figure.

**What the counting costs, DEC-083.** The node count is identical on the two
binaries, so `bench`'s nps is a pure timing. **12 interleaved pairs: paired
delta -0.10 %, sd 1.65 %, se 0.48 %, 95 % [-1.03, +0.83]**; parent mean
3583659 nps, candidate 3579527. `hyperfine -w 1 -r 10` over the same pair,
which does not interleave, reads 1.434 s +/- 0.023 against 1.438 s +/- 0.019.
Inside this machine's 3 % noise floor by both readings.

**The census, P3, and the seeds it gives.** 300 stratified positions at `go
depth 12` on the release build with the counting in it, 15 s of machine time
(`adocs/data/S132_node_share_census.tsv`, `.tuning/coord/S132_census.log`):

| statistic | value |
|---|---|
| minimum share | 4 % |
| first quartile | 35.8 % |
| **median, `f_med`** | **53.5 %** |
| third quartile | 73.0 % |
| maximum | 100 % |
| mean | 54.40 % |
| deciles | 22 31 40 47 54 59 69 78 89 |
| at 100 % | 1 position |
| at 0 % | none |

The two constraints then give `TM_NODE_SCALE_PCT = 7000 / (100 - 100 f_med)`,
which at this median rounds to **151**, and
`TM_NODE_BASE_PCT = 100 + 3000 / TM_NODE_SCALE_PCT`, which rounds to **120**
-- both **inside their declared ranges**, since the slope the floor
constraint asks for passes the range's top only above a median of about 0.77
and this one is nowhere near it, so no range decision is owed. (The formulas
are written with the compiled symbols rather than the option names on
purpose: `tools/plan_prose_check.py --params` reads "`<OptionName>` = 7000"
as a claim about the default, and it is the guard that caught this paragraph
stating one.)
At that pair the factor reads **99 at the median share** (100 to the rounding
of two integer divisions), **30 at a share of 100 %** (the floor, exactly)
and **181 at a share of 0 %**. The seeds are in
`src/search_params.hpp`, `tests/test_search_params.cpp` and `MANUAL.md`.

The instrument's rows carry a second confirmation of the accumulation
identity, over 300 positions rather than one: `total_nodes - root_nodes` is
the number of `search()` calls the loop made, 12 at the minimum -- one per
iteration -- with a median of 20 and a maximum of 40, never below 12 and
never zero.

**The off value, DEC-215.** The rule moves no node at any depth by
construction, and the tune build says so rather than the sentence: `bench` is
5066204 at `TmNodeScalePct` 0, at its shipped 151 and at `TmNodeBasePct` 400
alike. A bench equality is therefore necessary here and carries no
information -- which is why the probe is the proof. In the tune build at
`TmNodeScalePct` 0 the factor reads **100 at every share tested** (0, 37, 50,
99, 100), the setting is restored and the restoration checked, and with the
factor at 100 the loop's combined scale is S089's own scale to the point,
which is the parent's time decision exactly.

**Mutants: 7 of 7 killed, 100 %.** `tools/mutation_check.py` over `K01` to
`K07` in a fixture worktree holding this step's code (baseline green at 40
tests, `bench` 5066204), 1108 s
(`.tuning/coord/S132_mutation.log`, `S132_mutation_results.tsv`):

| mutant | killed by | tests red |
|---|---|---|
| `K01` the key collides on squares | `test_search` "the root's per-move buckets account for every node" | 1 of 40 |
| `K02` the snapshot lost | the same, and `test_engine`'s loop case | 2 of 40 |
| `K03` the factor inverted | `test_engine` "the node factor falls as the best move takes more of the tree" and the loop case | 1 of 40 |
| `K04` the floor off the product | `test_engine` "the iteration loop scales its soft limit by the share it measured" | 1 of 40 |
| `K05` the depth gate dropped | the same | 1 of 40 |
| `K06` buckets cleared per iteration | the same, on the whole-search identity | 1 of 40 |
| `K07` the denominator is the counter | the same, on four of its assertions | 1 of 40 |

**The bench signature is blind to all seven** -- the tool prints `bench same`
on every row -- which is the file's own opening claim measured rather than
argued: a clock bug moves no fixed-depth search by one node, so the direct
cases are the whole of the coverage here.

The fixture worktree was removed after the run (`git worktree remove --force
.ref-builds/mut`); the tool's header has the five commands that rebuild it,
and this tree's code has to be committed into it -- a detached fixture commit
-- because the tool reverts each mutant with `git checkout --`.

## 15. Fast check, landing and second tier (coordinator, 2026-09-21)

**Fast check** by a cold Opus 5 reviewer over the uncommitted diff before it
landed: **no real defect** in the classes it was pointed at -- the root
attribution traced exact (nothing counts a node between the snapshot and
`make_move`, the pruning exit needs `pruning_node` which carries `ply > 0`,
the bucket add precedes the abort return, so the per-call residual is the one
`explored_nodes++` even on an aborted call), the share bounded in [0, 100] by
construction, the floor on `combined_scale` and not on a factor, the hard
clamp untouched, every new case load-bearing (the one-move root's product
reads 26 and 21 under the 30 floor, so the floor assertion binds), no golden
re-typed, nothing copied. Four trivial findings, all comment or document
staleness, fixed by the coordinator before the gate: the `golden_defaults`
GOLDEN count 57 -> 60, `DEV_MANUAL.md`'s live `Tm*` count nine -> twelve,
`tools/node_share_census.cpp`'s header now saying `best_nodes` is re-derived
from `share_pct` and carries no information of its own, and a test comment
quoting the pre-census 140. The reviewer reproduced `bench` 5066204 on both
builds and at the off value, `search_bench` at depth 9 identical to
`.ref-builds/88ec74f` (21479 / 102462 / 33148, `g5f6` / `e2a6` / `d7c8q`),
the census's quartiles, seeds and factor readings from the TSV, and the seven
mutant anchors unique.

**Landed as `474c288`**, `bench` 5066204 (the parent's total; the message carries
`Bench:` and not `No functional change` because the commit changes play at a
clock, which the fixed-depth signature cannot see -- `tools/gate.sh`'s rule).

**Debug self-play, DEC-141 clause 1**, on the landing tree's Debug build: four
rounds at 4+0.04 on `books/noob_3moves.epd`, concurrency 8, `-log level=trace
engine=true` -- **8 games, 0 `Assertion`, 0 `disconnect`**, 187073 trace lines
with 1281 `bestmove` lines (`.tuning/coord/s132_debug_selfplay/`), 15:52 to
15:53.

`tools/gate_extra.sh` launched detached on `474c288` at 15:54
(`.tuning/gate_extra_2026-09-21_s132.log`), watcher armed with four exits and
a 90-minute ceiling; its marker is recorded below before `CAND` is pinned and
the SPRT starts.

**`tools/gate_extra.sh` on `474c288`: `GATE-EXTRA-DONE 5 stages 1145 s`**
(15:54 to 16:13, `.tuning/gate_extra_2026-09-21_s132/`), prose, citations,
debug, sanitize (710 s) and perft (58 s) green. **`CAND` pinned to `474c288`
and `REF` to `778c7b0`**, the commit the landing sits on (its `src/` is
`88ec74f`'s, the tree INV-6 was discharged against; the three commits between
are documents only), in `adocs/data/S132_sprt.sh`. The SPRT is the
coordinator's next machine action after S097 verdict 2's measurements, which
take the idle afternoon first (DEC-155); launch by 19:30 at the latest.

## 16. Verdict, 2026-09-21: H1 (coordinator)

The gainer SPRT of `474c288` (the node-fraction time manager at its census
seeds) against `778c7b0` (the tree without it), `{0, 5}` nElo at 8+0.08 with
Hash 16 on `noob_3moves.epd`, seed 20260921172443, launched 2026-09-21
17:24:43 and `SPRT-RUN-DONE` at 19:13:32, **accepted H1 after 3822 games**:

```
SPRT | cand 474c288 vs ref 778c7b0, 8+0.08, Hash=16, noob_3moves.epd, {0, 5} nElo
Elo | 16.28 +/- 8.47, nElo 21.21 +/- 11.01
LLR | 2.95 (-2.94, 2.94) -> H1
Games | N: 3822 W: 1239 L: 1060 D: 1523, Ptnml [149, 406, 664, 501, 191]
Wall | 1 h 48 m, 2115.5 games/h, forfeits 0
Log | adocs/data/S132_sprt.log
```

LOS 99.99 %, draw ratio 34.75 %, pairs ratio 1.25. **0 time forfeits on
either side** over the PGN's 3825 games (2603 adjudications, 1222 natural
ends) -- the abort rule this clock change was taken under never came near.
`Incomplete mating PV` 2 candidate against 11 reference, an observation and
not a diagnosis (CHESS). `adocs/data/S105_pairs.py`: 1911 complete pairs, pair
score mean 1.0154, variance 0.3047, sd 0.5520, buckets 8.7 / 23.3 / 33.9 /
24.1 / 9.9 %, white winning both of 167 pairs (8.7 %), 117.1 plies and 19.9 s
a game. Evidence: `adocs/data/S132_sprt.log`, `adocs/data/S132_sprt_pairs.txt`;
the run directory `.tuning/sprt_s132_20260921_172443`.

### The reading

The pre-registered H1 reading binds. **The multiplier stays at the census
seeds it was measured at** -- `TmNodeScalePct` 151, `TmNodeBasePct` 120,
`TmNodeMinDepth` at `AspirationMinDepth`'s value -- and the claim is "at least
5 nElo", never the stopping estimate (DEC-063): the nElo interval
[10.2, 32.2] misses the bounds pair on the high side, which is DEC-223's
fast class, and 3822 games is the shortest gainer since S098 verdict 2. The
three constants are refitted by S127's lane and not here, **with S085's
caveat doubled**: the time-management family is the one S085 recommends
excluding from a tune at a control the verification does not share (an
SPSA'd time manager measured +23.8 at 20+0.2 and -22.9 at 10+0.1), so a fit
of these three wants the playing control or a second-control verification,
and S127's file says so before it runs.

**The one question this step asks the owner rather than deciding** (section
6 and the pre-registration): before the constants are called shipped, one
confirmation at a second control -- a 40+0.4-class run, or folded into S152's
rated run at the list's own control (DEC-108) -- for the S085 reason and
because every published node-TM patch this step read was verified at two to
four controls. It is stated in `status.md`'s Parked list as a question with
the coordinator's recommendation, and no night is spent on it on the
coordinator's authority.
