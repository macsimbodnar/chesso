id:         S259
goal:       the S097 multicut row and the S113 ProbCut row in "pruning does not hide a forced mate" separate their guard mutants E21 and B04 again on today's tree
accepts:    a targeted `tools/mutation_check.py --only E21` and `--only B04` on HEAD each read killed, and the kill is observed at the row named for that guard (or, if another case already kills the mutant, that is recorded and the row is still re-mined or the decision not to is recorded); every moved row is re-derived by the mining script its site names (DEC-142), with the mutant observed red first and the shipped build green; the second tier's mutant clause (DEC-141) holds for both guards
touches:    tests/test_search.cpp (the two rows and their golden comments), adocs/data/ for the re-mine's log and targeted tsv, tools/mutants/ only if an anchor is stale
excludes:   any change to src/; any change to the multicut or ProbCut rules themselves
decisions:  DEC-141, DEC-142, DEC-171
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S259_brief.md), 2026-10-13
done:       2026-10-13 -- both mined mate rows separate their guard mutants again on `338705e`, the tree S119's H1 left. First move: `--only E21 B04` on HEAD killed each by its direct case alone (S243's case; "probcut does not run with beta in the mate band"); neither row red. Re-derived by their own scripts (DEC-142), S097's stage 1 not re-run and S113's `fens` run: the multicut row (`S097_mine_mate_row.py`, guard mode, DEC-238, and default mode) back to S114 verdict 2's `4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4 42`, depth 14, mate in 6; the ProbCut row (`S113_mine_mate_row.py`, default mode, its header's; guard-mode witness for the record by `S259_b04_fires_guard.py`) to `4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73`, depth 11, mate in 6. Oracle-labelled, red under each mutant at its row and green shipped. After: `--only E21 B04` on fixture `0e79ba6` kills both at their rows and by their direct cases (DEC-141 clause 2 holds). No `src/` change. `DEV_MANUAL.md`'s two golden rows updated (not in `touches:`). Fast suite 45/45 in both builds, format clean, prose checks clean. Evidence `adocs/data/S259_remine.log`, `adocs/data/S259_mutation_targeted.tsv`, `adocs/data/S259_b04_fires_guard.py`. Fast check: goldens clean; three record items (the guard witness scripted and re-run 4 of 4, the data index, stage-1 wording) fixed before commit. The `--mode` filler is S277.

## Why this exists (2026-10-06, the coordinator, from S120's finding 9)

S120's implementer re-checked the mined mate rows on the parent `1eaa776`:
the S097 row (`mate_the_multicut_hides`, depth 14) and the S113 row
(`mate_probcut_hides`, depth 11) no longer separate their mutants. E21
(`E21_multicut_mate_band_gate_dropped`) finds the S097 row's mate at d11,
d13 and d14 like the shipped build, and B04
(`B04_probcut_defender_gate_dropped`) reads the S113 row exactly as shipped
(d9, d11 to d14). Both cases stay green, so nothing is red today, but neither
row is a guard on this tree any more. The raw runs are machine-local under
`.tuning/coord/S120_files/rows/`, and `rowcheck.sh` there re-runs them.

S188 met the same thing on this row once and wrote the lesson down: a mined
row is a property of the tree it was mined on. The test's comment block above
`mate_the_multicut_hides` records every re-mine so far.

A test-strength defect that reaches no play: filler under DEC-171, behind the
next strength step, and named as an open finding in every run's
pre-registration until it closes.

## First move

Run the targeted mutation check for both mutants on HEAD before mining
anything. If the whole fast suite already kills them, the guards are still
guarded and the question is only whether the named rows are re-mined. If
either survives, a guard has no test: re-mine its row first.

## As built (2026-10-13, the Opus subagent)

### Deviations, first

- **`DEV_MANUAL.md` is touched and `touches:` does not name it.** Its DEC-142
  golden table states each row's position, distance and profile
  (`mate_the_multicut_hides`, `mate_probcut_hides`), so a re-mine that moves
  them moves those two rows (DOCS rule). Each gains its S259 sentence and its
  evidence path; nothing else in the file changed. `MANUAL.md` and
  `adocs/specs.md` name neither row: checked, no change.
- **B04's pick took its witness in the default mode.** `S113_mine_mate_row.py`
  has no `--mode`; its header's six commands are what ran, and DEC-238's
  consequence asks a guard row's re-derivation to state its mode, so this one
  does. For the record only, the same witness was also taken in guard mode on a
  tune library built with B04 applied, by `adocs/data/S259_b04_fires_guard.py`
  (new): it imports S113's module, so the driver is re-pointed to `ProbCut`,
  and calls S097's `cmd_fires` with `mode="guard"`. Neither miner was edited,
  `adocs/data/` stays append-only. Its first run was an inline call left in the
  log as a placeholder line (fast check); the script re-ran it and every
  moved-depth list and `mate:nodes` cell matched the first run, 4 of 4
  positions (`.tuning/coord/S259/rerun_b04_guard.sh`). Both witnesses admit the
  same four separators, so the pick does not depend on the mode. **Proposed
  filler**: give `S113_mine_mate_row.py`'s successor a `--mode` passed through
  to S097's `fires` and `pick`, as DEC-238 gave S097's, so the next ProbCut
  re-mine takes its guard witness from the miner's own header.
- **The mutants were built out of tree in detached worktrees**
  (`.ref-builds/obs_e21`, `.ref-builds/obs_b04` at `338705e`), the mutant
  applied by `tools/mutation_check.py`'s own loader, rather than applied to
  this worktree's `src/` and reverted as the miners' headers phrase it. Same
  libraries; the step's `src/` was never touched.
- **The after-run fixture is a commit object on no branch**, `0e79ba6`
  (`338705e` plus this step's `tests/test_search.cpp`, through a temporary
  index), S252's form. No ref moved.

### The first move: `--only E21 B04` on HEAD

Fixture `.ref-builds/mut` at `338705e`, clean; baseline green, 45 tests, bench
3872032; 359 s. **Both killed, each by its direct case alone**, neither mined
row red:

- `E21_multicut_mate_band_gate_dropped` -- killed, fast 1/45, bench same:
  "the multicut never ends a node on a mate from its verification",
  `CHECK( last_score < MATE_MIN_LOCAL )` and `REQUIRE( !record.se_multicut )`
  (S243's case).
- `B04_probcut_defender_gate_dropped` -- killed, fast 1/45, bench same:
  "probcut does not run with beta in the mate band",
  `REQUIRE( !record.probcut_entered )`.

So neither guard was without a test; the rows had gone quiet, which is
`DEV_MANUAL.md`'s named sign (a kill list shrunk to the direct case). Both
rows were re-mined, the step's goal. On `338705e` the old rows read:

| row | shipped | mutant |
|---|---|---|
| S131's multicut row `7k/5p1p/p2p1N2/2p2P2/4P3/1r3n1P/3K2R1/6R1 w - - 2 42`, d11..d14 | m6 at 11, 13, 14 | E21: m6 at 11, 13, 14 |
| S113's ProbCut row `5r2/2Q3k1/3p2p1/7p/p6P/P1P2q1b/5B1K/8 b - - 0 44`, d9..d12 | m5 at 9, 11, 12 | B04: m5 at 9, 11, 12 |

### The rows re-mined

Stages 2 to 6 of each script's header on `338705e`. S097's stage 1 was not
re-run: the 269 of `adocs/data/S097_candidates.tsv` are the corpus and the
oracle. S113's stage 1, its `fens`, ran and wrote them out as the list both
miners swept, byte-identical to S114 verdict 2's list. Four sweeps in parallel, 07:41 to
07:51, about 10 minutes. Evidence: `adocs/data/S259_remine.log` (libraries and
their hashes, every stage, the witness files, the separators' cells, the old
rows' cells, the oracle, the red/green runs).

- **Multicut row, `adocs/data/S097_mine_mate_row.py`, `--mode guard` and the
  default mode for the record.** 2 of 269 separate; both modes take
  `4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4 42` -- **S114 verdict 2's own
  row, back** -- at **depth 14, mate in 6** (depth and distance unchanged,
  position moved): shipped `d12 d13 d14`, E21 `d12 d13`, run 3; the
  mutant's multicut moves the tree at 13 and 14 (the shipped one too). Cell
  2191754 nodes, 308 ms. The other separator,
  `5R2/2p2R1p/7k/n7/8/1p1B4/3q2PP/7K w - - 6 42`, run 2 at 13.
- **ProbCut row, `adocs/data/S113_mine_mate_row.py`, default mode as its header
  has it.** 4 of 269 separate: one moves only the distance (d12 #5 -> #6, not
  taken), and the rule takes `4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73` at
  **depth 11 (unchanged), mate in 6 (was 5)**: shipped `d11 d12`, B04 `d12`,
  run 2, tied with `2r4r/kq3pb1/N3p1p1/QPp1Pn1p/2PPRP2/7P/5BP1/R5K1 w - - 1 31`
  and taken on the cheaper cell, 768752 nodes and 87 ms against 6913112 and
  904. The default witness moves the tree at 9, 10 and 12 and **not at 11**,
  the row's own depth; the guard-mode witness moves it at 9 to 12 -- at 11 the
  shipped ProbCut leaves the tree as it is and B04's loses the mate.
- **Oracle (CHESS), not chesso**: python-chess and stockfish at depth 20 in a
  fresh process (S230's `line_facts`). Multicut row: in check, 4 legal moves,
  `#+6` for Black, pv `Kg4 Qe6+ Kxg3 Qg8+ Ng4 Qxg4+ Kxg4 h5 Kg3 c4 Qf1#`. ProbCut
  row: not in check, 30 legal moves, 3 captures, `#+6` for White, pv `h7 Re3+
  Kg4 Re4+ Kh5 Rh4+ Kxh4 Nf3+ Kg3 Nd2 h8=Q#`. Each pv ends in checkmate by
  python-chess, and each equals the committed label.
- **Mutant red first, shipped green** (the S033 protocol in the throwaway
  worktrees, `./test_search --test-case="pruning does not hide a forced
  mate"`): shipped 57/57 assertions green. E21: red at `REQUIRE(
  result.mate_found )`, logged `mate the multicut hides, depth 14`, 40 of 41
  passed before it. B04: red at `REQUIRE( result.mate_found )`, logged `mate
  probcut hides, depth 11`, 43 of 44 passed before it -- the new multicut row
  holds under B04, so B04 reaches its own row in the suite (S113 needed a
  non-fatal copy for that; not needed here).
- In `tests/test_search.cpp` both GOLDEN blocks gain an S259 paragraph and the
  old rows stay quoted in them (the multicut block's table of rows gains the
  S259 entry and names S131's row explicitly). The ProbCut GOLDEN line's
  distance is 6. Case cost: the new cells are 0.31 s and 0.09 s against 0.21 s
  and 0.08 s for the old rows on this tree.

### After: `--only E21 B04` on the fixture with the new rows

Fixture `0e79ba6`, clean; baseline green, 45 tests, bench 3872032; 348 s.
**Both killed at their named rows and by their direct cases**:

- E21: "pruning does not hide a forced mate", `REQUIRE( result.mate_found )`,
  logged `mate the multicut hides, depth 14`; and S243's case.
- B04: "pruning does not hide a forced mate", `REQUIRE( result.mate_found )`,
  logged `mate probcut hides, depth 11`; and "probcut does not run with beta
  in the mate band".

Both runs: `adocs/data/S259_mutation_targeted.tsv` (two blocks, the fixture
named above each).

### Second tier (DEC-141)

- Clause 2, the mutant clause: **holds for both guards**. Each has a direct
  guard case that kills its mutant on `338705e` and on the fixture, and each
  mined row is a second witness again.
- Clause 1 (Debug self-play) and clause 3 (`tools/gate_extra.sh`) do not
  apply: no `src/` change, no new rule.

### Suites

- `cmake --build build -j12 && ctest --test-dir build -L fast
  --output-on-failure && cmake --build build-tune -j12 && ctest --test-dir
  build-tune -L fast --output-on-failure && ./clang-format.sh --check` with
  `CLANG_FORMAT_MAJOR=22`: **release 45/45, tune 45/45, format clean**, 236 s.
- `tools/plan_prose_check.py`, one mode per call, all exit 0: `--citations`
  0 flagged over 34 files; `--touches` 0 flagged over 34 files (4 symbols
  read); `--params` silent.

### Proposed `done:` stamp

2026-10-13 -- both mined mate rows separate their guard mutants again on
`338705e`, the tree S119's H1 left. First move: `--only E21 B04` on HEAD killed
each by its direct case alone (S243's case; "probcut does not run with beta in
the mate band"); neither row red. Re-derived by their own scripts (DEC-142),
S097's stage 1 not re-run and S113's `fens` run: the multicut row
(`S097_mine_mate_row.py`, guard mode, DEC-238, and default mode) back to S114
verdict 2's `4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4 42`, depth 14, mate in
6; the ProbCut row (`S113_mine_mate_row.py`, default mode, its header's;
guard-mode witness for the record by `S259_b04_fires_guard.py`) to `4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73`, depth
11, mate in 6. Oracle-labelled, red under each mutant at its row and green
shipped. After: `--only E21 B04` on fixture `0e79ba6` kills both at their rows
and by their direct cases (DEC-141 clause 2 holds). No `src/` change.
`DEV_MANUAL.md`'s two golden rows updated (not in `touches:`). Fast suite 45/45
in both builds, format clean, prose checks clean. Evidence
`adocs/data/S259_remine.log`, `adocs/data/S259_mutation_targeted.tsv`,
`adocs/data/S259_b04_fires_guard.py`.

### Commit text

    Re-mine the multicut and ProbCut mate rows (S259)

    On 338705e, the tree S119's H1 left, neither mined row of "pruning
    does not hide a forced mate" separated its guard mutant: a targeted
    mutation_check killed E21 and B04 by their direct cases alone. Both
    rows re-derived by their own scripts (DEC-142); old rows kept in the
    GOLDEN blocks.

    Multicut (S097 miner, guard mode, DEC-238): S114 verdict 2's row
    4Q3/p7/2p2p2/P3n2k/7P/2P3P1/5q2/7K b - - 4 42 returns, depth 14,
    mate in 6. ProbCut (S113 miner): 4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8
    w - - 3 73, depth 11, mate in 6. Each observed red under its mutant
    at its row and green shipped; mutation_check on a fixture of this
    tree kills both at the rows and by the direct cases (DEC-141).

    DEV_MANUAL's two golden rows follow. Closes S120's finding 9, open
    under DEC-171. No src/ change, so no Bench line.
