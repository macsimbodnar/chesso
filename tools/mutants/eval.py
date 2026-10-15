"""Evaluation mutants: the side-to-move sign, capture ordering and the lazy
shortcut's margin.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.
"""

E = "src/evaluation.cpp"

m("M29_eval_expensive_sign", E, "eval",
  'mobility/king-safety sign flipped for the side to move',
  ('return (board->active_color == WHITE) ? bounded : -bounded;',
   'return (board->active_color == WHITE) ? -bounded : bounded;'),
  origin="2026-09-04_test_review")

m("M30_mvv_lva_sign", E, "ordering",
  'MVV-LVA adds the attacker value instead of subtracting',
  ('return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];',
   'return piece_values_abs[victim] + piece_values_abs[MOVE_PIECE(move)];'),
  origin="2026-09-04_test_review")

m("M31_lazy_bound_no_margin", E, "eval/lazy",
  'lazy shortcut taken without its margin',
  ('if (cheap - LAZY_EVAL_MARGIN >= beta) { return cheap - LAZY_EVAL_MARGIN; }',
   'if (cheap >= beta) { return cheap - LAZY_EVAL_MARGIN; }'),
  origin="2026-09-04_test_review")
