id:         S205
goal:       `test_perft` either counts the four check columns its assets already carry, or stops parsing values it will never compare
accepts:    a census, in the step file, of how many asset layers carry a non-null value for each of `checks`, `discovery_checks`, `double_checks` and `checkmates`, derived by a script recorded under `adocs/data/S205_*`; then either (a) `perft()` counts them, `columns_match` gates them, their `print_stats` columns are uncommented, and each of the four is observed red under a scratch asset with one value off by one -- with the cost of counting them measured against `ctest -L slow`'s current 53 s and stated; or (b) the four fields are deleted from `expected_stats_t`, `stats_t` and `load_expected_stats`, and this file records that the values in the assets are unclaimed and why counting them was refused. A third outcome is valid and must be stated as one: keeping the parse and adding a guard that fails if any asset layer carries a non-null value for a column nothing compares
touches:    tests/test_perft.cpp, adocs/data/
excludes:   any `src/` change; the five columns S193 already gated; the asset files themselves
decisions:  DEC-142
closes:     <!-- found by S193's fast check, 2026-09-09 -->
blocks:
paused_by:
author:
done:

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
