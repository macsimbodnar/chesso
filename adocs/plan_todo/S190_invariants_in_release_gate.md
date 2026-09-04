id:         S190
goal:       a Release fast test compares the evaluation accumulators and `squares[]` against a full rebuild after every make and unmake over the corpus, so INV-2 and INV-4 are enforced by the gate
accepts:    a new `fast` doctest target walks every test FEN to depth 3, and to depth 4 on the five positions `tests/test_engine.cpp`'s hash oracle uses, and after every `make_move` and `unmake_move` REQUIREs `material`, `psqt_mg`, `psqt_eg` and `phase` equal to `eval_refresh`'s rebuild and `squares[]` equal to the bitboards -- the two helpers now under `#ifndef NDEBUG` in `src/bitboard.cpp` made available to tests in every build without changing their Debug use; observed red under a mutant that drops one accumulator update in `add_piece` before green; `adocs/specs.md`'s INV-2 and INV-4 rows name the test beside the Debug assertions; Release wall time under 5 s; `DEV_MANUAL.md` "Test" carries the DEC-141 Debug self-play habit with the exact `fastchess` line and what to grep; fast suite green in both builds
touches:    tests/, tests/CMakeLists.txt, src/bitboard.cpp, src/bitboard.hpp, adocs/specs.md, DEV_MANUAL.md
excludes:   any change to `make_move`, `unmake_move` or the accumulators; putting the Debug binaries or sanitizers in the automatic gate (DEC-025)
decisions:  DEC-139, DEC-141
closes:     2026-09-04_test_review-F01
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F01`. Both gated builds are `Release`, so every
`assert(` in `src/` is dead in them, and INV-4's only enforcement is
`assert(eval_accumulators_match(...))` at three sites in `src/bitboard.cpp`;
INV-2's `squares[]` half is the same shape. The hash already has a Release
oracle -- `tests/test_engine.cpp`'s "hash and board survive make/unmake"
compares against `compute_full_hash` after every make -- and it caught the
en-passant hash mutant of the review's fault-injection pass. This step gives
the accumulators the same oracle. `CLAUDE.md` names the accumulator alphabet
as the NNUE hook; a drift there is a wrong static score everywhere and an SPRT
reads it as strength.

## Cost

Machine-free, hours. The Debug self-play habit is a rule (DEC-141) and costs
minutes per step that touches the paths it names.
