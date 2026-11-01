# What false comments cost and true ones bought, from this repository's records

Read at `dcd3dae`, 2026-11-01. Records only; no engine built or run. Scripts beside this file: `benefit_census.py`, `benefit_rows.py` (the rows), `benefit_incidents.py` (validates every quote verbatim against its record, ages, blocks), `benefit_report.py` (this file). Every count is a floor: a case the record never mentions is not here, and a sweep that looks for false comments finds B and not A by design. No recommendation.

## The five answers

**1. Incidents.** A 6, B 110, C 11 steps, D 34, E 9. Over 272 done steps: A 2.2 % (6/272), D 12.5 % (34/272). D's rows name 30 distinct steps. A row is a finding or a use, not a step, so the rates are incidents per step and not steps affected. B counts S306's and S308's list row by row, 21 of the 110.

**2. Cause against symptom.** Class A is small. 2 of 6 have the comment as the actual cause, 0.7 % of steps, and strictly 1. A03: a later brief, S093, cites a false comment, the only row where the record shows someone acting on the text. A02: a test tolerance derived from a false count, but comment and number entered in one commit, `10bef50`, so it can also be read as one false belief written twice. Together they cost one step, one clause in another, 0 games and 0 runs. The other 4 are symptoms: code or a step file held the same belief. The largest, A01, has 3340 games played under the false property, with no re-run, no rating error measured, and no record that anyone read the comment. A05's 7 % of speed is the largest measured loss, and its comment copies a step file. No record in this search shows a run re-done because a comment misled it; S247's 400 timed runs are a repair of a figure.

**3. Old or new.** Of A and B, 40 were written in the step that found them or at most 3 days before, 54 were older, 22 unknown. A's ages run 2 to 33 days, median 13.5; the two actual causes are 2 and 4 days. B's median is 8.0 days, 33.0 without the same-step and same-day rows. At 30 days or more: 1 A against 31 B, and that one A is a symptom. All 6 A rows were false when written and 0 came from drift, while 75 of 110 B rows are drift. So recorded harm sits on young text that was never true, and old text that went stale shows up as false with no recorded harm. Limits: `achesso` is 91 days old, so 'old' is capped; A02, A03 and A04 were found when it was 11 to 18 days old, when all text was young; 44 % of comment bytes are under a month old anyway (study); old text has had longer to be found; the study's sweep finds old false text without testing for harm. A rule on new comments would have barred the form of 0 of 6 A rows and 43 of 110 B rows, had it been in force when written (N 65, U 8). Taken literally, given text already in the tree, it prevents 0 of 116: every row predates it.

**4. Class C.** 7 done steps exist only or mainly for comments, 2.6 % of 272: S155, S213, S245, S247, S249, S287, S306. 4 more have comments as a named large part: S192, S290, S277, S157. Strict: 945 lines in code files and 2914 in documents, by `git show --numstat` of their commits. Comment-only commits, by the study's lexer: 35 of 1000 non-merge commits since `bitboard`, 743 lines; 18 of them repairs (423 lines), 3 close passes (191), 12 adding a record where nothing was false (116), 1 a comment that broke the build, 1 format. [E] 7 steps at the study's median 2.9 M and mean 6.81 M weighted units: 20.3 M to 47.7 M. These diffs are mostly small beside the study's mean, so the upper end is likely high; no token figure for them is on this machine. S213's 650 code-file lines include dead API removed and a new test, so the strict total is not all comment. Open and not counted: S308, S311, S312, S313.

**5. Script or reader.** Of 116 A and B rows: script 48, reader 66, unsettled 2. A: script 1, reader 5; the one script row, A01, needs a check nobody had written. B: script 47, reader 61, unsettled 2. The script class here is a dead `file:line`, a gone path, a claim on a closed step, a number against its command or its test. Of the 6 A rows, 5 needed a reader, so the rows with recorded harm are mostly the class a script does not reach. 22 of the 26 same-step B rows name a fast check; the workflow already catches that class. In S306's and S308's list of 21: script 15, reader 6, against the literature report's 8 of 11.

## Other facts the decision needs

- **Length.** Of 78 A and B rows with a block found: 44 long, 56.4 %, and 19 log-shaped. In the live code today long blocks are 304 of 4941 and 40.8 % of bytes, log-shaped 76 and 15.6 % of bytes (DEC-336). So 56.4 % of located false text sits in long blocks against 6.2 % of blocks and 40.8 % of bytes, and 24.4 % in log-shaped ones against 1.5 % and 15.6 %. Biased: S306's list came from the study's size-stratified sample, and same-step rows mostly have no located block. 5 of A's 6 blocks are short and only A02 is long; A01 is 198 B. Length does not track harm here, on 6 rows.
- **D's blocks.** Of 31 D rows with a block found: 14 long, 3 log-shaped. Median age at use 21 days. Kinds: fact 14, hazard 10, stop 4, rule 3, seed 1, scope 1, hypothesis 1. Log-shaped blocks carried D14, D23 and D27, so a log can hold a fact a step used. D13 alone is quoted in 9 step files.
- **Tombstones.** No record found of a tombstone stopping a retry. S022, S114, S235, S237 and S238 wrote them; no later step says it read one and did not retry. The study's 'tombstones stop retries' is design, not a recorded use. A05 is the opposite case: a comment saying a change was not worth making, wrong by 7 %.
- **Near misses.** B04 ('Nearly propagated S035 into the manual on the strength of a code comment') and B10. F04's 'the sentence a future reader will rely on' is a prediction. Counted as B, cost 0.
- **Repairs can find things.** B13: re-deriving a stale figure found a floor that could not fail, S250. B56 is the same shape. That is 2 real test defects from comment work, and 9 false sentences introduced by it (E). S306's rate is 2 in 12 rewrites, after the writer's own check; the others have no denominator on record.
- **Audit false alarms.** F06 suspected a comment's robustness claim; S154 measured it true. S232's draft said a comment had lost a number; 'that was wrong and is withdrawn'. Neither is a row.
- **S254 is not a comment incident.** The study lists it. Its five stale anchors are mutant `old` strings naming code, `!is_check_move` against `!is_check_move()`. Not rowed. The real comment-to-mutant dependency: S114's M03 anchor sat on a comment and was re-pointed three times.
- **Comments breaking things, not false ones.** `d54f690`: a trailing backslash in a `//` comment stopped the workstation's build under g++ -Werror, no step could complete until fixed. S238 and S111: format gate red on comment reflow. `status.md:6971`: a comment grown by 8 lines moved 7 line citations, restored mechanically. The study has 5 of 244 mutant anchors on comments, loud at next run.
- **Same-diff maintenance is not rowed.** A step that moves a fact and updates its comment in the same diff is no incident. 118 step-file lines in 70 files name such an update, a rough count by grep. Five steps name a close pass: S123, S125, S135, S136, S296. The cost of these is in the study's churn figure, 61.3 % of changed `src` lines.
- **Age is a floor** where text moved between files: `git log -S` runs on a one-line fragment in one path, and `src/search_params.hpp` was created on 2026-08-16. The date is the record's; 8 rows carry the finding date the record gives instead of the repair's.

## How it was searched

- **Read every line by script, step files +/- 130 characters around the match, the others the first 240 to 260 characters:** `comment` in `adocs/plan_done/` (1017 lines, 167 of 272 files), `adocs/audit/` (95, 14 of 17 files), `adocs/decisions.md` (86), `adocs/worklog.md` (23), `adocs/status.md` (74), commit subjects (96). Commit bodies: `comment|docstring` beside `stale|false|wrong|lied|misled|mislead|untrue|no longer|outdated|said|claimed|claims|correct` (24).
- **Worded differently, lines without `comment`, over steps, audits, decisions and worklog:** `misled, mislead, false premise, wrong premise, stopped describ, no longer true, no longer describ, went stale, gone stale, had drifted, drifted from, no longer matches/said/says/holds/reads/names/states`, docstring, note, header, prose, block and GOLDEN beside `stale, wrong, false, untrue, lied, contradict` (103 hits). A location word (`.cpp .hpp .py .sh CMakeLists .cmake`, docstring, header, the case's, the script's) with `said, claimed, stated, promised, quoted` and `stale, false, wrong, untrue, no longer, contradict, mistaken` (16 hits).
- **Harm words within a line of `comment`, status included, across line wraps:** `wasted, re-run, misled, believed, trusted, relied, assumed, mistaken, discarded, void, on the strength, would have, nearly, invites, chased, misread` (38 hits).
- **Benefit:** `own comment, comment already, comment says, comment names, comment warns, comment forbids` from the same 1017 lines; `tried` beside comment, note or site, for tombstones.
- **Read in context, about 150 sites, not whole files.** No step file, audit or decisions.md read whole. `git log -S` for birth wherever a one-line fragment could be named. `adocs/data/` not searched but for this study's reports; its README has 20 `comment` lines, none about a false comment.
- **Coverage of the study's starting set:** F04 A01; F05 B16; S213 C02, B31 to B33, E07, E08; S245 C03, B36 to B38; S249 C05, B14; S254 not a comment incident; S276 B48; S287 C06, B17, E05; S292 D01, B107; S306 C07, B62 to B74, E01, E02; S308 B75 to B82; S125 D20, B46.

## What could not be read

- **`.tuning/` is not on this machine, and neither are the workstation's transcripts.** So: briefs under `.tuning/coord/`, including the study's S289, S121, S125, S123 and S083 pointers to comments, not verified and not rowed. Fast-check raw output, run logs, and anything an agent read and did not write down. Whether an agent read a false comment and lost turns: unmeasurable, and it is the most likely place an uncounted A sits. Whether a true comment changed what an agent did, beyond the sentence a step file gives: same, so D is a floor too. Token cost per incident: none here.
- **Before 2026-08-09** there are no step records, S001 to S016 being retro-stamped, so older text has a birth date and no history of who read it. `adocs/data/` scripts are records and only the rows named are checked.

## Classes and columns

Record: `S###:n` is `adocs/plan_done/S###_*.md` line n, or `adocs/plan_todo/` for S308; a dated name is under `adocs/audit/`; `git:sha` a commit message. A caused wrong work. B found and repaired, no recorded harm. C exists for comments. D helped. E introduced by a repair or rewrite. **Age**: days from the fragment's first commit to the record's date. **Origin**: birth, false when written; drift, made false later. **Rule**: N, the rule's form allows this text, only review could catch it; F, a form the rule bars (per-step reading, line number, a promise on a step, a number nothing reads, narration), which bites only if in force when written; U unsettled. All rows predate the rule. **Script**: S, R, U as in answer 5. Block: long is 1200 B or more; log is the study's selector.

## Class A

| id | cause | date | record | evidence | cost on record | block | age d | origin | rule | script |
|---|---|---|---|---|---|---|---|---|---|---|
| A01 | symptom | 2026-09-10 | 2026-09-10_adversarial:265 | "The comment is inverted, and the property it claims is precisely the one the rating run depends on" | S088's rating, 3340 games, 5 h 03 m, played one-sided: 514 of 2627 decisive adjudications one-sided. Not re-run. No rating error measured ('a hypothesis and not a cause', n = 5). Repair: one finding in S212, whose A/A read twosided as costing nothing. Comment and flag entered in the same commit, a9f2b33; no record that S088, DEC-077 or anyone read the comment | 198 B short | 23 | birth | N | S |
| A02 | actual | 2026-08-13 | decisions:2127 | "The comment was wrong and so was the audit's correction of it." | Test tolerance 2.0 derived from the count in the comment, both from commit 10bef50: 3446 of 200000 corpus positions past it, hidden by a 26-FEN corpus. S038, one step, no run, 0 games. Bias in a fit: hypothetical, not measured | 1295 B long | 2 | birth | N | R |
| A03 | actual | 2026-08-20 | 2026-08-20_plan_review:900 | "S093's accepts derives band safety from a bound that does not provide it" | Wrong brief: S093:28 cites this comment for the band. Caught before S093 ran. S142 lowered the bound to 699900 and asserted the clearance. No run, 0 games, one clause in S142 | 219 B short | 4 | birth | N | R |
| A04 | symptom | 2026-08-20 | git:428db08 | "Both claims in the comment were wrong." | S085's 30000-pair SPSA spent 72.5 % of RfpMinPly's iterations at bound 0, a value equal to 1 and unshippable. The record blames the declared minimum, which contradicted the comment; the false part credited the wrong guard. No re-run; S145 and S142 followed for the mate set, not for the comment | 515 B short | 4 | birth | N | R |
| A05 | symptom | 2026-10-08 | 2026-10-08_performance:69 | "justifies the cost with another engine's Elo figure, not a timing of this one" | Moving the rules before make gained -7.1 % cycles, +7.16 % nps, 'about +10 Elo ... a conversion, not a verdict' (S268). Forgone from S109 to S268. S109's step file holds the same sentence, so the comment is a copy of the decision. No record that it stopped a try | 527 B short | 25 | birth | N | R |
| A06 | symptom | 2026-09-10 | S207:39 | "The comment at the case states the mechanism as intended." | Code, test and comment agreed on a draw that does not exist. S207, its own SPRT, H1, nElo +4.47 +/- 6.72. Cost belongs to the defect; none is shown for the comment | 249 B short | 33 | birth | N | R |

## Class B

| id | date | record | evidence | cost on record | block | age d | origin | rule | script |
|---|---|---|---|---|---|---|---|---|---|
| B01 | 2026-08-09 | git:5590c71 | "A comment that confidently describes absent features is worse than no comment" | none recorded; the harm is stated as a hypothetical | 178 B short | 1 | drift | N | R |
| B02 | 2026-08-12 | worklog:1589 | "A stale comment was caught by the agent doing the re-anchor" | none recorded | 690 B short | 0 | drift | U | R |
| B03 | 2026-08-12 | worklog:1901 | "Two stale doc claims spotted, untouched" | none recorded; left in place | 1786 B long | 0 | unknown | N | R |
| B04 | 2026-08-13 | worklog:2671 | "Nearly propagated S035 into the manual on the strength of a code comment" | near miss, caught in S040; four comments named a step id wrong; 0 cost recorded Note: near miss; the id exists, so an existence check passes it. | 2020 B long, log | 1 | birth | N | R |
| B05 | 2026-08-11 | worklog:1228 | "Its header said "773 numbers"" | none recorded | 1726 B long | 1 | drift | F | S |
| B06 | 2026-08-13 | worklog:2296 | "only its comment, which described efficiency cores" | none recorded | 1115 B short | 0 | drift | N | R |
| B07 | 2026-10-05 | S055:93 | "Two comments in `src/evaluation.cpp` state the old tolerance and are false" | none recorded; S038's excludes forbade the edit, S055 and later e7b1d04 repaired | 1380 B long | 53 | drift | N | R |
| B08 | 2026-08-14 | S066:36 | "The comment is right about what it fixes and wrong about what is left." | measured: 99.47 % of games straddled the split, held-out 0.117352 against 0.117380, 'recorded as **zero**' (S066, DEC-056) | 255 B short | 4 | birth | N | R |
| B09 | 2026-08-20 | S106:319 | "`TT_PV_NODE`'s doc block carried a pasted copy of `TT_ALPHA_NODE`'s" | none recorded | unknown | unknown | birth | N | R |
| B10 | 2026-08-20 | S107:134 | "comment invites S020 to delete a flag LMR still needs" | near miss by the record's own word; rewritten in the same commit, 0 cost Note: near miss. | 144 B short | 15 | drift | N | R |
| B11 | 2026-09-30 | S115:296 | "**Its comment's schedule was stale**" | none recorded | 1855 B long | 44 | drift | U | S |
| B12 | 2026-09-30 | S115:560 | "the two floors' golden comments table shipping ends the parent no longer reads" | S249 written for it; half read true again on the next tree | 3302 B long | unknown | drift | F | S |
| B13 | 2026-10-01 | decisions:13662 | "Writing 146 beside the old table: a comment claiming a separation that is gone." | the repair found a dead gate: EXACT_FLOOR 143 no longer separated 146 from 143. S250, one step. A find by the repair, no harm shown from the comment Note: repair surfaced a real test defect. | 3475 B long | unknown | drift | F | S |
| B14 | 2026-10-01 | S249:18 | "found S132's docstring calling offsets 0/1/2" | none recorded | unknown | unknown | birth | N | R |
| B15 | 2026-08-18 | S088:116 | "`--bracket` printed 204 in a comment and played 272" | none recorded | 39 B short | 0 | drift | F | S |
| B16 | 2026-08-13 | 2026-08-13_adversarial:282 | "`LAZY_EVAL_MARGIN`'s stated precondition is false, and the re-decision it promised has not happened" | no unsoundness; open 69 days across four reports; S287 closed it. Harm named is expressiveness and was not traced to the text | 760 B short | 2 | drift | F | R |
| B17 | 2026-10-21 | S287:16 | "eight statements the revert made false again" | S287 exists for them, plus 22 more it found Note: a promise on a step, false on its H0. | 212 B short | 66 | drift | F | S |
| B18 | 2026-08-21 | S142:56 | "**`RFP_MAX_DEPTH`: the comment is wrong, not the number.**" | none recorded; DEC-095: 'no demonstrated defect' | 2075 B long | 5 | drift | N | R |
| B19 | 2026-08-21 | S142:145 | "A comment naming a guard that never binds is a doc claim about code that is false" | none recorded | 2075 B long | unknown | unknown | N | R |
| B20 | 2026-09-09 | git:2b198f6 | "All four are stale and point at quiescence code" | none recorded | 2555 B long | 20 | drift | F | S |
| B21 | 2026-08-29 | git:987510e | "`.moltke.json` no longer exists" | none recorded | 1206 B long | 16 | drift | N | S |
| B22 | 2026-08-21 | git:04bcc43 | "books/fetch_book.sh said the run walks 15000 rounds; it walks 30000" | none recorded | 632 B short | 1 | drift | F | S |
| B23 | 2026-08-21 | S138:10 | "whose own docstring said 32 positions and is corrected to the 100 per sample" | none recorded | 1628 B long, log | unknown | unknown | N | S |
| B24 | 2026-10-25 | S083:1228 | "the header comment credited the tables to S028's" | none recorded | 2687 B long, log | unknown | drift | N | R |
| B25 | 2026-10-25 | S083:1361 | "The comment above that check still gives the walk at S076's weights" | none recorded; the floor beside it went red for its own reason (DEC-313) | 1496 B long | 13 | drift | F | S |
| B26 | 2026-10-25 | S083:530 | "every sentence that states a pawn's value as 94" | none recorded | 2963 B long, log | unknown | drift | F | S |
| B27 | 2026-09-03 | S146:19 | "the two "163141 entries" comments" | none recorded | 420 B short | 0 | drift | F | S |
| B28 | 2026-09-04 | S193:224 | "comment: "a regression to pseudo-legal generation fails here"" | a test claim nothing held; DEC-163 chose to assert it; R7 adds a null make per generated move in Debug | 571 B short | 21 | birth | N | R |
| B29 | 2026-09-10 | 2026-09-10_adversarial:814 | "same-coloured bishops are not drawn, and the stated reason is wrong." | none recorded; behaviour accepted, the justification rewritten (S210) | 570 B short | 33 | birth | N | R |
| B30 | 2026-09-14 | S210:105 | "why the old "bounded by MAX_PLY" was false" | sat over a real defect, `bestmove 0000` at 4999 plies, out of reach of a match; fixed in S210; no cost shown for the comment | 285 B short | 37 | birth | N | R |
| B31 | 2026-09-10 | 2026-09-10_adversarial:840 | "**F26 low stale zero-weight comments in `src/evaluation.hpp`.**" | none recorded; 'the file was never swept'; S213 | 633 B short | 30 | drift | N | R |
| B32 | 2026-09-10 | 2026-09-10_adversarial:855 | "but the reason given does not establish it, and no test asserts" | conclusion true, reason wrong; S213 added the missing test | 2716 B long, log | unknown | birth | N | R |
| B33 | 2026-09-10 | 2026-09-10_adversarial:895 | "still says *"20 bytes of content in 24"*" | none recorded | 1279 B long | 23 | drift | F | S |
| B34 | 2026-09-10 | S039:99 | "comment's "p99 128 and max 330"" | none recorded | 2399 B long | 29 | drift | F | S |
| B35 | 2026-09-10 | S288:11 | "the current 150 first" comment | the tool lacked the shipped margin since S085; S288, code change; the comment is a symptom | 229 B short | 29 | drift | N | S |
| B36 | 2026-09-29 | S245:15 | "a source comment quoting a node count the engine no longer reports" | none recorded; one of S245's five | 1574 B long | 16 | drift | F | S |
| B37 | 2026-09-29 | S245:20 | "needs fractional reductions" comments that still name S237 and S238 after both left | none recorded; the reason for keeping the unit was false, the unit is behaviour-neutral | 2789 B long, log | 5 | drift | F | S |
| B38 | 2026-09-29 | S245:189 | "**The budget rule was the pre-DEC-162 text in three places**" | none recorded | 2121 B long, log | 21 | drift | U | R |
| B39 | 2026-10-01 | S247:11 | "13.8 ms reproduced by no run" | S247, one step, 400 timed runs to re-take a figure no assertion reads (its floor is 3 ms) | 1445 B long | 18 | drift | F | S |
| B40 | 2026-10-03 | S251:55 | "said 1 and 0 give "13 of 26 on time"" | none recorded | 8098 B long, log | 2 | drift | F | S |
| B41 | 2026-10-01 | S250:3 | "the stale figures in the comment around `MATE_IN_THREE_FLOOR`" | folded into S250 | 7296 B long, log | 30 | drift | F | S |
| B42 | 2026-10-14 | S099:951 | "the golden comment's list of six had been stale since S202" | none recorded | unknown | unknown | drift | F | S |
| B43 | 2026-10-15 | S023:581 | "`test_mate_carry`'s GOLDEN line was stale on this base and is" | none recorded; a revert put stale text back after S099 had fixed it | 10999 B long, log | 12 | drift | F | S |
| B44 | 2026-10-15 | S039:1631 | "the 1.4x the comment stated, reported and not fixed" | none recorded | 1025 B short | unknown | unknown | F | S |
| B45 | 2026-11-01 | S118:10 | "DEV_MANUAL.md, MANUAL.md; a stale comment each in src/chesso.cpp, tools/datagen.cpp, tools/tuner_trace.hpp," | none recorded; five comments, one line each in S118's touches | unknown | unknown | drift | U | U |
| B46 | 2026-10-31 | S125:396 | "The stale comment over the flats now names S083's refit." | none recorded | unknown | unknown | drift | U | R |
| B47 | 2026-09-30 | S244:16 | "which S210's comment in `quiescence` said could not" | S244 is a code fix: one wasted preliminary and a stale entry no main-search node reads; comment a witness, not a cause | 924 B short | unknown | drift | N | R |
| B48 | 2026-10-13 | S276:99 | "**Comments in `src/` now stale, not edited**" | commit e7b1d04, 38 + / 43 - lines; none else recorded Note: names a deleted file. | 655 B short | 62 | drift | N | S |
| B49 | 2026-10-13 | S264:66 | "comment "None of the reference engines exposes it" (`Threads`) is untrue" | none recorded; left | 896 B short | 56 | drift | N | R |
| B50 | 2026-10-12 | S274:59 | "`cmake/pgo.cmake` lines 39 and 40 carry the same two stale line numbers." | none recorded | unknown | unknown | drift | F | S |
| B51 | 2026-10-30 | S302:28 | "comment about "the other eight" is stale, since there are nine now." | none recorded | 3471 B long | 72 | drift | F | S |
| B52 | 2026-10-14 | S280:99 | "Their comments say the recorded URL is `git@github.com:` "here"" | none recorded; evidence scripts, left | unknown | unknown | drift | N | R |
| B53 | 2026-09-14 | S225:11 | "the stale "Seven cases"" | none recorded | 744 B short | 4 | drift | F | S |
| B54 | 2026-09-27 | S238:458 | "One test comment cited "the hindsight drives' position"" | none recorded | 174 B short | 1 | drift | N | R |
| B55 | 2026-10-15 | S282:126 | "so by this reading the trigger had not fired" | a script defect read wrongly since S231; comment and manual restated it; S282 is the script's | 4050 B long, log | 0 | drift | N | R |
| B56 | 2026-09-01 | 2026-08-21_adversarial:460 | "the assertion had been unable to fail for nine days" | a dead floor under a comment saying 'placed strictly between'; S154 re-placed it; the audit's other claim, that the comment's robustness was unmeasured, came out true when measured | 2683 B long | 11 | drift | U | S |
| B57 | 2026-08-21 | 2026-08-21_adversarial:154 | "so the assertion passes while the property it describes in its own comment is false" | a vacuous test; S149 re-targeted it and measured the dedupe at -11 Elo, so the behaviour was kept. Test defect, comment a witness | unknown | unknown | birth | N | R |
| B58 | 2026-08-21 | 2026-08-21_adversarial:532 | "Neither the step file nor the test comment says the replacement carries the same monoculture" | over-claim, not false text; S155 stated the qualifier | unknown | unknown | birth | N | R |
| B59 | 2026-09-04 | 2026-09-04_test_review:201 | "comment names lazy evaluation as the one exception and not the S130" | none recorded; S192 rewrote it | unknown | unknown | drift | N | R |
| B60 | 2026-09-11 | decisions:10181 | "F03's premise is written in a comment and enforced nowhere" | 'not reproduced'; estimated 6e-5 per iteration; S210 closed it | 406 B short | 37 | birth | N | R |
| B61 | 2026-09-29 | S131:295 | "capture_score() ranks a quiet evasion below any capture" untrue | none recorded | 1584 B long | 55 | birth | N | R |
| B62 | 2026-11-01 | S306:22 | "the second of them ships switched off" | none recorded | 2496 B long | 41 | drift | N | R |
| B63 | 2026-11-01 | S306:23 | "Golden (DEC-142)": its depth-1 iteration is 25933707 nodes | none recorded; the figure was also quoted in `S022_v2_sprt.sh` | 3129 B long, log | 33 | drift | F | S |
| B64 | 2026-11-01 | S306:24 | "S098 owns moving the rule onto the sum" | none recorded | 2316 B long | 47 | drift | F | S |
| B65 | 2026-11-01 | S306:25 | "the build prints 4493659" | none recorded | 3799 B long, log | 39 | drift | F | S |
| B66 | 2026-11-01 | S306:26 | "cites `evaluation.cpp` lines 33 to 37" | none recorded | 2307 B long, log | 72 | drift | F | S |
| B67 | 2026-11-01 | S306:27 | "the zero weights still fold" | none recorded | 3989 B long | 81 | drift | N | R |
| B68 | 2026-11-01 | S306:28 | "evaluate() is material plus tapered tables" | none recorded | 1025 B short | 75 | drift | N | R |
| B69 | 2026-11-01 | S306:29 | "S065 is a pending 827-constant paste" | none recorded | 4674 B long, log | 79 | drift | F | S |
| B70 | 2026-11-01 | S306:30 | "an H0 on S234 flips `RfpTtEstimate`" | none recorded | 7814 B long, log | 38 | drift | F | S |
| B71 | 2026-11-01 | S306:31 | "cites a numbered paragraph of `AGENTS.md`" | none recorded | 1722 B long | 70 | drift | F | S |
| B72 | 2026-11-01 | S306:31 | "`tests/test_uci_surface.cpp`'s header is the same class" | none recorded | 1002 B short | 84 | drift | F | S |
| B73 | 2026-11-01 | S306:32 | "cites lines 72 to 77 of `rating.sh` as doing the same" | none recorded | 771 B short | 73 | drift | F | S |
| B74 | 2026-11-01 | S306:33 | "`24 cases, ~1.6 s` for `tests/test_mutation_check.py`" | none recorded; S305 saw it and left it | 796 B short | 52 | drift | F | S |
| B75 | 2026-11-01 | S308:20 | "false in every release build" | none recorded; open | 854 B short | 41 | drift | N | R |
| B76 | 2026-11-01 | S308:21 | "the sum belongs with S098" | none recorded; open | 1484 B long | 48 | drift | F | S |
| B77 | 2026-11-01 | S308:22 | "**Golden**: 36165 nodes" | none recorded; open; floor still clears | 1445 B long | 33 | drift | F | S |
| B78 | 2026-11-01 | S308:23 | "while the weights are zero no test can say so" | none recorded; open | 4566 B long, log | 81 | drift | N | R |
| B79 | 2026-11-01 | S308:24 | "the zero weights downstream fold" | none recorded; open | unknown | unknown | drift | N | R |
| B80 | 2026-11-01 | S308:26 | "cites lines 71 to 84 of `fastchess.sh` by number" | none recorded; open | 258 B short | unknown | drift | F | S |
| B81 | 2026-11-01 | S308:27 | "a key `AGENTS.md` no longer has" | none recorded; open | 1000 B short | 84 | drift | F | S |
| B82 | 2026-11-01 | S308:28 | "the search costs 109575" | none recorded; open | 4674 B long, log | 79 | drift | F | S |
| B83 | 2026-09-13 | S109:739 | "The three in-test comments said otherwise until the fast check over this" | none; fast check | unknown | 0, same step | birth | N | R |
| B84 | 2026-09-13 | status:5077 | "comment mismatches of the same class and one breadth case no mutant can" | none; four more, found by re-observing mutants | unknown | 0, same step | birth | N | R |
| B85 | 2026-09-13 | S109:865 | "where the history comment also contradicted itself" | none; fast check | unknown | 0, same step | birth | N | R |
| B86 | 2026-09-28 | S113:711 | "the probe field's and the store case's comments said the stored depth is the shallow search's" | none; fast check | unknown | 0, same step | birth | N | R |
| B87 | 2026-09-28 | S113:716 | "`ProbCutMargin`'s comment named "the parent build"" | none; fast check | unknown | 0, same step | birth | N | R |
| B88 | 2026-10-01 | S114:1734 | "two comments that claimed a measurement not yet taken" | none; fast check | unknown | 0, same step | birth | F | R |
| B89 | 2026-10-29 | S121:1065 | "Not safe mobility: no enemy-pawn-attack mask" was false at mask 5 | none; fast check | unknown | 0, same step | drift | N | R |
| B90 | 2026-10-29 | S121:695 | "its stale "158 of the 768 columns" now reads 281" | none | unknown | unknown | drift | F | S |
| B91 | 2026-10-30 | S123:722 | "the false "Margin: 1 below the shipping count (2 since S135)"" | none | 11144 B long, log | 4 | drift | F | S |
| B92 | 2026-10-26 | S135:1452 | "said every fit since S065 held them at zero" | none; fast check | unknown | 0, same step | drift | N | R |
| B93 | 2026-09-09 | git:fbffd36 | "`short_line_ceiling`'s comment cited `F_mate6_inherited_no_line`'s worst cell as 2000000" | none; prose only, fast check | unknown | 0, same step | birth | N | R |
| B94 | 2026-10-04 | S202:520 | "comment's "closes 13 of the 33" is right for (c2) alone" | none; fast check | unknown | 0, same step | birth | N | R |
| B95 | 2026-09-11 | git:1138bd3 | "so both kills and the 13 % stand and only the explanation was wrong" | none; fast check | unknown | 0, same step | birth | N | R |
| B96 | 2026-09-11 | git:013600d | "so it never probed the comparison its comment claimed" | none; fast check | unknown | 0, same step | birth | N | R |
| B97 | 2026-09-12 | git:31f4b7d | "The reproduction's comment claimed python-chess reports is_repetition(3) True; it is False" | none; fast check | unknown | 0, same step | birth | N | R |
| B98 | 2026-09-11 | git:52f4c37 | "so the number a reader met first, in `src/`, was the one nobody had reproduced" | none; fast check | unknown | 0, same step | birth | N | R |
| B99 | 2026-10-14 | git:bacead1 | "comment said 2 above the weakened end where 159 against 156 is 3" | none; fast check | unknown | 0, same step | birth | F | S |
| B100 | 2026-10-25 | S083:1205 | "`adocs/data/S083_truncation_pick.py`'s docstring said the four were" | none | unknown | 0, same step | drift | N | R |
| B101 | 2026-09-21 | S095:633 | "one over-claim in the mate row's comment" | none; fast check, with three kill-attribution comments | unknown | 0, same step | birth | N | R |
| B102 | 2026-09-14 | S210:361 | "four comments describing pre-S210 mechanics" | none; fast check | unknown | 0, same step | drift | N | R |
| B103 | 2026-09-13 | status:4848 | "a comment and the stamp quoting python-chess as `VALID`" | none; fast check, with a stale comment on `king_zone()` | unknown | 0, same step | birth | N | R |
| B104 | 2026-10-11 | S273:11 | "fastchess.sh's stale comment" | none; fast check | unknown | 0, same step | drift | N | R |
| B105 | 2026-10-14 | S279:69 | "The source comment said the aborted node's 0 decides nothing" | none; fast check | unknown | 0, same step | birth | N | R |
| B106 | 2026-10-26 | S295:350 | "The doc comment of `cut_unreached_mate_pv()`" | none; fast check | unknown | 0, same step | birth | N | R |
| B107 | 2026-10-23 | S292:11 | "two stale comments and five notes" | none; fast check | unknown | 0, same step | drift | U | U |
| B108 | 2026-10-13 | S263:11 | "the fill-then-widen comment" | none; fast check | unknown | 0, same step | birth | N | R |
| B109 | 2026-08-10 | worklog:210 | "its justification comment was rewritten to say it is tidiness and not a fix" | none recorded | unknown | 0, same step | birth | N | R |
| B110 | 2026-08-12 | git:7375546 | "Also removes a stale comment block this session left above the pawn" | none | unknown | 0, same step | drift | U | R |

## Class C, done steps

| id | tier | date | step | goal, quoted | what it held | commits | lines in code files | lines in documents |
|---|---|---|---|---|---|---|---|---|
| C01 | strict | 2026-09-01 | S155:2 | "the constructed mate set's single motif is stated where its breadth is claimed" | comment, specs and a census script | 3d2fb61 120497e | 43 | 502 |
| C02 | strict | 2026-09-13 | S213:2 | "the evaluation header's five stale zero-weight blocks" | also dead API and two `<cctype>` calls, one new test | 58c31f9 6096fd4 | 650 | 530 |
| C03 | strict | 2026-09-29 | S245:2 | "five stale or wrong things the fillers of 2026-09-28 and 2026-09-29 noted" | two comments, one timing golden, one checker defect, one budget table | 6e8bc63 | 108 | 607 |
| C04 | strict | 2026-10-01 | S247:2 | "the eight-queens stop half's recorded timing figure" | comment only | 0cf3ec8 | 18 | 241 |
| C05 | strict | 2026-10-01 | S249:2 | "three golden comments read what their trees report" | comments and a docstring | 3f7acd3 | 0 | 240 |
| C06 | strict | 2026-10-21 | S287:17 | "Documentation and comments only" | comments and prose | 1174478 | 39 | 470 |
| C07 | strict | 2026-11-01 | S306:2 | "the eleven comments the 2026-10-31 study read as false, and one S305 found" | comments only, plus a clean reader | 4d3edf6 | 87 | 324 |
| C08 | loose | 2026-09-10 | S192:2 | "every golden number in `tests/` is named as one with the script that re-derives it" | names and scripts, plus a soft-limit test | 0abe648 | 362 | 635 |
| C09 | loose | 2026-10-25 | S290:2 | "is tracked in the tree and named where the budgets are" | a script and the comments naming it | b7be983 | 8 | 484 |
| C10 | loose | 2026-10-13 | S277:3 | "the ProbCut row's GOLDEN comment" | a miner and one GOLDEN comment | 1edb3b1 | 12 | 628 |
| C11 | loose | 2026-08-30 | S157:2 | "every recorded conclusion that states an SPRT bound in plain Elo says nElo instead" | documents and `fastchess.sh`'s comment | b59c52a | 15 | 275 |

### Comment-only commits, `benefit_census.tsv`

Rule: at least one code file changed, every changed code file identical with comments stripped and whitespace collapsed; documents may change beside it; `adocs/data/` counts as a record. 35 of 1000. A commit that mixes code and comments is not here, so close passes inside code commits are not counted.

| kind | commits | code-file lines added + removed |
|---|---|---|
| repair | 18 | 423 |
| close pass | 3 | 191 |
| record | 12 | 116 |
| build break | 1 | 2 |
| format | 1 | 11 |
| total | 35 | 743 |

## Class D

| id | kind | date | record | evidence | what it did | block | age d at use |
|---|---|---|---|---|---|---|---|
| D01 | hazard | 2026-10-23 | S292:24 | "The guard's own comment named a way it could cost more than predicted" | that hypothesis is what S292 measured; its H1 guard is that condition (DEC-310) | 3007 B long | 1 |
| D02 | seed | 2026-09-29 | S131:24 | "the seed here is this engine's own comment" | S131 was the comment's measurement: H1, the code stays | 590 B short | 52 |
| D03 | hazard | 2026-09-22 | S188:119 | "S107 left a comment warning against reusing it" | S188's check rule read a shared scan; which moves S091 exempts stayed exact | 499 B short | 33 |
| D04 | hazard | 2026-09-14 | S091:396 | "is what its own comment warns against" | S091 wrote `capture_gives_check` rather than reuse the flag | 499 B short | 25 |
| D05 | scope | 2026-09-04 | 2026-09-04_plan_review:478 | "the code's own comment and the S161 FEN sanitiser say the key must move in three places" | F08 caught S042's scope before it ran; S184 closed it | 566 B short | 34 |
| D06 | stop | 2026-09-24 | S236:411 | "The case's own comment says what to do and forbids the shortcut" | the agent re-ran a 108-cell, about 40-minute sweep rather than raise the ceiling | 317 B short | 15 |
| D07 | stop | 2026-09-30 | S022:457 | "**The test's own comment says a raised ceiling needs a decision, and DEC-241" | a ceiling moved by decision, not by edit | 317 B short | 21 |
| D08 | stop | 2026-09-30 | S022:1029 | "Its own comment says what that asks for: "under that it needs a heavier position, not a smaller floor"" | the floor was not lowered | 1270 B long | 17 |
| D09 | fact | 2026-09-14 | S230:25 | "the table's own comment already records that the shipped engine answers these positions in centipawns" | mining drove `search()` against the brief; UCI mining 'would have picked rows the case then fails' | 1715 B long | 0 |
| D10 | rule | 2026-09-11 | S209:3 | "the rule the file's own comment states for every path that mutates the table" | the accepts of a race fix took its rule from the comment | 115 B short | 40 |
| D11 | hazard | 2026-09-14 | S210:24 | "a comment naming exactly the harm the increment then" | the true comment beside `load_FEN` named the harm in F17's defect; S210 fixed it | 399 B short | 37 |
| D12 | stop | 2026-09-28 | S112:375 | "FutBase's own comment records the mistake of reading a margin in `see_value`'s units." | section 4's '200 cp' was not used as written | 2841 B long | 15 |
| D13 | fact | 2026-10-12 | S134:177 | "is `src/eval_tables.hpp`'s own comment, not a citation" | the 'degenerate' split is quoted as a seed's unit in 9 step files: S097, S098, S099, S109, S112, S113, S114, S134, S180 | 1884 B long | 62 |
| D14 | fact | 2026-09-20 | S232:225 | "comment does still record it, in the sentence "median 11 at the tuning" | median depth 11 seeded bounds in S109, S180, S222, S232. S232 then moved it to a constant | 3040 B long, log | 30 |
| D15 | hazard | 2026-09-09 | S191:84 | "That is the reverse-futility comment's own "a mate deeper than ply 3 can still be missed for an iteration", met head on." | a guard fixture failed where the comment said it could; also cited by S085 and S142 | 2066 B long | 24 |
| D16 | fact | 2026-09-08 | decisions:8497 | "the version the script's comment already names" | an owner question answered itself (DEC at S198) | 1273 B long | 18 |
| D17 | rule | 2026-08-17 | S068:307 | "**re-target the test file's own comment prescribes**" | a golden moved with a parameter, recorded as no relaxation | 513 B short | 1 |
| D18 | hazard | 2026-09-18 | S098:2047 | "That is the case's own comment doing what it was written to do" | a red precondition read as the tree moving, not a defect | 1883 B long | 3 |
| D19 | hazard | 2026-08-17 | S076:198 | "exactly the failure its own comment predicts" | the residual read as the weights' and the positions re-picked | 1814 B long | 2 |
| D20 | fact | 2026-10-31 | S125:801 | "The site's comment already held the class" | a budget move read as a known class | 828 B short | 19 |
| D21 | hypothesis | 2026-09-07 | S171:61 | "`extend_mate_pv()` comment already names this as the reason a stored move is re-validated" | one of S171's listed causes | 478 B short | 5 |
| D22 | fact | 2026-09-11 | decisions:9942 | "`src/chesso.cpp`'s own provenance comment says so" | explained why S208's two clauses collide; DEC-177 replaced the constant | 1837 B long | 3 |
| D23 | rule | 2026-10-16 | S286:59 | "The constant's own comment says to re-run the census" | raised owner question R2; the census was not re-taken | 2263 B long, log | 25 |
| D24 | fact | 2026-09-09 | S159:315 | "`fastchess.sh`'s comment on exactly this" | an empty log not misread | 699 B short | 20 |
| D25 | hazard | 2026-09-11 | S207:89 | "(the comment there says so)" | the repetition test stayed above the table probe | 216 B short | 37 |
| D26 | fact | 2026-09-10 | S195:107 | "comment reads "nothing reads it to decide, order or prune a move"" | scoped which readings a warm carry can move | 387 B short | 8 |
| D27 | hazard | 2026-09-20 | S231:186 | "which is the same edge S222's comment names at 2100" | a band edge read as known | 6006 B long, log | 6 |
| D28 | fact | 2026-10-13 | S259:27 | "records every re-mine so far" | a log block: the re-mine history S259 read | 18659 B long | 22 |
| D29 | fact | 2026-10-30 | S298:116 | "The script's own comment already reserved "stage 6"." | placed a new stage | 1042 B short | 50 |
| D30 | hazard | 2026-10-15 | S023:229 | "where the comment says the margin clears it "with room"" | a case flagged as at risk in the pre-registration | 239 B short | 28 |
| D31 | fact | 2026-09-12 | S042:286 | "its own comment names this step by number" | found a test the fix moves | 955 B short | 26 |
| D32 | fact | 2026-10-03 | S252:45 | "the way S145's mate case does and for the reason its comment gives" | the drive took the same form | unknown | unknown |
| D33 | fact | 2026-08-20 | S100:27 | "states the case for splitting the pair" | S100's diagnosis read a measured split from it | unknown | unknown |
| D34 | fact | 2026-09-04 | S175:22 | "quoted in the file's own comment at `src/openings.cpp:549-552`" | the format the audit and S175 tested against | unknown | unknown |

## Class E

| id | date | record | evidence | how it was caught | where |
|---|---|---|---|---|---|
| E01 | 2026-11-01 | S306:145 | "the rewrite read "the tapered sum of material, tables, ..."" | a clean reader caught it after the writer's own check | src/data_structures.hpp |
| E02 | 2026-11-01 | S306:150 | "`bestmove` after 6746 to 6776 ms" quoted the `time` field | a clean reader caught it after the writer's own check | tests/test_engine.cpp |
| E03 | 2026-10-31 | study report:413 | "a fabricated pointer, "(DEC-204 class)", on S237's tombstone" | only reading old against new caught it; the id exists | pilot, never landed |
| E04 | 2026-10-31 | study report:413 | "a wrong denominator, "0 of 14589403 calls disagreed" for 0 of 3368027 read-backs" | same; pilot, never landed | pilot, never landed |
| E05 | 2026-10-21 | S287:237 | "as first restated, made `tools/eval_spread` the tool that measures the share clamped at 184" | fast check caught it before commit; led to S288 | src/evaluation.hpp |
| E06 | 2026-10-15 | S282:236 | "Phase 1 attributed it to S023's tree, and that attribution is gone from the comment" | phase 2's run caught it | tests/test_search.cpp |
| E07 | 2026-09-13 | status:4812 | "and gained "this term has not"" | S213's fast check caught it | src/evaluation.hpp |
| E08 | 2026-09-13 | status:4815 | "sentence restated a "nearly nothing" that S100's three fits" | S213's fast check caught it | src/evaluation.hpp |
| E09 | 2026-08-21 | git:bfc0463 | "src/search_params.hpp:74 cited src/search.cpp:521 for" | S142's Tier 1 caught it; one line off, in the rewrite of 428db08 and ac4c588 | src/search_params.hpp |

## Age against class, A and B

| age | A | B |
|---|---|---|
| same step | 0 | 26 |
| 0-3 d | 1 | 13 |
| 4-29 d | 4 | 18 |
| 30 d or more | 1 | 31 |
| unknown | 0 | 22 |

| origin | A | B |
|---|---|---|
| birth | 6 | 31 |
| drift | 0 | 75 |
| unknown | 0 | 4 |
