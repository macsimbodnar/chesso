"""The re-search depth, S098 verdict 3: the one path it has, its guard, its
base, its cap and the site that applies them.

Seven mutants over one step's rule. Four classes, and each is a bug this engine
could plausibly ship:

  a condition inverted   the margin read the wrong way round, so the moves
                         verified hardest are the ones the node already has
                         something better than. Silent everywhere -- no crash,
                         no wrong node count, only rating
  a guard dropped        the deeper re-search without its reduction guard is
                         the bare form the published record measured negative,
                         where the guarded form measured positive
  a clamp wrong          the cap raised into an even-deeper search, or lowered
                         a ply so the path is clamped away
  the site               the recursion ignoring the rule, or the call feeding
                         it the window where it wants the fail-soft best. Both
                         are invisible to a case that replays the rule on the
                         numbers the node recorded, which is why the rule echoes
                         its own base out and why the child's table entry is
                         read

**FIVE MUTANTS LEFT WITH THE BRANCH THEY BROKE, 2026-09-18.** Verdict 3 shipped
a second path -- a ply *off* the re-search where the reduced score beat alpha by
less than an `LmrShallowerMargin` of its own -- and the pair measured
`Elo -9.97 +/- 7.56` over 4496 games. The pre-registered bisection's leg 1
switched that path off and read H1, `Elo 5.75 +/- 4.37` over 14510 games, so the
path was removed outright. `D02_shallower_inverted`, `D04_shallower_guard_dropped`
and `D07_shallower_two_plies` mutated the branch itself;
`D05_precedence_swapped` reordered it against the branch that remains, and there
is no longer an order to get wrong; `D10_floor_dropped` moved a floor only that
branch could reach, and the floor went with it. Four of the five were declared
`equivalent` here while the leg held the margin at 0 and `D02` was the one it
still killed; none of them is a bug this tree can have, because their anchors do
not exist.

**One mutant that is not here, declared and not inferred.** A mutant that only
deletes `if (depth > child_depth + 1) { depth = child_depth + 1; }` is
**equivalent**: the rule answers `child_depth` or one more, so nothing it can
build reaches the cap, and a clamp that makes a class of bugs unobservable is a
clamp doing its job. `D06` and `D11` are the killable forms of the same bug --
the bound moved up together with the branch that would then reach it, and the
bound moved down on its own -- and they are what the cap is tested through.
**The rule's lower bound is now in that same class and has no mutant at all.**
Since the shallower path went, the rule answers `child_depth` or one more and
`child_depth` is at least 1 by precondition, so no input can drive the result
below the bound and a mutant of the bound alone is unobservable. It is asserted
rather than clamped for that reason, and said here so that a green suite is not
what the claim rests on.

This is DEC-141 clause 2 for verdict 3's rule: a new reduction rule ships with
a direct guard test and a mutant that test kills. Every anchor here is a guard,
a sign or a bound, never a constant moved -- moving `LmrDeeperMinReduction` to
its range top is the off value the bisection protocol used and not a bug.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. A mutant's several pairs are applied
together as one bug. `expected` is "killed" unless a person has argued the
mutant is behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: a new mutant takes the next free number across every file
here, and this file opens the `D` prefix because `T` is verdict 2's and `L` was
verdict 1's, whose mutants left with the term DEC-213 removed.
"""

S = "src/search.cpp"

m("D01_deeper_inverted", S, "search/reduction",
  'the deeper path fires where the reduced score is **below** the fail-soft '
  'best by the margin instead of above it, so the moves verified hardest are '
  'the ones the node already has something better than',
  ('      score > best + LMR_DEEPER_MARGIN) {',
   '      score < best + LMR_DEEPER_MARGIN) {'),
  origin="S098")

m("D03_deeper_guard_dropped", S, "search/reduction",
  'the deeper path fires at any reduction, which is exactly the bare form the '
  'published record separates from the guarded one: the bare form measured '
  'negative where the guarded form measured positive, and this step ships the '
  'guard for that reason alone. The `(void)` orphans the parameter -- with the '
  'shallower path gone this guard is the only thing in the rule that reads it '
  'outside an assert -- and the Release build this tool runs refuses that under '
  "-Werror; search.py's header carries the reason in full",
  ('  if (reduction >= LMR_DEEPER_MIN_REDUCTION &&\n'
   '      score > best + LMR_DEEPER_MARGIN) {',
   '  if (score > best + LMR_DEEPER_MARGIN) {'),
  ('  int depth = child_depth;',
   '  (void) reduction;\n  int depth = child_depth;'),
  origin="S098")

m("D06_deeper_two_plies", S, "search/reduction",
  'the cap is raised and the deeper path becomes an even-deeper search, two '
  'plies past the reduced move rather than one -- which makes the child deeper '
  'than its parent and is the device the published record calls do-even-deeper, '
  'one this step does not ship and has measured nothing about. Two edits and '
  'one bug: the branch alone is caught by the cap, which is what the cap is '
  'for, so the killable form of "the cap is wrong" has to move both',
  ('    depth = child_depth + 1;\n  }\n\n'
   '  if (depth > child_depth + 1) { depth = child_depth + 1; }',
   '    depth = child_depth + 2;\n  }\n\n'
   '  if (depth > child_depth + 2) { depth = child_depth + 2; }'),
  origin="S098")

m("D08_site_ignores_the_rule", S, "search/reduction",
  'the recursion re-searches at `child_depth` and the rule is computed and '
  'thrown away, which is the whole step wired up and switched off in one line. '
  'The local survives without a `(void)`: the probe block reads it in the '
  'probing instantiation, so no build refuses it',
  ('        score = -negamax_at<false>(-alpha - 1, -alpha, research.depth, '
   'ply + 1,\n'
   '                                   game, state, moves[i], again.is_pv,\n'
   '                                   again.cut_node);',
   '        score = -negamax_at<false>(-alpha - 1, -alpha, child_depth, '
   'ply + 1,\n'
   '                                   game, state, moves[i], again.is_pv,\n'
   '                                   again.cut_node);'),

  origin="S098")

m("D09_deeper_margin_off_alpha", S, "search/reduction",
  'the deeper margin is measured from the window instead of from what the node '
  'has already found, which is the published re-basing undone -- the record '
  'moved this margin from alpha onto the best value and kept it there through '
  'five years of churn. `best` is at or below alpha at every site, so the '
  'mutant is the **stricter** rule and fires on a thinner class; it is a bug '
  'with almost no node-count reach, which is exactly why it needs a direct '
  'guard rather than a golden. It needs no `(void)`: the rule echoes `best` '
  'back out in its return, so the parameter is read whatever the condition '
  'compares against',
  ('      score > best + LMR_DEEPER_MARGIN) {',
   '      score > alpha + LMR_DEEPER_MARGIN) {'),
  origin="S098")

m("D11_cap_one_ply_low", S, "search/reduction",
  'the cap is one ply low, so it clamps the deeper path away and the rule '
  'silently becomes what it replaced: a re-search at `child_depth` everywhere, '
  'which is the engine before this verdict wearing the verdict\'s code. The '
  'other bug a bound takes, and the one an off-by-one is: invisible everywhere '
  'except at the branch it was written for, and its bench is the off tree\'s',
  ('  if (depth > child_depth + 1) { depth = child_depth + 1; }',
   '  if (depth > child_depth) { depth = child_depth; }'),
  origin="S098")

m("D12_site_rebases_on_alpha", S, "search/reduction",
  'the **call** hands the rule the node\'s window where the fail-soft best '
  'belongs, so the deeper margin is measured from alpha -- the published '
  're-basing undone at the site rather than inside the rule, and a different '
  'bug from D09 because the rule itself is still right. It is the one the '
  "Tier-1 fast check applied and the whole suite stayed green on: a case that "
  'replays the rule on the numbers the node recorded moves both sides of its '
  'comparison together and cannot see it. `lmr_research_depth` echoes the base '
  'it used back out for exactly this, and "the node measures the deeper margin '
  'from its own fail-soft best" is what reads it',
  ('        const research_decision_t research = lmr_research_depth(\n'
   '            child_depth, reduction, score, alpha, best_so_far);',
   '        const research_decision_t research = lmr_research_depth(\n'
   '            child_depth, reduction, score, alpha, alpha);'),
  origin="S098")
