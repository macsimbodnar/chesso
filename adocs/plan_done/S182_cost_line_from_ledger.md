id:         S182
goal:       the plan's "What this costs" section prices verdicts from the project's own ledger of runs since S105, by effect class, and states the rule by which it is re-derived at every completing commit that lands a verdict
accepts:    the section states the mean and the median wall time per verdict over every SPRT run stamped in `adocs/plan_done/` since S105 landed the harness regime (2026-08-20), with the table of runs beside it -- step, wall time, games, bounds pair, and the stamp the figures are read from -- split into two classes, block-class effects and +5-class effects at DEC-063's pairs, and multiplied by the pending verdict count per class into a total in machine-hours; the 45-to-75-minute figure and the 75-to-110-hour total are struck through with the date, not deleted; the rule "re-derived in the completing commit of every step that lands a verdict" is written in "How this file works" beside the status-rewrite rule; the seven runs the review tabled appear with the stamps' own figures; `tools/plan_prose_check.py --params` and `--prose` report nothing new; nothing else in `adocs/plan.md` changes except the table below; **and (2026-09-05, DEC-143)** the section and `DEV_MANUAL.md` "Which bounds" carry the bounds cost table of `adocs/testing_strategy.md` section 1.1 -- expected games at the interval's midpoint and at a bound per pair from the nElo formula, converted at the ledger's measured 2337 games an hour -- and the ledger's mean is read against it
touches:    adocs/plan.md, DEV_MANUAL.md
excludes:   changing any bounds pair, the harness or which steps owe a verdict -- a cost-cutting decision is its own entry if the owner wants one; the Elo arithmetic, which is S183
decisions:  DEC-063, DEC-136, DEC-143
closes:     2026-09-04_plan_review-F03
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **The plan prices a verdict from its own ledger of eight runs and the answer is four to nine times the figure that stood: 287 to 354 machine-hours against a struck 75 to 110.** The ledger is in `adocs/plan.md`'s "What this costs" as a table, every row read from the owning step's completion stamp and not from a run log: **S093 v1** 2 h 44 m / 6412 games / `{0,5}` / H1, **S093 v2** 6 h 35 m / 15398 / `{0,5}` / H0, **S107** 1 h 37 m 52 s / 3812 / `{-5,0}` / H1, **S149** 1 h 05 m / 2522 / `{-5,5}` / H0, **S108** 5 h 26 m 38 s / 12774 / `{-5,0}` / H1, **S165** 7 h 58 m 07 s / 18598 / `{-5,0}` / H1, **S130** 7 h 12 m / 16784 / `{0,5}` / **no verdict**, **S148** 6 h 19 m 35 s / 14808 / `{-5,0}` / H0. **Mean 4 h 52 m, median 5 h 53 m** over the eight, and 91108 games in 38.97 hours is **2337.9 games an hour** -- the seven the review tabled plus S148, which landed on 2026-09-09 after this file was written and is the reason the mean moved from the review's 4 h 40 m. **The throughput is a constant across the machine change and that is measured, not assumed**: every one of the eight sits between 2328 and 2346 games an hour, S148 on the workstation at 2341 against the seven MacBook runs, so the wall-time spread is entirely game count. **Only two of eight landed inside the struck 45-to-75-minute window and neither was a strength verdict.** **The two classes, and the split is where the truth sat rather than luck:** fast class, an effect far outside the interval -- S149 1 h 05 m, S107 1 h 38 m, S093 v1 2 h 44 m, **mean 1 h 49 m**; slow class, an effect inside the interval or a true zero -- S108, S148, S093 v2, S130, S165, **mean 6 h 42 m**, three of the five on a null or a small negative, which is exactly what DEC-063 says a `{0,5}` or `{-5,0}` pair does to a true zero. **The pending list is almost all slow class**, and the criterion is stated at the site rather than assumed: this plan prices exactly one effect in the block class (S109's +40 to +120) and two more steps carry a sourced figure above +20 at a comparable band (S024's +44.68 / +33.95 from Weiss #477, S126 against S028's measured +188.74 here). So `3 x 1 h 49 m + 42 to 52 x 6 h 42 m = 287 to 354 machine-hours`, with the arithmetic written out so the next reader can redo it. **The flat mean, 219 to 268 hours, is named as the floor and not the estimate** -- the ledger's fast runs are three of eight where the pending list's fast-class effects are three of about fifty -- and the two SPSA nights, the one to three datagen nights and S152's two gauntlets are on top of both. **The formula and the ledger both go in, with what separates them (DEC-143).** `{-5,5}` 10465 games at the midpoint and 6398 on a bound; `{0,5}` and `{-5,0}` **41861** and **25591**; `--fast`'s `{0,10}` 5828 worst case; converted at the ledger's 2337 and, in a fourth column, at **S198's measured 2277 on this workstation** -- **18.4 h and 11.2 h** for the pair every strength verdict uses. **The slow-class mean of 6 h 42 m is about 15660 games, below even the on-a-bound case, because five of the eight hit a bound rather than the midpoint**: a step that budgets on the mean and gets the midpoint waits three times as long, which is the reason DEC-143 puts the worst case in the script before the first game. The same table and the same two-number reading are now in `DEV_MANUAL.md` "Which bounds" as *What each pair costs, before the run and after it*, above the existing measured-throughput table it points at. **The standing rule is written twice, deliberately**: in the cost section itself and in "How this file works" beside the status-rewrite rule -- a step that lands a verdict re-derives this section in the same commit, the table gaining one row and the four figures moving with it (DEC-136). The figure it replaced stood for three weeks while eight runs said something else, which is the sentence the rule carries as its reason. **Struck and not deleted**, dated at the site: the 45-to-75-minute line and the 75-to-110-hour total are `~~struck through~~` with "struck 2026-09-11 by S182". **Two things beyond the accepts, both noted rather than hidden.** S159 and S203 were checked as candidate ledger rows and **neither is one**: S159's ageing scheme was implemented, counted and reverted *unrun* -- the measurement removed the reason to take a verdict -- and S203's 3000-game 1 h 18 m 10 s was a fixed-rounds drift reading, not a verdict; the 2277 figure it reports is S198's and is already in the table. And an **empty list item** that had sat above `## Done recently` since before HEAD~2 was removed, the rule's new paragraph landing where it stood; it carried nothing. Checks: `--prose` **0 flagged over 134 ids** (one flag was raised and fixed: the struck clause had merged into the "pending order" sentence, which the tense rule reads as a completed id in a pending sentence), `--citations` **0 flagged over 63 files**, `--touches` **0 flagged**, `--params` clean. Documents only, no `src/`, no `Bench:` line, no run of its own -- written while S207's SPRT held the machine. Docs: `DEV_MANUAL.md` carries the new table; `MANUAL.md` checked and carries no cost figure, so no change; `specs.md` untouched. `README.md` untouched, human-owned. Closes `2026-09-04_plan_review-F03`.

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

## Amended 2026-09-05: the cost is a formula before it is a ledger

The nElo run-length formula prices a pair before the run: a `{0,5}` or
`{-5,0}` pair costs 41861 expected games with the truth at the midpoint and
25591 with it on a bound, 17.9 h and 10.9 h at the 2337 games an hour the
seven ledger runs average (2328 to 2346); `{-5,5}` costs 10465 and 6398;
`--fast`'s `{0,10}` at alpha = beta = 0.10 costs 5828 worst case. The ledger's
4 h 40 m mean is about 10900 games, between the two `{0,5}` cases, and the
45-to-75-minute figure is what these pairs return only for effects far outside
the interval. `adocs/testing_strategy.md` section 1.1 has the derivation and
the sources; DEC-143 makes the worst case part of every pre-registration.
