"""History scaling of the late move reduction, S098 verdict 1.

Six mutants over the three sites the term has: the helper that computes it
(`lmr_adjusted_reduction`), the gate that reads it (`lmr_depth_of`), and the
two reads in `negamax_at` -- the sum the term divides and the reduction it
adjusts. Nothing here moves a constant. A divisor a little larger is a
reduction that is tuned differently and `S127` is what decides that; what these
break is a sign, a clamp, a source or a guard, which is the class a test can be
red on without arguing about strength.

This is DEC-141 clause 2 for the rule that lands here: a new reduction rule
ships with a direct guard test and a mutant that test kills.

**The one thing this file cannot do is move the constants**, and that is worth
stating because the obvious mutant is not here. `LmrHistClamp` at 0 switches
the term off, and a release rebuild at that value is the S109 bisection
protocol rather than a mutant: the suite is *supposed* to stay green there
except for the six cases about the term, and the case that holds the off value
itself -- "the history term is inert at a sum of zero" -- is a property case no
mutant reddens.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. A mutant's several pairs are applied
together as one bug. `expected` is "killed" unless a person has argued the
mutant is behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: L is this file's own prefix and nothing else here uses
it. `origin` says which step wrote the mutant.
"""

S = "src/search.cpp"

m("L01_lmr_history_sign", S, "search/reduction",
  'the history term is added instead of subtracted, so the quiets the tables '
  'like are searched shallower and the ones they have written off deeper -- '
  'the slip the step file calls silent, whose only symptom is lost rating',
  ('  return lmr_reduction(depth, move_number) - shift;',
   '  return lmr_reduction(depth, move_number) + shift;'),
  origin="S098")

m("L02_lmr_history_no_clamp", S, "search/reduction",
  'the term loses its clamp, so a sum moves the reduction by as many plies as '
  'the division returns. The case that kills this one feeds the helper sums '
  'well past `LmrHistClamp * LmrHistDiv` and outside the band the tables can '
  'hold, because what the clamp guarantees is a bound for any input at any '
  'divisor -- and both of those numbers are seeds a lane and a refit will '
  'move',
  ('  if (shift > LMR_HIST_CLAMP) { shift = LMR_HIST_CLAMP; }\n'
   '  if (shift < -LMR_HIST_CLAMP) { shift = -LMR_HIST_CLAMP; }\n\n',
   ''),
  origin="S098")

m("L03_lmr_gate_unscaled", S, "search/pruning",
  "the shallow-depth block's gate goes back to the raw table while the "
  'reduction keeps the term, so S109\'s four rules price a quiet against a '
  'depth it is not searched at -- the half of the re-point that is invisible '
  'from the reduction the probe records',
  ('  const int left = depth - lmr_adjusted_reduction(depth, move_number, hist_sum);',
   '  const int left = depth - lmr_reduction(depth, move_number);\n'
   '  (void) hist_sum;'),
  origin="S098")

m("L04_lmr_reduction_unscaled", S, "search/reduction",
  'the reduction goes back to the raw table while the gate keeps the term, '
  'the other half of the same re-point: every rule of the block is then '
  'priced at a reduced depth the search does not use',
  ('        reduction = lmr_adjusted_reduction(\n'
   '            depth, static_cast<int>(legal_moves_counter), hist_sum);',
   '        reduction =\n'
   '            lmr_reduction(depth, static_cast<int>(legal_moves_counter));'),
  origin="S098")

m("L05_lmr_history_banded", S, "search/reduction",
  "the term divides score_move()'s ordering score instead of the raw history "
  'sum, so a killer\'s 900000 and a countermove\'s 700000 buy the whole clamp '
  'for a reason that is not history at all -- S093\'s band hazard, which '
  'history pruning reads the raw entry to avoid',
  ('        is_quiet ? quiet_history_sum(game, state, moves[i], prev_move)\n'
   '                 : NO_HISTORY_SUM;',
   '        is_quiet\n'
   '            ? score_move(game, state, moves[i], tt_move, ply, prev_move)\n'
   '            : NO_HISTORY_SUM;'),
  origin="S098")

m("L06_lmr_root", S, "search/reduction",
  'the root exemption is dropped, so the mating move at the root is searched '
  'shallower than the root is deep -- S013\'s own bug, which this step re-arms '
  'the guard against because the accepts asks for the assertion with its '
  'precondition',
  ('    const bool may_reduce = ply > 0 && depth >= 3 && legal_moves_counter > 3 &&\n'
   '                            !is_in_check && !is_check_move &&\n'
   '                            !capture_gives_check;',
   '    const bool may_reduce = depth >= 3 && legal_moves_counter > 3 &&\n'
   '                            !is_in_check && !is_check_move &&\n'
   '                            !capture_gives_check;'),
  origin="S098")
