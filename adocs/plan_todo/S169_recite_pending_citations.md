id:         S169
goal:       the 97 stale citations in the pending step files are re-anchored at HEAD, before S144's conversion moves their baselines and erases the report
accepts:    `tools/plan_prose_check.py --citations` reports **0 flagged**, down from **97** (25 ANCHOR, 72 DRIFT, 0 BOUNDS) over 20 of the 53 pending step files; **a green run is not the proof on its own** -- rewriting a step file moves that file's own DRIFT baseline, so the run goes green whether or not anything was repaired, and the evidence is a tracked mapping under `adocs/data/` giving, per repaired citation, the old range, the text it held at its baseline commit, the new range, and the text the new range holds at HEAD, with the two texts asserted equal; every repair locates the cited text, and no citation is repaired by deleting it, by dropping the line number, or by widening a range until it stops failing; an ANCHOR repair is proved by the checker itself, whose title test does not consult the baseline; a citation whose subject no longer exists in the tree is reported as such with its sentence quoted, not silently re-pointed
touches:    adocs/plan_todo/, adocs/data/, adocs/plan.md
excludes:   the bare `:line` continuations and the citation style rule, which are S144's and are why this step exists; `adocs/plan_done/`, which is history and is never rewritten; the numeric and technical claims the cited sentences make, which the 2026-08-20 plan_review listed as deferred; any change to `tools/plan_prose_check.py`, whose behaviour this step relies on being the one already documented
decisions:  DEC-119
closes:
blocks:     S144
paused_by:
done:

## Why this goes in front of S144

S144 converts 383 bare `:line` continuations in 19 pending step files into full
`path:line` citations. Doing that rewrites those files, and `baseline()` at
`tools/plan_prose_check.py:328` reads a step file's DRIFT baseline as **the
commit that last wrote it**. So the conversion would move 18 of those files'
baselines to S144's own commit, and DRIFT would compare each surviving citation
against a snapshot taken after the drift rather than before it.

Measured at HEAD, 2026-09-01: `--citations` flags **97** over 20 files, and
**92 of the 97 sit in the files S144 has to edit**. Converting first would
therefore clear 92 live staleness reports without repairing one of them, and
the checker would print a green run over a tree that had got no better. The
tool's own docstring says the same thing from the other side -- "correcting a
citation and moving the baseline are the same act" -- which is true when the
edit *is* the correction and is exactly what it stops being here.

The owner's decision of 2026-09-01, DEC-119: repair first, as its own step, so
the two effects carry two numbers -- 97 flags to 0 here, 383 loose references to
a stated remainder at S144 -- instead of one diff that does both and can be read
as either.

## What is stale, measured at HEAD

97 flags, 0 of them BOUNDS, over 20 of the 53 pending step files.

| class | count | what it means |
|---|---|---|
| DRIFT | 72 | the cited lines hold different text than at the step file's baseline commit |
| ANCHOR | 25 | a doctest title quoted beside the citation is not the test the cited lines open |

Where they point, by cited file: `tests/test_search.cpp` 31,
`src/search.cpp` 30, `src/search_params.hpp` 8, `src/chesso.cpp` 12 across its
rooted and bare spellings, and eleven others once or twice each.

The ANCHOR block is one repair repeated: **"pruning does not hide a forced
mate"** is cited by S091, S095, S097, S098, S113, S114 and S116 at
`tests/test_search.cpp:1887` or `:1923`, and it opens at **2808**. That is the
class `2026-08-20_plan_review-F01` found and S138 repaired once already: seven
pending steps, every one of them a pruning or a reduction step, each telling its
implementer to extend the mate-safety gate at a line that is now inside another
test. `CLAUDE.md` names hiding a mate as the recurring bug in this engine, so
this is the flag class that costs something when it is wrong.

## How a repair is proved when the checker cannot prove it

DRIFT's baseline moves with the file, so after this step's commit every
repaired citation is trivially DRIFT-green and the checker's verdict says
nothing about whether the repair was right. Two things carry the proof instead:

1. **ANCHOR is baseline-free.** It compares a quoted title to the current file,
   so its 25 repairs are gated by the tool after the commit exactly as they were
   before it.
2. **The mapping file is the evidence for DRIFT.** For each repaired citation it
   records the baseline text and the text at the new range, and the two are
   asserted equal by the script that writes it. A repair that moved a citation
   somewhere the cited text does not appear cannot be written to that file.

The script is a generator, not a gate: it is run once, its output is tracked,
and re-running it after the commit is meaningless because the baselines have
moved. That is stated here so nobody later reads the file as a check.

## Cost

No match, no verdict, no engine source touched. Document work plus one
generator script under `adocs/data/`, and the completion gate green.
