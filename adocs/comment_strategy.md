# The comment plan: fix the old ones or leave them, and how

Second version, 2026-11-01, on the MacBook Air M5, after the adversarial audit `adocs/audit/2026-11-01_comment_plan.md` and the owner's ruling in DEC-337.
**Nothing here has been executed.** Marks: [M] measured here, [E] estimated, assumption stated, [R] a record. Figures print from
`adocs/data/comment_study/literature/benefit_report.py` and `head_55d7e75/options_reach.py`; O0's and O1's costs are the coordinator's estimate at the size of one small step each, no batch having run.

## 1. What the owner asked

1. Clear rules for comments, from a study of literature and community on LLM use, with how they enter this repository's framework.
2. Bring the existing comments into line, **now**. **No work on source other than comments**: S101 is paused on the workstation and resumes after this.
3. Before proceeding: this plan, a full adversarial audit of it by an agent with no stake, and an evaluation of fixing the old comments against leaving them.

## 2. Where things stand

- `55d7e75`, re-measured by the audit at `b949eea`: 164 live files, 4941 blocks, 1969706 comment bytes, 31355 lines; `bench` 5707449 [M].
- **`tools/gate.sh` is red here before any comment**: `test_fastchess_script` (72 s against a 60 s clock, and a `BARE_FEN=1` case whose cause is only partly read) and `clang-format.sh --check` on two lines of `tt_cluster_t` in `src/data_structures.hpp` under major 23. S314 holds it. Major 22 and g++ are not on this machine.
- **S101's paused diff touches more than code.** 13 blocks, 33477 B, in seven code files, context included [M], and also `DEV_MANUAL.md` (old-side lines 1504, 2030, 2355 to 2389, 3823, 6405), `MANUAL.md`, and the end of `adocs/data/README.md`'s table. A row appended at that table's end conflicts. REF stays `4e34e61`, so a comment commit reaches neither the SPRT's REF nor what it plays.
- Evidence: `adocs/data/comment_study/report.md`, `literature/report.md` and `notes/` (no peer-reviewed study measures comment removal with a frontier agent on a real repository), `literature/benefit_incidents.md`.
- Read by no one: the OpenAI harness post (403), Xia ASE 2026's numbers (403), Reddit. The audit had no web access, so the literature's sources are unchecked by it. Machine limits: no `.tuning/`, no books, no workstation transcripts.

## 3. Fix or leave: the evaluation

**The answer: fixing all of them does not pay; the cheap and safe parts do.** Tokens do not justify any sweep, recorded harm is small, and a rewrite writes the one class of false text with recorded harm.

**Harm** [M, floor, one reader, 170 rows, 272 done steps, `benefit_incidents.md`]:

| what | count | reading |
|---|---|---|
| A, a false comment tied to wrong work | 6, 2.2 % per step | one reader calls a comment the actual cause in 2, strictly 1: together one step, one clause, 0 games, 0 runs. **The other four carry the larger costs, each called a symptom by that one reader**: 3340 games under a false property, not re-run, no rating error measured; 72.5 % of a parameter's iterations in a 30000-pair SPSA at an unshippable bound; 7.16 % nps forgone, its comment copying a step file; one SPRT. No run was re-done because a comment misled it |
| A's origin | 6 of 6 false when written, 0 from drift | ages 2, 4, 4, 23, 25, 33 days. Recorded harm sits on text never true, the class a rewrite produces |
| B, false, found, no recorded harm | 110, 75 from drift | old stale text shows up false, with no recorded harm. 43 of 110 had a form the rule bars |
| C, steps existing mainly for comments | 7 of 272 | S155, S213, S245, S247, S249, S287, S306. 20.3 M to 47.7 M [E], the top likely high; S213 includes dead API and a test |
| D, a comment that demonstrably helped | 34, 12.5 % per step | 14 facts, 10 hazards, 4 stops. No record of a tombstone stopping a retry. Of 31 located: T1 holds 3, T2 11 |
| E, a false sentence from a comment repair | 9 | S306: 2 in 12 rewrites after the writer's own check. The pilot: 2 in 1136 lines |
| script could find it | 48 of 116 A and B | a reader for 66; 5 of 6 A needed a reader |

**The floor is the main limit**: whether an agent lost turns to a false comment is unmeasurable here, and so is what a true one did beyond what a step file says. The branch is 91 days old, and old text had longer to be found.

**The best argument for cleaning has a first reading and no proof** (audit F11): over the last 500 commits, C++ only, within one commit, the density of the file touched against the comment share of what was added reads Spearman 0.61 in `src` and 0.34 in `tests`; added lines in the dense half of `src`'s file-commits are 51.6 % comment, in the sparse half 14.8 %. **Correlation, not cause**: a dense file is partly dense because of the work done in it, and "logs teach logs" specifically is unread. The sentence "match its comment density" was read in the coordinator's prompt; the auditor's own subagent prompt holds none, and an implementer's is read at S311.

**Tokens** [E on M inputs]. Live comments read are 5.75 % of weighted use, pass-through unmeasured, 0.27 to 1.00. The open plan is about 100 M units.

| option | adds | cost added [E] | cumulative [E] | false claims the tier holds [E, from 11 events], **present, not repaired** | new false sentences before a reader [E] | helped rows located, of 31 | saved of the open plan, mid [E] |
|---|---|---|---|---|---|---|---|
| O0 leave | the rule, `--report`, fix a false one in a hunk you touch | 1 to 3 M | 1 to 3 M | those touched; file-level ceiling 67.6 % of T1, 53.6 % of T2, 65.4 % of T3 bytes | none here; rewrites move into steps | | 0 |
| O1 known and script | S308's ten rows, dead references across all live files | +2 to 4 M | 3 to 7 M | S308's plus the script class | few, minimal substitutions | | about 0 |
| **O2 + T1 by deletion, chosen** | 76 log-shaped blocks, 307577 B, 22.5 % of reads | at most +3.4 to 7.3 M | **at most 6.4 to 14.3 M** | about 30 | golden forms only. With paraphrase it would be 8 to 13 | 3 | 0.41 %, 0.4 M |
| O3 + T2 | 228 blocks of 1200 B or more, 496002 B, 21.8 % of reads | +4.5 to 9.6 M | 10.9 to 23.9 M | about 43 | none by deletion, about 7 if false ones are replaced | 11 | about 0.24 %, 0.2 M, assuming a 30 % cut |
| O4 + T3 | 4637 short blocks, read-only, 55.7 % of reads | at most +10.3 to 22.3 M | 21.2 to 46.2 M | about 22, from 2 events | about 4 | 17 | about 0.04 % |

The coordinator is not in those: a batch's rate is writer plus reader. At a third of the study's 1.27 M per step up to all of it: +1.3 to 5 M on T1's 4 batches, +11 to 34 M over 27. Unknown until a batch runs. A cost column is an upper bound for deletion, which reads as much as a rewrite and writes less.

**Whole scope, as DEC-336 first took it:** 21.2 to 46.2 M, about 85 to 185 $ at list, against about 0.7 M saved in the open plan, a net 3.0 to 6.7 mean steps. Maintenance is the only way it could pay. C-class work runs at 0.22 to 0.52 M a day [E]; at the located share of log-shaped rows, 24.4 % to 36.8 %, T1 pays back in 18 to 136 days against doing nothing. **Against O0 the figure is unknown**, since the rule alone already forbids the appends.

**Why O2 and not O0.** O1 closes known defects at almost no risk. T1 by deletion takes the blocks where steps append, holding the fewest helped rows of any tier, and writes no free text: the rate that makes a rewrite dangerous does not apply. It also removes the rewrites O0 would move inside later steps. O0 stays defensible on recorded harm alone: this is a judgement on an unmeasured benefit, and it is recorded as one.

**T2 is decided by a quantity that can be read, now fixed (audit F01).** S313 cannot decide it: 5 steps can detect 5.7 points of comment share and T1's effect is 0.41, about 1845 steps per arm, and the rule enters before T1 so nothing separates them. T1's register can. The question of T2 goes to the owner only if T1's register records at least 15 dead claims in its 76 blocks, half the estimate, and T1's pooled second read stays inside the stop below. Otherwise T2 is recorded as not worth it and is not asked. T3 is no sweep either way.

## 4. The plan

Order, each a commit or commits, stoppable at every commit:

1. **S311**, one clean subagent: `tools/comment_budget.py`, its test, one `add_test` line in `tests/CMakeLists.txt`, `adocs/data/S312_local_gate.sh`, a `DEV_MANUAL.md` entry outside S101's hunks; the coordinator adds the COMMENTS rule to `AGENTS.md` in the closing commit. **New files and one CMake line: the one exception to "comments only"**, stated to the owner. An implementer's report quotes what its own prompt says about comments.
2. **S308**, one clean subagent: its ten rows across seven code files, `DEV_MANUAL.md` and `rating.sh`. Three of those files hold S101 hunks, so `S101_pause_check.sh` decides.
3. **Script pass, S312 part one**: `--report`'s candidates across all live files, 23 `file:line`, 28 mentions of 9 gone paths, closed-step forward claims. A candidate is read, never applied blindly; history told on purpose stays. No new prose but the symbol that replaces a line number.
4. **Calibration batch, S312**: `tests/test_mate_carry.cpp`. Outside S101's hunks, no mutant targets it, and the pilot's copy compares at the end. Reads cost, wall time, writer and reader recall.
5. **T1, S312**, by traffic from `read_files.json`, about 4 batches: `src/search_params.hpp`'s 15 blocks, 50831 B, are the largest. Two batches in series including the calibration, then writers on disjoint files.
6. **The T2 question**, by section 3's rule.
7. **S313**, later, with the local A/B proposed then.

**Held, on any scope.** `fastchess.sh` and `tests/test_fastchess_script.sh`: the lexer calls code a comment at the latter's line 1419 and the proof skips its test here, so they wait for a machine that runs it (audit F07). The blocks S101's diff overlaps, and the blocks `DEV_MANUAL.md`'s goldens rows 2376, 2377 and 2385 say hold history, among them `tests/test_search.cpp`'s at `4815`, because those rows sit in S101's hunk and cannot be corrected until it closes (F06). They go on S312's list for after S101's close. A removed log's residue does not go in that table's range until then: it goes in the step file.

**Writer.** A fresh subagent per batch. It never touches the tree: it emits an edit list and a register line per block. In T1, three outcomes. **Keep** byte for byte, the default. **Delete** whole sentences, no new word; a false sentence is deleted and registered as a dead claim, and if the reader needs what it tried to say, it stays and is reported, for a later step. **Golden**, DEC-142's form: name, command, what moves it, margin, the value read from the constant the test asserts, and a number no test reads is no golden. **No paraphrase and no replacement** (DEC-337). A deletion that leaves a pronoun or qualifier dangling is a false sentence too. Chess claims stay word for word (the CHESS rule); no chess judgement. A golden whose false value this machine cannot re-derive, its script needing `.tuning/`, a book or fastchess, keeps its value and goes on S312's list for the workstation; the step does not stop for it. `src/search_params.hpp`'s lines 49 to 1341 are format-off inside a macro, so a backslash misaligned there is invisible to the format gate. `clang-format -i` is never run.

**Seeds, in both copies, never in the tree.** Three known-false claims per batch in the writer's copy of the old text, each substituted in place so no line moves, read as its recall on dead claims, since a claim it keeps is seen by nobody else (F09). Three in the reader's copy of the diff. Seed classes include a natural one: a true fact attached to the wrong term.

**Reader.** A fresh subagent, not the writer, no session history, the rule in its brief. For every changed block: old, new, the code around it, sources on request. Is each sentence of the new in the old or the code; which claim is false, a dangling reference included; which removed fact is in no record. A **second reader over all of T1**, pooled, gives the residual. The coordinator checks each finding against the code before a fix.

**Proof at every commit**, `adocs/data/S312_local_gate.sh`, DEC-336 (4):

| check | sees | blind to |
|---|---|---|
| lexer-stripped text identical, blank lines aside | every language | directives; docstrings used at runtime; **shell inside `$(...)` quotes until S311 fixes its copy** |
| `clang++ -Xclang -dump-raw-tokens` for C++, 84 of 84 agreeing with the lexer | inactive `#if` branches too | a cc1 flag, not a stable interface, hence the lexer beside it |
| shell, each script in a function printed by `declare -f`, original against new | bash's own parse | |
| Python `ast` without docstrings, tokens; four files reading `__doc__` treated as code | Python code | |
| directive comments, multiset unchanged, about 121 lines | `clang-format`, `noqa`, `NOLINT`, `shellcheck` | |
| no `//` comment ending in `\` | g++ `-Wcomment` under `-Werror`, `d54f690`'s precedent on these two machines | g++ itself |
| `--anchors`: each of 274 mutant anchors found once, 7 overlap comments | a broken mutant now | |
| `S101_pause_check.sh` **on a candidate commit in a throwaway worktree**, since the script reads `HEAD` | S101's hunks, before the commit exists | |
| both builds' fast suite without `test_fastchess_script`; format by content, the flagged lines being `tt_cluster_t`'s and no other; `bench` 5707449; `--citations` | behaviour, format | **major 22's opinion, until the evening's full gate** |

No proof here sees a false sentence: only the readers do. Object identity was read once, 10 of 10 Release TUs on Apple clang, not g++, and is not in the proof. Debug builds differ by `__LINE__`.

**Stops, to the owner**, read pooled, since a batch reads noise (F10):

- **Residual:** the denominator is comment lines in changed blocks. Over T1's pool, more than 1 meaning-changing error per 1000 such lines from the second reader, after the first reader's fixes. Certifying 1 per 1000 needs about 3000 lines read clean, so T1's reading is an estimate with its interval, and is stated as one.
- **Seed recall, reader or writer:** 12 seeds pooled over 4 batches, stop on fewer than 6 found; after the first 2 batches, 6 seeds, stop on fewer than 2. That is 6 % and 2 % of the time at a true recall of 0.67, 39 % and 11 % at 0.50, 83 % and 36 % at 0.33 [exact binomial]. Reported with an interval. Seeds are the classes already seen, so recall on them is not recall on what occurs.
- **Cost:** T1's scope ceiling is 19 M with the coordinator at its top. Read at the second batch from `transcripts.py`, its `ROOT` adapted in a scratch copy to this machine's two project directories, which S312 owns; if that fails, the agents' token counts as a proxy, stated as one.
- A removed reason re-derived or its defect re-introduced, one case enough.
- A red proof: the commit does not exist until its cause is read. A format disagreement at the evening's gate is read before S101 resumes (DEC-335 (7)). The evening: the last green commit is where it stands.

**Execution.** DEC-336 (3): two batches in series, then writers on disjoint files, the coordinator applying, proving and committing in series. One local proof is about 6 minutes, 201 s plus 161 s, read once on battery [M].

**Untouched.** `adocs/plan_done/`, `README.md`, `tests/third_party/`, `tests/doctest`, the held blocks, directive comments, the four `__doc__` docstrings, DEC-134's seed forms, `INV-n` numbers, every true block the rule allows. In `adocs/data/`, evidence is untouched and only new files are added: `S312_local_gate.sh`, `comment_study/cleanup/`.

## 5. The rule

```
- COMMENTS: **a comment says what the code cannot, about the code as it stands, to a reader without this session, diff or step.** It holds a contract, an invariant, a hazard, a sourced reason, or a deciding number with its id or command. Present tense; symbols, never line numbers. **No per-step log and no narration, in a comment or a docstring:** a new reading replaces the old in place, the story goes in the step file and the commit. A rejected idea keeps one line where it would be re-added: what, verdict, id. One id per claim, at its end, never instead of a reason the reader there needs. A golden keeps DEC-142's form; a number no test reads is no golden. **Code beats comment:** fix a false one in a hunk you touch, report it otherwise. Do not shorten a comment this rule allows; do not invent a reason. A rewrite gets a clean reader, old against new: identity checks cannot see a false sentence. `adocs/data/` and `adocs/plan_done/` are not cleaned. `tools/comment_budget.py --report` checks shape, review checks truth. **This overrides agent defaults:** ids and deciding numbers stay though a default forbids saying where something came from, and the density around you is no model. DEC-336.
```

**The harmed class is not an old-text problem.** All six A rows were false when written, so the part of this rule that matters most is the write-time one: "do not invent a reason", the clean reader on a rewrite, and Tier-1 over the step's diff. 22 of the 26 B rows written in the step that found them name a fast check, so that workflow already catches this class.

**Integration.** The rule in `AGENTS.md`'s Project rules. `--report` run by REVIEW's Tier-1 fast check over a step's diff, as a finding and never a gate: no comment check has a measured precision. `tools/gate.sh` unchanged; a single check may be promoted after 5 steps under 10 % false positives with no exception. Briefs carry the rule's text, session decay being the largest measured compliance effect; close briefs say "replace, never append". `DEV_MANUAL.md`: the tool and the reader's brief. `status.md`: a monthly line of live bytes per area.
**Not done, each with its reason in `literature/report.md` section 3:** a gate on form, an LLM detector in CI, hooks, a skill, `AIDEV` anchors, a byte ratchet, a ban on comments, moving reasons to a store.

## 6. What would prove this wrong

The rule does not halve 15.5 KB a day or lower the comment share of changed lines over 5 steps, read as bytes and shares and not as tokens per step, which 5 steps cannot resolve. Dead claims per KB, by the study's sampler at a new seed, a second rater and claims counted, do not fall (S313). The override sentence changes nothing in a with-and-without test. One removed reason re-derived. The readers miss their seeds. A `--report` check runs above 10 % false positives. A zero is recorded as a zero.
