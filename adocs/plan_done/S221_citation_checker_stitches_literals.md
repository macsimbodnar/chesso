id:         S221
goal:       `tools/plan_prose_check.py` reads a test title that clang-format has split across adjacent C++ string literals as one title, so a phrase citation into a test no longer breaks when the formatter re-wraps it
accepts:    a red-first case in the checker's own tests (or a fixture under `tests/`, whichever the checker's tests use) where a `TEST_CASE`/`TEST_CASE_FIXTURE` title is written as two adjacent string literals and a step file cites a phrase spanning the join -- observed flagged `MISSING` on HEAD, resolved after; the `TITLE` regex (or the code that reads it) concatenates adjacent literals the way the C++ compiler does, for `TEST_CASE`, `TEST_CASE_FIXTURE`, `SUBCASE` and `TEST_SUITE`; the raw-text fallback `holds_phrase` is left as it is or made to match the same concatenation, stated either way; the two citations S042 and S024 reworded to dodge the gap (`adocs/plan_done/S042_en_passant_only_when_capturable.md` "Evidence taken" item 1, and the two S024 titles the subagent shortened) are left as they are -- history -- and the step file says so; `--citations` clean over `adocs/`; `DEV_MANUAL.md` where it describes the checker updated if its wording changes
touches:    tools/plan_prose_check.py, its tests, DEV_MANUAL.md
excludes:   any change to what counts as a citation (DEC-135's line-form rule and the bare-symbol rule stand); any `src/` change
decisions:  DEC-135, DEC-159
closes:
blocks:
paused_by:
author:     a Sonnet 5 subagent briefed by the coordinator (DEC-185, DEC-188); started 2026-09-12 08:15 while S024's SPRT holds the machine
done:       2026-09-12 12:25. `tools/plan_prose_check.py`'s `TITLE` matches through the opening quote and gained `SUBCASE`; a new `_stitched_literal` joins adjacent string literals as the C++ compiler does before a title becomes a citable phrase, so clang-format's wrapping no longer hides a title from a phrase citation. Red first on `tests/S221_split_title_fixture.cpp` (a title over two literals cited by a planted step file): `MISSING` on HEAD, clean after; `tests/test_plan_citations.py` 19/19; `--citations` 0 flagged over 60 files, `--prose` 0. `holds_phrase`'s fallback left as is, the reason at its site. The citations S042 and S024 shortened to dodge the gap stay as history. Gate green in both builds (34/34) with `clang-format.sh --check` clean, run by the coordinator after S024's SPRT freed the machine; the fixture is formatted and never compiled. `DEV_MANUAL.md` and `MANUAL.md` unaffected; `README.md` human-owned, untouched. By a Sonnet 5 subagent (DEC-185, DEC-188), stamped by the coordinator

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

## Evidence, 2026-09-12

**Fixture.** `tests/S221_split_title_fixture.cpp`, a `TEST_CASE_FIXTURE` whose
title is written as two adjacent string literals (`"first half of a " "title
continued"`), never compiled -- `tests/CMakeLists.txt` does not list it, it
exists only for `tests/test_plan_citations.py` to cite. That file cites it
untracked (this suite does not `git add`), so `check_citations` is called
directly with a hand-built `tracked` set (`run_direct`, next to the new
`test_title_split_across_adjacent_literals_passes`) rather than through the
CLI subprocess the other cases in that file use -- the one part of the new
case that does not mirror them, and its docstring says why.

**Red, on HEAD, before the fix** (`python3 -m unittest
tests.test_plan_citations.CitationChecks.test_title_split_across_adjacent_literals_passes`):

```
AssertionError: 1 != 0 : .../S999_planted.md: 1 code citations, 1 flagged (0 line form, 0 bare, 0 document ungated)
  MISSING .../S999_planted.md:1  tests/S221_split_title_fixture.cpp "first half of a title continued"  -- tests/S221_split_title_fixture.cpp holds no such phrase
```

**Fix, three sentences.** `TITLE` now matches only up to the opening quote
(a lookahead, not a capture) and gained `SUBCASE` alongside the three
`TEST_`-prefixed macros the accepts names, guarded by a lookbehind so it
cannot match the tail of `DOCTEST_SUBCASE` in the vendored doctest header;
`titles_of()` then calls a new `_stitched_literal`, which walks forward from
that quote consuming one string literal at a time and concatenating their
contents whenever only whitespace (any amount, newlines included) separates
one literal's closing quote from the next one's opening quote, exactly the
rule a C++ compiler applies to adjacent literals. `holds_phrase`'s raw-text
fallback is left as it is, on purpose, stated in a comment at its call site:
the fallback exists for phrases quoted out of *comments*, which a C++
compiler never concatenates across a line break at all, so there is no
literal-adjacency gap there to stitch, and a title split the way S042's and
S024's were is now caught by `titles_of()` before the fallback is ever
reached.

**Clean, after the fix.** The same test:

```
test_title_split_across_adjacent_literals_passes ... ok
```

Full `tests/test_plan_citations.py`, all 19 cases: `Ran 19 tests in 1.224s /
OK`. `--citations` over `adocs/`: `citations flagged: 0 over 60 files`, exit
0. `--prose` over `adocs/plan.md`: `137 ids in prose, 66 of them completed`
/ `sentences flagged: 0`, exit 0. The two ctest script tests, run directly
since neither depends on a compiled target (`build/tests/CTestTestfile.cmake`
registers both as plain `python3` commands):

```
1/2 Test #33: test_plan_citations ..............   Passed    1.26 sec
2/2 Test #34: test_plan_citation_freshness .....   Passed    0.62 sec
100% tests passed, 0 tests failed out of 2
```

**S042 and S024 left as they are.** `adocs/plan_done/S042_en_passant_only_when_capturable.md`'s
"Evidence taken" item 1 still quotes the shortened phrase (`"a pre-root
repetition through a non-capturable"`), and `plan_done/` is history and is
never edited -- untouched. `adocs/plan_current/S024_continuation_history.md`
is also untouched. Checked directly rather than assumed:
`tests/test_search.cpp` (the S024 title that begins "the cutoff move is credited", still written over two literals) still carries a title split exactly this way
today -- `TEST_CASE_FIXTURE(search_fixture_t, "the cutoff move is credited
and the quiets before it are " "charged")` -- and S024's step file quotes
the joined phrase in prose at its own line 524. It is not a citation by
DEC-135's rule (no backticked path precedes the quote) and `--citations`
never touches it either way, so there was nothing there for this fix to
break or to leave broken. This step's own fixture is the regression case
that exists because of this.

**`DEV_MANUAL.md`.** `grep -n plan_prose_check DEV_MANUAL.md` -- the mode
table (lines 970-981) and the `--citations` prose (993-1016) describe
resolution order ("matched against the doctest titles first and then,
case-folded, as text anywhere in the file") and never claimed a title was
read from a single literal, so nothing there was false and nothing changed.
No change needed. `MANUAL.md`: no mention (`grep plan_prose_check MANUAL.md`
returns nothing) -- unaffected, it documents the UCI surface.

**Left for the coordinator.** The full gate
(`cmake --build build -j8 && ctest --test-dir build -L fast
--output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
build-tune -L fast --output-on-failure && ./clang-format.sh --check`),
barred here by S024's SPRT holding the machine. In particular
`clang-format.sh --check` has not run over the new fixture `.cpp` file or
the edited `.py` files (Python is untouched by clang-format, but the fixture
is C++ and its formatting is by hand, not verified). `git add` of the new
and modified files is also the coordinator's, per the hard limit this
subagent worked under.
