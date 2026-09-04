"""Hand-written mutants for a fault-injection pass over chesso's fast suite.

Each mutant is one plausible engine bug: a guard dropped, a sign flipped, an
off-by-one. `old` must occur exactly once in `file`; the driver refuses a
mutant whose anchor is ambiguous or missing. `pairs` lists (old, new) edits
applied together as one mutant.
"""

M = []

def m(mid, file, klass, note, *pairs):
    M.append(dict(id=mid, file=file, klass=klass, note=note, pairs=list(pairs)))

S = "src/search.cpp"
B = "src/bitboard.cpp"
E = "src/evaluation.cpp"
C = "src/chesso.cpp"

m("M01_nmp_in_check", S, "search/pruning", "null move allowed while in check",
  ("if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&",
   "if (!is_pv && ply > 0 && prev_move != 0 &&"))
m("M02_nmp_mate_band_neg", S, "search/pruning", "S165 guard dropped: null move at beta <= -MATE_MIN",
  ("depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN &&",
   "depth - 1 - null_reduction >= 1 && beta < MATE_MIN &&"))
m("M03_nmp_zugzwang", S, "search/pruning", "null move in pawn endings (game_phase 0)",
  ("      game_phase(&game->board) > 0) {\n    const int reduction = null_reduction;",
   "      true) {\n    const int reduction = null_reduction;"))
m("M04_nmp_mate_artifact", S, "search/pruning", "null-move mate score returned as a real score",
  ("return (null_score >= MATE_MIN) ? beta : null_score;",
   "return null_score;"))
m("M05_rfp_margin_flat", S, "search/pruning", "RFP margin not scaled by depth",
  ("const int margin = RFP_MARGIN * depth;",
   "const int margin = RFP_MARGIN;"))
m("M06a_rfp_ply_floor_minus1", S, "search/pruning", "RFP ply floor one ply lower",
  ("static_cast<int>(ply) >= RFP_MIN_PLY &&\n      depth <= RFP_MAX_DEPTH &&",
   "static_cast<int>(ply) >= RFP_MIN_PLY - 1 &&\n      depth <= RFP_MAX_DEPTH &&"))
m("M06b_rfp_ply_floor_minus2", S, "search/pruning", "RFP ply floor two plies lower",
  ("static_cast<int>(ply) >= RFP_MIN_PLY &&\n      depth <= RFP_MAX_DEPTH &&",
   "static_cast<int>(ply) >= RFP_MIN_PLY - 2 &&\n      depth <= RFP_MAX_DEPTH &&"))
m("M07_lmr_captures", S, "search/reduction", "LMR reduces captures",
  ("if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {",
   "if (ply > 0 && depth >= 3 && legal_moves_counter > 3 &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {"))
m("M08_lmr_checks", S, "search/reduction", "LMR reduces checking moves (S107 exemption dropped)",
  ("if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move) {",
   "(void)is_check_move;\n    if (ply > 0 && depth >= 3 && legal_moves_counter > 3 && !is_capture &&\n        !MOVE_PROMOTED(moves[i]) && !is_in_check) {"))
m("M09_lmr_no_research", S, "search/reduction", "reduced search beating alpha is believed, no re-search",
  ("if (!state->aborted && reduction > 0 && score > alpha) {",
   "if (false && !state->aborted && reduction > 0 && score > alpha) {"))
m("M10_pvs_no_research", S, "search/pvs", "null-window fail-high never re-searched with the full window",
  ("if (!state->aborted && score > alpha && score < beta) {",
   "if (false && !state->aborted && score > alpha && score < beta) {"))
m("M11_tt_cut_on_pv", S, "search/tt", "TT cutoffs taken at PV nodes",
  ("if (!is_pv && ply > 0) {\n    int tt_score = 0;",
   "if (ply > 0) {\n    int tt_score = 0;"))
m("M12_tt_depth_ignored", S, "search/tt", "TT entry answers regardless of stored depth",
  ("if (entry == nullptr || entry->depth < depth) { return false; }",
   "(void)depth;\n  if (entry == nullptr) { return false; }"))
m("M13_tt_bounds_swapped", S, "search/tt", "lower/upper bound types confused on probe",
  ("if (entry->type == TT_BETA_NODE && tt_score >= beta) {",
   "if (entry->type == TT_BETA_NODE && tt_score <= alpha) {"),
  ("if (entry->type == TT_ALPHA_NODE && tt_score <= alpha) {",
   "if (entry->type == TT_ALPHA_NODE && tt_score >= beta) {"))
m("M14_mate_normalize_sign", S, "search/mate", "mate score normalised with the wrong sign",
  ("if (score > MATE_MIN && score < MATE_MAX) { return score + ply; }",
   "if (score > MATE_MIN && score < MATE_MAX) { return score - ply; }"))
m("M15_killer_slot1_dead", S, "search/ordering", "second killer slot never written",
  ("state->killer_moves[1][ply] = state->killer_moves[0][ply];\n        state->killer_moves[0][ply] = moves[i];",
   "state->killer_moves[0][ply] = moves[i];"))
m("M16_history_malus_sign", S, "search/ordering", "history malus applied as a bonus",
  ("                           -malus);",
   "                           malus);"))
m("M17_qs_standpat_in_check", S, "search/quiescence", "standing pat allowed while in check",
  ("  if (!in_check) {\n    if (stand_pat >= beta) {",
   "  if (true) {\n    if (stand_pat >= beta) {"))
m("M18_qs_see_prune_in_check", S, "search/quiescence", "losing captures pruned even when in check",
  ("if (!in_check && !capture_cannot_lose(&game->board, moves[i]) &&",
   "if (!capture_cannot_lose(&game->board, moves[i]) &&"))
m("M19_fifty_off_by_one", S, "rules", "fifty-move draw one halfmove late",
  ("if (game->board.halfmove_clock >= 100) {",
   "if (game->board.halfmove_clock >= 101) {"))
m("M20_mated_distance_off", S, "search/mate", "main-search mate distance off by one ply",
  ("return is_in_check ? -(MATE_MAX - static_cast<int>(ply)) : DRAW_SCORE;",
   "return is_in_check ? -(MATE_MAX - static_cast<int>(ply) - 1) : DRAW_SCORE;"))
m("M21_standpat_lower_bound_lowers", S, "search/tt", "a lower bound lowers the stand pat (DEC-102 inverted)",
  ("(tt_entry->type == TT_BETA_NODE && tt_score > stand_pat) ||",
   "(tt_entry->type == TT_BETA_NODE && tt_score < stand_pat) ||"))
m("M22_castling_rights_on_capture", B, "board/make_move", "castling right not lost when the rook is captured",
  ("board->castling & castling_rights[move.from] & castling_rights[move.to]);",
   "board->castling & castling_rights[move.from]);"))
m("M23_ep_hash_skipped", B, "board/hash", "en-passant square changes without touching the hash",
  ("    board->hash ^= randoms->ep_randoms[board->en_passant];\n    board->hash ^= randoms->ep_randoms[new_en_passant];\n    board->en_passant = new_en_passant;",
   "    board->en_passant = new_en_passant;"))
m("M24_halfmove_not_reset_on_capture", B, "board/make_move", "halfmove clock not reset by a capture",
  ("if (move.capture || move.piece == W_PAWN || move.piece == B_PAWN) {",
   "if (move.piece == W_PAWN || move.piece == B_PAWN) {"))
m("M25_insufficient_two_minors", B, "rules", "two minors called insufficient material",
  ("return count_bits(minors) <= 1;",
   "return count_bits(minors) <= 2;"))
m("M26_repetition_skips_first", B, "rules", "repetition scan starts four plies back",
  ("for (size_t back = 2; back <= limit; back += 2) {",
   "for (size_t back = 4; back <= limit; back += 2) {"))
m("M27_castle_through_check", B, "movegen", "king-side castling ignores an attacked f-square",
  ("!is_attacked(tables, board, f_square, opponent) &&\n            !is_attacked(tables, board, g_square, opponent)) {",
   "!is_attacked(tables, board, g_square, opponent)) {"))
m("M28_see_ge_ep_victim", B, "see", "see_ge forgets the en-passant victim",
  ("    occupancy ^= BB_1 << victim;\n    gain = see_value[0];",
   "    occupancy ^= BB_1 << victim;\n    gain = 0;"))
m("M29_eval_expensive_sign", E, "eval", "mobility/king-safety sign flipped for the side to move",
  ("return (board->active_color == WHITE) ? bounded : -bounded;",
   "return (board->active_color == WHITE) ? -bounded : bounded;"))
m("M30_mvv_lva_sign", E, "ordering", "MVV-LVA adds the attacker value instead of subtracting",
  ("return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];",
   "return piece_values_abs[victim] + piece_values_abs[MOVE_PIECE(move)];"))
m("M31_lazy_bound_no_margin", E, "eval/lazy", "lazy shortcut taken without its margin",
  ("if (cheap - LAZY_EVAL_MARGIN >= beta) { return cheap - LAZY_EVAL_MARGIN; }",
   "if (cheap >= beta) { return cheap - LAZY_EVAL_MARGIN; }"))
m("M32_hard_limit_no_overhead", C, "time", "hard limit ignores MOVE_OVERHEAD_MS",
  ("hard_ms = std::min(hard_ms, static_cast<int64_t>(remaining_ms) -\n                                  static_cast<int64_t>(MOVE_OVERHEAD_MS));",
   "hard_ms = std::min(hard_ms, static_cast<int64_t>(remaining_ms));"))
m("M33_soft_limit_unbounded", C, "time", "soft limit not capped by the hard limit",
  ("soft_ms = std::max<int64_t>(std::min(soft_ms, hard_ms), 1);",
   "soft_ms = std::max<int64_t>(soft_ms, 1);"))
