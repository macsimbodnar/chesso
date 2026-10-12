id:         S275
goal:       every exact null direction of the 823-parameter evaluation is pinned in the tuner, or recorded as harmless with a measurement, before the next fit
accepts:    (1) `tools/feature_audit`'s identity report (S134: rank 784 of 823, nullity 39 -- 32 always-zero pawn columns on ranks 1 and 8, the five documented material-against-table directions, and two new king directions, one per king table, since one king a side makes each king table's signed occupancy sum to zero) is re-run on HEAD and the 39 are listed by name; (2) for each class, the tuner either pins it (a gauge: a fixed coordinate, a projection after each step, or the columns dropped from the fitted vector) or the step records, by a measured fit, that Adam does not drift along it, with the drift read off the fitted values; (3) a test that fails if a fit moves along a pinned direction, observed red with the pin removed; (4) a fit taken before and after on the same corpus slice agrees in loss to the last printed digit, so the pin changes nothing the evaluation can see
touches:    tools/tuner.cpp, tools/tuner_model.hpp, tools/tuner_groups.hpp, tools/feature_audit.cpp, tests/test_tuner_gradient.cpp, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   any change to src/ or to a shipped weight; regularisation (DEC-170 ruled against it); the five material/table directions' convention if one is already enforced -- then it is cited, not re-done
decisions:  DEC-170, DEC-090, DEC-142
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-12 -- the seven exact null directions of the 823-parameter evaluation are held in the tuner, the 32 zero pawn columns measured harmless. No enforced convention existed (DEC-059's re-anchor was by hand, `POSITIONAL_ROOM` checks shipped weights). `descend()`: Adam, then each held level put back where the run started -- per piece the `psqt_mg` + `psqt_eg` sum, the piece's value taking the shift; per king table its own sum -- held only where the mask frees the whole direction, so only the king pair under `--only psqt` or `--freeze material`. Unpinned drift on real labels, 1 M rows, epoch 2000: king tables +16.4 / +15.1 a square, queen +11.8, the others 1.4 to 2.6; zero pawn columns 0 in every run (Adam's step from an always-zero gradient is 0). Loss: the real-label fit prints every line identical before and after; scores within 0.0005 cp on 1 M rows; on planted labels the runs part in the seventh decimal as `--threads` alone does, and differ only along the seven directions. `load()` refuses a row without one king a side (red first, 6 of 6; 0 corpus rows refused). `feature_audit`: 7 gauges null on every row, 39 of 39 null dimensions spanned. Tests: three cases in `test_tuner_gradient`, two in `test_tuner_groups`; red under six mutants -- every pin removed, the king pair or the queen's direction dropped, the pawn level over its dead squares, the shift kept out of the value, the mask ignored. Fast suites 44/44 both, format clean, prose checks clean, bench 3872032. DEV_MANUAL updated, MANUAL checked (no change). Fast check: nothing real; "each table keeps its level" and the comparability claim reworded (the level held per piece is the mg + eg sum, and only the king pair is held under `--only psqt` / `--freeze material`), fixed before commit.

## Why

S134's accepts required that no unreported exact degeneracy remain after its
folds, "or a new finding is filed before a fit". Its identity report
(`adocs/data/S134_feature_audit.txt`, exact rank over integer X'X) found
two it had no record of: with one king a side, the signed occupancy of
`psqt_mg[KING]` sums to zero on every row, and so does `psqt_eg[KING]`. A
constant added to a king table moves no evaluation, so its level is
unidentified. Adam rescales each coordinate, so its update need not stay
orthogonal to a null direction, and a fit can drift along one: harmless to
the score, but it moves 64 shipped numbers for nothing and makes two fits'
tables incomparable. This is that filing.

## Lane

Agent work, tuner tooling only; a short fit on a corpus slice, minutes.
Filler under DEC-171 (no defect reaches play), ordered before the next fit
(S262's trace tuner).

## As built (2026-10-12)

### Deviations

- **Accepts (4) holds on real labels and not to the letter on planted ones.**
  Over the real-label slice every line `tuner` prints is identical before and
  after. Over the planted slice -- labels from a known vector, training error
  printing 0.000000 from epoch 100 -- the two trajectories part in the seventh
  decimal (24 of 82 common reports differ: 18 in the `*` alone, 6 in a
  0.000000 / 0.000001 / 0.000002), and so does the unchanged tuner at `--threads 6`
  against 12 (24 of 95, 20 in the `*` alone). The hold's last-bit difference is
  inside the tuner's own floating-point envelope; the emitted vectors differ only
  by a uniform shift along each of the seven directions plus integer rounding,
  and in none of the 50 term weights.
- **The real-label drift was read with `adocs/data/S275_walk.cpp`, not the
  `tuner` binary.** On the 1 M-row slice the held-out error never beats the
  shipped constants, so `tuner` emits the start and no drift can be read off
  it. The walk runs the same `adam_update()` -- the pre-S275 step, moved and
  bit-identical -- and `descend()`, and reads both vectors at the last epoch.
  The `tuner`-binary measurement to convergence is on the planted slice.
- **`load()` refuses a row without exactly one king a side.** Not in the
  accepts: it is the king gauges' precondition (they are null only with one king
  a side), mirroring `load_FEN()`. Test written first and observed red, 6 of 6
  such rows accepted; 0 of the 10795695 dedup rows refused.
- **Adam's step moved from `tools/tuner.cpp` into `tools/tuner_model.hpp`**
  (`adam_t`, `adam_update()`, `descend()`), so the test runs the fit's own step.
  Arithmetic unchanged: the real-slice logs are line-identical, and the walk's
  plain run prints the pre-S275 tuner's training errors.
- **`tuner` prints one more line and stamps one more header line**:
  `gauges held: 7 of 7: ...` after the line with the free count, and
  `// gauges     7 of 7 held at the levels this started from`.
- **Files outside `touches:`**: `adocs/data/README.md` (index rows) and five
  data files, `adocs/data/S275_drift.py`, `S275_walk.cpp`, `S275_walk_real.txt`,
  `S275_fits.txt`, `S275_feature_audit.txt`.
- Logs are under the worktree's `.tuning/coord/S275/`.

### The five material/table directions had no enforced convention

None in the tuner: its header comment called the split free on purpose
("Nothing distinguishes those solutions and nothing needs to"). DEC-059 spent
the freedom once, by hand, on an emitted header (`.tuning/reanchor.py`, the
queen -432 / +432). `test_evaluation`'s `POSITIONAL_ROOM` (150) bounds a
piece's value against its tables loosely, on the shipped weights only. Neither
is a pinned coordinate or a fixed value in the fit, so this step pins them.

### The 39 null directions, by class

`feature_audit` re-run on `0d9faba` over the dedup corpus: identical to
`S134_feature_audit.txt` line for line. Rank exactly 784 of 823.

- **32 zero columns**: `psqt_mg[pawn]` and `psqt_eg[pawn]` on a8..h8 and
  a1..h1 (model squares 0..7 and 56..63).
- **King pair, 2**: sum of `psqt_mg[king]` = 0, sum of `psqt_eg[king]` = 0.
- **Material/table, 5**: `material[t]` = the sum of every `psqt_mg[t]` and
  `psqt_eg[t]` column, for pawn (its 96 reachable squares), knight, bishop,
  rook, queen.

### Each class's call

The gauge (`tools/tuner_groups.hpp`): the level -- the sum over the squares
its direction moves -- stays at the run's start. For a piece that is the sum of
its `psqt_mg` and `psqt_eg` squares together, and the piece's value takes the
shift; for a king table it is that table's own sum. Applied after every Adam
step by `descend()`; a move along the null direction, so invisible to every
score. Chosen over holding the value fixed because, with material and psqt
both free, a fit's tables then compare square by square with the start and a
change in a piece's worth shows in its define. Held only when the mask frees
every column of it: `--only psqt` and `--freeze material` hold the king pair
alone; `--only material`, `--freeze psqt` and the term groups hold none -- the
freeze fixes the direction there, and a hold would write a frozen parameter.
Under `--only psqt` or `--freeze material` the tables therefore carry any change
in a piece's worth, and the comparability holds only between fits with material
and psqt both free.

- **King pair: pinned.** Unpinned drift, Adam alone, real labels, first 1 M
  rows of the dedup corpus, epoch 2000 (`S275_walk_real.txt`): **+16.38 and
  +15.08 a square** on `psqt_mg[king]` and `psqt_eg[king]`. Pre-S275 `tuner`
  to convergence on the planted slice: -0.141 and +0.141 a square. Fixture:
  +3.29 and +0.22.
- **Material/table five: pinned.** Unpinned level drift, real labels: pawn
  +2.63, knight -1.67, bishop -1.39, rook -2.35, **queen +11.80** a square,
  material compensating (queen -57.66 unpinned against -45.86 pinned). Planted,
  pre-S275 `tuner`: +0.24, +0.48, +0.98, +0.77, +0.48. Pinned: within 2.0e-14
  of every level on real labels; on the planted emitted integers -0.07 to
  +0.08, the rounding.
- **32 zero columns: measured harmless, not pinned.** No row touches them, so
  their gradient is exactly 0.0, Adam's moment and velocity stay 0, and the
  step is lr * 0 / (0 + 1e-8) = 0. Largest change 0 in every run: both planted
  fits, both walk runs, both fixture runs. A pin would be dead code with no red
  to observe; the test guards them instead.

`feature_audit` now ends on the fit's side: each of the seven held gauges
checked null on every row, and `32 zero columns and 7 held gauges span 39 of
at most 39 null dimensions` over the dedup corpus (`S275_feature_audit.txt`).
With the queen's gauge dropped it reads 38 of 39, "a null direction the tuner
does not hold".

### The test and its red

`test_tuner_gradient`, three cases:

- "load() refuses a row without one king a side": six rows (no White king, no
  Black king, two White, two Black, two a side, none), each appended to the
  legal fixture, each refused; precondition a legal row loads. **Red before the
  fix**, 6 of 6 accepted (`king_refusal_red.log`).
- "moving along a gauge moves no fixture row's score": each of the seven
  moved by 37, worst score change under 1e-9; non-vacuity, one level column
  alone moves some row by more than 1.
- "the fit holds every gauge still and no score sees the hold": 300 epochs of
  `descend()` against `adam_update()` over the fixture plus four rows with
  kings apart (the curated list's kings cancel: its plain run does not walk
  `psqt_mg[king]` at all). Its own written-out list of the seven, not
  `gauges()`. Plain level shifts 0.22 to 3.70 against a floor of 0.1; held
  under 1e-9 (measured 1.2e-14 at most); scores apart 6.2e-13 cp after the
  plain run moved one by 1580 cp; the 32 pawn columns bit-identical in both.

`test_tuner_groups`, two cases: the seven are disjoint, inside the material and
psqt blocks, 741 columns that with the 32 dead pawn squares are the whole of
both; and for every `--only` group and every single `--freeze` the held set is
the written-out expectation, a hold restores each held level and writes no
frozen parameter.

**Observed red, each mutant alone, the file restored after**
(`.tuning/coord/S275/red_m*.log`):

- M1, `descend()` without the hold (every pin removed): the fit case red on
  all seven, held shifts equal to the plain ones (2.27, -3.70, -2.49, -1.03,
  -2.64, 3.29, 0.22).
- M2, the king pair dropped from `gauges()`: the fit case red on
  `psqt_mg[king]` (3.29) and `psqt_eg[king]` (0.22); `REQUIRE( 5 == 7 )` in the
  other three gauge cases.
- M3, the queen's gauge dropped: the fit case red on the queen (-2.64);
  `REQUIRE( 6 == 7 )` elsewhere; `feature_audit` reads 38 of 39.
- M4, the pawn level over its dead squares: the 32 pawn columns moved by -1.70
  in the held run and the pawn level off by 0.57, red 33 times; the shape case
  red 35 times.
- M5, the shift not moved into the piece's value: scores apart 2.62 cp, red.
- M6, the mask ignored: `--only material` holds 7, expected 0, red.

### The loss comparison

`S275_fits.txt`, `tuner --threads 12` at defaults, before (`0d9faba`) and after:

- **Real labels, first 1000000 rows** (sha256 `379aa7a6...`): every printed
  line identical -- `fitted K = 0.7415`, `start: train 0.119429 validation
  0.122103`, all 20 reports to `epoch 2000 train 0.116897 validation
  0.122918`, the stop, `best`, the warning. Both emit the start.
- **The walk on the same labels** (`S275_walk_real.txt`): both descents'
  training errors identical to six digits at all 20 reports (largest
  difference 4.3e-10); scores within **0.000473 cp** on every one of the
  1000000 rows at epoch 2000, against 497.3 cp moved.
- **Planted labels** (sha256 `45305e64...`): as under Deviations. Before: best
  epoch 7500, stopped at 9500. After: best 6200, stopped at 8200. Control,
  before at 6 threads: best 8000, emitted vector identical to 12 threads.

### Suites

`CLANG_FORMAT_MAJOR=22`, i7-8700K, g++ 13.3, `-j12`: build 44/44 fast,
build-tune 44/44 fast, `./clang-format.sh --check` clean, exit 0
(`.tuning/coord/S275/gate4.log`, the final tree). The final `tuner` binary
re-run over the real-label slice reproduces the after run's log and header
byte for byte. `tools/plan_prose_check.py` `--citations`,
`--touches` and `--params`, one mode per call, exit 0 each. `bench` 3872032;
`src/` untouched. No second tier: tuner tooling only. No timing taken.
DEV_MANUAL updated (the gauges, the audit's new lines, the define
comparability note, one stale `tools/tuner.cpp` line range); MANUAL checked,
no change.

### Proposed `done:` stamp

2026-10-12 -- the seven exact null directions of the 823-parameter evaluation
are held in the tuner, the 32 zero pawn columns measured harmless. No enforced
convention existed (DEC-059's re-anchor was by hand, `POSITIONAL_ROOM` checks
shipped weights). `descend()`: Adam, then each held level put back where the
run started -- per piece the `psqt_mg` + `psqt_eg` sum, the piece's value taking
the shift; per king table its own sum -- held only where the mask frees the
whole direction, so only the king pair under `--only psqt` or `--freeze
material`. Unpinned drift on real labels, 1 M rows, epoch 2000: king
tables +16.4 / +15.1 a square, queen +11.8, the others 1.4 to 2.6; zero pawn
columns 0 in every run (Adam's step from an always-zero gradient is 0).
Loss: the real-label fit prints every line identical before and after; scores
within 0.0005 cp on 1 M rows; on planted labels the runs part in the seventh
decimal as `--threads` alone does, and differ only along the seven directions.
`load()` refuses a row without one king a side (red first, 6 of 6; 0 corpus rows
refused). `feature_audit`: 7 gauges null on every row, 39 of 39 null
dimensions spanned. Tests: three cases in `test_tuner_gradient`, two in
`test_tuner_groups`; red under six mutants -- every pin removed, the king pair
or the queen's direction dropped, the pawn level over its dead squares, the
shift kept out of the value, the mask ignored. Fast suites 44/44 both, format clean, prose checks clean, bench
3872032. DEV_MANUAL updated, MANUAL checked (no change).

### Commit text

```
Hold the evaluation's null directions still in the tuner (S275)

feature_audit puts the 823-parameter design at rank 784: 32 pawn
columns no row touches, five material-against-table directions and one
per king table. Adam's per-coordinate steps walk along the last seven:
over 1 M real rows, 2000 epochs, the king tables moved +16.4 and +15.1
a square and the queen's +11.8 for nothing, leaving two fits' tables
incomparable. The 32 pawn columns never move: their gradient is exactly
zero, so Adam's step is zero, and they are measured, not pinned.

descend() now follows each Adam step by putting each held level back
where the run started: per piece the sum of its mg and eg squares, the
piece's value taking the shift; per king table its own sum. That is a
move along a null direction, so no score changes. A direction with a
frozen column is left to the freeze. The real-label fit prints every
line identical before and after, and feature_audit checks each held
direction null on every row and the set complete. load() refuses a row
without one king a side, as load_FEN() does: the king directions are
null only there.
```
