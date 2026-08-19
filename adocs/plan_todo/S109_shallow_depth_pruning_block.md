id:         S109
goal:       late move pruning, futility pruning, history pruning and quiet SEE pruning enter the move loop together, gated on the reduction-adjusted depth, as one step and one verdict
accepts:    all four rules land in one commit and are measured by **one** SPRT, whatever it returns, recorded as it comes (INV-6); every threshold and margin is a constant in src/search_params.hpp with a stated range and none of them is a number copied from anywhere (DEC-084); the four rules are gated on `depth - lmr_reduction(depth, move_number)` and not on raw depth; the late-move rule sets a skip-quiets flag the staged generator honours rather than `continue`-ing, so the quiet stage is abandoned and not merely skipped over; **a position with a forced mate inside the pruned depth is added to the "pruning does not hide a forced mate" case in tests/test_search.cpp, observed red with the guards removed and the printout recorded**; no quiet is pruned while in check, at a PV node, on the first move, when the move gives check, or when alpha or beta is near mate, and the test asserts the precondition that would otherwise prune it; the fast suite green
touches:    src/search.cpp negamax, src/search_params.hpp, tests/test_search.cpp
excludes:   razoring, which is a node-level rule and is S116; SEE pruning of **captures** in the main search, which is S091; futility inside quiescence, which is S112; the improving flag, which S108 supplies and this step consumes
decisions:  DEC-071, DEC-082, DEC-084
closes:
blocks:
paused_by:
done:

## Why this is one step and not four

**Per-part attribution is deliberately forfeited here, and DEC-082 is the
decision that permits it.** The four rules prune overlapping sets of moves, so
each one measured against a tree the other three are absent from returns
nothing. Stockfish's own removal test is the evidence: move-count pruning
measures **~0 Elo alone** and the block it belongs to measures **~204**. Four
steps would spend four verdicts to record four zeros and leave this plan
believing a real +60 to +120 does not exist.

If the block fails, *then* it is bisected -- a failing block is evidence that
one part is wrong, which is the attribution question the block form was not
asked.

**This step absorbs S090 and S026, both retired.** Their ids are not reused.
S090 was late move pruning alone; S026 was "drop nodes near the horizon that
cannot reach alpha", whose razoring half is now S116 and whose futility half is
here. The mate clause below is S026's own, carried forward, and the retired
S060 existed to put it in S026's `accepts:` where it now sits in this step's.

## The four rules

For a quiet move, at `lmr_depth = depth - lmr_reduction(depth, move_number)`:

1. **Late move pruning.** Past a move count that grows with depth, stop
   generating quiets. The count is **doubled when improving** -- that is the
   single highest-leverage use of the flag S108 supplies.
2. **Futility.** Static score plus a base plus a slope times `lmr_depth` at or
   below alpha, skip the remaining quiets.
3. **History pruning.** History below a margin scaled by depth, skip.
4. **Quiet SEE pruning.** The exchange evaluation says the move loses more than
   a margin scaled by `lmr_depth` squared, skip.

The functional forms are read from the published record; **every constant is
ours and is fitted** -- a first setting from a sweep here, and SPSA at S127.
DEC-084.

## Hazard, three times observed and once more expected

Null move pruning hid a mate in two by reducing to depth 0. Late move reduction
reduced the mating move at the root. Reverse futility returns a static bound
and therefore cannot see a mate at all, which is why S033 bounded it to depth 6
and ply 3. All three were caught by a mate test rather than by a benchmark.
This is the fourth, fifth, sixth and seventh pruning rule in the engine and
they get the same treatment before any of them is called done -- the clause is
in the `accepts:` above rather than left for a later step to add.
