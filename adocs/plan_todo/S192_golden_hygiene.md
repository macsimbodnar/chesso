id:         S192
goal:       every golden number in `tests/` is named as one with the script that re-derives it, the piece anchors are re-derivable from the repository, and the soft-limit scaling test asserts the rule on a constructed history
accepts:    an inventory in this file of every golden in `tests/` -- the static-score anchors 563/567/198/-505/-569 and the piece anchors 135/244/325/563/787, the `test_mate_carry` per-game floors, the `test_mate_breadth` floor, the mate-in-three floor, the node budgets of "ordering keeps the tree small", the `test_search.cpp` table-independence claim -- each with a comment at its site naming it a golden and the command that re-derives it; `.tuning/anchors.py` committed as `adocs/data/S192_anchors.py` (rewritten if it is lost) and shown to reproduce 135/244/325/563/787 and 563/567 at the shipped weights; `tests/test_engine.cpp`'s "the iteration loop scales its soft limit by what the search found" replaced by a case that feeds the loop's scaling from a constructed stability and score-drop history and asserts the rule, with the tree-dependent assertions removed; `tests/test_search.cpp`'s "the table never changes the answer" comment names the S130 stand-pat substitution and quiescence answering from main-search entries as the property's known exceptions; the fault-injection driver re-run over M06a, M09, M29 and M30 shows each still caught, by a golden or by S191's cases; fast suite green in both builds
touches:    tests/, adocs/data/, .gitignore, DEV_MANUAL.md
excludes:   changing any golden's value; any retune or refit
decisions:  DEC-139, DEC-142
closes:     2026-09-04_test_review-F03
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F03`. A large share of the suite's sensitivity comes
from golden numbers that every legitimate search or evaluation change also
moves: the eval-sign mutant turned 16 cases red, twelve of them on the static
anchors; `test_mate_carry` went red on 21 of the 22 search mutants; the
soft-limit scaling case went red on eight search mutants that were not
time-management defects, because it asserts on a fixed position's tree rather
than on the rule. The numbers are right today and they did their job in the
pass. The cost is that every pending search step and every refit will redden
several of them for no defect, each needing a re-derivation, and a floor
re-derived under time pressure is how a gate gets weakened -- S145 recorded
two surveyed engines that disabled their mate tests rather than their pruning.
DEC-116 states the rule for one floor; DEC-142 generalises it and this step
applies it.

## Cost

Machine-free, about a day. `adocs/data/S154_floor_margin_sweep.py` is the
pattern for a re-derivation script.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

A golden is a number a test asserts that was read from a measurement rather
than derived from a rule: the static score 563 of a one-rook position, a floor
of 143 mates found, a node budget of 440000. The suite has about a dozen and
they are its best detectors -- the 2026-09-04 fault-injection pass killed 31 of
32 mutants and the goldens did much of the killing. Their cost is that every
legitimate change to the same code also moves them, so each search step and
each refit reddens several and someone re-derives a number under time
pressure. DEC-142 makes that a procedure: every golden is named as one at its
site, carries the script that re-derives it, is re-derived by that script and
never re-read from a green run, states its margin, and stands beside a
property case that does not move with the tree. This step applies the
procedure once over `tests/`. It changes no golden's value and no engine
source.

Three premises of the accepts have moved since the finding was written:

- `.tuning/anchors.py` is **tracked**, not gitignored: commit `c56ab41`
  (2026-08-16, "Track the fit scripts, keep the corpus out") added it,
  `.gitignore` carries `!.tuning/*.py` for it, and `git ls-files .tuning/`
  lists it with four siblings. It is not rewritten; it is `git mv`-ed and
  repaired (section 3). `status.md`'s Parked note and F03's "gitignored" are
  stale; the coordinator retires the note when this lands.
- The script does not run as tracked: `ROOT = "/home/max/ws/chesso/"` is
  hard-coded and the unpatched run ends in `FileNotFoundError` here. With the
  root pointed at this checkout it printed `10 of 10 reproduced` at `97e1f0c`
  on 2026-09-05, so the derivation is sound.
- The mate-in-three floor is **11**, not 8: S154 re-derived 8 over 48
  positions and S168 re-derived 11 over 82 the same day
  (`adocs/data/S168_floor_sweep.log`). `DEV_MANUAL.md` "Mate safety" still
  says "a floor of 8 on the mate in three count"; correct it here, since
  `DEV_MANUAL.md` is in `touches:`.

### 2. The technique as published

Feathers: "The purpose of characterization testing is to document your
system's actual behavior, not check for the behavior you wish your system had"
(https://michaelfeathers.silvrback.com/characterization-testing). Approval
testing is its tooling: "taking a snapshot of the results, and confirming that
they have not changed", a legitimate change being reviewed and re-approved
(https://approvaltests.com/). Both make the program's own output the reference.
Chesso is stricter exactly there: S028 found that "an anchor copied from the
thing it anchors asserts nothing", so a chesso golden is re-derived from a
**specification or a rule** -- `anchors.py` is a second implementation of
`evaluate()` written from `src/evaluation.cpp`'s prose, and a floor sits
strictly between two measured ends (DEC-116) -- never re-approved from a run.
DEC-142's sentence, now in the TESTS rule: "Every golden value or floor in
`tests/` is named as a golden at its site with the command that re-derives it;
a golden is re-derived by its script whenever either end of it moves, with the
margin stated, and never re-read from a run; a golden that cannot be scripted
is a finding; `anchors.py` enters the repository. Where a golden stands in for
a property, the property gets its own case so coverage survives a
re-derivation."

### 3. What chesso has today, and where the change plugs in

The inventory, from the repository at `97e1f0c`. "Today" is what re-derives
the value now, "proposed" what this step leaves, "moves on" the legitimate
change after which the script is run and the number re-derived -- any other
movement is a defect first.

| # | site | value | pins | today | proposed | moves on |
|---|---|---|---|---|---|---|
| 1 | `tests/test_evaluation.cpp` "each piece is worth what the tables say" | 135, 244, 325, 563, 787, 0 | `evaluate()` of six one-piece positions, White to move, at the shipped weights | `.tuning/anchors.py` (tracked; root hard-coded, fails here) | `adocs/data/S192_anchors.py`, reads `src/eval_tables.hpp` and `src/evaluation.cpp` | a refit; an evaluation term added or changed (section 5) |
| 2 | `tests/test_search.cpp` "a quiet position stands pat" | 563 `evaluate`, 567 `evaluate_cheap` | the rook-on-d1 position, two evaluation paths | same | same. The pair recurs in "a quiescence entry carries the static score, never a bound", "quiescence stands pat on the stored static score", "quiescence stands pat on the stored score where the bound allows it", "a mate score is never used as a stand pat", "a substituted stand pat is stored as the bound it is", "a capped stand pat beaten by a capture is still a bound", "reverse futility prunes on the stored static score", "a main-search store records this node's evaluation": two named constants in `tests/test_search.cpp` with one golden comment, so a refit edits two lines and not eleven | a refit |
| 3 | `tests/test_search.cpp` "a side in check may not stand pat" | 198 | `evaluate()`, Black to move, in check | `anchors.py` case "black in check, Re8" | `S192_anchors.py` | a refit |
| 4 | `tests/test_search.cpp` "a quiet evasion is a legal answer to a check" | -505 | `quiescence()` in check: a one-ply negamax, best of four leaves' `-evaluate()`, not an evaluation call | `anchors.py` `LEAVES` and `QUIESCE_IN_CHECK` | `S192_anchors.py` | a refit; **also** any change to how quiescence treats a checked side -- that end is the property under test, so a move there is a finding before it is a re-derivation |
| 5 | `tests/test_search.cpp` "the losing side takes an available repetition" | -569 | `evaluate()`, Black to move, a rook down | `anchors.py` case "black a rook down, Kh7" | `S192_anchors.py` | a refit |
| 6 | `tests/test_mate_carry.cpp` `short_line_ceiling` in "a mate score carried across searches keeps a line that reaches it" | 5, 11, 0, 1, 8 | ceiling on the short mating PVs per replayed game of `adocs/data/S170_cases.tsv` at the row's `go` budget, the worst cell the recorded grid shows for that case | `adocs/data/S203_case_sweep.sh --ceilings` over `adocs/data/S204_sweep_head.txt` and `adocs/data/S204_sweep_killer_iter_clear.txt` | **done by S204, DEC-162.** The row read `expected_mate_lines`, a per-case floor on mate lines, and this row's own "21 of 22 search mutants moved it" was the defect: it was one cell of a sparse grid and every tree-moving change re-pinned it. Vacuity moved to a fixture-wide majority with no per-case number to re-derive | a re-sweep of the budgets, which is a Zobrist redraw (DEC-154); S202 closing lowers the ceilings |
| 7 | `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 143 | exact mates over the 318 mined positions at depth 10; ends 145 shipping, 141 guard weakened | `adocs/data/S156_mined_floor_sweep.py`, runs, builds a throwaway worktree | keep; add the golden header | a search change that costs or buys mate finding; re-derive when either end moves |
| 8 | `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 11 | exact mates in three over the 82 constructed positions; ends 12 and 10 | `adocs/data/S154_floor_margin_sweep.py floor` and `red`; `adocs/data/S168_floor_sweep.log` | keep; the comment carries the rule and both logs, add the header | same |
| 9 | `tests/test_engine.cpp` `MATE_DEPTH_SLACK` | 8 | **not a golden**: a depth budget, priced by `S154_floor_margin_sweep.py slack` | that script | comment says "a measured window, not a golden" and names the mode | S154 re-decides it |
| 10 | `tests/test_search.cpp` "ordering keeps the tree small" | `node_limit` 440000, lower bound 20000 | depth-5 `search()` node count on `TRICKY_POS` from a cold table, 109575 when measured 2026-08-14, held inside [count / 5, 4 x count] | none | `adocs/data/S192_node_budget.py`: runs `build/tests/test_search --test-case="ordering keeps the tree small" --success`, reads the count from a `MESSAGE` the case gains, prints both bounds by the stated ratios | any ordering or search change; re-derive when the count leaves the middle half of the band |
| 11 | `tests/test_search.cpp` "the table never changes the answer" | none | a property: the answer at depth 2 and 3 is independent of table contents | -- | the comment names the three known exceptions (section 6) | -- |
| 12 | `tests/test_engine.cpp` "the iteration loop scales its soft limit by what the search found" | `drop == 0`, `drop > 0`, `scale < 100` at depth 8 on two fixed positions | that the loop feeds `search_time_scale_percent()` the stability and drop it counted | none | replaced by a constructed-history case (section 6) | -- |
| 13 | `tests/test_search_params.cpp` `golden_defaults` | 28 defaults with ranges | a deliberate-change detector, already named golden; `src/search_params.hpp` is its derivation | the source table | no script owed; one comment line says so | a step that moves a default |
| 14 | outside the accepts, unresolved | -- | `tests/test_eval_model.cpp`'s four pinned positions: S076's commit message says `tools/truncation_scan` re-derives them and the test does not name it -- verify at HEAD, add the name or record a finding. `tests/test_uci_surface.cpp`'s lists are the SURFACE rule's, out of scope. The two `explored_nodes < 100000` ceilings in `tests/test_search.cpp` carry orders of magnitude of slack and are not goldens | | | |

The work, in order. **(a)** `git mv .tuning/anchors.py adocs/data/S192_anchors.py`;
compute the root from `__file__` as `adocs/data/S154_floor_margin_sweep.py`
does with `REPO`; replace every `file:number` in its comments with the
`TEST_CASE` title the anchor belongs to (DEC-135); keep the contract -- no
argument reads the shipped weights and exits 1 on a mismatch, a header path
prints `got (was old)`. Run it, require `10 of 10 reproduced`, save the output
as `adocs/data/S192_anchors.log`. Re-point the live citations of
`.tuning/anchors.py`: `tests/test_evaluation.cpp` "each piece is worth what the
tables say", `tests/test_search.cpp` "a quiet position stands pat" and "a
quiescence entry carries the static score, never a bound", `DEV_MANUAL.md`
"Re-measure the truncation-bound positions after a fit". `plan_done/` keeps the
old path; the other four `.tuning/*.py` stay. **(b)** The golden comments, rows
1 to 10 and 13, in section 6's shape. **(c)** The two scripts of rows 6 and 10
and their logs. **(d)** The soft-limit replacement and the table-independence
comment. **(e)** The four-mutant re-run, then `DEV_MANUAL.md` and
`adocs/data/README.md`.

The piece anchors need **no corpus**: `anchors.py` reads the weights out of the
two source files and multiplies hand-derived feature counts. The 683 MB
`selfplay_v2.tsv` -- gone from this machine, only `selfplay_v1.tsv` remains in
`.tuning/` -- produced the weights and does not derive the anchors, so the
committed recipe is the script alone and the corpus stays out under the trade
`c56ab41` and `.gitignore` record.

### 4. Constants and seeds

None. No engine constant moves and every golden's value is excluded by
`excludes:`. The only numbers written are row 10's ratios, 4 x and 1/5, which
the existing case already embodies (440000 / 109575, 20000 / 109575) --
DEC-105 form (b), derived from the repository's own reading.

### 5. Interactions and traps

- **The anchors script parses the header by regex** -- `#define NAME value`,
  `static constexpr int psqt_mg[6][64]`, `const int name[N] = {...}`, `const
  int name = v;`. A refit that emits another layout (S135, S136) breaks the
  parse silently except for the PSQT length assert. Run it with no argument
  first, `10 of 10 reproduced`, then against the fitted header.
- **An evaluation term added or changed is the expensive case.** Every case's
  feature vector (`phase`, `psqt`, `passed`, `structure`, `placement`,
  `mobility`, `attackers`, `shelter`) was hand-derived for the present term
  set and `score()` mirrors `evaluate()` down to `trunc_div` and the
  `LAZY_EVAL_MARGIN` clamp. A new term is a new field in every case and in
  `score()`, written from `src/evaluation.cpp`'s prose, never by calling the
  engine. That is the second-implementation cost and its point.
- **Row 4 has two ends.** -505 moves with the weights and with quiescence's
  in-check behaviour. Before re-deriving after a search change, confirm the
  four leaves are still the four king moves, each standing pat; otherwise the
  property case has caught something.
- **Floors are never re-read from a green run.** The S156 trap: a `setoption`
  outside the declared range is refused and the engine answers with its
  default, so a sweep that ignores `info string refused` reads a perfect
  null. Both sweep scripts check it; do not hand-roll a sweep.
- **Row 6 is fragile by construction** -- it guards what the table happens to
  hold -- and its margin is zero. DEC-142 wants the margin stated: state "0";
  widening it is the owner's (section 10).
- **In-process `search()` and UCI `go depth` count differently.** Row 10 calls
  `search(5, ...)` once on a cold table with no aspiration; a UCI drive of the
  same FEN gives another number. Hence the `MESSAGE` and the test-binary
  filter with `--success`.
- **`searchmoves` cannot construct a stable best move**: the `go` parser in
  `src/chesso.cpp` skips the token (`token == "searchmoves"`, beside `"mate"`
  and `"ponder"`) and nothing in `iterative_deepening_search` reads
  `uci_search_options_t::searchmoves`. A root with exactly one mate in one is
  the construction that works (section 6).
- **The deepening loop has no early exit on a mate.** It breaks on
  `state.aborted`, the stop signal, `conf.nodes` and the soft limit only, so
  at `go depth d` with no clock all d iterations complete; the mate score
  disarms the aspiration window (`aspiration_ready`), which does not touch the
  stability count.
- **Both builds.** Under `build-tune` every `SEARCH_PARAM` is a variable: no
  `static_assert` on a `TM_*` value (DEC-118).
- **`run.py` expects a worktree it does not create**, at `adocs/data/mut`
  (`WT = os.path.join(os.path.dirname(HERE), "mut")`) with a configured
  `build/`. Add it with `git worktree add`, remove it after, never commit it.
  After S196 the tool takes the worktree path as an argument.
- **DEC-023.** The mate-in-one root comes from a case that already
  tool-verified it; no new position is invented.

### 6. Tests

**The golden comment**, one shape everywhere, first line greppable:

```
// GOLDEN (DEC-142): <what the number pins, one clause>.
// Re-derive: <exact command>. Moves legitimately on: <refit | search change>.
// Margin: <exact | n on each side | 0>. Property beside it: "<TEST_CASE title>".
```

Row 1: `Re-derive: python3 adocs/data/S192_anchors.py` (must print `10 of 10
reproduced` before a fitted header is fed to it), margin `exact`, property "the
lazy shortcut cannot change a decision". Rows 7 and 8 cite the sweep script's
modes and the log; row 13 says `no script: src/search_params.hpp is the
derivation`.

**The soft-limit case, replaced.** Keep the `probe` lambda and the `uci_last_*`
instrumentation; replace both `SUBCASE`s:

```
TEST_CASE("the iteration loop scales its soft limit by the history it counted")
{
  // Construction, not measurement. On a root with exactly one mate in one the
  // best move is the mating move at every iteration and the score the same
  // mate score, whatever the pruning rules do deeper, so at [go depth d] with
  // no clock the loop counts a stability of exactly d - 1 and a drop of
  // exactly 0. "7k/6pp/8/8/8/8/8/R6K w - - 0 1" is the suite's "mate in one"
  // position, one mate in one, Ra8#, verified there by tool (DEC-023).
  for (int depth : {2, 5, 8}) {
    const probe_t scaled = probe("position fen 7k/6pp/8/8/8/8/8/R6K w - - 0 1", depth, true);
    REQUIRE_EQ(scaled.stability, depth - 1);     // the construction held
    REQUIRE_EQ(scaled.drop, 0);
    CHECK_EQ(scaled.scale, search_time_scale_percent(depth - 1, 0));
    CHECK(scaled.scale < 100);                   // TM_STABILITY_PERCENT > 0, asserted by the pure case
    const probe_t fixed = probe(<same>, depth, false);
    REQUIRE_EQ(fixed.stability, depth - 1);
    CHECK_EQ(fixed.scale, 100);
  }
}
```

The drop half of the rule stays where it is held as a pure function, "the time
scale moves with stability and with a falling score"; what the loop owes for
the drop is the identity `scale == search_time_scale_percent(stability, drop)`
at whatever drop the tree produced, and a tree-independent drop needs a hook
in `src/` (section 10). Observe the old case red under M06a before deleting
it, so the replacement is a recorded trade and not a silent loss.

**The table-independence comment.** After the lazy-evaluation paragraph: "Two
more paths break the stated purity and are already in the tree. S130:
`quiescence` in `src/search.cpp` raises or lowers `stand_pat` from a table
entry's bound (`stand_pat_type` carries which), so a warm table changes the
stand pat. S094 and S130: `tt_entry_answers` accepts a main-search entry at
`TT_DEPTH_QS`, so quiescence can answer from a node the main search stored.
Depth 2 to 3 over this corpus exposes neither; a failure here reads as 'one of
three things changed'." Name S130 and both symbols; add nothing else.

**The four-mutant re-run.** Before S196: `python3
adocs/data/2026-09-04_test_review/run.py M06a_rfp_ply_floor_minus1
M09_lmr_no_research M29_eval_expensive_sign M30_mvv_lva_sign` with the
worktree at `adocs/data/mut`; after S196, `tools/mutation_check.py` with the
same ids over `tools/mutants/`. Expected detectors, from
`adocs/data/2026-09-04_test_review/kills.txt`: M06a -- `test_mate_carry`
only, once the soft-limit case no longer fires on it, because S191's accepts
covers reverse futility in check, at a PV node, above `RFP_MAX_DEPTH` and in
the mate band but **not the ply floor**; M09 -- S191's "a reduced move that
beats alpha is re-searched at full depth" plus `test_mate_carry`; M29 -- the
twelve anchor sites plus "the unclamped terms are the engine's own" and
`test_eval_model`; M30 -- "MVV-LVA prefers a cheap attacker" and "the declared
history ceiling clears the band above it", both properties, plus
`test_mate_carry`. Record the four rows in the stamp.

**Not owed:** INV-6 (`tools/search_bench.py`) and the Debug self-play -- `git
diff --stat HEAD -- src/` is empty. **Owed:** the gate in both builds,
`./clang-format.sh --check`, the scripts run once with their output committed
as logs.

### 7. Measurement

None: machine-free, no timing lane and no SPRT. The evidence is the scripts'
outputs (`10 of 10 reproduced`, the per-game counts, the node count and its
bounds) and the four-mutant kill table.

### 8. Completion checklist

1. `cmake --build build -j8 && ctest --test-dir build -L fast
   --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
   build-tune -L fast --output-on-failure && ./clang-format.sh --check`.
2. `git diff --stat HEAD -- src/` empty; no `Bench:` trailer owed or written
   -- DEC-140 binds commits that touch `src/`.
3. `DEV_MANUAL.md` "Test": a subsection "Goldens: named, scripted, re-derived"
   with DEC-142's sentence, the inventory's script column as a table and
   `grep -rn 'GOLDEN (DEC-142)' tests/` as the listing; "Mate safety" corrected
   from 8 to 11. `MANUAL.md`: no UCI change, "checked, no change".
   `adocs/specs.md`: no behaviour change, same.
4. `adocs/data/README.md`: rows for `S192_anchors.py`, `S192_anchors.log`,
   `S192_mate_carry_floor.py`, `S192_node_budget.py` and their logs.
   `.gitignore`: untouched -- the scripts print and the logs are committed --
   and the stamp says so.
5. `python3 tools/plan_prose_check.py --touches | tail -1` still prints
   `touches flagged: 0`.
6. Stamp: the inventory count; `10 of 10 reproduced` at the sha; the
   mate-carry counts per game and margin 0; the node count and both bounds;
   the old soft-limit case observed red under M06a then replaced; the
   four-mutant table; gate times in both builds; docs checked.
7. `plan.md` (out of Open, into Done), `status.md` (Parked `anchors.py` note
   retired) and the F03 `Status:` line in the audit report go through the
   coordinator.

### 9. Sources read

- https://michaelfeathers.silvrback.com/characterization-testing and
  https://approvaltests.com/ -- the two quotations of section 2, fetched
  2026-09-05.
- `adocs/decisions.md` DEC-116, DEC-139, DEC-142, DEC-145;
  `adocs/audit/2026-09-04_test_review.md` F03; `adocs/testing_strategy.md`
  R4 -- the rule, its evidence and the recommendation.
- `.tuning/anchors.py`, run in a scratch copy with the root patched: `10 of
  10 reproduced` at `97e1f0c`; `git log -- .tuning/anchors.py` for `c56ab41`
  and `77d7450`.
- `adocs/plan_done/S154_mate_floor_margin.md`, `S156_mined_set_floor.md`,
  `S168_second_mate_motif.md`, `S170_mate_line_across_searches.md`;
  `adocs/data/S154_floor_margin_sweep.py`, `S156_mined_floor_sweep.py`,
  `S168_floor_sweep.log`, `S170_replay.py`, `S170_cases.tsv` -- how each floor
  was placed and re-derived.
- `adocs/data/2026-09-04_test_review/mutants.py`, `run.py`, `kills.txt`,
  `results.tsv` -- the four mutants and what kills them today.
- Every site in the table, read at HEAD in `tests/test_evaluation.cpp`,
  `tests/test_search.cpp`, `tests/test_engine.cpp`, `tests/test_mate_carry.cpp`,
  `tests/test_mate_breadth.cpp`, `tests/test_search_params.cpp`.
- `src/chesso.cpp` `iterative_deepening_search`, `search_time_scale_percent`
  and the `go` parser; `src/uci.hpp` the `uci_last_*` declarations;
  `src/search.cpp` `quiescence`, `tt_entry_answers`.
- `DEV_MANUAL.md` "Test" and "Mate safety"; `adocs/data/README.md`;
  `adocs/plan_todo/S191_pruning_guard_tests.md`, `S196_mutation_check_tool.md`.
- Nothing here is from outside the repository except the two quotations;
  nothing is unverified.

### 10. Questions deferred to the owner

1. **Row 6's margin.** The five `test_mate_carry` floors equal the counts
   observed 2026-09-02, margin 0, and 21 of 22 search mutants moved them. Keep
   at the count (maximum sensitivity, re-derived by every search step) or set
   each to `max(1, count - 1)` (fewer false reds, weaker detector)? The
   accepts forbids changing a value, so either way it is a follow-up decision.
2. **M06a becomes a single-detector mutant** once the soft-limit case stops
   asserting on the tree: only `test_mate_carry`, a golden, catches the ply
   floor lowered by one, and S191's accepts does not include the ply floor.
   Add a guard case there (precondition: a node at `ply == RFP_MIN_PLY - 1`
   whose static score clears beta by the margin; assert it is searched), or
   accept.
3. **The drop half of the soft-limit rule.** A tree-independent drop needs a
   hook in `src/` -- the loop's two update rules extracted into a function the
   test can feed -- which changes `touches:` and owes INV-6. Form A above, the
   identity at whatever drop the tree gave, needs none. Which?
4. **Two named constants for the nine 563/567 sites** is hygiene beyond "a
   comment at its site". Approve, or comment each site.
5. `status.md`'s Parked `anchors.py` item and F03's premise are stale: retire
   with this step's stamp, or by a separate status rewrite now.
