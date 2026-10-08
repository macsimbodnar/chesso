"""No rule tests on a quiet past late move pruning's count, S269.

Once late move pruning has set its flag, a quiet is skipped unless it gives
check whatever futility, history pruning and quiet SEE say, and S091's exchange
test is read on a quiet only behind a reduction guard such a quiet cannot pass.
S269 stops asking all four of a quiet in that state. Three mutants: the change
undone, and the change reaching past the quiets into S091's two rules, which
late move pruning does not end.

  LQ01  the three rules asked of a late quiet again   S269's parent; the same
                                                       tree node for node, so
                                                       the bench cannot see it
  LQ02  the extra ply's exchange test skipped for      a late losing capture
        every move past the count                      loses its extra ply
  LQ03  the capture rule skipped past the count        a late capture that
                                                       loses more than the
                                                       margin is searched

Each is killed by its own case in tests/test_search.cpp: "a quiet past late
move pruning's count carries the late move mark" (LQ01), "a capture past late
move pruning's count still takes the extra ply" (LQ02) and "a capture past late
move pruning's count is still skipped by the capture rule" (LQ03).

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide. LQ01 is not
equivalent in that sense: the probed node records a different rule, and the
case reads it.

Ids are never reused: LQ is this file's own prefix, and no file in
tools/mutants/ has used it before.
"""

S = "src/search.cpp"

m("LQ01_late_quiet_rules_asked", S, "search/pruning",
  'futility, history pruning and quiet SEE are asked of a quiet past late '
  'move pruning\'s count again: the engine searches the same tree, and the '
  'probed node records the first of the three that fired instead of the late '
  'move mark',
  ('    if (may_prune && is_quiet && !skip_quiets) {',
   '    if (may_prune && is_quiet) {'),
  origin="S269")

m("LQ02_late_skip_reaches_extra_ply", S, "search/reduction",
  'the exchange test behind S091\'s extra ply is skipped for every move past '
  'late move pruning\'s count, captures included, so a late capture that '
  'loses material is searched a ply deeper than it should be',
  ('        !MOVE_PROMOTED(moves[i]) && !(skip_quiets && is_quiet) &&',
   '        !MOVE_PROMOTED(moves[i]) && !skip_quiets &&'),
  origin="S269")

m("LQ03_late_skip_reaches_capture_rule", S, "search/pruning",
  'S091\'s capture rule is not asked once late move pruning has set its flag, '
  'so a late capture that loses more than the margin is searched',
  ('    if (may_prune && is_capture) {',
   '    if (may_prune && is_capture && !skip_quiets) {'),
  origin="S269")
