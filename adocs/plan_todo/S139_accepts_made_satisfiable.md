id:         S139
goal:       every pending accepts field states something the harness can actually produce and the plan order can actually reach
accepts:    S119's accepts asks for its verdict at the S105 harness setting rather than at Hash 128, which is the edit DEC-088's own Consequences line ordered and which was never applied; S117's accepts no longer admits a truncation-behaviour change with "no SPRT owed", because INV-6 allows that only on identical node counts and identical best moves and S117's own body says the bound moves; S125's accepts either drops its dependency on S118's pawn hash or the plan reorders the two, and whichever is chosen is stated in the file; S136's accepts and DEC-092 agree with the taper division count that will be live once S055 has landed, since the plan orders S055 first; S109's gives-check clause either binds every rule it gates or names the rules it binds; each edit is checked against the code or the harness file that decides it, quoted in the step file
touches:    adocs/plan_todo/, adocs/decisions.md
excludes:   implementing any of the five steps; reordering the plan beyond the S118/S125 choice if that is the option taken; the numeric claims the audit listed as deferred
decisions:  DEC-088, DEC-092, DEC-087
closes:     2026-08-20_plan_review-F02, 2026-08-20_plan_review-F03, 2026-08-20_plan_review-F04, 2026-08-20_plan_review-F07, 2026-08-20_plan_review-F15
blocks:
paused_by:
done:

## What unites these five

Each is an `accepts:` field that cannot be satisfied as written, so each is a
step that would either stall at its own gate or pass by ignoring it. Two are
worth stating in full because they are decided already and simply not applied.

**S119 asks for Hash 128.** `adocs/plan_todo/S119_tt_cluster_layout.md:3`
demands "an SPRT verdict at **Hash 128** -- the regime S105 sets", and
`fastchess.sh:225` hardcodes `option.Hash=16` with no override. DEC-088 is the
decision that *chose* 16 over 128, on the ground that what transfers across time
controls is table pressure, and its own `Consequences:` line ordered this clause
changed to "at the S105 harness setting". The edit never happened. 128 MB is
also, in DEC-088's words, the setting that "would flatter every table-hungry
change S119 is about to make" -- so the unapplied edit is not cosmetic, it is
the difference between measuring S119 and flattering it.

**S117 grants itself an exemption INV-6 does not give.** Its accepts admits the
change with "no SPRT owed", and INV-6 allows that only for a change proven
behaviour-neutral by identical node counts and identical best moves. S117's own
body says the truncation bound moves. A step cannot both alter play and skip the
verdict.

## Cost

No match. Document work, with each edit traced to the harness file or decision
that settles it.
