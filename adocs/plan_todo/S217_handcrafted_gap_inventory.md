id:         S217
goal:       a sourced inventory of the hand-crafted techniques that engines rated 3000 to 3130 on the CCRL Blitz 1CPU list carried at a network-free version, and this plan lacks -- banded, priced, and turned into steps by the coordinator (DEC-179)
accepts:    `adocs/data/S217_handcrafted_gap.md` lists, for at least four hand-crafted engines rated 3000 to 3130 on the CCRL Blitz 1CPU list at a version with no network -- drawn from the DEC-071 set (Texel 1.07, Weiss 1.2, Ethereal 11.75, Laser 1.7, Defenchess 2.2, Booot 6.3.1, rofChade 2.3) with each rating and version re-read from the list on the day and the network-free claim verified against the version's release notes or the author's own statement -- the search and evaluation techniques it carried, from release notes, commit-message prose, papers, the wiki and the authors' write-ups, **never from source** (DEC-016, and DEC-105 for any constant quoted in prose); the intersection with what chesso ships at HEAD and with every pending step is computed technique by technique, and what remains is the gap; each gap technique carries its published figure with URL or the word unverified, its band per DEC-087 as corrected by DEC-176, and its worst-case verdict games per DEC-143 at the fast-class pair; the coordinator creates one step per gap technique with evidence at or below the 3100 band, in `plan_todo/` and at the block DEC-081's order puts it, and records in a decision which gap techniques got no step and why; **a finding that the gap is empty or nearly so is recorded as such**, and the decision then says what "a longer list" means instead -- further tuning passes, a different discount, or a re-priced order -- rather than inventing steps to fill a table; S183's arithmetic is re-derived over the extended list (DEC-136) and the paragraph in `adocs/plan.md` says where the extended list lands; `adocs/data/README.md` gains the row; `tools/plan_prose_check.py` `--prose`, `--citations`, `--touches` and `--params` clean
touches:    adocs/data/, adocs/plan.md, adocs/plan_todo/, adocs/decisions.md, adocs/status.md
excludes:   any `src/` change; any run; the network, which DEC-179 places after the mark; reading another engine's source, tables or training data (DEC-016); seeding any constant from another engine's constants, wherever republished (DEC-084, DEC-105)
decisions:  DEC-071, DEC-179, DEC-136, DEC-087, DEC-176, DEC-143, DEC-016, DEC-105
closes:
blocks:
paused_by:
author:
done:

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
