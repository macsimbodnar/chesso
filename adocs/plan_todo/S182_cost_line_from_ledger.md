id:         S182
goal:       the plan's "What this costs" section prices verdicts from the project's own ledger of runs since S105, by effect class, and states the rule by which it is re-derived at every completing commit that lands a verdict
accepts:    the section states the mean and the median wall time per verdict over every SPRT run stamped in `adocs/plan_done/` since S105 landed the harness regime (2026-08-20), with the table of runs beside it -- step, wall time, games, bounds pair, and the stamp the figures are read from -- split into two classes, block-class effects and +5-class effects at DEC-063's pairs, and multiplied by the pending verdict count per class into a total in machine-hours; the 45-to-75-minute figure and the 75-to-110-hour total are struck through with the date, not deleted; the rule "re-derived in the completing commit of every step that lands a verdict" is written in "How this file works" beside the status-rewrite rule; the seven runs the review tabled appear with the stamps' own figures; `tools/plan_prose_check.py --params` and `--prose` report nothing new; nothing else in `adocs/plan.md` changes
touches:    adocs/plan.md
excludes:   changing any bounds pair, the harness or which steps owe a verdict -- a cost-cutting decision is its own entry if the owner wants one; the Elo arithmetic, which is S183
decisions:  DEC-063, DEC-136
closes:     2026-09-04_plan_review-F03
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_plan_review-F03`. `adocs/plan.md`'s "What this costs" prices a
typical verdict at 45 to 75 minutes and the pending 45 to 55 verdicts at 75
to 110 machine-hours. The runs the repository has actually recorded since the
S105 regime, each read from its step's completion stamp:

| run | wall | games | bounds |
|---|---|---|---|
| S093 verdict 1, history malus and gravity | 2 h 44 m | 6412 | 0 / 5, H1 |
| S093 verdict 2, history persistence | 6 h 35 m | 15398 | 0 / 5, H0 |
| S107, killers for checking quiets | 1 h 37 m 52 s | 3812 | -5 / 0, H1 |
| S149, killer slot dedupe | 1 h 05 m | 2522 | -5 / 5, H0 |
| S108, static evaluation at every node | 5 h 26 m 38 s | 12774 | -5 / 0, H1 |
| S165, null-move mate-band guard | 7 h 58 m 07 s | 18598 | -5 / 0, H1 |
| S130, table score as stand pat | 7 h 12 m, no verdict | 16784 | 0 / 5 |

Mean 4 h 40 m, median 5 h 26 m; two of seven inside the plan's window. At the
plan's own count that is 210 to 256 machine-hours, before the two SPSA
nights, the datagen nights and S152's two gauntlets. The plan already records
that S093 "cost more than that estimate" and leaves the total standing.

The cause is the bounds rule, not bad luck: DEC-063 says a true effect near
+3 crawls at `elo0=0 elo1=5`, and a non-regression pair runs to the wall on a
true zero. Only S109's block is priced in the +40 to +120 class the
45-minute figure needs; the rest are +5-class effects at exactly the pairs
that produced the table.

## What the rewrite says

Two classes, priced separately, and the arithmetic shown so the next reader
can redo it: block-class verdicts at the fast end, +5-class verdicts at the
ledger's mean, times the count of each in the Open list. And a standing rule,
DEC-136: the line is re-derived in the completing commit of every step that
lands a verdict, the way `status.md` is rewritten -- the ledger grows by one
row and the figure moves with it.

## Cost

Documents only. An hour.
