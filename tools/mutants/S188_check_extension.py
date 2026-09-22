"""The check extension, its budget and the two gates on it. S188.

Twelve mutants over one rule that is four lines, and the split is DEC-141's:
**one per rule, one per gate**. The rule is a ply added to a move that gives
check; the gates are the switch, the root, the explosion cap, and the scan the
rule reads the check off.

WHAT EACH CLASS IS.

  the ply          added to every move, to no move, or two at a time. A ply
                   landing on the wrong move is the silent failure this class
                   of rule has -- no crash, no wrong node count, only rating --
                   and one ply is the wiki's own form, so a second one is a
                   different technique and not a setting

  the budget       S097 and this step both want to add a ply and the node has
                   one to give. A sum is the natural bug: both rules fire on
                   the table's move when it gives check, and the move is then
                   searched two plies deeper than the node's own depth allows.
                   **X07 is declared equivalent for the release build** and the
                   reason is arithmetic, not an argument about behaviour: S097
                   wants `depth >= SeMinDepth` 10 and DEC-228's third form
                   wants `depth <= CheckExtMaxDepth` 8, so at the shipped seeds
                   no node reaches both rules, the two terms are never both
                   non-zero, and a sum and a `?:` compile to the same tree.
                   The bench signature is the cross-check: a mutant declared
                   equivalent whose signature moves is scored `survived` and
                   flagged

  the scan         `is_check_move` is hardcoded false on a capture in
                   src/search.cpp -- S107's own comment warns against reusing
                   it for capture logic -- so a rule that reads it extends no
                   checking capture at all, and the shared scan it reads
                   instead has a window term of its own that can be dropped
                   without the reduction exemption noticing

  the gates        the switch read the wrong way round, the root, the
                   explosion cap, the exchange gate and the horizon
                   restriction. The cap is the one that keeps a chain of
                   checks finite -- an extended child keeps its parent's
                   remaining depth, so along a chain only the ply rises -- and
                   the last two are the two forms DEC-228 measured the step
                   into: dropping either hands back a form whose numbers
                   missed the decision's own bar, which is a fact about the
                   tree and not about the rule's shape, so each has a case
                   that reads it at a node

  the confusion    a node **in** check against a move that **gives** check.
                   They are one token apart at the site, the engine already has
                   a rule about the first (`may_reduce`), and extending every
                   move at a node in check is a different published technique

WHY NO MUTANT MOVES A CONSTANT. `CheckExtPlyFactor` is a seed S127 refits and
`CheckExtend` at 0 is the off value DEC-215 asks to be proved on the tree --
held by a tune-build case and by the bench signature, as `SeExtend`'s is. What
these break is a guard, a variable or an arithmetic form.

WHAT THE RELEASE BUILD CAN AND CANNOT HOLD. tools/mutation_check.py compiles
Release only, so every killer named here is a case that runs there, X07
excepted. **Three** cases of this step are `CHESSO_TUNE`-only: the off value,
the chain claim read as a node-count identity against the rule switched off,
and the shared budget, which needs `CheckExtMaxDepth` raised into S097's range
before any node can reach both rules. X04's chain half is covered by the
release case that drives the cap's own boundary; X07's is covered by the tune
build and, in Debug, by the site's own
`assert(child_depth >= depth - 1 && child_depth <= depth)`.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file. `old` must occur exactly once in `file` -- an ambiguous anchor mutates a
site nobody chose, and the tool refuses the whole run before it writes
anything. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent, which no tool can decide.

WHAT KILLS EACH, all in `tests/test_search.cpp`'s "search: pruning and
reduction guards" suite:

  X01, X02, X08, X09, X10   "a checking move is extended and no other move is"
  X01                       "every checking move at a node is extended, not
                             only the first"
  X03                       "the root extends no checking move"
  X04                       "no checking move is extended past the ply cap"
  X05, X06                  "a capture that gives check is extended too"
  X11                       "a checking move the exchange calls unsafe is not
                             extended"
  X12                       "no checking move is extended above the horizon
                             restriction"
  X07                       "a move that is both singular and checking is
                             extended one ply" -- **tune build only**, and in
                             Debug the site's own assertion; equivalent in the
                             release build the tool compiles, for the reason
                             under "the budget" above

Ids are never reused: X is this file's own prefix and nothing else in
tools/mutants/ uses it.
"""

S = "src/search.cpp"

m("X01_extension_on_every_move", S, "search/check_extension",
  'the ply is added to **every** move at a node inside the window and not '
  'only to a move that gives check, which is a node-level deepening wearing '
  'the name of an extension: the tree grows, no count is wrong and nothing '
  'crashes',
  ('    const bool gives_check = is_capture ? capture_is_check : '
   'is_check_move;',
   '    const bool gives_check = true;'),
  origin="S188")

m("X02_extension_dropped", S, "search/check_extension",
  'the window is computed, the scan is paid on every capture inside it, and '
  'no ply is ever added: the step wired up and switched off in one token, '
  'which is how S095\'s J02, S097\'s E03 and S098\'s T06 are written and for '
  'the same reason -- a feature that costs its nodes and buys nothing is what '
  'an SPRT would price at zero for a reason that is not the technique',
  ('                                 ? 1\n'
   '                                 : ((moves[i] == tt_move) ? se_extension '
   ': 0));',
   '                                 ? 0\n'
   '                                 : ((moves[i] == tt_move) ? se_extension '
   ': 0));'),
  origin="S188")

m("X03_root_gate_dropped", S, "search/check_extension",
  'the root extends. A root move that gives check is searched a ply deeper '
  'than its siblings, so the move the search is about to play is chosen off a '
  'deeper look at one class of move -- the comparison the root exists to make, '
  'taken at two depths',
  ('      CHECK_EXTEND != 0 && ply > 0 && depth <= CHECK_EXT_MAX_DEPTH &&',
   '      CHECK_EXTEND != 0 && depth <= CHECK_EXT_MAX_DEPTH &&'),
  origin="S188")

m("X04_ply_cap_dropped", S, "search/check_extension",
  'the explosion cap goes and the MAX_PLY walls are the only thing left. An '
  'extended child keeps its parent\'s remaining depth, so along a chain of '
  'checks the depth never falls and only the ply rises -- the step file\'s own '
  'hazard, and the one the accepts asks to see observed rather than argued',
  ('      CHECK_EXTEND != 0 && ply > 0 && depth <= CHECK_EXT_MAX_DEPTH &&\n'
   '      static_cast<int>(ply) < CHECK_EXT_PLY_FACTOR * depth;',
   '      CHECK_EXTEND != 0 && ply > 0 && depth <= CHECK_EXT_MAX_DEPTH;'),
  origin="S188")

m("X05_quiet_checks_only", S, "search/check_extension",
  'the rule reads `is_check_move`, which src/search.cpp hardcodes **false** on '
  'a capture, so a capture that gives check -- the most forcing move there is '
  '-- is never extended and the rule silently covers half of what it says it '
  'does. S107 left a comment at that variable warning against exactly this '
  'reuse',
  ('    const bool gives_check = is_capture ? capture_is_check : is_check_move;',
   '    const bool gives_check = is_check_move;'),
  origin="S188")

m("X06_capture_scan_window_dropped", S, "search/check_extension",
  'the shared scan loses the extension\'s own term, so a capture is only '
  'scanned where S091\'s two rules were already going to ask. At a node where '
  'neither is about to act -- a PV node, a node in the mate band, an early '
  'move -- a checking capture then reads as not giving check and is not '
  'extended. The reduction exemption is unmoved, which is what makes this '
  'invisible to every case S091 wrote',
  ('        (prune_rule != PRUNE_NONE || see_loses_material ||\n'
   '         check_extension_safe) &&',
   '        (prune_rule != PRUNE_NONE || see_loses_material) &&'),
  origin="S188")

m("X07_budget_is_a_sum", S, "search/check_extension",
  'the two extension rules add instead of sharing the node\'s one ply: a move '
  'that is both the table\'s singular move and a checking move is searched '
  '**two** plies deeper than this node\'s remaining depth allows. The budget '
  'is what makes a line finite together with the cap, and a sum is what the '
  'obvious refactor of this expression produces',
  ('    const int child_depth = depth - 1 +\n'
   '                            ((check_extension_safe && gives_check)\n'
   '                                 ? 1\n'
   '                                 : ((moves[i] == tt_move) ? se_extension '
   ': 0));',
   '    const int child_depth = depth - 1 +\n'
   '                            ((check_extension_safe && gives_check) ? 1 '
   ': 0) +\n'
   '                            ((moves[i] == tt_move) ? se_extension : 0);'),
  origin="S188", expected="equivalent")

m("X08_extend_switch_inverted", S, "search/check_extension",
  'the switch is read the wrong way round, so the rule runs at its off value '
  'and not at the shipped one. The H0 path of this step\'s verdict is a flip '
  'of that default, so a gate the wrong way round hands the candidate back '
  'under the name of the parent -- S097\'s E18 in this step\'s shape',
  ('      CHECK_EXTEND != 0 && ply > 0 &&',
   '      CHECK_EXTEND == 0 && ply > 0 &&'),
  origin="S188")

m("X09_node_in_check_not_move", S, "search/check_extension",
  'the rule extends every move at a node that **is** in check instead of the '
  'move that **gives** one. They are one variable apart at the site, the '
  'engine already has a rule about the first (`may_reduce` refuses to reduce '
  'there), and check-evasion extensions are a different published technique '
  'with a different cost',
  ('    const bool gives_check = is_capture ? capture_is_check : '
   'is_check_move;',
   '    const bool gives_check = is_in_check;'),
  origin="S188")

m("X10_extends_two_plies", S, "search/check_extension",
  'a checking move is extended **two** plies. One ply is the form the wiki '
  'states and every engine the step file traces carries; two is a fractional-'
  'ply mechanism\'s worth of depth handed out by an integer, and along a chain '
  'of checks it makes the remaining depth **rise**. **Killed by assertion, hand-observed on this tree**: with it applied, `test_search --test-case="a checking move is extended and no other move is"` fails at `REQUIRE_EQ( record.child_depth[k], depth )`, `values: REQUIRE_EQ( 5, 4 )`, in seconds. The tool\'s own run reads `unmeasured` instead, because two plies a check blows the tree up and all four of its failing tests are Timeouts, which DEC-165 refuses to score as a kill (`adocs/data/S188_mutation_pass.tsv`); the row there is left as the tool wrote it',
  ('                                 ? 1\n'
   '                                 : ((moves[i] == tt_move) ? se_extension '
   ': 0));',
   '                                 ? 2\n'
   '                                 : ((moves[i] == tt_move) ? se_extension '
   ': 0));'),
  origin="S188")

m("X11_see_gate_dropped", S, "search/check_extension",
  'DEC-228\'s exchange gate goes and every check is extended again, including '
  'one that hangs the checking piece on the square it checks from -- the form '
  'measured at +94.6 % of `bench` and five plies of iteration depth, which is '
  'what the decision re-formed the step away from. The gate is one '
  'conjunction and dropping it is the whole difference between the two forms',
  ('        check_extension_window && see_ge(&game->board, moves[i], 0);',
   '        check_extension_window;'),
  origin="S188")

m("X12_horizon_restriction_dropped", S, "search/check_extension",
  'the horizon restriction goes and the rule fires at every depth again, which '
  'is DEC-228\'s second form: +72.4 % of `bench` against the third form\'s '
  '+28.1 %, and the bar missed. One conjunction, and the bench signature alone '
  'cannot tell which of the two forms it left behind -- the case that can is '
  'the one named below',
  ('      CHECK_EXTEND != 0 && ply > 0 && depth <= CHECK_EXT_MAX_DEPTH &&',
   '      CHECK_EXTEND != 0 && ply > 0 &&'),
  origin="S188")
