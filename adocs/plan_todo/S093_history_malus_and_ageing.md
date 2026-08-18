id:         S093
goal:       history gets a malus for the moves that were tried and failed, ageing, and survives across go within one game
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
