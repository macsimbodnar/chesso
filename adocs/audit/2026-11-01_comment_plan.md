# Adversarial audit of the comment plan, at `b949eea`

Scope: `adocs/comment_strategy.md`, its evaluation of fixing the old comments against leaving them, and
what it cites. The plan is unexecuted, so a tool named below that does not exist yet (`comment_budget.py`,
`S312_local_gate.sh`) is judged by what S311 and S312 accept. Experiments ran in a scratch clone at
`b949eea`, with `tests/doctest` copied in, Apple clang 21 and clang-format 23.1.4, on battery, load 1.6 to 3.5
of 10 cores.

**A side effect of this audit, on the record.** I ran `adocs/data/comment_study/literature/benefit_incidents.py`
in the repository to read its validation. Its docstring, line 11, says it writes `benefit_rows.json` beside itself,
and it rewrote that tracked file: 4 `line` values for `status.md` rows, +1 each, since `status.md` moved after `dcd3dae`.
I restored that one path with `git restore --worktree`; `git hash-object` equals `HEAD`'s blob, `85f5567`, and `git diff` is
empty. Copy that directory before re-running it.

Verdict, for the reader who stops here: O1 survives, and O0 is defensible as the plan says. The recommendation's
reasons for T1 do not hold as written (F01, F02, F04), though F11 finds the unmeasured case for cleaning has more behind
it than the plan knew. Four parts of the proof design fail on this tree as it stands (F05, F06, F07, F08).

## Findings

### 2026-11-01_comment_plan-F01  high  T1 cannot measure pass-through or imitation for T2's decision, which is the recommendation's deciding reason
Status: closed

**Evidence.**
- `adocs/comment_strategy.md:59`: T1 "is the only cut that can measure pass-through and the imitation argument for T2's decision"; the recommendation is "O2, measure, then decide T2". The only instrument is S313: "five steps after S312".
- Noise: `adocs/data/comment_study/report.md:312`, live comment share per step "5.4 % of weighted (IQR 2.4 to 8.4)", from 87 steps. SD about 6.0 / 1.349 = 4.45 points.
- Signal: `report.md` section 9, B1 saving "0.18 / **0.41** / 0.65" % of weighted. Separating pass-through 0.27 from 1.00 is a difference of 0.47 points.
- Arithmetic: 5 steps against 87, SE = 4.45 x sqrt(1/5 + 1/87) = 2.05 points; detectable at 80 % power, 2.8 x SE = 5.7 points, 14 times T1's mid effect. For 0.41 points, 2 x (2.8 x 4.45 / 0.41)^2 = 1845 steps per arm. 272 steps took 91 days.
- Confound: the rule enters with S311, before T1, so every "after" step has both. `CLAUDE.md` rule 6: "Two at once and neither number means anything." S313 has no arm holding one fixed.
- Imitation has no instrument at all: S313's `accepts` names comment tokens, pass-through and bytes per area, none of which separates a file teaching logs from the rule forbidding them.

**Impact.** The owner is told T1 buys the measurement that decides T2. S313 will return a number inside its noise, and T2 then gets decided with no more evidence than today, after T1's 3.4 to 7.3 M. A zero read from S313 will look like a verdict on pass-through when it is a verdict on 5 steps.

**Suggested resolution.** State T2's decision rule now, by a quantity that can be read: defect evidence from T1's register, or a replay of recorded line windows against old and new trees for a per-file pass-through. Or say plainly that S313 cannot decide T2. Drop "only cut that can measure" from the recommendation.

### 2026-11-01_comment_plan-F02  high  The option table counts dead blocks repaired gross, never nets the false text T1 writes, and "lowest review risk per dead block" contradicts its own columns
Status: closed

**Evidence.**
- `benefit_incidents.md` origin table: A rows birth 6, drift 0. The plan's own line 33: "all 6 false when written, none from drift". So all recorded harm sits on the class a rewrite produces, and the class T1 repairs is the one with "no recorded harm" (line 34).
- T1 is the one tier allowed free paraphrase (line 72), at "about 1 in 6 before a reader" (line 53). Expected false text in T1 before a reader, from the plan's two rates: 76 / 6 = 12.7 by S306's rewrites; 4647 comment lines (re-measured, T1's 76 blocks) x 2 / 1136 = 8.2 by the pilot's.
- Against that: "about 30" dead blocks and "none clearly" harm averted (line 53). No column nets the two. The earlier report did make this comparison: `literature/report.md:169`, about 142 false if everything were rewritten, at least 50 on long blocks alone, against about 95 dead, "Senza lettore il repo può non diventare più vero". The plan dropped it.
- Risk per dead block, by the plan's figures: T1 12.7 / 30 = 0.42. T2, deletions and replacing only, 43 x 1/6 / 43 = 0.17. T3 the same. T1 is the highest, not "the lowest" (line 59). Cost per dead block also favours T2: 0.10 to 0.22 M against 0.11 to 0.24 M.
- The after-review stop allows 1 per 1000 comment lines (line 89), which is 4.6 fresh false sentences on T1's lines. A reader of recall 2/3 to 1/2 leaves 3 to 6 of the 8 to 13, so the stop tolerates about what is expected.
- No option targets the harmed class. 5 of 6 A rows need a reader (line 38), the rule bars the form of 0 of 6 (line 42), and every option above O0 spends on old text.

In T1's favour, and also not in the table: of located rows, log-shaped blocks hold 19 of 78 A and B but 3 of 31 D; T2 holds 25 of 78 and 11 of 31 (`benefit_rows.json`).

**Impact.** The owner compares options on a column that can only grow with spend. By the plan's own data T1 trades text with no recorded harm for text of the only class with recorded harm, at a rate the stop tolerates, and the recommendation describes that as the lowest risk.

**Suggested resolution.** Add a "false sentences expected, before and after reader" column and a helped-rows column. Evaluate T1 as deletion plus golden form with no free paraphrase, which DEC-336 (5)'s own reason supports and which the table does not offer. Evaluate an option aimed at write-time truth for new comments. Correct or withdraw "lowest review risk".

### 2026-11-01_comment_plan-F03  medium  The whole-scope cost omits its own table's rows and the coordinator, the 37 M stop sits inside the estimate, and the instrument that reads it has no owner
Status: closed

**Evidence.**
- Line 57: "O4 is 18 to 39 M". Lines 51 to 55 sum to 21.2 to 46.2 M: 1+2+3.4+4.5+10.3 and 3+4+7.3+9.6+22.3. 18 to 39 is T1+T2+T3 alone. At the plan's rates the corrected total is a net 3.0 to 6.7 mean steps and about 85 to 185 $, not 2.6 to 5.6 and 70 to 155 $. The recommended O2 is cumulatively 6.4 to 14.3 M, and the table prints only the increment.
- The coordinator: `report.md:310`, "Coordinator on top: 1.27 M units per step". The batch rate is writer plus review anchor (`report.md`, section 8), and the plan's coordinator runs 27 apply, prove, check, commit cycles with seeded copies and a 10 % second reader. Counted at 1.27 M a batch that is +34 M; at a third, +11 M. The real figure is unknown, and it is in no estimate.
- The stop: DEC-336 (7) "past 37 M"; line 89 "the scope's ceiling", undefined if the owner takes O2. 37 M is below the plan's own top of 39 and 46.2.
- The projection: "at the second batch" comes after calibration and one T1 batch, rewrite-class work. T3's read-only rate is by the plan's own words unknown ("at most"), so that projection cannot read 59 % of the bytes.
- The instrument: line 68 "units per byte from this machine's transcripts". `adocs/data/comment_study/transcripts.py:28` is `ROOT = "/home/max/.claude/projects/-home-max-ws-chesso"`. On this machine sessions split by working directory into `-Users-max-ws-chesso` and `-Users-max-ws-chesso-build`. Neither S311's nor S312's `accepts` nor `touches` owns a port, and S313 reads "Workstation only".

**Impact.** The owner decides on a figure about 15 % low before the coordinator is counted. The stop either fires inside the predicted range or is read from an instrument nobody has built.

**Suggested resolution.** Fix line 57's sum. Add the coordinator as a row, unknown if unknown. Define the ceiling per scope, and project T3 separately. Name the step that ports `transcripts.py` before the calibration batch.

### 2026-11-01_comment_plan-F04  medium  "T1 pays back in 13 to 66 days" assumes half of C-class work where the plan's own rows give a quarter to a third, and takes no baseline for the rule
Status: closed

**Evidence.**
- Line 47: "At an assumed half, T1 pays back in 13 to 66 days". The arithmetic is right: 3.4 / 0.26 and 7.3 / 0.11.
- `benefit_incidents.md` "Length": 19 of 78 located A and B rows are log-shaped, 24.4 %. In the C steps themselves, from `benefit_rows.json`: S245 2 of 3, S247 0 of 1, S249 0 of 1, S287 0 of 1, S306 5 of 13, so 7 of 19, 36.8 %.
- At those shares: 3.4 / (0.52 x 0.368) = 18 days to 7.3 / (0.22 x 0.244) = 136 days.
- Baseline: every option holds the rule, and O0's rule already forbids the appends and requires fixing a touched false comment (line 51). The saving is credited wholly to T1 with no subtraction for what O0 alone removes. `report.md` section 9 says "The rule beats any sweep on tokens".
- The rate's top end is already marked "likely high" (line 35), and S213's 650 lines include dead API and a test (`benefit_incidents.md`, answer 4).

**Impact.** The one payback figure that favours T1 is about twice too short at its upper end by the plan's own rows, and it is measured against no action rather than against O0, which the plan calls defensible.

**Suggested resolution.** Replace the assumed half with the located share, and state payback as T1 over O0, marked unknown where it is.

### 2026-11-01_comment_plan-F05  medium  `S101_pause_check.sh` reads `HEAD`, so a proof run before the commit cannot see the batch
Status: closed

**Evidence.** `adocs/data/S101_pause_check.sh:15`: `git worktree add -q --detach "$tmp/w" HEAD`. In a scratch clone at `b949eea`:

```
$ sed -i '' -e '2377s/$/ AUDIT/' DEV_MANUAL.md
$ bash adocs/data/S101_pause_check.sh | tail -1
S101-PAUSE-OK
$ git add DEV_MANUAL.md; git commit -qm scratch
$ bash adocs/data/S101_pause_check.sh | tail -1
S101-PAUSE-FAILED
```

The plan requires both "A red proof: the commit does not exist until its cause is read" and "`S101_pause_check.sh` red: the commit gives way" (line 89), and lists the check under "Proof at every commit" (line 85). `AGENTS.md` GIT: "No history rewriting". S311's observed-red requirement for that gate is "an injected code change", which identity catches first.

**Impact.** Run before the commit, the check reads the parent and each conflict is reported one commit late, against the wrong batch, or not at all for the day's last commit. Run after it, red means a landed commit that needs a revert commit, so "gives way" costs a second commit. DEC-335 (3) rests on this.

**Suggested resolution.** `S312_local_gate.sh` runs the check on a candidate commit it creates in a throwaway worktree, without editing the evidence script, which already works from `git rev-parse --show-toplevel`. Add to S311's `accepts`: observed red on an uncommitted edit that conflicts.

### 2026-11-01_comment_plan-F06  medium  S101's overlap is counted over code files only, and S312's own required edits to `DEV_MANUAL.md` are inside one S101 hunk
Status: planned

**Evidence.**
- Line 20 and `s101ov.py` count "13 blocks ... in seven code files"; the script filters to `src/` and `tests/`. Hunks in the diff, old side: `DEV_MANUAL.md` (1504,1510), (2030,2037), **(2355,2389)**, (3823,3829), (6405,6411); `MANUAL.md` (231,237); `adocs/data/README.md` (1551,1553).
- S101's hunk rewrites 21 rows of the goldens table. S312 `accepts`: "what a removed log held that no step file holds goes into `DEV_MANUAL.md`'s goldens table, and its three rows that say history stays in a comment are corrected". Plan line 101 repeats it. Those rows are 2376, 2377, 2385.
- Observed: `AUDIT` appended to row 2376, 2377 or 2385, each alone and committed: `S101-PAUSE-FAILED`. All three together print `conflicts with S101's paused diff ... @@@ -2355,35 -2355,35 +2355,49 @@@`. A row appended at the end of `adocs/data/README.md` for a new data file: `S101-PAUSE-FAILED`. A `DEV_MANUAL.md` line added at 2400: `S101-PAUSE-OK`.
- Row 2377 says "the case keeps the row's history in comments where it stood". That block is `tests/test_search.cpp:4815-4847`, 2106 B, in T1 by the selector and outside S101's hunks (4861 on), so nothing stops it being rewritten while the row cannot be touched.
- `adocs/data/S101_sprt.sh` prices that wait: 12.13 h to 19.84 h for the run, 79.4 h on H0 through the parts.

**Impact.** Taken literally, T1 makes `DEV_MANUAL.md` false until S101 closes, or S312 cannot complete its `accepts` before S101 closes. A log's residue cannot go where S312 sends it. The plan's description of the overlap gives no warning of either, since line 65's "Not in S101's diff" is true of the new file and the CMake line and not of S311's `DEV_MANUAL.md` entry.

**Suggested resolution.** Re-state the overlap over the whole diff. Decide before T1: hold the blocks those rows name, as S101's other hunks are held, or send a removed log's residue to a place outside the hunk until S101 closes.

### 2026-11-01_comment_plan-F07  medium  The shell identity proof calls code a comment on `tests/test_fastchess_script.sh:1419`, in the one test the local proof does not run
Status: closed

**Evidence.** Line 80: lexer-stripped text "sees every language". S311: "shell and CMake by the lexer", with no second arm. `lex_shell`'s double-quote scan does not know `$(...)` nests quotes.

```python
import sys; sys.path.insert(0,'adocs/data/comment_study')
from comment_lexer import lex, strip_comments
f='tests/test_fastchess_script.sh'; t=open(f).read()
c=[c for c in lex(f,t)[0] if c.start_line==1419][0]
print(repr(t[c.start_off:c.end_off]))
n=t.replace('"$relay_dir/relay_calls")" != 4 ]]','"$relay_dir/relay_calls")" != 5 ]]',1)
print(strip_comments(f,t)==strip_comments(f,n))
```
```
'# S272 smoke relay" "$relay_dir/relay_calls")" != 4 ]] \\'
True
```

So a change to a test's expected count passes the only identity check shell has. Differential against bash's own parser, each script wrapped in a function and printed by `declare -f`, original against `strip_comments`, over the 16 live shell files: 15 print identically. This one's stripped text does not parse: `bash -n` rc 2, "unexpected EOF while looking for matching". `lexer_selftest.py` holds no `$(` case. Line 85 excludes `test_fastchess_script` from the proof, and that line is the assertion S314's table says stops that test on this machine: "expected four isolated (-I) launches". DEC-336 (4) waives DEC-143's A/A on the ground "identity proves nothing moved".

**Impact.** For shell, identity is weaker than the plan states, on the file with no test behind it until the evening. S311's `--apply` would refuse that line, being a code line, so the exposure is a hand fix or any `--identical` over an edit not made through `--apply`. Nothing is known about this file's other 601 comments, since the strip cannot be parsed to compare.

**Suggested resolution.** Add a parser arm for shell as C++ has raw tokens: the `declare -f` comparison, or `shfmt` if the owner takes the dependency. Fix `lex_shell`'s `$(` handling in S311's copy with this line as a fixture. Hold `fastchess.sh` and `tests/test_fastchess_script.sh` until a machine can run the test.

### 2026-11-01_comment_plan-F08  medium  `--apply`'s line ranges and the lexer proof cannot express the whole-block delete in T1's largest file, and the acceptance test never meets it
Status: closed

**Evidence.** T1's largest file by bytes is `src/search_params.hpp`, 15 blocks, 50831 B, 16.5 % of T1, all inside the `CHESSO_SEARCH_PARAMS` macro (re-measured). Each such block ends `*/   \`.

```
macro block comments 33 bytes 95083
last line of block is a code line by the lexer: 33 of 33
delete 51..70: lexer strip identical, blank lines aside: False
delete 51..71: lexer strip identical, blank lines aside: False
replace by one bare backslash line: identical: False
raw tokens identical after deleting block 51..70: True 1031 1031
```

(`lex`, `strip_comments` and `rawtok.py`'s `toks` over the file with those lines dropped; non-blank line lists compared.) S311: `--apply` "refuses a range touching a code token"; a whole-block range holds the `\`, which the lexer counts as code. So a line-range edit cannot remove a block's last line nor its `*/`, and the last sentence of each block is out of reach. If the tool lets `\` through, the lexer arm goes red on a correct delete while the raw-token arm stays green. The `accepts` is "green on `adocs/data/comment_study/pilot/`", and the pilot holds `search.cpp` and `test_mate_carry.cpp`, no macro. Line 72 names the misaligned-backslash trap and not this one.

Same format, other blocks: 334 of 4941 blocks, 8383 B, are trailing comments on code lines, and a line range refuses all of them, so a false one cannot be replaced through the tool.

**Impact.** Fails safe: a red proof stops the commit. But it is found at T1's largest file, after S311 is closed, as a tool change during the day's run, or a writer leaving orphan `\` lines to get green.

**Suggested resolution.** S311 takes character ranges or treats a `\`-only remainder as blank inside `format-off` macros, with a fixture from this file and a trailing-comment fixture in its `accepts`.

### 2026-11-01_comment_plan-F09  medium  The yield columns assume a writer that finds every dead claim, and nothing measures whether it does
Status: planned

**Evidence.**
- Lines 53 to 55 title "dead blocks repaired" with "about 30", "about 43", "about 22", which are population estimates of what exists. T2 and T3 are detection tasks, with keep as the default (line 72).
- Seeds go "in its copy", the reader's, and the reader sees "each changed block" (line 74). A dead claim the writer keeps is seen by nobody. No seed tests the writer.
- A demonstrated miss. `src/chesso.cpp:1129` says "`go depth 1` there reports 36165 nodes now" of `q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1`. Scratch build at `b949eea`, fresh process: `info score mate 1 time 7 depth 1 nodes 49231 nps 6614402 hashfull 49 pv d2h6`. S306 read the same figure false on the same board (`S306_false_comments.md:118`), S308 lists only `tests/test_engine.cpp`, and its `touches` has no `src/chesso.cpp`. None of the study, S306's writer and reader, or S308 names this site, though the number was in hand. None was asked to sweep, so this shows one miss and not a rate. Filler by DEC-171: it moves no score, move or line.
- Same shape as a record: S308 exists because S306's writer and reader kept finding more beside a list of twelve.
- Line 106's "dead claims per KB do not halve" and its override test are not named in S313's `accepts`, which says only "the study's stop conditions read against it".

**Impact.** The yield per million units is an upper bound presented as an estimate, and T3's only yield is this detection. The stop rules cannot see a low detection rate, and neither can the end-of-scope reading.

**Suggested resolution.** Seed known-false claims into the writer's copy of the old text and read its recall per batch. Label the column "dead blocks present". Give the before-and-after sampling an owner. Add this comment to S308.

### 2026-11-01_comment_plan-F10  medium  The stops cannot read their own thresholds
Status: closed

**Evidence.**
- Residual errors: "More than one meaning-changing error per 1000 comment lines after review" (line 89), read from "a second reader on 10 %" (line 74). T1 is 4647 lines. Second-read lines: 465, or 186 at the pilot's 40 % size. With zero errors found, the 95 % upper bound is 6.5 or 16 per 1000, and with one found the stop fires. Certifying 1 per 1000 needs 3000 second-read lines, 30000 changed lines at 10 %, against 31355 comment lines in the tree. The reader's own recall is not in this.
- The denominator is undefined. Over changed lines or over all lines read decides whether T3's 18763 lines allow about 19 more errors. The pilot's pre-review rate, 2 in 1136, is already 1.76.
- Seeds: stop on "two seeds of three in two batches running". By exact binomial, two consecutive flagged batches:

| reader's true recall | flags per batch | stop within 4 batches | within 27 |
|---|---|---|---|
| 0.33 | 0.745 | 0.84 | 1.00 |
| 0.50 | 0.500 | 0.50 | 1.00 |
| 0.67 | 0.255 | 0.16 | 0.77 |
| 0.80 | 0.104 | 0.03 | 0.23 |
| 0.90 | 0.028 | 0.00 | 0.02 |

  A reader at the literature report's own pass line, 2 of 3, stops the job three times in four over the full scope, and a coin-flip reader survives T1's 4 batches half the time. The seeds are "le classi già viste", wrong number, real id, inverted condition (`literature/report.md:205`), and E01's "tapered sum" is none of them, so recall on seeds is not recall on the errors that occurred.

**Impact.** The safety argument of DEC-336's Why, "a reader and a stop at every commit", rests on two numbers that are noise at batch level. The residual stop can fire on one find or pass on none, and neither reading says much.

**Suggested resolution.** State the denominator. Pool over a tier and state the lines needed before the stop is read. Second-read 100 % of T1, the only tier that writes free text. Report seed recall with its interval, and add a natural-error class such as a true fact attached to the wrong term.

### 2026-11-01_comment_plan-F11  medium  "Neither has a number" is false for imitation: a reading is free from `git log`, it is positive, and its one citation is the wrong agent's prompt
Status: closed

**Evidence.** Lines 46 and 47: "this session's own harness prompt says 'match its comment density' ... Neither has a number." The literature report's header says the sentence was read from the coordinator's prompt and "il prompt dei subagent non è stato visto". `AGENTS.md` AGENTS: the coordinator "implements nothing". Writers are subagents. This auditor's own subagent prompt holds no such sentence, and an implementing subagent's remains unread.

The reading, last 500 non-merge commits touching `src` or `tests`, C++ files only. For each file-commit with at least 5 added non-blank lines: `d`, the parent blob's share of lines starting `//`, and `s`, the share of added lines starting `//`. Spearman of `d` against `s`, then the same after removing each commit's mean within its area, which holds the agent, the step and the date fixed.

```
src   n 661  median density 0.14  Spearman 0.70  added-line comment share: dense half 51.6 %, sparse half 14.8 %
tests n 515  median density 0.27  Spearman 0.53  added-line comment share: dense half 49.1 %, sparse half 33.1 %
within one commit:
src   rows 495  commits 168  Spearman 0.61   denser than commit mean +10.2 pts, sparser -10.8 pts
tests rows 381  commits 112  Spearman 0.34   denser than commit mean  +5.6 pts, sparser  -4.7 pts
```

Whole-line `//` only. Not causal: a dense file is partly dense because of what is done in it, and file kind is not held. It does not separate imitation from the work needing more explanation there.

**Impact.** This cuts in favour of cleaning, against F01's direction, and the owner should have it. The plan calls the argument for "unmeasured" and sends it to T1 for a reading, when a first reading costs no tokens and no cut. It also does not establish the plan's "logs teach logs", which needs its own reading.

**Suggested resolution.** Put this reading in section 3 with its confounds. Read an implementing subagent's actual prompt before resting the override clause on it. Run a same-shape reading of id density and of log-block growth by file, then decide whether T1 is needed to answer the question at all.

### 2026-11-01_comment_plan-F12  low  The proof's blind column omits g++ `-Wcomment`, whose precedent is this exact pair of machines
Status: closed

**Evidence.** Commit `d54f690`: a trailing backslash in a `//` comment, "clang does not warn on it, which is why it survived S171 (136b03f): that step was built on the MacBook", and on the workstation "no step could have been completed". `CMakeLists.txt:55` has `-Wall -Wextra -Werror`. Here:

```
// usage: foo \
// second line
apple clang rc=0, homebrew clang rc=0
lexer strip identical old vs new with trailing backslash: True
```

Raw tokens over the two forms compare identical, 5 tokens each, since the next line is comment too. Line 85's blind column names `fastchess.sh` and major 22 only. `benefit_incidents.md` records the case under "Comments breaking things"; the plan does not carry it. g++ is not on this machine, so the g++ half is `d54f690`'s record and was not reproduced here.

**Impact.** A wrapped usage line in a comment passes every local proof and stops both workstation builds at the evening's gate, the act that precedes resuming S101.

**Suggested resolution.** A one-line check in `--apply` or the local gate: no `//` comment ends in `\`. Name g++ in the blind column.

### 2026-11-01_comment_plan-F13  low  Section 3's harm row states cost for the two "actual" rows only, and the larger costs sit in four single-rater "symptom" rows
Status: closed

**Evidence.** Line 32: "Together: one step, one clause, 0 games, 0 runs". `benefit_incidents.md` class A, the other four: A01, S088's 3340-game rating played one-sided under the false property; A04, 72.5 % of a parameter's iterations in a 30000-pair SPSA at an unshippable bound; A05, 7.16 % nps forgone from S109 to S268, "about +10 Elo ... a conversion, not a verdict"; A06, its own SPRT. Each may well be a symptom, for the reasons the file gives, and A05's comment does copy its step file. But "actual" against "symptom" is one reader's call (line 28), and the owner sees only the small total. The file's own text is fair; the plan's row is not its equal.

**Impact.** "Harm recorded ... is small" reads firmer than a one-rater split supports. Re-classing A05 alone puts a measured speed loss in the row.

**Suggested resolution.** One clause in the row naming the symptom rows' costs and the single rater.

### 2026-11-01_comment_plan-F14  low  Plan and record disagree in places an implementer will trip on
Status: closed

**Evidence.**
- Line 66, S308: "four code files". S308's table and `touches` name six: `src/data_structures.hpp`, `src/search.cpp`, `src/evaluation.cpp`, `tests/test_engine.cpp`, `tests/test_search.cpp`, `tests/test_uci_surface.cpp`. Three are S101 files, so step 2 enters them before step 5's "S101's seven files last".
- DEC-336 (1): "The COMMENTS rule as `AGENTS.md` now states it". `grep -n COMMENTS AGENTS.md CLAUDE.md` finds nothing at `b949eea`; the text is only in the plan's section 5 and, with its 15-line clause, in the literature report.
- DEC-336 (4): "`clang-format.sh --check` reading exactly today's two lines". Deleting one comment line above them moved the output to `src/data_structures.hpp:638` and `:639`. 291 comments, 17314 B, sit above line 639. By line number that proof goes red on correct work. It does continue past that file: a long comment in `tests/test_mate_carry.cpp` was reported at `:6:81`.
- Line 93: "Untouched. `adocs/data/`", against `adocs/data/S312_local_gate.sh` (line 65) and S312's `adocs/data/comment_study/cleanup/`.
- A false golden: line 72 "A golden whose script needs `.tuning/` keeps its value and says so"; S312 `excludes`: "re-derived by its script or the step stops". 8 of 32 blocks with `GOLDEN` name `.tuning`, books, fastchess or an epd.
- `adocs/data/S101_sprt.sh` header: "S308 touches src/ and lands after this step's close, and these guards refuse it before then", against DEC-335 (4). `status.md`: "the rule and then the 72 log-shaped blocks", against 76 and DEC-336 (2).

**Impact.** Each costs a stop or a wrong turn in a subagent briefed from these files, and none moves a score, a move or a line.

**Suggested resolution.** Correct line 66 and 93. Match the two lines by content. Choose one rule for a false golden that this machine cannot re-derive. DEC-336 (1) by a following decision, since an entry is not rewritten.

### 2026-11-01_comment_plan-F15  low  Several figures in the options table have no tracked derivation
Status: closed

**Evidence.** `git grep` over `adocs/data/comment_study`, the plan, `status.md` and `decisions.md`: 67.6, 53.6 and 65.4 (line 51), 22.5, 21.8 and 55.7 (lines 53 to 55) and "0.22 to 0.52" occur in `comment_strategy.md` only. O0's 1 to 3 M and O1's 2 to 4 M have no stated basis, and "201 s plus 161 s ... [M]" has no log. Re-derived here: 22.5, 21.8 and 55.7 reproduce exactly from `read_files.json`, uniform within a file. The ceilings do not: union of `touches:` over open steps without the comment steps gave 68.7, 58.1 and 73.1, by a method I had to guess. Tracked scripts exist for `benefit_*.py` and not for this table.

**Impact.** The next reader cannot check or update the table the owner decides from. O0's "those touched" ceiling is the only number describing what leaving the comments achieves.

**Suggested resolution.** A script beside `benefit_report.py` printing every cell.

## Re-measured and held

Counted so they need not be re-checked. Census at `b949eea`: 164 files, 4941 blocks, 1969706 B, 31355 lines. Tiers 76 / 307577, 228 / 496002, 4637 / 1166127. `bench` 5707449 on a Release scratch build. Fast suite Release 49 of 50, `test_fastchess_script (Timeout)`, 191.8 s; tune build 49 of 49 with that test excluded, 135.9 s, so the suite side of the local proof holds. Rebuild after touching `src/data_structures.hpp` 15.5 s. `clang-format.sh --check`: exactly the two named lines. `S101_pause_check.sh`: green at `b949eea`. S101 overlap 13 blocks, 33477 B, over code files. Raw tokens 84 of 84, 10.8 s, both controls right. 262 mutants, 274 anchors, all once, 7 on comments. Directive lines 121; "fall through" comments are prose here, not annotations. Four `__doc__` readers under `tools/` and `tests/`, as named; no live shell script reads its own comments. `--citations` flags 0 over 25 files. Dead references: 23 `file:line`, 28 mentions of 9 gone paths. `benefit_incidents.py` 170 rows, 0 problems; A 6, B 110, C 11, D 34, E 9; age and origin tables sum. `test_mate_carry.cpp` unchanged since `5954c12` and code-identical to the pilot's copy. `S101_sprt.sh` reads `.tuning/s101/bin/stamp` only from the main checkout and otherwise works in its worktree, so a comment commit reaches neither REF nor what it plays. A search of `adocs/plan_done/`, `adocs/audit/` and `worklog.md` for harm words beside `comment` found no A row the census missed. No tombstone-stops-a-retry record found either.

## Could not confirm

- The literature: no web access in this run. Internal agreement between the plan, `literature/report.md` and the notes was read for the figures named above, and no source was opened.
- Major 22's and g++'s opinion of anything: neither is on this machine.
- What an implementing subagent's prompt says about comment density.
- Whether a coordinator's per-batch cost is near 1.27 M: no comment batch has run.
- Whether the study's one-rater classes survive a second rater. None exists.
