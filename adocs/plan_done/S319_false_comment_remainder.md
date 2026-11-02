id:         S319
goal:       the false comments S308 found and held to its rows are made true or removed, and `depth_1_floor_ms` takes DEC-142's form
accepts:    each row below re-checked against the code at landing, with what the check read, then true or gone; `depth_1_floor_ms` named a golden at its site with what re-derives it and listed with `DEV_MANUAL.md`'s goldens, or a stated reason it is not one; `tests/test_search.cpp`'s GOLDEN block and `DEV_MANUAL.md`'s row agree; comments only but for that, the code identical by S306's form, `No functional change`; a clean reader old against new
touches:    src/chesso.cpp, src/evaluation.cpp, src/search_params.hpp, src/search.cpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_tuner_gradient.cpp, fastchess.sh, DEV_MANUAL.md, adocs/data/S319_identity.txt, adocs/data/README.md
excludes:   shortening comments or removing history, which is parked (DEC-342); `adocs/data/` and `adocs/plan_done/`, but for this step's own identity file and its README line, which `touches:` names
decisions:  DEC-171, DEC-142, DEC-342, DEC-266
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-02 -- all eight rows read false or incomplete against the code at `046c3dd` and each now says what the code does, one line each in this file's table; a ninth item, the dated sibling in `tests/test_eval_trace.cpp`, read true and stayed, its 478 not re-run. `king_shelter_features`: the word still changes g++'s code, and removing it reads zero by DEC-266, which here means unresolved: instructions +0.18 % and +0.20 %, the cycles and wall CIs above 0 and reaching inside the A/A's floor, 16 pairs each. The word stays and the comment says what it rests on. `depth_1_floor_ms` is a golden, named at its site and in `DEV_MANUAL.md`'s table, the command written there being the re-derivation by this step's choice; it reads the lowest `time`, 9 on the i7-8700K, 4 on the M5 from S308 and not re-taken, against the 3. **A DEC-142 finding stays open at both places: no script produces 3, the command only re-reads a run, and the floor's other end has no script.** The GOLDEN block and the row agree, 10467 by the script, the pair unmoved. Comments only by `adocs/data/S319_identity.txt`, `S319-IDENTITY HOLDS`; bench 5482277 in both builds; both fast suites 53 of 53; format and the five prose checks rc 0; 264 mutants pass `validate`, no file under `tools/mutants/` changed. Tier 2 (DEC-141) does not bind: no code moved. `MANUAL.md` checked, no change; `DEV_MANUAL.md` one row. Four things beyond the rows listed in this file and not fixed. A clean reader compared old against new, the coordinator's: its five findings and one beyond the rows were each confirmed here, the five fixed and the sixth listed. That nothing else was lost from any row, and that its independent stripper and old-against-new objects read SAME and byte-equal, are the reader's readings, not re-taken here. Filler (DEC-171), no score, move or line moved.

## Why

Found 2026-11-01 by S308's implementer and its clean reader, and by neither changed, that step being bound to its rows. Filler by DEC-171: none moves a score, a move or a line.

| block | what it says | what was read |
|---|---|---|
| `src/chesso.cpp`, at the first board | "reports 36165 nodes now" | 48840 |
| `src/evaluation.cpp`, `neighbour_files` | "invisible while the weights are zero" | the groups are live |
| `src/evaluation.cpp`, `king_shelter_features` | "`inline` is load-bearing" | rests on a clang timing at zero weights; a timing would answer, else say what it rests on |
| `src/search_params.hpp` and `src/search.cpp`, the history band | "twice the band's own edge" | `QuietHistoryMax` ships 8831, its range top 16384; true of 8192 |
| `fastchess.sh`, the watcher line | "AGENTS.md 12" | no numbered paragraphs since DEC-109 |
| `tests/test_engine.cpp`, `depth_1_floor_ms` | an asserted floor | a golden by `DEV_MANUAL.md`'s definition, no DEC-142 form or row |
| `tests/test_search.cpp`, the GOLDEN block | its readings stop at 7366 | `DEV_MANUAL.md`'s row has 10467 |
| `tests/test_tuner_gradient.cpp`, the fixture's two mentions | "the 2764 positions" | S309 read 2908 rows, 2 x 106 curated + 2696 JSON; S310 edits this file first, so re-read at landing |

## 2026-11-02: the eight, read against the code (Opus 5.5 subagent, coordinated)

Base `046c3dd`, bench 5482277, `build/` on the i7-8700K. `touches:` took the identity file and its README line, S308's two, because the brief names them. **All eight rows read false or incomplete and were changed. A ninth item, not one of the eight, the dated sibling in `tests/test_eval_trace.cpp`, read true and was left; it is 8b below, 8a being row 8 itself.**

| # | block | what the check read | what it became |
|---|---|---|---|
| 1 | `src/chesso.cpp`, at the first board | `position fen`, `go depth 1`: `nodes 48840` over 20 fresh processes. The case the sentence names exists under that name | "there read 48840 nodes at S319, on `046c3dd`": dated, so it stays true |
| 2 | `src/evaluation.cpp`, `neighbour_files` | `passed_pawn_mg` `{9, -5, -2, 21, 38}`, `isolated_backward_mg` `{-6}`, `supported_mg` `{8, 5, 7, 32, 187}`. The sentence entered at `6950be1`, "Add the passed pawn term as bitboard fills, weights at zero" | past tense, dated: "it was invisible while the weights were zero, as they were when the first of them landed". The two counts stay, as measured then |
| 3 | `src/evaluation.cpp`, `king_shelter_features` | below. The word still changes g++'s code, and removing it reads zero by DEC-266 | "was load-bearing, and that was measured", what that rested on, then this step's reading. The word and the code unmoved |
| 4 | `src/search_params.hpp` and `src/search.cpp`, the history band | `X(QUIET_HISTORY_MAX, "QuietHistoryMax", 8831, 1, 32767)`, `X(HP_COEFF, "HistPruneCoeff", 612, 0, 16384)`, 16384 / 8831 = 1.855. An entry holds inside `[-QuietHistoryMax, +QuietHistoryMax]`, so "no entry sits below it" holds at the shipped band and not at a tuned one past 16384 | "twice the 8192 edge it was set against and about 1.86 times the 8831 shipping at S319, so no entry of that band sits below it", and the same two figures, with 16384 named, at the rule in the other file. The move to 8831 at `d0a6667` is what falsified the old sentence, so the figure is dated |
| 5 | `fastchess.sh`, the watcher line | `AGENTS.md` has named rules, none numbered; WATCHERS says "The run prints a terminal marker as its last action, success and failure both" | "AGENTS.md WATCHERS", the two lines rewrapped to three |
| 6 | `tests/test_engine.cpp`, `depth_1_floor_ms` | below: a golden by the definition, with an open finding | `GOLDEN (DEC-142)` at its site with the command, the rule and the margin, and a row in `DEV_MANUAL.md`'s table |
| 7 | `tests/test_search.cpp`, the GOLDEN block | `python3 adocs/data/S192_node_budget.py`, this tree: count 10467, c = 16256, 0.64 of c, inside [7270, 32512], shipping 65024 and 3251, so the pair stays. `DEV_MANUAL.md`'s row reads the same. S296's 7366, S121's 8367, S123's 12104, S125's 10306 and S101's 10467 each read in that step's `_goldens.txt`, under the same script | the block's history carries those five, and that the script read 10467 again here. `DEV_MANUAL.md`'s row was already right and did not change |
| 8a | `tests/test_tuner_gradient.cpp`, the fixture's two mentions | S310 left both. `trace_positions()` and `danger_positions()` build one list: 106 curated rows both ways round, then `all_test_fens()`. `test_eval_trace` "the trace reconstructs evaluate() exactly, clamp included" under `--success` logs `checked := 2916`, which is that list, 2 x 106 + 2696 = 2908, plus 2 x 4 pinned truncation rows | "The positions of tests/test_eval_trace.cpp's all_positions(), 2908 at S319", since two of that file's cases run over 2916; "over these positions" at the second |
| 8b | `tests/test_eval_trace.cpp`, "478 of the 2764" | **True as dated, left.** It says "At f57b92f's weights and margin". It entered at `bee84d8`, S262's landing, whose parent is `f57b92f`. `git show bee84d8:tests/test_eval_positions.hpp` has 34 curated rows, so 2 x 34 + 2696 = 2764, and S039's step file repeats "478 of 2764 at `f57b92f`". **478 itself was not re-run**: that needs a tree at that commit and this step takes none. Today's case logs `clamped_high := 246`, `clamped_low := 246` over 2916, which is another reading and contradicts nothing | no edit |

### Row 3: what `inline` rests on

**What it rested on.** clang, per the comment; zero weights, per `690de1f`, "Add king safety as nine linear features, weights at zero"; before S118, when stage two ran it every time. None of the three holds now: g++ 13.3 is the reference (DEC-049), `king_safety_mg` is non-zero, and at `if (shelter->king[colour] != tag)` a Release search reaches it only on a slot miss. The pure forms hand it an empty slot, so they miss every time.

**The word still changes the code.** `evaluation.cpp` compiled by its own Release line in the scratch directory, `inline` taken off a copy of the one line: shipped, `nm -C` lists no `king_shelter_features` and the object is 45760 bytes, byte-equal to `build/`'s; without the word, `t king_shelter_features`, 45120 bytes, four calls, two each in `evaluate_mobility_and_king_safety<true>` and `expensive_score`, into which `<false>` folds. So stating what it rests on could not end at "not re-measured"; a timing could answer, and was taken.

**The timing, DEC-266's form.** Both sides padded; `base` is `build/src/chesso` at this HEAD, `copy` its bytes, `noinline` the same archive with the one object replaced, linked by `chesso`'s own line, which reproduces `base` byte for byte from the shipped archive. S270's method as `adocs/data/S118_cost.sh`'s `pairs()` runs it: 16 interleaved pairs per comparison, the order swapped every pair, `build/tools/perf_counters` on `bench 16` and on pos11, hyperfine wall on pos11. Read by `adocs/data/S135_cost.py`'s `counters` and `wall` and `adocs/data/S118_cost.py`'s `verdict`, so the CI and the rule are theirs. 2026-11-02, 04:04:34 to 04:30:38, governor `performance`, load 0.27 before and 1.07 after. Nothing else ran but this agent's reads and comment edits, no compile. Every counted run read the same totals, `bench 16` 14931387 nodes and pos11 39180277 over 300 positions, on both comparisons.

| comparison, + is slower | `bench 16` instructions | `bench 16` cycles | pos11 instructions | pos11 cycles | pos11 wall |
|---|---|---|---|---|---|
| A/A | -0.000 % | -0.145 %, CI -0.336 to +0.046 | +0.000 % | +0.096 %, CI -0.040 to +0.232 | -0.023 %, CI -0.253 to +0.207 |
| without the word | **+0.178 %**, CI +0.178 to +0.178 | +0.270 %, CI +0.157 to +0.383 | **+0.199 %**, CI +0.198 to +0.199 | +0.257 %, CI +0.200 to +0.313 | +0.231 %, CI +0.097 to +0.365 |

A/A floor: `bench 16` cycles 0.336 %, pos11 cycles 0.232 %, wall 0.253 %. **Verdict: zero, DEC-266's record for unresolved.** No cycles or wall CI lies wholly beyond its floor. All three lie wholly above zero, though, and pos11's cycles read +0.257 % against its 0.232 % floor, so the reading does not show no cost: it shows the word buying those instructions and no time this machine resolves. The size is under what DEC-266 says this machine resolves. The word stays: this step changes no code. Outputs: `/home/max/ws/chesso/.tuning/coord/S319/inline_cost/`, with the scripts beside it.

### Row 6: `depth_1_floor_ms` is a golden, with one end unscripted

**Why a golden.** `reported_ms >= depth_1_floor_ms` asserts it, and it was read off a measurement: `9075bf8`'s site says "13.8 ms on this machine" and "asserts 3 ms of it, which is where a four times faster machine would still leave the case separating". `DEV_MANUAL.md`'s definition is exactly that. S308 took the mark off the figure no assertion reads, and this is the number that one is judged by.

**What re-derives it, and what that means.** No tracked script exists: `.tuning/coord/S247/depth1_time.py` was gitignored and the main tree here does not hold it. Writing none was this step's choice: the command at the site and in the row is the re-derivation, as `test_uci_surface.cpp`'s option-line count's is. It reads the upper end, machine by machine. Run on `046c3dd`'s binary, 200 fresh processes, one-minute load 0.65 before and 0.48 after, 40 s:

```
    158  time 9
     40  time 10
      2  time 11
```

**What it does not do.** It does not produce 3. The value stays a choice under a reading, and the command says whether the choice still holds: at or above it, it stands; under it, a heavier board and not a smaller floor, the case's own rule, which S022's second verdict applied. Margin 9 to 3 here. 4 to 3 on the MacBook Air M5 at `1c86c1a`, from S308's step file, on 49231 nodes where this tree reads 48840, and not re-taken.

**The finding, per DEC-142, open, in two parts.** First, this golden is not re-derived as that rule asks: no script produces 3, the command only re-reads a run, and the rule says when 3 stands, not what it should be. Second, the floor's other end has no script. Half the floor is `prompt_stop_us`, the clock the stop half reads a send against; S242's reading of that send, median 63 us and 299 of 300 under 1500 us beside a match, came from a temporary MESSAGE in a diagnostic build. It is not the binding end, 24 times clear at the median, but re-reading it means editing the case. Both parts named at the site and in the row, not closed here.

### Proof that it is comments only

`adocs/data/S319_identity.txt`, `S319-IDENTITY HOLDS`: seven C++ files byte-equal under `g++ -fpreprocessed -dD -E -P`, its control reproducing S308's two hashes; `fastchess.sh` only comment lines by script, `bash -n` rc 0; `DEV_MANUAL.md` one added row. Beside the form: every member of `libchesso_engine.a` byte-equal to `046c3dd`'s, `chesso.cpp.o` when the old source is compiled against the same generated build header, since `id name` now reads `046c3dd-dirty`.

Anchors: 264 mutants over 34 files pass `tools/mutation_check.py`'s `load_mutants` and `validate` at the worktree, and no file under `tools/mutants/` changed.

**Tier 2 (DEC-141) does not bind, and `tools/gate_extra.sh` was not run.** The diff touches `src/search.cpp` and `src/evaluation.cpp` in comments and nothing in `make_move`, `unmake_move` or the generator, the stripped sources are identical, and S306 and S308 are the precedent. A comment edit in `fastchess.sh` moves no harness behaviour, so DEC-143's A/A does not read it as a change, S306's reading.

**Manuals.** `DEV_MANUAL.md`: the one row; the budget's row already agreed. `MANUAL.md`: its `HistPruneCoeff` row reads 612 and 0 to 16384 and says nothing about twice; it holds none of these figures; no change.

### Found and not fixed: this step is its rows

- `DEV_MANUAL.md`, "The tests" under the tuner: "the 2696 positions of the JSON suites, 478 of which bind the clamp" is undated and is row 8b's reading of `f57b92f`. Today's case logs 246 and 246 over 2916. How many of the 2696 alone bind today was not read.
- `src/search.cpp`, the block row 4 edits: "S222 lands that table", "The step's one SPRT already prices" and "S222's lane fits HistPruneCoeff where it stands ... and its SPRT re-prices the rule" are in the present tense of a closed step. Stale tense, not a false claim; left, row 4 being its one clause.
- Row 6's finding, both parts: no script produces 3, and the send has no tracked script.
- `tools/spsa_driver.py`, the snapshot docstring: cites lines 141 to 147 of `fastchess.sh` by number for the incident. At `046c3dd` those lines are the comment on the `git` wrapper, and "That has already happened once" is in the comment above `snapshot=`. Wrong at base, and line numbers decay. The clean reader's finding, confirmed here; `rating.sh` had the same class, S308's row 7.
- `adocs/specs.md`: nothing false found under these rows' terms. `surface_guard`, which S308 listed, is gone from it.

### The `accepts`, clause by clause

- Each row re-checked with what the check read, then true or gone: the table. Held; 8b read true and stayed.
- `depth_1_floor_ms` named a golden at its site with what re-derives it and listed: yes, `grep -rn 'GOLDEN (DEC-142)' tests/` finds it. Held, with both parts of the finding above stated.
- The GOLDEN block and `DEV_MANUAL.md`'s row agree: 10467 by the script, and every count the row gives after 7366 is now in the block. Held.
- Comments only, identical by S306's form, `No functional change`: the identity file and bench below. The message is the coordinator's.
- A clean reader old against new: below, five findings, each confirmed and fixed.

### Gates

Serial, `CLANG_FORMAT_MAJOR=22`, 2026-11-02 05:07 to 05:11, ends `S319-FINAL-DONE`, every stage below in `/home/max/ws/chesso/.tuning/coord/S319/final_gate.log`. Both builds without a warning. Fast suites 53 of 53 in `build` and 53 of 53 in `build-tune`. `./clang-format.sh --check` rc 0. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, `--gate`, `--prose`, each rc 0. Bench 5482277 in both builds. `validate` over 264 mutants, no file under `tools/mutants/` changed. `S319-IDENTITY HOLDS`. After it, every member of `libchesso_engine.a` byte-equal to `046c3dd`'s again. The five prose checks ran once more over this stamp, the tail of the same log, and `tools/gate.sh --message` over the staged tree is `gate_message.log` beside it.

## 2026-11-02: the clean reader (the coordinator's)

A clean reader compared old against new and moved no code. It found five things to fix and one false citation beyond the rows. Each was checked here against the code and the evidence before anything changed, and all six stand.

| # | what it found | what confirmed it here | what it became |
|---|---|---|---|
| 1 | "shown to buy those instructions and no time" is false, and "each CI" covers five readings against three floors | `inline_cost/reading.txt`: cycles CI +0.157 to +0.383 and +0.200 to +0.313, wall +0.097 to +0.365, all above zero, pos11's cycles +0.257 against a 0.232 floor; the instruction CIs lie beyond their A/A | "no time this machine resolves", and "those three CIs above 0 and each reaching inside" the floor. This file's verdict says the same |
| 2 | "the 8831 that ships" is undated, in two files | `git show d0a6667`: `QuietHistoryMax` 8192 to 8831, the move that falsified the old sentence | "shipping at S319" in both, and "of that band" for "of the shipped band" |
| 3 | the stamp's and the table headline's "seven" are wrong | the diff edits all eight rows, 8a among them; `tests/test_eval_trace.cpp` is not in it | "all eight", 8b named a ninth item outside them, and the commit subject carries no count |
| 4 | "excludes take no new file under `adocs/data/`" is no reason, in a step adding one there | this file said it beside a `touches:` naming two paths there. The brief did bar a script there, but one under `tools/` was open by widening `touches:`, so it was a choice | stated as the choice it was, the command being the re-derivation; `excludes:` now names the two exceptions `touches:` holds |
| 5 | the open finding names only S242's send | the command prints a histogram of `time`, and nothing computes 3 | the finding has two parts at the site, in `DEV_MANUAL.md`'s row and above |
| + | `tools/spsa_driver.py` cites the wrong lines of `fastchess.sh` | `git show 046c3dd:fastchess.sh`, read as listed above | listed under "Found and not fixed" |

**The reader's readings, not re-taken here and relayed by the coordinator:** nothing else was lost from any row, and its own independent stripper and old-against-new objects read SAME and byte-equal. This step's own are the identity file and the object comparison under Gates, and they agree with it.

**Also this round.** `final_gate.log` held no bench, prose, format or `validate` output though this file pointed at it for those, so that log is now `final_gate_first.log` and the one under Gates holds every stage. The first run of it went red in both builds, `test_plan_citation_freshness`, and at `--citations`, on a line-form citation of `fastchess.sh` this round had written into the list above. The sentence was reworded and no test moved.
