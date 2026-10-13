id:         S277
goal:       the ProbCut mate-row miner takes a `--mode` as S097's multicut miner does (DEC-238), so its next re-mine reads the guard-mode witness from its own header
accepts:    (1) `adocs/data/S113_mine_mate_row.py`'s successor (a new script; the S113 evidence is never edited) passes `--mode guard|default` through to S097's `fires` and `pick`, default unchanged; (2) run in guard mode on the tree S259 mined, it reproduces `adocs/data/S259_b04_fires_guard.py`'s four moved-depth lists and `mate:nodes` cells exactly, and the pick is S259's row; (3) the ProbCut row's GOLDEN comment in `tests/test_search.cpp` and `DEV_MANUAL.md`'s golden row name the new script (DEC-142); no assertion changes
touches:    adocs/data/ (the new miner), tests/test_search.cpp (one GOLDEN comment), DEV_MANUAL.md (one golden row), adocs/data/README.md
excludes:   any change to src/, to the ProbCut rule, or to the row itself
decisions:  DEC-142, DEC-171, DEC-238
closes:
blocks:
paused_by:
author:     Opus subagent briefed by the coordinator (.tuning/coord/S277_brief.md), 2026-10-13
done:       2026-10-13 -- `adocs/data/S277_mine_mate_row.py`, S113's ProbCut miner imported with S097's `--mode` (`rule`, the default, or `guard`, DEC-238) passed through to `fires` and `pick`, plus two guard-mode refusals: the shipped tune library by its resolved path, and a `pick` with no witness or an empty one. On `338705e`'s libraries, rebuilt from an export, guard mode reproduces `S259_b04_fires_guard.py`'s four moved-depth lists and `mate:nodes` cells exactly and takes S259's row, `4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73`, depth 11, mate in 6; the default mode matches S113's witness and prints S113's pick byte for byte. Evidence `adocs/data/S277_repro.log` (not in `touches:`). The ProbCut row's GOLDEN block and `DEV_MANUAL.md`'s row name the new script and its guard mode (DEC-142); no assertion changed, no `src/` change. Fast suite 45/45 in both builds, format clean, prose checks clean. `--mode` values are S097's names, not the step's `guard|default`. Fast check: nothing blocking; three items fixed before commit (an unbacked "two builds" claim dropped, the tune-library refusal by resolved path, the empty-witness refusal), refusals 7 of 7 re-run on the final miner.

## Why

S259 re-mined the ProbCut row with `S113_mine_mate_row.py`, which has no
`--mode`; its default-mode witness does not move the tree at d11, the row's own
depth -- the case DEC-238 gave S097's miner a guard mode for. S259 took the
guard-mode witness by calling S097's `fires` through S113's module in a
one-off script, `adocs/data/S259_b04_fires_guard.py`. The next re-mine should
not need a one-off. Proposed in S259's As built; filed by the coordinator.

## Lane

Agent work, tooling and one comment; minutes of machine. Filler under DEC-171:
a test-tooling gap that reaches no play.

## As built (2026-10-13, the Opus subagent)

### Deviations, first

- **`--mode` takes S097's values, `rule` and `guard`, `rule` the default.**
  The step writes `guard|default`; "default" is the default mode, which S097
  names `rule`, and a pass-through takes S097's choices. Nothing else reads
  the name.
- **Two guard-mode checks the step does not name**, each exit 2; the
  default mode reaches neither. (a) `pick --mode guard` with no `--fires`, or
  with a witness holding no data rows: S113's `pick` takes `--fires` as
  optional, and S097's reads a missing or empty witness as no firing
  condition at all, so a guard-mode pick could otherwise run with no witness
  behind it. (b) `fires --mode guard` on the shipped tune library by the
  file `--lib` resolves to: S097 compares `normpath(lib)` with the relative
  `build-tune/src/libchesso_engine.a`, so an absolute path to the same file
  passed. The empty-witness half of (a) and all of (b) came from the fast
  check; both gaps were shown open on S097 alone first (the log's last
  section: S097 takes the row with witness `[]`; its comparison reads
  False).
- **A second file under `adocs/data/`, `S277_repro.log`**, the run accepts
  (2) rests on, S259's form; `touches:` names only the miner there. Indexed
  in `adocs/data/README.md` beside it.
- **The GOLDEN edit is a short paragraph, not a rename.** It names the new
  script, says its stages 5 and 6 run in guard mode since S277, and says
  S113's script derived every row so far in what is now the default mode
  (DEC-238: a guard row's re-derivation states its mode). `DEV_MANUAL.md`'s
  row says the same in its command column. No assertion moved.
- **The libraries were rebuilt, not reused**: `git archive 338705e` into two
  exports under `.tuning/coord/S277/repro/`, B04 applied to one by
  `tools/mutation_check.py`'s own loader (S259's procedure), release and tune
  each, no worktree added to the shared `.git`. Their bytes differ from S259's
  copies (other paths); node counts decide. The driver compiles against this worktree's `src/` headers
  (`b50a1fc`), byte-identical to `338705e`'s: the only `src/` file between
  them is `src/evaluation.cpp`, and the libraries are `338705e`'s. The
  miners S277 imports (S097, S113, S230) and `S097_candidates.tsv` are
  unchanged between the two.
- **The first reproduction run read DIFFER on the guard witness** -- a
  harness artefact: the extractor took the blank line after
  `S259_remine.log`'s `b04_fires_guard.txt` section as a fifth, empty row. The
  extractor now drops blank lines; the whole script re-ran all SAME, and ran
  once more after a docstring-only edit to the miner. The log holds the last
  run and says so in its header.
- **The stages ran on the miner before the fast check's two checks**
  (sha256 `7e4f97cb0317f2c5`); the miner as committed (`0e1706d81d073cea`)
  re-ran only the refusal list, `.tuning/coord/S277/refusals.sh`, with the
  guard-mode `fires` and `pick` through the new checks as controls: witness
  cells identical to the stage run, same take. No sweep re-ran. The log's
  header says which sections ran on which.

### The script, `adocs/data/S277_mine_mate_row.py`

S113's miner and nothing else, imported as S259's one-off imported it:
importing `S113_mine_mate_row` re-points S097's driver to `ProbCut`, derives
9..12 from `ProbCutMinDepth`, and supplies `fens`. Its parser is S113's five
stages with `--mode` added to `fires` and `pick`, handed to S097's
`cmd_fires` and `cmd_pick` behind two thin guard-mode wrappers (the checks
above); `fires --lib` defaults to `None` as S097's does, so S097's
guard-mode refusal of a missing `--lib` is reached and the rule mode still
resolves to `build-tune/src/libchesso_engine.a`. Its header's six commands run stages 5
and 6 in guard mode, and say how the default mode differs. S113's and
S259's scripts are not edited.

### Accepts (2): guard mode on `338705e` reproduces S259

`.tuning/coord/S277/build_libs.sh` then `.tuning/coord/S277/repro.sh`, every
stage of the header in both modes, about 3 minutes; evidence
`adocs/data/S277_repro.log`. Cells compared as `mate:nodes`.

| output | against | result |
|---|---|---|
| `fens` | S259's `candidates.fen` | byte-identical, 269 |
| shipped and B04 sweeps | S259's `b04_shipped.txt`, `b04_open.txt` | identical, 269 each |
| separators | S259's `b04_separators.fen` | byte-identical, 4 |
| **guard witness** | `S259_remine.log`'s `b04_fires_guard.txt` | **all four moved-depth lists and every cell identical**; first line names the mode |
| **guard pick** | S259's row | **takes `4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73`, depth 11, mate in 6, shipped `[d11 d12]`, B04 `[d12]`, 768752 nodes**, witness `[d9 d10 d11 d12]` |
| guard pick over S259's own sweep files | S259's row | the same take |
| default witness | `S259_remine.log`'s `b04_fires.txt`, and S113's `fires` on the same library | identical cells, identical header lines |
| default pick | S113's `pick` on the same files; `S259_remine.log`'s `b04_pick` | byte-identical stdout; identical with ms dropped |

Both modes take the same row, as S259 found: all four separators fire in
both witnesses. The seven refusals each exit 2 with their message, on the
miner as committed: guard `fires` without `--lib`, guard `fires` on
`build-tune/src/libchesso_engine.a` relative and absolute, guard `pick` on a
rule witness, rule `pick` on a guard witness, guard `pick` without
`--fires`, guard `pick` on a guard-marked witness with no data rows.

### Accepts (3): GOLDEN and `DEV_MANUAL.md`

`tests/test_search.cpp`, the ProbCut row's GOLDEN block: "Re-derive with the
six commands in `adocs/data/S277_mine_mate_row.py`'s own header", plus a
paragraph on its guard mode and on S113's script. `DEV_MANUAL.md`'s
`mate_probcut_hides` row, the command column: the S277 script, stages 5 and 6 in guard mode, S113's as
what derived the rows so far, and `S277_repro.log` beside `S259_remine.log`.
The row's assertions, position, depth and distance are unchanged.
`MANUAL.md`, `adocs/specs.md` and `TOOLCHAIN.md` name neither miner nor row:
checked, no change.

### Suites

- `CLANG_FORMAT_MAJOR=22`, `cmake --build build -j12 && ctest --test-dir
  build -L fast --output-on-failure && cmake --build build-tune -j12 && ctest
  --test-dir build-tune -L fast --output-on-failure && ./clang-format.sh
  --check`: **release 45/45 (114 s), tune 45/45 (112 s), format clean**.
- `tools/plan_prose_check.py`, one mode per call, all exit 0:
  `--citations` 0 flagged over 32 files; `--touches` 0 flagged over 32 files
  (4 symbols read); `--params` silent. Re-run after the fast check's
  edits (miner, data index, this file), same readings, and the five
  `test_plan_*` cases 5/5 in each build. The fast suites above ran before
  those edits; none of them is C++ and no fast case reads the miner.
- Second tier (DEC-141): does not apply -- no `src/`, generator, search or
  pruning change.

### Proposed `done:` stamp

2026-10-13 -- `adocs/data/S277_mine_mate_row.py`, S113's ProbCut miner
imported with S097's `--mode` (`rule`, the default, or `guard`, DEC-238)
passed through to `fires` and `pick`, plus two guard-mode refusals: the
shipped tune library by its resolved path, and a `pick` with no witness or
an empty one. On `338705e`'s libraries, rebuilt from an export, guard mode
reproduces `S259_b04_fires_guard.py`'s four moved-depth lists and
`mate:nodes` cells exactly and takes S259's row,
`4rk2/1R6/6PP/2pR4/3n4/1p4K1/8/8 w - - 3 73`, depth 11, mate in 6; the
default mode matches S113's witness and prints S113's pick byte for byte.
Evidence `adocs/data/S277_repro.log` (not in `touches:`). The ProbCut row's
GOLDEN block and `DEV_MANUAL.md`'s row name the new script and its guard
mode (DEC-142); no assertion changed, no `src/` change. Fast suite 45/45 in
both builds, format clean, prose checks clean. `--mode` values are S097's
names, not the step's `guard|default`.

### Commit text

    Give the ProbCut mate-row miner a --mode (S277)

    S259 re-mined the ProbCut row with S113_mine_mate_row.py, which has
    no --mode; its default witness does not move the tree at d11, the
    row's depth, and the guard-mode witness DEC-238 asks of a guard row
    took a one-off script. S277_mine_mate_row.py imports S113's miner
    and passes S097's --mode (rule, the default, or guard) through to
    fires and pick. In guard mode it also refuses the shipped tune
    library by resolved path (S097 compares the spelling) and a pick
    with no witness or an empty one.

    On 338705e's libraries guard mode reproduces S259's guard witness
    cell for cell and takes S259's row; the default mode prints S113's
    pick byte for byte (adocs/data/S277_repro.log). The row's GOLDEN
    block and DEV_MANUAL's row name the new script (DEC-142); no
    assertion changes. Filler under DEC-171. No src/ change.
