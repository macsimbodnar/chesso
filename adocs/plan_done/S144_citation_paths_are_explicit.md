id:         S144
goal:       a citation in a plan document carries its own path, so the 467 bare line references that inherit a path from prose become checkable
accepts:    a stated style rule that a citation repeats its path, written where the plan documents' conventions live; the bare `:line` references in pending step files carry a path, resolved by reading the prose that owns each one and never by guessing -- a guess is what this step exists to remove; `tools/plan_prose_check.py --citations` gates the converted references instead of reporting them as `loose, ungated`, and the count it reports as ungated falls to zero or to a stated remainder with a reason per item; the checker is observed red on a converted reference given a wrong path before it is trusted; no reference is resolved by deleting it
touches:    adocs/plan_todo/, adocs/plan.md, adocs/specs.md, tools/plan_prose_check.py, and -- amended at completion -- adocs/data/ for the generator and its mapping, adocs/decisions.md for DEC-120, DEV_MANUAL.md because the checker gained a failure class and the DOCS rule owes it a description
excludes:   adocs/plan_done/, which is history and is never rewritten; the numeric claims those sentences make, which the 2026-08-20 plan_review listed as deferred; re-anchoring the full citations, which S138 did
decisions:  DEC-120
closes:
blocks:
paused_by:
done:       2026-09-01. **383 bare `:line` continuations, 380 converted.** They
            sat in 19 of the 53 pending step files; the three left are this
            step file's own illustrations of the defect, and they leave the
            checked set when this file moves to `plan_done/`, which is what
            takes the reported count to zero.
            **The goal line says 467 and it is left as written**: that was the
            count at S138, and both ends have moved since -- every completion
            takes a file out of the set and every new step puts one in. 383 is
            the count at this step's own HEAD and it is stated here, which is
            where a number that moves belongs.
            `tools/plan_prose_check.py --citations`: **0 flagged over 53
            files**, `0 bare and 0 document, ungated`.

            **The style rule** is in `adocs/plan.md`'s "How this file works",
            beside the ordering rules a step file's author already reads: a
            citation repeats its path. Its enforcement is a fourth flag class,
            `BARE`, which fails a continuation with no path of its own and
            **never tries to resolve one** -- S138 measured that guess at sixty
            impossible line numbers and the abstention stays. DEC-120.

            **How each was resolved, and how many.**
            `adocs/data/S144_paths.py` does not infer the path, it is *told*
            it: 77 paragraph assignments and 48 per-citation overrides, each
            read out of the citing sentence. It then makes the assignment
            falsifiable by relocating the text the citation was written
            against, from the commit that wrote that line -- `git blame
            --ignore-rev fe25f46`, because S169 rewrote the full citation on
            many of these lines and a plain blame answers HEAD.
            **318 BLOCK** (the baseline text occurs exactly once in the owning
            file at HEAD), **4 SAME** (it occurs several times, one of them
            where the citation already pointed), **58 HAND**.

            **Why the checker's green is not the proof, again.** A wrong path
            is caught only by BOUNDS or ANCHOR, and DRIFT is blind on the
            commit that writes the citation. Observed both ways before
            trusting it: `src/search_params.hpp:181-182` mistyped as
            `src/search.hpp` gave `BOUNDS ... src/search.hpp has 99 lines` and
            exit 1; the same range mistyped as `src/search.cpp` -- in range,
            wrong file -- passed green. The evidence is
            `adocs/data/S144_pathings.tsv`, 380 rows carrying the baseline text
            and the text at the new range: **343 are identical on both sides,
            and all 37 that differ are HAND rows** whose reason column says
            what was read instead. No BLOCK or SAME row differs.

            **What the 58 hand rows are.** Two shapes and nothing else. The
            block sits in both `quiescence()` (`src/search.cpp:293`) and
            `negamax()` (`:589`) -- `make_move`, `unmake_move`, the abort
            check, the node counter, `tt_get_entry`, `move_t moves[MAX_MOVES]`
            -- and only the sentence says which. Or the cited region grew a
            comment under the citation: DEC-102 added six lines inside
            quiescence's stand-pat store, S149 rewrote the fail-high block's
            middle, S162 grew negamax's halfmove-clock test from one line to
            nine.

            **One citation was malformed, not merely pathless**, and that is
            why it survived every earlier pass. S120 read `:1099-:1039`: a
            range whose second half carries its own colon, so the citation
            regex reads the first number and *nothing at all* sees the second.
            The first half had been re-anchored and the second had not, 60
            lines apart. Repaired to `src/evaluation.cpp:1099-1100`, the two
            lazy-shortcut bound returns, recorded as `TEXT_FIX` in the
            generator rather than done silently.

            **Three sentences are stale in content and are left that way**, per
            `excludes:`. S113, S114 and S099 each say the static evaluation is
            set inside the RFP guard; S108 hoisted it to the top of the node
            (`src/search.cpp:720` and `:731`). The citations now point at where
            the value is set; restating the claim belongs to the step that owns
            it. S091's `:205` is the opposite case and is held on purpose: the
            sentence records what its own excludes line read *before* S138
            re-anchored it, so the path is added and the number is the record.

            **The conversion broke the hard wrap and the repair includes
            fixing it.** A path is fifteen characters where a bare `:line` was
            five, so 266 body lines went past 80 columns; `rewrap` in the
            generator re-flows to 79, and only a bullet or paragraph that
            actually holds an over-long line -- headers, tables and fenced code
            are never candidates. A handful of units were already past the wrap
            before this step and are re-flowed with the rest rather than left
            at two widths. The mapping's line column is re-pointed afterwards,
            so a row still names where its citation sits.

            Checked: `MANUAL.md` -- no change, this touches no UCI surface.
            `DEV_MANUAL.md` -- rewritten, the `--citations` section describes
            `BARE`, the rule, and the two red/green observations above.
            `adocs/specs.md` -- no change; the plan documents' conventions live
            in `plan.md` and specs.md holds engine behaviour.
            Completion gate green in both builds, `./clang-format.sh --check`
            clean.

## The blind spot S138 could not close

S138 re-anchored and now gates **201 full `path:line` citations**. It could not
touch the other **467 of 668**, which are bare `:line` continuations whose path
is inherited from a sentence rather than written down -- `(:1274) and :1314`,
`:1887`, `:98-102`. A checker cannot resolve those without deciding which file
the prose meant, and deciding wrongly is worse than not checking: S138 reports
that guessing the inherited path produced **sixty impossible line numbers**, and
that S095 cites `transposition_table.cpp` and means `src/search.cpp` two words
later.

So they are stale in bulk, in the same way and for the same reason the 107 full
citations were, and nothing reports it. `--citations` counts them as
`loose, ungated` precisely so the gap is visible rather than silently uncovered.

## Why a style rule and not only a pass

Converting 467 references once leaves the next 467 to be written the same way.
The rule -- a citation repeats its path -- is what makes the checker's coverage
grow with the documents instead of decaying against them, and it is the same
reasoning S138 used for naming a `TEST_CASE` title beside a line: a `file:line`
is a moving reference and the cheapest defence is redundancy a checker can read.

## The evidence that this is not hypothetical

S138 found that **eight** pending steps, not the five the audit reported, told
their implementer to extend the mate-safety gate at the wrong line. The three it
added -- S095, S113, S116, all of them pruning or reduction steps and so all of
them the hazard CLAUDE.md says has already bitten twice -- were missed by the
audit **because they wrote it as a bare `(:1274)` continuation** and the audit
grepped for the full path. The blind spot has already hidden real defects from
one review.

## Cost

No match, no verdict. Document work plus the checker, and the fast suite green.
