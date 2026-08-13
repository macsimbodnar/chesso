# Plan

The strongest open-source chess engine in the world, in C++20, bitboard based,
built on the `achesso` branch to find out what AI-driven development can produce
(DEC-013). See `specs.md` for what it must do and what it is today.

**This whole list is phase one:** reach the level the published literature
already describes, by reading documented technique and implementing it here —
never by copying it (DEC-016, DEC-014). Phase two is experimentation and has no
steps yet, and should not get any until the engine is strong enough for an
experiment to mean something. S029 is the one step where the agent stops short
of the run itself: it prepares the data and the training program, and the owner
runs the network training (DEC-015 as amended by DEC-041 — fits, measurements
and evaluation tuning are the agent's to run, and S028's fit was run that way).

The order below is not the order of expected Elo, and that is deliberate. S001
to S018 are already done and are here as the record of what each change cost and
bought. S017 came first because the workflow asserts a surface check that nothing
performed. S018 came second because this engine has now taken three published Elo
figures at face value and measured 0, 0 and *slower* (DEC-019), so what is worked
on next is chosen from chesso's own error distribution rather than from what
other engines report.

**The rest of the order is what S018 and DEC-033 measured, and it is not what it
was.** The plan used to run the search and ordering features first, on the
argument that a better score at the leaves is worth less when the tree above them
is the wrong shape. That argument was tested: 160 expensive moves re-asked at 16
times the search removed 24.1 % of the error, 10.3 cp per doubling, and 95 of the
160 moves did not change at all. The engine mostly is not missing the refutation,
it believes the move — and it believes it with material and a hand-written
piece-square table that has never been fitted to anything. So the evaluation
leads: **S028 fits the constants that already exist, S027 adds terms and fits
them the same way.** The search and ordering block follows, still worth doing and
now with a number on what it is worth. S029 is the network, for which the tuned
hand-crafted evaluation is the floor that generates training data. S030 to S032
are movegen work worth 1-3 % each — under 1 % for S031, its own file says — and
are last because that is what they are worth.

**S035 to S043 are the 2026-08-13 adversarial audit's nine findings, one step
each, and seven of them cut ahead of S033.** Not out of politeness to the
auditor: S035 was the SPRT harness, dead since `44877c4` with no verdict
obtainable below it, repaired and smoke-tested first; S036 was a search that
never returned on a 1 ms clock, corrupting the games beside it in a match; both
are done. S037 is the node count that `search_bench.py` reads, which is how
INV-6 is discharged — the last of the three instruments still pending. Those
three are the measuring instruments, and the rest of this plan is measured with
them. S043, S040, S041 and S038 follow because
they are cheap and each one removes a way for a later fit or a later
neutrality claim to be quietly wrong. S039 and S042 cost an SPRT each and buy
little, so they wait behind the search block rather than ahead of it.

**S044 to S052 are the 2026-08-13 plan_review audit's nine findings, one step
each, ahead of everything still pending: they are pure text, cost minutes
each, and they correct the documents the pending steps execute against.** They
correct the plan and its step files rather than the engine: S045, S046, S050
and S051 each remove a way for a later step to alter play unmeasured or to
record a verdict against the wrong thing; S044, S047 and S048 retire premises
that DEC-041 and the DEC-049 machine move made false; S049 and S052 re-point
one field each. All nine together cost less than one SPRT, and every step they
correct sits behind them in this order.

S019 is retired. It was written from one game, DEC-032 showed the endgame is the
cheapest phase per move, and DEC-033 showed endgame errors are the least
depth-fixable of all. Its content belonged to S027 from the start. The id is not
reused.

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered, so reordering is a one-line edit to this list.

Order is read from the list entries below, the lines starting `1.`, `-`, or `*`.
An id named in a sentence anywhere else in this file is prose: it does not change
the order, and it is not checked. Every pending step file must appear as a list
entry and every list entry must name an existing step file; the workflow checker
enforces the correspondence, adds an entry when a step is created, and prunes the
oldest completed entry — taking its testing.md rows with it — as newer
completions land. `plan_done/` and git history keep everything pruned.

<!-- 1. S001  short goal -->
33. S037  info nodes reports the whole search's node count so search_bench.py compares the whole tree
34. S043  delete the CMAKE_TOOLCHAIN_FILE line that names a file that does not exist
35. S040  re-derive the DEV_MANUAL tuner section from tools/tuner.cpp and eval_model.hpp
36. S041  a test that fails the moment a tuner group range is appended without re-ending the one before it
37. S038  the tuner-model guard states a tolerance the truncation arithmetic actually supports
38. S033  prune a node whose static score is already far enough above beta
39. S021  start the root search in a narrow window around the previous score
40. S026  drop nodes near the horizon that cannot reach alpha
41. S024  history indexed by the move played n plies ago and the current move
42. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
43. S025  retry searching losing captures after the quiets, now that capture history exists
44. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
45. S020  compute the in-check state once per node instead of once per call site
46. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
47. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
48. S029  a perspective network evaluation trained on chesso's own self-play
49. S030  move_t drops the moving piece and becomes 16 bits
50. S031  one unconditional xor for the side-to-move zobrist key instead of two
51. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
52. S053  testing.md's header states the checker's retention: a pruned plan entry takes its ledger rows with it
