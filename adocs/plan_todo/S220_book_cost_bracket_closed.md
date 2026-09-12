id:         S220
goal:       the balanced book's cost per verdict near zero strength difference is measured, not bracketed -- the score-slope ratio between `noob_3moves.epd` and `UHO_Lichess_4852_v1.epd` at a medium time handicap closes DEC-191's 0.97-to-1.29 bracket and decides the book
accepts:    a script `adocs/data/S220_slope_compare.sh`, adapted from `adocs/data/S219_book_compare.sh` and **pre-registered in its header before the first game**: one HEAD binary against itself at a quarter handicap, `8+0.08` against `6+0.06`, `Hash=16`, `Threads=1`, concurrency 12, the `fastchess.sh` adjudication, on `books/noob_3moves.epd` and `books/UHO_Lichess_4852_v1.epd` only, 1500 rounds (3000 games) per book per pass, two counterbalanced passes, the same seed on both books within a pass, busy guard as S219's; a reader `adocs/data/S220_read.py` importing `S105_pairs` and `S219_read` where it can, validated to the digit against fastchess's printed Elo, nElo and pentanomial on every match; **the reading, pre-registered**: per book pooled over both passes the mean pair score above one half `s(q)` with its error, the pair standard deviation, nElo; the slope ratio `r(q) = s_noob3(q) / s_uho(q)` at the quarter beside the doubling's `r(1) = 0.2323 / 0.2055 = 1.13` (`adocs/data/S219_book_compare.md`); the slope ratio at zero taken as the linear extrapolation of `r` in the handicap's logistic Elo from the two measured points, with its error; the hours-per-verdict ratio at zero `H = (0.2905 / 0.2430) x (1 / r(0))^2 x (2277 / 2110)` with its error, the variances from `adocs/data/S219_aa_calibration_pairs.txt` and `adocs/data/S198_calibration_pairs.txt`; **the decision rule, pre-registered**: `H` at or under 1.00 at its point estimate keeps `noob_3moves.epd` and closes the question; `H` over 1.10 reverts `fastchess.sh`'s `book=` and `books/fetch_book.sh`'s default to `UHO_Lichess_4852_v1.epd` in one commit that records the number and re-runs no past verdict; `H` between 1.00 and 1.10 is put to the owner with the bracket and the two books' other properties (size, balance, game length); either way the DEC-143 A/A that would follow a revert is the one S198 already took on that book at this regime, so no further A/A is owed; a decision entry records the outcome; `adocs/specs.md`'s harness paragraph, `DEV_MANUAL.md`'s cost paragraph and `adocs/data/README.md` are updated; every pre-registration written after the outcome names the book it decided
touches:    adocs/data/, fastchess.sh and books/fetch_book.sh only on a revert, DEV_MANUAL.md, adocs/specs.md
excludes:   any third book; any change to the time control, `Hash` or adjudication; measuring near zero directly (about 100000 games for the same resolution, DEC-191); re-running any past verdict -- every recorded verdict stays attributed to the book it ran on; any `src/` change
decisions:  DEC-191, DEC-190, DEC-189, DEC-182, DEC-143
closes:
blocks:
paused_by:
author:
done:

## Why this exists

S219 measured two things that disagree until a third is known. Its comparison
at one doubling of time found the balanced book turns the same strength
difference into 1.13 times the score (DEC-189); its A/A found the balanced
book's pair noise at zero difference is 1.20 times the old book's and its
throughput 0.93 (DEC-190). On nElo bounds the hours a verdict costs go as
noise over signal squared over throughput, so the balanced book costs 0.97 of
the old book's hours if the 1.13 holds where SPRTs run, near zero, and 1.29 if
it does not -- and the fast check over `6272149` showed that nothing measured
so far says which (DEC-191). The one quantity measured in both regimes, the
pair spread, reverses order between them, so the slope may too.

## What is measured, and why a quarter

A handicap small enough to sit between the doubling and zero, large enough to
be measured in a night: a quarter of the time gives roughly 80 to 100 nElo by
the doubling's 200, and 6000 games per book put about 9 nElo of 95 % error
on each, a slope-ratio error near 8 %. That is not a measurement at zero. It is
the second point of a trend whose first point is the doubling: if the ratio
holds or rises toward zero, the favourable end of the bracket is the reading;
if it falls toward one, the pessimistic end is. The extrapolation is linear in
the handicap's logistic Elo and its error is stated; the rule above is written
so that the extrapolation's uncertainty is not argued after the fact.

## Cost

12000 games at the time-odds rate, about 2500 an hour: **about 4.8 hours**,
over DEC-155's line, so a night run, behind S042's SPRT. The pins, switch and
A/A that a revert would need are already in the record for the old book. The
owner can short-circuit this step either way with one commit (DEC-191).
