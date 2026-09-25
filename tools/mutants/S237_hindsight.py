"""Hindsight reductions, S237.

Four mutants over the rule's two branches and the two things around them that
fail silently: the bound that keeps a correction from compounding, and the side
the evaluation delta is read from.

  the branches   Y01, Y02. Each inverts the direction of the evaluation test in
                 one branch, so a heavily reduced move gets its ply back when
                 the mover's evaluation improved rather than when it got worse,
                 and a lightly reduced one gives its ply up when the evaluation
                 got worse. The engine still corrects depths on about the same
                 number of nodes; it corrects the wrong ones.
  the bound      Y03. A node that corrected its own depth hands its children
                 the reduction it took anyway, so a corrected depth feeds the
                 same rule one ply down and corrections chain.
  the side       Y04. The delta is read from the side to move at the child
                 rather than from the mover's, which flips both branches at
                 once -- the parent's slot is the mover's view and the child's
                 is the opponent's, and only their negated sum is the mover's
                 delta.

Nothing here moves a constant: the two reduction thresholds at 126 and 0 are
the off values DEC-215 asks to be proved on the tree, not bugs.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed" unless a
person has argued the mutant is behaviourally equivalent.

Ids are never reused: Y is this file's own prefix and nothing in
tools/mutants/ or its history uses it.
"""

S = "src/search.cpp"

m("Y01_give_back_inverted", S, "search/reduction",
  'the give-back branch tests the evaluation the wrong way round: a heavily '
  'reduced move gets its ply back when the mover\'s evaluation **improved** '
  'past the margin instead of when it got worse. The rule still fires on '
  'about as many nodes, so node counts move without looking wrong; it spends '
  'the plies on the moves that least need them',
  ('          mover_delta < -HINDSIGHT_WORSE_MARGIN) {',
   '          mover_delta > HINDSIGHT_WORSE_MARGIN) {'),
  origin="S237")

m("Y02_give_up_inverted", S, "search/reduction",
  'the give-up branch tests the evaluation the wrong way round: a lightly '
  'reduced move gives a ply up when the mover\'s evaluation got **worse** '
  'past the margin instead of when it improved -- the move that most needs '
  'its depth loses one',
  ('                 mover_delta > HINDSIGHT_BETTER_MARGIN && depth >= 2) {',
   '                 mover_delta < -HINDSIGHT_BETTER_MARGIN && depth >= 2) {'),
  origin="S237")

m("Y03_bound_dropped", S, "search/reduction",
  'a node that corrected its own depth still hands its children the '
  'reduction it took, so a depth this rule moved is the input to the same '
  'rule one ply down and corrections compound along a line. Nothing crashes '
  'and no bound is violated; the tree simply drifts',
  ('    const int handed_reduction = (hindsight != 0) ? 0 : reduction;',
   '    const int handed_reduction = reduction;'),
  origin="S237")

m("Y04_delta_wrong_side", S, "search/reduction",
  'the delta is read from the child\'s side instead of the mover\'s: the sum '
  'of the two slots rather than its negation. Both branches flip together -- '
  'the give-back fires on improvements and the give-up on worsenings -- and '
  'the error is invisible in a count of how often the rule fires',
  ('      const int mover_delta = -static_eval - parent_eval;',
   '      const int mover_delta = static_eval + parent_eval;'),
  origin="S237")
