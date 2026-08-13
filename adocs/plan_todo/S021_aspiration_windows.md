id:         S021
goal:       start the root search in a narrow window around the previous score
accepts:    an SPRT with bounds matched to the expected effect size returns a verdict
touches:    src/chesso.cpp iterative_deepening_search, where the aspiration loop lives; src/search.cpp only to plumb the window through search()'s signature
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Expected

Small. One engine reported +9 with an error bar of +/-17, which is a reported
figure and therefore direction only (DEC-019). `elo0=0 elo1=10` cannot resolve
an effect this size -- use `elo0=-5 elo1=5` or similar or the run random-walks,
as S006's did for 340 games.
