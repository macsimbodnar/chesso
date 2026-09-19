id:         S232
goal:       the seed sections of S099, S110 and S111 are rewritten in DEC-134's three forms, so no constant quoted from another engine's commit message or pull request remains in a pending correction-history step
accepts:    each of the three files' constants-and-seeds section names every seed as one of (a) a value from a publication about the technique with its URL, (b) a derivation over chesso's own data or scale that the owning step runs at its start, or (c) the range midpoint or off value stated as such; the Stockfish commit-prose figures in S099 ("entry / 32", "~32 internal units", "clamp near 1024") leave or move to an anti-seed paragraph that names them as another engine's numbers; S111's pull-request source is cited as record only, never as a seed; S110's twelve unsourced figures are sourced or struck; `tools/plan_prose_check.py --citations` and `--touches` green over the three files; no code, no order change
touches:    adocs/plan_todo/S099_correction_history.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S111_correction_history_continuation.md
excludes:   any change to the three steps' goals, gating or placement (DEC-133, DEC-176 (c)); any code; running S099 -- that is the probe night DEC-133 permits, taken once this step is done
decisions:  DEC-134, DEC-105, DEC-222
closes:     2026-09-19_study_review-F06
blocks:
paused_by:
done:

## Why this exists

DEC-134 (2026-09-04) ruled that a constant quoted in another engine's commit
message is that engine's constant and had S180 reseed seven step files. S099
was not among them and still reads, in its section 4, an applied correction
of "`entry / 32` with the max adjustment ~32 internal units (implying an entry
clamp near 1024 in SF's scale) -- SF commit message prose"; its hedge, "take the
shape, not the number", does not remove the numbers from the page an
implementer reads. S111 names a Stockfish pull request as its source and S110
carries twelve figures the 2026-09-04 literature check could not source (row
A22). The 2026-09-19 study review found this while checking the study's first
recommendation (`adocs/audit/2026-09-19_study_review.md`, F06), and the owner
chose to run S099 as DEC-133's probe on the next idle night -- which cannot
happen while its seeds are another engine's.

## Shape

A document step in S180's pattern: each seed rewritten in one of the three
forms with the form named, anti-seeds kept as record in a paragraph that says
so, and the citation checker run over the three files. Nothing in `src/`
moves, so no `Bench:` line is owed; the completing commit says `No functional
change` only if it touches `src/`, which it does not.

## Cost

An hour of agent time, no machine. It gates the probe night and nothing else.
