id:         S285
goal:       the extra-ply mate row ("mate the extra ply hides" in tests/test_search.cpp) separates again on the tree it guards, re-mined by S095's procedure
accepts:    (1) on the current tree the row's cells read, for the shipped build and the build with S095's guard opened, the depths at which each finds the mate, recorded, showing the row no longer separates (the precondition, observed, not assumed); (2) the row re-mined by `adocs/data/S095_mine_mate_row.py` over its candidate list, the script's pick taken as given, never chosen by hand, observed green shipped and red with the guard opened; if no candidate separates in the row's range, one band deeper by DEC-288, and none there stops for a ruling; (3) its GOLDEN block names the script, the range and this step; the mate distance confirmed by Stockfish at depth 20 through python-chess per TOOLCHAIN.md; (4) S095's mutant killed by the new row in the mutation recipe; the fast suite green in both builds; the log under `adocs/data/` with a README row
touches:    tests/test_search.cpp (the row and its GOLDEN block), adocs/data/S285_remine_extra_ply.log, adocs/data/README.md
excludes:   any change to search; any other mined row
decisions:  DEC-142, DEC-171, DEC-288
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent, on fd33954
done:       2026-10-15 08:20 -- the extra-ply row re-mined on fd33954 (DEC-142, DEC-288; filler under DEC-171). Precondition observed: the old row 4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1 reads `2 2 2 2 - 2 2 - 2 -` shipped against `2 2 2 2 - 2 - 2 2 2` with S095's guard opened over depths 3..12, both finding the mate at its depth 8. S095's procedure over the 141 candidates at 3..12, 22 separators, `pick` takes k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80 at 8, mate in 4, shipped d7-d12 against the opened build's d7 d9-d12. This is the position S023 took on its candidate, returned by the script on this tree. Stockfish depth 20 through python-chess #+4. Observed green shipped and red with the guard opened, at the row, on fixture a78be44. Mutation recipe --only J01 J02 J03: 3 of 3 killed, this row the first failing assertion for J01 and J02, J03 killed by the multicut row and its direct case. GOLDEN block names the script, the range 3..12, S285 and DEC-288. DEV_MANUAL's golden row follows. Fast suite 46/46 in both builds, format clean, five prose modes clean, bench 5636835 (no src/ change). Evidence adocs/data/S285_remine_extra_ply.log. Accepts (4)'s "S095's mutant" read by the coordinator as J01 and J02, the rule's own site, J03 narrowing the condition and killed by its direct case; DEV_MANUAL's golden row updated though outside touches (in scope). Fast check: no defect; gated on the landed tree. Implemented by an Opus subagent; committed by the coordinator.

## Why this exists

S023's phase 2 (2026-10-14) found the extra-ply row stale already on the
parent, S261's engine: the shipped build and the build with S095's guard
opened both find its mate at its depth, so the row guards nothing. S023
re-mined it inside its candidate `adb5d78`; S023 verdict 1 read H0 and the
revert put the stale row back. A test defect, not reachable in play, no
score, move or line: filler under DEC-171, scheduled behind the next
strength step and named in every pre-registration while open. S023's
re-mine (`adocs/data/S023_remine_extra_ply.log`) is the model, run on the
reverted tree.

## As built (2026-10-15, the Opus subagent)

Worktree `/home/max/ws/chesso-s285`, branch `s285` at `fd33954`,
uncommitted. Evidence under `.tuning/coord/S285/` in the main checkout
(gitignored): `xp.sh` and `xp.log` (fixture, libraries, sweeps, pick; outputs
in `xp/`), `oracle.txt`, `fixture.sh` and `fixture.sha`, `obs.sh` and
`obs.log`, `mut.sh` and `mut.log` (per-mutant ctest logs in `mutation/`),
`gate.sh` and `gate1.log` to `gate3.log`. Scratch worktrees `.ref-builds/xp` and
`.ref-builds/mut` removed.

### Deviations and open points, first

1. **Which of S095's mutants accepts (4) means is not settled by the step.**
   `tools/mutants/S095_no_tt_move.py` holds three and none is the miner's
   mutation (the guard opened, `= true`). All three were run. The new row is
   the first failing assertion for `J01_no_tt_move_inverted` and
   `J02_no_tt_move_dropped`. `J03_site_entry_absent_only`, the guard site
   narrowed to `tt_entry == nullptr`, is killed, but not by this row: the
   case runs past it and dies on the multicut row, line 4642, and on J03's own
   direct case. If accepts (4) means J03, this row does not meet it.
2. `DEV_MANUAL.md`'s golden row for `mate_the_extra_ply_hides` is updated,
   though `touches:` does not name it: left alone it would call the old row
   current (DOCS; S259 updated its rows the same way).
3. The fixture for the observation and the mutation run is `a78be44`, a
   commit object on no branch made with `git commit-tree` through a temporary
   index (S254, S268 and S269's method). No ref moved.
4. The script picked the position S023 had taken on its candidate. It was
   not reused: the whole procedure ran on this tree and `pick` returned it.

### The precondition, observed

The old row, `4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1`, depths 3..12,
S230's driver against `libchesso_engine.a` of a clean worktree of `fd33954`:

| build | d3 .. d12 |
|---|---|
| shipped | `2 2 2 2 - 2 2 - 2 -` |
| guard opened | `2 2 2 2 - 2 - 2 2 2` |

Both builds report the mate at the row's depth 8, so the row no longer
separates. The cells match the ones S023's log gives for `7e36271`, whose
`src/` this tree has.

### The re-mine

The 141 FENs of `adocs/data/S095_candidates.tsv`, byte-identical to
`.tuning/coord/S095_candidates.fen`, swept 3..12 on both libraries, then
`S095_mine_mate_row.py pick`. 22 separate in the row's own range, so no
deeper band (DEC-288). The script took `k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80`
at **depth 8**: shipped `d7 d8 d9 d10 d11 d12` (mate 4 at each), guard opened
`d7 d9 d10 d11 d12`, longest run 6. Stockfish dev-20260810-5062aee5 through
python-chess, fresh process, depth 20: `#+4`, 11292 nodes, `Kc6 Rc3+ Qxc3 d2
Qg7 d1=R Qb7#`, matching the candidates file.

### Observed

On a clean worktree of `a78be44`, `./test_search --test-case="pruning does not
hide a forced mate"`: guard in place, 57 of 57 assertions pass. Guard opened:
`FATAL ERROR: REQUIRE( result.mate_found )` at the row, `logged: mate the
extra ply hides, depth 8` (the run's own line is in the log). Guard restored,
the worktree clean again: 57 of 57 pass.

### Mutation recipe

DEV_MANUAL.md's recipe with S280's submodule line, fixture `a78be44` clean,
`tools/mutants` clean, `--only J01 J02 J03`. Baseline green over 46 tests,
bench 5636835. **3 of 3 killed**, each at fast 2/46 with the bench moved.
First fatal assertion in "pruning does not hide a forced mate": J01 and J02 at
4335, this row; J03 at 4642, the multicut row.

### Gate

`cmake --build build -j8 && ctest --test-dir build -L fast
--output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
build-tune -L fast --output-on-failure && ./clang-format.sh --check` with
`CLANG_FORMAT_MAJOR=22`: 46 of 46 in each build, format clean, exit 0.
`plan_prose_check.py` `--prose`, `--citations`, `--touches`, `--params` and
`--gate` all exit 0. `chesso bench` 5636835, unchanged, with no `src/` change.
That is `gate3.log`. `gate2.log`, run after this section was first written,
was red on `test_plan_citation_freshness` and `--citations` alone: this file
cited the red line in path-and-line form (DEC-135). The citation now names the
row, and the gate was re-run.
Not run: Debug self-play, `gate_extra.sh`. The step touches no `make_move`,
generator or search code, so DEC-141's second tier does not apply.
`MANUAL.md` checked and unchanged (no UCI surface moves). `specs.md` unchanged
(no behaviour change).

### Proposed `done:` stamp

```
done:       2026-10-15 HH:MM -- the extra-ply row re-mined on fd33954 (DEC-142, DEC-288; filler under DEC-171). Precondition observed: the old row 4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1 reads `2 2 2 2 - 2 2 - 2 -` shipped against `2 2 2 2 - 2 - 2 2 2` with S095's guard opened over depths 3..12, both finding the mate at its depth 8. S095's procedure over the 141 candidates at 3..12, 22 separators, `pick` takes k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80 at 8, mate in 4, shipped d7-d12 against the opened build's d7 d9-d12. This is the position S023 took on its candidate, returned by the script on this tree. Stockfish depth 20 through python-chess #+4. Observed green shipped and red with the guard opened, at the row, on fixture a78be44. Mutation recipe --only J01 J02 J03: 3 of 3 killed, this row the first failing assertion for J01 and J02, J03 killed by the multicut row and its direct case. GOLDEN block names the script, the range 3..12, S285 and DEC-288. DEV_MANUAL's golden row follows. Fast suite 46/46 in both builds, format clean, five prose modes clean, bench 5636835 (no src/ change). Evidence adocs/data/S285_remine_extra_ply.log. Implemented by an Opus subagent; committed by the coordinator.
```

### Commit text

```
Re-mine the extra-ply mate row on the reverted tree (S285)

S023's revert put back the extra-ply row it had found stale. On
fd33954 the old row still finds its mate at depth 8 with S095's guard
opened, so it guards nothing. Re-derived by its own script (DEC-142):
S095's procedure over the 141 candidates at depths 3..12, 22
separators, no deeper band needed (DEC-288). The pick is
k7/6Q1/3K4/pp6/1P6/r2p4/8/8 w - - 2 80 at depth 8, mate in 4, the
position S023 took on its candidate, returned here by the script.
Stockfish at depth 20 agrees.

Observed red with the guard opened and green with it in place. In
mutation_check, the row is the first failing assertion for J01 and
J02; J03 is killed by the multicut row and its direct case.
DEV_MANUAL's golden row follows. Filler under DEC-171.
No src/ change.
```
