id:         S187
goal:       citations from pending step files into code name a file and a symbol -- a function, a constant, a macro or a test title -- and carry no line number, and the citation checker verifies the symbol exists in the named file, so a source commit can no longer stale the plan
accepts:    every `file:line` citation in `adocs/plan_todo/` is rewritten as the file and the symbol or `TEST_CASE` title it points at (the review counted 549, 58 of them drifted), keeping the symbol S144's rule already puts beside the citation; `tools/plan_prose_check.py --citations` fails a `file:line` form found in `adocs/plan_todo/` or `adocs/plan_current/` as `LINE` and a `file` plus symbol whose symbol is absent from that file at HEAD as `MISSING`, each observed red on a planted case before the conversion and green after; the four citations the review found wrong when written -- S109's accepts, S055's taper divisions, S024's Note, S119's `rating.sh` line -- come out right in the symbol form and S024's Note obeys S024's own accepts; the step states whether the mode joins the fast suite now that it holds no line numbers, and if so `tests/CMakeLists.txt` says why the old reason no longer applies; the "A citation repeats its path" paragraph of `adocs/plan.md` is rewritten for the symbol form and cites DEC-135; `DEV_MANUAL.md`'s citation section says the same; fast suite green in both builds
touches:    adocs/plan_todo/, tools/plan_prose_check.py, tests/CMakeLists.txt, adocs/plan.md, DEV_MANUAL.md
excludes:   `adocs/plan_done/`, which is history and keeps its line citations as written; `adocs/specs.md`'s invariant table, which cites lines into `src/bitboard.cpp` and is not a pending step file -- its own rule is a later decision; `adocs/decisions.md`; the engine
decisions:  DEC-120, DEC-135
closes:     2026-09-04_plan_review-F07
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_plan_review-F07`, the third recurrence of
`2026-08-20_plan_review-F01`'s class. S169 re-anchored 97 citations on
2026-09-01; three days and four source commits later `--citations` flags 59
over 54 files, one of them inside S159's accepts, and the review's own census
agrees at 58 of 549. Beside the drift there is a class the checker cannot
see, because its baseline is the file's own last commit: a citation that was
wrong when it was written. S109's accepts and body cite `src/search.cpp` lines
for `is_check_move` and the reduction guard that hold other code, while the
same file's later sections name the right places; S055 cites taper divisions
at lines that moved before S055 was last touched; S024's Note cites the
countermove write and read at lines holding other statements, in a file whose
own accepts says every citation names a symbol; S119 cites a `rating.sh` line
that S177 rewrote.

The reviewer's observation is the fix: every drifted citation sits beside the
symbol it names, so an implementer who greps recovers. The symbol is the
durable reference and the line the decaying one. DEC-135 is the owner's
decision of 2026-09-04: in the pending directories, citations into code name
symbols and no lines.

## The form

`src/search.cpp` `is_check_move`; `tests/test_search.cpp` "pruning does not
hide a forced mate"; `src/evaluation.cpp` `evaluate_expensive()`. The path is
still repeated every time (DEC-120 stands). A citation into a document quotes
the phrase it points at. The checker resolves the symbol as
`--touches` already resolves one -- a word-bounded search in the named file --
and fails on absence; it fails a `file:line` in the pending directories
outright, so the old form cannot creep back.

## Whether the mode joins the fast suite

`tests/CMakeLists.txt` keeps `--citations` out because "any source commit
shifts lines under fifty step files at once, so gating on citation freshness
would make red the normal state". In the symbol form a source commit that
renames or deletes a symbol is the only thing that turns it red, which is the
event the plan wants to hear about, and rare. This step decides and records
either way, in the same place the current reason lives.

## Cost

A tooling change of modest size and a mechanical conversion over ~549
citations with the checker as the guide. A day of documents, no match.
