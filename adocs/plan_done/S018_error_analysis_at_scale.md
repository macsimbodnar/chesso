id:         S018
goal:       rank chesso's own errors by game phase over hundreds of games, from Stockfish
accepts:    a script that takes a PGN of many games, runs the S016 pipeline over all of them, and reports total centipawns lost bucketed by game phase and by error size; run against the fixed mailbox build in ~/.local/bin and the numbers recorded here
touches:    tools/
excludes:   fixing anything it finds -- that is S019 and after
decisions:  DEC-029, DEC-030, DEC-031
closes:
blocks:
paused_by:
done:      2026-08-10. 210 games achesso vs sgambetto at 10+0.2 with loose adjudication; 13522 profiled moves over 27124 positions scored by Stockfish dev-20260803-762dd1da at 3000000 nodes, median depth 24, 8.8 h on an Apple M1 MacBookPro17,1. Verdict: early middlegame costs most at 44.1 cp/move and 36.0 percent of 407740 cp; endgame is the cheapest phase per move at 18.3 outside pawn endgames. Ranking stable after removing 1347 mate-touching moves and again after removing clamped reference scores. Evaluation bias positive in every phase, worst in the late middlegame at +100.2 mean. This contradicts the single-game anecdote that produced S019, which S019's own text told us to check. A defect in the profiler was found by reading the first report and fixed before any number was recorded: mated positions print no pv, so every delivered checkmate was charged 1000 cp, 20 moves and 20106 cp, and the corrected total is exactly 17000 lower. Machine, compiler, release flags, binary checksums and every match setting are recorded in the step file. DEC-029 opponent, DEC-030 node limits, DEC-031 adjudication. MANUAL.md checked, no change needed, nothing here reaches the UCI surface. DEV_MANUAL.md checked and updated in f371182. README.md checked, owner-written. Gate green: build, fast suite 7/7, clang-format check clean.

## Why this comes before any evaluation term

Every published Elo figure this project has borrowed has failed to transfer:
staged generation 30-50 predicted and 0 measured, SEE quiescence pruning 0,
move ordering reported around 150 and measured slower. See DEC-019. The
remaining way to choose the next evaluation term is to measure where this
engine actually loses centipawns, not where other engines report gains.

One game has already been analysed this way and produced S019. One game is an
anecdote; this step turns it into a distribution.

## What was run

The opponent is **sgambetto**, not the mailbox build this file's `accepts` names.
The mailbox build lost 6-0 in a smoke match, so its games profile an already-won
position rather than a fought one; sgambetto is close enough to give real games.
DEC-029 records the deviation and the options rejected.

| | |
|---|---|
| machine | Apple M1, `MacBookPro17,1`, 4 performance + 4 efficiency cores, 8 GB |
| system | macOS 14.8.5, build 23J423 |
| compiler | Apple clang 16.0.0 (clang-1600.0.26.6), target `arm64-apple-darwin23.6.0` |
| release flags | `-O3 -DNDEBUG -std=gnu++20 -arch arm64 -Wall -Wextra -Werror` |
| engine sources | as at `d31432a`; match binary `c7ea34b5d231`, snapshotted before the first game |
| opponent | `~/.local/bin/sgambetto`, "Sgambetto 0.1" |
| reference | `~/.local/bin/stockfish`, `dev-20260803-762dd1da`, Threads 1, default Hash |
| reference limit | 3000000 nodes per position, median depth 24 (DEC-030) |
| time control | 10+0.2, `option.Hash=16 option.Threads=1`, concurrency 3 |
| book | `books/8moves_v3.pgn`, `order=random`, book plies excluded from the profile |
| adjudication | `-draw movenumber=80 movecount=10 score=5 -resign movecount=8 score=900 -maxmoves 200` (DEC-031) |
| analysis workers | 6 |

Match: 210 games, `achesso +61 =43 -106`, 39.3 %. Median 133 plies, longest 380,
142 adjudications and 68 normal terminations. 400 games were asked for; the
match was killed by the harness at 210, the PGN was intact and all 210 parse,
and 210 satisfies "hundreds of games".

Analysis: 13522 profiled moves, 27124 positions scored, 8.8 h wall.

## The defect found while reading the first report

The first run reported 424740 cp and put `Ra7#`, `Qg4#` and `Qxh6#` -- checkmates
chesso delivered -- at the top of the most-expensive-moves table at 1000 cp each.

A mated position prints `info depth 0 score mate 0` and `bestmove (none)`, with
no `pv` field. `Engine.evaluate` required `" pv "` before accepting a scoring
line, so it fell through to its default of 0, and a delivered mate cost
`1000 + 0` instead of `1000 + (-1000) = 0`. The whole `>=1000` band was the
artefact: 20 moves, 20106 cp, 4.7 % of the total.

Fixed by keeping the last line carrying a score and preferring one with a `pv`
when there is one, so a normal search is unaffected. 32 of 210 games end in a
terminal position and 17 of those on a profiled move; those 17 were re-analysed
through `--resume`. The corrected total is 407740 cp, **exactly 17000 lower**,
which is 17 games times the 1000 cp each was wrongly charged and confirms the
repair changed that and nothing else.

## Centipawns lost by game phase

Phase is the engine's own `game_phase()`, emitted by `pgn_to_positions`, so it
names the quantity the tapered evaluation tapers on. 24 is a full board, 0 is
kings and pawns.

| phase | moves | cp lost | cp/move | share |
|---|---|---|---|---|
| opening (22-24) | 1934 | 77103 | 39.9 | 18.9 % |
| early middlegame (14-21) | 3323 | 146630 | **44.1** | **36.0 %** |
| late middlegame (7-13) | 3493 | 99050 | 28.4 | 24.3 % |
| endgame (1-6) | 4570 | 83579 | 18.3 | 20.5 % |
| pawn endgame (0) | 202 | 1378 | 6.8 | 0.3 % |

## Centipawns lost by error size

| error | moves | cp lost | cp/move | share |
|---|---|---|---|---|
| noise <25 | 9850 | 31434 | 3.2 | 7.7 % |
| 25-49 | 1285 | 46126 | 35.9 | 11.3 % |
| 50-99 | 1152 | 81762 | 71.0 | 20.1 % |
| 100-199 | 794 | 110065 | 138.6 | 27.0 % |
| 200-399 | 369 | 100663 | 272.8 | 24.7 % |
| 400-999 | 69 | 34584 | 501.2 | 8.5 % |
| >=1000 | 3 | 3106 | 1035.3 | 0.8 % |

Half the loss is in moves costing 100 to 400. Blunders above 400 are 0.5 % of
moves and 9.3 % of the loss; moves under 25 cp are 73 % of moves and 7.7 % of
the loss.

## Evaluation bias

The engine's own score, which fastchess writes into each move comment, against
the reference's score for the same position. Positive means chesso thought it
was doing better than it was. Mate scores excluded.

| phase | moves | mean | median | p90 |
|---|---|---|---|---|
| opening | 1928 | 39.2 | 21 | 209 |
| early middlegame | 3201 | 77.4 | 49 | 407 |
| late middlegame | 3128 | **100.2** | 55 | 392 |
| endgame | 3744 | 41.8 | 22 | 357 |
| pawn endgame | 174 | 29.5 | 0 | 180 |

Chesso is optimistic in every phase. This is a different quantity from move
cost: bias is how wrong the score is, cost is what the move gave away.

## The ranking is not an artefact of the clamp

Scores are clamped to +/-1000 before costing, and 1347 moves touched a mate
score. Removing them, and then also removing every move whose reference score was
itself at the clamp, does not move the ranking:

| phase | cp/move, all | no mate-touching | no clamped |
|---|---|---|---|
| opening | 39.9 | 40.0 | 40.0 |
| early middlegame | 44.1 | 45.4 | 46.1 |
| late middlegame | 28.4 | 31.0 | 31.9 |
| endgame | 18.3 | 21.1 | 22.6 |
| pawn endgame | 6.8 | 7.9 | 9.8 |

## What this says about S019

S019 is titled "evaluation terms for the phase the error analysis says costs
most" and its own text says: *"Confirm against S018 before writing any of them.
One game says endgame; the distribution may say otherwise, and this step's
content changes if it does."*

**The distribution says otherwise.** By cost per move and by share of total loss
the early middlegame is the largest pot, and the endgame is the cheapest phase
per move other than pawn endgames. By evaluation bias the worst phase is the
late middlegame. On neither measure is the endgame first.

The S016 anecdote is not contradicted as a fact -- it measured bias in one king
and pawn endgame, and the endgame p90 bias of +357 says such positions exist --
but it does not generalise to where the centipawns go.

What S019 becomes is the owner's decision, not this step's. This step excludes
fixing anything it finds.

## Caveats on the record

- **One opponent.** The distribution has been measured against sgambetto only.
  DEC-019 is three cases of a figure that failed to transfer; a second opponent
  is the check that this one does. Nodes-limited Stockfish is the candidate and
  is noted in DEC-029.
- **210 games, not the 400 asked for**, because the match was killed.
- **Cost attribution charges the mover for the whole swing**, including a swing
  caused by the opponent's previous move being met imperfectly. It is the S016
  definition, unchanged.
- Raw per-move records are kept, so any of this can be re-bucketed with
  `--from-raw` without an engine and without repeating the run.
