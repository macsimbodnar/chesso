id:         S180
goal:       the seven pending search steps whose constants are seeded from another engine's constants quoted in commit or PR prose are reseeded under DEC-105 -- a value from a publication about the technique, a derivation over chesso's own data or scale, or a range midpoint -- so no engine-originated number is a starting point anywhere in the plan
accepts:    every seed in the "Constants and seeds" sections of S095, S097, S098, S109, S113, S114 and S132 is one of three things and the file says which: (a) a number from a publication about the technique with its URL beside it -- a paper, an article, the wiki's own derivation or example; (b) a derivation procedure the owning step runs at its start over chesso's own positions, data or scale, written out as the procedure and never as a number taken from elsewhere; (c) the parameter's range midpoint or off value, stated as such; no seed cites another engine's commit message, PR body, source, table or shipped value, and the sentence in S114 that calls prose-quoted numbers legal seeds is deleted; each rewritten section carries the line "seeds re-derived 2026-09-04 under DEC-105 (DEC-134)"; the F01 table in `adocs/audit/2026-09-04_plan_review.md` is walked row by row and each row's replacement is named in this step's done stamp; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/plan_todo/S095_internal_iterative_reduction.md, adocs/plan_todo/S097_singular_extensions.md, adocs/plan_todo/S098_reduction_refinement.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S113_probcut.md, adocs/plan_todo/S114_null_move_refinements.md, adocs/plan_todo/S132_time_management_node_fraction.md
excludes:   the seeds S091, S112 and S116 already take from the wiki or from chesso's own scale, which are the pattern and need no change; any engine code; running any derivation -- the owning step runs it at its start, this step writes the procedure down
decisions:  DEC-105, DEC-134
closes:     2026-09-04_plan_review-F01
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_plan_review-F01`. The enrichment pass of 2026-08-19 wrote the
"Constants and seeds" sections of the block-1 search steps under DEC-084's
venue rule: a commit message or PR body is a published write-up, so a constant
quoted in one was a legal seed. Three days later DEC-105 made the rule
provenance-based -- a number that originates as another engine's tuned output
is never a seed, wherever it is republished -- and named `plan.md`,
`CLAUDE.md`, `AGENTS.md` and `specs.md` as the documents carrying the new
form. The step files were not on that list and were never re-read. S114 still
says, in so many words, that "numbers quoted from message prose are legal
seeds, the S113 pass's precedent"; S113 says its numbers "are legal seeds
(DEC-084)".

Nothing in the engine carries these constants. The defect is that an
implementer opening any of the seven files is instructed to start a sweep or
an SPSA run from Stockfish's, Weiss's, Lynx's, Ethereal's or Berserk's number,
and DEC-105's own reasoning is why that matters: a seeded-then-refit vector
converges near its seed by construction, so "we refit it" is not a defence.
The owner's decision of 2026-09-04 (DEC-134) is to stick to the rule.

## The seven, and what replaces each

Taken from the F01 table of the report, verified against the files at
`18deccf`. "Prose" means the number was read from a commit message or PR body.

| step | engine seed | replacement |
|---|---|---|
| S095 `IIR_MIN_DEPTH` 4 | Lynx #507's introduction value | the wiki's own page says "depth > 5, say" -- a literature seed, cite `https://www.chessprogramming.org/Internal_Iterative_Reductions` |
| S114 eval-scaled null-move margin 100 | SF prose "one pawn" | one pawn *in chesso's own material scale* -- the pawn weight `evaluate()` ships, read at the owning step's HEAD; the unit is chesso's, not Stockfish's 100 |
| S114 eval-scaled cap 3 | SF prose "three plies" | range midpoint, swept |
| S113 ProbCut margin 150 | SF, Weiss, Ethereal, Berserk constants averaged | Buro's own method, which the file already names: regress chesso's shallow-vs-deep scores over its own positions and set the margin at t x sigma; the procedure is written, the owning step runs it |
| S113 ProbCut depth offset 4 | SF prose "depth - 4 plies" | the depth pairs the wiki's ProbCut page describes, or the range midpoint -- the file says which |
| S132 time-management base 2.0, scale 1.0 | Lynx #1203 prose | own census: choose base and scale so the expected time spent equals today's allocation at chesso's measured mean best-move node fraction over a census run, so the seed is behaviour-preserving in expectation |
| S109 late-move-pruning threshold `depth * 10`, cap 3 | Lynx #512's shipped constants | own census: the index of the cutoff move per depth over fixed-depth runs on chesso's positions; threshold at the stated percentile |
| S109 quiet-SEE reduced-depth gate 9 | SF prose | range midpoint, swept |
| S098 history clamp 2 | Weiss #451 prose | a stated fraction of chesso's own reduction table range, or the range midpoint |
| S097 singular min depth 8, table-depth margin 3 | SF prose; a talkchess write-up of SF-inspired code | the 1988 Anantharaman, Campbell and Hsu paper's parameters if S186-style research fetches them -- a publication about the technique -- else a profile of chesso's own table-entry reliability by depth, or the midpoint |

The three clean sections are the pattern: S112 seeds its margin from the
wiki's "typically around 200" and its victim table from chesso's own
`see_value`; S116 seeds from the wiki's "about three pawns" and declines the
Stockfish snippet the same page reproduces; S091 and S109 derive their SEE
and history coefficients from chesso's own scale.

## What this step does not do

It runs nothing. A derivation is written as a procedure with its inputs, its
tool and its acceptance, and the owning step runs it at its start -- that is
where the census or regression belongs, against the tree as it stands then.
Where a derivation is not cheap the fallback is the range midpoint, stated as
such, and the fit finds the value.

## Cost

Documents only. A few hours of careful rewriting over seven files, and one
pass of the checker.
