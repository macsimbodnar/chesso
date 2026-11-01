# The comment plan: fix the old ones or leave them, and how

Third version, 2026-11-01, on the MacBook Air M5, after two adversarial audits, `adocs/audit/2026-11-01_comment_plan.md` and `.2.md`, the owner's ruling in DEC-337, and DEC-338.
**Nothing here has been executed.** Marks: [M] measured here, [E] estimated, assumption stated, [R] a record. `adocs/data/comment_study/literature/head_55d7e75/options_reach.py`, after `census.py HEAD blocks_head.json`, prints the reads and ceilings; `benefit_report.py` the incident tallies; `gate_reading.txt` the gate. The rest is arithmetic stated where it stands. O0's and O1's costs are the coordinator's estimate at the size of one small step each, no batch having run.

## 1. What the owner asked

1. Clear rules for comments, from a study of literature and community on LLM use, with how they enter this repository's framework.
2. Bring the existing comments into line, **now**. **No work on source other than comments**: S101 is paused on the workstation and resumes after this.
3. Before proceeding: this plan, a full adversarial audit of it by an agent with no stake, and an evaluation of fixing the old comments against leaving them.

## 2. Where things stand

- `55d7e75`, re-measured by the audit at `b949eea`: 164 live files, 4941 blocks, 1969706 comment bytes, 31355 lines; `bench` 5707449 [M].
- **`tools/gate.sh` is red here before any comment**: `test_fastchess_script` (72 s against a 60 s clock, and a `BARE_FEN=1` case whose cause is only partly read) and `clang-format.sh --check` on two lines of `tt_cluster_t` in `src/data_structures.hpp` under major 23. S314 holds it. Major 22 and g++ are not on this machine.
- **S101's paused diff touches more than code.** 13 blocks, 33477 B, in seven code files, context included [M], and also `DEV_MANUAL.md` (old-side lines 1504, 2030, 2355 to 2389, 3823, 6405), `MANUAL.md`, and the end of `adocs/data/README.md`'s table. A row appended at that table's end conflicts. REF stays `4e34e61`, so a comment commit reaches neither the family SPRT's REF nor what it plays.
- **One path it does reach (DEC-338 (1)).** On S101's H0 or no verdict, 14 of the last 23 verdicts by `plan.md`, a `src/` comment commit means no tree passes both guards the way they were meant to run: `S101_screen.sh apply` run from the main checkout refuses a tree at REF, and `S101_sprt.sh` refuses a REF at `achesso`. Run from `../chesso-s101` itself, with `OUT`, `S082_OUT` and `S083_OUT` pointing into the main checkout's `.tuning/`, the script's root is the worktree and that guard passes, no script amended. Read here as far as the guard, and past it only on the workstation. O0 alone avoids this. S308 is the first commit that reaches it.
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

**The best argument for cleaning has a first reading and no proof** (first audit, F11): over the last 500 commits, C++ only, within one commit, the density of the file touched against the comment share of what was added reads Spearman 0.61 in `src` and 0.34 in `tests`; added lines in the dense half of `src`'s file-commits are 51.6 % comment, in the sparse half 14.8 %. **Correlation, not cause**: a dense file is partly dense because of the work done in it, and "logs teach logs" specifically is unread. **And T1 barely moves that density where most steps work**: with all in-scope T1 blocks gone, `tests/test_search.cpp` goes from 0.396 to 0.385, `src/search.cpp` 0.510 to 0.484, `src/evaluation.cpp` 0.354 to 0.269; only `tests/test_mate_carry.cpp` and `test_mate_breadth.cpp` change class. T1 is not the cleaning that argument favours, and T2 and T3, which would be, are out. The sentence "match its comment density" was read in the coordinator's prompt; the first auditor's subagent prompt held none, and S311's implementer reports none in its own. The nearest an implementer gets is the owner's global `CLAUDE.md`: "Match surrounding style over any general preference." So the rule's closing clause answers that sentence and the coordinator's, and S313's with-and-without test reads whether it is needed.

**Tokens** [E on M inputs]. Live comments read are 5.75 % of weighted use, pass-through unmeasured, 0.27 to 1.00. The open plan is about 100 M units. **Spent already, before any batch: 12.8 M** on the study, this plan and the audits, read by `transcripts.py` at about 12:43 with S311's agent running. Sunk, and in no row below.

| option | adds | cost added [E] | cumulative [E] | false claims the tier holds [E, from 11 events], **present, not repaired** | new false sentences before a reader [E] | helped rows located, of 31 | saved of the open plan, mid [E] |
|---|---|---|---|---|---|---|---|
| O0 leave | the rule, `--report`, fix a false one in a hunk you touch | 1 to 3 M | 1 to 3 M | those touched; file-level ceiling 67.6 % of T1, 53.6 % of T2, 65.4 % of T3 bytes | none here; rewrites move into steps | | 0 |
| O1 known and script | S308's ten rows, candidates across all live files | +2 to 4 M | 3 to 7 M | S308's plus the script class | few, minimal substitutions | | about 0 |
| **O2 + T1 by deletion, chosen** | 76 log-shaped blocks, 307577 B, 22.5 % of reads; 11 held, 53717 B, 17.5 % | +3.4 to 7.3 M, a rewrite's rate and no upper bound; a second reader over T1, about +1.3 M; S311's modes beyond `--report`, read when S311 closes | **7.7 to 15.6 M, S311's excess aside** | about 30, 13 to 52 on a Wilson interval | **unmeasured**: golden forms about 2 at 1 in 6, 66883 B in 10 blocks; deletion's own rate read at calibration. 46 kept sentences lean on an id-naming one, a ceiling on places | 3 | about 0.25 %, 0.25 M, at the study's 40 % history and in-scope bytes |
| O3 + T2 | 228 blocks of 1200 B or more, 496002 B, 21.8 % of reads | +4.5 to 9.6 M | 10.9 to 23.9 M, T1's second reader aside | about 43 | unmeasured by deletion, about 7 if false ones are replaced | 11 | about 0.24 %, 0.2 M, assuming a 30 % cut |
| O4 + T3 | 4637 short blocks, read-only, 55.7 % of reads | at most +10.3 to 22.3 M | 21.2 to 46.2 M, the same aside | about 22, from 2 events | about 4 | 17 | about 0.04 % |

The coordinator is not in those: at a third of the study's 1.27 M per step up to all of it, +1.7 to 5.1 M on T1's 4 batches, +11 to 34 M over 27. Unknown until a batch runs. Nothing nets the held blocks against the extra reading.

**Whole scope, as DEC-336 first took it:** 21.2 to 46.2 M, about 85 to 185 $ at list, against about 0.7 M saved in the open plan, a net 3.0 to 6.7 mean steps. Maintenance is the only way it could pay. C-class work runs at 0.22 to 0.52 M a day [E]; at the located share of log-shaped rows, 24.4 % to 36.8 %, T1 with its coordinator pays back in 27 to 231 days against doing nothing. **Against O0 the figure is unknown**, since the rule alone already forbids the appends.

**Why O2 and not O0.** O1 closes known defects at almost no risk. T1 by deletion takes the blocks where steps append, 37.8 % of comment bytes dated within 7 days against 15.6 % of all, holds the fewest helped rows of any tier, and writes little free text. It also removes the rewrites O0 would move inside later steps. T1 deleted whole in a scratch clone kept `bench`, the Release fast suite and all 274 anchors. O0 stays defensible on recorded harm alone, and avoids S101's guard: this is a judgement on an unmeasured benefit, recorded as one, and neither audit overturned it nor confirmed it.

**T2 has no numeric gate (DEC-338 (2), replacing DEC-337 (2)).** S313 cannot decide it: 5 steps can detect 5.7 points of comment share and T1's effect is well under 0.41, about 1845 steps per arm, and the rule enters before T1 so nothing separates them. A threshold on T1's register does not work either: after S306, the held blocks, S308, the script pass and a writer's recall, the register can show 7 to 13 even if the estimate is exact, and T1's count says nothing direct about T2's blocks. **T1's register goes to the owner either way**, in blocks as the estimate counts them, seeds subtracted, with the writer's measured recall and intervals. T3 is no sweep.

## 4. The plan

Order, each a commit or commits, stoppable at every commit:

1. **S311**, one clean subagent: `tools/comment_budget.py`, its test, one `add_test` line in `tests/CMakeLists.txt`, `adocs/data/S312_local_gate.sh`, a `DEV_MANUAL.md` entry outside S101's hunks; the coordinator adds the COMMENTS rule to `AGENTS.md` in the closing commit. **New files and one CMake line: the one exception to "comments only"**, stated to the owner. An implementer's report quotes what its own prompt says about comments.
2. **S308**, one clean subagent: its ten rows across seven code files, `DEV_MANUAL.md` and `rating.sh`. Three of those files hold S101 hunks, so `S101_pause_check.sh` decides.
3. **Script pass, S312 part one**: `--report`'s candidates across all live files, 23 `file:line`, 28 mentions of 9 gone paths, closed-step forward claims. A candidate is read, never applied blindly; history told on purpose stays. No new prose but the symbol that replaces a line number.
4. **Calibration batch, S312**: `tests/test_mate_carry.cpp`. Outside S101's hunks, no mutant targets it, and the pilot's copy compares at the end. Reads cost, wall time, writer and reader recall.
5. **T1, S312**, by traffic from `read_files.json`, about 4 batches: `src/search_params.hpp`'s 15 blocks, 50831 B, are the largest. Two batches in series including the calibration, then writers on disjoint files.
6. **The T2 question**: T1's register goes to the owner, whatever it reads.
7. **S313**, later, with the local A/B proposed then.

**Held, on any scope.** `fastchess.sh` and `tests/test_fastchess_script.sh`: the lexer calls code a comment at the latter's line 1419 and the proof skips its test here, so they wait for a machine that runs it (audit F07). The blocks S101's diff overlaps, and the blocks `DEV_MANUAL.md`'s goldens rows 2376, 2377 and 2385 say hold history, among them `tests/test_search.cpp`'s at `4815`, because those rows sit in S101's hunk and cannot be corrected until it closes (F06). They go on S312's list for after S101's close. A removed log's residue does not go in that table's range until then: it goes in the step file. **S317, behind S101, owns the list and the three rows**, 11 blocks, 53717 B, once S312 has closed.

**Writer.** A fresh subagent per batch. It never touches the tree: it emits an edit list, each edit carrying the exact text it cuts, refused on mismatch, and a register line per block. In T1, three outcomes. **Keep** byte for byte, the default. **Delete** whole sentences, no new word; a false sentence is deleted and registered as a dead claim, and if the reader needs what it tried to say, it stays and is reported, for a later step. **Golden**, DEC-142's form: name, command, what moves it, margin, the value read from the constant the test asserts, and a number no test reads is no golden. **No paraphrase and no replacement** (DEC-337). A deletion that leaves a pronoun or qualifier dangling is a false sentence too, so a kept sentence that leans on one keeps it: whole-sentence and no-new-word leave keep or a fact lost by cascade, and keep is the default. Chess claims stay word for word (the CHESS rule); no chess judgement. A golden whose false value this machine cannot re-derive, its script needing `.tuning/`, a book or fastchess, keeps its value and goes on S312's list for the workstation; the step does not stop for it. `src/search_params.hpp`'s lines 49 to 1341 are format-off inside a macro, so a backslash misaligned there is invisible to the format gate. `clang-format -i` is never run.

**Seeds, in both copies, never in the tree, each by id.** In the writer's copy of the old text, three known-false claims per batch, each an **equal-length** substitution so no offset moves, read as its recall on dead claims, since a claim it keeps is seen by nobody else. `--apply --seeds` drops any edit overlapping a seed and lists it, so a caught seed leaves its true sentence in the tree and is subtracted from the register (second audit, F02). In the reader's copy of the diff, three seeds on **both sides**, since on a deletion-only diff a sentence added on the new side is found by `diff` whatever the reader can do; plus deletion's own classes: a dangling pronoun, a dropped qualifier, a removed unique reason. Seed classes include a true fact attached to the wrong term.

**Reader.** A fresh subagent, not the writer, no session history, the rule in its brief. For every changed block: old, new, the code around it, sources on request. Is each sentence of the new in the old or the code; which claim is false, a dangling reference included; which removed fact is in no record. A **second reader over all of T1**, pooled, gives the residual, deletion's error rate included. The coordinator checks each finding against the code before a fix.

**Proof at every commit**, `adocs/data/S312_local_gate.sh`, DEC-336 (4):

| check | sees | blind to |
|---|---|---|
| lexer-stripped text identical, whitespace-only and `\`-only lines read as blank | every language | directives; docstrings used at runtime; **shell inside `$(...)` quotes until S311 fixes its copy** |
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

- **Residual:** the denominator is comment lines in changed blocks. More than 1 meaning-changing error per 1000 such lines from the second reader, after the first reader's fixes, read after the second batch over what has landed and again over T1's pool, so a T1 commit can still be stopped by it. Certifying 1 per 1000 needs about 3000 lines read clean, so each reading is an estimate with its interval, and is stated as one. A commit already landed is corrected by a commit, never rewritten.
- **Seed recall, reader or writer:** 12 seeds pooled over 4 batches, stop on fewer than 6 found; after the first 2 batches, 6 seeds, stop on fewer than 2. That is 6 % and 2 % of the time at a true recall of 0.67, 39 % and 11 % at 0.50, 83 % and 36 % at 0.33 [exact binomial]. Reported with an interval. Seeds are the classes already seen, so recall on them is not recall on what occurs.
- **Cost:** T1's scope ceiling is 24 M, above the corrected top of 20.7 M with the coordinator and second reader, S311's excess being what the margin is for. Read at the second batch from `transcripts.py`, its `ROOT` adapted in a scratch copy to this machine's two project directories, which S312 owns; if that fails, the agents' token counts as a proxy, stated as one.
- A removed reason re-derived or its defect re-introduced, one case enough. This can fire only in later steps, so it stops further work and repairs by restoring from git; it stops no T1 commit.
- A red proof: the commit does not exist until its cause is read. A format disagreement at the evening's gate is read before S101 resumes (DEC-335 (7)). The evening: the last green commit is where it stands.

**Execution.** DEC-336 (3): two batches in series, then writers on disjoint files, the coordinator applying, proving and committing in series. One local proof is about 6 minutes, 201 s plus 161 s, read once on battery [M].

**Untouched.** `adocs/plan_done/`, `README.md`, `tests/third_party/`, `tests/doctest`, the held blocks, directive comments, the four `__doc__` docstrings, DEC-134's seed forms, `INV-n` numbers, every true block the rule allows. In `adocs/data/`, evidence is untouched and only new files are added: `S312_local_gate.sh`, `comment_study/cleanup/`.

## 5. The rule

```
- COMMENTS: **a comment says what the code cannot, about the code as it stands, to a reader without this session, diff or step.** It holds a contract, an invariant, a hazard, a sourced reason, or a deciding number with its id or command. Present tense; symbols, never line numbers. **No per-step log and no narration, in a comment or a docstring:** a new reading replaces the old in place, the story goes in the step file and the commit. A rejected idea keeps one line where it would be re-added: what, verdict, id. One id per claim, at its end, never instead of a reason the reader there needs. A golden keeps DEC-142's form; a number no test reads is no golden. **Code beats comment:** fix a false one in a hunk you touch, report it otherwise. Do not shorten a comment this rule allows; do not invent a reason. A rewrite gets a clean reader, old against new: identity checks cannot see a false sentence. `adocs/data/` and `adocs/plan_done/` are not cleaned. `tools/comment_budget.py --report` checks shape, review checks truth. **This overrides agent defaults:** ids and deciding numbers stay though a default forbids saying where something came from, and the density around you is no model. DEC-336.
```

**The harmed class is not an old-text problem, and no option reaches it beyond Tier-1.** All six A rows were false when written. Of 37 A and B rows false at birth, 19 were caught in the same step and 18 were not, those including all six A. Tier-1 entered `AGENTS.md` on 2026-08-19, and of the 18 only A05 has a known birth after it, 5 being unknown: the record does not settle whether the fast check already holds this class. The rule's form bars 0 of 6, and its reader clause applies to a rewrite, not to new text.

**Integration.** The rule in `AGENTS.md`'s Project rules. `--report` run by REVIEW's Tier-1 fast check over a step's diff, written into that rule in S311's closing commit, as a finding and never a gate: no comment check has a measured precision. `tools/gate.sh` unchanged; a single check may be promoted after 5 steps under 10 % false positives with no exception. Briefs carry the rule's text, session decay being the largest measured compliance effect; close briefs say "replace, never append". `DEV_MANUAL.md`, by S311: the tool, the reader's brief, and "replace, never append" for close briefs. Live bytes per area are read by S313, not by a monthly line nobody owns.
**Not done, each with its reason in `literature/report.md` section 3:** a gate on form, an LLM detector in CI, hooks, a skill, `AIDEV` anchors, a byte ratchet, a ban on comments, moving reasons to a store.

## 6. What would prove this wrong

The rule does not halve 15.5 KB a day or lower the comment share of changed lines, **over 30 steps or more**. At 5 steps that reading is a look and no verdict: with no rule in force, 29.2 % of 5-run windows read "halved" and 52.5 % read "share lowered", and tokens per step cannot be resolved at all. Dead claims per KB, by the study's sampler at a new seed, a second rater and claims counted, do not fall (S313). The override sentence changes nothing in a with-and-without test. One removed reason re-derived. The readers miss their seeds. A `--report` check runs above 10 % false positives. A zero is recorded as a zero.
