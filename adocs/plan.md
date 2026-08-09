# Plan

The strongest open-source chess engine in the world, in C++20, bitboard based,
built on the `achesso` branch to find out what AI-driven development can produce
(DEC-013). See `specs.md` for what it must do and what it is today.

**This whole list is phase one:** reach the level the published literature
already describes, by reading documented technique and implementing it here —
never by copying it (DEC-002, DEC-014). Phase two is experimentation and has no
steps yet, and should not get any until the engine is strong enough for an
experiment to mean something. S028 and S029 are the two steps where the agent
stops short of the run itself: it delivers the tuner, the data and the training
program, and the owner executes them (DEC-015).

The order below is not the order of expected Elo, and that is deliberate. S001
to S016 are already done and are here as the record of what each change cost and
bought. Of the rest: S017 comes first because the workflow asserts a surface
check that nothing currently performs. S018 comes second because this engine has
now taken three published Elo figures at face value and measured 0, 0 and
*slower* (DEC-004), so the next evaluation term is chosen from chesso's own
error distribution rather than from what other engines report. S019 to S026 are
the search and ordering features that make evaluation worth having -- a better
score at the leaves is worth less when the tree above them is the wrong shape,
which is the measured reason S006 was worth nothing. S027 and S028 are the
hand-crafted evaluation and its tuning, which exist mainly as the floor that
generates training data for S029. S030 to S032 are movegen work worth 1-3 % each
and are last because they are worth 1-3 % each.

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
19. S019  evaluation terms for the phase the error analysis says costs most
20. S020  compute the in-check state once per node instead of once per call site
21. S021  start the root search in a narrow window around the previous score
22. S022  skip a quiescence capture that cannot reach alpha even if it wins outright
23. S023  history indexed by piece, target and victim, to order captures MVV-LVA rates equal
24. S024  history indexed by the move played n plies ago and the current move
25. S025  retry searching losing captures after the quiets, now that capture history exists
26. S026  drop nodes near the horizon that cannot reach alpha
27. S027  mobility, king safety, passed pawns, pawn structure, bishop pair, tempo
28. S028  fit every evaluation constant at once against self-play game outcomes
29. S029  a perspective network evaluation trained on chesso's own self-play
30. S030  move_t drops the moving piece and becomes 16 bits
31. S031  one unconditional xor for the side-to-move zobrist key instead of two
32. S032  use _pext_u64 for sliding attacks where BMI2 exists, keeping magics as fallback
