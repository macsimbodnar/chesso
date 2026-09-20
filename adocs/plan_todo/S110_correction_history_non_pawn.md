id:         S110
goal:       a second correction table keyed on the non-pawn structure, split by colour
accepts:    an SPRT verdict, recorded whatever it is; the key is maintained incrementally in add_piece, remove_piece and move_piece and is never recomputed in evaluate() -- INV-4, and the 25 % of nps S014 removed is what a recomputation puts back; the corrected score can never cross into the mate or decisive band, and a test asserts it; the table is a constant-sized array with its dimensions and its clamp stated in src/search_params.hpp
touches:    src/search.cpp, src/bitboard.cpp piece primitives, src/data_structures.hpp, src/search_params.hpp
excludes:   the pawn table, which is S099 and lands first; continuation correction, which is S111
decisions:  DEC-071, DEC-084
closes:
blocks:
paused_by:
done:

## Reserve, 2026-08-19, DEC-087

Demoted behind the 3000 push by the second review: the non-pawn and
continuation tables measure +3 to +8 and only above ~3100 in the surveyed
record -- **the band is unverified** and the 2026-09-04 literature check says
so in as many words: one fetched point lands inside it, Sirius pull request
#176, minor-piece and king correction history, **+7.43 +/- 4.78** at 8+0.08
(https://github.com/mcthouacbb/Sirius/pull/176), and *nothing* fetched
supports "only above ~3100"; Starzix pull request #98, "Pieces correction
history", publishes no Elo figure at all
(https://github.com/zzzzz151/Starzix/pull/98), and CPW's page states none
(https://www.chessprogramming.org/Static_Evaluation_Correction_History, which
does say gains grow with the time control). **Searched again 2026-09-13**,
after S186's fast check: the same page, an open web search for the band and
for the rating claim, and the engine changelogs that search turns up -- no
source for "+3 to +8" and none for "only above ~3100"; the one ledger that
prices these tables separately (below) carries no rating band at all. The
band stays unverified and the order does not rest on it. The pawn table (S099, which stays
in the main order) has its figure sourced: **+11.35 +/- 5.16** at 8+0.08 over
7502 games, bounds [0, 3], Lynx pull request #1662, merged 2025-04-15
(https://github.com/lynx-chess/Lynx/pull/1662) -- the "+11.4" this file quoted
rounded. **The strength it was measured at is not sub-3000**: S181 banded that
merge at **3224 to 3291** CCRL Blitz 1CPU -- between v1.9.1, which the list
does not carry, and v1.10.0, with v1.9.0 the nearest rated release before it,
on the list computed 2026-09-05 (`adocs/data/S181_lynx_bands.md`). So the one
figure DEC-087 (b) called sub-3000 was measured **above** the ~3100 that
demoted this step and S111, and the criterion that separated them separates
nothing. DEC-133 is the owner's answer -- S099 to the head of the reserve as
the family's probe, this step and S111 gated on its verdict -- and DEC-176
records that the re-banding changes the record and not the order. That this
family's gains grow with the time control is one more
reason it reads better after S152, which absorbed S128's question
(DEC-108), moves the measurement nearer the list's control -- the *size* of
that growth had no source and was struck on 2026-09-19 (S232), the direction
did not.

## Order, and why it is three steps

The published record measures these separately and they are **not** inert
apart, so DEC-082 does not apply and they do not become one step. ~~Reported,
all short then long time control: pawn +11.29 / +12.40 and +4.87 / +11.70,
non-pawn +6.98 / +12.28 and +2.80 / +6.84, continuation +2.58 / +5.46 and
+2.75 / +5.46.~~

**Struck 2026-09-19 by S232.** Four passes have looked for a publication
carrying those twelve numerals and none found one, so they are neither a seed
nor a record of anything: a figure whose source cannot be named is not
evidence, and a hedge on the same page as the numerals is what F06 of the
2026-09-19 study review found insufficient in S099. They are struck with
`~~...~~`, dated, with the reason under them, rather than carried under a
hedge; S180 deleted the same class outright and recorded each removal in its
stamp, and a strike is that treatment with the history left readable.
The search history is kept because it says what was looked for: the
2026-09-04 literature check found no source (row A22); a third search on
2026-09-13, after S186's fast check, found none either -- an open web search
for the numerals themselves against "correction history", the CPW page below,
and the engine changelogs it leads to; S232 searched a fourth time on
2026-09-19 the same way and found none, the only figures returned for this
family being the ones already cited in this file. What the argument rests on
instead is below, and it is sourced.

**The shape has a source even though the twelve numerals did not** (2026-09-13,
after the fast check). Tcheran's `CHANGELOG.md`
(https://raw.githubusercontent.com/tcheran-chess/tcheran/master/CHANGELOG.md,
changelog entries only, DEC-016) prices four correction-history tables as four
separate patches in one engine, release 10.0: "Add pawn eval correction
history (21.76 +- 8.62)", "Add major and minor piece eval correction history
(18.86 +- 7.99)", "Add non-pawn eval correction history (12.69 +- 6.30)", "Add
threat eval correction history (9.30 +- 5.31)". Four tables, four positive
verdicts, each measured on top of the ones before it -- which is exactly the
"not inert apart" claim this section makes, now with a URL behind it, and the
non-pawn entry is the class this step is. **The changelog states one figure per
line and no time control for these entries**, so nothing here reads on the
short-versus-long doubling; that half of the shape still rests on CPW's
statement that the family's gains grow with the time control and on S099's
sourced +11.35. One engine's ledger is direction, not a prediction for this
one (DEC-019).

Two things follow. **The family's gains grow with the time control** -- CPW's
own words, re-fetched 2026-09-19: "Correction history has been shown to
exhibit non-linear scaling behavior with respect to increasing time control,
with larger gains in longer searches". So `--fast` at 8+0.08 under-measures
this family, and the verdict says so rather than reading the point estimate
as the effect. **By how much has no source**: the "about twice" factor this
paragraph carried until 2026-09-19 came from the twelve struck numerals and
went with them (S232).
And the whole family is reported to be worth *more* for a hand-crafted
evaluation than for a network, because the residual there is to correct is
larger. That is the one place on this plan where DEC-054's no-NNUE constraint
is an advantage rather than a cost.

## Constants and seeds

**Written 2026-09-19 by S232**, under DEC-105 as DEC-134 states it, so this
file names its seeds before it is ever started. Three forms, and each seed
says which: **(a)** a value from a publication *about the technique*, with
its URL; **(b)** a derivation over chesso's own data or scale, run by this
step at its start; **(c)** the range midpoint or the off value, stated as
such. No publication about the technique states a value for any constant
here -- the CPW page names its symbols without values, re-fetched
2026-09-19 -- so **(a)** is unavailable throughout and is not claimed, and
nothing is seeded from another engine's shipped constant wherever it is
republished.

- **Weight cap, EMA scale, maximum applied correction, grain and entry
  clamp** -- **(b), the values S099's own fit returns.** S099 seeds them from
  chesso's own recorded median depth, chesso's own material scale and two
  declared ranges, and then fits them over chesso's self-play; this step
  instantiates that same
  parameterised table for a second key and starts from what the fit
  returned, which is chesso's own output and not a number from anywhere
  else. Read them at this step's HEAD and not out of S099's file.
- **Table entry count for the non-pawn key** -- **(b), the census S099
  defines, re-run for this key.** Distinct non-pawn keys over `go depth 12`
  on the 300 positions of the S021 stratified pick; the next power of two
  above the 95th percentile; the percentile table in this step's stamp.
  Fallback **(c)**: the exponent range and midpoint S099's constants section
  declares.
- **Weight of this correction against S099's**, one named constant --
  **(c), the range midpoint, stated as such.** The two corrections have to
  combine, and the blend weight is this table's applied share expressed as a
  **percentage of the pawn table's**: range **0 to 200**, where 0 is this
  table switched off -- the state S099 measured -- and 200 is twice the pawn
  table's share. The midpoint is **100**, equal weight, and that is the
  seed: nothing here claims one key is worth more than the other before the
  fit says so. A percentage is the unit because a blend weight is a ratio
  between two corrections and not a quantity in the material scale. The
  blend across the correction tables is one fitted vector and S127 owns it;
  S111's `accepts:` says the same for the third table.

**Anti-seeds -- records, not seeds.** The Elo figures in the sections above
-- Sirius's +7.43 +/- 4.78, Tcheran's four changelog lines, S099's sourced
+11.35 -- are records of direction under DEC-019, never starting values, and
none of them is a constant of this table in the first place. The twelve
struck numerals are not even that.

seeds re-derived 2026-09-19 under DEC-105 (DEC-134)
