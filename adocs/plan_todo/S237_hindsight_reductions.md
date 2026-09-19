id:         S237
goal:       at a child node, the reduction its parent applied to the move that reached it is read together with how the static evaluation moved across that move: a heavily reduced move whose evaluation got worse for the mover gets a ply back, a lightly reduced one whose evaluation improved gives one up
accepts:    an SPRT verdict against a named commit at `{0, 5}` nElo, recorded whatever it is; the parent's reduction reaches the child as a trailing parameter of `negamax_at` in S231's pattern or as a field on `search_state_t`, the step stating which and pricing the read the way S191 and S231 did; two thresholds and two adjustments in `src/search_params.hpp` with stated ranges, seeded at form (b) from a census of reductions and evaluation deltas at depth 12 the step runs at its start, or at form (c), the step saying which per constant; at the off values the tree is the parent's exactly, bench signature identical (INV-6, DEC-215); a test per branch -- give back, give up, neither -- with the precondition counted; `tests/test_search.cpp` "pruning does not hide a forced mate" stays green; a mutant per branch is killed (`tools/mutation_check.py`); the form built is the base case only, no later variant; Debug self-play and `tools/gate_extra.sh` before completion (DEC-141); the fast suite green in both builds
touches:    src/search.cpp, src/search_params.hpp, src/data_structures.hpp, tests/test_search.cpp, tools/mutants/, adocs/data/
excludes:   any change to the reduction formula itself (S236, which lands first so the adjustment can be fractional); reading history in this rule; any constant from another engine (DEC-084, DEC-105, DEC-134)
decisions:  DEC-222, DEC-221, DEC-105, DEC-134, DEC-141, DEC-215
closes:
blocks:
paused_by:
done:

## Why this exists

Depth is decided at the parent and never revisited. The 2026-09-19 study
(`adocs/data/2026-09-19_search_technique_study.md`, row N5) found the engine
it read correcting it one ply down: the child reads its parent's reduction
beside the static-evaluation delta across the move and gives a ply back or up,
measured at +6.03 over 8590 games. Its review (`adocs/audit/2026-09-19_study_review.md`,
F09) records that the engine later simplified two variants away while keeping
the base rule, so the base rule is what this step builds. S098 made the
reduction a first-class quantity at the parent; S231 showed the cheap way to
hand a per-node value down (a trailing parameter, not a per-ply stack, 1.49 %
of nodes per second for one read per node at S191). Reported figures decide
what to try, never what to conclude (DEC-019).

## Shape

`src/search.cpp` `negamax` receives the reduction its parent applied to the
move that reached it; with `static_eval` at the child and the parent's, the
two conditions fire before the move loop and move `depth` by one ply either
way, bounded so a chain cannot compound (the S097 plumbing when it exists,
a plain cap before).

## Seeds (DEC-134)

Form (b) where a census gives a percentile to seed from -- the reduction
threshold at the p75 of applied reductions, the delta thresholds at the p50 of
absolute evaluation deltas across a reduced move -- and form (c) otherwise,
each stated at its site.

## Clean room (DEC-221)

The implementing agent's brief carries this file and the study's row N5 in
prose and nothing else; it does not open the clone `.moltke.local.md` names.

## Cost

One verdict at DEC-143's price, 12 to 20 hours, plus the census hour.
