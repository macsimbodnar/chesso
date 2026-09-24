"""The late move reduction accumulated in fixed point. S236, and what is left
of it after DEC-231.

**Three mutants over two parts, and three more left with the term.** S236
shipped six: three for the accumulator's unit and its rounding, three for the
history term it added. That term's two verdicts read a walk and then a zero and
it was removed (DEC-231), so W03 (its clamp), W04 (its sign) and W05 (its
whole-ply form) went with the code they broke -- a mutant whose anchor is not in
`src/` is a mutant nobody can run, and keeping one as a comment would be a
guard that guards nothing. What remains is the unit, which stayed because it
plays as the whole-ply engine does:

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
                    the whole-ply engine's truncation shipped under a parameter
                    that reports success and changes nothing, which is exactly
                    the shape DEC-215 asks an off value to be proved against.

Nothing here moves a constant. `LmrRoundBias` at 0 is the whole-ply engine's
own rounding and is the off value DEC-215 asks to be proved on the tree, not a
bug. What these break is an expression or its unit.

**W01 is a declared equivalent and stays one while `LmrRoundBias` ships at 0.**
Its own entry carries the argument in full; the short form is that at a bias of
0 the term it deletes is provably zero, so the mutated engine is the same
engine, and the rule is guarded in the tune build instead.

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

m("W06_node_terms_unscaled", S, "search/reduction",
  'the first of the five node terms is summed as a whole ply into a tick '
  'accumulator, so LmrCutNode is worth a thousandth of the ply it names and a '
  'cut node reduces its late quiets by nothing. One term rather than all five '
  'on purpose: the mutant is the mistake an implementer makes at one line, and '
  'a case that sums the five would catch it only if it reads them apart',
  ('  if (cut_node) { adjustment += LMR_CUTNODE * LMR_SCALE; }',
   '  if (cut_node) { adjustment += LMR_CUTNODE; }'),
  origin="S236")
