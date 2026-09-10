id:         S213
goal:       the evaluation header's five stale zero-weight blocks, the mate-band argument that names the wrong bound, two dead public entry points and two `<cctype>` calls on a signed `char` are corrected, and a test asserts the bound the argument relied on
accepts:    the five "Zero until the tuner fits them" blocks in `src/evaluation.hpp` and their dependent claims (the `king_safety` block's "comparing 0 against 0", the `passed_pawn` block's) describe the weights that ship, traced to `src/evaluation.cpp`; the comment at the reverse-futility site in `src/search.cpp` `negamax_at` and at `RFP_MARGIN`'s neighbours in `src/search_params.hpp` stop attributing the static score's distance from the mate band to `evaluate_expensive()`'s clamp, which bounds only the expensive stage, and state what does bound it -- the material and table sums -- with a case in `tests/test_evaluation.cpp` asserting `|evaluate()| < MATE_MIN` over every corpus position the suite already loads and over the pathological placements the audit measured (max 14391 against `MATE_MIN` 48000), so the argument has a test; `swap_side` and `set_en_passant` are deleted from `src/bitboard.hpp` and `src/bitboard.cpp` -- zero callers, and the second is a hash-mutating public entry that bypasses the S161 sanitiser -- or, if a test wants one of them, given the sanitiser's checks; `is_uint` in `src/utils.cpp` and `trim_whitespace` in `src/chesso.cpp` cast to `unsigned char` before `isdigit`/`isspace`; the `tt_entry_t` comment in `src/data_structures.hpp` says 22 bytes of content in 24, not 20; `bench` signature identical, `tools/search_bench.py` identical, commit `No functional change`
touches:    src/evaluation.hpp, src/search.cpp, src/search_params.hpp, src/bitboard.hpp, src/bitboard.cpp, src/utils.cpp, src/chesso.cpp, src/data_structures.hpp, tests/test_evaluation.cpp
excludes:   any change to a weight, a bound or a rule; the lazy clamp itself, which is S039's
decisions:  DEC-170, DEC-171
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
