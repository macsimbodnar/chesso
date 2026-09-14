"""Capture SEE pruning and the extra reduction, S091.

Two rules and six mutants: every one is a guard dropped or a comparison turned
round, never a constant moved. The case each belongs to establishes the
condition that would make its rule fire and asserts the guard refused it, which
is the S191 shape S109 followed.

This is DEC-141 clause 2 for the two rules that land here: a new pruning or
reduction rule ships with a direct guard test and a mutant that test kills.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. A mutant's several pairs are applied together as one bug. `expected` is
"killed" unless a person has argued the mutant is behaviourally equivalent to
the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

S = "src/search.cpp"

m("C02_capture_gives_check", S, "search/pruning",
  'the gives-check exemption stops binding the capture rule, so a forcing '
  'capture is skipped like any other -- the exemption reaches a capture '
  'through capture_gives_check and not through is_check_move, which is '
  'hardcoded false there',
  ('    if (prune_rule != PRUNE_NONE && !is_check_move && !capture_gives_check) {',
   '    if (prune_rule != PRUNE_NONE && !is_check_move) {'),
  origin="S091")

m("C05_capture_threshold_sign", S, "search/pruning",
  'the capture margin is passed positive, so the rule asks whether the '
  'capture gains the margin and skips every capture that does not',
  ('          !see_ge(&game->board, moves[i], -(SEE_CAPT_COEFF * lmr_depth))) {',
   '          !see_ge(&game->board, moves[i], (SEE_CAPT_COEFF * lmr_depth))) {'),
  origin="S091")

m("C06_capture_no_cap", S, "search/pruning",
  'the capture rule loses its depth cap, so it skips at every reduced depth '
  'instead of the shallow ones its margin is sized for',
  ('      if (lmr_depth < SEE_CAPT_MAX_LMRDEPTH &&\n'
   '          !see_ge(&game->board, moves[i], -(SEE_CAPT_COEFF * lmr_depth))) {',
   '      if (!see_ge(&game->board, moves[i], -(SEE_CAPT_COEFF * lmr_depth))) {'),
  origin="S091")

m("C07_capture_first_move", S, "search/pruning",
  "the capture rule reads pruning_node and its own alpha band instead of "
  "may_prune, so it loses the first-move guard alone and a node's only legal "
  'move can be skipped -- the no-legal-moves return then reports a stalemate '
  'that is not there',
  ('    if (may_prune && is_capture) {',
   '    if (pruning_node && alpha < MATE_MIN && alpha > -MATE_MIN &&\n'
   '        is_capture) {'),
  origin="S091")

m("R01_extra_reduction_gives_check", S, "search/reduction",
  'the extra ply stops exempting a capture that gives check, which it reaches '
  'through capture_gives_check for the reason the skip does',
  ('    const bool may_reduce = ply > 0 && depth >= 3 && legal_moves_counter > 3 &&\n'
   '                            !is_in_check && !is_check_move &&\n'
   '                            !capture_gives_check;',
   '    const bool may_reduce = ply > 0 && depth >= 3 && legal_moves_counter > 3 &&\n'
   '                            !is_in_check && !is_check_move;'),
  origin="S091")

m("R02_extra_reduction_sign", S, "search/reduction",
  'the extra ply reads the exchange evaluation the wrong way round, so what '
  'it reduces is the moves that win material',
  ('        !MOVE_PROMOTED(moves[i]) && !see_ge(&game->board, moves[i], 0);',
   '        !MOVE_PROMOTED(moves[i]) && see_ge(&game->board, moves[i], 0);'),
  origin="S091")
