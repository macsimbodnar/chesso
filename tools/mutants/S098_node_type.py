"""Late move reduction by node type, S098 verdict 2: the prediction, the four
terms and the pair that is not a node type.

Nine mutants over one step's rules. Three classes, and each is a bug this
engine could plausibly ship:

  the alternation     a child labelled with another child's rule. Silent
                      everywhere -- no crash, no wrong node count, only rating,
                      which is why the rules are named functions and why both
                      the pure walk and the recorded site label exist
  a term inverted     the condition read the wrong way round. Every one of the
                      four has a published direction and three of them have a
                      published *wrong* direction that some engine measured
                      negative, so this is the shape the bug takes
  the pair            (is_pv && cut_node) produced at a site, which is what the
                      Debug assert in negamax_at is for. `assert` is dead in
                      both gated builds, so the kill below is the Release
                      suite's and the assert is the second net -- the Debug
                      self-play of DEC-141 clause 1 is where it fires

This is DEC-141 clause 2 for verdict 2's rules: a new reduction rule ships with
a direct guard test and a mutant that test kills. Every anchor here is a guard
or a sign, never a constant moved -- moving `LmrCutNode` to 0 is the off value
the bisection protocol uses and not a bug.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. A mutant's several pairs are applied
together as one bug. `expected` is "killed" unless a person has argued the
mutant is behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here, and this file opens the `T` prefix because `N` is S191's.

**Five anchors were re-cut by S236 and no mutant here changed meaning.** That
step put the reduction in fixed point: the four terms are summed as ticks of a
ply (`LMR_CUTNODE * LMR_SCALE` and so on) and `lmr_adjusted_reduction` rounds
the sum once, so the five lines T02 to T06 anchored on no longer exist
verbatim. **T06's was re-cut a second time when that step's history term left
the tree** (DEC-231) and the helper lost its third argument: same bug, same id,
the line as it reads now. Each pair below names the line as it reads now and inverts, drops or
flips exactly what it inverted, dropped or flipped before -- an anchor is a
coordinate, and a coordinate that has moved is re-read rather than retired. The
alternative, retiring the ids and opening new ones, would have lost the kills
these five are on record for.
"""

S = "src/search.cpp"

m("T01_first_child_label", S, "search/reduction",
  'the first child of an ALL node is labelled ALL instead of CUT, so the '
  'alternation stops alternating and a whole subtree is predicted to fail low '
  'when every published rule says it will not',
  ('static constexpr child_label_t first_child(bool is_pv, bool cut_node)\n'
   '{ return {is_pv, !is_pv && !cut_node}; }',
   'static constexpr child_label_t first_child(bool is_pv, bool cut_node)\n'
   '{ return {is_pv, !is_pv && cut_node}; }'),
  origin="S098")

m("T02_cutnode_inverted", S, "search/reduction",
  'the cut-node ply is added at every node that is **not** predicted to fail '
  'high, which is the inverse of the published adjustment and reduces hardest '
  'exactly where the node is expected to have to look at everything',
  ('  if (cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }',
   '  if (!cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }'),
  origin="S098")

m("T03_improving_inverted", S, "search/reduction",
  'the ply is added when the side to move **is** improving -- the direction '
  'the published record tried first and closed, against the asymmetry it then '
  'shipped',
  ('  if (!improving) { adjustment += LMR_NOT_IMPROVING * LMR_SCALE; }',
   '  if (improving) { adjustment += LMR_NOT_IMPROVING * LMR_SCALE; }'),
  origin="S098")

m("T04_ttcapture_inverted", S, "search/reduction",
  "the ply is added where the entry's move is **not** a capture, which "
  'includes every node the table has nothing for at all -- so the term fires '
  'on the majority of nodes and means nothing about any of them',
  ('  if (tt_move_is_capture) { adjustment += LMR_TT_CAPTURE * LMR_SCALE; }',
   '  if (!tt_move_is_capture) { adjustment += LMR_TT_CAPTURE * LMR_SCALE; }'),
  origin="S098")

m("T05_pv_added", S, "search/reduction",
  'the PV term is added instead of subtracted, so the lines that get reported '
  'and played are the ones searched shallowest. A sign slip with no symptom '
  'but rating, which is the class S093 and S109 both left a comment against',
  ('  if (is_pv) { adjustment -= LMR_PV * LMR_SCALE; }',
   '  if (is_pv) { adjustment += LMR_PV * LMR_SCALE; }'),
  origin="S098")

m("T06_adjusted_reduction_ignores_node", S, "search/reduction",
  'the shared helper drops the adjustment, so both consumers -- the reduction '
  "and S109's shallow-depth gate -- read the raw table again and all four "
  'terms reach nothing. The step wired up and switched off in one line. The '
  '`(void)` orphans the parameter, which the Release build this tool runs '
  "refuses under -Werror; search.py's header carries the reason in full",
  ('  return lmr_plies_of(lmr_reduction_ticks(depth, move_number) +\n                      node_adjustment);',
   '  (void) node_adjustment;\n  return lmr_plies_of(lmr_reduction_ticks(depth, move_number));'),
  origin="S098")

m("T07_site_first_child", S, "search/reduction",
  'the recursion labels its first child by the scout rule, so a PV node scouts '
  'its own principal variation as a CUT node. The rules as functions are still '
  'right and only the site that applies them is wrong, which is why the walk '
  'alone would not see it',
  ('    const child_label_t child = (legal_moves_counter == 1)\n'
   '                                    ? first_child(is_pv, cut_node)\n'
   '                                    : scouted_child(is_pv, cut_node);',
   '    const child_label_t child = scouted_child(is_pv, cut_node);'),
  origin="S098")

m("T08_null_child_label", S, "search/reduction",
  "the child after a null move keeps the parent's own type instead of the "
  "opposite one, which is Garms's unconditional CUT reading applied where "
  "Kannan's paragraph and the window both say ALL",
  ('static constexpr child_label_t null_move_child(bool, bool cut_node)\n'
   '{ return {false, !cut_node}; }',
   'static constexpr child_label_t null_move_child(bool, bool cut_node)\n'
   '{ return {false, cut_node}; }'),
  origin="S098")

m("T09_pv_and_cut_together", S, "search/reduction",
  'the full-window re-search labels its child both PV and CUT, which is the '
  'pair that names no node type and the one negamax_at asserts against. The '
  'assert is dead in both gated builds, so the Release kill is the walk case '
  "and the Debug binary's abort under DEC-141's self-play is the second net",
  ('static constexpr child_label_t full_window_research(bool is_pv, bool)\n'
   '{ return {is_pv, false}; }',
   'static constexpr child_label_t full_window_research(bool is_pv, bool)\n'
   '{ return {is_pv, true}; }'),
  origin="S098")
