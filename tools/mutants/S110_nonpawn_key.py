"""The two non-pawn keys, S110.

Eleven mutants over board_t::nonpawn_hash: one per place its incremental
maintenance can drift from its definition -- each of the three primitives of
the alphabet, the colour a primitive files a piece under, the four places
unmake walks a piece that is not a pawn back by xor, the load, and the
derived key table folding in a pawn row or leaving out the kings. Each moves
the tree as well, since the non-pawn tables read the key, but the walks are
what is meant to see them, and each is named at the case meant to kill it:

  NK01  add_piece leaves the key alone     tests/test_invariants.cpp,
                                           "accumulators and squares survive
                                           make/unmake over the corpus" (a
                                           promotion is the alphabet's one
                                           add); tests/test_engine.cpp, "hash
                                           and board survive make/unmake"
  NK02  remove_piece leaves it alone       both walks (a capture of a piece)
  NK03  move_piece leaves it alone         both walks; "a line of moves and
                                           its fen share the non-pawn keys"
  NK04  move_piece files the piece under   both walks
        the other colour's key
  NK05  unmake skips the mover             both walks' board compares
  NK06  unmake skips the castling rook     both walks' board compares
  NK07  unmake skips the promoted piece    both walks' board compares
  NK08  unmake skips the captured piece    both walks' board compares
  NK09  load_FEN does not set the keys     "hash and board survive
                                           make/unmake", at the load
  NK10  the key table folds in the white   the corpus walk's oracle and its
        pawns' row                         "moves on no other" check; "each
                                           non-pawn key is one colour's
                                           pieces and nothing else"
  NK11  the key table leaves the kings     the corpus walk; "each non-pawn
        out                                key is one colour's pieces and
                                           nothing else", at the king's step

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`; `expected` is "killed".

Prefix NK is new: `git log --all -p -- tools/mutants` shows AW, B, C, CH, CX,
D, E, F, G, GC, H, I, J, K, L, LQ, LW, M, MW, N, NB, NG, NT, P, PD, PK, Q, R,
RZ, S, T, TC, U, V, W, X, Y and Z, and no NK.
"""

B = "src/bitboard.cpp"

m("NK01_add_piece_dropped", B, "board/nonpawn_key",
  'add_piece leaves the non-pawn keys alone, so the piece a pawn promotes to '
  'never enters its colour\'s key and every position after a promotion is '
  'filed under the structure before it',
  ("  board->pawn_hash ^= randoms->pawn_randoms[piece][square];\n"
   "  board->nonpawn_hash[Side] ^= randoms->nonpawn_randoms[piece][square];\n"
   "\n"
   "  eval_add_piece<Side>(board, piece, square);\n",
   "  board->pawn_hash ^= randoms->pawn_randoms[piece][square];\n"
   "  // NK01: the non-pawn key left alone\n"
   "\n"
   "  eval_add_piece<Side>(board, piece, square);\n"),
  origin="S110")

m("NK02_remove_piece_dropped", B, "board/nonpawn_key",
  'remove_piece leaves the non-pawn keys alone, so a captured piece stays in '
  'its colour\'s key',
  ("  board->pawn_hash ^= randoms->pawn_randoms[piece][square];\n"
   "  board->nonpawn_hash[Side] ^= randoms->nonpawn_randoms[piece][square];\n"
   "\n"
   "  eval_remove_piece<Side>(board, piece, square);\n",
   "  board->pawn_hash ^= randoms->pawn_randoms[piece][square];\n"
   "  // NK02: the non-pawn key left alone\n"
   "\n"
   "  eval_remove_piece<Side>(board, piece, square);\n"),
  origin="S110")

m("NK03_move_piece_dropped", B, "board/nonpawn_key",
  'move_piece leaves the non-pawn keys alone, so a knight, bishop, rook, '
  'queen or king that moves keeps its colour\'s old key',
  ("  board->nonpawn_hash[Side] ^= randoms->nonpawn_randoms[piece][from] ^\n"
   "                               randoms->nonpawn_randoms[piece][to];\n",
   "  // NK03: the non-pawn key left alone\n"),
  origin="S110")

m("NK04_move_piece_wrong_colour", B, "board/nonpawn_key",
  'move_piece files a moving piece under the other colour\'s key, so each '
  'key holds the other colour\'s moves',
  ("  board->nonpawn_hash[Side] ^= randoms->nonpawn_randoms[piece][from] ^\n",
   "  board->nonpawn_hash[!Side] ^= randoms->nonpawn_randoms[piece][from] ^\n"),
  origin="S110")

m("NK05_unmake_mover_dropped", B, "board/nonpawn_key",
  'unmake_move does not walk a moving piece back in its colour\'s key, so '
  'every piece move made and unmade leaves the key off by two piece-square '
  'keys',
  ("  board->nonpawn_hash[us] ^= randoms->nonpawn_randoms[move.piece][move.from] ^\n"
   "                             randoms->nonpawn_randoms[move.piece][move.to];\n",
   "  // NK05: the mover left out of the key\n"),
  origin="S110")

m("NK06_unmake_castling_rook_dropped", B, "board/nonpawn_key",
  'unmake_move puts a castling\'s rook back on the board and not in the key: '
  'the one piece move the mover\'s own undo does not cover',
  ("      board->nonpawn_hash[us] ^=\n"
   "          randoms->nonpawn_randoms[rook.piece][rook.from] ^\n"
   "          randoms->nonpawn_randoms[rook.piece][rook.to];\n",
   "      // NK06: the rook left out of the key\n"),
  origin="S110")

m("NK07_unmake_promotion_dropped", B, "board/nonpawn_key",
  'unmake_move takes the promoted piece off the board and not out of the key',
  ("    board->nonpawn_hash[us] ^= randoms->nonpawn_randoms[promoted_to][move.to];\n",
   "    // NK07: the promoted piece left in the key\n"),
  origin="S110")

m("NK08_unmake_captured_dropped", B, "board/nonpawn_key",
  'unmake_move puts a captured piece back on the board and not in its '
  'colour\'s key',
  ("    board->nonpawn_hash[them] ^=\n"
   "        randoms->nonpawn_randoms[entry->captured][move.to];\n",
   "    // NK08: the captured piece left out of the key\n"),
  origin="S110")

m("NK09_not_loaded", B, "board/nonpawn_key",
  'load_FEN leaves the non-pawn keys at the 0 cleanup_board wrote, so every '
  'position loaded from a FEN starts on the empty structure\'s keys',
  ("  board->nonpawn_hash[WHITE] = compute_full_nonpawn_hash(game, WHITE);\n"
   "  board->nonpawn_hash[BLACK] = compute_full_nonpawn_hash(game, BLACK);\n",
   "  // NK09: the non-pawn keys not loaded\n"),
  origin="S110")

m("NK10_key_table_folds_a_pawn", B, "board/nonpawn_key",
  'the derived key table copies the white pawns\' row as well, so every white '
  'pawn move moves White\'s non-pawn key -- the table is the one input of the '
  'branchless form no single site shows wrong',
  ("          pawn ? 0 : zobrist->piece_randoms[piece][square];\n",
   "          (piece == B_PAWN) ? 0 : zobrist->piece_randoms[piece][square];\n"),
  origin="S110")

m("NK11_key_table_drops_the_king", B, "board/nonpawn_key",
  'the derived key table zeroes the two king rows, so a king move moves no '
  'key: the king out of the key the description puts it in',
  ("          pawn ? 0 : zobrist->piece_randoms[piece][square];\n",
   "          (pawn || piece == W_KING || piece == B_KING)\n"
   "              ? 0\n"
   "              : zobrist->piece_randoms[piece][square];\n"),
  origin="S110")
