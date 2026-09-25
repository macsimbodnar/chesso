"""The point a node pruned by reverse futility returns. S235.

Four mutants over one expression and the guard that keeps a mate score out of
it. The site returns `beta + (bound - beta) * RfpReturnWeight /
RFP_RETURN_SCALE`, where `bound` is what the block's own test argued -- the
node's estimate less the margin -- so every mistake available here is an
arithmetic one, and every one of them still returns an integer, still fails the
node high and still leaves the search running:

  the blend     H01 puts the point on the wrong side of beta, which is the one
                mistake that breaks the fail-soft contract: the parent is
                handed a number below the window it asked about by a node that
                just decided it fails high. H02 drops the scale, so the weight
                stops being a share and becomes a multiplier and the node
                claims fifty times what its own test argued. H03 inverts the
                weight, which is the identity at the shipped midpoint and is
                declared equivalent below for exactly that reason.
  the guard     H04 lets a mate-band entry through S109's exclusion and into
                the blend, so a point between beta and a mate score comes back
                as this node's value -- a mate claim nothing here searched for.

**H04 cuts a line S109 wrote, and S234's G03 cuts the same one.** That is
deliberate and not an oversight: this step gives that guard a second consumer,
the blend, and the case that kills H04 here is this step's own -- "a mate-band
estimate is never the number the blend is taken over" -- where S234's case asks
whether a mate score can decide the cutoff. The two ids never load together in
one run; each step names its own list.

**H03 is the one this tool cannot kill, and the reason is arithmetic.**
`RfpReturnWeight` ships at 50 and the scale is 100, so `scale - weight` is
`weight` and the mutant is the shipped engine, line for line, in the release
build this tool measures. That is what `expected="equivalent"` means here and
it is a statement about the seed and not about the rule: the tune build tells
the two apart at 25 and 75, which is what "the returned bound walks from beta
to the site's own bound as the weight walks its range" drives, and S127's fit
moves the default off the midpoint the day it lands. The tune-build red is
observed by hand under the S033 protocol and recorded in the step file, as
S234's G02 was.

Nothing here moves a constant. `RfpReturnWeight` at its range top is the off
value DEC-215 asks to be proved on the tree and is not a bug.

The list is data: `m` is bound by tools/mutation_check.py, which execs this
file, so nothing here is a driver of its own. `old` must occur exactly once in
`file` -- an ambiguous anchor mutates a site nobody chose, and the tool refuses
the whole run before it writes anything. `expected` is "killed" unless a person
has argued the mutant is behaviourally equivalent to the engine, which no tool
can decide.

Ids are never reused: H is this file's own prefix and nothing else in
tools/mutants/ uses it.
"""

S = "src/search.cpp"

m("H01_blend_below_beta", S, "search/pruning",
  'the point is taken **downward** from beta instead of upward toward the '
  'bound, so a node that has just decided it fails high returns a value below '
  'the window it was asked about. Fail soft permits a return weaker than the '
  'bound and never one below beta: the parent reads a fail-low from a node '
  'that searched nothing, stores it as an upper bound and may never revisit '
  'the position. No crash, no assertion in the release build, and at the '
  'shipped weight the number is as far below beta as the honest one is above '
  'it',
  ('      const int rfp_blended =\n'
   '          beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE;',
   '      const int rfp_blended =\n'
   '          beta - (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE;'),
  origin="S235")

m("H02_blend_scale_dropped", S, "search/pruning",
  'the division by the scale is dropped, so the weight stops being a share of '
  'the gap and becomes a multiplier of it: at the shipped value the node '
  'returns fifty gaps above beta, far past the bound its own static test '
  'argued and past anything the position is worth. The direction is right and '
  'the unit is not, which is the shape a scale mistake takes -- and the bound '
  'is still finite and still below the mate band on most nodes, so nothing '
  'asserts and nothing overflows',
  ('      const int rfp_blended =\n'
   '          beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE;',
   '      const int rfp_blended =\n'
   '          beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT;'),
  origin="S235", expected="killed")

m("H03_blend_weight_inverted", S, "search/pruning",
  'the weight is read from the other end: the point sits `scale - weight` '
  'hundredths up from beta instead of `weight`. **Equivalent at the shipped '
  'default and provably so** -- the seed is the midpoint of 0 to 100, where '
  'the inversion is the identity, so the release build this tool measures '
  'cannot tell the two apart and neither can any case in it. It is in the list '
  'because the tune build can, at 25 and 75, and because S127 moves the '
  'default off the midpoint: the day it does, this mutant becomes an ordinary '
  'killable one and the expectation below moves with it',
  ('      const int rfp_blended =\n'
   '          beta + (rfp_bound - beta) * RFP_RETURN_WEIGHT / RFP_RETURN_SCALE;',
   '      const int rfp_blended =\n'
   '          beta + (rfp_bound - beta) * (RFP_RETURN_SCALE - RFP_RETURN_WEIGHT)\n'
   '                     / RFP_RETURN_SCALE;'),
  origin="S235", expected="equivalent")

m("H04_blend_over_a_mate_estimate", S, "search/pruning",
  'the mate band is not excluded from the estimate, so an entry holding a mate '
  'score reaches **the blend**: the node returns a point between beta and a '
  'mate score, which on any weight high enough is itself inside the band -- a '
  'mate this search never proved, handed to a parent that will report it. The '
  'guard is S109\'s and this step is its second consumer, which is why the cut '
  'is listed here as well as in tools/mutants/S234_tt_estimate_margins.py. '
  'Invisible to every bound-direction case, because a mate score passes the '
  'direction test by being far above the static one',
  ('    if (tt_score < MATE_MIN && tt_score > -MATE_MIN) {',
   '    if (true) {  // H04: the mate band reaches the blend.'),
  origin="S235")
