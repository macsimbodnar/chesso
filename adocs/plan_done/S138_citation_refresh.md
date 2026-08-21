id:         S138
goal:       every file:line citation and test title in a pending step file resolves to what it claims, and the eight steps pointing at the wrong mate test are re-pointed
accepts:    a tracked check re-resolves every `path:line` and `path:line-line` citation in every pending step file against HEAD and reports the ones whose line no longer holds what the citing sentence says it holds, and it is run and green at completion; the **eight** steps citing the old mate-safety line numbers -- S091, S095, S097, S098, S109, S113, S114, S116, three more than the audit's grep found because three cite the gate as a bare `:1274` continuation -- name the two tests that actually carry it -- `pruning does not hide a forced mate` and `pruning does not hide a mate against the material leader` -- by title as well as by line, so the next drift is survivable; every accepts clause gating on "the mate-in-quiescence case" names a test title that exists or states the test it needs written; S115's accepts names the file its 300 positions actually come from; no citation is corrected by deleting it
touches:    adocs/plan_todo/, tools/ for the checker
excludes:   the content of any pending step beyond the reference it cites wrongly; adocs/plan_done/, which is history and is never rewritten; re-measuring the numeric claims those steps make, which the audit listed as deferred and S143 would own if the owner wants it
decisions:  DEC-084
closes:     2026-08-20_plan_review-F01, 2026-08-20_plan_review-F12, 2026-08-20_plan_review-F13
blocks:
paused_by:
done:      107 of 201 gated code citations were stale, not the audit's 70 of 147, and all 107 are corrected -- 89 relocated mechanically with content-at-new-lines asserted byte-identical to content-at-old-lines, 24 resolved by hand with the cited lines asserted to contain what the sentence claims, none removed. tools/plan_prose_check.py --citations is the checker: BOUNDS, ANCHOR and DRIFT, exit 1 on a flag, verified red at 107 flagged against 04bcc43 and red again here on an injected :99999 with a BOUNDS diagnostic naming the file's real length, green either side. Eight steps pointed at the wrong mate test, not five: S095, S113 and S116 wrote it as a bare (:1274) continuation and the audit's grep missed them, and all three add pruning or a reduction. 13 sites re-pointed to :1887 and :1926 naming both TEST_CASE titles verbatim. F13 had five sites not four; F12's source is adocs/data/S018_raw.tsv via S021_aspiration_sweep.py, whose own docstring said 32 positions and is corrected to the 100 per sample its PER_PHASE and the corpus derive. Deliberately not in ctest: any source commit shifts lines under fifty step files, so gating the suite would make red normal and this check the thing weakened to clear it. 467 of 668 references are bare continuations that no checker can resolve without guessing a path, which is S144. Both suites 18/18, format clean, validate clean, README owner-written, MANUAL and DEV_MANUAL checked.

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

(The count above is what was believed when this step was planned. It was eight
steps and not five, and 107 citations and not 70 -- see "As executed". The
paragraph is left as written because it is the reasoning the step was started
on.)

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

## As executed

**The checker is `tools/plan_prose_check.py --citations`**, an extension of the
existing plan-hygiene tool rather than a new one, so completion keeps running
one command. Three failure classes, each an exact string comparison: `BOUNDS`
(path absent at HEAD, or line past end of file), `ANCHOR` (a doctest title
quoted on the citing line, or the line above it, is not the test the cited lines
open), `DRIFT` (the cited lines hold different text now than in the commit that
last wrote the step file). Not registered with ctest, deliberately and for the
same reason `--prose` is not: every source commit shifts lines under fifty step
files, so gating the suite on it would make a red suite the normal state of the
repository and the check the thing that gets weakened.

**Observed red before the fix**, the whole point of the exercise: the same
checker run against a throwaway worktree at `04bcc43` reports **107 flagged
over 56 files, exit 1** -- 101 `DRIFT`, 5 `ANCHOR`, 1 `BOUNDS`. Green after,
0 flagged. The `ANCHOR` class reproduces F01's hazard from the other direction
without being told to look for it: it prints
`"pruning does not hide a forced mate" opens at tests/test_search.cpp:1887`
against S091:165, S109:220 and S114:95. Non-vacuity checked three ways on the
corrected S109:220 citation, reverted after each: `:1266` (a real test, the
wrong one) → `ANCHOR`; `:1888`, one line off → `ANCHOR`; `:99999` → `BOUNDS`.

**The F13 fix broke `ANCHOR` and paying for it made the check better.** Naming
two titles in one accepts clause gave five false positives at once: the window
saw both titles for both citations and demanded each title sit at each line. The
fix pairs them positionally -- a citation answers to the title nearest to its
left in the flattened window, falling back to the whole window only when its own
text cannot be located there. Checked to still discriminate rather than merely to
pass: swapping S112's `:701` for `:1887` is caught as
`"a side in check may not stand pat" opens at tests/test_search.cpp:701`.

**The census is larger than F01's**, because F01's regex took full `path:line`
citations only. 201 gated code citations, 107 of them wrong; the audit measured
147 and 70. The extra come from citations written as a bare basename
(`chesso.cpp:698`), and from `src/search_params.hpp` and `src/evaluation.cpp`
moving again after the audit was written -- S085's tuned vector and `04bcc43`.

**Eight steps pointed at the wrong mate test, not five.** S095, S113 and S116
were missed by the audit's `grep test_search.cpp:1274` because they write the
citation as a bare `(:1274)` continuation of a path named earlier in the
paragraph. All three add pruning or a reduction -- internal iterative
reduction, ProbCut, razoring at depth one -- so they are the same hazard class
as the five. All eight now carry both titles and both full-path line numbers.

**The blind spot, stated rather than hidden.** 467 of the 668 citations are
bare `:line` continuations whose path is inherited from prose, and they are
counted and skipped, not checked. Guessing the path was tried and abandoned on
evidence: S095 writes "returns nullptr on a miss
(transposition_table.cpp:96-103), and :449 already computes", where `:449` means
`src/search.cpp`, and a same-paragraph inheritance rule produced sixty
impossible line numbers. Twelve continuations were corrected anyway -- the ones
sharing a line with a full citation, where leaving them made the line contradict
itself -- each accepted only when the baseline content relocated uniquely and in
bounds. The rest is a real gap and wants either a style rule (repeat the path)
or a step.

**Beyond the accepts, and flagged as such.** S085's tuned vector landed after
the audit and after the enrichment pass, so seven sites quoted shipping defaults
that had moved: `LMR_BASE 75`/`LMR_DIVISOR 225` (now 52/182) in S098 twice and
S109, `NULL_MOVE_BASE` 2 (now 3) in S114, `LAZY_EVAL_MARGIN` 150 (now 184) in
S039, `MAX_QSEARCH_DEPTH` 8 (now 19) in S131. Each is what a cited line holds,
so each was corrected; three sentences of arithmetic derived from them in S114
were corrected with them (NMP fires from depth 5 not 4, the floor bites depths
4..8 not 4..7, R is 3..4 not 2..3), because a half-corrected derivation is worse
than either a wrong one or a right one.

**F13 resolved by naming both cases that exist.** No test is called "the
mate-in-quiescence case". Two are candidates and they guard different things:
`"a side in check may not stand pat"` (tests/test_search.cpp:701) is the only
fast-suite case where quiescence has to *search* evasions to reach a mate -- it
asserts `score < -10000` -- and `"mate is recognised at depth zero"`
(tests/test_search.cpp:775) is a mate already on the board, reported with only
quiescence running. S112 and S131 both change the quiescence move filter, which
can break either, so both accepts name both. That is a wider reading than the
audit's suggestion, which named only the second; S131's body had already
resolved the phrase to the first, and the divergence between the two readings is
exactly the failure F13 predicted. S022:199 carried the same phrase and is
fixed with them -- the audit named four sites, there were five.

**F12**: the 300 positions come from `adocs/data/S018_raw.tsv` through
`adocs/data/S021_aspiration_sweep.py`, not from the results file
`adocs/data/S021_aspiration_sweep.tsv`. Re-derived rather than taken from the
report: `PER_PHASE = 4`, and the phase column of `S018_raw.tsv` has 25 distinct
values over 13522 rows with every one of them holding at least 4 rows, so
`positions()` returns 100, at each of three offsets. **Not fixed, outside
`touches:`**: that script's own docstring still says "Positions are 32 sampled
from adocs/data/S018_raw.tsv" (`adocs/data/S021_aspiration_sweep.py:17`), which
is a one-line correction the audit also asked for.
author:    Maksym Bodnar
