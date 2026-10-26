id:         S296
goal:       a node that skipped moves by a shallow-depth pruning rule no longer returns a mated score it did not prove, so the root never reports a mate shorter than the position's; one form chosen by measurement, decided by a {-5, 0} SPRT
accepts:    S295's chain (`adocs/data/S295_trace.txt`, S145 mined line 168) re-taken on the fixed tree: depth 8 no longer reports `mate 4` (the oracle's mate in 5, Stockfish depth 20), read from the engine's output; both forms of DEC-318, (a) a mated fail-low after a skip returns a bound that does not claim the mate and (b) the move-loop's shallow-depth pruning is not applied while no searched move has escaped the mated band, implemented from their descriptions (DEC-221) and measured before one is chosen -- exact mates gained and lost on S145's set, S289's four mate sets, the S170 cells and S294's grid, the census of printed mate distances shorter than the oracle's on those sets, and S292's ordinary-play census (firings, root moves changed, nodes); if neither form loses no mate the shipped tree reports, stop with both forms' figures for the coordinator; a case observed red first on the shipped tree and green after; a mutant it kills (DEC-141, new pruning-adjacent rule); every golden it moves re-derived by its script (DEC-142), the mate floors and S170 ceilings under DEC-299's guard; Tier 2 (DEC-141) run; S295's node-limited cell in `tests/test_mate_pv.cpp` (the `go nodes 140000` cell on S145 mined line 168, picked by `adocs/data/S295_long_line.py`) exists only because of the short `mate 4`, so it is re-derived by its script on the fixed tree or, if no cell separates, retired by a recorded decision, the cut's guard kept by the constructed-lines case (MW03's other killer); both builds and both fast suites green; then the coordinator's `{-5, 0}` SPRT at 8+0.08, Hash 16, against its parent, pre-registered with its three outcomes
touches:    src/search.cpp (the move loop's end: `node_value`, alpha0 at a mated fail-low after a skip, stored and returned; the probe's write); src/data_structures.hpp (`search_node_probe_t::loop_ended`, `loop_best`, test instrumentation); tests/test_search.cpp (one case, `<algorithm>`); tests/test_mate_pv.cpp (S294's cell re-picked, S295's node-limited case retired, the header); tests/test_mate_breadth.cpp (`EXACT_FLOOR` 158 -> 159 and its comment); tests/test_engine.cpp (`MATE_IN_THREE_FLOOR`'s comment); tests/test_mate_carry.cpp (the ceilings' command gains S296's grid, comments only); adocs/data/S170_cases.tsv (A's and C's budgets by the rule, and its header); tools/mutants/S296_skipped_mated_fail_low.py (SK01-SK02); tools/mutants/S097_singular_extension.py (E09 re-pointed); tools/mutants/S202_mate_walk.py (MW02's and MW03's killers, docstring only); adocs/data/S296_* (forms_patch.py, oracle.py, oracle.tsv, compare.py, census.py, forms.txt, root76.txt, red.log, goldens.txt, sweep_s170.txt, mutation.tsv, sprt.sh); adocs/data/README.md (twelve rows); MANUAL.md (the Known bugs entry and the `depth` row); DEV_MANUAL.md (the defender set's paragraph, the `test_mate_pv` paragraph, the goldens rows); this file
excludes:   reverse futility's margins and ceiling (DEC-158); DEC-310's null-move guard; the reporting layer (S295); the S170 budgets except by S170_pick_budgets.sh under DEC-299's guard
decisions:  DEC-318, DEC-317, DEC-310, DEC-171, DEC-158, DEC-221, DEC-141, DEC-142, DEC-299, DEC-143
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:

## Why this step exists (the coordinator, 2026-10-26, DEC-318)

S295 traced the completed iteration's short mate on S294's cell. A non-PV
node at ply 5, depth 3, searched nine replies, all mated, skipped six by
quiet SEE pruning and late move pruning -- among them g2g3 and g2g4, the
only two replies python-chess finds avoid mate in one -- and returned its
fail-soft mated best as its bound. The table carried that bound up to the
root, which reported `mate 4` where Stockfish reads mate in 5. Returning
alpha there in a scratch build restores mate 5 at depth 8. It moves a
reported score, so it is BUGS-binding under DEC-171 (c) and goes before
S136. It changes play, so it is measured alone and decided by an SPRT, as
DEC-310's null-move guard was.

## 2026-10-26: both forms measured; stopped, neither keeps every mate the shipped tree reports (the implementer)

One Opus 5.5 subagent, worktree `../chesso-s296` (branch `s296` at `b477459`,
bench 5184358). Three throwaway worktrees under `.tuning/s296/` (`f_ship`,
`f_a`, `f_b`, detached at `b477459`, removed at the end), each rewritten by
`adocs/data/S296_forms_patch.py` and built Release, `--target chesso` (and a
tune build of `f_ship` and `f_a` for one reading). Nothing in `src/` or
`tests/`; no commit object made. `adocs/data/S296_forms.txt` is the reading,
with every command.

**The forms, from DEC-318's descriptions (DEC-221).** (a) at the move loop's
end, a node where a shallow-depth rule skipped a move (S292's `move_skipped`)
and whose best searched score is at or below `-MATE_MIN` stores and returns
`alpha0`; the pawn correction still sees `best_so_far` and so still refuses
the mated score. **`alpha0`, not the band's edge:** each skipped move was
skipped on the premise that it scores no better than alpha, so the premises
bound the node by `alpha0` and by nothing lower; the edge is a value no search
returned, and one point short of the band it could surface as a cp score.
(b) `may_prune`, the one eligibility all five move-loop rules read --
futility, history pruning, quiet SEE pruning, capture SEE pruning and late
move pruning (its flag and its skip) -- also requires `best_so_far >
-MATE_MIN`: no rule fires while every searched move is mated. Both forms
cover the verification search and the probed node, which share the code.

**The oracle.** Stockfish through python-chess at **5000000 nodes**
(`adocs/data/S296_oracle.py`, cache `S296_oracle.tsv`, 610 positions), not
`go depth 20`: on these positions depth 20 stops after a few thousand nodes
on the first mate found (S145 mined line 96: `#+10` at depth 20, `#+4` at
depth 30 and at 200000 and 2000000 nodes) and disagreed with S145's and
S165's proved distances on 50 of 372 rows; the node-limited run agrees with
all 372. Outside the proved sets it is an oracle, not a proof.

**The table** (`S296_forms.txt` section 1):

| | shipped | form (a) | form (b) |
|---|---|---|---|
| S295's chain, line 168, `go depth 8` (oracle mate 5) | `mate 4`, 61914 nodes | `mate 5`, 74605 nodes | `cp 2200`, 5970 nodes; `mate 5` at 9 |
| exact, S145's 82 roots (m3) | 41 (13) | 41 (13), +1 -1 | 41 (13), +0 -0 |
| exact, S165's 104 / S145's 186 defender nodes | 83 / 143 | 83 / 143, +0 -0 | 83 / 143, +0 -0 |
| exact, S145's 318 mined rows, depth 10 (`EXACT_FLOOR` 158) | 160 | 160, +3 -3 | **156**, +3 -7 |
| exact, S294's mined grid, 12402 searches | 5208 | 5248, +94 -54 | 5145, +98 -161 |
| exact, the S170 grid | 121 | 137, +47 -31 | 148, +56 -29 |
| mates shorter than the oracle's: four sets / S294's grid / S170 grid | 2 iterations / 16 searches (82 lines) / 28 (319) | 0 / 0 / 0 | 0 / 0 / 0 |
| census, 11503 positions at depth 10: firings (searches) | the shapes 4993 (1532) and 126277 nodes (6648) | 5715 (1532) | 127592 nodes (6648) |
| root best moves / scores / node counts changed | -- | 189 / 504 / 954 | 450 / 1358 / 5074 |
| census nodes | 1013553757 | x1.000063 | x1.000767 |
| `chesso bench` | 5184358 | 5069022 | 4723001 |

The shipped tree's short mates are three positions: line 168 (depth 8, mate
4 against 5) and line 226 (depth 9, mate 5 against 6) of the mined set, and
case B of the S170 grid (every short line one move short). Both forms remove
every one.

**Why it stops.** DEC-318's consequence and the brief: if neither form clears
the mate sets without losing a mate the shipped tree reports, stop with both
forms' figures. Neither does:

- **Form (a)** loses S145 root 76 (proved mate in 5, `K3k3/1r6/8/8/8/p2p1p1p/
  P2P1P1P/4BRBR b`): the shipped tree reads m5 from depth 16, form (a) has no
  mate through depth 21 (cp 988). One tune option at a time on its tune build
  brings it back, `RfpMinPly` 63 from depth 13 (the shipped tree: 16, and 17
  with reverse futility off), and so do `RfpMaxDepth` 4, `NullMoveBase` 16,
  `SeMultiCut` 0, `ProbCut` 0, `RazorDepth` 0 and others; the shipped tree
  itself loses it under `SeExtend` 0, `NullMoveBase` 16 or
  `FutMaxLmrDepth` 0. One cold search's tree shape, as S293 found row 171;
  not traced further. Its three mined losses (lines 98, 170, 176) read the
  mate one or two iterations later (11, 12, 11), its 54 lost cells on S294's
  grid (10 positions) are all exact again at a larger budget, and 22 of its
  31 lost S170 cells are. It gains root 50 and three mined rows (each one or
  two iterations earlier than shipped), 94 grid cells and 47 S170 cells.
- **Form (b)** loses seven mined rows at depth 10, so the mined set reads 156
  against `EXACT_FLOOR` 158 -- a floor that falls, the owner's (DEC-313,
  DEC-299) -- and 161 cells of S294's grid, 84 of them never exact again on
  the grid. It keeps root 76 (m5 from 17).

**The ordinary-play root moves, from the engine's output**
(`S296_census.py read`): every search whose root answer moved is one where the
form fired in that search (0 moved elsewhere). Form (a): 189 best moves at a
median score change of 11 cp (p90 48, max 852), 1532 searches touched. Form
(b): 450, median 13 cp (p90 42), 6648 touched. The shape is common in ordinary
play -- 4993 mated fail-lows after a skip in 13 % of the searches -- which is
why a fix moves this many; S292's census found such bounds refuted by the
node's own search four times in five (DEC-310).

**Recommendation, for the coordinator's ruling:** form (a). It clears both
floors (mined 160, m3 13), its set losses are one row lost through depth 21
that reverse futility off restores (DEC-158's kind, DEC-312's reading of row
171) and three mined rows one or two iterations late against three one or
two early, its
grid losses all come back at a larger budget, it moves fewer ordinary-play
root moves than (b) and costs 0.006 % nodes, and S295's chain reads the
oracle's mate 5 at depth 8. Form (b) costs `EXACT_FLOOR` and more.

**Ready if a form is ruled in, not landed (excluded by the stop):** the
direct case -- S295's ply-5 node `6rk/5rqp/p7/2P1p3/P7/8/1P1n2P1/R5K1 w - - 0
35` (python-chess: 15 legal moves, only g2g3 and g2g4 leave Black no mate in
one; Stockfish `#-2`, `S295_trace.txt`), `negamax_probed` at ply 5, depth 3,
window [-2108, -2107], non-PV, a wiped table -- returns -48993 on the shipped
tree (mated in one, with g2g3 and g2g4 pruned by quiet SEE), -2108 under (a)
and -2145 under (b), observed in scratch builds (`S296_forms.txt` section 6);
its property, "the bound is not below mated in two", needs only the engine's
generator for its precondition. S295's node-limited cell (`go nodes 140000`
on line 168) prints `mate 5` with the 9-ply completed line under both forms,
so its precondition (`aborted.pv_length > needed`) is to be re-derived by
`S295_long_line.py` or the cell retired by a decision. No mutant, no goldens,
no SPRT script were written; the harness is unchanged since S284
(`fastchess.sh`, 2026-10-15), so an A/A would not be owed for the run.

**Gates.** No `src/` or `tests/` change. On the handed-over worktree:
`build/` rebuilt, 0 warnings; fast label **50 of 50** (102 s, with
`CLANG_FORMAT_MAJOR=22`, its prose and citation cases reading this file);
`./clang-format.sh --check` clean; `tools/plan_prose_check.py`
`--citations`, `--touches`, `--params` and `--gate`, one mode per call, each
exit 0. `build-tune/` was not built (no code changed). No Tier 2: nothing
in the search changed. Disk: `.tuning/s296/` 0.8 GB at its peak (three throwaway worktrees
with builds), the worktree 0.33 GB.

### Proposed commit message for the stop (superseded by the section below)

```
Measure DEC-318's two forms; both lose a mate, stopped (S296)

A node that skipped moves returns a fail-soft mated bound the table
carries to the root as a mate shorter than the position's (S295,
DEC-318). Both forms, built throwaway and measured against the
shipped tree on S289's four mate sets, S294's grid, the S170 grid and
S292's ordinary-play census, remove every reported mate shorter than
the oracle's (Stockfish at 5M nodes, agreeing with all 372 proved
distances). Neither keeps every mate the shipped tree reports: (a),
alpha0 at a mated fail-low after a skip, loses S145 root 76 through
depth 21 (back from depth 13 with reverse futility off) and three
mined rows by an iteration; (b), no move-loop pruning while every
searched move is mated, drops the mined set to 156 under EXACT_FLOOR
158. So S296 stops for the coordinator with both forms' figures
(adocs/data/S296_forms.txt), recommending (a). DEC-318, DEC-221.
```

## 2026-10-26: root 76 traced, form (a) landed (the implementer)

**The coordinator's ruling** (under the owner's standing instruction, to be
DEC-319): form (a), returning `alpha0`, on condition of one trace of S145 root
76; form (b) rejected (`EXACT_FLOOR` 156 < 158, 161 grid cells lost); the
three choices of the measurement accepted (`alpha0` over the band's edge, (b)'s
five rules, the oracle at 5000000 nodes).

### 1. The trace: the mate is lost where (a) does not fire (`adocs/data/S296_root76.txt`)

Root 76 (`K3k3/1r6/8/8/8/p2p1p1p/P2P1P1P/4BRBR b - - 0 1`, a proved mate in
5; the oracle: b7c7, b7d7, b7e7, b7f7, b7g7 and b7h7 each `#-4`). Throwaway
builds `t_ship` and `t_a` (b477459, `S295_trace_patch.py` then
`S296_forms_patch.py --after-trace`, benches 5184358 and 5069022, the untraced
builds' own), every node of every iteration logged with `s296=` marking the
shape (form (a)'s firing in `t_a`).

- **Where it is lost.** In every iteration 13 to 21 of (a) the root scouts
  each of the six mates in 5 in an aspiration window outside the mate band,
  and at ply 1 the mated side's node -- non-PV, a cut node -- makes a **null
  move that fails high**: its reduced search's attacker node (static -1038
  to -1080 against alpha 633 at depth 17, 988 at 21) futility-prunes 12 to 17
  quiets, and the moves it searches are answered by **reverse futility** at
  ply 3 (static about 1040 to 1190, margin 567) or by the table. Every node of
  that chain has `s296=0`. The shipped tree does the same at depths 13 to 15
  and finds the mate at 16 only because its b7b2 line climbs through nine
  aspiration searches into a full-window one, where every later scout's window
  lies inside the mate band and neither the null move nor the move-loop rules
  are tried.
- **Where (a) fires.** 58 times over 21 iterations, 57 in b7b2's subtree and
  1 in b7b8's, none in the subtree of a mate in 5; each at a node that had
  skipped moves, so none a whole search's bound (S292's definition). Of the
  58 replaced mated bounds 39 were the node's true distance and 19 shorter
  than the oracle's. The two logs are byte-identical up to the first firing,
  iteration 7. A one-key counterfactual on the shipped tree (form (a) at one
  key only): at the ply-17 node whose false mated-in-2 (the oracle `#-15`)
  recurs through the shipped depth 16, the mate stays at 16; at all nine shape
  keys of the shipped search, mate 5 from 17. The mate does not rest on any
  one replaced bound.
- **The reading, against DEC-311's "a guard that hides a mate the shipped tree
  reports is not the fix"**, as DEC-312 (2) read it: (a)'s own return never
  touches the mating lines; the mate is lost to the null move where nothing is
  proved -- DEC-306 reading DEC-087 (e) as having "priced the zugzwang where
  nothing is proved" -- and to reverse futility, of which DEC-158 says "the
  mates the engine does not find ... are what the pruning costs". On (a)'s
  tune build `NullMoveBase` 16 restores it from depth 16 and `RfpMinPly` 63
  from depth 13. So it is those prices on the changed tree, and the landing
  went on.

### 2. The change

At the end of `negamax_at`'s move loop, before the store: where
`move_skipped` (S292's flag, set at the two skip sites) and `best_so_far <=
-MATE_MIN`, the node's value is `alpha0` (`node_value`), stored as the upper
bound it already was and returned; otherwise `best_so_far` as before. The
pawn correction still reads `best_so_far`, so it still refuses the mated
score. An `assert(alpha == alpha0 && alpha0 > -MATE_MIN)` states why only a
fail-low reaches it (the rules that skip read `alpha > -MATE_MIN`, and alpha
rises only on a searched move above it). Implemented from DEC-318's
description of form (a) (DEC-221): no constant, no option. The probe gains
`loop_ended` and `loop_best` (the loop's best before the rule), written only
in the probed instantiation, for the case. The landed engine reproduces the
throwaway (a) build: bench 5069022, S295's chain `mate 5` at depth 8 in 74605
nodes, and the four sets' `S289_rows.py` files byte for byte; S295's mined
sweep of it equals the throwaway's row for row.

### 3. The table (`adocs/data/S296_forms.txt`; the landed tree is column (a))

| | shipped | landed, form (a) |
|---|---|---|
| S295's chain, line 168, `go depth 8` (oracle mate 5) | `mate 4`, 61914 nodes | `mate 5`, 74605 nodes |
| exact, S145's 82 roots (m3) / S165's 104 / S145's 186 | 41 (13) / 83 / 143 | 41 (13), +1 -1 / 83 / 143 |
| exact, mined 318 at depth 10 | 160 | 160, +3 -3 |
| exact, S294's grid / the S170 grid | 5208 / 121 | 5248 (+94 -54) / 137 (+47 -31) |
| mates shorter than the oracle's: sets / S294's grid / S170 grid | 2 iterations / 16 searches / 28 | 0 / 0 / 0 |
| census, 11503 positions at depth 10 | -- | 5715 firings in 1532 searches; 189 root best moves, 504 scores, 954 node counts moved, all where it fired; nodes x1.000063 |
| `chesso bench` | 5184358 | **5069022** (the eight replies identical) |
| `tools/search_bench.py` 9 and 12 | 48284 / 126091 / 62646, 226320 / 430872 / 157130 | identical, best moves too |

**The extra reach** the measurement found: besides line 168, the shipped tree
reported a mate shorter than the oracle's at mined line 226
(`8/5k2/p6p/1p2r1P1/1P6/P1P1Bp1b/5P1P/6K1 b - - 0 36`, depth 9 `mate 5`
against 6; 35 grid lines) and on S170 case B, both strides (319 lines, each
one move short). The landed rule removes all three.

### 4. The case, red then green (`adocs/data/S296_red.log`)

`tests/test_search.cpp`, "search: pruning and reduction guards", **"a node
that skipped moves claims no mate it did not search"**: S295's ply-5 node
(`6rk/5rqp/p7/2P1p3/P7/8/1P1n2P1/R5K1 w - - 0 35`), `negamax_probed` at ply
5, depth 3, window [-2108, -2107], non-PV, a wiped table. It REQUIREs, through
the engine's own generator, 15 legal moves of which exactly g2g3 and g2g4
leave Black no mate in one (python-chess agrees; Stockfish `#-2`), then that
the probed node reached its loop, pruned both and that its loop's best is
mated (`loop_best <= -MATE_MIN`); it CHECKs the return `> -MATE_MIN` and `==
ALPHA`, the stored entry an upper bound outside the band with
`no_move_skipped` 0, and the engine's own instantiation returning the same.
Red on b477459 with only the probe's fields added (`-48993`, the entry
`-48998`, three checks); green with the rule. **Golden (DEC-142):** `ALPHA`
-2108, the node's own window in S295's trace; margin 21 below and 203 above
-- the shipped tree returns the mated best for alpha -2129 to -1905 (and
-1804 to -1761), every alpha from -47999 to 0 swept (`S296_forms.txt` section
6). DEV_MANUAL's goldens table has its row.

### 5. Mutants (`tools/mutants/S296_skipped_mated_fail_low.py`, `adocs/data/S296_mutation.tsv`)

SK01 (the rule dropped, the engine before S296) and SK02 (the skip condition
dropped: a whole search's mated bound replaced too). E09 re-pointed to
`normalize_score(node_value, ply)`, its mutation unchanged. The whole catalogue
validates, 238 mutants. `tools/mutation_check.py --only` SK01, SK02, E09,
S202's MW01-MW07, S292's NF01-NF09 and S293's RW01-RW05 on fixture `66f6858`
(the landed tree, a commit object on no branch), baseline green at 50 tests
and bench 5069022, wall 3298 s: **24 of 24 killed**. SK01 by the new case
alone (`CHECK( value > -MATE_MIN_LOCAL )` and two more), bench moved; SK02 by
`test_search` (the mate-distance cases), `test_engine`, `test_mate_carry`
and two timeouts; MW03, its S295 cell retired, by the constructed-lines case;
MW02 by S294's re-picked cell and the constructed-lines case; the NF and RW
mutants by their own cases as before. `CLANG_FORMAT_MAJOR=22` set for the run (the first attempt refused a
baseline red on `test_clang_format_script` without it).

### 6. Goldens (`adocs/data/S296_goldens.txt`; fixtures `c3ed9cd`, `8e24184`, `66f6858`, commit objects on no branch)

| golden | before -> after | by |
|---|---|---|
| `chesso bench` (the commit's `Bench:`) | 5184358 -> 5069022 | `build/src/chesso bench` |
| `test_mate_pv.cpp` S294's cell | line 162 at `go nodes 150000` -> **line 101 (game 3499) at `go nodes 40000`**, run 30000..60000, margin 10000 below and 20000 above; 4 of 318 separate | `adocs/data/S294_fallback_cell.py`, its four stages |
| `test_mate_pv.cpp` S295's cell | line 168 at `go nodes 140000` -> **no cell separates, 0 of 318: the case retired** (its history kept in the file's comment; the coordinator records it) | `adocs/data/S295_long_line.py`, its four stages |
| `EXACT_FLOOR` | 158 -> **159** (the coordinator's ruling, DEC-319 (5)): the ends 160 shipping and 159 weakened (156 before), so 158 no longer separated; 160, the only separating value, is a zero-margin floor (DEC-116, DEC-313 (2)), so the floor sits one below the shipping count, DEC-313 (1)'s rule, and **no longer separates `RfpMinPly` 1**. Read at fixture `9f72404`: the gate built at `RfpMinPly` 1 with 159 compiled in is green at 159 on this count and red on `MATE_IN_THREE_FLOOR` at 8 >= 11; the shipping build is green on both | `adocs/data/S156_mined_floor_sweep.py --ref <fixture> --floor N`; `S154_floor_margin_sweep.py red` |
| `MATE_IN_THREE_FLOOR` | 11, unmoved: 13 shipping, 8 weakened, the gate red; **since S296 the floor that carries the `RfpMinPly` 1 separation** (its comment in `tests/test_engine.cpp` says so) | `adocs/data/S154_floor_margin_sweep.py floor`, `red` |
| S170 ceilings | 2, 6, 4, 1, 2, 3 unmoved over eight grids (`S296_sweep_s170.txt` added; alone 0, 6, 0, 0, 0, 0) | `adocs/data/S203_case_sweep.sh --ceilings` |
| S170 budgets | A 300000 -> **500000** (12 lines), C 500000 -> **1500000** (3), the rest unmoved, none short | `adocs/data/S170_pick_budgets.sh` on the grid |
| first-mate, s207's depth, S192's count, the ply-2 case, LQ02/LQ03 | 9 and 9, 4, 7366, 6, 7 and 3: unmoved, the candidate's outputs the parent's | `S188_repair_goldens.py first-mate` and `s207`, `S192_node_budget.py`, `S083s_rfp_ply2.py`, `S261_mine_late_capture.py` |
| `capture_mates`, `mate_probcut_hides`, `mate_the_extra_ply_hides` | asserted depths all hold; the fourth capture row's depth-10 mate is gone (9 and 12 remain, 12 asserted) | `S230_mine_r01_row.py depths` |
| instrument 4 (DEV_MANUAL's defender sweep) | 85 of 104 both trees, the mates in two at delay 6 against 4 | `S165_nmp_defender_sweep.py sweep` |

DEC-299's guard is met: `MATE_IN_THREE_FLOOR` holds and `EXACT_FLOOR` rises,
no ceiling rises, so the budgets are applied (with a header paragraph in
`S170_cases.tsv`). `EXACT_FLOOR` keeps one spare point below the shipping
count and no longer catches `RfpMinPly` 1; `MATE_IN_THREE_FLOOR` does, 3
above the weakened end. (A first landing raised it to 160, the band's only
value; the coordinator ruled 159, DEC-319 (5).)

### 7. Documents

`MANUAL.md`: the Known bugs entry restated as closed since S296 (what the
cell prints now, the measured reach, root 76's price), the `depth` row's last
sentence with it. `DEV_MANUAL.md`: a paragraph on the rule and its case in
the defender set's section (with instrument 4's reading, the stale 82 named),
the `test_mate_pv` paragraph (one cell now, 707 lines at depth 8), the
goldens rows (S294's cell, S295's retired, the new `ALPHA`, `EXACT_FLOOR`,
the ceilings and budgets, the rows read). `adocs/data/README.md`: twelve S296
rows. `tests/test_mate_carry.cpp`'s comment and `S202_mate_walk.py`'s
docstring follow. `README.md`, `specs.md`, `plan.md`, `status.md`,
`decisions.md` not touched.

### 8. Gates

`CLANG_FORMAT_MAJOR=22`, `-j12`, on the final tree. `build/` and
`build-tune/` built with `-Werror`, no warning; fast label **50 of 50 in
both** (111 s and 113 s); `./clang-format.sh --check` clean;
`tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, `--gate`,
one mode per call, each exit 0; both builds bench 5069022. Tier 2 (DEC-141;
the rule is in the search): `tools/gate_extra.sh` **GATE-EXTRA-DONE 5 stages
1053 s** (prose 0 s, citations 0 s, debug 368 s, sanitize 633 s, perft 52 s,
all ok; `.tuning/s296/gate_extra/`), on the tree of the mutation fixture plus
documents; **Debug self-play**, S293's script with this worktree's paths
(`build-debug`, Debug, four rounds `-repeat` at 4+0.04, Hash 16,
`noob_3moves.epd` read from the main checkout, `-check-mate-pvs`, trace log
with engine stderr): 8 games (+2 -4 =2), **`Assertion` 0** in the trace log
and stdout, `disconnect` 0, `Incomplete mating PV` 0.

Disk: `.tuning/s296/` about 1.5 GB at its peak (three throwaway form
worktrees, two trace worktrees and their 1 GB of logs, two cell worktrees,
the mutation worktree), 56 MB left (tables, sweeps, the six trace drivers);
every throwaway worktree removed, the trace logs deleted (the patchers
re-take them); this worktree 0.8 GB (`build`, `build-tune`, `build-debug`;
`build-sanitize` removed after gate_extra).

### 9. The SPRT script

`adocs/data/S296_sprt.sh`, `S293_sprt.sh`'s shape: CAND S296's commit, REF its
parent (src/ checked equal to `c8f1744`'s, the shipped engine; CAND's src/
changes in `src/search.cpp` and `src/data_structures.hpp` only), 8+0.08, Hash
16, `noob_3moves.epd`, `{-5, 0}` nElo at alpha = beta = 0.05; 25591 games
(12.13 h at 2110 an hour) with the truth on a bound, 41861 (19.84 h) at the
midpoint, cap 20950 rounds, abort at 40 h, the forfeit, crash, power and banner
rules. H1 or no verdict: the rule stays, S296 completes. H0: inspect; if it
stands, revert whole and the owner rules. Open DEC-171 fillers: none. The
harness is unchanged since `769ce7a` (S284, 2026-10-15), so no A/A is owed.
The coordinator's bracketed lines (the pin, REF, CAND) are left unfilled: run,
it refuses with `SPRT-RUN-FAILED: ... still holds the coordinator's unfilled
marker`. Expected 12 to 20 h: a night.

### Proposed specs.md wording (the coordinator writes specs)

The known-bug sentence after the S295 guarantee becomes: "**Closed since S296
(DEC-318, DEC-319):** a completed iteration's exact mate score could be
shorter than the position's -- on S145 mined line 168 `mate 4` at depth 8
against mate in 5 -- from a non-PV node that skipped moves by quiet SEE or
late move pruning and returned its fail-soft mated best as its bound
(`adocs/data/S295_trace.txt`); the search row's S296 rule removes it."

In the search row, after the S293 sentence: "**A node that skipped moves
claims no mate it did not search, since S296 (DEC-318, DEC-319)**: at the end
of the move loop, where a shallow-depth rule skipped a move (`move_skipped`)
and every move the node searched came back at or below `-MATE_MIN`, the node
stores and returns `alpha0` -- an upper bound at its own window's edge, the
claim the skipped moves' premise makes -- in place of its fail-soft mated
best; the pawn correction still reads the mated best and refuses it. DEC-318's
form (a), chosen over form (b) (no move-loop pruning while every searched move
is mated), which dropped `EXACT_FLOOR` (`adocs/data/S296_forms.txt`). Over
S289's four mate sets, S294's grid and the S170 grid no reported mate is
shorter than Stockfish's at 5000000 nodes, against 2 iterations, 16 searches
and 28 before; over 11503 ordinary-play positions at depth 10 it fires 5715
times in 1532 searches and moves 189 root best moves; S145 root 76's mate in
5 is lost through depth 21, to the null move and reverse futility where the
rule does not fire (`adocs/data/S296_root76.txt`). Held by
`tests/test_search.cpp`'s `a node that skipped moves claims no mate it did
not search`, two mutants killed (`tools/mutants/S296_skipped_mated_fail_low.py`)."

In the mate-breadth sentence: "(159 since S296, DEC-319 (5): its ends **160
shipping and 159 weakened**, re-derived by S156's script, so the floor sits one
below the shipping count and no longer separates `RfpMinPly` 1, which
`MATE_IN_THREE_FLOOR` carries instead, 13 against 8,
`adocs/data/S296_goldens.txt`; 158 since S083 ...)". In the S294/S295 paragraph: "both guarded by
`test_mate_pv`, one per half of the aborted-line fallback (S294, S295)"
becomes "S294's half guarded by a `test_mate_pv` cell and S295's by the
constructed-lines case since S296 retired its cell with the short mate it
rested on".

### Proposed commit message

```
Claim no mate a node that skipped moves did not search (S296)

A non-PV node where quiet SEE or late move pruning skipped a move and
every searched move came back mated returned that fail-soft mated best
as its bound, and the table carried it to the root as a mate shorter
than the position's: S145 mined line 168 read mate 4 at depth 8 where
the position is mate in 5 (S295's trace, DEC-318). The skipped moves'
premise bounds the node by alpha and no lower, so it now stores and
returns alpha0 there (DEC-318's form (a), chosen over (b) by
measurement, DEC-319). No reported mate over S289's four mate sets,
S294's grid or the S170 grid is now shorter than Stockfish's; it fires
in 13 % of ordinary searches. S145 root 76's mate in 5 is lost through
depth 21 to the null move and reverse futility where the rule does not
fire (adocs/data/S296_root76.txt). A test_search case red first, SK01
and SK02 killed; S294's cell re-picked, S295's retired (no cell
separates), EXACT_FLOOR 158 -> 159 (DEC-319 (5): MATE_IN_THREE_FLOOR
now carries the RfpMinPly 1 separation) and S170's A and C budgets by
their scripts inside DEC-299's guard. A {-5, 0} SPRT decides it
(adocs/data/S296_sprt.sh). DEC-318, DEC-319, DEC-221, DEC-142,
DEC-299, DEC-141.

Bench: 5069022
```

### 2026-10-26: the coordinator's fast check -- one ruling, five nits (the implementer)

1. **`EXACT_FLOOR` 159**, DEC-319 (5): `tests/test_mate_breadth.cpp`'s S296
   paragraph rewritten, `tests/test_engine.cpp`'s `MATE_IN_THREE_FLOOR`
   comment says it carries the separation again, the DEV_MANUAL rows, the
   TSV header, the table above, the specs wording and the commit message
   follow. Both ends observed on fixture `9f72404` (a commit object on no
   branch, 159 compiled in): the gate built at `RfpMinPly` 1 reads 159, green
   on `EXACT_FLOOR` (S156's script); the same weakened gate is red on
   `MATE_IN_THREE_FLOOR` at 8 >= 11 (S154's `red`); the shipping build is
   green on both (`test_mate_breadth` 160 exact, `test_engine`'s mate-set
   case 3756 of 3756). `adocs/data/S296_goldens.txt` section 4.
2. DEV_MANUAL's 707 traced: `adocs/data/S296_goldens.txt` section 8, the two
   sets' MESSAGE counts on both builds (191 + 516 against 191 + 514).
3. The case's comment says what its own drive skips (SEE b2b3, g2g3, g2g4,
   a1b1, a1f1; late move pruning g1h1) beside the trace's; the rule's comment
   in `src/search.cpp` now says the traced node's skipped moves included the
   only two escapes, comment only.
4. The S296 grid is "on fixture `c3ed9cd` (b477459 plus the rule)" in
   `tests/test_mate_carry.cpp` and DEV_MANUAL's ceilings row, as the sweep's
   header says.
5. `adocs/data/S296_red.log` cites the CHECKs' current lines (9733, 9734,
   9739) and says the run printed 9728, 9729, 9734 before the comment moved
   them.

`src/` changed in comments only (`git diff` of `src/` since the mutation
fixture `66f6858`: comment lines alone), so neither the mutation pass nor
Tier 2 was re-run. Gates after these edits, `CLANG_FORMAT_MAJOR=22`: both
builds, no warning; both fast suites in full, 50 of 50 each; format clean; the
four prose modes exit 0; `chesso bench` 5069022 in both builds.

