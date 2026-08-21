id:         S140
goal:       no plan, specs or status document routes work to a retired id, cites an invariant that is defined nowhere, or carries a census its own derivation contradicts
accepts:    `adocs/specs.md` no longer routes check extensions to S096, which DEC-087 retired, and says instead what the current answer is -- this one matters because specs outranks plan by precedence, so the retired route is the one a reader is told to trust; the three `excludes:` fields naming retired step ids either name a live step or state that the work is retired and why; `plan.md`'s block 3 prose names S134, S135 and S136, which its own list contains but its prose omits; INV-7 is either defined in `specs.md` with a number and a testable property or the commits referencing it are the only record and `specs.md` says which invariant they meant; `status.md`'s enrichment census is regenerated from `grep -L 'Technical details (SOTA research' adocs/plan_todo/*.md` and agrees with it, or states that the census is a snapshot and carries its date; **added mid-step, see "A sixth item, folded in from S139" below** -- S136's `goal:` line and its `plan.md` list entry, which both still carry the pre-S055 count of three taper divisions, are corrected to the count S139 derived, re-derived here from `S055_taper_stage_two_once.md` and `tests/test_eval_model.cpp` rather than copied
touches:    adocs/specs.md, adocs/plan.md, adocs/plan_todo/, adocs/status.md
excludes:   the enrichment pass itself, which is the owner's to resume; any renumbering of ids, which never happens; adocs/plan_done/
decisions:  DEC-087
closes:     2026-08-20_plan_review-F05, 2026-08-20_plan_review-F10, 2026-08-20_plan_review-F11, 2026-08-20_plan_review-F16, 2026-08-20_plan_review-F17
blocks:
paused_by:
done:      2026-08-21. Six items -- five from the accepts, one folded in mid-step -- each traced
            to the file that decides it, with the trace quoted in this file. No code touched, no
            match run.
            
            1. specs.md no longer routes check extensions to S096. The "Open items" sentence now
               opens "Check extensions have no step and are not owed one" and records S096 as
               retired outright by DEC-087 (a), id not reused, with the three reasons named:
               Ethereal +4.1/+4.5, Stormphrax as a simplification, and src/search.cpp:710 already
               refusing to reduce a checking move (!is_in_check && !is_check_move), which its own
               comment at :700-702 states in prose. This is the one with teeth: specs outranks
               plan, so the retired route was the one a reader was told to trust.
            
            2. Three excludes fields re-pointed. S098 to S109 for late move pruning (S090 retired
               into it, DEC-082) and to S108 for the improving flag (S092 retired into it,
               DEC-081) -- both precede it, at list entries 22 and 24 against S098 at 26. S097
               names the retirement instead of the id. S042 does too, quoting adocs/plan.md:110
               for S031, which is that retirement's only record: no decisions.md entry names it.
            
            3. plan.md's block 3 prose. Header range S082-to-S126 corrected to S134-to-S126,
               matching the list; S134 named as the block's opener -- the bit-exact fold of the
               two degeneracies S100 found, discharged on node counts under DEC-090, PARAM_COUNT
               to 823, and the only pending step that declares blocks:; S135 and S136 named after
               S083 as the two zero-weight groups held at zero rather than measured to it. "What
               this costs" deliberately untouched: its 45-to-55 range absorbs two one-verdict
               steps, S134 owes none, and re-deriving its "six steps" needs the enumeration behind
               the six, which is S150-class work.
            
            4. INV-7: no invariant is allocated, the record is corrected instead. specs.md's
               Invariants section now says the numbering ends at INV-6, names 68a61d0 as the only
               citation of the number anywhere outside git, and reads it as enforcing AGENTS.md
               sections 2 and 10. The commit body decides it -- "plan_done/ is append by move only:
               the file is the record of what the step was when it completed" -- which is a rule
               about which directories an agent may write to and not a property of the engine.
               git log --all --grep=INV-7 returns that one commit and nothing else. Defining INV-7
               would owe an adocs/testing.md row, and testing.md is not in this step's touches.
            
            5. status.md's enrichment census. The twenty-id enumeration and "28 of 48" are gone
               and the item says why -- both ends move. The grep -L recipe is the answer, with
               grep -l named for the done side and the note that neither reaches plan_current/.
               One number is kept and labelled: snapshot, 2026-08-21, 22 enriched and 44 not over
               the 66 files in plan_todo/, decomposed into 5 excluded by design, 19 created after
               the pass stopped, and the 20 the pass itself left. The item's closing sentence about
               S109's gives-check clause was stale too and now says what S139 left of it.
            
            6. Folded in by the orchestrator from S139's leftover and recorded as a scope
               amendment rather than applied silently (AGENTS.md sec 4; the accepts carries it and
               a new section explains it). S136's goal: and its plan.md list entry both said
               "three divisions", the pre-S055 count. Both now say tempo's zero weight holds the
               guard one division down, at two once S055 has landed. Re-derived and not copied:
               tests/test_eval_model.cpp:242-249 names four taper divisions and says three round
               today because tempo ships at zero; S055_taper_stage_two_once.md:2-3 merges two and
               re-pins to 2 x 23/24 = 1.917; adocs/plan.md orders S055 at entry 39 (:359) against
               S136 at 50 (:370) -- line numbers re-taken after `--step done` pruned S138's
               completed entry, which shifted every list line after entry 14 up by one; S125's
               two plan.md citations were re-taken with them. It is here because two lines against
               an 80-entry queue is not a step, both files are already in touches:, and S139 was
               right to refuse to widen its own.
            
            Repaired in the same commit, both caused by this step's own edits. S125:32 and :47
            cited plan.md ranges this step moved -- :251-253 to :260-262 for the block-3 rewrite,
            and :367-368 to :374-375 for that plus the completion prune, the latter already one
            entry low before either, naming 55 and 56 where its own sentence claims 54 and 55. And editing S098's excludes resets its drift baseline, which would have
            silenced its two DRIFT flags; both were inspected first and both were stale, so both
            are corrected -- src/search_params.hpp:122-123 to :158-159 for LMR_BASE/LMR_DIVISOR
            and tests/test_search.cpp:1883-1904 to :1919-1940 for the null-move mate case, each
            verified byte-identical between the old range at 7b54f18 and the new range at HEAD.
            
            Gates: build clean, ctest -L fast 18/18 in 14.97 s, clang-format --check clean,
            moltke --validate clean. tools/plan_prose_check.py 50 -> 48 flagged over 67 files,
            --prose exit 0. Unlike S139's drop this one is a fix and is claimed as one: the only
            two flags that stopped being reported are S098's, both were read before the edit, both
            were wrong, and both are corrected. No new flag survives. README.md owner-written and
            untouched; MANUAL.md and DEV_MANUAL.md checked -- none of the three names S096, INV-7,
            S134/S135/S136 or the enrichment pass, so no change needed.

## The one with teeth

Reading protocol precedence is **specs > plan > status**. `specs.md` still sends
check extensions to S096 and DEC-087 retired S096 -- Ethereal and Stormphrax both
removed check extensions for a gain, LMR here already exempts checking moves, and
S097 covers the forcing-line concern. So the document a reader is told to trust
above the plan points at work the plan has deleted. The rest of this step is
hygiene; this part is a contradiction between the two top documents.

**INV-7 is referenced by the repository's own history and defined nowhere.** An
invariant number that commits cite and `specs.md` does not define is either a
missing invariant or a wrong citation, and both are worth one line to settle.

## A sixth item, folded in from S139

**Added to `accepts:` mid-step by the orchestrator, and recorded here rather
than applied silently** (AGENTS.md section 4: never deviate silently). S139 fixed
S136's `accepts:` arithmetic and could not fix the two goal lines that state the
same count, because moving S136's list entry -- `adocs/plan.md:363` as the file
stood at `45a39a5`, `adocs/plan.md:370` after this step -- needed
`adocs/plan.md` in a `touches:` field S139 did not have. It said so in its own stamp and in S136's
body rather than widening its scope, which was right.

This step already has `adocs/plan.md` and `adocs/plan_todo/` in `touches:`, and
a goal line carrying a count its own `accepts:` contradicts is the defect class
this step is named for. A separate step for two lines against an 80-entry
pending queue is overhead the queue does not need. So it is folded in, and the
count is **re-derived** rather than copied from S139: `tests/test_eval_model.cpp
:242-249` names four taper divisions and says three can round today because
tempo ships at zero; `S055_taper_stage_two_once.md:2-3` merges the mobility and
king-safety divisions and re-pins the guard to `2 x 23/24 = 1.917`; the plan
orders S055 at entry 39 (`adocs/plan.md:359`) against S136 at entry 50
(`adocs/plan.md:370`). Three divisions exist when S136 runs and **two** can
round, so "three divisions" in the goal is the pre-S055 count and the corrected
statement is that tempo's zero weight holds the guard one division down, at two
once S055 has landed.

## Cost

No match, no build. Document work.
author:    Maksym Bodnar
