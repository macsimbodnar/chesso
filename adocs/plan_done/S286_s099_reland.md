id:         S286
goal:       S099's pawn key and pawn-keyed correction, measured H1 at 32+0.32 by S283, reland onto S039's outcome with every golden re-derived by its script
accepts:    (1) `8758622`'s change -- the branchless pawn key on board_t, the pawn-keyed correction table (4096 slots a side, owned by the UCI layer, one per datagen worker, cleared per game), its read at negamax_at's compute-or-read site with both TT stores keeping the raw evaluation, its update beside the node's store, `CorrHistMax` and `CorrHistWeightCap` -- re-applied onto S039's outcome (S039 kept, or reverted on its H0), not re-designed and not re-tuned; (2) the tune build at `CorrHistMax` 0 prints the parent's bench stream (DEC-215); bench and `tools/search_bench.py` recorded; (3) every golden the change moves re-derived by its own script, never read off a run (DEC-142): carry ceilings and S170 budgets, `EXACT_FLOOR`, `MATE_IN_THREE_FLOOR`, first-mate, the mined mate rows (re-mined by their scripts if they stop separating, DEC-288 and DEC-293), the ordering band by S282's rule; **no relaxation allowance**: DEC-272 and DEC-284 lapsed with S099's H0, so a ceiling rise, a loosened budget or floor, or a dropped mate floor stops for the owner's ruling; (4) DEC-141's second tier: S099's mutants and any a re-mine touches killed, Debug self-play four rounds at 4+0.04 with no `Assertion`, `tools/gate_extra.sh`; (5) no SPRT: it ships on S283's H1 (DEC-286), and its commit cites S283's record; INV-4 holds (the key is maintained in add_piece, remove_piece and move_piece, never recomputed)
touches:    src/data_structures.hpp, src/search.cpp, src/search.hpp, src/search_params.hpp, src/bitboard.cpp (the piece primitives), src/bitboard.hpp, src/correction_history.hpp (new), src/chesso.cpp (where the table is owned) and src/uci.hpp, tools/datagen.cpp, tests/, tools/mutants/, MANUAL.md, DEV_MANUAL.md, adocs/specs.md, adocs/data/S170_cases.tsv, adocs/data/README.md
excludes:   any change to the correction's design or its two parameters' defaults; S110 and S111, each its own step and verdict; the two consumers DEC-222 named
decisions:  DEC-286, DEC-271, DEC-222, DEC-142, DEC-215, DEC-141, DEC-294, DEC-295
closes:
blocks:     S110, S111
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent, phase 1 blind beside S039's SPRT (DEC-260), phase 2 on 8560a36
done:       2026-10-16 00:20 -- S099's pawn key and pawn-keyed correction relanded onto S039's outcome (<kept: ca321b6's tree | H0: ca321b6 reverted>), not re-designed and not re-tuned: 8758622's src/ change lines byte for byte, 4096 slots a side, CorrHistMax 47, CorrHistWeightCap 144. No SPRT owed: it ships on S283's H1 at 32+0.32 (ee5295b; nElo +12.56 +/- 9.89 over 4738 games), DEC-286. bench <parent> -> <PENDING>; the tune build at CorrHistMax 0 prints the parent's stream (DEC-215); search_bench 9 and 12 recorded. Goldens re-derived by their scripts (DEC-142): carry ceilings <..> and budgets <..> (<ruling id>); EXACT_FLOOR <..>; MATE_IN_THREE_FLOOR <..>; first-mate <..>; test_search's mined rows and node band unmoved by construction, confirmed (S192 count <..>, every row in its mutant's kill list). test_search_params 72 -> 74. Fast suite <n>/<n> in both builds, format clean; Debug self-play four rounds at 4+0.04, <g> games, 0 Assertion; gate_extra five stages green, the sanitizer bench equal to Release; mutants CH01-15 and PK01-05 <k> of 20 killed. INV-4: the key is kept by add_piece, remove_piece and move_piece, never recomputed. Implemented from S099's description and the publications it cites (DEC-221). Fast check: 5 findings, fixed (wording to what was measured, the evidence committed as adocs/data/S286_rows.log and S286_mutation_pass.tsv, a restored citation, the index, the step's decisions). Implemented by an Opus subagent in two phases; reviewed, stamped and committed by the coordinator.

## Why this exists

S099 read H0 at the harness setting (`74bb9ad`, nElo -4.71 +/- 6.84 at
8+0.08) and was reverted (`5a04ffa`); its 32+0.32 estimate read +12.46 +/-
15.23, and the owner booked one SPRT at that control (DEC-286). S283 read H1
there: nElo +12.56 +/- 9.89 over 4738 games (`adocs/data/S283_sprt.log`). By
DEC-286's H1 row S099 relands, rebased onto the then-tree, which is S039's
outcome (DEC-290); S099's own step file (`adocs/plan_done/S099_correction_history.md`)
holds the design, its readings and its mutants. Phase 1 is written blind while
S039's SPRT holds the machine (DEC-260); phase 2 runs on S039's outcome.

## As built, phase 1 (2026-10-15, Opus subagent; written blind)

**Superseded by "Phase 2 on `8560a36`" below.** Phase 1 was written on S039's
candidate, which S039's H0 reverted; its PENDING marks, its 72 -> 74 count,
its ceilings 2, 5, 6, 0, 0, 1 and its proposed stamp and commit text describe
that tree and do not stand. It is kept as the record of the blind rebase.

In the worktree `/home/max/ws/chesso-s286` (branch `s286` at `31aa81a`, whose
src/ is S039's candidate `ca321b6`, bench 6100949) while S039's SPRT holds
every core (DEC-260). `git diff 8758622^ 8758622` saved as
`.tuning/coord/S286/s099.diff` and applied with `git apply --3way`, the step
file, `adocs/data/README.md` and S099's data files excluded (S099's record and
data stayed through the revert; its README rows still read true). **Nothing
compiled, built, tested or run**; every outcome below is a prediction. No
commit, no ref moved, no stash. Line numbers are this tree's, written in
words (DEC-135).

### Deviations, first

1. **The test_search goldens the brief lists cannot move, by construction,
   and are marked so.** The mined rows (R01's `mate_the_extra_ply_hides`,
   E21's `mate_the_multicut_hides`, B04's `mate_probcut_hides`,
   `capture_mates`, the LQ02 and LQ03 drive) and the ordering band ("ordering
   keeps the tree small") are all read in `tests/test_search.cpp`, which
   drives no UCI path and whose every state is built with `pawn_correction`
   null -- only `src/chesso.cpp`'s `iterative_deepening_search` (line 1066)
   and `tools/datagen.cpp`'s `run_search` (line 132) attach a table, and the
   only test_search cases that attach one are S099's own. Their scripts drive
   the same thing: `S192_node_budget.py` runs `build/tests/test_search`, and
   the miners' drivers are `search_fen()` line for line. S099's phase 1 said
   the same ("Not S099's to re-derive"). Each is still marked PENDING at its
   site, as "expected unmoved", with the cheap confirmation phase 2 owes
   instead of a re-mine; a move there stops phase 2.
2. **The census is not re-taken** (`CORRHIST_PAWN_ENTRIES` stays 4096): the
   brief's "not re-tuned: the same key, table" and the step's `excludes:`.
   The constant's own comment says to re-run the census "when the search, the
   key or the table's gate moves", and S039 moved the search; that sentence
   is left unexercised and asked about below (R2). specs.md says "not
   re-taken for the reland".
3. **Docs beyond the brief's list**, each because S099's merged text was
   false on this tree: MANUAL.md's `RfpMinPly` row had merged clean with
   S099's breadth numbers on S261's tree (164, 165, 160 against 163) and now
   carries S039's (162, 160, 152 against 161) plus a PENDING note -- S039 had
   left S261's numbers there; specs.md's "absent, search" row still listed
   "correction history" (S099's own commit left it, and its revert's message
   assumed otherwise) and now names the non-pawn and continuation tables
   (S110, S111) as absent; DEV_MANUAL.md's golden table carries a PENDING
   prefix on the ten rows phase 2 reads.
4. **src/ is S099's byte for byte.** The `+`/`-` lines of `git diff HEAD`
   over `src/`, `tools/datagen.cpp`, `tools/mutants/`,
   `tests/test_invariants.cpp` and `tests/test_evaluation.cpp` equal
   `8758622`'s, line for line (diffed). No src line was written by hand.
5. **The index**: `git apply --3way` implies `--index`, so the clean files
   are staged and seven resolved files are still unmerged (`UU`): DEV_MANUAL.md,
   `adocs/data/S170_cases.tsv`, `adocs/specs.md`, `tests/test_engine.cpp`,
   `tests/test_mate_breadth.cpp`, `tests/test_mate_carry.cpp`,
   `tests/test_search_params.cpp`. The working files carry no marker; `git
   add` them before the commit. The two new mutant files and
   `src/correction_history.hpp` are already added.
6. **`touches:` names `src/uci.cpp`**, which does not exist: the table is
   owned in `src/chesso.cpp`, declared through `src/uci.hpp`, and the change
   also lands in `src/bitboard.hpp`, `src/correction_history.hpp` (new) and
   `tools/mutants/S234_tt_estimate_margins.py`. The header is left for the
   coordinator; `--touches` names no symbol of this goal, so it stays green.
7. **Found, not fixed** (outside the step): specs.md's "absent, search" row
   opens with "razoring", present since S116; DEV_MANUAL.md's `EXACT_FLOOR`
   row's script column reads `--floor 159` at S039's 161.

### Conflicts, and how each was resolved

Clean: all nine src/ files, `tools/datagen.cpp`, the two new mutant files and
S234's re-anchored G04, `tests/test_invariants.cpp`,
`tests/test_evaluation.cpp`, `tests/test_search.cpp` (offsets of 77 lines),
MANUAL.md (offsets of 6 and 7; one row rewritten, deviation 3). Seven
conflicted:

- `tests/test_search_params.cpp`: S039's "takes one out" sentence kept, S099's
  sentence restated as landed at 73 to 75, reverted, relanded 72 to 74;
  GOLDEN **74**. S099's two rows and its range case merged clean.
- `tests/test_engine.cpp`, first-mate: S039's "Re-read at S039" paragraph and
  the rows (9, 9, mate 5) kept; S099's reading kept as history on `6fc7f23`;
  a PENDING mark. `MATE_IN_THREE_FLOOR`: S039's **12** and its paragraph kept,
  S099's 11 reading as history, a PENDING mark. S099's pawn-key cases and
  "ucinewgame clears the pawn correction table" merged clean.
- `tests/test_mate_breadth.cpp`: S039's **161**, its ends 162 and 152 and its
  margin text kept; S099's 163 as history; a PENDING mark.
- `tests/test_mate_carry.cpp`: **S039's file whole**, then a PENDING mark. The
  3-way merge had also carried S099's E and F table rows (E 1 by DEC-284, F 2
  by DEC-272) in without a conflict; both decisions lapsed, so they are out.
  S099's grid is not added to `--ceilings`: it was swept on S261's tree.
  Ceilings stay **2, 5, 6, 0, 0, 1** (DEC-291).
- `adocs/data/S170_cases.tsv`: S039's rows and paragraph whole, a PENDING
  paragraph; S099's budgets (C 1200000, D 500000, F 300000, S261's tree) not
  applied.
- `adocs/specs.md`: the evaluation row is S039's (S099 never touched it). The
  search row was merged word by word -- one word a line, `git merge-file`
  over the three versions, 0 conflicts -- then "the SPRT candidate of that
  step" became "relanded by S286 on S039's tree after S283 read H1 at
  32+0.32 where S099's own SPRT had read H0 at 8+0.08 (DEC-286)", and the
  census clause gained "on S261's tree, not re-taken for the reland". Board
  row: 224 B (216 without S099's pawn key). INV-2 and INV-5 rows merged clean.
- `DEV_MANUAL.md`: `--floor 161` (S039's); the golden table's rows are S039's,
  ten with a PENDING prefix; `golden_defaults` reads **74**, relanded on
  S039's 72; the invariants paragraph merged clean; the datagen paragraph
  re-dated to S286 (a corpus is with the correction only from S286, or while
  `8758622` stood).

### S039 and S099, where they meet

- **`evaluate_lazy`: S099 called it nowhere.** Its one evaluation call is
  `negamax_at`'s compute-or-read site, `src/search.cpp` line 1813,
  `evaluate()` on S099's tree already. The only lazy caller, quiescence, S099
  left alone; on this tree it is S039's, `evaluate()` at every window (line
  1195) and `stored_eval = static_eval` (line 1207): its stand pat is the raw
  full score, uncorrected, as specs.md says.
- **The clamp: no S099 line read it.** `corrected_static_eval` clamps to
  `+/-(MATE_MIN - 1)` (line 1035), its own bound. Its comment leans on
  `tests/test_evaluation.cpp` "the static score never reaches the mate band"
  (line 593), which S039 kept and widened (16-bit fit, unclamped maximum
  23891), so a corrected static score is at most 23891 + 47, far inside
  47999. The case "a corrected score never reaches the mate band" uses 22945
  as one raw input (S213's old maximum): an input, not a dependence.
- **Raw read and write.** `raw_eval` (lines 1810 to 1813) is the entry's
  evaluation or `evaluate()`; `static_eval` (1821 to 1823) adds the slot; the
  update (3230 to 3234) learns `best_so_far - raw_eval`, gated by
  `correction_may_learn` on `raw_eval`. Both `negamax_at` stores pass
  `raw_eval`: ProbCut's (2301 to 2303) and the node's (3219 to 3220).
  Quiescence's four stores (1281, 1510, 1544, 1564) pass `stored_eval`,
  `evaluate()`. What S039 adds: every quiescence entry now carries an
  evaluation, so `tt_eval` at line 1813 hits more often -- the same number
  (S103 counted 0 disagreements). The int16 field never holds a corrected
  score.
- **INV-5 holds.** "a mirrored position reads the same corrected score"
  (`tests/test_evaluation.cpp` line 145) requires `raw == mirrored_raw` first,
  which S039's "colour symmetry over every test position" (line 104) asserts
  on the unclamped score; the slot is side-to-move relative.
- The max-ply return (line 1620) is raw `evaluate()`, as on S099's tree; it
  is not a consumer of `static_eval`.
- The RFP case "a correction moves reverse futility and not the store"
  (`tests/test_search.cpp` line 14735) needs the lowered drive to reach its
  store: it passes `prev_move` 0, which refuses the null move, and at depth
  3 neither ProbCut nor razoring runs, so S039's quiescence cannot cut it
  off first.

### Mutant anchors (accepts 4's input)

Counted literally over the whole file, multi-line anchors included (bash
substring count, no Python): CH01 to CH15, PK01 to PK05 and S234's G04 occur
**exactly once** each. For every other file, every line S286 adds to or
removes from src/ was grepped against `tools/mutants/`: the only hits outside
S099's files are `return;` (inside S119's and S132's multi-line anchors) and
`raw_eval);` (S234's G04), each block counted once. So no other anchor moves
from S039's count. `tools/mutation_check.py` validates them all in phase 2.

### `test_search_params`

By reading: 74 rows in `golden_defaults`, 74 `X(` rows in
`src/search_params.hpp` (S039's 72 plus `CorrHistMax` 47 in 0..94 and
`CorrHistWeightCap` 144 in 1..256).

### Goldens PENDING phase 2

Re-derived by their scripts, each marked at its site at S039's value (no
relaxation allowance: DEC-272 and DEC-284 lapsed with S099's H0):

| golden | value now (S039's) | script |
|---|---|---|
| `tests/test_mate_carry.cpp` ceilings | 2, 5, 6, 0, 0, 1 | `adocs/data/S203_case_sweep.sh`, then `--ceilings` over S202's, S261's, S039's and S286's grids |
| `adocs/data/S170_cases.tsv` budgets | A 500000, B 100000, C 2000000, D 500000, E 300000, F 100000 | the same grid, the rule applied by script (`.tuning/coord/S099/p2/pick_budgets.sh`'s form) |
| `tests/test_mate_breadth.cpp` `EXACT_FLOOR` | 161 (162, 152) | `adocs/data/S156_mined_floor_sweep.py --floor 161` |
| `tests/test_engine.cpp` `MATE_IN_THREE_FLOOR` | 12 (13, 7) | `adocs/data/S154_floor_margin_sweep.py floor`, `red` |
| `tests/test_engine.cpp` `first_mate_depth`, `mate_in` | 9, 9; 5 | `adocs/data/S188_repair_goldens.py first-mate` |
| bench | 6100949 | `chesso bench`, the commit's `Bench:` |

Marked "expected unmoved", confirmed rather than re-derived (deviation 1):

| golden | confirmation |
|---|---|
| "ordering keeps the tree small" | `python3 adocs/data/S192_node_budget.py` reads S039's 13702 |
| R01, E21, B04 rows; `capture_mates`; LQ02 and LQ03 drive | the mutation pass: each row still in its mutant's kill list, as in `adocs/data/S039_mutation_pass.tsv` |

Not marked: `QUIET_ROOK_EVAL`, the S192 anchors, the eval-trace pins and the
tuner's gauge floor read `evaluate()` or the trace, which the correction
never enters; `DRIVE_DEPTH` and the rest of test_search for deviation 1's
reason; the book goldens and the option-line count read no search.

### What phase 2's compiler and tests should say

- **Compiler**, Release, tune and Debug at -Werror: nothing. The src lines
  are `8758622`'s, which built clean on `bacead1`'s src; S039's src changes
  (quiescence, the evaluation's stages, `LAZY_EVAL_MARGIN`'s X-row, comments)
  touch no symbol S099 uses, and `src/correction_history.hpp` reads only
  `CORRHIST_*` from `src/search_params.hpp`.
- **Format**: every hand-written line is a comment within 80 columns
  (checked); S099's lines are as clang-format-22 left them.
- `test_search_params`: 74 rows and the range case green, both builds.
- `test_evaluation`: S099's mirror case green; S039's cases unchanged.
- `test_invariants`: green, with S099's two classes counted.
- `test_engine`: S099's three key cases and the `ucinewgame` case green;
  first-mate and `MATE_IN_THREE_FLOOR` at risk (PENDING, margin 1).
- `test_search`: S099's correction suite and ProbCut store case green; every
  other case byte-identical in behaviour (no table attached).
- `test_mate_breadth`: at risk, margin 1. S099 raised both ends on S261's
  tree; a rise past 162 shipping with the weakened end above 161 would leave
  161 not separating -- the script then raises it.
- `test_mate_carry`: **the likeliest red** -- margin 0, and S099 and S039 each
  raised ceilings on their own trees. A rise stops for the owner (R1).
- `test_uci_surface`: the release binary's 5 option lines unchanged; the tune
  build gains two lines from the table.
- `test_eval_trace`, `test_tuner_gradient`: untouched.
- `bench`: moves (S099 took 5636835 to 6042933 on S261's tree); the tune
  build at `CorrHistMax` 0 prints 6100949's stream line for line but nps.

### On S039's H0

`ca321b6` reverted puts src/, tests/, tools/ and the four documents back to
`be48e02`'s (equal to `2833636`'s; nothing after `ca321b6` touched them).
Probed per file by `git merge-file` (base S039's candidate, ours `be48e02`,
theirs this tree; `.tuning/coord/S286/h0_probe.sh`):

- **src/ (nine files), `tools/datagen.cpp`, `tools/mutants/` and
  `tests/test_invariants.cpp`: clean, and the result is `8758622`'s files
  byte for byte** -- `be48e02`'s src is `8758622^`'s. Bench is then S099's
  own **6042933**, and every src-side reading S099 took on `6fc7f23` is that
  tree's.
- `tests/test_evaluation.cpp`: clean (`be48e02`'s plus S099's mirror case).
- `tests/test_search.cpp`: S099's hunks clean; the six marks meet
  `be48e02`'s GOLDEN lines (S285's R01 row at 8, S261's E21 and B04 at 10,
  the LQ block without S039's "Read at S039" paragraph): keep those lines,
  marks above them, "as on S039" read "as on `be48e02`".
- `tests/test_engine.cpp`: first-mate without S039's paragraph;
  `MATE_IN_THREE_FLOOR` **11** without S039's paragraph; marks read the
  parent's values.
- `tests/test_mate_breadth.cpp`: **159** with S261's ends (160, 156), S039's
  paragraph out, the mark.
- `tests/test_mate_carry.cpp`: `be48e02`'s file (ceilings 0, 5, 4, 0, 0, 1;
  S202's and S261's grids; its GOLDEN line still the stale 6, 15, 5, 2, 11,
  5), the mark.
- `tests/test_search_params.cpp`: S039's sentence out, S099's own "73 to 75",
  GOLDEN **75**.
- `adocs/data/S170_cases.tsv`: S261's budgets, the PENDING paragraph without
  S039's.
- `adocs/specs.md`, MANUAL.md: `be48e02`'s equal `8758622^`'s, so S099's
  hunks apply verbatim, then S286's wording: the relanded sentence, the
  census clause, the board row, the absent row; MANUAL's `RfpMinPly` row
  keeps S261's 160, 163, 156 against 159.
- DEV_MANUAL.md: `be48e02`'s golden rows with the PENDING prefixes,
  `golden_defaults` 75, `--floor 159`.
- What phase 2 should expect there: S099's grid over S202's and S261's
  answered 2, 5, 4, 0, 1, 2 on this exact src, so A, E and F rise over
  S261's 0, 5, 4, 0, 0, 1 -- R1 again; `EXACT_FLOOR` 159 -> 163 (a rise, by
  the script), `MATE_IN_THREE_FLOOR` 11 and first-mate 9, 9 as S099 read.

### Phase 2, in order, with machine time

Times are S099's and S039's recorded wall times unless marked *est.*; the
worktree has no build directories.

1. Rebase onto S039's outcome: on H1 a fast-forward over its closing
   commit's documents; on H0 the list above. No machine.
2. Configure and build `build`, `build-tune`, `build-debug`: 2 min, *est.*
   15 min configured fresh.
3. `export CLANG_FORMAT_MAJOR=22`, AGENTS.md's TESTS gate, both builds and
   the format check: 5 min. Reds expected only in `test_mate_carry`,
   `test_mate_breadth` and `test_engine`'s two goldens; any other stops.
4. `bench` and `bench 12` on the parent and the candidate; the tune build at
   `setoption name CorrHistMax value 0` then `bench`, its stream against the
   parent's line for line but nps (DEC-215); `tools/search_bench.py` at 9
   and 12 on both (`.tuning/coord/S099/p2/bench.sh`'s form): 10 min.
5. `adocs/data/S203_case_sweep.sh` into `adocs/data/S286_sweep_s170.txt`, the
   budgets by the rule by script, `--ceilings` over S202's, S261's, S039's
   and S286's grids: 5 to 15 min. A rise or a loosened budget stops (R1).
6. `adocs/data/S154_floor_margin_sweep.py floor` and `red`,
   `adocs/data/S156_mined_floor_sweep.py --floor 161`,
   `adocs/data/S188_repair_goldens.py first-mate`: 15 min. A lowered floor
   stops.
7. `python3 adocs/data/S192_node_budget.py`: seconds; 13702 expected.
8. The gate again with every golden in: 5 min.
9. Mutants, S280's recipe on a whole-tree fixture: CH01 to CH15 and PK01 to
   PK05 (1916 s and 753 s at S099), plus R01, E21, B04, S091's C02, C05,
   C07, R02 and S269's LQ02, LQ03 for deviation 1's confirmation: *est.* 60
   min.
10. Debug self-play, four rounds at 4+0.04, grepped for `Assertion`: 10 min.
11. `tools/gate_extra.sh`, five stages, the sanitizer bench equal to Release:
    17 min (1019 s at S099).
12. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`:
    seconds.
13. Writing, no machine: the PENDING marks replaced by readings; README rows
    for S286's data files; DEV_MANUAL and MANUAL rows; the stamp and the
    commit's `Bench:`.

**About 2.5 hours** of machine, under DEC-155's four: a day run once S039's
verdict frees the machine. No SPRT and no timing are owed (DEC-286; DEC-271's
cost is inside S283's verdict). No census (R2).

### Rulings asked (DEC-265 form)

**R1. May the carry ceilings and budgets land as the script answers, in
S286's commit only?** *Why*: S286 holds no relaxation allowance (DEC-272 and
DEC-284 lapsed with S099's H0), and every tree-moving step since S261 has
raised a ceiling -- S099 A, E and F on S261's tree, S039 A and C on its own.
A rise is likely, and without a standing answer phase 2 stops at item 5.
*Options*: (a) yes, DEC-287's and DEC-291's form -- ceilings and budgets as
the script gives them, in S286's commit only, phase 2 still stopping on a
non-empty `unreached`, a lowered floor or a broken first-mate precondition;
costs nothing, risks accepting more DEC-122 residue unstudied, leaves which
lines rose and why unmeasured; (b) keep the stop -- phase 2 pauses at item
5, the owner answers on the grid, about the same day; (c) an S281-shaped
study of the short lines first -- delays S286 and so S082's corpus (DEC-294)
by a step. **Recommend (a)**: the strength verdict is in (S283), the
ceilings bound a reporting residue, the owner ruled this way for S023 and
S039 (DEC-287, DEC-291), and S082's corpus waits on this landing.

**R2. Is the census re-taken?** *Why*: `CORRHIST_PAWN_ENTRIES`'s comment says
to re-run it when the search moves, and S039 moved the search; the brief
and `excludes:` say the same table. *Options*: (a) not re-taken -- the size
is part of the pair S283 measured, S127 owns the size; 0 min; a sentence
naming S286 and DEC-286 goes on the comment in phase 2; (b) taken as a
reading only, recorded, not applied -- 10 to 20 min, informs S127, but a
reading past 4096 leaves the constant contradicting its own rule on record;
(c) taken and applied -- changes the measured pair, which DEC-286 excludes.
**Recommend (a).**

### Proposed `done:` stamp (phase 1, superseded by phase 2's)

    2026-10-HH HH:MM -- S099's pawn key and pawn-keyed correction relanded
    onto S039's outcome (<kept: ca321b6's tree | H0: ca321b6 reverted>), not
    re-designed and not re-tuned: 8758622's src/ change lines byte for byte,
    4096 slots a side, CorrHistMax 47, CorrHistWeightCap 144. No SPRT owed:
    it ships on S283's H1 at 32+0.32 (ee5295b; nElo +12.56 +/- 9.89 over
    4738 games), DEC-286. bench <parent> -> <PENDING>; the tune build at
    CorrHistMax 0 prints the parent's stream (DEC-215); search_bench 9 and
    12 recorded. Goldens re-derived by their scripts (DEC-142): carry
    ceilings <..> and budgets <..> (<ruling id>); EXACT_FLOOR <..>;
    MATE_IN_THREE_FLOOR <..>; first-mate <..>; test_search's mined rows and
    node band unmoved by construction, confirmed (S192 count <..>, every
    row in its mutant's kill list). test_search_params 72 -> 74. Fast suite
    <n>/<n> in both builds, format clean; Debug self-play four rounds at
    4+0.04, <g> games, 0 Assertion; gate_extra five stages green, the
    sanitizer bench equal to Release; mutants CH01-15 and PK01-05 <k> of 20
    killed. INV-4: the key is kept by add_piece, remove_piece and
    move_piece, never recomputed. Implemented from S099's description and
    the publications it cites (DEC-221).

### Commit text (phase 1, superseded by phase 2's)

    Reland S099's pawn-keyed correction onto S039's tree (S286)

    S283 measured S099's pawn key and pawn-keyed correction H1 at 32+0.32
    (ee5295b: nElo +12.56 +/- 9.89 over 4738 games), after its H0 at the
    harness's 8+0.08 had reverted it (5a04ffa). By DEC-286 it relands, not
    re-designed and not re-tuned: 8758622's src/ change lines byte for
    byte. board_t::pawn_hash is kept by add_piece, remove_piece and
    move_piece and undone by xor (INV-2, INV-4); the table, 4096 slots a
    side, is the UCI layer's and one per datagen worker, cleared per game;
    negamax_at adds it once at its compute-or-read site, learns beside its
    store, and both stores keep the raw evaluation. CorrHistMax 47,
    CorrHistWeightCap 144. No SPRT is owed (DEC-286).

    On S039's tree the correction rides on the full evaluation: S099's
    site read evaluate() and never the lazy shortcut, and quiescence,
    S039's, stands pat on the raw score untouched. The tune build at
    CorrHistMax 0 prints the parent's bench stream (DEC-215).

    Goldens re-derived by their scripts (DEC-142): <ceilings, budgets,
    EXACT_FLOOR, MATE_IN_THREE_FLOOR, first-mate>. test_search's mined
    rows and node band attach no table and drive no UCI path, so they do
    not move: confirmed by <the S192 count and the kill lists>.
    test_search_params 72 -> 74.

    Bench: <PENDING>

## Phase 2 on `8560a36` (2026-10-15, Opus subagent)

S039 read H0 (`ca578de`, nElo -16.46 +/- 9.52) and was reverted (`74d0adf`,
ledger `8560a36`), so S286's base is the reverted tree: src/ and tools/
`be48e02`'s, which are `8758622^`'s, bench 5636835. In a new worktree,
`/home/max/ws/chesso-s286b` (branch `s286b` at `8560a36`), `s099.diff` was
re-applied with `git apply --3way` (the same exclusions as phase 1) and the
phase-1 carry-overs taken by the H0 list above; `../chesso-s286` is untouched.
Every phase-2 item ran on this machine with nothing else on it; scripts, logs
and binaries are in `.tuning/coord/S286/p2/`. No commit, no ref moved, no
stash. **No stop**: DEC-295's guard held. Where this section and the phase-1
sections above disagree, this one is the tree.

### Deviations, first

1. **src/ is `8758622`'s but for one comment**: the two-line sentence on
   `CORRHIST_PAWN_ENTRIES` naming DEC-286 and DEC-295 (the census not
   re-taken, the coordinator's half of DEC-295). Every code line is S099's;
   src/ and tools/ diffed empty against `8758622` before that edit.
2. **No PENDING mark survives**: the goldens were re-derived in this pass, so
   readings replace phase 1's marks. The six table-less sites in
   `tests/test_search.cpp` carry a "Read at S286 and unmoved, by
   construction" note with the evidence that confirmed it.
3. **Phase 1 named the wrong mutant for the extra-ply row**: R01 is S091's
   check exemption; the row's guard is S095's no-move ply, J01 and J02, which
   both fail it here. The note at the site says J01 and J02.
4. **The capture rows share one test case with FATAL REQUIREs**, so a mutant
   that fails an earlier row stops the case: C02 and C05, row 3's labels, fail
   the multicut row first. R02 fails row 3 itself, and every row's own script
   reads the parent's mate distances -- and node counts, where the script
   prints them -- (item 8), so nothing moved.
5. **Phase 1's three found-not-fixed items, fixed as the coordinator
   asked**: specs.md's "absent, search" row no longer lists razoring
   (`src/search.cpp` `negamax_at` runs it under `RAZOR_DEPTH` and
   `RAZOR_MARGIN`; the row now says razoring left it at S116);
   DEV_MANUAL.md's `EXACT_FLOOR` script column reads `--floor 163`, this
   tree's floor; `touches:` names `src/chesso.cpp` and the other files the
   change lands in. The eight S039-stale statements are S287's and are left.
6. **Fixtures**: three commit objects on no branch, built in temporary indexes
   (`.tuning/coord/S286/p2/fixture.sh`): `9aece02` (the tree with the
   parent's goldens, for S154 and S156 at 159), `138630b` (the same with 163
   compiled in, for S156's red), `4063e59` (every golden in, the mutation
   fixture). The linked worktree `.ref-builds/mut286` that held the last was
   removed after the fast check (`git worktree remove --force`, DEV_MANUAL.md's
   teardown: a worktree with a submodule refuses the plain form).
7. **The index**: every changed path is staged, the six new
   `adocs/data/S286_*` files in full; nothing is left unstaged, and
   `git diff --cached 8758622 -- src tools` shows the census comment alone.
8. A first gate run was started with `ctest -j12` and stopped before it
   finished; every gate reading below is AGENTS.md's serial `ctest`.

### Conflicts

`DEV_MANUAL.md` (the ordering and carry rows; `golden_defaults`), `adocs/specs.md`
(the search row, merged word by word with `git merge-file`, 0 conflicts) and
`tests/test_mate_carry.cpp` (the ceilings) conflicted; the parent's side was
taken, then this tree's readings written in. Everything else applied clean.
S099's goldens that merged clean -- the S170 budgets, `EXACT_FLOOR` 163, the
first-mate and mate-in-three readings -- were put back to the parent's before
the scripts ran and then re-written from the scripts' own output, never read
off S099's record.

### Readings

- **1. Builds** (`build.log`): Release, tune and Debug configured fresh and
  built, no warning, no error, 120 s.
- **2. The gate before re-derivation** (`gate1.log`): 45 of 46 in each build,
  red only `test_mate_carry` (A reports 2 short of 6 lines against its
  ceiling 0); format clean; 241 s.
- **3. What the tree does** (`bench.log`; parent `.ref-builds/be48e02`):
  `bench` 5636835 -> **6042933**, replies `c3d5 d5e6 d7c8q g7h8q d8e7 a1b2
  e5e6 e5e6` (the second was e2a6, the fifth a7a6); `bench 12` 1937969 ->
  2339074, replies unchanged. `tools/search_bench.py` depth 9 43496 /
  160453 / 30816 -> 62042 / 125888 / 34107, depth 12 180917 / 509438 /
  147594 -> 214973 / 595000 / 217107, best moves `c3d5 e2a6 d7c8q` and
  `c3d5 d5e6 d7c8q` on both. **DEC-215**: the tune build at `setoption name
  CorrHistMax value 0` prints the parent's `bench` stream, 5636835, 121
  lines, none differing but nps. Every number equals S099's on `6fc7f23`.
- **4. The grid** (`adocs/data/S286_sweep_s170.txt`, 108 cells, 1461 mate
  lines, 18 short, 275 s split six ways): cell for cell
  `adocs/data/S099_sweep_s170.txt`. `--ceilings` over S202's, S261's and this
  grid: **2, 5, 4, 0, 1, 2** against the parent's 0, 5, 4, 0, 0, 1 -- A +2, E
  +1, F +1; this grid alone 2, 1, 1, 0, 1, 2. Budgets by the rule, applied by
  `.tuning/coord/S099/p2/pick_budgets.sh`: C 2000000 -> 1200000, D 2000000 ->
  500000, F 100000 -> 300000; A, B, E unmoved.
- **5. The floors**:

  | golden | shipping | weakened | gate at `RfpMinPly` 1 | value |
  |---|---|---|---|---|
  | `MATE_IN_THREE_FLOOR` | 12 of 24 | 7 | red (7 >= 11 fails) | 11, unmoved |
  | `EXACT_FLOOR` at the parent's 159 | 164 of 318 | 160 | green at 160: no longer separating | -- |
  | `EXACT_FLOOR` at 163 | 164 | 160 | red at 160 | **163** |
  | first-mate depths | 9 and 9, mate 5 | -- | -- | unmoved |

  `adocs/data/S286_floor_sweep.log`, `adocs/data/S286_mined_floor.log`,
  `adocs/data/S286_first_mate.txt`.
- **6. DEC-295's guard**: both floors held or rose (11 held, 159 -> 163) and
  no ceiling rose by more than 2 (A 2, E 1, F 1), so the ceilings and budgets
  landed as the script answered, each moved site citing DEC-295, in S286's
  commit only.
- **7. The node band** (`adocs/data/S286_rows.log`): `S192_node_budget.py` reads
  **13446**, the parent's count, 0.83 of the pair's 16256, inside the middle
  half [7270, 32512]: the pair stays (S282's rule).
- **8. The table-less mined rows** (`adocs/data/S286_rows.log`; the raw
  files in `.tuning/coord/S286/p2/rows/`): each row's own script on S286's
  library against the parent's (`.ref-builds/be48e02`): S230's depths for
  the extra-ply row and the capture table's FENs byte-identical -- mate
  distances only, the script prints no node count; S097's sweep for the multicut row and S277's for the
  ProbCut row identical in distance and nodes at every depth, differing only
  in the wall-time column. No row moved, so none is re-mined.
- **9. Mutants** (`adocs/data/S286_mutation_pass.tsv`, the per-row kills in
  `adocs/data/S286_rows.log`; S280's recipe on fixture `4063e59`, 4005 s):
  **34 of 34 killed** -- CH01 to CH15 and PK01 to PK05, S234's G04, and the
  rows' guards J01 to J03, E21, B04, C02, C05, C06, C07, R01, R02, LQ02,
  LQ03. The row kills: J01 and J02 fail the extra-ply row; E21 (and J03, C02,
  C05, C07, R01, G04) the multicut row; B04 the ProbCut row; R02 capture row
  3; LQ02 and LQ03 their drive's two cases.
- **10. Debug self-play**, four rounds at 4+0.04: 8 games, all normal
  terminations, 0 time forfeits, 0 disconnects, **0 `Assertion`** over a
  172791-line trace log with 1234 `bestmove` lines.
- **11. `tools/gate_extra.sh`**, JOBS=12: all five stages ok in 976 s (prose,
  citations, debug 341 s, sanitize 582 s with its bench 6042933 equal to
  Release's, perft 53 s).
- **12. The gate with every golden in** (`gate3.log`): **46 of 46 in both
  builds**, format clean, 230 s, before the fast check's edits. The fast
  check's fixes are comments and documents; `tools/gate.sh --message` with
  the commit text below runs last on the staged tree, after every edit, its
  log `.tuning/coord/S286/gate_final.log`.
- **13. Prose checks**, one mode a call (`prose_*.txt`): `--prose` 0 flagged,
  `--citations` 0 flagged over 28 files (this file 5 code citations, 0
  flagged), `--touches` 0 flagged, `--params` exit 0, `--gate` agreeing;
  re-run after the fast check's edits (`.tuning/coord/S286/p2/prose2_*.txt`).

### Fast check (coordinator), five findings, fixed

1. "Node for node" overclaimed where S230's `depths` prints mate distances
   only: the extra-ply and capture-table notes in `tests/test_search.cpp`,
   their DEV_MANUAL.md rows and deviation 4 now say "distance for distance";
   the multicut and ProbCut notes say distances and node counts, which their
   sweeps print. The evidence is committed: `adocs/data/S286_rows.log` (the
   S192 count, both libraries' row readings, the per-row kills) and
   `adocs/data/S286_mutation_pass.tsv`, each with a README row, cited from
   the notes.
2. specs.md's search row cites `adocs/data/S039_floors.log` again for the
   parent's readings (159 with 160 and 156; 12 and 6 on `be48e02`).
3. The index: all paths staged (deviation 7).
4. `tools/gate.sh --message` runs last, on the staged final tree (reading
   12).
5. `decisions:` gains DEC-295 (DEC-294 was there); phase 1's stamp and commit
   text are marked superseded, as is the phase-1 section.

### Proposed `done:` stamp

    2026-10-HH HH:MM -- S099's pawn key and pawn-keyed correction relanded
    onto S039's outcome, its H0 (8560a36, ca321b6 reverted), not
    re-designed and not re-tuned: 8758622's code line for line, 4096 slots
    a side (census not re-taken, DEC-295), CorrHistMax 47,
    CorrHistWeightCap 144. No SPRT owed: it ships on S283's H1 at 32+0.32
    (ee5295b; nElo +12.56 +/- 9.89 over 4738 games), DEC-286. bench
    5636835 -> 6042933; the tune build at CorrHistMax 0 prints the
    parent's stream (DEC-215); search_bench 9 and 12 recorded. Goldens
    re-derived by their scripts (DEC-142): carry ceilings A 0 -> 2, E 0 ->
    1, F 1 -> 2 and budgets C 1200000, D 500000, F 300000 under DEC-295's
    guard; EXACT_FLOOR 159 -> 163, a rise; MATE_IN_THREE_FLOOR 11 and the
    first-mate depths 9 and 9 unmoved; test_search's mined rows and node
    band unmoved by construction, confirmed (S192 count 13446, each row's
    script reading the parent's mate distances, and node counts where it
    prints them, each row failed by its mutant).
    test_search_params 73 -> 75. Fast suite 46/46 in both builds, format
    clean; Debug self-play four rounds at 4+0.04, 8 games, 0 Assertion;
    gate_extra five stages green, the sanitizer bench equal to Release;
    mutants 34 of 34 killed (CH01-15, PK01-05, G04 and the rows' guards).
    INV-4: the key is kept by add_piece, remove_piece and move_piece,
    never recomputed. Implemented from S099's description and the
    publications it cites (DEC-221).

### Commit text

    Reland S099's pawn-keyed correction on S283's H1 (S286)

    S283 measured S099's pawn key and pawn-keyed correction H1 at 32+0.32
    (ee5295b: nElo +12.56 +/- 9.89 over 4738 games), after its H0 at the
    harness's 8+0.08 had reverted it (5a04ffa). By DEC-286 it relands, not
    re-designed and not re-tuned, onto the tree S039's H0 left (8560a36),
    whose src/ is the one 8758622 was written on: 8758622's code line for
    line. board_t::pawn_hash is kept by add_piece, remove_piece and
    move_piece and undone by xor (INV-2, INV-4); the table, 4096 slots a
    side, is the UCI layer's and one per datagen worker, cleared per game;
    negamax_at adds it once at its compute-or-read site, learns beside its
    store, and both stores keep the raw evaluation. CorrHistMax 47,
    CorrHistWeightCap 144; the census is not re-taken (DEC-295). No SPRT is
    owed (DEC-286). The tune build at CorrHistMax 0 prints the parent's
    bench stream (DEC-215).

    Goldens re-derived by their scripts (DEC-142): test_mate_carry's
    ceilings A 0 -> 2, E 0 -> 1 and F 1 -> 2, with S286's grid added to
    the command, and the S170 budgets C 1200000, D 500000 and F 300000,
    applied as the script answers under DEC-295's guard -- both mate floors
    held or rose and no ceiling rose past 2; test_mate_breadth's
    EXACT_FLOOR 159 -> 163, a rise, the correction having raised both ends
    to 164 and 160; MATE_IN_THREE_FLOOR 11 and the first-mate depths 9 and
    9 unmoved. test_search's mined rows and node band attach no table and
    drive no UCI path: unmoved, confirmed by the S192 count (13446) and
    each row's script reading the parent's mate distances, and node counts
    where it prints them. test_search_params 73 -> 75. specs.md's
    absent-search row stops listing razoring and names the correction
    tables S110 and S111 still hold back.

    Bench: 6042933
