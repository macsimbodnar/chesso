"""Board mutants: make_move, the hash, the rules and move generation.

M26 is the one declared equivalent. A two-ply repetition needs two
consecutive null moves and `prev_move != 0` forbids the second, so a scan
starting four plies back cannot miss a repetition this search can reach.
The declaration is the argument, not the still bench: a green suite on a
still signature is what `equivalent` and `survived` share.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

B = "src/bitboard.cpp"

m("M22_castling_rights_on_capture", B, "board/make_move",
  'castling right not lost when the rook is captured',
  ('board->castling & castling_rights[move.from] & castling_rights[move.to]);',
   'board->castling & castling_rights[move.from]);'),
  origin="2026-09-04_test_review")

m("M23_ep_hash_skipped", B, "board/hash",
  'en-passant square changes without touching the hash',
  ('    board->hash ^= randoms->ep_randoms[board->en_passant];\n    board->hash ^= randoms->ep_randoms[new_en_passant];\n    board->en_passant = new_en_passant;',
   '    board->en_passant = new_en_passant;'),
  origin="2026-09-04_test_review")

m("M24_halfmove_not_reset_on_capture", B, "board/make_move",
  'halfmove clock not reset by a capture',
  ('if (move.capture || move.piece == W_PAWN || move.piece == B_PAWN) {',
   'if (move.piece == W_PAWN || move.piece == B_PAWN) {'),
  origin="2026-09-04_test_review")

m("M25_insufficient_two_minors", B, "rules",
  'two minors called insufficient material',
  ('return count_bits(minors) <= 1;',
   'return count_bits(minors) <= 2;'),
  origin="2026-09-04_test_review")

m("M26_repetition_skips_first", B, "rules",
  'repetition scan starts four plies back',
  ('for (size_t back = 2; back <= limit; back += 2) {',
   'for (size_t back = 4; back <= limit; back += 2) {'),
  expected="equivalent",
  origin="2026-09-04_test_review")

m("M27_castle_through_check", B, "movegen",
  'king-side castling ignores an attacked f-square',
  ('!is_attacked(tables, board, f_square, opponent) &&\n            !is_attacked(tables, board, g_square, opponent)) {',
   '!is_attacked(tables, board, g_square, opponent)) {'),
  origin="2026-09-04_test_review")

m("M28_see_ge_ep_victim", B, "see",
  'see_ge forgets the en-passant victim',
  ('    occupancy ^= BB_1 << victim;\n    gain = see_value[0];',
   '    occupancy ^= BB_1 << victim;\n    gain = 0;'),
  origin="2026-09-04_test_review")

m("M35_repetition_root_entry_counts", B, "rules",
  "the root's own occurrence scored as an in-tree repetition",
  ('if (index > root_history_size) { return repetition_kind_t::DRAW; }',
   'if (index >= root_history_size) { return repetition_kind_t::DRAW; }'),
  origin="S207")

# The `(void)` orphans root_history_size, which Release builds with
# -Werror=unused-parameter: without it the mutant does not compile, which is a
# fact about the mutant and not about the suite.
m("M36_repetition_ignores_the_root", B, "rules",
  'any earlier occurrence scored as a draw, the pre-S207 rule',
  ('    if (index > root_history_size) { return repetition_kind_t::DRAW; }\n\n'
   '    // A second match is a third occurrence of the position on the board, which\n'
   '    // FIDE 9.2 lets either player claim wherever the occurrences lie.\n'
   '    if (seen_one) { return repetition_kind_t::DRAW; }',
   '    (void) root_history_size;\n    return repetition_kind_t::DRAW;'),
  origin="S207")

# S208's two load-boundary rules. Both were shipped with tests and without
# mutants, and the Tier-1 fast check over 8aff8ac found the gap by trying M37
# by hand: it leaks a pawn on h8 and the whole fast suite stayed green, because
# no FEN anywhere in the tree has one. The cases that kill these two now cover
# all four back-rank corners and both sides of the 16-piece bound.
m("M37_back_rank_pawn_off_by_one", B, "rules",
  'the back-rank pawn refusal leaks the h8 corner',
  ('          (square < 8 || square >= 56)) {',
   '          (square < 7 || square >= 56)) {'),
  origin="S208")

m("M38_piece_count_bound_off_by_one", B, "rules",
  'the piece-count refusal admits seventeen of a colour',
  ('    if (piece_count[WHITE] > 16 || piece_count[BLACK] > 16) {',
   '    if (piece_count[WHITE] > 17 || piece_count[BLACK] > 17) {'),
  origin="S208")
