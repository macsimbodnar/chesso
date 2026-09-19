id:         S234
goal:       where a node's table entry holds a score whose bound points the same way as the gap between that score and the static evaluation, the pruning margins read the table's score as the node's estimate, while the raw static evaluation stays what is stored and what any later correction learns from
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; one estimate value feeds the margin tests in `src/search.cpp` `negamax` -- reverse futility and the null-move static-score condition today, razoring when S116 lands -- and the stored eval field and every history update read the raw `static_eval`, asserted by a test that plants a table entry and observes the margin decision move while the stored eval does not; a lower-bound entry above the static evaluation and an upper-bound entry below it are the only two cases that tighten, the other two leave the estimate alone, one case per branch with the precondition counted; a mate-band score in the entry never tightens, covered by a case; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant that tightens on the wrong bound direction is killed (`tools/mutation_check.py`); at the rule's off value the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, tests/test_search.cpp, tools/mutants/
excludes:   any change to what quiescence does with the table score -- S130 shipped that form at the stand-pat and DEC-103 keeps it; razoring, which does not exist until S116 (its site joins then); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-103, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
done:

## Why this exists

S130 measured the table score as the quiescence stand-pat at +1.14 +/- 4.04
over 16784 games -- no verdict, recorded as zero and kept (DEC-103). The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row N1)
found the same tightening generalised to every margin site in the main search,
measured at +6.07 over 8414 games in the open-source record, with the raw static
evaluation kept for storage and learning. A zero on the narrow form does not
close the wide one, and the wide one is where the margins are. Reported
figures decide what to try and never what to conclude (DEC-019); the cost of
this verdict is DEC-143's, not the record's.

## Shape

`src/search.cpp` `negamax` computes `static_eval` once per node (S108). This
step adds one local, the estimate, initialised to `static_eval` and replaced
by the entry's score under the two tightening conditions, and routes the
reverse-futility test (`RFP_MARGIN`) and the null-move static-score condition
through it. Storage of the eval field, the improving flag's history and every
history update keep reading `static_eval`. The quiescence stand-pat site
(`stand_pat` in `quiescence`) is untouched.

## Seeds (DEC-134)

The rule has no constant of its own: the tightening is a comparison, not a
margin. If the implementer finds a guard is wanted -- a minimum depth for the
entry, say -- it is declared in `src/search_params.hpp` with a range whose one
end is off, seeded at form (c), the midpoint, and stated as such.

## From the description (DEC-221)

The technique reached this step as prose from the 2026-09-19 analysis of
literature and open-source resources and its review; the implementer works
from that prose and a publication about the technique, seeds every constant
in DEC-134's forms, and the completion stamp says so.
## Cost

One verdict at DEC-143's price: 25591 games on a bound, 41861 at the midpoint,
12 to 20 hours.
