"""The gives-check test before make_move, S268.

Four mutants over `move_gives_check()` in src/bitboard.cpp, one per piece of the
after-the-move picture it builds, and one over the search site that asks it.
The four are killed by tests/test_movegen.cpp's "move_gives_check agrees with
make_move and is_check", which compares the predicate with the engine's own
make_move() and is_check() on every legal move of its corpus:

  GC01  the en-passant victim stays on its square   misses the check the
                                                     victim was blocking
  GC02  the castling rook stays in its corner       misses the rook's check
  GC03  a promotion attacks as the pawn             misses the new piece's
                                                     check, invents the pawn's
  GC04  the mover still stands on `from`            misses every discovered
                                                     check

GC05 is the search site: the engine's own node skips every move a rule
selected without asking whether it gives check. The probed node keeps the
exemption after make_move (src/search.cpp `negamax_at`), so the guard cases
that drive `negamax_probed` cannot see this one; what kills it is the suite's
unprobed searches -- the mates a pruned check hides -- and the bench signature.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide. Release
builds with -Werror, so a mutant that orphans a local carries a `(void)` of it.

Ids are never reused: GC is this file's own prefix, and no file in
tools/mutants/ has used it before.
"""

B = "src/bitboard.cpp"
S = "src/search.cpp"

m("GC01_gives_check_ep_victim", B, "board/gives_check",
  'the en-passant victim is left on its square in the after-the-move '
  'occupancy, so a capture whose victim blocked a rook or bishop line to the '
  'king is reported as no check',
  ('    const index_t victim =\n'
   '        static_cast<index_t>((us == WHITE) ? (to + 8) : (to - 8));\n'
   '    occupancy ^= BB_1 << victim;',
   '    const index_t victim =\n'
   '        static_cast<index_t>((us == WHITE) ? (to + 8) : (to - 8));\n'
   '    (void)victim;'),
  origin="S268")

m("GC02_gives_check_castling_rook", B, "board/gives_check",
  'castling moves the king alone, so the rook that lands beside it and checks '
  'along its file is not seen',
  ('      mine[W_ROOK - W_PAWN] ^= rook_bb;\n'
   '      occupancy ^= rook_bb;',
   '      (void)rook_bb;'),
  origin="S268")

m("GC03_gives_check_promotion_as_pawn", B, "board/gives_check",
  'a promotion attacks from its square as the pawn it was, so a promoted '
  'piece that checks is missed and a pawn check that is not there is '
  'reported',
  ('  mine[(promoted_to != TO_NONE) ? static_cast<int>(promoted_to) : moved] |=\n'
   '      to_bb;',
   '  (void)promoted_to;\n'
   '  mine[moved] |= to_bb;'),
  origin="S268")

m("GC04_gives_check_from_occupied", B, "board/gives_check",
  'the square the mover left stays occupied, so no slider behind it sees the '
  'king: every discovered check, the king\'s own move included, reads as none',
  ('  bb_t occupancy = (board->occupancies[BOTH] & ~from_bb) | to_bb;',
   '  bb_t occupancy = board->occupancies[BOTH] | to_bb;'),
  origin="S268")

m("GC05_premake_skip_without_asking", S, "search/pruning",
  'the engine\'s node skips every move a rule selected before make_move '
  'without asking whether it gives check: the gives-check exemption is gone '
  'from all five rules in the search the engine plays, while the probed node '
  'still honours it',
  ('        if (!move_gives_check(&game->board, moves[i])) { continue; }\n',
   '        continue;\n'),
  origin="S268")
