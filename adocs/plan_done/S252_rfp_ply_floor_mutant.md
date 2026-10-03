id:         S252
goal:       mutant M06a (reverse futility's ply floor at 2) is killed by a fast-suite case again, or declared equivalent with a stated proof
accepts:    `tools/mutation_check.py --only M06a_rfp_ply_floor_minus1` on a fixture of the tree of the day reads killed, by a case that establishes its premise first (a node at ply 2 that reverse futility would prune and a mate or a count that separates), or the mutant is declared equivalent in `tools/mutants/search.py` with the reason it cannot be distinguished; S145's mate set and S156's sweep are the first places to look; both fast suites green; no assertion relaxed
touches:    tests/test_search.cpp or tests/test_engine.cpp (a case), tools/mutants/search.py (only for an equivalence declaration)
excludes:   `RfpMinPly`'s range (S251), any engine change
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S252_brief.md), 2026-10-03
done:       2026-10-03 -- M06a killed again, on a mate property: "reverse futility at ply 2 does not hide a mate from the side to move" (test_engine, mate safety). Every mate set the suite reads is the attacker's, and at ply 2 the attacker is to move again, so the floor at 2 or 3 reads the same there (S154 here identical, mined 149 both); the hazard is the mated side's. Of S145's 186 defender nodes, rooted at the mated side, 20 separate at go depth 3 to 12, 19 the mutant losing the mate; row 70's ply-3 node is mate -2 at every depth 4 to 12 shipped and at none under M06a, oracle-confirmed. The case establishes its premise by two probe drives of the ply-2 node (reverse futility fires at the floor's ply; searched as PV it is mated) and asserts mate -2 first at iteration 4 through go. Mutation run on a throwaway fixture: M06a killed 2/41 (REQUIRE( first_exact == 4 ), 0 == 4), M06b killed 4/41. Found: at f433124 M06a was already killed, by test_mate_carry's vacuity check alone since 0b1920a's budgets, which says re-sweep rather than a mate hidden. Both fast suites green; clang-format clean; prose checks clean; no src change, no assertion moved. No other project's code was opened. Cold fast check: LAND; its three comment trivials (the mined set named S145's, the premise depth stated against iteration 4, a NOT A GOLDEN line for 4 and -2) fixed at landing.

## Why this exists (2026-10-02, the coordinator)

S116's mutation run (finding 2) found `M06a_rfp_ply_floor_minus1` surviving on
S116's candidate and on its parent `f82e5a3` alike
(`.tuning/coord/S116/mutation/parent_M06.log`): no fast-suite case separates
reverse futility at ply 2 from ply 3 any more, so the guard that floor protects
is unfenced at that edge. A test property reaching no play: a filler behind
S116 (DEC-171), item 21 of `adocs/data/S116_sprt.sh`'s open findings.

## As built (2026-10-03, the Opus subagent)

### Deviations and findings, first

1. **The step's premise is stale on today's tree: M06a is already killed at
   `f433124`, but only by a vacuity check.** A fixture of `f433124` with no
   change (`.ref-builds/mut0`) reads M06a **killed, 1 of 41**, by
   `test_mate_carry`'s `CHECK( reporting >= majority )`, `2 >= 3`: "the set
   has gone vacuous and the budgets need re-sweeping", silent
   `A_mate8_shallow`, `D_mate_minus6_depth10`, `E_mate_minus9`
   (`.tuning/coord/S252/mutation/head_M06a.log`). S116's run that found M06a
   surviving was on `c7de2c8`, before `0b1920a` landed the re-swept S170
   budgets, so the budgets are what changed. That kill is a meta-check over
   script-derived budgets that says "re-sweep", not "a mate was hidden", and
   the next re-sweep can move it; the case below kills M06a on a mate
   property, so the step was carried out as briefed. Neither verdict was
   assumed: both are logged runs.
2. **The case is in `tests/test_engine.cpp` and not `tests/test_search.cpp`**,
   both in `touches:`. A first version in `test_search.cpp`'s guard suite
   asserted the mate through `search_fen()` at depths 4 to 6, and **under
   M06a it stayed green**: a cold fixed-depth `search()` finds this mate with
   the floor at 2 (315 nodes at depth 4, against 226 and no mate through `go
   depth`). The UCI iterative deepening, with its aspiration windows and
   carried table, is what loses it, so the case drives `deepen_scores()`, the
   way S145's mate case does and for the reason its comment gives. That
   version was removed whole; `test_search.cpp` is unchanged.
3. **The drive depth is 3, not 2,** observed: at 2 the node at ply 2
   returned 866 and not a mate, because the attacker's quiet mating move then
   sits at a depth-1 node at ply 3, where razoring hands it to quiescence.
4. `#include <memory>` added to `test_engine.cpp` for `std::make_unique`.
5. No engine change: `git diff f433124 -- src` is empty.

### Why nothing separated, and where the separation is

Every mate set the suite reads (S145's constructed roots, S156's mined set)
is scored from the **attacker's** side, and at ply 2 the attacker is to move
again: a static cutoff there only says the attacker is doing well. Measured,
shipped against a Release build carrying M06a (scratch worktree,
`src/search.cpp`'s one line):

- S154 `here` over S145's 82 roots: identical rows, m2 26/26 on time 26,
  m3 12/24 with the same mask; nodes 9782127 against 9814769
  (`S154_here_shipped.log`, `S154_here_m06a.log`).
- S145's mined set, 318 rows at `go depth` 10: 149 exact through both; 21
  rows differ in score, none in an exact mate lost, and 219 in nodes
  (`mined_d10_ship_vs_m06a.log`).

So M06a changes nodes everywhere and the attacker-side property nowhere. The
hazard at ply 2 belongs to the **mated** side: with it at the root, ply 2 is
that side to move again after a quiet attacking move, a material leader
whose static score clears the margin. The 186 defender nodes S145's TSV
already carries (column `defender_nodes`, labelled mated in `m - 1 - j`),
rooted at the mated side and run fresh per depth at `go depth` 3 to 12
through both builds (`defender_profile.txt`, `cmp.py`, `profile.py`): **20
separate, 19 with the mutant losing the mate**; at depth 10, 132 against 119
exact. The chosen row reports `mate -2` through the shipped build at every
depth 4 to 12 and through the mutant at none.

### The case

`tests/test_engine.cpp`, suite "engine: mate safety", **"reverse futility at
ply 2 does not hide a mate from the side to move"**.

- Root `rbrb2R1/p1p1p2p/P1P1P2P/8/8/6K1/8/7k b - - 3 2`, the ply-3 defender
  node of S145 row 70 (`rook0`, mate in 4, key g1g8); ply-2 node
  `rbrb1R2/p1p1p2p/P1P1P2P/8/8/6K1/8/6k1 b - - 5 3`, the row's next defender
  node. Oracle (`oracle.txt`): stockfish depth 20 through python-chess, root
  `Mate(-2)` for Black, pv h1g1 g8f8 g1h1 f8f1; ply-2 node `Mate(-1)`;
  python-chess `is_valid()` and not in check at both, one legal move at
  each, g8f8 neither capture nor check.
- **Premise, from the engine, on the ply-2 node** (probe drives,
  `negamax_probed`, depth 3, fresh 1 MB table): reachable, not in check,
  `evaluate() > 300`, beta at `evaluate() - RFP_MARGIN * 3`, inside the
  band. (1) Non-PV at ply `RFP_MIN_PLY`: `probe.rfp_cutoff` and a non-mate
  bound, so the floor is the only thing between the node at ply 2 and the
  cutoff. (2) PV at ply `RFP_MIN_PLY - 1`, full window: no cutoff and a
  score `<= -MATE_MIN`, so the node is mated.
- **Property:** `deepen_scores(root, 8)`, eight iterations; `first_exact ==
  4` (the first iteration that holds a mate against the side to move in 2)
  and the last iteration `mate -2`.
- Green shipped; 7.2 s for the whole `test_engine` binary in the suites.

### The red, and the mutation log

Fixture `9b950a6`, a commit object on no branch made through a temporary
index (`HEAD` plus this `test_engine.cpp`), in `.ref-builds/mut`, run with
`CLANG_FORMAT_MAJOR=22` (a first attempt without it was refused at the
baseline by `test_clang_format_script`,
`mutation/mutation_refused_no_clang_env.log`):
`python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only
M06a_rfp_ply_floor_minus1 M06b_rfp_ply_floor_minus2 --jobs 8`.
Baseline green, 41 tests, bench 4081329; anchors validated.

- **M06a killed, 2 of 41**: `[test_engine] reverse futility at ply 2 does
  not hide a mate from the side to move | REQUIRE( first_exact == 4 )`
  (values `0 == 4`), and `test_mate_carry`'s vacuity check as at `HEAD`.
- **M06b killed, 4 of 41**, by its old cases and this one.
- Score 2 of 2, wall 317 s (`.tuning/coord/S252/mutation/mutation.log`,
  `mutation/logs/results.tsv`).

### Suites

Both fast suites 41 of 41 (Release 103.0 s, tune 102.8 s);
`./clang-format.sh --check` clean with `CLANG_FORMAT_MAJOR=22`
(`suites.log`). `tools/plan_prose_check.py` `--citations`, `--touches`,
`--params`, one mode per call (`prose_*.log`). No match, no `gate_extra.sh`
(the brief's; no engine change, so DEC-141's second tier is not owed).
`DEV_MANUAL.md` and `MANUAL.md` checked: neither names M06a or the cases
that kill it, no change. `README.md` human-owned, no change.

### Proposed `done:` stamp

`2026-10-03 -- M06a killed again, on a mate property: "reverse futility at
ply 2 does not hide a mate from the side to move" (test_engine, mate
safety). Every mate set the suite reads is the attacker's, and at ply 2 the
attacker is to move again, so the floor at 2 or 3 reads the same there
(S154 here identical, mined 149 both); the hazard is the mated side's. Of
S145's 186 defender nodes, rooted at the mated side, 20 separate at go depth
3 to 12, 19 the mutant losing the mate; row 70's ply-3 node is mate -2 at
every depth 4 to 12 shipped and at none under M06a, oracle-confirmed. The
case establishes its premise by two probe drives of the ply-2 node (reverse
futility fires at the floor's ply; searched as PV it is mated) and asserts
mate -2 first at iteration 4 through go. Mutation run on a throwaway fixture:
M06a killed 2/41 (REQUIRE( first_exact == 4 ), 0 == 4), M06b killed 4/41.
Found: at f433124 M06a was already killed, by test_mate_carry's vacuity
check alone since 0b1920a's budgets, which says re-sweep rather than a mate
hidden. Both fast suites green; clang-format clean; prose checks clean; no
src change, no assertion moved. No other project's code was opened.`

### Proposed commit

```
Kill mutant M06a with a mate the floor hides from the mated side

S252, a filler behind S116 (DEC-171, DEC-141). Every mate set the
suite reads is scored from the attacker's side, and at ply 2 the
attacker is to move again, so reverse futility's floor at 2 or 3 finds
the same mates there; the floor's hazard at ply 2 is the mated side's.
A defender node of S145's set, mated in 2, is found at iteration 4
through go by the shipped engine and never under M06a. The case first
shows by probe drives that the ply-2 node is mated and that reverse
futility fires on it at the floor's ply. Mutation run: M06a and M06b
killed. At f433124 M06a was already killed, but only by
test_mate_carry's vacuity check over re-swept budgets.

No functional change
```
