"""The late move reduction accumulated in fixed point, and the history term as
a fraction of a ply. S236.

Six mutants over three parts, and the parts are what the step's own bisection
splits on, so each mutant belongs to one of them and no mutant breaks two at
once:

  the unit          W02, W06. The table holds ticks and the node terms are
                    scaled into the same ticks. A build where the two disagree
                    is the failure this step can have that nothing else in the
                    suite would see: every probe still returns a number, every
                    older case still passes, and the engine simply reduces by
                    the wrong amount. W02 halves the table against the rounding
                    divide; W06 leaves the five node terms in plies, where each
                    is then worth a thousandth of the ply it names.
  the rounding      W01. The bias is the rule, not a coefficient of it, so
                    dropping it is not "rounding slightly differently" -- it is
                    the parent's truncation shipped under a parameter that
                    reports success and changes nothing, which is exactly the
                    shape DEC-215 asks an off value to be proved against.
  the term          W03, W04, W05. The clamp is the whole term's bound, the
                    sign decides whether history reduces the moves it likes or
                    the moves it has written off, and W05 is the interesting
                    one: it is S098 verdict 1's own shape, a whole-ply
                    quotient, re-expressed in ticks. That verdict measured zero
                    twice (DEC-213) and this step's hypothesis is that the unit
                    is why, so a suite that cannot tell the two forms apart
                    cannot tell whether this step did anything.

Nothing here moves a constant. `LmrHistClamp` at 0 is the off value DEC-215
asks to be proved on the tree and is not a bug; `LmrRoundBias` at 0 is the
parent's own rounding and is the same; `LmrHistDiv` and `LmrHistClamp` at their
census seeds are values a later SPSA lane may move. What these break is an
expression or its unit.

**W03, W04 and W05 need the term switched on to be killable at all.** At
`LmrHistClamp` 0 the helper returns 0 for every sum, which makes all three
equivalent mutants by construction -- so this file is only meaningful on a tree
whose clamp is the census seed, and the three cases that kill them assert that
clamp is non-zero before they assert anything else.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: W is this file's own prefix and nothing else in
tools/mutants/ uses it.
"""

S = "src/search.cpp"

m("W01_round_bias_dropped", S, "search/reduction",
  'the rounding bias is not added before the shift, so the accumulator floors '
  'at every setting of LmrRoundBias and the parameter decides nothing. '
  '**EQUIVALENT AT THE SHIPPED DEFAULT, AND THAT IS AN ARGUMENT AND NOT A '
  'SURVIVOR.** LmrRoundBias ships at 0, because with the history term live '
  'every non-zero bias loses the mate row tests/test_search.cpp "pruning does '
  'not hide a forced mate" guards -- 1, 8, 64, 128, 256 and 512 ticks were '
  'each built and run, and S236s step file records all six. At 0 the term '
  'this mutant deletes is provably zero, so the mutated engine is the same '
  'engine: same tree, same bench signature, same suite. No Release case can '
  'kill it and none is owed. The rule is guarded where it can be set, in the '
  'tune build: tests/test_search_params.cpp "the rounding bias moves the '
  'boundary it is the rule for" drives four biases across the range and was '
  'observed red under this exact cut by hand -- the S033 protocol, log '
  '.tuning/coord/S236_W01_red.log. A shipped bias above 0 makes this mutant '
  'killable again and the declaration below has to come back out',
  ('{ return (ticks + LMR_ROUND_BIAS) >> LMR_SCALE_SHIFT; }',
   '{ return ticks >> LMR_SCALE_SHIFT; }'),
  expected="equivalent",
  origin="S236")

m("W02_table_unit_halved", S, "search/reduction",
  'the table is built at half the scale the sum is divided by, so every '
  'reduction the table contributes is halved while the node terms and the '
  'history term stay whole. Self-consistent from the probes -- a case that '
  'reads the table through one probe and the sum through another cannot see '
  'it -- which is why the unit is pinned against build_lmr_table\'s own '
  'formula and against the parent\'s line at the off configuration',
  ('      table[depth][move_number] = static_cast<int32_t>(r * LMR_SCALE);',
   '      table[depth][move_number] = static_cast<int32_t>(r * (LMR_SCALE / 2));'),
  origin="S236")

m("W03_hist_clamp_dropped", S, "search/reduction",
  'the clamp is not applied, so a saturated history sum replaces the table\'s '
  'estimate instead of adjusting it: at the shipped divisor the band\'s own '
  'edge is worth six plies, which is more reduction than the table returns '
  'anywhere this engine searches',
  ('  if (ticks > LMR_HIST_CLAMP) { ticks = LMR_HIST_CLAMP; }\n'
   '  if (ticks < -LMR_HIST_CLAMP) { ticks = -LMR_HIST_CLAMP; }\n',
   '  // W03: the clamp is not applied.\n'),
  origin="S236")

m("W04_hist_sign_flipped", S, "search/reduction",
  'the term is added where it is subtracted, so the quiets the history tables '
  'have endorsed are the ones reduced hardest and the ones they have written '
  'off are searched deepest. The exact inversion of the rule, and silent: no '
  'crash, no wrong node count, only rating',
  ('                      node_adjustment - lmr_history_ticks(hist_sum));',
   '                      node_adjustment + lmr_history_ticks(hist_sum));'),
  origin="S236")

m("W05_hist_whole_plies", S, "search/reduction",
  'the term is computed as a whole-ply quotient and then scaled, which is '
  'S098 verdict 1\'s own shape in this step\'s unit: a sum under the divisor '
  'contributes nothing at all and the fraction this step exists to express is '
  'gone. The mutant that decides whether this step\'s suite can tell the two '
  'forms apart',
  ('  int ticks = (hist_sum * LMR_SCALE) / LMR_HIST_DIV;',
   '  int ticks = (hist_sum / LMR_HIST_DIV) * LMR_SCALE;'),
  origin="S236")

m("W06_node_terms_unscaled", S, "search/reduction",
  'the first of the five node terms is summed as a whole ply into a tick '
  'accumulator, so LmrCutNode is worth a thousandth of the ply it names and a '
  'cut node reduces its late quiets by nothing. One term rather than all five '
  'on purpose: the mutant is the mistake an implementer makes at one line, and '
  'a case that sums the five would catch it only if it reads them apart',
  ('  if (cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }',
   '  if (cut_node) { adjustment += LMR_CUTNODE; }'),
  origin="S236")
