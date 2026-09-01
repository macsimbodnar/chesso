id:         S154
goal:       the mate-in-three floor's tolerance to a neutral tree change is measured rather than asserted, and the RfpMinPly-1 mate-in-two count is restated as what the assertion actually fails
accepts:    `MATE_IN_THREE_FLOOR`'s tolerance is measured rather than asserted: how many of the sixteen mate-in-three detections move under a change already known to be behaviour-neutral or near-neutral, so the comment's claim that the floor "fails when the guard fails and not when the tree shifts underneath it" rests on a number; the floor is then either widened with the tolerance stated, or kept at 7 with the measurement recorded beside it; the step file and `adocs/data/S145_rfp_sweep.log`'s characterisation of the `RfpMinPly` 1 failure as "13 of 16" is restated as what the assertion actually fails -- 9 of 16 pass, because it also requires `first_exact == 2d-1` -- since the fence is stronger than documented and the next reader sizing the margin needs the real number; no default changes and the fast suite stays green
touches:    tests/test_engine.cpp, adocs/data/, DEV_MANUAL.md, adocs/decisions.md, adocs/plan_done/ is excluded -- see excludes
excludes:   rewriting `adocs/plan_done/S145_mate_safety_test_set.md`, which is history and is never rewritten -- the restatement lands in the test comment and in the sweep log's own header; the constructed set itself, which S145 built and which this step only measures against; `RfpMaxDepth`, which is S148
decisions:  DEC-019, DEC-095
closes:     2026-08-21_adversarial-F06
blocks:
paused_by:
author:     claude opus 5, coordinator
done:       2026-09-01. The tolerance is measured and the floor is 8, because 7 had stopped separating. Three probes at `fc5526e` on the machine `.moltke.local.md` describes, all in `adocs/data/S154_floor_margin_sweep.log`: nine transposition table sizes from 1 MB to 256 MB, the binary rebuilt at each of the **seventeen** commits that touched `src/` since `14748c9` placed the floor, and the `RfpMinPly` axis re-taken with the declared minimum relaxed to 0 in a throwaway worktree. **Positions changing verdict under a tree change with no guard in it: 0** -- every table size, every commit but one, against a node total that moved 5.9 % over the set and 17 % over the mates in three, so the tree shifted and the verdicts did not follow. `aa8c077` (S165) moved exactly one and moved it up. One ply of the guard itself moves five. So the comment's claim was true, and the finding is the other one: S165 lifted both ends of the floor from 8 and 6 to **9 and 7**, `7 >= 7` is green, and from 2026-08-23 to today this assertion could not fail for the reason it exists. **The floor is 8**, observed red at `REQUIRE( 7 >= 8 )` with the default weakened to 1 in a worktree. The mate-in-two restatement is in the test comment, in `adocs/data/S145_rfp_sweep.log`'s new header and in `DEV_MANUAL.md`: 13 of 16 found, **9 of 16 on time**, so what the assertion fails at `RfpMinPly` 1 is seven positions and not three -- corroborated by the real gate logging seven `first reported at iteration` failures by name. Three `RfpMaxDepth` numbers the comment quoted had also moved and are refreshed (40 of 48, 6 of 8, 5 of 8 against 34, 4 and 3); that axis stays S148's. No engine source touched, no default changed, no SPRT owed. Fast suite green, 22 tests, 42.57 s. DEC-116. `2026-08-21_adversarial-F06` closed.

## Why the margin matters now

Plan positions 19 to 31 -- S093, S130, S108, S024, S109, S091, S098, S095, S099,
S097, S112, S131, S022 -- all reshape the tree, several substantially. The floor
sits one position of sixteen below the shipping count of 8. A floor that reddens
on an unrelated tree change is the mechanism S145 itself documented in two
surveyed projects, which "switched the tests off rather than the pruning".


## What was measured, and what it cost

Three probes, because one of them alone answers a different question.

**The noise floor: nine table sizes, and nothing else changed.** `Hash` from
1 MB to 256 MB moves which nodes get a table hit and moves no rule. The node
total over the 48 positions moved 5.9 % and over the sixteen mates in three
17 %, so the tree genuinely reshaped; the mate-in-three verdicts moved by 0
positions. Seven seconds. Without this reading the ref sweep's zeros could not
be told from a probe that moved nothing.

**The realised drift: seventeen builds, one per commit.** Every commit that
touched `src/` since the floor was placed, in order and complete -- a sweep
that picks its own commits picks its own answer. Churn 0 at all of them except
`aa8c077`, which moved one position upward. `-Werror` is dropped in the
worktree because pre-S167 commits do not compile under Apple clang; it changes
no codegen. 111 s.

**The separation: the `RfpMinPly` axis, bound relaxed.** 14 of 16 at 5 and 4,
9 at 3 and 2, 7 at 1 and 0. The churn column is what makes this the sensitivity
reading rather than a count: one ply, from 3 to 4, moves five positions.

**A fourth reading that changed the shape of the answer.** `MATE_DEPTH_SLACK`
looked like the tighter fence -- one position now first reports its mate at
exactly the last iteration searched, where the worst delay was 4 when S145
chose the window. Sweeping it says widening is not the fix: at slack 12 the
shipping guard and the removed guard both find 10 of 16 and the separation is
gone entirely, at 4.1 s against 0.7 s. The window is a cost budget and the
mate-in-three count is a reading of lateness under it. The mate-in-two clause
is the one that does not depend on the window, because it asks for the first
iteration and not the last -- which is also why it is the clause that kept
catching the removed guard while the floor could not.

## What this step ran into that was not in its accepts

**The `accepts` anticipated two outcomes, widen the margin or keep 7 with the
measurement recorded, and the measurement produced a third.** The floor had to
go *up*, not down, because it had stopped separating. Recorded as DEC-116
rather than taken silently.

**`python-chess` was not installed on this machine.** `~/.venv/chess/bin/python`
is the path `TOOLCHAIN.md` and every S145 script names and it did not exist
here, so `adocs/data/S145_rfp_sweep.py` could not run at all. Installed at the
owner's decision, python-chess 1.11.2 on python 3.9.6, and `.moltke.local.md`
records it. `S154_floor_margin_sweep.py` needs none of it -- it drives the
engine over UCI with the standard library, as S156's does, because nothing in
it asks a chess question.

**A refused `setoption` is now an error and not a silent null.** S156 recorded
measuring one engine against itself because `RfpMinPly` 1 is outside the
declared range, is refused, and leaves the default in place. The engine answers
that refusal over UCI since S137, so this harness sends `isready`, reads the
answer, and stops on `info string refused`. The first `slack` sweep taken for
this step hit exactly that and the check is what caught it.

**`adocs/data/README.md` has no rows for S155's or S156's artefacts.** Found
while adding S154's. Not fixed here -- it is those steps' omission and this one
has no mandate over it.
