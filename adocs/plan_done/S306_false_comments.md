id:         S306
goal:       the eleven comments the 2026-10-31 study read as false, and one S305 found, are made true or removed
accepts:    each of the twelve below is re-checked against the code at landing, with what the check read, and then says what the code does or is gone; `tests/test_engine.cpp`'s figure is re-taken by its command (DEC-142), or its "Golden" mark goes with the reason stated; every commit touching `src/` reads `No functional change` and the gate passes it; every anchor in `tools/mutants/` is still found once; a clean reader compares old against new
touches:    src/search.cpp, src/search_params.hpp, src/evaluation.cpp, src/data_structures.hpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp, tools/timer_race_stress.py, fastchess.sh, tests/CMakeLists.txt, adocs/data/S306_identity.txt, adocs/data/README.md
excludes:   shortening comments or removing history, which waits for the owner's decision on the study (`adocs/status.md`); `adocs/data/` and `adocs/plan_done/`, which are evidence
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 02:10 -- all twelve read false against the code and each now says what the code does: one line each in the table above; rows 4, 6, 8 and 9 dated rather than dropped, row 8's instruction removed because its GOLDEN block replaced it. `tests/test_engine.cpp`'s figure is no golden, no assertion reading it, and its mark went with the reason stated; re-taken anyway by its command, 35516352 nodes and about 6.8 s, 6746 to 6776 ms with the timer disarmed against 1.9 to 2.1 ms as shipped, the 402 ms bound unmoved and about 17 times below. Comments only: `adocs/data/S306_identity.txt` `S306-IDENTITY HOLDS`; `No functional change`, bench 5707449 in Release, `tools/gate.sh --message` green, 50 of 50 in both builds, no new warning, format clean, the five `tools/plan_prose_check.py` modes clean, 262 of 262 anchors by `tools/mutation_check.py`'s `validate`. Tier 2 (DEC-141) does not bind and `tools/gate_extra.sh` was not run: comments only, `.tuning/coord/S123/brief_close.md` item 7. `MANUAL.md` and `DEV_MANUAL.md` checked, no change. Seven sibling comments and one test gap found and not fixed, listed above. A clean reader compared old against new (the coordinator's, 2026-11-01) and found two errors in these rewrites, both fixed before the commit: `data_structures.hpp`'s first rewrite made material tapered, and `test_engine.cpp` quoted the `time` column as `bestmove`.

## Why

A comment study on `5954c12` read a stratified sample of 140 live blocks against the code, and 11
held a claim the code no longer bears out, so this list is a floor. One agent read them, and the
coordinator confirmed only the first row's switch and that the texts exist, so each is checked
again here. Each row's quoted words find its block.

| block | what it says | what the study read |
|---|---|---|
| `src/search.cpp` | "the second of them ships switched off" | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)` ships on |
| `tests/test_engine.cpp` | "Golden (DEC-142)": its depth-1 iteration is 25933707 nodes, about 5.9 s | one run answered `nodes 35516352`, 8962 ms; no test holds the figure; the bound still separates |
| `src/search_params.hpp` | "S098 owns moving the rule onto the sum" | S098 is closed and the rule still reads the raw entry |
| `src/search_params.hpp` | "the build prints 4493659" | bench is 5707449 |
| `src/search_params.hpp` | cites `evaluation.cpp` lines 33 to 37 | `ORDER_COUNTER` is on line 39 |
| `src/evaluation.cpp` | "the zero weights still fold" | g++ is the reference since DEC-049; `passed_pawn_mg` is no longer zero |
| `src/data_structures.hpp` | "evaluate() is material plus tapered tables" | mobility, king safety and pawn terms too |
| `tests/test_search.cpp` | "S065 is a pending 827-constant paste" | S065 is done |
| `tests/test_search.cpp` | "an H0 on S234 flips `RfpTtEstimate`" | S234 is closed and it ships at 1 |
| `tools/timer_race_stress.py` | cites a numbered paragraph of `AGENTS.md` | none since DEC-109; `tests/test_uci_surface.cpp`'s header is the same class |
| `fastchess.sh` | cites lines 72 to 77 of `rating.sh` as doing the same | those lines count cores |
| `tests/CMakeLists.txt` | `24 cases, ~1.6 s` for `tests/test_mutation_check.py` | 31 cases since S305, found by that step and not by the study |

Filler by DEC-171: none moves a score, a move or a line. It runs in the gap after S118's landing
or refusal and before S101, because S118 rewrites `src/evaluation.cpp` and nine open steps touch
it. If the owner takes the study's cleanup, this folds into its step that covers `src/`. A comment
edit in `fastchess.sh` moves no harness behaviour, so DEC-143's A/A does not read it as a change.

## 2026-11-01: the twelve, read against the code (Opus 5.5 subagent, coordinated)

Base `b385761`, bench 5707449. **All twelve read false**, so none was left alone. Row 10 is two
files. The list's shape held; rows 2, 5 and 6 read differently in a detail, noted in each.

| # | block | what the code reads | what was done |
|---|---|---|---|
| 1 | `src/search.cpp` above the singular block | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)`; the `else if (SE_MULTICUT != 0 && ...)` branch is live; S097's stamp has verdict 2 moving the default 0 to 1 at `63ad848` | "ships switched on since its own verdict" |
| 2 | `tests/test_engine.cpp`, the timer board | below: 35516352 nodes, no assertion reads it, the bound still separates. The study's 8962 ms is the same node count on a busier machine; idle reads about 6.8 s | figure re-taken and dated, "Golden" mark gone with the reason, the old reading dated, the 0.46 ms derived from it now 0.39 ms. The bound did not move |
| 3 | `src/search_params.hpp` `HP_COEFF`'s block | the `PRUNE_HISTORY` site compares `state->quiet_history` directly. DEC-209 (3) is where S098 was given the move; S098's step file holds no such change; no file in `plan_todo/` and no line of `plan.md` names `HistPruneCoeff` or the move | "DEC-209 gave S098 the move onto the sum; S098 closed without making it and no open step holds it" |
| 4 | `src/search_params.hpp` `LMR_ROUND_BIAS`'s block | bench is 5707449; S236's table has 4493659 with all eight replies identical and `search_bench` identical at both depths | true when written, so kept and dated: "at S236 the build printed", "reproduced". Removing it would lose the proof the off value rests on |
| 5 | `src/search_params.hpp` `QUIET_HISTORY_MAX`'s block | lines 33 to 37 of `src/evaluation.cpp` are `MVV_PAWN` to `MVV_QUEEN`; the bands are `ORDER_TT_MOVE` to `ORDER_COUNTER`, lines 40 to 44 at this tree (the study's line 39 was `5954c12`'s) | the line citation became "src/evaluation.cpp, ORDER_*", a name that does not drift |
| 6 | `src/evaluation.cpp` above `pawn_pass` | `pawn_pass<false>` has one caller, `pawn_entry_fill`: true. `passed_pawn_mg` is `{9, -5, -2, 21, 38}`; of the groups the pass feeds only `doubled_file` and `backward_rank` are all zero, compiled out by their `_live` flags. `nm -C build/src/chesso` lists `t void pawn_pass<false>(...)`: out of line on g++ 13.3 | kept "exactly one caller", dated the fold with "once", said what holds now. **The sentence could not simply become "stays inlined": `nm` says it does not.** "cost 21 % once" was clang's, stays dated, and is the reason left for the one caller |
| 7 | `src/data_structures.hpp` `eval` field | `evaluate()` is `evaluate_cheap() + evaluate_expensive()`; `src/eval_trace.hpp`'s term list has material, tables, mobility, king safety, the pawn terms and piece placement. `tempo_mg` and `tempo_eg` are 0, so tempo is not named | the term list. S213's 22945 is dated by "S213's case measured" and stays; the case on this tree prints 21807 over the same 66430 placements, corpus 14746, so the 16-bit argument holds |
| 8 | `tests/test_search.cpp` "ordering keeps the tree small" | S065's stamp: H1, landed by 2026-08-17 (S076 followed it). The band has moved only by the GOLDEN block's rule, and that block has the pair still 440000 and 20000 on 2026-09-10 | "S065 was then a pending paste, which has since landed". The instruction "tighten it when a fit lands" is the rule that block replaced, so it was removed and the block named instead |
| 9 | `tests/test_search.cpp`, the extra-ply mate row | S234's stamp: one verdict, H1; `X(RFP_TT_ESTIMATE, "RfpTtEstimate", 1, 0, 1)` | conditional past, then what happened: "S234 read H1 and `RfpTtEstimate` ships at 1, so the 11 stays as history". Kept: it says why the block names a depth the row no longer has |
| 10a | `tools/timer_race_stress.py` docstring | `AGENTS.md` has named rules and no numbered paragraph; the four exits are WATCHERS. The script does not read `__doc__` | "(AGENTS.md WATCHERS)" |
| 10b | `tests/test_uci_surface.cpp` header | same class; the rule is SURFACE; DEC-024 says what the sentence says | "AGENTS.md SURFACE and DEC-024" |
| 11 | `fastchess.sh` above `outdir` | lines 72 to 77 of `rating.sh` count cores; its `stamp` and `outdir` are what the sentence means, further down | "as rating.sh's outdir already does" |
| 12 | `tests/CMakeLists.txt` above `test_mutation_check` | `Ran 31 tests`; 3.81, 3.81 and 3.90 s here | "31 cases, ~3.8 s" |

### The figure in `tests/test_engine.cpp`: it is no golden, and the mark went

**What the case asserts** on `timer_fen`: `elapsed_ms <= bestmove_bound_ms`, 402 ms, and a playable
move. Nothing else. `git grep` finds 25933707 in that comment, in `adocs/data/S022_v2_sprt.sh`, in
DEC-240 and in S022's step file, and in no assertion. `DEV_MANUAL.md` "Goldens: named, scripted,
re-derived" defines a golden as "a number the suite asserts" and its table has no row for this one,
so no row moved. The mark also sat outside the rule's own listing: `**Golden** (DEC-142)` is not
what `grep -rn 'GOLDEN (DEC-142)' tests/` finds. Asking every search or evaluation step to re-derive
a number no test reads is the obligation that went unmet: the iteration went from 25933707 to
35516352 nodes and only `58585f8` and S022 v2's two commits ever changed the figure in the file. So
the mark goes, and the figure stays as a dated reading the bound's margin is judged against.

The command, the comment's own (`position fen <below>`, `go depth 1`), a fresh process each run, on
the Release build of `b385761`, 2026-11-01 from 01:46, load average 4 falling to 2 after a build:

| run | nodes | `time` | `bestmove` wall from `go` |
|---|---|---|---|
| `go depth 1`, 6 runs | 35516352 every time | 6742 to 6798 ms | 6743 to 6799 ms |
| `go movetime 1`, timer disarmed, 3 runs | 35516352, depth 1 | 6746 to 6776 ms | 6747 to 6777 ms |
| `go movetime 1`, as shipped, 20 runs | none reported | | 1.9 to 2.1 ms, over the pipe |

The disarmed build is a throwaway in the session's scratch directory: this tree with the store of
`stop_search_signal` removed from `stop_search_after_ms`, bench still 5707449, never in the worktree.
It shows directly what a timer that never fires produces: the soft limit ends the first iteration
that completes, as the comment says.

**The bound still separates**: 6746 / 402 is about 17 times, against 14 when the block was written
and 10 under S022 v2. At the study's 8962 ms it is about 22 times. The bound did not move and no
test logic did. The 0.46 ms in the same comment was 2048 nodes at the old rate; at 5.26 M nodes a
second it is 0.39, still under the `check_granularity_ms` of 1.

### Proof that it is comments only

`adocs/data/S306_identity.txt`: seven C++ files equal under `g++ -fpreprocessed -dD -E -P`, the
Python file's `ast` without docstrings and its tokens after the docstring equal, with the one
docstring edit, and `fastchess.sh` and `tests/CMakeLists.txt` with only comment lines changed, read
by eye and by script. All 262 anchors pass `tools/mutation_check.py`'s own `load_mutants` and `validate` at the worktree,
and no file under `tools/mutants/` changed.

**Tier 2 (DEC-141) does not bind, and `tools/gate_extra.sh` was not run.** The diff touches
`src/search.cpp` in one comment and nothing in `make_move`, `unmake_move` or the generator, and the
stripped source is identical, so there is no new behaviour to self-play and no new rule to guard.
`.tuning/coord/S123/brief_close.md` item 7 is the precedent.

**Manuals.** `MANUAL.md`: its rows for `SeMultiCut` and `RfpTtEstimate` already read 1 and nothing it
says moved, so no change. `DEV_MANUAL.md`: the goldens table has no row for the figure, its `4493659`
paragraphs are history dated by step, and its sentence on `tests/test_mutation_check.py` already says
thirty-one and dates the 1.6 s, so no change. Its "AGENTS.md par.12" is listed below.

### Found and not fixed: this step is the twelve

Each is false or stale in the same way, and none moves a score, a move or a line.

- `src/data_structures.hpp` `se_multicut`: says the field is "false in every release build", the
  switch being off there and its branch folded away. The switch ships on: the study's first row again.
- `src/search.cpp` at the `PRUNE_HISTORY` site: "the sum belongs with S098", row 3's claim at the
  rule itself.
- `tests/test_engine.cpp`, the first board: "**Golden**: 36165 nodes" reads 49231 nodes and 9 ms, one
  run, on this tree. Its 3 ms floor is asserted and still clears.
- `src/evaluation.cpp`: above `pawn_pass`, "while the weights are zero no test can say so"; at
  `king_shelter_features`, "the zero weights downstream fold". Both row 6's class.
- `DEV_MANUAL.md`: "AGENTS.md par.12 requires four exits", row 10's class in a document.
- `rating.sh`: cites lines 71 to 84 of `fastchess.sh` by number, row 11's class the other way round.
- `tests/test_uci_surface.cpp`: "`surface_guard` is `cli`" names a key `AGENTS.md` no longer has;
  DEC-009 and `specs.md` still use it.
- **A gap, not a comment:** nothing asserts that the timer board's iteration stays above the bound.
  The first board has `depth_1_floor_ms`; this one has this comment, and no mutant in `tools/mutants/`
  names `stop_search_after_ms`. A board grown light would pass silently with a dead timer.

### The `accepts`, clause by clause

- The twelve re-checked at landing, each saying what the code does or gone: the table. Held.
- The figure: no test holds it, so the mark went with the reason stated, and it was re-taken anyway. Held.
- `No functional change` on every commit touching `src/`, and the gate passes it: `tools/gate.sh --message` over the proposed message on the staged tree printed
  `GATE-DONE 5707449 (no functional change)`, 50 of 50 in both builds. Holds for the message the coordinator
  commits if it keeps the line.
- Every anchor found once: all 262, by `tools/mutation_check.py`'s `load_mutants` and `validate`. Held.
- A clean reader compares old against new: **the coordinator's, open.**

## 2026-11-01: the clean reader (the coordinator)

A separate agent read old against new, and against the code and the record, and passed every id, sha
and figure but two. **Both errors were this step's own rewrites**, and no identity check could see them:

- `src/data_structures.hpp`: the rewrite read "the tapered sum of material, tables, ...". Material is
  not tapered: `score = board->material + positional`, the old sentence had that right, and it is what
  lets fifteen queens reach the 22945 this comment's 16-bit argument rests on. Now "untapered material,
  plus the tapered tables, pawn terms and piece placement, plus mobility and king safety, tapered and
  clamped".
- `tests/test_engine.cpp`: "`bestmove` after 6746 to 6776 ms" quoted the `time` field; `bestmove` read
  6747 to 6777 in this step's own table.

So two errors in twelve rewrites, after the writer's own check, which is the study's warning measured
again. The reader also found stale text beside row 8, which goes to S308 with the siblings above. S307
takes the test gap and S308 the siblings, both filler by DEC-171, behind S101.
