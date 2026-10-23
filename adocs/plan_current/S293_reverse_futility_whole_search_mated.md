id:         S293
goal:       close the defect S291 found -- reverse futility failing high at a node whose entry is a mated upper bound or exact score from a node that skipped no move, DEC-306's shape -- as its own measured change, with S292's condition reused, or show by measurement that no guard of that kind is correct
accepts:    mined row 171's loss under S291's scratch guard (`adocs/data/S291_reach.txt`: m5 -> cp 2104) traced from the engine's own output to its node and rule before anything ships; if the guard hides a mate the shipped tree reports and no narrower form avoids it, stop with the figures for the coordinator; otherwise a case observed red first on the shipped tree -- a node driven so its entry holds S292's mated whole-search bound and reverse futility's margin clears beta, S191's probe `rfp_cutoff` false once guarded -- and green after; a mutant it kills (DEC-141); both builds green; bench and `tools/search_bench.py` d9/d12; every moved golden by its script inside DEC-299's guard; gate_extra; the Debug self-play command written for the coordinator; S291's four mate sets and its census re-read with the shipped form; implemented from published descriptions, each named and quoted (DEC-221): reverse futility as static null-move pruning, and the null move's premise in David-Tabibi and Netanyahu, ICGA J. 25(3), 2002; a `{-5, 0}` SPRT against its parent pre-registered in `adocs/data/S293_sprt.sh` in S292_sprt.sh's shape, with the worst-case expected games from the nElo formula, the abort rule, the three outcomes (H0 reverting it whole), and the expectation that with 0 ordinary-play firings at depth 10 the run sits near a true zero and may reach its cap; the fast suite green in both builds
touches:    src/search.cpp (reverse futility's guard: `whole_search_mated_entry` and the conjunct in the condition, the guard list's pointer to it); tests/test_search.cpp (one case); tests/test_mate_breadth.cpp (`EXACT_FLOOR`'s comment: the shipping end 164 -> 163, the floor held); tests/test_mate_carry.cpp (the ceilings' command gains S293's grid, comments only); adocs/data/S170_cases.tsv (C's budget by the rule, and its header); tools/mutants/S293_reverse_futility_whole_search_guard.py (RW01-RW05) and two anchors re-pointed in tools/mutants/S191_guards.py (N05, N06); adocs/data/S293_* (S293_trace.py, never shipped; S293_row171.txt, S293_red.log, S293_reach.txt, S293_goldens.txt, S293_sweep_s170.txt, S293_mutation.tsv, S293_sprt.sh); adocs/data/README.md (S293's rows); DEV_MANUAL.md (the defender set's paragraph, the `EXACT_FLOOR` and `short_line_ceiling` goldens rows); MANUAL.md (the `RfpMinPly` row's mined-set figures, the reverse futility Known-bugs entry); this file
excludes:   reverse futility's margins and depth ceiling (DEC-158, S127); the null move and DEC-310's guard; the returns on bounds from nodes that skipped a move (not a proof, DEC-310); S083's constants
decisions:  DEC-311, DEC-310, DEC-306, DEC-171, DEC-158, DEC-143, DEC-221, DEC-141, DEC-142, DEC-299
closes:
blocks:     S083
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:

## Why this step exists (the coordinator, 2026-10-23, DEC-311)

S291 found reverse futility returning a fail-high, with beta outside the
mate band, at nodes whose entry is a mated upper bound or exact score stored
by a node that skipped no move. That is the comparison DEC-306 read as a
zugzwang for the null move, and DEC-310 guarded the null move against it.
DEC-158 priced reverse futility only where nothing is proved. S291 counted
90 such returns over S289's four mate sets (45, 2, 13, 30) and none over
11503 ordinary-play positions at depth 10. Over the mate sets its scratch
guard moved eight reported scores and two best moves. S145 root 76, a
proved mate in 5 for the side to move, reports cp 0 with b7c7 shipped and
cp 529 with b7b2 guarded. It gained and lost no exact mate in S145's 186 defender
nodes, and it lost mined row 171 (m5 -> cp 2104)
(`adocs/data/S291_reach.txt`, `S291_census.txt`, S291 section 5).

DEC-311 reads it as BUGS-binding under DEC-171 (c): the defect moves
reported scores and best moves, and (c) carries no ordinary-play limit. So
it is fixed before S083's run, and S083 is paused by it. The scratch
guard's loss of row 171 says the obvious form is not yet known to be
correct. Tracing that loss comes first.

## 2026-10-23: row 171 traced, the guard shipped with S292's condition (the implementer)

One Opus 5.5 subagent, in `../chesso-s293` (branch `s293` at `6ebeadc`), with
one scratch worktree, `../chesso-s293t`: first the trace build (`6ebeadc` plus
`adocs/data/S289_trace.py`, `S291_trace.py` and the new `S293_trace.py`, never
shipped; blob `84c79c8` before the third patch, S291's build-tune-ship), then
the parent's own Release and tune builds, then the mutation fixture. Nothing
committed; the fixtures are commit objects on no branch, made through a
temporary index (no ref moved): `052c173` (the goldens, `--ref`) and `8f02693`
(the mutation pass; the same `src/`, with C's re-derived budget and the
records). The engine's output, python-chess's move application and the sets'
own oracles only; nothing read off a board.

### 1. Row 171: the node, the rule, the kind (`adocs/data/S293_row171.txt`)

Through S289_rows.py's protocol (UCI, Hash 16, `ucinewgame`, `go depth 10`):
shipped m5 at 10, scratch guard cp 2104 at 10 and m5 from 11, best move c4f4
both.

- **The guard's node.** The shape occurs **once** in the shipped search
  (`$S291_CENSUS`, with `S293_trace.py`'s path line): iteration 9, ply 7, White
  to move, line a4a3 d6d3 a3d3 e3f2 c4f4 f2e1 d3d2, depth 3, window
  [-2117, -2116]; its entry an upper bound -48991 at depth 2 from a whole search
  that the same node stored earlier in the iteration. Shipped, reverse futility
  returns -2112 (static -1923, margin 189). Guarded, the node searches b5b6,
  gets -48991 and stores that upper bound at depth 3: it **re-proves its own
  mate**, and its ply-4 ancestor then reads c4f4 as a mate (48991) where the
  shipped tree read 2112. The guard at that key alone (`$S293_GUARD_KEYS`)
  reproduces cp 2104.
- **The node and rule that lose the mate.** At depth 10 the changed table
  sends the root's final pass down c4f4 first at the full window; e3d3 at ply 1
  is not extended (shipped, its verification at depth 4 found e3e2 mated and it
  was); at ply 2 (depth 8) f4d6 returns 2104, so the mating a4a2 is probed with
  a zero window, and at **ply 3, `8/5p1k/3R2p1/pP6/5q2/3K4/r7/8 w - - 4 59`
  (line c4f4 e3d3 a4a2), White to move, a non-PV cut node, depth 7, window
  [-2105, -2104], reverse futility returns -1809** (static -1368, margin 441)
  with **no table entry at all**. Shipped, f4d6 had already returned 48989 at
  that node's parent, so a4a2's node met a mate-band beta and reverse futility's
  band guard kept it out. One node at a time: guard on and reverse futility
  removed at that ply-3 node alone (`$S291_NO_RFP`), m5 at depth 10; the same
  removal on the shipped tree, m5 too.
- **The kind.** The guard does not hide the mate: it re-proves its own node and
  exposes a tree in which reverse futility, where nothing is proved, hides it --
  **DEC-158's priced kind** (remaining depth 7 under the ceiling 15; S291's
  attribution for the multicut row, DEC-311 (1)), not a further table proof
  outvoted. `RfpMaxDepth` 4 (DEC-158's challenger) brings the row back under the
  guard, and so does every other tune option tried one at a time (`RfpMinPly`
  63, `RfpMaxDepth` 0, `NullMoveBase` 16, `SeExtend` 0, `SeMultiCut` 0,
  `LmrBase` 0, `ProbCut` 0, `RfpTtEstimate` 0, `RazorDepth` 0): one cold
  search's tree shape at one depth.

### 2. The form and its source (step 2)

The loss is not the guard's condition, so the step goes on with S292's
condition reused (DEC-311's form). **No narrower form keeps the row and closes
the defect**: the guard at the one node of the shape alone reproduces the loss,
so any guard that stops that return loses the row at depth 10, and one that
does not leaves that return of the defect in the tree.

**The rule.** `whole_search_mated_entry` in `negamax_at`, computed from the
copies taken before anything recursed -- the entry an upper bound or an exact
score at or below `-MATE_MIN` with its `no_move_skipped` byte set -- is one more
conjunct of reverse futility's condition. No constant, no option. The null-move
block and DEC-310's guard are untouched; its expression is S292's, under its
own name.

**The sources, DEC-221.** The Chess Programming Wiki, "Reverse Futility
Pruning" (https://www.chessprogramming.org/Reverse_Futility_Pruning, read
2026-10-23): "Reverse Futility Pruning, (RFP, Static Null Move Pruning)";
"Thus, Reverse Futility Pruning relies on the null move observation"; "or a
special case of null move pruning without explicitly making one". Omid
David-Tabibi and Nathan S. Netanyahu, "Verified Null-Move Pruning", ICGA Journal
25(3), 2002, as S292 and DEC-310 quoted it -- section 2: "the value of the best
(legal) move has to be better than that obtained from the null move"; section
3: "If the null-move search indicates a cutoff, but the search shows that the
best value is smaller than β, this implies that the position is a zugzwang".
The rule rests on the null move's premise, the best move's value, and a whole
search's mated bound is that value found below beta. No engine's code or
constant read.

### 3. The change and its proof (step 3)

- **Red first** (`adocs/data/S293_red.log`): `tests/test_search.cpp`'s
  `reverse futility does not fire at a node a whole search found mated` on
  `6ebeadc`'s `src/` (the byte present, the guard absent): both guarded legs
  red on all 104 rows of `S165_defender_set.tsv`, the five premise legs clean;
  green in both builds with the guard. Seven seeds a row, S292's case's form: a
  beta the static score less the margin clears by equality, depth 3 against
  seeds at depth 2, the two out-of-band seeds at the static score so S234's
  estimate leaves the margin's number unmoved; `rfp_cutoff` false only on the
  mated upper bound and exact score of a whole search, and every uncut drive
  reaching its move loop.
- **Mutants** (`tools/mutants/S293_reverse_futility_whole_search_guard.py`, RW
  a new prefix): RW01 dropped, RW02 a pruned bound trusted, RW03 any bound type,
  RW04 the band dropped, RW05 the exact disjunct dropped. S191's N05 and N06
  were anchored on the condition's last line and are re-pointed, their
  mutations unchanged. The whole catalogue validates, 231 mutants.
  **41 of 41 killed** in 4675 s (`adocs/data/S293_mutation.tsv`, fixture
  `8f02693`): RW01-RW05, each by the new case (RW03 also by `pruning does not
  hide a forced mate`, RW04 also by the mined floor); S191's N03-N06, N05 and
  N06 on their re-pointed anchors; M05, M06a, M06b and E11; the estimate's
  G01-G04 (G03 now also by the new case); S292's NF01-NF09; and S289's sixteen
  whose cases a table guard could mask (M01-M04, M06a, N01, N02, E21, B04,
  J01, J02, R02, C02, C05, LQ02, LQ03).
- **Bench 6042933 -> 6042933**, the eight replies identical;
  `tools/search_bench.py` at 9 and 12 node- and move-identical to the parent's
  Release build (`adocs/data/S293_goldens.txt`). A functional change all the
  same (the mate sets and S170's grid move), so the commit line is `Bench:
  6042933`, not `No functional change`.
- **The four sets and the census re-read with the shipped form**
  (`adocs/data/S293_reach.txt`): the candidate's output files equal S291's
  scratch guard's byte for byte over all four sets -- 40, 83, 143 and 163 exact
  against the parent's 40, 83, 143 and 164; eight scores and two best moves
  moved, the rows S291 listed; no exact mate gained, mined row 171 lost. The
  census (11503 S219 A/A positions at depth 10, Release against Release): the
  same 981078145 nodes, 0 best moves, 0 scores, 0 node counts moved.
- **Goldens** (`adocs/data/S293_goldens.txt`, each by its script, DEC-299's
  guard met): `MATE_IN_THREE_FLOOR` 12 shipping / 7 weakened, gate red (11
  stays); **`EXACT_FLOOR` 163 shipping / 160 weakened** (164 / 160 before), gate
  at `RfpMinPly` 1 red -- 163 holds inside the script's band, now at the
  shipping count, its spare point gone, since 162 would be a falling floor
  (`tests/test_mate_breadth.cpp`'s comment says so); first-mate 9 and 9 at mate
  5; S192's 13446; the four table-less mined rows identical but for the
  milliseconds column; instrument 4's defender sweep 82 of 104 both. **S170's
  grid moves** (`adocs/data/S293_sweep_s170.txt`, 108 cells, 1420 mate lines, 26
  short; the parent's grid is S292's cell for cell): C at 500000, 1000000 and
  1200000 now reports no mate line, so the rule moves C's budget **500000 ->
  1500000** (42 lines, 1 short), applied by `pick_budgets.sh`'s output with a
  header paragraph; `--ceilings` over the five grids answers 2, 5, 4, 0, 1, 2,
  none moving, and the grid joins the command in `tests/test_mate_carry.cpp`.
- **Gates.** The fast suite **50 of 50 in both builds**, format clean
  (`CLANG_FORMAT_MAJOR=22`, DEC-146), both builds benching 6042933, after the
  last code, test and budget edit (`.tuning/coord/S293/gate_final.log`).
  `tools/gate_extra.sh` **five stages green in 1031 s** (prose, citations, the
  Debug binaries' fast suite 380 s, the sanitizer build 599 s, deep perft 52
  s; `.tuning/coord/S293/gate_extra.log`). `plan_prose_check.py` `--prose`,
  `--citations`, `--touches`, `--params` and `--gate` flag nothing. Debug
  self-play is the coordinator's (below).
- **Debug self-play** (DEC-141 (1)), for the coordinator on the committed tree:
  `.tuning/coord/S293/debug_selfplay.sh` -- S292's script with this step's
  paths: `build-debug`, `fastchess` four rounds `-repeat` at 4+0.04, Hash 16,
  `noob_3moves.epd`, trace log with engine stderr, then `grep -c Assertion` and
  `grep -c disconnect`; prints `DEBUG-SELFPLAY-DONE` last.
- **The SPRT**: `adocs/data/S293_sprt.sh`, S292_sprt.sh's shape, refusing to
  start while the coordinator's bracketed lines stand: CAND S293's commit, REF
  its parent, 8+0.08, Hash 16, `noob_3moves.epd`, `{-5, 0}` nElo at alpha = beta
  = 0.05; 25591 games (12.1 h at 2110 an hour) with a true zero, 41861 (19.8 h)
  at the midpoint, the worst case; cap 20950 rounds, abort at 40 h, the forfeit
  and crash rules. Expected near a true zero, and with no firing in ordinary
  play at depth 10 it may run to its cap. H1 or no verdict: the guard stays, S083
  re-lands on this tree. H0: inspect; if it stands, revert whole and the owner
  rules. Its guards check that CAND changes `src/search.cpp` and nothing else
  under `src/`, and that REF's `src/` is `e27d3e5`'s.

### 4. Proposed, for the coordinator

- **specs.md**, the search row, after the S234 reverse futility sentence:
  "**Reverse futility is not tried at a node a whole search found mated, since
  S293 (DEC-311)**: where the node's entry is an upper bound or an exact score at
  or below `-MATE_MIN` written by a node at which no shallow-depth rule skipped a
  move (the entry's `no_move_skipped`, S292's condition), the margin is not
  tried. The rule is the null move's observation without the null move ("a
  special case of null move pruning without explicitly making one", the Chess
  Programming Wiki), whose premise is the best move's value (David-Tabibi and
  Netanyahu, ICGA Journal 25(3), 2002), and the table holds a search of every
  move that found the side to move mated. S291 found the rule failing high there
  90 times over S289's four mate sets and never in 11503 ordinary-play positions
  at depth 10; without those returns eight reported scores and two best moves
  move, no exact mate is gained, and mined row 171's mate in 5 at depth 10 is
  lost to reverse futility at another node with no entry, DEC-158's price
  (`adocs/data/S291_reach.txt`, `adocs/data/S293_row171.txt`,
  `adocs/data/S293_reach.txt`). Held by `tests/test_search.cpp`'s `reverse
  futility does not fire at a node a whole search found mated`, five mutants
  killed (`tools/mutants/S293_reverse_futility_whole_search_guard.py`). A cut
  where no whole search has found the node mated stays DEC-158's price." And in
  the mate-breadth sentence: "(163, its ends **163 shipping** and 160 weakened
  since S293, re-read by S156's script, `adocs/data/S293_goldens.txt`; 164 and
  160 at S286 ...)".
- **DEC text** (proposed): "DEC-3nn S293's reverse futility table guard ships
  as a correctness fix, decided by a `{-5, 0}` SPRT. Context: DEC-311 (2); S293
  traced the scratch guard's loss of mined row 171 to reverse futility at another
  node of the changed tree, with no table entry (DEC-158's price), the guard's
  own node re-proving its mate. Decision: no reverse futility where the entry is
  a whole search's mated upper bound or exact score (S292's condition); from the
  Chess Programming Wiki's static-null-move description and David-Tabibi and
  Netanyahu 2002 (DEC-221). Costs `EXACT_FLOOR`'s spare point (shipping 164 ->
  163, the floor holding at 163) and moves S170's C budget 500000 -> 1500000 by
  the rule. Rejected: a narrower form (none keeps row 171 and stops the shape's
  return at its node); lowering `EXACT_FLOOR` (a falling floor, DEC-299).
  Consequences: S083 re-lands on this tree on H1 or no verdict."
- **DEV_MANUAL.md**: written (the defender set's paragraph for this case; the
  `EXACT_FLOOR` and `short_line_ceiling` goldens rows, whose command now also
  names S292's grid, missing there since S292). **MANUAL.md**: written -- the
  `RfpMinPly` row's mined-set figures (163, 166, 160) and one sentence in the
  reverse futility Known-bugs entry saying where the rule is not tried since
  S293. No option, default or output moves. **README.md**: not touched.

### 5. Deviations from the brief

- `touches:` widened past the step file's list: `tests/test_mate_breadth.cpp`
  (`EXACT_FLOOR`'s comment, the shipping end moved), `tests/test_mate_carry.cpp`
  (the ceilings' command gains S293's grid, comments only),
  `adocs/data/S170_cases.tsv` (C's budget by the rule, and its header),
  `tools/mutants/S191_guards.py` (N05 and N06 re-pointed), `adocs/data/README.md`
  (S293's rows).
- Two fixtures (commit objects on no branch, as S292's), because C's budget
  moved after the goldens chain: `052c173` and `8f02693` share `src/` and differ
  in `adocs/`, `MANUAL.md` and test comments plus the TSV's budget.
- Disk: `../chesso-s293` 1.7 GB (Release, tune, and gate_extra's Debug 351 MB
  and sanitizer 948 MB builds), `../chesso-s293t` 352 MB (the trace and
  parent builds and the mutation build, now detached at `8f02693`), the
  scratchpad 70 MB -- about 2.1 GB; the S154 and S156 scripts' temporary
  worktrees under `/tmp` were removed by them. 6.1 GB free after.
