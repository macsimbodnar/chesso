id:         S214
goal:       three measurement tools stop failing silently -- `analyse_game.py` refuses to return a score it never read, `spsa_driver.py check` proves every axis reaches the search, and a fit's provenance stamp records every flag that selects which vector is emitted -- and, by DEC-184, the two copies of the completion gate are checked against each other and the tuner's last-group assertion names what it detects
accepts:    `tools/analyse_game.py` raises, or exits non-zero with the position named, when no `info` line carrying both a score and a `pv` arrived before `bestmove`, and it ignores `lowerbound`/`upperbound` lines for the final score, with a test or a recorded reproduction against a stubbed engine that answers `bestmove` alone; `tools/spsa_driver.py check` probes node-count reachability for **every** parameter in the config, not the hardcoded `RfpMargin` alone, and a parameter whose two probe values search identically fails the check by name; `tools/tuner.cpp`'s provenance stamp carries `--epochs`, `--report`, `--patience` and `--threads` beside the commit, digest, K, seed, lr and split, because `--report` and `--patience` select which vector is emitted; `DEV_MANUAL.md`'s tuner and SPSA sections state the new behaviour where they describe the old; **DEC-184** `tools/plan_prose_check.py` gains a `--gate` mode that reads the completion command from `AGENTS.md`'s TESTS rule and from `DEV_MANUAL.md`'s test section and exits non-zero when they differ, registered in the fast suite in `tests/CMakeLists.txt` beside `test_plan_params`, observed red against a one-character edit to either copy before green; `tests/test_tuner_groups.cpp`'s precondition assertion on `TEMPO_EG_BASE + TEMPO_COUNT == PARAM_COUNT` carries a message naming what it detects -- a parameter block appended after `tempo` and given no group of its own is silently covered by `tempo` -- so a failure names the right thing (S041's parked finding, accepted as to structure)
touches:    tools/analyse_game.py, tools/spsa_driver.py, tools/tuner.cpp, tools/plan_prose_check.py, tests/, tests/CMakeLists.txt, tests/test_tuner_groups.cpp, DEV_MANUAL.md
excludes:   the tuner's clamp treatment in its gradient (F24, folded into S126's file), the regularisation question (DEC-170 records the ruling), the corpus phase column (F36, folded into S134's file), `eval_spread`'s candidate margins (F25, folded into S039's file)
decisions:  DEC-170, DEC-171, DEC-184
closes:     2026-09-10_adversarial-F28, 2026-09-10_adversarial-F29, 2026-09-10_adversarial-F35
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-12 18:20, machine idle
done:       2026-09-12 18:44. All five accepts clauses hold, no `src/` change, no run. `tools/analyse_game.py` raises `EngineError` and exits 1 naming the ply, the move and the FEN when no `info` line carrying both a score and a `pv` arrived before `bestmove`, when a `bestmove` is bare, and when the engine closes its output before `uciok` or `bestmove`; `lowerbound` and `upperbound` lines are read past, so a bound cannot become the reported score. `tools/spsa_driver.py check` probes every parameter in the config down a four-rung ladder (`probe_axis`) and fails by name when both bounds search identically: 28 axes, 27 reachable, and `TmHardPercent` named as the one no node-count probe reaches -- which is a property of the parameter and is written up rather than papered over. `tools/tuner.cpp`'s stamp carries `--epochs`, `--report`, `--patience` and `--threads` on a new `// run` line, observed on a real 2000-row fit at non-default values. `tools/plan_prose_check.py --gate` holds `AGENTS.md`'s TESTS command against `DEV_MANUAL.md`'s, registered as `test_plan_gate`; `tests/test_tuner_groups.cpp`'s `TEMPO_EG_BASE + TEMPO_COUNT` precondition is now a `REQUIRE_MESSAGE` naming the event. Red observed before green in all three places: the four-case `tests/test_analyse_game.py` failed three ways against HEAD's tool (the silent case printing `1. e4 e2e4 0 0 0`, a dead draw); `test_plan_gate` failed on a one-character `-j8` -> `-j9` edit to AGENTS.md and again to DEV_MANUAL.md, green after each revert, both files byte-restored; the tuner-groups message was observed by breaking its condition to `+ 1 ==` and reverting. Gate green in both builds -- `build` 36/36 and `build-tune` 36/36 fast, `./clang-format.sh --check` clean under `CLANG_FORMAT_MAJOR=22` (DEC-146). `DEV_MANUAL.md` updated in four places: the `analyse_game` section, the SPSA `check` paragraph, the emitted-table provenance section and the `plan_prose_check` mode list and its new `--gate` paragraph. `MANUAL.md` checked and needs no change -- it documents the UCI surface, nothing here touches `src/`, and its two "tuner" mentions are about the tune build and option refusals. `README.md` untouched, human-owned. One thing found while writing the doc and stated rather than assumed: `--threads` is **not** provably result-neutral, because `error_range()` and `gradient()` stride by the thread count and sum partials in thread order; the emitted constants and errors were identical at 1, 4 and 8 threads on a 2000-row fit and that is not a proof. The step file stays in `plan_current/`; the coordinator moves it and commits. By an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199) -- 2026-09-12 19:10, after the fast check. The fast check raised four findings over this step's own diff and all four are repaired. **1, the one-sided jitter guard** (`tools/spsa_driver.py` `probe_axis`): a clock rung re-ran the low bound only, so one unconfirmed high sample decided the verdict and a bimodal dead axis passed as reachable with probability `p(1-p)` per axis -- 21 % at a 70/30 split, 25 % at worst -- over the nine `Tm*` axes only that rung reaches. Both bounds are now re-measured and every sample at each has to agree: `UNSTABLE_SAMPLES = 5`, because n agreeing samples per bound leaves `2(p(1-p))^n` and two would only halve a number that needed two orders of magnitude (12.5 % per axis at worst, 70 % over the nine); five leaves 0.20 % per axis and under 2 % over the nine. Sampling bails at the first disagreement, so the cost lands on axes that do repeat. **2, the failure message** (`unreached`): a rung rejected for jitter fell out of the loop and was reported as "both searched N nodes ... and every probe before it agreed", both clauses false; the two failure classes are now named apart and every rung tried is listed with what it measured. **3, `tests/test_analyse_game.py`** leaked four `mkdtemp` directories with an executable stub into `/tmp` per run -- 48 had accumulated -- now `addCleanup(shutil.rmtree, ..., ignore_errors=True)` as `tests/test_mutation_check.py` does; the 48 were removed. **4, collapsed bounds**: `uci_value` clamps and rounds, so `min 1.2 / max 1.4` is 1 and 1, both probes were the same probe and the axis was blamed by name; `check` now refuses it as a config fault before probing, and the bounds comparison uses `uci_value` where it used `int()`, so one conversion decides what is sent and what is quoted. Also fixed in passing, surfaced by the new tests: `probe_nodes` never closed its pipes, which the extra samples would have multiplied. **The tests.** `tests/test_spsa_probe.py`, 12 cases, ~1 s, registered `fast` beside `test_analyse_game`; the probe path had no test at all. The engine is a stub whose `info ... nodes N` is a formula over the option, the `go` command and a `go`-count persisted across processes, which is the only way to make a jittery clock reproducible: cases are a live axis, a dead one, one only a clock rung reaches, and a dead one whose clock count alternates. Red observed against the pre-fix driver before each fix. Finding 1: `probe_axis -> (True, 'a 1 s clock, sudden death with increment', 0, 7000, 1000, 9000)` and `check -> []` on a stub that never reads the option -- a dead axis reported reachable, no problem raised. Finding 2: `AssertionError: 'depth 9, midgame' not found in 'Axis 0 and 1000 both searched 4242 nodes, the last probe being a 1 s clock ..., and every probe before it agreed'`. Finding 4: `AssertionError: 'collapse' not found in 'Axis 1 and 1 both searched 1001 nodes, ... the axis would random-walk and read as tuned'`, and for the truncation half `'collapse' not found in "Axis: config bounds [0, 1] against the binary's [1, 1] ..."` on bounds `[0.6, 1.4]` -- a value quoted that nothing sends. Finding 3 observed by count: `/tmp/chesso-analyse-game.*` 44 before a run and 48 after, 48 and 48 after the fix. **Re-derived `check`, tune build, 28 axes, machine not idle** (`cosmic-term` 3.3 %, a live Claude session): **15.4 s, 27 reachable** -- the 8.1 s the stamp above first quoted was the one-sided guard's cost and is superseded. 19 axes separate at rung 1 (`RfpMargin` 0 -> 54843 nodes, 2000 -> 569950 at depth 9); eight of the nine `Tm*` separate on the clock rung, now on ten agreeing samples rather than three; `TmHardPercent` is identical at all four rungs and is reported with the trail, `100` and `1000` both searching 121515 / 949446 / 489570 / 638719. `tools/spsa_dryrun.json` 0.2 s and `tools/spsa_s085.json` 0.7 s, both unchanged -- neither reaches the clock rung, which is where the new samples are spent. **One consequence, stated rather than hidden, and it is load-dependent.** The clock rung is not a reliable instrument on a shared machine and the old guard hid that by measuring the high bound once. Ten runs early in the session, load ~0.3: six named `TmHardPercent` alone, four named one further `Tm*` axis *not repeatable*. Ten runs later, load 0.87 with a parallel `codex` session live: **one** clean, the rest naming one to three of `TmSuddenDeathPercent`, `TmStabilityPercent`, `TmScaleMinPercent`. The guard fails closed -- never *reachable* -- and says to re-run on an idle machine, so no verdict is wrong; the tool is simply unusable as a gate while an agent session runs. Measured why, five samples at each bound of all nine `Tm*` axes: the clock counts quantise to a handful of iteration boundaries and the jitter is between **the same two values that also appear at the other bound** -- `TmStabilityMax` gave `{638719, 949446}` at its low bound and `{638719}` at its high, `TmScaleMinPercent` the mirror image. A looser criterion is therefore not available from this rung: requiring disjoint sample bands instead of exact repetition separates the same 6 of 9 axes on that data, because the bands overlap wherever the repetition fails. Making `Tm*` reachability decidable needs a different instrument -- a longer clock that does not straddle an iteration boundary, or a pinned core -- which is a step and not this repair. Gate green in both builds: `build` 37/37 fast, `build-tune` 37/37 fast, `./clang-format.sh --check` exit 0 under `CLANG_FORMAT_MAJOR=22`. `DEV_MANUAL.md` updated in two further places: the SPSA `check` paragraph (the two-sided confirmation and its arithmetic, the collapse refusal, 15.4 s and 27, and the not-repeatable verdict a reader will meet) and "What decides whether it worked" (a paragraph for `tests/test_spsa_probe.py`). `MANUAL.md` re-checked, still no change -- no `src/` file moved. By a second Opus 5 subagent briefed by the coordinator

## Why this exists

Three low findings of `2026-09-10_adversarial` in the tools the rules lean on,
none touching `src/`, so the step is filler beside any run (DEC-171, DEC-172).

- **F28.** `tools/analyse_game.py` initialises `score, best = 0, "-"` and
  returns them if no `info` line carrying both a score and a `pv` arrives; it
  also accepts bound lines. It is the tool `CLAUDE.md` mandates *because* agent
  chess judgement is banned, so a silent 0.00 is the DEC-023 failure arriving
  through the instrument that exists to prevent it.
- **F29.** `spsa_driver.py check` validates presence and bounds for every
  parameter and then probes reachability for one, `RfpMargin`. An axis wired
  to a variable nothing reads would random-walk and read as tuned. S127's
  full-set run is what this guards.
- **F35.** The provenance line `tools/tuner.cpp` emits omits four flags, two
  of which decide which vector the run emits; a fit is therefore not
  reproducible from its own stamp.

## Cost

Agent work, two to three hours; no run, no `src/` change.

## Evidence, 2026-09-12

**F28, the refusal.** `tools/analyse_game.py` `EngineError`, raised from
`evaluate` on three paths -- no scoring `info` line before `bestmove`, a bare
`bestmove`, and an engine whose output closes early -- and from the constructor
when the handshake never answers `uciok`. The old handshake loop spun forever on
a dead engine's empty reads; it now reports instead, which is the same class and
was fixed in place. `main` catches, kills the engine (it may be mid-search and
not reading its stdin) and returns 1 with the ply, the SAN and the FEN. Bound
lines are skipped by reading the token directly after the value, which is where
UCI puts `lowerbound`/`upperbound`.

`tests/test_analyse_game.py`, four cases, in the fast suite as
`test_analyse_game`. The engine is a `/bin/sh` stub written per case into a
temporary directory -- the device `tests/test_mutation_check.py` uses -- because
the three answers under test are ones no correct engine produces on demand.
Observed red against HEAD's tool by stashing the tool alone:

```
FAIL: test_no_info_line_is_refused_and_names_the_position
AssertionError: 0 == 0 :   move  played    best   before    after    cost
   1.       e4    e2e4        0        0       0
FAIL: test_bound_lines_alone_are_not_a_score
AssertionError: 0 == 0 :   move  played    best   before    after    cost
FAIL: test_a_bound_does_not_overwrite_the_score_before_it
- ['999', '-999', '1998']
+ ['12', '-12', '24']
```

The first line is F28 itself: an engine that says nothing produced a dead draw
and a cost table. The control case, a stub that answers properly, passed on HEAD
and passes now, so the refusals are not a tool that refuses everything. Green
after: `Ran 4 tests in 0.109s / OK`. Also smoked against the real engine --
`tools/analyse_game.py` over two positions with `--engine build/src/chesso
--depth 8` exits 0.

**F29, every axis.** `tools/spsa_driver.py` `probe_axis` searches an axis at
both of its own bounds -- the widest pair the run will ever send -- down a
ladder that stops at the first rung to separate the node counts: depth 9 on the
midgame position, depth 13 on it, depth 13 on `tools/search_bench.py`'s tactical
position, then `go wtime 1000 btime 1000 winc 200 binc 200`. `probe_nodes` now
takes a whole `go` command instead of a depth. An axis that fails the bounds
comparison is not probed, because a probe at a value the binary refuses proves
nothing.

The clock rung is there because **nothing under `go depth` consults a clock**,
so the nine `Tm*` parameters are invisible to a fixed-depth probe however deep
it goes -- measured, all nine identical at depth 9 at min and max. A clock
probe's node count is not a function of the options alone, so a separation there
is re-run at the low value and believed only when the two low runs agree
exactly; without that, jitter alone would report a dead axis as reachable, which
is the exact failure this check exists to catch. In practice the counts quantise
to whole iterations and repeated runs were identical.

Measured on the tune build, 2026-09-12, over a config holding every spin option
but `Hash` and `Threads` (`Threads` has min == max and cannot be a config
parameter at all): **28 axes, 8.1 s, 27 reachable**, byte-identical verdict on
three consecutive runs. 19 separate at the first rung; eight of the nine `Tm*`
ones separate on the clock. `tools/spsa_dryrun.json` takes 0.2 s and
`tools/spsa_s085.json` 0.7 s -- the latter now also reports its stale
`RfpMinPly` lower bound, which is the pre-existing bounds check and not this
change.

**The one axis nothing reaches, and why it is not a false alarm.**
`TmHardPercent` searched identically at 100 and 1000 under every probe tried:
depth 9, 13, the tactical position, `go wtime 400 movestogo 1`, `go wtime 1000
winc 200`, `go wtime 2000 movestogo 4`, `go wtime 3000 movestogo 3` and `go
wtime 5000 movestogo 5` on all three positions. That is arithmetic and not bad
luck. `src/chesso.cpp` `compute_search_time_budget` sets soft to
`TmSoftPercent` percent of the base allocation, 60 % as shipped;
`search_time_scale_percent` tops out at 150 % with `TmStabilityPercent` and
`TmFallingPercent` as they compile, so the scaled soft limit never exceeds
0.9 of the base while the hard limit at the probe's low value of 100 -- the
lowest that range allows, and well below what ships -- is a full 1.0 of it. The clamp `soft = min(soft, hard)` therefore cannot bind, and the hard timer
only fires when one iteration overruns its own start by more than 11 %.
Separating that axis needs a second option held at a non-default value, which
would be exactly the name-keyed hardcoding F29 objects to -- so the check fails
by name and says what it tried. What it is telling a reader is that tuning that
axis alone is tuning something the search almost never consults.

**F35, the stamp.** `tools/tuner.cpp` `write_tables` gained one `// run` line
and two continuation lines. Observed on a real fit, 2000 rows of
`.tuning/smoke_v2.tsv` at `--epochs 20 --report 5 --patience 3 --threads 4`:

```
// seed       1, lr 1.000, validation split 0.10
// run        20 epochs max, report every 5, patience 3 reports, 4 threads
//            the emitted vector is the best reported epoch, so --report and
//            --patience choose it as much as the data does
```

Nothing in `tests/` or `adocs/data/` reads the stamp's format, so DEC-142 has
no golden to re-derive here -- checked by grep over both. The tables under
`adocs/data/S075_fits/` carry the pre-S214 header and are evidence, byte for
byte what the tool wrote; they are not regenerated.

**DEC-184 item 1, `--gate`.** `tools/plan_prose_check.py` `check_gate`, with
`_gate_agents` reading the backticked spans of the `- TESTS:` bullet and
`_gate_manual` reading fenced lines. Both copies are found by content -- a
command holding **both** `ctest --test-dir build -L fast` and
`clang-format.sh --check`, two anchors because `DEV_MANUAL.md` also prints the
first on its own as the everyday invocation and that line is not the gate.
Exactly one copy each or the run fails; a copy wrapped over two lines is
reported missing rather than rejoined. Registered as `test_plan_gate` beside
`test_plan_params`, 0.03 s.

Red observed twice, one character each time, and the file restored after each:

```
# AGENTS.md line 133, -j8 -> -j9
1/1 Test #33: test_plan_gate ...***Failed    0.03 sec
GATE   AGENTS.md's TESTS command and DEV_MANUAL.md's test section give different completion gates:
         AGENTS.md      cmake --build build -j9 && ctest --test-dir build -L fast ...
         DEV_MANUAL.md  cmake --build build -j8 && ctest --test-dir build -L fast ...

# DEV_MANUAL.md line 640, one space inserted before --check
GATE   DEV_MANUAL.md's test section holds 0 copies of the completion command, not one

# DEV_MANUAL.md line 640, -j8 -> -j9
GATE   AGENTS.md's TESTS command and DEV_MANUAL.md's test section give different completion gates:
```

Green after each revert, and `git diff --stat AGENTS.md DEV_MANUAL.md` empty at
the end of that sequence. Both failure classes -- the copies differing and a
copy going missing -- were observed, and from both documents.

**DEC-184 item 4, the message.** `tests/test_tuner_groups.cpp` "the group list
and the parameter vector are what the code says" now uses `REQUIRE_MESSAGE`.
Observed by temporarily breaking the condition to `+ 1 ==`:

```
FATAL ERROR: REQUIRE( eval_model::TEMPO_EG_BASE + eval_model::TEMPO_COUNT + 1 == eval_model::PARAM_COUNT ) is NOT correct!
  values: REQUIRE( 828 == 827 )
  logged: a parameter block was appended after `tempo` and given no group of its own, so `tempo` -- the last group, whose range runs to PARAM_COUNT -- silently covers it: TEMPO_EG_BASE 826 + TEMPO_COUNT 1 = 827 against PARAM_COUNT 827. The partition cases below cannot see this and will all pass.
```

reverted and green. The structure stays as S041 left it and DEC-184 accepted:
the last `--only` group still runs to `PARAM_COUNT`, and this assertion is the
only thing between an appended block and a fit that hands its weights back
unchanged.

**Documents.** `DEV_MANUAL.md` in four places: "Analyse a game" gains the
refusal paragraph; the SPSA `check` paragraph is rewritten around the ladder,
with today's figures replacing the S137-era `RfpMargin` pair it quoted (that
older measurement is still in this file where it is dated, at the tune-build
paragraph); "What an emitted table says it came from" gains the flag lines; and
the `plan_prose_check` list goes from four checks to five with a `--gate`
paragraph beside the other four. Every statement about a flag was traced to the
code that produces it, and one claim was corrected in the writing: `--threads`
is not provably result-neutral, since `tools/tuner_model.hpp` `error_range` and
`tools/tuner_model.hpp` `gradient` stride their range by the thread count and
sum the partials in thread order, so the summation order is a function of it.
Measured at 1, 4 and 8 threads on a 2000-row fit at 200 epochs: byte-identical
constants and identical errors to six decimals, which the integer rounding
absorbs and which is not a proof over 11 million rows. Both the manual and the
comment in `tools/tuner.cpp` say that rather than the neat claim.

`MANUAL.md`: checked, no change. It documents the UCI surface, this step changes
no `src/` file, and its two mentions of "tuner" are about the tune build and
about option refusals. `README.md` is the owner's and was not opened for
writing.
