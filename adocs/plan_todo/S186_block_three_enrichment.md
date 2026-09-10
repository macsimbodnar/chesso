id:         S186
goal:       the DEC-097 enrichment pass runs over block 3's step files before block 3 starts -- every evaluation figure traced to its source or marked unverified, every technique's form checked against the wiki's definition, every seed in a DEC-105 form -- so the evaluation block is executed against figures somebody can check
accepts:    each of S134, S082, S083, S135, S136, S039, S121, S123, S125, S118, S101, S122, S124, S102, S133 and S126 carries a "Technical details (SOTA research ...)" section in the form the block-1 files have; every figure in it has a URL or the word unverified beside it; every seed is a literature value with its URL, a derivation the step runs at its start, or a range midpoint (DEC-105, DEC-134); the wiki's definition of each technique is cited and any place the step's form departs from it is stated; the engine records are read as commit messages, pull-request bodies, changelogs and release notes only, never as source or tables (DEC-016), and every file read is listed; S185's unverified list is the work list and each entry is resolved or stays marked with what was searched; block 3's ordering paragraph in `adocs/plan.md` is re-read against the traced figures and any reordering is proposed as a decision, not made as an edit; the pass lands as one commit per file or per few files so a review can follow it; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/plan_todo/, adocs/plan.md, adocs/eval_tuning_strategy.md
excludes:   the block-1 and block-2 files, enriched 2026-08-19 to 2026-08-20, whose corrections are S180 and S181; any engine code; running any derivation, which the owning step does at its start
decisions:  DEC-097, DEC-105, DEC-134, DEC-137
closes:     2026-09-10_adversarial-F37
blocks:
paused_by:
author:
done:

## Why this exists

DEC-097 (2026-08-21) resumed the SOTA enrichment of the pending step files
"in parallel, on nights". By 2026-09-04 `git log --since=2026-08-21 --
adocs/plan_todo/` showed 40 commits and none of them an enrichment, and 35 of
54 pending files carried no "Technical details" section -- all of block 3 and
the reserve among them. Block 3 is ordered by figures the review had to go and
find (S185 records them), and S133 was added on two numbers that turned out to
be release bundles. A standing promise for nights produced nothing in
fourteen; DEC-137 makes it a step with a place in the order: before S134, the
first block-3 entry, so that no evaluation step starts on a figure nobody can
check.

## How the pass is done

The block-1 enrichment of 2026-08-19 is the template for the section's shape.
Two rules changed since then and both bind here. DEC-105 and DEC-134: a
constant from another engine, wherever it is quoted, is not a seed -- a
literature value with its URL, a derivation over chesso's own data written as
a procedure, or the range midpoint. DEC-135: a citation into code names a
symbol, not a line. The evidence standard is the review's own: a figure
without a URL is written as unverified, and unverified figures carry no weight
in an ordering argument.

The sources are the ones the literature check used and lists:
`mhouppin/stash-bot`'s `CHANGELOG.md`, the Ethereal release notes and commit
messages, Berserk's and Weiss's pull requests, Leorik's release notes, the
wiki's Evaluation pages -- read for form and figure, never for source or
tables (DEC-016).

## Cost

Documents only, sixteen files, several hours spread over the machine-scope
lane's idle moments. It owes no match and no run.

## Amended 2026-09-11, DEC-170: the strategy document's two unrecorded divergences (F37)

`2026-09-10_adversarial-F37`: `adocs/eval_tuning_strategy.md` specifies
mini-batch Adam at batch 16k to 64k where `tools/tuner.cpp` is full-batch, and
its Phase A requires an in-engine evaluation trace and UCI exposure of every
tunable, neither of which exists -- the trace lives in `tools/eval_model.hpp`
as a second implementation. Its other divergences (leaf labels, non-linear
king safety) are recorded and planned. This pass, which reads that document
for every block-3 file, records the two as decided departures or corrects the
document, and says which, so block 3 is executed against a strategy that
describes the tools it has.

## The work list S185 handed over, 2026-09-11

S185 gave every figure in the thirteen pending files either its URL or the word
unverified where it sits. **These are the unverified ones**, file by file, each
with what was already searched for it in the 2026-09-04 literature check
(`adocs/data/2026-09-04_plan_review_literature_check.md`). Resolve or re-state
each one with what *this* pass searched; a figure that survives two searches
unresolved is worth deleting, and deleting it is a decision.

| file | figure | what it is quoted for | searched already |
|---|---|---|---|
| S023 | Lynx four failed SPRTs at ~2600, -35.8 to -11.1 | capture history is negative as an ordering term below 3000 | no source located; the band itself is S181's |
| S023 | Ethereal +7.2 / +2.4 | reducing tacticals with bad capture history -- the *input* use | no source located |
| S110 | +3 to +8, "only above ~3100" | the whole non-pawn and continuation demotion | row A22: one point inside the band (Sirius #176, +7.43 +/- 4.78), nothing at all for "above ~3100" |
| S110 | twelve figures: pawn +11.29 / +12.40 and +4.87 / +11.70, non-pawn +6.98 / +12.28 and +2.80 / +6.84, continuation +2.58 / +5.46 and +2.75 / +5.46 | that the three tables are not inert apart, and long control is about twice short | row A22: no cited source for any of the twelve |
| S111 | +1.8 to +4.6 per table, "only above ~3100" | the demotion | row A22; SF #5617 is where the feature landed, with no figure |
| S118 | a 10 % slowdown from caching a cheap pawn evaluation | why S118 sits after S123 and S125 | CPW *Pawn Hash Table* states no speed or Elo figure |
| S118 | +10.11 Elo and a 10 to 12 % speed-up | what the cache is worth | same page, same answer |
| S119 | 5 to 15 Elo for bucket-plus-ageing over always-replace | the expectation that the scheme is worth little | CPW *Transposition Table*: buckets and ageing described, no figure |
| S121 | +10.5 for excluding enemy pawn attacks from knight mobility | the cheap half of the mobility area | CPW *Mobility* defines safe mobility, states no figure |
| S122 | a from-scratch attack-unit table regressing 8 to 10 Elo, its attack bonus fitted to +7 | the corpus warning the accepts acts on | write-up not located; Stash v31's +4.24 then +9.69 is what traces for the form |
| S123 | +36.1 for the whole passed-pawn feature at about 2600 | the group's size | no source located |
| S124 | +11.3, +13.72, +7.56, +9.41 | endgame scaling's size and the whole-versus-endgame-half change | no source located; the coverage gap is what the order rests on |
| S124 | +0.29, +0.94, +0.18, +0.11 at 3300 | the shape of the tail | no source located |
| S125 | +10.68 conditioning, +6.60 and +3.77 indexing, +4.01 double-count | that conditioning pays more than adding -- the step's whole shape | no source located; Stash v31's +25.38 is what traces for the group |
| S126 | +5 to +15 for a full retune, and 2529 -> 2910 for a fitted evaluation as a release's only change | that a refit is a large cheap number | neither located; S028's +188.74 is the local evidence |
| S129 | "two thousand lines of foreign code that I don't understand for a mere 5 Elo" | the cost-to-gain ratio | quotation not located; the 13 and 25 *were* traced and are six-men figures (row A29) |

Two further items for this pass, both from the same check and neither a
figure. **S121's "mobility area" and per-count curve are not on CPW's
*Mobility* page** -- the term is Stash's, from its `evaluate.c` and changelog
-- so the form check owes a statement of where the definition comes from.
**S117's packed score is not on CPW as a technique** either (Tapered Eval,
Score and SIMD_and_SWAR_Techniques all fail to describe it; Berserk pull
request #65 and Ethereal's 9.45 release line are the write-ups), and S117 is a
block-2 file, so that one belongs to S180 or S181 rather than here -- named so
it is not lost.
