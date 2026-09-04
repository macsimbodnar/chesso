id:         S184
goal:       the pending documents state the engine as it ships at HEAD -- retuned parameters at their shipped values, S042's touches naming every site that builds the en-passant key, prose that calls no done step pending and routes to no folded id, step headers complete -- and the params checker reaches the pending step files so the class stops recurring
accepts:    S115's "What is there" section and its constants table are rewritten against the aspiration triple `src/search_params.hpp` compiles at this step's HEAD (S085's vector), the sweep's off row is taken at those values and the "keep 5" instruction is gone; S082's quiescence-cap sentence, S120's and S122's clamp figures, `adocs/plan.md`'s "150 centipawns" sentence in "Three things the first review measured" and S118's "What it costs today" paragraph state the shipped values or carry the date they were measured; S042's `touches:` names `load_FEN`'s en-passant sanitiser and every other place the key is built beside `make_move`, and its body states the one rule applied at all of them; the five prose defects of F09 are corrected -- the machine-scope lane paragraph to the list as it stands, `status.md`'s Next line, S159's S093 clause, S171's section title, the three S128 routes to S152 -- and `adocs/plan.md`'s sentence claiming the surveyed engines test at 8+0.08 names the split the OpenBench presets show, 14 of 20 at 8+0.08 with Ethereal, Berserk, Weiss and Stockfish at 10+0.1; S148 and S171 gain their `done:` field and the `author:` lines of S148, S171 and S024 are cleared or explained in one line naming DEC-128 or DEC-111; `tools/plan_prose_check.py --params` covers `adocs/plan_todo/` with named-constant phrases for the parameters those files state, observed red on S115's old text before the rewrite and green after; fast suite green in both builds
touches:    adocs/plan_todo/S115_aspiration_refinements.md, adocs/plan_todo/S082_datagen_qsearch_leaf_labels.md, adocs/plan_todo/S120_eval_cache.md, adocs/plan_todo/S122_king_safety_rebuild.md, adocs/plan_todo/S118_pawn_hash_table.md, adocs/plan_todo/S042_en_passant_only_when_capturable.md, adocs/plan_todo/S159_killer_slot_ageing.md, adocs/plan_todo/S171_inherited_mate_distance.md, adocs/plan_todo/S109_shallow_depth_pruning_block.md, adocs/plan_todo/S110_correction_history_non_pawn.md, adocs/plan_todo/S132_time_management_node_fraction.md, adocs/plan_todo/S148_rfp_ceiling_against_deep_mates.md, adocs/plan_todo/S024_continuation_history.md, adocs/plan.md, adocs/status.md, tools/plan_prose_check.py, tests/CMakeLists.txt
excludes:   any change to what S042 does at the sites it names -- the rule is stated here, the code is S042's; re-anchoring line citations, which is S187; sourcing figures, which is S185; the engine
decisions:  DEC-108, DEC-111, DEC-128
closes:     2026-09-04_plan_review-F05, 2026-09-04_plan_review-F08, 2026-09-04_plan_review-F09, 2026-09-04_plan_review-F10
blocks:
paused_by:
author:
done:

## Why this exists

Four low-to-medium findings of the 2026-09-04 plan review, all of the same
kind: a pending document describes an engine or a plan that no longer exists,
and an implementer following it would act on the stale picture.

**F05, medium.** S115 designs its aspiration sweep on the triple S021 shipped
and S085's verified vector replaced on 2026-08-21, and its constants table
says "keep 5" for the depth gate -- followed, that reverts an axis a +21.02
SPSA verdict moved, silently, inside a step whose verdict would then be
attributed to the window refinements. S082 reasons from a quiescence cap of 8
that has been 19 since S085; S120, S122 and one sentence of `adocs/plan.md`
size their argument on a lazy clamp of 150 that has been 184 since the same
run; S118 prices itself on the pre-S104 binary. The S150 checker keys four
aspiration phrases against `specs.md`, `MANUAL.md`, `DEV_MANUAL.md` and
`plan.md` and never opens `plan_todo/`.

**F08, low.** S042's `touches:` names `make_move` alone. The code's own
comment at the en-passant update in `src/bitboard.cpp` says the key is built
in three places and they must agree, and `load_FEN`'s sanitiser since S161
keeps an en-passant square whenever the victim pawn stands behind an empty
target -- it tests the victim's presence, not whether any pawn can capture.
Change `make_move` alone and a position reached by moves drops a
non-capturable square while the same position loaded from a FEN keeps it: the
transposition mismatch the step exists to remove, moved from move order to
input path, at the root of every SPRT game.

**F09, low.** The lane paragraph counts done S147 among what is left; the
status Next line skips S179; S159 says it "lands before S093, never after"
and S093 landed 2026-08-22; S171's section is titled "Why it is first in the
Open list" while it is postponed and last; S109, S110 and S132 route
follow-up measurements to S128, folded into S152 by DEC-108. And
`adocs/plan.md` says the engines the plan reads from test at 8+0.08 where the
OpenBench presets fetched by the review show Ethereal, Berserk, Weiss and
Stockfish at 10+0.1 and 14 of 20 engines at 8+0.08.

**F10, low.** S148 and S171 have no `done:` field to write the stamp into;
S148, S171 and S024 carry a filled `author:` while sitting in `plan_todo/`,
which the schema reads as a started step.

## The checker extension

`--params` holds no line numbers, which is why it is in the fast suite where
`--citations` is not. The pending files name their parameters by constant
name, so the phrase for them is the constant -- "`ASPIRATION_DELTA` is 50" --
and a named-constant phrase holds no line number either. The extension adds
`adocs/plan_todo/` to the files the mode reads and one phrase family keyed on
the constant names `src/search_params.hpp` declares. Red first on S115's
current text, then green.

## Cost

Documents and a small checker change. Two to three hours.
