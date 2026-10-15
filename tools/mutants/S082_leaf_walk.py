"""The quiescence leaf walk, S082.

Nine mutants over src/leaf_walk.cpp, the walk that names the position a
corpus row records (LW06 to LW08 added by the S082 fast check, LW09 by its
rebase onto S279's quiescence, which tests the cap at every node). None of them
can move `bench` -- no engine path calls the walk -- so the suite is the only
oracle, and each is killed by a case in tests/test_leaf_walk.cpp's "leaf walk"
suite:

  LW01  the bound records a truncated walk     "a walk stopped by its ply
        as a leaf                               bound is refused as truncated"
  LW02  the walk stops after its first move    "every quiet leaf the walk
        and calls that the leaf                 records is one quiescence
                                                stands pat at"
  LW03  a position the rules decide is         "a line that ends in mate is
        recorded as a leaf                      terminal, not quiet"
  LW04  one move of the walk is left made on   "every quiet leaf the walk
        the game it was handed                  records is one quiescence
                                                stands pat at"
  LW05  the root is recorded in place of the   "every quiet leaf the walk
        leaf                                    records is one quiescence
                                                stands pat at"
  LW06  the PV's end is recorded unresolved    "a principal variation is
                                                played out and then resolved"
  LW07  a repetition before the root refuses   "a repetition refuses a leaf
        the leaf (the game-level rule)          only where the search draws it"
  LW08  a repetition inside the line is        "a repetition refuses a leaf
        walked past                             only where the search draws it"
  LW09  the walker's node cap never reaches    "a call past the walker's node
        quiescence's limit check                cap is refused as aborted"

LW01 is the step's own trap -- a leaf reached by exhausting the bound is not
quiet -- and LW02 is the truncation the quiet case alone must catch: a walk
cut short of its leaf records a position quiescence still moves from.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent, which no tool can decide.

Ids are never reused: LW is this file's own prefix, and no file in
tools/mutants/ has used it (L is the reduction family's, LQ S269's).
"""

W = "src/leaf_walk.cpp"

m("LW01_bound_records_truncated", W, "corpus/leaf",
  'a walk that reaches its ply bound is reported quiet, so datagen records the '
  'position it stopped at -- one a capture still changes, the noise the leaf '
  'exists to remove',
  ('    if (plies >= bound) { return LEAF_TRUNCATED; }',
   '    if (plies >= bound) { return LEAF_QUIET; }'),
  origin="S082")

m("LW02_walk_stops_after_one_move", W, "corpus/leaf",
  'the walk makes quiescence\'s first move and calls the position after it '
  'the leaf without asking again, so a recapture quiescence would still make '
  'is left on the board the row records',
  ('    (*made)++;\n'
   '    plies++;\n',
   '    (*made)++;\n'
   '    plies++;\n'
   '    return LEAF_QUIET;\n'),
  origin="S082")

m("LW03_terminal_recorded", W, "corpus/leaf",
  'a position the rules have decided -- a mate the line ran into -- is '
  'recorded as a quiet leaf, fitting evaluate() to a position it is never the '
  'final word on',
  ('    if (rules_decide(game, root_history_size)) { return LEAF_TERMINAL; }',
   '    if (rules_decide(game, root_history_size)) { return LEAF_QUIET; }'),
  origin="S082")

m("LW04_game_left_moved", W, "corpus/leaf",
  'one move of the walk is not unmade, so the game datagen goes on playing '
  'stands a ply further on than the game it handed over',
  ('  for (int i = 0; i < made; ++i) {',
   '  for (int i = 1; i < made; ++i) {'),
  origin="S082")

m("LW05_root_recorded", W, "corpus/leaf",
  'the position is written down after the walk has been unmade, so every row '
  'names the root again and the leaf is computed and thrown away',
  ('  walk->fen = generate_FEN(&game->board);\n'
   '  walk->phase = game_phase(&game->board);\n'
   '\n'
   '  for (int i = 0; i < made; ++i) {\n'
   '    unmake_move(game);\n'
   '  }\n',
   '  for (int i = 0; i < made; ++i) {\n'
   '    unmake_move(game);\n'
   '  }\n'
   '\n'
   '  walk->fen = generate_FEN(&game->board);\n'
   '  walk->phase = game_phase(&game->board);\n'),
  origin="S082")

m("LW06_pv_end_unresolved", W, "corpus/leaf",
  'the principal variation is played out and its end recorded as the leaf '
  'without resolving it, so a line that ends on a capture writes a position '
  'quiescence still moves from -- the amendment\'s "a PV leaf out of check and '
  'not terminal can still be unresolved"',
  # The (void): Release drops the assert, the only other reader of `walker`,
  # so without it the mutant does not compile under -Werror (stillborn in
  # phase 2's first pass, 2026-10-16).
  ('    walk.outcome = walk_from(walker, game, root_history_size, &made);',
   '    (void)walker;\n'
   '    walk.outcome = LEAF_QUIET;'),
  origin="S082")

m("LW07_repetition_before_root", W, "corpus/leaf",
  'the game-level rule, a two-fold anywhere, refuses a leaf for one earlier '
  'occurrence before the root -- a position the search goes on searching and '
  'evaluate() is asked about, dropped for the game\'s history',
  ('  if (classify_repetition(&game->history, &game->board, root_history_size) ==\n'
   '      repetition_kind_t::DRAW) {\n',
   '  (void)root_history_size;\n'
   '  if (classify_repetition(&game->history, &game->board, game->history.size) !=\n'
   '      repetition_kind_t::NONE) {\n'),
  origin="S082")

m("LW08_repetition_ignored", W, "corpus/leaf",
  'a position repeated inside the line, a draw to the search, is walked past '
  'and resolved as if the rules had not decided it',
  ('  if (classify_repetition(&game->history, &game->board, root_history_size) ==\n'
   '      repetition_kind_t::DRAW) {\n'
   '    return true;\n'
   '  }\n',
   '  (void)root_history_size;\n'),
  origin="S082")

m("LW09_node_cap_unread", W, "corpus/leaf",
  'the walker never hands its node cap to the search state, so quiescence\'s '
  'limit check reads the unlimited budget the state starts with, and a call '
  'that should be refused as aborted runs on to an answer',
  ('  state->node_limit = walker->node_cap;\n',
   ''),
  origin="S082")
