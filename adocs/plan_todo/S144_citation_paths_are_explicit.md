id:         S144
goal:       a citation in a plan document carries its own path, so the 467 bare line references that inherit a path from prose become checkable
accepts:    a stated style rule that a citation repeats its path, written where the plan documents' conventions live; the bare `:line` references in pending step files carry a path, resolved by reading the prose that owns each one and never by guessing -- a guess is what this step exists to remove; `tools/plan_prose_check.py --citations` gates the converted references instead of reporting them as `loose, ungated`, and the count it reports as ungated falls to zero or to a stated remainder with a reason per item; the checker is observed red on a converted reference given a wrong path before it is trusted; no reference is resolved by deleting it
touches:    adocs/plan_todo/, adocs/plan.md, adocs/specs.md, tools/plan_prose_check.py
excludes:   adocs/plan_done/, which is history and is never rewritten; the numeric claims those sentences make, which the 2026-08-20 plan_review listed as deferred; re-anchoring the full citations, which S138 did
decisions:
closes:
blocks:
paused_by:
done:

## The blind spot S138 could not close

S138 re-anchored and now gates **201 full `path:line` citations**. It could not
touch the other **467 of 668**, which are bare `:line` continuations whose path
is inherited from a sentence rather than written down -- `(:1274) and :1314`,
`:1887`, `:98-102`. A checker cannot resolve those without deciding which file
the prose meant, and deciding wrongly is worse than not checking: S138 reports
that guessing the inherited path produced **sixty impossible line numbers**, and
that S095 cites `transposition_table.cpp` and means `src/search.cpp` two words
later.

So they are stale in bulk, in the same way and for the same reason the 107 full
citations were, and nothing reports it. `--citations` counts them as
`loose, ungated` precisely so the gap is visible rather than silently uncovered.

## Why a style rule and not only a pass

Converting 467 references once leaves the next 467 to be written the same way.
The rule -- a citation repeats its path -- is what makes the checker's coverage
grow with the documents instead of decaying against them, and it is the same
reasoning S138 used for naming a `TEST_CASE` title beside a line: a `file:line`
is a moving reference and the cheapest defence is redundancy a checker can read.

## The evidence that this is not hypothetical

S138 found that **eight** pending steps, not the five the audit reported, told
their implementer to extend the mate-safety gate at the wrong line. The three it
added -- S095, S113, S116, all of them pruning or reduction steps and so all of
them the hazard CLAUDE.md says has already bitten twice -- were missed by the
audit **because they wrote it as a bare `(:1274)` continuation** and the audit
grepped for the full path. The blind spot has already hidden real defects from
one review.

## Cost

No match, no verdict. Document work plus the checker, and the fast suite green.
