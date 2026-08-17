id:         S021
goal:       start the root search in a narrow window around the previous score
accepts:    an SPRT with bounds matched to the expected effect size returns a verdict; and, checked before that verdict is read, the fast suite's three mate cases -- "mate in one", "mate in two is found at the right distance" and "pruning does not hide a mate against the material leader" -- are green at the window schedule that ships
touches:    src/chesso.cpp iterative_deepening_search, where the aspiration loop lives; src/search.cpp only to plumb the window through search()'s signature
excludes:
decisions:  DEC-063
closes:
blocks:
paused_by:
done:

## The mate clause, and why this step in particular

The three cases are `TEST_CASE_FIXTURE`s in `tests/test_search.cpp`, all under
the `fast` label that `add_doctest_target` sets (`tests/CMakeLists.txt`), so
`ctest -L fast` runs them. Follow them by title, not by line: "mate in one" and
"mate in two is found at the right distance" are in the `search: mate
detection` suite, and "pruning does not hide a mate against the material
leader" is the position S033 built for reverse futility.

DEC-060 measured that a pruning rule's mate exposure is a property of **the
bound the parent passes down**, not of the static score: the harmful prune fired
at `alpha=-965 beta=-964` because the parent was a null-window scout hunting a
mate score, so the mate-band guard on `beta` did nothing. Aspiration windows
change exactly that quantity -- the bounds every node below the root inherits.
Whether narrowing the root window moves any node into that shape is unmeasured.
The expected effect here is +9 +/- 17, a size at which an SPRT cannot separate a
missed mate from noise, so the suite has to say it rather than the match.
S074, DEC-060.

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
