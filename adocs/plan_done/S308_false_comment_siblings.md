id:         S308
goal:       the eleven comments S306, its reader, S101's close and S307 found beside S306's twelve are made true or removed
accepts:    each row below is re-checked against the code at landing, with what the check read, and then says what the code does or is gone; the first board's figure in `tests/test_engine.cpp` is treated as S306 treated the timer board's (DEC-142); commits touching `src/` read `No functional change`; code identity shown per file as `adocs/data/S306_identity.txt` shows it; every anchor in `tools/mutants/` still found once; a clean reader compares old against new
touches:    src/data_structures.hpp, src/search.cpp, src/evaluation.cpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp, DEV_MANUAL.md, rating.sh, adocs/data/S308_identity.txt, adocs/data/README.md
excludes:   shortening comments or removing history, which is parked (DEC-342); `adocs/data/` and `adocs/plan_done/`
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 -- all eleven rows, the nine and the two added, read false or incomplete against the code at `7071407` and each now says what the code does or is gone, one line each in this file's table. `tests/test_engine.cpp`'s first-board figure is no golden, no assertion reading it, and its mark went with the reason stated; re-taken by its command on the i7-8700K, 48840 nodes every run, 9 to 13 ms, medians 9.8 and 9.9 ms, 400 runs, the 3 ms floor unmoved. The timer board's reading now 36024721 nodes and about 7.1 s. Comments only by `adocs/data/S308_identity.txt`, `S308-IDENTITY HOLDS`; bench 5482277 in both builds; both fast suites 50 of 50; format and the five prose checks rc 0; 264 mutants pass `validate`, no file under `tools/mutants/` changed. Tier 2 (DEC-141) does not bind: no code moved. `MANUAL.md` checked, no change; `DEV_MANUAL.md` row 6's line only. The clean reader is the coordinator's and open. Filler (DEC-171), no score, move or line moved.

## Why

Reported on 2026-11-01 by S306's implementer and its clean reader, and by neither of them changed,
that step being bound to its twelve. Each row's quoted words find its block.

| block | what it says | what was read |
|---|---|---|
| `src/data_structures.hpp`, the `se_multicut` field | "false in every release build" | the switch ships on |
| `src/search.cpp`, at `PRUNE_HISTORY` | "the sum belongs with S098" | S098 closed without the move and no open step holds it, as S306's `HP_COEFF` row says |
| `tests/test_engine.cpp`, its first board | "**Golden**: 36165 nodes" | 49231 nodes, 9 ms, one run; its 3 ms floor still clears |
| `src/evaluation.cpp` | "while the weights are zero no test can say so" | the groups are no longer all zero |
| `src/evaluation.cpp` | "the zero weights downstream fold" | the same; only `doubled_file` and `backward_rank` are all zero |
| `DEV_MANUAL.md` | "AGENTS.md par.12" | no numbered paragraphs since DEC-109 |
| `rating.sh` | cites lines 71 to 84 of `fastchess.sh` by number | line numbers decay; name the symbol |
| `tests/test_uci_surface.cpp` | `surface_guard` | a key `AGENTS.md` no longer has |
| `tests/test_search.cpp`, the budget beside S306's S065 row | "the search costs 109575", "The budget is **4x** that" | the pair is 65024 from 16256, as the GOLDEN block below it says |

Two errors in S306's twelve rewrites survived its writer's own check, so the reader is not optional.
Filler by DEC-171: none moves a score, a move or a line. If the owner takes the comment study's cleanup,
this folds into its step that covers `src/`.

## Added 2026-11-01

S101's close flagged a tenth: `src/evaluation.cpp`'s fusion comment, "not only for the zeros king safety currently ships", looks stale. `085180b`
holds this step done once on another machine and removed with the rest (DEC-341): re-read it before redoing the work.
S307 flagged another, in `tests/test_engine.cpp` above the timer board: S306's dated 35516352 reads 36024721 at S101's fit, and "the bound below being what the case holds" is incomplete since S307.

## 2026-11-01: the eleven, read against the code (Opus 5.5 subagent, coordinated)

Base `7071407`, bench 5482277, `build/` on the i7-8700K. **All eleven read false or incomplete at HEAD, so none took no edit.**
`085180b` was re-read and not trusted: its row on `src/chesso.cpp` is not this file's, so it did not land, and rows 3 and 11 are re-taken here.

| # | block | what the check read | what was done |
|---|---|---|---|
| 1 | `src/data_structures.hpp`, `se_multicut` | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)`. "the multicut returns the verification's score and searches nothing" requires `record.se_multicut` in Release, 92 assertions. "the multicut does not fire at its off value" sits in the `#ifdef CHESSO_TUNE` block, 55 assertions in `build-tune`, 0 in `build` | says the field is true wherever the multicut fired, release included, the switch shipping at 1, and that only the tune build can set 0, where its case reads the field |
| 2 | `src/search.cpp`, at `PRUNE_HISTORY` | no file in `plan_todo/` or `plan_current/` but this one and no line of `plan.md` names `HistPruneCoeff` or `HP_COEFF`; S098's file names neither. `lmr_adjusted_reduction(depth, move_number, node_adjustment)` takes no history, so "scales its reduction by that same sum" was false too (DEC-231, same file) | both clauses gone; one sentence points at `HP_COEFF`'s block, which reads "DEC-209 gave S098 the move onto the sum; S098 closed without making it and no open step holds it" |
| 3 | `tests/test_engine.cpp`, the first board | below: 48840 nodes, 9 to 13 ms. No assertion reads the figure | figure re-taken and dated, "Golden" gone with the reason, S247's reading dated, "a twice faster machine" now three times (the minimum 9575 us over the 3 ms floor), "depth 1 here is" now "read", "the golden above" in the timer half now "the reading above". The floor did not move |
| 4 | `src/evaluation.cpp`, `pawn_pass` | `passed_pawn_mg` `{9, -5, -2, 21, 38}`, `pawn_structure_mg` `{1, -7, -6}`, so a reversed bucket moves a score. `const int bucket = 6 - (get_lsb_index(white_passed) >> 3)`. `tests/test_eval_model.cpp`, `tests/test_eval_trace.cpp` and `tests/test_evaluation.cpp` read `passed_pawn_counts` and `pawn_structure_counts` | the false clause gone; "passed_pawn_counts() below reads the buckets without a score, which is why `collect` is here" |
| 5 | `src/evaluation.cpp`, `king_shelter_features` | `king_safety_mg` `{18, 13, 5, 30, -27, 16, 7, -47, -25}`, `king_safety_eg` `{-5, -9, 7, -52, 10, -8, -1, -4, 19}`, the four shelter features none of them zero. The sentence entered at `690de1f`, "S027 term 1 ... shipping the eighteen weights at zero". `nm -C build/src/chesso` lists no copy of the function, so g++ 13.3 inlines both callers | past tense, dated by "the zero weights it landed with" and "Those weights are non-zero now" |
| 6 | `DEV_MANUAL.md`, the watcher loop | `AGENTS.md` WATCHERS: "A watcher has four exits and all four are mandatory" | "AGENTS.md WATCHERS" |
| 7 | `rating.sh`, above `snapshot` | `fastchess.sh` lines 71 to 84 are the SPRT-class comment; "That has already happened once" is in the comment above `snapshot=` | "fastchess.sh's own comment above `snapshot`" |
| 8 | `tests/test_uci_surface.cpp` | `grep -c surface_guard AGENTS.md` reads 0 | "`surface_guard` is `cli` and" gone |
| 9 | `tests/test_search.cpp`, "ordering keeps the tree small" | `state.node_limit = 65024`, its message "the 65024 budget was derived from 16256"; S067's stamp "measured from 109575". `python3 adocs/data/S192_node_budget.py` reads 10467, 0.64 of 16256 and inside the middle half [7270, 32512], `DEV_MANUAL.md`'s row for S101 agreeing, so the pair stays and no re-derivation is owed | "until S067, when the search cost 109575"; "**4x** the count the GOLDEN block below derives it from". "rounded" went: 65024 is 4 x 16256 exactly |
| 10 | `src/evaluation.cpp`, the fusion, added row | `king_safety_mg` and `_eg` as in row 5. `taper(mobility_sum + threat_sum, phase)` and `taper(safety_sum, phase)` are still two truncating divisions, S101's family sharing mobility's as the comment at that site says | "not only for the zeros king safety currently ships" gone; "for any weights" stays |
| 11 | `tests/test_engine.cpp`, above the timer board, added row | below: 36024721 nodes, 7061 to 7088 ms. The case since S307 also asserts the iteration outlasts the bound, by `go depth 1` and `stop` at `still_running_at_ms` | current reading replaces S306's as what the iteration reads; S306's disarmed run keeps its own count and commit, since it was not re-taken; the 0.39 ms derived from the old rate now 0.40; "the case holding the bound below and that the iteration outlasts it, which stores no figure" |

### The readings

Fresh `chesso` process each run: `uci`, `isready`, `position fen <board>`, `go depth 1`, `quit` only after `bestmove`. `build/src/chesso` at `7071407`,
2026-11-01 about 21:59, one-minute load 1.5, nothing CPU-bound running. S247's script was not on this machine, so the session wrote the same shape again: rounds of 20 interleaved with the start position.

| board | runs | nodes | `nodes * 1e6 / nps`, us | `time` ms |
|---|---|---|---|---|
| first, set 1 | 200 | 48840 every run | min 9575, median 9933, p90 12048, max 13385 | 9: 103, 10: 32, 11: 44, 12: 9, 13: 12 |
| first, set 2 | 200 | 48840 every run | min 9583, median 9779, p90 11105, max 13266 | 9: 148, 10: 22, 11: 26, 12: 2, 13: 2 |
| start position, control | 400 | 49 | median 590 to 601 | 0 every run |
| timer board | 3 | 36024721 every run | | 7088, 7061, 7087; nps about 5.08 M |

**What the first board's case asserts:** `deepest_completed_depth(capture) == 1` and `reported_ms >= depth_1_floor_ms`, 3 ms. `git grep 36165` outside `adocs/` finds this comment and `src/chesso.cpp`, and `DEV_MANUAL.md`'s goldens table has no row, so no row moved.
The mark goes as S306's did on the timer board. **`085180b`'s table read this board on a MacBook Air M5 at `1c86c1a`, 49231 nodes, `time` 4 ms at the least and 4.68 ms by `nodes * 1e6 / nps`; not re-taken here.** That is about twice the i7-8700K's rate, so the floor clears by about 1 ms there and a machine about 1.5 times faster than the M5 reads under it. "Three times faster" is the i7-8700K's, as the comment names it.

### Proof that it is comments only

`adocs/data/S308_identity.txt`, `S308-IDENTITY HOLDS`: six C++ files byte-equal under `g++ -fpreprocessed -dD -E -P`, its control holding; `rating.sh` only comment lines by script, `bash -n` rc 0; `DEV_MANUAL.md` one line of prose.
Anchors: 264 mutants over 34 files pass `tools/mutation_check.py`'s `load_mutants` and `validate` at the worktree, and no file under `tools/mutants/` changed.

**Tier 2 (DEC-141) does not bind, and `tools/gate_extra.sh` was not run.** The diff touches `src/search.cpp` in one comment and nothing in `make_move`, `unmake_move` or the generator, the stripped source is identical, and S306 is the precedent.

**Manuals.** `MANUAL.md`: its `SeMultiCut` row already reads 1, it holds none of these figures, no change. `DEV_MANUAL.md`: row 6's line; its goldens table has no row for either figure and its row for the budget is right, so nothing else.

### Found and not fixed: this step is its rows

- `src/chesso.cpp`, the stop's comment: "`go depth 1` there reports 36165 nodes now" reads 48840. Row 3's board, no row here. `085180b` deleted the sentence.
- `src/evaluation.cpp`, `neighbour_files`: "it is invisible while the weights are zero, so it was not reasoned about", row 4's class.
- `src/evaluation.cpp`, `king_shelter_features`: "`inline` is load-bearing" rests on a clang timing at zero weights, before S118 put the shelter behind the pawn table. Unread rather than false: it needs a timing.
- `src/search_params.hpp` at line 840 and `src/search.cpp` at `PRUNE_HISTORY`: "16384 is the range top -- twice the band's own edge" and "a range top at twice that edge". `QuietHistoryMax` ships 8831, and twice is 17662. True of the 8192 that then shipped.
- `tests/test_engine.cpp` `depth_1_floor_ms`: an asserted floor read from a measurement, so a golden by `DEV_MANUAL.md`'s definition, with no `GOLDEN (DEC-142)` form and no table row.
- `tests/test_search.cpp`, the GOLDEN block: its readings stop at 7366; `DEV_MANUAL.md`'s row has 10467.
- `adocs/specs.md`, Behaviour: "why `surface_guard` is `cli`", row 8's key in a shared document. The coordinator's.

### The `accepts`, clause by clause

- Each row re-checked with what the check read, and each says what the code does or is gone: the table. Held.
- The first board's figure treated as the timer board's: dated reading, mark gone with the reason, re-taken. Held.
- `No functional change`: bench 5482277 in both builds and the stripped sources identical; the message is the coordinator's.
- Code identity per file as S306's shows it: `adocs/data/S308_identity.txt`. Held.
- Every anchor found once: 264 mutants, by `validate`. Held.
- A clean reader compared old against new, one Opus 5.5 subagent, 101 changed comment lines: 0 meaning-changing errors from the rewrite. One false clause inherited inside row 8, on what a removal fails, and one thin claim in row 3, both fixed by the coordinator, comments only, the stripped sources unmoved.

### Gates

Both builds without a warning. Fast suites 50 of 50 in `build` and in `build-tune`, `CLANG_FORMAT_MAJOR=22`. `./clang-format.sh --check` rc 0. `tools/plan_prose_check.py` `--citations`, `--touches`, `--params`, `--gate`, `--prose`, each rc 0. Bench 5482277 in both builds. `tools/gate.sh` not run: it commits nothing here and the message is the coordinator's.
