id:         S327
goal:       `corpus_dedupe --verify` counts 64-bit collisions only, not two FENs whose en passant fields S042's key merges
accepts:    a failing test first, observed red: two rows whose FENs differ only by an en passant square no pawn of the side to move attacks (F08's form) dedupe to one key and `--verify` reports 0 collisions; then `hashed_fields()` compares the fields as the key sees them, with the capturable test `make_move_impl()` and `load_FEN()` use, not as raw text; the test green, every other `test_corpus_dedupe` case unchanged and green; the header's and `DEV_MANUAL.md`'s sentences on what `--verify` measures re-read and made true, history kept (DEC-348); both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green; bench unchanged, no `src/` change
touches:    tools/corpus_dedupe.hpp, tests/test_corpus_dedupe.cpp, DEV_MANUAL.md, adocs/data/S327_*, adocs/data/README.md
excludes:   the key itself; any corpus already deduped; `src/`
decisions:  DEC-171, DEC-348
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03 -- `accepts` by clause. Case 11, F08's three rows, read red on the base with the base's header put back (4 checks, `--verify` counting 2) and green after: 15 test cases, 107 assertions. `hashed_fields()` reads the four fields `generate_FEN()` writes for the board `load_FEN()` filled, so the en passant square is the one `en_passant_is_capturable()` let `load_FEN()` keep, reached by loading the FEN, not copied. The base's twelve test cases unedited and green; case 12 holds a square a pawn can take; case 13 plants a real collision from the engine's randoms, no seam added, and is the one case a by-hand mutant of the counter fails. Over `.tuning/selfplay_v2.tsv` (`ad8c5dfe…`, read in place) the kept rows are `cdb43ba2…` on the base and on S327, with and without the flag; `--verify` 503 on the base, 0 on S327; `a3e84e1` reproduces S076's `0a6b59b6…` and 0. The 503 against the 492 extra drops is 11 rows repeating an earlier row of their group, read by `adocs/data/S327_gap.cpp`. Header and `DEV_MANUAL.md` dated, history kept (DEC-348); the fast check's header edit comments only, preprocessed equal and its four dependent objects byte-equal; `MANUAL.md` checked, unchanged. Both fast suites 53 of 54, the one red `test_tuner_gradient_plants_threats` at its 60 s clock under S122's load, green alone unclocked in both (86 s, 88 s); `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` and `tools/plan_prose_check.py`'s five checks green; bench 5482277 in both builds; nothing in `src/`. Re-run whole by a second implementer after a shutdown, `adocs/data/S327_verify.txt` its record. Three findings, not fixed, above; the third answered since by DEC-348.

## Why

Found 2026-11-03 by S325's implementer. Since S042 the key drops an en passant square no pawn of the side to move
attacks, but `hashed_fields()` still compares the raw FEN text. On F08's three rows, which S325 ran
(`adocs/data/S325_ep_key.txt` once S325 lands), `--verify` reports 2 collisions on `1bb3080` and 0 on S042's parent.
On any corpus written before S042, `--verify` therefore no longer measures what it says it measures. Filler by DEC-171:
a tool's diagnostic count, which moves no fit, score, move or line.

## 2026-11-03: the comparison, read from the board the key was taken from (Opus 5.5 subagents, briefed by the coordinator)

Two implementers. The first wrote the change and was stopped by a shutdown while it re-ran its suites; the coordinator
staged its work unverified. The second, this section's author, re-read all of it and re-ran every reading: the red
test, the green, the mutant, the six passes over the corpus and both suites. Each agreed with the first's record. Kept:
the code in `tools/corpus_dedupe.hpp` and the three test cases, unedited. Reworked: the header's and `DEV_MANUAL.md`'s
new paragraphs, reworded with the same content; `adocs/data/S327_verify.txt`, regenerated from the second run, whose
red output is the one below (the first record's line numbers were one short of the file as staged); this section.
Dropped: the first record's wall times for an `istringstream` version, which no longer exists to re-run.

**The failing test first.** Three cases appended to `tests/test_corpus_dedupe.cpp`, numbered 11 to 13 in its list;
the base's twelve test cases, items 1 to 10 of that list, are not edited. Case 11, "--verify reads an en passant
square no pawn can take as the key does", takes F08's form as `adocs/data/S325_ep_key.txt` holds it: the two FENs F08
printed, `c3` and `d3`, and the same position with `-`. python-chess 1.11.2, re-run for S327, writes all three as `-`
in X-FEN with `has_pseudo_legal_en_passant()` False. The case first establishes both preconditions, that the three
texts differ and that `position_key()` gives them one key. It then asserts that `hashed_fields()` reads all three as
`... b KQkq -` and that `--verify` over the three rows writes one, drops two and counts 0 collisions. On the base, with
the base's `corpus_dedupe.hpp` put back in the worktree, it failed:

```
tests/test_corpus_dedupe.cpp:604:
TEST CASE:  --verify reads an en passant square no pawn can take as the key does
tests/test_corpus_dedupe.cpp:637: ERROR: CHECK( c3_fields == "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq -" ) is NOT correct!
  values: CHECK( rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 == rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq - )
tests/test_corpus_dedupe.cpp:639: ERROR: CHECK( d3_fields == c3_fields ) is NOT correct!
tests/test_corpus_dedupe.cpp:640: ERROR: CHECK( none_fields == c3_fields ) is NOT correct!
tests/test_corpus_dedupe.cpp:656: ERROR: CHECK( stats.collisions == 0 ) is NOT correct!
  values: CHECK( 2 == 0 )
[doctest] test cases:  15 |  14 passed | 1 failed | 0 skipped
[doctest] assertions: 107 | 103 passed | 4 failed |
```

The 2 is S325's reading of the tool on the same rows. After the fix: 15 test cases, 107 assertions, all passed. Both
outputs whole are in `adocs/data/S327_verify.txt`, with the two binaries run on the same three rows: the base's counts
2, S327's 0.

**The second case, and how the counting path is covered.** Case 12 holds the other side of the normalisation:
`AFTER_D5_EP`'s `d6`, which a pawn can take (python-chess, the same run), still reads `d6` in `hashed_fields()` and
still differs from the same position without it. So a `hashed_fields()` that wrote `-` for every square would pass
case 11 and fail here. Case 13 plants a real 64-bit collision rather than arguing for one, so the brief's "cannot be
produced on demand" did not hold here. No seam was needed and none was added. A key is the xor of piece-square
randoms, so Gaussian elimination over GF(2) on the engine's own randoms finds a set of them that xors to zero. The
randoms are read from a game `initialize_game_const_data()` fills, exactly as `run()` fills its own. Splitting the set
between two placements gives two FENs with one key. The pool keeps every split loadable: White to move, kings on a1
and h8, black knights anywhere and white knights off f7 and g6. The case then establishes the collision through the
engine, not through its own arithmetic: both FENs load, `position_key()` returns one key for both, and
`hashed_fields()` differs. Over the two rows the pass writes the first and drops the second, with and without
`--verify`, which is the cost the header names, and `--verify` counts 1. The base's binary counts the same pair as 1
too. A by-hand mutant, the `stats->collisions++` in `run()` deleted, fails case 13 alone: every other test case, the
base's twelve and cases 11 and 12, passes against a counter that never counts. The base has one `--verify` case, item
10, and it asserts 0. So before S327 no test reached the counter's increment, and no case showed the count could read
anything but 0. The planted pair, key 15911397024038511196, and the mutant's output are in the data file.

**The fix.** `hashed_fields(const board_t*)` returns the first four fields `generate_FEN()` writes for a board, and
`run()` calls it on the board `position_key()` has just loaded, the one the key was taken from. That board is what
`compute_full_hash()` reads, so the en passant square is the one `load_FEN()` kept by `en_passant_is_capturable()`.
The engine's test is reached by loading the FEN, not copied: the function is static to `src/bitboard.cpp`, and
loading the FEN is the brief's preferred route. Anything else `load_FEN()` clears reads cleared on both sides too.
`hashed_fields(fen, out)` keeps its signature for item 9's case, which is unedited: it loads the FEN into its own game
and reads that board, and is false when the FEN does not load. `run()`'s refusal "row does not carry four FEN fields"
is gone with the text split. It could not fire: `load_FEN()` refuses a FEN that does not split into six
whitespace-separated fields (`split_string()`, the same tokenising the `istringstream` did), and `run()` had already
refused such a row as "does not carry a loadable FEN". `<sstream>` left the header, which no longer uses it. Over the
corpus the `--verify` pass took 61 s against the base's 46 s, one run each under S122's load, not a timing: one
`generate_FEN()` per row.

**The rest is unchanged.** The tool's output over `.tuning/selfplay_v2.tsv`, read in place (sha256
`ad8c5dfe1a236af8cbf946a67692e5fbfe533c7d38b6c7468c8a8ec202d14e05`, 11003693 rows, a corpus written before S042),
the kept rows hashed off stdout:

| tree | dropped | kept rows sha256 | `--verify` |
|---|---|---|---|
| `a3e84e1`, S042's parent, from `git archive` | 207998 | `0a6b59b6…` | 0 |
| S327's base | 208490 | `cdb43ba23a090e687957015639ff211c78a582675c2c44e5a094a0f31661a5a0` | **503** |
| S327 | 208490 | `cdb43ba23a090e687957015639ff211c78a582675c2c44e5a094a0f31661a5a0` | **0** |

On each tree the kept rows are the same with and without the flag. Base and S327 are byte-identical. `a3e84e1`
reproduces S076's figures exactly. Its `load_FEN()` already cleared every field S161's sanitiser clears (`ecd735e`,
2026-08-22), and its `--verify`, the same code as the base's, counted 0. The only clearing `load_FEN()` gained between
`a3e84e1` and `1bb3080` is S042's capturable term; its other additions refuse a FEN, and no row is refused (`git
diff`). So the base's 503 are rows S042's key merged, and S327's 0 says none of them, and no other pair on the corpus,
is a collision.

**503 against 492, added at the coordinator's fast check.** The base drops 492 more rows than `a3e84e1`, and its text
comparison counted 503. The gap is real, not an error in either count. At the base each key's rows are a union of row
groups `a3e84e1` kept apart, and the text comparison counts every row outside the first row's group. So a row that
repeats an earlier row of its own group, a duplicate `a3e84e1` dropped too, counts as well: rows `-`, `c3`, `c3` drop
one more row and count 2. `adocs/data/S327_gap.cpp`, a program no build compiles, groups the corpus by the engine's
key and splits each key by raw text, as the base compared it. Over the corpus in place it reads 503 rows that differ
from their key's first row, all in the en passant field alone, in 492 groups of one key and one text, and 11 rows that
repeat an earlier row of their group. 10795203 keys and 492 groups make `a3e84e1`'s 10795695 written. The clause is
in the header's history paragraph, in `DEV_MANUAL.md`'s new paragraph and in the data file's reading. The header's
edit is comments only; its identity is in the data file.

**The prose, history kept.** `tools/corpus_dedupe.hpp`: S076's identity paragraph is unedited. A paragraph after it
dates the change: until S327 the fields were the row's text, and from S042 on the text was not what the key reads. It
gives the readings above and says what the fields have been since. The comment on `hashed_fields()` describes the new
function; the function itself changed, and the paragraph above carries the old one's history. `DEV_MANUAL.md`
"Deduplicate a corpus by position": "reported **0** of the 207998" became "at S076 reported", and "Two runs over the
same input are byte identical: `0a6b59b6…`" became "were". A new paragraph says what the fields are since S327, what
they were before, and the readings and digests above. The rule followed is DEC-348's, which records it; the step was
first written citing DEC-342, which parks the comment work, and its `accepts:` and `decisions:` lines now cite DEC-348
at the coordinator's word. The `--verify` usage text in `tools/corpus_dedupe.cpp` was
re-read: "the four hashed FEN fields ... which is what a 64-bit collision would look like" is true after the fix, so it
is unchanged. `MANUAL.md` names neither the tool nor the flag (grep), unchanged.

**Found, not fixed.**

1. **DEC-183's recipe no longer rebuilds the dedup file it names.** DEC-183 records that the archive's
   `CORPUS_README.md` carries "the `corpus_dedupe` command that rebuilds the derived dedup file", digest
   `0a6b59b6…`. The archive was made on 2026-09-11, the day before S042 (`50b1ff9`, 2026-09-12). Since S042 that
   command over the same TSV writes `cdb43ba2…`, 10795203 rows, the table above. `.tuning/selfplay_v2_dedup.tsv` on
   this machine still hashes `0a6b59b6…` (`sha256sum`, today). But a machine restoring from the archive with a
   current binary gets a different file from the one the step records name: twelve files in `adocs/plan_done/` name
   `selfplay_v2_dedup` or `0a6b59b6`, S039, S077, S262 and S276 among them. For the coordinator to class by DEC-171:
   it moves no fit until someone refits on a regenerated file.
2. **`DEV_MANUAL.md`'s "Measured on `selfplay_v2.tsv`" sentence** (10795695 out, 207998 dropped, the repeat
   histogram) is S076's reading in the present tense; a current binary drops 208490. The new paragraph states the
   difference, but the sentence itself is undated, S329's kind. Taken into S329's `accepts:` since by the coordinator.
3. **DEC-342 does not say what S325, S327 and S329 cite it for.** All three steps cite it for "history kept, never
   shortened". Its text, 2026-11-01, parks the comment cleanup and resumes S101, and says nothing about a passage's
   history; no decision in `adocs/decisions.md`, read through DEC-347, holds "never shortened" or "history kept".
   Answered since by DEC-348, which records the rule; this step now cites it.
