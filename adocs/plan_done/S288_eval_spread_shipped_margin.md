id:         S288
goal:       tools/eval_spread reports the margin that ships, read from src/search_params.hpp rather than typed
accepts:    (1) eval_spread's candidate margins include LAZY_EVAL_MARGIN's value as `src/search_params.hpp` defines it, read from the parameter list and not typed as a literal, first in the list as its comment says the current one is; (2) a fast test fails when the shipped margin is absent from the tool's candidates (observed red first by removing it); (3) its comment no longer names 150 as current; (4) no functional `src/` change (amended 2026-10-21 by the coordinator: the margin comment in `src/evaluation.hpp` is restated in this commit, comment only), bench the parent's, `No functional change`; the fast suite green in both builds, format clean, the five prose modes clean
touches:    tools/eval_spread.cpp, tests/; widened 2026-10-21 (As built): tools/eval_spread_margins.hpp (new), DEV_MANUAL.md; by the coordinator: src/evaluation.hpp (the margin comment only), adocs/audit/2026-09-10_adversarial.md (F25's status)
excludes:   any change to the margin, the shortcut or the clamp; re-measuring the spread
decisions:  DEC-171
closes:     2026-09-10_adversarial-F25
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus 5.5 subagent
done:       2026-10-21 13:21 -- tools/eval_spread's candidate margins are tools/eval_spread_margins.hpp's candidate_margins(): LAZY_EVAL_MARGIN first, the symbol src/search_params.hpp's LazyEvalMargin row generates (constexpr in release, the never-set variable in tune), then 150, 200, 250, 300, 400 less any equal to it; MARGINS and its "the current 150 first" comment gone. tests/test_eval_spread.cpp, fast, both builds: the shipped default (search_param_info() by name) first and once; the five fixed points once each, no duplicates; tune only, the first entry follows search_param_set (777, then 200 for the dedup) and is restored. Red first, observed and reverted: the shipped margin removed (release case 1 red, 2 of 10 assertions; tune cases 1 and 3, 8 of 23), typed as {184} (tune case 3 red, cases 1-2 green), dedup removed (tune case 3 red). Smoke run over 20000 rows of selfplay_v2.tsv prints 184 first and the clamp check holds; not a measurement. DEV_MANUAL.md's eval_spread paragraph restated without the number; MANUAL.md checked, unchanged. Fast suite 50/50 in both builds (CLANG_FORMAT_MAJOR=22, DEC-146), format clean, the five prose modes clean, tools/gate.sh --message green, on the tree as committed. src/evaluation.hpp's margin comment restated by the coordinator (comment only), the fast check's three findings fixed before the commit: bench 6042933, the parent's, No functional change. Not run: Debug self-play, gate_extra.sh (no make_move, generator or search code), mutation_check.py (no pruning rule). 2026-09-10_adversarial-F25 closed. Filler under DEC-171. Implemented by one Opus 5.5 subagent beside S110's runs at -j2; gate and commit by the coordinator.

## Why this exists

`2026-09-10_adversarial-F25`: `tools/eval_spread.cpp`'s `MARGINS` is
`{150, 200, 250, 300, 400}`, and the margin has shipped at 184 since S085.
S039 named the finding and would have retired the tool's question with the
clamp; S039 read H0 (record `ca578de`) and kept both, so the defect is live
again. S287's restated comment in `src/evaluation.hpp` says eval_spread does
not report 184 and points at the tuner's load report instead. A tool, not
play: filler under DEC-171, created 2026-10-21 by the coordinator from
S287's fast check.

## As built (2026-10-21, the Opus 5.5 subagent)

**The list.** `tools/eval_spread_margins.hpp` (new) `candidate_margins`
returns `LAZY_EVAL_MARGIN` first, then 150, 200, 250, 300 and 400, dropping
one equal to it so no row prints twice. The first entry is the symbol
`src/search_params.hpp`'s X row for `LazyEvalMargin` generates, not a typed
number: an `inline constexpr` in the release build, the variable in the tune
build, which eval_spread never sets, so both hold the list's default. It is
also the margin the tool's own clamp check uses, so the combined column of the
first row counts the positions the clamp binds on. The symbol rather than
`search_param_info()`'s default by name, so that the test can read the list
the other way and so that the tune build can move it -- which is what tells a
read from a typed 184. The header is the restructure testability needed and
nothing more: `tools/eval_spread.cpp` has no `main` a test can link around.

**The tool.** The `MARGINS` constant is gone from `tools/eval_spread.cpp`; the
exceedance loop reads `eval_spread::candidate_margins()`, and the comment
moved onto that loop says "the shipped one first" and no longer names 150 as
current. Output format unchanged but for the extra row. Smoke run, release
build, first 20000 rows of `.tuning/selfplay_v2.tsv`: rows 184, 150, 200,
250, 300, 400 in that order, combined 59 (0.295 %) above 184, and the clamp
check's last line "clamped pair reproduced evaluate() - evaluate_cheap() on
every position". Consecutive plies of a few games, so a check that the row
prints and not a measurement; re-measuring the spread is excluded.

**The test.** `tests/test_eval_spread.cpp` (new), registered in
`tests/CMakeLists.txt` with `add_doctest_target` and `../tools` on its include
path, fast label, both builds:

1. `tests/test_eval_spread.cpp` "the shipped margin is the first candidate":
   the front equals the `LazyEvalMargin` default found by name in
   `search_param_info()`, and appears once. A second read of the list, not the
   symbol the header uses.
2. `tests/test_eval_spread.cpp` "the fixed points stay and no candidate is
   listed twice": 150, 200, 250, 300 and 400 each once, no duplicates. Named
   at the site as chosen, not measured: not goldens (DEC-142).
3. `tests/test_eval_spread.cpp` "the first candidate follows the parameter
   when it is set", tune build only: `search_param_set` to 777 puts 777 first
   with six rows; to 200 puts 200 first, once, with five rows -- the only way
   to reach the deduplication while the shipped value is on no fixed point;
   restored to the default and checked.

**Red first, each observed and reverted** (`tools/eval_spread_margins.hpp`
saved and copied back, `cmp` identical):

| mutant | build | result |
|---|---|---|
| shipped margin removed, the pre-S288 list | release | case 1 red: `150 == 184`, `0 == 1`; 2 cases, 1 failed, 2 of 10 assertions |
| the same | tune | cases 1 and 3 red, 8 of 23 assertions |
| shipped margin typed as `{184}` | tune | case 3 red: `184 == 777`, `184 == 200`, `0 == 1`; cases 1 and 2 green, which is why case 3 exists |
| deduplication removed | tune | case 3 red: `2 == 1`, `6 == 5` |

Restored: 2 of 2 cases, 10 assertions, release; 3 of 3, 23 assertions, tune.

**Gate.** Fast suite 50/50 in both builds (49 before plus
`test_eval_spread`), `CLANG_FORMAT_MAJOR=22` (DEC-146), `-j2` under
`nice -n 19` beside the coordinator's S110 runs; the release suite re-run
after the last header restore, 50/50. `./clang-format.sh --check` clean.
`tools/plan_prose_check.py` `--prose`, `--citations`, `--touches`,
`--params`, `--gate` each exit 0. No `src/` change: `chesso bench` 6042933,
the parent's; `No functional change`. DEV_MANUAL.md's "Size the lazy
evaluation margin" restated: the shipped margin first, read and not typed, the
five after it, no number written into the prose to go stale on a refit.
MANUAL.md checked: it does not document eval_spread, unchanged. Not run:
Debug self-play and `tools/gate_extra.sh` (no `make_move`, generator or search
code), `tools/mutation_check.py` (no pruning rule; the four mutants above were
run by hand).

**Done by the coordinator in this commit.** `src/evaluation.hpp`'s margin
comment, which said eval_spread's candidates do not include 184 and sent the
reader to the tuner's load report, would be false once this lands; it now
says eval_spread reports the share clamped at the shipped margin first, and
that the tuner's load report counts the same share (comment only, bench
unchanged). F25's `Status:` line moved to closed. The fast check over the
staged diff found both left open and the citation above stale, and they are
fixed before the commit.
