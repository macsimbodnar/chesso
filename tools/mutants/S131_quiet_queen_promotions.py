"""Quiet queen promotions in quiescence, S131.

Ten mutants over the one condition this step adds to quiescence's filter and
the two tests the admitted move meets after it. Every one is a test dropped,
inverted or narrowed, never a constant moved; `QsQueenPromotions` at 0 is the
off value DEC-215 asks to be proved on the tree and is not a bug. Each is
killed by a case in tests/test_search.cpp's "search: quiescence promotions"
suite, or for V09 and V10 in S112's "search: quiescence futility", named per
mutant. The table is the final pass's: all ten in one run on a fixture of
the tree S131 lands on, `3c7cf84` with S113's ProbCut in and S097's multicut
row of "pruning does not hide a forced mate" re-derived for it in the miner's
guard mode (DEC-238; the step file's "Rebased onto S113's tree,
2026-09-28"). On that row V04 is red as well: it is the tree without the
class, `3c7cf84`'s engine, which reports no mate there at 14. The second
pass, on `029ff61` after S131's fast check moved the exact node count out of
the admission case and gave the underpromotion case evidence of its own, had
V02 and V09 killed by "pruning does not hide a forced mate" as well, through
that tree's multicut row, and V10 in a targeted third run (the step file's
"Fast-check fix-ups, 2026-09-28"):

  V01  the queen test dropped          "an underpromotion that takes nothing
       -- every promotion that takes    is not searched", and no other case
       nothing admitted
  V02  the queen test inverted --      "a queen promotion that takes nothing
       the queen dropped, the three     is searched", with "an underpromotion
       underpromotions admitted         that takes nothing is not searched"
                                        and "futility does not skip a queen
                                        promotion that takes nothing"
  V03  the capture arm broken -- a     "a promotion that takes something is
       promotion that takes something   searched as a capture"
       must pass the queen test too,
       so its underpromotions drop
  V04  the switch read the wrong way   "a queen promotion that takes nothing
       round                            is searched", with "futility does not
                                        skip a queen promotion that takes
                                        nothing" and "pruning does not hide a
                                        forced mate" at its multicut row;
                                        S112's promotion case stays green
                                        under it, the tree without the class
  V05  the switch ignored              declared equivalent in the release
                                        build; killed in the tune build by
                                        "no promotion that takes nothing is
                                        searched at the switch's off value"
  V06  S112's promotion exemption      "futility does not skip a queen
       narrowed to the promotions       promotion that takes nothing"; S112's
       that capture                     own promotion case stays green under
                                        it, which is why this one exists
  V07  S015's exchange gate skipped    "the exchange gate declines a queen
       for a promotion that takes       promotion onto a defended square"
       nothing
  V08  the knight admitted beside the  "an underpromotion that takes nothing
       queen                            is not searched", and no other case
  V09  S112's exemption dropped and    S112's "a capturing promotion below
       the gate skipped for quiet       the threshold is searched" (its count
       promotions together              already sees it), with "the exchange
                                        gate declines a queen promotion onto
                                        a defended square" and "futility does
                                        not skip a queen promotion that takes
                                        nothing"
  V10  S112's exemption narrowed to    S112's "a capturing promotion below
       the promotions that take         the threshold is searched", at the
       nothing, and the gate skipped    move its root settles on, with "the
       for quiet promotions             exchange gate declines a queen
                                        promotion onto a defended square"

V06, V07, V09 and V10 edit S112's and S015's lines and not this step's: they
are the admitted class's interactions, each a plausible way to land this step
wrongly, and each case above exists to see one of them. **V10 is the masking
S131's fast check named**: every capturing promotion below the threshold
skipped on its victim while the quiet queen promotion beside it is searched
ungated, which S112's `nodes >= 2` cannot tell from the capture it means; the
case now asks which move its root settled on. V09 was the first try at it and
is not the masking -- dropping the exemption whole skips the quiet promotion
too, so the count alone goes red -- and it is kept under its id as the
second pass measured it.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: V is this file's own prefix, and no file in
tools/mutants/ has used it on any branch (A and O are
tests/test_mutation_check.py's stubs).
"""

S = "src/search.cpp"

FILTER = ("    if (!in_check && !MOVE_CAPTURE(moves[i]) &&\n"
          "        !(QS_QUEEN_PROMOTIONS != 0 && "
          "MOVE_PROMOTED(moves[i]) == TO_QUEEN)) {")


def filter_with(old_term, new_term):
    """The filter condition with one term replaced; the term occurs once."""
    assert FILTER.count(old_term) == 1, old_term
    return (FILTER, FILTER.replace(old_term, new_term))


m("V01_queen_test_dropped", S, "search/quiescence",
  'every promotion that takes nothing passes the filter, the three '
  'underpromotions with the queen: four children where the published move '
  'set has one, and the underpromotions searched beside a queen that '
  'dominates them',
  filter_with("MOVE_PROMOTED(moves[i]) == TO_QUEEN",
              "MOVE_PROMOTED(moves[i]) != TO_NONE"),
  origin="S131")

m("V02_queen_test_inverted", S, "search/quiescence",
  'the queen test reads the wrong way round: the queen promotion that takes '
  'nothing is dropped as before S131 and the three underpromotions are '
  'searched in its place, so the class this step exists for is still '
  'invisible and the node count still moves',
  filter_with("MOVE_PROMOTED(moves[i]) == TO_QUEEN",
              "MOVE_PROMOTED(moves[i]) != TO_QUEEN"),
  origin="S131")

m("V03_capture_arm_broken", S, "search/quiescence",
  'the capture test stops admitting a promotion, so a promotion that takes '
  'something has to pass the queen test as well: its three underpromotions '
  'are dropped, which is "underpromotions stay out" read as covering the '
  'capturing ones the filter always kept',
  filter_with("!MOVE_CAPTURE(moves[i]) &&",
              "(!MOVE_CAPTURE(moves[i]) || MOVE_PROMOTED(moves[i]) != TO_NONE) &&"),
  origin="S131")

m("V04_switch_inverted", S, "search/quiescence",
  'the filter reads its own switch the wrong way round, so the class is off '
  'at the value that ships and on at the one that switches it off: the '
  'SPRT would measure the parent against the parent and the H0 path would '
  'flip a default and get the candidate back',
  filter_with("QS_QUEEN_PROMOTIONS != 0", "QS_QUEEN_PROMOTIONS == 0"),
  origin="S131")

# Equivalent in the build this tool runs, and argued rather than inferred: the
# release build compiles QsQueenPromotions as the constant 1, so the dropped
# term was already true everywhere and the binary is the same engine. The tune
# build, where the switch is a variable, is where the term is live; its case
# "no promotion that takes nothing is searched at the switch's off value" is
# red under this mutant, observed by hand (.tuning/coord/S131_logs/).
m("V05_switch_ignored", S, "search/quiescence",
  'the filter no longer consults its switch, so 0 stops being the tree '
  'before S131: the off value the verdict\'s H0 row returns to would move '
  'nothing',
  filter_with("QS_QUEEN_PROMOTIONS != 0 && ", ""),
  expected="equivalent",
  origin="S131")

m("V06_futility_exemption_capturing_only", S, "search/pruning",
  'S112\'s promotion exemption covers only the promotions that capture, so a '
  'queen promotion that takes nothing is priced as a capture of the empty '
  'square and skipped on the futility base alone -- exactly the class this '
  'step admits, pruned silently wherever the stand pat is a margin below '
  'alpha (the step file\'s pitfall 1)',
  ('    if (qs_futility && !MOVE_PROMOTED(moves[i])) {',
   '    if (qs_futility && !(MOVE_PROMOTED(moves[i]) && MOVE_CAPTURE(moves[i]))) {'),
  origin="S131")

# **V07, V09 and V10 were re-pointed at S022's first verdict**, which puts
# S015's gate behind `QS_SEE_GATE`: each gate pair now anchors on the switched
# condition and makes the same edit, the gate asking only about captures. While
# the switch ships at 0 the gate is dead code in the release build
# tools/mutation_check.py runs, so V07, whose whole edit is the gate, is
# declared equivalent there, and its case, "the exchange gate declines a queen
# promotion onto a defended square", runs in the tune build at `QsSeeGate` 1,
# where it kills it; V09 and V10 are still killed in the release build by their
# futility halves. S022's verdict restores these anchors and V07's "killed"
# with the gate, or retires V07 with it (adocs/data/S022_v1_sprt.sh).
m("V07_see_gate_skipped_for_quiet_promotions", S, "search/quiescence",
  'the exchange gate asks only about captures, so a queen promotion that '
  'takes nothing is searched onto a square the opponent holds, where the '
  'engine\'s own exchange evaluation says it loses the queen -- the SEE '
  'treatment of promotions this step\'s excludes keep unchanged',
  ('    if (QS_SEE_GATE != 0 && !in_check &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {',
   '    if (QS_SEE_GATE != 0 && !in_check && MOVE_CAPTURE(moves[i]) &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {'),
  expected="equivalent",
  origin="S131")

m("V08_knight_admitted_beside_queen", S, "search/quiescence",
  'the knight promotion that takes nothing passes the filter beside the '
  'queen -- the one underpromotion some move sets keep, and a second child '
  'where the published quiescence has one',
  filter_with("MOVE_PROMOTED(moves[i]) == TO_QUEEN",
              "(MOVE_PROMOTED(moves[i]) == TO_QUEEN ||\n"
              "          MOVE_PROMOTED(moves[i]) == TO_KNIGHT)"),
  origin="S131")

m("V09_capturing_promotions_skipped_quiet_one_searched", S, "search/pruning",
  'S112\'s promotion exemption dropped and the exchange gate asking only '
  'about captures, together: every capturing promotion below the threshold is '
  'skipped on its victim alone, and so is the quiet queen promotion beside '
  'it, which the dropped exemption prices at the empty square -- the first '
  'try at the masking V10 is, and one the node count alone already sees',
  ('    if (qs_futility && !MOVE_PROMOTED(moves[i])) {',
   '    if (qs_futility) {'),
  ('    if (QS_SEE_GATE != 0 && !in_check &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {',
   '    if (QS_SEE_GATE != 0 && !in_check && MOVE_CAPTURE(moves[i]) &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {'),
  origin="S131")

m("V10_capturing_promotions_futile_quiet_one_ungated", S, "search/pruning",
  'S112\'s promotion exemption narrowed to the promotions that take nothing, '
  'and the exchange gate asking only about captures: every capturing '
  'promotion below the threshold is skipped on its victim alone while the '
  'quiet queen promotion beside it, exempt and ungated, is searched in their '
  'place -- a child for the node count and the wrong move for the claim',
  ('    if (qs_futility && !MOVE_PROMOTED(moves[i])) {',
   '    if (qs_futility && (!MOVE_PROMOTED(moves[i]) || MOVE_CAPTURE(moves[i]))) {'),
  ('    if (QS_SEE_GATE != 0 && !in_check &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {',
   '    if (QS_SEE_GATE != 0 && !in_check && MOVE_CAPTURE(moves[i]) &&\n'
   '        !capture_cannot_lose(&game->board, moves[i]) &&\n'
   '        !see_ge(&game->board, moves[i], 0)) {'),
  origin="S131")
