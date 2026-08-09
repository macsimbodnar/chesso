id:         S025
goal:       retry searching losing captures after the quiets, now that capture history exists
accepts:    fixed-depth time is not worse than the two-stage build on all three search_bench positions; only then an SPRT
touches:    src/search.cpp move picker and staging, src/evaluation.cpp score_move
excludes:   attempting it before S023 and S024 exist, which is the whole reason it was set aside
decisions:  DEC-022
closes:
blocks:
paused_by:
done:

## It was tried once and measured slower, three ways

Depth 13, three positions, against the two-stage build:

| where the exchange analysis happened | result |
|---|---|
| exact `see()` inside `score_move` | +17.6 % |
| `see_ge` inside `score_move` | +13 % |
| lazily, in the picker, on the move about to be searched | +3 % |

Nodes do fall where it fires -- midgame 2288701 to 1870912 -- but the calls
cost more than the ordering saves on this engine. See DEC-022.

## How to rebuild it, since the code is not in the tree

- an `ORDER_BAD_CAPTURE` band below zero, since history scores start at zero and
  only climb, so the sign of the best remaining score says the good captures
  have run out;
- `score_move()` keeps scoring every capture optimistically;
- a `demote_if_losing_capture(game, move, &score)` called from the picker after
  `pick_next_move`, looping while it returns true, so each capture is asked at
  most once and only if the search actually reaches it;
- `negamax` generates the quiets when the best remaining score goes negative,
  rather than when the captures run out.

## The trap

The first version guarded on `*score >= ORDER_CAPTURE`. A losing capture is
scored `ORDER_CAPTURE` plus a *negative* victim term, so a queen taking a pawn
sits at 999200 -- below `ORDER_CAPTURE` -- and the guard rejected exactly the
moves it existed to catch. It looked like it worked and changed nothing.
**Identical node counts exposed it; the timings did not.** Guard on `*score < 0`
for "already demoted" instead.

## What would make it pay

Anything that makes the exchange analysis rarer or cheaper. S023 orders captures
without SEE at all, which is the reason this step sits after it.
