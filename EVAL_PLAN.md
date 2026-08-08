# Evaluation plan

Companion to `TMP_PLAN.md`, which covers move generation and is now mostly
history. This one covers evaluation, move ordering and the search features that
decide whether evaluation work pays for itself.

## What the engine actually has today

Read at commit `ca597fb`.

| area | state |
|---|---|
| board, move generation | good. Legal-only, templated, 89.6 Mnps perft, well tested |
| search skeleton | alpha-beta, transposition table, quiescence, PV, iterative deepening |
| move ordering | TT move, MVV-LVA, killers, countermove, history |
| **evaluation** | **material count only. 14 lines** |
| null move pruning | **absent** |
| late move reduction | **absent** |
| principal variation search | **absent** |
| aspiration windows | **absent** |
| futility, razoring, singular extensions | **absent** |
| static exchange evaluation | **absent** |
| capture history, continuation history | **absent** |
| delta pruning in quiescence | **absent** |

`evaluate()` sums `piece_values[]` over the twelve bitboards and returns. There
is no piece-square table, no game phase, no pawn structure, no king safety, no
mobility. Everything the engine knows about a position is how much wood is on
it.

## Integrate, do not restart

**Integrate.** There is nothing in `evaluation.cpp` worth keeping and nothing
worth deleting either - it is 14 lines behind a one-line interface,
`int evaluate(const board_t*)`. A rewrite of *that* is an afternoon.

What a rewrite would throw away is everything around it, and that is the
expensive part: a legal move generator verified to 90 million assertions, a
perft suite with per-column checks, a benchmark that reports its own resolution,
a search-timing harness, and an SPRT setup that has already resolved a 66 Elo
change. Those took a day to get right and they are what makes the next hundred
changes measurable. Starting again discards the asset and keeps the liability.

## One correction to the proposed order

The plan as stated is "really strong evaluation with heuristics first, AI
evaluation later". The second half is right. The first half buys much less than
it looks, for two measured reasons:

**1. The search is the binding constraint, and today proved it.** Staged move
generation was worth **0 Elo** on this engine against a published expectation of
30-50. The reason it failed is that this engine has no late move reduction and
no null move pruning, so it searches quiet moves that a normal engine prunes
outright - their ordering matters more than the cost of generating them. That
is the same reason evaluation improvements will underdeliver: a better score at
the leaves is worth less when the tree above them is the wrong shape.

Reported figures from other engines, which are **not** transferable and should
be treated as direction only:

| feature | reported |
|---|---|
| null move pruning | +100 to +200 Elo |
| late move reduction | ~+100 Elo |
| aspiration windows | ~+9 Elo, error bar +/-17 |

**2. Most hand-crafted evaluation is thrown away when NNUE lands.** NNUE is
reported as several hundred Elo over a mature hand-crafted evaluation. The
terms below king safety and mobility - the ones that take the longest and tune
the hardest - are exactly the ones a network replaces wholesale.

But a floor of hand-crafted evaluation is genuinely needed, because NNUE
training data comes from self-play by an engine that already plays reasonably.
The question is how much. The answer from the field is: **tapered piece-square
tables and little else.** PeSTO, an engine whose evaluation is piece-square
tables and nothing more, is rated about 3125 on CCRL Blitz - on the strength of
its search.

So the order below is: enough evaluation to be sane, then the search features
that make evaluation worth having, then more evaluation, then NNUE.

I have flagged this once. If you want the full hand-crafted evaluation first
regardless, phases 4 and 5 are written out and can be pulled forward - the work
is real either way, it is the sequencing that is at issue.

---

## Phase 0 - unblock evaluation work

Small, and everything after depends on it.

**0.1 Take the king out of material.** `piece_values[]` prices a king at
100000. It cancels in a legal position, but it leaks: `capture_score()` can
return -99900, `search()` needs a special guard because "evaluate() prices a
king above MATE_MAX", and the move-ordering bands survive only by a margin of
100 points. A real evaluation has no king material term.

**0.2 Fix the evaluation's sign convention.** `evaluate()` is White-relative and
every call site multiplies by `(active_color == WHITE) ? +1 : -1`. Make it
return side-to-move-relative once, inside.

**0.3 Add a game phase.** A single integer from the material on the board,
24 at the opening down to 0 in a bare endgame. Every tapered term needs it.

**0.4 Decide incremental versus recomputed.** Piece-square tables can be
accumulated in `make_move`/`unmake_move` for the cost of two adds per move, or
recomputed per evaluation for the cost of a loop over 32 pieces. Incremental is
what NNUE will need later anyway, and `make_move` already has the hooks where
pieces move. **Measure both** - this codebase has already punished one
"obviously fewer operations" change.

Verify: perft unchanged, `ctest` green, `search_bench.py` node counts identical
where the change is meant to be behaviour-neutral.

## Phase 1 - tapered piece-square tables

The single largest evaluation item, and the floor NNUE needs.

Two tables per piece type, middlegame and endgame, interpolated on the phase
from 0.3. Take the PeSTO tables as a starting point - they are published, they
are tuned, and they are the reason a PSQT-only engine reaches 3125.

- Expected: large. This is the difference between an engine that shuffles and
  one that develops.
- Verify: SPRT against the current build. This one should pass quickly.

## Phase 2 - the search features that make evaluation pay

Do these before any further evaluation terms. Each is independently testable.

**2.1 Principal variation search.** Search the first move with a full window
and the rest with a null window, re-searching on a fail-high. Cheap to write,
and it is the precondition for reductions being safe.

**2.2 Null move pruning.** Give the opponent a free move; if the result still
fails high, prune. Needs a zugzwang guard - disable it in the endgame using the
phase from 0.3, and never at a node that is in check.

**2.3 Late move reduction.** Search moves late in the order at reduced depth
and re-search if they beat alpha. Reduce quiets, not captures, and not while in
check. This is the one that makes the staged generation committed today start
paying.

**2.4 Aspiration windows.** Start the root at a narrow window around the
previous score. Small - one engine measured +9 +/- 17 - and worth doing only
after 2.1 to 2.3.

Verify: SPRT each separately. Expect these to dominate everything else on this
list.

## Phase 3 - move ordering

Cheap, and it multiplies the value of phase 2.

**3.1 Static exchange evaluation.** Decide whether a capture wins material
without searching it. Uses it in three places: order losing captures after
quiets rather than among the winners, prune losing captures in quiescence, and
skip them under reductions. The `squares[64]` array added today makes the
attacker/victim lookups O(1), so the implementation is straightforward.

**3.2 Delta pruning in quiescence.** Skip a capture that cannot bring the score
near alpha even if it wins the piece outright.

**3.3 Capture history.** History indexed by moved piece, target square and
captured piece, to order captures that MVV-LVA rates equal.

**3.4 Continuation history.** History indexed by the move played *n* plies ago
and the current move. One-ply is the countermove table already present; two-ply
is the usual next step.

## Phase 4 - hand-crafted evaluation terms

In descending order of what engines generally report. Each one is an SPRT.

1. **Mobility** - legal move count per piece type, tapered. The generator can
   already produce the attack sets cheaply.
2. **King safety** - pawn shield, attacker count and weight on the king zone,
   open files near the king.
3. **Passed pawns** - tapered by rank. `passed_w_pawns_masks[]` and
   `passed_b_pawns_masks[]` already exist in `bb_tables.hpp`, unused.
4. **Pawn structure** - isolated, doubled, backward. `isolated_file_masks[]`
   exists, unused. Cache these in a pawn hash table; pawn structure changes
   rarely and the table hits most of the time.
5. **Bishop pair**, **rook on open and half-open file**, **rook on seventh**.
6. **Tempo** - a small bonus for the side to move.

Expect single or low double digits of Elo each, and expect some of them to
measure at zero. That is normal, and the SPRT harness exists to catch it.

## Phase 5 - tuning

Hand-picked weights are the wrong weights. Texel tuning fits every evaluation
constant at once by minimising prediction error against the results of a large
set of positions.

Needs: a few hundred thousand quiet positions with known game outcomes,
generated by self-play, and a tuner. Worth more than several of the phase 4
terms put together, and it is what turns a set of plausible numbers into a
tuned evaluation.

## Phase 6 - NNUE

Only after phases 1 to 5. The hand-crafted evaluation is what generates the
training data.

- Architecture: the standard `(768 -> N) x 2 -> 1` perspective network is where
  every engine starts. `N` of 256 to 512.
- The accumulator is updated incrementally in `make_move`/`unmake_move`, which
  is the same hook phase 0.4 puts in place for the piece-square tables. Doing
  0.4 incrementally now makes this a smaller step later.
- Inference is integer SIMD. **This is the point where x86 hardware stops being
  optional**: AVX2 and VNNI are where the performance is, and Apple Silicon has
  no equivalent.
- Training is a separate program in Python or Rust, not part of the engine.

## What this needs that does not exist yet

- **A faster test loop.** Every item above is an SPRT, and at 10+0.2 with three
  usable cores a verdict costs an hour. A shorter time control for early
  filtering, and a book with more openings than `8moves_v3.pgn`.
- **SPRT bounds that match the effect.** `elo0=0 elo1=10` cannot resolve a small
  change; today's staged generation run random-walked for 340 games and was
  stopped. Use `elo0=-5 elo1=5` or similar for terms expected to be small.
- **An idle machine.** Today's runs were taken with a background daemon eating
  half a core, and `fastchess.sh` now warns about it.
- **An x86-64 Linux box**, before phase 6 and useful well before that.

## The rule this file inherits

Every estimate in `TMP_PLAN.md` that came from another engine's published
numbers was wrong, and the largest miss was 30-50 Elo. Numbers in this file
marked "reported" are direction, not prediction. The only number that counts is
the one this engine measures on this hardware against its own previous commit.
