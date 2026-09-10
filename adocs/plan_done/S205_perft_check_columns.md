id:         S205
goal:       `test_perft` either counts the four check columns its assets already carry, or stops parsing values it will never compare
accepts:    a census, in the step file, of how many asset layers carry a non-null value for each of `checks`, `discovery_checks`, `double_checks` and `checkmates`, derived by a script recorded under `adocs/data/S205_*`; then either (a) `perft()` counts them, `columns_match` gates them, their `print_stats` columns are uncommented, and each of the four is observed red under a scratch asset with one value off by one -- with the cost of counting them measured against `ctest -L slow`'s current 53 s and stated; or (b) the four fields are deleted from `expected_stats_t`, `stats_t` and `load_expected_stats`, and this file records that the values in the assets are unclaimed and why counting them was refused. A third outcome is valid and must be stated as one: keeping the parse and adding a guard that fails if any asset layer carries a non-null value for a column nothing compares
touches:    tests/test_perft.cpp, adocs/data/
excludes:   any `src/` change; the five columns S193 already gated; the asset files themselves
decisions:  DEC-142
closes:     <!-- found by S193's fast check, 2026-09-09 -->
blocks:
paused_by:
author:     Claude Opus 5 (agent), 2026-09-10
done:       outcome (a). `test_perft` gates **nine** columns, not five: `perft()` counts `checks`, `discovery_checks`, `double_checks` and `checkmates` at the depth-1 leaf, `load_expected_stats` parses the two it never parsed, `columns_match` decides on all four and their `print_stats` cells are printed. Census by `adocs/data/S205_check_columns.py`: 42/30/30/42 over the glob reproduces, but a run compares **58 layers, 33 with checks and checkmates, 21 with discovery and double** -- 29 distinct layers, 84 expected values that no run had ever read. **The obvious reading of the four columns is wrong and matched only 22 of the 29**: the convention, established by classifying every affordable layer under each candidate and taking the one with no exception, is `checks` including mates, `checkmates` all mates, and the two classification columns describing the **non-mating** checks only, exclusive of each other, with a castle's rook counting as moved. **Kiwipete depth 5 decides both exclusions and nothing else does**: 8 of its 2645 two-checker leaves are mate (2637), 12 of its 19895 discovered checks are castles (19883). **No asset value is wrong** -- the 2645 paths were replayed through python-chess, 2645 of 2645 legal and ending in a two-checker position, so engine and asset are both right and counting different things; the assets were not touched. Cost **+27 %**, 52.9 s -> 67.4 s over three runs each, 68.90 s through `ctest -L slow`, cheap because `generate_moves()` is legal-only so mate is an empty list, and it is paid at check leaves alone. Each of the four observed red under a scratch asset one off at Kiwipete depth 4, one red cell each in the perturbed column and no other, then restored to exit 0. `--max-nodes 5000000` recounts 23 layers up to Kiwipete depth 4 with python-chess as an independent board: every column agrees. Gate green in both builds, 33/33 and 33/33, format clean, slow label green. No `src/` change, no `Bench:` line, no SPRT. Closes the S193 fast-check finding

## Why this exists

S193 made `test_perft` refuse a missing or unparseable asset and made all five
of the columns it reads decide the run, where only `nodes` had. The fast check
over its diff found that the binary parses **four more** columns it never
compares -- `checks`, `discovery_checks`, `double_checks` and `checkmates` are
fields of `expected_stats_t`, `load_expected_stats` fills two of them, and
`get_move_stats` sets all four to 0 unconditionally, so `perft()` does not count
them at all. Their `print_stats` columns have been commented out since before
the branch.

The reviewer read this as latent, on the belief that every asset layer leaves
the four null. **It does not.** Counted over `tests/assets/perft_json/*.json`
on 2026-09-09, across 71 depth layers:

| column | layers with a non-null value |
|---|---|
| `checks` | 42 |
| `checkmates` | 42 |
| `discovery_checks` | 30 |
| `double_checks` | 30 |

So these are real expected values, sitting in tracked assets, that no run has
ever compared -- and two of the four are not even parsed. That is the same class
of defect S193 removed for the other five columns, at a larger scale than the
review that found it believed.

## What decides it

**Counting checks is not free**, which is why this is a step and not a
one-line fix: `perft()` today makes a move and recurses, and a check count needs
`is_check()` at every node. The slow label runs 53 s. The measurement decides
between (a) and (b): if counting them costs little, they are 84 or more expected
values of unclaimed signal over a generator this project's INV-1 rests on; if it
costs a lot, deleting the dead fields is the honest outcome and the assets keep
their values for whoever wants them later.

Whichever is chosen, **the outcome that is not acceptable is the current one**:
a binary that parses a number and silently ignores it.

## Order

Independent of the test block's other steps. It touches one file none of them
touch, owns no run beyond the slow label, and nothing is gated on it.

## The census

`adocs/data/S205_check_columns.py`, run 2026-09-10:

```
scope                                layers   checks  discovery_checks  double_checks  checkmates
glob, every layer                        71       42                30             30          42
files test_perft loads, every layer      65       36                24             24          36
layers a run compares                    58       33                21             21          33
```

The 42/30/30/42 this file was opened on reproduces, and it is the count over
`tests/assets/perft_json/*.json`. **Two smaller numbers matter more.**
`debug_perft.json` is commented out of `test_files`, so six of those layers are
not loaded at all; and seven more sit past their case's `depth_limit`. What a
run actually compares is **58 layers, 33 of them carrying `checks` and
`checkmates` and 21 carrying `discovery_checks` and `double_checks`** -- 29
distinct layers with at least one value.

## What the measurement decided: outcome (a)

`perft()` counts all four, `columns_match` gates all four, and their
`print_stats` columns are printed rather than commented out.

**Cost, measured on the workstation, `build/tests/test_perft` run directly,
three runs each, machine otherwise idle:**

| | run 1 | run 2 | run 3 |
|---|---|---|---|
| before | 53.35 s | 52.67 s | 52.69 s |
| after | 67.47 s | 67.23 s | 67.39 s |

**+27 %, about 14.5 s.** Through `ctest -L slow`: 68.90 s against the 53 s this
file quoted. It is a check test at every leaf and a move generation at every
check leaf -- and only at check leaves, which is why it is a quarter and not a
multiple. Two facts in `src/` made the second half nearly free and were read off
the code, not assumed: `generate_moves()` emits legal moves only, so mate is an
empty move list and needs no make/unmake to confirm, and `make_move` therefore
always returns true.

Counting was accepted rather than refused because the columns are 84 expected
values over the generator INV-1 rests on, the slow label is not on the automatic
gate's critical path, and 14.5 s is the whole price.

## What it found: the four columns are not four independent counts

The obvious reading -- `checks` is a check, `double_checks` is two checkers,
`discovery_checks` is a checker that is not the piece that moved -- **matches 22
of the 29 layers and is wrong**. Implemented that way the run went red on seven
layers. The disagreements were then classified rather than argued: a scratch
tool counted every leaf of every affordable layer under each candidate reading,
and exactly one reading matched **all 29 layers with no exception**:

- `checks` -- every check leaf, **mates included**.
- `checkmates` -- every mate leaf.
- `double_checks` -- two or more checkers, **mates excluded**.
- `discovery_checks` -- exactly one checker and that checker is not a piece this
  move moved, **mates excluded**, and **a castle's rook counts as having
  moved**, so the check it gives is not a discovery.

**Kiwipete at depth 5 is the layer that decides both exclusions**, and it is the
only one that does. Of its 2645 two-checker leaves, exactly **8 are mate**, and
2645 - 8 = 2637, the asset's value. Of its 19895 discovered checks, exactly
**12 are castles**, and 19895 - 12 = 19883, the asset's value. At every other
layer the exclusions are empty and every reading agrees.

**No asset value is wrong.** That was the first conclusion and it did not
survive: the 2645 two-checker paths were dumped and replayed move by move
through python-chess, which is a different board, a different legality test and
a different check detector -- **2645 of 2645 legal, 2645 of 2645 ending in a
two-checker position, histogram `{2: 2645}`**. The engine's raw count is right
and the asset's 2637 is right; they are counting different things. Had the
assets been edited to match the raw count, the step would have destroyed correct
reference data to make a wrong classifier green, which is what `excludes:` is
for.

`adocs/data/S205_check_columns.py --max-nodes 5000000` re-derives the convention
independently: it recounts **23 asset layers up to Kiwipete depth 4 (4085603
nodes)** with python-chess as the board and compares all nine columns.
`23 layers recounted, 10 over the node budget, every column agrees on every
layer recounted`. The two exclusions are outside any python budget -- the layer
that exercises them is 193690690 nodes -- and rest on the classification above.

## Each of the four observed red

A scratch asset under the scratchpad: `perft.json` trimmed to the start position
and Kiwipete at `depth_limit` 4, `talkchess_perft.json` emptied. Unperturbed the
binary exits 0. One value at Kiwipete depth 4 raised by one, one column at a
time:

| perturbed | exit | red cell |
|---|---|---|
| `checks` 25523 -> 25524 | 1 | checks, expected 25524, engine 25523 |
| `discovery_checks` 42 -> 43 | 1 | discovery, expected 43, engine 42 |
| `double_checks` 6 -> 7 | 1 | double, expected 7, engine 6 |
| `checkmates` 43 -> 44 | 1 | checkmates, expected 44, engine 43 |

One red cell each, in the perturbed column and no other. Restored, exit 0.
