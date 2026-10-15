"""Evaluation mutants: the side-to-move sign, capture ordering and the clamp
S039 retired.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver. `old` must occur exactly once in `file` --
an ambiguous anchor mutates a site nobody chose, and the tool refuses the whole
run before it writes anything. A mutant's several pairs are applied together as
one bug. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here. `origin` says which step or review wrote it.

**S039, the lazy shortcut and the clamp retired together (DEC-258).** M29's
anchor was re-read, not retired: `evaluate_expensive()` returns the unclamped
`stage_two` where it returned `bounded`, and the bug -- the stage's sign
flipped for the side to move -- is the same one on the same line.
`M31_lazy_bound_no_margin` left with the function it broke: it took the
shortcut in `evaluate_lazy()` without its margin, and there is no
`evaluate_lazy()` any more, so no tree this file can be run against has that
bug. Its id is not reused. M41 is the bug the step itself could regress into:
the clamp put back.
"""

E = "src/evaluation.cpp"

m("M29_eval_expensive_sign", E, "eval",
  'mobility/king-safety sign flipped for the side to move',
  ('return (board->active_color == WHITE) ? stage_two : -stage_two;',
   'return (board->active_color == WHITE) ? -stage_two : stage_two;'),
  origin="2026-09-04_test_review")

m("M30_mvv_lva_sign", E, "ordering",
  'MVV-LVA adds the attacker value instead of subtracting',
  ('return piece_values_abs[victim] - piece_values_abs[MOVE_PIECE(move)];',
   'return piece_values_abs[victim] + piece_values_abs[MOVE_PIECE(move)];'),
  origin="2026-09-04_test_review")

m("M41_eval_clamp_restored", E, "eval",
  'mobility plus king safety clamped again at the 184 S039 retired: evaluate() '
  'truncates the expensive stage past it, the cap that kept a king safety term '
  'from saying more than the margin in the real score',
  ('  return (board->active_color == WHITE) ? stage_two : -stage_two;',
   '  const int bounded = std::clamp(stage_two, -184, 184);\n'
   '  return (board->active_color == WHITE) ? bounded : -bounded;'),
  origin="S039")
