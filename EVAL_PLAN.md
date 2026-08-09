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

## Phase 0 - unblock evaluation work — 0.1 to 0.3 DONE

Behaviour-neutral by construction, and confirmed: node counts after the change
are identical to before it, 43691503 / 36729994 / 27160039 on the three
`search_bench.py` positions, with the same best moves.

- **0.1 done.** The king is out of `piece_values[]`. It could only ever cancel
  in a legal position, and pricing it meant an illegal one scored above every
  mate. `piece_values_abs[]`, which move ordering uses, keeps its king price -
  the king really is the least desirable capturer, and changing that is a
  move-ordering change that needs its own SPRT.
- **0.2 done.** `evaluate()` returns from the side to move's point of view. The
  two call sites no longer apply a sign.
- **0.3 done.** `game_phase()` returns 24 down to 0, clamped so promotions
  cannot push it past the maximum, with pawns and kings contributing nothing.
- **0.4 deferred to phase 1.** Incremental versus recomputed only becomes a real
  question once there is something to accumulate. Measure it there.

Four test contracts changed with the code, each rewritten to assert the new
behaviour rather than relaxed:

- colour symmetry now expects the mirrored score to *agree* rather than negate,
  because `mirror_fen()` swaps the side to move along with the colours;
- "score is from White's point of view" became "from the side to move's",
  and gained the mirror case so a symmetric sign error cannot pass;
- the missing-king anchor asserts the king is worth nothing and that the score
  stays inside the mate band;
- two search tests that compensated for the old convention by hand.

New tests cover `game_phase()`: the full board, bare kings, a pawn endgame,
each piece's weight, and the promotion overflow case.

## Phase 0 - original entry

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

## Phase 1 - tapered piece-square tables — DONE, awaiting SPRT

Two tables per piece, middlegame and endgame, interpolated on `game_phase()`.
Written by hand from ordinary positional principles rather than taken from a
published set; phase 5 replaces them by fitting to game outcomes, and until then
they are worth much less than a tuned table.

**Cost: about 25 % of nodes per second**, 13.0 down to 10.1 Mnps, because the
tables are recomputed from the bitboards on every call. That is the 0.4
question arriving: whether to accumulate the score in `make_move` instead. Left
as it is for now, so the SPRT measures the evaluation rather than the
evaluation and an optimisation together.

Three test positions turned out to be over-specified rather than wrong, and all
three had been passing by accident of move ordering:

- "the doubled rooks win the queen" had the black king on e8, where the queen is
  pinned to it and *every* white move wins it. Moved the king to d8, where the
  queen can run and Rxe7 has to be played at once.
- "take the free pawn" had the kings close enough that the black king walks back
  and wins the pawn again, so the position is drawn whatever White plays. Kings
  moved to opposite corners.
- "a bare king endgame is a draw" asserted a score of exactly zero, which only
  held because a material-only evaluation is flat. It is now true for the right
  reason - see below.

### Insufficient material

Added, because the alternative was weakening that last test. Without it the
search prefers one drawn position to another - a centralised king scores better
than a cornered one - and reports a bare king endgame as a small advantage.

Deliberately strict: king against king, and a single minor against a bare king.
Knight against knight and same-colour bishops are drawn in practice but not by
the laws, and claiming them would throw away positions still winnable on the
clock. Applied at every node except the root, which still has to return a move.

**This is bundled into the same commit as the tables**, so the SPRT below
measures both. They are separable if the result is bad.

## Phase 1 - original entry

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

### 3.0 Bad captures after the quiets — TRIED, SET ASIDE, revisit

Other engines get their SEE Elo here rather than from quiescence pruning.
Stockfish splits captures into `GOOD_CAPTURE` and `BAD_CAPTURE` stages: good
ones are searched before the quiets, bad ones after. Move ordering at fixed
depth is reported to be worth on the order of 150 Elo, and the case it fixes is
a queen taking a defended pawn being searched ahead of every killer.

**It measured slower here, three ways.** Depth 13, three positions:

| where the exchange analysis happened | against the two-stage build |
|---|---|
| exact `see()` inside `score_move` | +17.6 % |
| `see_ge` inside `score_move` | +13 % |
| lazily, in the picker, on the move about to be searched | +3 % |

Nodes do fall where it fires - midgame 2,288,701 to 1,870,912 - but the calls
cost more than the ordering saves on this engine.

**How to rebuild it**, since the code is not in the tree:

- an `ORDER_BAD_CAPTURE` band below zero, since history scores start at zero and
  only climb, so the sign of the best remaining score says the good captures
  have run out;
- `score_move()` keeps scoring every capture optimistically;
- a `demote_if_losing_capture(game, move, &score)` called from the picker after
  `pick_next_move`, looping while it returns true, so each capture is asked at
  most once and only if the search actually reaches it;
- `negamax` generates the quiets when the best remaining score goes negative,
  rather than when the captures run out.

**The trap to avoid.** The first version guarded on `*score >= ORDER_CAPTURE`.
A losing capture is scored `ORDER_CAPTURE` plus a *negative* most-valuable-victim
term, so a queen taking a pawn sits at 999200 - below `ORDER_CAPTURE` - and the
guard rejected exactly the moves it existed to catch. It looked like it worked
and changed nothing. Identical node counts is what exposed it; the timings did
not. Guard on `*score < 0` for "already demoted" instead.

**What would make it pay.** The cost is the exchange analysis, so anything that
makes it rarer or cheaper: a capture history table that orders captures without
SEE, or reaching the point where quiet move ordering is good enough that the
extra work buys a real cut. Worth retrying after 3.3 and 3.4 exist.

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
