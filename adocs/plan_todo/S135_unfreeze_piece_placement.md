id:         S135
goal:       unfreeze the piece placement group and refit it, one bundled SPRT over the three remaining features, by the owner's decision of 2026-08-20
accepts:    S134 has landed first, so the group is three identified features and carries no unidentified column -- a bundled verdict over this bundle is at least attributable, which S027's was not; the fit is `--only piece_placement` on the corpus S082 and S083 produce, so the SPRT measures the term and not a joint refit of the other 820 constants; the SPRT bounds are chosen for the effect size and stated in advance, not `--fast` -- DEC-063 is the measurement that the pair and not the hardware sets the cost, and the published parts are +8.2 and +9.86 (DEC-084, order of magnitude only); the term's own speed cost is inside its verdict, measured with the weights forced non-zero because at zero the compiler deletes it (DEC-047); a verdict of zero is recorded as zero, and **if the bundle fails it is bisected** rather than zeroed by hand, which is the S027 failure this step exists not to repeat (DEC-082's bisect rule); the fast suite green
touches:    src/evaluation.cpp piece_placement_mg/eg, the tuner's freeze list in whatever script runs the fit
excludes:   tempo, which is S136; the seventh-rank feature, deleted at S134; any change to how the three features are computed
decisions:  DEC-091, DEC-057, DEC-063, DEC-084
closes:
blocks:
paused_by:
done:

## What S100 established, and what it did not

Refuted for all three of these features: a wrong count (10795695 rows
re-extracted, 0 disagreements), a wrong gradient (worst 4.9e-8 over all 827
parameters), a pipeline that cannot recover a planted vector, a coverage desert
(bishop pair 15.81 %, rook open 31.38 %, rook half-open 31.26 % of rows), and
redundancy with the tables (R^2 0.487, 0.265, 0.168 -- against 0.99 for
redundancy, on the widest right-hand side the model offers).

Not refuted, and the operative cause: **these three have never been separately
measured.** They shared one SPRT with the seventh-rank feature at `--fast`
bounds `elo0=0 elo1=10`, returned -5.48 +/- 11.46 over 2284 games, and were then
zeroed **by hand**; every fit since S065 has held them there with
`--freeze piece_placement` (DEC-057). The freeze's stated reason was that S027
verdict, and S100 calls it procedural.

**Still unresolved and not refuted:** the correlation form of the corpus
hypothesis -- chesso's self-play cannot show the value of a feature chesso
weights at zero. That is why this step's fit is on S082's corpus and not on the
outgoing one.

## The owner's decision, and the concern it overrides

The agent's recommendation was to split the group and measure the bishop pair
alone first: it is the free feature -- the compiler rewrites
`count_bits(x) >= 2` into `x & (x - 1)` -- it carried the largest fitted weight
of the four, and the three rook features cost 3.1 to 4.0 % of a search between
them, so a bundle prices a cheap term that may work together with expensive ones
that may not.

**The owner chose one bundled fit and one SPRT over all of them** (DEC-091). The
concern is recorded, not re-argued. Two things make this bundle materially better
than S027's, and they are why the choice is defensible: S134 removes the one
member that was not an identified quantity, and the bisect-on-failure rule means
a negative verdict is followed by attribution rather than by a hand revert.

## Measurement

One SPRT, bounds stated before the run. DEC-063 is the constraint that decides
whether the night buys a verdict at all: the same constant took 6 h 36 m over
9036 games for nothing at `elo0=0 elo1=5` and returned H1 in 1 h 41 m at
`elo0=-5 elo1=5`. At S105's measured 23.1 to 38.7 games a minute that is roughly
1.5 to 3 hours for a resolving pair.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**All three features are on the wiki, two of them carry a published
magnitude, and none carries an Elo figure.** CPW *Evaluation of Pieces*
(https://www.chessprogramming.org/Evaluation_of_Pieces, fetched 2026-09-13) is
a link hub: it names *Bishop Pair* and *Rook on Open File* and describes
neither. The definitions and the numbers are on the dedicated pages.

- CPW *Bishop Pair* (https://www.chessprogramming.org/Bishop_Pair, fetched
  2026-09-13): "Larry Kaufman proposed the value of half a pawn", citing
  Kaufman's 1999 *The Evaluation of Material Imbalances*. That is the only
  number on the page; **no Elo figure**.
- CPW *Rook on Open File* (https://www.chessprogramming.org/Rook_on_Open_File,
  fetched 2026-09-13): "Bonuses applied to a rook on an open file vary from 8
  to 20 centipawns", and "Typical bonus for a semi-open file is half of that
  for a fully open file". **No Elo figure**. **Its 20 is an engine's**: the
  page's footnote 1 reads "20 cp comes from Toga log user manual, after adding
  actual bonus and penalty for a rook on a closed file" (footnote re-read
  2026-09-13 after the fast check), so the upper end of that range is Toga's
  documented constant republished, and DEC-105 as amended by DEC-134 refuses
  it as a seed wherever it is quoted. The range is read for form only.

**One of the two is usable as a seed and one is not.** Kaufman's half a pawn
originates in a 1999 article about material imbalances -- a publication about
the technique, DEC-105 form (a). The 8-to-20 range does not: its stated upper
end is Toga's, and a range whose end is an engine's constant cannot be
laundered into literature by its venue, which is the exact hole DEC-105
closed. Section 4 states what replaces it. The form check owes no departure
statement: chesso's three features are the wiki's three features, one flat
weight each, and the whole question is the weight.

**The two figures this file already quotes are traced and are the right size.**
Lynx pull request #390, "tapered bishop pair", merged 2023-09-10:
**+8.2 +/- 6.1** short and **+9.5 +/- 6.7** long
(https://github.com/lynx-chess/Lynx/pull/390). Weiss pull request #231, "Tune
Rook+Queen" -- rook and queen piece-square tables together with open and
semi-open file bonuses, 2020-04-06: **+9.86 +/- 5.74** and **+8.40 +/- 5.42**
(https://github.com/TerjeKir/weiss/pull/231). Both are bundles: #390 tapers
an existing bonus rather than adding one, and #231 retunes tables alongside
the file bonuses, so neither prices a file bonus alone. They are quoted for
order of magnitude and nothing else (DEC-019, DEC-084).

**Three figures this file used to quote are gone (DEC-203, applied here
2026-09-13 after the fast check).** The plan's older per-feature numbers for
this group -- one for the bishop pair, one for a rook on an open file, one for
a rook on a half-open file -- survived three searches with no source and are
deleted, on the same rule and for the same reason as the fourteen DEC-203
removed elsewhere: an unverified figure carries no weight and a kept one gets
quoted again by the next reader. Searched this pass: GitHub's commit search
over `TerjeKir/weiss` for "open file" (three commits, the only priced one
being #231's retune) and over `jhonnold/berserk` for "bishop pair" (no
commits), and an open web search for the three values together -- nothing. The
2026-09-04 check searched Weiss's pull requests for "bishop pair", "seventh"
and "open file", Berserk's for "bishop pair", and Stash's changelog, and found
the same nothing. **What is sourced and stays**: Weiss #95 "Tune Eval 1", a
CLOP of the bishop pair together with a king-vulnerability term, 2019-11-26,
**+16.10 +/- 8.74** / **+12.24 +/- 7.21**
(https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22Tune+Eval%22)
-- a bundle, quoted for order of magnitude only. No fetched source gives a
half-open-file figure on its own at all. The accepts prices the group on +8.2
and +9.86 and never needed the deleted three.

**Nobody in the surveyed record measured these three as a bundle**, which is
the one thing worth knowing before the run: every located figure is a single
term or a retune, so the bundle's expected size is a sum of single digits and
the bounds have to be chosen for that, not for a group.

### 2. Shape for chesso

The features exist and are computed; only the weights are frozen.
`src/evaluation.cpp` `piece_placement_mg` and `src/evaluation.cpp`
`piece_placement_eg` are the two rows, `PL_FEATURE_COUNT` their width. After
S134 the width is three and every column is an identified quantity -- which is
the precondition the accepts states and the whole reason this step waits.

Departure from the published form, stated: the surveyed engines apply the
bishop-pair bonus and the rook-file bonuses inside a tapered score with their
own middlegame and endgame halves, which chesso also does. The one thing
chesso does differently is that it **fits** all three rather than hand-setting
them, which is DEC-084's rule and not a departure from any published
technique.

### 3. Implementation sketch

No evaluation code changes. The work is the freeze list, the fit and the run:
`--only piece_placement` on the S082/S083 corpus, weights pasted, speed cost
measured with the weights forced non-zero (DEC-047: at zero the compiler
deletes the term and the measurement is of nothing), SPRT.

### 4. Constants and seeds

**No seeds are needed and none is used.** A `--only piece_placement` fit
starts from the shipping weights, which are zero, and every weight that ships
is the tuner's output on this project's own corpus -- DEC-084's "no constant
ships unfitted" in its simplest form.

What a published magnitude can buy is a **sanity band, stated in advance and
never a target**, and only one of section 1's two is allowed to:

- **Bishop pair.** DEC-105 form **(a)**: Kaufman's half a pawn, from his 1999
  *The Evaluation of Material Imbalances*
  (https://www.chess.com/article/view/the-evaluation-of-material-imbalances-by-im-larry-kaufman,
  as CPW *Bishop Pair* cites it), which is **47** on chesso's material scale
  (`src/eval_tables.hpp` `piece_value`, `PAWN` 94). A published article about
  the technique, not an engine's shipped constant -- which is the whole
  difference between this bullet and the next.
- **Rook on an open file.** **Not form (a) and not the wiki's range.** The
  8-to-20 band's upper end is Toga's constant by the page's own footnote
  (section 1), so the band is refused entire -- half a refused range is still
  the range. What replaces it is **(c) the midpoint of a range this file
  declares by purpose**, its bounds chesso's own: a file bonus is a placement
  preference and must never be worth trading material for, so a single rook's
  open-file bonus is bounded above at **half a pawn** -- 0 to 47 on the scale
  above, midpoint **24**, semi-open half of open by the same declaration. The
  band is wide, and that is the honest cost of refusing the narrow one
  somebody else tuned.

If the fit lands near either band that is a confirmation and a good outcome
(DEC-084); if it lands far from them that is the measurement and the band does
not move it. No engine's shipped bonus is used, wherever it is republished
(DEC-105, DEC-134).

The one number this step chooses rather than fits is the **SPRT bounds pair**,
and it is **(b) derived from this project's own data**: DEC-063 measured that
`elo0=0 elo1=5` took 6 h 36 m over 9036 games for nothing on this same
constant set while `elo0=-5 elo1=5` returned H1 in 1 h 41 m. The pair is
chosen from the expected effect size in section 1 -- a sum of single digits --
and stated in advance with its worst-case expected games from the nElo formula
(DEC-143).

### 5. Pitfalls

- **S027's failure was attribution, not the terms.** One bundled run at
  `--fast` bounds over four features, one of them unidentified, then a hand
  zeroing. DEC-082's bisect rule is what replaces the hand revert, and the
  accepts requires it.
- **The compiler deletes a zero-weighted term.** Any speed measurement taken
  with the weights at zero measures the absence of the term (DEC-047). Force
  them non-zero.
- **The corpus hypothesis is not refuted.** S100 refuted count, gradient,
  recovery and redundancy; it did not refute the claim that chesso's own
  self-play cannot show the value of a feature chesso weights at zero. That is
  why the fit is on S082's corpus and why a zero verdict here is a corpus
  finding to record, not a verdict on the bishop pair.
- **A published bonus is not a floor.** If the fit returns near zero for the
  bishop pair, that is the measurement. Nudging it toward +16 because an
  engine ships +16 is exactly what DEC-105 forbids.

### 6. Measurement

One SPRT, bounds stated in advance with the nElo worst case and the abort rule
(DEC-143). At S105's measured 23.1 to 38.7 games a minute a resolving pair is
roughly 1.5 to 3 hours -- under DEC-155's four-hour line, so daytime work. The
term's speed cost is inside the verdict. A verdict of zero is recorded as zero;
on a failure the bundle is bisected, not zeroed.

### 7. Interactions

- **S134 (blocks this)**: removes the unidentified fourth column. Without it
  this is S027 again.
- **S082 and S083 (before)**: the corpus this fits on. Fitting on the outgoing
  corpus would leave the one unrefuted hypothesis untested.
- **S136 (beside)**: the other half of `--freeze tempo,piece_placement`. Two
  separate fits and two separate verdicts -- one change at a time.
- **S126 (block end)**: refits everything, including whatever this lands.

### 8. References

- - https://github.com/lynx-chess/Lynx/pull/390 -- tapered bishop pair,
  2023-09-10, +8.2 +/- 6.1 / +9.5 +/- 6.7. Pull-request body only.
- - https://github.com/TerjeKir/weiss/pull/231 -- "Tune Rook+Queen", 2020-04-06,
  +9.86 +/- 5.74 / +8.40 +/- 5.42; a retune bundled with the file bonuses.
  Pull-request body only.
- - https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22Tune+Eval%22
  -- #95 "Tune Eval 1", 2019-11-26, CLOP of bishop pair and a king-vulnerability
  term, +16.10 +/- 8.74 / +12.24 +/- 7.21. Commit message only; it is **not**
  the per-feature figure the plan used to carry, which is deleted above.
- - https://api.github.com/search/commits?q=repo:TerjeKir/weiss+%22open+file%22
  -- searched 2026-09-13 after the fast check: "Simplify queen open file"
  (#438, 2021-06-07, 0.18 +/- 2.07), "Combine eval functions for NBRQ part2"
  (#187, 2020-02-25, no figure) and #231 above. No per-feature open-file or
  half-open-file figure. Commit messages only.
- - https://api.github.com/search/commits?q=repo:jhonnold/berserk+%22bishop+pair%22
  -- searched 2026-09-13 after the fast check: no commits.
- - https://www.chessprogramming.org/Evaluation_of_Pieces -- a link hub; names
  the two pages below and describes neither. Fetched 2026-09-13.
- - https://www.chessprogramming.org/Bishop_Pair -- "Larry Kaufman proposed the
  value of half a pawn", footnote 1 citing Kaufman 1999, *The Evaluation of
  Material Imbalances*, first published in Chess Life, March 1999, online at
  https://www.chess.com/article/view/the-evaluation-of-material-imbalances-by-im-larry-kaufman;
  **no Elo figure**, and the page attributes the value to the article and to
  no engine. Fetched 2026-09-13, footnote re-read after the fast check.
- - https://www.chessprogramming.org/Rook_on_Open_File -- open-file bonus "vary
  from 8 to 20 centipawns", semi-open "half of that"; **no Elo figure**;
  footnote 1, "20 cp comes from Toga log user manual, after adding actual
  bonus and penalty for a rook on a closed file", which is why the range is
  read for form and **refused as a seed**. Fetched 2026-09-13, footnote
  re-read 2026-09-13 after the fast check.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` row A28 -- the
  record of what was searched for the three deleted per-feature figures and
  why none of them resolved. Read as a record, not as evidence: the numerals
  it holds are the ones DEC-203's rule removed from this file.
