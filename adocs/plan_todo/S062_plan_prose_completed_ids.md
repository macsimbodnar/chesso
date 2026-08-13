id:         S062
goal:       plan.md's prose stops describing completed steps as pending instruments and next work
accepts:    no sentence in plan.md's prose describes a step in plan_done/ as pending or as next; the audit-block paragraph reads S037 in the past tense alongside S035 and S036, the "S044 to S052 ... ahead of everything still pending" clause reads as history, and the S054 paragraph is rewritten as history or dropped; the check is mechanical — every id named in prose is resolved against plan_done/ and the sentence around it agrees
touches:    adocs/plan.md
excludes:   changing the order of the list; renumbering ids; editing any step file
decisions:
closes:     2026-08-13_plan_review.2-F07
blocks:
paused_by:
done:

## What is there

Three prose claims in `plan.md` name completed steps as pending, at the commit
this step was created:

- "S037 is the node count that `search_bench.py` reads, which is how INV-6 is
  discharged — the last of the three instruments still pending."
  `adocs/plan_done/S037_cumulative_info_nodes.md` exists.
- "**S054 is first of everything pending**, and it is not an audit finding."
  `adocs/plan_done/S054_clang_format_untracked.md` exists.
- "S044 to S052 are the 2026-08-13 plan_review audit's nine findings, one step
  each, ahead of everything still pending". All nine are in `plan_done/`.

plan.md is the second document in the reading order and the prose is what a cold
session reads before the list. It currently says the instrument that discharges
INV-6 does not exist yet, which is an argument against making any neutrality
claim, and names a completed step as the next thing to do. The list itself is
right, and plan.md says an id named in a sentence is prose that is not checked —
but these are not ids in passing, they are status claims about tooling and about
what to work on next.

`2026-08-13_plan_review-F05` reported the same class ("the stale harness prose
tells a cold session the project's only verdict instrument is still broken");
S048 cleared it and four completions later it is back. The habit that prevents
the next recurrence is part of the fix: at step completion, re-read the prose
that names the completed id.

Full evidence: 2026-08-13_plan_review.2-F07.
