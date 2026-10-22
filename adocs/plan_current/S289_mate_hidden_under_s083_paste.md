id:         S289
goal:       find by tool why S083's pasted constants hide a mate the parent finds and lose seven exact mined mates, and fix the search rule at fault as its own measured change
accepts:    the node and the rule that hide `tests/test_engine.cpp` "reverse futility at ply 2 does not hide a mate from the side to move"'s mate at S083's candidate constants are named from the engine's own output (a trace, a counter or a one-option-at-a-time bisection in the tune build), never from reading the board (CHESS); the mined mates S156's sweep reads exact on the parent and not on the paste are listed by key and each attributed the same way; if a rule is at fault, its fix ships as its own change with a guard test observed red first at the paste's constants and green at both the paste's and the shipped constants, a mutant that test kills (DEC-141), Debug self-play and gate_extra, every golden it moves re-derived by its script, and its verdict decided by SPRT against its parent if it moves play, bounds pre-registered; if no rule is at fault, a proposed decision stating what the paste trades, for the owner (DEC-304 (11)); the fast suite green in both builds
touches:    src/search.cpp negamax_at (the null-move block: `table_proves_mated` beside the depth floor, and the guard list's pointer to it); tests/test_search.cpp ("a node the table proves mated makes no null move"); tests/test_engine.cpp ("a root mated in two reports the mate even where its node could pass", `<fstream>`); tests/CMakeLists.txt (`CHESSO_SOURCE_DIR` for test_engine); tests/test_mate_carry.cpp (the ceilings' command gains S289's grid, comments only); adocs/data/S170_cases.tsv (A, C and D budgets by the rule, and its header); tools/mutants/S289_null_move_table_guard.py; adocs/data/S289_* and README rows; this file
excludes:   S083's constants (no refit, no re-anchor, no golden S083's landing owes); the values of the search's margins beyond what the fix needs (S127 owns them); any lowering of a mate floor
decisions:  DEC-305, DEC-304, DEC-303, DEC-299, DEC-142, DEC-141, DEC-171
closes:
blocks:     S083
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:

## Why this step exists (the coordinator, 2026-10-22, DEC-305)

S083's landing pasted curve 0's candidate (`.tuning/s083/curve/s0/fit_6226944_s1.hpp`,
every game of S082's training side at 46 qs rows a game) onto `5a22a9c` in
`../chesso-s083l` and stopped under DEC-304 (11) -- S083's step file, "the
landing -- stopped at the floors and a hidden mate", and
`adocs/data/S083_rfp_ply2.txt`, `S083_mined_floor.log`, `S083_floor_sweep.log`:

- `EXACT_FLOOR`'s mined set reads 157 exact against the floor 163 (154 with
  the guard weakened); the parent reads 164 and 160.
- The ply-2 property case is red in both builds: through UCI the parent
  reports `mate -2` from depth 4 to 8, the paste cp 1168 to 913 at depths 1 to
  8 and never a mate. One tune option at a time: `NullMoveBase` 16 restores
  the mate at depth 6, `RfpMinPly` 63 at depth 8; `RfpMaxDepth` 0,
  `RfpMargin` 2000, `RazorDepth` 0 and `ProbCut` 0 do not.
- `MATE_IN_THREE_FLOOR` holds (12) but no longer separates (11 weakened).

The paste moves the material scale (PAWN 94 -> 84, KNIGHT 327 -> 371,
BISHOP 308 -> 352, ROOK 487 -> 519, QUEEN 716 -> 710, every table) under
search margins fitted at the old scale (S261's lane). Whether the cause is a
rule that can hide a mate whenever a mated side leads in material by enough
-- reachable at the shipped constants in other positions, which would make
it a BUGS-rule defect (DEC-171) -- or a margin the scale change crossed, is
what this step finds. Every later evaluation refit (S135 onward) meets the
same search, so the answer is wanted once.

The paste itself: `.tuning/coord/S083/s083_landing.diff` (red, not a
commit); the worktree `../chesso-s083l` holds it with both builds made.

## 2026-10-22: the node, the rule, the reach and the fix (the implementer)

One Opus 5.5 subagent, in `../chesso-s289` (branch `s289` at `4a3a0e5`), with
scratch worktrees for the paste (`../chesso-s289p`: `4a3a0e5` plus the paste's
`src/eval_tables.hpp` and `src/evaluation.cpp` hunks, byte for byte the staged
ones in `../chesso-s083l`) and for the trace (`../chesso-s289t`,
`../chesso-s289pt`: the same two trees with `adocs/data/S289_trace.py` applied,
never shipped). Nothing committed; two fixtures are commit objects on no branch
(F1 `efa6831`, F2 `9d7639e`, F3 `c72ce84`, P `64b7fd8`; S083's landing's form). Chess facts come from the
case oracles, S145's enumeration, python-chess's legal-move lists and stockfish
through python-chess; nothing was read off a board.

### 1. The node and the rule (the ply-2 case at the paste's constants)

`adocs/data/S289_trace.txt`, the engine's own trace through `go depth 8` as
`deepen_scores()` drives it. The line is h1g1 g8f8 g1h1 f8f1 (python-chess):
the root and the ply-2 node each have one legal move.

- **Iterations 4 and 5: late move reduction, not a hiding rule.** At ply 1
  (White, a PV node, depth 3 then 4) the mating quiet g8f8 is ordered 13th of
  13 and reduced by 1 then 2, so the ply-2 node is searched at depth 1 and the
  table answers it with iteration 3's depth-1 lower bound, 1141 for Black -- a
  correct depth-1 answer, since quiescence cannot see the quiet f8f1. The
  reduced score is below alpha, so no re-search. On the parent the same move is
  ordered 3rd from iteration 2 on, unreduced at 4, the mate found there, and
  the line is the PV from then on, where no null move is made. The paste's
  weights reorder the history, nothing else.
- **Iterations 6, 7 and 8: the null move at the mated side's node.** The
  reduced search now reaches the ply-2 node at depth 2 or 3 and finds Black
  mated by the move it searched (`-48996`, stored as an upper bound; the node
  has one legal move); the score clears alpha, so late
  move reduction re-searches the move at depth 5, 6 and 6 (`deeper`, one past
  the child depth). There, at ply 2, **Black to move -- the side being mated,
  its static score 1134, 1125 and 1116 against beta 996, 978 and 913 -- makes a
  null move** (R 3 or 4, the null child at depth 1), and the pass's reduced
  search fails high at 1406. The node returns 1406 a moment after the table
  bounded it at -48996 -- the comparison David-Tabibi and Netanyahu's verified
  null-move pruning reads as a zugzwang (section 4); no tool here was asked
  what the position is. g8f8 is refuted, and the root reports the best of the
  rest, cp 996 to 913.
- **The four toggles of S083's record, read off the same trace.**
  `NullMoveBase` 16 removes the pass, so the first re-search, iteration 6,
  keeps the mate. `RfpMinPly` 63 finds it at 8 only because the root's score
  collapsed to about 0 on a repetition line, so beta at the ply-2 node is 21 and
  the same depth-1 null search no longer clears it. `RfpMaxDepth` 0,
  `RfpMargin` 2000, `RazorDepth` 0 and `ProbCut` 0 touch neither mechanism.
- **So a rule is at fault, and it is not a margin the scale crossed.** The null
  move has no margin; the scale moved the ordering, which exposed a pass that
  outvotes what the search has just found at the same node. Section 3 shows
  the same exposure at the shipped constants.

### 2. The mined mates (S156's set at depth 10)

`adocs/data/S289_mined.txt`. The paste's net -7 is **15 lost** (rows 51, 91,
110, 130, 146, 166, 168, 171, 180, 185, 186, 216, 221, 225, 246, each with its
game id and FEN there) **and 8 gained** (89, 104, 107, 179, 192, 210, 226,
231). One tune option at a time on the paste: `RfpMinPly` 63 restores 15 of 15,
`RfpMaxDepth` 0 10, `RazorDepth` 0 5, `NullMoveBase` 16, `SeMultiCut` 0,
`SeExtend` 0, `LmrBase` 0, `HistPruneMaxLmrDepth` 0 and `RfpTtEstimate` 0 four
each. The trace along the parent's own mating line (first non-loop return at a
mated-side node, last iteration): **reverse futility 6** (51, 130, 146, 166,
216, 225), **the multicut 4** (91, 171, 221, 246), **the null move 1** (110), a
table bound carried from an earlier cut 2 (168, 180), unattributed 2 (185,
186). The paste's gains are the same rules' (`RfpMaxDepth` 0 restores 4 of the
8 on the parent). **Reading: no new defect.** The mined floor's fall is
reverse futility's and the multicut's measured mate price -- DEC-158's trade --
redistributed by a scale whose static scores sit higher above beta at the
mated side's nodes; it is S127's lane (margins at the new scale) and DEC-304
(11)'s ruling, not this guard's. The guard regains **none of the 15** --
row 110, the one loss the trace pins on the null move, stays lost -- and the
paste with the fix reads **158 against 163** because it finds one row the
parent misses as well (131: the parent cp 2289, the paste cp 2110, the paste
with the guard mate 5). The floor still falls, and per DEC-305 that is the
owner's.

### 3. Reach at the shipped constants: **yes** -- a BUGS-rule defect (DEC-171 (c))

`adocs/data/S289_reach.txt`. The same mechanism hides proved mates the shipped
engine should report, from the mated side's root:

| set (side to move) | ship | ship, no null move | fix |
|---|---|---|---|
| S165's 104 defender nodes (mated) | 77: m1 50/50, **m2 25/31**, m3 2/15, m4 0/8 | 89: m2 31/31, m3 6/15, m4 2/8 | 83: **m2 31/31**, m3 2/15, m4 0/8 |
| S145's 186 defender nodes (mated; distances proved here) | 131: **m2 42/54** | 149: m2 54/54 | 143: **m2 54/54** |
| S145's 82 roots (mating) | 40 | 43 | 40 |
| S156's mined set (mating), depth 10 | 164 | -- | 164 (three scores move, no exactness) |

The table's readings are `adocs/data/S289_rows.py`'s, a fresh `ucinewgame` per
row. Instrument 4's own sweep over S165's set, one process for the whole set,
reads the parent 77 (m2 25/31, m3 2/15) and the fix 82 (m2 31/31, m3 **1/15**):
the fix loses a mate in three there while it gains six in two
(`adocs/data/S289_defender_sweep.txt`), the either-way movement section 4
states.

Six of S165's 31 roots proved mated in two (S145's enumeration; stockfish
`Mate(-2)` on 31 of 31, `adocs/data/S289_oracle.txt`) report `cp 0` in four
and `cp -676` / `cp -672` in two **through all twelve iterations**; the trace
of one (`rbrb4/p1p1p1k1/P1P1P3/6K1/8/7Q/8/8 b - - 1 1`, the trace file's
section 4) shows the paste's mechanism exactly: the reduced search finds the
defender's ply-2 node mated at depth 3 or 4, the re-search at 6, 7 or 8 passes
there and the pass's search fails high at 209 to 280. Every one is reported `mate -2` from
iteration 7 with the guard. Ordinary play: S210's census shape over 11503 S219
A/A positions at depth 10 (`adocs/data/S289_census.txt`) -- the guard changes
145 searches, 4 root best moves and 11 root scores, nodes x1.000109.

What the guard does not reach: six deeper defender rows (mated in 3 and 4) the
no-null build finds and the fix does not. Row 101's trace: the mating move is
the 24th at its node, reduced by 4, and the mated side's node is first reached
at depth 6 with nothing found about it in the table, so the guard has nothing
to read and the pass's search fails high -- the exposure DEC-087 (e) accepted
when it dropped the verification search. A separate question for the owner,
not this fix (section 4's alternatives).

### 4. The fix: the table's proof outranks the pass

**The rule.** In `negamax_at`'s null-move block, beside the depth floor:
`table_proves_mated` -- the node's table entry (the copies taken before
anything recursed) is a `TT_ALPHA_NODE` or `TT_PV_NODE` whose score is at or
below `-MATE_MIN` -- and the pass is made only where it is false. A shallower
search of this node's own moves found the side to move mated by every move it
searched -- not necessarily every legal move: at a non-PV node with alpha
outside the band the shallow-depth rules skip moves without touching
`best_so_far`, so such a bound can be stored with moves unsearched. A pass that
fails high against that bound is the comparison the paper reads as a zugzwang.
No constant, so no seed (DEC-134); no UCI option, so no surface change.

**The source, DEC-221.** Omid David-Tabibi and Nathan S. Netanyahu, "Verified
Null-Move Pruning", ICGA Journal 25(3), 2002, pp. 153-161 (arXiv:0808.1125),
section 2: "Null-move pruning will fail badly in zugzwang positions since the
basic assumption behind the method does not hold." Section 3, their zugzwang
detection: "If the null-move search indicates a cutoff, but the search shows
that the best value is smaller than β, this implies that the position is a
zugzwang, as the value from the null move is greater than or equal to the value
from the best move." The guard makes that comparison with the search of the
moves already in the table, so the pass is never made; only inside the mated
band, where that search's answer is the strongest a search gives. What holds
whatever the bound is worth: at the node it fires at, the guard only removes a
pass, so a wrong bound costs the subtree the pass would have saved and cuts
nothing there. Elsewhere in the tree it is not neutral -- the entries that node
then stores and the ordering they feed can move a score or a first-mate
iteration either way, and the readings show both (`S289_reach.txt` row 93,
`m-2@8` -> `m-2@9`, row 131 -474 -> -548; `S289_defender_sweep.txt`, one mate
in three lost beside six mates in two gained). The
Chess Programming Wiki's Null Move Pruning page says the same of the remedy
class: "For piece endgames, verification search can be an effective heuristic
in preventing zugzwang blindness from NMP." Implemented from those
descriptions; no engine's code was read.

**Alternatives, and why not.**

- *The paper's verified null-move pruning in full* (on a fail-high, reduce the
  depth by one ply and search on, re-searching at the full depth on a detected
  zugzwang). It reaches the six deeper rows of section 3 that this guard cannot
  -- but it is a play-altering technique with a tree cost (the paper's own
  Table 3: verified R = 3 about twice standard R = 3's nodes at depths 9 to
  11); DEC-087 (e) dropped it, and S114's step file records the price measured
  elsewhere (Berserk #524, -0.68 STC). It is the owner's to reopen as its own
  step, with this guard in place as its baseline.
- *Mate-band table entries answering at any depth* (in `tt_entry_answers`):
  would answer the re-search with the reduced search's finding before any
  pass, but changes what the table means everywhere, for lower bounds too,
  where a defender's pruned move can make a mate optimistic; a larger change
  than the defect.
- *Skip the pass on any upper bound below beta* (the table-driven null-move
  skip, NM03's shape): moves ordinary play by design, a candidate with its own
  verdict (S114's step file lists a table skip for S127), not a fix.
- *Less reduction for the late quiet*: `LmrBase` 0, `LmrDivisor` 2000 or
  `LmrPv` 2 each bring the ply-2 case's first iteration back at the paste
  (section 5), and each changes every tree; not a defect's fix. An extension
  for a node with one legal move would be another such change; not measured.
- *Nothing* (accept it under DEC-087 (e)): the six shipped-constant reports of
  `cp 0` or about -670 for a proved mate in two are a reported-score defect
  (DEC-171 (c)), and the mechanism -- a pass outvoting what a search just found
  at the same node -- is not the trade DEC-087 (e) priced, which was the
  verification search's cost against the pass's exposure where nothing has
  been found.

**Tests, observed red** (`adocs/data/S289_red.log`; its section A is the final
form, section B the first draft's red on `4a3a0e5` before the guard existed):

- tests/test_search.cpp "a node the table proves mated makes no null move", six
  legs over the 104 S165 rows: no entry, a mated upper bound, a mated exact
  score, the mated score as a lower bound, an upper bound and an exact score
  outside the band. Only the second and third may stop the pass. On fixture F3
  under NM01 (the flag false, so the null-move condition reads as `4a3a0e5`'s):
  104 rows red on each of the two mated legs, the four premise legs clean.
  Under NM04: 104 rows red on the mated-exact leg alone. Green with the guard,
  both builds.
- tests/test_engine.cpp "a root mated in two reports the mate even where its
  node could pass": under NM01 the six roots of section 3 never report (four
  `cp 0`, `cp -676`, `cp -672`). Green with the guard, both builds, 0.23 s. Its
  checks establish the root only (legal, reachable, not in check, more than
  kings and pawns); the pass is never made at the root, and that the case can
  go red is shown by the red above, not by a premise of its own.
- At the paste's constants (`../chesso-s289p` with the final change): both
  green in both builds.

**Mutants** (`tools/mutants/S289_null_move_table_guard.py`, NM is a new
prefix; `adocs/data/S289_mutation.tsv`). On fixture F3 (the final cases):
**NM01** (the flag made false -- the engine before S289) killed by both new
cases and by `test_mate_carry`'s majority check (the budgets sit where the
guard's grid put them); **NM02** (a lower bound in the band also stops the
pass) by the lower-bound leg alone, bench-blind; **NM03** (any upper or exact
bound stops it) by the two out-of-band legs and `test_mate_breadth`; **NM04**
(the exact disjunct dropped) by the mated-exact leg alone, bench-blind. 4 of 4.
NM01's first form, the use deleted, was stillborn (`-Werror`, an unused
variable) and is recorded so. On fixture F2, the sixteen existing mutants whose
cases the guard could mask: M01-M04, M06a, N01, N02, E21, B04, J01, J02, R02,
C02, C05, LQ02, LQ03 -- **16 of 16 killed**, M06a (RFP's floor at 2) now by the
new defender-root case as well as the ply-2 case. 20 of 20.

**Bench 6042933 -> 6042928**: positions 1, 3 and 4 of eight read 1, 1 and 3
nodes fewer at depth 14, same scores and lines, **all eight replies
identical**; `tools/search_bench.py` at 9 and 12 node- and move-identical.
**Goldens** (`adocs/data/S289_goldens.txt`, each by its script on fixture F1,
DEC-299's guard): `EXACT_FLOOR` 164 shipping / 160 weakened (163 stays),
`MATE_IN_THREE_FLOOR` 12 / 7 (11 stays), first-mate 9 and 9 at mate 5, S192's
13446, `short_line_ceiling` 2, 5, 4, 0, 1, 2 over S202's, S261's, S286's and
S289's grids -- all unmoved; **the S170 budgets move by the rule**, A 1000000
-> 500000 (6 lines), C 1200000 -> 1000000 (15), D 500000 -> 1200000 (3; D's old
cell went silent), applied by the script's own rewrite in this change, the
grid added to the ceilings' command; the four table-less mined rows read the
same distance at every depth (the multicut row's node count differs at depth
14 alone). No floor fell, nothing rose.

**Gates.** The fast suite **50 of 50 in both builds**, format clean
(`CLANG_FORMAT_MAJOR=22`, DEC-146), both builds benching 6042928, after the
last edit. `tools/gate_extra.sh` **five stages green in 997 s** (prose,
citations, the Debug binaries' fast suite 382 s, the sanitizer build 563 s,
deep perft 52 s; `.tuning/coord/S289/gate_extra.log`). `plan_prose_check.py`
`--prose`, `--citations`, `--touches` and `--params` flag nothing. Debug
self-play is the coordinator's (section 6).

### 5. The paste's constants with the fix (`../chesso-s289p`, bench 3945685 unchanged)

`adocs/data/S289_paste.txt`. **The ply-2 case reports `mate -2` from
iteration 6 to 8** (the paste alone: never), so the null move no longer hides
it; the case is **still red at `REQUIRE( first_exact == 4 )`, 6 against 4**.
What remains is section 1's iterations 4 and 5: late move reduction of the
mating quiet, 13th of 13 at the PV node at ply 1 -- not a hiding rule, and no
guard of this step's kind can reach it (`LmrBase` 0, `LmrDivisor` 2000 or
`LmrPv` 2 each bring 4 back, deciding nothing). Section 7 has the question.
**`EXACT_FLOOR`: 158 shipping, 155 weakened** (S156's script on fixture P;
the paste alone 157 and 154), so 163 still falls -- DEC-305 leaves it to the
owner. **`MATE_IN_THREE_FLOOR`: 12 and 11**, the paste's own, still not
separating; the gate at `RfpMinPly` 1 is red on mate-in-two exactness alone.
Both new cases are green at these weights in both builds.

### 6. The SPRT and the Debug self-play

`adocs/data/S289_sprt.sh`, pre-registered and refusing to start until the
coordinator pins the pair and fills the bracketed sentence: CAND the fix's
commit, REF its parent, 8+0.08, Hash 16, `noob_3moves.epd`, **`{-5, 0}` nElo**
-- a correctness fix measured as a non-regression, S039's and S210 F22's form;
a true zero sits on H1's bound. Cost (DEC-143): 25591 games, 12.1 h at 2110 an
hour with a true zero; 41861, 19.8 h, at the midpoint; cap 20950 rounds (41900
games, 19.9 h), abort at 40 h, the forfeit and crash rules. **H1**: the guard
stays, S083 re-lands on it. **H0**: first a reason to look at the run; if it
stands the guard is reverted whole and the defect goes back to the owner, the
BUGS rule against a measured cost (DEC-158's form the precedent); S083 stays
paused. **No verdict**: the guard stays, as on H1. Census before it:
`adocs/data/S289_census.txt` (DEC-242 does not discharge: the bench moves, 4
root answers move).

Debug self-play (DEC-141 (1)), for the coordinator on the committed tree:
`.tuning/coord/S289/debug_selfplay.sh` -- `build-debug`, `fastchess` four
rounds `-repeat` at 4+0.04, Hash 16, `noob_3moves.epd`, trace log with engine
stderr, then `grep -c Assertion` and `grep -c disconnect`; prints
`DEBUG-SELFPLAY-DONE` last.

### 7. Proposed, for the coordinator

- **specs.md**, the search row, after the S165 sentences on the null move's
  mate band: "**A null move is not made at a node the table proves mated, since
  S289**: where the node's entry is an upper bound or an exact score at or below
  `-MATE_MIN` -- a shallower search of the node's own moves found the side to
  move mated by every move it searched -- the pass is not tried: a pass that
  fails high against that bound is the comparison verified null-move pruning
  reads as a zugzwang (David-Tabibi and Netanyahu 2002). It closes the case
  S083's paste exposed and S289 reached at the shipped weights: a reduced
  search finds the defender's node mated, late move reduction re-searches the
  move at full depth, and the pass made there fails high. At that node the
  guard only removes a pass; elsewhere the entries and ordering it changes can
  move a score or a first-mate iteration either way. Held by
  `tests/test_search.cpp`'s `a node the table proves mated makes no null move`
  over the 104 S165 rows and `tests/test_engine.cpp`'s `a root mated in two
  reports the mate even where its node could pass` over its 31 rows mated in
  two. The pass where nothing has been found stays exposed, DEC-087 (e)." And
  the bench figure where specs quotes one.
- **DEC text** (the owner's or the coordinator's): "DEC-306 (proposed) S289's
  null-move table guard ships as a correctness fix decided by a `{-5, 0}` SPRT.
  Context: S083's paste hid the ply-2 case's mate; the trace names the null move
  at the mated side's node in the late move reduction's re-search, failing high
  against the table's mated bound from the reduced search a moment earlier; the
  same mechanism hides six of S165's 31 proved mates in two at the shipped
  weights (`cp 0` through 12 iterations), a DEC-171 (c) defect. Decision: no
  pass at a node whose entry is an upper bound or exact score in the mated band;
  `{-5, 0}` at 8+0.08, H0 returning the defect to the owner. Rejected: the full
  verified null-move search (DEC-087 (e), a tree-cost technique, the owner's to
  reopen); mate-band entries answering at any depth (changes the table's
  meaning); accepting it under DEC-087 (e) (that priced the pass's exposure
  where nothing has been found, not a pass outvoting what a search just found).
  Consequences: S083 re-lands
  on the guard's tree; the ply-2 case's first-iteration clause at the paste is
  the owner's question below; the mined floor at the paste (158 against 163) is
  DEC-305's ruling, unchanged by this."
- **A question for the owner (DEC-265 form), before S083 re-lands.** *Why*: at
  the paste's weights with the guard the ply-2 case reports `mate -2` from
  iteration 6 to 8 and is still red at `REQUIRE( first_exact == 4 )`; the
  remaining delay is late move reduction of the late-ordered mating quiet at
  the PV node at ply 1 (13th of 13 under the paste's history), which no
  mate-hiding rule touches (`LmrBase` 0, `LmrDivisor` 2000 or `LmrPv` 2 each
  bring 4 back, deciding nothing). The test cannot be edited without the owner
  (TESTS). *Options*: (a) keep the case as written -- S083 cannot land these
  weights until a later change orders or reduces that move differently; (b)
  amend the first-iteration clause to "the mate by the last iteration" for this
  defender-root case, as the new S289 case asserts (the S145 attacker-root
  case keeps its first-iteration clause for mates in two) -- a test edit, the
  owner's, which would let S083 re-land with the case green at 6; (c) treat the
  reduction of a late quiet at a shallow PV node as a defect -- no evidence it
  is one, and a play-altering change of its own. Recommend (b): the clause was
  written to kill M06a (RFP's floor at 2), which the attacker-root case and the
  new case also hold, and what the paste shows is a one-reduction delay, not a
  hidden mate.
- **DEV_MANUAL.md**: written in this change (the goldens table's rows and the
  mate-safety section's instrument 5). **MANUAL.md**: checked, no change -- no
  option, default or output moves. **README.md**: not touched (the owner's).

### 8. Deviations from the brief

- The ply-2 case is **not green at the paste's constants** with the fix (first
  mate at 6, not 4); the reason and the owner's question are in section 7. It
  is green at the shipped constants, as before.
- The guard tests were observed red at the **shipped** constants (the first
  draft on `4a3a0e5`, the final form under NM01 on F3), which section 3 made
  the brief's case; the paste's red is the ply-2 case's and is recorded in
  section 1.
- The mutation pass ran a subset (20 of 220): the four new mutants and those
  whose cases the guard could mask.
- Fixtures F1, F2, F3 and P are commit objects on no branch, not commits;
  nothing moved a ref.
- After the coordinator's fast check (2026-10-22): wording that claimed the
  table's bound covers every legal move, that the guard cannot move anything
  elsewhere, and that a position is a zugzwang, corrected to what the tools
  show; the exact legs and NM04 added; the red re-taken on the final cases;
  instrument 4 re-swept (`adocs/data/S289_defender_sweep.txt`: 77 -> 82, m2
  25/31 -> 31/31, m3 2/15 -> 1/15) and DEV_MANUAL's line rewritten from it;
  the census driver committed (`adocs/data/S289_census.py`); the SPRT script
  checks REF's src/ against `4a3a0e5`'s and says CAND is the fix commit
  itself; section 2's row 131 corrected (none of the 15 losses comes back).
