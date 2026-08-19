id:         S112
goal:       quiescence skips a capture whose best case cannot reach alpha, per move, before the exchange evaluation is consulted
accepts:    an SPRT verdict, recorded whatever it is; the prune path raises best_value to the futility value rather than dropping it, because quiescence is fail-soft and skipping that assignment returns bounds that are too low; no futility while in check, at a promotion, or on a move that gives check; the margin is a constant in src/search_params.hpp with a range; the mate-in-quiescence case in the fast suite still passes
touches:    src/search.cpp quiescence, src/search_params.hpp
excludes:   delta pruning, which is S022 and is re-decided after this
decisions:  DEC-019
closes:
blocks:
paused_by:
done:

## Why this comes before S022 and re-opens S015

Two of this project's three "published figure measured zero" cases live in
quiescence, and the published record now explains both.

**S015, SEE pruning in quiescence, measured 0.** The reported figures for SEE
pruning assume per-move futility is present; the two prune overlapping sets and
whichever arrives second measures the remainder. Chesso has the second and not
the first. So S015's zero is consistent with the literature rather than a
contradiction of it, and this step is the one that makes it re-measurable --
`specs.md` already parks that re-run inside S022.

**S022, delta pruning.** One engine *gained* by **deleting** delta pruning once
per-move futility existed. So S022 is re-targeted by this step from "add delta
pruning" to "decide between the two, by measurement", and deleting is a valid
outcome to record.
