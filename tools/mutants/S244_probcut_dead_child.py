"""ProbCut's screen of a dead-board child, S244.

Two mutants over the one test this step adds to ProbCut's capture loop in
`negamax_at`: a capture that leaves insufficient material is answered
`DRAW_SCORE`, neither the preliminary quiescence nor the shallow search is
asked for it, and the draw meets the bar like any other value. Each is a
screen dropped or its answer written wrong, never a constant moved, and each
is killed by one case in tests/test_search.cpp's "search: pruning and
reduction guards" suite:

  PD01  the screen dropped            "probcut answers a capture that leaves a
                                      dead board as a draw", both legs
  PD02  the dead child never cuts     the same case, its leg with the bar on
                                      the draw

PD01 is the tree before this step. PD02 is the other way to write a screen,
skipping the dead child rather than answering it: no search and no stale
entry, but a draw that clears the bar no longer ends the node.

Both leave the bench signature where it was, because no bench position at any
depth the census took puts a dead board inside ProbCut's loop (0 of 3040
captures at `bench`, the step file's reach section); only the case sees them.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused, and every single-letter prefix has been: tools/mutants
on every branch has used all but A and O (`git log --all -p -- tools/mutants`),
and tests/test_mutation_check.py's sandbox list uses those two. So this file
opens a two-letter one, PD, which nothing has used.
"""

S = "src/search.cpp"

m("PD01_probcut_dead_screen_dropped", S, "search/pruning",
  'a capture that leaves a dead board pays the preliminary again: quiescence '
  'scores the material left and stores it at TT_DEPTH_QS, and a draw that '
  'clears the bar ends the node only where that material score cleared it '
  'first',
  ("        const bool pc_dead = is_insufficient_material(&game->board);\n",
   "        const bool pc_dead = false;\n"),
  origin="S244")

m("PD02_probcut_dead_child_never_cuts", S, "search/pruning",
  'the dead child is skipped rather than answered: nothing is searched or '
  'stored, but its draw never meets the bar, so a node whose bar the draw '
  'clears is searched at full depth for a result the capture already '
  'settled',
  ("        int value = pc_dead ? -DRAW_SCORE\n",
   "        int value = pc_dead ? MIN\n"),
  origin="S244")
