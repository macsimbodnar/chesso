id:         S124
goal:       the endgame half of the score is scaled toward a draw by what is actually on the board, and won lone-king and specialised endgames are scored so the stronger side converts them
accepts:    an SPRT verdict, recorded whatever it is; the factor scales **only the endgame half** of the tapered score and not the whole score; each case -- strong-side pawn count, opposite-coloured bishops, and the mop-up and specialised-endgame cases this family gains (DEC-258) -- is screened separately before the family's one SPRT (DEC-259), and a case the corpus barely holds is screened on a tool-labelled position set instead (tablebase WDL or Stockfish through python-chess, never the agent's reading, CHESS rule), because the surveyed record shows several such cases measuring zero and being removed again; mop-up is shown to convert: won lone-king positions, labelled won by a tablebase, are mated by the engine in self-play at a fixed node budget within a move bound stated before the run; every constant is fitted (DEC-084); the existing insufficient-material draw detection is not duplicated by this and the step says how the two divide
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/tuner_trace.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp, tests/test_eval_positions.hpp
excludes:   tablebases, which are S129; the 50-move decay, which is a separate small term in the reserve; the complexity term, which is S266
decisions:  DEC-071, DEC-084, DEC-258, DEC-259
closes:
blocks:
paused_by:
author:     the coordinator; phase 0 (research pass) by an Opus 5.5 subagent, briefed by the coordinator, 2026-11-03
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
S262 has landed. S263 built no per-side attack set, since no term read one;
a term here that needs one builds it once per evaluation. S039 read H0 (record `ca578de`,
revert `74d0adf`): the lazy shortcut and the clamp are kept. S122's design
waits on the owner's amendment before S121 starts (DEC-258), and that
amendment binds here.

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
here for a mechanical reason: `src/evaluation.cpp` `cheap_score` divides the
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

## 2026-11-03: the research pass

Read at the stacked tree `88c6c7a` (HEAD `1bb3080` with the seven fillers S324 to S330 staged), bench 5482277
reproduced: counts, tablebase probes and instructions only, no fit, no match, no timing, nothing under `src/` or
`tools/` changed. Every figure's source is adocs/data/S124_census.txt, by adocs/data/S124_census.sh
(adocs/data/S124_census.cpp, S124_tb_probe.py, S124_twofold_check.py, S124_conversion.py), and adocs/data/S124_cost.txt, by
adocs/data/S124_cost.sh (S124_cost_sink.patch). The census reads the FEN, phase, game and ply columns; the result
column only of S122's prepared training corpus (`.tuning/s122/prepared/corpus.tsv`, 6230857 rows, 137994 games) at its
own K 0.7433, and of the raw qs pool's training games into the tablebase dump. S082's three fixed views of that corpus
(qs, root, pv, 12000 held-out games) are read for counts and their labels are unread; their game ids are read from the
qs view's game column alone. Every "won" or "drawn" below is the Syzygy 3-4-5 tables' through `python-chess`
(TOOLCHAIN.md's tablebase section), on training rows only, **at the row's own halfmove clock**: `probe_wdl` assumes a
zeroed clock and the games ran under the fifty-move rule, so where it reads 2 or -2, `probe_dtz`'s n decides, a win
only when clock + n + 1 <= 100 (python-chess documents that a returned n can mean n + 1 plies, DTZ50'' rounding),
drawn when clock + n > 100, and the rows at exactly 100 counted apart and read in no label. No position was judged
(DEC-023). Revised 2026-11-03 after the coordinator's fast check: the readings below were first taken at a zeroed
clock, games were classified by their qs leaves, the truncation bound and the cell's staleness in a refit were
misstated, and the datagen fix was named wrongly; each is corrected in place. **Two
departures from a reading-only pass, stated:** the raw pools `.tuning/s122/corpus/pool.tsv.qs` and `.root` were read
in place as well, for the rows past the 1000 filter and for the root rows games are classified by; and
adocs/data/S124_twofold_check.py ran about 2600 searches of
head's engine at 100000 nodes -- a census of what the search plays, no game, no timing. The proposal is
`.tuning/coord/S124/decision_draft.md`.

**What exists.** `src/evaluation.cpp` `cheap_score`: `taper` over `board->psqt` plus the pawn pair, one truncating
division by `GAME_PHASE_MAX`, then `board->material` added **untapered**, then tempo's own taper, negated for Black.
Stage two (`evaluate_mobility_and_king_safety`) tapers mobility with threats and king safety with its curve apart,
and `expensive_score` clamps their sum. So chesso has no stored "endgame half": stage one's is `eg_value(psqt +
pawns)`, stage two's are its two pairs' endgame values, and the material is in neither. **That is load-bearing for
the accepts' "scales only the endgame half".** `tools/tuner_groups.hpp` `gauges` holds five material/table directions
null: c onto `piece_value[t]` and c off every square of `psqt_mg[t]` and `psqt_eg[t]` is the same evaluation. Scale
`eg_value(psqt + pawns)` alone and those five stop being null on every scaled row, so the factor would scale an
arbitrary share of a piece's worth (the queen's endgame squares read -234 to -11 since DEC-307's re-anchor) and
`hold_gauges` would pin directions the loss now sees. Scale E1 = `board->material` + `eg_value(psqt + pawns)` and
both halves of every gauge move together: E1 is invariant. **The endgame half the factor scales is E1, material in
it.** What the cases read, all present: `board->bitboards` per piece (ten popcounts give every count a case needs),
`one_colour` (the bad-bishop columns' square colour, so bishop colours are two ANDs), the king squares, `board->phase`
(accumulated, both sides summed), and `src/bitboard.cpp` `is_insufficient_material` (king against king, or a lone
minor against a bare king). Per-side material is not accumulated -- `board->material` and `board->phase` are sums
over both sides -- so a case reads popcounts of bitboards the board already holds; no accumulator is rebuilt and no
piece walked, INV-4 is untouched. **The cost**, adocs/data/S124_cost.txt, the case, its cell and the scaled taper at
forced cells into an asm barrier in stage one, the score the landing's, node counts equal (5482277, 14931387), five
interleaved `tools/perf_counters` runs, spread under 320 instructions:

| sink build, instructions | `bench` | `bench 16` |
|---|---|---|
| eager: every count at every call | +2.554 % | +2.557 % |
| gated: past two pawns a side with a knight or a queen on the board, or not one bishop a side, nothing more read | +1.316 % | +1.264 % |

Stage one runs at every lazy call, the shortcut's included, which is where the cost sits; no cycle, wall or nps
was read. A gate on an accumulated per-side count, or a byte in the pawn entry, is cheaper and unmeasured.

**The cases, by counts** (adocs/data/S124_census.cpp; the first that holds). E1 > 0 makes White the strong side; P
is the strong side's pawns; d its men's phase units (knight and bishop 1, rook 2, queen 4, `phase_value`) less the
other's, so no count moves with a refit; which side is strong, E1's sign, does, and the form below
traces both candidate cells for that reason. DEAD is the detector's; LONE a king alone; OCB one bishop a side on
opposite colours, PURE with no knight, rook or queen, ROOK with one rook each and nothing else; PAWNS the rest with a
strong side. S122's corpus, rows and games; the views and S083's corpus as a share of rows:

| case or proposed cell | rows | % | games | qs view % | root % | pv % | S083 % |
|---|---|---|---|---|---|---|---|
| DEAD | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| LONE | 13274 | 0.213 | 3500 | 0.405 | 0.349 | 1.353 | 0.189 |
| of it mop-up's domain | 9742 | 0.156 | 2087 | 0.311 | 0.281 | 0.596 | 0.112 |
| of that, KBN | 1954 | 0.031 | 212 | 0.050 | 0.047 | 0.068 | 0.022 |
| OCB_PURE | 28600 | 0.459 | 2624 | 0.550 | 0.510 | 0.790 | 0.441 |
| OCB_ROOK | 57432 | 0.922 | 5228 | 1.175 | 1.122 | 1.444 | 0.945 |
| OCB_OTHER | 289610 | 4.648 | 28012 | 4.962 | 4.897 | 5.710 | 4.968 |
| PAWNS P=0 d<=0 | 8733 | 0.140 | 1957 | 0.204 | 0.190 | 0.287 | 0.111 |
| PAWNS P=0 d=1 | 24090 | 0.387 | 3176 | 0.670 | 0.627 | 0.973 | 0.381 |
| PAWNS P=0 d>=2 | 14567 | 0.234 | 2840 | 0.346 | 0.339 | 0.553 | 0.237 |
| PAWNS P=1 d<=0 | 58333 | 0.936 | 10568 | 1.297 | 1.185 | 1.982 | 0.821 |
| PAWNS P=1 d=1 | 54954 | 0.882 | 9253 | 1.253 | 1.166 | 1.780 | 1.013 |
| PAWNS P=1 d>=2 | 20724 | 0.333 | 4649 | 0.410 | 0.390 | 0.748 | 0.432 |
| PAWNS P=2 d<=0 | 139857 | 2.245 | 24868 | 2.930 | 2.695 | 3.995 | 2.092 |
| PAWNS P=2 d=1 | 102488 | 1.645 | 18196 | 2.140 | 1.993 | 2.922 | 1.983 |
| PAWNS P=2 d>=2 | 28867 | 0.463 | 6400 | 0.495 | 0.501 | 0.845 | 0.636 |
| OCB_PURE P<=2 | 8401 | 0.135 | 1047 | 0.159 | 0.146 | 0.258 | 0.119 |
| OCB_PURE P=3-4 | 14225 | 0.228 | 1747 | 0.250 | 0.235 | 0.352 | 0.225 |
| OCB_PURE P>=5 | 5907 | 0.095 | 840 | 0.139 | 0.128 | 0.180 | 0.096 |
| NONE (E1 = 0) | 13984 | 0.224 | 11698 | 0.222 | 0.214 | 0.190 | 0.207 |

Every proposed cell but one stands on 1047 games or more against DEC-326 (5)'s 1000; OCB_PURE P>=5 stands on 840 and
is tied to P=3-4 at scale 1 before any fit. P=0 d<=0, d>=2 and P=1 d>=2 are merged from the census's finer grid, whose
thin corners (P=0 d<=-1, 30 games; the d>=3 cells, 22 to 1081) are in adocs/data/S124_census.txt. **The fixed qs view,
the screen's instrument, holds 93, 162 and 101 games in OCB_PURE's three cells and 16 in KBN**; the PAWNS cells 191 to
2538. **The corpus barely holds mop-up's rows, and the filter is
why:** the raw qs pool has 227476 lone-king rows in 27756 games, 175227 of them past the recipe's |column 3| < 1000,
and the sampled corpus keeps 13274. The low-phase signatures are rook endings first (KRP2vKRP1 21359 rows, KRP1vKR
17158); 4290 signatures at phase 6 or below.

**What the labels read beside the shipped score.** S122's corpus at K 0.7433, reading A (below), a flat f per cell,
in sample, whole-game errors, f from 0 to 1.25 in steps of 1/32: a reading, never a fit. dMSE against f = 1, in 1e-6, per
the cell's rows and over all 6230857:

| cell | label | prediction | f | dMSE, cell | over all rows | f, reading B | dMSE B, cell |
|---|---|---|---|---|---|---|---|
| P=0 d<=0 | 0.496 | 0.521 | 0.312 | -1231 +/- 183 | -1.7 | 0.250 | -1689 +/- 167 |
| P=0 d=1 | 0.510 | 0.746 | 0.000 | -58694 +/- 923 | -226.9 | 0.000 | -59892 +/- 923 |
| P=0 d>=2 | 0.701 | 0.883 | 0.188 | -28619 +/- 2175 | -66.9 | 0.219 | -29676 +/- 2196 |
| P=1 d<=0 | 0.532 | 0.628 | 0.219 | -13356 +/- 444 | -125.0 | 0.188 | -13496 +/- 456 |
| P=1 d=1 | 0.702 | 0.787 | 0.531 | -7082 +/- 557 | -62.5 | 0.531 | -7203 +/- 564 |
| P=1 d>=2 | 0.875 | 0.897 | 0.906 | -181 +/- 93 | -0.6 | 0.875 | -227 +/- 138 |
| P=2 d<=0 | 0.614 | 0.651 | 0.750 | -1563 +/- 131 | -35.1 | 0.719 | -1602 +/- 140 |
| P=2 d=1, d>=2 | 0.786, 0.892 | 0.790, 0.898 | 1.000 | 0 | 0 | 1.000 | 0 |
| OCB_PURE P<=2 | 0.535 | 0.659 | 0.250 | -20192 +/- 1738 | -27.2 | 0.219 | -20147 +/- 1753 |
| OCB_PURE P=3-4 | 0.612 | 0.696 | 0.562 | -7592 +/- 1050 | -17.3 | 0.531 | -7805 +/- 1079 |
| OCB_PURE P>=5 | 0.627 | 0.689 | 0.656 | -4105 +/- 1166 | -3.9 | 0.625 | -4292 +/- 1210 |
| OCB_ROOK | 0.618 | 0.660 | 0.719 | -1677 +/- 267 | -15.4 | 0.688 | -1735 +/- 284 |
| OCB_OTHER | 0.667 | 0.673 | 0.906 | -46 +/- 28 | -2.1 | 0.938 | -40 +/- 19 |
| mop-up's domain | 0.666 | 0.943 | 0.156 | -74600 +/- 3435 | -116.6 | | |
| of it KBN | 0.518 | 0.963 | 0.000 | -192579 +/- 4659 | -60.4 | | |

The labels are the strong side's. The thirteen factor cells sum to -582 in sample; S122's pass read one linear safe-check cell
at -114.1 the same way, and S101's parts read +0.6e-5 to +5.3e-5 held out, a different
instrument. Reading B sits within about one error of A in every cell but P=0 d<=0 (-1689 against -1231 +/- 183, 0.6 over all
rows); at P=0 d=1, -59892 against -58694 +/- 923. OCB_OTHER, the
record's removed "general" case, reads -2.1. Past P = 2 the labels ask for f above 1, the side up material with many
pawns scored low: P=4 d=1 f 1.219 -19.2, P=5 d=1 1.25 (the grid's edge) -18.2, P=6 d=0 1.188 -16.6, P=5 d=0 1.156
-16.1; outside a factor toward a draw.

**What the tablebase reads, and why the labels cannot be fitted as they are.** The same reading over the training
corpus's rows of five men or fewer, once against each game's result and once against the tables' verdict at the
row's own clock under the fifty-move rule (WON 1, DRAWN and SPOILED 0.5, LOST 0; EDGE rows, at exactly 100, left out
of both), strong side's view (adocs/data/S124_tb_probe.py). Coverage is the share of the cell's rows the tables
reach:

| cell, five men or fewer | rows | coverage | games | game label | law label | f, games | f, law |
|---|---|---|---|---|---|---|---|
| P=0 d<=0 | 6849 | 78.4 % | 1605 | 0.500 | 0.500 | 0.281 | 0.281 |
| P=0 d=1 | 15252, 3 EDGE | 63.3 % | 2103 | 0.507 | 0.508 | 0.000 | 0.000 |
| OCB_PURE P<=2 | 1542 | 18.4 % | 208 | 0.501 | 0.501 | 0.000 | 0.000 |
| P=0 d>=2 | 9080, 2 EDGE | 62.3 % | 2065 | 0.670 | 0.932 | 0.125 | 1.250 |
| P=1 d<=0 | 24469 | 41.9 % | 5026 | 0.547 | 0.667 | 0.188 | 0.938 |
| P=1 d=1 | 5285 | 9.6 % | 1338 | 0.802 | 0.933 | 0.750 | 1.250 |
| P=1 d>=2 | 2035 | 9.8 % | 792 | 0.913 | 0.994 | 0.938 | 1.250 |
| P=2 d<=0 | 1744 | 1.2 % | 773 | 0.688 | 0.755 | 1.250 | 1.250 |
| mop-up's domain | 9742 | 100 % | 2087 | 0.666 | 0.990 | 0.156 | 1.250 |
| of it KBN | 1954 | 100 % | 212 | 0.518 | 0.954 | 0.000 | 0.938 |

P=2 d>=1, OCB_PURE P>=3 and OCB_ROOK have no row of five men or fewer. OCB_PURE P<=2's covered rows are its P=0 and
P=1 rows alone, so its P=2 rows, 82 % of the cell, are not read. Where the tables say drawn, the games agree within
0.002 and both ask the same f: KRvKB 1378 of 1418 rows drawn, KRvKN 1364 of 1529, KBPvKB on opposite colours 1312 of
1316. **Where the tables say won under the rule, the games drew**, and a factor fitted on them learns to scale such
endings toward a draw. At their own clocks mop-up's 9742 rows read 9546 won,
192 spoiled past the rule and 4 drawn; KBN's 1954 read 1775, 175 and 4. The zeroed clock overstated it: there KBN's
rows read won 1950 times. Across the PAWNS cells 56 rows won at a zeroed clock sit past the rule and 5 on its edge.

**How the games that reached a lone-king ending on the board ended** (adocs/data/S124_conversion.py, games classified
by their root rows in the raw root pool, positions the game stood in; a qs row is a leaf and no evidence that the game
reached it). Training games, from the side with men's view:

| signature | games reaching it | won | drawn | drawn with a root row won under the rule | drawn, last root row in it and won there |
|---|---|---|---|---|---|
| KRvK | 2315 | 894 | 1421 | 915 | 914 |
| KQvK | 1700 | 1451 | 249 | 249 | 249 |
| KBNvK | 212 | 14 | 198 | 195 | 155 |
| KRBvK | 739 | 448 | 291 | 290 | 284 |
| KRNvK | 623 | 379 | 244 | 242 | 230 |
| KBBvK | 68 | 30 | 38 | 38 | 37 |

Of the 1421 drawn KRvK games, 506 end on a root row the tables read drawn, every one with the lone king to move at a
clock of 0: the ending entered by a capture into a position the tables score drawn, never a won KRvK. Of the 914 that
end on a won root row, the clock there is under 40 in 859 and none is 90 or more. KBNvK's 43 drawn games not won at
their last root row are 40 past the rule there and 3 drawn: the fifty-move rule can account for those, not for the
155.

**Why those games drew, as far as counts say: an indication.** `tools/datagen.cpp` `terminal_result` scores a game
0.5 on `is_position_repeated`, a two-fold anywhere, while `src/search.cpp` `negamax_at` reads a single occurrence
before the root as no draw (DEC-173), so the search may repeat once and datagen ends the game there. The clocks above
rule out the fifty-move rule for most of them (inferred: the plies after a game's last root row are unrecorded). adocs/data/S124_twofold_check.py: from the last root row of each game
of the table's last column, the engine at 100000 nodes reaches, within two plies, a position the game's root rows
already hold in 240 of
those 914 KRvK games, 64 of 249 KQvK and 41 of 155 KBNvK. **An indication and not a bound either way:** that search is not datagen's -- its table did
not carry across the game, and it can repeat where datagen's search did not or miss one it made -- and a repetition
through a position in check is never recorded. **Nothing here separates head's play from datagen's rule**: S331
measures the rule's share and P0 measures head's conversion under the laws. In the match harness the same endings
mostly never play out: `fastchess.sh` resigns at 400 cp held three moves by both sides, and KRvK rows read |E1| about
620.

**The form, as this pass proposes it.** Every definition is chesso's own and is said to be (DEC-221). CPW *Mop-up
Evaluation*, read 2026-11-03: decided pawnless endings against a lone king, a bonus on the losing king's centre
Manhattan distance and on the kings' closeness, one formula with constants attributed to Chess 4.x; no scale factor
and no KBN feature. No engine's source was read; the step file's section 1 reads commit messages for figures only.

- **The scale factor.** One integer f_c per cell, F = 128 the identity, the domain [0, F]: 0 a draw, F unscaled, so the
  term never flips E1's sign. Thirteen cells, the first that holds: DEAD nothing (the search's draw); LONE nothing
  (mop-up's); OCB_PURE by P <= 2, 3 to 4, >= 5; OCB_ROOK; PAWNS for P 0 to 2 by d <= 0, 1, >= 2; anything else F.
  Twelve free after OCB_PURE P>=5's tie.
  Stage one's integer becomes material + (F X + (f_c - F) E1 (24 - phase)) / (24 F), X being the numerator `taper`
  divides today, truncating once as today: at f_c = F it is today's integer exactly. **The truncation bound moves once
  a cell leaves F:** one division by 24 F = 3072 loses up to 3071/3072 where `taper`'s lost 23/24, so a row in a cell
  off F reads within 2 x 23/24 + 3071/3072 = 2.916 of the smooth score with tempo zero, and 2.916 + 23/24 = 3.874 once
  tempo is fitted, against 2.875 and 3.833 today; on a build with every cell forced off F every case row is such a
  row. `tests/test_eval_trace.cpp` "the smooth score is within the truncation bound" therefore widens in phase 1 as a
  design decision recorded with its arithmetic, and `truncation_positions` and the comments stating 69/24 follow
  their script, as S122's would if its curve survives. |F X| is under 1.01e8 by the packed halves' 16-bit bounds, and
  the whole fits int32 for any |material| under 600000. F is DEC-134 form (b) from chesso's scale: a unit of f moves the score by under E1/128, under 8 cp up to
  E1 of 1000 (the cells' |E1| w means run 36 to 390), and 24 F is a constant divisor. **Reading A, stage one only:**
  stage two's endgame half stays unscaled (|E2| w means 18 to 37 cp over the cells), its clamp, the margin and S122's
  re-derivation untouched; B buys under one error above in all cells but P=0 d<=0, 0.6 over all rows. **The seed is the identity, f_c = F, DEC-134 form (b)
  as S121's identity seed**, so the landing is `No functional change` with liveness flags compiling the factor out;
  section 4's midpoint F/2 (form (c)) would move every case row at landing, and in a fit with the old constants frozen
  each cell is linear in f, so the seed does not decide the optimum.
  **The cell is a function of the counts and of E1's sign**, and E1 moves whenever material or the tables are free:
  a cell stored per row goes stale in a refit. So the trace records **both candidate cells**, the one White's being
  strong would select and Black's, and `exact_parts`, `smooth` and `gradient` pick by the current E1's sign. Where the
  pick switches E1 is zero and so is the factor's term, so the score stays continuous across it.
- **Mop-up.** Its domain: a lone king against a side with no pawn and men worth two phase units or more, knights alone
  excepted; all 9742 of its corpus rows have five men or fewer, and at their own clocks the tables read 9546 of them
  won, 192 spoiled past the fifty-move rule and 4 drawn (mean 0.990), KBN's 1954 at 0.954 with 175 spoiled. Three linear weight pairs in stage one's positional pair, signed for the side with men: the lone
  king's centre Manhattan distance, 0 to 6, and 14 less the kings' Manhattan distance, 0 to 12 (the page's two
  features); for K, B and N alone, 7 less the lone king's Chebyshev distance to the nearer corner of the bishop's
  colour (chesso's own). The page's 4.7 and 1.6 are a program's constants: not seeds (DEC-105, DEC-134). Seeded at
  zero, the identity, `No functional change`. **Not fitted on the loss:** the labels are the draws mop-up is for
  (games 0.666 against the tables' 0.990 under the rule), and at |E1| w of 597 the sigmoid at K 0.7433 is past 0.92, so the
  loss barely sees where the lone king stands (inferred, no fit run). Fitted instead by SPSA on the conversion test's
  own objective over chesso's own self-play from tablebase-won starts (DEC-084's "our own self-play data"), S261's
  lane in form; whether `tools/spsa_driver.py` takes a conversion objective unchanged is not read.
- **Specialised endgames: none beyond the cells.** KBN's corner is mop-up's; Stash's v26 entry records a material-key
  and a material-quantity version tried and the simpler kept, without saying here which, and the record's three
  opposite-bishop removals are the tail section 5 warns of. Below P = 3 the cells are a quantity form; KRP1vKR alone holds 17158 rows and its games and the tablebase
  disagree (P=1 d<=0: f 0.188 against 0.938), so no sub-class can be priced on the games' labels.

**What S262's machinery carries and what it lacks.** Carried: linear terms of any width in stage one's positional
pair (mop-up's three are ordinary `EVAL_TRACE` terms), `exact_parts`, `smooth`, `gradient`, the gauges, ties, `--only`.
**Lacking, for the factor: a product of parameters.** `group_t` is a per-side transform of its own inputs' sum and
cannot read another site's sum, so it cannot express f_c times E1. Missing: (1) the row's two candidate cells, a
byte each, which the trace records and `dataset_t` stores, picked by the current E1's sign; (2) the cells as parameters, one half each like material, in `tools/tuner_layout.hpp`;
(3) `exact_parts`: SITE_POSITIONAL's division with E1 in it, as above; (4) `smooth`: (f_c / F - 1) E1 w_eg added; (5)
`gradient`: d/df_c = E1 w_eg / F, every material weight a further coef (f_c / F - 1) w_eg, every positional endgame
weight's w_eg times f_c / F -- with the old constants frozen the cells are linear and only the first is needed, but
S126 refits everything and inherits the rest; (6) a box [0, F] applied after each step, as `hold_gauges` is; (7)
`tools/feature_audit.cpp` audits trace features, and a cell's column at the seed, E1 w_eg / F, is none, so the audit
is silent on the factor until it learns that column; (8) `--only` names and tie rows. The gauges stay null by
construction, E1 holding the material.

**The identity plan.** `tools/truncation_scan` holds `exact_score` to `evaluate` on every row and is the factor's
identity; under reading A `tools/eval_spread` reads stage two, which the factor does not touch, so it stays green and
blind to it, and its header says so. At the seed the factor is exactly zero and no plant shows, so the plants are
read on a build with every cell forced off F: a candidate cell one off, `exact_parts` scaling the endgame sum
without the material, its divisor 24 for 24 F, and the strong side read from E1 + E2 (the "split" rows,
adocs/data/S124_census.txt: 852 in OCB_PURE, 15 in P=0 d>=2) each turn the scan red. **A stale pick is invisible to
the scan**, which reads at the engine's own weights where the stored and the current sign agree: the tuner's tests
move one material weight until some row's E1 changes sign and hold the cell `exact_parts` and `smooth` pick to E1's
sign recomputed at the moved weights, red on a plant that picks by the shipped sign. A wrong count in the engine -- P read from the weak
side -- passes both tools, the trace reading the engine's own variable: the candidate cells and mop-up's three
distances get an independent walker by squares, `tests/test_walked_counts.hpp`'s form, red on a planted swap.
`tests/test_tuner_gradient.cpp` gains the cells' derivative and the chain rule into material and the positional
endgame weights against finite differences, a plant seen red.

**The screen, as proposed** (S122's form, DEC-331 (6), DEC-344 (3), DEC-345 (1)):

- **P0, a reading:** head's conversion test. Starts are distinct training-side root positions of mop-up's domain
  (positions the games stood in, not qs leaves), one a game: 200 each of KRvK, KQvK, KRBvK, KRNvK and KBNvK, and
  every KBBvK game's. **Each start's clock is zeroed**, and a start is kept only where the tables read 2 there, which
  at a zeroed clock is a win under the rule by their own definition (|dtz| <= 100); so the fifty-move rule and the
  bound coincide, and a start the game itself held past the rule is a fair start all the same. Self-play at 100000
  nodes a move under the laws -- threefold and the fifty-move rule, no adjudication; converted means mated within 50
  moves of the start. About 1100 games of at most 100 plies at 0.04 s
  a search: about 10 minutes on 12 cores. **If head converts every start here, mop-up has nothing to buy, is
  recorded as zero, and the drawn games above are datagen's rule alone.**
- **P1, mop-up:** SPSA over its three pairs on the P0 objective (plies to mate, the bound for a failure), about 3 h;
  then the conversion test on fresh starts. Kept iff it converts more paired starts than head by McNemar's one-sided
  z > 1.645 and no signature fewer.
- **The factor's parts**, on S122's prepared corpus at the landed tree's own K, the old constants frozen, earlier
  survivors live, each paired against the landed tree on S082's fixed qs view (DEC-326 (3)), kept iff d > max(1.645 sd(d_s), 2e-6) over five quarter samples, every fit at
  `--epochs 20000 --report 100 --patience 20` (DEC-345 (1)), largest signal first: **F1** P=0 d<=0, P=0 d=1, OCB_PURE
  P<=2 (3 cells, -256 in sample, where the tables and the games agree on the rows the tables cover -- 78 %, 63 % and
  18 % of the three cells' rows, OCB_PURE P<=2's being its P=0 and P=1 rows alone); **F2** P=0 d>=2 and P=1 and P=2 all d (7
  cells, -290, the classes the tablebase reads won); **F3** OCB_PURE P=3-4 and P>=5, OCB_ROOK (3 cells, -37, no row of
  five men or fewer). **And the tablebase guard, the accepts' tool-labelled screen:** a part's fitted cells read on the
  training side's rows of five men or fewer against the tables' verdict at each row's own clock (WON 1, DRAWN and
  SPOILED 0.5, LOST 0, EDGE rows left out, adocs/data/S124_tb_probe.py); a part that raises that loss by more than
  1.645 whole-game errors is refused whatever its held-out d. F2 is expected to be refused (inferred from the table above); F3
  has no such row and is screened on the loss alone, and its stamp says so.
- **The cost**, S122_cost.sh's form over masks F1, F2, F3, mop-up and all; it decides nothing, the verdict carries it.
- **The SPRT:** one `{0, 5}` nElo of the survivors at 8+0.08, Hash 16, `noob_3moves.epd`, concurrency 12, against the
  landed tree (DEC-259 (b)). DEC-143: 41861 games at the midpoint, 19.84 h at 2110 an hour; 25591 on a bound, 12.13
  h; cap 20950 rounds, 41900 games, 19.86 h. Abort rule S101's: a time-forfeit rate past 1.0 % on either side, a crash
  or a disconnect, mains power lost, another load on the machine, a banner not the pinned one, or 40 h with neither
  bound. An A/A of 1000 games first if the harness moved since the last. **The adjudication reads the engines' own
  scores**: a factor below 400 cp on a won ending postpones the two-sided resign, and mop-up's conversions mostly sit
  past it, so this verdict sees mop-up only where play reaches a lone king under 400.
- **On H0 or no verdict, in order, each `{0, 5}` against what has landed, survivors only, refit:** A = mop-up, B = F1,
  C = F2 and F3's survivors.
- **Machine hours, inferred, not measured:** P0 10 min; P1 about 3 h plus 10 min; each factor part six fits, about
  36 min at patience by S122's 0.87 s a part-epoch, up to 4 h 50 m at the cap: 2 to 15 h for three parts; the guard
  minutes; the cost about 1 h; the SPRT 12.1 to 19.9 h. About 19 to 39 h, the SPRT most of it, and up to three more
  verdicts of 19.9 h on H0.

**Interactions.**

- **S122 lands first.** Its terms are stage two's, which reading A does not scale, so its curve and its margin
  re-derivation are untouched by this factor, and this family's fits freeze S122's fitted constants: S124's screen
  starts on S122's landed tree, after S122's screen frees the machine.
- **S331 lands before S124's fits.** It changes `datagen` and regenerates nothing, so the labels S124 fits on are
  still S122's games, scored under the two-fold rule; what S331 brings here is its count of the games and rows that
  rule relabelled, on which Q1's pick waits.
- **The clamp and the lazy margin:** under A stage two and its clamp are unchanged and |`evaluate` - cheap| <=
  margin still holds, so the shortcut stays sound by construction; scaled cheap scores move which nodes take it, a
  play change the SPRT reads. Under B the clamp is still taken after the scaling and the bound holds too.
- **S266**, complexity, after this step: an additive term on the endgame half, sign-preserving. Its E must be E1 with
  the material in it, or its adjustment is gauge-dependent for the reason above; it composes with f on the same half
  and its step decides the order.
- **The fifty-move decay** (reserve, not built): a factor by the halfmove clock, which no key carries; this factor
  reads the board's men alone, so a stored evaluation stays a function of the position.
- **The insufficient-material detector:** no corpus or view holds a DEAD row (datagen ends the game there); the
  classifier tests the detector's condition first and leaves f = F, and the search returns `DRAW_SCORE` above the
  root before `evaluate` is asked. Same-colour bishops, dead by the laws and not by the detector (S067), fall in
  P=0 d<=0, which the games and the tablebase both read at f 0.281.
- **S129, tablebases:** a probe answers a five-man node exactly and supersedes both terms there; the factor's cells
  and mop-up still fire on six men and more, and on a machine without the files.
- **S099's pawn correction** learns the gap between search and raw evaluation in-game; a scaled raw score leaves it
  a smaller gap in these endings. Read by the SPRT, not argued.

**Questions**, each in DEC-265's form; the draft at `.tuning/coord/S124/decision_draft.md` carries them as proposals.

1. **The labels the factor is fitted on. Owner's if (b) is taken, the coordinator's otherwise.** *Why:* DEC-259
   screens on S082's fixed views and DEC-084 fits on self-play, and here the self-play results contradict the tables
   wherever they read a win under the fifty-move rule (P=0 d>=2: 0.670 against 0.932); the held-out views come from
   the same games, so the screen itself rewards learning draws the laws would not score, and nothing recorded says
   which label wins. **The pick waits on S331's measurement** of how much of that is datagen's rule. *(a)* Fit on the
   games' labels and refuse any part the tablebase guard refuses: 0 h; F2 is expected to fall, F1 and F3 stand on
   merit; F3's labels stay unchecked, six men and more. *(b)* Replace the five-man rows' labels by the tablebase's in
   the fit, at each row's clock: F2 can be fitted on the tables' verdicts where they exist, about 1.3 % of rows; a tool's verdict becomes a training
   target, which DEC-084's "our own self-play data" does not cover and no decision has allowed; six-man rows unchanged.
   *(c)* Regenerate after mop-up and S331: S122's recipe, 150000 games, about 15 to 19 h plus preparation, labels of
   games scored by the laws and played by an engine with mop-up; F2 read on them; it moves S122's corpus, which S126
   inherits. **Recommended (a) now, and (c) as S126's corpus question, unless S331 reads a share large enough that F2
   should wait for (c):** (a) costs nothing, ships only what both instruments agree on, and does not decide S126's
   corpus from here.
2. **Datagen's two-fold: settled, S331**, planned before S124 and in progress. The fix is `datagen`'s game-ending
   test reading `classify_repetition(&history, &board, history.size)` and ending the game on `DRAW` alone: with the root
   at the history's end every earlier occurrence lies before it, so only a third occurrence draws, the laws' and the
   harness's rule and the search's for positions before its root (DEC-173). A root of 0 would keep the defect, every
   earlier index lying above it, so the first recurrence would read `DRAW`. S331 also measures how many games and rows
   the two-fold rule relabelled, and Q1 waits on that number.
3. **Reading A or B. The coordinator's.** *Why:* the accepts says "the endgame half" and chesso's has two stages. *A:*
   stage one only; stage two, its clamp and S122's margin untouched; stage two's endgame half, 18 to 37 cp on average,
   unscaled in a scaled row. *B:* both stages; the case needed in stage two too, one more computation of it or a
   parameter through `expensive_score`, the clamp read after the scaling, `eval_spread` and the collecting
   instantiation gaining the factor; it reads within about one error of A in every cell but P=0 d<=0 (0.6 over all
   rows). **Recommended A.**
4. **The seed. The coordinator's.** *Why:* section 4 proposed the midpoint F/2, DEC-134 form (c), and S121 seeded at
   the identity, form (b). *Identity:* landing `No functional change`, INV-6 by `search_bench`, the fit's start the
   shipped evaluation. *Midpoint:* every case row moves at landing, a play change before any fit; with the old
   constants frozen each cell is linear in f, so it buys no other optimum. **Recommended the identity.**
5. **Mop-up's fit. The coordinator's for (a) and (c), the owner's for (b).** *Why:* DEC-084 ships no constant
   unfitted, and the loss cannot fit these three (above). *(a)* SPSA on the conversion objective, chesso's own
   self-play from tablebase-won starts: about 3 h, the thing the accepts tests; noisy at three parameters, read by the
   conversion test after. *(b)* Regress on the tablebase's distance to zeroing: minutes, but a tool's number as the
   target. *(c)* The tuner on the games' labels: 0 h, and it fits those draws. **Recommended (a).**
6. **The tuner's product. The coordinator's.** *Why:* with the old constants frozen only d/df_c is needed; S126 refits
   everything. *Frozen form:* items (1) to (4), (6) and d/df_c, and the tuner refusing any free old constant while a
   cell is free; S126 then builds the rest. *All of it:* items (1) to (8) now, one finite-difference check over
   material, the positional endgame weights and the cells. **Recommended all of it now**: the gradient check is written
   once, against the arithmetic it covers.

**Findings beyond the step, not fixed here.** (a) `tools/datagen.cpp` `terminal_result` adjudicates a draw on any
two-fold through `is_position_repeated`, the convention DEC-173 retired from the search; S331 takes it (Q2). (b) The step's `touches:` lacks `src/eval_trace.hpp`,
`tools/tuner_layout.hpp`, `tools/tuner.cpp`, `tools/feature_audit.cpp`, `tests/test_eval_trace.cpp`,
`tests/test_tuner_gradient.cpp` and `tests/test_walked_counts.hpp`, which this design needs. (c) Sections 3 and 4 still
name `tools/eval_model.hpp` and the midpoint seed; the 2026-10-08 amendment discharges the first, Q4 the second. (d)
Past P = 2 the labels read f above 1 on four cells, about -70 in sample together: an imbalance signal for S126 or S133,
outside a factor toward a draw, and unguarded by any tablebase.

**Unmeasured or inferred, flagged.** No fit ran: every "the fit would" is inferred. The label readings are in sample,
flat per cell, at the corpus's own K; held-out loss is the screen's. The tablebase reading covers five men or fewer.
The two-fold count is an indication, not a bound. The cost rows are a sink's instructions; the shipped form's are the cost
run's. No part's Elo, no nps.

## 2026-11-04: the ruling, DEC-350

The research pass's eleven items are ruled in DEC-350. Taken as written:
- the factor on E1, f in [0, 128], seeded at the identity;
- the thirteen cells;
- mop-up's three linear pairs, seeded at zero and fitted by SPSA on conversion;
- the tuner's product in full, with both candidate cells traced;
- the identity plan and its plants;
- the one `{0, 5}` SPRT and its bisection;
- the widened `touches:`.

Two items changed with DEC-349, which voids S122's corpus because `datagen` drew games at any two-fold until S331:
- The parts are read on S122's regenerated prepared corpus and its own held-out games, not on S082's fixed view.
- The labels are the games' results under the laws, written by S331's `datagen`. F2 is screened like F1 and F3, and no
  label is replaced by a tablebase's.

The tablebase guard and P0's conversion test stay, the guard at each row's own clock.

**Order.** Phase 1 is built while S122's corpus and screen run, and lands after S122's screen has ended. S124's fits wait
for S122's landing. The research pass's corpus counts are the void corpus's, and are re-read on the regenerated one.
