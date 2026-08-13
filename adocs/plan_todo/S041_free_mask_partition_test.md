id:         S041
goal:       a test that fails the moment a tuner group range is appended without re-ending the one before it
accepts:    a test asserts that every group's range is non-empty, that the groups are pairwise disjoint, and that their union is exactly [0, PARAM_COUNT); it was observed failing against a deliberately re-introduced swallowing bug
touches:    tools/tuner.cpp free_mask (split so a test can reach it), tests/
excludes:   the fit itself, its objective, and its parameters
decisions:  DEC-041
closes:     2026-08-13_adversarial-F07
blocks:
paused_by:
done:

## The recurring defect

`tools/tuner.cpp:157-209`. Four consecutive comments in one function record the
same bug four times: `king_safety` ran past the passed pawn weights,
`passed_pawns` past the pawn structure weights, `pawn_structure` past the piece
placement weights, and the fourth comment says outright that it is "the fourth
term in a row to meet the same defect in the same function".

The fifth is already loaded. `tempo` ends at `PARAM_COUNT`
(`tools/tuner.cpp:198-200`), so the next group appended after it is swallowed
unless whoever appends it remembers to move that end.

```
$ grep -rn "free_mask\|GROUP_LIST" tests/
(no output)
```

Nothing in `tests/` links `tools/tuner.cpp` at all. `test_eval_model` is the
only test reaching into `tools/`, and it tests the model, not the mask.

## Why it stays invisible

A swallowed group hands the new term's weights back exactly as it received them,
which reads as "the fit found nothing" — and DEV_MANUAL already teaches that a
term fitting to almost nothing is normal, because residual fits do that. So the
failure is silent and specifically camouflaged by the surrounding workflow.

One occurrence costs a wasted fit plus the SPRT after it, which S027 prices at
three to four and a half hours.

## Shape

No dataset needed. The three properties are enough, and the union clause is the
one that fires the moment a group is appended without re-ending its predecessor.

`free_mask` is about 50 lines with no dependencies. Either split it into a
header or compile the test against the translation unit.

Red first: re-introduce one of the four recorded swallowings, observe the union
assertion fail, put it back.
