"""The pawn correction table, S099 part (b).

Fifteen mutants over the table, its gate, its read and where the search
uses it. The accepts' own mutant is CH01: the corrected score's clamp to the
mate band's edge dropped, which "a corrected score never reaches the mate
band" exists to kill. Part (a)'s pawn key has its own file,
tools/mutants/S099_pawn_key.py, because it lands on its own and stays
whatever this part's verdict reads.

    CH01, CH02  the two bounds on what is applied -- the mate band's edge and
                CorrHistMax -- each dropped
    CH03..CH06  the four published gate clauses, each dropped or loosened
    CH07, CH08  the two stores writing the corrected evaluation instead of the
                raw one: the double correction the step file's pitfalls name
    CH09, CH11  the side to move read wrongly: the other side's slot, and the
                correction negated for Black
    CH10, CH14  the difference learned the wrong way: negated, and taken from
                the corrected evaluation instead of the raw one
    CH12        the update's weight uncapped
    CH13        `ucinewgame` leaving the table warm
    CH15        the capture clause refusing a fail-low as well, the gate as
                phase 2's fast check found it (added 2026-10-14)

Which case is meant to kill each is named at the case, under "Mutation:".

**CH06 is equivalent in the engine and killed anyway.** The search's only
in-check nodes carry TT_EVAL_NONE, which the gate refuses on its own, so the
`in_check` clause decides nothing a search reaches and the bench signature
cannot move; "the gate refuses each published case and nothing else" hands
the gate an in-check node with a real raw score and kills it there. It is
`expected="killed"` because the gate's contract is the clause, and the probe
is what holds the contract.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file`; `expected` is "killed" unless a
person has argued the mutant equivalent to the engine.

Prefix CH is new: `git log --all -p -- tools/mutants` shows AW, GC, LQ, MW,
NG, NT, PD, RZ, TC and every single letter taken, and no CH.
"""

S = "src/search.cpp"
H = "src/correction_history.hpp"
C = "src/chesso.cpp"


m("CH01_mate_clamp_dropped", S, "search/correction",
  'the corrected static evaluation is the raw one plus the correction with no '
  'clamp to the mate band\'s edge, so a raw score near the band plus a '
  'positive correction is a static score inside it -- a distance read as a '
  'value by every margin downstream. Unreachable from evaluate() today, which '
  'stays far from the band; the accepts asks for the bound to be asserted, not '
  'argued',
  ("  return std::clamp(raw_eval + correction, -(MATE_MIN - 1), MATE_MIN - 1);\n",
   "  return raw_eval + correction;\n"),
  origin="S099")

m("CH02_read_clamp_dropped", H, "search/correction",
  'the read applies entry / grain whatever the entry holds, so a CorrHistMax '
  'lowered in the tune build over a warm table, or any entry past the clamp, '
  'applies more than CorrHistMax says',
  ("  return std::clamp(value, -CORRHIST_MAX, CORRHIST_MAX);\n",
   "  return value;\n"),
  origin="S099")

m("CH03_mate_gate_dropped", S, "search/correction",
  'a mate score teaches the table: every position sharing the pawn structure '
  'has its static score moved toward a mate distance, the commit family the '
  'step file\'s section 1 records',
  ("  if (score >= MATE_MIN || score <= -MATE_MIN) { return false; }\n",
   "  // CH03: the mate clause dropped\n"),
  origin="S099")

m("CH04_bound_direction_dropped", S, "search/correction",
  'a bound teaches whatever side of the raw score it lies on, so a fail-high '
  'below the static score teaches a negative error it never established',
  ("  if (type == TT_BETA_NODE) { return score >= raw_eval; }\n"
   "  if (type == TT_ALPHA_NODE) { return score <= raw_eval; }\n",
   "  if (type == TT_BETA_NODE) { return true; }\n"
   "  if (type == TT_ALPHA_NODE) { return true; }\n"),
  origin="S099")

m("CH05_capture_gate_dropped", S, "search/correction",
  'a node whose best move is a capture teaches the table, so a material swing '
  'the static score was never meant to see is learned as positional error',
  ("  if (type != TT_ALPHA_NODE && MOVE_CAPTURE(best_move) != 0) { return false; }\n",
   "  (void)best_move;\n"),
  origin="S099")

m("CH06_check_gate_dropped", S, "search/correction",
  'the in-check clause dropped, leaving the sentinel test alone. Equivalent in '
  'the engine -- every in-check node carries TT_EVAL_NONE -- and killed by the '
  'gate case, which holds the clause as the contract (the header says why)',
  ("  if (in_check || raw_eval == TT_EVAL_NONE) { return false; }\n",
   "  (void)in_check;\n"
   "  if (raw_eval == TT_EVAL_NONE) { return false; }\n"),
  origin="S099")

m("CH07_store_writes_corrected", S, "search/correction",
  'the node\'s store writes the corrected evaluation, so the next read of the '
  'position corrects it again: the compounding the raw-in-table layering '
  'exists to prevent',
  ("best_move,\n"
   "                   raw_eval, !move_skipped);\n",
   "best_move,\n"
   "                   static_eval, !move_skipped);\n"),
  origin="S099")

m("CH08_probcut_store_writes_corrected", S, "search/correction",
  'ProbCut\'s store writes the corrected evaluation -- the same compounding, '
  'through the other store a node makes',
  ("pc_moves[j],\n"
   "                         raw_eval);\n",
   "pc_moves[j],\n"
   "                         static_eval);\n"),
  origin="S099")

m("CH09_read_side_flipped", S, "search/correction",
  'the node reads the other side\'s slot for its pawn key, so White\'s learned '
  'error for a structure corrects Black\'s static score and the other way '
  'round',
  ("      state->pawn_correction, board->active_color, board->pawn_hash);\n",
   "      state->pawn_correction, !board->active_color, board->pawn_hash);\n"),
  origin="S099")

m("CH10_update_sign_flipped", S, "search/correction",
  'the table learns the raw score less the search\'s, so every correction '
  'pushes the static score away from what the search found',
  ("game->board.pawn_hash, best_so_far - raw_eval, depth);\n",
   "game->board.pawn_hash, raw_eval - best_so_far, depth);\n"),
  origin="S099")

m("CH11_read_negated_for_black", S, "search/correction",
  'the correction is applied from White\'s point of view, negated when Black '
  'is to move, so a position and its mirror read corrections of opposite sign '
  '-- INV-5 broken through the table',
  ("  return std::clamp(raw_eval + correction, -(MATE_MIN - 1), MATE_MIN - 1);\n",
   "  return std::clamp(\n"
   "      raw_eval + ((board->active_color == WHITE) ? correction : -correction),\n"
   "      -(MATE_MIN - 1), MATE_MIN - 1);\n"),
  origin="S099")

m("CH12_weight_cap_dropped", H, "search/correction",
  'the update\'s weight is capped by the scale alone, so a deep node replaces '
  'the entry outright and the table remembers one observation',
  ("  return std::min(std::min(grown, CORRHIST_WEIGHT_CAP), CORRHIST_SCALE);\n",
   "  return std::min(grown, CORRHIST_SCALE);\n"),
  origin="S099")

m("CH13_newgame_keeps_correction", C, "uci/state",
  '`ucinewgame` leaves the correction table as the last game left it, so a '
  'game starts on another game\'s structure errors and bench\'s positions '
  'learn from each other',
  ("  tt_reset(&tt);\n"
   "  correction_reset(&pawn_correction);\n"
   "  proven_mate_line.length = 0;\n",
   "  tt_reset(&tt);\n"
   "  proven_mate_line.length = 0;\n"),
  origin="S099")

m("CH14_update_learns_corrected", S, "search/correction",
  'the difference is taken from the corrected evaluation, so the table learns '
  'its own residual and an entry stops moving once it is half right -- the '
  'variant the step file names as a later tweak, taken here by mistake',
  ("game->board.pawn_hash, best_so_far - raw_eval, depth);\n",
   "game->board.pawn_hash, best_so_far - static_eval, depth);\n"),
  origin="S099")

m("CH15_capture_gate_refuses_fail_low", S, "search/correction",
  'the capture clause refuses every node whose best move takes something, '
  'fail-lows included -- the gate as it stood until phase 2\'s fast check, '
  'when a fail-low\'s best move, only the highest of its upper bounds and '
  'this engine\'s form of the published "no best move", kept its capture '
  'from teaching the table',
  ("  if (type != TT_ALPHA_NODE && MOVE_CAPTURE(best_move) != 0) { return false; }\n",
   "  if (best_move != 0 && MOVE_CAPTURE(best_move) != 0) { return false; }\n"),
  origin="S099")
