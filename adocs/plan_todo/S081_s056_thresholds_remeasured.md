id:         S081
goal:       S056's four merged disagreements are measured on the pinned positions the test loads today
accepts:    `S056`'s paragraph quotes the four disagreements measured on the `truncation_positions` at HEAD, with the command that produced them, or drops the four literals entirely and leaves S055 to measure and re-pin in its own commit as its `accepts:` already requires; S056's citations into `tests/test_eval_model.cpp` resolve; S056's conclusion is restated only if the re-measurement changes it
touches:    adocs/plan_todo/S056_s055_pinned_threshold_retarget.md
excludes:   S055 itself, the merge, and the thresholds in `tests/test_eval_model.cpp`, which S055 re-pins in its own commit; `tools/eval_model.hpp`
decisions:  DEC-053
closes:     2026-08-16_plan_review-F03
blocks:
paused_by:
done:

## Why this exists

S056 re-targets the four pinned thresholds S055 would break. `S056:22-29`:

> Those four were pinned *because* they exceed 2.0 under today's three effective
> truncations. [...] the four merged disagreements are 1.8750, 1.3333, 1.2500
> and 1.1250

Those values were measured on the pinned set as it stood on 2026-08-13. S065
replaced the set outright:

```
$ git log --oneline -S'2r1r1k1/4Q1p1' -- tests/test_eval_model.cpp
33aa3b4 Land S065's fit with the queen re-anchored, DEC-059
b904d1b Complete S038: the model guard's slack matches the truncation arithmetic
```

`tests/test_eval_model.cpp:227-232` now pins four different FENs, and the
distribution moved with them: `:392` records "Every position pinned above sits at
69/24 = 2.875, all three losing their maximum at once", where the old set ran
2.1250 to 2.8750.

**The conclusion survives and the numbers do not.** `2 x 23/24 = 1.917 < 2.0`, so
the merge still falsifies `difference > 2.0` for every position, which is S056's
argument. But the four literals are what a session copies into S055 to re-pin
against, and they were derived from FENs the test no longer loads.

## Why this is not left to S055

S056 sits at entry 60 and S055 at 61, and S056's output is the text S055's gate
is read from. Thresholds derived from the wrong positions leave the guard either
pinned too loosely or failing on its first run -- in the one test whose purpose
is to be a bound rather than a number someone widened
(`2026-08-13_adversarial-F04`, S038, DEC-053).

Dropping the literals is an acceptable outcome and is the cheaper one: S056's
`accepts:` already says "each threshold re-measured in S055's own commit", so the
body is the only stale part.

## Cost

Minutes plus one `test_eval_model` run. No match.
