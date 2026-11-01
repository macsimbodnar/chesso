# The comment plan: fix the old ones or leave them, and how

Written 2026-11-01 on the MacBook Air M5, at the owner's request, for an adversarial audit before any comment moves. **Nothing here has
been executed.** DEC-336 records the owner's answers given before section 3's incident count existed, so section 4 asks again, and a
later answer amends it. Marks: [M] measured here, [E] estimated, assumption stated, [R] a record or another's report.

## 1. What the owner asked

1. Clear rules for comments, from a study of literature and community on LLM use, with how they enter this repository's framework.
2. Bring the existing comments into line, **now**, across the codebase. **No work on source other than comments**: S101 is paused on the
   workstation and resumes after this.
3. Before proceeding: this plan, a full adversarial audit of it by an agent with no stake, and an evaluation of the benefit of fixing the
   old comments against leaving them as they are.

## 2. Where things stand

- `55d7e75` read on this machine: 164 live files, 4941 blocks, 1969706 comment bytes, 31355 lines; `bench` 5707449 [M].
- **`tools/gate.sh` is red here before any comment**: `test_fastchess_script` (72 s against a 60 s clock, and a `BARE_FEN=1` case whose cause is only
  partly read) and `clang-format.sh --check` on `src/data_structures.hpp` lines 639 and 640 under major 23. S314 holds it. Major 22 is not on this machine.
- S101's paused diff (`adocs/data/S101_phase2_pause.diff`) overlaps 13 blocks, 33477 B, in seven code files, context included [M]. REF stays `4e34e61`, so a
  comment commit reaches neither the SPRT's REF nor what it plays. `adocs/data/S101_pause_check.sh` proves the overlap at each commit (DEC-335).
- Evidence: `adocs/data/comment_study/report.md` (the project's study at `5954c12`), `literature/report.md` and its `notes/` (literature and community;
  no peer-reviewed study measures comment removal with a frontier agent on a real repository), `literature/benefit_incidents.md` (section 3).
- Read by no one: the OpenAI harness post (403), Xia ASE 2026's numbers (403), Reddit. Machine limits: no `.tuning/`, no books, no workstation transcripts.

## 3. Fix or leave: what the records show

**Harm recorded from false comments is small, and it does not sit on old text** [M, floor, one reader, 170 rows, 272 done steps; `benefit_incidents.md`]:

| what | count | reading |
|---|---|---|
| A, a false comment tied to wrong work | 6, 2.2 % per step | a comment as actual cause in 2, strictly 1: brief S093 citing a false comment. Together: one step, one clause, 0 games, 0 runs. No run was re-done because a comment misled it |
| A's age | 2, 4, 4, 23, 25, 33 days | all 6 false when written, none from drift. At 30 days or more: 1 A, a symptom, against 31 B |
| B, false, found, no recorded harm | 110 | 75 from drift. So old stale text shows up as false and with no recorded harm |
| C, steps existing mainly for comments | 7 of 272 | S155, S213, S245, S247, S249, S287, S306: 945 code-file lines, 2914 document lines. 20.3 M to 47.7 M units at the study's median and mean step [E], the top likely high. Four more have comments as a large part |
| D, a comment that demonstrably helped | 34, 12.5 % per step | 14 facts, 10 hazards, 4 stops. No record shows a tombstone stopping a retry |
| E, a false sentence from a comment repair | 9 | S306: 2 in 12 rewrites after the writer's own check. The pilot: 2 in 1136 lines |
| script could find it | 48 of 116 A and B | a reader for 66; 5 of 6 A needed a reader |

**The floor is the main limit.** Whether an agent read a false comment and lost turns is unmeasurable here: that is where an uncounted A most likely sits, and D is a floor for the same reason.
The branch is 91 days old, so "old" is short, and old text had longer to be found. A rule on new comments, taken literally, prevents 0 of 116, since that text is already in the tree;
had it been in force when written, it bars the form of 0 of 6 A and 43 of 110 B.

**Tokens do not pay for it** [E on M inputs]. Comments in live code read are 5.75 % of weighted use, and pass-through is unmeasured, 0.27 to 1.00. The open plan is about 100 M units.

**The argument for, unmeasured:** this session's own harness prompt says "match its comment density", so a log-dense file may teach logs. A rule forbidding appends also means each step touching a log block rewrites it inside
that step's work, so leaving the logs moves rewrites to steps instead of removing them. Neither has a number. C-class work runs at 0.22 to 0.52 M units a day [E]; how much of it a cleanup would remove is unknown. At an assumed half, T1 pays back in 13 to 66 days, everything in 70 to 350.

| option | adds | cost [E] | dead blocks repaired [E, from 11 events] | saved of the open plan, mid [E] | recorded harm it would have averted | what it risks |
|---|---|---|---|---|---|---|
| **O0 leave** | the rule, `--report`, fix a false one in a hunk you touch | 1 to 3 M | those touched; file-level ceiling 67.6 % of T1, 53.6 % of T2, 65.4 % of T3 bytes in files open steps name | 0, growth aside | 0 | false text stays, logs teach logs, rewrites happen unreviewed inside steps unless Tier-1 catches them |
| **O1 known and mechanical** | S308's nine, 23 `file:line`, 28 mentions of 9 gone paths, dead forward claims; no prose rewriting | +2 to 4 M | S308's 9 plus the script class, up to 8 of 11 sampled by a method favouring it | about 0 | none clearly: those rows had no harm | few: mostly substitution and deletion |
| **O2 + T1** | 76 log-shaped blocks, 307577 B, 22.5 % of comment reads | +3.4 to 7.3 M | about 30, 0.11 to 0.24 M each | 0.41 %, 0.4 M | as O1 | rewriting creates claims: about 1 in 6 before a reader on S306's n of 12 |
| **O3 + T2** | 228 blocks of 1200 B or more, 496002 B, 21.8 % of reads; history deleted, false replaced | +4.5 to 9.6 M | about 43, 0.10 to 0.22 M each | about 0.24 %, 0.2 M, assuming 30 % cut | as O1 | deletes D-class facts if the "held nowhere else" test misses; conflict windows in files 9 open steps touch |
| **O4 + T3** | 4637 short blocks, read-only, 55.7 % of reads | at most +10.3 to 22.3 M | about 22, from 2 events, 0.47 to 1.0 M each | about 0.04 % | as O1 | a large cost on the least evidence; a reader's false positives rewrite true short reasons |

Whole-scope arithmetic: O4 is 18 to 39 M against about 0.7 M saved in the open plan, a net 2.6 to 5.6 mean steps of token budget, about 70 to 155 $ at list, no workstation time. The study's payback figure, 134 to 291 mean steps, assumed a 55 % cut, which T3 read-only does not take.

**Recommendation: O2, measure, then decide T2. T3 not as a sweep.** O1 is the part with almost no risk and it closes known defects. T1 has the study's defect rate (4 of 10 sampled), is where steps append, is the largest single token item at the lowest review risk per dead block, and is the only cut that can measure pass-through and the imitation argument for T2's decision. The records do not support T3's cost, and a read at T3 gets its yield from two events. O0 is defensible on the recorded harm alone; what argues against it is the rewrite-in-steps cost and the unmeasured imitation. **This differs from the scope in DEC-336 (2), which the owner gave before this count, and the owner decides.**

## 4. The plan, if the owner confirms a scope

Order, each a commit or commits, stoppable at every commit:

1. **S311**, one clean subagent: `tools/comment_budget.py`, its test, one `add_test` line in `tests/CMakeLists.txt`, `adocs/data/S312_local_gate.sh`, a `DEV_MANUAL.md` entry; the coordinator adds the COMMENTS rule to `AGENTS.md` in the closing commit. New file plus one CMake line: **the one exception to "comments only"**, named so the owner can refuse it. Not in S101's diff.
2. **S308**, one clean subagent: its nine rows. `DEV_MANUAL.md`, `rating.sh`, four code files.
3. **Mechanical pass**: dead references to symbols, no new prose. Some mentions are history told on purpose and stay. Candidates are read, never applied blindly.
4. **Calibration batch**, `tests/test_mate_carry.cpp`: outside S101's hunks, no mutant targets it, and the pilot's copy is a blind comparison at the end. Measures units per byte from this machine's transcripts, wall time, reader recall.
5. **T1**, then **T2** and **T3** only if the scope holds them, by traffic from `read_files.json`, S101's seven files last. Batches of about 75 KB: `tests/test_search.cpp` in 5 line ranges, `src/search.cpp` 2, `tests/test_engine.cpp` 2, `src/search_params.hpp` 2.
6. **S313** extended with the local A/B, proposed then. Its "before" arm stays in git.

**Writer.** A fresh subagent per batch. It never touches the tree: it emits `file, start line, end line, text` and a register line per block, with one of four outcomes. Keep byte for byte, the default. Delete whole sentences, no new word. Replace a false claim with a true one, its source read and named. Golden, DEC-142's form. No free paraphrase outside T1. Chess claims stay word for word or are checked by a tool (the CHESS rule); no chess judgement. A golden whose script needs `.tuning/` keeps its value and says so. A number with no test reading it is no golden. `src/search_params.hpp`'s lines 49 to 1341 are format-off inside a macro, so a backslash misaligned there is invisible to the format gate. `clang-format -i` is never run: it would reformat code, and the pin here is 23 against the workstation's 22.

**Reader.** A fresh subagent, not the writer, no session history, the rule in its brief. For each changed block: old, new, the code around it, sources on request. Three questions only: is each sentence of the new in the old or the code; which claim is false; which removed fact is not in the record. Three errors are seeded in its copy and never in the tree. A second reader on 10 % estimates what remains after review. The coordinator checks each finding against the code before a fix, since a detector's flags were 37 % to 60 % wrong in the literature.

**Proof at every commit**, `adocs/data/S312_local_gate.sh`, DEC-336 (4):

| check | sees | blind to |
|---|---|---|
| lexer-stripped text identical, blank lines aside | every language | directives; docstrings used at runtime |
| `clang++ -Xclang -dump-raw-tokens` for C++, 84 of 84 agreeing with the lexer, 11 s | inactive `#if` branches too | a cc1 flag, not a stable interface, hence the lexer beside it |
| Python `ast` without docstrings, tokens; four files reading `__doc__` treated as code | Python code | |
| directive comments, multiset unchanged, about 121 lines | `clang-format`, `noqa`, `NOLINT`, `shellcheck` | |
| `--anchors`: each of 274 mutant anchors found once, 7 overlap comments | a broken mutant now, not at its next run | |
| both builds' fast suite without `test_fastchess_script`; format reading the same two lines; `bench` 5707449; `--citations`; `S101_pause_check.sh` | behaviour, format, S101's hunks | **what `fastchess.sh` comments do to its own test, and major 22's opinion, both until the evening's full gate** |

No proof here sees a false sentence: only the reader does. Object identity was read once, 10 of 10 Release TUs with all comments stripped on Apple clang, not on g++, and it is not in the proof. Debug builds differ by `__LINE__` through `assert`.

**Stops, to the owner.** More than one meaning-changing error per 1000 comment lines after review. A reader missing two seeds of three in two batches running. Cost projecting past the scope's ceiling at the second batch. A removed reason re-derived or its defect re-introduced, one case enough. A red proof: the commit does not exist until its cause is read. `S101_pause_check.sh` red: the commit gives way and the hunk goes on S312's list. A format disagreement at the evening's gate is read before S101 resumes (DEC-335 (7)). The evening itself: the last green commit is where it stands.

**Execution.** Two batches in series, then three or four writers on disjoint files, the coordinator applying, proving and committing in series (DEC-336 (3), a waiver of DEC-185's one task at a time, this job only). One local proof is about 6 minutes, 201 s plus 161 s, read once on battery [M]. Series throughout is about 15 to 20 h [E, unmeasured writer time]; 27 proofs alone are about 2.7 h.

**Untouched.** `adocs/data/`, `adocs/plan_done/`, `README.md`, `tests/third_party/`, `tests/doctest`, S101's hunks, directive comments, the four `__doc__` docstrings, DEC-134's seed forms, `INV-n` numbers, every true block the rule allows.

## 5. The rule, as DEC-336 takes it

```
- COMMENTS: **a comment says what the code cannot, about the code as it stands, to a reader without this session, diff or step.** It holds a contract, an invariant, a hazard, a sourced reason, or a deciding number with its id or command. Present tense; symbols, never line numbers. **No per-step log and no narration, in a comment or a docstring:** a new reading replaces the old in place, the story goes in the step file and the commit. A rejected idea keeps one line where it would be re-added: what, verdict, id. One id per claim, at its end, never instead of a reason the reader there needs. A golden keeps DEC-142's form; a number no test reads is no golden. **Code beats comment:** fix a false one in a hunk you touch, report it otherwise. Do not shorten a comment this rule allows; do not invent a reason. A rewrite gets a clean reader, old against new: identity checks cannot see a false sentence. `adocs/data/` and `adocs/plan_done/` are not cleaned. `tools/comment_budget.py --report` checks shape, review checks truth. **This overrides agent defaults:** ids and deciding numbers stay though a default forbids saying where something came from, and the density around you is no model. DEC-336.
```

**Integration.** The rule in `AGENTS.md`'s Project rules. `--report` run by REVIEW's Tier-1 fast check over a step's diff, as a finding and never a gate: no comment check has a measured precision, and a length gate teaches splitting. `tools/gate.sh` unchanged; a single check may be promoted after 5 steps under 10 % false positives with no exception. Briefs carry the rule's text, since session decay is the largest measured compliance effect and Explore and Plan agents start without `CLAUDE.md`; close briefs say "replace, never append". `DEV_MANUAL.md`: the tool, the reader's brief, and three goldens rows that say a history stays in a comment. `status.md`: a monthly line of live bytes per area.
**Not done, each with its reason in `literature/report.md` section 3:** a gate on form, an LLM detector in CI, hooks, a skill, `AIDEV` anchors, a byte ratchet, a ban on comments, moving reasons to a store.

## 6. What would prove this wrong

From `literature/report.md` section 5: the rule does not halve 15.5 KB a day or lower the comment share of changed lines over 5 steps; the override sentence changes nothing in a with-and-without test; dead claims per KB do not halve; one removed reason re-derived; the reader misses its seeds; a `--report` check runs above 10 % false positives. A zero is recorded as a zero.
