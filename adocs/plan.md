# Plan

The strongest open-source chess engine in the world, in C++20, bitboard based,
built on the `achesso` branch to find out what AI-driven development can produce
(DEC-013). See `specs.md` for what it must do and what it is today.

**This whole list is phase one:** reach the level the published literature
already describes, by reading documented technique and implementing it here —
never by copying it (DEC-016, DEC-014). Phase two is experimentation and has no
steps yet, and should not get any until the engine is strong enough for an
experiment to mean something. S028 and S029 are the two steps where the agent
stops short of the run itself: it delivers the tuner, the data and the training
program, and the owner executes them (DEC-015).

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
are movegen work worth 1-3 % each and are last because they are worth 1-3 % each.

**S035 to S043 are the 2026-08-13 adversarial audit's nine findings, one step
each, and seven of them cut ahead of S033.** Not out of politeness to the
auditor: S035 is the SPRT harness, which has not run since `44877c4`, so no
verdict on anything below it is obtainable until it is fixed; S036 is a search
that never returns on a 1 ms clock, which corrupts the games running beside it
in the same match; S037 is the node count that `search_bench.py` reads, which is
how INV-6 is discharged. Those three are the measuring instruments, and the rest
of this plan is measured with them. S043, S040, S041 and S038 follow because
they are cheap and each one removes a way for a later fit or a later
neutrality claim to be quietly wrong. S039 and S042 cost an SPRT each and buy
little, so they wait behind the search block rather than ahead of it.

S019 is retired. It was written from one game, DEC-032 showed the endgame is the
cheapest phase per move, and DEC-033 showed endgame errors are the least
depth-fixable of all. Its content belonged to S027 from the start. The id is not
reused.

Order lives here and nowhere else. Step detail lives in the step files under
`plan_todo/`, `plan_current/`, and `plan_done/`. Ids are allocated in creation
order and never renumbered, so reordering is a one-line edit to this list.

Order is read from the list entries below, the lines starting `1.`, `-`, or `*`.
An id named in a sentence anywhere else in this file is prose: it does not change
the order, and it is not checked. Every step file must appear as a list entry, and
every list entry must have a step file — both are INV-3.

<!-- 1. S001  short goal -->
18. S018  rank chesso's own errors by game phase over hundreds of games, from Stockfish
19. S028  fit every evaluation constant at once against self-play game outcomes
20. S034  compute the cheap evaluation terms first and skip the expensive ones when the score is already outside the window
21. S027  king safety, passed pawns, pawn structure, bishop pair, tempo
22. S035  restore fastchess.sh so a match actually runs, and stop the EXIT trap masking a failure as status 0
23. S036  a go command with a 1 ms clock returns a bestmove instead of searching forever
24. S037  info nodes reports the whole search's node count so search_bench.py compares the whole tree
25. S043  delete the CMAKE_TOOLCHAIN_FILE line that names a file that does not exist
26. S040  re-derive the DEV_MANUAL tuner section from tools/tuner.cpp and eval_model.hpp
27. S041  a test that fails the moment a tuner group range is appended without re-ending the one before it
28. S038  the tuner-model guard states a tolerance the truncation arithmetic actually supports
29. S033  prune a node whose static score is already far enough above beta
30. S021  start the root search in a narrow window around the previous score
31. S026  drop nodes near the horizon that cannot reach alpha
32. S024  history indexed by the move played n plies ago and the current move
33. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
34. S025  retry searching losing captures after the quiets, now that capture history exists
35. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
36. S020  compute the in-check state once per node instead of once per call site
37. S039  re-decide LAZY_EVAL_MARGIN from measured spread at the weights that ship today
38. S042  set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
39. S029  a perspective network evaluation trained on chesso's own self-play
40. S030  move_t drops the moving piece and becomes 16 bits
41. S031  one unconditional xor for the side-to-move zobrist key instead of two
42. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
