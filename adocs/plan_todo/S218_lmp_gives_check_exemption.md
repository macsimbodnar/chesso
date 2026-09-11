id:         S218
goal:       late move pruning is bought the gives-check exemption the three per-move rules already have, and the exemption is decided by its own SPRT against S109's shipped form (DEC-180)
accepts:    after S109 lands, a variant in `src/search.cpp` `negamax` where the late-move rule no longer abandons the quiet stage at generation alone but lets each late quiet through `make_move` and skips it unless `is_check_move` -- so LMP's skip is a per-move rule sharing the predicate the other three use -- behind a constant in `src/search_params.hpp` with a stated range and a comment saying which form is which; or, if the make/unmake cost of that form eats the gain at fixed depth, a pre-make gives-check predicate that lets the generation stage keep only the checking quiets, with the discovered-check case handled or over-approximated and the choice stated; the "pruning does not hide a forced mate" case in `tests/test_search.cpp` gains a position whose mating quiet is a late checking move inside the pruned depth, **observed red with the exemption disabled and green with it**, printout recorded; the cost of the exemption at fixed depth is measured with `tools/search_bench.py` -- nodes and nps at 9 and 12 -- and stated; one SPRT against S109's shipped form at the DEC-143 pair, pre-registered with its worst-case games and its abort rule, the verdict recorded as it comes and zero as zero; a negative or zero verdict reverts the exemption and S109's form stands, with the reason in the stamp; **if S109's own mate guard went red without the exemption, this step folds into S109 as its fix and this file says so** (DEC-180); a mutant that disables the exemption dies under the new case (`tools/mutation_check.py`); Debug self-play four rounds at 4+0.04 with `Assertion` grepped (DEC-141); `Bench:` line on the commit
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, adocs/data/
excludes:   any other change to S109's four rules or their thresholds; an exemption for captures -- LMP is a quiets rule; any change to the LMR guard that already refuses to reduce a checking move
decisions:  DEC-180, DEC-082, DEC-141, DEC-143, DEC-105
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S109's `accepts` states the exposure in so many words: the late-move rule sets
a skip-quiets flag that the staged generator honours *before any move is
made*, where the engine has no gives-check predicate to consult, so LMP may end
a quiet stage that still holds checking quiets. The published LMP form carries
no exemption and S109 ships that form. Whether to buy the exemption with a
post-make prune -- a departure from the published form -- was the owner's call
by S109's own wording, and the owner took it on 2026-09-11 (DEC-180): S109
first, one verdict; then this, one verdict.

The project's recurring bug is pruning that hides a mate -- null move pruning
hid a mate in 2, late move reduction reduced the mating move at the root, both
caught by the mate case in the fast suite. A late checking quiet that mates is
exactly the move LMP's published form is allowed to skip; whether that costs
Elo at this engine's depths is what the SPRT answers, and the mate case is
what says whether it is a bug before any game is played.

## Two shapes, and the order they are tried in

**(a) Post-make skip.** Keep S109's generation-stage flag. When it is set,
quiets are still generated; each is made, and skipped unless it gives check.
Simplest, shares `is_check_move`, and pays a make/unmake per skipped quiet --
which is the cost LMP exists to avoid, so it is measured at fixed depth
before any match is booked.

**(b) Pre-make predicate.** A cheap test from the destination square to the
enemy king -- direct checks by piece attack, discovered checks over-approximated
by "the moving piece leaves a line between a slider and the king" -- so the
generator can keep the checking quiets and drop the rest without making any.
No make cost, new code, and a correctness burden the test set must carry. Taken
only if (a)'s cost is measured to eat its gain.

Published record on exempting checking quiets from move-count pruning is
**unverified at the time of writing** and is sourced by the implementer before
the pre-registration is written, in DEC-105 form: figures and URLs, no
constants as seeds.

## Cost

One `src/` change behind a constant, one test position, one SPRT in the fast
class -- priced by DEC-143's formula in the pre-registration, not here. If
S109's verdict is negative the block is re-decided first and this file is
re-scoped then.
