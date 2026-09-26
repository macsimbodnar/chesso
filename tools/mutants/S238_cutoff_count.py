"""Cutoff count, S238.

Seven mutants over the rule's three parts -- the comparison and its term, the
count's lifetime, and which slot is read when -- the last class being the
call-site one S237's fast check found no mutant for there.

  the term       Q01, Q02. The threshold test inverted, so the rule fires on
                 the nodes whose children have failed high least; and the
                 adjustment subtracted, so the nodes it fires on reduce less.
  the lifetime   Q03, Q06, Q07. The slot never cleared, so a node reads the
                 count its predecessor at the same ply left behind; cleared
                 before every move, so the count read is always 0, the clear
                 landing before the read with no child between; and S097's
                 verification counted as one of the parent's children.
  the call site  Q04, Q05. The count read from the grandchildren's slot, the
                 most recent child's own children, instead of this node's
                 children; and read once when the move loop starts, before any
                 child has returned, instead of at each move.

Nothing here moves a constant: `CutoffCountReduction` at 0 is the off value
DEC-215 asks to be proved on the tree, not a bug.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed" unless a
person has argued the mutant is behaviourally equivalent.

Ids are never reused: Q is this file's own prefix and nothing in
tools/mutants/ or its history uses it.
"""

S = "src/search.cpp"

m("Q01_threshold_inverted", S, "search/reduction",
  'the threshold test is inverted: the reduction rises at the nodes whose '
  'children have failed high **at most** the threshold -- the ones whose '
  'later moves are most likely still worth depth -- and the nodes the rule is '
  'about keep the table\'s number',
  ('{ return (count > CUTOFF_COUNT_THRESHOLD) ? CUTOFF_COUNT_REDUCTION : 0; }',
   '{ return (count <= CUTOFF_COUNT_THRESHOLD) ? CUTOFF_COUNT_REDUCTION : 0; }'),
  origin="S238")

m("Q02_wrong_sign", S, "search/reduction",
  'the adjustment is applied with the wrong sign: over the threshold the late '
  'quiets are reduced **less**, spending depth exactly where the rule says it '
  'is least worth spending',
  ('{ return (count > CUTOFF_COUNT_THRESHOLD) ? CUTOFF_COUNT_REDUCTION : 0; }',
   '{ return (count > CUTOFF_COUNT_THRESHOLD) ? -CUTOFF_COUNT_REDUCTION : 0; }'),
  origin="S238")

m("Q03_not_cleared", S, "search/reduction",
  'the children\'s slot is never cleared: a node starts from whatever count '
  'the last node at its ply left there, so the rule fires on siblings of a '
  'node that failed high elsewhere in the tree',
  ('  state->cutoff_counts[ply + 1] = 0;\n',
   '\n'),
  origin="S238")

m("Q04_wrong_child", S, "search/reduction",
  'the count is read from the grandchildren\'s slot, `ply + 2`: the cutoffs '
  'the most recently searched child counted among **its** children, rather '
  'than how many of this node\'s children failed high. Plausible as a reading '
  'of "the child\'s count" and wrong for this rule',
  ('            cutoff_count_ticks(state->cutoff_counts[ply + 1]);',
   '            cutoff_count_ticks(state->cutoff_counts[ply + 2]);'),
  origin="S238")

m("Q05_read_at_loop_entry", S, "search/reduction",
  'the count is read once when the move loop starts rather than at each '
  'move: the snapshot is taken before any child has returned, so it is '
  'always the empty count and the rule never fires',
  ('  state->cutoff_counts[ply + 1] = 0;\n',
   '  state->cutoff_counts[ply + 1] = 0;\n'
   '  const int cutoffs_at_loop_entry = state->cutoff_counts[ply + 1];\n'),
  ('            cutoff_count_ticks(state->cutoff_counts[ply + 1]);',
   '            cutoff_count_ticks(cutoffs_at_loop_entry);'),
  origin="S238")

m("Q06_cleared_per_move", S, "search/reduction",
  'the slot is cleared before every move instead of once per node; the '
  'clear sits at pick_next_move, before this move\'s reduction is read and '
  'with no search between, so the count read is always 0 and the rule never '
  'fires -- near-equivalent to Q05 by another route',
  ('    pick_next_move(moves, scores, moves_count, i);\n',
   '    pick_next_move(moves, scores, moves_count, i);\n'
   '    state->cutoff_counts[ply + 1] = 0;\n'),
  origin="S238")

m("Q07_verification_counted", S, "search/reduction",
  'S097\'s verification search counts its own cutoff in the parent\'s slot, '
  'so the parent sees one more child failing high than it searched whenever '
  'a verification under it fails high',
  ('      if (excluded_move == 0) { state->cutoff_counts[ply]++; }',
   '      state->cutoff_counts[ply]++;'),
  origin="S238")
