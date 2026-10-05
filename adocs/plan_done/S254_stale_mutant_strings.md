id:         S254
goal:       every mutant whose `old` string names `is_check_move` matches the tree again, so `tools/mutation_check.py` applies it instead of refusing it
accepts:    `tools/mutation_check.py` applies every mutant in `tools/mutants/` on today's tree with no "old string not found" refusal; S091's C02 and R01, `tools/mutants/S109_shallow_pruning.py` and `tools/mutants/search.py` read killed or survived (a survivor named as a finding), not refused
touches:    tools/mutants/*.py
excludes:   any engine or test change beyond a case a survivor calls for, which is its own step
decisions:  DEC-141, DEC-171
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05 -- every mutant in tools/mutants/ validates on 636bbf4 (173 of 173, no anchor refused; 5 refused before). The five stale anchors named `!is_check_move` where S020 made it the call `!is_check_move()`; each `old`/`new` pair now names the call and the mutation is the same edit as before. `--only C02 R01 P05 M08 L06` on a fixture of HEAD: 5 of 5 killed, bench moved on all five, wall 608 s. Found: HEAD has been red on test_plan_touches since 4a7e8ce (S255's goal names LAZY_EVAL_MARGIN and its touches named no file carrying it); S255's touches line widened to name src/search_params.hpp as read-only, which the mutation fixture also carried. Both fast suites 41 of 41, clang-format clean. DEV_MANUAL's stale M08 `(void)` sentence corrected; MANUAL.md checked, no change. No src change. The whole list (about 4.8 h) not run: over DEC-155's line.

## Why this exists (2026-10-05, the coordinator, from S055's report)

S020 turned `is_check_move` into a memoised call, `is_check_move()`, and the
mutants' `old` strings were not updated: S091 C02/R01,
`tools/mutants/S109_shallow_pruning.py` and `tools/mutants/search.py` no
longer match, so `mutation_check.py` would refuse them and DEC-141's mutant
guard is silently thinner. S055's sweep driver applied C02 and R01 with the
call form by hand. A tooling defect reaching no play: a filler (DEC-171).

## As built (2026-10-05, Opus subagent)

### Refusals, before and after

`mutation_check.validate` over `load_mutants(['tools/mutants'])` against the
main tree at `636bbf4` (src clean), 173 mutants:

| before | cause |
|---|---|
| `C02_capture_gives_check` (S091_capture_see.py) | pair 0 anchor occurs 0 times in src/search.cpp |
| `R01_extra_reduction_gives_check` (S091_capture_see.py) | same |
| `P05_prune_gives_check` (S109_shallow_pruning.py) | same |
| `M08_lmr_checks` (search.py) | same |
| `L06_lmr_root` (search.py) | same |

All five for one cause: the anchor wrote `!is_check_move`, the tree since
S020 (`5ad8837`) writes `!is_check_move()`. No refusal of any other cause.
After: 173 of 173 validate, none refused.

### What changed

- `tools/mutants/S091_capture_see.py`, `tools/mutants/S109_shallow_pruning.py`,
  `tools/mutants/search.py`: `!is_check_move` becomes `!is_check_move()` in
  both `old` and `new` of the five pairs. Each mutation is the edit it was:
  C02 drops `!capture_gives_check` from the skip, R01 drops it from
  `may_reduce`, P05 drops `!is_check_move()` from the skip, M08 drops it from
  `may_reduce`, L06 drops `ply > 0`. With the memoised lambda, dropping one
  of its two readers leaves the other, so none needs a `(void)` (and the
  lambda is not a variable whose last use goes, in any case).
- `DEV_MANUAL.md`, the mutation check's Release paragraph: it said "the two
  mutants in that class -- `M08` inserts `(void)is_check_move;`". M08 has
  carried no `(void)` since S091 (`d785b89`) gave the symbol a second reader,
  and the class has many members now; the sentence says so.
- `adocs/plan_todo/S255_anchor_clamp_margin.md`, `touches:` only: found red.
  `test_plan_touches` failed on `636bbf4` in both the fixture and the main
  `build/` -- S255's goal names `LAZY_EVAL_MARGIN`, its touches named only
  `adocs/data/S192_anchors.py`, which does not carry the symbol. The step
  reads the margin from `src/search_params.hpp`, so the line now reads
  `adocs/data/S192_anchors.py; src/search_params.hpp (read, never written)`.
  A one-line repair of another step's file, made because the gate cannot be
  green without it; the coordinator may word it otherwise.

### Mutation run

The first launch on a fixture of `636bbf4` refused at the baseline, red on
`test_plan_touches` (`.tuning/S254/mutation/mutation_refused_red_baseline.log`,
`logs_refused/`). The second ran on fixture `7db1825`, a commit object on no
branch made through a temporary index: `636bbf4` plus the S255 touches line
and nothing else. The mutant list was read from the main tree (dirty with
this step's three files, as the header says).
`python3 tools/mutation_check.py tools/mutants .ref-builds/mut --only C02 R01 P05 M08 L06 --jobs 12`,
`CLANG_FORMAT_MAJOR=22`. Baseline green, 41 tests, bench 4081329.

| id | fast | bench | verdict | s |
|---|---|---|---|---|
| C02_capture_gives_check | 2/41 | moved | killed | 110.8 |
| R01_extra_reduction_gives_check | 2/41 | moved | killed | 105.3 |
| P05_prune_gives_check | 4/41 | moved | killed | 97.2 |
| M08_lmr_checks | 4/41 | moved | killed | 91.6 |
| L06_lmr_root | 3/41 | moved | killed | 90.4 |

Score 5 of 5, wall 608 s; no survivor. Direct guards among the kills: C02 by
"a capture that gives check is not pruned", R01 by "a capture that gives
check is not reduced", P05 by "a quiet move that gives check is not pruned",
M08 by "a quiet move that gives check is not reduced", L06 by "a mate found
at the root is never reduced". Logs: `.tuning/S254/mutation/mutation.log`,
`.tuning/S254/mutation/logs/` (`results.tsv`, per-mutant build and ctest).

The whole list was not run: 173 mutants at about 100 s each is about 4.8 h,
over DEC-155's four-hour line, and the other 168 anchors did not move here.
The fixture worktree was removed after the run.

Fixture note: `git submodule update --init tests/doctest` in a fresh
worktree fails on this machine (the `.gitmodules` URL is SSH and no key
answers); pointing `submodule.tests/doctest.url` at the main tree's checkout
and passing `-c protocol.file.allow=always` worked.

### Gate

`cmake --build build -j8 && ctest --test-dir build -L fast` 41 of 41
(102.3 s), the same in `build-tune` (104.1 s), `./clang-format.sh --check`
clean, `CLANG_FORMAT_MAJOR=22` (`.tuning/S254/gate.log`). No src change, so
no bench and no second tier. `MANUAL.md` names no mutant: no change.
