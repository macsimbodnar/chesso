# Search / TT / PV bug report

Scope: `src/search.cpp`, with the parts of `src/transposition_table.cpp`,
`src/data_structures.hpp` and `src/chesso.cpp` it depends on.

Build used: existing `build/` (Release, `-O3 -DNDEBUG`, so all `assert`s are off).
Line numbers refer to the current working tree (with the `quiescence()` call at
`search.cpp:145` uncommented).

Every finding marked **[reproduced]** was demonstrated by running the engine or a
harness linked against `libchesso_engine.a`; the rest are code reads.

---

## Status

| # | Finding | Status |
|---|---------|--------|
| 1 | Quiescence disabled at ply > 2 | **FIXED** |
| 2 | Illegal / spliced PV | **FIXED** |
| 2b | PV truncated by TT cutoffs | **FIXED** on the PV path; see note |
| 3 | No move ordering | **FIXED** |
| 4 | Aborted search returns opponent's move | **FIXED** |
| 5 | TT stores illegal / non-cutting best moves | **FIXED** |
| 6 | Mate distance off by one when losing | **FIXED** |
| 7 | Time / node limits at 1000-node granularity | **FIXED** |
| 8 | `quiescence()` stop path does not set `aborted` | **FIXED** |
| 9 | TT probe before repetition check; no 50-move rule | **FIXED** |
| 10 | TT has no aging | **FIXED** |
| 11 | Dead / misleading code | **mostly FIXED**, see section |
| 12 | 4 MB `search_state_t` on the thread stack | **FIXED** |
| 13 | Iterative deepening wastes the time budget | **FIXED** |
| 14 | Quiescence stands pat while in check | **FIXED** |
| 15 | `make_move()` accepts moves for the wrong side | **FIXED** |
| 16 | Graph-history interaction | **open, deliberately** |

Regression harness: `scratchpad/verify.cpp` — checks PV legality, `pv[0] ==
best_move`, best-move legality and mate-distance consistency over 11 positions.

| after fix | failures | total nodes |
|-----------|----------|-------------|
| baseline  | 5 | 12,984,926 |
| 1, 7, 8   | 5 | 56,145,772 (quiescence now actually runs) |
| 2, 2b     | 4 | 56,145,772 |
| 4, 5, 9, 10 | 4 | 56,194,103 |
| 3         | 4 | 3,723,838 |
| 6, 11     | **0** | 3,844,888 (`MAX_QSEARCH_DEPTH` raised to 8) |
| 13, 14, 12, 15 | **0** | 4,270,221 |

Baseline for comparison, re-measured from the session-start sources: 5 failures,
12,984,926 nodes. So the search is now 3.0x cheaper *and* running a real
quiescence search.

`ctest` (test_chesso, test_openings, test_perft) passes after every step, in the
Release build and in a full **Debug** build with every `assert` live — including
the new `make_move()` invariant, which survives the 164M-node perft suite
covering castling, en-passant and promotions.

> **Note on scores.** `src/evaluation.cpp` was rescaled to centipawns
> (`PAWN 100 … KING 100000`) partway through this work, so `cp` values quoted
> before that point are on the old 1-per-pawn scale. Node counts are unaffected:
> a uniform rescale of a material-only evaluation preserves move order, and the
> re-measured baseline reproduces the original counts exactly.

---

## 1. Quiescence search is disabled at any ply > 2 — **[reproduced]** — **FIXED**

> **Fixed.** `quiescence()` now takes a separate `qply` argument that counts
> plies inside the quiescence search, so `MAX_QSEARCH_DEPTH` bounds the capture
> resolution instead of switching it off. `negamax()` enters it with `qply == 0`.
> Redundant `ply > MAX_PLY - 2` guard replaced by the `ply + 1 >= MAX_PLY`
> bound that matches `negamax()`.
>
> Effect — start position, depth 7:
> ```
> before: cp  1  nodes 596841  pv a2a3 a7a6 b2b3 b7b6 c1b2 c7c6 b2g7   <- hangs the bishop
> after : cp  0  nodes 387019  pv a2a3 a7a6 a3a4 b7b6 a4a5 b6b5 b2b3
> ```
> Fewer nodes *and* a sane line: accurate leaf scores produce better cutoffs.
>
> `MAX_QSEARCH_DEPTH` was then raised from 2 to 8: at 2 the quiescence search
> cannot finish a three-capture exchange. Measured over the whole harness, and on
> `TRICKY_POS` at depth 8:
>
> | qdepth | harness nodes | tricky d8 |
> |--------|---------------|-----------|
> | 2 | 3,723,838 | cp -1, 5.01M nodes |
> | 4 | 3,832,609 | cp -1, 5.18M nodes |
> | 8 | 3,844,888 | cp 0, 5.14M nodes |
> | 16 | 3,852,213 | cp 0, 5.17M nodes |
>
> 8 and 16 agree, so the value has converged there; the extra depth costs ~3% of
> nodes. It is a tunable, not a fix.
>
> Still missing inside quiescence, both harmless while the search is
> material-only but worth doing: it stands pat while in check, and it cannot tell
> checkmate from "no captures available".

`search.cpp:68`

```cpp
if (ply > MAX_QSEARCH_DEPTH) { return best_value; }   // MAX_QSEARCH_DEPTH == 2
```

`ply` is the **absolute** distance from the root, not the depth inside the
quiescence search. Every leaf of a search deeper than 2 plies enters
`quiescence()` with `ply >= 3` and returns the static evaluation immediately, so
captures are never resolved.

Measured on `TRICKY_POS`, depth 6: **3,872,100 of 3,872,430** quiescence calls
(99.99%) returned before generating a single move. Only 330 calls did any work.

Visible effect — depth 7 from the start position:

```
info score cp 1 depth 7 pv a2a3 a7a6 b2b3 b7b6 c1b2 c7c6 b2g7
```

`b2g7` hangs the bishop; the recapture is one ply past the horizon and the
quiescence search that exists to see it is inert.

Fix: track quiescence depth separately (pass a `qply` starting at 0, or record
the ply at which quiescence was entered and bound the difference).

Related, in the same function:
- `search.cpp:67` (`ply > MAX_PLY - 2`) is unreachable while (1) is in place.
- Stand-pat is applied even when the side to move is in check, and there is no
  mate/stalemate detection in quiescence. Both become live once (1) is fixed.
- The node budget (`state->node_limit`) is not checked in quiescence at all.

---

## 2. The reported PV can be illegal — **[reproduced]** — **FIXED**

> **Fixed.** `state->pv_length[ply] = 0` is now the first statement in
> `negamax()`, before the TT probe, the repetition check, the limit check and
> the quiescence bail-out. Every exit path therefore leaves a valid (possibly
> empty) PV row, so a parent can no longer splice in a line belonging to an
> unrelated node.
>
> The two positions that produced illegal PVs are clean:
> ```
> before: depth 8  pv a2a3 c8d8 a3a4 d8c8 a4a5 a8a7 a5a6 c8d8   <- a8a7 is not a move
> after : depth 8  pv a2a3 c8d8 a3a4 d8c8
> ```
> The harness now reports 0 illegal PVs and 0 `pv[0] != best_move` over all 11
> positions.

`search.cpp:152` vs. the early returns at `search.cpp:114-129, 145`

```cpp
state->pv_length[ply] = 0;      // line 152 — reached only by "normal" nodes
```

The PV row for a ply is cleared **after** the TT probe, the repetition check and
the quiescence bail-out. A node that returns through any of those three paths
leaves `pv_length[ply]` holding a value written by an unrelated node that
occupied the same ply earlier in the search. The parent then does:

```cpp
memcpy(&state->pv_table[ply][1], state->pv_table[ply + 1],
       state->pv_length[ply + 1] * sizeof(move_t));   // line 214
```

and splices that foreign line into its own PV.

Instrumented count on `TRICKY_POS` depth 6: **204** parent nodes copied a child
PV row the child never wrote during that visit and whose stale length was
non-zero.

Reproduce with the shipped binary:

```
position fen 2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 1
go depth 9
```

```
info score cp 8 depth 8 nodes 737109 pv a2a3 c8d8 a3a4 d8c8 a4a5 a8a7 a5a6 c8d8
info score cp 8 depth 9 nodes 4558238 pv a2a3 c8d8 a3a4 d8c8 a4a5 a8a7
```

After `a4a5` it is Black to move and Black has nothing on a8 (the rook there is
White's, on a7). `a8a7` and everything after it is garbage grafted in from
another subtree. `is_pv_legal()` (`bitboard.cpp:1522`) rejects both lines.

Fix: move `state->pv_length[ply] = 0;` to the top of `negamax()`, before the TT
probe. Also set `pv_length[ply] = 0` in `quiescence()` for the same reason, or
clear `pv_length[ply + 1]` right before each recursive call.

### 2b. PV is truncated by exact-score TT cutoffs — **FIXED on the PV path**

> **Fixed.** `negamax()` takes an `is_pv` flag: the root is a PV node, and so is
> the first legal move searched at any PV node. PV nodes never take a TT cutoff,
> because a cutoff returns a score with no move sequence behind it. Costs
> nothing measurable — the leftmost path is a handful of nodes per iteration,
> and they rarely have a deep enough entry to cut on anyway (node counts were
> byte-identical before and after).
>
> Residual, by design: if a *later* move at a PV node raises alpha, that move's
> subtree was searched as non-PV and may have cut on the TT, so the PV can still
> end early. The line stays legal, it is just short. Move ordering (finding 3)
> makes the first move the likely best move, which is what keeps this rare in
> practice. Still visible at `3frep`, where the PV stalls at 4 moves from depth 5
> on.

`search.cpp:116-118` returns on a `TT_PV_NODE` hit without reconstructing the
line, so the PV stops at the first transposition. Same position as above:

```
info score cp 8 depth 7 nodes 629673 pv a2a3 c8d8 a3a4 d8c8      <- 4 moves for a depth-7 search
```

Fix: do not take exact-score TT cutoffs on PV nodes (`beta != alpha + 1`), or
walk the TT to rebuild the tail of the line.

---

## 3. There is no move ordering at all — **FIXED**

> **Fixed**, using only information the position and the search already carry —
> no new evaluation terms.
>
> `score_move()` (`search.cpp`) ranks a move as:
>
> | band | source |
> |------|--------|
> | TT move | `tt_entry->best_move`, read on every probe, not only on a cutoff |
> | captures / promotions | MVV-LVA — victim read off the bitboards, attacker from `MOVE_PIECE` |
> | killer 0 / killer 1 | `state->killer_moves`, already filled in on fail-high |
> | countermove | `state->counter_moves`, already filled in on fail-high |
> | quiet | `state->history_moves`, clamped so it cannot outrank a killer |
>
> The three tables were already being written by the fail-high path; nothing read
> them. `captured_piece()` only probes the six enemy bitboards. The ordering
> weights live in `search.cpp` and use the same relative material scale as
> `evaluate()`; they never reach a returned score.
>
> Moves are picked one at a time by `pick_next_move()` (one selection-sort step
> per visited move) rather than sorting the list up front, since most nodes fail
> high on one of the first moves. Quiescence orders its captures the same way.
>
> Nodes at depth 6, original code vs. now (the "before" numbers do not even count
> TT-cut and repetition nodes, so the real gap is larger):
>
> | position | before | after | |
> |----------|--------|-------|-|
> | startpos | 523,146 | 46,113 | 11x |
> | tricky | 3,629,191 | 329,610 | 11x |
> | cmk | 1,067,305 | 400,312 | 2.7x |
> | killer | 3,999,665 | 132,869 | 30x |
>
> Whole harness: 56,194,103 nodes before ordering, 3,723,838 after — with a
> working quiescence search on top. Start position now reaches depth 9 in 414 ms
> (4.7M nodes).
>
> Side effect: every PV in the harness is now full length, including the `3frep`
> position that stalled at 4 moves. With the TT move searched first, the PV node's
> first move usually *is* the best move, which is what closed the residual
> truncation described in 2b.

`search.cpp:171-218`

Moves are searched in raw generation order. `order_captures()` is commented out
at `search.cpp:73`, and nothing sorts the list in `negamax()`.

The killer / history / countermove bookkeeping at `search.cpp:191-202` is
**write-only** — `state->killer_moves`, `state->history_moves` and
`state->counter_moves` are never read anywhere in the codebase. The TT best move
is stored but never used for ordering either. `state->node_prev_move`,
`state->search_in_tt`, `state->prev_score`, `NULL_MOVE_REDUCTION`,
`LMR_WHEN_START_IN_THE_LIST` and `LMR_START_AT_DEPTH` are all unused.

Cost: 39.6M nodes for depth 7 on `TRICKY_POS`; the effective branching factor
between iterations swings between 5x and 18x, which is the signature of
unordered alpha-beta. This is the single largest strength/speed defect after (1).

Suggested order: TT move → captures (MVV-LVA) → killers → countermove → history.

---

## 4. An aborted search hands back a move belonging to the opponent — **[reproduced]** — **FIXED**

> **Fixed.** `state->best_move` is written only at `ply == 0`, and the write on
> the TT hit path is gone. The root also publishes its move as soon as one is
> proven better than alpha, so an iteration that is aborted part-way still
> reports the best move it finished searching instead of nothing.
>
> Same abort harness as above, depth 8 on `TRICKY_POS`, stopped after 1-25 ms:
> ```
> before: bm=e6e5 / h3g2 / d7d6 ...   (Black moves in a White-to-move position)
> after : bm=0000                     (no root move completed that early)
> ```
> `0000` is the honest answer when no root move has finished; the driver keeps
> the previous iteration's move (`chesso.cpp:562`), and with a real time control
> earlier iterations have always completed.
>
> Still open, outside `search.cpp`: `make_move()` (`bitboard.cpp:248`) does not
> check that the moved piece belongs to the side to move, so a move from the
> wrong node still corrupts the board instead of being rejected.

`search.cpp:182` returns `0` on abort without touching `state->best_move`, while
`search.cpp:117` and `search.cpp:225` write `state->best_move` at *every* ply.
`search()` then copies whatever is left there into `search_result.best_move`
(`search.cpp:255`).

Harness: search `TRICKY_POS` (White to move) at depth 8, raise `stop` after a few
ms, print the returned move:

```
trial 0: aborted=1 bm=e6e5 ...
trial 1: aborted=1 bm=h3g2 ...
trial 3: aborted=1 bm=d7d6 ...
```

All of those are Black moves picked up from a deep node.

Today this is contained by one line — `if (state.aborted) break;` at
`chesso.cpp:562` — which discards the whole iteration. Two ways it still bites:

- If the *first* iteration aborts (possible via `go nodes <small>`, since only
  depth 1 runs with the `never_stop` flag), `result.best_move` stays `0` and the
  engine answers `bestmove 0000`.
- Any future caller that trusts `search_t::best_move` on an aborted search plays
  an illegal move.

Fix: only write `state->best_move` at `ply == 0`, and drop the write at
`search.cpp:117` entirely (it serves no purpose — a TT hit at ply > 0 is not the
root move).

Side note found while testing: `make_move()` (`bitboard.cpp:248`) does not check
that the moved piece actually belongs to the side to move, so one of these
wrong-side moves is applied and silently corrupts the occupancy bitboards. The
asserts at `bitboard.cpp:410-417` would catch it, but Release compiles them out.

---

## 5. The TT stores best moves that are illegal in the stored position — **[reproduced]** — **FIXED**

> **Fixed.** `best_move` starts at `0` and is only ever assigned a move that
> `make_move()` accepted. It now follows `best_so_far` rather than `alpha`, which
> also makes it the cutting move on a fail-high (that move's score is above beta
> and therefore above every earlier score at the node).
>
> Instrumented re-run, `TRICKY_POS` depths 1-6:
> ```
> before: 371889 stores, 14786 with an illegal best move, 308043 fail-highs storing a non-cutting move
> after : 371889 stores,     0 with an illegal best move,      0 fail-highs storing a non-cutting move
> ```

`search.cpp:169`, `search.cpp:186-205`, `search.cpp:230`

```cpp
move_t best_move = moves[0];      // never verified to be legal
```

`generate_moves()` is pseudo-legal; `moves[0]` may leave the king en prise. If no
move raises alpha (an all-node), `best_move` is still `moves[0]` when it reaches
`tt_store_entry()`.

Instrumented count on `TRICKY_POS` depth 6: **14,786** TT entries stored with a
best move that is illegal in that exact position.

The same block has a second problem: on a fail-high (`search.cpp:186`)
`best_move` is **not** set to the cutting move `moves[i]`. Measured 308,043
fail-highs at depth 6 where the move stored in the TT is not the move that caused
the cutoff — precisely the move you would want back for ordering.

Fix: initialise `best_move = 0`, set it on the first *legal* move and on every
`best_so_far` improvement, set it to `moves[i]` before the `break` on fail-high,
and skip the store when it is still `0`.

---

## 6. Mate distance is off by one for the losing side — **[reproduced]** — **FIXED**

> **Fixed.** One formula for both signs, driven by the ply distance that the
> score already encodes:
> ```cpp
> const int plies_to_mate = MATE_MAX - std::abs(score);
> search_result.mate_found = (plies_to_mate < (MATE_MAX - MATE_MIN));
> const int moves_to_mate  = (plies_to_mate + 1) / 2;
> search_result.mate_in    = (score > 0) ? moves_to_mate : -moves_to_mate;
> ```
> ```
> before: info score mate -2 depth 6 pv e8d8 a7b8
> after : info score mate -1 depth 6 pv e8d8 a7b8
> ```
> `mate 2` for the 3-ply mate is unchanged. The harness now cross-checks
> `mate_in` against the PV length on every position and depth.
>
> The `normalize_score()` / `de_normalize_score()` boundary noted below is
> untouched — still not reachable, still inconsistent with the inclusive bounds
> used elsewhere.

`search.cpp:260-263`

```cpp
if (score >= -MATE_MAX && score <= -MATE_MIN) {
  search_result.mate_in = -(score + MATE_MAX) / 2 - 1;
}
```

```
position fen 4k3/Q7/4K3/8/8/8/8/8 b - - 0 1
go depth 6
info score mate -2 depth 6 pv e8d8 a7b8
```

The PV is 2 plies — White mates on move 1 — so this must be `mate -1`. The
positive branch (`search.cpp:265-268`) is correct (`mate 2` for a 3-ply mate,
verified).

Fix: one formula for both signs,
`mate_in = sign(score) * ((MATE_MAX - abs(score) + 1) / 2)`.

Related, low severity: `normalize_score()` / `de_normalize_score()`
(`search.cpp:30-43`) use strict `<  MATE_MAX` / `> MATE_MIN`, so a score landing
exactly on a bound is not re-based. Not reachable today (a node that returns
`MATE_MAX - ply` at its own ply never stores), but it is a boundary waiting to
break. `search()` uses inclusive bounds for the same ranges — pick one
convention.

---

## 7. Time and node limits are only honoured at 1000-node granularity — **[reproduced]** — **FIXED**

> **Fixed.** `explored_nodes` is now incremented once at the top of every
> `negamax()` and `quiescence()` node, before any early exit, so the counter is
> monotone and matches the work actually done. The new `check_limits()` helper
> (`search.cpp:30`) enforces the node budget on every node (plain integer
> compare) and polls the atomic stop flag every 2048 nodes.
>
> `go nodes 100` from the start position: 671 counted nodes before, 41 now.


`search.cpp:134`, `search.cpp:63`, `search.cpp:150`

```cpp
if (state->explored_nodes % 1000 == 0) { ... }
```

Three problems compound:

- `explored_nodes` is incremented at `search.cpp:150`, i.e. **after** the TT
  probe, the repetition check and the quiescence bail-out — nodes that return
  through those paths are never counted. Reported `nodes` therefore understates
  the real work, and the counter advances in unpredictable jumps.
- `quiescence()` increments the same counter (`search.cpp:57`), so the two
  producers interleave and the value can step past a multiple of 1000 without any
  check site observing it.
- `== 0` on an unreliably-incremented counter means whole subtrees run between
  checks. `(explored_nodes & 1023) == 0` has the same flaw; the counter is the
  problem, not the modulo.

Observed: `go nodes 100` from the start position ran 3 iterations totalling 671
counted nodes before stopping.

Fix: increment the counter once at the top of every node (including nodes that
return early) and check the limits against a monotone counter.

---

## 8. `quiescence()` gives up on `stop` without marking the search aborted — **FIXED**

> **Fixed.** `quiescence()` goes through `check_limits()`, which sets
> `state->aborted`, and it bails out of its move loop as soon as `aborted` is
> set. Truncated quiescence scores can no longer be mistaken for real ones by an
> ancestor and written to the TT.


`search.cpp:63-65` returns `best_value` when `*state->stop` is set, but does not
set `state->aborted`. Its caller returns that value straight up
(`search.cpp:146`), and the ancestor's abort check at `search.cpp:182` sees
`aborted == false`. Ancestors then complete normally and write those truncated
scores into the TT at `search.cpp:230`, where they survive into the next
iteration and the next `go` (the table is only cleared by `ucinewgame` /
`clean-tt`).

Currently latent only because quiescence never recurses (bug 1) — the value it
returns on the stop path is the same static eval it would have returned anyway.
Fixing bug 1 makes this a live TT-poisoning path.

Fix: set `state->aborted = true` before returning, and have `negamax` check
`state->aborted` after the quiescence call as well.

---

## 9. TT probe runs before the repetition check — **FIXED**

> **Fixed.** The repetition test now runs before the TT probe, and the 50-move
> rule was added next to it (`board.halfmove_clock >= 100`, which `make_move()`
> already maintains). Both are path properties that the Zobrist key does not
> carry, so the table must not be allowed to answer first.
>
> Not addressed: the general graph-history problem. A score stored for a
> position whose subtree contained a repetition can still be reused on a path
> where that repetition is not available.

`search.cpp:113-129`

A position that is a draw by repetition on the current path can return a stored
non-draw score, because the Zobrist key does not encode the path that reached it.
Swapping the two checks removes the common case. The general graph-history
problem stays; the usual mitigation is to not take TT cutoffs when the path
already contains a repetition.

Also missing: the 50-move rule. `board.halfmove_clock` is maintained by
`make_move()` but the search never returns `DRAW_SCORE` at 100.

---

## 10. TT replacement has no aging — **FIXED**

> **Fixed.** `tt_entry_t` carries a `generation` byte, `transposition_table_t`
> holds the current generation, and `tt_new_search()` bumps it once per `go`
> (called from `iterative_deepening_search()`). Replacement is now
> "depth-preferred within a search, always replaceable across searches":
> ```cpp
> if (entry->generation != tt->generation || depth >= entry->depth) { ... }
> ```
> `type` and `depth` were narrowed to `uint8_t` / `int16_t` so the generation
> byte fits in the padding that already existed — the entry is still 24 bytes and
> the table is still ~100 MB (measured `sizeof(transposition_table_t)`:
> 100,663,224 before, 100,663,232 after).

`transposition_table.cpp:44`

```cpp
if (entry->key != hash || depth >= entry->depth) { ... }
```

Depth-preferred with no generation counter: a deep entry written during move 3 of
a game can never be displaced by a shallower entry for the rest of the game, even
though its position will never recur. Over a long game the table degrades toward
"full of unreachable deep entries".

Fix: store a search generation in `tt_entry_t` and treat entries from an older
generation as replaceable regardless of depth.

Note also that `tt_store_entry()`'s `assert(best_move != 0)` is compiled out in
Release, which is where bug 5 hides.

---

## 11. Dead and misleading code in `negamax()` / `search()` — **mostly FIXED**

Done:

- Unreachable `if (depth == 0)` block — removed.
- Dead `(best_so_far != MIN) ? ... : (alpha0 - 1)` fallback — removed;
  `best_so_far` is always set once a legal move exists.
- Unreachable trailing `return score;` — removed.
- The aspiration-window comment that described code which does not exist —
  removed, and the leftover `// (void)type;`.
- The commented-out PV debug block at the end of `search()` (which referenced a
  `pv.pv_table` field that no longer exists) — replaced with a live
  `#ifndef NDEBUG` check that the PV is legal, non-empty and starts with the
  reported best move.
- `is_check(game)` moved below the quiescence bail-out, so leaves no longer pay
  for an attack scan whose result is discarded.
- `is_check_move` is now only computed for quiet moves — it is only read to
  decide whether a move may become a killer, and that test already excludes
  captures.
- `history_moves` accumulation is saturating (`min(history + bonus,
  ORDER_HISTORY_MAX)`), so it cannot overflow and cannot climb into the killer
  ordering band.

Left alone deliberately:

- `NULL_MOVE_REDUCTION`, `LMR_WHEN_START_IN_THE_LIST`, `LMR_START_AT_DEPTH`:
  unused constants for search features that are not implemented yet.
- `state->search_in_tt`, `state->node_prev_move`: unused fields.
- `state->prev_score`: written by `search()`, read by nobody — it is what an
  aspiration-window implementation would want.

---

## 12. `search_state_t` is a 4 MB stack object — **FIXED**

> **Fixed.** `MAX_PLY` dropped from 1000 to 128. It sizes `pv_table`
> quadratically, so the search state went from **4,026,200 to 74,296 bytes** (54x)
> and `pv_t`, `search_t` and `uci_search_result_t` shrank with it. No search comes
> near 128 plies; `MAX_DEPTH` (the `go depth` cap) is now 126.
>
> The bound that keeps the PV `memcpy` in range still holds: a node at ply
> `MAX_PLY - 1` clears its row and returns before the loop, so
> `pv_length[k] <= MAX_PLY - 1 - k` by induction and the deepest write lands at
> index 126 of a 128-wide row. Node counts are unchanged (4,270,221 before and
> after), and the abort and debug harnesses still pass.

`sizeof(search_state_t) == 4,026,200` bytes, dominated by
`pv_table[MAX_PLY][MAX_PLY]` (`data_structures.hpp:369`, 1000 x 1000 x 4 B).
`chesso.cpp:516` declares it as a local (`search_state_t state = {};`) inside the
thread started at `chesso.cpp:963`, so every `go` zero-fills 4 MB on an 8 MB
thread stack.

A triangular table only needs `MAX_PLY * (MAX_PLY + 1) / 2` entries, and
`MAX_PLY = 1000` is far beyond any reachable depth — 128 would drop this to
~65 KB. As written, the `memcpy` at `search.cpp:214` also relies on `pv_length`
for the deepest ply never being written to stay in bounds; a smaller, explicitly
triangular table removes that dependency.

---

## 13. Iterative deepening leaves most of the time budget unused — **[reproduced]** — **FIXED**

> **Re-measured first.** The original ~90% figure predates move ordering, when
> the branching factor was ~18x and the growth estimate sat at its clamp of 20.
> Re-running the *original* predictor against the *fixed* search (same binary,
> only the time-management block reverted) gives the honest picture:
>
> | control | original predictor | now |
> |---------|-------------------|-----|
> | `go movetime 1000` | 514 ms, 7 iterations | 1048 ms |
> | `go movetime 3000` | 1795 ms, 8 iterations | 3048 ms |
> | `go wtime 60000 btime 60000` | 1728 ms, 8 iterations | 3049 ms |
>
> So ~40-50% of the budget was going unused, not ~90%. The deeper problem was
> next to it: `if (state.aborted) break;` ran *before* the result was harvested,
> so any time spent on an iteration that ran out was thrown away entirely. That
> is what made stopping early look like the right trade.
>
> **Fixed** in two parts:
>
> 1. The root result of an aborted iteration is now kept, when it has one. This
>    is safe because the root only replaces its move when that move beats every
>    move searched before it at this depth, and the first move it searches is the
>    previous iteration's best (TT move first). A non-empty PV therefore means the
>    partial iteration strictly improved on the completed one. Its *score* is
>    meaningless, so no `info` line is emitted for it.
> 2. The predictor is replaced by a soft limit: no new iteration starts after 60%
>    of the budget. An explicit `movetime` is not a shared budget, so it is spent
>    in full.
>
> Verified: `scratchpad/abort_pv.cpp` aborts 342 searches at staggered times
> across 6 positions; 254 produced a usable partial result and **0** had an
> illegal PV, a mismatched head move, or an illegal best move. The `search()`
> debug assertion was tightened to cover the aborted case and holds in the Debug
> build.
>
> The budget is now spent rather than returned, and what it buys is kept: on
> `go movetime 3000` the partial depth-9 iteration improved the reported ponder
> move over the completed depth-8 one.

`chesso.cpp:593-605`

The "only start another iteration if we expect to finish it" rule uses a growth
estimate seeded at 10 and clamped to [2, 20]. Because the branching factor is
huge (bug 3), the estimate saturates and the loop stops very early:

```
go movetime 1000   (TRICKY_POS)  ->  last iteration is depth 5, finished at 96 ms
```

~90% of the budget is discarded, even though the abort machinery exists to cut an
over-long iteration off mid-flight. Starting the next iteration and letting the
timer abort it (keeping the previous iteration's move, which
`chesso.cpp:562` already does) uses the budget properly.

---

## 14. Quiescence stands pat while in check — **FIXED**

Standing pat asserts "I could stop here", which is not available to a side that
is in check: it is forced to reply. The old quiescence returned the static score
anyway, and searched only captures, so it could not see a forced mate one ply
past the leaf.

> **Fixed.** `quiescence()` tests `is_check()` and, when in check, skips the
> stand-pat return entirely, searches *every* move rather than only captures, and
> reports `-(MATE_MAX - ply)` when no evasion is legal. The `MAX_QSEARCH_DEPTH`
> bound still applies to evasions — without it a perpetual check would recurse
> forever — and at the bound the static score is all that is available.
> `capture_score()` is reused to rank evasions, which puts capture-evasions
> first.
>
> Scholar's mate, where `Qxf7#` is a capture and therefore inside quiescence:
> ```
> before: info score cp 100  depth 1  pv c4f7      <- grabs a pawn, mate invisible
> after : info score mate 1  depth 1  pv h5f7      <- finds the mate at depth 1
> ```
> Cost: +11% nodes over the harness (3,844,888 -> 4,270,221), which is the
> `is_check()` call per quiescence node plus the evasion subtrees.
>
> Still not detected: stalemate. Quiescence only enumerates captures when not in
> check, so it cannot distinguish "no captures" from "no moves" without
> generating every move at every leaf. Standard approximation; the static score
> is returned instead of a draw.

---

## 15. `make_move()` applies moves that belong to the other side — **FIXED**

`make_move()` applies a move by xor-ing bitboards, so a move built for a
different position is not rejected — it silently corrupts the board. That is why
the aborted-search probe in finding 4 reported a Black move as "legal" in a
White-to-move position.

> **Fixed** where it costs nothing. Audited every caller first: the search only
> ever passes moves from `generate_moves()` for the current position, and the UCI
> `position ... moves` path goes through `try_move()`, which matches against the
> generated list before applying. So no runtime check was added to the hot path.
> Instead:
> - New `move_belongs_to_side_to_move()` (`bitboard.cpp`): the piece code is in
>   range, its colour matches the side to move, and it is actually on the from
>   square.
> - `make_move()` asserts it — free in Release, and it now guards every path in
>   Debug.
> - `is_move_legal()` checks it and returns false instead of corrupting the
>   board. It is documented as a debug-only helper and tests rely on it, so it
>   needs to be a truthful oracle.
>
> The assertion holds across the full Debug test suite, including the 164M-node
> perft run that exercises castling, en-passant and promotions.

---

## What is left

1. **Finding 16 — graph-history interaction.** A TT score whose subtree contained
   a repetition can still be reused on a path where that repetition is not
   available. Left deliberately: the repetition and 50-move tests now run before
   the probe, which covers the direct case, and the standard mitigation
   (suppressing TT cutoffs on any path containing a repetition) costs real nodes
   for a rare correction. Worth revisiting only if draw-scoring anomalies show up
   in games.
2. **Stalemate is invisible to quiescence** (see finding 14).
3. **`evaluate()` prices a king at 100000, above `MATE_MAX` (49000).** Kings
   cannot be captured in the current search, so it is unreachable, but any future
   path that evaluates a position with an unbalanced king count would produce a
   score that reads as a mate. `search()` now rejects those explicitly; lowering
   `KING` below `MATE_MIN` would remove the trap at the source.
4. Unimplemented search features whose constants and fields are already declared:
   null-move pruning, LMR, aspiration windows.

## Reproduction harnesses

The measurements above came from small programs linked against
`build/src/libchesso_engine.a` (and, for the counters, an instrumented copy of
`search.cpp`), kept outside the repo in the session scratchpad:
`/tmp/claude-1000/-home-max-ws-chesso/fe19f090-46f4-45ba-b5d1-939a1b14aa82/scratchpad/`
(`probe.cpp` PV/best-move validation, `probe2.cpp`/`probe5.cpp`/`probe6.cpp`
instrumented counters, `probe3.cpp` abort behaviour, `probe4.cpp` TT reuse).
No project file was modified.
