id:         S325
goal:       the line-number citations in the developer documents point at what they name or cite it by a phrase, and `corpus_dedupe.hpp`'s account of the en passant key says S042 closed it
accepts:    each `path:line` citation in `DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md` (8 by S323's pattern A on `b3739e9`, among them `DEV_MANUAL.md`'s four: `tools/tuner.cpp` at lines 26 to 30 and 37 to 45, `src/bitboard.cpp` at line 883, `tools/datagen.cpp` at lines 334 to 343) re-read against what it pointed at when written and against `HEAD`, with what the check read, then made true, cited by a phrase or symbol, or removed; the search re-run at landing, its pattern and count in the step; `tools/corpus_dedupe.hpp`'s sentence that the key inherits "the defect S042 is open on" re-read against `make_move_impl()` and S042's step file and made true without removing the history (DEC-342), the code identical by S321's form (preprocessed sources and objects, bench the parent's in both builds); both fast suites, `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` and `tools/plan_prose_check.py`'s checks green
touches:    DEV_MANUAL.md, MANUAL.md, TOOLCHAIN.md, tools/corpus_dedupe.hpp, adocs/data/S325_*, adocs/data/README.md
excludes:   citations in `adocs/plan_done/`, `adocs/audit/`, `decisions.md` and `specs.md`; shortening a passage or removing history (DEC-342); any change to code
decisions:  DEC-171, DEC-342
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- every `path:line` citation in the three documents read against the commit that wrote it and against `1bb3080`, old and new side by side in this file. All eight of pattern A's are in `DEV_MANUAL.md`; `MANUAL.md` and `TOOLCHAIN.md` have none by any pattern and are checked, unchanged. None of the step's four code pointers still pointed at what it names whole, the second only half: each is now a symbol or a phrase, `tools/tuner.cpp` "degenerate on purpose" twice, `make_move_impl()` and `play_games()`. Of the four quotations, the compiler message is kept word for word and dated, its tree never a commit; the check's two examples are kept as they stand; S187's history sentence is kept and given one sentence, because S187's own mapping reads two of its "three baselines" and none of the three citations was wrong when written: S169 wrote S132's and S159's at `fe25f46` and S144 wrote S099's at `7862f41`, where the line was the construction they name, and all three had drifted by `124e557`; corrected after the coordinator's fast check. The search at landing: A 8 lines to 4, B 4 lines and 12 matches unchanged, the S145 data rows, C 2 unchanged, every hit placed. `tools/corpus_dedupe.hpp`'s sentence read against `28fd8c5`, S042's step file and `make_move_impl()` and `load_FEN()` at `1bb3080`: true when written, now in the past tense, with what S042 did in `50b1ff9` and what a corpus deduped before it keeps, observed on `a3e84e1` and `1bb3080` in `adocs/data/S325_ep_key.txt`. Nothing shortened, no history removed (DEC-342). Code identical: `adocs/data/S325_identity.txt` `S325-IDENTITY HOLDS`, the header byte-equal preprocessed, its line count 206 to 211; with ccache off 328 of 340 objects, archives and generated files byte-equal, the twelve others by the `-dirty` stamp, each shown. Bench 5482277 in both builds, `No functional change`. Suites 53 of 54 in each build, 07:44 to 07:57 at a one-minute load near 12 from the S122 screen: `test_tuner_gradient_plants_threats` red by its 60 s clock in both, and run alone and unclocked green in both, 19.6 s and 16.0 s. `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0; `tools/plan_prose_check.py` with no flag, all five checks, rc 0, `--citations` 0 flagged over 20 files. Four things beyond the step found and not fixed, below, now S327, S328 and S329. Diff applies to `1bb3080` alone and after S324's. The gate and the clean reader are the coordinator's. Filler (DEC-171), no score, move or line moved.

## Why

Found 2026-11-03 by S323's implementer, beyond S323's rows (its search covered comments under `src/`, `tools/` and
`tests/`). `DEV_MANUAL.md` cites four lines whose content has moved: its `tools/datagen.cpp` citation, lines 334 to 343, names the output lock,
which is now near line 511. `tools/corpus_dedupe.hpp` says the dedupe key inherits a defect "S042 is open on", the en
passant square written whether or not a capture is available; S042 is done and `make_move_impl()` now keeps only a
square a pawn can capture. Filler by DEC-171: no reported score, move or line moves.

## 2026-11-03: the citations, read against their targets (Opus 5.5 subagent, briefed by the coordinator)

Base `1bb3080`, bench 5482277 in both builds, in the worktree `chesso-s325` on the i7-8700K, beside the coordinator's
S122 screen. **All eight pattern-A hits are in `DEV_MANUAL.md`; `MANUAL.md` and `TOOLCHAIN.md` have none by any of the
three patterns.** Of the eight, the four the step names are code pointers and none still points at what it names
whole: each is now a symbol or a phrase. The other four are quotations, kept: one compiler message, now dated, two
examples in the citation check's own documentation, and one history sentence whose example the check found was not
wrong when written at all, so a sentence now says what S187's own mapping and the writing commits read. The dedupe header's sentence is put in the past tense
for what was true when it was written, and says what S042 did and what that means for a corpus deduped before it.
Nothing was shortened and no history was removed (DEC-342).

"Then" is the commit that wrote the citation, found by `git log -S` on `DEV_MANUAL.md`; "now" is `1bb3080`. None of
`DEV_MANUAL.md`, `MANUAL.md`, `TOOLCHAIN.md`, `tools/tuner.cpp`, `src/bitboard.cpp`, `tools/datagen.cpp`,
`tests/test_engine.cpp` or `src/chesso.cpp` differs between `b3739e9` and `1bb3080`, so every reading holds on both.
The line numbers below are written in words, so this file carries no citation the check refuses.

| # | where | what the check read | what it became |
|---|---|---|---|
| 1 | `DEV_MANUAL.md`, "The second fit's constants are the ones in `eval_tables.hpp`, re-anchored": lines 26 to 30 of `tools/tuner.cpp`, the degeneracy DEC-059 spent | then (`33aa3b4`, S065's landing) the header's paragraph "The parameterisation is degenerate on purpose", whole, five lines: a constant added to a piece's tables is the same evaluation as one added to its value. Now those five lines are the opening of S262's trace paragraph, "It does not run the search", and the degeneracy paragraph opens sixteen lines on, its first clause still that direction, the king tables beside it since S275. Off target | the paragraph by its phrase, `tools/tuner.cpp` "degenerate on purpose", in the file's header; the paragraph rewrapped from that line to its end |
| 2 | `DEV_MANUAL.md`, S100's four runs: lines 37 to 45 of `tools/tuner.cpp`, the material-against-table null direction | written as lines 36 to 40 at `9202f5d` (S100), the same paragraph whole there; S275 re-anchored it at `f57b92f` to 37 to 45, the paragraph's first nine of thirteen lines, the direction in its first three. Now 37 to 40 are the end of the scoring paragraph, "Every row is also scored the engine's way", 41 is blank, and 42 to 45 are the degeneracy paragraph's first four lines, which still state the direction. Half on target | the same paragraph by the same phrase; rewrapped to the paragraph's end |
| 3 | `DEV_MANUAL.md`, the four-column split: line 883 of `src/bitboard.cpp`, the full-move increment | then (`7436c6a`) `if (us == BLACK) { board->fullmove_counter++; }` in `make_move_impl()`. Now it is inside S042's comment above `en_passant_is_capturable()`, and the increment is still in `make_move_impl()`, which `make_move()` calls for either colour; only `unmake_move_impl()` decrements. S323's row 4 read the same for `tools/tuner_split.hpp` | `src/bitboard.cpp` `make_move_impl()` by name, in S323's row 4 words, "increments the full-move counter after Black moves"; three lines rewrapped |
| 4 | `DEV_MANUAL.md`, "This makes `datagen`'s row order load-bearing": lines 334 to 343 of `tools/datagen.cpp` | then (`7436c6a`) the output block of `play_games()`, opening brace to closing: the `lock_guard` on `output_lock`, the loop over `samples`, the `fflush`. Now those lines are inside `sample_leaf()`, which fills one sample, and the block is further on, still in `play_games()`: one guard around the loops over every leaf kind's `samples` and their flushes, so "the whole `samples` loop runs inside one `lock_guard`" holds | `tools/datagen.cpp` `play_games()` by name; rewrapped |
| 5 | `DEV_MANUAL.md`, "Why `build-tune` is in it": the fenced compiler message, line 1628 column 19 of `tests/test_engine.cpp` | written at `2fbc4dd` (S143). An observation of a working tree that was never a commit: `git log --all -S` finds `static_assert(ASPIRATION_MIN_DEPTH` under `tests/` in no commit, and `04bcc43`, the S085 fix that tree became, says "The guard was first written as a static_assert" and landed it as a `REQUIRE`. The passage named the event, not the tree | **kept word for word**, and a three-line paragraph after the fence dates it, S323's row 12 form. It carries no line number of its own |
| 6 | `DEV_MANUAL.md`, "`BARE`, and why the check refuses to resolve one": its first example, an evaluation citation and its bare continuation (pattern C's first hit) | written at `7862f41` (S144). The documentation of the check: the form it refuses, quoted from S055's step file, whose text S138 rewrote at `7b54f18` and kept as the example in `tools/plan_prose_check.py`'s docstring, as `git log -S` finds. Not a pointer | kept as it stands |
| 7 | the same paragraph, S095's transposition-table citation and its bare continuation (pattern C's second hit) | written at `7862f41`; S095's text as `c0954ec` wrote it, quoted by S138's step file too. Not a pointer | kept as it stands |
| 8 | `DEV_MANUAL.md`, "What a citation is worth, and what proves it": line 674 of `src/chesso.cpp`, cited by three steps, "had fallen into the book-move helper at all three baselines" | written at `dc878de` (S187's stamp), history. Read against S187's mapping, `adocs/data/S187_symbols.tsv`, and the file at each step's baseline: S132's at `7504c3d` (S148) and S159's at `192a5a3` (S184) were `move_t legal_moves[MAX_MOVES];` in `search_book_move()`; S099's at `7862f41` read `search_state_t state = {}; state.tt = &tt;` in `iterative_deepening_search()`, the row's own `then` field. So the helper held it at two of the three baselines. **But a baseline is the last commit that edited the step file, not the one that wrote the citation, and none of the three was wrong when written** (first delivery read S132's and S159's as wrong; the coordinator's fast check corrected it, and this reading is re-derived): S169 wrote S132's and S159's at `fe25f46`, moving both from 647 to 674 (`adocs/data/S169_recitations.tsv`, rows 94 and 97, `BLOCK`, head text `search_state_t state = {};`), and S144 wrote S099's at `7862f41`; `src/chesso.cpp` is the same in those two trees, and there line 674 is `search_state_t state = {};` in `iterative_deepening_search()`. It moved at `d08ab52`, `62d07d4` (S172), `4dcb292` and `dfee410` (S189), the last putting it in `search_book_move()`, so all three had drifted by S187's start, `124e557`. Now line 674 is inside `compute_search_time_budget()`, and the state is heap-owned, a `std::make_unique<search_state_t>` in `iterative_deepening_search()` | **kept**, and one sentence added before `DEC-119.`: two of the three baselines by the mapping, each a later edit of its step file, none wrong when written, and the commits that wrote them. "Six", S187's own count, is not touched: S329, below |
| 9 | `tools/corpus_dedupe.hpp`, "The key is the engine's, not the FEN text": "the defect S042 is open on" | then (`28fd8c5`, S076, 2026-08-17) true: S042 was in `adocs/plan_todo/`; `make_move_impl()` set `new_en_passant` on every double push; `load_FEN()` kept any square that parsed; `compute_full_hash()` keyed it. S042's step file has it done on 2026-09-12; `git log` has the fix in `50b1ff9` and the close in `9502175`, both that day. Now `make_move_impl()` sets the square only when `en_passant_is_capturable()`, a pawn of the side to move attacking it, and `load_FEN()`'s sanitiser drops one that fails that test, so the key the pass takes through `position_key()` no longer carries the square for a row written before S042 either. Observed, `adocs/data/S325_ep_key.txt`: F08's two FENs and the bare one, uncapturable by python-chess's X-FEN, are three keys on `a3e84e1`, `50b1ff9`'s parent, and one on `1bb3080`; `tests/test_corpus_dedupe.cpp` asserts the same merge, "an en passant square with no capturing pawn no longer separates two positions". A corpus deduped before S042 was keyed the old way: S076's `selfplay_v2_dedup.tsv`, 10795695 rows, which S276's truncation scan still read at that count | the account in the past tense, then what S042 did, in both places that decide the square, and what a pre-S042 dedupe keeps; six lines become eleven. Code unchanged, below |

### Old and new, side by side

Row 1, `DEV_MANUAL.md`, from the paragraph's third line:

```
old: `POSITIONAL_ROOM`, and DEC-059 answered it by spending the degeneracy
     `tools/tuner.cpp:26-30` documents rather than by widening the guard: 432 off
     `#define QUEEN` and 432 onto all 128 of her squares, which moves the evaluation
     by one centipawn on one of seven pinned positions and by nothing on the other
     six. `adocs/plan_done/S065_corpus_regen_loosened_filter.md` records the
     transformation and both fits' guard outcomes.
new: `POSITIONAL_ROOM`, and DEC-059 answered it by spending the degeneracy
     `tools/tuner.cpp` documents in its header, "degenerate on purpose", rather than
     by widening the guard: 432 off `#define QUEEN` and 432 onto all 128 of her
     squares, which moves the evaluation by one centipawn on one of seven pinned
     positions and by nothing on the other six.
     `adocs/plan_done/S065_corpus_regen_loosened_filter.md` records the
     transformation and both fits' guard outcomes.
```

Row 2:

```
old: material-against-table one `tools/tuner.cpp:37-45` names, which showed up as the
     pawn table uniformly 5 low against `PAWN` 5 high. `--only passed_pawns` recovers
     all twelve exactly, bucket 5 included, because holding the table resolves the
     degeneracy.
new: material-against-table one the header of `tools/tuner.cpp` names in its
     "degenerate on purpose" paragraph, which showed up as the pawn table uniformly 5
     low against `PAWN` 5 high. `--only passed_pawns` recovers all twelve exactly,
     bucket 5 included, because holding the table resolves the degeneracy.
```

Row 3:

```
old: (`src/bitboard.cpp:883`), so a row whose ply does not advance on its predecessor
     cannot belong to the same game. The predicate is one-sided and the direction is
     what makes the result exact:
new: (`make_move_impl()` in `src/bitboard.cpp` increments the full-move counter after
     Black moves), so a row whose ply does not advance on its predecessor cannot
     belong to the same game. The predicate is one-sided and the direction is what
     makes the result exact:
```

Row 4:

```
old: `tools/datagen.cpp:334-343`. A change that interleaved two games' rows would
     leave the reconstruction silently wrong and `tests/test_tuner_split.cpp` could
     not see it: it holds the splitter, not the writer.
new: in `play_games()` in `tools/datagen.cpp`. A change that interleaved two games'
     rows would leave the reconstruction silently wrong and
     `tests/test_tuner_split.cpp` could not see it: it holds the splitter, not the
     writer.
```

Row 5, three lines added after the fence, the message unchanged:

```
old: ```
     
     The old gate went green on that tree. `build-tune` is not a convenience: it is
new: ```
     
     The line and column are that working tree's and were never committed:
     `04bcc43` landed the guard as a `REQUIRE` instead, so they are part of the
     observation and not a pointer into this tree.
     
     The old gate went green on that tree. `build-tune` is not a convenience: it is
```

Row 8, the sentence kept and one added:

```
old: the per-`go` `search_state_t` is built and had fallen into the book-move helper
     at all three baselines. DEC-119.
new: the per-`go` `search_state_t` is built and had fallen into the book-move helper
     at all three baselines. By S187's own mapping the helper held it at two of the
     three baselines, S132's at `7504c3d` and S159's at `192a5a3`, each a later edit
     of its step file. None of the three was wrong when written: S169 wrote S132's
     and S159's at `fe25f46` (`adocs/data/S169_recitations.tsv`) and S144 wrote
     S099's at `7862f41`, where the cited line was that construction in both trees,
     and all three had drifted into the helper by `124e557`. DEC-119.
```

Row 9, `tools/corpus_dedupe.hpp`, from the paragraph's sixth line:

```
old: // it would inherit the defect S042 is open on -- `make_move` writes an en
     // passant square whether or not a capture is available
     // (`2026-08-13_adversarial-F08`), so two transposing move orders reach one
     // position under two FENs. That defect is in the key here too, and it is the
     // project's own definition of a position rather than a second one invented for
     // this tool.
new: // when this pass was written it would have inherited the defect S042 was then
     // open on -- `make_move` wrote an en passant square whether or not a capture
     // was available (`2026-08-13_adversarial-F08`), so two transposing move orders
     // reached one position under two FENs. That defect was in the key here too,
     // and it is the project's own definition of a position rather than a second
     // one invented for this tool. S042 closed it in `50b1ff9`, 2026-09-12, in both
     // places that decide the square: make_move_impl() sets it only when a pawn of
     // the side to move attacks it, and load_FEN() drops one no such pawn attacks.
     // So a key taken since then is the same for both move orders, a row written
     // before S042 included. A corpus deduped before S042 was keyed the old way and
     // keeps one row of such a position per en passant field its rows carry.
```

### The search

S323's three patterns, `grep -nIE` (case-blind for B), over `DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md`, and over
`tools/corpus_dedupe.hpp`:

```
A, a path glued to a line:
   [A-Za-z0-9_-]+\.(cpp|hpp|h|cc|py|sh|txt|cmake|md|json|epd|tsv|toml|yml|yaml|in):[0-9]{1,5}
B, a line named in words:
   \blines? +#?[0-9]+|\bL[0-9]{2,5}\b|#L[0-9]+
C, a bare continuation after an opener:
   (^|[[:space:](,/;]):[0-9]{1,5}([^0-9A-Za-z]|$)
```

**On `b3739e9` and on `1bb3080`, identical: A 8 lines, B 4 lines holding 12 matches, C 2 lines, all in
`DEV_MANUAL.md`; 0, 0 and 0 in `MANUAL.md`, in `TOOLCHAIN.md` and in `tools/corpus_dedupe.hpp`.** Every hit read:

| group | hits | what they are | treatment |
|---|---|---|---|
| a code pointer | A 4 | rows 1 to 4, the step's four | a symbol or a phrase |
| a quoted observation | A 1 | row 5, the compiler message | dated, unchanged |
| the citation check's documentation and history | A 3, C 2 | rows 6 and 7, whose two bare continuations are C's two; row 8 | kept; row 8 given a sentence |
| a row of a frozen data file, a position's name | B 4 lines | "S145 mined line 168", "line 162 (game 4593) of `S145_mined_set.tsv`" and the like, in the mate-goldens table and its prose. The set has one commit, `14748c9`, 2026-08-21; read against it, file line 101 is game 3499, 114 is 4543, 162 is 4593 and 168 is 5850, as the cells say | kept, as S323 kept the same rows in code comments |

Re-run on the edited tree, at landing: **A 4 lines** (rows 5 to 8, kept), **B 4 lines and 12 matches**, **C 2 lines**,
unchanged; 0, 0 and 0 in the other two documents and in the header. None of the added text names a line by number.

### The dedupe sentence, and why six lines become eleven

The sentence was true when written, so it keeps its words in the past tense, "when this pass was written", "was then
open on", "wrote", "was available", "reached", "was in the key". What S042 changed is added, not substituted: the
commit and date, the two places that decide the square, by name, and the consequence a reader of the tool needs,
which runs both ways. A key taken now merges a row written before S042 with the same position written without its
square, as `adocs/data/S325_ep_key.txt` shows. A corpus deduped before S042 kept one row per en passant field, and
`selfplay_v2_dedup.tsv`, S076's output on 2026-08-17, is one that later steps read. Whether re-deduping it now would
drop more rows was not measured: the step does not ask for it, and the machine is the screen's.

`compute_full_hash()` is not named as a deciding place: S042's table puts it with the consumers, and the paragraph's
first sentence already cites it for the four fields. The sentence's "the side to move" is the side to move after the
push, `them` in `make_move_impl()`, and the side to move in the FEN in `load_FEN()`: the same rule, as
`src/bitboard.cpp` `en_passant_is_capturable()` states it once.

### Proof that the code is identical

`adocs/data/S325_identity.txt`, `S325-IDENTITY HOLDS`, S321's form as S323 ran it:

- `tools/corpus_dedupe.hpp` byte-equal under `g++ -fpreprocessed -dD -E -P`, old against new, 2922 bytes, sha256
  prefix `03a343a50fbdc8d5`, the same as S323's on `b3739e9`. **Its line count moves, 206 to 211**: the header holds no
  `__LINE__` user and no object differs for it, below.
- Every other path the diff touches is a document, a step file or a data file, which no build compiles.
- **Every object.** Both builds were snapshotted at the base before any edit, stamps reading `1bb3080`: 340 files,
  every object and archive with the generated header, the CTest files, `flags.make`, `link.txt` and
  `compile_commands.json`. They were rebuilt with `CCACHE_DISABLE=1` after the edit. The two objects in each build
  whose dependency file names the header, `corpus_dedupe.cpp.o` and `test_corpus_dedupe.cpp.o`, were each seen
  recompiled. **328 of 340 are byte-equal**, both of those among them. The twelve others differ by the `-dirty` stamp:
  the generated header twice, by the stamp alone; `chesso.cpp.o` four times and `tuner.cpp.o` twice, each byte-equal
  to the base's when recompiled by its own command from its unchanged source against the base's header; and the four
  archives, for their `chesso.cpp.o` member alone, by `ar x` and `cmp`. `tests/CMakeLists.txt` is untouched, so no
  CTest file differs.
- Bench 5482277 in `build` and in `build-tune`, the parent's. `No functional change`.

Tier 2 (DEC-141) does not bind, and `tools/gate_extra.sh` was not run: no code moved, and nothing in `make_move`,
`unmake_move`, the generator or the search changed. S306, S321 and S323 are the precedent.

The scratch scripts behind both data files are kept beside the brief, gitignored, in `.tuning/coord/S325/scripts/`:
S323's `identity.py`, `snapshot.sh`, `compare.sh`, `recompiled.py`, `stamped.py`, `archives.sh` and `assemble.sh`
adapted, `ep_key.sh` with its input `ep_rows.tsv`, and the run wrappers `build_base.sh`, `rebuild.sh` and
`suites.sh`. The tree before S042 was taken by `git archive a3e84e1` into the scratch directory, not as a
worktree, and built for its one target.

### Manuals

`DEV_MANUAL.md`: changed, rows 1 to 5 and 8. S324's two paragraphs on the build's `tests/assets` copy are untouched,
and this diff applies after S324's, below. `MANUAL.md` and `TOOLCHAIN.md`: checked, no line citation by any of the
three patterns and none of these statements, no change. `MANUAL.md`'s en passant convention, "actually play the
capture" since S042, agrees with the header's new sentence.

### Found and not fixed: this step is its rows

Each is now a planned step, named by the coordinator after the fast check; their step files are not in this diff.

- **`--verify` counts S042's merge as a collision.** `hashed_fields()` in `tools/corpus_dedupe.hpp` compares the raw
  FEN text's four fields, while the key since S042 drops an en passant square no pawn attacks. On rows written before
  S042, `--verify` reports the merge as key-equal rows that disagree: 2 of F08's three rows on `1bb3080`, 0 on
  `a3e84e1` (`adocs/data/S325_ep_key.txt`). So the header's "the exposure is measured on the real corpus instead of
  being argued from the bound" and `DEV_MANUAL.md`'s "which is what a 64-bit collision looks like" no longer hold on
  a pre-S042 corpus, and `tests/test_corpus_dedupe.cpp`'s "--verify changes no output and reports no collision" has no en
  passant row. Tools only, filler (DEC-171). **S327.**
- **`DEV_MANUAL.md`'s "The second fit's constants are the ones in `eval_tables.hpp`, re-anchored."** is the present
  tense of S065's fit: six commits since `33aa3b4` have changed `src/eval_tables.hpp`. Not a line citation. **S329.**
- **The count "six" in row 8's paragraph** is S187's stamp's "Six more were wrong when written", and the stamp's own
  list names eight citations: three at line 674, S132's node counter and per-iteration reset, S115's two, S024's
  one. The three at line 674 were right when written (row 8), so five remain, and this step did not verify any of
  the five: at most five against the stamp's six. Not reconciled here; `plan_done/` is history and the sentence
  reports it. **S329.**
- **`adocs/audit/2026-08-13_adversarial.md`, F08, still reads `Status: planned`**, though S042's `closes:` names it
  and S042 is done. AGENTS.md lets a report's status line move; it is outside this step's `touches:`. **S328**, which
  covers 60 findings like it.

### Gates

The step's `accepts`, clause by clause, with the gates' readings:

- The eight pattern-A citations, each read against the commit that wrote it and against `1bb3080`, with what the
  check read: the table, rows 1 to 8. Four by a symbol or a phrase; four quotations kept, one dated and one given the
  sentence its check required. None removed.
- The search at landing, its patterns and counts: above, A 8 to 4, B 4 and 4, C 2 and 2.
- The dedupe sentence, read against `make_move_impl()` and S042's step file, made true without removing history: row
  9. Code identical by S321's form: `adocs/data/S325_identity.txt`, bench 5482277 in both builds.
- Suites: 53 of 54 in each build, `test_tuner_gradient_plants_threats` red by its 60 s clock under the screen's load
  and green run alone, unclocked, in both (19.6 s in `build`, 16.0 s in `build-tune`). `test_plan_citation_freshness`,
  `test_clang_format_script` and `test_corpus_dedupe` green in both. Not a green gate: the coordinator's, on a quiet
  machine, is the one that counts.
- `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` rc 0; `tools/plan_prose_check.py`, all five checks, rc 0, run
  again on this file as it stands.
