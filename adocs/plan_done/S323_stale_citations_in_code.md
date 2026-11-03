id:         S323
goal:       the line-number citations in code comments point at what they name or cite it by a phrase, and the test list's clamp count says what the case reads
accepts:    each site below re-read against its target at landing, with what the check read, then true, cited by a phrase or symbol, or gone; any further line-number citation in `src/`, `tools/` and `tests/` comments found by a stated search and treated the same way, the search and its count in the step; comments only, code identical by S306's form (objects byte-equal or each difference explained, as S321 did), `No functional change`; a clean reader old against new
touches:    tests/CMakeLists.txt, tools/corpus_dedupe.hpp, tools/tuner_split.hpp, tools/mutants/time.py, tests/test_audit_fen_semantics.cpp, tests/test_helpers.hpp, tests/test_search.cpp, tests/test_evaluation.cpp, adocs/data/S323_*, adocs/data/README.md
excludes:   shortening comments or removing history (DEC-342); any change to what a test asserts; S322's changes to `tests/CMakeLists.txt` beyond the one comment
decisions:  DEC-171, DEC-342
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- every listed site read against its target on `b3739e9`, old and new side by side in this file. Of the ten code pointers nine pointed elsewhere and one, the castling block's, a line off; each now cites a symbol or a phrase, `compute_full_hash()`, `CHESSO_PROJECT_SEED`, `make_move_impl()` twice, `generate_moves_body()`'s CASTLING section, `castling_rook()`, `load_FEN()`'s "3. En passant target square", INV-1, `negamax_at()` and "degenerate on purpose". The S262 comment says what `DEV_MANUAL.md`'s sentence says: 492 of the 2696 at S321 on `d0220d1`, 246 each way, no curated row, the 478 kept with its 2764 and `f57b92f`; re-read on `b3739e9` by S321's method, the same, `adocs/data/S323_clamp_split.txt`. `tools/mutants/time.py`'s line number sits in a quoted compiler message, which is evidence of what the compiler said on `0abe648`: the quote stays word for word and a sentence dates it, since a new number would misquote and removing it would remove history. The search, three patterns over 179 files under `src/`, `tools/` and `tests/`, 30, 18 and 6 hits, every one placed: it found one more pointer, `tests/test_search.cpp`'s insufficient-material case, fixed, and one more quoted observation, the F10 sanitizer message, dated already and kept. Nothing shortened, no history removed (DEC-342). Comments only: `adocs/data/S323_identity.txt` `S323-IDENTITY HOLDS`, the six C++ files' line counts unmoved; with ccache off 326 of 340 objects, archives and generated files are byte-equal, every test object among them, and the fourteen others differ by the `-dirty` stamp or by the CTest file's CMake line numbers, each shown. Bench 5482277 in both builds. Suites 52 of 54 in each build beside datagen, the two plants red by the 60 s clock and green run alone; a third red, `test_plan_citation_freshness`, was this file's own quoting of line citations, fixed and seen green. `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. Two things beyond the rows found and not fixed. The gate's result and the clean reader are the coordinator's. Filler (DEC-171), no score, move or line moved.

## Why

Found 2026-11-02 by S321's implementer, beyond its rows. Filler by DEC-171: none moves a score, a move or a line.

| where | what it says | what was read |
|---|---|---|
| `tests/CMakeLists.txt`, the comment above `add_trace_doctest_target(test_eval_trace)` | "the 2696 positions ... the clamp binding on 478 of them" | false two ways, as `DEV_MANUAL.md`'s sentence S321 corrected: 478 was of the 2764 positions at `f57b92f`, and at `d0220d1` the 2696 JSON rows bind 246 high and 246 low |
| `tools/corpus_dedupe.hpp` (2), `tests/test_audit_fen_semantics.cpp` (3), `tools/tuner_split.hpp`, `tests/test_helpers.hpp`, `tests/test_search.cpp`, `tests/test_evaluation.cpp`, `tools/mutants/time.py` (inside a quoted compiler message) | a `file:line` citation each | S321 row 3's class, a citation by line number that a later edit moves; not checked whether each still points at what it names |

## 2026-11-03: the sites, read against their targets (Opus 5.5 subagent, briefed by the coordinator)

Base `b3739e9`, bench 5482277 in both builds, in the worktree `chesso-s323` on the i7-8700K, beside the coordinator's 12-thread datagen. **Of the ten code pointers, nine no longer pointed at what they name and one, the castling block's, was a line off; each is now a symbol or a phrase.** The clamp sentence says what the case reads. The two quoted messages are observations of an earlier tree: the compiler's is now dated, and the sanitizer's already was. Nothing was shortened and no history was removed (DEC-342). Every C++ edit keeps its file's line count, so no `__LINE__` moved.

"Then" is the commit that wrote the citation, found by `git log -S`; "now" is `b3739e9`.

| # | where | what the check read | what it became |
|---|---|---|---|
| 1 | `tests/CMakeLists.txt`, S262's comment above `add_trace_doctest_target(test_eval_trace)` | false two ways, as the row says. Re-read on `b3739e9` by S321's method, a scratch copy of the clamp case splitting its two counters by the list's three parts: the 212 curated rows 0 high and 0 low, the 2696 JSON rows **246 and 246**, the 8 pinned rows 0 and 0; totals 2916, 246, 246, which the shipped case logs under `--success`, the control. Unchanged from S321's `d0220d1`. `adocs/data/S323_clamp_split.txt` | `DEV_MANUAL.md`'s sentence in the comment's words: 492 of the 2696 at S321 on `d0220d1`, 246 each way, no curated row, and the 478 kept with its denominator and commit |
| 2 | `tools/corpus_dedupe.hpp`, "The key is the engine's" | then (`28fd8c5`) lines 1518 to 1545 of `src/bitboard.cpp` were `compute_full_hash()` whole; now they are inside `see()`, and `compute_full_hash()` opens at line 1843. It still hashes exactly placement, side to move, castling and the en passant square | the function by name, in `src/bitboard.cpp` |
| 3 | `tools/corpus_dedupe.hpp`, `run()`'s determinism paragraph | then (`28fd8c5`) line 2597 was `init_zobrist()`'s "// Fixed seed." above `std::mt19937_64`; now it is inside `count_legal_moves()`. Since S179 and S203 `init_zobrist()` draws splitmix64 from `CHESSO_PROJECT_SEED`, a constant, so "fixed-seed" holds | the seed by its symbol, in `src/bitboard.cpp` |
| 4 | `tools/tuner_split.hpp`, `ply_index()` | then (`7436c6a`) line 883 was `if (us == BLACK) { board->fullmove_counter++; }` in `make_move_impl()`; now it is in the comment above `en_passant_is_capturable()`, and the increment is still in `make_move_impl()`, at 1078, which `make_move()` calls for either colour. Nothing in it decrements the counter; `unmake_move_impl()` does | `make_move_impl()` by name, in `src/bitboard.cpp`; the three lines rewrapped |
| 5 | `tests/test_audit_fen_semantics.cpp`, the header's class 1, the castling block | then (`93a397b`, the audit's test) lines 395 to 435 were the generator's castling block, opening brace to closing; now that block is 394 to 434, under the `CASTLING` banner in `generate_moves_body()`, one line up, so the range still lands in it but for one blank line past its end. Still tests the rights, the empty and the attacked squares, and never the king's or the rook's square | the banner and the function by name; the paragraph rewrapped, same line count |
| 6 | the same, `castling_rook()` | then lines 645 to 659 were `castling_rook()`; now they are the ends of `gen_masks()` and `masks_match()`, and `castling_rook()` is 778 to 792. It still answers from the target square alone | the symbol was already in the sentence; the parenthesis names the file |
| 7 | the same, class 2, the en passant square | then lines 1755 to 1765 were `load_FEN()`'s two-character parse check; now they are inside `move_gives_check()`, and the check opens at 2103 in `load_FEN()`'s section "3. En passant target square". That section still only parses: the victim check since S161 and the capturer check since S042 are a later block of `load_FEN()`, so "checks only that the square parses" is the finding's sentence, as the header's "The finding:" says | the section by its heading |
| 8 | `tests/test_helpers.hpp`, `legal_moves()` | then (`d01e61c`) line 754 was the guard's `return false;`; now it is inside `squares_match_bitboards()`, and the guard is in `make_move_impl()`, its one `return false`. "No test position can reach" holds: `load_FEN()` empties the history, and the one case that reaches the guard, `test_engine.cpp` "make_move refuses once the stack is full", plays toward `HISTORY_MAX_SIZE` itself | `make_move_impl()` by name, in `src/bitboard.cpp` |
| 9 | `tests/test_search.cpp`, S093's malus case | then (`40f5b56`) line 885 was the comment "No legality check here. generate_moves() emits legal moves only" in `make_move_impl()`; now it is in the comment above `en_passant_is_capturable()`, and that comment is at 1089 | INV-1, the invariant that comment states and the one `tests/test_helpers.hpp` cites for the same fact |
| 10 | `tests/test_search.cpp`, the insufficient-material case, **found by the search below** | then (`d01e61c`) `src/search.cpp` line 264 was negamax's `if (is_insufficient_material(...)) { return DRAW_SCORE; }`; now it is inside the comment above `lmr_reduction_ticks()`, and the return is in `negamax_at()` under `if (ply > 0)`, "the root is exempt" | `negamax_at()` by name, in `src/search.cpp`; four lines rewrapped |
| 11 | `tests/test_evaluation.cpp`, DEC-059's paragraph | then (`33aa3b4`) lines 26 to 30 of `tools/tuner.cpp` were "The parameterisation is degenerate on purpose: adding c to every square of psqt_mg[t] and psqt_eg[t] is the same evaluation as adding c to piece_value[t]"; now they are S262's trace paragraph, and that paragraph opens at line 42 with the same sentence, which now covers the king tables too | the paragraph by its phrase, "degenerate on purpose"; four lines rewrapped |
| 12 | `tools/mutants/time.py`, M34's comment, the quoted compiler message | written at `353b635`, whose `src/` is its parent `0abe648`'s; there and then line 823 of `src/chesso.cpp` is `int previous_score = 0;`, the declaration a set-but-unused warning names. Now the declaration is at 1193 | **kept**, the quote unchanged, and dated: the line number is `0abe648`'s and is part of the observation. Why below |
| 13 | `tests/test_audit_fen_semantics.cpp`, the F10 block, **found by the search** | `src/evaluation.cpp`'s line 516, column 36, in a sanitizer's quoted output, and its continuation, line 529: the block already dates it, "observed on `23f926d`", and at `23f926d` those lines are White's and Black's `passed_pawn_mg[bucket]` reads | **kept as it stands**: dated already, an observation and not a pointer |

### Row 12: a line number inside a quoted compiler message

The string is there as evidence, not directions. It says why the mutant carries its two `(void)` lines: without them the mutant failed to compile, and the message is what the compiler said. Its line number belongs to the tree it compiled. Rewriting it to today's 1193 would quote a message no compiler printed. Replacing it with a symbol would make it no longer a quote. Dropping it would remove history, which DEC-342 parks. So the quote stays word for word, and one sentence after it names the tree, `0abe648`, and says the number is part of the observation. The F10 block in row 13 is the same kind of thing and already named its commit, so it needed nothing.

### Old and new, side by side

Row 1, `tests/CMakeLists.txt` (the paragraph from its third line; the first two unchanged):

```
old: # round and the 2696 positions of the JSON suites, the clamp binding on 478 of
     # them -- and every coefficient is the count its index names, against
     # counts the trace did not take. Since S276 it also holds the four positions
     # pinned for the truncation bound, moved from test_eval_model with the mirror.
     # No dataset, so it is a fast test like the rest.
new: # round and the 2696 positions of the JSON suites, the clamp binding on 492 of
     # them at S321 on `d0220d1`, 246 each way, where no curated row does (478 of
     # the 2764 positions the list held at `f57b92f` bound it) -- and every
     # coefficient is the count its index names, against counts the trace did not
     # take. Since S276 it also holds the four positions pinned for the truncation
     # bound, moved from test_eval_model with the mirror. No dataset, so it is a
     # fast test like the rest.
```

`DEV_MANUAL.md`'s sentence, re-read at landing, for comparison: "the 2696 positions of the JSON suites, 492 of which bind the clamp at S321 on `d0220d1`, 246 each way, where no curated row does (478 of the 2764 positions the list held at `f57b92f` bound it)".

Rows 2 and 3, `tools/corpus_dedupe.hpp`:

```
old: // `compute_full_hash()` at `src/bitboard.cpp:1518-1545` covers exactly the four
new: // `compute_full_hash()` in `src/bitboard.cpp` covers exactly the four

old: // fixed-seed (`src/bitboard.cpp:2597`).
new: // fixed-seed (`CHESSO_PROJECT_SEED` in `src/bitboard.cpp`).
```

Row 4, `tools/tuner_split.hpp`:

```
old: // it: `src/bitboard.cpp:883` increments the full-move counter after Black
     // moves, and the side to move alternates, so this is strictly increasing over a
     // game's positions whatever the search does in between.
new: // it: make_move_impl() in `src/bitboard.cpp` increments the full-move counter
     // after Black moves, and the side to move alternates, so this is strictly
     // increasing over a game's positions whatever the search does in between.
```

Rows 5 to 7, `tests/test_audit_fen_semantics.cpp`, the header:

```
old: //      generator's castling block (src/bitboard.cpp:395-435) tests the rights
     //      bits, the empty squares and the attacked squares, and never that the
     //      king stands on e1/e8 or the rook on its corner. make_move() then moves
     //      a rook — castling_rook() answers from the target square, not from the
     //      board (src/bitboard.cpp:645-659) — and move_piece()'s xors *create*
     //      pieces on squares that were empty.
new: //      generator's castling block (generate_moves_body()'s CASTLING section
     //      in src/bitboard.cpp) tests the rights bits, the empty squares and the
     //      attacked squares, and never that the king stands on e1/e8 or the rook
     //      on its corner. make_move() then moves a rook — castling_rook() answers
     //      from the target square, not from the board (src/bitboard.cpp) — and
     //      move_piece()'s xors *create* pieces on squares that were empty.

old: //      square parses (src/bitboard.cpp:1755-1765); an ep square whose victim
     //      square holds no enemy pawn licenses an en-passant capture whose
     //      remove_piece() xor *adds* the missing pawn.
new: //      square parses (its "3. En passant target square" in src/bitboard.cpp);
     //      an ep square whose victim square holds no enemy pawn licenses an
     //      en-passant capture whose remove_piece() xor *adds* the missing pawn.
```

Row 8, `tests/test_helpers.hpp`:

```
old: // way to refuse - the history stack overflow guard at src/bitboard.cpp:754,
     // which no test position can reach.
new: // way to refuse - the history stack overflow guard in src/bitboard.cpp's
     // make_move_impl(), which no test position can reach.
```

Rows 9 and 10, `tests/test_search.cpp`:

```
old:   // generate_moves() emits legal moves only (src/bitboard.cpp:885) and
new:   // generate_moves() emits legal moves only (INV-1) and

old:       // src/search.cpp:264 returns DRAW_SCORE off this function at every node
           // above the root, so agreeing with the rule would change what the search
           // scores and is an SPRT, not a test edit. S067 pins the disagreement
           // rather than resolving it. 2026-08-14_test_review-F05.
new:       // negamax_at() in src/search.cpp returns DRAW_SCORE off this function
           // at every node above the root, so agreeing with the rule would change
           // what the search scores and is an SPRT, not a test edit. S067 pins the
           // disagreement rather than resolving it. 2026-08-14_test_review-F05.
```

Row 11, `tests/test_evaluation.cpp`:

```
old:   // which tuner.cpp:26-30 documents as the same evaluation -- six of these
       // seven positions came out identical either way and this one moved by one
       // centipawn, 716 to 715, on a truncation that crosses zero. A fitted material
       // value only means anything together with its own tables.
new:   // which tuner.cpp documents as the same evaluation ("degenerate on purpose")
       // -- six of these seven positions came out identical either way and this one
       // moved by one centipawn, 716 to 715, on a truncation that crosses zero. A
       // fitted material value only means anything together with its own tables.
```

Row 12, `tools/mutants/time.py`, two lines added after the quote, the quote unchanged:

```
old: # mutant is stillborn instead of measured. Observed: the first form failed to
     # compile at src/chesso.cpp:823, `variable 'previous_score' set but not used`.
new: # mutant is stillborn instead of measured. Observed: the first form failed to
     # compile at src/chesso.cpp:823, `variable 'previous_score' set but not used`.
     # The line number is `0abe648`'s, where line 823 declares `previous_score`:
     # it is part of the observation, not a pointer into this tree.
```

### The search for further line-number citations

Over every tracked file under `src/`, `tools/` and `tests/` but the vendored `tests/doctest`, 179 files, on `b3739e9` before any edit, with `grep -nIE` (case-blind for B):

```
A, a path glued to a line (the project's retired form, plan_prose_check.py's RETIRED widened to more extensions):
   [A-Za-z0-9_-]+\.(cpp|hpp|h|cc|py|sh|txt|cmake|md|json|epd|tsv|toml|yml|yaml|in):[0-9]{1,5}
B, a line named in words:
   \blines? +#?[0-9]+|\bL[0-9]{2,5}\b|#L[0-9]+
C, a bare continuation after an opener:
   (^|[[:space:](,/;]):[0-9]{1,5}([^0-9A-Za-z]|$)
```

**A: 30 hits in 10 files. B: 18 in 9. C: 6 in 2.** Every one read, and each is in one of four groups:

| group | hits | what they are | treatment |
|---|---|---|---|
| a code pointer in a comment | A 10 | nine of the ten the step names, and `tests/test_search.cpp`'s insufficient-material case, row 10, which the step's list did not have | rows 2 to 11: a symbol or a phrase |
| an observation quoted in a comment | A 2 | `tools/mutants/time.py`'s compiler message, the step's tenth, row 12; `tests/test_audit_fen_semantics.cpp`'s sanitizer message, row 13 | dated, the quotes unchanged |
| a row of a frozen data file, the position's name | B 12 | "S145 mined line 168", "line 162 of adocs/data/S145_mined_set.tsv" and the like: `src/search.cpp` once, `tests/test_mate_pv.cpp` 7, `tests/test_search.cpp` 2, `tools/mutants/S202_mate_walk.py` once, `tools/mutants/S296_skipped_mated_fail_low.py` once, in a string | kept. The set has one commit, `14748c9`, 2026-08-21, and no code edit moves its rows. Checked against it: row 101 is game 3499, 162 is 4593 and 168 is 5850, as the comments say |
| not a citation | A 18, B 6, C 6 | strings in tests: fabricated output a stub prints for `/sandbox` paths (`tests/test_mutation_check.py`, A 5) and inputs the citation check must flag or pass (`tests/test_plan_citations.py`, A 5, C 1). `tools/plan_prose_check.py`'s docstring and comments, which show the form it detects and quote earlier step files' text as the history of why (A 8, B 1, C 5). B's others: list indices in `tests/test_bare_fen_relay.py` (2), "reads as line 0" in `tests/test_gate_extra_script.sh`, a PV's "line 9 of 13 plies" in `tools/mutants/S202_mate_walk.py`, and the mutant id L06 in `tools/mutants/search.py` | none: no pointer into code that an edit moves |

A: 10 + 2 + 18 = 30. B: 12 + 6 = 18. C: 6. Run again on the edited tree, A finds 20: the two observations, kept, and the 18 that are no citation. B finds 19, the one more being `tools/mutants/time.py`'s new sentence, which names line 823 inside the observation's dating. C is unchanged. Outside the step's scope, the root `CMakeLists.txt` and `cmake/` give 0 for A and B.

### Proof that it is comments only

`adocs/data/S323_identity.txt`, `S323-IDENTITY HOLDS`:

- the six C++ files byte-equal under `g++ -fpreprocessed -dD -E -P`, old against new, each with its line count unmoved;
- `tools/mutants/time.py` by S306's Python form: `ast` without docstrings SAME, tokens without comments and docstrings SAME, its one docstring unchanged;
- `tests/CMakeLists.txt` changes only `#` comment lines, printed.

**Beside the form, every object.** Both builds were snapshotted at the base, before any edit, stamps reading `b3739e9`: 340 files, every object and archive with the generated header, CTest files, `flags.make`, `link.txt` and `compile_commands.json`. They were then rebuilt with `CCACHE_DISABLE=1`, so each recompile is a real one, and the 24 objects in each build whose dependency file names an edited source were each seen recompiled after the edit. **326 of 340 are byte-equal**, every object of every test among them. Fourteen differ, each for a stated reason:

- `chesso_build_info.hpp` twice, by the stamp alone, now `b3739e9-dirty`;
- `chesso.cpp.o` four times and `tools/tuner.cpp.o` twice, by that stamp: each recompiled by its own command from its unchanged source against the base's header is byte-equal to the base's. `tools/tuner.cpp` includes the edited `tools/tuner_split.hpp`, so its byte-equal recompile also covers that header;
- the four archives, for their `chesso.cpp.o` member alone, by `ar x` and `cmp`;
- `tests/CTestTestfile.cmake` twice: CMake records each test's call lines in `tests/CMakeLists.txt`, its `__LINE__`, and the S262 comment grew two lines. The base's file with every line past 235 read two more is byte-equal to the new one; 40 of its 168 line fields moved, the 128 before the comment did not.

Bench 5482277 in `build` and in `build-tune`, the parent's. Tier 2 (DEC-141) does not bind and `tools/gate_extra.sh` was not run: no code moved, and nothing in `make_move`, `unmake_move`, the generator or the search changed, not even a comment. S306, S308, S319 and S321 are the precedent.

### Manuals

`DEV_MANUAL.md`: checked, no change. Its clamp sentence is the one row 1 now agrees with. Its own line citations are a finding below, not this step's rows. `MANUAL.md`: checked, it holds none of these statements and no line citation; no change.

### Found and not fixed: this step is its rows

- **`DEV_MANUAL.md` carries the same class in prose, four times**, each a path glued to a line, written out here so this file does not carry the form: lines 26 to 30 of the tuner's source for the degenerate parameterisation, now S262's trace paragraph (row 11's target); lines 37 to 45 of the same file for the material-against-table direction, now the end of the scoring paragraph and the first four lines of the degenerate one; line 883 of the engine's board source for the full-move increment, now the comment above `en_passant_is_capturable()` (row 4's); lines 334 to 343 of datagen's source for the lock that keeps a game's rows contiguous, now inside the code that fills a sample, the `lock_guard` on `output_lock` being 170 lines further on. Its other line-number hits are a quoted compiler error and the citation check's own documentation. The step's scope is `src/`, `tools/` and `tests/`; `DEV_MANUAL.md` is not in `touches:`.
- **`tools/corpus_dedupe.hpp`, the paragraph row 2 edits, says "the defect S042 is open on"**, that `make_move` writes an en passant square whether or not a capture is available, and "That defect is in the key here too". S042 is in `adocs/plan_done/`, and `make_move_impl()` now keeps a square only when `en_passant_is_capturable()`. The present tense of a closed step, the class S321's row 2 fixed. Not a line citation, so not this step's.

### Gates

03:57 to 04:31, serial, one-minute load 12.9 to 15.1 throughout, from datagen:

- Both builds rebuilt with ccache off: rc 0, no warning.
- Bench 5482277 in `build` and in `build-tune`.
- Both fast suites, 54 entries each, with `CLANG_FORMAT_MAJOR=22` exported: 51 passed in each, three red in each.
  - `test_tuner_gradient_plants_pawns` and `..._plants_threats` timed out at their 60 s clock, the load the brief names. Run alone and directly, outside the clock, each passed in both builds: pawns 109 s and 109 s, threats 161 s and 161 s.
  - `test_plan_citation_freshness` was red in both, **by this file**: its found-not-fixed item quoted `DEV_MANUAL.md`'s four line citations in their own glued form, which the check refuses in a step file. Written out in words instead, `--citations` reads 0 flagged and the entry, re-run alone, passes in both builds.
  - `test_clang_format_script` passed in both.
  - So the suites are 52 of 54 in each with this file fixed, the two plants red by the clock alone. This is not a green gate; the coordinator's, with datagen paused, is the one that counts.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check`: rc 0.
- `tools/plan_prose_check.py`: `--citations`, `--touches` and `--prose` over this file, and `--params` and `--gate`, each rc 0, with this file as it stands.

The scratch scripts behind both data files are kept beside the brief, gitignored, in `.tuning/coord/S323/scripts/`, as run from the session's scratch directory: S321's `identity.py` adapted, the edits as one script, the snapshot and the comparisons, and the clamp case's scratch addition.

### The `accepts`, clause by clause

- Each site re-read against its target at landing, with what the check read, then true, cited by a phrase or symbol, or gone: the table and the side-by-side above. Row 1 true; rows 2 to 11 by a symbol or a phrase; row 12 kept and dated, row 13 kept, both observations. None removed.
- Further line-number citations under `src/`, `tools/` and `tests/` by a stated search, the search and its count in the step: three patterns, 30, 18 and 6 hits, every hit in one of four groups; one more code pointer, row 10, and one more observation, row 13.
- Comments only, code identical by S306's form, objects byte-equal or each difference explained: `adocs/data/S323_identity.txt`, `S323-IDENTITY HOLDS`, 326 of 340 byte-equal and the fourteen others accounted for. `No functional change`: bench 5482277 in both builds. The message is the coordinator's.
- A clean reader old against new: the coordinator's fast check; the side-by-side above is written for it.
- `adocs/data/README.md`: one row each for `S323_identity.txt` and `S323_clamp_split.txt`.
