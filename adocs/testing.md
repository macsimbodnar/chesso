# Testing ledger

Acceptance criteria with their covering tests. Rows are added together with the
feature, never afterwards. Append only. A step cannot complete without a row
referencing its id.

Two kinds of row appear here, and the difference matters. A **test** row names
something in `ctest` that fails on regression. A **measurement** row names a run
that produced a number at a point in time; it does not re-run, and it is
evidence rather than a guard. INV-6 is why measurement rows exist at all: a
change that alters play is retained only against a verdict, and a verdict of
zero is recorded as zero.

Rows S001 to S016 were written retrospectively when moltke was adopted
(DEC-017). Their measurements are transcribed from the commits and the plan
documents they replace; their test columns name tests that exist today.

| Step | Criterion | Covering test | Result |
|---|---|---|---|
| INV-1 | legal-only generation, perft exact against an oracle | `test_movegen` "shallow perft matches every column"; `test_perft` (label `slow`); `bench_movegen` verifies counts before printing | green |
| INV-2 | make/unmake are exact inverses | `test_engine` "hash and board survive make/unmake"; debug build asserts `squares[]` against the bitboards on every make and unmake | green |
| INV-3 | captures and quiets partition the full list | `test_movegen` "captures and quiets partition the list", 90.5 M assertions over a three-ply tree from every test FEN | green |
| INV-4 | accumulators equal a full recomputation | `assert(eval_accumulators_match(...))` at three sites in `src/bitboard.cpp`, debug build only | green |
| INV-5 | `evaluate()` is side-to-move relative | `test_evaluation` "colour symmetry over every test position", "a mirrored start position is balanced" | green |
| INV-6 | behaviour-neutral changes prove it | `tools/search_bench.py` node counts and best moves, compared by hand per change | procedure, not a test |
| S001 | captures and quiets partition `generate_moves` | `test_movegen` "captures and quiets partition the list" | green |
| S001 | search node counts and best moves unchanged | `search_bench.py`, 46622276 / 36703759 / 26765104 identical before and after | measured, identical |
| S001 | quiescence stops generating what it discards | fixed-depth total 9.676 s to 8.593 s | measured, -11.5 % |
| S002 | perft node counts unchanged after templating on colour | `bench_movegen` count check | green |
| S002 | gain tracks the profile | three interleaved rounds per function, 552.5 ms to 485.5 ms | measured, -12.1 % |
| S003 | repetition detection still correct without the stack | `test_engine` "a shuffled knight repeats", "an irreversible move clears the window" | green |
| S003 | `history_entry_t` is 16 bytes and perft is faster | `hyperfine`, 12 interleaved runs, sigma 0.013 s, 2.036 s to 1.925 s | measured, -5.5 % |
| S004 | the unconstrained instantiation drops the masks as dead code | `bench_movegen` count check plus object size 61 to 68 KB | green |
| S004 | gain consistent across every round | three interleaved rounds, perft 475.0 to 466.9 ms, generator 659.2 to 582.1 ms | measured, -1.8 % perft, below the 3 % bar |
| S005 | `board_t` smaller, perft unchanged | `bench_movegen` count check, 208 to 200 bytes | green |
| S005 | the result is recorded even if zero | best-of-N over five interleaved rounds, within 0.3 % | measured, **no effect**, kept anyway |
| S006 | fixed-depth time falls | three positions, about -17 % | measured |
| S006 | an SPRT verdict is recorded whatever it is | fastchess against `62fbdcb`, 340 games, +4.09 +/- 27.52, LLR 0.01, stopped | measured, **0 Elo**, kept |
| S007 | `sizeof(game_t)` falls and perft is unchanged | `bench_movegen` count check; `game_t` 2515 KB to 87 KB | green |
| S008 | no open-coded piece mutation left in `make_move` | debug-build `squares[]` assertions on every make and unmake | green |
| S009 | behaviour-neutral by construction | `search_bench.py`, 43691503 / 36729994 / 27160039 identical, same best moves | measured, identical |
| S009 | `evaluate()` is side-to-move relative | `test_evaluation` "colour symmetry over every test position" (rewritten with the change, not relaxed) | green |
| S009 | a missing king does not price above mate | `test_evaluation` "a missing king is not worth anything" | green |
| S009 | `game_phase()` covers full board, bare kings, per-piece weight, overflow | `test_evaluation` "runs from a full board down to bare kings", "weights the pieces the tables expect", "never exceeds the maximum" | green |
| S010 | tapered tables pass an SPRT | fastchess against the preceding commit, 110-1-1 | measured, passed |
| S010 | the tables prefer the right squares | `test_evaluation` "the tables prefer the right squares", "each piece is worth what the tables say" | green |
| S010 | a bare king endgame is drawn for the right reason | `test_evaluation` bare-king case with the insufficient-material rule | green |
| S011 | PVS reduces fixed-depth time | three positions, -21 % | measured |
| S011 + S012 | an SPRT verdict | fastchess, **+132.4 Elo, passed** | measured, passed |
| S012 | the reduced search never falls to depth 0 | `test_search` "mate in one" and the depth-4 mate case that caught the bug | green |
| S013 | LMR reduces fixed-depth time | three positions, 4.1x | measured |
| S013 | an SPRT reaches a clear trend | fastchess, +129.2 +/- 33.8 over 183 games, killed at 96 % LLR | measured, **never formally concluded** |
| S013 | the root move is never reduced | `test_search` "the winning move is found", "mate in one" | green |
| S014 | behaviour-identical | `search_bench.py` node counts and best moves identical | measured, identical |
| S014 | the accumulators match a full recomputation | `eval_accumulators_match` asserted at three sites, debug build | green |
| S014 | the search is faster | fixed depth | measured, -27 % |
| S015 | `see()` agrees with an independent cross-check | `see_ge` against exact `see()`, 455233 assertions | green |
| S015 | quiescence pruning measured in games | fastchess | measured, **0 Elo**, kept; rerun outstanding, folded into S022 |
| S015 | `see()` cheap enough to be worth calling | fixed depth | measured, -12.1 % |
| S016 | SAN in, per-move cost out, no new dependency | `build/tools/pgn_to_positions` uses the engine's own `algebraic_to_move` | green |
| S016 | a 158-ply game analysed in about 45 seconds at depth 18 | `tools/analyse_game.py`, 159 positions in ~47 s | measured |
| S017 | the command set fails when a command is added, renamed or removed | `test_uci_surface` "the command set is exactly the documented one", read from `uci_command_names()`; observed red with a 17th command, reported `Present but not in the golden list: [eval]` | green |
| S017 | `help` reports the table it walks | `test_uci_surface` "help prints the dispatch table and nothing else" | green |
| S017 | an unrecognised command prints nothing, and a recognised one does | `test_uci_surface` "an unknown command is answered with silence" | green |
| S017 | the option declarations fail on a changed name, default or range | `test_uci_surface` "the option declarations are exactly the documented ones"; observed red with `Threads ... max 4`, reported both directions of the diff | green |
| S017 | `MANUAL.md` fails until it documents every command and option | `test_uci_surface` "MANUAL.md documents every command and every option"; observed red with the `clean-tt` row deleted | green |
| S017 | `MANUAL.md` fails until it documents every `go` and `position` argument | `test_uci_surface` "MANUAL.md documents every go and position argument"; observed red with the `fine70` row deleted | green |
| S017 | `go mate`, `go searchmoves` and `go ponder` are swallowed and the rest of the line still searches | `test_uci_surface` "the ignored go arguments leave the rest of the line working" | green |
| S017 | every `position` shortcut still reaches the board, and no two land on the same one | `test_uci_surface` "every position argument still reaches the board" | green |
| S018 | a PGN of many games in, centipawns lost by phase and by error size out | `tools/error_profile.py` over 210 games, 13522 profiled moves, 27124 positions at 3000000 nodes | measured |
| S018 | the reference limit has a bounded cost | 12 sampled positions: depth 18 median 0.71 s, max 925.90 s; 20 sampled positions: 3000000 nodes mean 4.84 s, max 6.21 s | measured, **fixed depth rejected**, DEC-030 |
| S018 | the profiling match reaches every game phase | 10-game probe: SPRT adjudication 0 endgame moves, loose adjudication 289 | measured, **loose adopted**, DEC-031 |
| S018 | a delivered checkmate costs nothing | three real mated FENs from `pgn_to_positions` score -100000, clamp to -1000, so cost is 1000 + -1000 = 0; observed wrong first, at +1000 for 20 moves and 20106 cp | green, red observed |
| S018 | the repair changed only the contaminated games | corrected total 407740 cp against 424740, a difference of exactly 17000 = 17 games x 1000 | measured, identical elsewhere |
| S018 | an interrupted run resumes without loss | 3 games, then `--resume`, gives the same 115 moves as an uninterrupted run | green |
| S018 | the phase ranking is not an artefact of the clamp | re-bucketed excluding 1347 mate-touching moves, then excluding clamped reference scores; ranking unchanged in both | measured, stable |
| S018 | the phase that costs most is named by measurement | early middlegame 44.1 cp/move and 36.0 % of loss; endgame 18.3 cp/move and 20.5 % | measured, **contradicts the S016 anecdote**, S019 affected |
| DEC-033 | 16x more search removes only a quarter of the error | `tools/depth_vs_eval.py` over 160 positions: 170.1 cp/move at 4M nodes, 129.1 at 64M, 95 of 160 moves unchanged; evidence in `adocs/data/DEC033_depth_vs_eval.tsv` | measured, **evaluation-limited**, plan reordered |
| S028 | the tuner's model computes what evaluate() computes | `test_eval_model` "the model reproduces evaluate() on every phase", 13 positions from a full board to bare kings, both sides to move | green, red observed: dropping the black mirror in `parse_placement` gives 408 cp of disagreement at the start position |
| S028 | the model tapers on the engine's own phase | `test_eval_model` "the model's phase is game_phase()" | green |
| S028 | those positions can tell colours and phases apart | `test_eval_model` "the positions can tell colours apart": at least 5 score non-zero, and both a phase <= 6 and a phase >= 22 appear | green, non-vacuous by construction |
| S028 | the fit reduces held-out error on data it never saw | `tools/tuner` over 1490839 positions, 1341756 train / 149083 validation, K = 1.1141 fitted from the data: validation 0.113852 to 0.108043, train 0.113554 to 0.107749, stopped at epoch 11200 on `--patience 20` | measured, **improved 5.1 %** |
| S028 | the ordering values are not silently unified with the evaluation's | `-Wmacro-redefined` under `-Werror` on the five names; the ordering values now live under `MVV_` in evaluation.cpp | green, red observed: the paste failed to compile with five errors before the rename |
| S028 | the rename does not change what the engine plays | `tools/search_bench.py` depth 9, both binaries: 486222 / 1269618 / 159208 nodes, best moves c3d5 / e2a6 / d7c8q | measured, identical, INV-6 |
| S028 | a changed table cannot pass unnoticed | `test_evaluation` "each piece is worth what the tables say" and four anchors in `test_search`; all five moved on the tuned tables | green, red observed: 105/260/292/500/897 and 500, 208, -475, -482 all failed |
| S028 | the re-anchored values are not read off the engine | an independent evaluate() over the fitted constants agrees on all of them: 112, 210, 277, 511, 971, 0, and 276, -453, and -474 as the best of four evasions | measured, two implementations agree |
| S028 | the promotion case has one right answer | Stockfish depth 20 multipv 4: the old position `8/P6k/8/8/8/8/8/4K3` gives four moves at mate 10; `8/P1k5/8/8/8/8/8/4K3` gives a8=Q mate 14 against a8=R +387, a8=B +10, Ke2 0 | measured, **test was asserting a preference**, position replaced |
| S028 | the replacement case tests the engine, not the tuning | `test_search` "the winning move is found" passes on the hand-written tables as well as the fitted ones | green, both |
