id:         S331
goal:       a self-play game in `datagen` ends on a repetition only where the laws and the match harness end it, the third occurrence, and the share of games and rows the two-fold rule relabelled since S207 is measured
accepts:    a failing test first, observed red: a game in which a position occurs a second time is not ended there by `datagen`'s game-ending test; then that test reads `classify_repetition` with every earlier entry treated as before the root (`history.size`), so only a third occurrence draws (the search's own rule for a position from before its root since DEC-173; corrected by the coordinator 2026-11-03 from "with the game's first position as root", which as `root_history_size = 0` is the two-fold rule again), green, every other `datagen` and repetition test unchanged and green; `datagen`'s closing summary gains, per run, the games that passed a two-fold before they ended and how many of those ended other than drawn, so the old rule's effect is read from any run; a measurement the coordinator runs with the step's binary at the corpus's settings (`--nodes 100000 --quiet-limit 30000 --leaf root,qs,pv`, a fresh seed, at least 2400 games): the share of games the old rule would have ended as a draw that end otherwise, and the share of root rows whose game label differs, overall and by phase, each with its error; `DEV_MANUAL.md`'s `datagen` section says what ends a game; both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green; bench unchanged (no `src/` change) or the reason stated
touches:    tools/datagen.cpp, tools/datagen_rules.hpp, tests/test_datagen.cpp, tests/test_datagen_repetition.sh, tests/test_s331_measure.sh, tests/CMakeLists.txt, DEV_MANUAL.md, adocs/data/S331_*, adocs/data/README.md; comments only in src/bitboard.hpp and src/leaf_walk.cpp (widened by the implementer, below)
excludes:   regenerating any corpus; refitting anything; the search's repetition rule (DEC-173); the leaf walk's `rules_decide`, which already follows the search
decisions:  DEC-173, DEC-171, DEC-084
closes:
blocks:     S124
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause. Red first, observed: `tests/test_datagen.cpp`'s second-occurrence case read `CHECK_LT( 0.5, 0 )` on the base rule moved verbatim into `tools/datagen_rules.hpp` (the move byte-neutral on a 4-game run); the fix reads `classify_repetition(..., history.size) == DRAW`, every earlier entry before the root, a third occurrence; green, 39 assertions; the third-occurrence case green before and after; the root-0 form and the two-fold rule planted and killed, the root-0 output quoted from the delivered file. Every other datagen and repetition test unchanged and green. The closing `repetition:` line gives the games that recurred after their opening, how many ended other than drawn (won and lost by White), their root rows and those before the first recurrence, and openings ended on one apart; existing lines unchanged in form, every log reader checked; `--games-out` added for the reading, `tests/test_datagen_repetition.sh` holds the line to the files. The measurement, the coordinator's run of `adocs/data/S331_measure.sh` at the corpus's settings, 2400 games, seed 2026400331, 1160 s: 1297 games recurred, 547 ended other than drawn, 22.79 % +/- 0.86 of all games relabelled from a draw, 19.74 % +/- 0.82 of the old rule's root rows relabelled (12.21 to win, 7.53 to loss), by phase 13.98 to 22.47 % (`S331_reading.txt`, the table above); its premise held on the 24-game smoke (`S331_smoke.txt`). The wrapper's first pass failed after the complete run because I rewrote the script in place mid-run, reproduced exactly; the body is one brace group since, `tests/test_s331_measure.sh` red on the first delivery and green now, and the finished run read again without replaying, `S331-MEASURE-DONE`, its files unchanged. The fast check's four taken in. `DEV_MANUAL.md` says what ends a game, history kept (DEC-348); `MANUAL.md` checked, unchanged. Comments made false corrected in `src/bitboard.hpp`, `src/leaf_walk.cpp` and `tools/datagen.cpp`, `src/` objects byte-equal (`S331_identity.txt`), bench 5482277 in both builds. Run here, `CLANG_FORMAT_MAJOR=22`, nice 10, the machine quiet: `build` and `build-tune` fast 57 of 57 each; format and `tools/plan_prose_check.py` rc 0. Earlier, under S122's screen, `test_tuner_gradient_plants_threats` timed out once at 60.15 s and passed alone in 62.12 s, load. `touches:` widened by the implementer for the header, the two shell tests, `tests/CMakeLists.txt` and the two `src/` comments. Filler by the letter of DEC-171, scheduled ahead of S124's fits by the coordinator.

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

## 2026-11-03: the third occurrence, and the reading of the second (Opus 5.5 subagent, briefed by the coordinator)

**The red test, a unit test and not a fixture game.** `terminal_result()` and `has_legal_move()` moved verbatim from
`tools/datagen.cpp`'s anonymous namespace into `tools/datagen_rules.hpp`, so `tests/test_datagen.cpp` drives the
function the games are played by. The move alone is behaviour-neutral: the base datagen and the moved one wrote
byte-equal `root`, `qs` and `pv` files and the same log over 4 games at 2000 nodes, seed 7, 2 threads. A fixture game was
the other road and was refused: `datagen` has no start-position option, so a game reaching a two-fold would be a seed
chosen for it, a golden that moves with every engine change (DEC-142) and pins nothing the unit test does not.

Red on the base rule, as moved, in the case `a position occurring a second time does not end the game`:

```
TEST CASE:  a position occurring a second time does not end the game
ERROR: CHECK_LT( terminal_result(&game), 0.0 ) is NOT correct!
  values: CHECK_LT( 0.5, 0 )
[doctest] test cases:  2 |  1 passed | 1 failed | 0 skipped
```

Its precondition is read from the engine: the position's hash equal to its first occurrence's,
`classify_repetition(..., history.size)` answering `ONCE_PRE_ROOT`, and `is_position_repeated()`, the old rule, true.
The second case, the third occurrence ending the game 0.5 with `classify_repetition` answering `DRAW`, was green before
the fix and after.

**The coordinator's correction, taken in.** The accepts named "the game's first position as root". As
`root_history_size = 0` that draws any match above entry 0, the two-fold rule again but at the start. The fix was written
with `history.size` from the first, and the red test was then found not to kill the root-0 form: its repeated position
was the start position, entry 0, which `index > 0` does not reach. A subcase was added whose first occurrence is entry 2,
the position after 1.e4 e5. Planted, `classify_repetition(..., 0)`, re-run on the delivered file (the first quote named
line 85, the line before `clang-format` moved it):

```
TEST CASE:  a position occurring a second time does not end the game
DEEPEST SUBCASE STACK REACHED (DIFFERENT FROM THE CURRENT ONE):
  the position after 1.e4 e5
tests/test_datagen.cpp:83: ERROR: CHECK_LT( terminal_result(&game), 0.0 ) is NOT correct!
  values: CHECK_LT( 0.5, 0 )
[doctest] assertions: 39 | 38 passed | 1 failed |
```

and the base rule planted again over the two subcases fails both (`37 passed | 2 failed`). Restored, 39 of 39.

**The fix.** `terminal_result(game, bool* recurred = nullptr)`: the fifty-move rule and insufficient material first,
as before; then `classify_repetition(&game->history, &game->board, game->history.size)`, every earlier entry before the
root, a draw only on `DRAW`, a third occurrence; then mate and stalemate. `recurred` reports `!= NONE`, which is
where the old rule ended the game, and stays false where the fifty-move rule or material decided first, as they did
before the repetition test then too. The opening's own check reads the same function, so an opening whose last
position occurred once before is kept now, and counted apart.

**The summary line and the games file.** `datagen` prints, after `walk:`, one line no reader of its log matches
(`S082_recipe.sh`, `S082_pool_merge.py` -- its `CLOSING`, `DATAGEN` and `COVERAGE_ROW` patterns -- `S083_corpus.py`,
`S122_corpus.sh`, `S122_screen.sh`, `test_datagen_exit.sh`, all anchored on other prefixes); the lines before it are
unchanged in form. `--games-out FILE` writes one line a game: game, result, first ply of play, ply of the first
recurrence or -1, last ply. The rows are not changed in form, only by the rule. The reading needs the file: which rows
fall before a game's first recurrence is not in the rows, since the recurring position need not be a row (in check,
or a mate found).
`tests/test_datagen_repetition.sh` (fast label, six games at 1000 nodes): the line's figures equal the games file's and
the root file's, every row of `root` and `qs` carries its game's result, and games go on past their first recurrence;
with the old rule planted it fails (`no row after a first recurrence`), and the doctest beside it.

**The smoke**, `S331_measure.sh premise` (`adocs/data/S331_smoke.txt`): 24 games at 10000 nodes, 2 threads, nice 10,
seed 2026410331, once by the base datagen and once by the step's. The new run's line:

```
repetition: 14 games reached a position that had occurred before, which drew a game until S331; 7 of them ended other than drawn, 4 won by White and 3 lost; 2447 positions passed in them, a root row each, 1293 before the first recurrence; 0 openings ended on one, discarded until S331
```

and the premise the reading rests on holds on all three leaves: the old run's 2211 root, 2211 qs and 2198 pv rows are
each the new run's row before its game's first recurrence, field for field but the label, which is 0.5 where the game
recurred and the game's own elsewhere. A smoke at a tenth of the corpus's budget, not a reading of it.

**The measurement, the coordinator's.** `adocs/data/S331_measure.sh` with no argument, from the main tree, `build/` at
the step: 2400 games, `--leaf root,qs,pv --nodes 100000 --quiet-limit 30000 --threads 12`, seed 2026400331, into
`.tuning/s331/measure`, read into `reading.txt`; last line `S331-MEASURE-DONE` or `-FAILED`. About 20 minutes on the
machine alone, its run having taken 1160 s. The estimate before it was 25: S122's two parts ran about 9136 and 8858
games an hour at these settings, 16 minutes for 2400 games under the old rule, and the smoke's games under the new rule
cost 1.54 times the old rule's search nodes (37691661 against 24500000) and wrote 1.52 times its root rows (3365 against
2211). A watcher's ceiling 1 h.
The seed range [2026400331, 2026402731) and worker range [2026400331, 2026400343) meet none of the 14 runs named in the
logs under `.tuning/` and `adocs/data/` on 2026-11-03, the nearest S122's top-up [2026300203, 2026347791); the script
checks again before it starts. Its reading: the games the old rule drew and how many ended 1 or 0, binomial errors;
the root rows whose label changed, overall, by the coverage table's five phase bands and the census's cut at phase 6
(the census has no other phase bands), to win and to loss by White, errors clustered by game; and the new rows the old
rule never wrote.

**The measurement, run by the coordinator 2026-11-03** (`adocs/data/S331_reading.txt`):
`adocs/data/S331_measure.sh` (run), datagen `730f827a…` (the step's Release binary), `--leaf root,qs,pv --games 2400
--nodes 100000 --threads 12 --seed 2026400331 --quiet-limit 30000 --games-out games.tsv`, the machine to itself, 22:14:21,
1160 s, 7448 games an hour. Its line:

```
repetition: 1297 games reached a position that had occurred before, which drew a game until S331; 547 of them ended other than drawn, 344 won by White and 203 lost; 187472 positions passed in them, a root row each, 109667 before the first recurrence; 0 openings ended on one, discarded until S331
```

Games, binomial errors: 1297 of 2400 recurred after their opening, 54.04 % +/- 1.02; of those 547 ended other than
drawn, 42.17 % +/- 1.37, White winning 344 (26.52 % +/- 1.23) and losing 203 (15.65 % +/- 1.01); so 22.79 % +/- 0.86 of
all games the old rule drew end 1 or 0, and the draws fall from 1335 (55.62 % +/- 1.01) to 788 (32.83 % +/- 0.96). No
opening ended on a recurrence. Root rows the old rule wrote, and those whose label it changed, errors clustered by game:

```
                     old   differs   share               to win   share               to loss   share
all phases        222839     43988   19.740 % +/- 0.820   27205   12.208 % +/- 0.667   16783    7.531 % +/- 0.543
phase 0 to 4       29308      4098   13.983 % +/- 1.311    2370    8.087 % +/- 1.008    1728    5.896 % +/- 0.918
phase 5 to 9       50636      8309   16.409 % +/- 1.034    5286   10.439 % +/- 0.856    3023    5.970 % +/- 0.643
phase 10 to 14     33125      6934   20.933 % +/- 1.181    4221   12.743 % +/- 0.946    2713    8.190 % +/- 0.824
phase 15 to 19     25290      5668   22.412 % +/- 1.289    3521   13.922 % +/- 1.081    2147    8.490 % +/- 0.850
phase 20 to 24     84480     18979   22.466 % +/- 0.911   11807   13.976 % +/- 0.758    7172    8.490 % +/- 0.604
phase <= 6         56351      8244   14.630 % +/- 1.051    5103    9.056 % +/- 0.852    3141    5.574 % +/- 0.683
phase > 6         166488     35744   21.469 % +/- 0.861   22102   13.275 % +/- 0.707   13642    8.194 % +/- 0.577
```

Of the 43988 rows that differ, the new label goes the way of the row's own score in 36542, against it in 6829, score 0
in 617. The new rule also writes 77805 rows the old one never did, after a first recurrence: 25.879 % +/- 0.646 of the
300644 root rows. Every figure is the engine's own play, its labels and scores; none is a judgement of a position.

**The wrapper failed once, after a complete run, and the cause was mine.** The first pass printed `line 128: s-out:
command not found`, `datagen failed:` and `S331-MEASURE-FAILED` after datagen had exited 0 and written `RUN-DONE`. The
coordinator ran the script from this worktree, and at 22:28 I rewrote its header in place, for the fast check's run
time, while it played. Bash reads a script as it goes, so after `play` returned it resumed at an offset the edit had
moved, inside `--games-out`. Nothing in `play()` mangles an argument. Replaying that same edit on the first-delivery
script over a stub datagen reproduced the three lines exactly. The body is one brace group since, read whole before
it runs, and every path out of it exits inside it. `tests/test_s331_measure.sh` (fast label, a stub datagen) holds
that the arguments reach datagen intact, that a finished run is read again and not replayed, that an unfinished run
and an overlapping seed range are refused, and that an edit mid-run does not reach the run. That last case was red on
the first-delivery script (`unexpected EOF while looking for matching`, then `S331-MEASURE-FAILED`); the first four pass
on it, which is the evidence `play()` was never the defect. Against the real run, `LOG_DIRS=... OUT=.tuning/s331/measure
adocs/data/S331_measure.sh run` from this worktree printed `finished before, read again` and `S331-MEASURE-DONE`, and
every file of the run hashed the same before and after, `reading.txt` included.

**The fast check's four, taken in.** `tests/test_datagen_repetition.sh` builds its expected game list as the left side
does, `seq | tr`, since BSD `seq -s` ends on the separator; its assertion that a game ending on its first recurrence's
ply ended drawn is gone, said why in its header (that ply is still searched and adjudicated, so a resignation scores 1
or 0 there with nothing wrong); the run time is the run's; the root-0 plant is quoted from the delivered file above.

**Comments made false by the rule, corrected with their history (DEC-348).** `src/leaf_walk.cpp` above `rules_decide`
("That is the difference from datagen's game-ending test, is_position_repeated(), which adjudicates a game on any
two-fold") and `src/bitboard.hpp` above `is_position_repeated` ("what draw adjudication in self-play wants"): both
now say what was so until S331 and what datagen does since; `rules_decide`'s code untouched, as `excludes:` asks. And
`tools/datagen.cpp`'s zero-hash comment, "a draw two plies into every game", four since. Comments only in `src/`:
the two files' bytes under `g++ -fpreprocessed -dD -E -P` equal before and after, and all 11 objects of `chesso_engine`
and `chesso` recompiled with ccache off byte-equal in `build/` and `build-tune/` (`adocs/data/S331_identity.txt`); bench
5482277 in both builds.

**Not done here.** `adocs/specs.md` does not state datagen's ending rule, so it had nothing to update; the coordinator's.
