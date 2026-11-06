id:         S124
goal:       the endgame half of the score is scaled toward a draw by what is actually on the board, and won lone-king and specialised endgames are scored so the stronger side converts them
accepts:    an SPRT verdict, recorded whatever it is; the factor scales **only the endgame half** of the tapered score and not the whole score; each case -- strong-side pawn count, opposite-coloured bishops, and the mop-up and specialised-endgame cases this family gains (DEC-258) -- is screened separately before the family's one SPRT (DEC-259), and a case the corpus barely holds is screened on a tool-labelled position set instead (tablebase WDL or Stockfish through python-chess, never the agent's reading, CHESS rule), because the surveyed record shows several such cases measuring zero and being removed again; mop-up is shown to convert: won lone-king positions, labelled won by a tablebase, are mated by the engine in self-play at a fixed node budget within a move bound stated before the run; every constant is fitted (DEC-084); the existing insufficient-material draw detection is not duplicated by this and the step says how the two divide
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, src/eval_trace.hpp, tools/tuner_trace.hpp, tools/tuner_groups.hpp, tools/tuner_layout.hpp, tools/tuner_optimiser.hpp, tools/tuner.cpp, tools/feature_audit.cpp, tools/audit_design.hpp, tools/truncation_scan.cpp, tools/eval_spread.cpp, tests/CMakeLists.txt, tests/test_evaluation.cpp, tests/test_eval_positions.hpp, tests/test_endgame_positions.hpp, tests/test_eval_model.cpp, tests/test_eval_trace.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, tests/test_tuner_box.sh, tests/test_audit_design.cpp, tests/test_walked_counts.hpp, adocs/data/S101_gradient_rows.cpp, adocs/data/S310_adagrad_step.cpp, adocs/data/S122_screen.py, adocs/data/S083_truncation_pick.py, DEV_MANUAL.md
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

## 2026-11-04: phase 1

One Opus 5.5 subagent, coordinated; worktree `../chesso-s124p1`, branch `s124p1` on `d83d6c0`, the diff staged and not
committed. DEC-350's items (1) to (6) built at the identity -- every cell's factor at F = 128, mop-up's three pairs at
zero -- so the landing is `No functional change`; nothing fitted on real data, timed or played, and no game generated.
Every definition is chesso's own and stated in src/evaluation.hpp (DEC-221); mop-up's first two counts are CPW *Mop-up
Evaluation*'s features, its constants a program's and no seed (DEC-105); no engine's source was read. F is DEC-134 form
(b); every forced value in a test, a plant or a sink is written where it is used and derived from nothing.

### The design as built, DEC-350

- **The factor, reading A** (`scaled_taper()`, src/evaluation.cpp): stage one's integer is `material + (128 X + (f_c -
  128) E1 (24 - phase)) / 3072`, X `taper()`'s numerator, one truncating division, so at f = 128 it is `taper()`'s
  integer exactly. E1 = `board->material + eg_value(pair)`, the pair being the psqt sum, the pawn families, placement
  and mop-up; mop-up's share of E1 never reaches a factor, mop-up firing only on LONE, which has no cell. The strong
  side is E1's sign; at E1 = 0 no cell. Stage two, its clamp and S122's margin are untouched. An int32 headroom
  `static_assert` covers the numerator at any weights the stage-one bound admits; a second keeps every cell in [0, F].
  **f = 0 removes E1's share and no more**: stage one keeps (material + the pair's middlegame half) phase / 24, a draw
  at phase 0 and elsewhere only where the two cancel. The research pass's "0 a draw" (above, "The scale factor") holds
  at phase 0 alone; the fast check's finding (6), corrected in src/evaluation.hpp's comment too.
- **The cells** (`endgame_cell(board, strong)`): LONE (a side's king alone) first, which holds every position
  `is_insufficient_material()` calls dead, so the detector and the factor divide without the detector being called
  here: the search draws those before `evaluate()`, LONE gives f = F, and mop-up's two phase units exclude them. Then
  OCB (one bishop a side, `one_colour` apart): PURE by P <= 2, 3-4, >= 5; ROOK, one rook a side and no knight or
  queen; any other no cell. Then PAWNS, P 0 to 2 by d <= 0, 1, >= 2. Numbered `endgame_cell_t`, PAWNS 3 P + column
  first, then the four OCB cells.
- **The gate**, the cells' first test: past two pawns of the strong side, with a knight or queen on the board or not
  one bishop a side, no cell can hold and nothing more is read. Tighter than the research pass's (both sides past two),
  the strong side being known; sound by the definitions, and the walk and the census below hold it so.
- **Mop-up** (`mopup_counts()`, `mopup_terms()`): the domain is one side lone (its occupancy its king's square, two
  compares) against a side with no pawn, two phase units or more, not knights alone; the three counts, the side with
  men's own, signed White's less Black's into stage one's pair, `TERM_MOPUP` in the trace. The stage-one 16-bit bound
  gains mop-up's reach (6, 12, 7 a weight) in `stage_one_with_mopup`.
- **Liveness, S101's form**: `endgame_scale_live` (a cell off F) and `mopup_live` (a weight off zero) gate both out of
  the search's `evaluate_cheap()`; the trace build computes both whatever their weights.
- **INV-4, what each reads**: the cells, popcounts of `board->bitboards` (pawns, knights, bishops, rooks, queens per
  side), the two side occupancies against the king bitboards, the bishops against `one_colour`, `board->material` and
  `eg_value(pair)`; mop-up, the same occupancies, one popcount pass over a side's men and two `get_lsb_index` for the
  kings. No accumulator rebuilt, no piece walked.
- **19 parameters, `PARAM_COUNT` 1171 -> 1190**: `MOP_` (3 a half) then `EGS_` (13, one half each, as a piece value),
  `ENDGAME_SCALE_END == PARAM_COUNT` asserted, no earlier index moved; groups `mopup` and `endgame_scale`; parts
  `endgame_scale.f1`, `.f2`, `.f3` (DEC-350 (7)'s F1 to F3, each by its cells); the tie row `endgame_scale`, one half
  and no suffix, so `endgame_scale 11 10` is DEC-350 (2)'s tie; the header emits `mopup_mg`, `mopup_eg` and
  `endgame_scale` in the engine's own form.
- **The trace** carries the two candidate cells in `record_t::cells`, set by `EVAL_TRACE_CELLS`, a byte each in
  `dataset_t::cells`; `sum_record()` refuses a cell past the thirteen (a new refusal case in `test_eval_trace`).

### The tuner's product, DEC-350 (5)

- `exact_parts()`: E1 the material and the positional endgame sum at the weights read, its sign's candidate, the
  factor inside the one positional division; `exact_parts_t` gains `e1` and `cell`. `smooth()`: `(f_c / F - 1) E1
  w_eg`, zero at F and at E1 = 0, so the score is continuous where the pick switches. `gradient()`: `d/df_c = E1 w_eg /
  F`; a piece value's coefficient `1 + (f_c / F - 1) w_eg`; a positional endgame weight's taper weight `w_eg f_c / F`.
- **The box** `tuner_groups::box_scales()` after each step and its ties, in tools/tuner.cpp. Not inside `descend()`:
  the box is no null direction, and "the fit holds every gauge still and no score sees the hold" compares `descend()`
  with the optimiser alone, so a box there read 348.6 and 130.2 apart (Adam, AdaGrad) on the cells it boxed, the box
  and not the hold. Out of `descend()` the case reads 7.96e-13 apart under Adam, and 3.50e-12 under AdaGrad at its old
  step of 2, 1.14e-12 at the 3 its golden now carries (below); its form unchanged.
- `tools/feature_audit`: the cells' columns while every cell is at F, the integer E1 (24 - phase) on the rows whose E1
  picks the cell (the 1/F a common factor); off F the identity report says `not computed`. The design check leaves the
  cells out of its sum, as it leaves the curve's inputs. `support endgame_scale[c] rows N games G candidate_rows M` and
  `mop-up (S124) support:` lines for the screen's ties. And one dependency by construction, found by the recovery
  below: **mop-up's corner pair is one direction** -- the count is non-zero on KBN against a king alone, always phase
  2, so `11 mopup_mg[2] - mopup_eg[2] = 0` on every row; held when the free set freezes a half, otherwise reported as a
  direction nothing holds (S121's mobility levels' form).
- `tools/truncation_scan`'s and `tools/eval_spread`'s headers say what each sees of the factor (DEC-350 (6)).

### The walk and the census

`walked_counts::endgame_family_features()` (tests/test_walked_counts.hpp): the men counted by walking the placement, a
bishop's colour from rank and file, the centre distance as the nearest of its four squares, the corner as the nearer
of the two of the bishop's colour found by asking all four; DEAD written out though LONE holds it. `test_eval_model`
"eval counts: S124's endgame family": S101's population and S124's (`tests/test_endgame_positions.hpp`, 49 signatures
each way round, seeded 20261104, 5719 positions), 17794 in all, both candidates and every mop-up count per side; the
rarest cell or count non-zero on 47 (White's OCB_PURE P >= 5); and 16 hand cases from the definitions, each as
written and mirrored. Green on first run; the plant is what says it can fail.

**The census as a check** (adocs/data/S124_cell_check.cpp, adocs/data/S124_cells_check.txt): over S083's corpus
(sha256 2e0e3de9...) and S082's three fixed views, read in place, FEN column only, the engine's candidates and counts
against the walk and against the trace's record: **0 rows differ** in 6226944 + 1015332 + 1015607 + 988086. On S083's
corpus the cells E1 picks read the research pass's `S083 %` column to the third decimal (P=0 d<=0 0.111 to P=2 d>=2
0.636, OCB_PURE 0.119 / 0.225 / 0.096, mop-up's domain 0.112 and KBN 0.022); OCB_ROOK 0.944 against 0.945, the 0.001
being the case's rows at E1 = 0, which pick no cell. S122's regenerated corpus does not exist yet.

### INV-6

`chesso bench` 5482277 in `build` and in `build-tune`. `tools/search_bench.py` against `d83d6c0`'s
binary (a clean `git archive` build, sha256 097eddc2...): depth 9 c3d5 39080 / d5e6 110485 / d7c8q 44098, depth 12
c3d5 219796 / e2a6 307606 / d7c8q 256487, node counts and best moves identical. `adocs/data/S124_objdiff.py` over the
two `evaluation.cpp.o`: every function identical once the padding is dropped, `endgame_family_counts()` the one new
(`adocs/data/S124_inv6.txt`).

### The cost, INV-4

`adocs/data/S124_phase1_cost.sh` (`S124_phase1_cost.patch`): the landed `mopup_terms()` and `scaled_taper()` copied
with forced non-zero weights into an asm barrier, the score the landing's, node counts checked equal first; five
interleaved `tools/perf_counters` runs, instructions only (`adocs/data/S124_phase1_cost.txt`):

| sink build, instructions | `bench` | `bench 16` |
|---|---|---|
| the factor alone (`S124_SINK=1`): the cell, its gate, the scaled taper | +1.205 % | +1.212 % |
| mop-up alone (`=2`): the domain's two compares, its counts where it holds | +1.037 % | +0.993 % |
| both (`=3`) | +1.906 % | +1.861 % |

Node counts 5482277 and 14931387 on all four builds before any count; spreads 308 to 921 instructions over five runs.
The factor sits under the research pass's gated +1.316 % / +1.264 %, its gate reading the strong side's pawns; mop-up's
+1 % is, inferred and not split, the domain test at every stage-one call, its counts running only in the domain. No cycle,
wall or nps was read: the verdict carries the cost (DEC-350's screen, "The cost").

### Guards, each seen red on a plant

`adocs/data/S124_phase1_plants.sh` on a scratch copy of the working tree with `adocs/data/S124_phase1_plants.patch`: a
`plants` build (the plant picked by `S124_PLANT`) and a `forced` one (`-DS124_FORCED`, every cell off F, the values
`test_eval_trace` forces); `adocs/data/S124_phase1_guards.txt` has every run's tally. Read again after the rulings,
plants 9 to 14 and the engine's own library new; the counts below are that run's.

| guard | plant (`S124_PLANT`) | read |
|---|---|---|
| walk, `test_eval_model` "eval counts: S124's endgame family" | none | green, 179059 assertions |
| | 6 the engine reads P from the weak side | red: 16078 checks, the first `engine.cell[side] == walk.cell[side]` |
| identity and forced cells, `plants` build, `test_eval_trace` "the trace reconstructs evaluate() exactly" and "the factor's integer arithmetic is reconstructed at cells that move it" | none | green, 55459 assertions |
| | 1 the trace's White candidate one off | red: 2024 checks, the forced-cell case's positional part; the identity at F blind, as DEC-350 (6) says |
| | 2 E1 without the material | red: 15211 checks, the forced-cell case (`parts.e1`); the identity at F blind |
| | 3 the divisor 24 for 24 F | red: 10246 checks, the identity at F as well, the numerator being 128 times taper()'s |
| | 4 the strong side from E1 + E2 | red: 97 checks, the forced-cell case, on the rows where the two signs part and the candidates differ |
| identity on the `forced` build, `test_eval_trace` "the trace reconstructs evaluate() exactly" and "the smooth score is within the truncation bound" | none | green, 20939 assertions; the worst gap 2.89193, the cell row (below) |
| | 1 | red: 367 checks, `exact_score() == evaluate()` |
| | 2 | red: 1294 checks, the same |
| | 3 | red: 2366 checks, the same |
| | 4 | **green**: no position of the identity case's list is a split row -- E1 and E1 + E2 apart in sign with candidates that differ; the forced-cell case, S124's population among its rows, reads it (above), and so does `truncation_scan` (below) |
| the tuner, `test_tuner_gradient` "the endgame factor's gradient is the derivative, chain rule in", "the factor's cell follows E1's sign when a fit moves it", "a step boxes the scale factors to [0, F]" | none | green, 12684 assertions |
| | 5 the pick by the shipped sign, the stale pick | red: 3 checks, `after.cell` and `s.cell` (4 against Black's 0) and the factor's share there |
| | 7 the chain rule dropped | red: the finite-difference worst 1.99 at parameter 476, `psqt_eg[knight][h6]` |
| | 8 the box left out | red: 2 checks, the factors left at 328 and -72 |
| the per-row truncation bound, `plants` build (every cell at F), "the smooth score is within the truncation bound" | none | green, 9000 assertions |
| | 10 the smooth score moved 0.03 on every row at F | red: 4 checks, `gap <= truncation_bound(...)`: a row at F keeps 3 x 23/24 |
| the forced-cell case, `plants` build | 9 `truncation_bound()` the old 3 x 23/24 on every row | red: 2 checks, the 2.89193 row and its mirror |
| identity and bound on the `forced` build | 9 | red: 2 checks, the same two rows |
| | 11 the engine's one pick reversed, both libraries | red: 796 checks, `exact_score() == evaluate()` |
| `tests/test_tuner_box.sh` on the `plants` build's tuner | none | green: `128 128 128 128 0 128 128 128 128 128 128 128 128` |
| | 13 the in-loop box call left out | red: `header factor 2 is 422, outside [0, 128]` |
| | 14 that call moved before the step | red: `header factor 2 is 132` |
| `adocs/data/S124_engine_eval.cpp`, `chesso_engine` against `chesso_engine_trace` on the `forced` build over S083's corpus | none | green: 0 rows of 6226944 differ |
| | 11 the pick reversed in both | green, 0 rows differ: the libraries agree, and the identity reads it red (above, 796 checks) |
| | 12 the pick reversed in `chesso_engine` alone | red: 840136 rows of 6226944 differ |
| `test_tuner_groups` "a tie names S124's factors by cell", the refusal of a scaled tie on a boxed row | the fix absent | red: 6 of its 20 assertions; green with it |
| `truncation_scan` on the `forced` build over S083's corpus, read in place | none | green: every row's identity holds, 6226944 rows; 102066 past 2.0, one past 2.875 (below) |
| | 1 | red: refused at its first disagreement, 108 against 85, `8/8/8/5p2/1k5K/8/6PP/8 b` |
| | 2 | red: 88 against 74, `8/5p2/8/5K2/2k4p/P7/1Q4PP/4q3 w` |
| | 3 | red: -5302 against 74, the same row |
| | 4 | red: -112 against -119, `6R1/6pp/4p3/4qpk1/8/R6P/6P1/6K1 w`, a split row the test positions lack |

### The planted recoveries

`test_tuner_gradient` "a planted vector comes back: endgame", a new `fast` entry: the thirteen cells from F to a plant
inside the box over the population's rows in a cell, 6000 epochs; mop-up from zero over its domain's rows, 32000
epochs. Green; 201 s wall and 17.4 s of CPU under the corpus's load (base's entry 115 s and 22.3 s), so S309's 30 s at
-j12 is unread and the coordinator's with the machine idle. **The corner pair is one direction** (above): the fit
returns 12.348 and 14.696 against 9 and 15 planted, 2 mg + 22 eg = 348.0 either way, so the case holds the pair to that
sum and the other four halves to their plants. The middlegame halves are slow, the domain's phases 2 to 6 giving them
a twelfth of the weight: the centre's 6.715 against 6 at 12000 epochs, 6.124 at 24000, 6.038 at 32000.

### Coverage and the preconditions

- **Coverage rows** (DEC-142), `adocs/data/S124_coverage_rows.cpp`, S121's rule over the family's 19: from the base 106
  rows 13 needs open (mop-up's six, seven cells); `all_test_fens()` closes six, S083's corpus the rest after 359421
  rows, 9 pairs, every row VALID by python-chess; the program reproduces its block; the gradient case's non-vacuity
  and the trace's both-ways case pass over the 124 (the smallest gradient from `MOB_MG_BASE` 3.06e-8, over S124's
  family 9.73e-7).
- The gradient case: **4.63e-7 at parameter 992, `passed_king_enemy_mg[1]`**, over the 124 (S122's 2.68e-7 at 985 on
  106). The factor's case: **3.02e-7 at parameter 726, `psqt_eg[king][b6]`, over the 482 parameters the product
  reaches**, 5715 rows; over all 1190 the worst is 2.4e-5 at `backward_rank_mg[0]`, an analytic 8.39669e-11 against
  8.39689e-11, the summation's rounding over a gradient that cancels on endgame rows, which is why the case reads the
  product's parameters and says so.
- `tools/feature_audit --free endgame_scale` over S083's corpus, read in place: the trace's identity on all 6226944 rows
  (0 differ); rank exactly 1118 of 1190, nullity 72, the 34 zero columns and 38 dependencies by construction (S122's
  37 and the corner pair), 0 further; the span `72 of at most 72`, the corner pair held by the freeze; the design check
  1.36e-12 over 200034 rows; every cell's column non-zero. Its support: the cells picked on 798 to 23018 games,
  OCB_PURE P<=2 905, P=3-4 1657, P>=5 798, so two of the three under DEC-326 (5)'s 1000 on this corpus; mop-up's
  centre and kings on 1789 and 1837 games, the corner on 196 (`adocs/data/S124_preconditions.txt`).
- A tuner smoke on the population written as a corpus (5719 rows, scratch): `--only endgame_scale --tie` with
  `endgame_scale 11 10`, 300 epochs: the load's identity 0 differ, 12 free, the tie and the box held (the fit drove
  nine cells to 0 against random labels), the header emitting the three new arrays. A smoke and no reading.
- The gauge walk (`S083_tuner_preconditions.cpp walk`): smallest plain shift 2.30 under Adam, the bishop's, and 1.76
  under AdaGrad at its step of 3, `psqt_eg[king]`'s, every hold at most 1.2e-14. `S310_adagrad_step.cpp`, the AdaGrad
  step golden, its second member found by identity since ruling 2 (below): the family's onset moves from 5 to 6 with
  the nine pairs, so its rule answers **3** where the case carried 2, and `ADAGRAD_HOLD_STEP` follows it (DEC-142); at
  3 `apart` 1.14e-12 at epoch 300, at most 5.24e-11 over the family through 1200, every hold at most 1.2e-14. The
  first report's 4 was the script's answer with its member dropping the list's last pair, one of S124's.
- **RED, and not mine to settle: the clamp case's `CLAMP_PUSH` golden has no re-derivation.** At +30 the KBN coverage
  pair (`8/8/8/2N1K3/2B5/4k3/8/8 b`, its mirror) reads 184.75, 0.75 from the kink, so "the clamp's derivative is exact
  where it binds" fails its precondition on two rows, and `S083_tuner_preconditions.cpp clamp` answers **no push from
  +10 to +200**: on the base 106 rows +30 was already the only push its rule took (every other binding push's finite
  difference past 2e-7, 3.4e-7 at +40, at `passed_king_enemy_mg[5]`), and the pair takes that one away. Tried and
  reverted: S121's eligibility with a clause keeping a pick a centipawn from the kink at CLAMP_PUSH picks another KBN
  pair (`8/1b6/3n4/K7/3k4/8/8/8 b`), and then the gradient case reads 1.01e-6 at parameter 112, red, and the clamp
  rule's +30 3.22e-7 at 141, so it trades one red for two. DEC-316's class; ruling (a), one DEC-316 pair, tried
  and taken out again, did not settle it (below, "The rulings on phase 1").

### Gates

Both builds without a warning (`-Werror`). `ctest -L fast -j12` under the corpus's load, the delivered tree after the
rulings, 59 entries with `test_tuner_box`: **`build` 45 pass at their clock, `build-tune` 46; every other entry but one
times out at its clock and passes run alone and unclocked** -- `build`'s 13: `test_movegen` 125 s, `test_search` 113,
`test_engine` 64, `test_leaf_walk` 129, the plant entries base 142, pawns 312, threats 415, king_safety 121 and
endgame 138, `test_mate_breadth` 328, `test_mate_carry` 279, `test_fastchess_script` 79, `test_plan_params` 37;
`build-tune`'s 12, the same less `test_plan_params`, 45 to 290 s -- **and `test_tuner_gradient` fails 2 of 61092
assertions, the clamp case's precondition above, in both builds** (`build`'s timed out at its clock and was run alone,
`build-tune`'s failed within it). `test_tuner_box` passes in both, 0.9 and 1.0 s.
`test_clang_format_script` with `CLANG_FORMAT_MAJOR=22`. `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` clean.
`tools/plan_prose_check.py`, each mode alone, exit 0: `--touches` 0 flagged, `--params`, `--citations` 0 flagged,
`--gate` agrees, `--prose` 0 flagged. `chesso bench` 5482277 in both builds. **Tier 2 (DEC-141) does not bind**: no
`make_move`, `unmake_move`, generator or search change and no pruning, reduction or extension rule; under `src/` only
`evaluation.cpp`, `evaluation.hpp` and `eval_trace.hpp`. `tools/gate_extra.sh` was not run. DEV_MANUAL.md updated (the
gradient check's counts and the factor's case, `eval_spread`'s and `truncation_scan`'s view of the factor, the groups,
parts, tie row and box, `feature_audit`'s cell columns, support lines and the corner dependency, the coverage blocks and
the two goldens' rows; since the rulings the 482-parameter count, the tie refusal, `test_tuner_box` and both goldens'
rows again); MANUAL.md read and unchanged: no option, command or output of the engine moves.

**The truncation bound, DEC-350 (1): the conditional form, by the coordinator's ruling on the fast check.** First
built unwidened: on the `forced` build the case was green at 3 x 23/24 over its positions, none of which is a cell row
past 2.875. `truncation_scan` on that build over S083's corpus then found **one** row past it, `8/5k2/8/5K2/5NRP/8/8/5r2
b`, a row in a cell, at 2.891927, under 2 x 23/24 + 3071/3072 = 2.9163 (six more at 2.875 exactly), and the ruling took
that as the evidence the brief asked for. So the case's bound is now per row (`truncation_bound()`, DEC-350 (1) cited
there): a row whose cell is off F gets 2 x 23/24 + 3071/3072, every other row keeps 3 x 23/24, unchanged at the
identity. That row and its mirror are `cell_truncation_positions`, in the case and in the forced-cell case, whose
precondition now asserts a cell row past 3 x 23/24 (2.89193). Readings below, "The rulings on phase 1".

### The rulings on phase 1

The coordinator's two messages of 2026-11-04: rulings on the first report, then the fast check's six findings. Every
new guard below was seen red on its plant before it was read green; the readings are `adocs/data/S124_phase1_guards.txt`'s
and `adocs/data/S124_preconditions.txt`'s.

**Ruling 1, the clamp case: (a), DEC-316's form as DEC-343 applied it. Tried; it does not land, and I stopped as the
ruling says.** `adocs/data/S124_gradient_rows.cpp` (new) is `S101_gradient_rows.cpp`'s rule in a mode for the clamp
case's setting: the gradient read at the shipped constants with +40 on the eighteen king safety weights, the case's
labels, K 1; S121's eligibility and the case's own clause, each row of the pair more than a centipawn from the kink at
+40; TARGET `passed_king_enemy_mg[5]` (996), MULTIPLE 3; one pair. Over the 124 rows the base reads -4.759e-5 at 996;
153 eligible pairs tried, the pick `7K/1k6/8/8/8/8/pppppppp/NNNNNNNN w - - 1 2` and its mirror, which takes 996's
relative error at +40 from 3.40e-7 to 9.90e-8. `S083_tuner_preconditions.cpp clamp` over the 126 rows then still
answers no push from +10 to +200: +30 keeps the KBN coverage pair 0.75 cp from the kink, which no added row moves, and
+40's worst moves to 984, `passed_king_own_mg[5]` -- S101's own DEC-316 target -- at 3.39e-7, past 2e-7. The main
gradient case read 4.92e-7 at 992 with the pair, green; the clamp case red on the same two checks. The pair is out
again (tests/test_eval_positions.hpp at its 124 rows); the program stays as the reading, its header saying it did not
land, and on the delivered tree it prints its pick and reads the marker absent. (b), (c) and a second pair were not
tried.

Draft lines for the coordinator's decision:

- *Context:* S124's coverage block puts its KBN pair 0.75 cp from the clamp's kink at `CLAMP_PUSH` +30, and
  `S083_tuner_preconditions.cpp clamp` answers no push from +10 to +200 over the 124 rows. DEC-316's one pair, picked
  by `S101_gradient_rows.cpp`'s rule at the clamp case's setting (`adocs/data/S124_gradient_rows.cpp`), takes +40's
  failing parameter, `passed_king_enemy_mg[5]`, from 3.40e-7 to 9.90e-8, and the next, `passed_king_own_mg[5]`, then
  reads 3.39e-7: one pair does not settle it.
- *Decision:* the coordinator's, among (b) the rule's margin from 2e-7 to 4e-7, which answers +40 on the 124 rows and
  still +30 on the base 106; (c) the coverage rule's kink clause, which trades this red for two (above, "Coverage and
  the preconditions"); or more than one pair, which ruling 1 excluded.

**Ruling 2, S310's positional member: by identity.** S310's step file and the script's header both name the member as
the fixture less DEC-316's pair at S101's candidate, "the rows S101's fit went chaotic without"; "less its last pair"
was how S310 found it, the pair being last then, and S124's block made it stale. So `dec316_pair()` finds the pair by
its marker comment in tests/test_eval_positions.hpp (exit 2 if absent or not in the compiled list), and
`write_gauge_fixture()` drops it by identity, every other row keeping its own label and score. **Re-derived: the rule answers 3, not the
first report's 4.** The identity member is the harsher one: its four starts (shipped, 1, 3, 10 cp) first pass 1e-9 at
10, 6, 7, 7, where the positional member's, which had dropped S124's last pair, did at 12, 9, 9, 9 and the family's
onset was 8 (the case's own fixture, 12, 10, 8, 10, unchanged). Onset 6, half 3, and the shipped constants at 3 walk
1.7570 (176 times the floor) and travel 1428.1. `ADAGRAD_HOLD_STEP` is 3 and the script, run again with it, exits 0:
at 3 `apart` 1.14e-12 at epoch 300, at most 2.16e-12 through 1200 on the case's fixture and 5.24e-11 over the family,
4.3 decades under the bound, every hold at most 1.15e-14.

**Fixed on the way:** `adocs/data/S101_gradient_rows.cpp` asserted the fixture held exactly its base rows and its pair,
true until S124's block was appended; now at least that. It reproduces its pair (`yes`; 2.07e-6 to 1.11e-7 at 984).

**The fast check's six findings.**

1. *One pick, both libraries.* `scaled_taper()` computes the cell once, `endgame_cell(board, E1's sign)`, in both
   builds; the trace build alone also computes both candidates and records them, and nothing it records reaches the
   score. INV-6 re-read: `S124_objdiff.py` exit 0, every function identical, `endgame_family_counts()` the one new;
   `chesso bench` 5482277 in both builds. The guard over the non-trace library with cells off F:
   `adocs/data/S124_engine_eval.cpp` (new), built against the forced build's `libchesso_engine.a` and
   `libchesso_engine_trace.a`, prints `evaluate()` for every row of S083's corpus, compared line for line; the trace
   library is in turn held to `exact_score()` by the identity case and `truncation_scan`. Unplanted, 0 rows of 6226944
   differ; plant 12, the pick reversed in `chesso_engine` alone, 840136 differ; plant 11, reversed in both, 0 differ,
   and the forced identity case reads that one red, 796 checks. A plants-script guard, not a ctest entry: it needs the
   forced build.
2. *The box's call sites.* `tests/test_tuner_box.sh` (new, `test_tuner_box`, fast): 48 rows, KRP against KB labelled 0
   (cell 4, P=1 d=1) and KQ against KR labelled 1 (cell 2, P=0 d>=2), two of each a game; `--only endgame_scale --lr
   4 --epochs 200 --report 1`, so every epoch can be the emitted one; the header's and `--vector`'s factors in [0, 128]
   and equal, and the precondition that cells 4 and 2 sit at 0 and 128. Green, `128 128 128 128 0 128 128 128 128 128 128 128
   128`; red with the in-loop call left out (plant 13, `header factor 2 is 422`) and moved before the step (plant 14,
   `132`). The call before the loop is not
   guarded: it boxes the shipped constants, in [0, F] by a `static_assert`, after ties of scale 1, so no fit sees it.
3. *Ties on boxed rows.* `parse_ties()` refuses an `endgame_scale` tie of any scale but 1, "a tie on endgame_scale
   takes scale 1: the box after the ties would break any other". `test_tuner_groups` "a tie names S124's factors by
   cell, one half and no suffix" gained 0.5, 2 and -1 refused and an explicit 1 accepted: red before the fix, 6 of its
   20 assertions failed (the three refusals' two checks each); green after, 29042 assertions in the binary.
   DEV_MANUAL.md's sentence says so.
4. *DEV_MANUAL.md's count.* "all 1190" is now the 482 parameters the product reaches, with the reason: over all 1190
   the worst is 2.4e-5 at `backward_rank_mg[0]`, rounding over a gradient that cancels on endgame rows.
5. *The truncation bound, conditional* (above, "Gates"): `truncation_bound()` in tests/test_eval_trace.cpp, DEC-350
   (1) cited there; `cell_truncation_positions`, the scan's 2.89193 row and its mirror, in both cases. With every cell
   at F the case is green over 9000 assertions and red on plant 10, the smooth score moved 0.03 on every row at F (4
   checks), so a row at F keeps 3 x 23/24. With plant 9, the old bound on every row, the forced-cell case is red on the
   two rows (2 checks), and so is the forced build's bound case; unplanted both green, the forced build's worst gap
   2.89193.
6. *"0 is a draw"* corrected in src/evaluation.hpp's comment, in "The design as built" above and in the specs draft.

### Draft wording for specs.md, the coordinator's

- The `evaluation` row, after S122's sentence on its landing: "**S124's endgame family sits in the cheap stage at its
  identity** (2026-11-04, DEC-350): a scale factor f_c in [0, 128] per cell multiplies E1 -- `board->material` plus
  stage one's positional pair's endgame value -- inside stage one's one taper, `material + (128 X + (f_c - 128) E1 (24
  - phase)) / 3072` with X `taper()`'s numerator, so at 128 it is `taper()`'s integer; thirteen cells by counts, the
  first that holds: a lone king (every dead position among them) none; one bishop a side on opposite colours, by the
  strong side's pawns P <= 2, 3 to 4, >= 5 with nothing but pawns, one cell with a rook a side, none otherwise; and P 0
  to 2 by d, the strong side's men in phase units less the other's, <= 0, 1, >= 2; the strong side E1's sign; at 0
  stage one keeps (material + the pair's middlegame half) phase / 24, E1's share alone removed. Mop-up's
  three linear pairs sit in the same pair in its domain, a lone king against a side with no pawn and two phase units or
  more, knights alone excepted: the lone king's centre Manhattan distance, 14 less the kings' Manhattan distance, and
  for K+B+N 7 less the lone king's Chebyshev distance to the nearer corner of the bishop's colour. At 128 and zero both
  are in no expression of the search's evaluation (INV-6, `adocs/data/S124_inv6.txt`); stage two and its clamp are
  untouched. The trace records both candidate cells and the tuner picks by E1's sign at the weights it holds."
- The `absent, evaluation` row: "the tuner carries ..., S122's 50 ... and, since S124's landing, S124's 19 -- the
  thirteen factors at 128 and mop-up's six at zero, in no expression -- making 1190 fitted and 1095 shipped (1171 and
  1095 before S124's landing; ...)".
- The `absent, evaluation model` row: "endgame scaling factors" leaves the absent list for "endgame scale factors and
  mop-up, present in the code since S124's landing at the identity and in no expression (row above)".

### Pre-registered: the screen, a draft (DEC-350's parts and order, S122's form)

Every corpus figure below is the research pass's, on the void corpus, **to be re-read on S122's regenerated corpus**.
Nothing here is fixed until the coordinator writes the screen's script and this becomes its header; a change after
the first fit is a dated section here and a decision.

- **The corpus**: S122's regenerated prepared corpus (DEC-349) and its own held-out games, at the landed tree's K; the
  labels the games' results under the laws (DEC-349 (5), DEC-350 (9)). The identity rule before any fit:
  `feature_audit --free endgame_scale` on it -- at the landing, on S083's corpus, rank exactly 1118 of 1190, nullity
  72: S122's 71 (its 34 zero columns and 37 dependencies by construction) and the corner pair, 0 further; to be
  re-read -- every cell's column non-zero, and the corner pair held by the freeze.
- **P0, a reading: head's conversion test.** Starts: distinct training-side root positions of mop-up's domain, one a
  game, 200 each of KRvK, KQvK, KRBvK, KRNvK, KBNvK and every KBBvK game's; each start's clock zeroed and kept only
  where Syzygy reads 2 there; self-play at 100000 nodes a move under the laws, no adjudication; converted = mated within
  50 moves. About 10 min. If head converts every start, mop-up is recorded zero and P1 is not run.
- **P1, mop-up's SPSA** over its pairs on P0's objective (plies to mate, the bound for a failure), about 3 h, **the
  corner as one parameter** -- its middlegame half tied or frozen, the pair being one direction (above) -- then the
  conversion test on fresh starts; kept iff McNemar's one-sided z > 1.645 over head and no signature converts fewer.
  The SPSA instrument is not built (below).
- **F1, F2, F3**, `--only endgame_scale.f1`, `.f2`, `.f3`, in that order, the old constants frozen, earlier survivors
  live, each paired against the landed tree on the regenerated corpus's own held-out games, kept iff d > max(1.645
  sd(d_s), 2e-6) over five quarter samples, every fit `--epochs 20000 --report 100 --patience 20` (DEC-345 (1)); F3
  with `--tie` holding OCB_PURE P >= 5 to P 3-4 at scale 1 (`endgame_scale 11 10`) unless that cell's support reads
  1000 games or more (DEC-326 (5)). OCB_PURE P <= 2 stood on 1047 games on the void corpus and stands on 905 on
  S083's: if under 1000 on the regenerated one it has no supported neighbour in F1, and that stops for the
  coordinator before F1 is fitted. Research figures, void corpus: F1 -256 in sample, F2 -290, F3 -37 (1e-6, all rows).
- **The tablebase guard**, every part: the training side's rows of five men or fewer labelled by the tables at each
  row's own clock (WON 1, DRAWN and SPOILED 0.5, LOST 0, EDGE left out); a part raising that loss by more than 1.645
  whole-game errors is refused whatever its held-out d. F3 has no such row and is screened on the loss alone, and its
  stamp says so; F1's OCB_PURE cell is guarded on its P=0 and P=1 rows alone.
- **The cost**, `S124_phase1_cost.sh`'s sink at the candidate's masks; it decides nothing, the verdict carries it.
- **The SPRT**: one `{0, 5}` nElo of the survivors, 8+0.08, Hash 16, `books/noob_3moves.epd`, concurrency 12, against
  the landed tree; 41861 games and 19.84 h at the midpoint, 25591 and 12.13 h on a bound; cap 20950 rounds, 41900 games,
  19.86 h; S101's abort rule; an A/A of 1000 games first if the harness moved. The two-sided resign at 400 cp hides most
  of mop-up's conversions from the verdict, and this pre-registration says so.
- **On H0 or no verdict**, in order, each `{0, 5}` against what has landed, survivors only: A = mop-up, B = F1, C = F2
  and F3's survivors.
- **Machine hours, inferred**: P0 10 min; P1 about 3 h 10 m; a part's six fits about 36 min at patience to 4 h 50 m at
  the cap; the guard minutes; the cost about 1 h; the SPRT 12.1 to 19.9 h.

### Open, and what phase 2 builds

- **The clamp case's golden**: red, and ruling (a) did not settle it (above, "The rulings on phase 1"). What is left
  is the coordinator's: (b), the rule's margin, which answers +40 on these 124 rows by the table above, or more than
  one pair, which the ruling did not allow.
- **Mop-up's SPSA instrument**: not built. DEC-350 (4) fits mop-up by SPSA on conversion; the tune build exposes no
  evaluation weight over UCI, so the six would need tune-build options -- a UCI surface change (`test_uci_surface`,
  MANUAL.md, specs.md) -- or a driver that rebuilds per point; and `tools/spsa_driver.py` plays matches, not conversion
  starts. Phase 2's, with P0's conversion script.
- The screen's script, `S124_screen.sh`, is not written; the draft above is its header.

### Findings beyond the step, not fixed

- (a) **Mop-up's corner pair is one degree of freedom**, DEC-350 (4)'s "three linear pairs" notwithstanding: KBN
  against a king alone is always at phase 2, so the engine, the loss and an SPSA all see 2 mg + 22 eg of it and nothing
  else. P1 should fit it as one number (above). Not changed here: the ruling names three pairs.
- (b) `adocs/data/S310_adagrad_step.cpp`'s positional member: fixed by identity, the coordinator's ruling 2 (above,
  "The rulings on phase 1"); its rule then answers 3.
- (c) S122's screen pins the layout: `adocs/data/S122_screen.py` `PARAMS = 1171`, `EXPECTED_RANK = 1100`, and
  `.tuning/verify_fit.py` reads 1171 (S122's record). On a tree with this phase the audit reads rank 1118 of 1190, so
  an S122 `audit-check`, `apply` or `verify_fit` run there refuses; and `tools/tuner`'s header now emits `mopup_mg`,
  `mopup_eg` and `endgame_scale`, which `.tuning/apply_fit.py` (not read) would meet. Harmless while S122's screen
  runs on its frozen tools; the coordinator's order to settle.
- (d) The step's `touches:` (DEC-350 (11)) lacks what this phase also touches: `tests/test_eval_model.cpp`,
  `tests/test_eval_positions.hpp`, `tests/test_endgame_positions.hpp` (new), `tests/test_tuner_groups.cpp`,
  `tests/test_evaluation.cpp`, `tests/CMakeLists.txt`, `tools/tuner_groups.hpp`, `tools/tuner_optimiser.hpp`,
  `tools/truncation_scan.cpp`, `tools/eval_spread.cpp`, `DEV_MANUAL.md`. The header is the coordinator's.
- (e) Environment, not code: a command this harness moves to the background runs its later children at `SCHED_IDLE`
  (or nice 19 under `nice -n 10`), starved by the corpus's nice-0 threads; long runs here went through `nohup setsid
  nice -n 10`, which keeps them at nice 10. `.moltke.local.md`'s watcher section may want the note.

## 2026-11-05: phase 1 rebased onto S122

One Opus 5.5 subagent, coordinated; worktree `../chesso-s124p1b`, branch `s124p1b` at `bcc41f3`, S122's landing. Phase
1's diff (`c3c8f2d2...`, 40 files on `d83d6c0`) applied with `git apply --3way --index`, staged and not committed. No new
design; nothing fitted, timed or played. Every reading with its command is in `adocs/data/S124_rebase_goldens.txt`;
INV-6's is the second section of `adocs/data/S124_inv6.txt`. The sections above are phase 1's on `d83d6c0`, kept as
history. **Two reds remain that neither of the brief's stops names, and the diff does not land until they are ruled**
(below, "Two reds").

### The conflicts, each resolved by intent

Four of the nine overlapping paths conflicted. The other five (`adocs/data/README.md`, `adocs/data/S101_gradient_rows.cpp`,
`src/evaluation.cpp`, `src/evaluation.hpp`, `tests/test_evaluation.cpp`) merged cleanly and were read after.
- `tests/test_eval_positions.hpp`: S122's re-picked DEC-316 pair (candidate 5, with its DEC-354 comment) is kept. S124's
  block follows it, and the S101 pair that phase 1's side still carried is dropped. The block then re-derives unchanged
  (below).
- `tests/test_eval_trace.cpp`: S122 made the bound case's bound a local, `3 x 23/24 + 1` while its curve is live. S124
  made it per row, `truncation_bound()`. The curve's `+ 1` moves into `truncation_bound()` and into the forced-cell case's
  `plain_bound`, so that a row at F has one bound in both cases. The curve's truncations are in stage two and the factor's
  division is in stage one, so the two add. This is the merge that turns the forced-cell precondition red (RED 2).
- `tests/test_tuner_gradient.cpp`: `ADAGRAD_HOLD_STEP`'s and the walk floor's comments keep both histories, with the values
  re-derived. Phase 1 had put its endgame recovery case between S122's curve-recovery comment and that comment's own
  case, leaving the comment over the wrong case. The endgame case now sits before the comment, and S122's new paragraph
  ends it. A trivial misplacement, fixed in scope.
- `DEV_MANUAL.md`, three conflicts: S122's text is kept and S124's additions are laid on it with the rebased readings. They
  are the gradient check's paragraph and the rows for the coverage blocks, `CLAMP_PUSH` and `ADAGRAD_HOLD_STEP`.
- Beyond the text: `adocs/data/S124_coverage_rows.cpp` and `adocs/data/S124_gradient_rows.cpp` each carried a copy of
  `stage_two()` without S122's curve. S122's phase 2 fixed that copy in the other tools (DEC-354). Each now adds the curve
  in the same form; the coverage block's picks are unchanged by it.
- `adocs/data/S310_adagrad_step.cpp` finds DEC-316's pair by its marker, whose first line still reads "at S101's
  candidate". The two rows after it are now S122's re-pick, so the script's header, the function's comment and its
  error message say so. The identity pick drops S122's pair, and the logic is unchanged.

### Where the factor sits against S122's terms

The factor multiplies E1, `board->material + eg_value(pair)`, the pair being the psqt sum, the pawn families, placement
and mop-up, inside stage one's one taper in `cheap_score()`. S122's terms are in stage two: the checks, weak squares,
storm and curve, beside the linear five, in `evaluate_expensive()`'s mobility and king safety sum. That sum has its own
taper and is clamped at `LAZY_EVAL_MARGIN` 275. So the factor never scales a king safety term, and the clamp never sees
the factor. The two meet only in the residual between the smooth and the exact score, where each adds its own
truncation. **The identity holds at the new base:** with every factor at 128 and mop-up at zero, nothing scores
differently from `bcc41f3`.
- `chesso bench` reads 4707943 in both builds, `bcc41f3`'s.
- `tools/search_bench.py` against `bcc41f3`'s own binary gives node counts and best moves identical at depth 9 (c3d5
  25473, e2a6 127664, d7c8q 34197) and at depth 12 (c3d5 94541, e2a6 353715, d7c8r 409231). These are S122's phase 2
  streams.
- `adocs/data/S124_objdiff.py`: every function identical, `endgame_family_counts()` the one new.
- The cell census, `S124_cell_check` over S083's corpus and S082's three views, reads `adocs/data/S124_cells_check.txt`
  line for line, every block with 0 rows differing.
- The engine-against-trace guard, `S124_engine_eval.cpp` on a forced build, reads 0 rows of 6226944 differing. Plant 11
  reads 0 and plant 12 reads 840136, as on `d83d6c0`.

### Every golden, each by its script

| golden | phase 1 on `d83d6c0` | S122's landing | rebased | script |
|---|---|---|---|---|
| DEC-316's pair | S101's | candidate 5 | **candidate 5, reproduced** | `S101_gradient_rows` |
| S124's coverage block | 9 pairs; smallest gradients 3.06e-8, family 9.73e-7 | -- | **the same 9 pairs, reproduced**; 3.117e-8, family 1.020e-6 | `S124_coverage_rows`, S083's corpus |
| S121's, S123's, S125's, S101's blocks | reproduce | reproduce | **reproduce** | their four tools |
| the gradient case's worst | 4.63e-7 at 992 | 8.61e-7 at 984 | **8.60521e-7 at 984**, x1.16 | `test_tuner_gradient` |
| `CLAMP_PUSH` | no push, +30 red | +50 | **+40**: 4 rows bound, the farthest at 280.4, the nearest other at 253.3, every row 2.18 from the kink, finite differences 1.12e-7; +50 reads 2.05e-7 | `S083_tuner_preconditions clamp` |
| the fixture clear of the clamp | 113.3 against 183 | 174.4 against 274 | **174.4 against 274** | the same |
| `ADAGRAD_HOLD_STEP` | 3, onset 6 | 2, onset 5 | **3, onset 6**; apart 5.68e-13 at 300, the family at most 7.5e-12 | `S310_adagrad_step` |
| gauge walk, Adam / AdaGrad | 2.3017 / 1.7570 at 3 | 1.2516 / 0.3832 at 2 | **0.6927 / 1.3259 at 3**, both the rook's; apart 1.14e-12 / 5.68e-13, holds at most 1.04e-14 | `S083_tuner_preconditions walk` |
| the walked counts | 179059 assertions, 17794 positions | -- | **unchanged**; the counts read no weight | `test_eval_model` |
| the factor's gradient case | 3.02e-7 at 726 | -- | **3.4757e-6 at 726, red (RED 1)** | `test_tuner_gradient` |
| `cell_truncation_positions`, the forced-cell precondition | 2.891927 past 2.875 | -- | **no re-derivation; red (RED 2)** | `truncation_scan --min` on a forced build |
| forced `truncation_scan` | 7 past 2.875, worst 2.891927 | -- | 7030 past 2.875, worst 3.690165, every row's identity holding | the same |
| `feature_audit --free endgame_scale` | rank exactly 1118 of 1190, nullity 72 | -- | **not computed**: the tool's own refusal while S122's curve is off its seed; every support line unchanged | `tools/feature_audit` |
| ruling (a)'s pair | did not land | -- | moot: no pair among 784, exit 1 by its own clause | `S124_gradient_rows` |

Truncation and spread: S122's four `truncation_positions` and its bound case are green with S124's rows (worst 3.67232
past 3.6). Phase 1 changed no spread case. The plants, run again on scratch builds (the record's last section): every
plant red on `d83d6c0` is red again by its own check, except plants 9 and 10 (below, RED 2).

### The brief's two stops: neither fires

- (a) The gradient case's worst is 8.60521e-7 at 984, its rows as the rules pick them, under 1e-6. Not DEC-344 (9)'s third
  recurrence.
- (b) `CLAMP_PUSH`'s rule answers +40 on the rebased fixture. So `adocs/data/S124_clamp_options.txt` was not written.

### Two reds, not settled here: the coordinator's

Neither stop names these, and neither can be made green without changing a test, so both are left as they read. Each is
red in both builds; `ctest -L fast` reads 57 of 59 in `build/` and in `build-tune/`.

**RED 1, "the endgame factor's gradient is the derivative, chain rule in": 3.4757e-6 at parameter 726,
`psqt_eg[king][b6]`, against 1e-6.** Phase 1 read 3.02e-7 at the same parameter. The case's rows are S124's population,
which no rule picks and no weight moves, so S122's constants moved the error and not the rows.
- 726's analytic gradient, -1.98e-9, is 1.1e-5 of the largest the case reads.
- Numeric and analytic agree to about 6e-15 at every h. The relative error reads 2.1e-6 to 4.9e-6 from h 0.04 to 0.0025,
  flat in h, and Richardson's difference reads 3.99e-6. This is the summation's rounding floor over a weight nearly
  stationary on these rows, DEC-316's class, and the derivative is right.
- 1 of the 482 parameters is past 1e-6; the next, 739, reads 5.9e-7. The normwise reading is 5.63e-9.
- In DEC-354's names: B (smaller h) is red, F (Richardson) is red, C (normwise) is green, and D (rows) has no rule that
  picks rows for this case.

So only a metric change or a new row rule answers it, and either is a ruling. The case uses the same file and the same
1e-6 bound as DEC-344 (9)'s case. Whether it counts as that recurrence is the coordinator's to say; by the brief's words
it does not.

**RED 2, the forced-cell case's precondition, `worst_scaled > plain_bound`: 3.01941 against 3.875; and its golden
`cell_truncation_positions` has no re-derivation.** With S122's curve live, a row at F is bounded by 3.875 and a row in a
cell off F by 3.9163. The precondition needs a cell row past 3.875, and S083's corpus has none at the forced cells:
- The scan's worst cell row reads 3.544352, and the worst row of all 3.690165.
- With the curve held at its seed (a scratch probe over the whole corpus), the worst cell row reads 2.864258, and none
  passes 2.875.
- The golden's row itself reads 2.338053 with the curve live and 2.058594 with it at its seed. S122's linear weights
  moved it.

Two guards are blind at the new base for the same reason:
- **Plant 9**, the old bound on every row, now reads green on the forced build, and in the forced-cell case only the
  precondition fails.
- **Plant 10**, the smooth score moved 0.03 on every row at F, reads green in the bound case. The bound at F is 3.875
  and the worst row is 3.67232.

Options, each the coordinator's:
- (i) A per-row curve allowance, `+ 1` only where the two kings' T can differ. This is new design for both cases.
- (ii) The forced-cell case holds the curve at its seed, as it forces the cells. Even then S083's corpus has no evidence
  row, so it needs another corpus, S122's prepared one, or a constructed row.
- (iii) The precondition and plant 9's guard read only while the curve is at its seed, in S122's own form for its
  curve-constant clause. This is a recorded weakening while the curve is live.

### S122's pinned tools, finding (c) above: the coordinator's ruling

S122 is closed. `adocs/data/S122_screen.py`'s `PARAMS = 1171` and `EXPECTED_RANK = 1100` stay as they are, a tool
re-derived at its own tree in DEC-347's form, and one comment line beside the pins says so. The scripts under `.tuning/`
are gitignored and were not touched.

### Finding (d) above: `touches:` widened

The header now names every path phase 1 touched outside its own new data files, and the three other steps' tools it
edits.

### The pre-registration draft, what moved

- "The identity rule before any fit", `feature_audit --free endgame_scale`, no longer reads a rank at the landing. Its
  identity report is "not computed" while S122's curve is off its seed, so the screen's header needs another form for
  that rule.
- The supports are unchanged: OCB_PURE P<=2 on 905 games, P=3-4 on 1657, P>=5 on 798. Mop-up's corner is on 196 games.

### Gates

- Both builds: no warning. `chesso bench` 4707943 in both.
- `ctest -L fast -j10` in `build/` and `build-tune/`: 57 of 59 each. The two reds are RED 1 (`test_tuner_gradient`) and
  RED 2 (`test_eval_trace`). Every other entry is green within its clock, the machine idle but for this work.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` is clean.
- `tools/plan_prose_check.py`, each of the five modes exit 0: `--touches` and `--citations` 0 flagged, `--params`,
  `--gate` agrees, `--prose` 0 flagged.
- Tier 2 (DEC-141) does not bind: the rebase adds no `src/` change beyond phase 1's.
- The cost was not run again, as the brief says.
- `DEV_MANUAL.md` takes the rebased readings and the curve's allowance in the truncation scan's paragraph. `MANUAL.md`
  was read and is unchanged.
- `tools/truncation_scan.cpp`'s S124 header and usage lines gain the curve's allowance.
- `adocs/data/S124_phase1_plants.patch` is regenerated so that it applies without fuzz: one hunk's context moved with
  `truncation_bound()`'s curve term.

### Findings beyond the rebase, not fixed

- `feature_audit`'s identity report is not computed at S122's shipped constants (above). Every later family's
  rank-before-fit rule, S126's included, meets the same refusal.
- S122's own truncation bound at F, 3.875, sits 0.185 above the corpus maximum of 3.690. So `test_eval_trace`'s bound
  case cannot see a residual error under that, which is plant 10's blindness, S122's form and not S124's.
- `DEV_MANUAL.md`'s DEC-142 table has no row for phase 1's golden `cell_truncation_positions`. That was phase 1's
  omission, left until RED 2 is ruled.
- `adocs/data/S124_gradient_rows.cpp` answers a question DEC-354 withdrew. Its header now says it is moot, and it stays as
  the record of the reading on `d83d6c0`.

### 2026-11-05: the coordinator's rulings on the two reds, read

**RED 2, option (i) as ruled: it cannot work, so it was stopped before any change and (ii) was not tried.** The rule
gives a row its +1 only where S122's curve is non-zero there at the constants under test: either king, either half, x >
0. The shipped curve's endgame constant is 229, and over S083's corpus that rule fires on every row, 6226944 of
6226944, both at the shipped constants and at the case's forced cells (`adocs/data/S124_rebase_goldens.txt`, its last
section). So every bound keeps its +1:
- the precondition stays 3.01941 against 3.875;
- plants 9 and 10 stay green;
- S122's bound case at F keeps its 0.185 of slack.

Nothing of (i) is in the tree. For the ruling, the same probe also read the tightest per-row form, the curve's own
residual. That is `|d_mg phase + d_eg (24 - phase)| / 24`, where d_h is the two kings' fractions of T(x) in a half,
White's taken off.
- It holds on every row and is reached exactly.
- On the forced build one cell row passes a row at F's bound by 0.0078, `1k2r3/1pp5/p7/5q2/3Q4/2N3P1/5P2/3R2K1 w`.
- 13 rows (16 at the shipped constants) sit within 0.03 of their bound.

So that form would let `cell_truncation_positions` be re-derived and plants 9 and 10 read red, but it is not the rule
as ruled. `DEV_MANUAL.md`'s golden table gains the `cell_truncation_positions` row, which states that the golden is
pending.

**RED 1, the owner's: options A and B read, nothing applied** (`adocs/data/S124_factor_gradient_options.txt`).
- **A**: DEC-316's rule, written before it ran. The candidates are the population's own generator at twice the
  placements, with S101_gradient_rows.cpp's eligibility plus the case's E1 filter. It picks one pair after 157 eligible
  ones, `8/8/8/3r4/8/1k6/8/K3R3 w` and its mirror, which takes 726's gradient x65. The case then reads 6.27485e-7 at 739,
  `psqt_eg[king][g5]`, x1.59 under its bound, so the rule stops after one pair. With the pair appended to `population()`,
  every reader stays green and moves no golden. The only red left is RED 2's precondition, unchanged.
- **B**: normwise reads 5.632e-9 here. On the two earlier recurrences it reads S101's 2.63e-7 and at most 8e-9 for
  S135's, so it would have caught none of the three.

**`feature_audit`'s refusal, a finding for phase 2, not built.**
- The identity report has a column for S122's curve only where every side of every row shares one x per half. That
  holds at the seed, where x is the constant on every side. There each half's column is one slope times the integer
  `n_W,i - n_B,i` (`group_slopes()`).
- S122's own rank, 1100 of 1171 with nullity 71, was read there: phase 1 at the seed, where the constant's two columns
  are zero and the curve's other inputs' 22 columns duplicate the linear cells.
- At the shipped 1245 curve each king has its own x, so the tool refuses.
- What it lacks is the curve's columns linearised at the shipped x, row by row:
  `sign (max(x_W,h, 0) n_W,i - max(x_B,h, 0) n_B,i)` per half, times the half's taper weight.
- At integer weights x is an integer, so T'(x) = 2 max(x, 0) / 2048 is that integer over 1024. The common factor 1024
  changes no rank, and the column stays exact for the modular rank.
- The rank read then is the design's local rank at the shipped point. The curve's columns no longer duplicate the
  linear five there, so the by-construction count is not S122's 22. The pre-registration's identity rule must be
  restated for it.

### 2026-11-06: DEC-355 implemented

One Opus 5.5 subagent, coordinated, in `../chesso-s124p1b`, staged on the rebase and not committed. Nothing fitted,
timed or played. Every reading with its command is in `adocs/data/S124_rebase_goldens.txt`, its section "DEC-355
implemented".

**The two pick rules, amended before either script ran** (2026-11-06, each also at its golden's site and, for
`truncation_positions`, in `adocs/data/S083_truncation_pick.py`'s docstring):

- **`cell_truncation_positions`.** `truncation_scan` reads each row less its own curve residual since DEC-355, so
  `truncation_scan --data <corpus> --min 2.875` on the `-DS124_FORCED` build prints the rows past the bound a row at
  F would have in their place, which only a row whose cell is off F can pass. The golden is the hit with the largest
  reading, ties to the scan's first, and the cases add its mirror. With no hit there is no golden and the forced-cell
  precondition is red.
- **`truncation_positions`.** The scan's first column is the difference less the row's curve residual since DEC-355:
  on the 1/24 lattice again, its maximum 69/24, and past 69/24 on no row at F. So `S083_truncation_pick.py` keeps the
  hits at the scan's largest reading, each FEN once, as S083's rule wrote it, and S122's branch past 3.0 cannot fire.
  The command is unchanged: `truncation_scan --data <corpus> --min 2.8` on `build/`, then the script on its stdout.
  "The pinned positions reach the truncation bound" and the bound case's top clause read the same per-row quantity,
  past 2.0 each and the worst past 2.8.

**What changed.** `tools/tuner_trace.hpp` gains `curve_residual()`, the curve's exact share of a row's residual at the
integer weights read, `|d_mg phase + d_eg (24 - phase)| / 24` from each king's `(max(x, 0)^2 mod 2048) / 2048`, and
`truncation_bound()`, the taperings' per-row bound plus that residual. `tests/test_eval_trace.cpp`'s own
`truncation_bound()` and `curve_is_live()` are gone: the forced-cell case and the bound case call the shared one, and
`tools/truncation_scan` holds every corpus row to it (a row past it is counted, the first named, exit 1). Where a case or
the scan reads how near a row comes to its bound, it reads the difference less the row's residual against the
taperings' part: the forced-cell precondition, the bound case's top clause, the pinned case's two clauses, and the
scan's printed column, past-2.0 count, `--min` and worst. At the curve's seed the residual is zero and nothing reads
differently. Tempo's term in the shared bound reads the weights' tempo, which is the engine's at every weights the cases
use.

**Red first.** Under the per-row form, before either script ran, three cases read red on the stale goldens: the
forced-cell precondition 2.57812 against 2.875, and the bound case's and pinned case's top clauses 2.75 against 2.8.
No row any case reads was past its own bound. S122's four read 2.667, 2.625, 2.625 and 2.750 so.

**Condition 1, the bound on every corpus row: holds.** `truncation_scan` over S083's 6226944 rows reads 0 past their
own bound at the shipped constants (`build/`, `--min 2.8`: 88743 past 2.0, 66 past 2.8, the worst 2.875, the largest raw
difference 3.690165) and at the forced cells (a `-DS124_FORCED` build, `--min 2.875`: 90088 past 2.0, 5 past 2.875, the
worst 2.882772). 16 distinct positions sit exactly on their bound at the shipped constants.

**Condition 3, the goldens re-derived by their scripts after the amendments above.**
- `cell_truncation_positions`: `8/5k2/8/5K2/5NRP/8/8/5r2 b - - 6 68` -> `1k2r3/1pp5/p7/5q2/3Q4/2N3P1/5P2/3R2K1 w - - 0 32`,
  the largest of the forced scan's 5 hits. The other four sit at 69/24 and pass 2.875 by an ulp. The row reads 2.882772:
  raw 2.894674 less its residual 0.011902, 0.0078 past a row at F's 2.875 and 0.0336 under its own bound. It reads
  2.1845 at F.
- The precondition `worst_scaled > plain_bound` now reads **2.88277 > 2.875**, green on that row: checked 8673, moved
  4082, worst 3.29171.
- `truncation_positions`: S122's four (phases 1, 9, 15, 23) -> `8/8/3K2k1/p2B2r1/5RP1/8/8/8 b - - 1 69`,
  `8/6bk/1p2p1p1/4p2p/5r2/P6P/2R2BP1/1r1B1K1R w - - 6 38`, `r4k1r/6p1/p1nN3p/5b1n/1P5P/PP4P1/4N3/R4RK1 b - - 0 25` and
  `r1bqkb1r/ppp1pp1p/n5p1/3p4/6P1/P1N2N2/1PPPPPP1/R1BQKB1R w KQkq - 0 6`. These are phases 5, 11, 13 and 23, each at
  2.875, from 16 distinct positions at the maximum, by `S083_truncation_pick.py` on the `--min 2.8` scan.
- The bound case's top clause and the pinned case's each read 2.875 against 2.8, and every pinned row is past 2.0.
- The bound case's `worst > 3.6` -> `worst > 2.8`, and the pinned case's `3.0`/`3.6` -> `2.0`/`2.8`. Each is read on
  the per-row quantity, the clauses S122's landing had replaced.

**Condition 2, plants 9 and 10: red by their own checks.** The plants patch is regenerated: plant 9's hunk moved with
`truncation_bound()` into `tools/tuner_trace.hpp`, and the patch applies with no offset or fuzz. Plant 9 joins the forced
scan's runs.
- Plant 10 (the smooth score +0.03 at F) is red in the bound case on 4 checks: the four pinned rows, one of each mirror
  pair.
- Plant 9 (the old bound for every row) is red by `gap <= truncation_bound` in three places: the forced-cell case
  (2 checks, the golden's row and its mirror), the forced build's bound case (2), and the forced scan (exit 1, 1 corpus
  row past its own bound).
- Plant 0 is green in every run. Every other plant is red by its own check.

**Not touched:** RED 1 stayed as it read until DEC-356 below. Neither `src/` nor `feature_audit` was touched.

### 2026-11-06: DEC-356 implemented, the owner's option A

The pair DEC-356 names is appended to `population()` after the generator's rows. To keep the pick re-derivable, the
generator is now `placements()` (the same loop, seed and order), and `population()` is `placements()` followed by
`factor_gradient_rows`, the pair, named at its site as a DEC-142 golden with DEC-356 and its script.

- `adocs/data/S124_factor_gradient_rows.cpp`: the options file's scratch program promoted, in `S101_gradient_rows.cpp`'s
  form. The rule is in its header, as written before it first ran, and so is its build line. Its base is `placements()`
  and its candidates `placements(80)`. It exits 1 unless the pick is `factor_gradient_rows` and `population()` is the
  generator followed by them.
- Run, it reproduces the options file's output line for line: base 5715 rows, 3.4757e-6 at 726; the pick after 157
  eligible pairs (candidate 233 of 11339), |g| x65; the case with 5717 rows at 6.27485e-7 at 739. It ends "reproduces
  the landed pair: yes", exit 0.
- Its check, planted with the pair's rows swapped in a scratch header, reads NO and exits 1.
- The readers of `population()`:
  - the factor case reads **6.27485e-7 at 739** over 5717 rows, green (RED 1 closed);
  - the walk's two S124 cases read 179059 -> 179079 assertions;
  - the forced-cell case reads checked 8673 -> 8675 and moved 4082 -> 4084, its worsts 3.29171 and 2.88277 unchanged.
  No other golden script reads `population()`.
- The plants on the final tree (`.tuning/s124/plants_dec356`): every plant is red by its own check and plant 0 is green
  in every run. Plants 5, 7 and 8 are now red alone on their own cases, where RED 1's failure used to sit beside them.

### Gates, both rulings in

- Both builds: no warning; `ctest -L fast -j10` reads **59 of 59** in `build/` and in `build-tune/`, with
  `CLANG_FORMAT_MAJOR=22`.
- `chesso bench` reads 4707943 in both. No `src/` change.
- `clang-format.sh --check` is clean, and `tools/plan_prose_check.py` exits 0 in all five modes.
- `DEV_MANUAL.md` takes three things:
  - the golden rows for `cell_truncation_positions`, `truncation_positions`, the bound case and the new pair;
  - the truncation scan's per-row paragraph;
  - the factor case's DEC-356 reading.
- `MANUAL.md` was read: it says nothing about the truncation bound, the scan or the tuner's fixtures, so it does not
  change.

**Beyond the task, not fixed:**
- `adocs/data/S136_screen.py` (S136, in `plan_todo/`) reads `truncation_scan`'s first column. Since the fast check's
  finding 2 that column is |smooth - exact - C|, on the 1/24 lattice its tempo arithmetic assumes wherever stage two's
  clamp does not bind (until then `raw - |C|`, which is not, the claim this note first made). Its rules were written
  before S122's curve and should be re-read when S136 starts.
  - *The coordinator, 2026-11-06:* S136 is in `plan_done/` (tempo's H0, 2026-10-28), not `plan_todo/`. Its screen is a
    tool of a closed step, read at its own tree in DEC-347's form, so nothing is re-read; a later tempo step re-reads it.
- `adocs/data/S083_truncation_pick.py` (its docstring) and `adocs/data/S124_phase1_plants.sh` are touched and are not in
  the header's `touches:`; the header is the coordinator's.
  - *The coordinator, 2026-11-06:* `S083_truncation_pick.py` is added to `touches:`. `S124_phase1_plants.sh` is this
    step's own data file, which the header does not list.

### 2026-11-06: phase 1 lands, the coordinator

DEC-355 and DEC-356 hold as read above. The staged tree (`7c02c575` on `bcc41f3`, diff sha256 `24eb9284...`,
`.tuning/coord/S124/S124_phase1_dec355.diff`) is applied to HEAD with `git apply --3way --index`; `adocs/data/README.md`
merged with S332's rows kept. `specs.md` takes the draft wording above, its counts read on S122's H1 (1190 fitted, 1143
shipped; the draft's 1095 predates that H1). Phase 1 lands at the identity, `No functional change`, bench 4707943.
Phase 2 next: `feature_audit`'s local rank first (above), then the screen at K refit (DEC-352).

### 2026-11-06: the fast check's three findings

The coordinator's fast check over the DEC-355 and DEC-356 delta found the arithmetic exact and nothing weakened. It
raised three findings, and all three are fixed here, staged.

1. **Ulp rows.** A row exactly at 69/24 passes 2.875 by an ulp: four of the forced scan's five hits were such rows.
   So "only a row whose cell is off F can pass" and "one hit" were false. After a refit the rule could pick an ulp row,
   the precondition would then pass by about 1e-16, and plant 9 would go blind under its 1e-9 slack.
2. **The abs form is a lower bound only.** `curve_residual()` returned |C|, so `raw - |C|` is the taperings' loss
   only where the loss and C share a sign. Elsewhere it is less, and the claims "on the 1/24 lattice" and "what the
   taperings lost" were false; the bound carried 2|C| of unused slack on those rows.
3. **A stale comment.** The factor case's comment in `tests/test_tuner_gradient.cpp` still read 3.0e-7 and did not
   name DEC-356's pair.

**The signed form, proved before adoption** (a scratch probe over S083's 6226944 rows, C computed from the side
entries apart from `tuner_trace`'s, read against the taperings' bound alone; `adocs/data/S124_rebase_goldens.txt`):
- At the shipped constants and at the forced cells, 0 rows are past the bound and the worst sits exactly on it.
- Stage two's clamp binds on 9539 rows, and none of them is past: the worst reads 1.9598, 0.915 under its bound.
- At F every row off the 1/24 lattice is a row where the clamp binds: 9467 at the shipped constants, 9098 at the
  forced cells, and none elsewhere.
- At the forced cells exactly one row passes 69/24 + 1e-6: the cell golden's, at 2.906576.

The arithmetic for the clamp: the clamped sum's two sides differ by t + C, the two taperings' losses and the curve's
share. Where both sides bind, the reading is |t_pos - C|. Where one binds, the clamp's step is an integer between 0 and
t + C, so the reading less t_pos lies between -C and t. Neither reaches past 23/24 + max(46/24, |C|) = 69/24.

**The two pick rules, amended again before either script ran** (2026-11-06, the fast check's findings 1 and 2; each
also at its golden's site and, for `truncation_positions`, in `adocs/data/S083_truncation_pick.py`'s docstring):

- **`cell_truncation_positions`.** The scan reads each row as |smooth - exact - C|, C the curve's signed residual, and
  prints a row only where that passes `--min` by more than 1e-6. So a row exactly at 69/24 is not printed. The rule:
  - `truncation_scan --data <corpus> --min 2.875` on the `-DS124_FORCED` build prints only rows whose cell is off F,
    since every row at F is bounded by 69/24;
  - the golden is the hit with the largest reading, ties to the scan's first, and the cases add its mirror;
  - with no hit there is no golden, and the forced-cell precondition, read past 2.875 + 1e-6, is red.
- **`truncation_positions`.** The scan's first column is |smooth - exact - C|:
  - wherever stage two's clamp does not bind, exactly what the three taperings lost, on the 1/24 lattice with its
    maximum 69/24;
  - where the clamp binds, bounded by the same and off the lattice.

  `S083_truncation_pick.py` keeps the hits at the scan's largest reading, each FEN once, as S083's rule wrote it. Its
  code is unchanged, and so is the command: `truncation_scan --data <corpus> --min 2.8` on `build/`, then the script
  on its stdout.

**What changed, and what it read** (every line with its command in `adocs/data/S124_rebase_goldens.txt`, its section
"The fast check's three findings"):
- `curve_residual()` returns C with its sign. The new `taperings_loss()` reads |smooth - exact - C|, and
  `truncation_bound()` is now the taperings' bound alone. `LATTICE_MARGIN` = 1e-6 is shared: the scan's `--min` and
  past-2.0 counts and the forced-cell precondition (`worst_scaled > plain_bound + LATTICE_MARGIN`) use it. The test's
  `past_curve()` became `taperings_loss()`, and every comment, `DEV_MANUAL.md` line, docstring and note that said "on
  the 1/24 lattice" or "what the taperings lost" now says it of the signed reading, wherever the clamp does not bind.
- `cell_truncation_positions` stands: the forced scan's only hit, `1k2r3/1pp5/p7/5q2/3Q4/2N3P1/5P2/3R2K1 w`, at
  2.906576. That is 0.0316 past 2.875 and 0.0098 under its own bound, and the precondition reads 2.90658 against
  2.875 + 1e-6.
- `truncation_positions` -> `5k2/pp3p2/5P2/7p/P6P/3Bb1P1/2PrN3/4K3 b - - 0 32`,
  `8/6bk/1p2p1p1/4p2p/5r2/P6P/2R2BP1/1r1B1K1R w - - 6 38`, `2r4k/4np1p/3q1n2/pB1p1P2/P2P4/5Q2/6PP/R4RK1 b - - 0 25` and
  `r1b2rk1/ppp1ppbp/2nq2p1/8/N1B1P3/5N2/PPP2PPP/R1BQ1RK1 w - - 0 10`, phases 5, 11, 17 and 23, each at 2.875.
  - The shipped scan reads 105 past 2.8, 28 distinct positions at the maximum, and 0 rows past their own bound.
  - The pick of earlier that day, phases 5, 11, 13 and 23, still reads 2.875 each, but the rule now finds a phase 17
    row and takes the scan's first row nearest each target.
- The bound case's and the pinned case's top clauses read 2.875 against 2.8.
- Plants 9 and 10 are red by their own checks again, every other plant is red by its own, and plant 0 is green. The
  plants patch is regenerated; only its hunk headers moved.
- Finding 3: the factor case's comment now names DEC-356's pair and reads 6.27485e-7 at 739 over 5717 rows, x1.59
  under 1e-6. `S124_factor_gradient_rows` still reproduces the pair.
- Gates: both builds 59 of 59 with no warning, bench 4707943 in both, the format check clean, and `plan_prose_check.py`
  exits 0 in all five modes.

## 2026-11-06: phase 2 starts, the coordinator

Worktree `../chesso-s124p2`, branch `s124p2`, at `0bb182d`. Phase 2a, one Opus 5.5 subagent: `feature_audit`'s local
rank at the shipped point and the identity rule restated; the draft's corpus figures re-read on S122's regenerated
corpus; `S124_screen.sh` with the pre-registration as its header; P0's conversion test. No fit, SPSA or match is
started by the agent; the coordinator runs P0 and the parts. Mop-up's SPSA instrument waits for P0's reading (if head
converts every start, P1 is not run).

## 2026-11-06: phase 2a

One Opus 5.5 subagent, coordinated; worktree `../chesso-s124p2`, branch `s124p2` at `a4ef975`, the diff staged and not
committed. Nothing fitted, played or timed by the agent: the corpus scans, the audit, the tablebase labelling and a smoke
of the conversion script are readings; every fit, P0 and the SPRT are the coordinator's, from `adocs/data/S124_screen.sh`.

### Amended 2026-11-06, before the number was read: the identity rule for a local rank

Written before `feature_audit` first computed a rank on S122's regenerated corpus at the shipped constants. The draft's
rule ("Pre-registered: the screen, a draft", its first item) read the design at S122's curve's seed: rank 1118 of 1190 on
S083's corpus, nullity 72, the 34 zero columns being the 32 dead pawn squares and the curve constant's two, and 38
dependencies by construction, S122's 37 (S134's 7, S121's 8, the curve's 22 duplicates of linear cells) and the corner
pair. At the shipped constants each king has its own x, so the audit now reads the design linearised there (below,
"feature_audit's local form"): the curve's 24 columns are each side's slope at its own x times its own counts, the
constant's columns are no longer zero and the other 22 no longer duplicate a linear cell. So the rule is restated for
that local rank, and the expected figures follow from the design's construction alone:

- the trace's identity on every row (0 differ), and the report in its local form;
- the design check and the new linearisation check each under 1e-6;
- the zero columns the 32 dead pawn squares and nothing else;
- the dependencies by construction S134's 7, S121's 8 and the mop-up corner pair, 16, each holding on every row; no
  curve duplicate, and no exact dependency beyond the 16;
- so the rank over the rationals exactly 1190 - 32 - 16 = **1142, nullity 48**;
- under `--free endgame_scale`, the span 48 of at most 48: the 32 zero columns, the 7 held gauges, the 8 mobility
  levels and the corner pair held by the freeze;
- every cell's support line at rows > 0.

Anything else stops the screen for the coordinator before any fit, S122's form. The local rank is the design's at one
point: it says which directions the loss sees there, not that a direction stays seen once a fit moves the curve's x,
which no S124 part frees.

### feature_audit's local form, task 1

- **What it reads.** `tools/audit_design.hpp` (new) holds the design `tools/feature_audit` ranks, moved verbatim from
  `tools/feature_audit.cpp` so a test can read it, and adds the local form: where the curve's sides do not share one x,
  each half's column for input i is `sign (max(x_W, 0) n_W,i - max(x_B, 0) n_B,i)` times the half's taper weight times
  24, at the weights read, the divisor / 2 = 1024 dropped as a factor common to the group's columns; integers wherever
  every side's x is one (`local_form_exact()`), otherwise `not computed`. At a shared x the old form is kept unchanged,
  so the report at the seed reads as before. The rank is the design's local rank at that point.
- **Two checks in the report.** The design check sums the curve's columns at 1/divisor (T is of degree 2 in its
  weights, so its columns dotted with the weights are 24 divisor times its score), and the clamp's skip reads the curve
  inside stage two. A **linearisation check** reads every column of the sampled rows, over its scale (24, 24 F for a cell
  at F, 24 divisor / 2 for the curve), against the smooth score's central difference at h 1e-3, skipping rows within a
  centipawn of the clamp or with a side's x at 0; `tuner_trace::smooth_t` gains `stage_two`, the sum before the clamp,
  for that skip. `X'X` is now summed modulo 2^64 beside each diagonal in 128 bits and refused past 2^63: the curve's
  columns reach the 10^6s (largest diagonal 2.6e14 on the corpus, under 9.2e18), and by Cauchy-Schwarz a diagonal under
  2^63 bounds every partial sum, so each entry is read back exactly.
- **The test**, `tests/test_audit_design.cpp` (new, fast, the trace build; 0.3 s), over the curated list both ways
  round, the JSON suites and S124's population. (1) At the shipped weights the local form is the smooth score's
  derivative: 8048 rows, 432310 columns compared, 62802 of them the curve's, 6471 rows with both kings' x positive and
  apart; worst 5.68e-10 against 1e-6; the scales written in the test from the definitions, the difference its own code.
  (2) At the curve's seed (45 in both halves, every other input 0), the old form defined: the local form is the shared
  form times 45 column for column, 0 mismatches over 56270 curve entries; the rank over the curve's 24 columns and the
  22 linear cells they duplicate reads 22 in both forms (the test's own Gauss-Jordan); at the shipped weights 46.
- **Red on each plant** (`adocs/data/S124_phase2_plants.sh`, `.patch`, the tally in `adocs/data/S124_phase2_plants.txt`;
  `feature_audit` over the corpus's first 300000 rows):

  | plant | derivative case | seed case | design check | linearisation check | rank |
  |---|---|---|---|---|---|
  | 0 none | green, 5.68e-10 | green | 1.14e-12 | 1.56e-10 | 1142 |
  | 1 x for max(x, 0) | red, 1.05 | green | 132 | 1.27 | 1140 |
  | 2 Black's side at White's x | red, 2.54 | green | 207 | 4.05 | 1140 |
  | 3 the endgame half at the middlegame's x | red, 5.57 | green | 65.6 | 5.06 | 1142 |
  | 4 the slope squared | red, 2478 | red, mismatches | 510 | 2.82e3 | 1142 |

  Plants 1 to 3 agree with the old form at the seed, where both kings share x and it is positive, so only the shipped
  point sees them; plant 4 breaks the seed's proportionality too. The rank alone moved under two plants of four: it is
  no guard of the linearisation, the checks are.
- **Read on S122's regenerated corpus** (`adocs/data/S124_phase2_preconditions.txt`, section 1): rank exactly **1142 of
  1190, nullity 48**, the rule's figures written before the run: the 32 dead pawn columns, S134's 7, S121's 8 and the
  corner pair, 16 by construction each holding, 0 further dependencies, no curve duplicate; the span 48 of 48 under
  `--free endgame_scale`; design check 9.09e-13, linearisation check 1.56e-10 over 203125 rows; every cell's column
  non-zero. `S124_screen.py identity-check` passes it and refuses HEAD's report, whose identity read `not computed`.

### The draft's corpus figures re-read, task 2

Every figure, with its command, in `adocs/data/S124_phase2_preconditions.txt`. Amended 2026-11-06, before any fit, where
the draft's void-corpus figures moved:

- **Supports.** The thirteen cells on 1131 to 37360 games. **OCB_PURE P<=2 on 2438 games** (draft: 1047 void, 905
  S083): over the 1000, so F1 does not stop. **OCB_PURE P>=5 on 1131** (840 void, 798 S083): over the 1000, so the
  draft's F3 tie, `endgame_scale 11 10` "unless that cell's support reads 1000 games or more", does not bind and F3 fits
  the cell free. The rule is the draft's; its outcome moved because S331's datagen plays on past a two-fold and the games
  run longer.
- **Mop-up's domain**: 30812 rows in 4970 games (draft: 9742 in 2087), the engine's count and the piece counts' agreeing;
  KRvK 22522 rows in 4331 games, KBNvK 6546 in 389, KBBvK 1306 in 129, KRBvK 251, KRNvK 178, KQvK 4 (the |score| < 1000
  filter). The corner pair on 6205 rows, 389 games (draft: 1954 rows, 212 games).
- **The guard's rows**: 292936 of five men or fewer in 29986 games; at each row's own clock WON 46052, LOST 35108, DRAWN
  197583, SPOILED 13675 (White relative), EDGE 518 left out, none unprobed. By cell: F1's 38157, 72749 and 10959
  (OCB_PURE P<=2's P=0 and P=1 rows); F2's 28593, 81983, 9960, 3511, 3996, 0, 0; F3's none.
- **K at the identity: 1.2171** at the landed tree (the record's 1.2302 is `7fe9f89`'s, S122 at its seed), read by
  `adocs/data/S124_k_refit.cpp`, the tuner's `fit_k()` at the shipped constants; no tuner was run. The screen's `k` stage
  reads the tuner's own beside it.
- **P0's starts** from the raw root pool's training games: KRvK 7515 games with a root row (4121 won and distinct at a
  zeroed clock), KQvK 3869 (3525), KRBvK 1375 (1354), KRNvK 1175 (1156), KBNvK 407 (384), KBBvK 130 (126): 200 of each
  and KBBvK's 126, 1126 starts.

### The screen, `adocs/data/S124_screen.sh`, task 3

Stages `locate`, `tools`, `identity`, `k`, `guard`, `p0`, `part F1` to `F3`, `read`, `all`, `smoke`, each ending
`S124-<STAGE>-DONE` or `-FAILED`; helpers `adocs/data/S124_screen.py`. Its header is the pre-registration, every figure
from task 2. The tuner's groups `endgame_scale.f1`, `.f2`, `.f3` exist (phase 1); nothing was added to
`tools/tuner_groups.hpp`. **Amended 2026-11-06, before any fit, where the script departs from or makes exact the draft:**

1. **K, DEC-352** (the draft predates it): on each sample, round r fits the part at K_(r-1) from the landed constants and
   K_r is K refit at that fit's emitted vector by `S124_k_refit`; decided when K_r is within 1e-4 of K_(r-1), at most six
   rounds, else K-STOP. K_0 is the tuner's zero-epoch K at the identity, and `k` refuses unless `S124_k_refit` agrees
   with it within 1e-4 on every sample. A fit is read at its own decided K: the landed tree at K_0 (round 1's `start:`),
   an earlier survivor at its decided K, the part at its; round 1's d, every side at K_0, is printed beside and decides
   nothing.
2. **The tuner is build/'s**, not build-tune's: no part moves the margin, so the shipped 275 is every fit's.
3. **The guard's statistic**: "1.645 whole-game errors" is the paired rise of the tablebase-labelled loss, part less
   predecessor, over 1.645 of its error by game (the jackknife over games, `S122_screen.py side-read`'s), each vector at
   its decided K through `tools/margin_census`'s integer score. The guard reads every part; F3's own cells move no guard
   row, so F3 alone is screened on the loss, and earlier survivors refit with it are guarded.
4. **No escalation**: DEC-345 (2)'s re-run at 80000 was the curve's; these parts are linear in f with the old constants
   frozen, and a fit stopping on its 20000 cap is printed, a reading.
5. **P0's start rule made exact**: a game's first root row in the class; the WDL filter at the zeroed clock before the
   200 are taken; one start a position; one seeded shuffle a class, seed 20261106; `--skip 200` the next 200 for P1. P0
   decides no part and stops nothing: `P0-CONVERTS-EVERY-START` records mop-up zero, `P0-FAILS-TO-CONVERT` sends P1's
   instrument to the coordinator.
6. **Paired against the predecessor**: the draft's "each paired against the landed tree" is read in DEC-326 (3)'s and
   S122's form, the brief's: a part against the landed tree plus the survivors so far, which for F1, and for any part
   with no survivor before it, is the landed tree itself; a later part's fit frees the earlier survivors' cells again
   and is read against their decided fit set.
7. **Machine hours re-estimated** in the header: a part 2 to 3 h at three to four rounds, 6 to 9 h for the three,
   about 30 h a part at the worst; DEC-155's night.

8. **Amended 2026-11-06, DEC-357, before any fit and before the change is made: the guard reads both vectors at one
   K.** Amendment 3 read each vector at its own decided K, and K alone moves the guard's loss: the landed vector read
   at K 1.2171 and at 1.2671 differs by +0.0023, and at 1.3578 by +0.0063, each a refusal, so a part changing nothing
   but K is refused (the smoke refused F3's refit) and a falling K would blind the guard. Now the part's decided full
   fit and its predecessor (the landed tree, or the last survivor's decided full fit) are both read at the part's
   decided full-corpus K, DEC-352's reading K, and the guard refuses on that reading alone: a rise of the
   tablebase-labelled loss past 1.645 of its error by game. The same comparison with both at K_0, the identity's K, is
   printed beside it and decides nothing.
9. **Amended 2026-11-06, the fast check's finding 5: a cell under 1000 games other than the two the draft names stops.**
   The draft named OCB_PURE P<=2's stop and P>=5's tie; `S124_screen.py ties` also stops on any other cell under 1000
   games, a case no tie is pre-registered for, DEC-326 (5)'s rule having no supported neighbour named for it. Recorded
   here as the rule; it does not bind on this corpus, whose smallest cell stands on 1131 games.

**The smoke** (`S124_screen.sh smoke`, 40000 rows and a quarter of each view, 300 epochs, two starts a class): every stage
ran end to end on the working tree, the stamp `a4ef975-dirty`. The tuner's and `S124_k_refit`'s K agreed to the fourth
place on all six samples; K iterated two to four rounds a sample and converged everywhere; the identity and support rules
printed and did not bind on a slice (P<=2 on 14 games); F3's tie applied (P>=5 on 6 games, cells 10 and 11 fitted
equal); the guard passed F1 and F2 and refused F3's refit. A smoke, not a reading.

### P0's conversion script, task 4

`adocs/data/S124_p0.py` (`S124_conversion.py` is the research pass's game classification and is not touched):
`select`, `play`, `read`, `mcnemar`, the rules in its docstring and the screen's header. One record a start, `id class
game ply fen wdl dtz converted plies end moves`, `id` `class:game` the key P1's McNemar pairs by. Smoke only, twice: 12
starts, two a class, in 3 s at 100000 nodes; the records replay legally to their final positions. **The coordinator's
run**, about 5 to 10 min: `adocs/data/S124_screen.sh p0` (after `locate` and `tools`), or alone:

    ~/.venv/chess/bin/python adocs/data/S124_p0.py select --pool .tuning/s122/corpus/pool.tsv.root \
        --heldout .tuning/s122/prepared/pool.tsv.qs.validation --out OUT/starts.tsv
    ~/.venv/chess/bin/python adocs/data/S124_p0.py play --starts OUT/starts.tsv --engine build/src/chesso \
        --out OUT/records.tsv --nodes 100000 --workers 12
    ~/.venv/chess/bin/python adocs/data/S124_p0.py read OUT/records.tsv

### Mop-up's SPSA instrument: options, not built, task 5

Decided only if P0 reads a start head fails to convert. Whichever is built, the driver is new: `tools/spsa_driver.py`
plays paired mini-matches, and P1's objective is plies to mate over a batch of starts, the bound for a failure; its
`schedule()` and Rademacher step can be imported. The corner is one parameter (2 mg + 22 eg), five free.

- **(a) Tune-build UCI options for the six weights.** `mopup_mg` and `mopup_eg` read from settable variables under
  `CHESSO_TUNE`, `src/search_params.hpp`'s X-list form, `mopup_live` a runtime flag there; six options on the tune
  build's UCI surface, so `test_uci_surface`'s golden, `MANUAL.md` and `specs.md` move first (SURFACE), and DEC-118's
  class of divergence grows. Cost: one agent task under `src/` and the tune build's checks; at run time a point is a
  `setoption`, the draft's 3 h. Risk: the release build must read INV-6 identical (constexpr there; bench, objdiff),
  and the tune build pays the domain test at every stage-one call. Buys an instrument a later evaluation SPSA (S127)
  could reuse.
- **(b) A driver that rebuilds per point.** A scratch worktree, the six constants written into `src/evaluation.cpp`,
  `chesso` rebuilt (evaluation.cpp and the link under ccache, about 30 to 60 s at -j12, unmeasured), the starts played.
  No surface change and nothing under `src/` on the landed tree: S124's data files only. Cost: two builds an iteration,
  about 200 to 400 over 100 to 200 iterations, 2 to 6 h of builds beside the games, inferred; the stage-one bound's
  `static_assert` refuses a point past mop-up's reach at build time.
- **(c) The tuner on conversion-labelled rows**, P0's self-play positions labelled by whether their start converted:
  minutes and no instrument. But DEC-350 (4) rules SPSA; at |E1| w about 600 the sigmoid sits past 0.95 at K 1.2171, so
  the loss barely sees where the lone king stands whatever the label (the research pass's inference); a label "converted
  within the bound" is the engine's own outcome and not a game's result under the laws, a decision amending DEC-350 (4);
  and a Syzygy DTZ target is a tool's number, the owner's (the research pass's question 5 (b)).
- **Recommended (b)**: it changes nothing the engine ships or exposes, it is S124's alone, and its cost is machine hours
  a night holds. (a) is worth its surface change once a second evaluation term wants SPSA, as its own step.

### Gates, phase 2a

- Both builds without a warning; `ctest -L fast -j10` **60 of 60** in `build/` and in `build-tune/`, `test_audit_design`
  the new entry (0.5 s), with `CLANG_FORMAT_MAJOR=22`. `chesso bench` 4707943 in both: no `src/` change.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` clean; `tools/plan_prose_check.py` exits 0 in all five modes.
- Tier 2 (DEC-141) does not bind: no `make_move`, `unmake_move`, generator or search change, no pruning rule.
- `feature_audit` re-read on the corpus with the final, formatted binary: its output identical line for line to the
  reading above.
- `DEV_MANUAL.md`: the identity report's local form, the linearisation check, the Gram's exact summation, and the
  phase 2a reading; the stale "a trace carrying per-side entries is not ranked at all" (stale since S122's shared form)
  replaced. `MANUAL.md` read and unchanged: no option, command or engine output moves. `specs.md`: nothing proposed, the
  engine's behaviour does not move. `adocs/data/README.md` gains the nine new data files.

### Findings beyond phase 2a, not fixed

- **The record's K 1.2302 is not the landed tree's**: `.tuning/s122/prepared/k.txt` was read at `7fe9f89`, before S122's
  fit; at `a4ef975` K at the identity is 1.2171. The screen reads its own.
- **P0's two smokes failed to convert 3 of 12 and 5 of 12 starts**, every failure a fifty-move end at the bound, KBNvK's
  four starts among them: two draws of two starts a class, not a reading, but they make task 5's decision likely.
- KQvK stands on 4 rows of the training corpus, the |score| < 1000 filter dropping it: mop-up's corpus support is
  KRvK's and KBNvK's. P0 reads the raw root pool, where KQvK has 3525 starts.

### 2026-11-06: the fast check's findings on phase 2a

The coordinator's fast check found one blocker, ruled as DEC-357, and five low findings. All six are fixed, staged.

1. **The guard read each vector at its own K** (blocker, DEC-357; amendment 8 above, written before the change). Now
   `part` reads the part's decided full fit and its predecessor at the part's decided K, refuses on that reading alone,
   and prints the same comparison at K_0 for both beside it, `beside, deciding nothing: ...`, which carries no verdict.
   A new stage, `guard-probe`, which the smoke runs after F3, shows it on F3's predecessor at F3's K:
   - an inert probe, the predecessor with F3's own factors (cells no row of five men or fewer can stand in), moves the
     guard's loss by exactly 0 and passes;
   - a planted probe, every factor at 4 F = 512, moves it +0.0956 +/- 0.0101 by game and is refused.

   The smoke's F3 refit is still refused at the common K, +0.0000167 +/- 0.0000078. That is a reading and not K: the
   refit frees F1's and F2's cells again, and those cells move guard rows. F1 and F2 pass at their own K.
2. **A failed `S124_cell_check` was taken as done.** The census now writes a temporary file, keeps it only when it ends
   `S124-CELLS-OK`, and removes it on a failure.
3. **A failed `chesso bench` was stamped anyway.** `tools` now refuses unless the bench exits 0 and prints a node count,
   and writes no stamp otherwise.
4. **Only `src/` and `tools/` were held at HEAD.** The clean-at-HEAD check now covers every file the screen builds or
   runs:
   - `src/`, `tools/` and `tests/`, which holds the header `S124_cell_check.cpp` includes;
   - `S124_screen.sh`, `S124_screen.py`, `S135_screen.py` (imported), `S124_guard.py`, `S124_p0.py`,
     `S124_k_refit.cpp` and `S124_cell_check.cpp`.

   The Python helpers and `tools/corpus_split.py` run from copies frozen with their sha256 in `OUT/bin`, each checked
   at every stage.
5. **Any other cell under 1000 games stopping** was in no amendment. It is recorded as amendment 9 above; it does not
   bind here.
6. **The design check assumed one clamped group.** `audit_design::local_form_refusal()` now names a model with more than
   one group, or a group outside the clamp, and `feature_audit` prints `not computed` for it rather than misreading.
   `test_audit_design`'s third case checks the engine's model is accepted and both other forms are refused; it reads
   red on plant 5, the refusal disabled, and nowhere else.

**Re-run on the final tree:**
- `ctest -L fast` reads 60 of 60 in both builds, and bench reads 4707943 in both.
- The format check is clean, and all five prose modes exit 0.
- The plants script reads plant 0 green, plants 1 to 4 red as before, and plant 5 red in the refusal case alone
  (`adocs/data/S124_phase2_plants.txt`).
- The smoke runs end to end, the guard probes included.
- `feature_audit` on the corpus reads identically: rank 1142 of 1190, nullity 48.

## 2026-11-06: P1's instrument, option (b)

One Opus 5.5 subagent, coordinated; worktree `../chesso-s124spsa`, branch `s124p1spsa` at `8cc9a38`, the diff staged
and not committed. The coordinator chose option (b) (above, "Mop-up's SPSA instrument: options, not built, task 5"): a
driver that rebuilds per point, `adocs/data/S124_p1.py`. Nothing under `src/` changes and no UCI surface moves. P1 runs
because P0 read head failing 272 of 1126 starts (the screen's reading, 2026-11-06: 854 converted, 0.758; KBNvK 21 of
200, 171 of them by the fifty-move rule). Built and smoked only: no iteration was played on the registered batch and no
fresh-starts test was run on the full set. Both are the coordinator's, after the screen ends.

### The instrument

- **Scratch copies.** `setup` exports HEAD's `CMakeLists.txt`, `cmake/`, `src/`, `tests/` and `tools/` four times
  under `OUT/scratch/` with `git archive HEAD`: head, plus, minus and final. It configures each copy Release with
  ccache. `GIT_CEILING_DIRECTORIES` keeps the build stamp from reading the enclosing checkout, so `id name` reads
  `unknown`. A point is written as the two `mopup_mg` and `mopup_eg` lines of the copy's `src/evaluation.cpp`, each
  matched exactly once, and read back. Only `chesso` is rebuilt: `evaluation.cpp` and the link.
- **Setup's three checks**, before any game:
  - the head copy at the landed constants benches what `build/` benches;
  - ROOM is read by the compiler: `stage_one_bound` through a diagnostic, then held to the `static_assert` itself, so
    a half reaching exactly ROOM compiles and one reaching ROOM + 1 fires it;
  - **the build evaluates what was written**: `adocs/data/S124_engine_eval.cpp`, built against each copy's own
    `libchesso_engine.a`, reads `evaluate()` at a check vector and at head on every training start and four rows
    outside the domain. The difference is held to the file's own count of the three features times the vector,
    tapered: within 2 in the domain (two truncations) and exactly 0 outside. The counts are computed in Python from
    src/evaluation.hpp's definitions, not read from the engine.
- **The run**, `tools/spsa_driver.py`'s `Config`, `schedule()` and `Param.uci_value` imported from a copy frozen at
  setup, its form of the step.
  - Per iteration: the Rademacher draw; both points clamped, rounded and held under ROOM; written and built in
    parallel; the batch played by both builds through `S124_p0.play_one`, 12 games at a time.
  - y is the batch's mean of score(theta-) - score(theta+), and theta steps by r(k) c_i(k) y delta_i, held to its box.
  - Each iteration's records go to `OUT/run/iters/`, with both vectors and both binaries' sha256, and a row goes to
    `trajectory.tsv`. The checkpoint holds theta, k and the generator; `--resume` replays nothing and redraws the
    same deltas and batches. A run that failed inside iteration 0 has a `run.json` and no checkpoint, and `--resume`
    restarts it at 0 (the fast check's finding 2, below).
  - A digest of the config, the tree, the starts, ROOM, the frozen helpers and the script refuses a resume under any
    of them changed. Setup refuses a script that differs from HEAD's, except in the smoke.
- **The test.** `S124_p0.py select` with the training starts' own pool, held-out view and seed, `--skip 200`. Its pool
  and view sha256 are held to P0's, and the fresh starts are held disjoint from P0's 1126 by id and by position. Head
  and the candidate both play the fresh starts and KBBvK's 126 training starts. The verdict is DEC-358's exact
  tests (below), with `S124_p0.py mcnemar`'s reading printed beside, deciding nothing. It prints `S124-P1-KEPT` or
  `-NOT-KEPT`, then `S124-P1-TEST-DONE`. `rule-check` holds the rule on synthetic paired records.
- **`all`** runs setup, run (resumed if a `run.json` exists) and test, and ends `S124-P1-DONE` or `S124-P1-FAILED`.
  Every exception lands on the marker.

### Pre-registered, the script's header; amended from the draft before any iteration

The header carries every number. Where it departs from or makes exact the draft's P1 line ("Pre-registered: the
screen, a draft"):

1. **The corner's middlegame half is frozen** at its landed 0, the draft's "tied or frozen" read as the freeze, the
   form `tools/feature_audit` holds the same direction by. The endgame half carries 2 mg + 22 eg. Five free:
   centre and kings in both halves, the corner's endgame half.
2. **A failure scores 200 plies, twice the bound**, not the draft's "the bound for a failure". At the bound a failure
   scores what a mate on the last ply scores. Turning one failure into a mate at ply 80 then gains 20 plies, about
   what one start's mate length moves between theta+ and theta-: 20.2 on average over the smoke's 25 pairs both
   builds converted, at the smoke's larger perturbations. At 200 it gains 120, so the objective ranks conversion
   first, the order the kept rule reads.
3. **The batch**: 40 of each of the five classes and 21 of KBBvK, 221 of P0's 1126 an iteration. Each class has its
   own seeded shuffle a cycle, so every start is played once every 5 iterations (KBBvK every 6). K = 150, fixed: no
   early stop, the candidate theta after the 150th update. 442 games an iteration, 66300 in all.
4. **c and a scaled to the taper.** A middlegame weight reaches the domain's score at phase / 24, the phases being 2
   to 4 (mean 2.71), so c_end is 32 for the middlegame halves and 4 for the endgame halves. In Spall's form a_i =
   2 r_end c_end,i^2 (K + A)^alpha, so each axis moves the score alike. Spall's c is 6.635 and 53.08; a is 8.649 and
   553.6; A is 15; alpha 0.602, gamma 0.101.
5. **r_end = 0.0125, read on a synthetic objective.** The first calibration, 0.025, was sized from a guessed first
   gradient. It diverged on the smoke's synthetic quadratic at ten times the guessed steepness. The rule became "no
   divergence from a quarter to ten times it, at a per-start noise of 50 or 150 plies", and 0.0125 meets it. Of the
   seed's excess, 0.0125 leaves 28 to 30 % at a quarter, 2 to 3 % at one and 0 to 1 % at four and ten; 0.025 left 11
   to 26 %, 0 to 4 % and 0 to 2 %, then 609 to 1054 % at ten. The steepness is a guess, so the grid calibrates
   nothing beyond it.
6. **The integers.** A point is clamped to its box (middlegame [-256, 256], endgame [-64, 192]) and rounded half to
   even, as `spsa_driver` rounds. The frozen slot keeps its landed value. A half reaching past ROOM = 32767 -
   `stage_one_bound` (27929 at `8cc9a38`, ROOM 4838) is scaled toward zero, w to trunc(w ROOM / reach), and flagged.
   The boxes reach 4608 and 4800, so at this tree the clamp is a guard and binds nowhere.
7. **The fresh starts**: KRvK, KQvK, KRBvK, KRNvK 200 each and **KBNvK 184**, not 200 (384 won and distinct, 200 taken
   by P0), and **KBBvK none**. McNemar's z and the per-class rule read the five classes present. KBBvK is read on its
   126 training starts, head and candidate both played there. In sample, it can only refuse: the candidate is not
   kept if it converts fewer of them, and converting more helps nothing. *The rule is superseded by DEC-358, below.*
8. **The start**: the landed zeros, read from the tree; setup refuses any other start. No other engine's or page's
   number (DEC-105).

### Amended 2026-11-06, DEC-358, before iteration 0: the kept rule's exact tests

The fast check found that the kept rule compared raw per-class counts. Self-play is deterministic, so a class the
candidate does not touch still moves by chaos and falls below head's count about 40 % of the time. A candidate that
fixes KBNvK would mostly read NOT-KEPT on that alone. The uncorrected asymptotic z also keeps b = 0, c = 3 (z 1.73,
exact p 0.125). DEC-358 rules the replacement, which is in the header and in `test`:

- **Kept iff** the exact one-sided McNemar test over every fresh start reads p < 0.05: the binomial at 1/2 on the
  discordant pairs, the candidate converting more.
- **And no class** reads the exact one-sided test the other way, the candidate converting fewer, at p < 0.05.
- **KBBvK**, in sample on its 126 training starts, is held to the same per-class refusal. It can refuse and never
  helps keep.
- `S124_p0.py mcnemar` is unchanged, and its reading is printed beside, deciding nothing. The screen freezes that
  file, and P1 computes its own verdict.

`rule-check` holds the rule on synthetic paired records at the fresh set's sizes and head's P0 rates. The smoke runs
it, and `--plant raw` puts the raw count comparison back:

| case | DEC-358's rule | the rule replaced |
|---|---|---|
| b = 0, c = 3 in KBNvK | not kept, p 0.125 | kept |
| 120 KBNvK starts gained, one KRBvK start lost by chaos | kept | not kept: fewer in KRBvK |
| as above, and eight KRvK starts lost | not kept: KRvK p 0.0039 | not kept |
| as the second, and six KBBvK training starts lost | not kept: KBBvK p 0.0156 | not kept, on KRBvK |

### Measured, and the wall estimate

- **A point's build**, one `evaluation.cpp` and the link under ccache: 1.9 s alone at load 5, a cache miss. An
  iteration's two, built in parallel under S124's screen, took 2.3 to 3.2 s on a miss and 0.4 s on a hit. A hit is a
  point compiled before, as when a resumed run repeats one.
- **A ply**, in core-seconds:
  - 0.0420 on the idle machine at 12 games: P0, 1126 starts, 59111 plies, 207 s;
  - 0.0361 and 0.0471 in the two full smokes, at 4 games under the screen.
- **An iteration** at head's mix is 442 games and 23104 plies. That is 81 s of games at P0's figure and 91 s at the
  smokes' higher one, so 84 to 94 s with the builds, and 3.5 to 3.9 h for K = 150. The draft guessed 3 h.
- **The test**, about 9 min: the fresh select read the 1.4 GB pool in 71 s, then 2 x 984 fresh starts and 2 x 126
  KBBvK.
- **In all, 3.7 to 4.1 h**, at DEC-155's four-hour line, so where it runs is the coordinator's call. Conversions
  shorten the games as theta improves, so head's mix is the high figure. At worst every game runs to the bound, 4.2
  core-seconds a start, about 6.7 h. A watcher's ceiling: 8 h.
- **Play is deterministic** at `go nodes`: head played six starts twice, and all six move lists were identical. Every
  difference between theta+ and theta- on a start is the parameters', none of it the harness's.

### The smoke and its reds

`S124_p1.py smoke` runs the whole instrument on a slice: 4 training starts a class, 2 a class an iteration, 3
iterations, 2 fresh starts a class, 4 games at a time, under the screen. Output and every plant's red line are in
`adocs/data/S124_p1_smoke.txt`. It ran green in 169 s on the delivered script, checking these in order:

- **setup**: the head copy benches 4707943, as `build/` does. `evaluate()` at the check vector `11,5,0|7,13,19` moved
  within 1.167 of the Python counts' expectation on all 24 domain rows (bound 2), and by exactly 0 on the 4 rows
  outside. ROOM 4838 (`stage_one_bound` 27929): a half reaching 4838 compiles and one reaching 4839 fires the assert.
- **the clamp**: `400,400,0|800,800,800` (reaches 7200 and 20000) came back `268,268,0|193,193,193` (4824 and 4825)
  and compiles.
- **the identity**: the plus copy benches 4637995 at the check vector and 4707943 back at the zeros, head's.
- **the synthetic objective**: the registered schedule and batch take the seed's excess from 29.00 plies to 0.49; the
  same run with the update negated ends at 656.61.
- **the rule check**: DEC-358's four synthetic cases read as written (the amendment above), the replaced rule beside.
- **the run**: a planted failure inside iteration 0, before any checkpoint. `--resume` restarted it at 0 with the same
  draw, and its iteration 0 read what an uninterrupted run reads (y -47.67). It was then stopped after two iterations
  and resumed for the third, the trajectory holding k = 0, 1, 2.
- **determinism**: six of six move lists identical.
- **the test**: 10 fresh starts and KBBvK none, disjoint by id and by position; head, the candidate, the exact tests
  and the KBBvK guard all ran. The smoke's candidate after three noisy iterations read `S124-P1-NOT-KEPT` (p 1): a
  smoke, not a reading.

Each plant (`--plant`, a test hook in the script, its own OUT) turns the smoke red at its own guard:

| plant | break | caught by | red line |
|---|---|---|---|
| `swap` | each array's values written into the other array, read back through the same map | the eval check | worst 159.167 against 2, all 24 domain rows |
| `order` | each array's features written in reverse, read back the same way | the eval check | worst 61.333, 20 rows |
| `room` | ROOM read one too large, 4839 | the room check | 4839 does not compile |
| `noclamp` | the stage-one clamp skipped | the clamp check | reaches 7200 and 20000, does not compile |
| `sign` | the update negated | the synthetic check | 656.61 after the run, 0.49 negated |
| `raw` | the per-class raw count comparison DEC-358 replaced | the rule check | misreads `kbn-gain-chaos` |

The textual read-back cannot see `swap` or `order`, because the plant maps the writer and the reader alike. Only the
engine's own evaluation can, which is the point of the check.

### The coordinator's run

After the screen ends, with this diff committed, from the checkout whose HEAD carries it and with `build/` built at
that HEAD. Setup refuses a script that differs from HEAD's, and a `build/` that benches other than the head copy.
P0's starts are copied into `OUT/bin` at setup, so the screen's directory need not outlive the run.

    mkdir -p .tuning/s124/p1
    nohup setsid -w ~/.venv/chess/bin/python adocs/data/S124_p1.py all --out .tuning/s124/p1 \
        --train-starts /home/max/ws/chesso-s124p2/.tuning/s124/screen/p0/starts.tsv \
        > .tuning/s124/p1.log 2>&1 & echo $!

The watcher has four exits: `S124-P1-DONE`, `S124-P1-FAILED`, the PID gone, and a ceiling of 32400 s (9 h, twice
the high estimate; the fast check's finding 3). After a
failure, `S124_p1.py run --out .tuning/s124/p1 --resume`, then `test`, continues the same run. The verdict is the
line above `S124-P1-TEST-DONE`, `S124-P1-KEPT` or `-NOT-KEPT`. The candidate's two C++ lines are in
`.tuning/s124/p1/final.txt`.

### Gates

- Nothing under `src/` or `tools/` changed. `build/` (Release, ccache) builds without a warning, and `chesso bench`
  reads 4707943.
- `ctest -L fast -j6` in `build/`: 59 of 60 without the override. `test_clang_format_script` failed there because
  `CLANG_FORMAT_MAJOR` was not exported (`.moltke.local.md`, DEC-146), and passes with it set (1.41 s).
  `test_tuner_gradient_plants_threats` passed inside its timeout under the screen's load (44.95 s).
- The format check is clean with `CLANG_FORMAT_MAJOR=22`, and `tools/plan_prose_check.py` exits 0 in all five modes.
- After the fast check's three findings: the smoke and its six plants were re-run as above, `py_compile` passes,
  `ctest -R plan_ -j6` reads 5 of 5, and the format check and the five prose modes are clean again. The diff holds no
  shell file, so there is nothing for `bash -n`.
- Tier 2 (DEC-141) does not bind: no `make_move`, `unmake_move`, generator or search change, and no pruning rule. The
  build-tune gate is not run here: no source either build compiles moved.
- Docs:
  - `DEV_MANUAL.md` gains "An evaluation constant: a rebuild per point" at the end of its SPSA section;
  - `MANUAL.md` was read and is unchanged, since no option, command or engine output moves;
  - `specs.md` is unchanged, since the engine's behaviour does not move;
  - `adocs/data/README.md` gains `S124_p1.py` and `S124_p1_smoke.txt`;
  - `touches:` is not widened: the new files are S124's data files, as phase 2a's are, and `tools/spsa_driver.py` is
    imported, not changed.

### Findings beyond the task, not fixed

- **The fresh KBNvK set is 184 starts**, not the 200 a class the draft's P1 line implies: P0 took 200 of 384. It is
  recorded as amendment 7 above. KBNvK holds most of head's failures (171 of 272), so the test reads it on 184 pairs.
- **Self-play at `go nodes` is deterministic**, six of six in the smoke. So head's own P0 records are reproducible
  from its binary, and a pair's difference is the parameters' alone. It also means a start's outcome can move only
  when theta moves: the per-start noise the synthetic grid assumed is chaos in the parameters, not in the harness.
- **ccache is at its 5 GB cap here** (99.97 %, 1550 cleanups when read). A point is a miss either way, about 2 s, so
  the run does not depend on it; only a resumed iteration's repeat would hit.
- The main checkout moved to `d0cb418` while this was built, through S334's landing and DEC-358's ruling. The staged
  diff applies onto it cleanly (`git apply --check --3way`).

## 2026-11-06: the screen, stopped for a shutdown, the coordinator

The screen ran from `../chesso-s124p2` at `c496403` from 14:44 and was stopped at 18:09 for a shutdown, during F3's
full round 2.
- **P0:** 854 of 1126 converted.
- **F1 kept:** d +0.00368, m 0.0000282, guard -0.0339 +/- 0.0005; cells 46 / 0 / 20.
- **F2 kept:** d +0.001851, m 0.0000209, at K 1.3248; guard -0.00686 +/- 0.00027; fit
  `endgame_scale 45 0 9 29 81 114 86 128 123 19 128 128 128`.
- Readings: `../chesso-s124p2/.tuning/s124/screen/read/`.
- **The resume:** `all` reuses every decided fit and every round with `best:`. F3's round 2 re-runs. The command is in
  `adocs/status.md`'s handover.
