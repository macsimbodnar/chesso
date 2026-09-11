# S219 -- book comparison, read 2026-09-12

run            `20260911_204110` (`.tuning/s219_compare_20260911_204110/`)
engine         `98bf3e1` (2026-09-11)
fastchess      `alpha 1.8.1 20260720-daa3ea2`
regime         full `8+0.08` vs half `4+0.04`, `Hash=16 Threads=1`, concurrency 12, adjudication as `fastchess.sh`
rounds         750 per match, 2 counterbalanced passes, 1500 games per book per pass, 12000 games total
governor       `powersave`
preregistered  yes -- `adocs/data/S219_book_compare.sh`'s header, written before the run

Read by `adocs/data/S219_read.py`:

    python3 adocs/data/S219_read.py .tuning/s219_compare_20260911_204110

All numbers below are copied from that output, never retyped from memory.

## Cross-check

All eight matches (4 books x 2 passes) agree with fastchess's own printed
`Elo:`, `nElo:` and `Ptnml(0-2):` lines in `.tuning/s219_compare.log`, to the
printed digit. The reader's own per-match `[pentanomial agrees]` tag confirms
all eight; none disagreed. The switch below proceeds on a checked run.

## Per book, pooled over both passes

Ranked by the selection metric, largest first.

| book | balanced | pentanomial [0, ½, 1, 1½, 2] | Elo (full-half) ± 95 % | nElo ± 95 % | games/h | M ± 1 sd | draws | warnings\* |
|---|---|---|---|---|---|---|---|---|
| `noob3` | yes | [20, 111, 354, 485, 530] | +174.85 ± 11.22 | +228.27 ± 12.43 | 2957.3 | 154101610 ± 8564337 | 26.3 % | 101 |
| `uho4060v4` | no | [28, 97, 443, 474, 458] | +152.32 ± 10.58 | +202.95 ± 12.43 | 3212.4 | 132307458 ± 8270825 | 26.7 % | 355 |
| `popularpos` | yes | [28, 117, 375, 510, 470] | +157.94 ± 10.86 | +206.96 ± 12.43 | 3010.0 | 128933263 ± 7903361 | 28.6 % | 313 |
| `uho4852` (current) | no | [31, 109, 419, 478, 463] | +151.76 ± 10.76 | +198.69 ± 12.43 | 3136.8 | 123838572 ± 7907059 | 26.1 % | 519 |

\* `Warning; PV continues after threefold repetition` (the S042 defect,
DEC-187), plus a handful of `Incomplete mating PV`, summed over both passes'
`stdout.log`. Both sides of every match here are the same binary, so the
defect fires from `full` and `half` alike and cancels in this self-play
reading -- it is not read as a per-book signal, only carried for the record.

## Per pass

| book | pass | pentanomial | Elo ± 95 % | nElo ± 95 % | games/h | draws | warnings |
|---|---|---|---|---|---|---|---|
| `uho4852` | 1 | [13, 58, 220, 222, 237] | +150.51 ± 15.23 | +196.50 ± 17.58 | 3134.1 | 26.1 % | 242 |
| `uho4852` | 2 | [18, 51, 199, 256, 226] | +153.02 ± 15.21 | +200.91 ± 17.58 | 3139.5 | 26.1 % | 277 |
| `popularpos` | 1 | [13, 62, 190, 268, 217] | +151.07 ± 14.92 | +201.48 ± 17.58 | 2995.0 | 29.7 % | 221 |
| `popularpos` | 2 | [15, 55, 185, 242, 253] | +164.93 ± 15.80 | +212.56 ± 17.58 | 3025.2 | 27.5 % | 92 |
| `noob3` | 1 | [9, 54, 168, 248, 271] | +181.10 ± 15.98 | +237.64 ± 17.58 | 2988.4 | 24.9 % | 51 |
| `noob3` | 2 | [11, 57, 186, 237, 259] | +168.69 ± 15.77 | +219.30 ± 17.58 | 2926.8 | 27.7 % | 50 |
| `uho4060v4` | 1 | [18, 48, 227, 222, 235] | +149.40 ± 15.27 | +194.16 ± 17.58 | 3191.5 | 26.4 % | 166 |
| `uho4060v4` | 2 | [10, 49, 216, 252, 223] | +155.26 ± 14.64 | +212.44 ± 17.58 | 3233.5 | 27.0 % | 189 |

Forfeits: 0 of 1500 in every one of the eight matches. No match voided.

## The rule, applied

From `adocs/data/S219_book_compare.sh`'s header: the pick is the book with the
largest pooled M; two books whose M differ by less than one combined standard
error are tied; a tie breaks toward the balanced book; and nothing beating
`uho4852` (the incumbent) by more than one combined standard error keeps
`uho4852`, unless the tied candidate is balanced.

1. Largest M: `noob3` at 154101610.
2. Checked against every other book for a tie (difference in M against one
   combined standard error, `hypot` of the two `metric_se`):
   - vs `uho4060v4`: diff 21794152, combined sd 11906091 -- not tied.
   - vs `popularpos`: diff 25168347, combined sd 11653762 -- not tied.
   - vs `uho4852`: diff 30263038, combined sd 11656343 -- not tied.
3. No book ties `noob3`. It stands alone as the largest, already more than one
   combined standard error clear of the incumbent, so the "keeps the
   incumbent unless tied and balanced" clause does not apply either.

**Pick: `noob_3moves.epd`** -- the largest M outright, balanced, DEC-189.

## What the pick costs

Hours per verdict at fixed nElo bounds go as `1/M`, so hours per verdict on
the pick relative to the current book is `M_current / M_pick`:

    123838572 / 154101610 = 0.80  (1 sd: propagated relative error
                                    sqrt((7907059/123838572)^2
                                       + (8564337/154101610)^2) = 0.085,
                                    so 0.80 +/- 0.07)

A verdict on `noob_3moves.epd` costs about 0.80 +/- 0.07 of the machine-hours
a verdict on `UHO_Lichess_4852_v1.epd` costs at the same nElo bounds -- roughly
a fifth fewer hours per verdict, with the error bar wide enough that the true
saving could be anywhere from about a tenth to a quarter.
