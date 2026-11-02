id:         S320
goal:       the six test binaries that still fail from the repository root read their fixtures from the source tree, so a binary's tally does not depend on the directory it runs from
accepts:    `test_eval_trace`, `test_invariants`, `test_leaf_walk`, `test_eval_model`, `test_evaluation` and `test_movegen` each read the same tally from the repository root, from `build/tests` and from a third directory, before and after shown; every other test binary checked the same way, with what it read; no bound moves and no case is dropped; `test_tuner_gradient`'s `CAPTURE(tuner_optimiser::kind_name(kind))` prints the optimiser's name, seen on a failure; both builds green
touches:    tests/CMakeLists.txt, tests/test_tuner_gradient.cpp, tests/test_chesso.cpp, tests/test_perft.cpp, DEV_MANUAL.md
excludes:   any change to what a case asserts; refactoring the fixture loader beyond S310's `CHESSO_SOURCE_DIR` form
decisions:  DEC-171
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-02 -- `accepts` by clause. All 27 compiled test binaries, run by hand from the repository root, from `build/tests` and from an empty third directory in both builds, read one tally from all three after the change, each equal to its `build/tests` tally before; before, eight read red from the root and the third directory, the step's six (`test_evaluation` 20 of 55, `test_eval_trace` 8 of 12, `test_eval_model` 5 of 24, `test_movegen` 5 of 22, `test_leaf_walk` 4 of 8, `test_invariants` 1 of 2) and two the step had not listed because each has its own `load_json`, `test_chesso` 3 of 12 and `test_perft` exit 2, every red the loader's open. The fix is S310's form: `CHESSO_SOURCE_DIR` on the eight, the same `#ifdef` prefix in the two loaders, and the eight ctest entries moved to the root as the guard, all eight observed red there before the definition. No case, bound, assertion or clock changed, no case dropped, nothing under `src/` or `tools/`. The hold case's `CAPTURE` read `0x5737f65f7eb0` and `0x5737f65f7eb5` on a provoked failure and reads `adam` and `adagrad` now, provoked on scratch copies the tree never held. Fast suites 52 of 54 in both builds, the two reds `plants_pawns` and `plants_threats` timing out at 60 s under datagen, the parent's binary as slow unclocked (110.8 and 162.3 s against 110.2 and 163.6 s); `test_perft` green from the root, 119.38 s; format rc 0 with `CLANG_FORMAT_MAJOR=22`; prose checks rc 0. Tier 2 does not bind. `DEV_MANUAL.md` takes the directory rule and `test_perft`'s line, and, found by the fast check, that paragraph's "the slow label's one binary", false since `b62becb` added `test_tuner_gradient_plants_king_curve` to the label, now names both entries, as does the same claim in the comment above `add_doctest_target_linking` in `tests/CMakeLists.txt`; `MANUAL.md` checked, no change. Filler (DEC-171).

## Why

Found 2026-11-02 by S310's implementer and reproduced by its fast check. S310 made `load_json` in `tests/test_helpers.hpp` read from the source tree under `CHESSO_SOURCE_DIR`, defined on `test_tuner_gradient` and already on `test_search`. The binaries without the macro still open `assets/...` from the working directory. Green under ctest, which runs them from `build/tests`, so filler by DEC-171: not reachable in play, and it costs whoever runs a binary by hand.

| binary | cases failing from the root at S310, every red `Cannot open` |
|---|---|
| `test_evaluation` | 17 of 52 |
| `test_movegen` | 5 of 22 |
| `test_eval_trace` | 6 of 10 |
| `test_leaf_walk` | 4 of 8 |
| `test_eval_model` | 4 of 21 |
| `test_invariants` | 1 of 2 |

Also seen and left by S310: `CAPTURE(tuner_optimiser::kind_name(kind))` in the hold case prints an address and not the name, so a failure there does not say which optimiser it is. Filler for the same reason.

## Done, 2026-11-02

At `b62becb`, in the worktree `chesso-s320`, Release and tune builds, with the coordinator's datagen on 12 threads throughout. Tally logs and the two provoked runs are in the main tree's `.tuning/coord/S320/logs/`, gitignored.

### What changed

- `tests/CMakeLists.txt`: `CHESSO_SOURCE_DIR` on `test_chesso`, `test_movegen`, `test_evaluation`, `test_eval_model`, `test_leaf_walk`, `test_eval_trace` and `test_invariants`, one loop, and on `test_perft` beside its own target: S310's form. Their entries run from the repository root, as `test_tuner_gradient`'s main entry has since S310, because `build/tests` holds a copy of `tests/assets/` that a relative read passes on: from the root a relative read is red in the suite and not only by hand.
- `tests/test_chesso.cpp` and `tests/test_perft.cpp`, `touches:` widened: each has a `load_json` of its own and not `test_helpers.hpp`'s, which is why the step's table, taken over the binaries that include that header, left them out. Both failed from the root the same way, 3 of 12 cases and exit 2, and both take the same `#ifdef CHESSO_SOURCE_DIR` prefix, nothing more. `test_perft`'s refusal loses its "Run from a directory holding assets/perft_json/" line, which the absolute path makes false, and names the path it tried.
- `tests/test_tuner_gradient.cpp`: the hold case captures `optimiser`, a `std::string` made from `kind_name(kind)`. doctest 2.4.11 prints a `const char*` as an address unless `DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING` is defined, and a local string is the form every other `CAPTURE(name)` in `tests/` already takes.
- `DEV_MANUAL.md`: "Run test binaries from `tests/`" becomes "Test binaries run from any directory", and `test_perft`'s paragraph no longer sends it to `build/tests`. `MANUAL.md` checked, no change: it names no test binary's working directory.

Nothing under `src/` or `tools/`, no case, bound, assertion or clock touched.

### Observed red first

The tally below, and ctest: with the eight entries on the root and the definition not yet added, all eight red in `build` (`ctest -R`): six on `Cannot open assets/test_jsons/castling.json` or `assets/perft_json/perft.json`, `test_chesso` on `REQUIRE( file )` and `test_perft` on `cannot open asset`. With the definition the seven fast ones pass from the root in both fast suites below.

### Tallies

Every compiled test binary, 27, run by hand from the repository root, from its build's `tests/` and from an empty directory in the scratchpad, in both builds, before the change and after it. Cases passed of total and doctest's assertion count; `test_perft` has its own `main`, so its tally is the positions it started and the depth rows it printed. A cell holds for both builds unless a tune figure follows it. Checked by script, not by eye: before, the root and the third directory agree on every binary in both builds; after, all three directories agree and equal `build/tests` before, on every binary in both builds.

| binary | root and third, before | `build/tests`, before | root, `build/tests` and third, after |
|---|---|---|---|
| `test_chesso` | 9/12, 1282, rc 1 | 12/12, 31041 | 12/12, 31041 |
| `test_openings` | 6/6, 62 | 6/6, 62 | 6/6, 62 |
| `test_movegen` | 17/22, 31807451, rc 1 | 22/22, 93019551 | 22/22, 93019551 |
| `test_evaluation` | 35/55, 312117, rc 1 | 55/55, 11095854 | 55/55, 11095854 |
| `test_search` | 223/223, 1039070 / tune 230/230, 1039336 | 223/223, 1039070 / tune 230/230, 1039336 | 223/223, 1039070 / tune 230/230, 1039336 |
| `test_engine` | 87/87, 1002719 / tune 88/88, 1005246 | 87/87, 1002719 / tune 88/88, 1005246 | 87/87, 1002719 / tune 88/88, 1005246 |
| `test_eval_model` | 19/24, 25626, rc 1 | 24/24, 3308650 | 24/24, 3308650 |
| `test_tuner_groups` | 17/17, 23842 | 17/17, 23842 | 17/17, 23842 |
| `test_tuner_split` | 12/12, 6535 | 12/12, 6535 | 12/12, 6535 |
| `test_leaf_walk` | 4/8, 390, rc 1 | 8/8, 36587 | 8/8, 36587 |
| `test_tuner_gradient` | 17/17, 99229 | 17/17, 99229 | 17/17, 99229 |
| `test_eval_trace` | 4/12, 3849, rc 1 | 12/12, 1924972 | 12/12, 1924972 |
| `test_tuner_target` | 7/7, 418 | 7/7, 418 | 7/7, 418 |
| `test_corpus_hash` | 7/7, 627 | 7/7, 627 | 7/7, 627 |
| `test_corpus_dedupe` | 12/12, 70 | 12/12, 70 | 12/12, 70 |
| `test_eval_spread` | 2/2, 13 / tune 3/3, 26 | 2/2, 13 / tune 3/3, 26 | 2/2, 13 / tune 3/3, 26 |
| `test_audit_fen_semantics` | 29/29, 100 | 29/29, 100 | 29/29, 100 |
| `test_en_passant_key` | 4/4, 54 | 4/4, 54 | 4/4, 54 |
| `test_audit_polyglot_key` | 1/1, 36 | 1/1, 36 | 1/1, 36 |
| `test_audit_go_infinite` | 3/3, 29 | 3/3, 29 | 3/3, 29 |
| `test_invariants` | 1/2, 21, rc 1 | 2/2, 2736 | 2/2, 2736 |
| `test_search_params` | 6/6, 837 / tune 14/14, 1218 | 6/6, 837 / tune 14/14, 1218 | 6/6, 837 / tune 14/14, 1218 |
| `test_uci_surface` | 16/16, 181 / tune 16/16, 3506 | 16/16, 181 / tune 16/16, 3506 | 16/16, 181 / tune 16/16, 3506 |
| `test_mate_breadth` | 1/1, 4 | 1/1, 4 | 1/1, 4 |
| `test_mate_pv` | 5/5, 1214 | 5/5, 1214 | 5/5, 1214 |
| `test_mate_carry` | 1/1, 105 | 1/1, 105 | 1/1, 105 |
| `test_perft` | 0 positions, 0 depth rows, rc 2 | 21 positions, 58 depth rows | 21 positions, 58 depth rows |

Eight binaries moved, the step's six and the two above, and every red before is the loader's open, one per red case; the step's table had drifted since S310 (`test_evaluation` 20 of 55 red now, `test_eval_trace` 8 of 12, `test_eval_model` 5 of 24). The other nineteen read the same tally everywhere before and after. No case dropped: every binary's case and assertion counts after are its `build/tests` counts before. No run left a file in the third directory or in the worktree. By hand `test_tuner_gradient` runs every case, `king_curve`'s included, and took 568 s to 696 s a run under this load.

### The optimiser's name, seen on a failure

Provoked on scratch copies outside the tree, `CHECK(travelled > 50.0)` tightened to `5e9` so both optimisers fail inside the `CAPTURE`'s scope, each copy built with `build`'s flags against its trace library and run from the root on the one case. No file of the tree held the provocation, so there was nothing to revert there. The parent file:

```
ERROR: CHECK( travelled > 5e9 ) is NOT correct!
  values: CHECK( 1907.02 >  5e+09 )
  logged: tuner_optimiser::kind_name(kind) := 0x5737f65f7eb0
...
  logged: tuner_optimiser::kind_name(kind) := 0x5737f65f7eb5
```

This file:

```
ERROR: CHECK( travelled > 5e9 ) is NOT correct!
  values: CHECK( 1907.02 >  5e+09 )
  logged: optimiser := adam
...
  logged: optimiser := adagrad
```

Both 1 of 1 case red, 2 of 272 assertions, the same values; the change moves nothing but the label.

### The suites

Serial, `CLANG_FORMAT_MAJOR=22`, on the final code, before this section was written, with datagen running. `build`: 52 of 54, `build-tune`: 52 of 54. The two reds are the same in both, `test_tuner_gradient_plants_pawns` and `test_tuner_gradient_plants_threats`, Timeout at 60.07 s and 60.08 s, and they are the load: rerun alone they time out again in both builds, and unclocked from `build/tests` the parent's binary and this one take 110.8 s and 110.2 s on `pawns`, 162.3 s and 163.6 s on `threats`, each 1 of 1 passed. Neither case is one this step changed, and the tally above runs both, passing, from every directory. The seven fast entries that moved to the root pass in both suites; `test_perft`, `slow`, passes from the root through ctest in `build`, 119.38 s. `./clang-format.sh --check` rc 0; `plan_prose_check.py` `--touches`, `--params`, `--gate` and `--citations` rc 0 on this file. The coordinator's gate with the machine paused is the reading that counts for the two plant clocks.

Tier 2 (DEC-141) does not bind: no `make_move`, generator or search change, no pruning rule. Filler (DEC-171): nothing under `src/`, no score, move or line moved, so the bench is the parent's.

### Found, not fixed

- `tests/CMakeLists.txt` still copies `tests/assets/` into `build/tests` at configure, and no test binary reads that copy now; `test_bare_fen_relay` takes its session from the source tree. Dead, harmless, left for a decision.
- The guard covers the entries it moved. A new doctest binary that calls `load_json` without `CHESSO_SOURCE_DIR` still passes under ctest in `build/tests` and fails by hand from the root, the class this step closed. Defining it in `add_doctest_target_linking`, or running every entry from the root, would close the class; both reach past S310's form.
- `adocs/data/S136_screen.sh` runs `test_eval_trace` from `build/tests` with the comment that the position list "reads assets/test_jsons/ relative to it", "as ctest runs them". Both halves are false now; the script still works, from any directory.
- `test_tuner_gradient_plants_pawns` and `_threats` overrun 60 s with datagen alone on the machine, serial, in both builds: S309's 30 s bar was read on a quieter machine, and any gate taken while datagen runs will read them red.
