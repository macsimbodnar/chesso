id:         S215
goal:       the one assignment the repetition rule hangs on is pinned by a test that reaches it through `search()`, and by a mutant that test kills
accepts:    a case in `tests/test_search.cpp` drives `search()` -- not `negamax()` with the field set by hand -- from a position whose draw can only come from `index > root_history_size`, so it fails if `src/search.cpp` `search()` sets `state->root_history_size` to anything but `game->history.size`; the case asserts against `DRAW_SCORE` and a non-draw score at the same board with a different pre-root history, both the S192 way against zero rather than against a golden; a mutant anchored on that assignment joins `tools/mutants/search.py` -- `history.size - 1`, which is the one that restores the pre-S207 behaviour for the root-recurrence class -- and `tools/mutation_check.py` reports it **killed** from a worktree at the completing commit; `+ 1` is either killed by the same case or is stated at the mutant as the surviving direction with what it costs; INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature, because the fix is a test and not a rule
touches:    tests/test_search.cpp, tools/mutants/search.py
excludes:   any change to `classify_repetition()` or to the rule, which is S207's and is correct; `tools/datagen.cpp`'s two-fold game adjudication, which is S082/S083's question and is recorded at S207's own file
decisions:  DEC-141, DEC-173
closes:
blocks:
paused_by:
author:
done:

## Why this exists

Found by the Tier-1 fast check over S207's completing commit `23f926d`, on
2026-09-11, and **verified by measurement before it was written down**.

S207's rule has two halves. `classify_repetition()` compares the matching
history index against a boundary -- that half is pinned, by *the root's own
occurrence is the boundary* and by mutants `M35` and `M36`. The other half is
that the boundary handed in is the root's own index, one line in
`src/search.cpp` `search()`:

    state->root_history_size = game->history.size;

**Nothing in the suite reads that line.** Mutated to `game->history.size - 1`
and rebuilt, `ctest -L fast` is green on all 32 functional tests
(`test_clang_format_script` fails only for DEC-146's reason), and `bench`
reads **26117924** against the tree's **30046849** -- a tree 13 % different,
so the mutant is emphatically not equivalent and the suite is simply blind to
it.

What `- 1` does is restore the pre-S207 behaviour for one class: the root's own
entry becomes "in tree", so **the root position recurring once inside the tree
scores `DRAW_SCORE` again**. That is part of the F08 shape and it is the class
whose removal moved `bench` by 1.34 % at S207.

Why the three existing cases miss it, from their own asserted inputs:

- *the losing side takes an available repetition* reaches `search()`, but its
  draw comes from **two** matches, at history indices 0 and 4 with the boundary
  at 7. Both are below the boundary at 6, 7 and 8 alike, so `seen_one` fires
  either way and the case is boundary-invariant.
- *one occurrence before the root is not a draw* reaches `search()` with one
  match at index 0 and a boundary of 3; `0 > 2`, `0 > 3` and `0 > 4` are all
  false, so it reads `ONCE_PRE_ROOT` either way.
- *the root's own occurrence is the boundary* sets `root_history_size` by hand
  and calls `negamax()`, so it never executes the assignment.
- No fast test pins an exact node count out of `search()`, only inequalities
  and run-to-run equality, so the node shift is invisible too.

The `SIZE_MAX` default is what makes the gap structural: no `negamax()`-driven
test can reach the in-tree branch without setting the field by hand, and both
`search()`-driven cases land elsewhere.

## Shape

One case, and the position has to be built so the *only* available draw is an
in-tree one. A root that recurs inside its own tree is the direct form: a
position from which the side to move can shuffle back to it in two plies, with
no pre-root occurrence at all, where the side to move is losing by the engine's
own evaluation so the draw score is visibly preferred. Assert `DRAW_SCORE`
there; then assert a non-draw at the same board reached with one pre-root
occurrence, which is S207's own rule from the other side and is what makes the
pair sensitive to the boundary rather than to the position.

This is a test and a mutant. **It changes no rule**, so it owes node counts and
a bench signature rather than a verdict.

## Cost

Agent work, an hour. No run.
