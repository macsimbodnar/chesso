id:         S213
goal:       the evaluation header's five stale zero-weight blocks, the mate-band argument that names the wrong bound, two dead public entry points and two `<cctype>` calls on a signed `char` are corrected, and a test asserts the bound the argument relied on; the `info` line's field set enters the UCI golden (DEC-184)
accepts:    the five "Zero until the tuner fits them" blocks in `src/evaluation.hpp` and their dependent claims (the `king_safety` block's "comparing 0 against 0", the `passed_pawn` block's) describe the weights that ship, traced to `src/evaluation.cpp`; the comment at the reverse-futility site in `src/search.cpp` `negamax_at` and at `RFP_MARGIN`'s neighbours in `src/search_params.hpp` stop attributing the static score's distance from the mate band to `evaluate_expensive()`'s clamp, which bounds only the expensive stage, and state what does bound it -- the material and table sums -- with a case in `tests/test_evaluation.cpp` asserting `|evaluate()| < MATE_MIN` over every corpus position the suite already loads and over the pathological placements the audit measured (max 14391 against `MATE_MIN` 48000), so the argument has a test; `swap_side` and `set_en_passant` are deleted from `src/bitboard.hpp` and `src/bitboard.cpp` -- zero callers, and the second is a hash-mutating public entry that bypasses the S161 sanitiser -- or, if a test wants one of them, given the sanitiser's checks; `is_uint` in `src/utils.cpp` and `trim_whitespace` in `src/chesso.cpp` cast to `unsigned char` before `isdigit`/`isspace`; the `tt_entry_t` comment in `src/data_structures.hpp` says 22 bytes of content in 24, not 20; `bench` signature identical, `tools/search_bench.py` identical, commit `No functional change`; **DEC-184** `tests/test_uci_surface.cpp` gains a golden over the `info` line's field set -- the field names and their order as `MANUAL.md` documents them, read from one fixed-depth search on the start position, values excluded -- so the S037 class (a field added or a meaning changed with no test noticing) is caught, and it is refreshed only after `MANUAL.md` describes a change (SURFACE)
touches:    src/evaluation.hpp, src/evaluation.cpp, src/search.cpp, src/search_params.hpp, src/bitboard.hpp, src/bitboard.cpp, src/utils.cpp, src/chesso.cpp, src/data_structures.hpp, tests/test_evaluation.cpp, tests/test_engine.cpp, tests/test_uci_surface.cpp, MANUAL.md
excludes:   any change to a weight, a bound or a rule; the lazy clamp itself, which is S039's
decisions:  DEC-170, DEC-171, DEC-184
closes:     2026-09-10_adversarial-F26, 2026-09-10_adversarial-F27, 2026-09-10_adversarial-F33
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-13 19:23 on the idle machine
done:       2026-09-13. Every accepts clause held, and the stale `test` labels and `king_zone()` with them. `bench` **7105111** and `search_bench` node- and move-identical at depths 9 and 12 against a Release build of `58c31f9` -- `No functional change` (INV-6). Gate green in `build`, `build-tune` and `build-debug`, format clean. Three assertions observed red before being relied on; the `<cctype>` fix has no red and the silent sanitizer run is reported. `MANUAL.md` rewritten ahead of the new UCI golden (SURFACE); two `specs.md` edits proposed below for the coordinator. See `## Done, 2026-09-13`.

## Why this exists

Three low findings of `2026-09-10_adversarial`, Part E, all in comments or in
code nothing calls, scheduled as daytime filler under DEC-171.

- **F26.** Five blocks in `src/evaluation.hpp` say "Zero until the tuner fits
  them"; the dependent claims are the defect. One guards `king_safety`, whose
  weights are `{17, 20, 19, 34, -27, 26, 14, -40, -25}`, and says only the
  counts can be checked "because the weights ship at zero". S039's own file
  records this staleness class in this file for a different sentence; the
  file was never swept.
- **F27.** The "static score cannot approach the mate band" argument at the
  reverse-futility site names the clamp on `evaluate_expensive()`, which
  bounds one stage; `evaluate_cheap` is bounded by nothing named. The
  conclusion is true -- measured max `|evaluate()|` 14391 over 80000
  pathological placements, table bound 16991, `MATE_MIN` 48000 -- and no test
  asserts it.
- **F33.** `swap_side` and `set_en_passant` have zero callers; `is_uint` and
  `trim_whitespace` pass a possibly negative `char` to `isdigit`/`isspace`,
  undefined outside `unsigned char`; `tt_entry_t`'s comment says 20 bytes of
  content since the `eval` field landed and it is 22 (`sizeof` 24 is the
  load-bearing half and is right).

## Cost

Agent work, an hour or two; no run.


## Added 2026-09-11: three stale labels in the `test` command's table

Found by the Tier-1 fast check over S208's completing commit `8aff8ac` and
**confirmed by running `test`**, label against what the engine actually prints:

| position | the label in `src/chesso.cpp` | what the engine prints |
|---|---|---|
| `KIWIPETE_POS` | `bestmove e2a6 ponder b4c3` | `bestmove e2a6 ponder e6d5` |
| `BLOCKED_CENTRE_POS` | `bestmove h7h6 ponder c2c3` | `bestmove a7a6 ponder f3g5` |
| `FINE_70_POS` | `bestmove a1b2 ponder a7b7` | `bestmove a1b2 ponder a7b6` |

So two ponder moves are stale and **`BLOCKED_CENTRE_POS`'s expected bestmove
is stale too**. Nothing asserts these -- they are printed beside the result as a
human-readable expectation -- which is why no test caught them, and it is also
why they are worth either re-deriving or deleting: a label nobody checks that
disagrees with the output beside it is worse than no label.

Two that were checked and are **correct**, so they are not in the list:
`KILLER_POS`'s label was re-derived by S208 (`ponder c5d4`), and
`MATE_IN_2_B_POS`'s label reads identically to `MATE_IN_2_W_POS`'s because the
engine genuinely prints `bestmove e5e6 ponder e8d8` for both -- the two
positions are colour swaps on the same squares, not rank mirrors. The fast
check read that as a copy-paste duplication; it is not one.

Decide which: re-derive all three from the engine and say at the site that they
are re-derived at every step that moves the tree (which is most of them, and is
why they went stale), or drop the expectation from the label and leave the
title. The second is the cheaper contract and this step should prefer it unless
the owner wants the expectations.

## Amended 2026-09-12 by the coordinator

`set_en_passant` is gone: S042 deleted it on 2026-09-12 (`50b1ff9`), having
found zero callers, as this step's own list anticipated ("whichever step lands
first, the other adjusts"). Whatever this file says about deleting it is
done; the rest of the step stands.


## Done, 2026-09-13

### Clause by clause

**F26, the zero-weight blocks.** Five blocks said "Zero until the tuner fits
them" and three of the five were false. `passed_pawn_mg`/`_eg`,
`pawn_structure_mg`/`_eg` and `king_safety_mg`/`_eg` have shipped fitted and
non-zero since S027; `piece_placement_mg`/`_eg` and `tempo_mg`/`_eg` are still
zero and say so, and `piece_placement`'s block now says **which** of its
neighbours moved so the next reader can tell a live claim from a stale one.
Tempo's was stale the other way round -- it reads "zero, and not because the
tuner has not reached them", since `--only tempo` fitted mg 10 and the SPRT
that followed reached neither bound, which is a measurement and not a gap.

The dependent claims were the actual defect and all four are rewritten:
`passed_pawn_counts`, `pawn_structure_counts`, `piece_placement_counts` and
`king_safety_features` no longer say the accessor exists "only because the
weights ship at zero" and no longer claim the model comparison is "0 against
0" where it is not. What replaces it is the reason that survives a fit: the
score is one weighted White-minus-Black sum, so a miscount that hits both
sides equally, or one feature's offset by another's, cancels before it reaches
the number, whatever the weights are. `piece_placement_counts` is the one where
the old reason still holds in full and it says so.

**Two more of the same class, found while sweeping and fixed here.** The
`LAZY_EVAL_MARGIN` block in `src/evaluation.hpp` said the mobility maximum of
143 "is still the whole correction only because king safety ships at zero
weight", which stopped being true at S027; it now carries the combined
`tools/eval_spread` reading taken at that point -- p50 27, p95 90, p99 128,
p99.9 178, max 330, 0.365 % clamped against 0.104 % at zero weights -- with
the note that the reading was taken at the margin of 150 and the clamped share
at 184 has not been re-measured. And the `pawn_structure_mg` comment in
`src/evaluation.cpp` illustrated "a residual, not a valuation" with a doubled
weight of +5 that S065 and S076 have since refitted twice; the sentence now
names both refits and makes the same point about the sign that ships.

**F27, the mate-band argument.** The comment at the reverse futility site in
`negamax_at` and the `RFP_MARGIN` block in `src/search_params.hpp` both said
the static score cannot approach the mate band *because* `evaluate_expensive()`
clamps to `+/-LAZY_EVAL_MARGIN`. That clamp bounds one stage. Both now say what
does bound it -- material and the tapered table sums, finite sums of two- and
three-digit constants over at most sixteen men a side, the load boundary
refusing a seventeenth -- and both point at the test. Two stale `src/search.cpp`
line citations in the `RFP_MARGIN` and `RFP_MIN_PLY` blocks became symbol
references in passing (`negamax_at`, `search()` entering it at ply 0); one more
in the `FUT_BASE` neighbourhood was left alone as out of scope.

The test is `tests/test_evaluation.cpp`, **"the static score never reaches the
mate band"**. Two populations. Every FEN `all_test_fens()` loads -- 2696
positions, max `|evaluate()|` **14948**. And 80000 generated placements, of
which `load_FEN` accepts **66430**, max `|evaluate()|` **22945**, on
`3Q1Q1K/3Q1Q2/4QQ2/2Q1Q2Q/4Q1Q1/Q7/3k1Q2/4Q1Q1 b - - 0 1`. Both against
`MATE_MIN` 48000, pinned locally as `MATE_MIN_LOCAL` the way
`tests/test_search.cpp` pins it. The audit measured max 14391 over its own
80000; this generator is coarser and reaches higher, so the assertion is the
stronger one.

The generation rule is mechanical and no chess judgement enters it: a fixed
seed shuffles the 64 squares, both kings go down first, and then either
"maximal" -- sixteen men for one side with fifteen of them queens against a
lone king, which is the largest material sum the boundary permits and more than
promotions could deliver -- or "mixed", both counts and every piece type drawn.
A pawn drawn onto rank 1 or 8 becomes a queen; both sides to move are offered
and `load_FEN` decides which placements are positions. Two non-vacuity
assertions guard it: the accepted count and `pathological_max > corpus_max`.

**F33.** `swap_side` deleted from `src/bitboard.hpp` and `src/bitboard.cpp`
(`git grep` finds zero callers outside its own definition; `set_en_passant` was
already gone with S042, per the 2026-09-12 amendment above). `is_uint` and
`trim_whitespace` go through `unsigned char`, the way `fold_case` has since
S209 -- `trim_whitespace` by a named predicate rather than `::isspace` as a
function pointer. The `tt_entry_t` comment states the layout it is claiming:
8 key + 4 score + 4 move + 2 depth + 1 type + 1 generation = 20 of 24 without
`eval`, **22 of 24 with it**, `sizeof` 24 either way -- compiled and printed,
not counted by eye.

**The three stale `test` labels (the 2026-09-11 addition).** Dropped rather
than re-derived, which is the cheaper contract the step file asked for. All
seven entries in `command_test` are now the position name alone, and the site
says why: the expectation was printed next to the line the engine had just
produced, nothing asserted the two agreed, three of seven had gone stale, and
every change that moves the tree moves them. `MANUAL.md` describes the command
as "the built-in self-test over seven positions" and needed no change.

**DEC-184, the `info` line's field set.** `MANUAL.md`'s "What a search prints"
was rewritten first (SURFACE): the example line is now stated to be the
specification rather than an illustration, `score` first and `pv` last, and the
fields chesso does *not* print are named -- no `hashfull`, `multipv`,
`currmove`, `seldepth` or `tbhits`. `tests/test_uci_surface.cpp` then gained
`expected_info_fields` and the case **"the info line's fields are exactly the
documented ones"**, which holds three ends together: the golden, the field
names read off one `go depth 6` on the start position, and the field names read
off `MANUAL.md`'s own example through the same extractor. Values are excluded
deliberately -- a node count moves with every search change and a golden over
one would be re-derived away in a week. The golden carries its re-derivation
command per DEC-142, including the trap that `go` is asynchronous so a `quit` on
the same pipe prints a depth 1 line.

**The fourth kingless branch, from S223's report.** `king_zone` in
`src/evaluation.cpp` returned an empty zone on a board with no king of that
colour. `load_FEN` refuses such a board since S223, so it joins `is_check`,
`generate_moves_impl` and `king_shelter_features` as a Debug `assert`, for the
same reason: `get_lsb_index` answers 64 on an empty board and 64 indexes
`king_attacks` off its end.

### Red first

Three assertions were observed failing before being relied on, each by moving
one end and rebuilding.

1. "the static score never reaches the mate band" at `MATE_MIN_LOCAL` 20000,
   the pathological half:
   `FATAL ERROR: REQUIRE( score < MATE_MIN_LOCAL ) is NOT correct! values:
   REQUIRE( 21825 < 20000 )` on
   `7Q/Q7/Q2k2Q1/3QQK2/1Q1QQ3/1Q3Q2/Q4Q2/4Q1Q1 b - - 0 1`.
2. The same case at 14000, the corpus half, which fails first and shows that
   loop is live too: `REQUIRE( 14514 < 14000 )` on
   `1R5R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 b - - 1 1`.
3. "the info line's fields are exactly the documented ones", both ends. With
   ` hashfull 0` spliced into the engine's line:
   `Engine: [score cp time depth nodes nps hashfull pv], golden: [score cp time
   depth nodes nps pv]`. With ` seldepth 9` spliced into `MANUAL.md`'s example
   instead: `Manual: [score cp time depth seldepth nodes nps pv], golden:
   [score cp time depth nodes nps pv]`. Both edits reverted and the case is
   green on the tree as it stands.

**The `<cctype>` fix has no red and that is reported rather than hidden.**
`tests/test_engine.cpp` gained "a byte above 127 classifies as neither a digit
nor a space", and it was run under `build-sanitize` (ASan + UBSan) with both
casts reverted: **silent, and green**. glibc's `__ctype_b_loc()` table is
offset by 128 and defined over [-128, -1], so this platform's implementation
happens to answer correctly for the values the standard leaves undefined, and
neither sanitizer instruments a library precondition. The fix stands as a
correctness change -- the behaviour is undefined by the standard and another
libc need not agree -- and the new case pins the contract rather than the
defect.

### Neutrality

`No functional change`. `./build/src/chesso bench` **7105111**, identical to a
Release build of `58c31f9` in `.ref-builds/`. `tools/search_bench.py` against
the same parent binary, node counts and best moves identical at both depths:
depth 9 **47635 / 213916 / 26130**, `c3d5` / `e2a6` / `d7c8q`; depth 12
**141455 / 1038779 / 175684**, `c3d5` / `d5e6` / `d7c8q` (INV-6).

No Debug self-play is owed under DEC-141: `make_move`, `unmake_move`, the
generator and every search rule are untouched -- `src/search.cpp` carries a
comment change and nothing else -- and no pruning, reduction or extension rule
was added or moved. The one executable line added to `src/` is a Debug-only
`assert`, so the Debug binary was built and the fast suite run under it as
well.

### Gate

Green, `CLANG_FORMAT_MAJOR=22`, `-j12`: `build` 38/38 fast, `build-tune` 38/38
fast, `./clang-format.sh --check` clean. Run twice, the second time on the tree
as it stands after two comment reflows and the `MANUAL.md` rewording.

`build-debug` built and its fast suite run on top, for the new `assert` --
2683 s, 37/38, and the one failure is environmental and was reproduced as such:
`test_clang_format_script` was started in a shell without `CLANG_FORMAT_MAJOR`,
so the script looked for clang-format 23, found 18 and failed its four sandbox
cases with "clang-format 23 not found". Re-run with the variable exported:
passes. No `Assertion` anywhere in the Debug log, which is what the new
`king_zone()` guard was being checked for.

### Docs

`MANUAL.md` "What a search prints" gains the paragraph making the example
normative (SURFACE, ahead of the golden). `DEV_MANUAL.md` checked: nothing in
it describes the `info` fields, the `test` labels, the deleted entry point or
the two classifiers, so no change. `README.md` untouched, as always.

### Proposed `specs.md` wording, for the coordinator

Two edits, both to the `position input` row:

1. "Three branches that existed only for a kingless board became Debug
   assertions -- `is_check()`, `generate_moves_impl()` and
   `king_shelter_features()`" becomes **four**, adding `king_zone()` with
   "-- the fourth, `king_zone()` in `src/evaluation.cpp`, followed at S213".
2. In the `uci` row or wherever the `info` line is described, one sentence:
   **"The `info` line's field set is golden since S213 (DEC-184)** --
   `score cp|mate`, `time`, `depth`, `nodes`, `nps`, `pv`, in that order and
   nothing else -- held by `test_uci_surface` against both the engine's own
   line and `MANUAL.md`'s example, values excluded so that a node count cannot
   re-derive it away."

Nothing else in `specs.md` needed changing: its zero-weight sentence already
names the correct five (the four piece placement features and tempo), and its
`sizeof(tt_entry_t)` statement already says 24 before and after.

