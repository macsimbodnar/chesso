# S219 -- A/A calibration on the new book, read 2026-09-12

run            `20260912_005458` (`.tuning/s219_aa_20260912_005458/`)
engine         `5047070` (2026-09-12), both sides the same sha (A/A)
fastchess      `alpha 1.8.1 20260720-daa3ea2` (`fastchess --version`; the run
               log itself prints no version banner)
regime         `8+0.08`, `Hash=16`, concurrency 12, book `books/noob_3moves.epd`,
               adjudication as `fastchess.sh`
rounds         500 (1000 games), fixed rounds -- a calibration, not a verdict
governor       `powersave` (S198's calibration ran under `performance`)
preregistered  yes -- S219's step file, "A/A pre-registration, 2026-09-12
               00:55", written before the run started

Read by `adocs/data/S198_pairs.py`, which reads this run under engine name
`candidate` and checks its variance against `S105_calibration_after.pgn`'s
band:

    python3 adocs/data/S198_pairs.py adocs/data/S219_aa_calibration.pgn > adocs/data/S219_aa_calibration_pairs.txt

All numbers below are copied from that output and from
`adocs/data/S219_aa_calibration.log`, never retyped from memory.

## Cross-check

fastchess's own final summary (`S219_aa_calibration.log`): `Games: 1000,
Wins: 322, Losses: 325, Draws: 353`, `Ptnml(0-2): [54, 91, 198, 118, 39]`,
`Total Time: 00:28:26`, forfeits 0 of 1000. `S198_pairs.py`'s pentanomial
over the 500 complete pairs -- `[54, 91, 198, 118, 39]` -- agrees with
fastchess's printed `Ptnml(0-2)` on every bucket. 328 fastchess warnings "PV
continues after threefold repetition" in the log (S042, DEC-187), firing from
both sides equally in this self-play; recorded, not read as a book signal,
the same convention `S219_book_compare.md` uses.

## This run beside S198

| | this run, `noob_3moves.epd` | S198, `UHO_Lichess_4852_v1.epd` |
|---|---|---|
| governor | `powersave` | `performance` |
| pair score variance | 0.2905 +/- 0.0184 | 0.2430 +/- 0.0154 |
| games an hour | 2110.2 | 2277.0 |
| draw rate | 353/1000, 35.3 % | 323/1000, 32.3 % |
| white won both (opening-decided) | 65/500, 13.0 % | 88/500, 17.6 % |
| pair score 1.0 (1:1 pairs) | 198/500, 39.6 % | 229/500, 45.8 % |
| forfeits | 0/1000, 0.00 % | 0/1000, 0.00 % |
| pentanomial [0, .5, 1, 1.5, 2] | [54, 91, 198, 118, 39] | [34, 92, 229, 107, 38] |
| repetition warnings (S042, DEC-187) | 328 | 0 |

S198 ran under governor `performance`; this run under `powersave`. Games an
hour went 2277.0 -> 2110.2, about 7.3 % fewer. The time control bounds a
game's wall time regardless of governor, so this drop is reported and not
attributed further -- not to the governor alone, not to the new book's longer
games (114.0 plies a game here, median 101, against S198's 102.1 mean, median
92), not to any split between the two.

## Variance against the two bands

Against S105's band (`S105_calibration_after.pgn`, 0.2395 +/- 0.0152),
printed by the script itself:

    pair variance    0.2905 +/- 0.0184  vs  0.2395 +/- 0.0152
    ratio            1.213
    z                +2.14  (OUTSIDE the band, |z| < 1.96)

Against S198's own figure (0.2430 +/- 0.0154), same formula, computed here
(not printed by the script, which only bands against S105):

    diff             0.0475
    combined error   sqrt(0.0184^2 + 0.0154^2) = 0.0240
    z                +1.98  (OUTSIDE, on the same |z| < 1.96 convention)

Both reads land outside, S198's by a hair. The pentanomial shows why: "white
won both" fell (17.6 % -> 13.0 %) but "black won both" rose sharply (34/500,
6.8 % for S198 against 54/500, 10.8 % here) -- more pairs at *both* extremes,
which pushes variance up even while the one-sided metric the script names
("white won both") went down. Reported because it explains the move; per
`S198_pairs.py`'s own docstring (DEC-143) it does not decide anything.

## Cost per verdict

Cost per verdict is variance divided by throughput.

    cost(this run) = 0.2905 / 2110.2 = 0.00013766
    cost(S198)     = 0.2430 / 2277.0 = 0.00010672
    ratio (this/S198) = 1.290
    relative error    = sqrt((0.0184/0.2905)^2 + (0.0154/0.2430)^2) = 0.0896
                     => 1.290 +/- 0.116  (1 sd)

Cost per verdict here is 1.29 +/- 0.12 times S198's -- about 2.5 combined
standard errors above parity (1.0), worse, not a tie. Games an hour is
treated as exact per the step's own convention, so it cancels out of the
ratio's *relative* error exactly: the propagated error above is exactly the
propagated error of the variance ratio alone, scaled by the (exact)
throughput ratio.

## The rule, applied

From S219's `accepts`, literally: "lower or equal cost per verdict at
variance inside S105's band keeps it, a worse cost per verdict reverts
`book=` in a second commit and records the number -- and where the pick and
the current book are close the balanced one is preferred."

Cost per verdict is worse here (1.29 +/- 0.12x S198's), not lower or equal,
so the "keeps it" clause does not apply and the "worse cost reverts" clause
does, on its own wording alone. Variance is also outside S105's band
(z +2.14) and at the edge of S198's own figure (z +1.98) -- both readings
point the same way, so there is no tension between the two halves of the
rule to resolve here, unlike the case the step anticipates (variance outside
the band in the *favourable* direction while cost is better). The "close,
prefer balanced" clause is a tie-break for numbers that are close; 2.5
combined standard errors on the cost metric is not close by the standard
`S219_book_compare.md` itself uses for ties (one combined standard error), so
that clause is not reached either -- it would not save `noob_3moves.epd` even
though it is the balanced book.

**Recommendation: revert.** By the literal rule, `fastchess.sh`'s `book=`
should move back to `books/UHO_Lichess_4852_v1.epd` in a second commit that
records this number. This does not contradict S219's book-comparison survey
(`S219_book_compare.md`): that measurement ranked books by `M = nElo^2 x
games/hour` under a *handicapped* SPRT (8+0.08 vs 4+0.04, sensitivity to a
known effect), a different design and a different quantity from the
pair-score variance a plain A/A reads here, which is the test DEC-143 makes
the deciding one. The two measurements disagreeing is not an error in
either.
