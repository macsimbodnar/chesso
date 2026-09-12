id:         S221
goal:       `tools/plan_prose_check.py` reads a test title that clang-format has split across adjacent C++ string literals as one title, so a phrase citation into a test no longer breaks when the formatter re-wraps it
accepts:    a red-first case in the checker's own tests (or a fixture under `tests/`, whichever the checker's tests use) where a `TEST_CASE`/`TEST_CASE_FIXTURE` title is written as two adjacent string literals and a step file cites a phrase spanning the join -- observed flagged `MISSING` on HEAD, resolved after; the `TITLE` regex (or the code that reads it) concatenates adjacent literals the way the C++ compiler does, for `TEST_CASE`, `TEST_CASE_FIXTURE`, `SUBCASE` and `TEST_SUITE`; the raw-text fallback `holds_phrase` is left as it is or made to match the same concatenation, stated either way; the two citations S042 and S024 reworded to dodge the gap (`adocs/plan_done/S042_en_passant_only_when_capturable.md` "Evidence taken" item 1, and the two S024 titles the subagent shortened) are left as they are -- history -- and the step file says so; `--citations` clean over `adocs/`; `DEV_MANUAL.md` where it describes the checker updated if its wording changes
touches:    tools/plan_prose_check.py, its tests, DEV_MANUAL.md
excludes:   any change to what counts as a citation (DEC-135's line-form rule and the bare-symbol rule stand); any `src/` change
decisions:  DEC-135, DEC-159
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found twice on 2026-09-12. S042's red-first case title wrapped over two string
literals at 80 columns and the checker flagged the step file's phrase citation
as `MISSING` -- the phrase was in the file, split by `" "` between "en" and
"passant" -- so the citation was shortened to the first literal. S024's
subagent then hit the same thing on two titles and shortened the titles
instead, and noted that `clang-format` re-splits any long title, so the gap
recurs at every long test name. The checker's `TITLE` regex reads only the
first quoted string after the macro's first argument, and the whitespace-
flattened fallback sees the adjacent quote characters as text.

## Cost

An hour of one agent, documents and one tool; no machine time. Filler
alongside S213 (test and tooling hygiene), which is where `plan.md` places it.
