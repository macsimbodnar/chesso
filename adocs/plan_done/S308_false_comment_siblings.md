id:         S308
goal:       the ten comments S306 and its reader found beside its twelve are made true or removed
accepts:    each row below is re-checked against the code at landing, with what the check read, and then says what the code does or is gone; the first board's figure in `tests/test_engine.cpp` is treated as S306 treated the timer board's (DEC-142); commits touching `src/` read `No functional change`; code identity shown per file as `adocs/data/S306_identity.txt` shows it; every anchor in `tools/mutants/` still found once; a clean reader compares old against new
touches:    src/data_structures.hpp, src/chesso.cpp, src/search.cpp, src/evaluation.cpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp, DEV_MANUAL.md, rating.sh
excludes:   shortening comments or removing history, which is S312's by DEC-337; `adocs/data/` and `adocs/plan_done/`
decisions:  DEC-171, DEC-142
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, coordinated
done:       2026-11-01 -- all ten read false against the code at `1c86c1a` and each now says what the code does or is gone, one line each in this file's table, rows 1, 2, 8, 9 by deletion, rows 3, 4, 5, 6, 7, 10 by a replacement read against named code or a run. `tests/test_engine.cpp`'s first-board figure is no golden, no assertion reading it, and its mark went with the reason stated; re-taken by its command on the MacBook Air M5, 49231 nodes every run, `time` 4 ms to about 20 ms, median about 5 ms back to back and 16 to 19 ms at 0.3 s apart, cause unread, the 3 ms floor unmoved and 1 ms under the fastest; S247's reading stays dated. **A clean reader, old against new, 84 changed comment lines: four findings, all fixed before this commit: one false sentence of this step's own, the timing above, two references its edits left dangling, one of S306's in the same class.** So before a reader this step ran 1 false sentence in 6 replacements, the rate S306 read. Comments only: `adocs/data/S308_identity.txt` `S308-IDENTITY HOLDS`; `adocs/data/S312_local_gate.sh` green; `No functional change`, bench 5707449; 274 of 274 anchors; `S101-PAUSE-OK`. Tier 2 (DEC-141) is not owed, DEC-336 (4). `MANUAL.md` checked, no change; `DEV_MANUAL.md` row 6's line only; `specs.md`'s `surface_guard` removed by the coordinator. Six siblings found and not fixed, carried to S312 and S317.
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
| `src/chesso.cpp`, the board at `go depth 1` | "reports 36165 nodes now" | 49231 nodes in the audit's scratch build, F09; the same board S306 read false, and no list named this site |
| `tests/test_search.cpp`, the budget beside S306's S065 row | "the search costs 109575", "The budget is **4x** that" | the pair is 65024 from 16256, as the GOLDEN block below it says |

Two errors in S306's twelve rewrites survived its writer's own check, so the reader is not optional.
Filler by DEC-171: none moves a score, a move or a line. Run on its own, as DEC-337 keeps it. It is the first `src/` comment commit,
so DEC-338 (1) is in force from here.

## 2026-11-01: the ten, read against the code (Opus 5.5 subagent, coordinated)

Base `1c86c1a`, bench 5707449, `build/src/chesso` run on a MacBook Air M5. **All ten read false**, so none was
left alone and none gave way to S101: `adocs/data/S312_local_gate.sh --only s101` reads `S101-PAUSE-OK` on the
finished tree. Four rows delete and write no new sentence (1, 2, 8, 9); six replace (3, 4, 5, 6, 7, 10), each sentence read
against what the row names.

| # | block | what the check read | what was done |
|---|---|---|---|
| 1 | `src/data_structures.hpp`, `se_multicut` | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)`; "the multicut returns the verification's score and searches nothing" holds `record.se_multicut` true in Release, run here, 92 assertions; "the multicut does not fire at its off value" sits under `#ifdef CHESSO_TUNE`, run in `build-tune`, 55 assertions, 0 in `build` | false sentence gone; the true half kept, "`se_multicut` is what the tune build's case reads ..." |
| 2 | `src/search.cpp`, at `PRUNE_HISTORY` | no file in `plan_todo/` or `plan_current/` but this one and no line of `plan.md` names `HistPruneCoeff`; DEC-209 (3) gave S098 the move; `lmr_adjusted_reduction(depth, move_number, node_adjustment)` takes no history, and the file's own S236 block says the history term left (DEC-231), so "which scales its reduction by that same sum" was false too | both clauses gone. `HP_COEFF`'s block holds who had the move |
| 3 | `tests/test_engine.cpp`, the first board | below | figure re-taken and dated, "Golden" gone with the reason, S247's reading dated, the floor's reason dated to it, "depth 1 here is" now "read", and "the golden above" in the timer half now "the reading above". The floor did not move |
| 4 | `src/evaluation.cpp`, `pawn_pass` | `passed_pawn_mg` `{9, -5, -2, 21, 38}`, `pawn_structure_mg` `{1, -7, -6}`, so a reversed bucket moves a score. `int bucket = 6 - (get_lsb_index(white_passed) >> 3)`, `if constexpr (collect) { passed_out[WHITE][bucket]++; }`; `tests/test_eval_model.cpp` reads `passed_pawn_counts` and `pawn_structure_counts` against `tests/test_walked_counts.hpp` | "passed_pawn_counts() below says so without a score, which is why `collect` is here, and pawn_structure_counts() does the same for pawn structure". "The same hole" went with what it pointed at |
| 5 | `src/evaluation.cpp`, `king_shelter_features` | `king_safety_mg` `{18, 13, 5, 30, -27, 16, 7, -47, -25}`, `king_safety_eg` `{-5, -9, 7, -52, 10, -8, -1, -4, 19}`, the four shelter features among them. The sentence entered at `690de1f`, whose message is "S027 term 1 ... shipping the eighteen weights at zero" | dated: "the zero weights S027 landed folded ... came back to 18.54. Those weights are non-zero now", and "is supposed to be free" now "was then" |
| 6 | `DEV_MANUAL.md` | `AGENTS.md` WATCHERS: "A watcher has four exits and all four are mandatory" | "AGENTS.md WATCHERS". The line is outside S101's five hunks in that file |
| 7 | `rating.sh` | `fastchess.sh` lines 71 to 84 are now the SPRT-class comment; "That has already happened once" is in the comment above `snapshot=` | "fastchess.sh's own comment above `snapshot`" |
| 8 | `tests/test_uci_surface.cpp` | `grep -c surface_guard AGENTS.md` reads 0 | "`surface_guard` is `cli` and" gone |
| 9 | `src/chesso.cpp`, the board at `go depth 1` | below: 49231 | the sentence gone; `tests/test_engine.cpp` holds the reading |
| 10 | `tests/test_search.cpp`, "ordering keeps the tree small" | `state.node_limit = 65024`, its message "the 65024 budget was derived from 16256"; S067's stamp "measured from 109575"; `python3 adocs/data/S192_node_budget.py` reads 10306, inside the middle half [7270, 32512], so the pair stays and no re-derivation is owed | "until S067, when the search cost 109575"; "**4x** the count the GOLDEN block below derives it from". "rounded" went with 440000, and that history stays in the GOLDEN block |

### The figure in `tests/test_engine.cpp`: it is no golden, and the mark went

**What the case asserts** on `fen`: `deepest_completed_depth(capture) == 1` and `reported_ms >= depth_1_floor_ms`, 3 ms. No assertion
reads 36165; `git grep` outside `adocs/` finds it in the comment and `src/chesso.cpp`; `DEV_MANUAL.md`'s goldens
table has no row. So the mark goes, as S306's did on the timer board, and the figure stays as a dated reading.

The command, the comment's own, a fresh process each run, `build/src/chesso` at `1c86c1a`, 2026-11-01 14:41, load average 2.26:

| run | nodes | `time` | `nodes * 1e6 / nps` |
|---|---|---|---|
| `position fen <board>`, `go depth 1`, wait for `bestmove`, 50 runs back to back | 49231 every time | 4 to 5 ms | 4.68 to 5.71 ms, median 4.85 |

"a stop inside the first iteration ..." passes here, 7 assertions. **The floor clears by 1 ms on `time`**, and a machine about 1.6 times faster
than the M5 would read under it and fail the precondition by its own message, "pick a heavier position". The sentence that a
twice faster machine still separates was true of S247's 6.8 ms and is dated to it. S306's "9 ms, one run" is the i7-8700K's.

### Proof that it is comments only

`adocs/data/S308_identity.txt`, `S308-IDENTITY HOLDS`: seven C++ files equal under `-dump-raw-tokens`, the lexer and the directive
multiset; `rating.sh` equal under the lexer, directives and `declare -f`; `DEV_MANUAL.md` one line of prose. No `//` comment ends in a backslash.
`--anchors`: 262 mutants, 274 anchors, 0 bad. `S312_local_gate.sh --only identity,anchors,citations,s101,format` green; the suite and `bench` stages are the coordinator's.

`--new-facts` on the final text, each against the row that needed it: `49231`, `S308` and `1c86c1a` are row 3's run, `0.3` its spacing, the reader's finding 1. Nothing else. New words it does not list: `i7-8700K`, S247's stamp; `M5`, this machine; `WATCHERS`, `AGENTS.md`; `S027` in that block, `690de1f`'s message.

**Not run:** `tools/gate.sh`, `tools/gate_extra.sh`, both builds' suites. DEC-141's Debug self-play is not owed, DEC-336 (4). `MANUAL.md` checked: nothing here moves it, no change.
`DEV_MANUAL.md`: row 6's line; its goldens table has no row for this figure, so no row moved.

### Found and not fixed

- `tests/test_engine.cpp` `depth_1_floor_ms`: an asserted floor read from a measurement, so a golden by `DEV_MANUAL.md`'s definition and DEC-142's "value or floor", and it has no `GOLDEN (DEC-142)` form or table row. Naming it needs a row inside S101's `DEV_MANUAL.md` hunk at 2355, and this step's row is the figure.
- `src/evaluation.cpp`, `neighbour_files`: "it is invisible while the weights are zero, so it was not reasoned about", row 4's class.
- `src/evaluation.cpp`, `king_shelter_features`: "`inline` is load-bearing" rests on a measurement at zero weights, before S118 put the shelter behind the pawn table; not re-measured here, so unread rather than false.
- `src/search_params.hpp` and `src/search.cpp`: "16384 is the range top -- twice the band's own edge" and "a range top at twice that edge". `QuietHistoryMax` ships 8831; twice is 17662. True of the 8192 that then shipped.
- `tests/test_search.cpp`, the GOLDEN block: its readings stop at 7366; the script reads 10306 and `DEV_MANUAL.md`'s row has S121, S123 and S125.
- `adocs/specs.md`, Behaviour: "why `surface_guard` is `cli`", row 8's key in a shared document. The coordinator took it out in this tree; DEC-009 and DEC-024 keep the word as history.
- `adocs/data/README.md` has a row for `S306_identity.txt` and none for `S308_identity.txt`; existing files there were out of this brief.

### The `accepts`, clause by clause

- Each row re-checked with what the check read, and each says what the code does or is gone: the table. Held.
- The first board's figure treated as the timer board's: dated reading, mark gone with the reason, re-taken. Held.
- `No functional change`: `--identical` green and `build/src/chesso bench` 5707449 before the edits; the message is the coordinator's.
- Code identity per file as S306's shows it: `adocs/data/S308_identity.txt`. Held.
- Every anchor found once: 274 of 274. Held.
- A clean reader compares old against new: **the coordinator's, open.**

## 2026-11-01: the clean reader (the coordinator)

A separate agent read old against new over 84 changed comment lines and found four things; no identity check could see them. The
coordinator confirmed the first by a run.

| # | finding | done |
|---|---|---|
| 1 | `tests/test_engine.cpp`, first board: "4 to 5 ms ... median 4.85 ms ... 50 runs" holds only for runs fired back to back. The coordinator's 15 a set: 49231 every run; `time` 4 to 10 and 4 to 18 ms back to back, medians 5.3 and 5.5; 4 to 21 and 4 to 20 ms 0.3 s apart, medians 15.7 and 19.4. Re-run here, 15 a set: 49231 every run; 4 to 7 and 4 to 19 ms, medians 5.0 and 5.1; 5 to 21 and 5 to 21 ms, medians 19.2 and 19.3 | "49231 nodes every run and `time` from 4 ms to about 20 ms", median "about 5 ms over runs back to back and 16 to 19 ms over runs 0.3 s apart, the cause unread". "1 ms under the M5's fastest run" stays: 4 ms is the minimum in all sets. The brief's "the low end from runs back to back" was written as the medians, since 4 ms also occurs in the coordinator's spaced runs |
| 2 | Same block: "which no run here reproduces" now reads as the M5, whose spaced runs straddle 13.8. S247's stamp says it of the i7-8700K | "which no run on the i7-8700K reproduced" |
| 3 | `src/data_structures.hpp`: "show the off value is off" lost its antecedent with row 1's deletion | "show `SE_MULTICUT`'s off value is off" |
| 4 | `tests/test_engine.cpp`, `timer_fen`: "about 6.8 s on this machine" dangles the same way. `adocs/data/S306_identity.txt` reads "Machine: i7-8700K, g++ 13.3, 2026-11-01" for the same session as S306's readings | "about 6.8 s on the i7-8700K" |

Left: `src/evaluation.cpp`'s "`inline` is load-bearing". The reader could not show it false and it needs a build and a timing, so it stays in the list above.
So one false sentence of this step's own and two references its edits left dangling, after the writer's own check, and one of S306's in the same class, which naming a second machine here exposed.
