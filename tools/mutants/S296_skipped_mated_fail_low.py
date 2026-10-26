"""A node that skipped moves claims no mate it did not search, S296.

Two mutants over the one rule this step adds at the end of `negamax_at`'s move
loop: where a shallow-depth rule skipped a move (`move_skipped`) and every move
the node searched came back mated (`best_so_far <= -MATE_MIN`), the node stores
and returns `alpha0` -- an upper bound at its window's edge -- instead of its
fail-soft mated best (DEC-318's form (a), adocs/data/S296_forms.txt).

  SK01  the rule dropped: the engine before    tests/test_search.cpp's "a node
        S296, the mated best returned and      that skipped moves claims no mate
        stored as the node's bound             it did not search" (the return,
                                               the entry's score)
  SK02  the skip condition dropped: a node     a whole search's mated bound is a
        that searched every move and found     proof (S292); returned as alpha it
        each mated returns alpha as well       hides the mate from every parent

The rule is defeated in place -- `&& false`, `|| true` -- not deleted, so no
variable goes unused under `-Werror`.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`. `expected` is "killed" unless a
person has argued the mutant behaviourally equivalent, which no tool decides.

Ids are never reused: SK is this file's own prefix, and no file in
tools/mutants/ has used it, on any branch (`git log --all -p --
tools/mutants`).
"""

S = "src/search.cpp"

RULE = "  if (move_skipped && best_so_far <= -MATE_MIN) {\n"

m("SK01_skipped_mated_fail_soft_kept", S, "search/pruning",
  'a node that skipped a move and found every searched move mated returns '
  'and stores its fail-soft mated best, a mate distance no search established '
  '(the engine before S296: S145 mined line 168 reads mate 4 at depth 8 where '
  'the position is mate in 5)',
  (RULE, "  if (move_skipped && best_so_far <= -MATE_MIN && false) {\n"),
  origin="S296")

m("SK02_whole_search_mated_returns_alpha", S, "search/pruning",
  'a node that searched every legal move and found each mated also returns '
  'alpha: a proof of the mate replaced by the window\'s edge',
  (RULE, "  if ((move_skipped || true) && best_so_far <= -MATE_MIN) {\n"),
  origin="S296")
