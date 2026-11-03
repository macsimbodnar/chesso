id:         S325
goal:       the line-number citations in the developer documents point at what they name or cite it by a phrase, and `corpus_dedupe.hpp`'s account of the en passant key says S042 closed it
accepts:    each `path:line` citation in `DEV_MANUAL.md`, `MANUAL.md` and `TOOLCHAIN.md` (8 by S323's pattern A on `b3739e9`, among them `DEV_MANUAL.md`'s four: `tools/tuner.cpp` at lines 26 to 30 and 37 to 45, `src/bitboard.cpp` at line 883, `tools/datagen.cpp` at lines 334 to 343) re-read against what it pointed at when written and against `HEAD`, with what the check read, then made true, cited by a phrase or symbol, or removed; the search re-run at landing, its pattern and count in the step; `tools/corpus_dedupe.hpp`'s sentence that the key inherits "the defect S042 is open on" re-read against `make_move_impl()` and S042's step file and made true without removing the history (DEC-342), the code identical by S321's form (preprocessed sources and objects, bench the parent's in both builds); both fast suites, `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` and `tools/plan_prose_check.py`'s checks green
touches:    DEV_MANUAL.md, MANUAL.md, TOOLCHAIN.md, tools/corpus_dedupe.hpp
excludes:   citations in `adocs/plan_done/`, `adocs/audit/`, `decisions.md` and `specs.md`; shortening a passage or removing history (DEC-342); any change to code
decisions:  DEC-171, DEC-342
closes:
blocks:
paused_by:
author:
done:

## Why

Found 2026-11-03 by S323's implementer, beyond S323's rows (its search covered comments under `src/`, `tools/` and
`tests/`). `DEV_MANUAL.md` cites four lines whose content has moved: its `tools/datagen.cpp` citation, lines 334 to 343, names the output lock,
which is now near line 511. `tools/corpus_dedupe.hpp` says the dedupe key inherits a defect "S042 is open on", the en
passant square written whether or not a capture is available; S042 is done and `make_move_impl()` now keeps only a
square a pawn can capture. Filler by DEC-171: no reported score, move or line moves.
