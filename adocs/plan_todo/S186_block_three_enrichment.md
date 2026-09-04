id:         S186
goal:       the DEC-097 enrichment pass runs over block 3's step files before block 3 starts -- every evaluation figure traced to its source or marked unverified, every technique's form checked against the wiki's definition, every seed in a DEC-105 form -- so the evaluation block is executed against figures somebody can check
accepts:    each of S134, S082, S083, S135, S136, S039, S121, S123, S125, S118, S101, S122, S124, S102, S133 and S126 carries a "Technical details (SOTA research ...)" section in the form the block-1 files have; every figure in it has a URL or the word unverified beside it; every seed is a literature value with its URL, a derivation the step runs at its start, or a range midpoint (DEC-105, DEC-134); the wiki's definition of each technique is cited and any place the step's form departs from it is stated; the engine records are read as commit messages, pull-request bodies, changelogs and release notes only, never as source or tables (DEC-016), and every file read is listed; S185's unverified list is the work list and each entry is resolved or stays marked with what was searched; block 3's ordering paragraph in `adocs/plan.md` is re-read against the traced figures and any reordering is proposed as a decision, not made as an edit; the pass lands as one commit per file or per few files so a review can follow it; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/plan_todo/, adocs/plan.md
excludes:   the block-1 and block-2 files, enriched 2026-08-19 to 2026-08-20, whose corrections are S180 and S181; any engine code; running any derivation, which the owning step does at its start
decisions:  DEC-097, DEC-105, DEC-134, DEC-137
closes:
blocks:
paused_by:
author:
done:

## Why this exists

DEC-097 (2026-08-21) resumed the SOTA enrichment of the pending step files
"in parallel, on nights". By 2026-09-04 `git log --since=2026-08-21 --
adocs/plan_todo/` showed 40 commits and none of them an enrichment, and 35 of
54 pending files carried no "Technical details" section -- all of block 3 and
the reserve among them. Block 3 is ordered by figures the review had to go and
find (S185 records them), and S133 was added on two numbers that turned out to
be release bundles. A standing promise for nights produced nothing in
fourteen; DEC-137 makes it a step with a place in the order: before S134, the
first block-3 entry, so that no evaluation step starts on a figure nobody can
check.

## How the pass is done

The block-1 enrichment of 2026-08-19 is the template for the section's shape.
Two rules changed since then and both bind here. DEC-105 and DEC-134: a
constant from another engine, wherever it is quoted, is not a seed -- a
literature value with its URL, a derivation over chesso's own data written as
a procedure, or the range midpoint. DEC-135: a citation into code names a
symbol, not a line. The evidence standard is the review's own: a figure
without a URL is written as unverified, and unverified figures carry no weight
in an ordering argument.

The sources are the ones the literature check used and lists:
`mhouppin/stash-bot`'s `CHANGELOG.md`, the Ethereal release notes and commit
messages, Berserk's and Weiss's pull requests, Leorik's release notes, the
wiki's Evaluation pages -- read for form and figure, never for source or
tables (DEC-016).

## Cost

Documents only, sixteen files, several hours spread over the machine-scope
lane's idle moments. It owes no match and no run.
