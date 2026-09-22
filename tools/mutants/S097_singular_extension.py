"""The singular extension, its verification search and the gates on it. S097.

Twenty-two mutants over one block and the plumbing it needs, and the split is
the step's own: **one per rule, one per gate**. The rules are three lines -- a
window under the table entry's score, a ply added to one move, and a score
returned where some other move already clears the node's beta -- and the gates
are what keep the search that decides them from answering itself.

WHAT EACH CLASS IS.

  the extension    applied to every move, dropped, or fired on the wrong side
                   of the comparison. A ply landing on the wrong move is the
                   silent failure this whole block has: no crash, no wrong node
                   count, only rating

  the window       the margin's sign, and the halved depth the verification
                   runs at. Both decide what "singular" means, and both are one
                   token wide

  the entry gates  the root, the entry's depth, the entry's bound type and the
                   mate band -- twice, because a score one point outside the
                   band still lands inside it once the margin comes off

  the node gates   what a node searched under an exclusion may not do: take a
                   table cutoff, write an entry, pass, run reverse futility, or
                   start a verification of its own. Each is one comparison and
                   each would leave the verification answering its own question

  the plumbing     the move loop's skip, which is the exclusion itself, and the
                   return a node whose only legal move was excluded owes -- it
                   is neither mated nor stalemated and both terminal scores are
                   claims about a position nobody is in

  the switch       `SeExtend` read the wrong way round. The block ships behind
                   a switch because DEC-215 clause 2 requires one where the
                   settings have no off value between them, and the H0 path of
                   the step's first verdict is a flip of that default -- so a
                   switch the gate reads backwards would hand the candidate
                   back under the name of the parent

  the multicut     what the same verification search returns where it fails
                   high: the bound instead of the score it found, the mate
                   band dropped off the value that leaves the node, the PV
                   gate dropped, and the defender's own window admitted. E20
                   to E23, and they are the whole of the step's second
                   verdict. E21 and E23 look like one guard and are two --
                   one on the value the node hands back, one on the window it
                   was asked under -- so each has its own mutant and its own
                   killer, which is what DEC-141 asks of a condition

WHEN E20 TO E23 ARRIVED, AND WHY NOT SOONER. E20 to E22 were **named and
reserved** in this header at verdict 1 and are written here at verdict 2;
**E23 is not one of the three the header reserved** and was added after the
verdict's own fast check found the defender's-beta term pinned by nothing.
Verdict 2 is one default: `SeMultiCut` 0 to 1 in `src/search_params.hpp` and no line of the rule
moved. Until that flip the switch was `inline constexpr int` at 0 in the
release build -- the only build this tool compiles -- so the branch folded away
entirely and a mutant of a rule that is not in the binary is equivalent by
construction: it would have been scored as a survivor and would have meant
nothing. That is also why the rule's direct guard case could not land with
verdict 1, and why **every E mutant is re-run at the flip** rather than
inherited: the flip changes what the release build compiles, so verdict 1's
kills are re-proved and not assumed.

Nothing here moves a constant. `SeMultiCut` at 0 is the off value DEC-215 asks
to be proved on the tree -- held by a tune-build case and by the bench
signature, as `SeExtend`'s is -- and the four settings beside it are seeds S127
refits; what these break is a guard, a sign or a variable.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file` -- an ambiguous anchor mutates a
site nobody chose, and the tool refuses the whole run before it writes
anything. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent, which no tool can decide.

Ids are never reused: E is this file's own prefix and nothing else in
tools/mutants/ uses it.

E01's anchor was re-pointed at S188's landing and **put back when that step's
H0 removed the rule**: the check extension shared this expression, the two
lines E01 names became four while it was in the tree, and with `src/` back to
the byte the anchor is the two lines again. The mutation never changed. The
twenty-two were re-proved at that landing and again on the reverted tree for
E21 alone, which is the one a mined row carries.
"""

S = "src/search.cpp"

m("E01_extension_on_every_move", S, "search/extension",
  'the ply is added to **every** move instead of the one the verification '
  'called singular, so a node that finds one move much better than the rest '
  'searches the rest a ply deeper too -- the extension inverted into a '
  'node-level deepening, and silent: the tree grows, no count is wrong and '
  'nothing crashes',
  ('    const int child_depth =\n'
   '        depth - 1 + ((moves[i] == tt_move) ? se_extension : 0);',
   '    const int child_depth = depth - 1 + se_extension;'),
  origin="S097")

m("E02_extension_condition_inverted", S, "search/extension",
  'the extension fires where the verification **failed high** -- an '
  'alternative already clears the bar, which is the one case the table move is '
  'not singular in -- and never where it failed low. The comparison is one '
  'token and the two readings are exact opposites',
  ('      if (vscore < singular_beta) {',
   '      if (vscore >= singular_beta) {'),
  origin="S097")

m("E03_extension_dropped", S, "search/extension",
  'the verification runs, its answer is recorded, and no ply is ever added: '
  'the step wired up and switched off in one line, which is how S095\'s J02 '
  'and S098\'s T06 are written and for the same reason -- a feature that costs '
  'its nodes and buys nothing is what an SPRT would price at zero for a reason '
  'that is not the technique',
  ('        se_extension = 1;',
   '        se_extension = 0;'),
  origin="S097")

m("E04_root_gate_dropped", S, "search/extension",
  'the root verifies. The root has to produce a move and its table entry is '
  'the previous iteration\'s own answer, so the verification asks whether the '
  'move the search is about to play is better than the moves it is about to '
  'compare it with -- and it spends a half-depth search of the root to ask',
  ('  if (SE_EXTEND != 0 && ply > 0 && excluded_move == 0 && tt_move != 0 &&',
   '  if (SE_EXTEND != 0 && excluded_move == 0 && tt_move != 0 &&'),
  origin="S097")

m("E05_entry_depth_margin_dropped", S, "search/extension",
  'any entry is deep enough. A move stored by a search two plies deep decides '
  'that a node ten plies deep extends it, which is the condition the published '
  'form states first and the one that keeps the verification from being run on '
  'a guess. **The `(void)` below is a repair and not decoration**: the gate is '
  'the only reader of `tt_entry_depth`, and that local arrived with the fast '
  'check\'s copy-out *after* verdict 1\'s mutation run, so from the landing on '
  'this mutant orphaned it and Release with -Werror refused to compile it -- '
  'stillborn, scored as neither killed nor survived, and invisible until the '
  'mutants were re-run. Verdict 2 re-ran them, saw it, and fixed it the way '
  'S095\'s J-class and S098\'s T-class already do',
  ('      tt_entry != nullptr && depth >= SE_MIN_DEPTH &&\n'
   '      tt_entry_depth >= depth - SE_TT_DEPTH_MARGIN &&',
   '      tt_entry != nullptr && depth >= SE_MIN_DEPTH &&'),
  ('  int se_extension = 0;\n',
   '  (void) tt_entry_depth;\n  int se_extension = 0;\n'),
  origin="S097")

m("E06_bound_type_gate_widened", S, "search/extension",
  'an upper-bound entry is admitted. A TT_ALPHA_NODE score says the position '
  'is worth **at most** that much, so subtracting a margin from it builds a '
  'bar out of a ceiling and the alternatives are measured against a number '
  'nothing certified',
  ('      (tt_entry_type == TT_BETA_NODE || tt_entry_type == TT_PV_NODE) &&',
   '      (tt_entry_type == TT_BETA_NODE || tt_entry_type == TT_PV_NODE ||\n'
   '       tt_entry_type == TT_ALPHA_NODE) &&'),
  origin="S097")

m("E07_recursion_gate_dropped", S, "search/extension",
  'a node searched under an exclusion starts a verification of its own, so the '
  'question becomes what the node is worth without **two** of its moves -- and '
  'the answer is fed back as though it were about the first exclusion alone',
  ('  if (SE_EXTEND != 0 && ply > 0 && excluded_move == 0 && tt_move != 0 &&',
   '  if (SE_EXTEND != 0 && ply > 0 && tt_move != 0 &&'),
  origin="S097")

m("E08_tt_cutoff_gate_dropped", S, "search/extension",
  'the excluded node takes a table cutoff. The entry was written by a search '
  'that was allowed to play the very move the verification is asking the node '
  'to do without, so it answers the question with the thing being asked about '
  '-- and an entry whose lower bound already clears the window fails the '
  'verification high every time, vacuously',
  ('  if (!is_pv && ply > 0 && excluded_move == 0) {',
   '  if (!is_pv && ply > 0) {'),
  origin="S097")

m("E09_store_gate_dropped", S, "search/extension",
  'the excluded node writes its entry. What it computed is the value of a '
  'position with one of its moves removed and the key is the position with the '
  'move in it, so every later probe of that position -- including the one the '
  'verification\'s own parent is about to make -- reads a score that is wrong '
  'by construction and an ordering move that is not the best one',
  ('  if (excluded_move == 0) {\n'
   '    const int to_store = normalize_score(best_so_far, ply);',
   '  if (true) {\n'
   '    const int to_store = normalize_score(best_so_far, ply);'),
  origin="S097")

m("E10_nmp_gate_dropped", S, "search/extension",
  'the excluded node passes. A null-move bound answers the verification with '
  'no alternative searched at all, which is the one thing that search exists '
  'to do -- the node fails high on the pass, the table move is called not '
  'singular, and nothing looked at a move',
  ('      excluded_move == 0 && depth - 1 - null_reduction >= 1 &&',
   '      depth - 1 - null_reduction >= 1 &&'),
  origin="S097")

m("E11_rfp_gate_dropped", S, "search/extension",
  'the excluded node runs reverse futility. A static bound can only fail the '
  'node high, so inside a verification it can only ever answer "not singular", '
  'and it answers without a move having been searched -- at a depth the '
  'parent\'s own reverse futility, with its larger margin, already refused',
  ('  if (!is_pv && !is_in_check && excluded_move == 0 &&\n'
   '      static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&',
   '  if (!is_pv && !is_in_check &&\n'
   '      static_cast<int>(ply) >= RFP_MIN_PLY && depth <= RFP_MAX_DEPTH &&'),
  origin="S097")

m("E12_loop_skip_dropped", S, "search/extension",
  'the exclusion excludes nothing: the move loop searches the move it was '
  'handed, so the verification measures the node **with** its table move in it '
  'and every answer it gives is about a question nobody asked. The condition '
  'is kept as `false` rather than deleted so that the loop keeps its shape and '
  'the mutant is the rule and not a refactor',
  ('    if (moves[i] == excluded_move) { continue; }',
   '    if (false) { continue; }'),
  origin="S097")

m("E13_verification_depth_full", S, "search/extension",
  'the verification runs at the node\'s own remaining depth instead of about '
  'half of it. Every eligible node is then searched twice at full depth, which '
  'is the cost the halving exists to avoid -- and it is invisible to a node '
  'count read as a total, because the tree it produces is a different one and '
  'not simply a bigger one',
  ('      const int verification_depth = (depth - 1) / 2;',
   '      const int verification_depth = depth - 1;'),
  origin="S097")

m("E14_singular_beta_sign", S, "search/extension",
  'the window is a margin **above** the entry\'s score instead of below it, so '
  'the alternatives are asked to clear a bar higher than the move being tested '
  'and almost everything looks singular. A sign on one operator, and the '
  'symptom is a tree that extends everywhere',
  ('    const int singular_beta = tt_entry_score - SE_MARGIN_PER_DEPTH * depth;',
   '    const int singular_beta = tt_entry_score + SE_MARGIN_PER_DEPTH * depth;'),
  origin="S097")

m("E15_no_legal_alternative_return_dropped", S, "search/extension",
  'a node whose only legal move was excluded reports the terminal score again: '
  'mate where it is in check, a draw where it is not. Neither is true -- the '
  'move that answers is on the board and this search set it aside -- and the '
  'draw half is the dangerous one, reading as a fail-high to a verification '
  'whose window sits below zero',
  ('    if (excluded_move != 0) { return alpha0; }\n\n', ''),
  origin="S097")

m("E16_entry_mate_band_gate_dropped", S, "search/extension",
  'a mate score becomes a verification window. A mate score is a distance and '
  'not a value, so a margin subtracted from one is arithmetic across two '
  'units, and the window it produces asks the alternatives to clear a mate '
  'distance',
  ('    if (tt_entry_score < MATE_MIN && tt_entry_score > -MATE_MIN &&\n'
   '        singular_beta > -MATE_MIN) {',
   '    if (singular_beta > -MATE_MIN) {'),
  origin="S097")

m("E18_extend_switch_inverted", S, "search/extension",
  'the block reads its own switch the wrong way round, so the whole feature is '
  'off at the value that ships and on at the one that switches it off. The '
  'switch is what DEC-215 clause 2 asks of a rule whose settings have no off '
  'value between them, and a switch read backwards is a switch that proves '
  'nothing: the H0 path would flip a default and get the candidate back. '
  'Dropping the condition instead is equivalent at the shipped 1 and is '
  'declared here rather than written',
  ('  if (SE_EXTEND != 0 && ply > 0 && excluded_move == 0 && tt_move != 0 &&',
   '  if (SE_EXTEND == 0 && ply > 0 && excluded_move == 0 && tt_move != 0 &&'),
  origin="S097")

m("E17_window_mate_band_gate_dropped", S, "search/extension",
  'the derived window is not checked in its own right. An entry one point '
  'outside the mate band passes the guard above it and still lands inside the '
  'band once the margin comes off, so the verification is run against a window '
  'that reads as a mate distance to every rule below it',
  ('    if (tt_entry_score < MATE_MIN && tt_entry_score > -MATE_MIN &&\n'
   '        singular_beta > -MATE_MIN) {',
   '    if (tt_entry_score < MATE_MIN && tt_entry_score > -MATE_MIN) {'),
  origin="S097")

m("E20_multicut_returns_singular_beta", S, "search/extension",
  'the multicut hands back the **bound** the window was set to instead of the '
  'fail-soft score the verification actually found. The two are the same sign '
  'and within a margin of each other, so no node count and no mate test has to '
  'move: what changes is the value every parent above this node reasons with, '
  'and the record says the difference is the whole rule -- one engine measured '
  'the score form at +6.2 LTC and withdrew the bound form at -0.8. Killed by '
  '"the multicut returns the verification\'s score and searches nothing", '
  'which asserts the two are different numbers at its drive before it asserts '
  'which one came back',
  ('        return vscore;\n',
   '        return singular_beta;\n'),
  origin="S097")

m("E21_multicut_mate_band_gate_dropped", S, "search/extension",
  'the value the multicut returns is not kept out of the mate band. A '
  'half-depth search under a window below the entry\'s score is the instrument '
  'that **misses** a mate, and a mate distance out of one is a distance '
  'nothing proved: the node ends on it, its parent carries it, and a mate is '
  'reported or a real one is hidden behind a fabricated shorter distance. The '
  '`beta > -MATE_MIN` term beside it is S165\'s guard on the node\'s own '
  'window and is a different rule, so it is left in place here. Killed by the '
  'mined row of "pruning does not hide a forced mate"',
  ('      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&\n'
   '                 vscore < MATE_MIN && vscore > -MATE_MIN && '
   'beta > -MATE_MIN) {',
   '      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&\n'
   '                 beta > -MATE_MIN) {'),
  origin="S097")

m("E22_multicut_pv_gate_dropped", S, "search/extension",
  'the multicut fires at a PV node. That line is reported and played, and the '
  'node would end on a score no move of its own was ever searched for -- the '
  'house rule every other bound-returning rule in this function follows, '
  'reverse futility and the null move alike, and this project\'s own choice '
  'where the published record is silent (DEC-221). Killed by the second leg of '
  '"the multicut returns the verification\'s score and searches nothing", the '
  'same drive relabelled',
  ('      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&\n'
   '                 vscore < MATE_MIN && vscore > -MATE_MIN && '
   'beta > -MATE_MIN) {',
   '      } else if (SE_MULTICUT != 0 && vscore >= beta &&\n'
   '                 vscore < MATE_MIN && vscore > -MATE_MIN && '
   'beta > -MATE_MIN) {'),
  origin="S097")

m("E23_multicut_defender_beta_gate_dropped", S, "search/extension",
  'the multicut fires at a node whose own beta sits inside the **negative** '
  'mate band. That node is inside a mate proof as the defender: every score '
  'it can produce clears beta, so a reduced search fails high there by '
  'construction and its word says nothing about whether the mate is held -- '
  'and the node ends on that word instead of searching. S165 found the same '
  'shape in the null move and this guard is its rule applied here. E21 '
  'deliberately leaves this term in place, because a guard on the node\'s own '
  'window is a different rule from the guard on the value the node returns, '
  'so it needs a mutant of its own. Killed by the third leg of "the multicut '
  'returns the verification\'s score and searches nothing", the same drive '
  'with its window moved into the band',
  ('      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&\n'
   '                 vscore < MATE_MIN && vscore > -MATE_MIN && '
   'beta > -MATE_MIN) {',
   '      } else if (SE_MULTICUT != 0 && vscore >= beta && !is_pv &&\n'
   '                 vscore < MATE_MIN && vscore > -MATE_MIN) {'),
  origin="S097")
