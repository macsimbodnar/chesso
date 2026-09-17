"""The re-search depth, S098 verdict 3: the two paths, their guards, the
precedence between them and the site that applies them.

Twelve mutants over one step's rule. Four classes, and each is a bug this engine
could plausibly ship:

  a condition inverted   the margin read the wrong way round, so the path that
                         should verify harder verifies less and the other way
                         about. Silent everywhere -- no crash, no wrong node
                         count, only rating
  a guard dropped        the deeper path without its reduction guard is the
                         bare form the published record measured negative; the
                         shallower path without its arithmetic guard re-searches
                         at the depth it just searched, which is the reduced
                         search run twice for nothing
  a clamp wrong          the cap raised into an even-deeper search or lowered
                         a ply so the deeper path is clamped away, the
                         shallower path dropping two plies, the floor written
                         at 0 so a re-search can reach quiescence
  the site               the recursion ignoring the rule, or the call feeding
                         it the window where it wants the fail-soft best. Both
                         are invisible to a case that replays the rule on the
                         numbers the node recorded, which is why the rule echoes
                         its own base out and why the child's table entry is
                         read

**One mutant that is not here, declared and not inferred.** A mutant that only
deletes `if (depth > child_depth + 1) { depth = child_depth + 1; }` is
**equivalent**: the three branches produce `child_depth` and its two
neighbours, so nothing the rule can build reaches the cap, and a clamp that
makes a class of bugs unobservable is a clamp doing its job. `D06` and `D11`
are the killable forms of the same bug -- the bound moved up together with the
branch that would then reach it, and the bound moved down on its own -- and
they are what the cap is tested through. The floor at 1 is
reachable from the rule's declared domain even though the engine's own clamps
never send it there, so `D10` is an ordinary mutant.

This is DEC-141 clause 2 for verdict 3's rule: a new reduction rule ships with
a direct guard test and a mutant that test kills. Every anchor here is a guard,
a sign or a bound, never a constant moved -- moving `LmrShallowerMargin` to 0
or `LmrDeeperMinReduction` to its range top is the off value the bisection
protocol uses and not a bug.

**FOUR OF THE TWELVE ARE EQUIVALENT ON THE SHIPPED CONFIGURATION, AND THAT IS A
FACT ABOUT THE CONFIGURATION AND NOT ABOUT THE BUGS.** Verdict 3's SPRT read H0
and its pre-registered bisection's leg 1 ships `LmrShallowerMargin` at 0, where
the shallower branch is never taken: the site requires `score > alpha` and the
branch asks for `score < alpha + 0`. A mutant that only changes that branch
therefore changes nothing any input reaches -- `D04`, `D07` -- and so does one
that only reorders it against the deeper branch (`D05`) or moves a floor the
remaining two outcomes never touch (`D10`). They are declared `equivalent`
here, **qualified by the configuration and never deleted**: leg 2 of the same
bisection restores the margin to 47, and each of the four is an ordinary
killable mutant again the moment it does, with the case that killed it named in
the step file. The declaration is what `tools/mutation_check.py` requires --
equivalence is argued by a person, never inferred from a green suite -- and the
tool's second oracle holds the argument to account: a bench signature that
moved makes the verdict `survived` whatever this file declares, so each of the
four is run and its signature read like any other.

`D02` is the one that moves in the other direction. At a margin of 0 inverting
the shallower condition makes the path fire at **every** site with a reduction
of at least 2 rather than at none, so the case asserting the path fires nowhere
is what kills it, and the kill is stronger than the one it had at 47.

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
  ('             score > best + LMR_DEEPER_MARGIN) {',
   '             score < best + LMR_DEEPER_MARGIN) {'),
  origin="S098")

m("D02_shallower_inverted", S, "search/reduction",
  'the shallower path fires where the score beat alpha by **more** than the '
  'margin, so the moves that cleared the window widest are the ones verified '
  'least -- the inverse of the published shape, and of the sentence the rule '
  'is written from',
  ('  if (reduction >= 2 && score < alpha + LMR_SHALLOWER_MARGIN) {',
   '  if (reduction >= 2 && score > alpha + LMR_SHALLOWER_MARGIN) {'),
  origin="S098")

m("D03_deeper_guard_dropped", S, "search/reduction",
  'the deeper path fires at any reduction, which is exactly the bare form the '
  'published record separates from the guarded one: the bare form measured '
  'negative where the guarded form measured positive, and this step ships the '
  'guard for that reason alone',
  ('  } else if (reduction >= LMR_DEEPER_MIN_REDUCTION &&\n'
   '             score > best + LMR_DEEPER_MARGIN) {',
   '  } else if (score > best + LMR_DEEPER_MARGIN) {'),
  origin="S098")

m("D04_shallower_guard_dropped", S, "search/reduction",
  'the shallower path fires at a reduction of 1, where `child_depth - 1` **is** '
  'the reduced depth: the re-search becomes the reduced search run a second '
  'time, so a move that beat alpha is confirmed by the very search that was '
  'not believed. The one bug the strict inequality exists against. '
  '**EQUIVALENT while `LmrShallowerMargin` is 0** (leg 1): the condition the '
  'guard sits on is false at every site, so dropping the guard admits '
  'nothing. Killable again at leg 2\'s margin of 47, where '
  '"LmrShallowerMargin decides whether a re-search that only just beat alpha '
  'goes a ply shallower" reads it at a reduction of 1',
  ('  if (reduction >= 2 && score < alpha + LMR_SHALLOWER_MARGIN) {',
   '  if (score < alpha + LMR_SHALLOWER_MARGIN) {'),
  expected="equivalent", origin="S098")

m("D05_precedence_swapped", S, "search/reduction",
  'the deeper path is tested first, so it wins the region where both '
  'conditions hold -- which is every scout node whose fail-soft best sits more '
  'than a margin below alpha, the common case and not a corner. The rule then '
  'searches deepest exactly where the score only just cleared the window. '
  '**EQUIVALENT while `LmrShallowerMargin` is 0** (leg 1): the region where '
  'both conditions hold is empty, so the two orders decide every site the '
  'same way. Killable again at leg 2\'s margin of 47, where '
  '"LmrShallowerMargin decides the region where both re-search paths could '
  'fire" reads it',
  ('  if (reduction >= 2 && score < alpha + LMR_SHALLOWER_MARGIN) {\n'
   '    depth = child_depth - 1;\n'
   '  } else if (reduction >= LMR_DEEPER_MIN_REDUCTION &&\n'
   '             score > best + LMR_DEEPER_MARGIN) {\n'
   '    depth = child_depth + 1;\n'
   '  }',
   '  if (reduction >= LMR_DEEPER_MIN_REDUCTION &&\n'
   '      score > best + LMR_DEEPER_MARGIN) {\n'
   '    depth = child_depth + 1;\n'
   '  } else if (reduction >= 2 && score < alpha + LMR_SHALLOWER_MARGIN) {\n'
   '    depth = child_depth - 1;\n'
   '  }'),
  expected="equivalent", origin="S098")

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

m("D07_shallower_two_plies", S, "search/reduction",
  'the shallower path drops two plies, so at a reduction of 2 -- a third of '
  'all re-search sites by this step\'s own census -- the re-search is the '
  'reduced search repeated, and at a reduction of 3 it is shallower still than '
  'the search that was not believed. **EQUIVALENT while `LmrShallowerMargin` '
  'is 0** (leg 1): the branch it rewrites is never taken. Killable again at '
  'leg 2\'s margin of 47, where the shallower path\'s own case and the '
  'inequality case both read it',
  ('    depth = child_depth - 1;\n'
   '  } else if (reduction >= LMR_DEEPER_MIN_REDUCTION &&',
   '    depth = child_depth - 2;\n'
   '  } else if (reduction >= LMR_DEEPER_MIN_REDUCTION &&'),
  expected="equivalent", origin="S098")

m("D08_site_ignores_the_rule", S, "search/reduction",
  'the recursion re-searches at `child_depth` and the rule is computed and '
  'thrown away, which is the whole step wired up and switched off in one line. '
  'The `(void)` orphans the local, which the Release build this tool runs '
  "refuses under -Werror; search.py's header carries the reason in full",
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
  'guard rather than a golden. The `(void)` orphans the parameter, which the '
  "Release build this tool runs refuses under -Werror; search.py's header "
  'carries the reason in full',
  ('             score > best + LMR_DEEPER_MARGIN) {',
   '             score > alpha + LMR_DEEPER_MARGIN) {'),
  ('  int depth = child_depth;',
   '  (void) best;\n  int depth = child_depth;'),
  origin="S098")

m("D10_floor_dropped", S, "search/reduction",
  'the floor is written at 0 instead of 1, so a shallower re-search at a child '
  'depth of 1 runs at depth 0 -- which is quiescence, and quiescence sees only '
  'captures and will report that a quiet move is fine. The reduced child '
  'reaching depth 0 is a bug the published record prices in the tens of Elo, '
  'and the engine reaches this corner only if a later step moves the '
  "reduction's own clamp -- which is the case for keeping the floor at all. "
  '**EQUIVALENT while `LmrShallowerMargin` is 0** (leg 1): the floor is '
  'reached only through a shallower re-search at a child depth of 1, and the '
  'two outcomes left -- `child_depth` and `child_depth + 1`, at a site whose '
  'reduction of at least 1 forces `child_depth >= 2` -- never go below it. '
  'Killable again at leg 2\'s margin of 47, where "the re-search depth stays '
  'inside its cap, floor and inequality" drives the corner directly',
  ('  if (depth < 1) { depth = 1; }',
   '  if (depth < 0) { depth = 0; }'),
  expected="equivalent", origin="S098")

m("D11_cap_one_ply_low", S, "search/reduction",
  'the cap is one ply low, so it clamps the deeper path away and the rule '
  'silently becomes the shallower path alone -- half a verdict measured as the '
  'whole of it. The other bug a bound takes, and the one an off-by-one is: '
  'invisible everywhere except at the branch it was written for',
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
