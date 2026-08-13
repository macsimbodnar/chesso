id:         S052
goal:       S021 touches line points at the file where iterative deepening lives
accepts:    S021's touches names src/chesso.cpp iterative_deepening_search for the aspiration loop, with src/search.cpp only for plumbing the window through search()'s signature; it no longer disagrees with S037 about where the loop lives
touches:    adocs/plan_todo/S021_aspiration_windows.md
excludes:   implementing aspiration windows
decisions:
closes:     2026-08-13_plan_review-F09
blocks:
paused_by:
done:      S021's touches names src/chesso.cpp iterative_deepening_search (the loop, chesso.cpp:544) with src/search.cpp only for signature plumbing, agreeing with S037's location claim. Gate green.

## What is there

`S021_aspiration_windows.md` says `touches: src/search.cpp iterative
deepening`. The loop is at `src/chesso.cpp:544`
(`iterative_deepening_search`, declared `src/uci.hpp:103`); `src/search.cpp`
exposes `search(int depth, game_t*, search_state_t*)` with no window
parameters. S037's step file cites the correct location for the same loop, so
two pending step files currently disagree about where the code lives.

Full evidence: 2026-08-13_plan_review-F09.
author:    Maksym Bodnar
