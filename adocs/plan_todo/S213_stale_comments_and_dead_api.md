id:         S213
goal:       the evaluation header's five stale zero-weight blocks, the mate-band argument that names the wrong bound, two dead public entry points and two `<cctype>` calls on a signed `char` are corrected, and a test asserts the bound the argument relied on; the `info` line's field set enters the UCI golden (DEC-184)
accepts:    the five "Zero until the tuner fits them" blocks in `src/evaluation.hpp` and their dependent claims (the `king_safety` block's "comparing 0 against 0", the `passed_pawn` block's) describe the weights that ship, traced to `src/evaluation.cpp`; the comment at the reverse-futility site in `src/search.cpp` `negamax_at` and at `RFP_MARGIN`'s neighbours in `src/search_params.hpp` stop attributing the static score's distance from the mate band to `evaluate_expensive()`'s clamp, which bounds only the expensive stage, and state what does bound it -- the material and table sums -- with a case in `tests/test_evaluation.cpp` asserting `|evaluate()| < MATE_MIN` over every corpus position the suite already loads and over the pathological placements the audit measured (max 14391 against `MATE_MIN` 48000), so the argument has a test; `swap_side` and `set_en_passant` are deleted from `src/bitboard.hpp` and `src/bitboard.cpp` -- zero callers, and the second is a hash-mutating public entry that bypasses the S161 sanitiser -- or, if a test wants one of them, given the sanitiser's checks; `is_uint` in `src/utils.cpp` and `trim_whitespace` in `src/chesso.cpp` cast to `unsigned char` before `isdigit`/`isspace`; the `tt_entry_t` comment in `src/data_structures.hpp` says 22 bytes of content in 24, not 20; `bench` signature identical, `tools/search_bench.py` identical, commit `No functional change`; **DEC-184** `tests/test_uci_surface.cpp` gains a golden over the `info` line's field set -- the field names and their order as `MANUAL.md` documents them, read from one fixed-depth search on the start position, values excluded -- so the S037 class (a field added or a meaning changed with no test noticing) is caught, and it is refreshed only after `MANUAL.md` describes a change (SURFACE)
touches:    src/evaluation.hpp, src/search.cpp, src/search_params.hpp, src/bitboard.hpp, src/bitboard.cpp, src/utils.cpp, src/chesso.cpp, src/data_structures.hpp, tests/test_evaluation.cpp, tests/test_uci_surface.cpp
excludes:   any change to a weight, a bound or a rule; the lazy clamp itself, which is S039's
decisions:  DEC-170, DEC-171, DEC-184
closes:     2026-09-10_adversarial-F26, 2026-09-10_adversarial-F27, 2026-09-10_adversarial-F33
blocks:
paused_by:
author:
done:

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
| `TRICKY_POS` | `bestmove e2a6 ponder b4c3` | `bestmove e2a6 ponder e6d5` |
| `CMK_POS` | `bestmove h7h6 ponder c2c3` | `bestmove a7a6 ponder f3g5` |
| `FINE_70_POS` | `bestmove a1b2 ponder a7b7` | `bestmove a1b2 ponder a7b6` |

So two ponder moves are stale and **`CMK_POS`'s expected bestmove is stale
too**. Nothing asserts these -- they are printed beside the result as a
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
