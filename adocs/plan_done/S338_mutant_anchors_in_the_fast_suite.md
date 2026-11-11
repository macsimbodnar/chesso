id:         S338
goal:       a mutant whose anchor no longer matches the tree turns the fast suite red in the commit that broke it, not at the next mutation run
accepts:    a fast-suite test validates every mutant under `tools/mutants/` against the tree it runs in, by the tool's own loader and validator and nothing parallel to them, building and running nothing; it is seen red first on a planted break of each kind the validator refuses (an anchor matching no line, an anchor matching twice, a mutant file that does not load), each naming the mutant and the file; it is green on the tree as it stands; its wall time is read and it sits under the fast label's clock with room; DEV_MANUAL.md's mutation section says what the test covers and what it does not (a mutant that validates and is no longer killed is still the mutation run's to find)
touches:    tools/mutation_check.py, tests/test_mutation_check.py, tests/CMakeLists.txt, DEV_MANUAL.md
excludes:   running any mutant; re-reading any kill; changing any mutant's meaning; the parked full mutation pass (`adocs/status.md`, Parked)
decisions:  DEC-171, DEC-141
closes:
blocks:
paused_by:
author:     an Opus 5.5 subagent, briefed by the coordinator, 2026-11-11
done:       2026-11-11 -- `accepts` by clause. `tests/test_mutation_check.py` `MutantAnchorsTest`, under the ctest entry `test_mutant_anchors` (labels `fast;mutant_list`, TIMEOUT 60), validates every mutant under `tools/mutants/` against the tree it sits in. It uses the tool's own `load_mutants` and `validate`, imported rather than reached by a flag, so `main`'s three guards are untouched, and builds and applies nothing. Its precondition is structural: the loader's files must equal the directory's `*.py` files. That check is red when a file is dropped or nothing is read, and the same plant is green with the check cut. Red first: 3 failures in the class and 1 in the driver's gate before the tool and CMake changed. Seen red on a scratch copy for every refusal the loader and validator produce, each naming the mutant and its file (a load failure through its traceback): an anchor matching no line, an anchor matching twice, a file that does not compile, raises a NameError or a TypeError, or defines no mutant, no mutant file, a duplicate id, a bad `expected`, a file outside src/, no such file, no pair, a pair that changes nothing. Red on the real one too: `42f2aa7`'s list against S102's staged src names PT08, pair 2, 0 matches, and the anchor. S102's staged re-anchor reads green. Green on the tree: 264 mutants in 34 files, 276 pairs. Wall 0.15 to 0.18 s in ctest with every core held. **A measuring run now excludes the label** (`LIST_LABEL`, `-LE mutant_list` on every ctest call), a deviation from the brief the coordinator ruled kept: with a mutant applied the test is red under 261 of 264, each a false kill, so the judging suite stays the 60 tests from before S338. `validate`'s lines name the mutant file and quote the anchor; only the refusal text changes (ruled accepted). `test_mutation_check` names `MutationCheckTest`, one class per entry (ruled accepted). The fast check fixed two findings, each red on a plant first. The registration guard now reads the live calls as cmake does, comments dropped; it passed four evasions in its first form and refuses all four now, with four more, in a fixture case (8 subtests red against the first form). `MutationCheckTest` checks that the anchors entry keeps `fast` and `mutant_list`, red when its labels are cut to `mutant_list` alone. Cases: `MutationCheckTest` 33, `MutantAnchorsTest` 4. `DEV_MANUAL.md` mutation section updated, including the 2026-09-04 review driver's missing exclusion; `MANUAL.md` checked, nothing to change; no `src/` change and no `adocs/data/` file. Gates under a node-budgeted run, before the fast check: both builds 59 of 61, the two failures being `test_tuner_gradient_plants_*` on their clocks alone (13166 and 14357 assertions pass without the clock, in each build). After it: the affected entries 7 of 7 in each build, format and the six `plan_prose_check.py` modes rc 0. Findings 1 to 3 go to S339; findings 4 and 5 are recorded only. **The coordinator's gate, 2026-11-11 22:42, the machine idle after P1:** 61 of 61 in `build/` and in `build-tune/`, format clean, bench 4707943. One fast check: a real finding (the registration guard read comments and missed entries of another form) and a low one (the `fast` label checked from inside the class it gates), both fixed, each red first; its third, the 2026-09-04 review driver, is a sentence in DEV_MANUAL.md. Rulings by the coordinator: the `mutant_list` exclusion, the refusal text and one class an entry are kept.

## Why

Found by S102 phase 1's follow-up, 2026-11-11. S102 gave `src/evaluation.cpp` `outside_pawn_terms` a sixth argument,
and the anchor of mutant PT08 in `tools/mutants/S118_pawn_cache.py` then matched no line. The tool's validator refuses
a whole mutant file on one such anchor, so no mutation run over `tools/mutants/` was possible on that tree, and both
gates were green: no fast test loads the real mutant list. The break was seen only because the follow-up had to
re-read another mutant's kill. S102's delivery re-anchors PT08; this step makes the next one visible.

It is filler under DEC-171: the defect is in a tool, reaches no play, no score and no UCI output, and sits behind the
strength step in progress. Agent-only, no `src/`, owns no run.

## Notes for the implementer

- `tools/mutation_check.py` `validate` takes a worktree and `main` refuses the main tree; the test needs the
  validation alone, on whatever checkout it runs in, without the clean-commit and green-baseline conditions a
  measuring run needs. Whether that is a flag or a function the test imports is the implementer's, stated in the stamp.
- The existing `tests/test_mutation_check.py` gates the driver on fixtures. The new check reads the real list, so it
  moves whenever `src/` moves under an anchor: that is its purpose, and its failure message must say which mutant, which
  anchor, how many matches, so the fix is a re-anchor and never a deletion.

## What was built (2026-11-11)

**An import, not a flag.** `tests/test_mutation_check.py` `MutantAnchorsTest` loads `tools/mutation_check.py` as a
module (`importlib`, with `sys.dont_write_bytecode` so a gate run leaves no `tools/__pycache__`) and calls
`load_mutants` and `validate`, the two calls `main` makes before it writes a byte. Why: `main` refuses the main tree
and demands a clean fixture and a green baseline, so a flag would need a second path through the function a measuring
run takes, skipping three guards, plus a command-line surface to document and gate. The import leaves `main` as it
was and calls nothing else of the tool's.

Four cases in the new class, the fourth from the fast check below:

| case | what it asserts |
|---|---|
| `test_every_mutant_validates_against_this_tree` | the real list against the tree the file sits in. The precondition comes first and pins no number: the directory holds at least one `*.py` file, and the loader read every one of them and nothing else. Then a refusal from `validate` is the failure, with the re-anchor instruction after it |
| `test_a_red_names_the_mutant_its_file_the_anchor_and_the_count` | a fixture (the sandbox's `SOURCE` and its `AMBIGUOUS` and `MISSING` mutants in a temp directory): the refusal carries `<id> in <mutant file>: pair 0 anchor occurs <n> times in src/x.cpp, needs exactly 1` and the `anchor '<text>'` line, for 2 and for 0 |
| `test_a_mutation_run_leaves_this_class_out` | the live calls in `tests/CMakeLists.txt`, as rewritten by the fast check below: every entry whose command mentions the file names exactly one of its classes and nothing else, every class has one, and every entry running this class carries the tool's `LIST_LABEL` |
| `test_the_registration_guard_reads_what_cmake_reads` | the fast check's fixture: a correct registration passes, and eight plants are refused, each by its own line |

**A measuring run changes by one exclusion: a deviation from the brief, ruled by the coordinator on 2026-11-11 (kept,
over an environment variable or a label of its own).** A
mutant applied removes its own anchor, so a fast test that validates the list goes red inside a mutation run. Counted
in memory over this tree's 264: the whole list stops validating under 261 of them, and stays valid only under LW02,
TC04 and K06, whose replacements keep their anchors. Plant p18 shows the same on disk, with M22 applied by the tool's
own `apply_mutant`. Each of the 261 would score `killed` by a test that never ran the engine. So
`tools/mutation_check.py` `LIST_LABEL` is `mutant_list`, every ctest call in `tools/mutation_check.py` `ctest` adds
`-LE mutant_list`, and the new entry carries `fast;mutant_list`. Real ctest on `build`: `-L fast -N` lists 61 tests,
`-L fast -LE mutant_list -N` lists 60, the set before S338. The suite that judges a mutant is the pre-S338 one. A
run's baseline count is now one less than `ctest -L fast`'s.

`tests/test_mutation_check.py` `MutationCheckTest` gains `test_the_suite_that_judges_a_mutant_leaves_out_the_list_readers`.
The ctest stub logs its arguments: one line in the stub, and `CTEST_ARGS_LOG` set by `Sandbox.run` and `Sandbox.spawn`.
Both ctest calls of a one-mutant run must carry `-L fast -LE mutant_list`. The fast check added a second case,
`test_the_anchors_entry_runs_under_fast_and_its_own_label`, so the class has 33.

**The validator's lines name the mutant file and quote the anchor.** Every line `validate` writes now starts
`<id> in <mutant file>`. Before, it named the id alone, and a duplicate named only the first file. A count line gains
an indented `anchor <repr>`. Only the refusal text changes: what is refused, and when, is the same. The six existing
refusal cases assert substrings that still hold, and pass. Ruled accepted by the coordinator, 2026-11-11.

**Registration.** In `tests/CMakeLists.txt`, `test_mutant_anchors` runs the file with `MutantAnchorsTest`, labels
`fast;mutant_list`, TIMEOUT 60. `test_mutation_check` now names `MutationCheckTest`, which is its 31 cases plus the two
new ones. An entry that names no class would run the anchors class under `fast` alone and put it back into a mutation
run's suite. The guard case reads that, and it also reads a class added later with no entry of its own (g2 and g3
below). One class per entry was ruled accepted by the coordinator, 2026-11-11.

`tools/mutation_check.py`'s module docstring says what the run excludes and why.

## Reds

**Red first, before the tool or CMake changed** (`.tuning/coord/S338/evidence/red_stage1_anchors.log`,
`red_stage1_sandbox.log`). The anchors class: 3 failures. The registration guard read `[''] != ['MutantAnchorsTest', 'MutationCheckTest']`, and
both subtests of the message case failed: the old line named neither the mutant file nor the anchor. The real list was
already green on this tree. The sandbox class: 1 failure of 32, `'-L fast -LE mutant_list' not found in '--test-dir
.../wt/build -L fast --output-on-failure'`. After the tool change: the guard alone, 1 failure. After the CMake change:
35 of 35 OK.

**Plants.** Each plant got its own scratch copy, made by `plants.py` and `make_tree.sh` (both now in
`.tuning/coord/S338/evidence/`), of the tool, `tools/mutants/`, `src/`, the test file and `tests/CMakeLists.txt`. The test file's real-list case ran in each copy;
the g plants ran the guard case. The unplanted copy exits 0. Paths below are shortened to `...`.

| plant | break | red line |
|---|---|---|
| p01 anchor matches no line | `src/bitboard.cpp`: M22's two operands swapped | `M22_castling_rights_on_capture in .../tools/mutants/board.py: pair 0 anchor occurs 0 times in src/bitboard.cpp, needs exactly 1`, then `anchor 'board->castling & castling_rights[move.from] & castling_rights[move.to]);'` |
| p02 anchor matches twice | M24's line copied into a comment above it | `M24_halfmove_not_reset_on_capture in .../board.py: pair 0 anchor occurs 2 times in src/bitboard.cpp, needs exactly 1`, then the anchor |
| p03 file does not compile | the closing paren of M22's pair deleted | ERROR, the traceback ends in `.../tools/mutants/board.py", line 22` at `m("M22_castling_rights_on_capture", ...`, `SyntaxError: '(' was never closed` |
| p04 file raises, a name | `B` renamed `BOARD` | ERROR, `.../board.py", line 22, in <module>`, `NameError: name 'B' is not defined` |
| p05 file raises, a call | `orgin=` for `origin=` | ERROR, `.../board.py", line 22`, `TypeError: load_mutants.<locals>.m() got an unexpected keyword argument 'orgin'` |
| p06 file defines no mutant | `S999_empty.py`, a docstring and a constant | ERROR, `Refused: no mutant defined in .../tools/mutants/S999_empty.py` |
| p07 no mutant file | the directory emptied | ERROR, `Refused: no mutant file found in .../tools/mutants` |
| p08 duplicate id | M22's id given again in `time.py` | `M22_castling_rights_on_capture in .../time.py: duplicate id, also in .../board.py` |
| p09 expected not a verdict | M26's `equivalent` misspelt | `M26_repetition_skips_first in .../board.py: expected='equivalnet', not 'killed' or 'equivalent'` |
| p10 file outside src/ | a mutant of `tests/test_search.cpp` | `M996_outside_src in .../time.py: file tests/test_search.cpp is outside src/` |
| p11 no such file | `src/bitboard.cpp` renamed `src/board.cpp` | 20 lines, the first `PK01_move_piece_dropped in .../S099_pawn_key.py: no such file src/bitboard.cpp in the worktree` |
| p12 no pair | an `m()` with no pair | `M995_no_pair in .../time.py: no (old, new) pair` |
| p13 pair changes nothing | anchor equal to replacement | `M994_self in .../time.py: pair 0 replaces its anchor with itself` |
| p14 pair not a pair | a bare string as the pair | ERROR, `ValueError: too many values to unpack (expected 2)` at `validate`'s unpacking. **Names neither mutant nor file**: finding 1 |
| p15 own `m`, entry without pairs | a file with its own `m` and `M` and no `pairs` key | ERROR, `KeyError: 'pairs'` in `normalise`. **Names no file**: finding 2 |
| p16 precondition, a file dropped | the copy's loader skips its first file | `the loader's files are not the directory's: none read from ['.../S082_leaf_walk.py'], read from outside it []` |
| p16b precondition, nothing read | the copy's loader returns `[]` | the same line, all 34 files listed. With the precondition cut from the copy, this plant is **green** (`plant_p16b_cut_precondition.log`): the precondition is what catches it |
| p17 the real one, PT08 | `42f2aa7`'s `tools/mutants/` against the staged `src/` of `/home/max/ws/chesso-s102p1`, exported read-only (`git ls-files -s`, `git cat-file`) | the only line: `PT08_outside_terms_stored in .../S118_pawn_cache.py: pair 2 anchor occurs 0 times in src/evaluation.cpp, needs exactly 1`, then `anchor '  outside_pawn_terms<false>(board, entry, &pawns, nullptr, nullptr);\n'` |
| p18 a mutant applied, why the exclusion | M22 applied by the copy's `apply_mutant` | `M22_castling_rights_on_capture in .../board.py: pair 0 anchor occurs 0 times in src/bitboard.cpp, needs exactly 1` |
| g1 label dropped | `LABELS "fast"` on the anchors entry | `AssertionError: 'mutant_list' not found in ['fast']` |
| g2 entry names no class | `test_mutation_check`'s command without its class | `Lists differ: ['', 'MutantAnchorsTest'] != ['MutantAnchorsTest', 'MutationCheckTest']` |
| g3 class without an entry | a third `TestCase` class in the file | `Lists differ: ['MutantAnchorsTest', 'MutationCheckTest'] != ['MutantAnchorsTest', 'MutationCheckTest', 'ThirdTest']` |

Every refusal `load_mutants` and `validate` can produce was planted: p06 and p07 are the loader's two `Refused`, p03
to p05 a file whose `exec` fails, and p01, p02 and p08 to p13 the validator's eight lines. p14 and p15 are not refusals.
They are crashes, which the test reads as red without naming the file. No refusal kind went unplanted.

**And green where it should be.** p19 is S102's staged `tools/mutants/`, with PT08 re-anchored, against its staged
`src/`: exit 0. S102's delivery lands green on top of this step.

## Readings

**The count read**, on this tree (`893419b` plus this diff, `tools/mutants/` untouched): 264 mutants in 34 files, 276
pairs, 259 `killed` and 5 `equivalent` expected. All validate. The 264 read earlier on another tree agrees.

**Wall time.** `MutantAnchorsTest` alone took 0.12 to 0.13 s wall over three runs (`/usr/bin/time`, 0.05 s inside
unittest), and `MutationCheckTest` took 8.65 s. Both were read with a node-budgeted run holding every core, load 13 on
12 threads, so these are loaded-machine figures. ctest read `test_mutant_anchors` at 0.18 s (Release) and 0.08 s
(tune) inside the full runs, and at 0.14 s and 0.13 s alone. TIMEOUT 60 leaves room of several hundred times.

## Gates

All of these ran with a node-budgeted run holding every core (load 13 to 17 on 12 threads), at `nice -n 15`, with
builds at `-j6` and `CLANG_FORMAT_MAJOR=22`.

- `cmake --build build -j6` and `cmake --build build-tune -j6`: rc 0. No C++ was touched.
- `ctest --test-dir build -L fast --output-on-failure`: **59 of 61**, 6 m 55 s. Two failed by their 60 s clocks
  alone: `test_tuner_gradient_plants_pawns` and `test_tuner_gradient_plants_threats`. Run alone without the clock,
  they passed 13166 of 13166 and 14357 of 14357 assertions, in 1 m 13 s and 1 m 49 s. `test_mutant_anchors` took
  0.18 s and `test_mutation_check` 10.18 s.
- `ctest --test-dir build-tune -L fast --output-on-failure`: **59 of 61**, 6 m 56 s, with the same two failing by
  their clocks alone. Without the clock: 13166 of 13166 and 14357 of 14357, in 59 s and 1 m 31 s.
  `test_mutant_anchors` took 0.08 s and `test_mutation_check` 6.49 s.
- After the last edit, which touched only a comment in `tests/CMakeLists.txt` and the documents, both builds were
  rebuilt. `test_mutation_check` and `test_mutant_anchors` passed 2 of 2 in each. In each build `-L fast -N` lists
  61 tests and `-L fast -LE mutant_list -N` lists 60.
- `./clang-format.sh --check`: rc 0.
- `tools/plan_prose_check.py`, the default and `--prose`, `--citations`, `--touches`, `--params`, `--gate`: rc 0
  each. Citations: 0 flagged over 18 files, this file 7 code citations and 0 flagged.
- DEC-141's second tier does not apply. Nothing in `make_move`, the generator or the search changed, no rule was
  added, and nothing in `src/` changed, so no Debug self-play was run.

**Evidence** is in `.tuning/coord/S338/evidence/`: the red-first logs, one log per plant, `plants_summary.txt`, both
gate logs, the no-clock logs, and the scripts `plants.py` and `make_tree.sh`. The scripts write under `.tuning/s338/`
when re-run. The scratch trees and the S102 export are removed.

## Docs checked

`DEV_MANUAL.md`'s mutation section:

- new paragraphs on `test_mutant_anchors`: what it covers, what it does not (a mutant that validates and is no longer
  killed, an anchor that still matches once at a site whose meaning moved, a mutant that no longer compiles, a pair
  made ambiguous by an earlier one), and why a mutation run leaves it out
- the cost paragraph: the fast label "less the `mutant_list` label"
- the driver's gate paragraph: 33 cases and S338's two
- after the fast check: what the registration guard reads, and the sentence on the 2026-09-04 review's driver

`MANUAL.md` names neither the tool nor the mutants, so nothing changes there. `adocs/specs.md` names the tool only
where search rules cite their kills. It says nothing about the fast label's members or the run's selection, so it
needs no change; that file is the coordinator's. No `adocs/data/` file.

## Found beyond the step (not fixed)

Findings 1 to 3 go to S339, a filler step the coordinator files (ruling, 2026-11-11); they are not fixed here.

1. **`validate` has no refusal for a pair that is not an (old, new) pair of strings** (p14). A bare string, or a tuple
   of three, raises `ValueError` at the unpacking, and a non-string side would raise `TypeError` at `text.count`.
   Neither names the mutant or its file. A measuring run ends in a traceback and `MUTATION-RUN-FAILED` before any
   write; the anchors test reads it as red. Tool-only, DEC-171 filler.
2. **`normalise` raises `KeyError: 'pairs'`** for an entry without pairs from a file that brings its own `m` (p15).
   The file is not named. Only the two evidence copies under `adocs/data/` use that form, and nothing reads them in a
   gate.
3. **A skipped test reads as a kill.** `CTEST_FAILED_RE` also matches the rows ctest prints under "The following tests
   did not run:", so `ctest` returns `asserted` true for a `(Skipped)` row. Observed on a two-test scratch project
   (its log is `.tuning/coord/S338/evidence/skipcheck_ctest.log`): an unmutated tree with one test exiting 77 under
   `SKIP_RETURN_CODE` scores `killed`.
   `test_bare_fen_relay` skips that way when no python imports python-chess, so on such a machine a mutation run
   would score every mutant killed by it. It is latent here, because the venv answers. Tool-only, DEC-171 filler,
   and it bears on any mutation reading taken on another machine.
4. **The loader reads `*.py` files only, and does not recurse.** A mutant file saved under another name is skipped
   silently. That is not a refusal and cannot be planted as a red, and the precondition uses the same filter.
5. **A consequence, by design:** the fast gate of a screen (the `adocs/data/S1nn_screen.sh` scripts) or of a candidate
   tree now runs this test, so a candidate that moves an anchor goes red there, and its re-anchor has to land with
   it. Worktrees already in flight sit at their own commits and are unaffected until rebased.

## The fast check (2026-11-11)

Two findings, both fixed, each seen red on a plant before green, and one sentence for the documents. Evidence is in
`.tuning/coord/S338/evidence/fast_check/`, with the plant script `guard_plants.py`.

**A. The registration guard was weaker than DEV_MANUAL.md and the CMake comment said.** Its first form read the raw
text of `tests/CMakeLists.txt` with two regexes. Reproduced first on scratch copies of `tools/` and `tests/`
(`guard_plants.py before`): each of four edits passed it (`Ran 1 test`, `OK`, exit 0) while putting the anchors class
back into a mutation run's suite under `fast`:

| plant | edit |
|---|---|
| a | the anchors entry's `set_tests_properties` commented out, a live `LABELS "fast"` line below; `re.search` matched the comment |
| b | an extra entry running the file with `-v`, labelled `fast`; the regex never saw it |
| b2 | an extra entry through `${Python3_EXECUTABLE}` running `MutantAnchorsTest`, labelled `fast` |
| b3 | an extra entry naming `MutantAnchorsTest.test_every_mutant_validates_against_this_tree`, labelled `fast` |

The fix is four module-level functions in the test file:

- `cmake_live` blanks line and bracket comments outside strings.
- `cmake_calls` reads every live `add_test`, `set_tests_properties` and `set_property`, with parentheses balanced and
  arguments unquoted.
- `file_entries` takes every `add_test` whose arguments mention `test_mutation_check`, whatever the interpreter. It
  records the arguments after the one ending `test_mutation_check.py`, and the labels the last live
  `set_tests_properties` leaves.
- `registration_problems` requires each entry to name exactly one class of the file and nothing else, every class to
  have an entry, and every entry running `MutantAnchorsTest` to carry `mutant_list`. It refuses labels set through
  `set_property`. That refusal goes beyond the brief: a later `set_property` was a fifth way past the label.

The guard case now prints its problems whole.

The new fixture case `test_the_registration_guard_reads_what_cmake_reads` starts from `REGISTRATION`: the two
entries, plus a commented-out `add_test` that must not count. That passes, and each of eight plants is refused by its
own line: the four above, a later live `set_tests_properties` overriding the labels, an entry naming no class, labels
from `set_property`, and a class with no entry.

**Red first.** The case ran against the first form, re-expressed as a function in a scratch copy:

- `red_fixture_old_guard_no_comment.log`: 8 failures. The first form returned no problem for six plants (a, the
  override, b, b2, b3, `set_property`) and caught the other two under another message (no class, a class with no
  entry).
- `red_fixture_old_guard_with_comment.log`, with the base's commented-out `add_test` left in: 9 failures, the base
  itself among them, because the first form read the comment as an entry.

With the new form the case is green. On the scratch copies (`guard_plants.py after`) the guard case is red on each
plant:

| plant | red line |
|---|---|
| a | `test_mutant_anchors runs MutantAnchorsTest with labels ['fast'], without 'mutant_list', so a mutation run would judge by it` |
| b | `x runs the file with arguments ['-v'], not one class of ['MutantAnchorsTest', 'MutationCheckTest']` |
| b2 | `x runs MutantAnchorsTest with labels ['fast'], without 'mutant_list', so a mutation run would judge by it` |
| b3 | `x runs the file with arguments ['MutantAnchorsTest.test_every_mutant_validates_against_this_tree'], not one class of ['MutantAnchorsTest', 'MutationCheckTest']` |

On the tree the parser reads exactly two live entries: `test_mutation_check` (`MutationCheckTest`, labels `fast`) and
`test_mutant_anchors` (`MutantAnchorsTest`, labels `fast` and `mutant_list`). It does not read the comments in
`tests/CMakeLists.txt` that mention the file.

**B. The check on the anchors entry's own `fast` label lived in the class that stops running when that label goes.**
Plant: the anchors entry's labels cut to `mutant_list` alone. Before the fix, `MutationCheckTest` passed 32 of 32
(`before_B_labels_mutant_list_alone.log`): nothing under `fast` read it. Fix: `MutationCheckTest`
`test_the_anchors_entry_runs_under_fast_and_its_own_label`, through the same `file_entries`. An entry running
`MutantAnchorsTest` must exist, and its live labels must be exactly `fast` and `mutant_list`. After the fix: red, 1
failure of 33, `Lists differ: ['mutant_list'] != ['fast', 'mutant_list']`.

**C.** `adocs/data/2026-09-04_test_review/run.py` is evidence and not edited. DEV_MANUAL.md now says that it runs
`ctest -L fast` with no exclusion, so if re-run on a tree after S338 it would credit `test_mutant_anchors` with nearly
every kill.

**Counts:** `MutationCheckTest` 33 cases, `MutantAnchorsTest` 4, 37 in the file.

**Gates after the fast check** (same load, `nice -n 15`, `-j6`, `CLANG_FORMAT_MAJOR=22`):

- `cmake --build` in `build` and in `build-tune`: rc 0.
- `ctest -R 'test_mutation_check|test_mutant_anchors|test_plan'` passed **7 of 7 in each build**:

  | entry | `build` | `build-tune` |
  |---|---|---|
  | `test_mutation_check` | 8.89 s | 5.81 s |
  | `test_mutant_anchors` | 0.16 s | 0.15 s |

  The other five are the `test_plan_*` entries. A timing figure in a `tests/CMakeLists.txt` comment changed after
  that run, so both builds were rebuilt and the run repeated on the final tree: 7 of 7 again in each, with
  `test_mutation_check` 9.21 s and 6.65 s and `test_mutant_anchors` 0.16 s and 0.08 s.
- `-L fast -N` lists 61 tests and `-L fast -LE mutant_list -N` lists 60, in each build.
- `./clang-format.sh --check`: rc 0. `tools/plan_prose_check.py`, the default and its five modes: rc 0 each, 0 citations flagged
  over 19 files.

The full fast gate after these edits is the coordinator's, on an idle machine.
