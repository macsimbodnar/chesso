id:         S255
goal:       `adocs/data/S192_anchors.py` clamps stage two at `LAZY_EVAL_MARGIN` as the engine does, read from the source rather than written as 150
accepts:    the script's clamp equals `src/search_params.hpp`'s `LAZY_EVAL_MARGIN` (184 today) by reading it, and the anchors it re-derives are unchanged on today's tree (stated, with the run)
touches:    adocs/data/S192_anchors.py; DEV_MANUAL.md; src/search_params.hpp (read, never written)
excludes:   any engine or golden change
decisions:  DEC-142, DEC-171
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05. The script reads `LAZY_EVAL_MARGIN` (184) from `src/search_params.hpp` and clamps with it; on HEAD `afb8e46` its output is byte for byte the old one plus the new margin line, 10 of 10 reproduced; a scratch copy at margin 100 moved 198 to 236 and exited 1, and a missing or duplicated row stops it. Gate green in both builds, 41 of 41 each, format check clean. DEV_MANUAL.md's traps paragraph names the read; MANUAL.md checked, no change. Implementer: one Opus subagent.

## Why this exists (2026-10-05, the coordinator, from S055's report)

The script that re-derives the stand-pat anchors clamps at 150 while the
engine clamps at 184. It binds on no case today, so every golden it produced
is right, but a re-derivation after S039 or S122 moves the margin or the sums
could disagree with the engine and pin a wrong golden. A tooling defect
reaching no play: a filler (DEC-171).

## As built

`adocs/data/S192_anchors.py` gains `lazy_eval_margin()`: it reads
`src/search_params.hpp` and takes the default from the one
`X(LAZY_EVAL_MARGIN, "LazyEvalMargin", <default>, ...)` row, exiting with a
message when it finds that row zero times or more than once -- no hard-coded
fallback. It reads the source on every run, a fitted header included, because a
refit's emitted header carries weights and not search parameters. `score()`
takes the margin and clamps where `evaluate_expensive()` does: `std::clamp` of
the White-relative tapered mobility-plus-king-safety sum, then the side to
move's sign. The run prints the margin it used on its second line.

**Unchanged on today's tree.** `python3 adocs/data/S192_anchors.py` on HEAD
`afb8e46`, before and after the edit, exit 0 both times: `diff` of the two
outputs is the one added line `stage two clamped at +/-184, LAZY_EVAL_MARGIN in
src/search_params.hpp`, every anchor and leaf identical, `10 of 10 reproduced`.
The largest stage-two sum among the cases is 138 (case "black in check, Re8",
mobility 124 plus safety 14), under both 150 and 184, which is why the old
constant bound nothing.

**Live, not vacuous.** The script and `src/eval_tables.hpp`,
`src/evaluation.cpp` copied to a scratch tree with `search_params.hpp`'s default
edited to 100: the run prints `+/-100`, "black in check, Re8" reads `evaluate
236 vs 198 MISMATCH`, `9 of 10 reproduced`, exit 1. The same tree with the row
deleted, and with it duplicated, exits 1 naming the count found (0, 2). The
repository's `src/` was never written.

**Gate.** `export CLANG_FORMAT_MAJOR=22`, both builds, `-L fast`: 41 of 41
passed in each, `./clang-format.sh --check` clean.

**Docs.** DEV_MANUAL.md's "Two traps" paragraph under the goldens table now
says the script reads the margin from the source and stops when the row is not
found exactly once; `touches:` names DEV_MANUAL.md for it. MANUAL.md describes
the UCI surface and does not mention the script: no change.
