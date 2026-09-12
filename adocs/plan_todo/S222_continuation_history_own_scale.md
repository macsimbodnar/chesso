id:         S222
goal:       one-ply continuation history retried with a scale of its own -- its bonus, malus and bound as tune-build parameters and the weight of the continuation term against plain history, fitted in S127's SPSA lane before the SPRT -- because S024's table, on plain history's scale, measured H0 (DEC-194)
accepts:    the one-ply table of S024 verdict 1 (`cace216`'s shape: `cont_hist[12][64][12][64]`, one index helper, the guard on a previous move existing, the sentinel cases and the two mutants) rebuilt with its own `CONT_HIST_BONUS`/`CONT_HIST_MALUS`/`ContHistMax` and a `ContHistWeight` (or an equivalent parameterisation stated in the file) declared in `src/search_params.hpp` the way existing tunables are, so the tune build can move them; the two hypotheses DEC-194 wrote are tested by the fit rather than argued -- that summing two equal-weight terms doubled plain history's share of the quiet band, and that the shared gravity bound clipped the table; **the fit runs first**: one S127-lane SPSA pass over the new parameters (and `QuietHistoryMax`) at the S127 regime, pre-registered in its own script's header, its result recorded whatever it is; **then one gainer SPRT** `{0, 5}` nElo at the harness regime against the commit before it, pre-registered per DEC-143 with the fitted values named; the band-clearance case re-stated for the new bound; `bench` line; Debug self-play; the census driver `adocs/data/S024_census_run.py` re-run on the fitted build and its shares recorded beside S024's (97.6 / 96.2 / 27.1 %); H1 keeps it and opens the two-ply table as its own follow-up step; H0 records the zero and the technique leaves the plan with a decision saying why
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp, src/search_params.hpp, tests/, adocs/data/
excludes:   the two-ply table (its own step after this one passes); any constant taken from another engine (DEC-084, DEC-105); reusing S024's unfitted result as evidence for or against the fitted one
decisions:  DEC-194, DEC-019, DEC-084, DEC-105, DEC-143, DEC-141
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S024 verdict 1 built the one-ply table on plain history's own formula and
bound, with no constant of its own, and the gainer SPRT accepted H0 at
-5.48 +/- 7.20 nElo over 8954 games while the census showed the table
exercised in 96 % of quiet reads (DEC-194). So the technique as the published
record describes it did not transfer on this engine at that scale, and the
plan's rule for a technique whose value the record puts between +2 and +45
Elo is to fit before concluding (DEC-019, DEC-084): the fit is S127's lane,
and this step waits behind it rather than guessing.

## Cost

One SPSA pass (a night, S127's regime) and one gainer SPRT (2.5 h if the
effect is real, 19.8 h at the wall), plus the rebuild from `cace216`'s shape,
which `git show cace216` holds in full.
