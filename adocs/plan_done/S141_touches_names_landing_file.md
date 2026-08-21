id:         S141
goal:       every pending step's touches field names the file its change has to land in
accepts:    S039's `touches:` names `src/search_params.hpp`, which is where `LAZY_EVAL_MARGIN` has lived since S073 and without which the step cannot change the constant its goal is about; S120's `touches:` likewise names the files its change lands in; a tracked check reads each pending step's `touches:` and reports a step whose goal names a symbol that no listed file contains, and it is run and green at completion
            (**The S085 clause is amended out, 2026-08-21, and discharged in the
            stamp instead of produced -- see "The S085 clause" below.** S085
            completed on 2026-08-21 and its file is `adocs/plan_done/`, which
            AGENTS.md sec 10 hard-prohibits writing; the clause could not be
            satisfied by any means this step is allowed to use. It also did not
            need to be: the correction landed at `4fc359f` while the step was
            still in `plan_current/`.)
touches:    adocs/plan_todo/, tools/plan_prose_check.py and tests/CMakeLists.txt
            for the checker, adocs/testing.md for its ledger row,
            adocs/decisions.md for the proposal, DEV_MANUAL.md for how the check
            is invoked
excludes:   implementing S039, S085 or S120; widening a touches field to cover work the step excludes; adocs/plan_done/, which is where S085's file is
decisions:  DEC-100
closes:     2026-08-20_plan_review-F06
blocks:
paused_by:
done:      2026-08-21. Two pending touches fields repaired, one accepts clause discharged, and the
            class put behind a tracked check. No code touched, no match run.
            
            1. S039. touches: was `src/evaluation.hpp LAZY_EVAL_MARGIN and its comment` and now
               names src/search_params.hpp first -- where the value has lived since S073, at
               src/search_params.hpp:164, and where a re-decision has to land. src/evaluation.hpp
               is kept for the comment, which is all it carries: every occurrence of the symbol in
               that header is inside a `//` comment, including the trailing one on the #include at
               :3, so the file the step was scoped to could not have held the change. The run its
               accepts records is named as adocs/data/S039_eval_spread.log, a file rather than the
               directory, because a directory buys abstention from the check below and adocs/data/
               holds .hpp files from S075's fit dumps. S039's accepts was NOT rewritten: the audit
               suggested splitting the value and the comment into two obligations, but rewriting an
               accepts is S139's class, this step's excludes forbids implementing S039, and the
               accepts already presupposes the number moving ("an SPRT ... if the margin changes").
            
            2. S120. Adds src/chesso.cpp for the two clears its own section 5 requires -- beside
               tt_reset at ucinewgame (src/chesso.cpp:1128) and on the LazyEvalMargin setoption in
               the tune build (command_setoption at src/chesso.cpp:986, the CHESSO_TUNE branch at
               :1058) -- and tests/test_evaluation.cpp for the red-first properties section 3(a)
               lists. Neither was reachable under the old field. src/search_params.hpp is
               deliberately NOT added: S120's excludes disclaims choosing the margin, the setoption
               clear lands in the handler and not in the table, and adding it would be the widening
               this step's own excludes forbids. Its placement argument said "`touches:` names
               evaluation files only", which this edit falsified; corrected to "names no search
               file", which is the claim the argument actually needs.
            
            3. S085 -- discharged, not produced, and the accepts amended to say so (AGENTS.md sec 4:
               never deviate silently). S085 completed at 43bf189 and its file is in
               adocs/plan_done/, which AGENTS.md sec 10 hard-prohibits writing and a PreToolUse hook
               on Write|Edit refuses; going around the hook through a shell would have been the same
               prohibition broken more quietly. It also needed nothing: the correction landed at
               4fc359f while the step was still in plan_current/, and the field now names
               src/search_params.hpp with a sentence saying the original line named src/search.cpp
               and src/evaluation.hpp, which only consume the parameters. The audit report already
               recorded that -- "Planned in: S141; S085's own third of it fixed 2026-08-20" -- so the
               clause was true before this step started and the accepts had simply not caught up.
               And a completed step's touches: constrains nothing anyway: it is a contract read
               forward against a diff, and no further diff lands under it. DEC-100 proposes the rule
               that would have caught this when the accepts was written, marked as an agent proposal
               and not owner-approved.
            
            4. The check. `tools/plan_prose_check.py --touches`, a third mode of the existing tool
               rather than a new script -- same corpus, same pending-file walk, same reporting
               shape, and `--citations` had already built _tracked(), _at() and pending_step_files().
               It reports a pending step whose goal: names a code symbol that no file its touches:
               allows it to edit carries **outside a comment**. The comment-stripping is the whole of
               the S039 case and not a refinement: src/evaluation.hpp does contain LAZY_EVAL_MARGIN,
               seven times, all of them prose. Four symbol classes and no more -- ALL_CAPS with an
               underscore (which keeps SPRT, UCI and NNUE out), a call written with its parentheses,
               a _t type, and a UCI option name matched against the names src/search_params.hpp
               declares rather than against a CamelCase pattern that would read Stockfish and GitHub
               as symbols. A match carrying a file extension is a path, not a symbol: S157's goal
               says "DEV_MANUAL.md says which scale", and DEV_MANUAL read as a constant sent the
               check hunting for a definition of it.
            
               What it deliberately does not catch, written into the docstring so nobody trusts it
               further than it goes: a symbol named without those markings (`tempo`,
               `piece_placement_mg`, `evaluate` without parentheses); a landing site no symbol points
               at, **which is S120's own defect** -- its goal names nothing at all, and src/chesso.cpp
               was found by reading its pitfalls, not by the checker; a touches: that names too much
               rather than too little; and anything about accepts:, which is S139's and S140's ground.
               Two ungated note classes: a symbol in code nowhere, and a directory in touches: that
               already holds files of the symbol's kind, since a directory may grow a file that does
               not exist yet -- the same abstention --citations makes on a bare `:line`. The kind test
               narrows that and does not close it, and adocs/data/ is the measured hole: it holds
               .hpp because S075 dumped six fitted tables there. The script excludes its own file
               from the corpus, which is not fastidiousness -- its docstring names LAZY_EVAL_MARGIN
               and search_param_info(), and before the exclusion S150's `tools/` came back satisfied
               by this paragraph.
            
            5. Wired to run, and DEV_MANUAL corrected. Registered as test_plan_touches under the
               fast label, 0.06 s. DEV_MANUAL said "Neither check is registered with ctest, and that
               is deliberate" and gave the reason -- any source commit shifts lines under fifty step
               files at once, so gating on citation freshness makes red the normal state and this the
               check that gets weakened to clear it. That reason does not reach --touches, which
               holds no line numbers and moves only when a step file is written or a symbol changes
               file; the paragraph now says so and names which mode is registered and which two are
               not. adocs/testing.md carries the ledger row.
            
            Red first, and observed rather than assumed. Before the repair, both through the script
            and through ctest -R test_plan_touches (exit 8):
            
              adocs/plan_todo/S039_lazy_margin_redecide.md: 1 symbol(s) in goal, 1 flagged
                TOUCHES adocs/plan_todo/S039_lazy_margin_redecide.md  LAZY_EVAL_MARGIN  -- in code
                at src/evaluation.cpp:1045, src/search_params.hpp:164, tests/test_evaluation.cpp:301
                and 4 more; touches names src/evaluation.hpp
              touches flagged: 1 over 66 files (4 symbols read, 0 noted and ungated)
            
            Constructed cases, each reverted. S154's touches: cut to fastchess.sh -> TOUCHES
            RfpMinPly, in code at src/search_params.hpp:138. The same cut to adocs/testing.md did
            **not** flag, and that is the better evidence: testing.md genuinely names RfpMinPly, so
            the containment test is a real read and not a rubber stamp. S039 with tools/ appended ->
            clean pass, because tools/eval_model.hpp:976 carries the symbol; with adocs/data/
            appended -> note and abstention, which is the limit above and why S039 names a file.
            
            Gates: build clean, ctest -L fast 19/19 in 14.97 s (was 18), clang-format --check clean,
            tools/plan_prose_check.py --touches 0 flagged over 66 files exit 0, --prose 0 flagged.
            
            --citations 46 -> 44 flagged over 66 files, and the drop is a fix and not the baseline
            trap S139 fell into. Exactly two flags disappeared and none appeared, diffed line by
            line: S120:83 and S120:187, both citing src/search_params.hpp:128 for the
            X(LAZY_EVAL_MARGIN, ...) row. Both were live-wrong -- :128 holds S154 prose inside the
            RfpMinPly comment block at HEAD and the X row is at :164 -- and both were re-taken to
            :164 before the edit reset their drift baseline, which is the order that matters. Nothing
            else in S120 could have been masked by the reset: re-running DRIFT with the baseline
            forced back to 7b54f18 returns exactly those two citations and no third. S039 returns 0
            forced, as it did before the edit.
            
            README checked, owner-written, no change needed. MANUAL.md checked, no change: a plan
            check is developer-facing and the UCI surface is untouched. adocs/specs.md checked, no
            change: no engine behaviour moved and it holds no test inventory. DEV_MANUAL.md and
            adocs/testing.md changed as above.

## Why a checker and not three edits

`touches:` is the field that says where a change is allowed to land, so a step
whose goal is to change a constant and whose `touches:` omits the file holding
that constant is a step that cannot be completed without violating its own
scope. S039 is the sharp case: its whole goal is to re-decide
`LAZY_EVAL_MARGIN` from measured spread, and the constant moved to
`src/search_params.hpp:164` when S073 built the tune build. S085 had the same
defect for the same reason and its own research section already flagged it in a
Scope concern -- which is evidence that catching this by reading does not scale,
and that a check does.

## The S085 clause

The accepts named three steps and only two of them could be acted on. **S085
completed**, on 2026-08-21 at `43bf189`, and its file is
`adocs/plan_done/S085_spsa_first_run.md`. AGENTS.md sec 10 hard-prohibits
writing to `adocs/plan_done/` and a `PreToolUse` hook on `Write|Edit` refuses it
in this harness, so no edit to that file was available to this step by any means
it is allowed to use. Editing it around the hook, through a shell, would have
been the same prohibition broken more quietly.

It also did not need to be edited. The correction landed at **`4fc359f`, while
the step was still in `plan_current/`**, and the field now reads
`src/search_params.hpp -- where the defaults have lived since S073, and where
the post-run edit lands`, followed by a sentence saying the original line named
`src/search.cpp` and `src/evaluation.hpp`, that those only consume the
parameters, and that S141 owns the other two thirds of the finding. The audit
report says the same in its own header: "Planned in: S141; S085's own third of
it fixed 2026-08-20". So the clause was already true when this step started, and
the accepts had simply not been amended to say so.

And on a completed step the field constrains nothing anyway. `touches:` is a
scope contract read forward, against a diff that has not been written yet. Once
`done:` is stamped and the file is in `plan_done/`, no further change lands
under it: the field is historical record of what the step was allowed to touch,
and `plan_done/` is never rewritten precisely so that record stays true. A
correct `touches:` there is worth having because the next reader learns from it,
which is why `4fc359f` was right to make the edit at the last moment it was
legal -- not because anything is still gated on it.

DEC-100 proposes the rule that would have caught this when the accepts was
written, and marks it as an agent proposal.

## Cost

No match. Document work plus a checker, and the fast suite green.
author:    Maksym Bodnar
