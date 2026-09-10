id:         S206
goal:       the change in `truncation_scan`'s model-against-engine counts between S076 and 2026-09-10 is explained by a named commit, or the reading is shown to have been wrong when it was written
accepts:    the two readings reproduced -- the S076-era 135399 past 2.0 / 99 past 2.8 / 30 at 2.875 and 2026-09-10's 138331 / 105 / 33, both over `.tuning/selfplay_v2_dedup.tsv` at `--min 2.8`, both stated with the sha the binary was built at; the commit that moves the count named by bisection over the commits between `77d7450` and HEAD that touch anything the scan's loop reads -- `src/evaluation.cpp`, `src/eval_tables.hpp`, `tools/eval_model.hpp`, and `load_FEN` and everything it writes; the difference classified as one of (a) a deliberate change whose effect on this reading is correct and is then recorded beside the counts, (b) a defect in `load_FEN`, `evaluate()` or the model, which the BUGS rule makes the next thing fixed, or (c) the earlier reading having been taken on a different corpus or binary than the paragraph claims; `DEV_MANUAL.md` "Re-measure the truncation-bound positions after a fit" carries the answer in place of the sentence that currently says it is unexplained; no engine constant moves
touches:    DEV_MANUAL.md, adocs/data/, tools/truncation_scan.cpp (only if the tool is what is wrong)
excludes:   changing the four pinned positions in `tests/test_eval_model.cpp`, which belong to the weights (DEC-057) and are S126's business at the next refit; any refit; any change to `evaluate()` unless (b) is what the bisection finds, in which case the fix is its own step under the BUGS rule
decisions:  DEC-057, DEC-142, DEC-169
closes:
blocks:
paused_by:
author:     Claude Opus 5 (agent), 2026-09-10
done:       both readings reproduce at their own shas -- `77d7450` prints 135399 / 99 / 30 and HEAD 138331 / 105 / 33 over the same `.tuning/selfplay_v2_dedup.tsv` at `--min 2.8`, 10795695 rows either way -- and **two commits move them, neither a defect: classification (a) twice**. **`21b4a21`, S085's SPSA vector, is the whole of 99 -> 105 and 30 -> 33**: it raised `LAZY_EVAL_MARGIN` from 150 to 184, and `eval_model::evaluate` and `evaluate()` both clamp the tapered mobility-plus-king-safety sum at that margin, so **wherever the clamp binds the two agree exactly and the taper's truncation residual is not there to be measured**; unclamping restores it, which is why the count went *up*. **`883c255`, S104's `CHESSO_ARCH=native`, moves the 2.0 column alone by -991**: `-march=native` contracts the model's `mobility[t] * params[...] + sum` into an FMA and the double moves by an ulp, visible only at a threshold rows sit exactly on -- a residual is a multiple of 1/24 and **2.0 = 48/24**, where 2.8 is not. **Both proved by counterfactual, not by argument**: HEAD with the margin edited back to 150 reads **134408 / 99 / 30**, its parent's reading to the row, and `883c255` with `-ffp-contract=off` reads **135399** again. **The candidate the step was opened on moves nothing** -- `ecd735e` (S161, `load_FEN`) is dated after `21b4a21`, which already prints what HEAD prints. The weights were excluded by measurement and not by the anchors alone: every non-zero entry of the model's starting vector is identical at the two shas, and so is every extracted model feature on a row that entered the set -- what changed was neither weights nor features but the clamp on their product. Three flat segments with an interior point in each, so no third mover can hide without cancelling itself exactly inside one. `adocs/data/S206_truncation_drift.sh` re-derives six readings and both counterfactuals from clean worktrees, **8 of 8 reproduced in 7 m 02 s**; **its own first run was vacuous on two rows** -- `HEAD` passed to `git -C "$WT" checkout` resolves in the worktree, so both HEAD rows re-measured their predecessor and still printed `as recorded` -- and it resolves the sha in the repository now and refuses a counterfactual whose `sed` would match nothing. `DEV_MANUAL.md` carries the answer where the unexplained sentence was, the `GOLDEN (DEC-142)` note names the second trigger, DEC-169 records it: the four pinned positions are re-derived after a refit **and** after any move of `LAZY_EVAL_MARGIN`, which is S039's business. All four still qualify at HEAD's weights and none was re-pinned (the step's `excludes:`). No engine constant moved, no `src/` change, no `Bench:` line, no SPRT. Gate green in both builds, 33/33 and 33/33 with `CLANG_FORMAT_MAJOR=22` (DEC-146), format clean, `plan_prose_check.py` 0 flagged over 59 files

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

## What it found

Two commits move the reading, in opposite columns, and neither is a defect.
Every point below is a fresh Release build of that sha in a clean worktree, run
against the same untouched `.tuning/selfplay_v2_dedup.tsv` (mtime 2026-08-17
18:35 by the filesystem, 34 minutes older than `77d7450` itself), and
`adocs/data/S206_truncation_drift.sh` re-derives all eight rows.

| sha | what it is | past 2.0 | past 2.8 | at 2.875 |
|---|---|---|---|---|
| `77d7450` | S076's fit pasted; the reading `DEV_MANUAL.md` recorded | 135399 | 99 | 30 |
| `38ad4fe` | `883c255^` | 135399 | 99 | 30 |
| `883c255` | **S104 gives `build/` `CHESSO_ARCH=native`** | 134408 | 99 | 30 |
| `3488506` | `21b4a21^` | 134408 | 99 | 30 |
| `21b4a21` | **S085's SPSA vector: `LAZY_EVAL_MARGIN` 150 -> 184** | 138331 | 105 | 33 |
| `214a9f1` | HEAD | 138331 | 105 | 33 |
| `883c255` | counterfactual: the same commit, `-ffp-contract=off` | 135399 | 99 | 30 |
| `214a9f1` | counterfactual: HEAD with the margin back at 150 | 134408 | 99 | 30 |

Three flat segments with an interior point in each, so no third mover can hide
without cancelling itself exactly inside one of them.

**The margin is the whole of the 2.8 and 2.875 columns, and the mechanism is a
clamp that makes the two implementations agree.** `eval_model::evaluate` clamps
the tapered mobility-plus-king-safety sum at `LAZY_EVAL_MARGIN` because
`evaluate()` clamps the same sum at the same margin -- the lazy shortcut's
soundness is a bound on the sum of the expensive terms. Wherever that clamp
binds, the model returns the margin as a `double` and the engine returns it as
an `int`, they agree to the bit, and **the taper's truncation residual on those
terms is not there to be measured**. Raising the margin from 150 to 184
unclamps a band of positions and the residual reappears, which is why the count
went *up* rather than down. The counterfactual is the proof and it lands on the
parent's number to the row: HEAD, margin edited back to 150, reads
134408 / 99 / 30.

**The arch flag moves the 2.0 column alone, and only because rows sit exactly on
it.** `-march=native` lets GCC contract the model's
`mobility_mg_sum += mobility[t] * params[MOB_MG_BASE + t]` into an FMA, so the
model's `double` moves by an ulp. A residual between the float model and the
integer engine is a multiple of 1/24, and **2.0 = 48/24 is a value rows land
exactly on**, where 2.8 is not -- so an ulp can only ever be visible at that one
threshold, and it was, 991 times. The same commit built with `-ffp-contract=off`
reads 135399 again. Nothing asserts the 2.0 column; it is the diagnostic the
paragraph carries.

**The candidate the step was opened on moves nothing.** `ecd735e` (S161,
`load_FEN` clears fields the board cannot support) is dated 2026-08-22 and
`21b4a21` is 2026-08-21, so it lands *after* the last mover, and `21b4a21`
already prints exactly what HEAD prints.

**The weights were excluded by measurement, not by the anchors alone.** Dumping
the model's starting vector at both shas gives an identical set of non-zero
entries at identical values, and dumping the model's features -- phase, four
mobility counts, nine king-safety counts, six passer buckets, three pawn-structure
counts, four placement counts, tempo -- on a position that entered the set gives
identical numbers too. What changed was neither the weights nor the features but
the clamp applied to their product.

## Classification

Both movers are **(a)**: a deliberate change whose effect on this reading is
correct, recorded beside the counts. The margin is a search parameter that
governs where the two implementations are *forced* to agree, so a reading taken
at one margin is not comparable with a reading taken at another; the arch flag
makes the model's arithmetic no less right than it was. No `load_FEN`,
`evaluate()` or model defect (b), and the earlier reading was on the corpus and
the binary the paragraph claims (c) -- it reproduces at its own sha today.
The BUGS rule does not arm.

DEC-169 records the consequence: the four pinned positions are re-derived after
a refit **and** after any move of `LAZY_EVAL_MARGIN`, which is what S039 exists
to do, and the `GOLDEN (DEC-142)` note at their site says so.

## The script's first run was vacuous on two rows

Worth recording because it is the class this project keeps finding. The first
version of `adocs/data/S206_truncation_drift.sh` passed the word `HEAD` to
`git -C "$WT" checkout`, which resolves in **the worktree**, not the
repository -- so the two rows labelled HEAD re-measured whatever the previous
row had left checked out, and both still printed `as recorded`: row 6 happened
to follow `21b4a21`, which reads HEAD's numbers, and row 8 followed `883c255`,
where the margin is 150 already, so the counterfactual's `sed` matched nothing
and it re-measured its own predecessor. `8 of 8` with two rows proving nothing.
The sha is resolved in `$ROOT` before the worktree exists now, and the margin
counterfactual greps for the 184 it is about to replace and exits 2 if it is
not there, so the same edit against a future margin fails loudly instead of
passing.
