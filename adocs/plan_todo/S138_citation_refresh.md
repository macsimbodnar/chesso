id:         S138
goal:       every file:line citation and test title in a pending step file resolves to what it claims, and the five steps pointing at the wrong mate test are re-pointed
accepts:    a tracked check re-resolves every `path:line` and `path:line-line` citation in every pending step file against HEAD and reports the ones whose line no longer holds what the citing sentence says it holds, and it is run and green at completion; the five steps citing `tests/test_search.cpp:1274` as the mate-safety gate to extend name the two tests that actually carry it -- `pruning does not hide a forced mate` and `pruning does not hide a mate against the material leader` -- by title as well as by line, so the next drift is survivable; every accepts clause gating on "the mate-in-quiescence case" names a test title that exists or states the test it needs written; S115's accepts names the file its 300 positions actually come from; no citation is corrected by deleting it
touches:    adocs/plan_todo/, tools/ for the checker
excludes:   the content of any pending step beyond the reference it cites wrongly; adocs/plan_done/, which is history and is never rewritten; re-measuring the numeric claims those steps make, which the audit listed as deferred and S143 would own if the owner wants it
decisions:  DEC-084
closes:     2026-08-20_plan_review-F01, 2026-08-20_plan_review-F12, 2026-08-20_plan_review-F13
blocks:
paused_by:
done:

## Why this is first and why it is high

70 of 147 citations in the 51 pending step files no longer hold the text they
held when written, shifted by up to 613 lines, and the cause is ordinary: four
steps (S100, S106, S107, S137) landed after the SOTA enrichment pass wrote the
citations. A stale line number is usually harmless noise. Six of these are not.

**Five pending steps -- S091, S097, S098, S109 and S114 -- instruct their
implementer to extend the mate-safety gate at `tests/test_search.cpp:1274`.**
At HEAD that line is inside
`TEST_CASE("a mate bound is compared after the ply adjustment, not before")`,
which is a test about comparing a mate bound, not the gate that catches pruning
hiding a mate. The two tests that carry that gate are at :1887
(`pruning does not hide a forced mate`) and :1926
(`pruning does not hide a mate against the material leader`).

Every one of those five steps adds pruning or reduction. CLAUDE.md's standing
hazard is that "pruning that hides a mate is the recurring bug", caught twice
already "by a mate test in the fast suite, not by a benchmark". So the plan
currently tells the next five implementers of exactly that hazard to extend the
wrong test -- and a step that extends the wrong test and goes green has bought
nothing while reading as covered.

Independently confirmed the same night from the other direction: S085's
`RfpMinPly` measurement found that the tests which actually go red when the
mate-safety gate is loosened are :125, :1887 and :1926, and no others.

## The checker is the point, not the 70 edits

Correcting 70 line numbers by hand buys nothing that survives the next landing.
What survives is a check that fails when a citation drifts, which is why the
accepts asks for one in `tools/` rather than for a tidy diff. `plan_prose_check.py`
is the nearest existing thing and is where this probably belongs.

Titles alongside line numbers are the second half: a `file:line` is a moving
reference and a `TEST_CASE` title is a stable one, so a citation carrying both
degrades to something still findable rather than to something silently wrong.

## Cost

No match, no verdict. Document work plus a checker, and the fast suite green.
