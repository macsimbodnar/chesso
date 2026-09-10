id:         S206
goal:       the change in `truncation_scan`'s model-against-engine counts between S076 and 2026-09-10 is explained by a named commit, or the reading is shown to have been wrong when it was written
accepts:    the two readings reproduced -- the S076-era 135399 past 2.0 / 99 past 2.8 / 30 at 2.875 and 2026-09-10's 138331 / 105 / 33, both over `.tuning/selfplay_v2_dedup.tsv` at `--min 2.8`, both stated with the sha the binary was built at; the commit that moves the count named by bisection over the commits between `77d7450` and HEAD that touch anything the scan's loop reads -- `src/evaluation.cpp`, `src/eval_tables.hpp`, `tools/eval_model.hpp`, and `load_FEN` and everything it writes; the difference classified as one of (a) a deliberate change whose effect on this reading is correct and is then recorded beside the counts, (b) a defect in `load_FEN`, `evaluate()` or the model, which the BUGS rule makes the next thing fixed, or (c) the earlier reading having been taken on a different corpus or binary than the paragraph claims; `DEV_MANUAL.md` "Re-measure the truncation-bound positions after a fit" carries the answer in place of the sentence that currently says it is unexplained; no engine constant moves
touches:    DEV_MANUAL.md, adocs/data/, tools/truncation_scan.cpp (only if the tool is what is wrong)
excludes:   changing the four pinned positions in `tests/test_eval_model.cpp`, which belong to the weights (DEC-057) and are S126's business at the next refit; any refit; any change to `evaluate()` unless (b) is what the bisection finds, in which case the fix is its own step under the BUGS rule
decisions:  DEC-057, DEC-142
closes:
blocks:
paused_by:
author:
done:

## Where this comes from

S192, 2026-09-10, while resolving its inventory row 14 -- naming
`build/tools/truncation_scan` at the site of `tests/test_eval_model.cpp`'s four
pinned positions. The scan was run to confirm the command still works and still
produces the four:

```
10795695 rows scanned, 138331 past 2.0, 105 past 2.8000, worst 2.875000
```

`DEV_MANUAL.md` recorded 135399 past 2.0, 99 past 2.8 and 30 at 2.875 for the
same command on the same file at S076's constants. The row count is identical,
the worst value is identical, all four pinned positions are still in the set and
the suite is green, so nothing the tests assert has moved. Two runs back to back
gave the same numbers, so the tool is deterministic.

**What is already excluded.** The weights have not moved:
`adocs/data/S192_anchors.py` reproduces all ten pinned anchors at the values
S076 pinned, exit 0. `src/eval_tables.hpp` has no commit since `77d7450`.

**What is not excluded, and is the first thing to look at.** The scan's loop
calls `load_FEN` and then the engine's own `evaluate()` on every row, and
`ecd735e` (S161) changed what `load_FEN` keeps -- "clears fields the board
cannot support". A row whose FEN carries a field the board now drops evaluates
differently than it did, without any evaluation code changing. That is a
candidate and nothing was bisected; it is equally possible the earlier figure
was taken before the corpus was deduplicated in place, or on the non-dedup file.

## Why it is worth a step

`truncation_scan` is the instrument that re-chooses the four pinned positions at
every refit (DEC-057), and S126 refits everything once the search stops moving.
An instrument whose reading changed for a reason nobody can name is not one to
re-pin a test from. The count is also the only continuous measurement of how far
`tools/eval_model.hpp` and `src/evaluation.cpp` have drifted apart, which is the
thing the tuner's whole model rests on: if the answer is (b), the divergence is
a defect in one of the two and the fast suite does not see it, because
"the model reproduces evaluate() on every phase" runs over the curated corpus
and not over 10.8 M rows.

## Cost

Machine-free apart from the scan itself: 48 s per reading, plus a build per
bisection point. Six to ten points over the range, so under two hours, and no
match and no fit.
