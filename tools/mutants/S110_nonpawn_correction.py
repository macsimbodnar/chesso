"""The two non-pawn correction tables, S110.

Fourteen mutants over the tables' read, blend, update and ownership. The
mate band's clamp is the pawn table's line and stays CH01's
(tools/mutants/S099_correction_history.py); "a corrected score never reaches
the mate band with every table saturated" names it as well, so the accepts'
bound is killed with all three tables live.

    NC01..NC04  the blend, a weighted average since DEC-300: the sum it
                replaced, White's read counted for Black's, the pair summed
                instead of averaged in the weights, the weight ignored
    NC14        the blend's rounding not symmetric in sign
    NC05, NC06  the read: a colour's table read at the other colour's key,
                and for the other side to move
    NC07..NC10  the update: ungated, its difference negated or taken from the
                corrected score, a colour's table taught at the other's key
    NC11, NC13  the UCI layer: `ucinewgame` leaving the tables warm, and the
                tables never attached to a search
    NC12        the non-pawn reads never applied

**The ids were reassigned once, before any commit** (DEC-300): NC01 was the
total clamp to one pawn, which the average made unnecessary and S110 dropped;
NC01 is now the sum the average replaced, and NC14 is new with the rounding.
No commit ever carried the old NC01.

**NC04 is killed in the release build** although the weight there is a
constant 100 that the mutant's 100 equals: the blend takes the weight as a
parameter and "the blend is a weighted average of the pawn read and the
pair" holds it at 0, 50 and 200, which the search never passes.

Which case is meant to kill each is named at the case, under "Mutation:".

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`; `expected` is "killed".

Prefix NC is new: `git log --all -p -- tools/mutants` shows no NC (the list
is in tools/mutants/S110_nonpawn_key.py's header).
"""

S = "src/search.cpp"
H = "src/correction_history.hpp"
C = "src/chesso.cpp"


m("NC01_blend_is_a_sum", H, "search/correction",
  'the blend adds the non-pawn mean to the pawn read instead of averaging '
  'with it -- the design DEC-300 replaced: all three tables learn the same '
  'residual, so the sum applies about twice the correction the pawn table '
  'alone did, up to twice CorrHistMax',
  ("  const int denominator = 200 + 2 * weight;\n"
   "  const int half = denominator / 2;\n"
   "\n"
   "  return (numerator >= 0) ? (numerator + half) / denominator\n"
   "                          : -((-numerator + half) / denominator);\n",
   "  (void)numerator;\n"
   "  return pawn +\n"
   "         (nonpawn.by_colour[WHITE] + nonpawn.by_colour[BLACK]) * weight / 200;\n"),
  origin="S110")

m("NC02_blend_reads_white_only", H, "search/correction",
  'the blend counts White\'s key\'s read twice and Black\'s not at all, so '
  'half of the non-pawn structure is never corrected for',
  ("  const int numerator = 200 * pawn + weight * (nonpawn.by_colour[WHITE] +\n"
   "                                               nonpawn.by_colour[BLACK]);\n",
   "  const int numerator = 200 * pawn + weight * (nonpawn.by_colour[WHITE] +\n"
   "                                               nonpawn.by_colour[WHITE]);\n"),
  origin="S110")

m("NC03_blend_sums_the_pair", H, "search/correction",
  'the denominator counts the pair as one read rather than two, so the '
  'average weighs the two non-pawn reads\' sum where it means their mean: '
  'at 100 the pair outweighs the pawn read two to one and the blend can '
  'leave CorrHistMax',
  ("  const int denominator = 200 + 2 * weight;\n",
   "  const int denominator = 200 + weight;\n"),
  origin="S110")

m("NC04_blend_ignores_the_weight", H, "search/correction",
  'the blend averages at 100 whatever the weight says, so '
  'CorrHistNonPawnWeight 0 does not switch the tables off (DEC-215)',
  ("  const int numerator = 200 * pawn + weight * (nonpawn.by_colour[WHITE] +\n"
   "                                               nonpawn.by_colour[BLACK]);\n"
   "  const int denominator = 200 + 2 * weight;\n",
   "  (void)weight;\n"
   "  const int numerator = 200 * pawn + 100 * (nonpawn.by_colour[WHITE] +\n"
   "                                            nonpawn.by_colour[BLACK]);\n"
   "  const int denominator = 200 + 2 * 100;\n"),
  origin="S110")

m("NC14_blend_rounds_toward_floor", H, "search/correction",
  'the blend adds half the denominator and truncates whatever the sign, so '
  'a negative blend rounds toward zero where a positive one rounds away: a '
  'position and its mirror no longer blend to numbers of one size (INV-5)',
  ("  return (numerator >= 0) ? (numerator + half) / denominator\n"
   "                          : -((-numerator + half) / denominator);\n",
   "  return (numerator + half) / denominator;\n"),
  origin="S110")

m("NC05_read_crosses_the_keys", H, "search/correction",
  'White\'s key\'s table is read at Black\'s key, so what it learned about '
  'White\'s pieces is never what it returns',
  ("  return {{correction_value(&tables->by_colour[WHITE], side, keys[WHITE]),\n",
   "  return {{correction_value(&tables->by_colour[WHITE], side, keys[BLACK]),\n"),
  origin="S110")

m("NC06_read_side_flipped", H, "search/correction",
  'both non-pawn tables are read for the side not to move, so White\'s '
  'learned error for a structure corrects Black\'s static score',
  ("  return {{correction_value(&tables->by_colour[WHITE], side, keys[WHITE]),\n"
   "           correction_value(&tables->by_colour[BLACK], side, keys[BLACK])}};\n",
   "  return {{correction_value(&tables->by_colour[WHITE], !side, keys[WHITE]),\n"
   "           correction_value(&tables->by_colour[BLACK], !side, keys[BLACK])}};\n"),
  origin="S110")

m("NC07_nonpawn_gate_dropped", S, "search/correction",
  'the non-pawn tables learn at every store, in check, on a capture, on a '
  'mate score and on a bound pointing the wrong way: the pawn table\'s gate '
  'not applied to them',
  ("    if (state->nonpawn_correction != nullptr &&\n"
   "        correction_may_learn(is_in_check, raw_eval, best_move, type,\n"
   "                             best_so_far)) {\n",
   "    if (state->nonpawn_correction != nullptr && raw_eval != TT_EVAL_NONE) {\n"),
  origin="S110")

m("NC08_nonpawn_update_sign_flipped", S, "search/correction",
  'the non-pawn tables learn the raw score less the search\'s, so every '
  'non-pawn correction pushes the static score away from what the search '
  'found',
  ("          game->board.nonpawn_hash, best_so_far - raw_eval, depth);\n",
   "          game->board.nonpawn_hash, raw_eval - best_so_far, depth);\n"),
  origin="S110")

m("NC09_nonpawn_learns_corrected", S, "search/correction",
  'the non-pawn tables learn from the corrected score, so they learn the '
  'residual of all three tables and stop moving once that is small',
  ("          game->board.nonpawn_hash, best_so_far - raw_eval, depth);\n",
   "          game->board.nonpawn_hash, best_so_far - static_eval, depth);\n"),
  origin="S110")

m("NC10_update_crosses_the_keys", H, "search/correction",
  'White\'s key\'s table is taught at Black\'s key, so the slot it reads is '
  'never the slot it learned in',
  ("  correction_update(&tables->by_colour[WHITE], side, keys[WHITE], diff, depth);\n",
   "  correction_update(&tables->by_colour[WHITE], side, keys[BLACK], diff, depth);\n"),
  origin="S110")

m("NC11_newgame_keeps_nonpawn", C, "uci/state",
  '`ucinewgame` leaves the non-pawn tables as the last game left them, so a '
  'game starts on another game\'s errors and bench\'s positions learn from '
  'each other',
  ("  proven_mate_line.length = 0;\n"
   "  correction_reset(&nonpawn_correction);\n",
   "  proven_mate_line.length = 0;\n"),
  origin="S110")

m("NC12_nonpawn_not_applied", S, "search/correction",
  'the non-pawn tables are taught and never read: the corrected score is the '
  'pawn table\'s alone',
  ("  if (state->nonpawn_correction != nullptr) {\n"
   "    correction = correction_blend(\n",
   "  if (state->nonpawn_correction != nullptr) {\n"
   "    (void)correction_blend(\n"),
  origin="S110")

m("NC13_uci_does_not_attach_nonpawn", C, "uci/state",
  'the UCI layer never attaches its non-pawn tables to a search, so the '
  'engine a match plays is the tree before S110 while the tables sit empty',
  ("  state.nonpawn_correction = &nonpawn_correction;\n",
   ""),
  origin="S110")
