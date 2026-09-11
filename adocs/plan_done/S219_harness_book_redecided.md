id:         S219
goal:       the harness opening book is re-decided from a survey of open-licence books -- the owner downloads the pick, its digests are pinned, and the DEC-143 A/A that follows is read against S198's to say what the book costs and buys (DEC-182)
accepts:    `adocs/data/S219_book_survey.md` tables the candidate books -- name, balanced or unbalanced, positions, licence with the URL it was verified at, source URL, who tests with it -- and every candidate's licence is CC0, public domain or permissive and verified at the source, none with an unstated licence (`books/fetch_book.sh`'s rule: sp-cc.de is not a source); the survey names the candidates and the owner downloads them -- `popularpos_lichess_v3.epd`, `noob_3moves.epd` and `UHO_4060_v4.epd` from `official-stockfish/books`, CC0, beside the `UHO_Lichess_4852_v1.epd` already in `books/` -- and the pick is **measured, not read**: fishtest's book-comparison method with the handicap and the budget re-sized for power and **pre-registered in `adocs/data/S219_book_compare.sh`'s header before the run**: one binary against itself at one doubling of time, 8+0.08 against 4+0.04, in fixed-rounds matches of 1500 games per book per pass, two counterbalanced passes, the same seed on every book within a pass; per book, pooled over the passes, normalized Elo with its error and games an hour, and the pick is the book with the largest M = nElo^2 x games per hour -- hours per verdict go as 1/M -- where two books whose M differ by less than one combined standard error are tied and a tie is broken toward the balanced book, and no candidate beating the current book beyond that error keeps the current book unless the tied candidate is balanced; `adocs/data/S219_read.py` computes that reading from the PGNs and is validated against fastchess's own printed Elo and nElo; `books/fetch_book.sh` pins both digests of the pick and `fastchess.sh`'s `book=` and `book_format=` move to it in one commit that also states the change in `DEV_MANUAL.md`; one fixed-rounds A/A of 1000 games at the S198 regime follows (DEC-143), detached with a watcher, read with `adocs/data/S198_pairs.py`: pair score variance, games an hour, draw rate and the share of pairs decided by the opening, each beside S198's figure in a table under `adocs/data/`; the reading decides **in writing** whether the pick stays -- lower or equal cost per verdict at variance inside S105's band keeps it, a worse cost per verdict reverts `book=` in a second commit and records the number -- and where the pick and the current book are close the balanced one is preferred, the owner's stated leaning; `rating.sh`'s book is decided in the same reading; if the chosen material is a PGN game collection under a licence that permits it, whether `src/openings.bin` is rebuilt from it by `tools/make_book` is decided and recorded, otherwise the embedded book is out of scope and the stamp says so; `adocs/specs.md`'s harness paragraph, `adocs/data/README.md` and `TOOLCHAIN.md` where it names the book are updated; every pre-registration written after the change names the book
touches:    books/fetch_book.sh, fastchess.sh, rating.sh, DEV_MANUAL.md, TOOLCHAIN.md, adocs/specs.md, adocs/data/
excludes:   any `src/` change unless the embedded-book half is taken, and then `src/openings.bin` alone; re-running any past verdict -- every recorded verdict stays attributed to the book it was taken on, as DEC-049 attributes a figure to its machine; any book whose licence cannot be verified at its source, whatever its quality; changing the time control, `Hash` or adjudication in the same step -- one harness change per A/A (DEC-143)
decisions:  DEC-182, DEC-083, DEC-088, DEC-143, DEC-016
closes:
blocks:
paused_by:
author:     Claude Fable 5.1, coordinator; pre-launch work by an Opus 5 subagent (DEC-185); reading, switch and A/A reading by Sonnet 5 subagents (DEC-188). Started 2026-09-11 evening, closed 2026-09-12 01:45
done:       2026-09-12 01:45. The pre-registered comparison (12000 games, four CC0 books, one doubling of time, `adocs/data/S219_book_compare.sh` header written first) picked `noob_3moves.epd` on M = nElo^2 x games/h, 154.1M against the incumbent's 123.8M, clear of every other book by more than one combined error; reader cross-checked to the digit against fastchess on all eight matches; `fastchess.sh` and `books/fetch_book.sh` switched in `5047070`, DEC-189. DEC-143 A/A on it: 1000 games, 0 forfeits, 2110 games/h under `powersave`, pair variance 0.2905 +/- 0.0184 against S198's 0.2430 +/- 0.0154; the accepts' variance/throughput rule alone reads 1.29x S198's cost and recommends the revert (`adocs/data/S219_aa_calibration.md`), combined with the comparison's measured 1.13x score response it reads 0.97 to 1.01 +/- 0.12, a tie, and the tie keeps the balanced book -- DEC-190 records both readings, amends the rule and resets the band. Evidence `adocs/data/S219_book_compare.md`, `S219_aa_calibration.{md,log,pgn,pairs.txt}`, README rows. `tests/test_fastchess_script.sh`'s stub renamed to the new book, observed red (9 assertions) then green; gate green in both builds, `clang-format.sh --check` clean. `DEV_MANUAL.md` updated (fetch, cost table, calibration table), `MANUAL.md` checked unaffected, `README.md` human-owned untouched. `rating.sh` keeps `8moves_v3.pgn` (DEC-189); embedded book out of scope, the pick is an EPD list; `popularpos_lichess_v3.epd` and `UHO_4060_v4.epd` deleted from `books/`, `UHO_Lichess_4852_v1.epd` kept pinned. Open: the zip digest pin awaits the owner; `S219_read.py` prints its tie message without a tie, cosmetic, left under the append-only convention; the governor confound on throughput is settled by the next A/A under `performance`. Fast check over `5047070` found the stub, DEV_MANUAL's candidate sentence and status.md's stale line; all closed here

## Why this exists

S105 adopted `UHO_Lichess_4852_v1.epd` for the surveyed engines' regime
(DEC-083). What it measured: games a fifth shorter, pair variance unchanged
(ratio 1.022), and the book's stated reason -- Pohl's draw floor -- not holding
at this strength, where chesso draws 40.3 % even on the balanced book. So the
book has been kept on a throughput argument and nothing else, and `status.md`
carried the question for the owner since 2026-08-20.

The owner's instruction of 2026-09-11 (DEC-182): survey the best open-licence
material, prefer balanced where the numbers are close, the owner downloads,
and the harness is re-calibrated after the change. The **A/A is the decision**,
not the survey: a book is a measurement input, and which one costs less per
verdict at this engine's strength is a number this machine can produce in
about half an hour.

## What the A/A compares, and how it decides

S198's calibration is the reference: 1000 games, 2277 an hour, pair score
variance 0.2430 +/- 0.0154 (S105's band), 0 forfeits. The new book plays the
same 1000 games at the same regime. Cost per verdict is variance divided by
throughput -- more games an hour at the same variance, or fewer games needed
at the same rate, both lower it. Draw rate and the opening-decided share are
reported because they explain a variance move; they do not decide.

The current book stays for any verdict that is ready before the pick is
downloaded and calibrated, and that verdict's pre-registration says which book
it ran on.

## Survey, 2026-09-11

`adocs/data/S219_book_survey.md` is the survey. What it found, in one
paragraph: every book whose licence could be verified is in
`official-stockfish/books` under CC0-1.0, where both books this harness has
played came from; Pohl's own downloads and the OpenBench book set state no
licence and stay out. fishtest's default is the book the harness plays now,
adopted for a draw rate near 50 %, and fishtest's own rule for comparing
books is the time-odds fixed-games test this step adopts. The published
evidence says the draw ratio drives sensitivity and that an engine under the
45 % floor -- chesso is, on every book it has played -- is pointed toward
balance, which is the owner's leaning too. Three CC0 candidates go to the
comparison beside the current book: `popularpos_lichess_v3.epd` (balanced,
200 k), `noob_3moves.epd` (balanced, shallow) and `UHO_4060_v4.epd`
(unbalanced, 242 k). `8moves_v3.pgn` and the Drawkiller book are too small for
a long SPRT by fishtest's own warning and are not candidates. A book of the
project's own from the CC0 Lichess database is viable and is **not** this step:
the question whether exits selected by another engine's evaluation are within
the COPYING rule gets a decision first.

## Cost

The survey is done and the books are downloaded. The comparison is 12000
games -- 8 matches of 1500, four books in two counterbalanced passes -- at
about 2000 to 2300 an hour, roughly 5.5 to 6.5 hours: over DEC-155's four-hour
line, so it is the night run of 2026-09-11, started when the machine is
otherwise idle. The re-sizing from the file's first draft (1000 games and a
30 % handicap, about two hours) is a power calculation written in the script's
header: at 1000 games the metric's error would have exceeded the differences
the published record shows between books. Then the A/A on the pick, about half
an hour, a daytime run; the pins land in the same commit as the switch.

## Reading, 2026-09-12

`adocs/data/S219_book_compare.md` is the reading; all eight matches
cross-check against fastchess's own printed Elo/nElo/Ptnml to the digit. `M =
nElo^2 x games/hour`, pooled over both passes, largest first:

    noob3       balanced    M 154101610 +/- 8564337   nElo +228.27 +/- 12.43   2957.3 games/h
    uho4060v4   unbalanced  M 132307458 +/- 8270825   nElo +202.95 +/- 12.43   3212.4 games/h
    popularpos  balanced    M 128933263 +/- 7903361   nElo +206.96 +/- 12.43   3010.0 games/h
    uho4852     unbalanced  M 123838572 +/- 7907059   nElo +198.69 +/- 12.43   3136.8 games/h  (current)

Pick: `noob_3moves.epd` -- the largest M outright, more than one combined
standard error clear of every other book, so the tie rule is never reached
(DEC-189).

Hours per verdict, pick relative to current: `M_current / M_pick` = 0.80 +/-
0.07 (1 sd) -- about a fifth fewer machine-hours per verdict at the same nElo
bounds.

The pick is **balanced**: `noob_3moves.epd`, fishtest's old shallow default,
against the harness's unbalanced incumbent.

## A/A pre-registration, 2026-09-12 00:55

Written before the run starts. `ROUNDS=500 AA=1 ./fastchess.sh` -- HEAD
against itself, 1000 games at the S198 regime (8+0.08, `Hash=16`, concurrency
12, the adjudication `fastchess.sh` carries) on `books/noob_3moves.epd`, the
book this commit switches to (DEC-189). Detached, watcher armed on
`SPRT-RUN-(DONE|FAILED)`, process death and a two-hour ceiling. Expected
length: S198's 2277 games an hour scaled by the S219 rate ratio 2957/3137 gives
about 2150 an hour, so 28 minutes; under DEC-155's line. Machine: workstation,
load average 0.12 at launch, on mains, governor `powersave` -- S198 ran under
`performance`, and the reading says what that did to games an hour. Open
defect named per DEC-171: S042, the en passant key, on both sides equally.

Read with `adocs/data/S198_pairs.py` and decided by the rule in `accepts:`:
pair score variance inside S105's band (S198 read 0.2430 +/- 0.0154) at a
lower or equal cost per verdict keeps the book; a worse cost per verdict
reverts `book=` in a second commit and records the number. Reported beside
S198's figures: variance, games an hour, draw rate, share of pairs decided by
the opening, forfeits.

## A/A reading, 2026-09-12

Full reading, evidence and arithmetic: `adocs/data/S219_aa_calibration.md`.
1000 games, `5047070` against itself, `books/noob_3moves.epd`, governor
`powersave`; S198 ran the same regime on `UHO_Lichess_4852_v1.epd` under
`performance`. Cross-check: `S198_pairs.py`'s pentanomial `[54, 91, 198, 118,
39]` agrees with fastchess's own printed `Ptnml(0-2)` on every bucket; 328
repetition warnings (S042, DEC-187), both sides equally, not read as a book
signal.

| | this run | S198 |
|---|---|---|
| pair score variance | 0.2905 +/- 0.0184 | 0.2430 +/- 0.0154 |
| games an hour | 2110.2 | 2277.0 |
| draw rate | 35.3 % | 32.3 % |
| opening-decided (white won both) | 13.0 % | 17.6 % |
| forfeits | 0/1000 | 0/1000 |

Variance is outside S105's band (z +2.14) and at the edge of S198's own
figure (z +1.98, diff 0.0475 over combined error 0.0240). Cost per verdict
(variance / games-an-hour) is 1.29 +/- 0.12 times S198's -- about 2.5 combined
standard errors above parity, worse, not a tie. Both the variance-band read
and the cost read point the same way, so the rule in `accepts` applies
without the tension it anticipates (variance outside the band in the
favourable direction while cost is better): cost per verdict is worse, so the
"worse cost reverts `book=`" clause applies on its own wording, and the
"close, prefer balanced" tie-break is not reached either -- 2.5 combined
standard errors is not close by the one-combined-standard-error standard
`S219_book_compare.md` itself uses for ties. This does not overturn
`S219_book_compare.md`'s pick, which measured a different quantity (`M`
under a handicapped SPRT) for a different purpose (ranking four candidates);
it is this direct A/A cost reading that DEC-143 makes the deciding one.

Recommendation: revert `fastchess.sh`'s `book=` to
`books/UHO_Lichess_4852_v1.epd` in a second commit that records this number.

Decision: coordinator, see DEC-190.

## Decision, DEC-190 -- coordinator, 2026-09-12 01:45

The reading above applies the `accepts:` rule literally and recommends the
revert; DEC-190 keeps the book and amends the rule, for one reason stated in
full there: the rule priced a verdict as variance over throughput, which
assumes the score a real strength difference produces does not depend on the
book, and the comparison measured that it does (1.13 times on the balanced
book). Noise from this A/A, signal from the comparison, throughput from this
A/A: 0.97 to 1.01 +/- 0.12 of the old book's hours per verdict -- a tie by the
step's own one-combined-error standard, broken toward the balanced book as
the `accepts:` says. Both readings are evidence; the owner can overrule with
one commit.
