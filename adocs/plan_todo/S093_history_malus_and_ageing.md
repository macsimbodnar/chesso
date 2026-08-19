id:         S093
goal:       history gets a malus for the moves that were tried and failed, a gravity update that ages it by construction, butterfly indexing, and survives across go within one game
accepts:    an SPRT verdict per change, measured separately -- malus, ageing and persistence are three changes and one at a time is the rule; the malus applies to the quiet moves searched before the cutoff move and not to the cutoff move itself, asserted by a unit test on the table rather than through a game; the ageing keeps every score inside ORDER_HISTORY_MAX so the move-ordering bands still clear each other by 100 points, with the band clearance asserted (CLAUDE.md hazard, S023, S061); history carried across `go` is cleared on `ucinewgame` and on a position that is not a descendant of the last one searched, with a test for both; the fast suite green
touches:    src/search.cpp history update and the ordering scores, src/search_params.hpp, tests/test_search.cpp
excludes:   capture history, which is S023; continuation history, which is S024; correction history, which is S099
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The hazard is the band, not the search

`piece_values_abs` and the ordering bands clear each other by 100 points: a king
capturing a pawn scores 900100 against 900000 for a killer. `ORDER_HISTORY_MAX`
exists to keep an accumulated history score under a killer, and its own comment
in `src/search_params.hpp` says so. A malus introduces negative scores and
ageing rescales every value in the table, so both touch the one quantity whose
failure mode is a silent strength regression rather than a wrong node count.


## Re-scoped 2026-08-19, and it moved to the front of the search block

Three things rather than two, because the surveyed record treats them as one
mechanism.

**Malus.** A quiet move that caused a cutoff gets a bonus today; every quiet
that was tried at that node and failed gets nothing. Without a penalty, history
is a monotone "moves that ever worked" counter rather than a signed preference.
Reported **+37.49** in one engine -- the largest single history patch on record
and larger than most features on this plan.

**Gravity instead of ageing.** `history += bonus - history * abs(bonus) / MAX`
is self-normalising: entries asymptote to the bound, an unexpected cutoff moves
a lot and an expected one moves little. It **replaces** periodic halving rather
than joining it -- do one or the other, not both.

**Butterfly indexing.** `[piece][to]` conflates a knight on b1 with a knight on
g1 going to the same square. `[colour][from][to]` is what everything surveyed
uses.

### The hazard this walks straight into

`CLAUDE.md` records that the ordering bands clear each other by 100 points and
that inverting a capture against a killer is silent. **Malus makes quiet
history negative**, and quiets are scored today as the raw history value in a
band whose floor is zero. A quiet at -8192 underflows into whatever sits below
it. Give quiets their own band with headroom of at least twice the bound, or
clamp at ordering time -- and put the assertion in a test, because the symptom
is a strength regression and not a wrong node count.
