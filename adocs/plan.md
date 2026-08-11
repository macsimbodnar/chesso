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
1. S001  generate_captures and generate_quiets partition generate_moves, so quiescence stops generating what it discards
2. S002  make_move, unmake_move and generate_moves take colour as a template parameter
3. S003  drop repetition_t and walk history.entries[].hash instead
4. S004  specialise generate_moves on whether checkers or pins constrain the move list
5. S005  remove the padding in board_t and put hash next to the scalars make_move writes
6. S006  negamax searches the captures before generating the quiets
7. S007  one process-wide copy of the attack tables instead of one per game_t
8. S008  every piece change in make_move goes through add_piece, remove_piece, move_piece
9. S009  king out of material, side-to-move-relative evaluate(), game_phase()
10. S010  tapered piece-square tables and an insufficient-material draw rule
11. S011  search the first move with a full window and the rest with a null window
12. S012  give the opponent a free move and prune when the result still fails high
13. S013  search late quiet moves at reduced depth and re-search when they beat alpha
14. S014  maintain material, psqt and phase in make_move instead of recomputing them
15. S015  exact see() and fast see_ge(), and decline losing captures in quiescence
16. S016  turn a game into per-move cost from Stockfish instead of reading the move list
17. S017  a test over the UCI command and option surface that fails when it changes
18. S018  rank chesso's own errors by game phase over hundreds of games, from Stockfish
19. S028  fit every evaluation constant at once against self-play game outcomes
20. S034  compute the cheap evaluation terms first and skip the expensive ones when the score is already outside the window
21. S027  mobility, king safety, passed pawns, pawn structure, bishop pair, tempo
22. S033  prune a node whose static score is already far enough above beta
23. S021  start the root search in a narrow window around the previous score
24. S026  drop nodes near the horizon that cannot reach alpha
25. S024  history indexed by the move played n plies ago and the current move
26. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
27. S025  retry searching losing captures after the quiets, now that capture history exists
28. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
29. S020  compute the in-check state once per node instead of once per call site
30. S029  a perspective network evaluation trained on chesso's own self-play
31. S030  move_t drops the moving piece and becomes 16 bits
32. S031  one unconditional xor for the side-to-move zobrist key instead of two
33. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
