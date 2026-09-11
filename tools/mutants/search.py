"""Search mutants: pruning, reduction, the transposition table, ordering,
quiescence and the mate scores.

The twenty-one bugs the 2026-09-04 test review wrote for src/search.cpp,
each one a guard dropped, a sign flipped or an off-by-one that a person
could plausibly ship. Two of them consume the symbol they orphan with a
`(void)`: the Release build the gate and this tool run carries -Werror,
so M08 and M12 in their first form did not compile, which measures
nothing.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

S = "src/search.cpp"

m("M01_nmp_in_check", S, "search/pruning",
  'null move allowed while in check',
  ('if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&',
   'if (!is_pv && ply > 0 && prev_move != 0 &&'),
  origin="2026-09-04_test_review")

m("M02_nmp_mate_band_neg", S, "search/pruning",
  'S165 guard dropped: null move at beta <= -MATE_MIN',
  ('depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&',
   'depth - 1 - null_reduction >= 1 && beta < MATE_MIN &&'),
  origin="2026-09-04_test_review")

m("M03_nmp_zugzwang", S, "search/pruning",
  'null move in pawn endings (game_phase 0)',
  ('      game_phase(&game->board) > 0) {\n    const int reduction = null_reduction;',
   '      true) {\n    const int reduction = null_reduction;'),
  origin="2026-09-04_test_review")

m("M04_nmp_mate_artifact", S, "search/pruning",
  'null-move mate score returned as a real score',
  ('return (null_score >= MATE_MIN) ? beta : null_score;',
   'return null_score;'),
  origin="2026-09-04_test_review")

m("M05_rfp_margin_flat", S, "search/pruning",
  'RFP margin not scaled by depth',
  ('const int margin = RFP_MARGIN * depth;',
   'const int margin = RFP_MARGIN;'),
  origin="2026-09-04_test_review")

m("M06a_rfp_ply_floor_minus1", S, "search/pruning",
  'RFP ply floor one ply lower',
  ('static_cast<int>(ply) >= RFP_MIN_PLY &&\n      depth <= RFP_MAX_DEPTH &&',
   'static_cast<int>(ply) >= RFP_MIN_PLY - 1 &&\n      depth <= RFP_MAX_DEPTH &&'),
  origin="2026-09-04_test_review")

m("M06b_rfp_ply_floor_minus2", S, "search/pruning",
  'RFP ply floor two plies lower',
  ('static_cast<int>(ply) >= RFP_MIN_PLY &&\n      depth <= RFP_MAX_DEPTH &&',
   'static_cast<int>(ply) >= RFP_MIN_PLY - 2 &&\n      depth <= RFP_MAX_DEPTH &&'),
  origin="2026-09-04_test_review")

m("M07_lmr_captures", S, "search/reduction",
  'LMR reduces captures',
  ('if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {',
   'if (ply > 0 && depth >= 3 && legal_moves_counter > 3 &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {'),
  origin="2026-09-04_test_review")

m("M08_lmr_checks", S, "search/reduction",
  'LMR reduces checking moves (S107 exemption dropped)',
  ('if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {',
   '(void)is_check_move;\n    if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check) {'),
  origin="2026-09-04_test_review")

m("M09_lmr_no_research", S, "search/reduction",
  'reduced search beating alpha is believed, no re-search',
  ('if (!state->aborted && reduction > 0 && score > alpha) {',
   'if (false && !state->aborted && reduction > 0 && score > alpha) {'),
  origin="2026-09-04_test_review")

m("M10_pvs_no_research", S, "search/pvs",
  'null-window fail-high never re-searched with the full window',
  ('if (!state->aborted && score > alpha && score < beta) {',
   'if (false && !state->aborted && score > alpha && score < beta) {'),
  origin="2026-09-04_test_review")

m("M11_tt_cut_on_pv", S, "search/tt",
  'TT cutoffs taken at PV nodes',
  ('if (!is_pv && ply > 0) {\n    int tt_score = 0;',
   'if (ply > 0) {\n    int tt_score = 0;'),
  origin="2026-09-04_test_review")

m("M12_tt_depth_ignored", S, "search/tt",
  'TT entry answers regardless of stored depth',
  ('if (entry == nullptr || entry->depth < depth) { return false; }',
   '(void)depth;\n  if (entry == nullptr) { return false; }'),
  origin="2026-09-04_test_review")

m("M13_tt_bounds_swapped", S, "search/tt",
  'lower/upper bound types confused on probe',
  ('if (entry->type == TT_BETA_NODE && tt_score >= beta) {',
   'if (entry->type == TT_BETA_NODE && tt_score <= alpha) {'),
  ('if (entry->type == TT_ALPHA_NODE && tt_score <= alpha) {',
   'if (entry->type == TT_ALPHA_NODE && tt_score >= beta) {'),
  origin="2026-09-04_test_review")

m("M14_mate_normalize_sign", S, "search/mate",
  'mate score normalised with the wrong sign',
  ('if (score > MATE_MIN && score < MATE_MAX) { return score + ply; }',
   'if (score > MATE_MIN && score < MATE_MAX) { return score - ply; }'),
  origin="2026-09-04_test_review")

m("M15_killer_slot1_dead", S, "search/ordering",
  'second killer slot never written',
  ('state->killer_moves[1][ply] = state->killer_moves[0][ply];\n        state->killer_moves[0][ply] = moves[i];',
   'state->killer_moves[0][ply] = moves[i];'),
  origin="2026-09-04_test_review")

m("M16_history_malus_sign", S, "search/ordering",
  'history malus applied as a bonus',
  ('                           -malus);',
   '                           malus);'),
  origin="2026-09-04_test_review")

m("M17_qs_standpat_in_check", S, "search/quiescence",
  'standing pat allowed while in check',
  ('  if (!in_check) {\n    if (stand_pat >= beta) {',
   '  if (true) {\n    if (stand_pat >= beta) {'),
  origin="2026-09-04_test_review")

m("M18_qs_see_prune_in_check", S, "search/quiescence",
  'losing captures pruned even when in check',
  ('if (!in_check && !capture_cannot_lose(&game->board, moves[i]) &&',
   'if (!capture_cannot_lose(&game->board, moves[i]) &&'),
  origin="2026-09-04_test_review")

m("M19_fifty_off_by_one", S, "rules",
  'fifty-move draw one halfmove late',
  ('if (game->board.halfmove_clock >= 100) {',
   'if (game->board.halfmove_clock >= 101) {'),
  origin="2026-09-04_test_review")

m("M20_mated_distance_off", S, "search/mate",
  'main-search mate distance off by one ply',
  ('return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;',
   'return is_in_check ? -(MATE_MAX - static_cast<int>(ply) - 1) : DRAW_SCORE;'),
  origin="2026-09-04_test_review")

m("M21_standpat_lower_bound_lowers", S, "search/tt",
  'a lower bound lowers the stand pat (DEC-102 inverted)',
  ('(tt_entry->type == TT_BETA_NODE && tt_score > stand_pat) ||',
   '(tt_entry->type == TT_BETA_NODE && tt_score < stand_pat) ||'),
  origin="2026-09-04_test_review")


# S215's pair, both anchored on the one line that hands classify_repetition()
# its boundary. M35 and M36 pin the comparison; nothing pinned the value it is
# compared against until the case "search() hands the rule the root's own
# index" was written -- `- 1` left the whole fast suite green while `bench`
# moved 13 %. Neither is equivalent and both are killed: `- 1` by the depth-4
# search that must answer the material, `+ 1` by the depth-5 search that must
# answer the draw. The two settings that are not arithmetic on the size are
# killed by the same case and are not mutants here: `0` fails exactly where
# `- 1` does, and deleting the assignment -- leaving the SIZE_MAX default --
# fails exactly where `+ 1` does.
m("M39_root_boundary_one_low", S, "rules",
  "the root's own entry read as in-tree, the pre-S207 rule for that class",
  ('state->root_history_size = game->history.size;',
   'state->root_history_size = game->history.size - 1;'),
  origin="S215")

m("M40_root_boundary_one_high", S, "rules",
  'the first entry the search pushed read as pre-root',
  ('state->root_history_size = game->history.size;',
   'state->root_history_size = game->history.size + 1;'),
  origin="S215")
