id:         S122
goal:       king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks and weak squares, and it is no longer clamped
accepts:    an SPRT verdict, recorded whatever it is; the term can return several hundred centipawns and **no clamp truncates it** -- S039 and S120 have settled the lazy shortcut before this runs, and this step states which of the three outcomes it inherited; everything feeding the accumulator is a fitted weight and only the finalizer is non-linear, so the tuner's gradient still exists and tools/eval_model.hpp carries the same finalizer; safe checks per piece type and weak squares in the king zone are counted; shelter and storm come from the pawn hash (S118); the corpus this is fitted on **contains attacking positions**, and the step says how that was ensured; **the move-ordering band clearance is re-checked**, because this term's magnitudes are about to grow by an order of magnitude
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tests/test_eval_model.cpp
excludes:   mobility, which shares the loop but is S121; the lazy margin, which is S039
decisions:  DEC-071, DEC-084, DEC-039
closes:
blocks:
paused_by:
done:

## The clamp is the whole problem

`evaluate_expensive()` clamps mobility **plus** king safety to
`LAZY_EVAL_MARGIN`, 184 as shipped since S085's SPSA run raised it from 150, for
both together -- and `evaluate()` is `evaluate_cheap() + evaluate_expensive()`,
so the clamp is on the real score and not only on the shortcut's. **The
architecture forbids a king-safety term strong enough to matter.** A mating
attack is worth four to six hundred centipawns and this evaluation cannot say
more than a hundred and eighty-four about the king and the mobility combined.

That is why S039 was moved from the end of the plan to just before this step,
and why S120 is ordered ahead of both: retiring the clamp costs 11.7 % of nps
measured, and the cache and S104 are what pay for it.

## The formulation, and the one that failed

Four formulations exist and they differ in how accumulated danger becomes
centipawns. The attack-unit table indexed by a hand-written S-curve -- **which
is what chesso approximates today, badly, with a linear model** -- is the
weakest and the hardest to tune: its entries are sparsely represented in any
corpus. The failure mode this file is written around is the **corpus, not the
code** -- a tuner that never sees a mating attack fits the attack terms to
nearly nothing, whatever the form. The write-up this file used to quote for it,
with a regression figure and a fitted attack bonus, was never located by two
searches and both figures are deleted (DEC-203); the warning stands on its own
reasoning and the accepts is what acts on it. What *is* sourced for the form is
Stash's own rewrite at v31, **+4.24** then
**+9.69**, about +13 together, in `mhouppin/stash-bot` `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), and CPW *King Safety*
(https://www.chessprogramming.org/King_Safety), which states no figure.

The form to reproduce is the tuned linear accumulator with a quadratic
finalizer, because it is the one that stays differentiable -- every input is an
ordinary weight the existing Adam fit handles and only the finalizer is
non-linear. **The finalizer's own constants are ours and are fitted at S127**,
not taken (DEC-084).

The corpus warning is the part to act on rather than read: this term is fitted
on chesso's own self-play, and if chesso plays positionally tame chess the fit
will find nothing to fit. Say in the step how the corpus was made to contain
sharp positions.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki describes the technique in detail and every number on the page is
another engine's constant.** CPW *King Safety*
(https://www.chessprogramming.org/King_Safety, fetched 2026-09-13) gives the
attack-unit form -- "Stockfish counts each minor piece attack on a king
zone...as 2 attack units, rook attack on king zone as 3 attack units and a
queen attack as 5 attack units", "Stockfish adds 6 attack units for a safe
queen contact check and a couple attack units for a safe rook contact check"
-- presents the Glaurung safety table and Stockfish's S-shaped curve, notes
that "the whole is greater than the sum of parts", and covers safe checks,
weak squares near the enemy king and pawn shelter and storm ("It is best to
keep the pawns unmoved...The lack of a shielding pawn deserves a penalty").
**No Elo figure anywhere on the page, and no shelter or storm penalty values.**

**Those attack-unit weights are not seeds and the step must not treat them as
such.** 2, 3, 5 and 6 are Stockfish's tuned output and the Glaurung table is
Glaurung's; the wiki is the venue, not the origin. DEC-105 as it amends
DEC-084 is exactly this case -- "a tuned table on the wiki is still an
engine's table" -- and DEC-134 extends it to numbers quoted in prose. Section
4 declares its own ranges instead. The **form** -- accumulate weighted attacks
on a king zone, pass the accumulator through a non-linear finaliser -- is the
part that is used, and it is used freely (DEC-014).

**The form this step ships has a published instance and it is prose, not
source.** Ethereal commit da9a627039639957309c1c37917221c74998175e (2018-03-15,
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+king+safety):
"Define King Safety by a Polynomial. Now we can expand the array beyond 64."
That is the wiki's table replaced by a polynomial finaliser -- the same move
this step makes for the same reason, differentiability -- and the message
reports LLR and game counts with **no Elo figure**.

**The rest of the group, traced.** All Ethereal commit messages through the
search URL above, fetched 2026-09-13; Stash from
https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md.

| what | figure |
|---|---|
| Ethereal bd96fb9ea7758629e5abea28ddcad528c7c0d8ac, 2018-05-27, "Rewrite King Safety" (adds an attackers-threat component, scales attack counts by king-area size, applies king safety in the endgame) | LLR and game counts only, **no Elo stated** |
| Ethereal e707ace330769adee36ecb6e40d4d9430fa9d7c0, 2018-05-06, "Penalize weak squares in the king ring for King Safety eval" | LLR and game counts only, **no Elo stated** |
| Ethereal edf6e2cff2a727a81537a7934d71fd8e8569bab0, 2018-05-30, "Evaluate Safe Checks in King Safety" | LLR and game counts only, **no Elo stated** |
| Ethereal 7d3abca284179a510386e3f490023cdea87913ff, 2020-02-22, "Exclude attacks on double-pawn-protected squares for king safety" | **+3.12 +/- 2.45** at 12.0+0.12s over 37600 games; **+2.38 +/- 1.80** at 60.0+0.6s over 56428 games |
| Ethereal fecb9d83ce00f7c0e2109fcbb4896d0976a851a8, 2020-08-23, "Tune King Safety and modify some interactions of Safety terms" (AdaGrad tuner, drops a friendly-pawns term and an attackers weight multiplier) | **+3.86 +/- 2.85** at 12.0+0.12s over 20432 games; **+3.38 +/- 2.42** at 60.0+0.6s over 20864 games |
| Ethereal b1690086e2b725b1afa81c5bdc82bc1b2dde2ffe, 2020-09-22, "Introduce pawn shelter/storm arrays for king safety" | **+6.40 +/- 4.25** at 10.0+0.1s over 9072 games; **+5.32 +/- 3.38** at 60.0+0.6s over 9404 games |
| Stash v31 king-safety rewrite | **+4.24** then **+9.69**, about +13 together |

Three readings. **The two terms this step's accepts names -- safe checks and
weak squares in the king zone -- were both shipped at the one engine that
records them, and neither carries an Elo number**, so their size here is
unknown and the bounds cannot be sized on the record. **The shelter and storm
arrays are the one traced positive at +6.40**, and they are what S118's cache
feeds this step. And **every long-control half is smaller than its short-control
half** in the two rows that have both, which is the same downward
control-sensitivity S125's record shows.

**The from-scratch regression survived two searches unsourced and its two
figures are deleted (DEC-203).** The claim was that a from-scratch attack-unit
table regressed by a stated margin because the tuner drove the attack bonus to
a stated value, never having seen a mating attack. Searched 2026-09-04 by the
plan review: not located. Searched this pass: the Ethereal king-safety commit
set above; `mhouppin/stash-bot`'s `CHANGELOG.md`, scanned for both numbers,
which do not occur; a web search for a king-safety attack-unit tuning
write-up, which surfaced the mACE Chess post "King safety tuning"
(http://macechess.blogspot.com/2013/10/king-safety-tuning.html, Thomas,
2013-10-19) -- that post reports a **gain**, "Only 10 (selfplay) ELO", and
carries neither the regression nor the bonus nor the corpus diagnosis. Not
located. **The accepts' corpus clause stands on its own reasoning and this file
already says so**, so the deletion costs no ordering argument and removes two
numbers that priced nothing.

### 2. Shape for chesso

Today: `src/evaluation.cpp` `king_safety_mg` and `src/evaluation.cpp`
`king_safety_eg` over `KS_FEATURE_COUNT` counts, linear, and the whole thing
clamped with mobility to `LAZY_EVAL_MARGIN` inside `src/evaluation.cpp`
`evaluate_expensive`.

**Departures from the wiki's form, stated:**

- The wiki's finaliser is a **table** indexed by accumulated units (Glaurung)
  or an S-curve (Stockfish). This step ships a **quadratic** finaliser, because
  a table's entries are sparsely represented in any corpus and a polynomial
  keeps every input an ordinary weight the existing Adam fit handles. Ethereal
  made the same move in 2018 (da9a6270). The departure is deliberate and is
  the step's whole design.
- The wiki's attack-unit weights are fixed constants. Here **everything
  feeding the accumulator is a fitted weight** and only the finaliser is
  non-linear -- which is `adocs/eval_tuning_strategy.md` section 2.4 option
  (a), "tune the table entries themselves as free parameters, which restores
  linearity", applied to the accumulator rather than to a table.
- The wiki's king safety is a middlegame term; Ethereal's 2018 rewrite applies
  it in the endgame too. chesso tapers it like every other term and the fit
  decides the endgame half.

### 3. Implementation sketch

- `tools/eval_model.hpp` carries the **same finaliser**, or the model tunes a
  different function. `tests/test_eval_model.cpp` "the model reproduces
  evaluate() on every phase" is what holds the two together.
- Shelter and storm come out of the pawn hash S118 adds, which is why S118 is
  ordered directly before this step.
- Safe checks are counted per piece type and weak squares over the king zone;
  both read attack sets the mobility loop already builds (S121), not a second
  pass.
- The move-ordering band clearance is re-checked. `piece_values_abs` and the
  100-point band separation are the hazard `CLAUDE.md` records; this term's
  magnitudes are about to grow by an order of magnitude and a silent inversion
  shows up as a strength regression, not a wrong node count.

### 4. Constants and seeds

Every weight feeding the accumulator is **fitted** (DEC-084, and the accepts).
The finaliser's own constants are **ours and are fitted at S127**, which this
file already states. What this section adds is the rule for anything that must
be chosen before a fit can start:

- **No attack-unit weight is seeded from the wiki's 2 / 3 / 5 / 6 or from the
  Glaurung table.** Those originate as Stockfish's and Glaurung's tuned output
  and are not seeds wherever they are republished (DEC-105, DEC-134).
- Where a starting value is needed it is **(c) the midpoint of a range this
  file declares by purpose**, stated as such, or **(b) derived from chesso's
  own scale** -- `src/eval_tables.hpp` `piece_value` for anything compared
  against a material score. The units matter and the file states which scale
  each constant lives in, because `see_value` in `src/bitboard.cpp` is a
  different pawn.
- **The king zone's geometry is (b) derived from the board**, not seeded: the
  squares adjacent to the king plus the rank in front, with the enumeration
  written out.

### 5. Pitfalls

- **The clamp is the whole problem and it is not this step's to remove.**
  S039 sizes it and S120 pays for retiring it; this step **states which of the
  three outcomes it inherited** and does not quietly widen the margin itself.
- **The corpus is the documented failure mode for this term** -- it is the one
  part of the unlocated write-up worth acting on, and the accepts acts on it:
  say how the corpus was made to contain sharp positions. A term fitted on
  positionally tame self-play will fit to nothing whatever the form.
- **A quadratic finaliser makes the gradient non-linear in exactly one place.**
  `tools/tuner_model.hpp` must carry the same derivative, and
  `tests/test_tuner_gradient.cpp`'s finite-difference check needs a fixture in
  the region where the finaliser bends -- the same class of gap DEC-170's F24
  found for the clamp.
- **Double-counting with S101.** Both terms count attacks. The step says how
  they divide.
- **The band clearance.** See section 3.
- **Long control reads lower.** Both Ethereal rows with two figures shrink at
  60.0+0.6s; an 8.0+0.08s verdict over-reports.

### 6. Measurement

One SPRT at the S105 regime, bounds stated in advance with the nElo worst case
and the abort rule (DEC-143). The record cannot size them: the two terms the
accepts names carry no Elo figure anywhere located, so the bounds are chosen
for a term that may measure zero and the run is pre-registered accordingly.
nps beside the verdict; the move-ordering band clearance re-checked before the
run, not after.

### 7. Interactions

- **S039 (immediately before)**: the clamp decision. This step states what it
  inherited.
- **S118 (immediately before)**: supplies the shelter and storm slots -- the
  one traced positive in the group, +6.40.
- **S121 (before)**: the attack sets and the mobility area.
- **S101 (before)**: threats; the two must not count the same attack twice.
- **S127 (after)**: SPSA over the finaliser's constants.
- **S126 (block end)**: the full refit, whose gradient must carry this step's
  non-linearity.

### 8. References

- - https://www.chessprogramming.org/King_Safety -- attack-unit weights quoted
  above (Stockfish's, republished), the Glaurung table, the S-curve, safe
  checks, weak squares, shelter and storm; **no Elo figure and no shelter or
  storm values**. Fetched 2026-09-13. Read for form; its numbers are other
  engines' constants and are not seeds (DEC-105).
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+king+safety
  -- commits da9a6270 (polynomial finaliser), bd96fb9e (rewrite), e707ace3
  (weak squares), edf6e2cf (safe checks) -- all LLR only; 7d3abca2
  (+3.12 +/- 2.45 / +2.38 +/- 1.80), fecb9d83 (+3.86 +/- 2.85 / +3.38 +/-
  2.42), b1690086 (+6.40 +/- 4.25 / +5.32 +/- 3.38). Commit messages only.
  Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v31
  king-safety rewrite +4.24 then +9.69. Changelog entries only; scanned
  2026-09-13 for the regression and attack-bonus figures this file used to
  quote, neither of which occurs.
- - http://macechess.blogspot.com/2013/10/king-safety-tuning.html -- mACE
  Chess, Thomas, 2013-10-19: a king-safety tuning write-up reporting "Only 10
  (selfplay) ELO" as a **gain**; **not** the regression this file quotes.
  Fetched 2026-09-13.
