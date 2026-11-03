id:         S331
goal:       a self-play game in `datagen` ends on a repetition only where the laws and the match harness end it, the third occurrence, and the share of games and rows the two-fold rule relabelled since S207 is measured
accepts:    a failing test first, observed red: a game in which a position occurs a second time is not ended there by `datagen`'s game-ending test; then that test reads `classify_repetition` with the game's first position as root (a draw on the third occurrence, the search's own rule since DEC-173), green, every other `datagen` and repetition test unchanged and green; `datagen`'s closing summary gains, per run, the games that passed a two-fold before they ended and how many of those ended other than drawn, so the old rule's effect is read from any run; a measurement the coordinator runs with the step's binary at the corpus's settings (`--nodes 100000 --quiet-limit 30000 --leaf root,qs,pv`, a fresh seed, at least 2400 games): the share of games the old rule would have ended as a draw that end otherwise, and the share of root rows whose game label differs, overall and by phase, each with its error; `DEV_MANUAL.md`'s `datagen` section says what ends a game; both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green; bench unchanged (no `src/` change) or the reason stated
touches:    tools/datagen.cpp, tests/test_datagen.cpp, DEV_MANUAL.md, adocs/data/S331_*, adocs/data/README.md
excludes:   regenerating any corpus; refitting anything; the search's repetition rule (DEC-173); the leaf walk's `rules_decide`, which already follows the search
decisions:  DEC-173, DEC-171, DEC-084
closes:
blocks:     S124
paused_by:
author:
done:

## Why

Found 2026-11-03 by S124's research pass. `tools/datagen.cpp` `terminal_result` ends a game as a draw on
`is_position_repeated`, a two-fold anywhere. Since S207 (DEC-173) the search no longer reads a single pre-root
occurrence as a draw, so the engine repeats a position once when it sees no reason not to, and `datagen` then scores
the game a draw that the laws would not. On S122's corpus the tablebase reads won where the games drew: KRvK in 999 of
2130 games, KQvK 268 of 2174, KBNvK 202 of 232; a fresh search reaches a repetition within two plies in at least 235 of
the 988 drawn KRvK games (`adocs/data/S124_census.txt` once S124's research lands). A drawn label falls on every row of
the game, the middlegame's included, so the size of the effect on every corpus since S207 -- S083's and S122's among
them -- is unknown until measured. No decision ever ruled on `datagen`'s rule after DEC-173; the leaf walk's comment
names the difference without one. Classed by the coordinator: it blocks S124's fits, whose cases are exactly the endings
the rule relabels, and its measurement decides whether S122's corpus stands (DEC-171: a measurement tool's defect that
can move what a fit ships).
