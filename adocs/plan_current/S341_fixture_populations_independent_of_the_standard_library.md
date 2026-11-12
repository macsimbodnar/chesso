id:         S341
goal:       the fast label is green under clang with libc++: no pinned golden rests on a population the standard library's own shuffle or distribution drew
accepts:    (1) `STAGES=clang tools/gate_extra.sh` ends `GATE-EXTRA-DONE` on the tree, where at S340's fix it ends `GATE-EXTRA-FAILED: clang` on `test_tuner_gradient` in both clang directories, the failure seen first; (2) an inventory, in this file, of every `std::shuffle` and `std::*_distribution` under `tests/` and in the tools a golden's script runs, each read as one of: a property that holds for any draw (left alone, with the reason), or a population a pinned number rests on (made the same under both libraries by drawing from the generator's raw output with arithmetic the tree states itself); (3) every golden that rests on a population this step moves is re-derived by its own named script (DEC-142), never re-read from a run, and each such script reads the same under GCC and under clang; (4) DEC-356's pair is re-derived by `adocs/data/S124_factor_gradient_rows.cpp`'s rule on the new population; if the rule answers no pair, or any `test_tuner_gradient` bound recurs, the step stops for the owner (DEC-344 (9)) and changes nothing of the pair, the push or the bound; (5) no assertion is removed, no bound widened, no case skipped under either compiler; both gated builds green, `chesso bench` the parent's unless (6) applies; (6) `tools/datagen.cpp` and `src/chesso.cpp` draw with a library distribution too: the step reads whether any golden or recorded corpus depends on them and reports it, and changes neither without the coordinator's word, since either moves what a seed reproduces
touches:    tests/test_endgame_positions.hpp, tests/test_tuner_gradient.cpp, adocs/data/S124_factor_gradient_rows.cpp, DEV_MANUAL.md, and whatever the inventory names
excludes:   the finite-difference method itself (B1, with the owner); any evaluation or search change; moving the clang stage into the automatic gate; the MacBook's own build, which is the owner's to run
decisions:  DEC-171, DEC-142, DEC-344, DEC-356
closes:
blocks:     S340
paused_by:
author:
done:

## Why

Found by S340's implementer, 2026-11-12. With the two functions marked, clang compiles the whole tree in `build-clang`
and `build-clang-tune`, and the fast label then reads 60 of 61 in each: `test_tuner_gradient`'s case "the endgame
factor's gradient is the derivative, chain rule in" fails `CHECK( check.worst < 1e-6 )` at 4.70154e-06, over 5696
rows (`.tuning/coord/S340/green/01_clang.log`). Under GCC the same case reads 6.27485e-7 over 5717 rows and is green.

The engine and trace libraries are not the cause: HEAD's and the fixed `src/evaluation.cpp` compile to byte-identical
objects under clang. The likely cause, not yet verified: `placements()` in `tests/test_endgame_positions.hpp` draws
its squares with `std::shuffle`, whose algorithm the standard leaves to the implementation, so libc++ and libstdc++
draw different populations from one `std::mt19937` seed, and DEC-356's pair was picked on GCC's. The case never ran
under clang before, because clang's build stopped at S340's two errors, and before phase 1's landing on 2026-11-06 the
case did not exist.

A second sign of the same class, on S124's candidate: `test_datagen_repetition`'s precondition is red under GCC and
green under clang's library (`.tuning/coord/S124/gate_extra_cand/`). `tools/datagen.cpp` picks its random moves with
`std::uniform_int_distribution`.

Filler under DEC-171: a test fixture, no play, no score, no UCI output. It blocks S340, whose first accept is the clang
stage's `GATE-EXTRA-DONE`, and it is what the owner's MacBook reads on `ctest -L fast`.

## Shape

One Opus subagent, agent-only, no run. **It starts after S124's verdict**: S124's staged candidate moves
`tests/test_tuner_gradient.cpp` and the coverage blocks, and nothing compiles while its SPRT plays. If S124 lands, this
step works on the landed tree.
