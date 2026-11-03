id:         S328
goal:       every audit finding whose closing steps are all done, or that a re-run or a decision has settled, says so on its `Status:` line, or says why not
accepts:    a stated script lists each finding under `adocs/audit/` whose `Status:` reads `planned` and every step naming it in `closes:` is in `plan_done/` (60 of 72 on 2026-11-03, among them `2026-08-13_adversarial-F08`, closed by S042); each finding read against its steps' `done:` stamps, then its `Status:` line moved to `closed` with the closing step's id, or left `planned` with the reason stated in this step; found during the step and taken in: each `planned` finding no step names, moved to `closed` where a re-run's own entry reports it closed, naming that report and entry, and each `open` finding moved to `accepted` where a decision rules on it in terms and the re-runs agree, naming both, or to `planned` where a pending step names it in `closes:`, `Previously:` kept, or either left with the reason stated in this step; and each Part E finding of `2026-09-10_adversarial` that has no `Status:` line given one, after S134's precedent, `closed` on its done steps or `planned` on a pending one; no other byte of any report changes but those `Status:` lines and the one exactly empty line that separates each inserted line from the next paragraph (AGENTS.md: the `Status:` line is the only edit a report takes); the script and its before and after counts in `adocs/data/`, with a README row
touches:    adocs/audit/*.md, adocs/data/S328_*, adocs/data/README.md
excludes:   any finding's text; findings `accepted`, and `open` ones neither a decision nor a step settles; steps' files
decisions:  DEC-171, DEC-197, DEC-346
closes:
blocks:
paused_by:
author:     Opus 5.5 subagent, briefed by the coordinator
done:       2026-11-03. 73 of the 74 `Status:` lines read moved, and 18 inserted. 62 `planned` lines whose steps are all done moved to `closed` in the record's dominant form `closed — S<nnn>, <date>`, with the re-run that reported it closed or "no audit has re-run it since" (18), "moved by S328, 2026-11-03", and the old text as `Previously:`; one left `planned` with its reason (`2026-09-10_adversarial-F16`, which its own line hands to S122). Found during the step and taken in: the nine `planned` findings no step names moved to `closed` on `2026-09-04_plan_review`'s entry for each, in the record's re-run form; `2026-08-22_adversarial-F06` moved `open` to `accepted` on DEC-121 and both adversarial re-runs; `2026-08-21_adversarial-F04` moved `open` to `planned — S152`, no decision naming it and a pending step doing so; and the 18 Part E findings of `2026-09-10_adversarial` with no line given one after S134's `0d9faba`, 17 `closed` on their steps (13 with no re-run since) and F24 `planned — S126`. `adocs/data/S328_planned_done.py` reads 63 of 72 on `1bb3080`, not the 60 stated: three dotted ids a dotless grammar orphans, which `--dotless-ids` reproduces. After: 3 `planned`, 158 `closed`, 7 `accepted`, 0 `open`, 0 without a line. No other byte of any report changed: `--against-rev 1bb3080` prints `S328-REPORTS-STATUS-ONLY` over all seventeen, 73 changed, 18 inserted, refusals 0, and goes red on eight planted edits; `git diff -U0` has 0 changed lines that are neither `Status:` nor empty, and 18 added empty lines (`adocs/data/S328_evidence.txt`). Checks: `tools/plan_prose_check.py`, all five, exit 0; the five `test_plan_*` ctest entries green in `build` and `build-tune`, configured and not compiled, since each runs a Python script over the source tree and no ctest entry reads `adocs/audit/`; `CLANG_FORMAT_MAJOR=22 ./clang-format.sh --check` exit 0. Fast suites not run: no compiled or tested file changed. `DEV_MANUAL.md` and `MANUAL.md` checked, no change. One finding beyond the step recorded above, not fixed.

## Why

Found 2026-11-03 while landing S325: its implementer saw F08 still `planned` though S042 closed it in 2026-09. A
count over every report found 60 such findings out of 72 marked `planned`, and 69 already `closed`. The step that
closes a finding has not always moved its line. The reports are evidence, and a stale status hides which findings
are still live. Filler by DEC-171.

Found during the step, and taken in at the coordinator's request as AGENTS.md's "discovered mid-step": the nine
`planned` findings no step names, which the 2026-09-04 plan_review re-run reports closed, and two `open` findings the
2026-09-03 and 2026-09-04 adversarial re-runs report accepted, one of them named by a pending step. Both kinds are the same defect, a `Status:` line that
disagrees with the record. Then the 18 short-form findings of `2026-09-10_adversarial`'s Part E with no `Status:`
line at all, 17 of them with every closing step done: the report's header, "open on all of them", still spoke for
them. S134's `0d9faba` set the precedent for those, inserting the F34 and F36 lines in the form "Previously: open
(this report's header: open on all of them)", and the coordinator ruled that S328 follows it.

## 2026-11-03: the census, the rule, and the lines moved

On `1bb3080`, no build, no run beyond the script. By one Opus 5.5 subagent, briefed by the coordinator.

**The script.** `adocs/data/S328_planned_done.py` reads every finding of `adocs/audit/*.md` (a `### <report>-Fnn`
heading, or a `**Fnn ` paragraph of the 2026-09-10 report's Part E), its first `Status:` line, and every step's
`closes:` in `plan_todo/`, `plan_current/` and `plan_done/`. It prints the census of status words and each `planned`
finding in one of three classes: every step naming it is in `plan_done/`; a step naming it is not; no step names it.
`--against-rev` proves that the reports differ from a commit's only on `Status:` lines. It reads; it edits nothing.
Its readings are in `adocs/data/S328_evidence.txt`.

**It reads 63 of 72 on `1bb3080`, not 60.** The difference is three ids with a dot: `2026-08-13_plan_review.2-F06`,
`-F07` and `-F09`, named by S023, S062 and S064. An id grammar without the dot reads them as `2-F06`, `2-F07` and
`2-F09`, which name no finding, and then gives 60 of 72 exactly; `--dotless-ids` reproduces it. The 72 and the 69
`closed` agree with a plain grep (70 with the one `closed,`). No `planned` finding has a step outside `plan_done/`;
the other nine `planned` findings have no step at all.

**The closed line's form.** Of the 70 `closed` lines on `1bb3080`, 50 name a step and 42 lead with it after the dash;
the largest group of those, 19, reads `closed — S<nnn>, <date>`, and 27 carry the step's date in some form. Two name
a commit, so the form names none. Every line moved on a step therefore leads `closed — S<nnn>, <the step's completion date>`,
the date the step file entered `plan_done/` by `git log`. Three additions, each already in the record: the re-run that
reported the finding closed, where one did (15 lines name a re-run, as `closed -- S175, re-run 2026-09-04_adversarial`
does), or "no audit has re-run it since"; "moved by S328, 2026-11-03", the lag S162's and S180's lines record; and
`Previously:` with the old text verbatim, as 13 lines keep it, because the old text carries the digest's notes
(`VERIFIED`, the DEC ids). The dash is the one each report's own `Status:` lines use most: `--` in
`2026-09-04_adversarial` and `2026-09-10_adversarial`, `—` everywhere else.

**The rule each finding was read against.** Its steps' `done:` stamps first, then the re-runs, because nine of the
stamps say their findings stay `planned` until an audit is re-run (S035, S036, S038, S040, S041, S043, S067, S069 and
S070: the AGENTS.md section 10 of moltke before v1), and the moltke audit skill still closes a finding on a re-run
that no longer reports it. Four later reports carry a verdict on each prior finding they re-took:
`2026-08-16_plan_review` and `2026-09-04_plan_review` (plan_review re-runs), `2026-09-03_adversarial` (an adversarial re-run) and
`2026-09-04_test_review` (a test_review re-run). Each of the fourteen findings those nine stamps held back got its
re-run, and it reported each closed but `2026-08-14_test_review-F04`, found open again in a new wording and folded
into a finding S193 closed. A finding is moved when its steps did what it asks and no stamp, decision or later report
leaves part of it live today; it stays `planned` otherwise. Eighteen of the moved findings have had no re-run since
their step: they close on the stamp, as the record's S191 to S194, S207 to S209 and S268 to S271 lines did.

| finding | closing steps | what the stamp says | new status |
|---|---|---|---|
| `2026-08-13_adversarial-F01` | S035 | three defects fixed, the script runs; the exit-status half did not reproduce under bash 5.2 and is recorded unverifiable; F01 "stays planned: the audit has not been re-run" | closed; `2026-09-03_adversarial` reports it closed by S035 and S167, which fixed that half under bash 3.2 |
| `2026-08-13_adversarial-F02` | S036 | the budget floors at 1 ms, red first; stays planned until the re-run | closed; `2026-09-03_adversarial` closed |
| `2026-08-13_adversarial-F03` | S037 | `info nodes` is the whole search's, INV-6 discharged | closed; `2026-09-03_adversarial` closed |
| `2026-08-13_adversarial-F04` | S038 | tolerance re-derived at 3, four positions pinned; only a re-run closes it | closed; `2026-09-03_adversarial` closed by S038 and DEC-053 |
| `2026-08-13_adversarial-F06` | S040 | the tuner section re-derived from the code; only a re-run closes it | closed; `2026-09-03_adversarial` closed |
| `2026-08-13_adversarial-F07` | S041 | `test_tuner_groups`, three properties, red three ways; a blind spot past the last group reported, not closed; only a re-run closes it | closed; `2026-09-03_adversarial` closed |
| `2026-08-13_adversarial-F08` | S042 | the square set only when capturable, red first, `--nonreg` H1 | closed; no re-run since |
| `2026-08-13_adversarial-F09` | S043 | the dead line deleted, configure byte-identical; planned until the re-run | closed; `2026-09-03_adversarial` closed |
| `2026-08-13_plan_review.2-F06` | S023 | H0 and reverted, but for the band case, kept and green on the reverted tree | closed; `2026-09-04_plan_review` closed it, S023's accepts naming the instrument |
| `2026-08-13_plan_review.2-F07` | S062 | no completed step called pending, the checker red then green | closed; `2026-08-16_plan_review` closed |
| `2026-08-13_plan_review.2-F09` | S064 | the retention stated as the checker implements it | closed; `2026-08-16_plan_review` closed |
| `2026-08-14_test_review-F01` | S067 | a reachable position with three legal moves; F01 to F06 "planned, not closed" until a re-run | closed; `2026-09-04_test_review` closed |
| `2026-08-14_test_review-F02` | S067 | a legal mate at both sites; the same | closed; `2026-09-04_test_review` closed |
| `2026-08-14_test_review-F03` | S067 | the timeout follows the build type; the same | closed; `2026-09-04_test_review` closed as filed, its second half that report's F01, closed by S190 |
| `2026-08-14_test_review-F04` | S067 | `legal_moves()` states INV-1 and asserts it; the same | closed with S193; `2026-09-04_test_review` found it open again in a new wording and folded it into its F05, which S193's R7 closed |
| `2026-08-14_test_review-F05` | S067 | three two-minor endings pinned; the same | closed; `2026-09-04_test_review` closed |
| `2026-08-14_test_review-F06` | S067 | the ordering budget measured; the same | closed; `2026-09-04_test_review` closed |
| `2026-08-16_plan_review-F04` | S072 | the nine sweep files tracked | closed; `2026-09-04_plan_review` closed |
| `2026-08-16_plan_review-F05` | S069 | both items rewritten from their sources; planned until the re-run | closed; `2026-09-04_plan_review` closed |
| `2026-08-16_plan_review-F06` | S070 | a tool-verified pair; planned, not closed, until a re-run | closed; `2026-09-04_plan_review` closed |
| `2026-08-16_plan_review-F07` | S074 | S021's accepts names the three mate cases | closed; `2026-09-04_plan_review` closed |
| `2026-08-16_plan_review-F09` | S078 | both bodies corrected, checked by a script | closed; `2026-09-04_plan_review` closed |
| `2026-08-16_plan_review-F10` | S071 | the citations name the version and the symbols | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F01` | S138 | 107 of 201 citations stale, all corrected, `--citations` built | closed with S187; `2026-09-04_plan_review` found the wrong-test hazard gone and the census form back, its own F07, which S187 closed |
| `2026-08-20_plan_review-F02` | S139 | S119 asks for the S105 harness setting | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F03` | S139 | S136 derives the count at its own HEAD; DEC-092's literals left to the owner, a proposal appended | closed; `2026-09-04_plan_review` closed it for the plan documents and left the decision's literals to the owner; S136's guard was re-derived under DEC-322 instead |
| `2026-08-20_plan_review-F04` | S139 | S125 drops the S118 dependency | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F05` | S140 | `specs.md` no longer routes check extensions to S096 | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F06` | S141 | two `touches:` repaired, `--touches` in the fast suite | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F07` | S139 | S109's exemption list split | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F08` | S142 | the declared minimum moved off the values recorded red, re-measured first | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F09` | S085 | the run; nothing on F09 | closed; `2026-09-04_plan_review` closed, the goal amended by DEC-094 |
| `2026-08-20_plan_review-F10` | S140 | three `excludes:` re-pointed | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F11` | S140 | block 3 names S134 to S136 | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F12` | S138 | S115's source named | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F13` | S138 | the sites quote the real titles | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F14` | S142 | the history bound held under the band | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F15` | S139 | S117's exemption removed | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F16` | S140 | the census replaced by a recipe | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F17` | S140 | the numbering ends at INV-6, recorded | closed; `2026-09-04_plan_review` closed |
| `2026-08-20_plan_review-F18` | S085 | the run; nothing on F18 | closed; `2026-09-04_plan_review` closed, S085 done |
| `2026-08-22_adversarial-F01` | S161 | rights and squares cleared, red first | closed; `2026-09-03_adversarial` closed |
| `2026-08-22_adversarial-F04` | S165 | both edges guarded, `{-5, 0}` H1; "Closes" | closed; `2026-09-03_adversarial` closed |
| `2026-08-22_adversarial-F05` | S163 | one decision under the session mutex, red then green; "Closes" | closed; `2026-09-03_adversarial` closed, its stress harness not re-run |
| `2026-08-22_adversarial-F07` | S164 | one case pins the bound stand pat; "Closes" | closed; `2026-09-03_adversarial` closed |
| `2026-09-04_adversarial-F01` | S210 | `go infinite` holds open on `stop`, "2026-09-04 F01 with it" | closed; no re-run since |
| `2026-09-04_adversarial-F02` | S210 | a bad `moves` token refuses the whole command | closed; no re-run since |
| `2026-09-04_adversarial-F03` | S210 | a carried root move hint replaces the table premise | closed; no re-run since |
| `2026-09-04_plan_review-F02` | S181 | every Lynx figure banded; "Closes" | closed; no re-run since |
| `2026-09-04_plan_review-F03` | S182 | the cost line from the ledger; "Closes" | closed; no re-run since |
| `2026-09-04_plan_review-F04` | S183 | the arithmetic's inputs recorded; "Closes" | closed; no re-run since |
| `2026-09-04_plan_review-F06` | S185 | every figure sourced or marked unverified; "Closes" | closed; no re-run since |
| `2026-09-04_plan_review-F07` | S187 | every pending citation names a symbol, the freshness check gated | closed; no re-run since |
| `2026-09-04_test_review-F01` | S190 | INV-2 and INV-4 checked by the gate; "Closes" | closed; no re-run since |
| `2026-09-04_test_review-F07` | S189 | `bench` in the dispatch table, `tools/gate.sh` checks the message | closed; no re-run since |
| `2026-09-10_adversarial-F01` | S211 | the three tables derived, every value identical | closed; no re-run since |
| `2026-09-10_adversarial-F02` | S211 | the builders rewritten, the handle renamed | closed; no re-run since |
| `2026-09-10_adversarial-F03` | S211 | the inventory written, three unsourced assets deleted, the false sentence replaced; seven sound files named an unsourced gap | closed; no re-run since |
| `2026-09-10_adversarial-F04` | S212 | resignation two-sided in both harnesses | closed; no re-run since |
| `2026-09-10_adversarial-F05` | S212 | `id name` carries the sha, the identity check | closed; no re-run since |
| `2026-09-10_adversarial-F06` | S212 | a cached reference played only if clean and configured alike | closed; no re-run since |
| `2026-09-10_adversarial-F07` | S212 | the S088 regime gets no flag; `specs.md` corrected 2026-09-11 | closed; no re-run since |
| `2026-09-10_adversarial-F16` | S039 | H0 on retiring the shortcut and the clamp together; both kept | **left `planned`**: its own line, moved 2026-10-21, names S122 as where the clamp is removed or kept and says the clamp's own cost is unmeasured; S122 is in `plan_current/` |

## 2026-11-03: found during the step, eleven more lines read

The nine `planned` findings no step names (the step's "other twelve" less the three dotted ids above) and the two
`open` findings. The nine name no live step: S080, S079 and S081, on three of their lines, are among the steps
DEC-086 retired, folding each correction into the step it corrects and saying that "closure still needs a re-run that
no longer reports it". The re-run is `2026-09-04_plan_review`, a plan_review re-run at `18deccf`, whose "Verdicts on prior findings" gives one
entry per prior finding. A line moved on a re-run takes the form the record already has for one,
`closed — <report> re-ran this report and does not report it`, the 2026-08-13 plan_review's nine, with the entry's
own words in parentheses. An `open` moves to `accepted` only where a decision rules on the finding and both re-runs'
entries agree, and to `planned` where a pending step names it in `closes:`.

| finding | the record | new status |
|---|---|---|
| `2026-08-13_plan_review.2-F01` | the re-run's F01 entry: "does not reproduce", S055 re-pins the thresholds in its own commit, **closed** | closed |
| `2026-08-13_plan_review.2-F02` | entry F02: "does not reproduce as a plan defect", **closed** | closed |
| `2026-08-13_plan_review.2-F03` | entry F03: "does not reproduce", S030 splits the sites by what they key on, **closed** | closed |
| `2026-08-13_plan_review.2-F04` | entry F04: "does not reproduce", S025's accepts outcome-neutral, **closed** | closed |
| `2026-08-13_plan_review.2-F05` | entry F05: "does not reproduce", S026 retired into S109, whose accepts carries the mate clause, **closed** | closed |
| `2026-08-13_plan_review.2-F08` | entry F08: the three re-pointed, the class recurring as that report's F07, **closed**; that F07 is closed above by S187 | closed |
| `2026-08-16_plan_review-F01` | entry F01: S057 folded into S039 by DEC-086, **closed**; the stale comment it names beside it is `2026-08-13_adversarial-F05`, closed by S287; the line's S080 retired by DEC-086 | closed |
| `2026-08-16_plan_review-F02` | entry F02: re-anchored by S138 and S169, the class recurring as that report's F07, **closed**; that F07 closed by S187; the line's S079 retired by DEC-086 | closed |
| `2026-08-16_plan_review-F03` | entry F03: S056 folded into S055, **closed**; the line's S081 retired by DEC-086 | closed |
| `2026-08-22_adversarial-F06` | DEC-121, by the owner, 2026-09-02, names "the 2026-08-22 audit" and "finding F06" and rules the 781 constants the Polyglot format's published specification, kept and cited at the table; S146, in `plan_done/` with the id in its `closes:`, placed the citation, its section "The table half is settled: DEC-121"; `2026-09-03_adversarial` and `2026-09-04_adversarial` both read "accepted (DEC-121; report says open)" | **accepted** |
| `2026-08-21_adversarial-F04` | DEC-108 names neither the audit nor the finding; it drops the intermediate checkpoint and moves S152 to near the goal, "Deferred is not retired". S152, in `plan_todo/`, names the finding in its `closes:`. The two adversarial re-runs agree on "accepted (DEC-108)"; `2026-09-04_plan_review` files it "planned" under "recorded, not moved"; `2026-09-12_plan_adversarial_review` reads it "explicitly deferred by DEC-108/DEC-179" | **`planned — S152`**, the coordinator's ruling: what holds is that no decision names the finding and that a pending step does. The line names S152, DEC-108's words and all three readings |

## 2026-11-03: Part E's eighteen, after S134's precedent

Part E of `2026-09-10_adversarial` holds 21 short-form findings, `**Fnn ` paragraphs under the header "open on all of
them". Three had a `Status:` line: F25, which S287 inserted in `1174478` on 2026-10-21 as `planned — S288` and S288
moved to `closed` in `bb3d31d`, and F34 and F36, which S134 inserted in `0d9faba` on 2026-10-12; each on its own
paragraph after the finding's, followed by a blank line, `Previously: open (this report's header: open on
all of them).` DEC-197 had refused adding such lines on 2026-09-12 ("they never had one, and adding is not the moving
a report takes"); S134 and S287 added them after it, and DEC-346 (2026-11-03) rules the insertions: a finding whose
status is set by its report's header takes its own `Status:` line, in S134's form, when that status moves, and that
one rejected option of DEC-197 is overruled while the rest of DEC-197 stands. The eighteen
inserted lines take that placement and that `Previously:`, with the rest of this step's form, the em dash the three
existing Part E lines use, and each was read against its step's stamp as above. `2026-10-08_performance` re-measured
F17, F19, F20 and F21 and asked for their lines to be moved; that is the re-run they name.

| finding | closing step | what the stamp says | new status |
|---|---|---|---|
| F17 | S210 | the halfmove clock saturates at 255 | closed; `2026-10-08_performance` closed by re-run |
| F18 | S210 | a `moves` list past the cap refused, the whole `position` command transactional | closed; no re-run since |
| F19 | S210 | `go infinite` clears every finite limit and holds open on `stop` | closed; `2026-10-08_performance` closed by re-run |
| F20 | S210 | `movestogo 0` is sudden death | closed; `2026-10-08_performance` closed by re-run |
| F21 | S210 | the first iteration honours `stop` and the hard timer | closed; `2026-10-08_performance` closed by re-run |
| F22 | S210 | quiescence scores a dead board a draw, `--nonreg` H1 | closed; no re-run since |
| F23 | S210 | the comment names FIDE 5.2.2 and 6.9 and the accepted omission | closed for the justification, the behavioural half staying accepted as `2026-08-14_test_review-F05` (DEC-170); no re-run since |
| F24 | S126, in `plan_todo/` | not done | **`planned — S126`**, the step DEC-170 gave it |
| F26 | S213 | the zero-weight blocks describe the weights that ship | closed; no re-run since |
| F27 | S213 | the mate-band argument names what bounds the score, with a test | closed; no re-run since |
| F28 | S214 | `tools/analyse_game.py` fails by name where it returned 0.00 | closed; no re-run since |
| F29 | S214 | `check` probes every axis | closed; no re-run since |
| F30 | S199 | the reading rule carries F30's conversions term | closed; no re-run since |
| F31 | S212 | a termination outside the three expected voids the run | closed; no re-run since |
| F32 | S212 | the busy guard reads the one-minute load average | closed; no re-run since |
| F33 | S213 | the dead API deleted, the comments and casts corrected | closed; no re-run since |
| F35 | S214 | the tuner's stamp carries the four missing options | closed; no re-run since |
| F37 | S186 | both divergences disposed in the strategy document | closed; no re-run since |

So 13 more lines close on the stamp alone, beside the 18 above.

**Counts.** On `1bb3080`: 72 `planned`, 70 `closed` (one of them `closed,`), 6 `accepted`, 2 `open` outside a fence;
18 findings with no `Status:` line. After: **3 `planned`, 158 `closed`, 7 `accepted`, 0 `open`, 0 without a line**;
73 lines moved in eleven reports and 18 inserted in one.

**No other byte.** `--against-rev 1bb3080` proves what DEC-346's Consequences admit and nothing more: besides
changed `Status:` lines, only a `Status:` line and an exactly empty line inserted together under a finding that had
none, in `2026-09-10_adversarial` alone. Each inserted line must sit on its own paragraph between two exactly empty
lines, after its finding's text and before the next finding, a `---` rule or a heading; a second `Status:` line under
any finding is refused. With the inserted pairs taken out, each report must be `1bb3080`'s line for line, a line
differing only where both are a finding's `Status:` line. Over the edited tree: `S328-REPORTS-STATUS-ONLY`, 73
changed, 18 inserted, refusals 0 in all seventeen reports, the only line count moving `2026-09-10_adversarial`'s, 1045
to 1081. Eight plants, each in a copy read with `--tree`, read red with their own reasons: a space on the empty line
after an inserted line, and after a changed one; a pair inside a paragraph; a second line under a finding that has
one; a pair in another report; a character on a finding's text; a lone empty line; an inserted line deleted. By
`git diff -U0`: 0 changed lines that are neither `Status:` nor empty, 18 added empty lines, 0 removed. The second
reader's planted four, which the earlier proof passed, are the first four of these.

**Docs.** `DEV_MANUAL.md` and `MANUAL.md` checked: neither states an audit status. No change.

**Found beyond the step, not fixed.** S122's `closes:` does not name `2026-09-10_adversarial-F16`, which that
finding's line hands to S122, so a census by `closes:` reads S039 as its only step. The coordinator's, at S122's close.
