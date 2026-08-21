id:         S142
goal:       the two declared parameter ranges that contradict the purpose stated beside them are narrowed to what that purpose and the tests support
accepts:    `RFP_MIN_PLY`'s declared minimum is **2**, the owner's decision of 2026-08-21 (DEC-095), and it is set only after S145 has re-derived the mate-safety test set and re-measured the floor against it -- the floor stands or moves on S145's evidence, and if S145 finds the floor is not 2 then this step sets what S145 measured and records the difference; `RFP_MAX_DEPTH`'s bound stays at 63 by the same decision and only its comment is corrected, because 15 passed the full mate suite and no defect was demonstrated -- the comment claims the bound keeps reverse futility to "the last few plies" and 63 permits every depth, so the comment is what is wrong; `ORDER_HISTORY_MAX`'s declared maximum no longer admits a value that breaks the 100-point band clearance S093's accepts reasons from, with the clearance asserted arithmetically in a test rather than argued in a comment; behaviour neutrality for the shipping build is proven rather than asserted -- bounds are consumed only inside `#ifdef CHESSO_TUNE` (`src/search_params.cpp:75`, `src/chesso.cpp:942`, `:1064`), so the claim is that the release binary is byte-identical, and it is checked; `tests/test_search_params.cpp` and MANUAL.md's tune-build option table agree with the new bounds; the fast suite green
touches:    src/search_params.hpp, tests/test_search_params.cpp, MANUAL.md, adocs/decisions.md
excludes:   narrowing `RFP_MAX_DEPTH`'s bound, decided against on 2026-08-21 (DEC-095) -- its comment is corrected here, its number is not touched; re-deriving the mate-safety tests, which is S145's and which this step is paused behind; changing any default, which is a play-altering change and belongs to the step that measures it; re-running S085's SPSA on the narrowed bound
decisions:  DEC-084, DEC-095
closes:     2026-08-20_plan_review-F08, 2026-08-20_plan_review-F14
blocks:
paused_by:
done:      RfpMinPly 0 -> 2 and OrderHistoryMax 899999 -> 699900, RfpMaxDepth's 63 untouched and its comment corrected; the floor re-measured before it was set and reproduced S145 digit for digit, 16/16 mates in two at delay 0 from 2 up against 13/16 with delays to 7 below; release binary byte-identical, 3b83350b, with the mechanism checked -- the bounds are not in the shipping binary at all; fast suite 18 of 18 in 14.80 s release and 14.90 s tune, format clean, README checked owner-written

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

## What landed, 2026-08-21

**The floor was re-measured before the bound was set, not read off S145.**
`S145_rfp_sweep.py floor` re-run at this commit against the 48 constructed mates
with `RfpMaxDepth` held at 15 reproduced the log digit for digit: 19/48 at
`RfpMinPly` 0 and 1 with the mate-in-two class 13/16 and delays reaching 7,
24/48 at 2 and 3 with 16/16 at delay 0, 27/48 at 4 and 5. So DEC-095's number
stands and the minimum is 2.

**`ORDER_HISTORY_MAX`: 699900, not F14's suggested 699999.** The accepts asks
for the *100-point* clearance; 699999 leaves one point. `ORDER_COUNTER` is
700000 and is the band immediately above history -- the last branch
`score_move()` takes before returning the raw table entry -- so the bound is
`700000 - 100`. Derived from `src/evaluation.cpp:33-37` and `:1169` and from the
clamp at `src/search.cpp:768`, all three read rather than taken from the
finding. The shipping default of 600000 is untouched and well below it.

**The clearance test is in `tests/test_evaluation.cpp`, not in
`tests/test_search_params.cpp`.** The bands are `#define`s private to
`evaluation.cpp` and the file already has the mechanism for reaching them -- the
"bands are strictly ordered" case asserts relative order through `score_move()`
rather than naming the constants. Nothing was exported, which also keeps the
release binary out of it. The new case reads the ceiling from
`search_param_info()` so it follows the declared range instead of restating it,
and it derives the 100 from a king-takes-pawn capture in its own position rather
than quoting it from a comment.

**The declared ranges got a golden of their own.** `golden_defaults` held
defaults only; nothing anywhere held a bound, since the release build never
reads one and the tune build's `option` lines are generated from the same table
`test_uci_surface` compares them against. Writing the 22 rows by hand caught two
transcription errors of my own before the suite did.

**Both bounds observed red under their own mutation** before being called done:
`OrderHistoryMax` back to 899999 gives `CHECK( -199999 >= 100 )` in
test_evaluation and `CHECK( 899999 == 699900 )` in test_search_params;
`RfpMinPly` back to 0 gives `CHECK( 0 == 2 )`.

**Byte-identity, and why it is not a vacuous check.** The release binary is
`3b83350b0777c65599d05ec61e5cad258fd2e4d3539b1e0b40cffb60bf7916e6` at `8a3e3b2`
and after the change. Two controls: `touch src/search_params.hpp` recompiles 19
translation units and reproduces the hash, and moving `RfpMargin`'s *default*
from 63 to 64 in the same file moves it to `5662ec8d`. The mechanism was then
checked rather than assumed -- `chesso_engine` is a static library and the
release `chesso` references nothing in `search_params.cpp.o`, so the linker
never pulls it in: `strings build/src/chesso | grep -c OrderHistoryMax` is 0
against 1 on `build-tune`, and `nm` finds no `search_param_*` symbol there. The
table's bounds are not in the shipping binary at all, which is a stronger
statement than the accepts' "consumed only inside `#ifdef CHESSO_TUNE`".

## Four things outside the accepts, and why each was done

1. **`adocs/data/S145_rfp_sweep.py` filters to the declared range.** Raising the
   floor to 2 made `FLOOR_VALUES = [0, 1, 2, 3, 4, 5]` ask for two settings the
   tune build now refuses, and python-chess raises `EngineError` rather than
   sending them -- so the documented command in DEV_MANUAL.md would have ended
   in a traceback halfway through a table. The script now drops those settings
   and prints which and why. No wrong data was ever possible: the engine refuses
   loudly (`info string refused [RfpMinPly] value 1, outside [2, 63]`, S137) and
   python-chess refuses before sending. DEV_MANUAL.md carries the same sentence.
2. **`beta < MATE_MIN` removed from the list of guards** in `RFP_MARGIN`'s
   comment. S145 measured it inert -- `evaluate_expensive()` is clamped to
   `+/-LAZY_EVAL_MARGIN`, so the static score cannot approach the mate band --
   and `src/search.cpp:512-517` already says so. A comment naming a guard that
   never binds is a doc claim about code that is false (AGENTS.md section 7).
3. **`RFP_MAX_DEPTH`'s "the deepest node the assumption is made at"** is
   corrected to the largest *remaining* depth. The guard is `depth <=
   RFP_MAX_DEPTH` at `src/search.cpp:521`, a distance to the leaves. Same class
   of backwards reading as `8b4f63d` fixed in MANUAL.md and specs.md, and that
   commit's wording was read first so this neither undoes nor duplicates it.
4. **`adocs/specs.md`'s tune-build paragraph** now says the declared ranges are
   pinned by `test_search_params` too, since the sentence enumerating what that
   test holds became incomplete in this commit.

## Left open, deliberately

- **No new decision was written.** DEC-095 covers both reverse futility bounds,
  and `ORDER_HISTORY_MAX`'s narrowing is spelled out in this step's own accepts.
  Nothing was met that DEC-095 does not cover.
- **`tools/spsa_s085.json` still declares `RfpMinPly` `min: 0`** and is not
  touched: it is the record of what S085 actually ran, and rewriting it would
  rewrite that record. A rerun from it now fails preflight with a named error --
  `tools/spsa_driver.py:450-455` compares the config's bounds against the
  binary's and refuses on a mismatch -- so the stale file is loud, not silent.
  The next run writes its own config.
- **"13 of 16" is the sweep's exact count, not the number of assertions that
  fail** at `RfpMinPly` 1. The comment says so and points at S154, which owns
  the restatement (`2026-08-21_adversarial-F06`: 9 of 16 pass, because the
  assertion also requires `first_exact == 2m - 1`).
