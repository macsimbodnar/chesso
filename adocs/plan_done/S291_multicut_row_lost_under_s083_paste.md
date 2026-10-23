id:         S291
goal:       find by tool why S083's re-landed constants lose the multicut mined row's mate at depth 10 (found at 9, 11 and 12), and fix the rule at fault as its own measured change if one is
accepts:    the node and the rule that turn `tests/test_search.cpp`'s "mate the multicut hides, depth 10" (`6k1/5pp1/6np/1PP1p3/1Q2P1P1/3r1q1P/1B1R4/6K1 b - - 0 42`, mate in 5) into a non-mate score at depth 10 on S083's re-landed tree (`../chesso-s083r`, bench 3945487) are named from the engine's own output (a trace, a counter, one tune option at a time), never from reading the board (CHESS); the same mechanism's reach at the shipped weights (`cd50178`'s tree) is checked on the mate sets S289 used; if a rule is at fault and reachable at the shipped weights, its fix ships as its own change with a guard test observed red first, a mutant it kills, Debug self-play, gate_extra, every golden it moves re-derived by its script, and an SPRT against its parent; if no rule is at fault, the attribution is recorded for the owner's H1 ruling under DEC-308; the fast suite green in both builds
touches:    adocs/data/S291_row.py, S291_fens.py, S291_trace.py, S291_trace_filter.py, S291_trace_line.sh, S291_uci_row.py (added: the driver, the line FENs, the scratch trace patch, never shipped, and its helpers); adocs/data/S291_defender186.py, S291_census.py (added: the 186 defender nodes' proved distances, the ordinary-play census driver); adocs/data/S291_options.txt, S291_trace.txt, S291_reach.txt, S291_e21_direct.log, S291_census.txt (added: the readings); adocs/data/README.md (S291's rows); this file. No rule at fault, so nothing in src/ or tests/
excludes:   S083's constants; any lowering of a mate floor; re-mining the row (DEC-308)
decisions:  DEC-308, DEC-307, DEC-306, DEC-305, DEC-171, DEC-158, DEC-142, DEC-141
closes:
blocks:     S083
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-23 -- no rule at fault: at S083's re-landed constants on S292's tree the row's depth-10 mate is lost to reverse futility at ply 3 (a non-PV cut node, beta -1166, static -404, margin 441, over a lower bound; S291_trace.txt), the null move behind it on the same lower bound; RfpMaxDepth 4 restores it, so it is DEC-158's price and joins S083's H1 rulings with this attribution (DEC-308 (2), DEC-311 (1)); reach at the shipped weights yes in that kind over S289's four sets (S291_reach.txt); E21 lost the same way. Found beside it: reverse futility failing high against a whole search's mated bound, 90 returns over the four sets and 0 in 11503 ordinary-play positions (S291_census.txt), moving eight reported scores and two best moves under a scratch guard; ruled BUGS-binding under DEC-171 (c) by the coordinator (DEC-311 (2)), taken by S293, which pauses S083. No src/ or tests/ change; fast suite 50/50 both builds, bench 6042933. Implemented by one Opus 5.5 subagent, fast-checked twice (wording, two readings owed and taken, the classification ruled); stamped by the coordinator.

## Why this step exists (the coordinator, 2026-10-22, DEC-308)

S083's re-landing on S289's guard (`.tuning/coord/S083/s083_reland.diff`)
reads the multicut mined row's mate at depths 9, 11 and 12 and not at 10,
where the shipped engine reports it (`adocs/data/S083r_rows.txt`); a re-mine
by S097's commands over 7..10 found no row separating E21 on that tree
(`adocs/data/S083r_remine_multicut.log`). S289 attributed the paste's lost
mined mates to reverse futility's and the multicut's known price (DEC-158),
but not this row. The machine is held by S289's SPRT until its verdict; this
step runs after it and before S083's own run. S289's trace tooling
(`adocs/data/S289_trace.py`) and its reach method are the precedent.

## 2026-10-23: the node, the rule and the reach -- no rule at fault (the implementer)

One Opus 5.5 subagent. `../chesso-s291` (branch `s291` at `49ad271`) takes
`adocs/data/S291_*`, their README rows and this file. Scratch: `../chesso-s291t`,
detached `49ad271` plus the three `src/` hunks of `s083_reland.diff` --
**byte-identical to fixture Q `38a15f4`'s `src/`** (blob ids per file) -- with
`adocs/data/S289_trace.py` and then `adocs/data/S291_trace.py` applied to its
`src/search.cpp` (never shipped): `build-tune` at Q (bench 3945487),
`build-tune-ship` at `49ad271`'s constants (bench 6042933), each the tree's own
total and the row's own node counts with every trace variable unset; and a
Release `build-rel` at Q exactly, no trace, for E21's direct case.
`../chesso-s289q`'s libraries read only; `../chesso-s083r` not touched. The
engine's output and python-chess's move application only; nothing read off a
board. Nothing committed.

**The trees are S292's, not the accepts' wording.** The accepts name
`../chesso-s083r` and `cd50178`'s tree, both on S289's reverted guard; the brief
moved both: the re-landed constants are Q (`908ed57` plus the three hunks), the
shipped tree `49ad271` (S292's guard, bench 6042933). The row reads the same at Q
as S083r's re-landing read it (d9 d11 d12, 456117 nodes at 10).

### 1. The node and the rule (`adocs/data/S291_trace.txt`, `S291_options.txt`)

**One option at a time** at Q, depths 7 to 12 through `search_fen()`
(`S291_row.py`): `RfpMinPly` 63, `RfpMaxDepth` 0 or 4 (DEC-158's challenger),
`LmrBase` 0 and `SeeLmrExtra` 0 bring depth 10 back; `SeMultiCut` 0, `SeExtend`
0, `NullMoveBase` 16, `ProbCut` 0, `RazorDepth` 0, the shallow-depth caps,
`QsFutility` 0 and `CorrHistMax` 0 do not (`SeMultiCut` 0 also loses 11).

**The node, by the trace at depth 10.** The root searches f3e3 sixth at 1166 and
the mating move g6f4 twenty-first, against alpha 1166. Along g6f4 b2d4 e5d4 --
the line the engine reports at 9, 11 and 12 -- the ply-2 node (Black, PV, window
[1166, 48993]) searches d3d4 first at 1148, so e5d4 is probed with the zero
window, and at **ply 3, White to move -- the side being mated -- remaining depth
7, a non-PV cut node, window [-1167, -1166], reverse futility returns -845**:
static score -404 (the estimate the same), margin 441 (63 a ply over 7), -845 >=
beta -1166. The node's table entry then is a **lower bound**, -48991 at depth 7,
from its own earlier visit with a mate-band window: nothing proves White mated.
e5d4 scores 845 at ply 2, the ply-2 node 1158, g6f4 1158 at the root, and the
root reports f3e3 at cp 1166. **One node at a time** (`$S291_NO_RFP`,
`$S291_NO_NULL`): with reverse futility removed there alone the **null move**
fails high at the same node at -1078 -- S292's guard rightly does not fire, the
entry being a lower bound (DEC-310) -- and the root still reads 1166; with both
removed there alone the root reports **m5 at depth 10** (719941 nodes), the other
depths as before.

**Why 9, 11 and 12 report it.** Depth 9: g6f4 is sixth against alpha 38, the
ply-2 node's alpha is 287 when e5d4 is probed, so the ply-3 node sees beta -287
at depth 6 (-404 - 378 = -782, no cut) and finds the mate. Depth 11: a mate in 6
is already the root's alpha when g6f4 is searched, so the ply-3 node only meets
mate-band windows, where both rules are guarded off. Depth 12: the ply-2 PV
node's first move reaches 48989 before e5d4; the ply-3 node's full-depth visits
are mate-band again, its three reverse futility cuts at depths 1 to 3 shallow
ones that decide nothing at the root.

**The shipped constants at depth 10**: g6f4 sixth against alpha 45, the ply-2
node's alpha 640 at the probe, the ply-3 node's static -374: -815 < -640, no
cut, mate found. The row turns on whether the bar a sibling raised clears the
mated side's static score plus the margin; the re-landed constants move both the
static score (-374 to -404) and the root's order and alternative score (g6f4
sixth against 45, twenty-first against 1166).

**E21 at the same depth.** `E21_multicut_mate_band_gate_dropped` at Q loses depth
10 the same way: the same root order and the same reverse futility cut at the
same node, -845 against -1166, 454589 nodes (S083r's cell); 9, 11 and 12 found,
as at Q. At the shipped constants E21 also ends on that cut: under E21 the ply-2
node's first move d3d4 reads 931 (640 without), so the ply-3 node sees beta -931,
-815 clears it, cp 1032. What E21 moves on this row is a sibling's score
upstream; the loss is the same reverse futility cut. At Q the row no longer
separates E21, as S083r's re-mine read (0 of 269 in 7..10).

**Through UCI**, the engine as played (`S291_uci_row.py`, Hash 16, `ucinewgame`,
`go depth 14`): neither tree reports this mate before iteration 13; both report
m5 at 13 and 14 with bestmove g6f4. The depth-10 profile is a cold single
search's.

### 2. Reach at the shipped constants (`adocs/data/S291_reach.txt`): yes, and it is DEC-158's price

`S289_rows.py` over S145's 82 roots, S165's 104 defender nodes, S145's 186
defender nodes (distances proved by `adocs/data/S291_defender186.py`, S289's
enumeration) and S156's 318 mined rows at `49ad271`'s defaults, at `RfpMinPly` 63, and at that with
`NullMoveBase` 16 (exact at the last iteration):

| set | ship | no reverse futility | and no null move |
|---|---|---|---|
| S145's 82 roots (S154's mates in three) | 40 (m3 12/24) | 55 (m3 20/24), +15 -0 | 72 (m3 23/24), +32 -0 |
| S165's 104 defender nodes | 83 (m3 2/15) | 93 (m3 12/15), +10 -0 | 97, +14 -0 |
| S145's 186 defender nodes | 143 (m3 3/30, m4 0/16) | 157 (m3 16/30, m4 1/16), +14 -0 | 172 (m3 25/30, m4 7/16), +29 -0 |
| S156's 318 mined rows, depth 10 | 164 | 199, +36 -1 | 204, +40 -0 |

So reverse futility, with the null move behind it, hides proved mates at the
shipped weights in every set -- the trade DEC-158 measured and priced (S148's
sweep read 39 of 82 and mates in three 12 of 24 at the ceiling 15 against 52 and
18 at 4, and 4 read H0 at `{-5, 0}`). Each row it hides is listed in the file.
At the row's node nothing in the table proves a mate, so no proof is outvoted --
the line DEC-306 and DEC-310 drew for the null move. No gain in any set is a mate
in one or two: those classes read complete when shipped.

**The sharper shape, asked of the same rule** -- DEC-306's: reverse futility
returning at a node whose entry is a mated upper bound or exact score from a
shallower search that skipped no move. It occurs (90 returns over the four
sets) and hides no exact mate: removing it finds none (and loses mined row
171). Section 5 has the readings and the classification.

### 3. Outcome: no rule at fault; the attribution for the owner's H1 ruling (DEC-308 (2))

By the brief's step 4, the work stops after the reach. **Attribution**: the row's
depth-10 loss at S083's re-landed constants is reverse futility's measured mate
price (DEC-158's form) at the mated side's ply-3 node, with the null move's
exposure where nothing is proved (DEC-087 (e)) behind it at the same node, both
reached because the root's order put an alternative at 1166 ahead of the mating
move at that depth. Not the multicut: `SeMultiCut` 0 does not bring it back, and
E21 loses it the same way. Depths 9, 11 and 12 find it; through UCI both trees
find it from iteration 13 and neither earlier.

**For the owner, on S083's H1 (DEC-265 form).** *Why*: the row is a golden
(DEC-142) that is red at Q, the reason is a priced trade and not a defect, and a
re-mine found no successor (DEC-308 forbids widening it); only the owner edits or
retires a test. *Options*: (a) keep the row as written -- S083 cannot land these
constants; nothing else changes. (b) Rule it reverse futility's price at Q and
retire the row with S083's landing, recorded as a decision: at Q, E21 is still
killed by S243's direct case "the multicut never ends a node on a mate from its
verification" (`adocs/data/S291_e21_direct.log`: red under E21, green without;
the mined row red either way), the state S022 v2 and S259 recorded before their
re-mines. Costs the mined row's second witness of the multicut's guard; the
guard itself stays held. (c) Lower the reverse futility ceiling to keep the
row (`RfpMaxDepth` 4 brings it back at Q) -- DEC-158's H0 measured that cost
(Elo -5.66 +/- 4.33 at 4); not this step's. *Recommend* (b):
the miss is a priced trade at one depth of a cold search, the engine as played
finds the mate at the same iteration on both trees, and the guard the row was
mined for keeps its direct case.

### 4. Gates and documents

- Nothing in `src/` or `tests/`: bench 6042933 unchanged, no golden moved, no
  mutant, no SPRT, no Debug self-play; `tools/gate_extra.sh` not run (no code
  change).
- `../chesso-s291`, after the last edit: the fast suite **50 of 50 in both
  builds**, `./clang-format.sh --check` clean (`CLANG_FORMAT_MAJOR=22`), both
  builds benching 6042933; `plan_prose_check.py` `--prose`, `--citations`,
  `--touches`, `--params` and `--gate` flag nothing.
- `DEV_MANUAL.md` and `MANUAL.md` checked: no change -- no option, default,
  output or behaviour moves. `README.md` not touched.
- Disk: `../chesso-s291` 423 MB with both builds, `../chesso-s291t` about 270 MB
  with two tune builds and a Release build; free space 8.9 GB at the start, 8.1
  GB after; the fast check's readings added only scratch tables.

### 5. The sharper shape, read and classified (the implementer, asked by the fast check, 2026-10-23)

*The brief's sentence "if yes, that is a BUGS-rule defect" is superseded, for
the row's own mechanism, by DEC-308 (2)'s wording: a known price (DEC-158's
form) joins the H1 rulings with its attribution.*

**The shape**: a reverse futility return -- beta outside the mate band, a
fail-high -- at a node whose table entry is a mated upper bound or exact score
(`<= -MATE_MIN`) stored by a node that skipped no move, S292's condition. That
is DEC-306's shape for the null move exactly; DEC-158 priced reverse futility
where nothing is proved, so it does not settle this one. Read with
`adocs/data/S291_trace.py`'s `$S291_CENSUS` (the returns) and `$S291_RFP_GUARD`
(reverse futility not tried there; a scratch guard, never shipped).

- **The four mate sets** (`adocs/data/S291_reach.txt` section 3): 45, 2, 13 and
  30 returns of the shape over S145's roots, S165's 104, S145's 186 and the
  mined set. With the guard: **no exact mate gained or lost in S145's 186**
  (m2 54/54, m3 3/30 either way); one row there, proved mated in 3, reports an
  inexact `mate -4` at iteration 14 where the shipped engine reports cp 0, and
  both report the exact `mate -3` from iteration 15 with the same best move
  (its six returns at ply 4, depths 8 to 11, beta 0 or 1, on an upper bound of
  -48992 stored at depth 7). Elsewhere the guard gains nothing, loses mined row
  171 (m5 -> cp 2104), moves six roots' cp scores and two best moves (root 76,
  proved mate in 5, cp 0 -> cp 529; mined row 120, the same score).
- **Ordinary play** (`adocs/data/S291_census.txt`, S292_census_s210.txt's shape:
  the 11503 S219 A/A positions at depth 10): **0 returns of the shape** in
  981078145 nodes; the guard moves no node, no root score, no root best move,
  and no reported mate appears or disappears. Reverse futility does return 42
  times (8 positions) on a mated bound from a node that skipped a move -- not a
  proof (DEC-310), not the shape. The census build is the Release build root
  for root and node for node.

**Classification: a scoped defect for a filler step (DEC-171), not
BUGS-binding.** Neither reading shows a hidden mate or a moved root best move:
the 186 set gains no exact mate and moves no best move, and ordinary play never
meets the shape at depth 10. It is still a defect in kind -- the return argues
">= beta" at a node a whole search proved mated, the comparison DEC-306 read as
a zugzwang for the null move. Weighed and not counted: the inexact `mate -4`
one iteration before both builds' exact `mate -3`, and the two best moves the
guard moves in the constructed and mined sets, both in positions with no exact
mate reported either way.

**The coordinator's ruling (DEC-311): BUGS-binding under DEC-171 (c).** (c)
binds a defect able to move a reported score, best move or line, with no
ordinary-play limit, and its exclusions (illegal positions, implausible inputs,
custom commands, dead code) do not reach a legal position's search. The
readings show the shape moving eight reported root scores (six of S145's
roots, one of S145's 186 defender nodes, mined row 171) and two best moves,
among them S145 root 76, a proved mate in 5 for the side to move, reported cp 0
(b7c7) against cp 529 (b7b2) with the guard. S293 takes it, blocking S083; the
guard's loss of mined row 171 is traced there before any run.

**Proposed filler step** (for the coordinator): *goal* -- reverse futility is
not tried at a node whose entry is a mated upper bound or exact score from a
node that skipped no move (DEC-306's shape, S292's condition reused), shipped
only if measured, as its own change. *accepts* -- the case observed red first
on the shipped tree (a node driven so the entry holds S292's mated whole-search
bound and reverse futility's margin clears beta; S191's probe `rfp_cutoff`
false with the guard), a mutant it kills, both builds green, bench and
`search_bench` d9/d12, every moved golden by its script, gate_extra, Debug
self-play, the four sets and this census re-read, mined row 171's loss traced
and explained before any run, implemented from the literature's description
(DEC-221: reverse futility as static null move pruning, the null move's premise
in David-Tabibi and Netanyahu 2002), and an SPRT `{-5, 0}` against its parent;
H0 reverts it whole. *excludes* -- reverse futility's margins and ceiling
(DEC-158, S127), the null move.

