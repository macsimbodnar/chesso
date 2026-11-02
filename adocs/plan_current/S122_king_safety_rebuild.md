id:         S122
goal:       king safety becomes a fitted linear accumulator with a quadratic finalizer, counting safe checks, weak squares and the king zone's defenders, clamped at a margin re-derived from its own fitted spread (DEC-324)
accepts:    an SPRT verdict, recorded whatever it is; the term is clamped with mobility at a `LAZY_EVAL_MARGIN` re-derived from its own fitted spread (`tools/eval_spread` at the fitted weights and a fresh `truncation_scan`), binding on at most a pre-registered share of rows, the margin its own part in the family's H0 partition, and the step states how the fit and the margin are made consistent (S039 read H0; DEC-324, the owner's amendment DEC-258 names); everything feeding the accumulator is a fitted weight and only the finalizer is non-linear, fitted through S262's per-side nonlinear group -- both sides' sums traced, the gradient checked against finite differences, and the finalizer seeded off zero, because `max(0, x)^2` has no gradient at x = 0 (DEC-134 form b); safe checks per piece type, weak squares in the king zone and the defenders of the king zone are counted, each screened before the one SPRT (DEC-259); the shelter comes from the pawn table's slot (S118, `pawn_shelter_t`, raw before the clamp and the taper) and storm, which nothing computes yet, is added here, in that slot's score or its spare bytes (DEC-332 (5)); the corpus this is fitted on **contains attacking positions**, and the step says how that was ensured; **the move-ordering band clearance is re-checked**, because this term's magnitudes are about to grow by an order of magnitude
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/tuner_trace.hpp, tools/tuner_groups.hpp, tests/test_eval_trace.cpp, tests/test_evaluation.cpp, src/data_structures.hpp
excludes:   mobility, which shares the loop but is S121; retiring the clamp and the lazy shortcut (S039 read H0, DEC-324)
decisions:  DEC-071, DEC-084, DEC-039, DEC-258, DEC-259, DEC-324, DEC-344
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementers: Opus 5.5 subagents, one per phase. Research pass done 2026-11-02, ruled by DEC-344
done:

## Noted 2026-11-02, S310's fast check: the clamp case does not reach the finalizer

`tests/test_tuner_gradient.cpp` "the clamp's derivative is exact where it binds" derives its block from
`tools/tuner_trace.hpp` `clamped_site`, over the feature table. `model.groups` are outside that table and are
added after the clamp, so S262's nonlinear group, this step's finalizer, is not covered by the case. If the
curve sits inside the clamp, the case is extended to assert it silent on a bound row, with a plant seen red.

## Amended 2026-10-28, DEC-324: the clamp stays and its margin is re-derived

S039 read H0, so the lazy shortcut and the clamp stay. The owner amended
this design (DEC-324): the fit runs under the kept clamp, then
`LAZY_EVAL_MARGIN` is re-derived from the fitted spread with a fresh
`truncation_scan` (S039's section 4 procedure, DEC-169, DEC-172). If the
spread stays under 184, nothing moves and the margin owes no run. The
header above carries the amended goal and accepts. Below, "no clamp", "the
clamp is the whole problem" and S039 retiring the clamp are the design as
written before the amendment, kept as history.

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Scope gained 2026-10-08 (DEC-258):** king defenders, the own minor
pieces covering the king zone (CPW *King Safety*), one more input to the
accumulator. The finalizer's constants are fitted by the tuner through
S262's nonlinear group, not by S127's search lane.

Each term -- and each part this file used to give a verdict of its own --
is first fitted with every other constant frozen and kept only if it lowers
held-out loss on S082's fixed validation set by more than the margin the
pre-registration states; a term that fails is recorded with its figures and
does not enter the match. The survivors ship under **one** `{0, 5}` nElo SPRT
for the family, with DEC-143's worst case and abort rule pre-registered. On
H0 or no verdict the family is bisected along a partition written in the
pre-registration, each part its own SPRT. Where this file names
`tools/eval_model.hpp` or `test_eval_model` as the tuner's mirror of a term,
S262's trace and its exact-reconstruction test discharge that clause once
S262 has landed. S263 built no per-side attack set; S101 built the sets by class in stage two's loop (DEC-333), and a term here reads them. S039 runs before this family
and retires the lazy shortcut and the clamp; if S039 read H0 instead, the
owner's amendment of that outcome binds here.

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
measured, and the cache and S104 are what pay for it. [2026-10-06,
coordinator: S120 retired the cache (DEC-257); exact everywhere re-measures at
5.0 % nps for a 7.5 % smaller tree (`adocs/data/S120_measurements.txt`).]

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
arrays are the one traced positive at +6.40**; S118's slot feeds this step the shelter half, and storm is this step's own (DEC-332 (5)). And **every long-control half is smaller than its short-control
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

- The trace records the finaliser's inputs per side and the tuner's group
  carries the **same finaliser**, or the tuner fits a different function
  (S276 deleted the hand-written mirror this line named). **Nothing holds the
  two together yet**: exact_score() in `tools/tuner_trace.hpp` does not read
  groups -- its comment says so, there being no engine arithmetic to be exact
  to before this step -- so `tests/test_eval_trace.cpp` "the trace
  reconstructs evaluate() exactly, clamp included" and load()'s per-row
  identity pass whatever a group computes. This step extends exact_score() to
  its group, the engine's integer finaliser over the two per-side sums, so
  that the identity on every corpus row and that case cover it; the case's
  precondition gains a fixture row on which the group is non-zero.
- The shelter comes out of the slot S118 added, read past the lazy shortcut; storm is computed here. The slot read zero on its own (DEC-332 (5)), and this step is its only reason to stay, so what it adds to the slot is timed as S118's row was.
- Safe checks are counted per piece type and weak squares over the king zone;
  both read attack sets the mobility loop already builds (S101), not a second
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
  S039 sizes it and S120 was to pay for retiring it (retired, DEC-257); this step **states which of the
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
- **S118 (before)**: supplies the shelter slot, storm being this step's -- the one traced positive in the group, +6.40, is the two together.
- **S121 and S101 (before)**: S121's mobility area, and S101's attack sets by class, under the clamp.
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

## From S101's H1, 2026-11-01

S122 inherits the clamp's readings at S101's fit, deciding nothing there: it binds on 0.400 % of S083's rows; mobility p99 107, max 229;
threats p99 50, max 151; combined p99 156, max 415. `LAZY_EVAL_MARGIN` is S122's to re-derive with threats inside the sum (DEC-324, DEC-333 (6)). The
boundary is by target (DEC-333 (9)). `threat_king_mg` and the negative cells of `hanging_piece_eg` and `threat_king_eg` are listed for S126.

## 2026-11-02: the research pass

Read at `ba1cad6`, bench 5482277 reproduced: counts and instructions only, no fit, no match, no timing, nothing under `src/`
changed. Every figure's source is adocs/data/S122_census.txt, by adocs/data/S122_census.sh (adocs/data/S122_census.cpp,
adocs/data/S122_census_core.hpp, adocs/data/S122_research_prototype.patch, adocs/data/S122_census_read.py, and since the fast check
adocs/data/S122_scans.cpp, S122_scan_dump.cpp, S122_scan_rec.hpp, S122_identity_plants.patch, S122_cmp_units.cpp). The census reads the
FEN and game columns; for the label readings it reads the result column of S083's training corpus and of head's 12000-game sample, each
at its stated K, and no other, and S082's fixed set keeps its labels unread by anything that chooses a form. The instruction rows are `tools/perf_counters`' instructions on
sink builds whose bench reads 5482277 and whose `bench 16` reads 14931387; cycles and wall were printed and not read. The one
deviation from the reading-only pass: 12000 self-play games from head's engine were generated to answer the corpus question,
no fit and no match, kept under `.tuning/s122/smoke/` for the coordinator's close and in no tree. The proposal was `.tuning/coord/S122/decision_draft.md`, and DEC-344
rules: this section's false sentences are corrected in place, its proposals are marked as such, and the next section says what stood and what fell.
"Attacking" here means only a count threshold, and no position was judged (DEC-023).

**What exists.** `src/evaluation.cpp` `evaluate_mobility_and_king_safety`: nine counts per side, linear, `king_safety_mg` 18, 13,
5, 30, -27, 16, 7, -47, -25 and `king_safety_eg` -5, -9, 7, -52, 10, -8, -1, -4, 19, attackers N B R Q, zone incidences, near and far
shield, open and half-open file; the last four come through S118's slot. Raw attacks, no x-ray, no pins, over `zone[]`, the king's
square and its neighbours, nine squares at most. Stage two's locals: `zone[2]`, `pawn_attacks[2]`, `pinned[2]`, `king_square[2]`,
`attacks_by[2][4]` (S101's unions), the per-piece `attacks`; `threat_terms` holds `nonking[2]` and `all[2]`. **A weak square reads those
and nothing more; a defender by squares reads `attacks_by`; a safe check needs the sets from the king's square, two magic lookups a
king and four a call, since `pinned_to_king` reads the empty board.** `src/data_structures.hpp` `pawn_shelter_t`: `score_t score[2]`,
`uint8_t king[2]`, `uint8_t spare[6]`, 16 bytes, four to a line; the spare bytes hold nothing and storm fits the score, a miss being
26 % of probes on `bench` (S118's 74.441 % hits).

**What S262's group carries and what it lacks.** Carried: `EVAL_TRACE_SIDE`, `side_entry_t` (int16 coefficient), `group_t` with one
transform, `max(0, x)^2 / divisor`, per half, with a sign; `smooth` and `gradient` through it. **Lacking: the group is outside the clamp.**
`tools/tuner_trace.hpp` `smooth` adds it after the clamp and `gradient`'s group loop does not read `clamp_binds`, while the accepts puts the
curve inside (the 2026-11-02 note's "if"; it is). `exact_score` reads no group, `engine_model` is empty, `group_of_term` answers -1, so `load`
refuses a per-side entry; `derive_seed` has no caller outside the test; no per-group step exists; `tools/feature_audit.cpp` prints
"identities: not computed" the moment a row has a per-side entry, so S101's rank check would go silent. The prototype's scored build
moves it inside and adds the integer finalizer to `exact_parts`: **`tools/truncation_scan` holds `exact_score` to `evaluate` on every row, and it passes on all 6226944
at the eight probes below; that is the identity check.** `tools/eval_spread` also checks every row and also passes, and it reads something else: its clamped sum through the
collecting instantiation against `evaluate` less `evaluate_cheap`. adocs/data/S122_identity_plants.patch, on 200000 rows: the trace's count off by one, the exact score's divisor and
the exact score's sign each turn the scan red and leave the spread green; a T missing from the collecting instantiation does the reverse; and **a wrong count in the engine passes
both**, since the trace reads the engine's own variable. The census's units and the prototype's loop agree on both sides of 1000000 rows, which is one count against another and
no walker.

**The spread, S083's corpus at head** (`tools/eval_spread`, sha256 `2e0e3de9...`):

| | mobility | threats | king safety | combined |
|---|---|---|---|---|
| p50 | 24 | 5 | 15 | 31 |
| p90 | 64 | 25 | 49 | 85 |
| p99 | 107 | 50 | 104 | 156 |
| p99.9 | 143 | 73 | 158 | 225 |
| max | 229 | 151 | 341 | 415 |

The clamp binds on 24897 of 6226944, 0.400 %; 150 reads 1.197 %, 200 0.236 %, 250 0.041 %, 300 0.006 %, 400 nine rows. S082's qs set: 0.302 %,
combined p99 150, max 390. `tools/truncation_scan`: 99 rows past 2.8, worst 2.875 = 69/24.

**The corpus, by counts** (either king; S083's rows; adocs/data/S122_census.txt for S082's three views and the pool):

| grade, count thresholds | rows | % | games | of them \|eval\| < 400 | gradient mass % |
|---|---|---|---|---|---|
| >= 2 attackers | 1257659 | 20.197 | 123954 | 949010 | 18.302 |
| >= 3 | 134679 | 2.163 | 40588 | 94348 | 1.849 |
| >= 4 | 7179 | 0.115 | 3304 | 4662 | 0.092 |
| >= 3 with a queen | 55852 | 0.897 | 20323 | 41539 | 0.795 |
| a safe check, reading 2 | 2213250 | 35.543 | 135832 | 1710571 | 32.950 |
| >= 3, queen, safe check | 20348 | 0.327 | 10444 | 14413 | 0.279 |
| the same, two weak squares | 9733 | 0.156 | 5694 | 6632 | 0.130 |

On one king: 0 attackers 52.668 %, 1 36.364, 2 9.877, 3 1.033, 4 0.056, 5 0.002. Per side-row: incidences p90 3, p99 5, p99.9 7, max 14; safe checks
p90 1, p99 3, p99.9 5, max 9; weak squares p90 2, p99 4, max 8; flat units (attackers + incidences + safe checks + weak) p50 2, p90 7, p99 12, p99.9 16, max
29, and their White-minus-Black difference p90 6, p99 11, p99.9 15, max 29. **Every proposed input's half stands on at least 37097 games** (the d1 storm cell),
against DEC-326 (5)'s 1000: the group has no cell a tie would serve. The joint table, attackers by safe checks by weak squares, 80 cells: 20 under 1000 side-rows, 14 of them at attackers >= 4 (974 with no check and no weak square, the
thinnest 20), 3 at attackers 3 (831, 272, 927) and 3 at no attacker (200, 512, 21); no input of its own is among them. A curve of a linear sum has no per-cell parameter, so rows
per cell price the reading and not the fit. **S082's fixed set**: >= 3 attackers 20762 rows, 4092 games, 2.045 %; >= 3, queen, check 2890 rows, 1214 games, 0.285 %; safe queen checks on 11.8 % of its rows, weak squares on 55.7 %. The views, same grade: root 0.294 %, pv 0.472 %, the pool's qs training side 32505
rows against the corpus's 20348, its pv side 53538.

**What the labels read beside the shipped score.** At K 0.7119 (S101's zero-epoch K; head's evaluate asks 0.7147 of these rows and moves nothing below by more than 1.4), rows by the flat units' White-minus-Black difference, oriented to the side attacking more, |eval| < 300, whole-game errors (adocs/data/S122_scans.cpp), a reading and never a fit:

| difference | rows | gap, cp of the sigmoid | w of T = (w D)^2 / 4096, no taper | E[q]/D^2 | w per side, no taper | w per side, mg half alone |
|---|---|---|---|---|---|---|
| 5-6 | 432688 | 9.8 +/- 0.7 | 36 | 1.44 | 31.0 | 45.0 |
| 7-8 | 186635 | 17.6 +/- 1.0 | 36 | 1.31 | 31.8 | 48.7 |
| 9-10 | 69867 | 29.5 +/- 1.4 | 37 | 1.22 | 33.5 | 51.6 |
| 11-13 | 31542 | 41.2 +/- 2.3 | 34 | 1.15 | 32.9 | 51.2 |
| 14-17 | 4917 | 73.0 +/- 5.7 | 35 | 1.09 | 35.6 | 52.5 |
| 18-22 | 219 | 123.8 +/- 22.7 | 36 | 1.05 | 37.1 | 51.3 |

The difference form's w is level, 34 to 37, and that is the form the term would not have: read per side, q = u_max^2 - u_min^2, w climbs 31.0 to 37.1, and in the unit of the margin tables below, the mg half alone, it reads 45 to 52.5. Rows at a difference of 5 or more: 1211214, 19.45 %; 9 or more: 231654, 3.72 %; 23 or more: 17, 3 at |eval| < 300, nothing to read. **By phase the gap sits in the opening half**: at phase >= 16, 19.0, 45.6, 93.6, 131.7, 205.3, 249.7 on the same bins. **And it is not attackers' alone.** With safe checks and weak squares held equal, by |attackers + incidences| difference 1 to 8: -5.2, -5.0, -13.4, 0.4, -1.4, 4.3, 14.9, 35.1, flat to 6 and rising after it on 3265 and 968 rows. Nothing else is held there.

One count at a time, |eval| < 300, difference 1 to 5+: attackers -0.3, 9.8, 33.1, 79.5, 69.6 (50 rows); incidences -1.2, -0.4, 7.7, 13.9, 26.0; **safe checks 17.8, 31.6, 50.1, 73.3, 91.7, about linear and the largest unexplained signal, and nothing reads them today**: N 23.4, 17.8; B 15.9, 24.7; R 18.1, 17.7, 46.8; Q 23.0, 41.9, 51.8, 62.4, 53.1. Empty targets alone 18.1, 30.9, 49.0, 75.0. Weak squares 1.7, 8.8, 13.7, 14.7, 21.6; own minors -3.0, -3.5, -7.2, -14.1, the squares they cover -1.7, -3.0, -4.4, -7.6, -16.1; storm at d1 and d2 4.9, 12.9, 27.9.
Flat weights over these labels, the mg half alone, delta MSE against head at the margin 184, in 1e-6 (the scans; in sample, no tuner): one linear safe-check cell at 20, -114.1 +/- 4.0; T over all units, best flat w, -58.4 +/- 3.0 at w 48; T over attackers and incidences alone at w 48, -17.9 +/- 1.6. S101's parts read +0.6e-5 to +5.3e-5 held out, a different instrument and the nearest scale. The step's "a mating attack is worth four to six hundred centipawns" has no support inside the filter, where the labels stop at about 124 cp; past it they read 543 and 605 on 66 and 43 rows (below).

**Head's own games read about half of it, at their own K.** 12000 games from head's engine (`tools/datagen.cpp`, 100000 nodes a move, `--quiet-limit 30000`, seeds 2026110201 and 2026110202, the qs view, 1112728 rows; no corpus, no fit), split at today's 1000. The K the labels ask of those rows at head's evaluate is 0.7407, so they are read there. The grades agree: >= 3 attackers 1.989 % against 2.163, >= 3, queen, check 0.289 % against 0.327, 2970 rows from 1116 games; the clamp at 0.312 %. The labels, inside the filter, |eval| < 300:

| difference | rows | gap, head, K 0.7407 | w per side, mg half alone | gap, S083's corpus, K 0.7119 | w |
|---|---|---|---|---|---|
| 5-6 | 76048 | 3.2 +/- 1.7 | 26.4 | 9.8 | 45.0 |
| 7-8 | 33036 | 9.6 +/- 2.8 | 36.8 | 17.6 | 48.7 |
| 9-10 | 13041 | 12.0 +/- 4.5 | 34.1 | 29.5 | 51.6 |
| 11-13 | 5945 | 30.2 +/- 4.8 | 45.2 | 41.2 | 51.2 |
| 14-17 | 836 | 38.7 +/- 14.9 | 39.3 | 73.0 | 52.5 |
| 18-22 | 37 | 93.6 +/- 81.7 | 44.3 | 123.8 | 51.3 |

Safe checks 14.1, 22.7, 38.2, 42.2, 77.4 against 17.8, 31.6, 50.1, 73.3, 91.7, about 0.75; attackers -1.6, 3.6, 27.4, 36.6 against -0.3, 9.8, 33.1, 79.5; held equal, by attackers + incidences: 1.3, -4.7, -16.5, -3.7, -3.8, -13.8, -23.1, 73.8 +/- 36.1, no rise on these rows before 8. Flat weights, as above: one linear cell at 20, -73.1 +/- 11.1; T over all units, best w, -14.2 +/- 5.4 at w 32; T over attackers and incidences at w 48, +6.3 +/- 6.9. S083's rows were played by `5b0e170`, before S121, S123, S125 and S101, and head's weights were fitted to them, which would shrink their gap and not swell it, so the difference reads as who played and not as overfitting (inferred). Seed gradients at DEC-344's seed: `attN`, `attQ` and `inc` read -0.285 +/- 0.082, -0.261 +/- 0.063, -2.036 +/- 0.177 on S083's rows and +0.122 +/- 0.256, +0.120 +/- 0.230, +0.124 +/- 0.686 on head's: negative there, and on head's within about half an error of zero, so no sign is established on head's; `inc`'s shift is 3.0 errors, the other two 1.5 and 1.6. The checks' four hold their sign on both in the mg half, each at 3.8 errors or more.

**The filter.** `--quiet-limit` drops 83408 of those 1112728 rows, 7.50 %: rich in the grades (>= 3, queen, check 899 against 2970 inside, +30 %) but 96.8 % of the >= 3 rows there sit at |eval| >= 400, where the gradient is gone, and phase under 8 on 80.1 % of all of them. **The rows that matter are 1117, 0.10 % of all rows: |eval| < 300 with the search at 1000 or past**, and at K 0.7407 their gap runs 115, 131, 292, 234, 543, 605 cp at a difference of 1-2 to 11-13, on 358 rows down to 43. That is the failure mode the accepts names, in numbers: S083's corpus and S082's pool hold no such row, and getting them means new self-play. They are also selected on the search's verdict, so a fit over them is pulled by whatever tactic the search saw, the counts or not.

**The options, in machine hours** (S082's top-up 3.24 games/s, 11651 an hour; head's sample 9750 an hour, 3000 in 18 m 31 s and 9000 in 55 m 20 s beside niced builds):

| option | generated | adds | leaves unmeasured |
|---|---|---|---|
| S083's corpus as it stands | 0 h | | how much of the gap is the old engine's; rows past the filter |
| the pool's unsampled qs rows too | 0 h, fits about 1.9 times longer | 11690184 rows, >= 3, queen, check 32505 | S082: 8 rows a game beat 46 beat all, more rows of the same games do not fit better |
| reweight or oversample the graded rows | 0 h | nothing new | what it does to the noise; no held-out reading separates it |
| the pv view, on disk | 0 h | 0.470 % against 0.278 %; S082's qs reading 0.072297 against qs's 0.072154 | a recipe S082 did not choose |
| new self-play from head, `--quiet-limit 30000` | 150000 games, 12.9 h at S082's rate, 15.4 h at the sample's | the current engine, the tail | one family on a new corpus unless S126 refits on it |
| sampling the search's own evaluation calls | `tools/datagen.cpp` code plus the same hours | the search's distribution | everything: no such view exists |

All of it this project's own self-play; no other engine's output and Stockfish's evaluation is never a label. DEC-344 (1) takes the new self-play.

**The transfer gap.** S101's largest unknown does not carry to most of these. Attacker on move, S083's rows against `bench`'s lazy calls not in check (S122_CENSUS; 11.1 % of lazy calls are in check, 0 of a corpus's rows):

| count | corpus, on move, a side-row | search, on move, a side-call |
|---|---|---|
| attackers | 50.1 %, 0.595 | 46.6 %, 0.756 |
| incidences | 51.2 %, 0.968 | 48.6 %, 1.059 |
| weak squares | 51.5 %, 0.609 | 47.5 %, 0.424 |
| safe check N | 48.3 %, 0.033 | 53.7 %, 0.022 |
| safe check B | 47.8 %, 0.038 | 53.5 %, 0.067 |
| safe check R | 48.2 %, 0.117 | 52.5 %, 0.032 |
| safe check Q | 48.3 %, 0.101 | 49.5 %, 0.333 |
| **a safe check on a man** | **2.91 %**, 0.017 | **53.1 %**, 0.089 |

So one weight reads both cases but for the capture, which quiescence has resolved at the leaf: S101's gap in its own place. The levels differ, `bench` being eight positions and not a game (safe queen checks 3.3 times the corpus's, rook 0.27). On the labels the side on move carries a tempo-shaped offset of about 10 cp either way on every count, and past it safe checks still read more on move, 23 against 14 at one check, so the doubled cell is a lead; S136's tempo read H0 at -0.66, so a loss figure here prices nothing.

**The form, as this pass proposed it; DEC-344 rules otherwise at the order, the divisor, the seed and the counts of the audit, and they are the next section's.** The provenance is chesso's own (DEC-221). CPW *King Safety*, read 2026-11-02: the zone, attackers counted with their attacked squares, an attack count fed to a table, an S-curve, a queen contact check "safe"; **it has no quadratic, it never defines "safe", and "weak" and "defend" do not occur on the page**, so the step file's "weak squares near the enemy king" and DEC-258's "own minor pieces ... (CPW *King Safety*)" overstate it. The square is S262's group form and section 2's stated departure. *Square Control*: "controlled exclusively by the opponent side", no safe or weak square. *Attack and Defend Maps*: nothing on the king. No engine's source was read. Every definition below is chesso's own and is said to be:

- T(x) = max(0, x)^2 / 4096, per side and per half, White's king's less Black's, inside the safety pair's one taper, so under the clamp and no new taper.
- **Parts in screening order, earlier survivors live:** P1 `ks_core`, a constant, attackers N B R Q, incidences (6 inputs, 12 parameters); P2 weak zone squares, A's men attack and no man of K's but the king defends (1, 2); P3 zone squares K's minors cover (1, 2); P4 storm, seven linear cells in the slot's score, the closest enemy pawn on each of K's files at rank distance 1 to 4, free or blocked by K's pawn directly before it (d1 blocked means K's pawn beside K and the enemy pawn giving check: no corpus row is in check, so 0 rows, and `4k3/8/8/8/8/7p/6KP/8 w` reads 1) (7, 14); P5 safe checks N B R Q on empty squares (4, 8), reading 2: no man of K's but the king attacks the target, and if the king does, another class of A's attacks it too. **`PARAM_COUNT` 1121 -> 1159.** Then two parts that add no cell: P6, a mask, P1's counts over the zone plus its forward rank, section 4's geometry; P7, the shipped linear five at zero with the survivors refit.
- **The seed, DEC-134 form (b), and it is one number:** the constant at 64 in both halves, every other input at zero. x = 64 on every side of every row, so T_w = T_b and the term is exactly 0, while every input's gradient is 2c(n_w - n_b)/d. So phase 1 lands at `No functional change` with the group live and DEC-315's start the shipped evaluation, and no `derive_seed`.
- **The finalizer's constants.** The divisor is a gauge: (w, d) and (k w, k^2 d) are one T, so no fit can move it, and it is set from chesso's scale, the power of two at or above 4 x `QUEEN` = 3932, so one unit of x moves T by under a centipawn to a queen's worth, x reaching 2006 there. c is an ordinary input the tuner fits: positive gives a linear part, 2c u / d, negative a dead zone, as the page's sample table gives one attacker a weight of 0. S127 fits neither; DEC-258 moved this, and section 4's sentence is superseded.
- `feature_audit` audits at the seed, where the group's columns are n_w - n_b: the constant's two are zero and P1's ten duplicate the frozen linear five, so rank 1074 + 14 + 12 = **1100 of 1159, nullity 59**, the twelve named; expected, inferred, the run decides. 0 of 38 tied.
- **With S101: by target, DEC-333 (9).** S101 owns attacks on men, S122 squares and the king. A weak square may hold a man, 22.4 % of them. A safe check on a man, 5.7 % of them, is left to S101 and is the one transfer gap above.

**The cost**, forced-weight sink, every node count equal (S101's form):

| instructions, `bench` / `bench 16` | % |
|---|---|
| accumulator and finalizer over today's counts | +1.13 / +1.19 |
| the same over the zone plus its forward rank | +2.17 / +2.30 |
| weak squares, with the finalizer | +0.70 / +0.74 |
| defenders by piece, a test a minor | +1.65 / +1.73 |
| defenders by square, one popcount a side | +0.79 / +0.83 |
| storm, on a slot miss | +0.85 / +0.91 |
| safe checks, by type | +3.47 / +3.68 |
| accumulator and checks | +4.12 / +4.35 |
| accumulator, weak, by-piece defenders, storm, checks | **+6.16 / +6.50** |
| the same, checks gated on an attacked king | +5.83 / +5.92 |

Calls: 665368 `evaluate` + 2188446 lazy; stage two on 1730762; the shortcut skips 51.32 % of lazy calls. **One stage-two call: 1096 instructions at head, 1460 at all parts** (S101's pass read 743). Frame: S039's -4.42 % nodes, -2.79 % nps, -1.68 % wall with shortcut and clamp both out; S101 +4.68 % played.

**The clamp and the margin.** The shortcut, one tree, S122_CENSUS's count at each margin, and the first-order price of the extra calls:

| margin | skips, % of lazy | head's instructions | all parts' |
|---|---|---|---|
| 184 | 51.32 | | |
| 200 | 48.76 | +0.53 % | +0.66 % |
| 250 | 41.54 | +2.01 % | +2.53 % |
| 300 | 35.32 | +3.30 % | +4.13 % |
| 400 | 25.15 | +5.39 % | +6.76 % |
| 500 | 19.83 | +6.49 % | +8.14 % |
| 800 | 15.83 | +7.31 % | +9.17 % |
| out of reach | 0 | +10.57 % | +13.26 % |

**DEC-324's "at worst -2.79 %" is stale in direction only.** It is S039's nps with S039's -4.42 % nodes. Stage two is dearer now, 743 instructions a call by S101's pass and 1460 at all parts, 1.97 times, so the worst case has moved past it; but these rows are instructions and no nps was read, so nothing here replaces the figure. Real builds at each margin move the tree, `bench` 4263028 to 5840438 nodes and `bench 16` 9466622 to 17099433, so their instructions a node run -2.6 to +7.0 % and -6.7 to +10.5 %. At the margin 250 with all parts' sink, -2.2 and -6.5 % a node against first order's +2.53 %: **a first-order price carries no pick** (DEC-344 (4)).

The scored probe, a flat forced weight on every unit, inside the clamp, **the mg half alone, so that the w of this table is not the no-taper w of a gap**: `tools/eval_spread` over S083's corpus, each build's own check passing on every row, and `tools/truncation_scan`. A probe of reach, no seed:

| flat w | king safety p99, p99.9, max | combined p99, p99.9, max | bound at 184 | 200 | 250 | 300 | 400 | the >= 3, queen, check rows bound at 184, 250, 300 |
|---|---|---|---|---|---|---|---|---|
| head, no T | 104, 158, 341 | 156, 225, 415 | 0.400 % | 0.236 | 0.041 | 0.006 | 9 rows | |
| 32 | 113, 178, 416 | 164, 241, 487 | 0.551 % | 0.342 | 0.077 | 0.016 | 0.001 | 18.8, 5.4, 1.6 % |
| 48 | 127, 208, 584 | 176, 266, 649 | 0.806 % | 0.533 | 0.147 | 0.042 | 0.004 | 30.0, 11.7, 4.9 % |
| 64 | 151, 257, 838 | 196, 308, 903 | 1.278 % | 0.905 | 0.316 | 0.116 | 0.018 | 45.7, 23.2, 12.7 % |
| 96 | 228, 413, 1565 | 265, 454, 1631 | 3.149 % | 2.466 | 1.211 | 0.625 | 0.183 | |

**Can it fit under 184? In the row share, yes; for the rows it is for, no.** In this table's unit the labels imply w 45 to 52 on S083's rows and 26 to 45 on head's, and the flat scan's minimum sits at w 48 on S083's and w 32 to 36 on head's. At w 48, 0.806 % of rows bound at 184, but 30.0 % of the >= 3, queen, check rows, 24.67 % of their gradient mass (sigma(1 - sigma) at the unclamped score; 26.86 % at the score the clamp leaves); at w 32, 18.8 % and 15.17 %. Head at T = 0 is already 11.24 % of those rows and 8.80 % of their mass on S083's, 10.24 % and 8.56 % on the sample's. The row share hides that the blind rows are the term's own, and those rows give the fit nothing, in either direction.

**How the fit and the margin stay consistent.** No gradient passes the margin, so a bound row neither pulls T up nor reins it back; re-deriving after a fit at 184 would unbind rows whose scores no loss saw, and a screen at 184 reads a part with its best rows blind. The proposal keeps DEC-324's order: one fit of all parts under 184, `eval_spread`; if the spread meets the share nothing moves and the margin owes no run; otherwise the smallest multiple of 25 that meets it, refit from the same start under that margin (a tuner build whose clamp and identity are at it), read again, to a fixed point in at most three rounds, then stop. The screen and the candidate run at that margin, P0 reads the margin alone at the shipped constants offline, and a fresh `truncation_scan` (DEC-169) closes it. Fit and engine are then one function on every row, and the only blind rows are the share's. **The share, as this pass proposed it:** at most 0.5 % of rows and at most 10 % of the >= 3, queen, check rows; its readings were in the wrong unit and DEC-344 (4) replaces the second bar with gradient mass, which T = 0 passes at 184 so that a term adding nothing moves nothing. Read in that bar, the scans' margin table: S083's rows at w 48 pass at 250 (8.55 %), head's at w 32 at 225 (6.15 %), S083's at w 64 at 300 (8.76 %).

**Hazards, read.**
- **The ordering bands: untouched and cannot be.** `src/evaluation.cpp` `score_move` reads `piece_values_abs`, built from the fixed `MVV_` macros, the `ORDER_` bands and the history sum; no fitted value, no evaluation. 900100 against 900000, clearance 100, unmoved, and `tests/test_evaluation.cpp` "a king capturing a pawn outranks a killer at the same ply" holds it. `see_value` is its own table. S122's magnitudes reach the search through the eval-fed margins (reverse futility, futility, the null move's static test, aspiration), which is S127's and is no band.
- `tests/test_eval_trace.cpp` "the trace reconstructs evaluate() exactly, clamp included": its identity is blind to a group until `exact_parts` reads it, as the prototype does; the precondition gains a row with a non-zero group.
- `tests/test_tuner_gradient.cpp` "the clamp's derivative is exact where it binds": `clamped_site` covers the feature table only. With the group inside, the case asserts it silent on a bound row, a plant seen red: the 2026-11-02 note's extension binds. `gradient`'s group loop gains the gate; the prototype leaves it, no fit reading it.
- **The mate floors, OWNER by DEC-299, DEC-303, DEC-330:** `MATE_IN_THREE_FLOOR` 11 at 12 shipping, 10 weakened; `EXACT_FLOOR` 157 at 158 and 154 (DEC-343). S101 moved `EXACT_FLOOR`'s row count -10 at depth 10 and + at 11 and 12; this term raises the attacker's static score in exactly those lines, so expect a re-read, in DEC-334's form: the SPRT plays the staged tree and the owner rules only on H1. DEC-316's pair and the gradient's bound: a third recurrence reopens the floored metric, the owner's (DEC-343).
- **Goldens that move**, S101's table as the list, each by its script: the anchors (`S192_anchors.py`), `truncation_positions`, the coverage blocks, the fixture and its clear-of-the-clamp reading, `CLAMP_PUSH`, the gauge walk and `apart`, the gradient case, both floors, the node budget, first-mate and `DRIVE_DEPTH`, `capture_mates`, `mate_probcut_hides`, `LATE_CAPTURE_*`, the aborted-line cell, `ALPHA`, `short_line_ceiling`, S170's budgets. New, if the curve survives: the bound 3.875 where 2.875 was (below), a packed bound per array, `tools/eval_spread_margins.hpp` gaining 225 and 275, `adocs/specs.md`'s two rows. `src/search_params.hpp`'s margin row and `tests/test_eval_spread.cpp` if the margin moves.

**Truncation.** The two per-side divisions add under 1 to the safety taper, so the arithmetic bound goes 3 x 23/24 = 2.875 to 3.875 while tempo is zero. The scans at forced w read worst 3.07, 3.28 (3.29 at a margin of 300), 2.875 and 3.11 at w 32, 48, 64, 96, 0 rows past 3.3, and 105374 to 148030 rows past 2.0 against head's 98987. w 64 reads 2.875 because T is then the integer u^2, so flats understate it and a fit's arbitrary weights will reach further. One division over the whole numerator would hold it near 2.92 and needs int64: not proposed, the residual being under a centipawn. `truncation_positions` and the comments stating 69/24 follow their script.

**Unmeasured or inferred, flagged.** No fit ran: every "the fit would" is inferred, and whether Adam at lr 1 from the seed leaves the linear regime inside the patience window is phase 1's first check. The label gap sits on the shipped linear five and on a corpus these weights were fitted to. A flat scan is in sample and over labels, not a held-out fit. `bench` is eight positions. No part's Elo, no nps. x-rays, pins and exchanges are in no count. Rank and nullity are expected and not run: the audit needs the group first. Long control: Ethereal's two rows shrink at 60+0.6, S267 reads it.

## 2026-11-02: what the fast check corrected, and the ruling

The coordinator's fast check (`.tuning/coord/S122/fastcheck/`, `an1` to `an7`) re-read this pass's labels per side and by whole game, and DEC-344 rules on them: **head's own games are the corpus, the linear parts come first and the curve last, d is 2048 and c is 45, and the margin's second bar reads gradient mass.** The ruling and its record are the coordinator's. Every finding below was re-read before it was written: the scans DEC-344's Context and (4) rest on are now `adocs/data/S122_census.sh scans` (adocs/data/S122_scans.cpp, S122_scan_dump.cpp, S122_scan_rec.hpp) with whole-game errors on every delta and gap, and the plants are `plants`; both print in adocs/data/S122_census.txt. No fit, match or timing ran, and no game was generated.

**What stood.** The census's counts, held to the engine's on every row: the corpus holds the attacking rows the accepts asks for, by count, and every input's half at least 37097 games. The group is outside the clamp and `exact_score` does not read it, and the prototype's integer finalizer inside the clamp reproduces `evaluate` on every row. The instruction rows, sink builds with node counts equal; the shortcut's skip shares; the probes' clamp shares. The transfer table, with its one gap on a check that captures. CPW has no definition of "safe", and "weak" and "defend" are not on the page. Safe checks carry the largest signal, on both datasets. The order of DEC-324's procedure, fit under the kept clamp and re-derive to a fixed point.

**What fell, each confirmed here.**

1. **"One flat w reads every bin."** That is the difference form, (w D)^2, and T acts per side: E[q]/D^2 runs 1.44 to 1.05 and w 31.0, 31.8, 33.5, 32.9, 35.6, 37.1. With safe checks and weak squares held equal, the gap by |attackers + incidences| 1 to 8 is -5.2 -5.0 -13.4 0.4 -1.4 4.3 14.9 35.1, while the checks run about linear one way. So most of the bend the proposal rested on reads as the checks and the phase and not as attackers (inferred, the two not separated by any fit): at phase >= 16 the gap still reads 19.0, 45.6, 93.6, 131.7, 205.3, 249.7. The curve is not disproved, and P5 reads it.
2. **"Head's games at about 0.6."** That was read at S083's K. At the sample's own K 0.7407 the gaps are 3.2 9.6 12.0 30.2 38.7 93.6, about 0.5.
3. **"w 36" and "w 28" in the margin paragraphs.** The margin tables' w is the mg half alone, where the gap implies 45 to 52.5 on S083's rows and the flat scan's minimum sits at w 48, and 32 to 36 on head's. The row share of graded rows at T = 0 is 11.24 % at 184 and their gradient mass 8.80 %.
4. **"Both tools' identity checks."** They are not one check twice. Plants on the trace's count, the exact score's divisor and its sign turned `tools/truncation_scan` red and left `tools/eval_spread` green; a T missing from the collecting instantiation does the reverse; a wrong count in the engine passes both.
5. **"d1 cannot be blocked."** It holds where no king is in check, so on every corpus row: 0 of 6226944, and 0 of those rows are in check. `4k3/8/8/8/8/7p/6KP/8 w` reads d1 blocked 1, and the pawn gives the check.
6. **The script.** `read` raised FileNotFoundError on a fresh OUT, no stage writing head's spread or truncation, and `census`'s bare `wait` swallowed a failed census. A `head` stage writes them, each census's status is read, a `count_row` that got no count fails, and a sink row that moves a node fails `cost`. `read` runs clean from the stages' own outputs.
7. **False sentences, corrected above:** "under 1000 side-rows only where attackers >= 4" (20 of the 80 cells, 3 at attackers 3 and 3 at none, and 974 at attackers 4+ with no check and no weak square); "more than twice as dear" (1460/743 is 1.97); "from the description" for a quadratic the page does not have; and "one attacker has weight 0" in quotation marks, which paraphrases the sample table's row 1 : 0.
8. **DEC-324's -2.79 %** compared nps with instructions and is stale in direction only. At the margin 250 real builds read -2.2 and -6.5 % a node, all parts' sink live, against first order's +2.53 %: a first-order price carries no pick.

**What the re-reading found beyond those.**

- The research section's "123351 to 148030 rows past 2.0" missed w 64: it is 105374 to 148030. Corrected.
- **Seed gradients.** S083's `attN`, `attQ`, `inc` read -0.285 +/- 0.082, -0.261 +/- 0.063, -2.036 +/- 0.177 and head's +0.122 +/- 0.256, +0.120 +/- 0.230, +0.124 +/- 0.686, at DEC-344's seed in 1e-7, mg half. The signs differ, and head's three sit within about half an error of zero: what is established is that S083's gradients are not head's, `inc` by 3.0 errors, the other two by 1.5 and 1.6, and not that head's point the other way. The fast check's reading carried no errors.
- **P1's curve over linear cells.** -3.3 is cell a 30, b 4, w 32, +/- 0.8; +2.4 reads +2.5 paired and is another cell, a 20, b 4, w 24, +/- 1.7. The same cells on both datasets: a 30, b 4, w 32 reads -3.3 +/- 0.8 and +7.1 +/- 3.0; a 20, b 4, w 24 reads -3.4 +/- 0.4 and +2.5 +/- 1.7. Over each dataset's own best linear cell on the grid: S083's a 25, b 8 reads at best -1.2 +/- 0.4 at w 24, and head's a 20, b 2 is never negative, +0.1 +/- 0.2 at w 8. The direction stands.
- S083's best P1 grid point on head's rows is +8.4 +/- 8.1: not better than nothing and not shown worse. T over attackers and incidences at w 48 likewise, +6.3 +/- 6.9.
- **P1's own count.** The Context table's linear row counts men as well. On empty targets alone, DEC-344 (3)'s count, at 20: -108.1 +/- 4.1 and -69.2 +/- 11.1, against -114.1 +/- 4.0 and -73.1 +/- 11.1.
- **The K.** S083's rows are read at 0.7119, S101's zero-epoch K at a tree before S101's family; head's evaluate asks 0.7147 of them. There the table reads -113.7, -57.0, -17.2, -9.6, within 1.4 of 0.7119's. The sample at S083's K would read -79.3, -22.3 at w 40, -7.9.
- **The second bar's reading of mass.** Sigma(1 - sigma) at the unclamped score, 8.80 % on S083's rows and 8.56 % on the sample's at T = 0; at the score the clamp leaves, 9.41 % and 9.13 %. Both pass 10 %, by under 1.5 points, and phase 1's tool should say which it reads.

**The ruling, DEC-344.** The corpus is 150000 games of head's self-play, generating as this is written, the filter applied offline. The parts: P0 the margin alone, then linear safe checks on empty targets, weak squares, defenders by square and seven storm cells, then the curve over what survives and only if it earns, then the wide zone and the linear five's retirement only if it does. T(x) = max(0, x)^2 / 2048, c seeded at 45, at most 50 new parameters, `PARAM_COUNT` 1121 -> 1171, at the seed rank 1100 and nullity 71, decided by the run. The margin passes at most 0.5 % of rows and at most 10 % of the graded set's gradient mass, at the fit's own weights. The research section's T, parts, seed, counts and share are what it proposed, and they stand there as that.
