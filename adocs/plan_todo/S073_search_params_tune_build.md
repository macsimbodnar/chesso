id:         S073
goal:       the search constants become one addressable parameter set, settable in a tune build and unchanged in the shipping one
accepts:    every constant in the set keeps its value today, so `tools/search_bench.py` reports identical node counts and identical best moves against the commit before this one and no SPRT is owed (INV-6); the shipping build exposes no new UCI option and `test_uci_surface` is unchanged by it; a build configured `-DCHESSO_TUNE=ON` accepts `setoption name <param> value <n>` for every member of the set and `tools/search_bench.py` on that build, with no `setoption` sent, still reports the same counts as the shipping build; a test asserts the two builds' default parameter values are equal member by member, so a default that drifts in one build fails rather than ships; `MANUAL.md` and `DEV_MANUAL.md` state what the tune build is, that it is not the release binary, and how to set a parameter
touches:    src/search.cpp, src/evaluation.hpp, src/chesso.cpp, CMakeLists.txt, tests/, MANUAL.md, DEV_MANUAL.md
excludes:   changing any parameter's value, which is S068's, S039's and S085's; SPSA itself, which is S084; the evaluation weights, which the tuner already owns through `tools/eval_model.hpp`; adding a parameter that does not exist today
decisions:  DEC-019
closes:
blocks:
paused_by:
done:

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

So: the values stay `constexpr` in the default build and become variables only
under `-DCHESSO_TUNE=ON`. The shipping binary is unchanged, which is what makes
the INV-6 neutrality claim provable by identical node counts rather than argued.
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
