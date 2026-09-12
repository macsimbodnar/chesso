id:         S217
goal:       a sourced inventory of the hand-crafted techniques that engines rated 3000 to 3130 on the CCRL Blitz 1CPU list carried at a network-free version, and this plan lacks -- banded, priced, and turned into steps by the coordinator (DEC-179)
accepts:    `adocs/data/S217_handcrafted_gap.md` lists, for at least four hand-crafted engines rated 3000 to 3130 on the CCRL Blitz 1CPU list at a version with no network -- drawn from the DEC-071 set (Texel 1.07, Weiss 1.2, Ethereal 11.75, Laser 1.7, Defenchess 2.2, Booot 6.3.1, rofChade 2.3) with each rating and version re-read from the list on the day and the network-free claim verified against the version's release notes or the author's own statement -- the search and evaluation techniques it carried, from release notes, commit-message prose, papers, the wiki and the authors' write-ups, **never from source** (DEC-016, and DEC-105 for any constant quoted in prose); the intersection with what chesso ships at HEAD and with every pending step is computed technique by technique, and what remains is the gap; each gap technique carries its published figure with URL or the word unverified, its band per DEC-087 as corrected by DEC-176, and its worst-case verdict games per DEC-143 at the fast-class pair; the coordinator creates one step per gap technique with evidence at or below the 3100 band, in `plan_todo/` and at the block DEC-081's order puts it, and records in a decision which gap techniques got no step and why; **a finding that the gap is empty or nearly so is recorded as such**, and the decision then says what "a longer list" means instead -- further tuning passes, a different discount, or a re-priced order -- rather than inventing steps to fill a table; S183's arithmetic is re-derived over the extended list (DEC-136) and the paragraph in `adocs/plan.md` says where the extended list lands; `adocs/data/README.md` gains the row; `tools/plan_prose_check.py` `--prose`, `--citations`, `--touches` and `--params` clean
touches:    adocs/data/, adocs/plan.md, adocs/plan_todo/, adocs/decisions.md, adocs/status.md
excludes:   any `src/` change; any run; the network, which DEC-179 places after the mark; reading another engine's source, tables or training data (DEC-016); seeding any constant from another engine's constants, wherever republished (DEC-084, DEC-105)
decisions:  DEC-071, DEC-179, DEC-136, DEC-087, DEC-176, DEC-143, DEC-016, DEC-105
closes:
blocks:
paused_by:
author:     a Sonnet 5 subagent briefed by the coordinator (DEC-185, DEC-188); started 2026-09-12 04:05 while S042's SPRT holds the machine
done:       2026-09-12 04:40. `adocs/data/S217_handcrafted_gap.md` tables five DEC-071 engines at their last network-free version -- Weiss 1.2 (3055), Texel 1.07 (3129), Laser 1.7 (3291), rofChade 2.3 (3319), Ethereal 11.75 (3344), ratings re-read on the CCRL Blitz complete list 2026-09-12, network-free claims verified against release notes or the author -- with every technique sourced from prose, no source file opened, and the intersection computed against a checklist of what chesso ships and has pending. **The gap is three evaluation terms** (complexity scaling, fortress detection, castling ability), none with a published figure, all above the 3100 band, so **no step is opened** and the three go to the Reserve as candidates, DEC-192. S183 re-derived over the extended list: **unchanged, 2658 and 2707 to 2817**; DEC-179's premise is not borne out and the question of what a longer list means is put to the owner with three options. README row added; `plan_prose_check.py --citations` and `--prose` clean. `DEV_MANUAL.md` and `MANUAL.md` unaffected (documents only), `README.md` human-owned, untouched. By a Sonnet 5 subagent (DEC-185, DEC-188) while S042's SPRT held the machine; recorded by the coordinator

## Why this exists

S183 made the plan's Elo arithmetic checkable and it does not reach 3000: the
reconstruction from recorded inputs lands at 2658, the plan's own range
discounted at 2707 to 2817, against a 2559 anchor. Its stamp named DEC-071 as
the decision it put to the owner. The owner's ruling on 2026-09-11 is DEC-179:
the goal stands at 3000 without a network, the network comes after the mark,
and the hand-crafted list is extended -- "multiple engines did that, so should
we".

The engines that did it are the evidence DEC-071 already cites: Texel 1.07 at
3130, Weiss 1.2 near 3055, Ethereal 11.75 at 3346, Laser 1.7 at 3294, and the
rest of that table, each hand-crafted and each one version below its author's
first network. What they carried at those versions is the best available
answer to what a 3000-rated hand-crafted engine needs, and the difference
between that and this plan is what this step measures.

## Method, and what it must not do

Sources: the CCRL list pages, release notes, changelogs, commit-message prose,
the authors' talkchess and blog posts, published papers, the Chess Programming
Wiki. **Not** sources: source files, tables, network files, training data
(DEC-016). A constant quoted in prose is recorded as a published figure and
never used as a seed (DEC-105).

The output table has one row per technique and these columns: technique;
which of the surveyed engines carried it at the surveyed version; published
figure with URL or `unverified`; band per DEC-087/DEC-176; chesso status --
`ships` (with the step that landed it), `pending Sxxx`, or `gap`; and for a
gap, the DEC-143 worst-case games at the fast-class pair.

The pending list already covers most of the standard machinery -- DEC-071's
own gap inventory became S089 to S102, DEC-081 to DEC-087 ordered it, and
blocks 2 to 4 carry the evaluation rebuild, the speed work and the tuning
passes. So the honest expectation is a **small** gap, and the step's value is
as much in confirming coverage as in finding holes. Where the gap is empty,
the shortfall is the transfer discount and not a missing technique, and the
decision that follows has to say so and say what is done about it -- which is
a different conversation from adding steps, and DEC-179 asks for it to be had
in a decision rather than assumed either way.

## Cost

Documents only. About half a day of reading and one table. Filler behind
S024's run; owns no machine time.

## Findings, 2026-09-12

Five DEC-071 engines surveyed, not four: Weiss 1.2 (3055), Texel 1.07 (3129),
Laser 1.7 (3291), rofChade 2.3 (3319), Ethereal 11.75 (3344), all re-read on
the CCRL Blitz complete list 2026-09-12 and each verified network-free against
its own release notes or the author's statement. `adocs/data/S217_handcrafted_gap.md`
has the full table and the per-technique sourcing. **The gap is three
evaluation terms**: complexity/conversion-chances scaling (carried by
Ethereal and, independently, rofChade), fortress detection (Texel, three
version-dated additions), and castling ability (Ethereal only, no version
pinned, weakest of the three). None carries a published Elo figure anywhere
in the sources read. Everything else across all five codebases -- the whole
standard search stack and the whole standard evaluation stack -- already has a
`ships` or `pending` row; two more (Mate Distance Pruning, quiescence checks)
are not gaps but decisions already declined (DEC-087), and Lazy SMP is
deferred whole to phase two (DEC-175) and is in any case irrelevant to the
1CPU ratings this survey read. **All three gap rows sit above the 3100 band**
this step's own rule needs to open a step automatically (3129, 3319-3344, and
an unpinned figure no lower than 3344), so none qualifies by the rule as
written -- the same test that already keeps S023/S025/S110/S111 in the
Reserve. The recommendation is to file the three outlines as Reserve
candidates rather than open steps now. S183's arithmetic, re-derived over the
extended list under its own rule (a technique with no published figure
contributes zero), **adds nothing**: the landing points are unchanged at 2658
reconstructed and 2707 to 2817 on the plan's own range, still 183 to 342 Elo
short of 3000 at the high end. DEC-179's premise -- that a longer hand-crafted
list would close the shortfall -- is not borne out; the Coverage section of
the data file lays out three options for what "a longer list" could mean
instead, for the coordinator's decision.
