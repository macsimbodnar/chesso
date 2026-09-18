"""One-ply continuation history and its own scale, S222.

Four mutants over the three sites the table has: the write in
`history_on_quiet_cutoff`, the read in `score_move`, and the one call site that
has to say a node has no previous move to index -- the null-move child in
`negamax_at`. Two of them are S024's, re-established for code that no longer
shares plain history's constants; the other two are the read side and the null
boundary, which S024 left to a structural argument.

Nothing here moves a constant. A table whose bonus is a little smaller is a
table that is tuned differently, and the lane in `tools/spsa_s222.json` is what
decides that; what these break is a guard, a sign or an index, which is the
class a test can be red on without arguing about strength.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: H is this file's own prefix and nothing else here uses
it. `origin` says which step wrote the mutant.
"""

S = "src/search.cpp"
E = "src/evaluation.hpp"

m("H01_cont_hist_malus_sign", S, "search/ordering",
  'the continuation malus is credited instead of charged, so every quiet the '
  'node tried and rejected is promoted in the table that orders its siblings',
  ('          continuation_entry(state, prev_move, quiets_tried[i]), '
   '-cont_malus,',
   '          continuation_entry(state, prev_move, quiets_tried[i]), '
   'cont_malus,'),
  origin="S222")

m("H02_cont_hist_no_prev_guard", S, "search/ordering",
  'the guard on there being a previous move is dropped, so the root ply and '
  'the node after a null move write the (W_PAWN, a8) cell move 0 decodes to '
  '-- a wrong-cell write with no crash and no sanitizer finding',
  ('  const bool has_prev = prev_move != 0;',
   '  const bool has_prev = true;'),
  origin="S222")

# The anchor gained two arguments at S098 verdict 2, which threads `cut_node`
# through `negamax_at` and labels the null-move child by Kannan's rule rather
# than passing a literal `false`, and a third at S231, which threads the move
# two plies back and re-wrapped the call. The mutation is unchanged through all
# of it -- the `0` in the previous-move position becomes `prev_move` and
# nothing else -- and it was re-observed at each step. Note that the `0` this
# anchor names is the **previous move**, the seventh argument; S231's own
# `I03_null_child_drops_prev2` is about the tenth and they are different
# mutants of the same call.
m("H03_null_child_keeps_prev", S, "search/ordering",
  'the null-move child is handed the node\'s own previous move instead of 0, '
  'so everything it writes is keyed on a move that is two plies back and on '
  'the wrong side of the pass',
  ('        -beta, -beta + 1, depth - 1 - reduction, ply + 1, game, state, 0,\n'
   '        child.is_pv, child.cut_node, prev_move);',
   '        -beta, -beta + 1, depth - 1 - reduction, ply + 1, game, state,\n'
   '        prev_move, child.is_pv, child.cut_node, prev_move);'),
  origin="S222")

m("H04_cont_hist_unread", E, "search/ordering",
  'the quiet history sum stops adding the continuation term, so the table is '
  'written at every cutoff and orders nothing -- the shape a census would call '
  'exercised and a verdict would call inert. The anchor moved out of '
  'score_move and out of the .cpp at S098 verdict 1, which factored the sum '
  'into quiet_history_sum in src/evaluation.hpp for a second reader, the '
  'history-scaled reduction, that measured zero and left again (DEC-213); '
  'the factoring stayed, score_move is its one production reader, and the '
  'mutant takes the term away from it there',
  ('  if (prev_move != 0) {\n'
   '    score +=\n'
   '        (CONT_HIST_WEIGHT * continuation_entry(state, prev_move, move)) '
   '/ 100;\n'
   '  }\n'
   '\n',
   ''),
  origin="S222")
