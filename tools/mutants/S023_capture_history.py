"""Capture history in the two capture margins, S023 verdict 1.

The table, its one index, its update at a fail-high, and the two margins it
moves -- S091's capture SEE margin in the move loop and S112's per-move
futility margin in quiescence. Every mutant is a guard dropped, a sign turned
round, a read skipped or a cell taken from the wrong square, never a constant
moved. Each is killed by the case its row names, in tests/test_search.cpp:

  CX01  the SEE margin's record term subtracted   "the capture margins move with
                                                   the record and never go below
                                                   zero"; "a capture's record
                                                   moves the capture SEE margin
                                                   at a pruning node"
  CX02  the SEE margin's floor dropped            "the capture margins move ..."
                                                   and "no capture that holds
                                                   its material is skipped
                                                   whatever its record"
  CX03  the SEE rule reads no record              "a capture's record moves the
                                                   capture SEE margin at a
                                                   pruning node"
  CX04  the futility margin's record subtracted   "the capture margins move ..."
                                                   and "a capture's worst record
                                                   makes a capture that could
                                                   reach alpha futile"
  CX05  the futility margin's floor dropped       declared equivalent, below
  CX06  quiescence reads no record                "a capture's worst record
                                                   makes ..." and "a capture's
                                                   best record searches ..."
  CX07  the capture malus credited                "a capture that cuts off is
                                                   credited and the captures
                                                   tried before it are charged"
  CX08  the cutoff capture never credited         the same case, and "a search
                                                   fills the capture table both
                                                   ways"
  CX10  the victim column read off the source     "the capture table is indexed
                                                   by mover, target and victim"
  CX11  the update made on capture cutoffs only   "a node that cuts off on a
                                                   quiet charges the captures
                                                   it searched first"
  CX12  the capture span never appended to        the same case, and "a search
                                                   fills the capture table both
                                                   ways"

CX09 is not written and its number is not reused: it was the en passant
victim read from the target square, and that mutant is equivalent by an
accident of the enum -- EMPTY modulo six is the pawn's column, so an en
passant capture lands in the pawn's cell either way. capture_victim_type()
keeps its explicit branch so the answer does not rest on that spacing, and
says so; a mutant no case can kill is not one to carry.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. A mutant's several pairs are applied together as one bug. `expected`
is "killed" unless a person has argued the mutant is behaviourally equivalent
to the engine, which no tool can decide.

Ids are never reused: CX is this file's own prefix, and no file in
tools/mutants/ has ever used it (`git log --all -p -- tools/mutants`, read
2026-10-13). `origin` says which step wrote the mutant.
"""

S = "src/search.cpp"
D = "src/data_structures.hpp"

m("CX01_see_margin_record_sign", S, "search/pruning",
  'the record\'s term is subtracted from the capture SEE margin, so a capture '
  'that has been cutting off is skipped sooner and one that has been failing '
  'is searched later -- the table read upside down',
  ('  return std::max(0, SEE_CAPT_COEFF * lmr_depth + term);',
   '  return std::max(0, SEE_CAPT_COEFF * lmr_depth - term);'),
  origin="S023")

m("CX02_see_margin_no_floor", S, "search/pruning",
  'the capture SEE margin is not floored, so the worst record drives it below '
  'zero at a shallow reduced depth, the bar turns positive, and a capture that '
  'holds its material -- an even trade -- is skipped: a class S091 never '
  'pruned and its mate cases were never mined against',
  ('  return std::max(0, SEE_CAPT_COEFF * lmr_depth + term);',
   '  return SEE_CAPT_COEFF * lmr_depth + term;'),
  origin="S023")

m("CX03_see_margin_record_unread", S, "search/pruning",
  'the capture rule tests every capture with an empty cell\'s margin, so the '
  'table is written at every fail-high and moves nothing -- the shape a census '
  'calls exercised and a verdict calls inert',
  ('        const int see_margin = capture_see_margin(lmr_depth, record);',
   '        const int see_margin = capture_see_margin(lmr_depth, 0);\n'
   '        (void)record;'),
  origin="S023")

m("CX04_futility_margin_record_sign", S, "search/pruning",
  'the record\'s term is subtracted from the quiescence futility margin, so the '
  'worst record widens it and the best narrows it',
  ('  return std::max(0, QS_FUTILITY_MARGIN + term);',
   '  return std::max(0, QS_FUTILITY_MARGIN - term);'),
  origin="S023")

# Equivalent at the shipped values, and the argument is arithmetic: the term
# is at most CaptHistQsWeight in size, which ships under QsFutilityMargin, so
# `QS_FUTILITY_MARGIN + term` is positive for every record the table can hold
# and the floor never binds. Where the tune build sets the weight above the
# margin it does bind, and tests/test_search_params.cpp "the capture margins
# stay at zero or above at every weight the ranges admit" holds it there -- in
# the build this tool does not run. If a refit puts QsFutilityMargin under
# CaptHistQsWeight, the floor binds at the defaults and "the capture margins
# move with the record and never go below zero" kills this; the declaration
# then goes, in that commit.
m("CX05_futility_margin_no_floor", S, "search/pruning",
  'the quiescence futility margin is not floored, so a weight above the margin '
  'lets the worst record skip a capture where the stand pat already raised '
  'alpha, and S112\'s claim that a fold never lifts a node above its entry '
  'alpha stops holding',
  ('  return std::max(0, QS_FUTILITY_MARGIN + term);',
   '  return QS_FUTILITY_MARGIN + term;'),
  expected="equivalent",
  origin="S023")

m("CX06_futility_margin_record_unread", S, "search/pruning",
  'quiescence tests every capture with an empty cell\'s margin, so the table '
  'moves the main search\'s capture rule and nothing at the leaves',
  ('      const int margin = capture_futility_margin(record);',
   '      const int margin = capture_futility_margin(0);\n'
   '      (void)record;'),
  origin="S023")

m("CX07_capture_malus_sign", S, "search/ordering",
  'the capture malus is credited instead of charged, so every capture a node '
  'searched and rejected is promoted in the table both margins read',
  ('        capture_history_entry(state, board, captures_tried[i]), -capt_malus,',
   '        capture_history_entry(state, board, captures_tried[i]), capt_malus,'),
  origin="S023")

m("CX08_capture_bonus_dropped", S, "search/ordering",
  'the capture that cut off is never credited, so the table only ever learns '
  'what fails and every record drifts to the bottom of the band',
  ('  if (MOVE_CAPTURE(cutoff_move)) {',
   '  if (false && MOVE_CAPTURE(cutoff_move)) {'),
  origin="S023")

m("CX10_capture_index_victim_from_mover", D, "search/ordering",
  'the victim column is read off the source square, so every capture is filed '
  'under its own mover\'s type, a knight taking a rook shares a cell with a '
  'knight taking a pawn on that square, and the record a margin reads is '
  'another capture\'s',
  ('  return board->squares[MOVE_TO(move)] % 6;',
   '  return board->squares[MOVE_FROM(move)] % 6;'),
  origin="S023")

m("CX11_capture_update_on_capture_cutoffs_only", S, "search/ordering",
  'the call site updates the capture table only when the move that cut off is '
  'itself a capture, so the captures a node searched before a quiet refuted '
  'it are never charged',
  ('      history_on_capture_cutoff(state, &game->board, moves[i], captures_tried,\n'
   '                                captures_tried_count, depth);',
   '      if (is_capture) {\n'
   '        history_on_capture_cutoff(state, &game->board, moves[i],\n'
   '                                  captures_tried, captures_tried_count,\n'
   '                                  depth);\n'
   '      }'),
  origin="S023")

m("CX12_captures_never_tried", S, "search/ordering",
  'the capture span is never appended to, so no capture is ever charged and '
  'every record a margin reads is a credit or nothing',
  ('      captures_tried_count++;\n'
   '      quiets_tried[MAX_MOVES - captures_tried_count] = moves[i];\n',
   ''),
  origin="S023")
