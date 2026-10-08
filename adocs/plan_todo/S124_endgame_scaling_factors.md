id:         S124
goal:       the endgame half of the score is scaled toward a draw by what is actually on the board, and won lone-king and specialised endgames are scored so the stronger side converts them
accepts:    an SPRT verdict, recorded whatever it is; the factor scales **only the endgame half** of the tapered score and not the whole score; each case -- strong-side pawn count, opposite-coloured bishops, and the mop-up and specialised-endgame cases this family gains (DEC-258) -- is screened separately before the family's one SPRT (DEC-259), and a case the corpus barely holds is screened on a tool-labelled position set instead (tablebase WDL or Stockfish through python-chess, never the agent's reading, CHESS rule), because the surveyed record shows several such cases measuring zero and being removed again; mop-up is shown to convert: won lone-king positions, labelled won by a tablebase, are mated by the engine in self-play at a fixed node budget within a move bound stated before the run; every constant is fitted (DEC-084); the existing insufficient-material draw detection is not duplicated by this and the step says how the two divide
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp
excludes:   tablebases, which are S129; the 50-move decay, which is a separate small term in the reserve; the complexity term, which is S266
decisions:  DEC-071, DEC-084, DEC-258, DEC-259
closes:
blocks:
paused_by:
done:

## Amended 2026-10-08, DEC-259: screened offline, one SPRT for the family

**Scope gained 2026-10-08 (DEC-258):** mop-up -- the defending lone king
driven toward the edge, or toward a corner of the bishop's colour for king,
bishop and knight against king, the attacking king brought closer (CPW
*Mop-up Evaluation*) -- and specialised endgames beyond the two scaling
cases, each from a description (Grant 2020's scale factor for
opposite-coloured bishops and unwinnable material; Stash's changelog v26,
v30, v31, +3 to +9 a case). DEC-138 had deferred these on the premise that a
network would supersede them; DEC-179 removed it.

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
S262 has landed; the attack sets come from S263. S039 runs before this family
and retires the lazy shortcut and the clamp; if S039 read H0 instead, the
owner's amendment of that outcome binds here.

## Why this is high on the evaluation block

It is a group every surveyed engine ships and chesso ships **none** of, it is
cheap, and it is low-risk. That coverage gap is the whole argument: the
2026-09-04 literature check confirmed that Ethereal, Berserk, Weiss and Stash
all have endgame scaling and that chesso has none of it, and CPW has no page
pricing the group. **Four Elo figures this paragraph used to open on carried no
source through two searches and are deleted (DEC-203)** -- one for a
drawish-endgame factor, two for scaling by strong-side pawn count and one for
the change from scaling the whole score to scaling only the endgame half. What
replaces them is traced and much smaller: **+1.09 to +3.94** for the additions
this step builds (section 1). The order this step sits in never rested on the
deleted numbers; it rests on the gap, and the expected size is single digits.

The warning is the shape of the tail. One engine measured **+0.94, +0.29,
+0.18 and +0.11** for further opposite-bishop special cases and **removed them
as neutral simplifications** -- three removals, all traced with URLs in
section 1, over 35 to 80 thousand games each. The numbers are quoted for their
*shape*, which is the point being made: the tail of this group measures at the
resolution limit of any harness this project can afford. Build the two that
pay, measure, and stop -- do not build the matrix.

`specs.md` records that the endgame is the cheapest phase per move in chesso's
own error profile and that DEC-033 found endgame errors the least
depth-fixable. That is an argument for this step, not against it: what is least
fixable by searching deeper is exactly what has to be fixed by knowing more.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The group has no wiki page that prices it and the 2026-09-04 check said so;
that is unchanged.** Searched this pass: CPW has no page describing endgame
scale factors as an evaluation technique with a figure -- the material-draw
material is under *Material* and carries no number, which the review recorded.
The definitions this step implements therefore come from the engine record and
are read for form only.

**The tail figures are all four traced, and the tail is three removals, not
four cases.** Ethereal commit messages, reachable through
https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+Remove+SCALE
and https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+scaling
(fetched 2026-09-13). Commit messages only.

| this file quoted | traced to | figure as the message states it |
|---|---|---|
| +0.94 | commit 8db27ad52de3e0d029d3a122b4f22e17d099ce37, 2018-07-18, "Remove SCALE_OCB_TWO_KNIGHTS (#61)" | **+0.94 +/- 2.36** at 10.0+0.1s Hash 8MB, 35725 games |
| +0.29 | commit b314d7a037679ae8b0e513016218af12f1d8e007, 2018-08-30, "Remove SCALE_OCB_GENERAL (#71)" | **+0.29 +/- 1.83** at 10.0+0.1s, 59090 games; **+2.37 +/- 2.80** at 60.0+0.6s, 20200 games |
| +0.18 and +0.11 | commit 874e509ea284520c450159d3924ce09e2cc453a2, 2018-08-26, "Remove scaling for OCB with 2 Rooks" | **+0.18 +/- 1.71** short, 66670 games; **+0.11 +/- 1.39** long, 79950 games |

**Correction to this file's wording, from the trace.** The paragraph above used
to name a rating band and "four further opposite-bishop special cases", and
both halves of that were wrong. There are **three** removals, not
four: +0.18 and +0.11 are the short and long halves of the same one. And the
rating band had no source -- these are mid-2018 Ethereal commits and no fetched
source gives that engine's rating at the time -- so the band is deleted
(DEC-203) while the four numbers stay with their URLs. What the row establishes
is unchanged and is the point the file makes: every one of them sits inside its
own error bar, over 35 to 80 thousand games, at an engine that could afford
them.

**The additions this step builds are traced too, and they are an order of
magnitude below the figures this file used to quote for them.**

| what | figure | source |
|---|---|---|
| Ethereal "Endgame scaling for pawns (#176)", 2021-04-18 -- the strong-side pawn-count case | **+3.80 +/- 2.98** at 10.0+0.1s, 18836 games; **+3.83 +/- 2.76** at 60.0+0.6s, 14772 games | commit d62b0008aaef15bd2f86fdb73358d40ca20148fb |
| Ethereal "Scale eval to draw with a lone minor (#96)", 2019-10-31 | **+2.53 +/- 1.96** at 10.0+0.1s, 49225 games; **+1.70 +/- 1.15** at 60.0+0.6s, 106150 games | commit 725308a1bdf5852d8607ffaf74f5643766b4c30c |
| Ethereal "Evaluate OCB endgames using a scaling factor", 2018-07-10 | **+1.09 +/- 1.65** at 10.0+0.1s; **+2.47 +/- 1.95** at 60.0+0.6s | commit b4b24f48f8605417fcfaff8c06d890609476b2d4 |
| Stash v26, "Added a scaling function for endgames" | **3.94 +/- 3.09** at 60.0+0.6s Hash 64MB, bounds [0.00, 5.00], 20192 games | https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md |

**So the four figures this file used to open on are deleted (DEC-203), and the
traced replacements are three to four times smaller.** Searched 2026-09-04 by
the plan review: not located, and CPW has no page pricing the group. Searched
this pass: the two Ethereal commit searches above, the Stash changelog scanned
for each of them (none occurs), and Berserk's scale-related pull requests
(https://api.github.com/search/issues?q=repo:jhonnold/berserk+scale+type:pr --
#271 "Eval Scaling", #274 "Phased eval scaling", #283, #484 "Scale eval on
FMR" at +1.98 +/- 1.58 / +1.45 +/- 1.05; none of the four numbers). Not
located.

**What this does to the step's own argument, stated plainly.** This file used
to open by calling the group "the largest evaluation group in the surveyed
record that chesso has **none** of". The traced record does **not** support the
"largest" half: the traced additions are +1.09 to +3.94, below the Stash ledger
entries that order block 3 (mobility +19.95, passers +22.27, connected pawns
+25.38), and that clause is rewritten above. What the order actually rests on
is untouched -- the **coverage gap**: all four surveyed engines ship endgame
scaling and chesso ships none, which the 2026-09-04 check confirmed and which
needs no number. The step keeps its place on the gap and on the cheapness, and
its expected size is single digits at best. That is an input to the bounds, not to the order; `adocs/plan.md`'s
block-3 sentence gives S124 no figure and so needs no change.

### 2. Shape for chesso

**Departure from the record, stated and load-bearing.** The accepts requires
the factor to scale **only the endgame half** of the tapered score. Every
traced record above scales in its own engine's framing and none of the
messages states which half it multiplies, so the figure this file used to quote
for "the specific change from scaling the whole score to scaling only the
endgame half" had no source and the design choice is chesso's own. It is the right one
here for a mechanical reason: `src/evaluation.cpp` `evaluate_cheap` divides the
middlegame and endgame sums by phase, so a factor on the whole score would
scale a middlegame evaluation toward a draw in a position with a queen on the
board.

The order the accepts fixes -- strong-side pawn count first, opposite-coloured
bishops second -- now matches the traced sizes: +3.80 for the pawn-count case
against +1.09 for the OCB case.

### 3. Implementation sketch

- The factor is applied to the endgame summand before the taper division, and
  `tools/eval_model.hpp` carries the same arithmetic or the tuner fits a
  different function.
- The existing insufficient-material draw detection and this factor must
  divide cleanly: one returns a draw score, the other scales. The step states
  the boundary, per the accepts.
- One case at a time with a verdict each. The record is the argument: three of
  the OCB cases were removed again as neutral, over 35 to 80 thousand games.

### 4. Constants and seeds

**No seeds from any engine.** The scale factors are fitted (DEC-084 and the
accepts). No engine's scale-factor constants seed anything here wherever they
are republished (DEC-105, DEC-134), and no publication about the technique
gives a value, so form (a) is unavailable by fact and not by rule.

Where a starting value is needed it is **(c) the midpoint of a range declared
by purpose, stated as such**: a scale factor multiplies the endgame half, so
its range is 0 (a dead draw) to 1 (unscaled), and the midpoint is 1/2 expressed
in whatever fixed-point denominator the step picks. The denominator itself is
**(b) derived** -- chosen so the factor divides exactly in integers against
`GAME_PHASE_MAX`, and the step writes the arithmetic out.

### 5. Pitfalls

- **The tail measures at the resolution limit.** Three removals at +0.94,
  +0.29 and +0.18 over tens of thousands of games is the record saying that
  the matrix of special cases is not measurable by any harness this project
  can afford. Build the two that pay, measure, stop.
- **A bundle hides a neutral case.** The accepts forbids one, and the traced
  removals are why.
- **Scaling the whole score is a real bug, not a variant.** See section 2.
- **The draw detector and the factor can disagree.** A position the detector
  calls a draw and the factor scales to 40 % of a rook is a contradiction the
  step must not ship.
- **Long control does not rescue this group.** Two of the three traced
  additions are flat or lower at 60.0+0.6s.

### 6. Measurement

One SPRT per case at the S105 regime, bounds stated in advance with the nElo
worst case and the abort rule (DEC-143) -- and sized on **single digits**, not
on the deleted double-digit class. A verdict of zero is recorded as zero and the case
is not kept "because the engines have it"; the coverage argument justifies
*trying* it, never *keeping* it.

### 7. Interactions

- **S129 (block 4, optional)**: tablebases answer the same endgames exactly.
  The step states how the two divide if S129 ever lands.
- **S126 (block end)**: refits everything, including these factors.
- **S039 (before)**: the clamp is on the expensive stage; a scale factor on
  the endgame half is a cheap-stage change and the step says which stage it
  lands in.
- **DEC-033 and `adocs/specs.md`**: the endgame is the cheapest phase per move
  in chesso's own error profile and endgame errors were the least
  depth-fixable, which is the argument for the step and is local evidence, not
  literature.

### 8. References

- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+Remove+SCALE
  -- commits 8db27ad5 (+0.94 +/- 2.36), b314d7a0 (+0.29 +/- 1.83 / +2.37 +/-
  2.80), 874e509e (+0.18 +/- 1.71 / +0.11 +/- 1.39). Commit messages only.
  Fetched 2026-09-13.
- - https://api.github.com/search/commits?q=repo:AndyGrant/Ethereal+scaling --
  commits d62b0008 (#176, +3.80 +/- 2.98 / +3.83 +/- 2.76), 725308a1 (#96,
  +2.53 +/- 1.96 / +1.70 +/- 1.15), b4b24f48 (OCB scale factor, +1.09 +/- 1.65
  / +2.47 +/- 1.95). Commit messages only. Fetched 2026-09-13.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v26
  "Added a scaling function for endgames", 3.94 +/- 3.09 at 60.0+0.6s, bounds
  [0.00, 5.00], 20192 games; the entry also records that a material-key version
  and a material-quantity version were both tried and the simpler one kept.
  Changelog entries only; scanned 2026-09-13 for the four figures this file
  used to open on, none of which occurs.
- - https://api.github.com/search/issues?q=repo:jhonnold/berserk+scale+type:pr
  -- Berserk's scale-related pull requests; #484 "Scale eval on FMR"
  +1.98 +/- 1.58 / +1.45 +/- 1.05. Pull-request bodies only. Fetched
  2026-09-13.
