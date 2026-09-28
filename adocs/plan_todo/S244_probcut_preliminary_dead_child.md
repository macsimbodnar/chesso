id:         S244
goal:       ProbCut's preliminary answers a dead-board child as the draw it is instead of entering quiescence on it, so S210's rule holds for every caller of quiescence
accepts:    a capture in ProbCut's loop that leaves insufficient material is answered `DRAW_SCORE` before `quiescence` is entered, so no material score of a dead board is computed or stored at `TT_DEPTH_QS`; the S210 comment in `quiescence` reads true again without its S113 exception; a test drives the case and is observed red without the screen; node counts move only where a dead child was entered, and the change is decided the way its reach says -- INV-6 identity if the bench stream and `search_bench` do not move, otherwise one `{-5, 0}` non-regression SPRT priced per DEC-143 -- with the reach counted on the bench positions first (DEC-107's census discharge if it applies); the fast suite green in both builds
touches:    src/search.cpp negamax_at (the ProbCut loop), tests/test_search.cpp
excludes:   every other ProbCut behaviour; the quiescence side of S210
decisions:  DEC-171, DEC-215, DEC-233
closes:
blocks:
paused_by:
done:

## Why this exists

S113's cold fast check (2026-09-28): the preliminary of ProbCut is the first
caller that enters `quiescence` at a child `negamax_at` never screened, so a
capture leaving insufficient material is scored on the material left and
stored at `TT_DEPTH_QS`, which S210's comment in `quiescence` said could not
happen. No cut is wrong -- the shallow `negamax_at` that follows returns
`DRAW_SCORE` before the cut test, and `negamax_at` answers a dead node before
it probes the table -- so the cost is one wasted preliminary and a stale
entry no main-search node reads. Recorded as open finding 6 of
`adocs/data/S113_sprt.sh`, carried by S131's block, and scheduled here under
DEC-171 as a filler behind the strength steps. It is cheap to write and dear
to measure: a screen in the loop moves node counts wherever a dead child was
entered, so the accepts asks for the reach first -- over the bench positions
at depth 12 and 14 the census says how often the case occurs -- and for the
cheapest proof the reach allows. If a later ProbCut change owes an SPRT
anyway, this folds into it and the step closes on that verdict.
