id:         S125
goal:       backward, phalanx, supported and weak unopposed pawns join the three terms that exist, each fitted
accepts:    an SPRT verdict per group, recorded whatever it is; the terms are indexed by file or by rank where the surveyed record says the indexing is what pays, and the step states which indexing it chose and why; **an isolated pawn is never also counted backward**, which is a documented double-count worth +4.01 to fix; every constant is fitted (DEC-084); **the pawn hash is not a precondition and the plan order is not reversed** -- these terms are what make S118 worth building, so they are measured recomputed at every `evaluate()` call, and the per-call cost the additions carry is measured here (`bench_eval`, and `build/tools/eval_spread` for where it lands) and recorded as the baseline S118 is later asked to reclaim
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp
excludes:   passed pawns, which are S123
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## What is there

Three terms -- isolated, doubled, backward -- each one flat weight, fitted to
`{-10, -9, -11}` middlegame and `{-12, -32, -8}` endgame. What is missing is
phalanx and connected pawns, supported pawns, and weak unopposed pawns; and
what the surveyed record says pays more than any of them is **conditioning**
rather than adding: the largest pure pawn-structure patch on record is "apply
the isolated penalty only when there is no pawn capture available", at +10.68.
Indexing the existing three by file and by rank is reported at +6.60 and +3.77.

So this step is as much about giving the three terms that exist more shape as
about adding four more.

## The pawn hash comes after, and that is deliberate

The `accepts:` used to require these terms to "live behind the pawn hash from
S118", which S118 cannot supply at this point in the order. S139 dropped the
dependency rather than reorder, because every document that touches the
question orders the terms first and says why:

- `adocs/plan.md` "pawns with king distance (S123, +22.3 the largest" -- *"the
  connected and phalanx pawn work (S125, +25.4 class), the pawn hash that makes
  them affordable (S118)"*. Terms, then the cache.
- `adocs/plan.md` "block to land after the pawn terms it caches are" -- *"S118
  moves out of the speed block to land after the pawn terms it caches are worth
  caching (a cheap pawn eval cached measured a published slowdown)."*
- DEC-087 (h) -- *"S118 moves from the speed block into the evaluation block,
  behind the expensive pawn terms -- caching a cheap pawn evaluation measured a
  10 % slowdown in the published record"* -- and (i) prices the block in Stash
  ledger order, connected/phalanx ahead of the pawn hash.
- `adocs/plan_todo/S118_pawn_hash_table.md` "implementer who cached a
  still-cheap pawn evaluation" -- S118's own body: an *"implementer who cached
  a still-cheap pawn evaluation measured a 10 % slowdown"*, so *"this step now
  lands in the evaluation block, after S123 and S125 have made the pawn
  evaluation worth caching"*; and `:37`, *"it is what makes S125's richer pawn
  terms affordable"*.
- The list itself: `adocs/plan.md` "backward, phalanx, supported and weak
  unopposed pawns join the three", S125 at 54 and S118 at 55. (Both ranges
  moved when S140 rewrote the block-3 paragraph on 2026-08-21; this one was
  also one entry low before that, naming 55 and 56 while the sentence claims 54
  and 55.)

Reversing the pair would put a cache in front of the cheap computation it
caches, which is the measured 10 % slowdown DEC-087 moved S118 to avoid. So the
cost this step adds is paid per call and measured per call; S118 is the step
that reclaims it, and this step's numbers are what its verdict is read against.

**And it is not CLAUDE.md's accumulate-don't-recompute hazard either**, which
is about an accumulated quantity being rebuilt from the bitboards at every node
-- INV-4's four fields, and the 25 % of nps S014 took back. These four terms do
not exist in any form yet, so nothing stops being accumulated. What they cost
is what a new pawn term costs before a cache exists, and the SPRT per group
prices exactly that: term value net of term cost, which is the honest question
at this position in the order. S118 reads the same boundary from its side
(`adocs/plan_todo/S118_pawn_hash_table.md` "the bitboards at every node; this
is a cache keyed").