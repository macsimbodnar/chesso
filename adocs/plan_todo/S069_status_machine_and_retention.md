id:         S069
goal:       status.md's machine paragraph agrees with specs.md and its retention example is stated in positions
accepts:    status.md's measurement-capacity item prices an SPRT the way specs.md does and names no macOS daemon and no machine still to be acquired; its retention item states the rule in positions and cites plan.md rather than listing ids that the window keeps dropping; `moltke --validate` clean and the fast suite green
touches:    adocs/status.md
excludes:   adocs/specs.md, which is the correct one and the source this copies from; plan.md:130-147, which S064 already made correct; the `Last done` field itself, which is generated and whose disagreement with the newest completion is what the item explains
decisions:
closes:     2026-08-16_plan_review-F05
blocks:
paused_by:
done:

## Why this exists

`status.md` is the first file AGENTS.md section 1 names, and precedence runs
specs > plan > status, so where the two disagree status is the wrong one. Two of
its parked items disagree.

**The machine.** `adocs/status.md:53-59` still prices a verdict "at 10+0.2 with
three usable cores ... about an hour", blames `opendirectoryd`, and asks for "an
x86-64 Linux box ... needed for S032 and S029 regardless". `adocs/specs.md:189-194`
was rewritten for DEC-049 and says three to four and a half hours on all 12
threads, and that S032 and S029 now have the box. `opendirectoryd` is a macOS
daemon. `S068:48` prices S033's verdict at 44 m 10 s at 12 cores for 1012 games.

That paragraph is the one a session reads before deciding whether it can afford
a measurement, which is the decision the whole plan is rate-limited by.

**The retention example.** The first parked item explains why `Last done` reads
S053 while the newest completion is later, and its worked example has gone stale
three times over: three completions landed after S065 (`fca9522` S033, `a124f07`
S062, `2e7f561` S064); S038, S066 and S067 have no list entry at all, so the
"S038, S066, S067 and S065 sit at 38 to 41" line names positions that no longer
exist; and S053 is at position 67, not 66.

The item is right about the mechanism and wrong in every number, which is the
worst combination: it reads as a measurement and is a memory.

## What replaces it

The mechanism, not the census. Retention is a window over list positions and the
generated field follows it; `plan.md:130-147` states that and is correct, so the
item points there and stops re-deriving it. An example that has to be re-checked
on every completion is a maintenance cost with no reader.

## Cost

Minutes, no build, no match. It alters nothing that plays.
