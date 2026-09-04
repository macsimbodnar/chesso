id:         S185
goal:       every published figure the plan and the pending step files argue from carries its source or is marked unverified where it sits -- the Ethereal and Stash ledgers behind the block order recorded by commit and file, the figures the literature check could not find or found mis-attributed corrected, and the remaining gaps listed as S186's work
accepts:    DEC-087's Amended line and `adocs/plan.md`'s block-3 paragraph cite the Stash `CHANGELOG.md` entries the ordering rests on -- v26 initiative from threatened pieces +10.13, v27 mobility zone +19.95, v31 connected pawns +25.38, v32 king proximity in the passed-pawn term +22.27 -- and the Ethereal ledger commit e755a814 -- history -759.05, late move reductions -248.59, quiet move pruning -175.08, beta pruning -31.95 -- by URL, both taken from `adocs/data/2026-09-04_plan_review_literature_check.md`; the clause "single digits for most evaluation terms" is deleted from `adocs/plan.md`, since that ledger prices search steps only; the zero-weight paragraph of `adocs/plan.md` re-maps its four numbers to what the sources say -- +12.99 is Weiss pull request #241, tempo; +16.7, +4.2 and +9.2 are marked unverified with the nearest fetched figures named (Weiss #95 bishop pair +16.10, Weiss #231 rook and queen files +9.86, Lynx #390 bishop pair +8.2 / +9.5); S129 states that its 13 and 25 are six-men figures and that the only three-man figure found is Minic's 0; S109's citation of Stockfish pull request #4294 for the "~0 alone" figure is re-pointed to pull request #2401, where the number originates; the Leorik +436 sentence in `adocs/plan.md` records that the release also credits an nps increase; every remaining figure in the thirteen files without a URL (S023, S110, S111, S118, S119, S121, S122, S123, S124, S125, S126, S129, S133) is given its URL where the literature check holds one and marked unverified in the sentence that carries it otherwise, and the unverified list is written into S186's file as its work list; `adocs/data/README.md` carries the literature check's row; `tools/plan_prose_check.py --touches` and `--params` green
touches:    adocs/plan.md, adocs/decisions.md, adocs/data/README.md, adocs/plan_todo/
excludes:   fetching sources the literature check does not already hold -- that is S186's enrichment pass; changing the block order on any figure, which would be a decision; the Lynx bands, which are S181
decisions:  DEC-087, DEC-097, DEC-133, DEC-137
closes:     2026-09-04_plan_review-F06
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_plan_review-F06`, plus the figures the review's independent
literature pass found wrong or unsourced and the reviewer did not raise.

**The block order rests on two ledgers no tracked document sources.**
`adocs/plan.md` orders block 3 "in the order the Stash ledger prices them"
and calls Ethereal's feature-removal figures DEC-081's "third independent
line"; DEC-087 names "the Stash ledger" with no path, commit or URL, and two
enrichment passes recorded the Ethereal figures as "untraced publicly". The
literature pass found both: the Ethereal numbers are, to the decimal, the
table in Ethereal commit `e755a814` ("Add elo estimates to search steps",
2020-01-22, Ethereal 11.82); the Stash numbers are entries in
`mhouppin/stash-bot`'s `CHANGELOG.md`. So the order rests on what it says it
rests on, and nothing in the tree says so. This step writes it down.

**What the same pass found wrong.** `adocs/plan.md`'s sentence "single digits
for most evaluation terms" has no source: the Ethereal ledger prices search
steps only, and its single-digit rows are ProbCut, counter-move pruning and
futility. The zero-weight paragraph's "+12.99" is Weiss's tempo pull request,
not rook on the seventh, and its +16.7, +4.2 and +9.2 were found in no source
searched (Weiss, Berserk and Lynx pull requests, the Stash changelog). S129's
"13 to 25 Elo" are both six-men figures -- Stockfish 10dev classical at +13,
Topple at about 25 -- where S129 is three to five men and the only three-man
figure found is Minic's 0. S109 cites Stockfish pull request #4294 for
"move-count pruning ~0 alone" where the number is in pull request #2401's
comment updates. The Leorik +436 release also credits "a significant increase
of nodes searched per second", which the plan's "four search features and no
evaluation change" omits.

**Thirteen pending files carry about forty Elo figures and no URL.** They are
block 3 and the reserve. This step gives each figure the URL the literature
check holds or marks it unverified in place, and hands the unverified list to
S186, which is the enrichment pass DEC-137 orders before block 3 starts.

## What this step does not do

It fetches nothing new. The literature check is the source table; what it does
not hold stays marked unverified until S186 resolves it or records what was
searched. It reorders nothing: a figure that turns out smaller or larger than
the plan assumed is a reason for a decision, and the decision is the owner's.

## Cost

Documents only. Two to three hours over sixteen files.
