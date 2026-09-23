"""One more ply of reduction where the table entry carries no move, S095.

Three mutants over one rule, and the rule is one comparison and one addend, so
the three are everything that can plausibly go wrong with it:

  the condition inverted   the ply lands on the nodes an earlier search has
                           already resolved and nowhere else, which is the
                           inverse of the published condition and the shape a
                           guard bug takes here
  the term dropped         the input is computed, passed and read by nothing --
                           the step wired up and switched off in one line, which
                           is how S098's own T06 is written and for the same
                           reason
  the condition narrowed   the site asks whether there is an **entry** rather
                           than whether there is a **move**. That is the
                           narrower of the two published conditions and it is
                           not a typo class: it is the form one record
                           introduced and later widened, and here it would
                           silently exempt every entry quiescence wrote -- which
                           since S094 is the only kind of moveless entry this
                           engine produces, and the whole class the rule is
                           about

Nothing here moves a constant. `LmrNoTtMove` at 0 is the off value DEC-215 asks
to be proved on the tree and is not a bug, and at 2 it is the top of a declared
range a later SPSA lane may choose; what these break is a guard or its variable.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: J is this file's own prefix and nothing else in
tools/mutants/ uses it.

**Both anchors were re-cut by S236 and neither mutant changed meaning.** That
step sums the node terms as ticks of a ply, so the line these two anchor on
reads `LMR_NO_TT_MOVE * LMR_SCALE` now. The inversion and the drop are the same
two bugs on the same site; an anchor is a coordinate, and one that has moved is
re-read rather than retired.
"""

S = "src/search.cpp"

m("J01_no_tt_move_inverted", S, "search/reduction",
  'the extra ply is added where the entry **does** carry a move, so the rule '
  'reduces hardest exactly at the nodes an earlier search already resolved and '
  'not at all at the ones nothing has looked at -- the inverse of the '
  'published condition, and silent: no crash, no wrong node count, only rating',
  ('  if (no_tt_move) { adjustment += LMR_NO_TT_MOVE * LMR_SCALE; }',
   '  if (!no_tt_move) { adjustment += LMR_NO_TT_MOVE * LMR_SCALE; }'),
  origin="S095")

m("J02_no_tt_move_dropped", S, "search/reduction",
  'the term is dropped, so the condition is computed at every node, passed '
  'into the adjustment and read by nothing. The `(void)` is not decoration: '
  "S095 made `no_tt_move` a parameter of this function, so a mutant that "
  'deletes its only use orphans it and -Werror=unused-parameter refuses the '
  "Release build this tool runs -- which reads as `stillborn` and proves "
  "nothing. search.py's header carries the reason in full",
  ('  if (no_tt_move) { adjustment += LMR_NO_TT_MOVE * LMR_SCALE; }',
   '  (void) no_tt_move;'),
  origin="S095")

m("J03_site_entry_absent_only", S, "search/reduction",
  'the site asks whether the node has an **entry** instead of whether it has a '
  '**move**, which is the narrower of the two published conditions: every '
  'entry quiescence wrote without a move then counts as a resolved node and '
  'keeps its ply. Since S094 that is the only kind of moveless entry this '
  'engine writes, so the mutant removes the rule from the exact class it is '
  'about while leaving it firing everywhere the table is simply empty',
  ('  const bool no_tt_move = tt_move == 0;',
   '  const bool no_tt_move = tt_entry == nullptr;'),
  origin="S095")
