id:         S287
goal:       every statement that S039 retires or re-decides the lazy shortcut and the clamp is corrected to S039's H0: both are kept
accepts:    (1) each statement S039's revert left stale is restated for the kept shortcut and clamp, traced to the code and to S039's record (`adocs/plan_done/S039_lazy_margin_redecide.md`, record `ca578de`): DEV_MANUAL.md's two paragraphs that say S039 retires the clamp ("which S039 exists to do", "S039 retires the clamp ... the question is short-lived"), specs.md's sentence "S039 retires the shortcut and the clamp together", and the code comments "S039 re-decides this" (src/evaluation.hpp, two), "S039 re-decides it there" (src/search_params.hpp), "S039 is the step that re-decides it" (tests/test_search.cpp) and "S039 retires the clamp" (tools/tuner_trace.hpp); a grep for S039 over src/, tests/, tools/ and the documents finds no other present-tense claim; (2) comments only in src/: the commit carries `No functional change` and bench stays the parent's; (3) the fast suite green in both builds, format clean, the five prose modes clean
touches:    DEV_MANUAL.md, adocs/specs.md, src/evaluation.hpp, src/search_params.hpp, tests/test_search.cpp, tools/tuner_trace.hpp
excludes:   S122's design, which waits on the owner's decision before S121 starts (S039's accepts (4)); any change to the shortcut, the clamp or LazyEvalMargin
decisions:  DEC-171, DEC-258, DEC-274
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S039 read H0 (`ca578de`) and was reverted; its agent listed, before the
revert commit, eight statements the revert made false again, each saying S039
retires or re-decides the shortcut and the clamp. Documentation and comments
only: no play, no reported score, no UCI surface, so filler under DEC-171,
named in every pre-registration while open.
