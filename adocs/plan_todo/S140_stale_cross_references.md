id:         S140
goal:       no plan, specs or status document routes work to a retired id, cites an invariant that is defined nowhere, or carries a census its own derivation contradicts
accepts:    `adocs/specs.md` no longer routes check extensions to S096, which DEC-087 retired, and says instead what the current answer is -- this one matters because specs outranks plan by precedence, so the retired route is the one a reader is told to trust; the three `excludes:` fields naming retired step ids either name a live step or state that the work is retired and why; `plan.md`'s block 3 prose names S134, S135 and S136, which its own list contains but its prose omits; INV-7 is either defined in `specs.md` with a number and a testable property or the commits referencing it are the only record and `specs.md` says which invariant they meant; `status.md`'s enrichment census is regenerated from `grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md` and agrees with it, or states that the census is a snapshot and carries its date
touches:    adocs/specs.md, adocs/plan.md, adocs/plan_todo/, adocs/status.md
excludes:   the enrichment pass itself, which is the owner's to resume; any renumbering of ids, which never happens; adocs/plan_done/
decisions:  DEC-087
closes:     2026-08-20_plan_review-F05, 2026-08-20_plan_review-F10, 2026-08-20_plan_review-F11, 2026-08-20_plan_review-F16, 2026-08-20_plan_review-F17
blocks:
paused_by:
done:

## The one with teeth

Reading protocol precedence is **specs > plan > status**. `specs.md` still sends
check extensions to S096 and DEC-087 retired S096 -- Ethereal and Stormphrax both
removed check extensions for a gain, LMR here already exempts checking moves, and
S097 covers the forcing-line concern. So the document a reader is told to trust
above the plan points at work the plan has deleted. The rest of this step is
hygiene; this part is a contradiction between the two top documents.

**INV-7 is referenced by the repository's own history and defined nowhere.** An
invariant number that commits cite and `specs.md` does not define is either a
missing invariant or a wrong citation, and both are worth one line to settle.

## Cost

No match, no build. Document work.
