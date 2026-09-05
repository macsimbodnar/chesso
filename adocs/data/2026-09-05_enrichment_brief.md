# Enrichment brief, 2026-09-05 (DEC-145)

Repository: /Users/max/ws/chesso, branch achesso. Read `CLAUDE.md` and `AGENTS.md`
first (they are short and they are the law here), then this brief, then the step
file you were given.

## Your job

Enrich exactly ONE pending step file, the one named in your prompt. Append a
section `## Implementation guide (2026-09-05)` at the END of that file. Change
nothing else anywhere in the repository. Do not edit the header fields (`id:`,
`goal:`, `accepts:`, `touches:`, `excludes:`, `closes:`, `blocks:`, `paused_by:`,
`author:`, `done:`) and do not rewrite existing sections, with the single
exception stated under "Existing older section" below. Do not commit; the
coordinator commits.

## Who reads what you write

A weaker coding agent that will implement the step later on the owner's Linux
workstation under `AGENTS.md`. It has not read the literature, may not read other
engines' code, and will follow your text literally. It needs: the precise form of
the technique, where in this code it goes, the traps, exact commands, what to
measure and how to read the result, and what to write where on completion.
Write plain, precise English prose (normal register, not terse fragments).

## Rules that bind what you write

1. **COPYING (DEC-016, DEC-104).** Never read another engine's source code or
   tables, and never quote another engine's constant values (margins, table
   entries, reduction-formula numbers, weights). You MAY read and cite: the
   Chess Programming Wiki (chessprogramming.org), papers, articles, blog posts,
   and engine *records* -- commit messages, pull-request bodies, changelogs,
   release notes, OpenBench and fishtest result pages. Elo deltas and technique
   names from records are fine (DEC-019: they decide what to try, never what to
   conclude).
2. **SEEDS (DEC-105, DEC-134).** Every constant you propose is in one of three
   forms and you say which: (a) a value from a publication *about the technique*
   with its URL beside it -- a paper, an article, the wiki's own derivation or
   example formula, NOT a wiki page republishing an engine's tuned values;
   (b) a derivation procedure the implementing step runs at its start over
   chesso's own data or scale (for example "the 60th percentile of
   |static score - search score| over `adocs/data/S018_raw.tsv`"); (c) the
   midpoint of a stated range, said so. Units are chesso's own material scale
   (read `src/search_params.hpp` and `src/eval_tables.hpp` for it).
3. **CITATIONS (DEC-135, DEC-120).** A citation into code names the file and a
   symbol -- a function, a constant, a macro or a `TEST_CASE` title -- and never
   a line number. Repeat the path every time (`src/search.cpp` `negamax`, not
   "negamax at line 700"). A citation into a document quotes the phrase it
   points at. Verify every symbol exists at HEAD with grep before writing it.
4. **CHESS JUDGEMENT (DEC-023).** Do not assess any position, move or line
   yourself. Where a test needs a position, say how to obtain and verify it with
   a tool (`stockfish` via the safe invocation `TOOLCHAIN.md` describes,
   `build/tools/pgn_to_positions`, python-chess in `~/.venv/chess`).
5. **MEASUREMENT (INV-6, DEC-063, DEC-143).** Behaviour-neutral: identical node
   counts and best moves from `python3 tools/search_bench.py ./build/src/chesso 9`
   and `... 12` plus an interleaved `hyperfine` timing. Play-altering: SPRT via
   `./fastchess.sh` with a pre-registered bounds pair, the pair's worst-case
   expected games from the nElo run-length formula in
   `adocs/testing_strategy.md` section 1.1, converted to hours at 2337 games an
   hour, the abort rule, and the reading of each of the three outcomes. The
   machine's A/A (S198) must exist before the first verdict.
6. **TESTS (DEC-141, DEC-142).** A new pruning, reduction or extension rule
   ships with a direct guard test whose precondition is asserted (the test first
   shows the guard's condition holds at the node) and a mutant that test kills
   (`tools/mutation_check.py` after S196; `adocs/data/2026-09-04_test_review/mutants.py`
   before). A step touching `make_move`, `unmake_move`, the generator or the
   search self-plays the Debug binary, four rounds of fastchess at 4+0.04, and
   greps the log for `Assertion`. Every golden number is named as one at its
   site with the script that re-derives it.
7. **COMMITS (DEC-140).** A commit touching `src/` ends with `Bench: <nodes>`
   (from S189's completing commit on) or `No functional change`.
8. **DOCS.** Nobody writes `README.md`. `DEV_MANUAL.md`, `MANUAL.md` and
   `adocs/specs.md` are checked at completion; a UCI surface change refreshes
   `test_uci_surface` only after `MANUAL.md` and `specs.md` describe it.
9. **FIGURES.** Every number you quote from outside this repository carries its
   URL or the word `unverified`. If a fetch fails twice, mark it unverified and
   move on.
10. **ONE FILE.** `git status --short` must show only your step file changed.

## What to read before writing

- The step file, whole. Every `DEC-nnn` it names: `grep -n '^## DEC-nnn' adocs/decisions.md`
  then read that entry only -- never read `decisions.md` whole (7900+ lines).
- `adocs/specs.md`: the invariants table and the behaviour paragraphs about
  this step's area (grep for the technique's words).
- The code the `touches:` field names -- read the actual functions at HEAD.
- The closest precedents in `adocs/plan_done/` (ls it; read one or two: the
  last completed step of the same kind is the template for the stamp, the
  measurement and the pre-registration script under `adocs/data/S*_sprt.sh`).
- `DEV_MANUAL.md` sections "Test", "Measure", "Play games", "Which bounds",
  "Mate safety" for the exact commands; `adocs/testing_strategy.md` 1.1 for
  the run-length formula.
- `CLAUDE.md` "Known hazards and one-way doors".
- Literature: the wiki page(s) for the technique; the records the step file
  already names (fetch their URLs and confirm the figures);
  `adocs/data/2026-09-04_plan_review_literature_check.md` for figures already
  fetched on 2026-09-04. Load web tools with
  `ToolSearch` query `select:WebFetch,WebSearch` before using them.

## Section shape

Use exactly these H3 headings in this order under
`## Implementation guide (2026-09-05)`. Omit one only with a one-line reason.

### 1. What this step is, for someone new
### 2. The technique as published
Definition with URL (wiki or paper), what it buys and why, the published
form(s), which form chesso takes and why. For a document, tool or test step:
the standard or practice it follows and where it is described.
### 3. What chesso has today, and where the change plugs in
Functions, constants and data structures by symbol as they read at HEAD; the
data flow; the order of edits.
### 4. Constants and seeds
Each constant: its DEC-105 form, its range for `src/search_params.hpp`
(read how S073's `SEARCH_PARAM` entries declare a range), and why that range.
### 5. Interactions and traps
With existing pruning (mate hiding), the ordering bands, INV-4 accumulate not
recompute, table bound semantics, fail-soft, the improving flag, time
management; and this repository's own recorded traps from `plan_done/` and
`decisions.md`.
### 6. Tests
Guard test(s) with precondition as `TEST_CASE` pseudo-code; the mutant; goldens
touched and their scripts; the mate-safety instruments; the INV-6 command; the
Debug self-play command.
### 7. Measurement
Lane (timing or SPRT); the bounds pair with the reasoning; worst-case expected
games and hours; the three outcomes' readings; the exact commands; what to
record where (`adocs/data/S<id>_*.sh` header convention, the step file).
### 8. Completion checklist
The gate command; the Bench line; specs, MANUAL, DEV_MANUAL checks; stamp
content; `plan.md` and `status.md` go through the coordinator.
### 9. Sources read
Every URL or repository path with one line on what it gave; `unverified`
marks where a fetch failed.
### 10. Questions deferred to the owner
Anything that would change `accepts:` or needs a decision. Say "none" if none.

## Existing older section

If the file already carries `## Technical details (SOTA research ...)`, keep it
and do not rewrite it. Where it is wrong or stale (a symbol that no longer
exists, a claim S108 or later changed, a figure DEC-133 corrected, a citation
with a line number), say so by name in your section under a paragraph
"Corrections to the 2026-08-19 section". ONE exception: a seed value in that
section that originates in another engine -- quoted from a commit message or
pull-request body, or an engine's shipped value -- is replaced IN PLACE with a
DEC-105 form (a, b or c), and your section lists each replacement under
"Seeds replaced". That is S180's accepts done early; S180 will verify.

## Length

Technique steps: roughly 150 to 400 lines. Document, tool or test steps:
roughly 60 to 150 lines. Dense; no filler; do not repeat the header fields.

## Checks before you finish

- `git status --short` shows only the step file.
- `python3 tools/plan_prose_check.py --touches | tail -1` prints `touches flagged: 0`.
- `grep -nE '\.(cpp|hpp|h|py|sh):[0-9]+' <stepfile>` shows no new line-number
  citation into code inside your section.
- Every URL under "Sources read" was fetched, or is marked unverified.

## Report back, at most 25 lines

Step id; lines added; sources fetched and how many figures are unverified;
seeds replaced (list) or none; corrections to the older section (list);
questions deferred to the owner (list); anything that looks like a bug in the
tree or a stale `accepts:` (list).
