"""The node-level delta early-out in quiescence behind a switch, S022 verdict 2.

Eight mutants over the one block this verdict adds to quiescence(): out of
check and above `QsDeltaPhaseMin`, a node whose ceiling -- S112's futility
base, the queen's price in its victim table and, with a pawn of the side to
move on its seventh rank, the same table's queen less its pawn -- is at or
below alpha ends ungenerated, returning the ceiling and storing it as an upper
bound. Every one is a guard dropped, a term dropped or a claim written wrong,
never a constant moved; `QsDeltaEarlyOut` at 0 is the parent's tree, which the
verdict's H0 row returns to, and is not a bug. Each is killed by a case in
tests/test_search.cpp's "search: quiescence delta early-out" suite, named per
mutant:

  Z01  the promotion allowance       "a pawn on its seventh raises the ceiling
       dropped                       by the promotion", both halves and both
                                     sides to move
  Z02  the in-check guard dropped    "the early-out is never asked in check"
  Z03  the return as the bare stand  "the early-out's value is its ceiling and
       pat                           not the stand pat", and the second half
                                     of the allowance case
  Z04  the phase disable dropped     "the early-out is not asked in a pawn
                                     ending"; in the tune build "the endgame
                                     disable follows its threshold" as well
  Z05  the switch ignored            declared equivalent in the release build,
                                     where the switch is the constant 1 it
                                     ships at; killed in the tune build by
                                     "the early-out is off at the switch's off
                                     value"
  Z06  the compare's sign flipped    "the early-out does not fire where a
                                     queen's gain reaches alpha" and the first
                                     half of the allowance case
  Z07  the switch read the wrong way "the early-out ends a node no move can
       round                         lift to alpha", its value case and the
                                     allowance case
  Z08  the ceiling stored with the   "the early-out ends a node no move can
       stand pat's type              lift to alpha"

Z01 to Z06 are the six the brief names. Z07 is Z05's twin the release build
can kill, and Z08 is the one that makes the "stored as an upper bound"
assertion bite, as S112's F06 does for the fold.

Z06 flips `<=` to `>=`, and the first case does not see it: at an alpha equal
to the ceiling both fire. That is why the suite pins the other side of the
boundary too -- one below the ceiling must generate -- and that case is its
killer.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file`. `expected` is "killed" unless a person has argued the mutant is
behaviourally equivalent to the engine, which no tool can decide.

Ids are never reused: Z is this file's own prefix, and no mutant file on any
branch has used it (`git log --all -p -- tools/mutants adocs/data` has no
`m("Z`; tests/test_mutation_check.py's sandbox list uses A and O).
"""

S = "src/search.cpp"

GUARD = ("  if (QS_DELTA_EARLY_OUT != 0 && !in_check &&\n"
         "      game_phase(&game->board) > QS_DELTA_PHASE_MIN) {")


def guard_as(new):
    """The early-out's guard replaced whole; the guard occurs once."""
    return (GUARD, new)


# Release -Werror: dropping the allowance's only reads of `pawn_on_seventh` and
# `own_queen` orphans both, so the mutant carries a (void) of each, as the tool's
# header says such a mutant must.
m("Z01_allowance_dropped", S, "search/pruning",
  'a pawn on its seventh no longer raises the ceiling, so the early-out ends '
  'a node the promotion S131 searches could have lifted by queen minus pawn, '
  'at every node with a pawn a step from queening -- the step file\'s '
  'promotion pitfall, the class delta pruning is recorded as getting wrong',
  ('    const int promotion_allowance =\n'
   '        pawn_on_seventh\n'
   '            ? qs_futility_value[own_queen] - qs_futility_value[own_pawn]\n'
   '            : 0;\n',
   '    const int promotion_allowance = 0;\n'
   '    (void)pawn_on_seventh;\n'
   '    (void)own_queen;\n'),
  origin="S022")

m("Z02_in_check_guard_dropped", S, "search/pruning",
  'the early-out runs in check, where the side to move may not stand pat: a '
  'node with no legal reply hands back its ceiling, a positional number, '
  'instead of the mate -- a pruning that hides a mate',
  guard_as("  if (QS_DELTA_EARLY_OUT != 0 &&\n"
           "      game_phase(&game->board) > QS_DELTA_PHASE_MIN) {"),
  origin="S022")

m("Z03_returns_stand_pat", S, "search/pruning",
  'the early-out returns the bare stand pat, an upper bound lower than the '
  'moves it never generated could reach, handed to a parent that compares it '
  'as though a search had produced it',
  ("      return delta_ceiling;\n",
   "      return stand_pat;\n"),
  origin="S022")

m("Z04_phase_disable_dropped", S, "search/pruning",
  'the late-endgame disable no longer binds, so the early-out ends pawn '
  'endings too, where a capture can decide the game and piece values do not '
  'price it',
  guard_as("  if (QS_DELTA_EARLY_OUT != 0 && !in_check) {"),
  origin="S022")

# Equivalent in the build this tool runs, and argued rather than inferred: the
# release build compiles QsDeltaEarlyOut as the constant 1, so the term is true
# and dropping it leaves the same condition -- the same engine. The tune build,
# where the switch is a variable, is where 0 is reachable; its off-value case
# is red under this mutant, observed by the tool on a tune build of the fixture.
m("Z05_switch_ignored", S, "search/pruning",
  'the early-out no longer consults its switch, so 0 ships it too: the H0 row '
  'would flip the default and change nothing, and the verdict\'s reference '
  'could not be rebuilt from the candidate',
  guard_as("  if (!in_check &&\n"
           "      game_phase(&game->board) > QS_DELTA_PHASE_MIN) {"),
  expected="equivalent",
  origin="S022")

m("Z06_compare_sign_flipped", S, "search/pruning",
  'the compare turned round: the node ends exactly where a queen\'s gain '
  'could still reach alpha and is searched where nothing can',
  ("    if (delta_ceiling <= alpha) {",
   "    if (delta_ceiling >= alpha) {"),
  origin="S022")

m("Z07_switch_inverted", S, "search/pruning",
  'the early-out reads its own switch the wrong way round, so it is off at '
  'the value that ships and on at the one the H0 row returns to: the SPRT '
  'would measure the parent against the parent',
  guard_as("  if (QS_DELTA_EARLY_OUT == 0 && !in_check &&\n"
           "      game_phase(&game->board) > QS_DELTA_PHASE_MIN) {"),
  origin="S022")

m("Z08_stored_as_floor_type", S, "search/pruning",
  'the ceiling is stored with the stand pat\'s type, exact on a cold table, '
  'so the table certifies as the value a number no search established -- a '
  'bound written as a score, which a later probe answers any window with',
  ("normalize_score(delta_ceiling, ply), TT_ALPHA_NODE, 0,",
   "normalize_score(delta_ceiling, ply), stand_pat_type, 0,"),
  origin="S022")
