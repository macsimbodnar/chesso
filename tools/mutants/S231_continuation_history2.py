"""Two-ply continuation history and its own scale, S231.

Four mutants over the four sites the second table has: the write in
`history_on_quiet_cutoff`, the read in `quiet_history_sum`, the guard that says
a node has no move two plies back, and the one recursion that has to hand a
child its parent's previous move rather than its own key -- the plumbing that
makes the table a two-ply table at all. They are the shape of
`tools/mutants/S222_continuation_history.py`'s four, one ply lower, and the two
files are deliberately parallel: the second table is the first table's
mechanism at a different key, so a defect class that exists for one exists for
the other.

Nothing here moves a constant. A table whose bonus is a little smaller is a
table that is tuned differently, and the lane in `tools/spsa_s231.json` is what
decides that; what these break is a guard, a sign or an index, which is the
class a test can be red on without arguing about strength.

The failure this file is really written for is that **0 is a legitimate cell**.
Ply 0, ply 1 and the node two plies after a null move all pass 0 as the key,
and 0 decodes to (W_PAWN, a8) rather than to an out-of-range index -- so a
dropped guard is a silent wrong-cell write with no crash and no sanitizer
finding, and only a case that scans a table or a row can see it.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: I is this file's own prefix and nothing else in
tools/mutants/ uses it.
"""

S = "src/search.cpp"
E = "src/evaluation.hpp"

m("I01_cont_hist2_malus_sign", S, "search/ordering",
  'the two-ply continuation malus is credited instead of charged, so every '
  'quiet the node tried and rejected is promoted in the table that orders its '
  'siblings two plies later',
  ('          continuation2_entry(state, prev_move2, quiets_tried[i]), '
   '-cont2_malus,\n'
   '          CONT_HIST_BOUND);',
   '          continuation2_entry(state, prev_move2, quiets_tried[i]), '
   'cont2_malus,\n'
   '          CONT_HIST_BOUND);'),
  origin="S231")

m("I02_cont_hist2_no_prev2_guard", S, "search/ordering",
  'the guard on there being a move two plies back is dropped, so ply 0, ply 1 '
  'and every node two plies after a null move write the (W_PAWN, a8) cell 0 '
  'decodes to -- a wrong-cell write with no crash and no sanitizer finding',
  ('  const bool has_prev2 = prev_move2 != 0;',
   '  const bool has_prev2 = true;'),
  origin="S231")

m("I03_null_child_drops_prev2", S, "search/ordering",
  'the null-move child is handed 0 as its two-ply key instead of the move '
  'before the pass, so the one node whose parity the pass preserves stops '
  'writing the table -- the opposite reading of the null move to the one S231 '
  'took, and a silent one: nothing crashes, the table is merely a little '
  'emptier',
  ('        child.is_pv, child.cut_node, prev_move);\n'
   '\n'
   '    unmake_null_move(game);',
   '        child.is_pv, child.cut_node, 0);\n'
   '\n'
   '    unmake_null_move(game);'),
  origin="S231")

m("I04_cont_hist2_unread", E, "search/ordering",
  'the quiet history sum stops adding the two-ply term, so the second table is '
  'written at every cutoff and orders nothing -- the shape a census would call '
  'exercised and a verdict would call inert, which is exactly the reading '
  'DEC-194 had to take a census to rule out for the first table',
  ('  if (prev_move2 != 0) {\n'
   '    score +=\n'
   '        (CONT_HIST2_WEIGHT * continuation2_entry(state, prev_move2, move)) '
   '/\n'
   '        100;\n'
   '  }\n\n',
   ''),
  origin="S231")
