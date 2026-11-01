# Adversarial audit of the comment plan, second run, at `ca3ec71`

Scope: `adocs/comment_strategy.md`, its fix-or-leave evaluation, what it cites and depends on, and verdicts on `2026-11-01_comment_plan`'s 15 findings. The plan is unexecuted. Tools named in it are judged by S311's and S312's `accepts` at `ca3ec71`. Experiments ran in scratch clones of `ca3ec71`, Apple clang through `/usr/bin/c++`, clang-format 23.1.4, bash 3.2.57, Python 3.9.6, on battery, load about 1.5 of 10 cores. Nothing was run in the repository that writes. `tools/comment_budget.py`, `tests/test_comment_budget.py` and `adocs/data/S312_local_gate.sh` appeared untracked, and `DEV_MANUAL.md` and `tests/CMakeLists.txt` modified, in the working tree during this run, none by this audit and presumably S311's agent; they are not in the audited commit and were not read.

Verdict, for the reader who stops here: **five medium, seven low, no high.** S316's "no high and no medium" does not hold yet. The mechanical proof holds up hard: T1 deleted whole, held blocks aside, keeps `bench`, the fast suite, the anchors and S101's pause check. The defects are in what T1 relies on beyond that: S101's path on H0 (F01), how seeds reach the tree (F02), a claim of no false text from deletion with nothing measuring it (F03), T2's gate (F04), and the cost cells (F05). F01 needs an answer before S308 lands; F02, F03 and F04 before T1's calibration batch. Fix-or-leave itself: O1's cost now includes F01, O0 stays defensible, and the choice of O2 remains the stated judgement, with no number in this report overturning it.

## Findings

### 2026-11-01_comment_plan.2-F01  medium  On S101's H0 or no verdict, a `src/` comment commit leaves no tree both part scripts accept, and the plan says a comment commit reaches nothing
Status: closed

**Evidence.**
- `adocs/comment_strategy.md:17`: "REF stays `4e34e61`, so a comment commit reaches neither the SPRT's REF nor what it plays." True of the family run. The same plan's section 1 has S101 resuming straight after.
- DEC-335, Consequences: "On H0 the bisection's parts are not re-read here: `S101_screen.sh apply` compares its tree with the main checkout's HEAD, so a part needs that guard read against this split first." No step holds that reading: `git grep -nE "that guard|this split|apply.*main checkout|main checkout.*apply" -- adocs/plan_todo adocs/plan_current adocs/plan.md adocs/status.md adocs/comment_strategy.md` prints nothing.
- `adocs/data/S101_screen.sh:666` requires the tree at the main checkout's HEAD. `adocs/data/S101_sprt.sh:345-354` refuses a REF whose `src/` differs from `screened`: part A has no `carries`, so any difference fails; parts B and C allow `src/evaluation.cpp` and nothing else. Reproduced, main checkout standing in as `ca3ec71` plus T1's in-scope blocks deleted and committed, a worktree at `4e34e61`:

```
$ OUT=<scratch> bash adocs/data/S101_screen.sh apply <worktree at 4e34e61> <empty file>
refused: <worktree> is not at HEAD, the tree the fit started from
S101-APPLY-FAILED
$ git diff --name-only c2f2025 HEAD -- src cmake CMakeLists.txt
src/chesso.cpp src/data_structures.hpp src/eval_tables.hpp src/evaluation.cpp src/evaluation.hpp src/search.cpp src/search_params.hpp
```

  A tree at REF passes `S101_sprt.sh` and fails `apply`. A tree at `achesso` passes `apply` and fails `S101_sprt.sh`. At `ca3ec71` itself that `git diff` is empty, so today a tree at HEAD passes both: documents-only commits are safe. S308 is the first commit that closes the door, and its row in `src/data_structures.hpp` is enough.
- Base rate: `adocs/plan.md`, "Of the 23 verdicts from S231 to S055, 14 were H0 or no verdict". `S101_sprt.sh`: up to three part runs follow.
- Section 3's table charges no option for this. O0 keeps both guards satisfiable; O1 and up do not.
- Run: `apply`'s guard and that `git diff`. Read, its paths being the workstation's: `S101_sprt.sh`'s guard. `part` needs the workstation's corpus.

**Impact.** After a family SPRT of up to 19.86 h reads H0 or none, the bisection stops at its first `apply` until a pre-registered script changes. DEC-335 (1) says `S101_sprt.sh` is unamended, and S312 excludes existing files under `adocs/data/`. The fix is small; the risk is that it lands on an unattended path, on the likelier outcome, with no owner, and that the owner compared O0 and O1 without it. No game, score or move moves.

**Suggested resolution.** Add the cost to section 3. Name a step that reads the guard on the workstation before S308 lands, and state the amendment: `apply` comparing code-stripped `src/`, or the part's tree staying at REF with that guard waived by decision.

### 2026-11-01_comment_plan.2-F02  medium  A writer's edit list is read off a seeded copy and applied to the unseeded ref, and no step says how it is translated
Status: closed

**Evidence.**
- `comment_strategy.md:75`: seeds are "each substituted in place so no line moves", in "the writer's copy of the old text". `:73`: the writer "emits an edit list". S311: `--apply <edits>` "takes character ranges against a ref". `grep -ci seed adocs/plan_current/S311_comment_rule_and_tool.md` reads 0. S312 has one clause, "seeds in the writer's and the reader's copies, none in the tree". A sentence in a wrapped comment starts and ends mid-line, so deleting whole sentences needs the character form.
- A seed that keeps its line but not its byte length shifts every later offset. Reproduced on the calibration file, `tests/test_mate_carry.cpp`, seed `A 5 -> 6` to `A 15 -> 6`, the writer deleting a later sentence by its range in the seeded copy, that range applied to the ref:

```
lines moved by seed: 0
writer deletes     : "S292's narrowed null-move guard moved none, nor did S293's r"
ref bytes removed  : "292's narrowed null-move guard moved none, nor did S293's re" ... 'futility table guard. '
left behind in tree: " 1 and F 1 -> 2,\n// DEC-295. S|S083's refit moved B 5 -> 6 an"
lexer identity     : True
range inside comments: True
```

  The range touches no code token, so `--apply` as S311 states it passes it, and every proof arm is blind to comment text.
- A seed the writer catches is worse. Its register says "dead claim" and its list deletes the sentence, whose true original stands in the tree. The more seeds it catches, the more true sentences the list removes, each looking like a correct deletion to the reader. S312 counts "deleted false sentences" as dead claims with no subtraction, against F04's threshold of 15 with up to 12 writer seeds in the pool.

**Impact.** Either a mangled comment or a removed true sentence, in exactly the batches measuring recall, through a gate that cannot see either. The first is likely found by a reader; the second is not, since it reads as an ordinary deletion. Avoided only if someone translates each list by hand, and no record says who or how.

**Suggested resolution.** State the mechanism in S311 or S312: seeds of equal byte length, or edits expressed against the ref by a script that maps seeded offsets back; any edit touching a seed dropped before `--apply`, its true text kept; seeds subtracted from the register by id. Add a fixture.

### 2026-11-01_comment_plan.2-F03  medium  "Deletion writes no false text" is asserted with no measured rate, and neither seeds nor stops read the errors deletion makes
Status: closed

**Evidence.**
- The claim: line 47's cell, new false sentences "golden forms only", and line 55, "writes no free text: the rate that makes a rewrite dangerous does not apply". The same plan, line 73: "A deletion that leaves a pronoun or qualifier dangling is a false sentence too." Every rate on record is a rewrite's: S306's 2 in 12, the pilot's 2 in 1136. No deletion rate exists.
- How often it can arise. Over T1's in-scope 65 blocks, 3827 lines (F07 for the held 11): comment text stripped of markers, split on sentence ends, 1316 sentences; 654 name an id, 49.7 %; 46 others open on an anaphor or connective (`This|That|It|Its|They|So|Either|Same|...`) directly after an id-naming sentence. That is 12 per 1000 lines against the stop's 1 per 1000. Upper bound: it needs every id-naming sentence gone. Of 10 drawn at random, 9 lean on the sentence before and one follows a bare `DEC-064.`. Real ones: `tests/test_engine.cpp:1148`, "**Not a golden**, and S306 took that mark off ... It is a dated reading of what the bound is judged against"; `src/search_params.hpp:631`, "So \"no move\" reads as ..."; `tests/test_search_params.cpp:64`, "S114 added three: ...", then "Its first verdict's H0 took the margin and the cap out again". A careful writer deletes the dependent too, which is then a fact lost by cascade. Whole-sentence and "no new word" leave only those two choices.
- Golden is the channel that writes: 10 in-scope blocks hold the mark, 66883 B, 26.3 % of scope bytes, including the largest, `tests/test_mate_carry.cpp:379-670`. At the plan's 1 in 6 that is about 2 false sentences. O3 and O4 get a number in that column; O2 does not.
- Seeds. Line 75: "Three in the reader's copy of the diff." Line 77's first question: "Is each sentence of the new in the old or the code". On a deletion-only diff, a seed added on the new side is the one sentence not in the old, and a `diff` finds it, so recall passes whatever the reader can do. A seed on both sides passes that question. The form is unstated. The named natural class, "a true fact attached to the wrong term", is E01, a paraphrase error. There is no seed for a dangling reference, a dropped qualifier or a removed reason, so line 77's third question, "which removed fact is in no record", has no measure at all.
- Stops. Line 100's "removed reason re-derived" can only fire in later steps; S313 is item 14 of `plan.md`, behind nine other steps after S312. Line 97's residual is read "Over T1's pool", after T1 has landed, so it stops T2's question and no T1 commit. Only seeds and cost can stop T1 midway.
- Nothing mechanical bounds this. T1 deleted whole, `fastchess.sh` and its test aside, 73 blocks: `bench` 5707449, Release fast suite 49 of 49, all 274 anchors once. In T1's favour, by exact string, which is a weak test: 296 of 299 numbers of 4 or more digits or with a decimal, distinct per in-scope block, appear in a tracked document or a commit message, and the other 3 are hyperfine times at `src/evaluation.cpp:372`. So numbers are mostly safe, and reasons are unmeasured.

**Impact.** The plan's deciding reason for T1 over T2, and its reason for a low false-text cell, rests on a rate never read. A deletion batch can pass seeds with a reader that checks nothing about deletion, and the owner reads that as safety.

**Suggested resolution.** State line 47's cell as unmeasured, and read a deletion rate from the calibration batch before the next. Specify seed form: on both sides of the diff, plus deletion-class seeds, a dangling pronoun and a removed unique reason. Give the residual an interim read after the second batch. Put a number on golden forms.

### 2026-11-01_comment_plan.2-F04  medium  T2's gate is "15, half the estimate", and by the plan's own figures 15 is above what the register can show if the estimate is exactly right
Status: closed

**Evidence.** Line 57 and DEC-337 (2): T2 goes to the owner only if "T1's register records at least 15 dead claims in its 76 blocks, half the estimate", otherwise "recorded as not worth it and is not asked". The estimate is 76 x 4/10 at `5954c12` (`report.md:434,441`). By the time the register is read, on the plan's own order:

| step | expected remaining | source |
|---|---|---|
| the estimate | 30 | `report.md:441` |
| S306, landed after the sample | 25 | `benefit_rows.json`: B63, B65, B66, B69, B70 in log blocks |
| held, 17.5 % of T1's bytes | 21 | F07 |
| S308, ordered before T1 | 19 | B78, B82 in log blocks |
| script pass, ordered before T1, at most | 10 to 19 | in-scope T1 hits: 3 `file:line`, 3 gone paths, 3 forward claims |
| writer recall 0.67, the plan's own reference | **7 to 13** | line 98 |

The opposite biases are also unstated. The study's `stale` column flags a block (`classified.tsv`, B059 holds two claims in one flag), and the register counts claims. Seeds are not subtracted (F02). The estimate's own 95 % interval, Wilson on 4 of 10, is 13 to 52 blocks, so 15 sits at its floor. And a count of T1's dead claims says nothing direct about T2's value: T2's estimate comes from different sampled blocks, and the tier holds 11 of 31 helped rows.

**Impact.** This rule is the plan's answer to the prior audit's first high finding. As written its likely outcome is "not worth it, not asked", whatever T2 holds, and it will read as a finding from evidence when it is arithmetic from what ran first. "Not asked" also closes the one route back to T2 that DEC-337 leaves: "its case can wait for T1's register".

**Suggested resolution.** Restate the threshold against what steps 2 and 3 and the held list leave, divide by T1's measured writer recall, subtract seeds, count blocks as the estimate does, state the interval. Or drop the gate and put the register in front of the owner either way.

### 2026-11-01_comment_plan.2-F05  medium  "At most" fails on the plan's own anchors, the 19 M stop sits inside its range, and the payback omits the cost the plan says is omitted
Status: closed

**Evidence.**
- Lines 47 and 51: T1 costs "at most" +3.4 to 7.3 M, "an upper bound for deletion, which reads as much as a rewrite and writes less". The rate is `report.md:415`, a 75 KB batch at 0.8 to 1.8 M, anchored on one median review agent at 0.33 M. DEC-337 (3) added a second reader over all of T1, a register requiring the writer to check truth against code, which the pilot's 61 KB read did not, and seed preparation. What writing less saves: the pilot wrote 6 K real tokens, 30 K units at 5x, about 37 K per 75 KB, 2 to 5 % of the batch rate. One second reader at the anchor is 9 times that. Held blocks cut the other way, 17.5 % of T1 until S101 closes. Nothing nets them.
- Line 99, "T1's scope ceiling is 19 M with the coordinator at its top": 14.3 + 4 x 1.27 = 19.38 before any second reader, 20.7 with four at 0.33 M. F03 of the prior audit named this shape.
- Line 51's "+1.3 to 5 M on T1's 4 batches": a third of 1.27 M over 4 batches is 1.69, not 1.3. The 27-batch figure on the same line is right.
- Line 53's payback, "18 to 136 days", uses 3.4 and 7.3, the cells line 51 says exclude the coordinator. With line 51's own figures: (3.4 + 1.69) / (0.52 x 0.368) = 27 days, (7.3 + 5.08) / (0.22 x 0.244) = 231 days.
- The tool. O0 is "the rule, `--report`" at 1 to 3 M, "the size of one small step". S311's `accepts` holds `--report`, `--identical` over four languages with a `declare -f` arm, `--apply`, `--new-facts`, `--anchors`, `--tiers`, fixtures, a test and a local gate with observed reds. All but `--report` exist for T1. The study's median implementing agent alone is 2.7 M, its mean step 6.81 M. No basis is given, and the excess belongs on O2's line.

**Impact.** Same size as the prior F04: the one payback favouring T1 reads about 1.5 to 1.7 times short, and the owner's figure for O2 reads as a ceiling when it is not. The stop will fire inside the range or be raised on the day.

**Suggested resolution.** Drop "at most". Add the second reader, the truth check and the coordinator as cost lines, unknown where unknown. Move S311's T1-only modes to O2. Set the ceiling above the corrected range, or say it is meant to fire. Correct 1.3 to 1.7.

### 2026-11-01_comment_plan.2-F06  low  `--identical`'s lexer arm stays red on a whole macro block deleted by the form S311 now allows
Status: closed

**Evidence.** Line 83, S311: "the lexer's stripped text ... blank lines aside". S311's `\`-only clause sits on `--apply` alone, its macro and trailing-comment fixtures are `--apply`'s, and `--identical` need only be "green on `adocs/data/comment_study/pilot/`", which holds no macro. On the T1-deleted tree against `ca3ec71`:

```
changed 29  lexer strip, blank lines aside: red 1 ['src/search_params.hpp']
lexer strip, all whitespace aside: red 0
```

First T1 block, `src/search_params.hpp:72-100`, character range over the comment: `-(38 spaces)\`, `+(40 spaces)\`. Whole lines including the `\`: the line is gone. Each form is green with `\`-only lines dropped.

**Impact.** Fails safe, red on correct work, in T1's largest file. Found as a tool change on the day if S311's fixtures do not pipe `--apply`'s output through `--identical`.

**Suggested resolution.** One clause in S311: the macro fixture also passes `--identical`, whitespace-only and `\`-only lines read as blank there.

### 2026-11-01_comment_plan.2-F07  low  The held list has no owner once S312 closes
Status: planned

**Evidence.** S312's `accepts`: "the held blocks and goldens the machine could not re-derive are listed here, for after S101's close and for the workstation". DEC-335 (3): "Whatever has to stay is S312's after S101's close". `plan.md`: S312 is item 4, S101 item 5. `AGENTS.md`: `plan_done/` is never edited. No open step names the list, and S313 excludes further cleanup. Size, re-measured: 11 of 76 T1 blocks, 53717 B, 17.5 %, 820 lines: `fastchess.sh` 2, `tests/test_fastchess_script.sh` 1, S101 overlap 6 (`test_eval_trace.cpp:106`, `test_mate_pv.cpp:406`, `test_search.cpp:4849`, `test_tuner_gradient.cpp:511,650,987`), rows' blocks 2 (`test_search.cpp:4395` for row 2376, `:4815` for row 2377). The plan's table and T2's gate still say 76. S312's `closes` names the prior F06, and closing it leaves those rows and blocks with nobody.

**Impact.** S312 either stays open across up to 79.4 h of S101 or closes with a list in an immutable file that nothing reads. The largest log blocks in `test_search.cpp` stay as they are, and the reason the plan gives for T1 stays partly unmet without being recorded.

**Suggested resolution.** A step in `plan_todo/`, behind S101, owning the list and the three `DEV_MANUAL.md` rows.

### 2026-11-01_comment_plan.2-F08  low  Section 6's first test reads noise at 5 steps, the same defect the plan names for tokens
Status: closed

**Evidence.** Line 120: the rule fails if it does "not halve 15.5 KB a day or lower the comment share of changed lines over 5 steps, read as bytes and shares and not as tokens per step, which 5 steps cannot resolve". Read from `git log`, 575 code commits grouped into 206 runs by the step id in the subject, whole-line comments by marker, heuristic, no rule in force:

```
net comment bytes per run: mean 8083, sd 13617
5-run windows under half the long-run mean: 59 of 202, 29.2 %; over the last 90 runs 24.4 %
5-run windows with comment share below the overall 0.411: 106 of 202, 52.5 %; p10 0.273, p90 0.531
```

S313's `accepts` says 5 steps cannot resolve tokens per step and says nothing of bytes.

**Impact.** A coin flip reads "share lowered" and one window in four reads "halved" with no rule at all. The verdict S313 records can go either way on nothing.

**Suggested resolution.** State the steps a halving needs at this variance, or read the comparison over a named longer window and call the 5-step reading a look.

### 2026-11-01_comment_plan.2-F09  low  O2's saving is the study's rewrite figure, unchanged for deletion and the held blocks, and T1 has no exit criterion
Status: planned

**Evidence.** Line 47: "0.41 %, 0.4 M" is `report.md:425`, B1 at a 55 % cut, 72 blocks, with free paraphrase and goldens. DEC-337 forbids paraphrase and keep is the default (line 73). `report.md:441` puts history at 40 % of the selected blocks. At a 40 % cut and 82.5 % of bytes in scope: 0.41 x 0.40 / 0.55 x 0.825 = 0.25 %, about 0.25 M, level with O3's row at 0.24 %. S312's `accepts` asks a register line per block and nothing about what is left: a T1 of all keeps passes, and no reading says `--report` stops listing a block as log-shaped.

**Impact.** Immaterial beside 6 to 14 M and the plan does not rest on tokens. But nothing would show whether T1 did what it is for.

**Suggested resolution.** Re-derive the cell for the form chosen. Have S312 record bytes removed per block, and how many blocks remain log-shaped by the selector.

### 2026-11-01_comment_plan.2-F10  low  "That workflow already catches this class" is a numerator with no denominator
Status: closed

**Evidence.** Line 113: "22 of the 26 B rows written in the step that found them name a fast check, so that workflow already catches this class." Same-step rows are found by definition. `benefit_rows.json`: 37 A and B rows false at birth, 19 same-step, 18 not, those including 6 of 6 A. Fast checks are recorded from 2026-08-13 (`adocs/worklog.md:2607`), and A01, A03, A04 and A05 were born after that. Against the finding: Tier-1 entered `AGENTS.md` at `45ec005`, 2026-08-19, and of the 18 only A05 has a known birth after it, 5 being unknown. Younger text has also had less time to be found. So the record does not settle it either way.

**Impact.** The plan's only statement about the harmed class leans on this. Read the other way, the 2.2 % is mostly early text and the case for any new control is weaker, which also goes unsaid. Line 113's three write-time controls do not reach the A rows as written: the reader clause applies to "A rewrite" and A rows were new text, and the rule's form bars 0 of 6.

**Suggested resolution.** Replace the clause with those figures, and say no option in section 3 changes what reaches the harmed class beyond Tier-1.

### 2026-11-01_comment_plan.2-F11  low  The best argument for cleaning is about file density, and T1 barely moves density where most steps work
Status: closed

**Evidence.** Line 39 gives the correlation and calls it unproved, fairly. Re-run: `src` Spearman 0.70, within one commit 0.58; `tests` 0.55, 0.35 (plan 0.61, 0.34; dense half 62.6 % over non-blank added lines, against the plan's 51.6 %). It does not say what T1 does to the x-axis. Whole-line `//` density, all in-scope T1 blocks deleted, a ceiling: `tests/test_search.cpp` 0.396 to 0.385, `src/search.cpp` 0.510 to 0.484, `src/chesso.cpp` 0.294 to 0.280, `src/data_structures.hpp` 0.500 to 0.486, `tests/test_engine.cpp` 0.292 to 0.251, `src/evaluation.cpp` 0.354 to 0.269. Only `tests/test_mate_carry.cpp` (0.589 to 0.165) and `tests/test_mate_breadth.cpp` (0.584 to 0.140) change class. At OLS slope 1.16, `src/search.cpp` at the ceiling is about 3 points of comment share.

**Impact.** Read beside line 55's choice of O2, line 39 suggests T1 is the cleaning that argument favours. Even if causal, T1 hardly touches it. T2 and T3 would, which the same section rules out.

**Suggested resolution.** One sentence with those numbers.

### 2026-11-01_comment_plan.2-F12  low  Plan and record disagree in places a briefed subagent will trip on
Status: closed

**Evidence.**
- Line 5: "Figures print from `benefit_report.py` and `options_reach.py`". Neither prints 18 to 136, 3.0 to 6.7, 85 to 185, 8 to 13 or the cost and yield columns (`grep` on both). `options_reach.py` needs a `blocks_head.json` no line says how to make; `census.py HEAD blocks_head.json` does it. "201 s plus 161 s ... [M]" still has no log.
- Line 63: "An implementer's report quotes what its own prompt says about comments." `grep -ci prompt` on S311's file reads 0, so F11 of the prior audit's remainder rests on the brief.
- S312's `Why`: "Compress the logs, leave the reasons", against "no paraphrase". S308's `excludes`: "waits for the owner's decision on the comment study", and its last line, "If the owner takes the comment study's cleanup, this folds into its step", against DEC-337 (1), which rules on it and keeps S308 apart.
- Line 115's integration has no owner for `--report` in REVIEW's fast check, "replace, never append" in close briefs, or the monthly `status.md` line: `git grep -i "monthly\|replace, never append"` over the plan directories prints nothing. S311's `accepts` covers the rule's text and a manual entry.
- S311's `accepts` says "`DEV_MANUAL.md`'s entry outside S101's hunks; a `DEV_MANUAL.md` entry", twice.
- Line 65's "23 `file:line`, 28 mentions of 9 gone paths" are candidates and hold non-dead items: `tests/doctest/...` three times, a submodule path `git ls-tree` cannot see, and `src/x.cpp` twice, a fixture. Of the closed-step forward candidates by `deadref.py`'s regex, 29 of 37 read "S###'s own", not "owns". The plan does say a candidate is read; S312's goal still calls them "the script's dead references".

**Impact.** Each costs a wrong turn or a stop, none moves a score, a move or a line.

**Suggested resolution.** Correct line 5 and the two step files. Give line 115's items a step or drop them.

## Verdicts on `2026-11-01_comment_plan`

Status lines there are for the coordinator to move. Re-measured from each finding's reproduction.

| finding | verdict | read |
|---|---|---|
| F01 | closed | Line 57 says S313 cannot decide T2, S313's `accepts` says it, "only cut" is gone. The rule that replaces it: F04 here |
| F02 | closed, remainder carried | Columns added, "lowest review risk" withdrawn, deletion evaluated. O2's cell and the unmeasured deletion rate: F03 here. The write-time clause: F10 here |
| F03 | closed, remainder carried | 21.2 to 46.2 M, 3.0 to 6.7, 85 to 185 $ all re-derive; coordinator stated; ceiling per scope; instrument owned by S312, and adapted in a scratch copy it runs on this machine, one directory at a time. Second reader, ceiling inside the range: F05 here |
| F04 | closed, remainder carried | 18 and 136 re-derive; O0 baseline stated as unknown. Coordinator left out: F05 here |
| F05 | open, planned at S311 | Reproduced: a conflicting edit committed in a throwaway linked worktree prints `S101-PAUSE-FAILED`, so the design works. S311's `accepts` carries it. No tool at `ca3ec71` |
| F06 | open, planned at S312 | Overlap re-stated. Held list sufficient: all 65 other T1 blocks deleted and committed, `S101-PAUSE-OK`. Owner after close: F07 here |
| F07 | open, planned at S311 | Reproduced unchanged: line 1419 lexes as a comment, a changed count passes. `fastchess.sh` and its test held |
| F08 | open, planned at S311 | Reproduced. `--apply` half in S311; `--identical` half not: F06 here |
| F09 | open, planned at S312 | Reproduced: `src/chesso.cpp:1129` says 36165, the build reads `nodes 49231`, and S308 now lists it. Column relabelled, seeds in the writer's copy, S313 owns the sampling. How seeds reach the tree: F02 here |
| F10 | closed, remainder carried | Denominator, pool, 100 % second read, interval, natural class all there. 6 %, 2 %, 39 %, 11 %, 83 %, 36 % re-derive exactly. Seed form and the post-hoc residual: F03 here |
| F11 | closed, remainder carried | Reading in the plan with its confounds. Prompt read: F12 here. Effect on density: F11 here |
| F12 | open, planned at S311 | Reproduced: both clangs rc 0 on the backslash form. 0 `//` comments end in `\` at HEAD by the lexer, so the check starts green. A plain grep would false-flag 6 URLs inside `/* */` in `src/search_params.hpp`, so it must use the lexer |
| F13 | closed | Line 29 names the four symptom rows' costs and the single rater; its figures match `benefit_incidents.md` |
| F14 | closed, remainder carried | Seven code files, `adocs/data/` wording, one rule for a false golden, 76 in `status.md`, content match in S311. `S101_sprt.sh`'s header stays, being evidence, and DEC-335 (4) is the later word. Others: F12 here |
| F15 | partly closed | `options_reach.py` prints 22.5, 21.8, 55.7, 67.6, 53.6 and 65.4 exactly. The rest does not print: F12 here |

## Re-measured and held

So nobody re-checks them. Census at `ca3ec71`: 164 files, 163 with a block, 4941 blocks, 1969706 B, 31355 lines. Tiers 76 / 307577, 228 / 496002, 4637 / 1166127. `src/`, `tests/`, `tools/`, `fastchess.sh`, `rating.sh` identical to `4e34e61`; `src cmake CMakeLists.txt` identical to `c2f2025`. `clang-format.sh --check`: exactly `src/data_structures.hpp:639,640`.

**T1 deleted whole, 73 blocks in 29 files, `fastchess.sh` and its test aside:** Release build 17 s, `bench` 5707449, fast suite without `test_fastchess_script` 49 of 49 in 136 s, 262 mutants and 274 anchors all once, `tools/plan_prose_check.py` and `tools/mutants/S202_mate_walk.py` parse, `declare -f` identical on all four changed shell files. Format flags beyond the two lines came only from three T1 blocks carrying `// clang-format off` (`test_engine.cpp:4323`, `test_search.cpp:10608`, `test_search_params.cpp:30`), which the directive check covers, and one run of blank lines over `MaxEmptyLinesToKeep`. Tune build and Debug not run on that tree. **The same minus the 11 held blocks, committed:** `S101-PAUSE-OK`.

**"Where steps append" holds.** Blame by tier: T1 holds 37.8 % of comment bytes dated within 7 days of HEAD, against 15.6 % of all comment bytes. Growth over the last 90 step runs: 15.2 KB a day, against the plan's 15.5.

**Other readings.** S308's ten rows: none sits in an S101 hunk; three of its files hold hunks. By grep, `tests/test_fastchess_script.sh` reads only `fastchess.sh`'s text, which is held, so excluding that test hides no comment dependency in `rating.sh`, which `test_rating_script` runs anyway. No document or open step cites a code line number (`git grep` over `DEV_MANUAL.md`, `MANUAL.md`, `TOOLCHAIN.md`, `specs.md`, `plan.md`, `AGENTS.md`, `CLAUDE.md`, the plan directories: 0), so T1's moved lines falsify none; `--citations` 0 of 26 files. Pointers from documents into comments that hold history: rows 2376, 2377 and 2385 only. Four `__doc__` readers as named. Lines 29 to 37's tallies match `benefit_rows.json`: A 6, B 110 with 75 drift and 43 of form F, script 48 and reader 66, A's script 1 and reader 5, D's kinds, helped rows 3, 11 and 17 of 31 by tier. Line 53's arithmetic other than F05's. Measured for scale, `transcripts.py` adapted, read about 12:43: this comment work to date, study, plan and audits, is 12.1 M units in `-Users-max-ws-chesso-build` and 0.72 M in the other directory, S311's agent still running and this one included; each subagent there 0.83 to 1.82 M.

## Could not confirm

- The literature: no web access in this run. The plan's sources are unchecked again.
- Major 22's and g++'s opinion of anything: neither is on this machine.
- `S101_screen.sh part` past its guards, and how S101's parts would actually run after a guard change: `.tuning/` and the corpus are on the workstation.
- A deletion's real error rate: no deletion batch has run. F03's 46 is a count of places, not of errors.
- What an implementing subagent's prompt says about comment density.
- Whether the study's one-rater classes survive a second rater.
- Whether F02's translation has been handled inside S311's in-progress files, which are outside the audited commit.
