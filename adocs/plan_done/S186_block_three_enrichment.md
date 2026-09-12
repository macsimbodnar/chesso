id:         S186
goal:       the DEC-097 enrichment pass runs over block 3's step files before block 3 starts -- every evaluation figure traced to its source or marked unverified, every technique's form checked against the wiki's definition, every seed in a DEC-105 form -- so the evaluation block is executed against figures somebody can check
accepts:    each of S134, S082, S083, S135, S136, S039, S121, S123, S125, S118, S101, S122, S124, S102, S133 and S126 carries a "Technical details (SOTA research ...)" section in the form the block-1 files have; every figure in it has a URL or the word unverified beside it; every seed is a literature value with its URL, a derivation the step runs at its start, or a range midpoint (DEC-105, DEC-134); the wiki's definition of each technique is cited and any place the step's form departs from it is stated; the engine records are read as commit messages, pull-request bodies, changelogs and release notes only, never as source or tables (DEC-016), and every file read is listed; S185's unverified list is the work list and each entry is resolved or stays marked with what was searched; block 3's ordering paragraph in `adocs/plan.md` is re-read against the traced figures and any reordering is proposed as a decision, not made as an edit; the pass lands as one commit per file or per few files so a review can follow it; `tools/plan_prose_check.py --touches` and `--params` green; no engine file changes
touches:    adocs/plan_todo/, adocs/plan.md, adocs/eval_tuning_strategy.md
excludes:   the block-1 and block-2 files, enriched 2026-08-19 to 2026-08-20, whose corrections are S180 and S181; any engine code; running any derivation, which the owning step does at its start
decisions:  DEC-097, DEC-105, DEC-134, DEC-137
closes:     2026-09-10_adversarial-F37
blocks:
paused_by:
author:     an Opus 5 subagent briefed by the coordinator (DEC-185, DEC-199); started 2026-09-12 23:40 beside S151's match under the blocked-task exception, documents only
done:       2026-09-13 00:08 (the stamp first read 04:05, a clock error corrected by the coordinator before the move). All sixteen block-3 files carry a "## Technical
            details (SOTA research, 2026-09-13)" section in the block-1 template's
            shape -- S134, S082, S083, S135, S136, S039, S121, S123, S125, S118,
            S101, S122, S124, S102, S133, S126 -- each with a State of the art, a
            Shape for chesso stating every departure from the cited definition, an
            Implementation sketch, Constants and seeds in DEC-105 form, Pitfalls,
            Measurement, Interactions and a References list naming every file and
            URL read. Every engine record read was a commit message, a
            pull-request body, a changelog entry, a release note or a forum post;
            no source file and no table was opened (DEC-016).
            **Figures: 10 of S185's work-list entries resolved with a URL**
            -- S118's 10 % slowdown (TalkChess t=72195, xr_a_y 2019-10-28, whose
            published diagnosis is a 17 MB table against a 16 MB L3 and **not**
            the cheap-pawn-evaluation reading DEC-087 (h) gave it); S124's four
            tail figures (+0.94, +0.29, +0.18, +0.11 -- three Ethereal
            scale-factor removals, not four cases); S125's four (+10.68, +6.60,
            +3.77, +4.01 -- four Ethereal commits, +3.77 bundled with a PSQT
            retune); S126's "+5 to +15" (CPW *Texel's Tuning Method*'s
            seven-retune ledger, 2.8 to 39.4, median 10.2, 32000 games each).
            **19 restated unverified with what this pass searched**, across
            S082, S083, S102, S118, S121, S122, S123, S124 and S126.
            **14 proposed for deletion, 0 deleted** -- none carried an ordering
            argument nothing else supports, so every one is a proposal for the
            coordinator to record. **17 new sourced figures added** that the
            files did not have: Berserk #73's pawn hash +14.97, Stash v26's
            endgame scaling 3.94, three Ethereal endgame-scaling additions, five
            Ethereal/Stash passed-pawn figures, three Ethereal king-safety
            figures and four Lynx king-bucket figures (+2.86, +1.69, -0.98,
            -2.79 -- two of the four negative, which is new evidence against
            more buckets in S133). **Three new DEC-105 form (a) literature
            values** located and recorded as legal seeds: Kaufman's half a pawn
            for the bishop pair, CPW's 8-to-20 cp for a rook on an open file,
            the Toga log manual's 10-to-16 cp for a knight outpost. **CPW's
            King Safety attack-unit weights (2 / 3 / 5 / 6) and the Glaurung
            table are named as Stockfish's and Glaurung's tuned output
            republished, and refused as seeds** (DEC-105, DEC-134).
            **F37 disposed**, both divergences, in `adocs/eval_tuning_strategy.md`
            with dated notes at four sites: the mini-batch Adam paragraph
            (section 2.5) is a **decided departure** -- `tools/tuner.cpp` is
            full-batch, its `--epochs` help text says so, the document's own
            section 2.4 gives the reason mini-batching is unnecessary here, and
            no batch-size option is planned; Phase A's in-engine trace and UCI
            exposure are a **correction to the document** -- neither exists, the
            flat array and the feature extractor live in `tools/eval_model.hpp`
            as a second implementation held against the engine by
            `tests/test_eval_model.cpp`, and the only UCI parameter surface is
            `search_param_info()` over the *search* parameters under
            `CHESSO_TUNE`. Section 2.4's "Build this" and section 10's Phase A
            bullet carry pointers to the correction.
            **`adocs/plan.md` not written** (out of scope): one sentence change
            is proposed to the coordinator, the parenthetical "(a cheap pawn eval
            cached measured a published slowdown)" in the S118 sentence, whose
            traced source diagnoses cache size instead. Block 3's ordering
            paragraph was re-read against the traced figures and **no reordering
            is proposed**: it rests entirely on the Stash ledger entries, all of
            which carry URLs. One stale document citation re-anchored in S125.
            **Checks**: `--citations` 0 flagged over 57 files, `--touches` 0
            flagged over 57 files, `--params` 0, `--prose` 0 flagged -- all four
            exit 0. No engine file touched, no build run, no test binary run: a
            timed 1000-pair match held the machine throughout.
            **Amended 2026-09-13 00:19, DEC-203: the deletions are applied.**
            The fourteen figures the pass proposed are gone from the nine
            pending files -- S082 one (rows a game), S083 three (a Stash
            corpus size, an overfitting threshold quoted from its author, an
            Ethereal per-generation dump size), S121 one (the knight
            pawn-exclusion figure), S123 one (the whole passed-pawn feature at
            about 2600), S118 two (the Elo figure and the speed-up
            percentage), S122 two (the from-scratch regression and the fitted
            attack bonus), S124 four plus the "3300" rating band, S102 one
            (the claim that the literature's figures are small) and S126 one
            (the rating jump for a release whose only change was a fitted
            evaluation). Each numeral is gone from **every** site in its file,
            the older body text and the References scan records included, so
            none survives as a quotable number; the deletion notes name what
            was removed and what was searched. **The density is restated in
            S082 and S083** (DEC-203 clause 2): few rows a game is chesso's
            own choice, made because positions inside one game are correlated,
            written as a hypothesis, with the "12 to 25 M games" arithmetic
            gone and the one sourced density (Texel's about 140 a game, CPW)
            named as the opposite end; S083 gains a Shape clause measuring
            rows per game and games needed against held-out error, and its
            `accepts` is untouched. **S118's slowdown is cited for what it
            was** (clause 3): an oversized table, about 17 MB against a 16 MB
            L3, at all three sites, and no sentence still reads it as a cheap
            pawn evaluation cached. **Three deletions carried an argument and
            each is restated on what remains**, none on a new figure: S082's
            and S083's density (restated per clause 2), S083's corpus band
            (now the sourced 8.8 M) and S124's "largest group" clause (now the
            coverage gap, which section 1 had already established). The S185
            work-list table above carries a dated note that its figures are
            the record and not evidence. **Checks re-run**: `--citations` 0
            flagged over 57 files, `--touches` 0 flagged over 57 files,
            `--params` 0, `--prose` 0 flagged -- all four exit 0. Documents
            only; no build, no test binary, no match touched.
            **Amended 2026-09-13 00:46, after the fast check: its seven
            findings repaired, documents only, a timed match holding the
            machine throughout.**
            **(1) Two Stash figures restored to S083 with their URL.** The
            changelog's **v32.0 (2021-12-02)** entry does state dataset
            sizes -- "using a dataset of 4.5M positions coming from selfplay
            games at 1+0.01 time control" and "Removed the validation loss
            from the tuning code, since the eval doesn't overfit for datasets
            > 500k positions" -- so the pass's sentence that it "prices
            patches and states none" was false and is corrected in place and
            named as this file's error. **The band's other end is sourced
            too**: TalkChess t=75350, AndrewGrant, 2020-10-10, "3x Sets of
            ~10M positions of <fen> <result>", so S083's "4.5 to 10 M" wording
            is restored whole, the figure read and the data never used
            (DEC-016). Only the "per generation" reading of the Ethereal
            number stays deleted -- the post states three sets of a size, not
            a rate. S126 gains one pointer to the > 500k statement beside its
            own "no published figure" sentence.
            **(2) `adocs/plan.md` not edited** (coordinator's file): the exact
            replacement sentence, attributing 8.8 M to Peter Osterlund through
            CPW and naming Stash's 4.5 M and Ethereal's ~10 M separately, is
            in the report.
            **(3) Two seeds re-formed off engine constants.** S102's outpost
            bonus stood as DEC-105 form (a) on "the Toga log manual's 10
            centipawns", which is an engine's documented constant however CPW
            republishes it; it is now **(c)**, a range this file declares by
            purpose -- a placement bonus must never be worth trading material
            for, so 0 to half a pawn, 47 on chesso's scale, midpoint 24.
            S135's rook-on-open-file band stood as form (a) on CPW's "8 to 20
            centipawns" and on the claim that it is "not any engine's shipped
            constant"; the page's footnote 1 reads "20 cp comes from Toga log
            user manual", so the range is refused entire -- half a refused
            range is still the range -- and replaced by **(c)** with chesso's
            own bounds and midpoint, semi-open half of open. Both Toga figures
            are now cited only as figures read for form and refused as seeds,
            the treatment S122 gives CPW's attack units and the Glaurung
            table. **The other fourteen files were re-scanned for the same
            defect**: three form-(a) claims existed in the sixteen and the
            third stands -- S118's table size is CPW *Pawn Hash Table*'s own
            "a few K" with no engine behind it (page re-read 2026-09-13) --
            as does Kaufman's half a pawn in S135, which is a 1999 article's.
            Every other seed in the sixteen is (b), (c) or absent.
            **(4) S125's body now agrees with its own section 1**: the four
            figures are sourced, not unverified, the "S186 owns it" pointer is
            gone, and section 1's two readings -- +3.77 is a bundle, all four
            shrink at 60.0+0.6s -- are stated where the argument is made.
            **(5) S125's three stale citations re-anchored** (DEC-135): the
            deleted S118 phrase becomes "a cache of them saves little until
            S123 and S125 have made the pawn evaluation worth caching" and "is
            what makes S125's richer pawn terms affordable", the `:37` line
            citation is gone, and the vanished `plan.md` parenthetical is
            described instead of quoted. Two further sentences in S125 that
            still read the slowdown as a cheap pawn evaluation cached are
            corrected (DEC-203 clause 3). `--citations` now prints **no note
            MISSING anywhere**, where it printed one for S125 before.
            **(6) The queen's maximum is 27 and it came from a tool.** S121's
            "a queen with 28 reachable squares is legal" was wrong;
            `python-chess`, `max(len(board.attacks(square)))` over a queen on
            each square of an empty board, gives 27 (d4, d5, e4, e5) and 21 in
            a corner. Section 3 is corrected, section 4 records the derivation
            of all four widths from the same run -- knight 8, bishop 13, rook
            14, queen 27 -- and the `accepts`, which already said "queen 0 to
            27", is untouched and now agrees with both.
            **(7) The six S185 rows outside the sixteen are disposed and every
            S186 pointer is gone.** **Two resolved with URLs**: S023's
            Ethereal input figures are commit dcb8560c, 2020-09-26, "Use
            Capture History to apply LMR to some Tactical Moves",
            **+7.22 +/- 4.72** at 10.0+0.1s over 7696 games and
            **+2.37 +/- 1.90** at 60.0+0.6s over 31984 games -- and the
            direction is the opposite of the ordering use's, which the file
            now says; S129's quotation is **Morgan Houppin**, CCC,
            2021-03-25, talkchess t=76927, quoted at CPW *Syzygy Bases*
            footnote 32, and the file's version of it was a splice, now
            marked as one. **Four restated unverified with what this pass
            searched**: S110's band and its twelve figures, S111's per-table
            range and its rating floor, S119's 5-to-15 replacement range. Two
            of those gain a **new sourced neighbour** instead: Tcheran's
            changelog prices four correction-history tables separately in one
            release (pawn 21.76 +/- 8.62, major-and-minor 18.86 +/- 7.99,
            non-pawn 12.69 +/- 6.30, threat 9.30 +/- 5.31) and a 1-ply
            continuation correction history at 8.23 +/- 5.04, which sources
            the *shape* S110 argues from while the twelve numerals stay
            unverified; the changelog states no time control for those lines
            and the file says so. S023's withdrawn Lynx row needed nothing --
            S181 and DEC-176 had already disposed of it.
            **S135's +16.7 / +4.2 / +9.2 are deleted**, not kept as
            unverified: a third search (GitHub commit search over
            `TerjeKir/weiss` for "open file" and `jhonnold/berserk` for
            "bishop pair", plus an open web search) found nothing, and
            DEC-203's rule applied consistently deletes them. Every one of the
            three numerals is gone from every site in S135, References
            included; Weiss #95's sourced +16.10 / +12.24 stays, and
            `adocs/plan.md`'s dated audit paragraph still holds the three as a
            record of what was searched, which is the coordinator's to note.
            **So DEC-203's count changes**: 12 items and 17 numerals over nine
            files, not 14 and 17 -- S083's three come back, S135's three go in
            as one item.
            **Checks after every file**: `--citations` 0 flagged over 57
            files and **0 `note MISSING`**, `--touches` 0 flagged over 57
            files, `--params` 0, `--prose` 0 flagged -- all four exit 0. No
            build, no test binary, no `cmake`, no `ctest`, no `fastchess`; one
            `python-chess` snippet for the board fact. Files touched: S023,
            S083, S102, S110, S111, S119, S121, S125, S126, S129, S135 and
            this stamp.

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

**2026-09-13, DEC-203: the figures quoted in the table below are the work
list, not evidence.** The rows for S118, S121, S123, S124 and S126 name
numbers this pass could not source, and DEC-203 deleted every one of them from
the pending files. They survive here as the record of what was searched and
removed. **Nothing in this table is to be quoted forward into a step file.**

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
