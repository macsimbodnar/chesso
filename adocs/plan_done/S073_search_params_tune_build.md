id:         S073
goal:       the search constants become one addressable parameter set, settable in a tune build and unchanged in the shipping one
accepts:    every constant in the set keeps its value today, so `tools/search_bench.py` reports identical node counts and identical best moves against the commit before this one and no SPRT is owed (INV-6); the shipping build exposes no new UCI option and `test_uci_surface` is unchanged by it; a build configured `-DCHESSO_TUNE=ON` accepts `setoption name <param> value <n>` for every member of the set and `tools/search_bench.py` on that build, with no `setoption` sent, still reports the same counts as the shipping build; a test asserts the two builds' default parameter values are equal member by member, so a default that drifts in one build fails rather than ships; `MANUAL.md` and `DEV_MANUAL.md` state what the tune build is, that it is not the release binary, and how to set a parameter
touches:    src/search.cpp, src/evaluation.hpp, src/chesso.cpp, CMakeLists.txt, tests/, MANUAL.md, DEV_MANUAL.md
excludes:   changing any parameter's value, which is S068's, S039's and S085's; SPSA itself, which is S084; the evaluation weights, which the tuner already owns through `tools/eval_model.hpp`; adding a parameter that does not exist today
decisions:  DEC-019
closes:
blocks:
paused_by:
done:      2026-08-16. Ten search constants are one addressable set in src/search_params.hpp; -DCHESSO_TUNE=ON makes them UCI spin options and the release build folds them as before.
            
            INV-6, the whole point: tools/search_bench.py <binary> 9 reports 292313 / 1026739 / 103001 nodes and best moves c3d5 / e2a6 / d7c8q at e78faed, at this commit, and on build-tune with no setoption sent. Identical on all three positions and all three moves in all three runs. No SPRT is owed.
            
            Correction to this file's own body, written while doing the step: it said 'the values stay constexpr in the default build'. At e78faed they are not constexpr - eight are #define macros and four are literals inline in the code. The default build's half was a CONVERSION to inline constexpr int, not a preservation, and was proved neutral by the counts above rather than assumed to fold. accepts: is untouched.
            
            Use sites checked before anything became a variable: no member of the set is an array dimension, a template argument, a case label or a static_assert operand. MAX_QSEARCH_DEPTH and ORDER_HISTORY_MAX have one use each, both plain comparisons. Nothing resisted.
            
            Single source of truth: the defaults exist once, in an X-macro list, and both the folded constants and the tune variables are generated from it. search_param_info()'s rows are compiled outside every #ifdef CHESSO_TUNE, so each build holds its own live values against the same anchor - live == defaults twice gives live == live member by member. A second case pins each default to the number it ships at by hand.
            
            The LMR trap: search_param_set() calls search_params_rebuild_derived() on every set, not only the two coefficients the table is built from. search_lmr_reduction_probe() exists in the tune build for the test, whose expectations are computed from the formula in the test.
            
            Red observed five times, verbatim in testing.md: drifted tune default (CHECK( 600001 == 600000 ) and nine more), changed list value (CHECK( 120 == 100 )), rebuild call removed (CHECK( 6 == 11 ) and CHECK( 6 == 7 )), UCI dispatch short-circuited (CHECK( 100 == 55 )), MANUAL.md missing the option names (ten 'does not document the option').
            
            End to end on the real binary: RfpMargin 100 / 300 / 2000 gives 292313 / 549374 / 917971 nodes at depth 9, best c3d5 throughout.
            
            Two casts forced by -Wsign-compare in the tune build, src/search.cpp:167 and :348. Both counters small and non-negative; the node counts say the casts changed nothing.
            
            Gate: 13 of 13 fast in 19.56 s on build, 13 of 13 in 19.68 s on build-tune, ./clang-format.sh --check exit 0, moltke --validate clean, tools/plan_prose_check.py 0 flagged. Also ctest --test-dir build-debug -L fast 13 of 13, 478.74 s, INV-2 and INV-4 assertions on. g++ 13.3.0, -j12. testing.md carries four rows. README.md owner-written, no change needed; MANUAL.md and DEV_MANUAL.md both updated.

## Why this exists

`adocs/eval_tuning_strategy.md` Phase A: "Eval parameters exposed as a flat,
addressable array. [...] UCI options for every tunable so external tools can set
them. Prerequisite for everything else; produces no Elo by itself."

This repository has the negative evidence already. `2026-08-16_plan_review-F04`
found that S068's sweep -- the next step's whole evidence base -- was run with
`-DRFP_MARGIN=$margin` and that the method no longer builds:

```
/home/max/ws/chesso/src/search.cpp:38: error: "RFP_MARGIN" redefined [-Werror]
```

So the two pending hand-tunes of a search constant, S068 and S039, currently
cost a source edit and a full rebuild per point measured, and the sweep script
that did it swallows the failure into `BUILD_FAIL` and keeps going. Every later
tuning idea in the strategy document -- and every one of the ten to thirty
parameters SPSA wants per run -- is behind this.

## The set

The constants that are a setting rather than a definition:

| parameter | today | where |
|---|---|---|
| `RFP_MARGIN` | 100 | `src/search.cpp:38` |
| `RFP_MAX_DEPTH` | 6 | `src/search.cpp:39` |
| `RFP_MIN_PLY` | 3 | `src/search.cpp:47` |
| `MAX_QSEARCH_DEPTH` | 8 | `src/search.cpp:29` |
| `ORDER_HISTORY_MAX` | 600000 | `src/search.cpp:20` |
| null-move reduction base and divisor | `2 + (depth / 6)` | `src/search.cpp:370` |
| the two `lmr_table` coefficients | the log fit built at `src/search.cpp:57` | `src/search.cpp` |
| `LAZY_EVAL_MARGIN` | 150 | `src/evaluation.hpp:282` |

`MATE_MAX`, `MATE_MIN`, `DRAW_SCORE`, `MIN`, `MAX` and `GAME_PHASE_MAX` are not
settings and stay as they are. The move-ordering band constants are excluded on
purpose: CLAUDE.md records that the bands clear each other by 100 points and that
tuning them inverts a capture against a killer silently, which is S023's hazard
and not this step's.

## Why a tune build and not a UCI option in the shipping binary

A constant the compiler can fold is not the same code as a variable it must
load, and the difference shows up in a timed match rather than in a node count.
Exposing the set in the release binary would make this step a play-altering
change owing an SPRT for no gain -- the doc's own words are "produces no Elo by
itself".

So: the values are a constant the compiler folds in the default build and become
variables only under `-DCHESSO_TUNE=ON`. The shipping binary is unchanged, which
is what makes the INV-6 neutrality claim provable by identical node counts
rather than argued.

**Correction, written while doing the step.** The sentence above said "the
values stay `constexpr` in the default build". At `e78faed`, the commit this
step started from, they are not `constexpr` — eight are `#define` macros and the
other four are literals inline in the code (`2 + (depth / 6)` at
`src/search.cpp:370` and `0.75` and `2.25` inside the `lmr_table` lambda at
`:57`, both line numbers at `e78faed`). So the default build's half of this step
is a **conversion** to `inline constexpr int`, not a preservation of one, and
that conversion is itself a change that has to be proved neutral rather than
assumed to fold identically. It was: the node counts are in `testing.md`.
`accepts:` is untouched — a spec is not rewritten to fit its result.
The cost is that SPSA measures a binary that is not bit-identical to the one that
ships; S085 carries that and verifies its output with an SPRT of the **shipping**
build carrying the new constants.

## The trap

A default that exists twice can differ in the two builds, and then every tuning
run optimises a starting point the release binary does not have. That is why the
gate asks for a test comparing the two builds' defaults member by member rather
than trusting one definition to be included from the other.

## Cost

No match. One build of each configuration and one `search_bench` comparison.
author:    Maksym Bodnar
