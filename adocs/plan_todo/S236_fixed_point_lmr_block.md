id:         S236
goal:       the late move reduction is accumulated in fixed point -- the table's value and every node and move term as fractions of a ply -- and rounded to whole plies once at the site that uses it, and S098 verdict 1's history-scaled term returns as a fractional contribution, the two measured as one block because neither means anything without the other
accepts:    one SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; the accumulator's scale is a compile-time constant in `src/search.cpp` beside `build_lmr_table`, the table holds fixed-point values, `lmr_adjusted_reduction` sums table and node terms in that unit and rounds once at its return, and `lmr_node_adjustment`'s four S098 terms are expressed in the same unit; with the history term at its off value and the rounding set to truncate exactly as today's `uint8_t` cast does, the tree is the parent's exactly, bench signature identical (INV-6), proved on the tree and not assumed from a range's end (DEC-215); the history term's scale and clamp are parameters in `src/search_params.hpp` with stated ranges, seeded at form (b) from a census of the fitted quiet-history sum's distribution at depth 12 the step runs at its start (the DEC-212 pattern), never from S098 verdict 1's fitted 699 / 3, which were fitted against a term of a different unit; `accepts` states that per-part attribution is deliberately forfeited (DEC-082) and names the bisection on H0 -- leg 1 the accumulator alone at `{-5, 0}`, leg 2 the term alone at `{0, 5}`; a test asserts the rounding rule at its boundary and a test asserts the fractional term moves a reduction by less than a ply where the whole-ply form could not; a mutant per part is killed (`tools/mutation_check.py`); `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tools/mutants/, adocs/data/
excludes:   any further reduction term (S237, S238, corrplexity); which moves are reduced; the re-search rule (`lmr_research_depth`); any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-213, DEC-082, DEC-212, DEC-215, DEC-141, DEC-105, DEC-134
closes:
blocks:
paused_by:
done:

## Why this exists

`src/search.cpp` `build_lmr_table` truncates a `double` into a `uint8_t` and
`lmr_node_adjustment` adds whole plies to it: a term that wants to say
"reduce a third of a ply less here" can say nothing or a whole ply. S098
verdict 1 measured the history-scaled reduction at -2.92 +/- 5.02 and its
bisection leg at -2.34 +/- 4.73, and the term left the tree (DEC-213). The
2026-09-19 study (`adocs/data/2026-09-19_search_technique_study.md`, section
3.3) records that the open-source record measures the same idea at +9.09 over 4970
games on a reduction accumulated in fixed point, and names three differences
that could explain a sign flip; the accumulator is the one testable without
also changing the history sum. Its review (`adocs/audit/2026-09-19_study_review.md`,
F13) adds the honest framing: no published number prices fixed point itself,
and the accumulator alone changes only rounding, so the hypothesis is tested
only with the term restored as a fraction. Hence one block: the parts are
inert in isolation, which is DEC-082's precondition, and H0 is bisected.

## Shape

The table becomes an array of fixed-point values; `lmr_adjusted_reduction`
returns the rounded sum; the four S098 node terms scale to the unit so the
shipped tree is reproduced exactly at the off configuration. The history term
returns as `-(hist_sum * scale) / divisor`, clamped, in the same unit -- a
fraction of a ply at typical sums rather than S098 verdict 1's whole ply.

## Seeds (DEC-134)

The accumulator's scale is a power of two chosen for the integer range,
stated as such (form (c), a design constant and not a tuned one). The
history term's divisor and clamp: form (b), a census of `quiet_history_sum`'s
distribution at depth 12 over the bench positions, run at the step's start
and recorded under `adocs/data/`, seeding the divisor at the value that makes
the p90 sum a half-ply.

## From the description (DEC-221)

The implementing agent's brief carries this file, the analysis's section 3.3
in prose and DEC-213's record; the technique is implemented from that
description, every constant seeded in DEC-134's forms, and the stamp says so.

## Cost

One verdict at DEC-143's price, 12 to 20 hours; on H0 two legs more.
