id:         S142
goal:       the two declared parameter ranges that contradict the purpose stated beside them are narrowed to what that purpose and the tests support
accepts:    `RFP_MIN_PLY`'s declared minimum is **2**, the owner's decision of 2026-08-21 (DEC-095), and it is set only after S145 has re-derived the mate-safety test set and re-measured the floor against it -- the floor stands or moves on S145's evidence, and if S145 finds the floor is not 2 then this step sets what S145 measured and records the difference; `RFP_MAX_DEPTH`'s bound stays at 63 by the same decision and only its comment is corrected, because 15 passed the full mate suite and no defect was demonstrated -- the comment claims the bound keeps reverse futility to "the last few plies" and 63 permits every depth, so the comment is what is wrong; `ORDER_HISTORY_MAX`'s declared maximum no longer admits a value that breaks the 100-point band clearance S093's accepts reasons from, with the clearance asserted arithmetically in a test rather than argued in a comment; behaviour neutrality for the shipping build is proven rather than asserted -- bounds are consumed only inside `#ifdef CHESSO_TUNE` (`src/search_params.cpp:75`, `src/chesso.cpp:942`, `:1064`), so the claim is that the release binary is byte-identical, and it is checked; `tests/test_search_params.cpp` and MANUAL.md's tune-build option table agree with the new bounds; the fast suite green
touches:    src/search_params.hpp, tests/test_search_params.cpp, MANUAL.md, adocs/decisions.md
excludes:   narrowing `RFP_MAX_DEPTH`'s bound, decided against on 2026-08-21 (DEC-095) -- its comment is corrected here, its number is not touched; re-deriving the mate-safety tests, which is S145's and which this step is paused behind; changing any default, which is a play-altering change and belongs to the step that measures it; re-running S085's SPSA on the narrowed bound
decisions:  DEC-084, DEC-095
closes:     2026-08-20_plan_review-F08, 2026-08-20_plan_review-F14
blocks:
paused_by: S145  # 2026-08-21
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
author:    Maksym Bodnar

## Amended by DEC-095, 2026-08-21

Two changes from the owner's decision, and one of them is why this step is
paused.

**`RFP_MAX_DEPTH`: the comment is wrong, not the number.** 15 passed the full
mate suite, so nothing is demonstrated against it and the bound stays at 63.
What is wrong is the sentence claiming the bound "keeps the assumption to the
last few plies", which 63 has never done and which 15 makes visibly untrue --
median search depth at the tuning control was 11, so reverse futility now fires
at every depth this engine reaches. Correct the sentence; leave the range for
the tuner.

**`RFP_MIN_PLY`: the floor is 2, and S145 has to earn it first.** The owner's
objection is the operative part of the decision and it is not a quibble: the
three mate cases that establish the floor were **hand-picked by him for a
different engine**, on the mailbox and bitboard branches, and they are now the
only thing standing between the tuner and a value it spent 72.5 % of its
iterations pushing toward. Three positions chosen for other code is not a
sample and not evidence about this engine.

So the number 2 is decided and the *evidence for it* is not yet in. S145
re-derives the test set -- from the published technique for how pruning is kept
from hiding mates, and from positions this engine actually reached rather than
positions someone picked -- and re-measures the floor against it. This step
sets whatever that comes back as, and records the difference if it is not 2.

The order matters and is not caution for its own sake: setting the bound first
and validating the tests afterwards would mean the tuner's next run is fenced by
a number whose justification arrived later.
