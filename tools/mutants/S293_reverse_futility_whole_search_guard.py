"""Reverse futility's table guard, S293: S292's condition asked of the null
move's static form.

Five mutants over the one thing this step adds: `whole_search_mated_entry` in
`negamax_at`, read by reverse futility's condition -- no reverse futility at a
node whose table entry is an upper bound (or an exact score) inside the mated
band **written by a node that searched every legal move**. Each is the guard
dropped, widened or narrowed; never a constant moved, since it has none. The
entry's `no_move_skipped` and the marks that write it are S292's and are held by
tools/mutants/S292_null_move_full_search_guard.py (NF03-NF06).

Where a mutant would leave a variable unused the build refuses it (`-Werror`,
S289's stillborn NM01), so the term is defeated in place -- `|| true`,
`&& false` -- and not deleted. Each is killed by tests/test_search.cpp's
"reverse futility does not fire at a node a whole search found mated", in the
"search: pruning and reduction guards" suite:

  RW01  the guard dropped (the engine      both guarded legs, every row
        before S293)
  RW02  the guard reads a pruned search's  the pruned-bound leg
        bound too
  RW03  any bound type in the band stops   the lower-bound leg
        the rule
  RW04  the band dropped: any whole        the two out-of-band legs
        search's upper or exact bound
        stops the rule
  RW05  the exact disjunct dropped         the mated-exact leg

RW04 moves ordinary play: any shallower whole-search upper bound switches the
rule off, which is a table-driven skip with its own verdict, not this guard.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: RW is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch (`git log --all -p --
tools/mutants`).
"""

S = "src/search.cpp"

GUARD = ("  const bool whole_search_mated_entry =\n"
         "      tt_entry != nullptr && tt_entry_no_move_skipped &&\n"
         "      (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&\n"
         "      tt_entry_score <= -MATE_MIN;")


def guard_with(old_term, new_term):
    """The guard with one term replaced; the term must occur in it once."""
    assert GUARD.count(old_term) == 1, old_term
    return (GUARD, GUARD.replace(old_term, new_term))


m("RW01_rfp_whole_search_guard_dropped", S, "search/pruning",
  'reverse futility fires at a node a whole search found mated: the margin '
  'argues the opponent cannot win the difference back where a search of every '
  'move found the side to move mated by each of them',
  guard_with("tt_entry_score <= -MATE_MIN;",
             "tt_entry_score <= -MATE_MIN && false;"),
  origin="S293")

m("RW02_rfp_guard_trusts_a_pruned_bound", S, "search/pruning",
  'a mated bound from a node where a shallow-depth rule skipped a move also '
  'stops reverse futility -- the best of the moves searched read as the best '
  'move',
  guard_with("tt_entry_no_move_skipped &&", "(tt_entry_no_move_skipped || true) &&"),
  origin="S293")

m("RW03_rfp_guard_any_bound", S, "search/pruning",
  'a lower bound inside the mated band also stops reverse futility, though it '
  'says only that the node is worth at least a mate against it',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&\n      ", ""),
  origin="S293")

m("RW04_rfp_guard_band_dropped", S, "search/pruning",
  'any whole search\'s upper bound or exact score stops reverse futility, '
  'inside the band or not: a shallower estimate decides where the guard '
  'trusts a proof',
  guard_with("tt_entry_score <= -MATE_MIN;", "true;"),
  origin="S293")

m("RW05_rfp_guard_exact_dropped", S, "search/pruning",
  'an exact score inside the mated band no longer stops reverse futility: '
  'only the upper bound does, though the exact score is the same finding',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&", "tt_entry_type == TT_ALPHA_NODE &&"),
  origin="S293")
