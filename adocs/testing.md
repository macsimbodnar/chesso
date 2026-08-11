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
| S028 | the fitted constants beat the hand-written ones in play | `REF=9fc6fdf ./fastchess.sh`, full bounds `elo0=0 elo1=5 alpha=0.05 beta=0.05`, 10+0.2, concurrency 3: **+188.74 +/- 32.21 Elo** over 438 games, LLR 2.95 crossing 2.94, H1 accepted. 294 wins, 77 losses, 67 draws, Ptnml [5, 9, 65, 44, 96] | measured, **SPRT passed** |
| DEC-035 | the phase ranking still holds after the constants were fitted | `tools/error_profile.py` over 98 games and 5582 moves, same reference and 3000000-node limit as S018: early middlegame 35.2 cp/move and 38.8 % against 44.1 and 36.0 %; endgame flat at 18.6 against 18.3. Evidence `adocs/data/S028_raw.tsv` | measured, **ranking unchanged**, plan order kept |
| DEC-035 | the engine is still evaluation-limited after the fit | `tools/depth_vs_eval.py`, same criteria and sample size as DEC-033: 29.8 % of the error removed by 16x search against 24.1 %, 78 of 160 moves unchanged against 95, 10.4 cp per doubling against 10.3 | measured, **shifted toward search, order not turned over** |
| DEC-035 | the fitted evaluation is no longer systematically optimistic | same profile: mean bias -1.5 / -24.3 / -40.3 / -3.5 / +204.3 by phase, against +39 / +77 / +100 / +42 / +30 before; medians 6 / 0 / -4 / -1 / 227 | measured, **reversed in four phases of five**, MANUAL.md rewritten |
| S028 | the gain shows against a fixed opponent, not only against the previous commit | 120 games achesso vs sgambetto at 10+0.2 under the loose adjudication of DEC-031: 67.5 %, +126.97 +/- 60.30 Elo, against 39.3 % over 210 games under identical settings at S018 | measured, **chained SPRT gains are not drift** |
| DEC-036 | an evaluation term can be priced without the search confounding it | `tests/bench_eval`, ten positions from a full board to bare kings, scores and checksum printed before any timing; 40000000 calls a sweep, resolution 0.1 % over three interleaved passes | green, instrument |
| DEC-036 | recomputed mobility costs what INV-4 exists to prevent | `bench_eval`: 1.31 ns per call against 15.93, 12.2x. `search_bench` depth 12: nps -32.8 % kiwipete, -45.5 % midgame, -23.7 % tactical, wall time +33 % | measured, **probe reverted, nothing shipped** |
| DEC-037 | recomputed mobility is decided by play, not by nodes per second | `REF=c8fe860 ./fastchess.sh --fast`, 10+0.2: **-14.93 +/- 16.44 Elo** over 1164 games, LLR -2.23 crossing -2.20, H0 accepted, LOS 3.72 %, Ptnml [77, 129, 203, 113, 60] | measured, **rejected and reverted**, weights were never fitted |
| S034 | a table probe is priced against the cost of calling evaluate() | `tools/probe_cost`: index loop 0.08 ns, 524288-entry table access 0.80-1.08 ns, 256 KB cache access 0.36 ns, against `evaluate()` at 1.36 ns. `sizeof(tt_entry_t)` 24 bytes with 20 used | measured, **the probe is cheaper than the evaluation** — the step's opening concern was wrong |
| S034 | the saving is compared against the noise floor before anything is built | a node costs about 116 ns; replacing a 1.36 ns evaluation with a 0.36 ns hit saves under 1 %, against a 3 % noise floor | measured, **nothing for an SPRT to see today** |
| S034 | `bench_eval` is checked against the search it is meant to predict | kiwipete, 9095066 nodes against 8860613 so per-node cost is comparable: 115.7 ns against 172.1, a difference of 56.4 ns per node where `bench_eval` measured 14.6 | measured, **the instrument understates an occupancy term 3.9x**, cause is magic-table cache pressure |
| S034 | the lazy shortcut cannot return a score that changes a decision | `test_evaluation` "the lazy shortcut cannot change a decision" over the whole corpus: correction within `LAZY_EVAL_MARGIN`, exact score when the window contains it, cheap score plus a correct-side guarantee when it does not | green, **red observed**: `1Bk5/B1B5/1B1B4/B1B4B/8/B1B5/5K2/8` gave 155 against a margin of 150 before the term was clamped |
| S034 | the margin is chosen from data, not from a guess | tapered mobility over 149084 S028 self-play positions: p50 19, p75 34, p90 50, p95 60, p99 81, p99.9 107, max 143 | measured, margin set to 150 |
| S034 | quiescence still reports a sound cutoff under the shortcut | `test_search` "a quiet position stands pat", re-targeted: asserts the precondition, that the cutoff returns the cheap score, and that both it and the exact score clear beta | green |
| S034 | the staging recovers most of the term's cost | `search_bench` depth 12 kiwipete, node counts within 4 %: 116.6 ns per node baseline, 172.1 always computed, 138.4 behind the shortcut. -32.8 % nps becomes -14.8 % | measured, **61 % of the cost removed** |
| S034 | the staging is worth what the strength test says, not what the clock says | `REF=3f90f89 ./fastchess.sh --fast`: +4.19 +/- 24.11 Elo over 498 games, LLR +0.01 of +/-2.20, LOS 63.3 %, Ptnml [21, 61, 86, 53, 28] | **partial, stopped, consistent with zero** — not a verdict, DEC-040 |
| S034 | the tuner's model still computes what the engine computes | `test_eval_model`, PARAM_COUNT 773 to 781, mobility ray-walked in the model rather than shared with the engine | green, tolerance widened 1 to 2 cp for the second truncating division |
| S034 | the tuner emits every parameter it fits | 20-epoch smoke run over 3000 rows writes `mobility_mg` and `mobility_eg` into the pasteable header | green, **red observed**: the writer emitted only the tables, so eight fitted weights were being discarded |
| S034 | the tuner actually fits the parameters it added | same smoke run moves the weights off their starting values and reaches 0.063586 against 0.066072 without the gradient term | green, **red observed**: the weights came back exactly 4/5/2/1 and 4/5/4/2 because `gradient()` had no mobility block |
| S034 | the fitted weights are worth something in play | `REF=fcf0025 ./fastchess.sh --fast`, the only difference being the 781 fitted constants: **+28.46 +/- 18.61 Elo** over 942 games, LLR 2.24 crossing 2.20, H1 accepted, LOS 99.87 %, Ptnml [50, 76, 160, 117, 68] | measured, **SPRT passed**, DEC-040's revert clause does not fire |
| S034 | the fit finds what hand-picking missed | fitted mobility mg {0, 5, 9, 2} eg {-1, 6, 1, 3} against hand-picked {4, 5, 2, 1} and {4, 5, 4, 2}; held-out error 0.108134 to 0.107402 over 781 parameters | measured, knight mobility fitted to nothing, rook middlegame to four times the guess |
| S034 | a match interrupted by hibernation is not a measurement | three games flagged with ~988000 ms overruns, exactly the three in flight at concurrency 3; run discarded and restarted from zero | **contaminated run rejected**, DEC-020 |
| S027 | the engine's nine king safety counts can be read without a second implementation | `king_safety_features()` is the `<true>` instantiation of the fused loop `evaluate()` calls at `<false>`; one extraction, two instantiations | green |
| S027 | the tuner's model counts what the engine counts, per colour | `test_eval_model` "the model counts what the engine counts", 18 positions x 2 colours x 9 counts; per colour and not as the difference the tuner fits, which cancels a bug that credits both kings | green, red observed: one knight step altered in the model prints `CHECK( 3 == 2 )`, White zone attacks, on `r1bq2k1/ppp2ppp/2n4r/1B1p4/3P2n1/2N5/PPP2PPP/R1BQ1RK1 b - - 0 1` |
| S027 | every count is exercised, and in both directions | `test_eval_model` "the positions exercise every count": each of the nine non-zero somewhere, and White-minus-Black taking both signs | green, non-vacuous by construction; **red observed on the original 13 positions**: knight and rook attackers never non-zero at all and seven of nine counts only ever pointed one way, so five positions were added |
| S027 | no caller of the model can drop the king safety term by omission | the defaulted `king_safety = nullptr` deleted from `eval_model::evaluate()`; `tools/tuner` and `test_eval_model` both pass the features | green |
| S027 | the lazy margin bounds two terms and still holds | `test_evaluation` "the lazy shortcut cannot change a decision", unchanged over the whole corpus | green; `worst` is still mobility's alone while the king safety weights are zero, which is why the counts are checked in `test_eval_model` instead |
| S027 | exposing the counts changes nothing the engine plays | `tools/search_bench.py` depth 9, both binaries: 212136 / 870209 / 174998 nodes, best moves c3d5 / e2a6 / d7c8q; `bench_eval` checksum -36766996307977 identical over 10 positions | measured, identical, INV-6 |
| S027 | exposing the counts costs the hot path nothing | `hyperfine --warmup 1 --runs 8` interleaved on an idle machine: 627.0 +/- 0.9 ms before, 629.2 +/- 2.5 ms after; `bench_eval` 18.68 to 18.52 ns per call | measured, **+0.35 %**, below the 3 % bar with tight sigma |
| S027 | a second caller can un-inline a term that is supposed to be free | `bench_eval` 22.55 ns per call against 18.60 once the collecting instantiation gave `king_shelter_features` a second caller; `nm` shows it emitted out of line, and the zero weights no longer fold it away | measured, **21 % regression found and fixed**; `static inline` restores 18.54 |
| S027 | the documented A/B benchmark command runs | `DEV_MANUAL.md` quoted `bench_eval -r 1`, which exits 1 with `-r must be at least 2, or the two halves cannot be compared`; corrected to `-r 2` | green, red observed by running the documented line |
| S027 | the unclamped correction a tool reads is the engine's own arithmetic | `test_evaluation` "the unclamped terms are the engine's own": clamping `evaluate_expensive_terms()` reproduces `evaluate() - evaluate_cheap()` over the whole corpus, and the run counts the positions where the clamp binds | green; red observed by clamping the accessor, which fails `clamped > 0` at 0 and passes every equality — the guard exists for exactly that |
| S027 | exposing the unclamped correction costs the hot path nothing | `hyperfine --warmup 1 --runs 8` both orders, `bench_eval -r 9 -n 4000000`: 7.606 +/- 0.039 s HEAD against 7.616 +/- 0.029 s patched, then 7.772 +/- 0.034 s HEAD against 7.695 +/- 0.110 s patched; `bench_eval` checksum -36766996307977 identical | measured, **the sign flips with the order**, so the difference is drift under 1 % and not code; the accessor rides the collecting instantiation, so the search's keeps one caller |
| S027 | the shortcut returns a bound on both sides of the window, not a value | `test_evaluation` "the lazy shortcut cannot change a decision", re-targeted: at beta the return is `>= beta` and `<= evaluate()`, at alpha `<= alpha` and `>= evaluate()`, and exactly `evaluate()` when the window contains the score | green; strictly stronger than the `== evaluate_cheap()` it replaced, which pinned the implementation — that line printed `REQUIRE( -5466 == -5316 )` on `1BQKRBRN/PPPPPPPP/1N6/8/8/8/3k4/8 b - - 1 1` once the shortcut started returning the guaranteed bound |
| S027 | a shortcut return is never better than the truth | `test_evaluation` "a shortcut return is never better than the truth" over all 2696 corpus positions: the beta-side return is `<= evaluate()`, the alpha-side return is `>= evaluate()`, and the run counts the positions where returning the cheap score would break each bound rather than merely fail to prove it | green, **red observed** with `return cheap` put back: `CHECK( -3358 <= -3475 )` on `1Bk1B3/B1B5/1B1B4/B1B5/1B6/B7/5K2/8 b - - 1 1`, 2613 of 5392 bound assertions failing — 1473 at beta, 1140 at alpha |
| S027 | a fail-soft cutoff propagates a lower bound that is one | `test_search` "a quiet position stands pat", re-targeted from `cut == evaluate_cheap()`: the cutoff clears beta and the number returned is below the exact score, strictly, because mobility is worth +16 on that position so a quiescence that computed the expensive terms would have answered 540 | green; the replaced line printed `REQUIRE_EQ( 374, 524 )` |
| S027 | a test whose claim is wider than what it proves says so | comment on `test_search` "the table never changes the answer": whether the shortcut fires depends on the window, the table changes windows, so the search is still not a pure function of position and depth at any depth — passing on this corpus at depths 2 and 3 is weaker than purity | green and deliberately not narrowed; it is what caught the unsound bound, answering 110 or 59 on the same position depending on what was cached |
