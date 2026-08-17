id:         S021
goal:       start the root search in a narrow window around the previous score
accepts:    an SPRT with bounds matched to the expected effect size returns a verdict
touches:    src/chesso.cpp iterative_deepening_search, where the aspiration loop lives; src/search.cpp only to plumb the window through search()'s signature
excludes:
decisions:  DEC-063
closes:
blocks:
paused_by:
done:

## Expected

Small. One engine reported +9 with an error bar of +/-17, which is a reported
figure and therefore direction only (DEC-019). `elo0=0 elo1=10` cannot resolve
an effect this size -- use `elo0=-5 elo1=5` or similar or the run random-walks,
as S006's did for 340 games.

**2026-08-17: the paragraph above was right and S068 paid for not reading it.**
S068 ran `fastchess.sh`'s default `elo0=0 elo1=5` on an effect near +3 Elo and
got no verdict in 6 h 36 m and 9036 games; `elo0=-5 elo1=5` returned one in
1 h 41 m on 2312 games, same two binaries. The prescription is now **DEC-063**,
where every step can find it, and it binds this one: `./fastchess.sh` with its
default bounds will not do. Write the invocation, state the expected effect and
the reading of each outcome before launching, as
`adocs/data/S068_sprt_run2.sh` does.
