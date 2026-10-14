"""The table's score as the node's estimate at the reverse-futility margin.
S234.

Four mutants over the one rule this step routes and the one switch it routes it
behind. The rule itself -- which entries may replace the static score, and with
what -- landed with S109 and feeds the futility margin; this step adds the
second consumer, so two of the four cut the rule and two cut what this step
built around it:

  the rule          G01, G03. The bound direction is the whole licence: a
                    lower bound may raise the input and never lower it, an
                    upper bound the reverse, and an inversion is silent -- the
                    engine still prunes, on a number the entry never certified.
                    G03 drops the mate-band exclusion, which no bound-direction
                    case can see, because a mate score passes the direction
                    test by being far above the static one.
  this step         G02, G04. G02 reads the switch the wrong way round, so the
                    candidate ships as the parent and the parent as the
                    candidate -- a step whose verdict is a switch cannot have
                    that untested. G04 writes the estimate into the entry's
                    evaluation field, which is the one mistake that makes the
                    substitution compound: the next node reads a pruning input
                    where a static evaluation belongs, S099's correction would
                    learn from a search score, and nothing crashes.

**G01 and G03 cut lines S109 wrote.** They belong here rather than in
tools/mutants/S109_shallow_pruning.py because that file's mutants were mined
against the futility site alone and this step is what gives the rule a second
consumer; the cases that kill them are this step's, and a later step that adds
a third consumer inherits both.

Nothing here moves a constant. `RfpTtEstimate` at 0 is the off value DEC-215
asks to be proved on the tree and is not a bug. What these break is a
comparison, a switch or where a value is written.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: G is this file's own prefix and nothing else in
tools/mutants/ uses it.
"""

S = "src/search.cpp"

m("G01_estimate_bound_direction", S, "search/pruning",
  'the tightening takes the bound types the wrong way round: a lower bound is '
  'taken only when it sits **below** the static score and an upper bound only '
  'when it sits above, which is the exact inversion of what certifies the '
  'substitution. Every margin in the engine then prunes against a number no '
  'entry ever claimed -- a lower bound that says "worth at least this" used to '
  'lower the input, an upper bound that says "worth at most this" used to '
  'raise it -- and nothing crashes, no bound is violated in a way a search '
  'can see, and the only symptom is rating',
  ('      if ((tt_entry->type == TT_BETA_NODE && tt_score > static_eval) ||\n'
   '          (tt_entry->type == TT_ALPHA_NODE && tt_score < static_eval)) {',
   '      if ((tt_entry->type == TT_BETA_NODE && tt_score < static_eval) ||\n'
   '          (tt_entry->type == TT_ALPHA_NODE && tt_score > static_eval)) {'),
  origin="S234")

m("G02_estimate_switch_inverted", S, "search/pruning",
  'the reverse-futility site reads the switch the wrong way round, so at the '
  'shipped default it decides on the static score and at the off value on the '
  'estimate. The candidate and its own reference are exchanged, which is the '
  'one failure a verdict carried by a switch cannot survive: the SPRT would '
  'measure the parent against the parent, read a zero, and the zero would be '
  'true',
  ('    const int rfp_eval = (RFP_TT_ESTIMATE != 0) ? pruning_eval : static_eval;',
   '    const int rfp_eval = (RFP_TT_ESTIMATE == 0) ? pruning_eval : static_eval;'),
  origin="S234")

m("G03_estimate_mate_band_dropped", S, "search/pruning",
  'the mate band is not excluded from the tightening, so an entry holding a '
  'mate score becomes the node\'s estimate. A mate score is a distance and not '
  'a value: the margin is then subtracted from a number in the wrong unit and '
  'the node fails high on a claim no search made here. Invisible to every '
  'bound-direction case, because a mate score passes the direction test by '
  'being far above the static one, and invisible to the guards on beta, which '
  'bound the window and not the entry',
  ('    if (tt_score < MATE_MIN && tt_score > -MATE_MIN) {',
   '    if (true) {  // G03: the mate band is not excluded.'),
  origin="S234")

m("G04_estimate_stored_as_eval", S, "search/pruning",
  'the node stores the estimate in the entry\'s evaluation field instead of '
  'the raw static score. The one mistake that makes the substitution compound: '
  'the next node to probe this position reads a pruning input where an '
  '`evaluate()` result belongs and tightens **that** again, the improving flag '
  'is left alone only because it reads the stack, and a correction table would '
  'learn the difference between a search score and itself (S099). No crash, no '
  'wrong bound, and the field is clamped to int16 on the way in so even a mate '
  'score arrives looking ordinary',
  ('    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,\n'
   '                   static_eval);',
   '    tt_store_entry(state->tt, &game->board, depth, to_store, type, best_move,\n'
   '                   pruning_eval);'),
  origin="S234")
