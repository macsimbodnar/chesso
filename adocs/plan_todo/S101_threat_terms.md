id:         S101
goal:       evaluation terms for a piece attacked by a lesser piece, fitted like every other constant
accepts:    an SPRT verdict against a named commit, recorded whatever it is (INV-6); the terms are fitted with every other constant frozen and the held-out error reported before and after, which is how every constant in this engine was fitted; the term is accumulated or computed in a stage that already has the attack bitboards it needs, never rebuilt from the bitboards a second time, since evaluate() runs at every quiescence node and INV-4 exists to keep that cost out; INV-5 holds -- mirroring a position agrees rather than negates, asserted in tests/test_evaluation.cpp; the lazy evaluation bound still holds by construction if the term lands in the expensive stage, or the step states why it belongs in the cheap one; tools/eval_model.hpp gains the feature and tools/tuner_groups.hpp a group whose partition properties still hold; the fast suite green
touches:    src/evaluation.cpp, src/evaluation.hpp, tools/eval_model.hpp, tools/tuner_groups.hpp, tests/test_evaluation.cpp
excludes:   outposts and space, which are S102; king safety, which exists and is fitted
decisions:  DEC-071
closes:
blocks:
paused_by:
done:

## The hazard is where it is computed, not what it scores

Every evaluation term must be accumulated or shared, never recomputed.
`evaluate()` runs at every quiescence node and rebuilding the tables from the
bitboards was 25 % of nodes per second before S014 removed it. S027's three
pawn terms landed in the cheap stage precisely because the lazy clamp would
truncate them; a threat term has the same question to answer and answers it in
the step, with the answer measured.

## Technical details (SOTA research, 2026-09-13)

S186's enrichment pass, DEC-097 as amended by DEC-137. Every figure carries a
URL or the word **unverified** with what this pass searched. Citations into
code name a symbol and no line (DEC-135). Every engine record read below is a
commit message, a pull-request body, a changelog entry, a release note or a
forum post -- never a source file and never a table (DEC-016).

### 1. State of the art

**The wiki has no page for this technique.** `https://www.chessprogramming.org/Threat`
and `https://www.chessprogramming.org/Threats` both return HTTP 404 (fetched
2026-09-13). The nearest page is CPW *Hanging Piece*
(https://www.chessprogramming.org/Hanging_Piece, fetched 2026-09-13), which
defines a hanging piece as "an attacked piece not defended by own man exposed
to capture" and says loose pieces and undefended pawns "may be considered in
evaluation, specially if there are more than one or even two per side, and the
opponent has forces and possibilities for double attacks and knight forks".
**Absent from that page: any treatment of a piece attacked by a *lesser*
piece, which is this step's whole goal; any pawn-push threat; and every
centipawn and Elo figure.**

So the form check's answer is: **the definition this step implements comes from
the engine record and not from the wiki**, and that is stated rather than
implied. It is read for form only.

**The one traced figure for the group is Stash's, and it is a wider term than
this step's.** `mhouppin/stash-bot`'s `CHANGELOG.md`
(https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md), v26: dynamic
initiative from pieces threatened by lower-valued ones, **+10.13 +/- 6.50** at
10.0+0.1s and **+7.77 +/- 5.23** at 60.0+0.6s. The 2026-09-04 literature check
already recorded the mismatch and `adocs/plan.md` states it at the site:
Stash's is an *initiative* term **built on** threatened pieces, which is
broader than "a piece attacked by a lesser piece". The figure is quoted for
order of magnitude and for direction, never as this step's expectation
(DEC-019).

**What the record has that this step's goal excludes.** The 2026-09-04 check's
part B3 inventories the threat family the surveyed engines carry: a minor
attacked by a pawn, a minor attacked by a minor, a major attacked by a minor,
hanging pieces, pawn-push threats and overloaded pieces. This step's goal is
the first three; hanging, pawn-push and overload threats are outside it and
that is deliberate, recorded there as "partial". A later step or DEC-138's
inventory owns the rest; this file does not silently widen.

**No located source prices the narrow term alone**, at any band. Searched this
pass: CPW *Threat* and *Threats* (both 404), CPW *Hanging Piece*,
`mhouppin/stash-bot`'s `CHANGELOG.md`. Searched 2026-09-04: the same family in
part B3. The step's verdict is its own SPRT and no ordering argument here
rests on a published number.

### 2. Shape for chesso

The term needs, for each side, the set of squares attacked by each enemy piece
class. Those sets are built already, once, in the loop that computes mobility
(`src/evaluation.cpp` `mobility_mg` is the term; the attack sets are the
loop's). After S121 the same loop also builds a mobility area from the enemy
pawn-attack span, which is exactly the input a pawn-threat term needs.

**Departure, stated:** the surveyed engines fold their threat terms into a
larger initiative or threat block scored by a table; this step scores a
smaller, per-class set of weights fitted like every other constant, because
the tuner's model has to stay linear in the parameters
(`tools/eval_model.hpp`). That is a narrower form than the record's and it is
the form that keeps the gradient closed.

### 3. Implementation sketch

- The step states which stage it lands in and why, and the accepts requires the
  answer to be measured, not argued: if it lands in the expensive stage the
  lazy bound still holds by construction because the clamp is inside
  `src/evaluation.cpp` `evaluate_expensive`; if it lands in the cheap stage it
  escapes the clamp and is paid at every quiescence node.
- `tools/eval_model.hpp` gains the feature and `tools/tuner_groups.hpp` a
  group, with the partition properties in `tests/test_tuner_groups.cpp` green.
- INV-5 is asserted in `tests/test_evaluation.cpp`: mirroring a position agrees
  rather than negates.

### 4. Constants and seeds

**No seeds.** Every weight is fitted with every other constant frozen and the
held-out error reported before and after, which is the accepts. Stash's +10.13
is an Elo measurement, not a constant. No engine's threat table seeds anything
here wherever it is republished (DEC-105, DEC-134), and the wiki offers no
value to take even if one were wanted.

The one declared number is the **feature count** -- how many attacker/victim
class pairs the term carries -- and it is **(b) derived**: the pairs follow
from the piece classes chesso's evaluation already distinguishes, and the step
states the enumeration rather than choosing a width.

### 5. Pitfalls

- **Where it is computed, not what it scores.** `evaluate()` runs at every
  quiescence node and rebuilding attack sets a second time is the 25 % of nps
  S014 removed and INV-4 keeps removed. The term reuses the existing loop's
  sets or it does not ship.
- **The stage decision changes the lazy margin's input.** A term in the
  expensive stage enters the sum S039 sizes the clamp on; a term in the cheap
  stage does not. S039 runs before S122 and after this step by DEC-172's
  order, so the choice here is an input to that run and the step says so.
- **The band is not this engine's.** Stash's +10.13 is at a much higher rating
  than chesso's, and DEC-019's three recorded cases are exactly reported
  figures that did not transfer. Bounds are chosen for a term that may measure
  zero.
- **A zero verdict is a legitimate outcome** and is recorded as zero (DEC-019,
  and S005, S006 and S015 are the precedents).

### 6. Measurement

One SPRT against a named commit at the S105 regime, bounds stated in advance
with the nElo worst case and the abort rule (DEC-143). Held-out error before
and after the fit, reported. nps recorded, because the answer to "which stage"
is a speed question as much as a score question.

### 7. Interactions

- **S121 (before)**: builds the attack sets and the mobility area this term
  reads. Running before S121 would mean building them twice.
- **S039 (after, before S122)**: sizes the clamp on the sum this term may
  enter. DEC-172 moved S039 behind this step for exactly that reason.
- **S102 (separate)**: outposts and space, the other two reuse-the-fills terms.
- **S122 (after)**: king safety counts attacks too; the two must not
  double-count the same attack, and the step says how they divide.
- **S126 (block end)**: refits everything.

### 8. References

- - https://www.chessprogramming.org/Hanging_Piece -- "an attacked piece not
  defended by own man exposed to capture"; loose pieces "may be considered in
  evaluation"; **no piece-attacked-by-a-lesser-piece treatment, no pawn-push
  threat, no figure**. Fetched 2026-09-13.
- - `https://www.chessprogramming.org/Threat` and
  `https://www.chessprogramming.org/Threats` -- **HTTP 404, both**. Fetched
  2026-09-13. The technique has no wiki page.
- - https://github.com/mhouppin/stash-bot/blob/master/CHANGELOG.md -- v26
  dynamic initiative from pieces threatened by lower-valued ones,
  +10.13 +/- 6.50 at 10.0+0.1s and +7.77 +/- 5.23 at 60.0+0.6s. Changelog
  entries only.
- - `adocs/data/2026-09-04_plan_review_literature_check.md` part B3 -- the
  threat family inventory and the "partial" verdict on this step's scope.
