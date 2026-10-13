"""Quiescence's limit check above its table probe, S279.

One mutant, the tree before this step: quiescence counts its node, probes the
table, and checks the limits only below the probe, so a table cutoff returns
unchecked. A cutoff that spends the node budget lets the search run on to the
next node, or the next several, and one that lands on the stop flag's poll
skips the poll. It is killed by two cases in tests/test_search.cpp's
"search: budgets" suite:

  NB01  the check back below the probe   "a table cutoff that spends the node
                                         budget stops the search" (budget 2
                                         explored 3) and "every node budget
                                         stops on its own node" (prepared
                                         tree, budget 3 explored 4)

It leaves the bench signature where it was: `bench` sets no budget and never
raises the stop flag, so no check ever fires there and moving one changes no
node, which is the step's own INV-6 reading in reverse.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: NB is this file's own prefix, and nothing on any branch
has used it (`git log --all -p -- tools/mutants adocs/data`).
"""

S = "src/search.cpp"

m("NB01_qs_limit_check_below_probe", S, "search/quiescence",
  'quiescence checks the limits only after its table probe, as before S279: '
  'a table cutoff that counts the node spending the budget returns without '
  'aborting, and the search goes on past the budget',
  ('  // in play. S279.\n'
   '  if (check_limits(state)) { return 0; }\n',
   '  // in play. S279.\n'),
  ('  if (ply + 1 >= MAX_PLY) { return stand_pat; }\n',
   '  if (check_limits(state)) { return stand_pat; }\n'
   '  if (ply + 1 >= MAX_PLY) { return stand_pat; }\n'),
  origin="S279")
