id:         S187
goal:       citations from pending step files into code name a file and a symbol -- a function, a constant, a macro or a test title -- and carry no line number, and the citation checker verifies the symbol exists in the named file, so a source commit can no longer stale the plan
accepts:    every `file:line` citation in `adocs/plan_todo/` is rewritten as the file and the symbol or `TEST_CASE` title it points at (the review counted 549, 58 of them drifted), keeping the symbol S144's rule already puts beside the citation; `tools/plan_prose_check.py --citations` fails a `file:line` form found in `adocs/plan_todo/` or `adocs/plan_current/` as `LINE` and a `file` plus symbol whose symbol is absent from that file at HEAD as `MISSING`, each observed red on a planted case before the conversion and green after; the four citations the review found wrong when written -- S109's accepts, S055's taper divisions, S024's Note, S119's `rating.sh` line -- come out right in the symbol form and S024's Note obeys S024's own accepts; the step states whether the mode joins the fast suite now that it holds no line numbers, and if so `tests/CMakeLists.txt` says why the old reason no longer applies; the "A citation repeats its path" paragraph of `adocs/plan.md` is rewritten for the symbol form and cites DEC-135; `DEV_MANUAL.md`'s citation section says the same; fast suite green in both builds
touches:    adocs/plan_todo/, adocs/plan_current/, tools/plan_prose_check.py, tests/test_plan_citations.py, tests/CMakeLists.txt, adocs/data/, adocs/plan.md, DEV_MANUAL.md, adocs/decisions.md
excludes:   `adocs/plan_done/`, which is history and keeps its line citations as written; `adocs/specs.md`'s invariant table, which cites lines into `src/bitboard.cpp` and is not a pending step file -- its own rule is a later decision; the engine
decisions:  DEC-120, DEC-135
closes:     2026-09-04_plan_review-F07
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-09
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

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

The pending step files point their implementer at code with citations of the
form `path:line` or `path:line-line`. Every commit that touches `src/` or
`tests/` moves lines, so those citations rot, and they rot fastest exactly
where the plan is densest. This step replaces every such citation in
`adocs/plan_todo/` and `adocs/plan_current/` with the durable form DEC-135
prescribes -- the path and the symbol the line sits in, such as
`src/search.cpp` `negamax` or `tests/test_search.cpp` "pruning does not hide
a forced mate" -- and teaches `tools/plan_prose_check.py --citations` to
refuse the old form and to verify the new one against the tree. No engine
code changes, no match is played, and no commit of this step carries a
`Bench:` line because none touches `src/` (DEC-140).

### 2. The technique as published

There is no external literature for this; the practice and its reasons are
this repository's own record. `tools/plan_prose_check.py`'s docstring, under
"--citations", states the principle: a line number is a moving reference and
a `TEST_CASE` title is a stable one. DEC-135 is the decision that makes the
stable reference the only one allowed in pending files; DEC-120 keeps the rule
that every citation repeats its path; `2026-09-04_plan_review-F07` in
`adocs/audit/2026-09-04_plan_review.md` is the evidence -- 97 citations
re-anchored by S169 on 2026-09-01, 59 stale three days later, four wrong at
the moment they were written. The two earlier passes are the method
precedents: S144 (`adocs/data/S144_paths.py`, `adocs/data/S144_pathings.tsv`)
converted bare `:line` continuations, and S169 (`adocs/data/S169_recite.py`,
`adocs/data/S169_recitations.tsv`) re-anchored drifted lines. Both proved
their conversion with a tracked mapping rather than with the checker's green
run, because rewriting a step file moves its own DRIFT baseline (DEC-119).
This step inherits that discipline. No URL was fetched; nothing here is a
figure from outside the repository.

### 3. What chesso has today, and where the change plugs in

**The census, taken 2026-09-05 at `d14fcd6`.** The review's 549 is a
2026-09-04 number; both ends have moved since (S180 to S199 were created,
nine files gained an `## Implementation guide (2026-09-05)` section). Re-take
it at the step's start and put the fresh numbers in the stamp -- S144's stamp
does exactly that for a goal line that said 467.

`python3 tools/plan_prose_check.py --citations` parses **564 code citations**
(513 into `.cpp`, 49 into `.hpp`, 2 into `.sh`), 14 document citations, 0
bare, and flags **59 DRIFT, 0 ANCHOR, 0 BOUNDS** over 74 pending files; 28 of
the 74 carry any code citation. A direct grep,
`grep -oE '(src|tests|tools)/[A-Za-z_./]+\.(cpp|hpp|h|py|sh):[0-9]+' adocs/plan_todo/*.md | wc -l`,
prints 535; widened to bare basenames and root scripts (`rating.sh`,
`fastchess.sh`, `chesso.cpp`) it prints 571. The checker's parser is the
authoritative count -- it resolves a bare basename only when it is unique in
the repository, as `PATH` and `_tracked` in `tools/plan_prose_check.py` do.
Of the 564, 322 cite a single line and 242 a range; 11 sit inside header
fields (`accepts:` of S109 and S159, `excludes:` of S091, among others); 519
sit in a paragraph that already names a backticked identifier or a quoted
title, which is what makes the conversion mostly mechanical.

Per step file: S117 45, S132 43, S131 42, S020 41, S091 36, S112 36, S055 34,
S097 34, S115 32, S109 30, S095 25, S113 25, S098 24, S099 23, S114 21, S022
19, S120 19, S116 18, then S024 3, S039 2, S082 2, S119 2, S134 2, S159 2, and
one each in S042, S136, S173, S178. Per cited file: `src/search.cpp` 279,
`src/evaluation.cpp` 66, `src/bitboard.cpp` 57, `src/chesso.cpp` 50,
`tests/test_search.cpp` 35, `src/search_params.hpp` 14,
`src/data_structures.hpp` 12, and seventeen files with eight or fewer. The
nine sections written on 2026-09-05 use the symbol form already and hold no
line citation (checked by grepping each section); they are checked, not
converted. The 59 flagged sit in nine files: S132 30, S115 18, S097 3, S099
3, and one each in S024, S117, S119, S120, S159.

**How the checker reads a citation today.** `paragraphs` splits a step file
at blank lines; `cites_in` finds a `PATH` token followed by `DIRECT`
(`:a` or `:a-b`) or a `CONT` continuation with no path, which `check_citations`
fails as `BARE`; `baseline` is the last commit that wrote the step file, and
`_at` fetches a file at that commit or in the working tree; `titles_of` maps
doctest titles to opening lines through `TITLE`, which already accepts
`TEST_CASE`, `TEST_CASE_FIXTURE` and `TEST_SUITE`; `gated` sends citations
into `adocs/` or any `.md` to the ungated "document" bucket. `--touches` has
the pieces a symbol check needs: `code_of` returns a file with comments
blanked, and `carries` does a word-bounded search for a symbol in it.
`citations` accepts an explicit file list, which a planted-case test relies
on. `tests/CMakeLists.txt` registers `test_plan_touches` and
`test_plan_params` in the fast label and says in its comment why
`--citations` is not there.

**What each citation resolves to, measured.** A prototype that walks upward
from the cited line at the step file's baseline commit to the nearest
enclosing definition gives, over the 564: **493** sit inside exactly one
function, constant, macro or X-macro row; **38** inside a `TEST_CASE` or
`TEST_CASE_FIXTURE`, so the title is the symbol; **25** ranges span two
definitions -- pairs like `LMR_BASE` and `LMR_DIVISOR`, `passed_pawn_mg` and
`passed_pawn_eg`, table stretches such as `TM_SOFT_PERCENT` to
`TM_SCALE_MIN_PERCENT`, a run of test cases in `tests/test_engine.cpp`, and
`build_lmr_table` running into `lmr_reduction`; **8** sit at file scope in a
comment or a `static` variable (the "Not in the set, on purpose" comment
above `CHESSO_SEARCH_PARAMS` in `src/search_params.hpp`, the
`LAZY_EVAL_MARGIN` comment in `src/evaluation.hpp`, `output_mutex`, `gen` and
`last_aspiration_failures` in `src/chesso.cpp`). Those 33 are the hand cases;
everything else the helper decides.

**Order of edits.** (1) Bank the census before touching a pending file:
`python3 tools/plan_prose_check.py --citations > adocs/data/S187_citations_before.txt`,
as S169 did, since the conversion moves every converted file's baseline.
(2) Extend the checker and add its planted-case test; commit. (3) Run the
helper, review its mapping, convert a few files per commit with the gate green
after each. (4) Register the mode in the fast label, rewrite `adocs/plan.md`
"How this file works", `DEV_MANUAL.md` and the checker's docstring; stamp.
S180 (Open entry 7) and S184 (entry 8) both rewrite pending files before this
step starts, so the census is re-taken after they land, never copied from
here.

### 4. Constants and seeds

None. No `src/search_params.hpp` entry changes; the only numbers this step
produces are its own counts and the checker's wall time, both measured in
place.

### 5. Interactions and traps

- **The struct member.** S024's `counter_moves` lives in `struct
  search_state_t` in `src/data_structures.hpp`. A walk-up that recognises
  functions, `constexpr` and `#define` but not `struct` returns the nearest
  `#define` above the struct (`TT_EVAL_NONE`) -- observed with the prototype.
  The helper must treat `struct <name>` and `typedef struct` as enclosing
  definitions and then prefer the member name on the cited line, so the
  citation becomes `src/data_structures.hpp` `counter_moves`.
- **The blind class is caught by disagreement.** For S109, S055, S024 and
  S119 the baseline text is not the intended text either. The helper compares
  the enclosing symbol it finds with the backticked identifiers in the citing
  paragraph; when they disagree, the citation is a hand case and the sentence
  is read. At HEAD the right targets are: S109's `is_check_move` and the
  reduction guard `!is_check_move` are both in `src/search.cpp` `negamax`;
  S055's positional and tempo taper divisions are in `src/evaluation.cpp`
  `evaluate_cheap` and the mobility and king-safety ones in
  `src/evaluation.cpp` `evaluate_mobility_and_king_safety`; S024's countermove
  write is in `src/search.cpp` `negamax` and its read in `src/evaluation.cpp`
  `score_move`; S119's `hash_mb=128` is the `hash_mb` assignment in
  `rating.sh`; S159's `search_state_t state = {}` is in `src/chesso.cpp`
  `iterative_deepening_search`. Re-derive each with the helper before writing
  it; these were read at `d14fcd6` and are here so the accepts has something
  to check against.
- **`rstrip("()")` is not an argument stripper.** `carries` strips a trailing
  `()` from `evaluate_expensive()`; on `CHESSO_SEARCH_PARAMS(X)` -- a form the
  2026-09-05 sections use -- it leaves `CHESSO_SEARCH_PARAMS(X`. The new
  token grammar must drop a whole parenthesised tail before the word-bounded
  search.
- **Wraps.** The prose is hard-wrapped, so `tests/test_search_params.cpp` ends
  one line and `golden_defaults` opens the next (S148's guide does this).
  Flatten the paragraph's whitespace before matching, as `anchor_titles`
  already does for titles.
- **A comment is not a landing site.** `code_of` blanks comments, so a symbol
  that survives only in a comment is `MISSING` -- correct, and the reason a
  quoted phrase into a comment (the eight file-scope cases) is checked as a
  raw substring of the file text instead.
- **A path alone is prose.** 88 places today write a backticked path
  followed by a plain word (`and`, `is`, `runs`, `compiles`). The recogniser
  fires only on a backticked token or a double-quoted phrase directly after
  the path; a path with neither is legal and unchecked. The cost is that an
  unbackticked symbol escapes the check, so the writing rule in `plan.md`
  says: backtick the symbol.
- **Weak by design.** `src/search.cpp` `state` passes because `state` occurs
  in the file. The check is existence, not relevance -- DEC-135's mandate --
  and the mapping is where relevance is proved.
- **The step's own file, and this section.** Until S187 moves to `plan_done/`
  its text is in the checked set. Write illustrations as `path:NNN`, never
  with digits, or they are `LINE` flags against the step that removes them;
  S144's file lived with three such flags for the same reason.
- **Header fields are in scope here and nowhere else.** DEC-145's pass edits
  no header field; S187's `accepts:` orders every `file:line` in
  `adocs/plan_todo/` converted, and eleven sit in headers. The edit is the
  citation and nothing else on the line.
- **A bare basename must be unique.** `PATH` resolves `chesso.cpp` and
  `rating.sh` through `_tracked`'s unique-basename map; `CMakeLists.txt` is
  not unique and must be rooted. Keep that behaviour.
- **Two symbols under one path.** Repeat the path per symbol, which is
  DEC-120 read literally -- `src/search_params.hpp` `LMR_BASE` and
  `src/search_params.hpp` `LMR_DIVISOR` -- and let the checker examine only
  the first token after each path. Cheaper than teaching it lists, and no
  ambiguity about which path a second token belongs to.
- **The two fast-label concerns.** A commit that renames a symbol must fix
  every pending file citing it in the same commit, or the gate is red; that
  coupling is the point. Outside a git checkout the mode must skip with exit
  0 the way `touches` does when `_tracked` returns nothing.

### 6. Tests

No engine test changes. The checker gets `tests/test_plan_citations.py`,
modelled on `tests/test_spsa_driver.py`: `unittest` cases that write a step
file into a `tempfile.TemporaryDirectory`, run
`python3 tools/plan_prose_check.py --citations <that file>` through
`subprocess`, and assert the exit code and the flag word in stdout. A path
outside the repository has no baseline (`baseline` returns `None` when
`git log` finds nothing), which is fine because neither new class consults
one. Cases: a `src/chesso.cpp` line citation fails as `LINE`; a
`src/search.cpp` `no_such_symbol` pair fails as `MISSING`; a
`tests/test_search.cpp` "no such title" pair fails as `MISSING`;
`src/search.cpp` `negamax` and `tests/test_search.cpp` "pruning does not hide
a forced mate" pass; a `:NNN` continuation still fails as `BARE`; a path
followed by a prose word passes. Register it in `tests/CMakeLists.txt` beside
`test_spsa_driver` with the `fast` label. The accepts' "observed red on a
planted case before the conversion" is also satisfied on the real set: after
commit (2) of section 3, `python3 tools/plan_prose_check.py --citations`
prints `LINE` for all 564 and exits 1; paste the summary line into the stamp.

Symbol recognition to implement: after a `PATH` match (allow surrounding
backticks), optional whitespace including one newline, then either
`` `token` `` where token is `[A-Za-z_][\w:]*` with an optional `(...)` tail,
or `"phrase"` of 4 to 120 characters as `QUOTED` already defines. A `DIRECT`
match after a code path in a pending file is `LINE`. A token is verified with
`carries`; a phrase first against `titles_of` and then as a whitespace-
flattened substring of the raw file. A path into `adocs/` or a `.md` keeps
the document bucket: `LINE` applies to it too (the accepts says every
`file:line`), and the phrase check is reported as a note, ungated, unless the
owner decides otherwise (section 10). The old `BOUNDS`, `ANCHOR` and `DRIFT`
branches stop firing for code by construction once no line form is legal;
rewrite the docstring's failure-class list to `LINE`, `MISSING`, `BARE`, and
keep one paragraph saying what the three retired classes were and where
their evidence lives.

### 7. Measurement

No lane: nothing alters play, and INV-6 does not apply. Two numbers are
measured and recorded. First the checker's cost for the CMake comment:
today `--citations` takes 6.3 s wall on this machine because DRIFT runs
`git show` per baseline-and-path pair, while `--touches` takes 0.11 s; the
symbol form consults git only for `ls-files`, so expect a few tenths of a
second -- measure it with five runs of `time python3
tools/plan_prose_check.py --citations` after the conversion and put the
median in the comment. Second the census before and after, in the stamp: the
`--citations` summary line before the first conversion, the per-method counts
of the mapping, and `0 flagged` after.

The mapping is `adocs/data/S187_symbols.tsv`, one row per citation: step
file, line, the citation as written, the baseline commit, the text the cited
range held there, the symbol chosen, the method (`AUTO` for one enclosing
definition agreeing with the paragraph, `TITLE`, `SPAN` for a two-symbol
range, `HAND` with the reason), and the text at HEAD that the symbol resolves
to. The generator that writes it, `adocs/data/S187_symbolise.py`, follows
`adocs/data/S169_recite.py`'s `propose`, `detail`, `apply` shape and imports
`plan_prose_check` for `paragraphs`, `cites_in`, `baseline` and `_at`. It is
a one-shot generator under `adocs/data/`, like its two predecessors, not a
tool under `tools/`; a mode inside `plan_prose_check.py` would put a rewriter
inside a gate.

### 8. Completion checklist

- The TESTS command of `AGENTS.md`, both builds, plus
  `python3 tools/plan_prose_check.py --citations` and `--touches` at 0
  flagged, and `--prose` green after the `plan.md` edit.
- `tests/CMakeLists.txt`: `test_plan_citations` registered, and its comment
  says why the reason quoted at `test_plan_touches` no longer applies --
  a source commit no longer moves anything the check reads; only a renamed
  or deleted symbol does, which is the event the plan wants to hear about.
- `adocs/plan.md` "How this file works": the "A citation repeats its path"
  paragraph rewritten for the symbol form citing DEC-135, absorbing the
  "Since 2026-09-04" paragraph beneath it, with no `path:digits` example
  left in it. `DEV_MANUAL.md`: the `--citations` paragraph and the "not
  registered with ctest" paragraph under "Test" both rewritten; `MANUAL.md`
  unchanged (no UCI surface); `adocs/specs.md` unchanged (its invariant table
  is excluded by DEC-135). `README.md` untouched.
- Stamp: the two census lines, per-method counts, the four F07 cases with
  their symbols, the planted-case output, the timing, the fast-label
  decision, and the paths of the mapping and the banked pre-run. Commits:
  checker plus test first, then the conversions a few files each, then the
  registration and documents; every commit green; no `Bench:` line.
- `plan.md`'s Open and Done lists and `status.md` through the coordinator.

### 9. Sources read

All in this repository, none fetched: `adocs/decisions.md` DEC-119, DEC-120,
DEC-135, DEC-145; `adocs/audit/2026-09-04_plan_review.md` F07;
`adocs/plan_done/S144_citation_paths_are_explicit.md` and
`adocs/plan_done/S169_recite_pending_citations.md` with their generators
under `adocs/data/`; `tools/plan_prose_check.py` whole;
`tests/CMakeLists.txt` and `tests/test_spsa_driver.py`; `adocs/plan.md` "How
this file works" and the Open list; `DEV_MANUAL.md` "Test"; the census and
the enclosing-symbol prototype run at `d14fcd6`. Zero external figures, zero
unverified.

### 10. Questions deferred to the owner

1. `touches:` lists neither `adocs/data/` (generator and mapping, as S144 and
   S169 both needed) nor `tests/` (the planted-case test file). S144 amended
   its `touches:` at completion; the same amendment, or a decision to fold
   the test elsewhere, is the owner's.
2. The 14 document citations (`adocs/plan.md`, `adocs/specs.md`, two pending
   step files) become quoted phrases under DEC-135. Should a phrase absent
   from the document gate the run like `MISSING`, or stay a note as today?
3. The goal, `plan.md`'s Open entry and F07 say 549; the count at `d14fcd6`
   is 564 and moves again before the step starts. S144's precedent leaves
   the header and states the number in the stamp -- confirm.
4. Whether the fast-label decision gets an Amended line on DEC-135, or the
   CMake comment and `DEV_MANUAL.md` are the record, as the accepts reads.

## 11. The owner's answers, 2026-09-09

All four questions of section 10 were answered on the day the step started,
every one as the section recommended. They are recorded here because the
transcript is not the memory.

1. **`touches:` is amended now, not at completion**, so the field is true
   while the step is current: it gains `adocs/plan_current/`,
   `tests/test_plan_citations.py`, `adocs/data/`, `adocs/decisions.md` and
   loses nothing. `adocs/decisions.md` therefore leaves `excludes:` -- it is
   excluded no longer, because answer 3 puts a decision in it.
2. **The goal's 549 stays.** The census at `2b198f6`, the commit this step
   starts from, is **558 code citations and 14 document ones, 0 bare, 52
   DRIFT over 66 pending files**; the guide's 564 / 59 / 74 was taken at
   `d14fcd6`, before S180, S184, S203 and S148 landed. S144's precedent:
   the header keeps the number it was written with and the stamp states the
   measured one.
3. **A quoted phrase absent from a cited document stays a note, ungated.**
   The document bucket's existing reason holds -- `adocs/plan.md` and
   `adocs/specs.md` are rewritten at every completion, so gating a phrase in
   them makes red the normal state, which is the failure mode DEC-119's
   paragraph in the checker docstring describes. The `LINE` class still
   reaches them, so the six `adocs/plan.md:NNN` citations in S136 and the
   other eight document line citations are converted like every other.
4. **`--citations` joins the fast label, and the choice is DEC-159.** Not an
   Amended line on DEC-135 and not the CMake comment alone: what a future
   reader re-derives is why the 2026-08 reason for keeping the mode out
   stopped applying, and that is a decision rather than a comment.
