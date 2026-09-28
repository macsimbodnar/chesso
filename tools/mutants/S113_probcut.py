"""ProbCut, S113.

Fifteen mutants over the one block this step adds. Every one is a guard
dropped, a test inverted or widened, a claim written with the wrong type or
depth, or the cut not taken -- never a constant moved; `ProbCut` at 0 is the
off value DEC-215 asks to be proved on the tree and is not a bug. Each is
killed by a case in tests/test_search.cpp's "search: pruning and reduction
guards" suite, named per mutant:

  B01  the margin not added             "probcut ends a node whose good
                                         capture clears beta by the margin"
  B02  the PV guard dropped             "probcut does not run at a PV node"
  B03  the table skip inverted          "probcut ends a node whose good
                                         capture clears beta by the margin"
  B04  the defender mate-band guard     "probcut does not run with beta in
       dropped                           the mate band"
  B05  the store written exact          "a probcut cutoff is stored as a lower
                                         bound at the shallow depth"
  B06  the store at the node's depth    the same case
  B07  the cut recorded, not taken      "probcut ends a node whose good
                                         capture clears beta by the margin"
  B08  the root guard dropped           "probcut does not run at the root"
  B09  the in-check guard dropped       "probcut does not run in check"
  B10  the bar's mate-band guard        "probcut does not run with beta in
       dropped                           the mate band"
  B11  the skip read on any bound       "probcut skips a node the table
                                         already bounds under its bar"
  B12  the skip read at any depth       the same case
  B13  the minimum depth a ply short    "probcut does not run below its
                                         minimum depth"
  B14  the excluded-move guard dropped  "probcut does not run while a move is
                                         excluded"
  B15  the child-depth guard dropped    declared equivalent in the release
                                         build; killed in the tune build by
                                         "probcut does not search a capture's
                                         child at depth zero"

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: B is this file's own prefix, and no file in
tools/mutants/ has ever used it, on any branch.
"""

S = "src/search.cpp"

GATE = ("  if (PROBCUT != 0 && !is_pv && !is_in_check && ply > 0 && "
        "excluded_move == 0 &&\n"
        "      depth >= PROBCUT_MIN_DEPTH && probcut_child_depth >= 1 &&\n"
        "      beta > -MATE_MIN && probcut_beta < MATE_MIN) {")


def gate_with(old_term, new_term):
    """The gate with one term replaced; the term must occur in it once."""
    assert GATE.count(old_term) == 1, old_term
    return (GATE, GATE.replace(old_term, new_term))


m("B01_probcut_margin_dropped", S, "search/pruning",
  'the bar is beta itself, so every capture whose shallow search merely '
  'reaches beta ends the node: ProbCut with no statistical margin is a '
  'reduced search answering for a full one',
  ('  const int probcut_beta = beta + PROBCUT_MARGIN;',
   '  const int probcut_beta = beta + 0 * PROBCUT_MARGIN;'),
  origin="S113")

m("B02_probcut_pv_gate_dropped", S, "search/pruning",
  'the block runs at a PV node, whose line is reported and played, and hands '
  'it a bound from a shallow search in place of a score',
  gate_with("!is_pv && ", ""),
  origin="S113")

m("B03_probcut_tt_skip_inverted", S, "search/pruning",
  'the table skip reads the wrong way round: the block runs only where the '
  'table already says it cannot succeed and never where it can',
  ('    if (!tt_says_no) {', '    if (tt_says_no) {'),
  origin="S113")

m("B04_probcut_defender_gate_dropped", S, "search/pruning",
  'beta at or below -MATE_MIN is let in: a defender inside a mate proof, '
  'where every shallow score clears the bar and the shallow search is the '
  'instrument that misses the mate (S165)',
  gate_with("beta > -MATE_MIN && ", ""),
  origin="S113")

m("B05_probcut_store_exact", S, "search/transposition",
  'the cut is stored as an exact score, a claim no fail-high establishes, '
  'read back by every later probe of the position whatever its window',
  ('normalize_score(value, ply), TT_BETA_NODE, pc_moves[j],',
   'normalize_score(value, ply), TT_PV_NODE, pc_moves[j],'),
  origin="S113")

m("B06_probcut_store_full_depth", S, "search/transposition",
  'the cut is stored at the node\'s own depth, so a later full-depth probe '
  'takes the shallow search\'s word for a deep one',
  ('          tt_store_entry(state->tt, &game->board, probcut_depth,',
   '          tt_store_entry(state->tt, &game->board, depth,'),
  origin="S113")

m("B07_probcut_no_return", S, "search/pruning",
  'the cut is stored and recorded and the node searches on regardless, so '
  'the block costs its searches and saves nothing',
  ('          return value;\n', '          (void)value;\n'),
  origin="S113")

m("B08_probcut_root_gate_dropped", S, "search/pruning",
  'the block runs at the root, which has to produce a move and gets a bound',
  gate_with("ply > 0 && ", ""),
  origin="S113")

m("B09_probcut_in_check_gate_dropped", S, "search/pruning",
  'the block runs in check, where the move list is evasions and a capture '
  'clearing a bar says nothing about the replies it skipped',
  gate_with("!is_in_check && ", ""),
  origin="S113")

m("B10_probcut_attacker_gate_dropped", S, "search/pruning",
  'the bar may sit inside the positive mate band, so a shallow search is '
  'asked to prove a mate and its fail-high claims one',
  gate_with(" && probcut_beta < MATE_MIN", ""),
  origin="S113")

m("B11_probcut_tt_skip_any_bound", S, "search/pruning",
  'a lower bound under the bar is read as proof the block cannot succeed, '
  'which it is not: a lower bound says nothing about how high the node goes',
  ('(tt_entry_type == TT_ALPHA_NODE || tt_entry_type == TT_PV_NODE);',
   '(tt_entry_type != 255);'),
  origin="S113")

m("B12_probcut_tt_skip_any_depth", S, "search/pruning",
  'an entry shallower than the shallow search is taken to speak for it',
  ('tt_entry != nullptr && tt_entry_depth >= probcut_depth &&',
   'tt_entry != nullptr && tt_entry_depth >= 0 &&'),
  origin="S113")

m("B13_probcut_min_depth_off_by_one", S, "search/pruning",
  'the block runs a ply below its minimum depth',
  gate_with("depth >= PROBCUT_MIN_DEPTH ", "depth >= PROBCUT_MIN_DEPTH - 1 "),
  origin="S113")

m("B14_probcut_excluded_gate_dropped", S, "search/pruning",
  'the block runs inside S097\'s verification, whose capture loop then '
  'searches the move the verification set aside and answers it with a bound',
  gate_with("excluded_move == 0 &&", "true &&"),
  origin="S113")

# Equivalent in the build this tool runs, and argued rather than inferred: the
# release build compiles ProbCutMinDepth 8 and ProbCutDepthOffset 5 as
# constants, so at every depth that passes `depth >= PROBCUT_MIN_DEPTH` the
# child depth is at least 3 and the dropped term was already true. The tune
# build, where the two are variables, is where the term is live; its case
# "probcut does not search a capture's child at depth zero" was observed red
# under this mutant by hand (.tuning/coord/S113_b15_red.log).
m("B15_probcut_child_depth_gate_dropped", S, "search/pruning",
  'the capture\'s child may be searched at depth 0, which is quiescence: the '
  'preliminary asked twice and the pair collapsed to one search',
  gate_with("probcut_child_depth >= 1 &&\n      ", ""),
  expected="equivalent",
  origin="S113")
