"""The pawn key, S099 part (a).

Five mutants over the key board_t::pawn_hash: one per way its incremental
maintenance can drift from its definition -- a make that does not move it, an
unmake that does not walk a pawn back or put an en-passant victim back, a
load that never sets it, and the derived key table folding in a piece that
is not a pawn. On part (a)'s tree nothing read the key, so each was
node-identical there and only the walks could see it; with part (b) reading
the key they move the tree as well.

  PK01  move_piece leaves the key alone       tests/test_invariants.cpp,
                                              "accumulators and squares
                                              survive make/unmake over the
                                              corpus"; tests/test_engine.cpp,
                                              "hash and board survive
                                              make/unmake"
  PK02  unmake skips the mover                both walks' board compares
  PK03  load_FEN does not set the key         "hash and board survive
                                              make/unmake", at the load
  PK04  unmake skips the en-passant victim    the corpus walk, whose census
                                              floors hold en passant present
  PK05  the key table carries the white       the corpus walk's oracle and its
        knights' row as well                  "moves on no other" check

Since DEC-271 the alphabet xors the key for every piece from
zobrist_randoms_t::pawn_randoms, zero but for the pawn rows, instead of
testing the piece; PK01 and PK02 are anchored on that form and PK05 is new
with it. The file is kept apart from tools/mutants/S099_correction_history.py
because each is anchored on its own part.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`; `expected` is "killed".

Prefix PK is new: `git log --all -p -- tools/mutants` shows AW, GC, LQ, MW,
NG, NT, PD, RZ, TC and every single letter taken, and no PK.
"""

B = "src/bitboard.cpp"

m("PK01_move_piece_dropped", B, "board/pawn_key",
  'move_piece leaves the pawn key alone, so a pawn push or capture-by-a-pawn '
  'keeps the structure\'s old key and every pawn move after it is filed '
  'under the wrong structure',
  ("  board->pawn_hash ^=\n"
   "      randoms->pawn_randoms[piece][from] ^ randoms->pawn_randoms[piece][to];\n",
   "  // PK01: the pawn key left alone\n"),
  origin="S099")

m("PK02_unmake_mover_dropped", B, "board/pawn_key",
  'unmake_move does not walk a moving pawn back in the key, so every pawn '
  'move made and unmade leaves the key off by two piece-square keys',
  ("  board->pawn_hash ^= randoms->pawn_randoms[move.piece][move.from] ^\n"
   "                      randoms->pawn_randoms[move.piece][move.to];\n",
   "  // PK02: the mover left out of the key\n"),
  origin="S099")

m("PK03_not_loaded", B, "board/pawn_key",
  'load_FEN leaves the pawn key at the 0 cleanup_board wrote, so every '
  'position loaded from a FEN starts on the pawnless structure\'s key',
  ("  board->pawn_hash = compute_full_pawn_hash(game);\n",
   "  // PK03: the pawn key not loaded\n"),
  origin="S099")

m("PK04_unmake_en_passant_dropped", B, "board/pawn_key",
  'unmake_move puts the en-passant victim back on the board and not in the '
  'key: the one pawn event a walk that skips en passant would never see',
  ("    board->pawn_hash ^= randoms->piece_randoms[captured_pawn][captured_square];\n",
   "    // PK04: the victim left out of the key\n"),
  origin="S099")

m("PK05_key_table_folds_knights", B, "board/pawn_key",
  'the derived key table copies the white knights\' row as well as the two '
  'pawn rows, so every knight move moves the pawn key -- the table is the '
  'one input of the branchless form no single site shows wrong',
  ("    const bool pawn = piece == W_PAWN || piece == B_PAWN;\n",
   "    const bool pawn = piece == W_PAWN || piece == B_PAWN || piece == W_KNIGHT;\n"),
  origin="S099")
