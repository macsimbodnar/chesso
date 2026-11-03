id:         S327
goal:       `corpus_dedupe --verify` counts 64-bit collisions only, not two FENs whose en passant fields S042's key merges
accepts:    a failing test first, observed red: two rows whose FENs differ only by an en passant square no pawn of the side to move attacks (F08's form) dedupe to one key and `--verify` reports 0 collisions; then `hashed_fields()` compares the fields as the key sees them, with the capturable test `make_move_impl()` and `load_FEN()` use, not as raw text; the test green, every other `test_corpus_dedupe` case unchanged and green; the header's and `DEV_MANUAL.md`'s sentences on what `--verify` measures re-read and made true, history kept (DEC-342); both fast suites and `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` green; bench unchanged, no `src/` change
touches:    tools/corpus_dedupe.hpp, tests/test_corpus_dedupe.cpp, DEV_MANUAL.md
excludes:   the key itself; any corpus already deduped; `src/`
decisions:  DEC-171, DEC-342
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S325's implementer. Since S042 the key drops an en passant square no pawn of the side to move
attacks, but `hashed_fields()` still compares the raw FEN text. On F08's three rows, which S325 ran
(`adocs/data/S325_ep_key.txt` once S325 lands), `--verify` reports 2 collisions on `1bb3080` and 0 on S042's parent.
On any corpus written before S042, `--verify` therefore no longer measures what it says it measures. Filler by DEC-171:
a tool's diagnostic count, which moves no fit, score, move or line.
