id:         S231
goal:       a two-ply continuation history table -- keyed on the move two plies back and this move -- beside S222's one-ply table, on its own scale, fitted in a narrow SPSA lane and decided by one gainer SPRT, because S222's H1 (DEC-210) is what the two-ply table was waiting on
accepts:    `cont_hist2[12][64][12][64]` (or the shape the implementing agent states) on `search_state_t`, keyed on the (piece, to) of the move two plies back and this move's, written at every quiet cutoff beside the one-ply table and summed into the quiet ordering score with its own `ContHist2Weight`, `ContHist2Bonus` and `ContHist2Malus` declared in `src/search_params.hpp` the way S222's are (the bound a definition, three axes -- DEC-209's parameterisation); the guard on the move two plies back existing (ply 0, ply 1 and the two nodes after a null move pass none), each guard with a sentinel case and a mutant `tools/mutation_check.py` kills; the band-clearance case re-stated for both weights at their declared maxima; **the fit first**: one narrow SPSA lane over the three new axes together with S222's three, on `UHO_4060_v3.epd`, pre-registered in its own script's header with the estimate from the measured throughput, its result recorded whatever it is; **then one gainer SPRT `{0, 5}`** nElo at the harness regime against the commit before the step's first landing -- DEC-210's reading: one vector under one verdict, H0 reverts the whole step -- pre-registered per DEC-143 with the fitted values named; `bench` line; Debug self-play; `adocs/data/S024_census_run.py` re-run with the second table counted; H1 keeps it, H0 records the zero and the two-ply idea leaves the plan with a decision saying why
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp, src/search_params.hpp, tests/, tools/mutants/, tools/, adocs/data/
excludes:   a third ply; any change to S222's table or values except through the shared lane; S098's reading of the sum (S098 owns it); any constant from another engine (DEC-084, DEC-105)
decisions:  DEC-210, DEC-209, DEC-194, DEC-198, DEC-143, DEC-141, DEC-084, DEC-105
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S222's accepts: "H1 keeps it and opens the two-ply table as its own follow-up
step". Its SPRT read H1 on 2026-09-15 -- `Elo 11.13 +/- 6.90` over 6278
games against the tree before S222, one vector under one verdict (DEC-210).
The published record treats the two-ply table as the second half of the same
idea; whether it transfers here is the measurement and not the record
(DEC-019). Placed behind S098, as S222's pre-registration required: S098
scales its reduction by the sum these tables make and is measured against the
tree S222 left, and a second table landing first would move that baseline
under it.

## Cost

Agent work, about a day for the table, its guards, tests and mutants; a lane
night of about nine hours at S085's regime (`.moltke.local.md`'s 24 to 25 s an
iteration); the SPRT up to twenty hours worst case (41861 games at 2150 an
hour). Memory: a second 1.125 MiB table on `search_state_t`, heap-owned since
S222's fast check moved the struct off the thread stack.
