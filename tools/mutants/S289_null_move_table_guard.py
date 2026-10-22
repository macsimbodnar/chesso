"""The null move's table guard, S289.

Four mutants over the one condition this step adds to the null-move block in
`negamax_at`: no pass at a node whose table entry is an upper bound (or an
exact score) inside the mated band. Each is the guard dropped or widened,
never a constant moved -- the guard has none. NM01 drops it by making the flag
false rather than by deleting its use, which the build refuses as an unused
variable (`-Werror`): the first run read it stillborn that way. Each is killed by tests/
test_search.cpp's "a node the table proves mated makes no null move", in the
"search: pruning and reduction guards" suite, at the leg named:

  NM01  the guard dropped (the engine    the guarded leg, every row; also
        before S289)                       test_engine's "a root mated in two
                                           reports the mate even where its
                                           node could pass", six of 31 roots
  NM02  any bound type in the band       the lower-bound leg: a lower bound
        stops the pass                     proves nothing about the moves
  NM03  the band dropped: any upper      the out-of-band leg: a shallower
        bound stops the pass               upper bound is an estimate, and
                                           trusting it is another rule
  NM04  the exact disjunct dropped: an   the mated-exact leg
        exact score in the band no
        longer stops the pass

NM03 moves ordinary play: any shallower upper bound, a fail-low's estimate,
switches the pass off. A table-driven skip of that kind is a candidate with its
own verdict, not this guard.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: NM is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch (`git log --all -p --
tools/mutants`).
"""

S = "src/search.cpp"

GUARD = ("    const bool table_proves_mated =\n"
         "        tt_entry != nullptr &&\n"
         "        (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&\n"
         "        tt_entry_score <= -MATE_MIN;")


def guard_with(old_term, new_term):
    """The guard with one term replaced; the term must occur in it once."""
    assert GUARD.count(old_term) == 1, old_term
    return (GUARD, GUARD.replace(old_term, new_term))


m("NM01_nmp_table_mated_guard_dropped", S, "search/pruning",
  'the pass is made at a node the table proves mated: a reduced search '
  'finds the defender\'s node mated by every move it searched, the re-search '
  'at full depth passes, the pass fails high and the mating move is refuted',
  (GUARD, "    const bool table_proves_mated = false;"),
  origin="S289")

m("NM02_nmp_table_guard_any_bound", S, "search/pruning",
  'a lower bound inside the mated band also stops the pass, though it says '
  'only that the node is worth at least a mate against it',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&\n        ", ""),
  origin="S289")

m("NM03_nmp_table_guard_any_upper_bound", S, "search/pruning",
  'any upper bound stops the pass, inside the band or not: a shallower '
  'estimate decides where the guard only trusts a proof',
  guard_with("tt_entry_score <= -MATE_MIN;", "true;"),
  origin="S289")

m("NM04_nmp_table_guard_exact_dropped", S, "search/pruning",
  'an exact score inside the mated band no longer stops the pass: only the '
  'upper bound does, though the exact score is the same finding',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&", "tt_entry_type == TT_ALPHA_NODE &&"),
  origin="S289")
