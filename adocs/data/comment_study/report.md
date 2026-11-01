# The comment study: its report, verbatim

The owner asked, 2026-10-31, whether cleaning or compressing comments helps the project and token use. One Opus 5.5
subagent read `achesso` at `5954c12`; this is its final report, unedited but for this header. The harness refused its write
of a report file, so the text lived only in a session transcript until the coordinator saved it here on 2026-11-01, at the
owner's request. It predates S118's landing and S101, so its line numbers and its order around S118 and S101 are history; the
plan that binds is S311, S312 and S313 and DEC-335. "`.tuning/coord/comment_study/`" in the text is this directory, the
scripts are records with the workstation's paths in them, and regenerable outputs were left out: `blocks.jsonl`,
`blame_blocks.jsonl`, `sample.jsonl`, `sample_classified.jsonl`, `sample_dump_*.txt`, and `transcript_rows.jsonl`, which only the
workstation's transcripts can produce. `pilot/search.cpp` and `pilot/test_mate_carry.cpp` are the pilot's compressed copies, as
examples.

## SUMMARY

**Verdict.** Too many: true by volume. Old ones worse: refuted. Git holds it: true for history, false for reasons. Tokens: yes, but small. No sweep pays back inside the 15 open steps.

**Key numbers** ([M] measured, [E] estimated; tokens are chars/4 unless called real; real = x1.5, from API usage)
- [M] Comment share of bytes: `src` 60.9 %, `tests` 48.1 %, `tools` 41.1 %, live code 50.1 %, 1.93 MB, 483 K tokens. `search_params.hpp` 91 %, `search.cpp` 72 %.
- [M] Growth: `src` comments 13x in 12 weeks, code lines 2x. +15.5 KB/day in `src`, `tests`, `tools` over the last 30 days.
- [M, one reader, 140 live blocks, 90 % CI] Reason or hazard 44.3 [40.8, 47.6], contract 24.0 [20.9, 27.5], history 22.0 [19.7, 24.3], rule-required 6.1 [4.5, 7.8], restates code 2.1 [1.2, 3.0], pointer 1.0, stale text 0.4.
- [M] Age: 44 % of comment bytes under a month old, 16 % August or earlier. Stale against age: Spearman +0.04, p 0.66.
- [M] Stale tracks length: 9 of 36 blocks at 1200 B or more, 2 of 104 smaller (Fisher p 8e-5). Newest third is 28 % history, oldest 19 %.
- [M] Dead claims: 11 of 140 sampled. A floor. Includes a "Golden" number in `test_engine.cpp`: comment says 25933707 nodes, the binary answers 35516352, no test holds it. Also `search.cpp:2432` says multicut "ships switched off"; it ships on.
- [M] Recoverable: exact facts in history blocks, 93 % in a document the block cites. Why blocks 71 %.
- [hand test, chosen by me, 25 blocks] By id 10, with effort 8, comment only 7. A number with an id behind it is in the step file; a local reason usually is not.
- [M] Pointer cost: cited documents median 44x the block; one measured access 4 to 6 KB; median block 153 B. A pointer can replace a long log, never a short reason.
- [M] Agents barely use history: 44 `git log -S/-G/-L` and 1 `git blame` in 25004 Bash commands.
- [M, calibrated] Comments in code read are 7.4 % of weighted consumption, 8.1 % of raw. Live code 5.75 %; `adocs/data` scripts and diffs are the rest, and a cleanup does not touch them.
- [M, calibrated] Context, one line: command output 16 %, agent's own text and inputs 14 %, reminders 11 %, base context 10 %, `adocs/` documents 8 %, output at 5x 7 %. No single source dominates.
- [M] 73 % of comment bytes arrive through a line window, which caps the saving. Hence a pass-through bracket, low 0.27, mid 0.63 (an assumption), high 1.00.
- [E] Median step (2.90 M weighted units, 26.4 M tokens): 30 % cut saves 29.6 K units, 1.02 % at mid (0.43 to 1.61); 60 % cut saves 59.2 K, 2.04 % (0.86 to 3.21). Weighted uses read 0.05x on Opus 5.5, write 1.25x or 2x, output 5x.
- [M] Non-token harm: 61.3 % of changed `src` lines in the last 150 commits are comment-only; 15 comment-only commits; F04 (high) in `2026-09-10_adversarial`; F05 open 69 days; S213, S245, S249, S287.
- [M] Benefit: briefs send agents to comments (S289, S121, S125); S292's shipped guard came from a hazard its comment named.
- [M] Pilot, code identical by `g++ -fpreprocessed -dD -E -P`: `search.cpp` 309 lines -51 % comment bytes; `test_mate_carry.cpp` -74 %. 4 of 4 real questions still answered; "why did C rise at S116" no longer is.
- [M] **My first pilot draft held 2 real errors, a fabricated pointer and a wrong denominator.** Identity checks cannot see them; only re-reading old against new caught them.
- [M] Breaks: 5 of 244 mutant anchors overlap comments, plus 2 of 30 on `s118`, loud only at next run. Pending citations: 0 rest on comments. Comment wrap goes through the format gate.

**Options** ([E], % of weighted, mid; cost in weighted units; payback in mean steps of 6.8 M)
- A, rule only: 0 now, under 0.5 M. Halving growth avoids B1's whole removal in 3 weeks.
- B1, 72 log-shaped blocks, 294 KB: 0.41 %, 3.3 to 7 M, payback 120 to 250. 4 of 10 sampled hold a dead claim.
- B2, 16 files by traffic, 669 KB: 1.16 %, 6 to 13 M, payback 76 to 165.
- B3, all live code: 1.88 %, 17 to 37 M, payback 134 to 291.
- C, aggressive: 2.57 %, 15 to 30 M, payback 86 to 172. Deletes reasons held nowhere else. Not recommended.

**Recommendation: A now, then B1 after S118, justified by dead claims and append-logs, not tokens. B2 and above not in phase one.** A alone is also sound.

**Plan.**
- 0, owner decides the rule and scope. 1, `tools/comment_budget.py` (report and `--identical`), any gap.
- 2, rule in `AGENTS.md`, close briefs say "replace, never append".
- 3, tests' golden logs, after S118's reading. 4, `src/` dead claims and log blocks, in the gap after S118 and before S101, about 15 min of gates.
- 5, `tools/` and root scripts, no run active. 6, measure over 5 steps and record the verdict.

**S118.** Its branch rewrites `evaluation.cpp` (483 lines), `data_structures.hpp`, `test_evaluation.cpp` (511). Nothing on those files lands before DEC-332 (7), (8) are read. `evaluation.cpp` is touched by 9 open steps, so only a gap between steps works.

**Stop result.** For tokens alone it is already in. For B1: comment tokens per step fall under 2.4 % of baseline over 5 steps; or more than one meaning-changing error per 1000 comment lines after review; or a removed reason gets re-derived.

**Not measured.** Pass-through (the largest uncertainty, only a real cut measures it). A second rater. Whether agents do better with fewer comments. Release object identity (no build allowed). Option costs are model-based. No main transcripts for 08-15 to 09-29.

---

# FULL REPORT

## Comment study, `achesso` at `5954c12`, 2026-10-31

Marks: **[M]** measured by a named script. **[E]** estimated, assumption stated.

**Tokens.** No tokenizer here. Census figures use chars/4, as asked. API usage says that underestimates this tree: a regression of real tokens on bytes for 1410 code reads that arrived alone gives 0.374 tokens per comment byte (2.68 bytes/token) and 0.437 per other byte (2.29), overhead 377 tokens [M, `transcripts.py`, `token_regression.json`]. Reads carry line-number prefixes, so this is an upper-side reading, not a tokenizer. *Real* means chars/4 x 1.50. Do not read any token figure to better than about 20 %.

## Verdict on the owner's hypothesis

| claim | finding |
|---|---|
| too many | **True by volume.** 59 % of `src/` bytes are comment text, 61 % with indentation. `src/` comment bytes grew 13x in 12 weeks, code lines 2x. [M] |
| many not needed | **Partly.** History 22 % of live comment bytes, reasons and hazards 44 %, contracts 24 %, rule-required 6 %. Restating the code 2 %: the classic complaint is nearly absent. [M, one reader] |
| old ones above all | **Refuted.** 44 % of comment bytes under a month old, 16 % August or earlier. Staleness does not track age (+0.04, p 0.66); it tracks length (9 of 36 against 2 of 104, Fisher p 8e-5). Newest third 28 % history, oldest 19 %. [M] |
| git holds the history | **True for history, false for reasons.** History-block facts: 93 % in a cited document. 7 of 25 hand-tested reasons are in no document. Agents almost never ask git. [M] |
| cleanup reduces tokens | **Yes, small.** Comments are about 7 % of real consumption. 30 % cut about 1 %, 60 % cut about 2 %, at most 3.2 %. No sweep pays for itself inside the 15 open steps. [M inputs, E result] |

**Not asked, and larger:** comment bytes grow 15.5 KB a day [M]. Any one-off cut regrows in days or weeks. A rule on new comments is the cheapest and largest lever.

## 1. How much

Counter: `comment_lexer.py`, `count_comments.py`, blobs read with `git show 5954c12:<path>`.
- C++ and `.S`: character lexer for strings, raw strings with prefixes and delimiters, char literals, digit separators, line-continued `//`. `#if 0`: **none in the tree**.
- Python: `tokenize`, plus `ast` for docstrings (counted as comment, 176 KB of live). Shell: quotes, `$'...'`, heredoc bodies as code, `$#`, `${#x}`. CMake: bracket comments and arguments. Awk: `#`.
- Excluded: `tests/third_party/`, `tests/doctest`, 518 non-code files under these roots (`src/openings.bin`, JSON suites). No lex errors.
- "Blocks" merge adjacent whole-line comments. "Share of bytes" is removable bytes (comment, indentation, newline) over file bytes. Trailing comments are only 8 KB of live.

**Checks.** Hand counts match on one file per language:
- `src/log.hpp`: 4 comment lines, 8 trailing, 83 code, 25 blank, 11 blocks.
- `clang-format.sh`: 30, 1, 55, 12, 4 blocks.
- `tools/mutants/board.py`: 26 including an 18-line docstring, 60 code, 3 blocks.
- `cmake/build_info.cmake`: lexer 34 against grep 37; the 3 extra are `#define` inside a quoted string, so the lexer is right.
- `adocs/data/S264_strip_standings.awk`: 3, 18 code.
- Fixtures pass (`lexer_selftest.py`).
- `g++ -fpreprocessed -dD -E -P` over 8 C++ files equals the lexer's stripped text without whitespace. Two differ by exactly `#pragma once`, which g++ drops from a main file (`check/cmp_gpp.py`).

| area | files | code lines | comment lines | blocks | comment bytes | tokens, chars/4 | share of bytes | share of code+comment lines |
|---|---|---|---|---|---|---|---|---|
| `src/` | 27 | 9274 | 8144 | 1143 | 539585 | 134896 | 60.9 % | 46.8 % |
| `tests/` | 55 | 28064 | 15766 | 2822 | 987650 | 246912 | 48.1 % | 36.0 % |
| `tools/` | 72 | 12125 | 5378 | 777 | 333844 | 83461 | 41.1 % | 30.7 % |
| `cmake/` | 3 | 127 | 176 | 10 | 11221 | 2805 | 68.0 % | 58.1 % |
| root scripts and `CMakeLists.txt` | 6 | 887 | 962 | 87 | 61561 | 15390 | 64.7 % | 52.0 % |
| **live, all above** | 163 | 50477 | 30426 | 4839 | 1933861 | 483465 | 50.1 % | 37.6 % |
| `adocs/data/` scripts | 254 | 33554 | 24622 | 2021 | 1506848 | 376712 | 52.4 % | 42.3 % |
| **census** | 417 | 84031 | 55048 | 6860 | 3440709 | 860177 | 51.1 % | 39.6 % |

"Live" means every row above `adocs/data/`. That directory holds run records and pre-registrations, is treated as append-only (`S199_drift.py`), excluded as evidence by `clang-format.sh`, and out of any cleanup.

| file | bytes | code lines | comment lines | comment bytes | tokens, chars/4 | share of bytes |
|---|---|---|---|---|---|---|
| `tests/test_search.cpp` | 673125 | 6800 | 6142 | 375228 | 93807 | 59.6 % |
| `src/search.cpp` | 190123 | 1425 | 2020 | 129123 | 32280 | 71.7 % |
| `tests/test_engine.cpp` | 253792 | 3456 | 1825 | 114136 | 28534 | 48.1 % |
| `src/search_params.hpp` | 109260 | 165 | 1209 | 98492 | 24623 | 91.4 % |
| `src/evaluation.cpp` | 117773 | 1149 | 1020 | 66172 | 16543 | 57.5 % |
| `src/chesso.cpp` | 104915 | 1402 | 799 | 51941 | 12985 | 52.2 % |
| `src/bitboard.cpp` | 127651 | 2153 | 775 | 45995 | 11498 | 37.3 % |
| `fastchess.sh` | 62326 | 488 | 674 | 42995 | 10748 | 70.2 % |
| `src/data_structures.hpp` | 52635 | 439 | 600 | 38723 | 9680 | 76.2 % |
| `tests/test_evaluation.cpp` | 94155 | 1297 | 598 | 38210 | 9552 | 43.0 % |
| `tests/test_fastchess_script.sh` | 89241 | 1124 | 600 | 37608 | 9402 | 43.0 % |
| `src/evaluation.hpp` | 41581 | 153 | 545 | 35199 | 8799 | 86.1 % |
| `tests/test_mate_carry.cpp` | 42004 | 262 | 487 | 31982 | 7995 | 77.7 % |
| `tests/CMakeLists.txt` | 42232 | 210 | 453 | 30519 | 7629 | 73.5 % |
| `tools/plan_prose_check.py` | 55043 | 635 | 464 | 29272 | 7318 | 54.0 % |
| `tests/test_tuner_gradient.cpp` | 59111 | 902 | 382 | 25941 | 6485 | 45.5 % |
| `tests/test_eval_model.cpp` | 76379 | 1314 | 358 | 23123 | 5780 | 31.9 % |
| `adocs/data/S022_v2_sprt.sh` | 23277 | 20 | 342 | 22135 | 5533 | 96.6 % |
| `adocs/data/S145_mate_set.py` | 42771 | 502 | 372 | 21783 | 5445 | 52.4 % |
| `adocs/data/S115_sprt.sh` | 22604 | 20 | 342 | 21495 | 5373 | 96.6 % |

Growth, same lexer at dated commits (`growth.py`) [M]:

| date | `src` code lines | `src` comment bytes | `src` raw share | `tests` comment bytes | `tools` comment bytes |
|---|---|---|---|---|---|
| 2026-08-08 | 4505 | 40767 | 19.3 % | 29637 | 1371 |
| 2026-08-15 | 5376 | 103750 | 33.4 % | 106791 | 55662 |
| 2026-09-01 | 5912 | 173461 | 42.8 % | 239918 | 119259 |
| 2026-09-15 | 6870 | 316136 | 53.7 % | 539979 | 189433 |
| 2026-10-01 | 7413 | 414161 | 58.1 % | 731509 | 249537 |
| 2026-10-15 | 7985 | 451472 | 58.5 % | 853324 | 283859 |
| 2026-10-31 | 9274 | 539585 | 58.9 % | 987650 | 333844 |

The share has plateaued since October at about 58 %, so bytes now grow with the code: +465872 in 30 days.

## 2. What they are

`sample.py`: 163 blocks, seed 20261031, stratified by area and five size bands. `classified.tsv` holds each block's byte split, read by one reader with the code beside it. `analyze_sample.py`: stratum weights, within-stratum bootstrap, 4000 draws, 90 % interval. The interval covers sampling error only. **No second rater**, and "why" against "contract" is a soft boundary.

| class, % of comment bytes | all census | live code | `src` | `tests` | `tools` | `adocs_data` |
|---|---|---|---|---|---|---|
| contract or invariant | 25.3 [19.0, 36.7] | **24.0** [20.9, 27.5] | 32.6 | 14.3 | 37.9 | 26.9 |
| non-derivable why or hazard | 31.2 [28.3, 34.6] | **44.3** [40.8, 47.6] | 42.8 | 45.9 | 41.5 | 14.5 |
| required by a project rule | 20.3 [10.1, 27.3] | **6.1** [4.5, 7.8] | 5.9 | 8.6 | 0.5 | 38.6 |
| history or narration | 21.2 [15.9, 26.1] | **22.0** [19.7, 24.3] | 14.9 | 26.8 | 19.4 | 20.1 |
| pointer only | 0.6 [0.4, 0.9] | **1.0** [0.7, 1.5] | 1.6 | 1.0 | 0.5 | 0 |
| restates the code | 1.2 [0.7, 1.7] | **2.1** [1.2, 3.0] | 1.8 | 3.1 | 0 | 0 |
| stale or wrong (the claim's bytes) | 0.2 [0.1, 0.3] | **0.4** [0.2, 0.5] | 0.5 | 0.4 | 0.1 | 0 |

`adocs_data`'s "required" is pre-registrations and tool-written fit headers.

**Regex cross-check, all 6860 blocks (`pop_scan.py`) [M]:** 30 % of blocks hold a step, decision or invariant id and carry 79 % of the bytes. The id tokens are 1.6 % of bytes, so pointers are cheap and pointer-only comments rare. Blocks naming a golden: 55 in `tests`, 169 KB, 17 % of that area.

**Stale, verified against the code.** 11 of 140 live blocks hold a dead claim: 2.0 % of live blocks weighted by stratum, sitting in 15.4 % of live comment bytes; the dead sentences are 0.4 % of bytes. A floor.

| block | claim | what the code says |
|---|---|---|
| `src/search.cpp:2432` | "the second of them ships switched off" | `X(SE_MULTICUT, "SeMultiCut", 1, 0, 1)` ships on |
| `tests/test_engine.cpp:1116` | "**Golden** (DEC-142): its depth-1 iteration is 25933707 nodes and about 5.9 s" | `build/src/chesso` (`b2bf1c7-dirty`, no `src/` diff to HEAD) answers `nodes 35516352`, 8962 ms. No test holds it. The bound still separates |
| `src/search_params.hpp:813` | "S098 owns moving the rule onto the sum" | S098 closed, the rule still reads the raw entry |
| `src/search_params.hpp:525` | "the build prints 4493659" | bench is 5707449 |
| `src/search_params.hpp:72` | cites `src/evaluation.cpp:33-37` | `ORDER_COUNTER` is on line 39 |
| `src/evaluation.cpp:1118` | "clang still inlines it", "the zero weights still fold" | g++ is the reference since DEC-049; `passed_pawn_mg` is `{9, -5, -2, 21, 38}` |
| `src/data_structures.hpp:584` | "evaluate() is material plus tapered tables" | mobility, king safety and pawn terms too |
| `tests/test_search.cpp:1203` | "S065 is a pending 827-constant paste" | S065 is done |
| `tests/test_search.cpp:4275` | "an H0 on S234 flips `RfpTtEstimate`" | S234 closed, it ships at 1 |
| `tools/timer_race_stress.py:2` | "AGENTS.md par.12" | no numbered paragraphs since DEC-109; `tests/test_uci_surface.cpp:14` is the same class |
| `fastchess.sh:565` | "as rating.sh:72-77 already does" | those lines count cores |

Population probes, noisy, including history told on purpose [M]: 96 mentions of paths gone from the tree in live code, 24 `file:line` citations in live comments, 5 "S### owns" sentences about closed steps.

**Examples per class.**
- Contract: "The cheapest attacker of `color` in `attackers`, as a one-bit board, plus which piece it is. Empty when that side has run out of attackers." (`src/bitboard.cpp:1357`). "Refuses an unknown name and a value outside the declared range, and returns false in both cases rather than clamping" (`src/search_params.hpp:1376`).
- Why or hazard: "Digits and not int(), which also takes `-5` and `1_000`." (`tools/spsa_driver.py:426`). "**The floor belongs to the product and not to either factor.** [...] 30 % of 30 % is 9 % of the allocation" (`src/chesso.cpp:1403`). "Not `piece_values_abs`: that is the ordering band table [...] a pruning margin wired to it would be a back door into it." (`src/search.cpp:1097`).
- Rule: "GOLDEN (DEC-142): the pair 65024 and 3251 [...] Re-derive: python3 adocs/data/S192_node_budget.py" (`tests/test_search.cpp:1203`). "Seed **144, form (b)**" (`src/search_params.hpp:445`).
- History: "With S125's four coverage rows (92) at its zero-weight landing it answers +30 still: two rows bound [...]" (`tests/test_tuner_gradient.cpp:632`, one of ten such paragraphs in one block). "It was 2 for as long as S236's history term was in the tree" (`tests/test_mate_carry.cpp`).
- Pointer: `... S037.` at the end of a reason (`src/chesso.cpp:1284`). Almost never alone.
- Restates: `// Board manipulation` (`src/bitboard.hpp:27`), `// king` (`src/eval_tables.hpp:129`), `# 3. What came back.` (`tests/test_bare_fen_relay.py:212`).

## 3. Are old ones worse?

`blame_ages.py` runs `git blame -w` on all 417 files in 23 s; `age_analysis.py` reads it [M]. The branch is 12 weeks old, so "old" means August.

| area | bytes | pre-`achesso` | 2026-08 | 2026-09 | 2026-10 |
|---|---|---|---|---|---|
| `src` comment | 539585 | 2.3 % | 23.3 % | 38.9 % | 35.5 % |
| `tests` comment | 987650 | 0.2 % | 18.9 % | 42.8 % | 38.1 % |
| `tools` comment | 333844 | 0.0 % | 23.4 % | 35.0 % | 41.6 % |
| root and cmake comment | 72782 | 0.3 % | 40.7 % | 38.0 % | 21.1 % |
| `adocs_data` comment | 1506848 | 0.0 % | 8.1 % | 40.2 % | 51.7 % |
| all comment | 3440709 | 0.4 % | 15.8 % | 40.2 % | 43.6 % |
| all code | 3500292 | 3.6 % | 21.1 % | 31.0 % | 44.4 % |

Comment bytes are no older than code bytes. In `src`, code is the older: 25 % predates this branch.

| tercile by blame median, 140 live blocks | median age | contract | why | rule | history | stale blocks | mean block bytes |
|---|---|---|---|---|---|---|---|
| newest | 18 days | 30 | 30 | 10 | 28 | 2 of 46 | 1065 |
| middle | 47 days | 19 | 41 | 9 | 28 | 4 of 47 | 1340 |
| oldest | 75 days | 24 | 53 | 2 | 19 | 5 of 47 | 555 |

Spearman with age, permutation p: history -0.11 (0.19), why +0.18 (0.035), contract -0.20 (0.018), restates +0.09 (0.27), **stale +0.04 (0.66)**, block bytes -0.22 (0.009). Old blocks are shorter and more often a plain reason, the opposite of the hypothesis. Dead claims concentrate in long blocks (9 of 36 against 2 of 104) and in blocks several commits touched (9 of 39 against 2 of 101).

**Rewrite caveat, tested.** For 24 random blocks, the first commit holding the opening words (`git log -S`) differs from the blame median by 0 days in nearly every case, more than 7 days in 1 of 24. That checks only the opening words; later paragraphs are often younger (`tests/test_mate_carry.cpp`'s S202 and S261 paragraphs).

## 4. "Tanto c'è git"

**Nothing is destroyed.** A deleted comment stays reachable with `git log -S` if the reader knows what to search for. What is lost is discovery at the line.

**Agents do not go there [M, `git_usage.py`].** Of 25004 Bash commands: `git log -S/-G/-L` 44, `git blame` 1, `git log --grep` 36. They read comments (11.4 MB of comment bytes) and step files.

**Is it in the record?** `recover.py` indexes `plan_done`, `plan_todo`, `audit`, `specs`, manuals, each decision by entry, all commit messages and `adocs/data` text, 2244 documents, over 113 sampled live blocks with history plus why at 20 % or more.

| exact facts (numbers, backticked strings) | by the id it cites | anywhere | nowhere |
|---|---|---|---|
| all 493 | 84.0 % | 96.6 % | 3.4 % |
| history-heavy, H at 40 % or more, 26 blocks | 93 % | 97 % | 3 % |
| why-heavy, W at 60 % or more, 47 blocks | 71 % | 95 % | 5 % |

Sentence matching is fuzzy: a sentence counts as found if one paragraph holds that share of its content words. The control is 12 random step files. Read this as a range.

| threshold | by id | with effort | only the comment | control false-match rate |
|---|---|---|---|---|
| 0.60 | 56.9 % | 42.0 % | 1.0 % | 10.0 % |
| 0.75 | 24.8 % | 61.2 % | 14.0 % | 2.3 % |
| 0.85 | 13.4 % | 47.1 % | 39.5 % | 1.2 % |

**Hand test, 25 blocks, one distinctive fact or reason each, grep over the record.** Chosen by me: an illustration, not an estimate.

| outcome | n | blocks |
|---|---|---|
| found by the id it cites | 10 | B025 S104, B057 S278, B062 S117, B079 S097, B080 S021, B100 DEC-240 and S242, B101 S193, B112 S091 and S282, B124 S095, B142 S167 |
| found with effort | 8 | B050 in S132, B052 in S099, B060 in S263, B076 in S097, B114 in S130, B131 in S172, B145 in S179, B155 in S084 |
| only the comment | 7 | B038 the SEE bar, B039 "Stepping by two", B040 `isalpha`, B094 the 4.5 ms sweep, B134 split on single spaces, B161 the `all` typo, B059's 8.50 and 2.75 ns |

**A number with an id behind it is in the step file; a local reason usually is not**, because a step file records what was decided and measured, not why this one line is written this way.

**Can a pointer replace the text?** [M]

| what the pointer leads to | size |
|---|---|
| documents a sampled block cites, in full | mean 100648 B against a mean block of 1497 B; ratio median 44x, quartiles 25x and 92x, worst 980x |
| one `plan_done` file | mean 22982 B, largest 196218 B |
| one decision entry | median 3283 B, largest 13890 B |
| one measured agent access to `plan_done` | mean 5.8 KB per result, 381 results |
| one measured agent access to `decisions.md` | mean 4.2 KB per result, 601 results |

A followed pointer costs at least 4 to 6 KB; the median block is 153 B. Break-even is a follow probability under block bytes / 5 KB [E]: about 3 % for 150 B, 30 % for 1.5 KB, never binding for a 7 KB log. **A pointer can replace a long log, never a short reason.** The first loses the story in place; the second loses the reason, and the reader must know to go looking.

## 5. Where tokens really go

`transcripts.py`, `transcripts_report.py`, `options.py`. Script only; contents treated as data; only paths, ids and counts leave.

**Corpus [M].** 262 transcripts, 16 main and 246 subagent; this study's own excluded. 2026-08-13 to 2026-10-31, no main session from 08-15 to 09-29. 28371 requests deduplicated by `requestId`. 94 % of records `claude-opus-5-5`.

| | tokens |
|---|---|
| uncached input | 85604 |
| cache write | 213769516 (5 m 190745791, subagents; 1 h 23023725, main) |
| cache read | 7213280540 |
| output | 9950727, thinking 3416282 of it |
| context processed | 7427135660 |
| weighted, input-token units | 707502869 (main 110655732, subagents 596847136) |

**Caching, honestly.** Weights from the `claude-api` skill: read 0.05x on Opus 5.5 and 0.1x elsewhere, write 1.25x at 5 m and 2x at 1 h, output 5x. Read/write ratio 33.7. Raw counts a token once per request it sits in, so it is throughput, not cost. Per request the context is split by category from what has arrived, reset at compaction: a new item is a write; an old item a read, or a write in the proportion that request actually re-wrote (a miss after a long wait). A comment token read at the midpoint of a median implementing agent (132 requests) costs about 1.25 + 66 x 0.05 = 4.6 units and is counted 67 times in raw.

**Share of consumption, calibrated [E on measured inputs].** Comments x1.50, thinking x1, the rest x1.74 (2.3 bytes/token; per-category medians 1.5 to 2.8 in `transcript_tables.txt`). Rows close at about 92 %; the rest is mostly compaction summaries.

| group | raw | weighted |
|---|---|---|
| command output (engines, scripts, git, builds) | 16.5 % | 15.8 % |
| the agent's own text, tool inputs, edits | 12.9 % | 13.7 % |
| reminders and attachments | 10.8 % | 10.6 % |
| base context, 39 K tokens per subagent (system, tools, AGENTS, CLAUDE, memory) | 15.8 % | 10.0 % |
| `adocs/` documents | 8.5 % | 8.2 % |
| code read, not comment | 8.2 % | 7.5 % |
| **comments in code read** | **8.1 %** | **7.4 %** |
| output tokens at 5x | | 7.0 % |
| data and run files | 5.0 % | 4.8 % |
| thinking | 3.5 % | 3.4 % |
| root documents | 1.7 % | 1.8 % |
| briefs and reports | 1.5 % | 1.6 % |

No single source dominates. Command output and the agent's own output are each about twice the comments; comments are about the size of all `adocs/`. Of the 7.4 %, live code is 5.75 %; `adocs/data` scripts are 23 % of comment reads and diffs about 2 %, untouched by a cleanup.

**How code is read [M, `read_modes.json`].** Whole-line count, so a floor.

| mode | calls | bytes | comment bytes | share |
|---|---|---|---|---|
| Bash `sed`, `head`, `tail` window | 3013 | 12741899 | 6388159 | 50 % |
| Read with offset or limit | 744 | 3273752 | 1922324 | 59 % |
| Bash grep | 2106 | 3615957 | 1191226 | 33 % |
| Read whole | 101 | 2026840 | 914814 | 45 % |
| Bash cat and other | 312 | 1890874 | 949686 | 50 % |

**This caps the saving: 73 % of comment bytes arrive through a window.** A shorter file puts more code in the same window; an agent asking for 100 lines still gets 100. So: **low 0.27** (only whole reads and greps shrink), **mid 0.63** (half the windows shrink, an assumption), **high 1.00**. Nothing measures which is right until a cut exists.

**A typical step [M].** 87 steps attributed by the id in the subagent's description, 241 subagents of 3 requests or more. Median: 2 subagents, 158 requests, 26.4 M tokens processed, 2.90 M weighted units. **Mean 6.81 M units**, 75.0 M tokens: a few steps dominate (S023 59 M, S099 57 M). Coordinator on top: 1.27 M units per step, comments 0.66 % of it.

Live-code comments, real tokens: **5.4 % of weighted (IQR 2.4 to 8.4)**, 5.4 % of raw (IQR 2.1 to 8.3); aggregated 6.7 % and 7.1 %. About 21 K comment tokens enter the context per median step. By subagent kind, chars/4, median of weighted: implementing 5.2 %, closing 4.4 %, reviewing 4.0 %, exploring 5.7 %.

**What a cut saves, median step, subagents only [E: measured share x cut x bracket]:**

| cut | low 0.27 | mid 0.63 | high 1.00 |
|---|---|---|---|
| 30 %, weighted units | 12.5 K, 0.43 % | **29.6 K, 1.02 %** | 46.7 K, 1.61 % |
| 30 %, tokens processed | 115 K, 0.43 % | 271 K, 1.02 % | 427 K, 1.61 % |
| 60 %, weighted units | 25.1 K, 0.86 % | **59.2 K, 2.04 %** | 93.3 K, 3.21 % |
| 60 %, tokens processed | 229 K, 0.87 % | 541 K, 2.05 % | 853 K, 3.23 % |

With the coordinator's share, percentages fall by about 30 %: 0.71 % and 1.42 % at mid. New comment tokens per median step fall by about 6 K and 13 K before the bracket. At Opus 5.5's $4/MTok a median step is about $11.6 and a 60 % cut at mid saves about $0.24; the corpus is about $2800 at that rate, to size it, not as a bill.

**Writing comments.** Edit and Write inputs into code files: 2.49 MB, 50 % comment lines, 309 K tokens at chars/4; at 5x output 0.22 % of weighted [M, a floor: edits through Bash not counted]. Writing costs little. Keeping them true costs more.

## 6. Effects other than tokens

**Harm.**
- **A comment that lied about behaviour, high severity:** `adocs/audit/2026-09-10_adversarial.md` F04. `rating.sh`'s comment says both harnesses adjudicate resign two-sided; `fastchess --help` says it defaults to false. The comment justified the setting. S212 fixed it.
- **A false precondition open 69 days:** `2026-08-13_adversarial-F05`, `LAZY_EVAL_MARGIN`'s "king safety ships at zero weight", carried by `2026-09-03_adversarial`, `2026-09-04_adversarial`, `2026-09-04_plan_review`, closed by S287 on 2026-10-21.
- **Steps that exist for stale comments:** S213 (F26, F27, F33), S245, S249, S287, S254 (mutant strings). 28 `plan_done` files mention a stale comment, 36 times. 14 of 17 audit files use the word, 95 hits.
- **Every close pays a comment pass:** S123 and S125 `brief_close.md` "the comments and definitions become true for what ships", S136 "Comments made false by H0", S296 "Comments and documents the H1 makes false". 7 of 44 briefs have an explicit pass, 25 of 44 mention comments. S276 left known-stale `src/` comments for a separate commit (`e7b1d04`).
- **Nothing checks a comment's truth.** `tools/plan_prose_check.py` has five modes, all over documents: `--prose`, `--citations`, `--touches`, `--params`, `--gate`. It blanks comments before symbol matching. The drifted golden above shows the result.
- **Churn [M, `deps_scan.py`, exact, lexing both images], last 150 commits per area:**

| area | comment lines added, removed | code lines added, removed | comment share of changed lines | comment-only commits |
|---|---|---|---|---|
| `src` | 9648, 3873 | 6020, 2519 | **61.3 %** | 15 |
| `tests` | 20730, 6420 | 38968, 7514 | 36.9 % | 13 |
| `tools` | 6764, 2136 | 13182, 3671 | 34.6 % | 8 |

**Benefit.**
- **Agents are told to read them.** `.tuning/coord/S289/brief.md:60` "the long comment above it (its oracle, how it was found, why the hazard belongs to the mated side)". `S121/brief_inputs.md:11,32` the comment over `mobility_mg`. `S125/brief_inputs.md:127`. `S123/brief_inputs.md:193`. `S083/brief_reland.md:40`.
- **A named hazard that mattered.** `S292/brief.md:39`, `adocs/plan_done/S292_narrower_null_move_mate_guard.md:24`: "The guard's own comment named a way it could cost more than predicted". That hypothesis was what S292 measured, and its H1 guard (DEC-310) is that condition. `adocs/plan_done/S125_pawn_structure_completion.md:801`: "The site's comment already held the class".
- **Tombstones stop retries where they would happen:** `src/search.cpp:1842` S237, `:2030` S235, `:1404` S022's gate with `nElo -85.89 +/- 22.70`. History by class, functional. A compressed line keeps them.
- **The recurring bug class is guarded in prose:** guard tests carry mutant tables and observed-red records (`tests/test_search.cpp:7787`).

**Reading.** The harm comes from length and append-logging, not comments as such. Short reasons rarely go stale and are often held nowhere else.

## 7. What depends on comments, and breaks

| dependency | what it reads | breaks on a comment edit? |
|---|---|---|
| DEC-142 golden sites | 34 GOLDEN marks, 55 blocks in `tests/`; no tool checks the form | a review item. The rule asks name, command, margin, not a log. Blocks hold 169 KB; the form is about 500 B each, about 27 KB [E] |
| `tools/gate.sh` | commit message, built binary's bench | no. `src/` needs `Bench: <n>` or `No functional change`, checked against the parent |
| `plan_prose_check.py --citations` | 18 symbol and 2 phrase citations in pending files | symbols match with comments blanked. Phrases can land in a comment: **0 of 2 do** (`check/citations_in_comments.py`, the checker's own recogniser). 0 flagged over 17 files |
| `tools/mutation_check.py`, `tools/mutants/*` | 244 anchors in 32 files, all once at HEAD | **5 overlap a comment**: M36, M03, TC04, K06, NB01. On `s118`, 2 of 30. Refuses loudly, but only at next run; `tests/test_mutation_check.py` uses fixtures, so no ctest holds the real anchors; S254 is the precedent. Keep those lines or move the anchor in the same commit |
| `tests/test_fastchess_script.sh` | greps `fastchess.sh` | no, all hit code (`/proc/loadavg` at 913, 914) |
| `tests/test_uci_surface.cpp` | `MANUAL.md` | no |
| `adocs/data/S121_verify_fit.py`, `S135_screen.py`, `S192_anchors.py`, `S275_drift.py` | strip `//` before reading numbers | no |
| `clang-format.sh --check`, in the gate | wraps at 80 columns | **yes, trivially**: the pilot's first draft had 7 and 17 lines over; `clang-format -i` fixed them, code identical. `search_params.hpp`'s comments sit in a macro, each line ending in an aligned `\` |
| line citations | `decisions.md` 21, `DEV_MANUAL.md` 7, `specs.md` 2, `status.md` 2, `testing_strategy.md` 1, `plan_done/` 335 | they decay at every step anyway, by design since DEC-135. `plan_done/` is immutable; `git show` answers its quotes |
| `specs.md:121` | INV-n referenced from code comments | keep the numbers |

**Decisions governing comments, by grep:** DEC-142 goldens at their site. DEC-135, DEC-159, DEC-119, DEC-120, DEC-080 citations, about plan documents. DEC-140 the bench line. DEC-221's "the completion stamp says so" and DEC-134's seed forms live in the stamp and step file; writing the seed form at the site is a strong convention, and I found no rule demanding it. The owner's global `CLAUDE.md`: "Comment why, not what. No comment that restates the line below it." **No rule limits a comment's length or forbids a per-step log.** One block defends its own log: `tests/test_search.cpp:1203`. `DEV_MANUAL.md`'s goldens table (`:2376` on) already holds per-golden readings, so a log in a comment duplicates it.

**A comment-only `src/` commit proves itself** with `No functional change` and the gate: both builds, both suites, bench equal to the parent's. About 5 minutes (your figure; `gate_S202_complete.log` read 103 s per build's suite, older). Precedent: `S123/brief_close.md:118`, "`tools/gate_extra.sh` is not needed: comments only since its green run". `g++ -fpreprocessed -dD -E -P` proves the same in under a second per file, no build; replacing the gate with it is a proposal, DEC-140 does not allow it today. Line numbers move, so `__LINE__` in doctest and Debug `assert` changes. **I did not build to confirm the Release object is identical.**

**S118 conflict [M, `git diff 85b7847 s118`].** Rewrites `src/evaluation.cpp` (483 lines), `src/data_structures.hpp` (84), `src/evaluation.hpp` (12), `src/search.cpp` (12), `tests/test_evaluation.cpp` (511), `tests/test_search.cpp` (123), `tests/test_eval_trace.cpp` (89), `tools/tuner_trace.hpp`, one-liners elsewhere. A comment commit on those files before it lands means a three-way merge in the largest diff in flight, and its mutants anchor there. **Nothing on those files lands before DEC-332 (7), (8) are read.**

**Open steps on the same files, from `touches:`:**

| file | steps |
|---|---|
| `src/evaluation.cpp` | S118, S101, S122, S124, S102, S266, S133, S126, S025 |
| `src/evaluation.hpp` | S101, S122, S124, S102, S266 |
| `src/search.cpp` | S025, S265; S029 and S129 touch `src/` generally |
| `src/search_params.hpp` | S265, S127 |
| `src/data_structures.hpp` | S118 |
| `tests/test_evaluation.cpp` | S101, S102, S122, S124, S133, S266 |
| `tools/tuner_trace.hpp` | S101, S102, S122, S124, S133, S126 |

Only a gap between steps works; steps run in worktree branches, so a comment commit on `achesso` while one is open forces a rebase. The binding memory also applies: no commit in the main checkout during a screen, cost run or SPRT.

## 8. A measured pilot

`pilot/apply_pilot.py` edits scratch copies by line range over HEAD blobs, so no code line is retyped. `pilot/measure_pilot.py` measures, `pilot/lost.py` looks up what went. Rules: section 9's.

| | lines | bytes | comment bytes | chars/4 tokens | code identical |
|---|---|---|---|---|---|
| `src/search.cpp` 1781 to 2089, before | 309 | 18804 | 16498 | 4701 | |
| after | 184 | 9929 | 8034 | 2482 | **g++: identical, 53348 bytes; lexer strip identical** |
| change | -125 | **-47.2 %** | **-51.3 %** | -2219 | |
| `tests/test_mate_carry.cpp` whole, before | 827 | 42005 | 31982 | 10501 | |
| after | 477 | 17955 | 8318 | 4488 | **g++: identical, 9277 bytes; lexer strip identical without whitespace** |
| change | -350 | **-57.3 %** | **-74.0 %** | -6013 | |

Formatter clean after `clang-format-22 -i` on the copies [M]. The test file's 292-line golden block is the extreme: a 19.6 KB log became a 30-line golden form. The search section is the representative one, being reasons and hazards.

**What was lost.** Exact facts in removed text [M]: search section 11, all in a document the block cites. Test file 68: 63 in a cited document, 5 not found as exact strings (4 worst-cell counts such as "13 of 64", in sweep files in another form, 1 lexing artifact). In words: the per-step story of each ceiling's rises (S109, S095, S236, S116, S261, S286, S083, S135, S202), S291's "90 returns", S213's correction of the comment itself, why the futility site is unswitched, old budget values. All in step files and decisions, none at the line.

**Real questions.**

| question, where asked | compressed copy |
|---|---|
| `search.cpp`: which entries stop reverse futility, why not one from a skipped-move node? S293's `accepts` and brief | **answers**: "upper bound or exact score inside the mated band from a node that searched every legal move ... From a skipped-move node it is not a proof, S292" |
| `search.cpp`: does razoring read `RfpMinPly`, what breaks below it? S251's `goal` | **answers**: "ply floor RfpMinPly, reverse futility's own floor, DEC-248. Without it 9 of the 26 S145 mates in two were found an iteration late" |
| `test_mate_carry`: what does a step do when a ceiling moves? `S290/brief.md:33,48`, `S121/brief_close.md:117` | **answers**: the command, "never reading one off a failing run", "Raising one is relaxing a test: DEC-299's guard ... or the owner's decision" |
| `test_mate_carry`: who kills MW02 now? `S294/brief.md:21` | **answers**: "mutant MW02's killer is tests/test_mate_pv.cpp's cold cell (S294, ...)" |
| not from a brief: why did C's ceiling rise at S116? | **does not answer**. DEC-250 or `git log -S` has it |

**The errors, which are the finding.** My first draft held **two real errors**: a fabricated pointer, "(DEC-204 class)", on S237's tombstone, and a wrong denominator, "0 of 14589403 calls disagreed" for 0 of 3368027 read-backs. Identity checks cannot see either. Only re-reading old against new caught them. One sample, about 1100 source lines, but **compression by an agent can do what stale comments do: state a false thing confidently.** It needs a reader of both versions.

**Cost of the pilot [E].** Read 61 KB, about 24 K real tokens; wrote about 16 KB, 6 K real tokens. With a 39 K base that is about 2.9 weighted units per comment byte, solo, no review. Measured anchors: median review agent 0.33 M units, median implementing agent 2.7 M, though the latter has build loops this work lacks. So a batch of about 75 KB of comments costs **about 0.8 to 1.8 M units [E]**.

## 9. Options and recommendation

Fractions of comment reads from `read_files.json`; uniform within a file for B1. Cut within scope from the pilot: 55 % for reason-dense text, 75 % only by deleting reasons. Savings are % of weighted at low / mid / high. "Open plan" is 15 steps at the mean, about 100 M units.

| | A: rule only | B1: 72 log-shaped blocks | B2: 16 files by traffic | B3: all live code | C: aggressive |
|---|---|---|---|---|---|
| scope | new comments | 294 KB, 15 % of live, 16 % of reads | 669 KB, 45 % of reads | 1934 KB, 73 % of reads | 1934 KB, contract and hazard only |
| bytes removed [E] | none, stops about half of 15.5 KB/day | 162 KB | 368 KB | 1064 KB | 1450 KB |
| saving % [E] | 0 now | 0.18 / **0.41** / 0.65 | 0.50 / **1.16** / 1.84 | 0.80 / **1.88** / 2.97 | 1.09 / **2.57** / 4.05 |
| per median step, mid | 0 | 12 K units | 34 K | 55 K | 75 K |
| per mean step, mid | 0 | 28 K units | 79 K | 128 K | 175 K |
| cost, units [E] | under 0.5 M | 3.3 to 7 M, 4 batches | 6 to 13 M, 7 batches | 17 to 37 M, about 20 | 15 to 30 M |
| in median steps | 0.2 | 1 to 2.4 | 2 to 4.5 | 6 to 13 | 5 to 10 |
| machine, 5 min a gate | 5 min | about 20 min | about 35 min | about 1 h 40 m | about 1 h 40 m |
| payback, mean steps at mid | immediate, by growth avoided | 120 to 250 | 76 to 165 | 134 to 291 | 86 to 172 |
| saved in open plan vs cost | | 0.4 M vs 3.3 to 7 M | 1.2 M vs 6 to 13 M | 1.9 M vs 17 to 37 M | 2.6 M vs 15 to 30 M |
| days of current growth | | 10 | 24 | 68 | 94 |
| dead claims repaired [E] | none | about 30 of 72 (4 of 10 in the sample) | those, and more | about 100 | about 100, if reviewed |
| risk | none | 7 anchors at most; golden form; agent error | S101 to S133 conflict window; agent error | long conflict window; agent error at scale | **lost hazards in the recurring mate-hiding class** |
| what is lost | nothing | per-step stories at the line, all in step files | also dated measurements | also local history | reasons held nowhere else, 7 of 25 in the hand test |

**Recommend A, then B1. Not B2, B3 or C in phase one.**
- **Tokens do not justify a sweep.** No option pays back inside the open plan. The owner should hear that plainly.
- **The rule beats any sweep on tokens.** Halving growth avoids B1's whole removal in 3 weeks for almost nothing, and only the rule also cuts the 61 % churn and the comment passes.
- **B1 is justified by defects, not bytes.** The selector (at least 1200 B, at least 6 distinct ids, at least 1.5 ids per KB) picks 10 of 140 sampled blocks; 4 of those hold a dead claim, against 7 of the other 130. History is 40 % there, 21 % elsewhere. It is mechanical, and these are the blocks each step appends to. It costs about one mean step. A alone is also sound.
- **Revisit B2 at the phase boundary**, when the plan's length is known and B1 has measured the pass-through.

### The plan, as steps

Ids are yours. All agent-only except the gates. Each comment step's brief says: `adocs/data/` and `plan_done/` are not touched, and a review reads old against new.

**Step 0, now, decision only.** Owner decides under QUESTIONS (`DEC-nnn`). Why asked: no rule limits comments, and a rule is the owner's. Options: A only; A then B1 (recommended); A then B2.

**Step 1, `tools/comment_budget.py`. Any gap, independent of S118.**
- Scope: this study's lexers, productised, with `tests/` fixtures. `--report <ref>` flags new or grown blocks over 15 lines, new id chains, log-shaped blocks. `--identical <ref>` proves code identity (g++ for C++, tokens for Python, lexer for shell and CMake).
- Accepts: `lexer_selftest.py`'s fixtures as a fast ctest; `--identical` observed red on an injected code change and green on `pilot/`; `--report` on HEAD lists the 72; DEV_MANUAL entry. No dependency.
- Verify: both suites, formatter, 5 min. Report only, not a gate: a red would block a legitimate long specification.

**Step 2, the rule in `AGENTS.md` and the briefs.** After step 0; documents only, any time.
- Accepts: the COMMENTS line below; `--gate` still equal; the close-brief item reads "replace, never append"; Tier-1 fast check runs `--report` over a step's diff.

**Step 3, tests' golden logs. After S118 is decided. No `src/`.**
- Scope: log-shaped blocks in `tests/test_mate_carry.cpp`, `test_mate_breadth.cpp`, `test_engine.cpp`, `test_search.cpp`, `test_tuner_gradient.cpp`, `test_eval_trace.cpp`.
- Accepts: DEC-142's form at every golden with current value, command, what moves it, margin; **the 25933707 figure re-taken by its command** (a defect now, DEC-171 filler class; check whether it should hold a number at all); `--identical`; `DEV_MANUAL.md`'s goldens table holds whatever a removed log held that no step file does; S124's and S101's test files checked for conflict first.
- Verify: gate, no `Bench:`, 5 to 10 min.

**Step 4, `src/` dead claims and log-shaped blocks. In the gap right after S118's landing or refusal, before S101 starts.**
- Scope: section 2's dead claims in `src/`; log-shaped blocks in `search_params.hpp`, `evaluation.cpp`, `evaluation.hpp`, `search.cpp`. Seed forms and hazards kept, short reasons untouched.
- Accepts: `No functional change` per commit, `--identical`; bench and `search_bench` identical; every anchor found once across `tools/mutants/` and `S118_pawn_cache.py`; S102's `evaluate_pawns` citation resolved; `--citations` 0 flagged; `specs.md` untouched unless a claim leaned on one.
- Verify: 2 or 3 commits, 15 min; a clean fast check reading old against new. No Debug self-play and no `gate_extra`, on S123's precedent; the owner may rule otherwise.

**Step 5, `tools/` and root scripts. No run active.**
- Scope: `tools/plan_prose_check.py`'s 13.8 KB header (`DEV_MANUAL.md` holds its rules), the "list is data" paragraph repeated in 32 of `tools/mutants/`, `fastchess.sh`, `tests/test_fastchess_script.sh`.
- Accepts: suite, `--gate`, `--citations` green. My reading is that DEC-143's A/A is not owed since no harness behaviour moves and `--identical` proves it; that reading is the owner's to confirm.

**Step 6, stop and measure, after 5 further steps.** Re-run `transcripts.py`, compare comment tokens per step and the pass-through, record the verdict, zero included.

### Proposed rule for `AGENTS.md` (the owner's to take; a `DEC-nnn` to record)

> - COMMENTS: **a comment says what the code cannot, about the code as it stands.** Keep a contract, an invariant, a hazard, a reason that cannot be derived, and a number that decides something. Present tense. A new reading replaces the old one in place; what moved it goes in the step file and the commit. **No per-step log, in a comment or a docstring**: a block that would gain a paragraph for this step is rewritten instead, and a close's comment pass replaces and never appends. A rejected idea keeps one line at the site where it would be re-added: what, verdict, id. One id per claim, at its end, and an id never stands in for a reason the reader at that line needs. A golden keeps DEC-142's form and its current value: name, command, what moves it, margin. A block over 15 lines is a specification: it goes in the file's header or the step file, or says why not. `adocs/data/` and `adocs/plan_done/` are evidence and are not cleaned. Rewriting a comment reads old against new: an agent's compression can state a false thing, and code identity cannot see it.

### What stops regrowth

1. The rule, and `--report` in every Tier-1 fast check, so a violation is a finding on the step.
2. Close briefs saying "replace, never append".
3. `DEV_MANUAL.md`'s goldens table as the place a reading goes; a comment keeps only the current one.
4. A monthly line in `status.md` from `count_comments.py`: live comment bytes per area. Unmeasured, this drifts unseen, as it has.
5. A hard ratchet is not proposed: `leaf_walk.hpp`'s header is legitimately long, and a length gate trains agents to split blocks.

### What would make the cleanup not worth doing

**For tokens alone that result is already in: no sweep pays back inside the open plan.** For B1 on its other grounds, stop if:
- after step 4, comment tokens per step fall under 2.4 % of baseline over 5 steps (the low bracket for B1's 8.7 %), and no stale-claim yield is left to justify step 5;
- a clean reader finds more than one meaning-changing error per 1000 comment lines after review (the pilot's first draft ran about 2 per 1100 source lines before review);
- comment-commit conflicts cost any open step more than the batch saved;
- a removed reason is re-derived or its defect re-introduced in the following steps. One such case outweighs the batch.

## Not measured

- **Pass-through**, the largest uncertainty, 0.27 to 1.00. Only a real cut measures it.
- A second rater on the classes.
- Whether agents do better or worse with fewer comments. Fewer wrong turns against more re-derivation could outweigh every token figure here, either way.
- Tokens spent on comment passes as such: Bash-driven edits are not separated, so 0.22 % for writing is a floor.
- Sessions before 2026-08-13 and main sessions of 08-15 to 09-29: not on this machine.
- Release object identity after a comment edit: no build allowed.
- Staleness beyond grep and reading. Goldens were not re-derived, one number apart. `adocs/data` staleness not assessed, being a record.
- Option costs are model-based: no batch was run for real.

## Files

Under `/home/max/ws/chesso/.tuning/coord/comment_study/`: `comment_lexer.py`, `lexer_selftest.py`, `count_comments.py`, `files.tsv`, `blocks.jsonl`, `build_report.py` (prints the census tables), `sample.py`, `sample.jsonl`, `sample_dump_*.txt`, `classified.tsv`, `sample_shares.tsv`, `analyze_sample.py`, `pop_scan.py`, `pop_scan.json`, `blame_ages.py`, `blame_by_week.tsv`, `age_analysis.py`, `growth.py`, `recover.py`, `recover.tsv`, `git_usage.py`, `transcripts.py`, `transcripts_report.py`, `transcript_rows.jsonl`, `transcript_tables.txt`, `token_regression.json`, `read_modes.json`, `read_files.json`, `options.py`, `log_shaped.py`, `log_shaped.json`, `deps_scan.py`, `deps_scan.json`, `check/`, `pilot/`.
