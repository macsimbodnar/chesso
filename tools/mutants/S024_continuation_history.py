"""S024's one-ply continuation history: the mutant its own bookkeeping test
kills, DEC-141 clause 2.

The malus sign flipped, the same bug class M16_history_malus_sign already
covers for plain quiet_history -- a quiet tried before the cutoff would be
credited in the continuation table instead of charged.

The list is data: `m` is pre-bound by tools/mutation_check.py, which execs
this file, so nothing here is a driver. Ids are never reused: a new mutant
takes the next free number across every file here.
"""

S = "src/search.cpp"

m("S024_M01_cont_hist_malus_sign", S, "search/ordering",
  'continuation history malus applied as a bonus: a quiet tried before the '
  'cutoff is credited in its (prev_move, move) cell instead of charged',
  ('continuation_entry(state, prev_move, quiets_tried[i]), -malus);',
   'continuation_entry(state, prev_move, quiets_tried[i]), malus);'),
  origin="S024")

# The sentinel guard: ply 0 and the node right after a null move both reach
# history_on_quiet_cutoff() with prev_move 0, which decodes to a legitimate
# (W_PAWN, a8) cell rather than an out-of-range index -- so dropping this
# guard does not crash or trip a sanitizer, it silently writes that cell.
# Caught only by the cases that scan the whole table rather than one cell:
# "a quiet cutoff maluses the quiets tried before it" and "the cutoff move is
# credited and the quiets before it are charged", both of which drive a
# fail-high with prev_move 0 and require the continuation table to stay
# entirely at zero.
m("S024_M02_cont_hist_no_prev_guard", S, "search/ordering",
  'the previous-move guard dropped: history_on_quiet_cutoff() indexes the '
  'continuation table even with no previous move to index',
  ('const bool has_prev = prev_move != 0;',
   'const bool has_prev = true;'),
  origin="S024")
