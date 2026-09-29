"""S015's exchange gate in quiescence behind a switch, S022 verdict 1.

Five mutants over the one condition this verdict edits: the gate
`QS_SEE_GATE != 0 && !in_check && !capture_cannot_lose(...) &&
!see_ge(..., 0)` in quiescence(). Every one is a switch or a test read wrongly,
never a constant moved; `QsSeeGate` at 1 is the parent's tree, which the
verdict's H0 row returns to, and is not a bug. Each is killed by a case in
tests/test_search.cpp's "search: quiescence exchange gate" suite or S131's
"search: quiescence promotions", named per mutant:

  U01  the switch ignored -- the      "a losing capture is searched at the
       gate on at every value, so 0   gate's off value", "a queen promotion
       ships the parent's tree        onto a defended square is searched at
                                      the gate's off value", and the floor of
                                      "ordering keeps the tree small", the
                                      golden re-derived for the candidate
  U02  the switch read the wrong way  the same three in the release build;
       round                          in the tune build "the gate declines a
                                      losing capture at the switch's on
                                      value" and "the exchange gate declines
                                      a queen promotion onto a defended
                                      square" as well
  U03  the gate's sign flipped at 1   declared equivalent in the release
       -- a capture the exchange      build, where the gate is dead code at
       evaluation clears is declined  the shipped 0; killed in the tune build
       and a losing one searched      by "the gate declines a losing capture
                                      at the switch's on value" and "the
                                      exchange gate declines a queen promotion
                                      onto a defended square"
  U04  the gate kept at 0 for a       "a queen promotion onto a defended
       promotion that takes nothing   square is searched at the gate's off
                                      value", and no other case of the three
                                      quiescence suites
  U05  the gate kept at 0 for a       "a losing capture is searched at the
       capture                        gate's off value", with the floor of
                                      "ordering keeps the tree small"

"pruning does not hide a forced mate" kills none of them: its capture-mate
rows, re-derived for the candidate, report their mates on the tree with the
gate as well (a pre-check, each mutant against the named cases on a Release
and a tune build of the fixture, .tuning/coord/S022_precheck.log). The table
is that pre-check's and the tool's pass confirms it.

U04 and U05 are the two halves of a deletion done by class, each a plausible
way to land this verdict wrongly, and each exists so the two off-value cases
are shown to see different things.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: U is this file's own prefix, and no mutant file on any
branch has used it (`git log --all -p -- tools/mutants` has no `m("U`).
"""

S = "src/search.cpp"

GATE = ("    if (QS_SEE_GATE != 0 && !in_check &&\n"
        "        !capture_cannot_lose(&game->board, moves[i]) &&\n"
        "        !see_ge(&game->board, moves[i], 0)) {")


def gate_with(old_term, new_term):
    """The gate condition with one term replaced; the term occurs once."""
    assert GATE.count(old_term) == 1, old_term
    return (GATE, GATE.replace(old_term, new_term))


m("U01_switch_ignored", S, "search/quiescence",
  'the gate no longer consults its switch, so it declines a move the '
  'exchange evaluation writes off at every value: 0 ships the parent\'s '
  'tree and the verdict would measure the parent against itself',
  gate_with("QS_SEE_GATE != 0 && ", ""),
  origin="S022")

m("U02_switch_inverted", S, "search/quiescence",
  'the gate reads its own switch the wrong way round, so it is on at the '
  'value that ships and off at the one the H0 row returns to: the SPRT '
  'would measure the parent against the parent, and flipping the default '
  'on H0 would ship the deletion',
  gate_with("QS_SEE_GATE != 0", "QS_SEE_GATE == 0"),
  origin="S022")

# Equivalent in the build this tool runs, and argued rather than inferred: the
# release build compiles QsSeeGate as the constant 0, so the whole condition is
# false and the flipped term is never evaluated -- the same engine. The tune
# build, where the switch is a variable, is where the gate is live at 1; its
# on-value cases are red under this mutant, observed by the tool on a tune build
# directory of the fixture (.tuning/coord/S022_logs/).
m("U03_gate_sign_flipped", S, "search/quiescence",
  'at 1 the gate declines a capture the exchange evaluation clears and '
  'searches the ones it writes off -- the comparison turned round, which '
  'the tree before S022 would reproduce nowhere',
  gate_with("        !see_ge(&game->board, moves[i], 0)) {",
            "        see_ge(&game->board, moves[i], 0)) {"),
  expected="equivalent",
  origin="S022")

m("U04_quiet_promotions_still_gated", S, "search/quiescence",
  'the deletion stops at captures: at 0 a queen promotion that takes '
  'nothing is still declined onto a square it cannot hold, so the candidate '
  'is the deletion for one class and the parent for the other',
  gate_with("QS_SEE_GATE != 0 && ",
            "(QS_SEE_GATE != 0 || !MOVE_CAPTURE(moves[i])) && "),
  origin="S022")

m("U05_captures_still_gated", S, "search/quiescence",
  'the deletion reaches only the promotions that take nothing: at 0 a '
  'capture the exchange evaluation writes off is still declined, which is '
  'the parent\'s tree for every capture',
  gate_with("QS_SEE_GATE != 0 && ",
            "(QS_SEE_GATE != 0 || MOVE_CAPTURE(moves[i])) && "),
  origin="S022")
