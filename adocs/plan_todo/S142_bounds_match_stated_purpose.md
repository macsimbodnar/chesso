id:         S142
goal:       the two declared parameter ranges that contradict the purpose stated beside them are narrowed to what that purpose and the tests support
accepts:    `RFP_MIN_PLY`'s declared minimum is raised off 0 to the value the owner's decision selects, and the decision is recorded before the edit -- the tested floor is 2 (measured under S085: 3 of 18 mate cases fail at 0 and 1, all pass at 2) and the purpose stated in the comment argues 3, which no test exercises; `ORDER_HISTORY_MAX`'s declared maximum no longer admits a value that breaks the 100-point band clearance S093's accepts reasons from, with the clearance asserted arithmetically in a test rather than argued in a comment; behaviour neutrality for the shipping build is proven rather than asserted -- bounds are consumed only inside `#ifdef CHESSO_TUNE` (`src/search_params.cpp:75`, `src/chesso.cpp:942`, `:1064`), so the claim is that the release binary is byte-identical, and it is checked; `tests/test_search_params.cpp` and MANUAL.md's tune-build option table agree with the new bounds; the fast suite green
touches:    src/search_params.hpp, tests/test_search_params.cpp, MANUAL.md, adocs/decisions.md
excludes:   `RFP_MAX_DEPTH`, whose comment says "the last few plies" while its max of 63 permits every depth -- same shape, no red test, no number for "few", so it is recorded in the audit and left for a decision rather than guessed at here; changing any default, which is a play-altering change and belongs to the step that measures it; re-running S085's SPSA on the narrowed bound
decisions:  DEC-084
closes:     2026-08-20_plan_review-F08, 2026-08-20_plan_review-F14
blocks:
paused_by:
done:

## What was measured, and why the fix is a decision

S085's run walked `RfpMinPly` from 3 to 0 and sat at that bound for 30.8 % of
its iterations, which is what sent anyone to look. Measured two ways that night,
the tune build over UCI and `test_search` with the variable set through `gdb`:

- **0 and 1 are the same engine.** The guard is `!is_pv && ... ply >=
  RFP_MIN_PLY` (`src/search.cpp:521`) and the root is entered at :853 with
  `is_pv` true, so `!is_pv` exempts the root at every setting and this parameter
  never sees ply 0. Byte-identical node counts confirm it. So the range contains
  a value no tuner can distinguish -- a wasted axis value independent of the
  mate question.
- **3 of 18 mate cases fail at 0 and 1**; all 18 pass at 2.
- **RFP does fire at ply 2** when the setting is 2 (TRICKY 329598 against
  375687 at 3), and nothing goes red. So the ply-2 exemption the comment argues
  for is an argument no test exercises -- `src/search.cpp:517` already concedes
  as much: "A mate deeper than ply 3 can still be missed for an iteration, and
  no test covers that."

**Which floor to ship is therefore a decision and not a judgement call.** 2 is
what the tests support; 3 is what the stated purpose argues and rests on an
untested claim. The comment in `src/search_params.hpp` was corrected under S085
to say all of this; the bound was deliberately left at 0 so that narrowing it
would be a recorded decision rather than a silent one.

`ORDER_HISTORY_MAX` is the same shape from the other side: its declared maximum
is meant to be the band clearance itself -- a killer scores 900000 -- and S093's
accepts reasons from a clearance the declared range does not actually enforce.
CLAUDE.md lists this as a one-way door: "Tuning `piece_values_abs` can invert
that silently, and the symptom is a strength regression rather than a wrong node
count."

## Cost

No match: the bounds are tune-build-only metadata, so the shipping binary should
come out byte-identical and the accepts asks for that to be checked rather than
believed.
