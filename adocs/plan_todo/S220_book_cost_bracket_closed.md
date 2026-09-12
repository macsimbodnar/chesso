id:         S220
goal:       the balanced book's cost per verdict near zero strength difference is measured, not bracketed -- or the owner decides it is not worth the nights: `noob_3moves.epd` against `UHO_Lichess_4852_v1.epd` at a quarter time handicap, priced honestly, gated on the owner's word (DEC-191, DEC-193)
accepts:    **starts only on the owner's word (DEC-193)**; if it runs: a script `adocs/data/S220_slope_compare.sh`, adapted from `adocs/data/S219_book_compare.sh` and **pre-registered in its header before the first game**, one HEAD binary against itself at a quarter handicap, `8+0.08` against `6+0.06`, `Hash=16`, `Threads=1`, concurrency 12, the `fastchess.sh` adjudication, on `books/noob_3moves.epd` and `books/UHO_Lichess_4852_v1.epd` only, **12000 games per book** in two counterbalanced passes (the two-night size DEC-193 priced: about 13 % one-sigma error on the hours ratio, a 2.3-sigma separation of the bracket's ends; the 6000-per-book size is stated in the header as the one-night alternative with its 17 %), the same seed on both books within a pass, busy guard as S219's; a reader `adocs/data/S220_read.py` importing `S105_pairs` and `S219_read` where it can, validated to the digit against fastchess's printed Elo, nElo and pentanomial on every match; **the reading, pre-registered**: per book pooled over both passes the mean pair score above one half `s(q)` with its error, the pair standard deviation, nElo, games an hour; the slope ratio `r(q) = s_noob3(q) / s_uho(q)` beside the doubling's `r(1) = 1.13` (`adocs/data/S219_book_compare.md`); the hours-per-verdict ratio `H(q) = (0.2905 / 0.2430) x (1 / r(q))^2 x (2277 / 2110)` with its error propagated from the four measured quantities, the variances from `adocs/data/S219_aa_calibration_pairs.txt` and `adocs/data/S198_calibration_pairs.txt`, throughput exact; no extrapolation to zero is made -- the quarter is the nearest point to the SPRT regime this budget reaches and is reported as such; **the decision**: the run decides only if `H(q)` sits more than two of its sigmas from one end of the bracket, in which case that end is the reading and the book follows it (a revert is one commit, `fastchess.sh`'s `book=` and `books/fetch_book.sh`'s default back to `UHO_Lichess_4852_v1.epd`, recording the number and re-running no past verdict; a keep closes the question); otherwise the run reports and the owner decides; either way the DEC-143 A/A a revert would need is the one S198 already took; a decision entry records the outcome; `adocs/specs.md`'s harness paragraph, `DEV_MANUAL.md`'s cost paragraph and `adocs/data/README.md` are updated; every pre-registration written after the outcome names the book it decided
touches:    adocs/data/, fastchess.sh and books/fetch_book.sh only on a revert, DEV_MANUAL.md, adocs/specs.md
excludes:   any third book; any change to the time control, `Hash` or adjudication; measuring near zero directly (about 100000 games for the same resolution, DEC-191); re-running any past verdict -- every recorded verdict stays attributed to the book it ran on; any `src/` change
decisions:  DEC-193, DEC-191, DEC-190, DEC-189, DEC-182, DEC-143
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

## Power, re-checked 2026-09-12 (DEC-193)

The first draft of this file gave the rule "H at or under 1.00 keeps, over
1.10 reverts" for 12000 games. Priced: fastchess's nElo interval at 1500
games was +/- 17.58 (95 %) in S219 and scales with one over the square root
of the games -- +/- 8.8 at 6000 per book, +/- 6.2 at 12000. A quarter
handicap is worth roughly 85 to 95 nElo (the doubling gave 200 to 230), so one
book's slope carries about 5 % one-sigma error at 6000 games, the two books'
ratio 7 %, the ratio squared in H 14 %, and with the two A/A variances (6.3 %
each) **H carries about 17 % one-sigma error after one night and about 13 %
after two**. The rule's thresholds are 10 % apart and cannot be reached; the
bracket's ends, 0.97 and 1.29, are 1.7 sigma apart after one night and 2.3
after two. Extrapolating to zero from two handicap points adds error. So the
run can, at two nights, say which end of the bracket is more likely, and
nothing finer -- and that is what the amended `accepts:` promises.

The alternatives, for the owner (DEC-193): the ledger of realized SPRT games
per verdict class on the new book is a free measurement of the same quantity
in the regime that matters, readable after about five verdicts against the
old book's nine; or decide on the leaning and the mechanism and let the
ledger be the check. The coordinator recommends the last, with the ledger as
the check; this step waits for the owner's word and does not hold a night.
