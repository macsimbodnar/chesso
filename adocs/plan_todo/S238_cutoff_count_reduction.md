id:         S238
goal:       each node counts the beta cutoffs that happened at it, and the parent reduces its later moves more when the child's count is high -- a child that keeps failing high is a node whose siblings are not worth depth
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; a per-ply cutoff counter on `search_state_t` or handed back through `negamax_at`'s return path, cleared on entry to a node, the step stating which and pricing the cost the way S191 and S231 did; one threshold and one adjustment in `src/search_params.hpp` with stated ranges, seeded at form (c), the midpoint, or at form (b) from a census of per-node cutoff counts at depth 12, the step saying which; at the off value the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); a test drives a child to a stated number of cutoffs and observes the parent's reduction of its next move, with the precondition counted; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant is killed (`tools/mutation_check.py`); Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tools/mutants/, adocs/data/
excludes:   any use of the count outside the reduction; the accumulator (S236, first); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
done:

## Why this exists

The 2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, row
N4) found the open-source record counting beta cutoffs per node and reading the
child's count from the parent to reduce later siblings more, measured at
+6.00 over 8688 games and refined later at +3.52. The mechanism is one
counter and one comparison inside `src/search.cpp` `negamax`'s move loop,
beside `lmr_adjusted_reduction`. Reported figures decide what to try and
never what to conclude (DEC-019); the price is DEC-143's.

## Shape

A counter incremented where the node returns on `beta`, read by the parent
after the child returns and compared against the threshold; over it, the
parent's reduction of its later quiet moves rises by the adjustment, in
S236's fixed-point unit.

## Seeds (DEC-134)

Form (c), the midpoint of each declared range, unless the census the step
may run gives a percentile to seed from -- then form (b), stated at the site.

## From the description (DEC-221)

The implementing agent's brief carries this file and the analysis's row N4 in
prose and nothing else; the technique is implemented from that description,
every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours.
