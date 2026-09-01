#!/usr/bin/env python3
"""S144. Give every bare `:line` continuation in a pending step file its path.

    adocs/data/S144_paths.py propose   # what it would write, one row per citation
    adocs/data/S144_paths.py detail    # only the ones it cannot decide
    adocs/data/S144_paths.py apply     # rewrite them, and write the mapping

A bare continuation -- `pick_next_move :654, make_move :656` -- inherits its
path from a sentence. `tools/plan_prose_check.py --citations` counts those and
refuses to resolve them, because guessing the inherited path produced sixty
impossible line numbers when S138 tried it. This script does not guess either:
it proposes a path and then makes the proposal *falsifiable* by relocating the
text the citation was written against.

**Two things are being repaired at once and the second is not optional.** A
bare `:654` is not only pathless, it is as stale as the full citations S169
re-anchored -- S169 moved `src/search.cpp:645` to `:896` in S091 and left the
`:624`, `:654`, `:656` and `:658` beside it untouched, because they carried no
path for it to check. Converting those to `src/search.cpp:654` without moving
them would take a citation that is merely unchecked and make it checked and
wrong, and DRIFT could not say so: a step file's DRIFT baseline is the commit
that last wrote it, so this step's own commit becomes the baseline and every
converted citation is green by construction. DEC-119 again, one step later.

So the baseline is taken per *line* and not per file:

    git blame --ignore-rev <S169> -L n,n -- <step file>

`--ignore-rev` is what makes this work at all. S169 rewrote the full citation
on many of these lines and nothing else about them, so a plain blame answers
`fe25f46` -- S169's own commit, which is HEAD -- and the baseline text equals
the current text for every citation on such a line. Ignoring that one revision
attributes the line to the commit that actually wrote the number.

Methods, tried in this order per citation, per candidate path:

  BLOCK   the text `path:a-b` held at the citation's blame commit occurs
          exactly once in that file at HEAD. The repair is that location, and
          a candidate path that produces a unique hit is the evidence for the
          path as well as for the line.
  SAME    the block is found at HEAD in more than one place, one of which is
          where the citation already points. The citation stands; the path is
          the candidate that matched.
  TITLE   the block is gone, and a doctest title quoted beside the citation
          names a test that exists in the candidate. The repair is where that
          title opens now, keeping the span.

Anything none of them decides is UNRESOLVED and is read by hand into HAND
below. Nothing is resolved by deleting a citation, by dropping a line number,
or by widening a range.

Candidate paths come from the citing paragraph only, nearest first and
preceding before following, which is the inheritance the prose actually uses.
A citation whose paragraph names no path at all cannot be resolved here and is
reported as such.
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(REPO, "tools"))

import plan_prose_check as ppc  # noqa: E402

# S169's commit. Every line it touched, it touched to move a line number, so
# blame has to look past it to find who wrote the number being repaired here.
S169 = "fe25f46bfe3af7c1c966409d52d4b794aa4ba5f9"

# **The path is read out of the prose, not inferred from it.** A relocation
# only tests a path that the sentence could plausibly mean: a file that has not
# changed since the citation was written answers `BLOCK` at the exact line asked
# for, for every line in it, so letting the search pick among candidates makes
# an unchanged file win every ambiguous case. Measured on the first pass: 84 of
# 383 landed in `src/bitboard.cpp` that way, including nine in a paragraph whose
# every sentence is about `negamax`.
#
# So ownership is stated. OWNER keys a paragraph -- by (step file, its first
# line) -- to the path its bare continuations inherit; OVERRIDE keys a single
# citation, by (step file, line, citation as written), for the paragraphs that
# name two files and alternate between them. Both are filled in by reading the
# paragraph, which is what this step exists to do once so that no later reader
# has to.
OWNER = {
    # S020 -- the two search functions, with the generator preamble named
    # explicitly where the prose crosses into it.
    ("adocs/plan_todo/S020_single_check_computation.md", 43): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 47): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 56): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 61): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 67): "tools/datagen.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 70): "src/bitboard.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 88): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 113): "src/search.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 149): "src/search.cpp",
    # S022 -- quiescence throughout.
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 81): "src/search.cpp",
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 120): "src/search.cpp",
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 162): "src/search.cpp",
    # S055 -- the guard's paragraph is about the test file; the two sites and
    # the model are named against it.
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 127): "tests/test_eval_model.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 140): "src/evaluation.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 163): "src/evaluation.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 194): "src/evaluation.cpp",
    # S091 -- the SEE API paragraph names bitboard.cpp first and then moves to
    # the negamax loop without saying so.
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 98): "src/search.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 158): "src/search.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 200): "tests/test_search.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 246): "src/search.cpp",
    # S095
    ("adocs/plan_todo/S095_internal_iterative_reduction.md", 82): "src/search.cpp",
    ("adocs/plan_todo/S095_internal_iterative_reduction.md", 139): "src/search_params.hpp",
    ("adocs/plan_todo/S095_internal_iterative_reduction.md", 154): "src/search.cpp",
    # S097
    ("adocs/plan_todo/S097_singular_extensions.md", 122): "src/search.cpp",
    ("adocs/plan_todo/S097_singular_extensions.md", 179): "src/search.cpp",
    ("adocs/plan_todo/S097_singular_extensions.md", 209): "src/search.cpp",
    ("adocs/plan_todo/S097_singular_extensions.md", 225): "src/search_params.hpp",
    ("adocs/plan_todo/S097_singular_extensions.md", 250): "src/search.cpp",
    # S098 -- every citation is the LMR block in negamax.
    ("adocs/plan_todo/S098_reduction_refinement.md", 115): "src/search.cpp",
    ("adocs/plan_todo/S098_reduction_refinement.md", 117): "src/search.cpp",
    ("adocs/plan_todo/S098_reduction_refinement.md", 167): "src/search.cpp",
    ("adocs/plan_todo/S098_reduction_refinement.md", 206): "src/search.cpp",
    ("adocs/plan_todo/S098_reduction_refinement.md", 343): "src/search.cpp",
    # S099
    ("adocs/plan_todo/S099_correction_history.md", 111): "src/bitboard.cpp",
    ("adocs/plan_todo/S099_correction_history.md", 132): "src/chesso.cpp",
    ("adocs/plan_todo/S099_correction_history.md", 142): "src/search.cpp",
    ("adocs/plan_todo/S099_correction_history.md", 168): "src/search.cpp",
    ("adocs/plan_todo/S099_correction_history.md", 206): "src/search.cpp",
    # S109
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 178): "src/search.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 188): "src/search.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 226): "src/search.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 293): "src/search.cpp",
    # S112
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 103): "src/search.cpp",
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 164): "src/search.cpp",
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 214): "src/search.cpp",
    # S113
    ("adocs/plan_todo/S113_probcut.md", 99): "src/search.cpp",
    ("adocs/plan_todo/S113_probcut.md", 140): "src/search.cpp",
    # S114
    ("adocs/plan_todo/S114_null_move_refinements.md", 32): "src/search.cpp",
    ("adocs/plan_todo/S114_null_move_refinements.md", 86): "src/search.cpp",
    ("adocs/plan_todo/S114_null_move_refinements.md", 113): "src/search.cpp",
    ("adocs/plan_todo/S114_null_move_refinements.md", 152): "src/search.cpp",
    ("adocs/plan_todo/S114_null_move_refinements.md", 213): "src/search.cpp",
    # S115 -- the aspiration loop lives in the UCI driver, the return paths in
    # the search.
    ("adocs/plan_todo/S115_aspiration_refinements.md", 70): "src/search.cpp",
    ("adocs/plan_todo/S115_aspiration_refinements.md", 76): "src/chesso.cpp",
    ("adocs/plan_todo/S115_aspiration_refinements.md", 115): "src/chesso.cpp",
    ("adocs/plan_todo/S115_aspiration_refinements.md", 126): "src/chesso.cpp",
    ("adocs/plan_todo/S115_aspiration_refinements.md", 188): "src/search.cpp",
    # S116
    ("adocs/plan_todo/S116_razoring_depth_one.md", 81): "src/search.cpp",
    ("adocs/plan_todo/S116_razoring_depth_one.md", 104): "src/search.cpp",
    ("adocs/plan_todo/S116_razoring_depth_one.md", 107): "src/search.cpp",
    ("adocs/plan_todo/S116_razoring_depth_one.md", 144): "src/search.cpp",
    ("adocs/plan_todo/S116_razoring_depth_one.md", 187): "src/search.cpp",
    # S117 -- the evaluation's tables and sums, with the accumulator alphabet
    # and the test's REQUIRE named against them.
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 84): "src/evaluation.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 115): "src/evaluation.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 145): "tools/tuner.cpp",
    # S120
    ("adocs/plan_todo/S120_eval_cache.md", 100): "src/evaluation.cpp",
    ("adocs/plan_todo/S120_eval_cache.md", 130): "src/evaluation.cpp",
    ("adocs/plan_todo/S120_eval_cache.md", 147): "src/evaluation.cpp",
    # S131 -- the inventory paragraph runs generator, search, evaluation and
    # SEE through one bullet list; the search is its subject and the other
    # three are overridden citation by citation.
    ("adocs/plan_todo/S131_quiescence_promotions.md", 66): "src/search.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 118): "src/search.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 153): "src/search.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 187): "src/search.cpp",
    # S132
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 73): "src/chesso.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 86): "src/search.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 104): "src/chesso.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 114): "src/chesso.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 150): "src/chesso.cpp",
}

OVERRIDE = {
    ("adocs/plan_todo/S020_single_check_computation.md", 52, ":504"): "src/bitboard.cpp",
    ("adocs/plan_todo/S020_single_check_computation.md", 52, ":516-531"): "src/bitboard.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 128, ":953-954"): "src/evaluation.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 138, ":973-974"): "tools/eval_model.hpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 173, ":481-484"): "tests/test_evaluation.cpp",
    ("adocs/plan_todo/S055_taper_stage_two_once.md", 177, ":976-977"): "tools/eval_model.hpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 99, ":1185"): "src/bitboard.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 100, ":1196-1201"): "src/bitboard.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 100, ":1209-1212"): "src/bitboard.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 101, ":1160"): "src/bitboard.cpp",
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 102, ":1096"): "src/bitboard.cpp",
    ("adocs/plan_todo/S097_singular_extensions.md", 140, ":360-378"): "src/data_structures.hpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 210, ":1271"): "src/bitboard.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 211, ":1096"): "src/bitboard.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 211, ":1160"): "src/bitboard.cpp",
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 315, ":1099-1105"): "src/evaluation.cpp",
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 112, ":1160-1174"): "src/bitboard.cpp",
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 121, ":44-49"): "src/eval_tables.hpp",
    ("adocs/plan_todo/S113_probcut.md", 113, ":87"): "src/search.cpp",
    ("adocs/plan_todo/S113_probcut.md", 116, ":1185"): "src/bitboard.cpp",
    ("adocs/plan_todo/S113_probcut.md", 117, ":303-307"): "src/bitboard.cpp",
    ("adocs/plan_todo/S113_probcut.md", 140, ":41"): "src/search_params.hpp",
    ("adocs/plan_todo/S114_null_move_refinements.md", 114, ":77-78"): "src/search_params.hpp",
    ("adocs/plan_todo/S115_aspiration_refinements.md", 191, ":762"): "src/chesso.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 85, ":118"): "src/eval_tables.hpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 94, ":704"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 94, ":727-728"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 95, ":937-938"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 95, ":953-954"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 95, ":967"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 95, ":975-976"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 95, ":982"): "src/bitboard.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 108, ":481-484"): "tests/test_evaluation.cpp",
    ("adocs/plan_todo/S117_swar_packed_eval_score.md", 130, ":481-484"): "tests/test_evaluation.cpp",
    ("adocs/plan_todo/S120_eval_cache.md", 136, ":529-530"): "src/search.cpp",
    ("adocs/plan_todo/S120_eval_cache.md", 153, ":253"): "src/search.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 73, ":207-223"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 73, ":229-244"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 74, ":236"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 74, ":261-263"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 93, ":1094"): "src/evaluation.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 101, ":1203-1204"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 103, ":1209-1212"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 103, ":1293"): "src/bitboard.cpp",
    ("adocs/plan_todo/S131_quiescence_promotions.md", 103, ":1300-1307"): "src/bitboard.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 90, ":777"): "src/chesso.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 98, ":746-767"): "src/chesso.cpp",
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 171, ":654"): "src/search.cpp",
}

# One citation was not merely pathless, it was malformed, and the malformation
# is why it survived every earlier pass: `:1099-:1039` is a range whose second
# half carries its own colon, so the citation regex reads the first number and
# nothing at all sees the second. Repaired to a range before the conversion
# runs, and recorded here rather than done silently.
TEXT_FIX = [
    ("adocs/plan_todo/S120_eval_cache.md",
     "never the :1099-:1039\nbound",
     "never the :1099-1100\nbound",
     "the two lazy-shortcut bound returns as one range. The second half never "
     "carried a path and never moved when the first was re-anchored, so it "
     "named a line 60 short and no checker could see it."),
]


# Resolved by hand where relocation cannot decide: the cited text is gone, or
# the citation was already wrong when it was written. Keyed like OVERRIDE; the
# value is (full citation to write, why).
HAND = {
    # The same seven sites, cited from many steps. Each block sits in both
    # quiescence() and negamax() at HEAD, or grew a comment under the citation,
    # so the exact-block relocation cannot separate the two; the citing sentence
    # can, and does. quiescence() opens at src/search.cpp:293 and negamax() at
    # :589, which is what every "in negamax" reason below rests on.
    ("adocs/plan_todo/S020_single_check_computation.md", 117, ":656"): (
        "src/search.cpp:917", "negamax's `make_move` in the move loop; the "
        "identical line in quiescence is :510."),
    ("adocs/plan_todo/S020_single_check_computation.md", 117, ":727"): (
        "src/search.cpp:994", "negamax's `unmake_move`; quiescence's is :517."),
    ("adocs/plan_todo/S020_single_check_computation.md", 122, ":268-277"): (
        "src/search.cpp:421-436",
        "quiescence's stand-pat store-and-return. DEC-102 added six comment "
        "lines inside the block, so the span grew with it."),
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 101, ":289"): (
        "src/search.cpp:448",
        "quiescence's `move_t moves[MAX_MOVES];`, the generation the sentence "
        "puts the early-out before; negamax's identical line is :846."),
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 107, ":387-390"): (
        "src/search.cpp:576-578",
        "quiescence's final tt_store_entry, now three lines rather than four: "
        "DEC-102 hoisted the node type into `stored_type` above it."),
    ("adocs/plan_todo/S022_delta_pruning_quiescence.md", 187, ":387-390"): (
        "src/search.cpp:576-578", "the same store, cited a second time."),
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 114, ":205"): (
        "src/search.cpp:205",
        "**Historical, and deliberately not moved.** The sentence records what "
        "the excludes line read before S138 re-anchored it. The path is what "
        "was missing; the number is the record."),
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 118, ":656"): (
        "src/search.cpp:917", "negamax's `make_move`, the pre-make seam the "
        "capture skip sits in."),
    ("adocs/plan_todo/S091_see_pruning_main_search.md", 120, ":656"): (
        "src/search.cpp:917", "the same line, the second half of `between "
        ":654 and :656`."),
    ("adocs/plan_todo/S095_internal_iterative_reduction.md", 90, ":387-390"): (
        "src/search.cpp:576-578", "quiescence's fail-low store, the third of "
        "the three moveless quiescence stores the sentence lists."),
    ("adocs/plan_todo/S097_singular_extensions.md", 140, ":360-378"): (
        "src/data_structures.hpp:360-380",
        "`enum node_type_t`. S106 replaced TT_PV_NODE's pasted comment, so the "
        "enum is two lines longer than the citation was written against."),
    ("adocs/plan_todo/S097_singular_extensions.md", 140, ":445"): (
        "src/search.cpp:663", "negamax's `tt_get_entry` probe; quiescence's is "
        "at :318."),
    ("adocs/plan_todo/S097_singular_extensions.md", 167, ":656-663"): (
        "src/search.cpp:917-930",
        "make_move through `legal_moves_counter++`, which is what the sentence "
        "names. S107's comment and S149's history call grew the span."),
    ("adocs/plan_todo/S097_singular_extensions.md", 190, ":729"): (
        "src/search.cpp:996",
        "the move loop's abort check, after unmake_move; the other "
        "`if (state->aborted) { return 0; }` at :834 is the null-move one."),
    ("adocs/plan_todo/S097_singular_extensions.md", 281, ":729"): (
        "src/search.cpp:996", "the same abort check, cited again."),
    ("adocs/plan_todo/S099_correction_history.md", 147, ":529-536"): (
        "src/search.cpp:767-771",
        "the RFP margin and its fail-soft return. S108 hoisted the "
        "compute-or-read that opened the cited span to :720, so what remains "
        "inside the guard is the margin and the return."),
    ("adocs/plan_todo/S099_correction_history.md", 170, ":729"): (
        "src/search.cpp:996", "the move loop's abort check, not the null "
        "move's."),
    ("adocs/plan_todo/S099_correction_history.md", 181, ":536"): (
        "src/search.cpp:771", "the RFP decision, `static_eval - margin >= "
        "beta`."),
    ("adocs/plan_todo/S099_correction_history.md", 230, ":247"): (
        "src/search.cpp:349", "quiescence's `evaluate_lazy` call, which S130 "
        "moved under the TT substitution."),
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 180, ":656"): (
        "src/search.cpp:917", "negamax's `make_move`."),
    ("adocs/plan_todo/S109_shallow_depth_pruning_block.md", 184, ":739-758"): (
        "src/search.cpp:1006-1030",
        "the fail-high update block through the close of its quiet-only arm. "
        "S149's killer comment and history_on_quiet_cutoff rewrote its middle."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 106, ":339-366"): (
        "src/search.cpp:507-545",
        "quiescence's search loop, `for` to closing brace. DEC-102's "
        "`value_type` block grew it."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 142, ":253"): (
        "src/search.cpp:361", "`stored_eval`, which S130's comment moved."),
    ("adocs/plan_todo/S112_quiescence_move_futility.md", 157, ":387-390"): (
        "src/search.cpp:576-578", "quiescence's fail-low store."),
    ("adocs/plan_todo/S113_probcut.md", 99, ":511-537"): (
        "src/search.cpp:765-772", "the RFP block, condition to closing brace."),
    ("adocs/plan_todo/S113_probcut.md", 99, ":561-588"): (
        "src/search.cpp:814-841",
        "the null-move block, `null_reduction` to closing brace -- 28 lines "
        "then and now."),
    ("adocs/plan_todo/S113_probcut.md", 100, ":588"): (
        "src/search.cpp:841", "the null-move block's closing brace."),
    ("adocs/plan_todo/S113_probcut.md", 107, ":656"): (
        "src/search.cpp:917", "negamax's `make_move`, whose false return drops "
        "illegal moves."),
    ("adocs/plan_todo/S113_probcut.md", 129, ":532"): (
        "src/search.cpp:731",
        "`state->static_evals[ply] = static_eval;`. S108 hoisted this out of "
        "the RFP guard the sentence puts it in; the sentence is S113's to "
        "restate and the citation points at where the value is set now."),
    ("adocs/plan_todo/S113_probcut.md", 141, ":588"): (
        "src/search.cpp:841", "the same closing brace."),
    ("adocs/plan_todo/S113_probcut.md", 148, ":729"): (
        "src/search.cpp:996", "the move loop's abort check."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 89, ":569-571"): (
        "src/search.cpp:822-824", "the null-move entry conditions."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 92, ":570"): (
        "src/search.cpp:823", "the floor, `depth - 1 - null_reduction >= 1`."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 109, ":477"): (
        "src/search.cpp:720",
        "`const int static_eval =`. S108 hoisted the declaration to the top of "
        "the node, so the sentence's claim that it sits inside the RFP guard "
        "is stale -- that is S114's to restate, not a citation repair."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 109, ":532"): (
        "src/search.cpp:731",
        "the stack write S108 pairs with that declaration, the second of the "
        "two sites the sentence names."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 114, ":77-78"): (
        "src/search_params.hpp:181-182",
        "NULL_MOVE_BASE and NULL_MOVE_DIVISOR, the two the new constants sit "
        "beside -- the same pair the paragraph above cites in full."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 117, ":570"): (
        "src/search.cpp:823", "the floor, cited again."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 154, ":570"): (
        "src/search.cpp:823", "the floor, cited again."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 177, ":588"): (
        "src/search.cpp:841", "the null-move block's closing brace."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 177, ":561-588"): (
        "src/search.cpp:814-841", "the null-move block this step rewrites."),
    ("adocs/plan_todo/S114_null_move_refinements.md", 215, ":570"): (
        "src/search.cpp:823", "the floor, cited again."),
    ("adocs/plan_todo/S115_aspiration_refinements.md", 71, ":546"): (
        "src/search.cpp:771",
        "RFP's fail-soft return. It reads `static_eval` since S108 where the "
        "citation was written against `static_score`, which is why the block "
        "match could not find it."),
    ("adocs/plan_todo/S115_aspiration_refinements.md", 189, ":546"): (
        "src/search.cpp:771", "the same return, cited again."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 81, ":479-537"): (
        "src/search.cpp:733-772",
        "the RFP block with its comment, which is what `after the RFP block` "
        "means for a site that goes below it."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 82, ":539-588"): (
        "src/search.cpp:774-841", "the null-move block with its comment."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 84, ":536"): (
        "src/search.cpp:771", "RFP's fail-soft return."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 104, ":537"): (
        "src/search.cpp:772", "the RFP block's closing brace, which is where "
        "the razor's ten lines go."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 166, ":420"): (
        "src/search.cpp:608", "negamax's `state->explored_nodes++`."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 167, ":200"): (
        "src/search.cpp:302", "quiescence's `state->explored_nodes++`, the "
        "second count the sentence is about."),
    ("adocs/plan_todo/S116_razoring_depth_one.md", 197, ":539-588"): (
        "src/search.cpp:774-841", "the null-move block, cited again."),
    ("adocs/plan_todo/S120_eval_cache.md", 114, ":1099-1100"): (
        "src/evaluation.cpp:1099-1100",
        "the two lazy-shortcut bound returns. The citation was written as a "
        "range whose second half never carried a path and never moved when the "
        "first did, so it read `:1099-:1039` -- one live line and one stale "
        "one. The two lines are adjacent, so the range is the form."),
    ("adocs/plan_todo/S120_eval_cache.md", 136, ":529-530"): (
        "src/search.cpp:720-723",
        "negamax's `evaluate()` call. S108 moved it out of the RFP guard and "
        "gave it the in-check arm; it is still the direct caller the sentence "
        "means."),
    ("adocs/plan_todo/S120_eval_cache.md", 153, ":253"): (
        "src/search.cpp:361", "quiescence's `stored_eval`, what the exact flag "
        "writes on to the table."),
    ("adocs/plan_todo/S131_quiescence_promotions.md", 109, ":433-437"): (
        "src/search.cpp:621-655",
        "negamax's repetition test and its halfmove-clock test, the pair the "
        "sentence says quiescence lacks. S162 grew the clock arm from one line "
        "to nine plus its comment."),
    ("adocs/plan_todo/S131_quiescence_promotions.md", 162, ":287"): (
        "src/search.cpp:446", "the qply cap, `qply >= MAX_QSEARCH_DEPTH`."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 94, ":656"): (
        "src/search.cpp:917", "negamax's `make_move`, where the snapshot is "
        "taken."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 94, ":727"): (
        "src/search.cpp:994", "negamax's `unmake_move`, where the delta is "
        "taken."),
    ("adocs/plan_todo/S132_time_management_node_fraction.md", 100, ":420"): (
        "src/search.cpp:608", "negamax's own node count, the root's +1."),
}

_blame = {}


def blame_rev(path, line):
    key = (path, line)
    if key not in _blame:
        out = subprocess.run(
            ["git", "-C", REPO, "blame", "--ignore-rev", S169,
             "-L", f"{line},{line}", "--porcelain", "--", path],
            capture_output=True, text=True).stdout
        _blame[key] = out.split()[0] if out else None
    return _blame[key]


def para_paths(para):
    """Every path token in a paragraph, with the offset it sits at."""
    return [(m.start(), m.group(1)) for m in ppc.PATH.finditer(para)]


def file_paths(text, tracked, byname):
    """Every path token in a step file, as (line, resolved path)."""
    out = []
    for m in ppc.PATH.finditer(text):
        resolved = (m.group(1) if m.group(1) in tracked
                    else byname.get(m.group(1)))
        if resolved:
            out.append((text.count("\n", 0, m.start()) + 1, resolved))
    return out


def candidates(para, offset, line, whole, tracked, byname):
    """Paths a bare citation could be inheriting, nearest first.

    Paragraph-local first, because that is the inheritance the prose uses.
    Then the rest of the file, nearest preceding before nearest following: a
    section heading names a file once and then continues in bare form for a
    page, which is S098's shape and paragraph scope alone reports it as having
    no candidate at all. Widening the list does not widen what is accepted --
    every candidate still has to relocate the block, and a candidate that
    cannot is not written.
    """
    ordered, seen = [], set()
    local = []
    for start, spelled in para_paths(para):
        resolved = spelled if spelled in tracked else byname.get(spelled)
        if resolved:
            local.append((start, resolved))
    for start, resolved in (sorted([x for x in local if x[0] < offset],
                                   key=lambda t: -t[0])
                            + sorted([x for x in local if x[0] >= offset],
                                     key=lambda t: t[0])):
        if resolved not in seen:
            seen.add(resolved)
            ordered.append(resolved)
    rest = sorted(whole, key=lambda t: (0 if t[0] <= line else 1,
                                        abs(t[0] - line)))
    for _pline, resolved in rest:
        if resolved not in seen:
            seen.add(resolved)
            ordered.append(resolved)
    return ordered


def locate(path, rev, a, b, here):
    """Where the block `path:a-b` at `rev` sits in `path` at HEAD."""
    then = ppc._at(rev, path)
    if then is None or a < 1 or b > len(then):
        return None, None, None
    block = "\n".join(then[a - 1:b])
    now = ppc._at(None, path)
    if now is None:
        return None, None, block
    span = b - a + 1
    hits = [i for i in range(len(now) - span + 1)
            if "\n".join(now[i:i + span]) == block]
    if len(hits) == 1:
        return "BLOCK", hits[0] + 1, block
    # A block with almost nothing in it -- a closing brace, a blank line -- sits
    # in every file and in twenty places in this one, so a multi-hit match on
    # one is evidence of nothing. Reported, never accepted: the whole point of
    # relocating the block is that the match is what argues for the path.
    if len(" ".join(block.split())) < 12:
        return "THIN", None, block
    if len(hits) > 1 and (here - 1) in hits:
        return "SAME", here, block
    return None, None, block


def title_move(para, line, first, path, cite, a, b):
    claimed = ppc.anchor_titles(para, line, first, path, cite)
    known = ppc.titles_of(path)
    for t in claimed:
        if t in known:
            return known[t]
    return None


def rows():
    tracked, byname = ppc._tracked()
    out = []
    for step in ppc.pending_step_files(os.path.join(REPO, "adocs")):
        rel = os.path.relpath(step, REPO)
        text = open(step, encoding="utf-8").read()
        whole = file_paths(text, tracked, byname)
        for first, para in ppc.paragraphs(text):
            marks = []
            for m in ppc.PATH.finditer(para):
                d = ppc.DIRECT.match(para, m.end())
                if d:
                    marks.append((d.start(), d.end()))
            for m in ppc.CONT.finditer(para):
                if any(s <= m.start() < e for s, e in marks):
                    continue
                a = int(m.group(1))
                b = int(m.group(2)) if m.group(2) else a
                line = first + para.count("\n", 0, m.start())
                cite = f":{a}" + (f"-{b}" if b != a else "")
                row = dict(step=rel, line=line, cite=cite, a=a, b=b,
                           first=first, mstart=m.start(), mend=m.end(),
                           rev=blame_rev(rel, line))
                key = (rel, line, cite)
                if key in HAND:
                    row.update(method="HAND", write=HAND[key][0],
                               why=HAND[key][1])
                    out.append(row)
                    continue
                owner = OVERRIDE.get(key) or OWNER.get((rel, first))
                if owner:
                    cands = [owner]
                    row["owned"] = True
                else:
                    cands = candidates(para, m.start(), line, whole,
                                       tracked, byname)
                    row["owned"] = False
                row["cands"] = cands
                thin = None
                for path in cands:
                    method, new, block = locate(path, row["rev"], a, b, a)
                    if method == "THIN":
                        # Kept only if no candidate does better: a blank line
                        # in one file is a blank line in every other.
                        thin = thin or (path, block)
                        continue
                    if method:
                        span = b - a + 1
                        end = new + span - 1
                        row.update(method=method, path=path, new=new, end=end,
                                   block=block, rank=cands.index(path))
                        break
                if "method" not in row:
                    for path in cands:
                        opens = title_move(para, line, first, path, cite, a, b)
                        if opens:
                            row.update(method="TITLE", path=path, new=opens,
                                       end=opens + (b - a), block=None)
                            break
                    else:
                        if thin:
                            row.update(method="THIN", path=thin[0],
                                       block=thin[1])
                        else:
                            row["method"] = "UNRESOLVED"
                # An unowned paragraph's proposal is a guess and is labelled
                # one, however cleanly the block relocated.
                if not row["owned"] and row["method"] in ("BLOCK", "SAME",
                                                          "TITLE"):
                    row["method"] = "GUESS-" + row["method"][:5]
                out.append(row)
    return out


def spell(row):
    if row["method"] == "HAND":
        return row["write"]
    if row["method"] in ("UNRESOLVED", "THIN"):
        return None
    tail = f"{row['new']}" + (f"-{row['end']}" if row["end"] != row["new"]
                              else "")
    return f"{row['path']}:{tail}"


def review(data):
    """Every proposal under the paragraph that has to justify it.

    The relocation is evidence for a path only when the candidate file is the
    one the sentence means; an unchanged file answers `BLOCK` at the very line
    asked for, for any line in it, which is why the proposals are read here
    and not trusted from the tally.
    """
    tracked, byname = ppc._tracked()
    by_step = {}
    for r in data:
        by_step.setdefault(r["step"], []).append(r)
    for step, rs in by_step.items():
        text = open(os.path.join(REPO, step), encoding="utf-8").read()
        print("=" * 78)
        print(f"### {step}")
        seen = set()
        for first, para in ppc.paragraphs(text):
            here = [r for r in rs
                    if first <= r["line"] <= first + para.count("\n")]
            if not here:
                continue
            print("-" * 78)
            for i, pl in enumerate(para.split("\n")):
                print(f"{first + i:5}| {pl}")
            for r in here:
                seen.add(id(r))
                w = spell(r)
                print(f"  {r['method']:10} {r['cite']:12} -> {w or '?'}"
                      f"   rank={r.get('rank', '-')}")
                if w and ":" in w:
                    path, tail = w.rsplit(":", 1)
                    lo = int(tail.split("-")[0])
                    hi = int(tail.split("-")[-1])
                    now = ppc._at(None, path) or []
                    for n in range(lo, min(hi, len(now)) + 1):
                        print(f"        {path}:{n}| {now[n - 1][:96]}")


def line_offsets(text):
    out, at = [0], 0
    for raw in text.split("\n"):
        at += len(raw) + 1
        out.append(at)
    return out


def apply_fixes():
    """The malformed citations, repaired before anything is converted."""
    for step, old, new, _why in TEXT_FIX:
        path = os.path.join(REPO, step)
        text = open(path, encoding="utf-8").read()
        if old not in text:
            print(f"TEXT_FIX already applied or stale: {step}")
            continue
        open(path, "w", encoding="utf-8").write(text.replace(old, new, 1))
        print(f"TEXT_FIX {step}: {old!r} -> {new!r}")


def apply(data):
    """Write the paths in, and the mapping that is the evidence for them."""
    by_step = {}
    for r in data:
        if r["method"] == "UNRESOLVED":
            continue
        by_step.setdefault(r["step"], []).append(r)

    tsv = [("step\tline\tcite\tmethod\tbaseline\twritten\t"
            "then_text\tnow_text\tsame\treason")]
    written = 0
    for step, rs in by_step.items():
        path = os.path.join(REPO, step)
        text = open(path, encoding="utf-8").read()
        offs = line_offsets(text)
        edits = []
        for r in rs:
            base = offs[r["first"] - 1]
            edits.append((base + r["mstart"], base + r["mend"], spell(r)))
        for start, end, new in sorted(edits, key=lambda e: -e[0]):
            text = text[:start] + new + text[end:]
            written += 1
        open(path, "w", encoding="utf-8").write(text)

    for r in data:
        if r["method"] == "UNRESOLVED":
            continue
        new = spell(r)
        npath, ntail = new.rsplit(":", 1)
        lo = int(ntail.split("-")[0])
        hi = int(ntail.split("-")[-1])
        now = ppc._at(None, npath) or []
        now_text = " / ".join(l.strip() for l in now[lo - 1:hi])
        then = ppc._at(r["rev"], npath) if r["rev"] else None
        then_text = ""
        if then and r["b"] <= len(then):
            then_text = " / ".join(l.strip() for l in then[r["a"] - 1:r["b"]])
        why = HAND.get((r["step"], r["line"], r["cite"]), ("", ""))[1]
        tsv.append("\t".join([
            r["step"], str(r["line"]), r["cite"], r["method"],
            (r["rev"] or "")[:7], new,
            then_text[:300], now_text[:300],
            "yes" if then_text == now_text else "no", why]))

    out = os.path.join(HERE, "S144_pathings.tsv")
    with open(out, "w", encoding="utf-8") as fh:
        fh.write("\n".join(tsv) + "\n")
    print(f"{written} citations rewritten, mapping in {out}")




# ---------------------------------------------------------------- rewrapping

# A path is fifteen characters where a bare `:line` was five, so a converted
# sentence overflows the hard wrap these documents are written to. 266 lines
# went past 80 columns in the conversion. Re-flowing is therefore part of the
# repair and not a tidy-up, and it is deliberately timid: only a unit that
# actually holds an over-long line is touched, everything else stays
# byte-identical, and headers, tables, fenced code and indented code are never
# units at all.
WIDTH = 79
HEADER = re.compile(r"^[a-z_]+: {2,}")
BULLET = re.compile(r"^(\s*)((?:[-*]|\d+\.)\s+)(.*)$")


def flow_units(lines):
    """(start, end, first_indent, cont_indent) for each re-flowable unit."""
    units, i, n = [], 0, len(lines)
    fenced = False
    while i < n:
        line = lines[i]
        if line.strip().startswith("```"):
            fenced = not fenced
        if (fenced or not line.strip() or line.startswith("|")
                or line.lstrip().startswith("```")
                or line.startswith("#") or HEADER.match(line)
                or line.startswith("    ")):
            i += 1
            continue
        m = BULLET.match(line)
        first = m.group(1) + m.group(2) if m else re.match(r"^\s*", line).group(0)
        cont = " " * len(first) if m else first
        start = i
        i += 1
        # The unit runs to the next blank line, bullet, table row, heading or
        # fence. A continuation line of a bullet is indented under it, which is
        # what stops a nested list from being swallowed.
        while i < n:
            nxt = lines[i]
            if (not nxt.strip() or nxt.startswith("|") or nxt.startswith("#")
                    or nxt.lstrip().startswith("```")
                    or BULLET.match(nxt) or HEADER.match(nxt)):
                break
            i += 1
        if m and i > start + 1:
            cont = re.match(r"^\s*", lines[start + 1]).group(0)
        units.append((start, i, first, cont))
    return units


def rewrap_text(text):
    lines = text.split("\n")
    out, at = [], 0
    for start, end, first, cont in flow_units(lines):
        out.extend(lines[at:start])
        block = lines[start:end]
        if max(len(b) for b in block) <= WIDTH:
            out.extend(block)
        else:
            words = " ".join(b.strip() for b in block).split()
            line, wrapped = first, []
            for w in words:
                cand = line + w if line.endswith(" ") or not line.strip() \
                    else line + " " + w
                if line.strip() and len(cand) > WIDTH:
                    wrapped.append(line.rstrip())
                    line = cont + w
                else:
                    line = cand
            wrapped.append(line.rstrip())
            out.extend(wrapped)
        at = end
    out.extend(lines[at:])
    return "\n".join(out)


def rewrap():
    """Re-flow what the conversion overflowed, then re-point the mapping.

    Driven from the mapping and not from a fresh scan, because by the time this
    runs there are no bare continuations left to scan for.
    """
    tsvp = os.path.join(HERE, "S144_pathings.tsv")
    rows = open(tsvp, encoding="utf-8").read().rstrip("\n").split("\n")
    steps = sorted({r.split("\t")[0] for r in rows[1:]})
    moved = 0
    for step in steps:
        path = os.path.join(REPO, step)
        text = open(path, encoding="utf-8").read()
        new = rewrap_text(text)
        if new != text:
            open(path, "w", encoding="utf-8").write(new)
            moved += 1
    print(f"{moved} step files re-flowed")

    # The mapping's line column is the citation's line in the file, so it has
    # to follow. Rows are in document order and so are the occurrences, which
    # is what makes the nth-occurrence match exact rather than a search.
    head, body = rows[0], rows[1:]
    seen, fixed = {}, []
    for row in body:
        col = row.split("\t")
        step, written = col[0], col[5]
        text = open(os.path.join(REPO, step), encoding="utf-8").read()
        k = (step, written)
        start = seen.get(k, 0)
        at = text.find(written, start)
        seen[k] = at + 1 if at != -1 else start
        col[1] = str(text.count("\n", 0, at) + 1) if at != -1 else col[1]
        fixed.append("\t".join(col))
    with open(tsvp, "w", encoding="utf-8") as fh:
        fh.write("\n".join([head] + fixed) + "\n")
    print(f"{len(fixed)} mapping rows re-pointed at their line after the wrap")


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "propose"
    if mode == "rewrap":
        rewrap()
        return 0
    if mode == "apply":
        apply_fixes()
    data = rows()
    if mode == "apply":
        apply(data)
        rewrap()
        return 0
    if mode == "review":
        review(data)
        return 0
    tally = {}
    for r in data:
        tally[r["method"]] = tally.get(r["method"], 0) + 1
        if mode == "detail" and r["method"] in ("BLOCK", "SAME", "HAND"):
            continue
        if mode in ("propose", "detail"):
            print(f"{r['method']:10} {r['step']}:{r['line']}  {r['cite']}"
                  f"  ->  {spell(r) or '?'}"
                  f"   [{r['rev'][:7] if r['rev'] else 'none'}]"
                  f" rank={r.get('rank', '-')}"
                  f" cands={','.join(r.get('cands', []))[:70]}")
    print("total", len(data), tally)
    return 0


if __name__ == "__main__":
    sys.exit(main())
