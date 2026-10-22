"""The null move's table guard, narrowed to a whole search's bound, S292.

Nine mutants over the two things this step adds: the guard in the null-move
block of `negamax_at` -- no pass at a node whose table entry is an upper bound
(or an exact score) inside the mated band **written by a node that searched
every legal move** -- and the entry's `no_move_skipped`, which negamax's own store
writes and the two skip sites clear. Each is the guard dropped, widened or
narrowed, or the mark misplaced; never a constant moved, since neither has one.

Where a mutant would leave a variable unused the build refuses it (`-Werror`,
S289's stillborn NM01), so the term is defeated in place -- `|| true`,
`&& false` -- and not deleted. Each is killed by the case named, in the
"search: pruning and reduction guards" suite of tests/test_search.cpp unless
tests/test_engine.cpp is named:

  NF01  the guard dropped (the engine      "a node a whole search found mated
        before S292)                         makes no null move", both guarded
                                             legs, every row; test_engine's "a
                                             root mated in two reports the
                                             mate even where its node could
                                             pass", six of 31 roots
  NF02  the guard reads a pruned search's  the pruned-bound leg: S289's guard
        bound too                            as it read H0 (DEC-309)
  NF03  negamax's store says every move    "a node that skips a move says so
        was searched                         in its entry", both pruned drives
  NF04  negamax's store never says it      test_engine's defender-root case;
                                             the store case's whole-search
                                             drives
  NF05  the engine's own skip, before      the store case's unprobed pruned
        make_move, leaves no mark            drive
  NF06  the probed node's skip, after      the store case's probed pruned
        make_move, leaves no mark            drive
  NF07  any bound type in the band stops   the lower-bound leg
        the pass
  NF08  the band dropped: any whole        the two out-of-band legs
        search's upper or exact bound
        stops the pass
  NF09  the exact disjunct dropped         the mated-exact leg

NF08 moves ordinary play: any shallower whole-search upper bound switches the
pass off, which is a table-driven skip with its own verdict, not this guard.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: NF is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch (`git log --all -p --
tools/mutants`).
"""

S = "src/search.cpp"

GUARD = ("    const bool whole_search_found_mated =\n"
         "        tt_entry != nullptr && tt_entry_no_move_skipped &&\n"
         "        (tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) &&\n"
         "        tt_entry_score <= -MATE_MIN;")

STORE = "                   raw_eval, !move_skipped);"


def guard_with(old_term, new_term):
    """The guard with one term replaced; the term must occur in it once."""
    assert GUARD.count(old_term) == 1, old_term
    return (GUARD, GUARD.replace(old_term, new_term))


m("NF01_nmp_full_search_guard_dropped", S, "search/pruning",
  'the pass is made at a node a whole search found mated: a reduced search '
  'finds the defender\'s node mated by every move, the re-search at full '
  'depth passes, the pass fails high and the mating move is refuted',
  guard_with("tt_entry_score <= -MATE_MIN;",
             "tt_entry_score <= -MATE_MIN && false;"),
  origin="S292")

m("NF02_nmp_guard_trusts_a_pruned_bound", S, "search/pruning",
  'a mated bound from a node where a shallow-depth rule skipped a move also '
  'stops the pass -- the best of the moves searched read as the best move, '
  'S289\'s guard as it read H0',
  guard_with("tt_entry_no_move_skipped &&", "(tt_entry_no_move_skipped || true) &&"),
  origin="S292")

m("NF03_tt_store_marks_every_node", S, "search/table",
  'negamax\'s store says every move was searched whatever a rule skipped, '
  'which hands the guard the pruned bounds S289\'s read',
  (STORE, "                   raw_eval, !move_skipped || true);"),
  origin="S292")

m("NF04_tt_store_marks_no_node", S, "search/table",
  'negamax\'s store never says a node searched every move, so the guard '
  'never fires and the six mates in two are hidden again',
  (STORE, "                   raw_eval, !move_skipped && false);"),
  origin="S292")

m("NF05_engine_skip_not_marked", S, "search/table",
  'the skip the engine\'s own node takes before make_move no longer marks '
  'the entry, so a pruned bound reads as a whole search\'s in play',
  ("        if (!move_gives_check(&game->board, moves[i])) {\n"
   "          move_skipped = true;\n"
   "          continue;",
   "        if (!move_gives_check(&game->board, moves[i])) {\n"
   "          continue;"),
  origin="S292")

m("NF06_probe_skip_not_marked", S, "search/table",
  'the probed node\'s skip, after make_move, no longer marks the entry',
  ("      move_skipped = true;\n"
   "      unmake_move(game);\n"
   "      continue;",
   "      unmake_move(game);\n"
   "      continue;"),
  origin="S292")

m("NF07_nmp_guard_any_bound", S, "search/pruning",
  'a lower bound inside the mated band also stops the pass, though it says '
  'only that the node is worth at least a mate against it',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&\n        ", ""),
  origin="S292")

m("NF08_nmp_guard_band_dropped", S, "search/pruning",
  'any whole search\'s upper bound or exact score stops the pass, inside the '
  'band or not: a shallower estimate decides where the guard trusts a proof',
  guard_with("tt_entry_score <= -MATE_MIN;", "true;"),
  origin="S292")

m("NF09_nmp_guard_exact_dropped", S, "search/pruning",
  'an exact score inside the mated band no longer stops the pass: only the '
  'upper bound does, though the exact score is the same finding',
  guard_with("(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE) "
             "&&", "tt_entry_type == TT_ALPHA_NODE &&"),
  origin="S292")
