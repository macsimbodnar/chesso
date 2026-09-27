"""Per-move futility in quiescence, S112.

Six mutants over the one rule this step adds. Every one is a guard dropped, a
lookup taken from the wrong square or a claim written with the wrong type,
never a constant moved; `QsFutility` at 0 is the off value DEC-215 asks to be
proved on the tree and is not a bug. Each is killed by a case in
tests/test_search.cpp's "search: quiescence futility" suite, named per mutant:

  F01  the fail-soft raise dropped          "a capture that cannot reach alpha
                                             is skipped and its best case is
                                             the node's value"
  F02  the in-check guard dropped           "futility does not skip an evasion
                                             while in check"
  F03  the promotion exemption dropped      "a capturing promotion below the
                                             threshold is searched"
  F04  the gives-check exemption dropped    "a capture that gives check below
                                             the threshold is searched"
  F05  en passant's victim from MOVE_TO     "an en passant capture is priced
                                             at the pawn it takes"
  F06  the fold keeps the floor's type      "a folded futility value is stored
                                             as an upper bound"

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: F is this file's own prefix, and no file in
tools/mutants/ has ever used it (Q is S238's, on its own branch).
"""

S = "src/search.cpp"

m("F01_qs_futility_no_raise", S, "search/pruning",
  'the skip drops the capture without folding its best case into the '
  'fail-soft maximum, so a node that skipped everything reports its stand pat '
  '-- a bound lower than the skipped moves might reach, handed to a parent '
  'that stores and compares it as though a search had produced it',
  ('          if (futility_value > futility_best) {\n'
   '            futility_best = futility_value;\n'
   '          }\n',
   '          (void)futility_best;\n'),
  origin="S112")

m("F02_qs_futility_in_check", S, "search/pruning",
  'futility runs while in check, where every evasion must be searched: a '
  'skipped capture evasion leaves legal_moves at zero and the node declares a '
  'mate in a position with a legal reply',
  ('  const bool qs_futility = QS_FUTILITY != 0 && !in_check;',
   '  const bool qs_futility = QS_FUTILITY != 0;'),
  origin="S112")

m("F03_qs_futility_promotion", S, "search/pruning",
  'a capturing promotion is priced by its victim alone, under-pricing it by '
  'queen-minus-pawn, and skipped like any other capture',
  ('    if (qs_futility && !MOVE_PROMOTED(moves[i])) {',
   '    if (qs_futility) {'),
  origin="S112")

m("F04_qs_futility_gives_check", S, "search/pruning",
  'the gives-check exemption stops binding, so a forcing capture is skipped on '
  'a stand pat that says nothing about a line the opponent has no choice in',
  ('        if (!gives_check) {\n'
   '          if (futility_value > futility_best) {',
   '        if (!gives_check || true) {\n'
   '          if (futility_value > futility_best) {'),
  origin="S112")

m("F05_qs_futility_ep_victim", S, "search/pruning",
  'the victim is read from the target square for en passant too, where the '
  'square is empty, so the capture is priced at nothing and skipped whenever '
  'alpha sits between the futility base and the base plus a pawn -- the '
  'published first bug of this very rule',
  ('      const piece_t victim =\n'
   '          MOVE_EN_PASSANT(moves[i])\n'
   '              ? ((game->board.active_color == WHITE) ? B_PAWN : W_PAWN)\n'
   '              : game->board.squares[MOVE_TO(moves[i])];',
   '      const piece_t victim = game->board.squares[MOVE_TO(moves[i])];'),
  origin="S112")

m("F06_qs_futility_fold_type", S, "search/transposition",
  'the fold raises the value but keeps the floor\'s claim, so a stand pat a '
  'lower bound raised stores a best case nobody searched as a lower bound -- a '
  'false "at least this" in the table, read back by every later probe',
  ('    best_value = futility_best;\n'
   '    value_type = TT_ALPHA_NODE;\n',
   '    best_value = futility_best;\n'),
  origin="S112")
