# Measurement evidence

Raw output of runs that a decision or a completed step rests on. Kept because
regenerating any of it costs hours of reference search, and because a number in
`decisions.md` that cannot be re-derived is an assertion rather than a
measurement.

Append only in practice: a file here is the evidence for something already
recorded, so it is added, never edited.

| file | what it is |
|---|---|
| `S018_match.pgn` | the 210 games achesso vs sgambetto at 10+0.2 that S018 profiled, loose adjudication (DEC-031) |
| `S018_raw.tsv` | 13522 per-move records from `tools/error_profile.py` over that PGN, scored by Stockfish `dev-20260803-762dd1da` at 3000000 nodes (DEC-030). Re-bucket with `tools/error_profile.py --from-raw`, no engine needed |
| `DEC033_depth_vs_eval.tsv` | 160 positions from `S018_raw.tsv` re-asked of chesso at 4M and 64M nodes and re-costed, produced by `tools/depth_vs_eval.py`. The evidence for DEC-033 |
| `S028_match.pgn` | 98 profiled games of the same match, achesso vs sgambetto at 10+0.2, loose adjudication, played by the fitted evaluation. 120 were played; the profile was cut at 98 for time and the PGN holds all 120 |
| `S028_raw.tsv` | 5582 per-move records over those 98 games, same reference and same 3000000-node limit as `S018_raw.tsv`, so the two are comparable. The evidence for DEC-035 |
| `S028_depth_vs_eval.tsv` | 160 positions from `S028_raw.tsv` through the same probe as `DEC033_depth_vs_eval.tsv`. Also DEC-035 |
| `S033_rfp_sweep.tsv` | 45 reverse-futility settings, margin x lower depth bound x upper depth bound. The evidence for S033's "green is luck" section and for DEC-060 |
| `S033_rfp_ply_sweep.tsv` | 10 settings, first pruned ply x margin, at the upper depth bound that shipped. The evidence for the ply bound S033 shipped, and the table S068 argues from |
| `S033_rfp_guard_sweep.tsv` | 11 settings, five candidate guards x margin. The evidence that no guard works, DEC-060 |
| `S033_rfp_*.sh` | the script that produced the table of the same name, verbatim as run. See "the scripts no longer run" below |
| `S033_rfp_*.log` | the console transcript of that run. Carries no column the `.tsv` lacks -- read the `.tsv`; the log is here only to show the run went to completion in order |
| `S068_sprt_run1.sh` | the script that ran S068's first SPRT, verbatim as run except for its filename (`run_s068_sprt.sh` in the session that ran it). `elo0=0 elo1=5`, the `fastchess.sh` default, which returned no verdict |
| `S068_sprt_run1.log` | that run's full console, 21753 lines. Every game's result and adjudication reason, every periodic SPRT block, the deliberate `Terminated` and the wrapper's `elapsed_seconds=23775` / `status=143` |
| `S068_sprt_run2.sh` | the script that ran the second SPRT (`run_s068_sprt2.sh` as run). `elo0=-5 elo1=5`. **Its header comment carries the pre-registered interpretation of each outcome and the bounds reasoning, written before the run started**, which is what makes S068's reading of the result a pre-registration rather than a choice made after seeing it. DEC-063 |
| `S068_sprt_run2.log` | that run's full console, 5571 lines, ending `SPRT ([-5.00, 5.00]) completed - H1 was accepted` and `Total Time: 01:41:13` |
| `S068_run2_match.pgn` | the 2312 games of run 2, the run that decided S068. One match, one reference, with per-move score, depth and time comments |
| `S105_calibration.sh` | the script that calibrated the S105 harness regime, verbatim as run. Two A/A runs of 1000 games each -- the old regime and the new one, same machine, same hour, fixed rounds so both numbers share a denominator. Its header carries why there is no `-sprt` in it: both sides are one binary, so there is nothing to detect and an SPRT would have ended each run at a different count |
| `S105_calibration.log` | that run's console, both summaries, ending `S105-CALIBRATION-DONE` |
| `S105_calibration_before.pgn` | the 1000 games of the old regime: `10+0.2`, `books/8moves_v3.pgn`, `Hash=16`, concurrency 12. 40.3 % draws, 0 time forfeits |
| `S105_calibration_after.pgn` | the 1000 games of the new one: `8+0.08`, `UHO_Lichess_4852_v1.epd`, `Hash=16`, concurrency 12. 29.5 % draws, 0 time forfeits. Kept rather than regenerated because the book is gitignored and the opening order is random, so this run is not reproducible even from its own script |
| `S105_pairs.py` | the pair-level analysis of both, and **it still runs**: pentanomial, pairs decided by the opening, and the pair score variance that decides whether the unbalanced book bought anything. It did not -- ratio 1.022. The draw rate is a proxy for that variance and this is the thing itself |
| `S105_calibration_pairs.txt` | its output over the two PGNs, which is the evidence cited by DEC-083's corrected Consequences and by `specs.md` |
| `S198_calibration.log` | the workstation's DEC-143 calibration, 2026-09-08: the full console of one fixed-rounds A/A of 1000 games at `bbbf8f6`, both sides the same binary by sha256. Carries the banner the seed is now printed in, `20260908021324`, and fastchess's own `Ptnml(0-2): [34, 92, 229, 107, 38]` to check the pair counts against |
| `S198_calibration.pgn` | those 1000 games, and the first PGN here carrying `n=` and `tl=` on every move -- 5.1 MB against S105's 2.8 MB for the same game count, which is what the two fields cost. Each `[Event]` header carries the seed. Not reproducible even from its own seed: search under a clock is not deterministic |
| `S198_calibration_pairs.txt` | `S198_pairs.py`'s output over it, the reading DEC-143 asks for: pair variance **0.2430 +/- 0.0154** against S105's after-run 0.2395 +/- 0.0152, ratio 1.014, **z +0.16, inside the band**, 0 forfeits on either side |
| `S198_pairs.py` | the band check itself, and **it runs**: it imports `S105_pairs` rather than editing it (this directory is append-only), reads each PGN under its own run's engine name -- `candidate` here, `chesso-a` there, and a name matching neither side inflates the variance with nothing printed to say so -- and prints the `z` that decides materiality at 1.96 |
| `S021_aspiration_sweep.py` | the aspiration window sweep, and **the first script here that still runs** -- it sets parameters over UCI against the tune build instead of rebuilding per point, so it needs no source edit and no `-D`. See "the scripts no longer run" above for what it replaces |
| `S021_aspiration_sweep.tsv` | 21 schedules x 3 independent 100-position samples, node counts at depth 11. The evidence for the schedule S021 shipped, and for why one sample would have chosen a different one |
| `S021_sprt.sh` | the script that ran S021's SPRT, verbatim as run. `elo0=-5 elo1=5` under DEC-063, with the pre-registered interpretation of each outcome in its header |
| `S021_sprt.log` | that run's full console, 2001 lines, ending `SPRT ([-5.00, 5.00]) completed - H1 was accepted` and `Total Time: 00:36:41` |
| `S076_dedupe_fit.sh` | S076's refit on the corpus `tools/corpus_dedupe` reduced to one row per distinct position, verbatim as run. **Its header carries the count that decided a match would be spent at all — 207998 of 11003693 rows dropped, 1.8903 %, against a 1 % threshold pre-registered in the step file before the tool was ever pointed at the corpus — and the rules of the run, all written before the fit** |
| `S076_dedupe_fit.log` | that fit's console: 10795695 rows, 119978 blocks, `fitted K = 0.7801`, held-out error against the game result 0.119608 at the shipping constants to 0.119458, best at epoch 1600, stopped by patience at 3600, 1378 s |
| `S076_fits/` | the constants that fit emitted, byte for byte as `tools/tuner` wrote them |
| `S076_sprt.sh` | the script that ran S076's SPRT, verbatim as run. `elo0=-5 elo1=5` under DEC-063, with the pre-registered interpretation of each outcome and the attribution limit in its header |
| `S076_sprt.log` | that run's full console, 2537 lines, ending `SPRT ([-5.00, 5.00]) completed - H1 was accepted` and `Total Time: 00:46:48` |
| `S076_sprt.pgn` | the 1047 games of that run, 3.0 MB, the run that decided S076. One match, one reference, committed on the same two tests as `S021_sprt.pgn` below |
| `S021_sprt.pgn` | the 824 games of that run, 2.4 MB. One match, one reference, with per-move score, depth and time comments. Committed where S068's run 1 PGN was not, on the same two tests: it is a clean artifact of one run — the script writes its own `-pgnout` path rather than `fastchess.sh`'s shared, appended `/tmp/fastchess_full.pgn` — and it is the run that decided the step |
| `S145_mate_set.py` | the construction and the two-oracle verifier for the mate-safety set, and **it runs**: `generate` rebuilds the file from a seeded sample, `verify` re-proves every row from scratch, `emit-cpp` prints the table `tests/test_engine.cpp` holds. The proof is an exhaustive AND/OR search over `python-chess`, iterative-deepening in the mate distance so the distance it returns is exact and not an upper bound; `stockfish` at a node limit is the second oracle and only ever proposes. Nothing is copied and nothing is derived from another engine's search (DEC-016) |
| `S145_mate_set.tsv` | what it produced: one row per position with the proved distance, the mated side's material lead, the proof's node count, the quiet key, and the defender nodes at plies 1, 3, 5 and 7 that are the nodes actually under test. Regenerable, unlike every other file here, which is the point of keeping the script beside it — **but regenerate a motif at a time with `--only`**: the proposer is version-bound and a plain `generate` on a machine whose stockfish differs replaces rows rather than adding them, measured at 6 of 8 reproduced (S168) |
| `S145_mined_set.py` | the breadth set: one position per game -- the final one -- from `.spsa/S085/games.pgn`, labelled by `stockfish`, and a `score` mode that counts how many of them an engine finds at a stated depth and stated options. Scored as a count with a floor and never per position, for the reason its docstring gives |
| `S145_mined_set.tsv` | that set, `fen distance game` |
| `S145_rfp_sweep.py` | the floor and the ceiling measured against both sets on the tune build, one axis at a time with the other held at its shipping value read from the binary. Reports `found`, `exact` and `delay`, the last being iterations between `2m - 1` and the first iteration that reports the mate -- the reading a fixed-depth call cannot produce |
| `S154_floor_margin_sweep.py` | what the constructed set's counts tolerate, and **it runs**, on the standard library alone. Five modes: `hash` resizes the table and nothing else, `refs` rebuilds the engine at every commit that touched `src/` since the floor was placed, `floor` re-takes the `RfpMinPly` table with the declared minimum relaxed in a throwaway worktree, `slack` prices the gate's depth window, `red` rebuilds the gate with the guard weakened and requires it to fail. Every row carries a per-position mask and the Hamming distance from the row above it, because a net count that holds still while four positions swap is a coincidence and not a tolerance. It sends `isready` after its options and stops on `info string refused`, which is the silent-null failure S156 recorded turned into an error |
| `S168_generate.log` | what each generation run measured while the second and third motifs were built: the four-candidate probe that chose the wall and the two forces, the first run whose knight rows came out 12 of 14 mated by a *pawn*, the rule that refused every knight family outright, and the rule that ships |
| `S168_verify.log` | both oracles over all 82 rows from scratch, 0 checks failed, stockfish corroborating 81 of 82 at 4000000 nodes |
| `S168_floor_sweep.log` | `S154_floor_margin_sweep.py floor` and `red` re-taken over the 82: the ends move to 12 shipping and 10 with the guard removed, so `MATE_IN_THREE_FLOOR` is 11, and `REQUIRE( 10 >= 11 )` is the red observed. Carries the per-motif split that says the queen rows reproduce S154's reading exactly |
| `S154_floor_margin_sweep.log` | every mode's output at `fc5526e`, and the evidence for DEC-116. The count moves 0 positions over seventeen commits and nine table sizes, and 5 under one ply of the guard; the floor of 7 had stopped separating since `aa8c077` and is 8; `REQUIRE( 7 >= 8 )` is the red observed. Also carries the `RfpMaxDepth` sweep re-taken, because three numbers the test comment quoted from it had moved |
| `S148_rfp_ceiling_sweep.py` | the ceiling on the finer grid, every value 0 to 15 over the 82 constructed rows and the mined set, and **it runs**: it imports `S145_rfp_sweep` and calls its `sweep()` rather than editing or copying it, which is what this directory being append-only means in practice. The floor is read from the binary and stated, not assumed |
| `S148_rfp_ceiling_sweep.log` | that run at `192a5a3`, 2026-09-08, and the table S148's challenger rule reads. The mate in five class is the binding one -- 11, 9, 6, 4, 1 of 16 at ceilings 0 to 4 and 0 from 5 up -- so C1 = 4; the whole set moves 39 to 52 of 82 between the shipping 15 and 4, the curve is flat from 10 up, and `short` and `sign` are 0 at all sixteen settings |
| `S148_sprt.sh` | S148's run: the one integer from and to, the whole 0-to-15 sweep table, both builds' `search_bench.py` and `bench` counts, the pair with its worst-case games and hours, the abort rule and the three interpretations -- **all written before a single game was played**, which is what makes the reading of H0 a pre-registration and not a choice made after seeing it. It also states the known `Incomplete mating PV` class and the budget evidence it is to be read against, so the run's count could not be attributed to the ceiling by default |
| `S148_sprt.log` | that run's full console, 35666 lines, ending `SPRT ([-5.00, 0.00]) completed - H0 was accepted`, `Total Time: 06:19:35` and `forfeits 0 of 14809, 0.00 %`. **`Elo: -5.66 +/- 4.33, nElo: -7.31 +/- 5.60` over 14808 games at LLR -2.95**: the ceiling of 4 that recovers the deep mates costs more than five nElo, so 15 stays (DEC-158). The 73 MB PGN is not committed; every game's result and every periodic SPRT block is in here. Grep `Incomplete mating PV` for the class reading -- 7 candidate against 13 reference |
| `S165_nmp_defender_sweep.py` | the same question from the defender's side, which is where a guard on a mate *bound* is reachable at all: `beta <= -MATE_MIN` happens where the engine is the side **being** mated, and every sweep above asserts from the attacker's. `generate` re-proves each distance rather than computing it from the root's -- `S145_mate_set.py`'s `representative_line()` walks the defender's *first legal reply*, so `root_distance - (ply + 1) / 2` is wrong on 3 of the 104 nodes -- and a mate in k against the side to move is **2k** plies, not the attacker-side `2k - 1` |
| `S170_cases.tsv` | the four games whose searches produced the ten `Incomplete mating PV` lines S147's clean 3000-game run left, root FEN and moves taken verbatim from the `Position;` and `Moves;` lines fastchess printed beside each warning. Each row also carries the cheapest `go` budget and first ply that still reproduce it, swept 2026-09-02. Kept because the run's log is gitignored and this is the reproduction |
| `S170_replay.py` | replays a game move by move through **one** engine process and reports every `info` line whose mate line is shorter than the distance it claims, and **it runs**. One process and one `ucinewgame` is the only shape that reproduces any of S170's three causes: on a cold table -- which is what `tests/test_mate_pv.cpp` gives every case -- none of them exists. Node budgets rather than movetime, so a case is a property of the tree and not of how busy the machine is |
| `2026-09-04_plan_review_literature_check.md` | the source pass behind the 2026-09-04 plan review: 33 of `plan.md`'s cited figures fetched at their sources on 2026-09-04 with URL and verdict (CONFIRMED, DIFFERS, NOT FOUND), an inventory of the documented techniques of Stockfish, Ethereal, Berserk, Weiss, Stash and Leorik against the Open list, and the wiki's definition of each technique the steps describe. Technique names and numbers only, no code and no table from any engine (DEC-016). It is the table S185 records from and S186 starts from, and it locates the Ethereal ledger (commit e755a814) and the Stash ledger (`CHANGELOG.md`) the block order rests on. DEC-137 |
| `S183_elo_inputs.md` | the Elo arithmetic re-derived from recorded inputs, behind `2026-09-04_plan_review-F04`: every pending step's published figure with its source URL or the word unverified, the per-block sums, and the three discounts -- with the selection rule and the discount rule both written down **before** any sum was computed. It records all eight of this project's published-to-measured transfers (five with a figure on both ends: two zero, one wrong sign, 0.10, 0.33; mean 0.061, median 0.00) and takes the largest, 0.38, as the headline. **The plan's own range lands at 2707 to 2817 and the reconstruction at 2658, so the high end does not clear 3000** -- which is the question DEC-071 exists for and not a decision this file takes. Re-derived when S024 and S109 land. DEC-136 |
| `S181_lynx_bands.md` | the band correction behind `2026-09-04_plan_review-F02`: every Lynx pull request the pending steps and DEC-087 cite, mapped to its `merged_at` from the GitHub API, the two releases that date falls between, and the CCRL Blitz 1CPU rating of each. Ratings from the list **computed 2026-09-05**, read here **2026-09-11** with `curl` and a browser user-agent (`WebFetch` gets HTTP 403 on that host); it records the 0-to-4-point drift against the 2026-08-28 reading, which is why the read date sits beside every figure. Three band words in the tree were low by 180 to 380 points and one claim was wrong in kind -- Lynx *merged* capture history rather than failing four SPRTs on it. DEC-176 |
| `S181_lynx_prs.tsv` | the twenty-one API records the table is built from, one row per pull request: number, `merged_at` or `not-merged`, title, verbatim as fetched 2026-09-11. An API record and a rating table were the only things read; no Lynx source file, evaluation table or test data (DEC-016) |
| `2026-09-05_enrichment_brief.md` | the brief every agent of the DEC-145 enrichment pass follows: the rules that bind what it writes (DEC-016, DEC-105, DEC-135, DEC-023, DEC-141 to DEC-143), what to read first, the ten-section shape of `## Implementation guide (2026-09-05)`, the checks before finishing and the report shape. A later session resumes the pass by handing this file and one step path to one agent per file, sequentially. DEC-145 |
| `2026-09-05_enrichment_pass.md` | the report of the 2026-09-05 session: the reorder (DEC-144) in a paragraph, the twenty files enriched with lines, sources and unverified counts per file, where the pass stopped (next file S181), twenty findings that look like bugs or stale clauses -- none fixed -- and the owner questions gathered from every file's section 10. DEC-145 |
| `S165_defender_set.tsv` | that set, `fen mated_in ply root_distance family root_fen`. 104 rows, each distance the smallest k with `_and_mate(node, 2k)` true and every shorter one refuted, corroborated by `stockfish` at 4000000 nodes on 104 of 104 |
| `S165_sprt.sh` | S165's run: the reachability counts, the sweep readings and the three interpretations, all written before the first game, then `exec ./fastchess.sh --nonreg` |
| `S165_sprt.log` | what it printed. H1 accepted in 18598 games and 7 h 58 m, `Elo 0.95 +/- 3.56`, and it went nearly the full distance because a true effect at zero against `[-5, 0]` crawls to H1 rather than stopping early |
| `2026-09-04_test_review/` | the evidence behind `adocs/audit/2026-09-04_test_review.md`. `mutants.py` is the 33 hand-written engine bugs, each one exact-anchored edit of `src/` (a guard dropped, a sign flipped, an off-by-one), `run.py` the driver that applies one at a time to a scratch worktree of `5cffb70`, rebuilds, runs the fast label and `tools/search_bench.py` at depth 9, and reverts -- **it runs**, from any checkout, given a worktree beside it; `results.tsv` one row per mutant (compiled, tests failed, node counts and best moves, wall times); `kills.txt` the failing assertion per mutant, extracted from the ctest logs; `coverage_summary.txt` and `coverage_unexecuted.txt` the `llvm-cov` report of the fast label over `src/`, Release flags plus instrumentation, with the unexecuted lines and never-taken branches by file. Kill rate 31 of 32 non-equivalent mutants; the survivor is the fifty-move boundary, the equivalent one the repetition scan's start. Logs are not kept, they are 468 KB of ctest output and regenerable by the driver |
| `S078_body_check.py` | S078's acceptance check over the two step bodies it rewrote, run from the repository root as `python3 adocs/data/S078_body_check.py adocs/plan_todo/S060_*.md adocs/plan_todo/S061_*.md`. Costs no run at all — it is here because it is the executable form of the claim, and because it stays runnable against S060 and S061 until both are done. Non-vacuous by construction: it extracts the `TEST_CASE_FIXTURE` titles and the four source lines from `tests/` first and exits **2** if any is not really there, so a rename in `tests/` turns it red on the precondition rather than on the gate. Exit 0 clean, 1 flagged, 2 precondition |
| `S196_full_pass.tsv` | the 40-mutant kill table of S196's full pass, 2026-09-10, worktree at `44440b4`, 3948 s wall: **39 of 39 killed, M26 the one declared equivalent**, baseline `bench 26851183`. One row per mutant with the binaries that caught it, from `tools/mutation_check.py tools/mutants .ref-builds/mut`. The baseline the next pass diffs against; the mutant list itself is live under `tools/mutants/`, and it has grown by one since this table: **M34**, added by S192's fast check, **survived the whole fast label at `0abe648`** -- the loop handing the time manager a constant zero fall, with the bench signature blind to it -- and is killed by the subcase written for it, so the next full pass reads 41 |
| `S192_anchors.py` | the second implementation of `evaluate()` for the ten pinned test anchors, written from `src/evaluation.cpp`'s prose rather than by calling the engine (S028). `.tuning/anchors.py` until S192 moved it here, computed its root from `__file__` instead of hard-coding it, and named each anchor by the `TEST_CASE` that asserts it. No argument reads the shipped weights and exits 1 on any mismatch; a fitted header as the argument prints `got (was old)` and is how a refit's new anchors are taken. The corpus that produced the weights is not needed and is not here: the script reads `src/eval_tables.hpp` and `src/evaluation.cpp` and multiplies hand-derived feature counts |
| `S192_anchors.log` | its output at the shipped weights, 2026-09-10: **10 of 10 reproduced**, exit 0. The evidence that the derivation is still right at HEAD, which is the precondition before the script is trusted against a fitted header |
| `S192_node_budget.py` | re-derives the node band of `tests/test_search.cpp` "ordering keeps the tree small" by running the case through `build/tests/test_search --success`, reading the count off the `MESSAGE` the case prints and multiplying by the ratios the band has always carried -- 4x above, a fifth below. Reads the two shipping numbers out of the source so the run can be compared with them, and asserts nothing: what to do about a count that has left the middle half of the band is the step's decision |
| `S192_node_budget.log` | its output, 2026-09-10: **179851 nodes**, against the 109575 the band was placed on in 2026-08. The tree has grown 64 % under a band that did not move, and the count is still inside the middle half, which is the condition on leaving 440000 and 20000 alone |
| `S197_script_mutants.py` | the sixteen cuts to `tools/gate_extra.sh` that `tests/test_gate_extra_script.sh` was observed red under, S197, 2026-09-10: **12 of 12 at the completing commit, 16 of 16 after its fast check**. Shell-script mutants, so deliberately not in the `tools/mutants/` registry `tools/mutation_check.py` drives -- nothing here builds the engine. M13 to M16 are the four defects the fast check found, and three of the ten cases exist only because an earlier pass left `M5_no_bench_compare`, `M12_no_root_check` or `M13_trust_any_cache` alive -- which is why the cuts are kept rather than described. Every anchor must appear exactly once or the script refuses, and four anchors moved when the defects were fixed |

`S018_raw.tsv` columns: `game ply phase cost ref own mate san fen`. `cost` is
the reference's swing across the move and may be negative, which the profiler
floors at zero before reporting; the spread of those negatives is the noise
floor of the reference limit.

## S145 — the mate-safety sets, and the sweep that separated the two bounds

The reverse futility sweeps above were the evidence for S033's guard. S145 is the
evidence for whether that guard is worth anything, measured on positions built
for this engine instead of three picked for another one.

### Why the positions are constructed

Measured before anything was built: of 191 positions in a 6347-position sample
where chesso says the side to move is mated within six, **one** has a
non-negative score for the mated side and the median is **-1093**. The hazard
reverse futility walks into needs the mated side to be *ahead*, so it is absent
from the sample frame and no sample size fixes that. `S145_mate_set.py` builds it
instead: a frozen defending army worth 760, 1020 or 1160 centipawns more than the
attacking force, the attacker's moves quiet everywhere except the mate, and mate
distances two to five so the guarded defender nodes land at plies 1, 3, 5 and 7.

**82 positions since S168 (2026-09-01), three motifs**, all proved by exhaustive
AND/OR enumeration — iterative-deepening in the distance, so every shorter
distance is refuted rather than merely unfound. The mating force is a lone queen
in 48, a lone rook in 32 and two knights in 2; `S155_motif_census.py` is the
count. The knight row is thin because it is enforced: a motif may declare
`mates_with`, and a candidate is refused unless every move that mates at the end
of its line is that piece and no promotion is available anywhere along it —
without which twelve of the first fourteen knight positions were pawn mates.

### Two things about the oracles, both learned the hard way

**Stockfish corroborates; it does not decide.** At 4000000 nodes in its own
process it agreed with 46 of the 48 on the Linux machine, and with 81 of the 82
on the MacBook against `dev-20260803-762dd1da` (S168). Of the two it does not: one reads +1879 with
no mate at all, one reads mate 6 against a proved 5. Both were re-proved and
every shorter distance re-refuted. The cause is in the construction — a frozen
army is a position class its network scores badly wrong, and it called the
attacking side better while that side was 760 behind — so the disagreement is
about stockfish's ordering, not about the claim. Only a *shorter* mate would
falsify a proof, and that is the only direction `verify` treats as a failure.

**A node-limited engine is reproducible only inside one identical call
sequence.** The generator runs thousands of positions through one process, which
is fine for a proposer and useless for a check: the first `verify` reported seven
false failures purely because the hash was cold. `verify` now runs one process
per position at ten times the budget.

### The measurement, and it is per mate distance

`S145_rfp_sweep.log`, the full table. One axis swept with the other held at its
shipping value, because S145 measured the two substituting for each other. Every
setting: **0 mate scores with the wrong sign and 0 closer than the proved
minimum**, over twelve settings times 48 positions and, since S168, six
settings times 82.

| | exact | m2 | m3 | m4 | m5 |
|---|---|---|---|---|---|
| `RfpMinPly` 0 and 1 | 19/48 | 13/16, delay to 7 | 6/16 | 0/8 | 0/8 |
| `RfpMinPly` 2 and 3 | 24/48 | **16/16, delay 0** | 8/16 | 0/8 | 0/8 |
| `RfpMinPly` 4 and 5 | 27/48 | 16/16, delay 0 | 11/16 | 0/8 | 0/8 |
| `RfpMaxDepth` 0 | 34/48 | 16/16 | 11/16 | **4/8** | **3/8** |
| `RfpMaxDepth` 6, S033's | 27/48 | 16/16 | 10/16 | 1/8 | 0/8 |
| `RfpMaxDepth` 15, shipping | 24/48 | 16/16 | 8/16 | 0/8 | 0/8 |

Three readings come out of it. **The floor of 2 is exactly where every mate in
two comes back immediately**, and 2 and 3 are indistinguishable across the whole
set — which is DEC-095's decision resting on 48 proved positions instead of 3
picked ones, and S142 is where it lands. **The deep classes belong to the
ceiling, not the floor**, and S085 moved that ceiling from 6 to 15, which is
S148. **And the old gate could not have seen either**, because all three of its
cases were mates in two and the mate in two class is 16 of 16 at every setting of
both bounds.

### The mined set, scored differently on purpose

`S145_mined_set.tsv`, 318 positions, the final position of one game each from
6000 games of `.spsa/S085/games.pgn`, labelled by stockfish. Scored as a count
with a floor and never per position: two of the seventeen engines S145 surveyed
wrote per-position mate tests, watched their own pruning break them, and disabled
the tests rather than the pruning.

At depth 10 the shipping build finds 147 with the right sign, **146 exact**, 0
wrong sign; at `RfpMinPly` 1 and 0 it reads 140 and 139. So this set does see the
1-versus-2 boundary, which the earlier node-count work suggested it would not,
and it does **not** see 2 versus 3. Floor 143, between the two.

## The reverse futility sweeps, S033

Three runs on 2026-08-16. Each row is one build: the fast search suite run
against it, and `tools/search_bench.py <binary> 9` summed over the three
positions. `suite` is `green` or `RED`; `nodes_total` is that sum;
`failed_cases` is the red case names, truncated by the script.

### They were measured on a tree that is in no commit

The sweeps ran at 13:08 to 13:28. `6bd650e`, the commit that introduced reverse
futility pruning at all, landed at **13:40:34** the same day and is the only
commit that has ever touched `RFP_MARGIN` in `src/`. So all three tables were
produced against S033's uncommitted working tree, and that tree differed from
what shipped in two ways that matter to anyone reading the columns:

- it had `#ifndef` guards around the constants, which is why `-D` worked at
  13:08 and does not now
- it had a constant `RFP_MIN_DEPTH`, which is the `min` column below

Neither survives in the history. `git log -S 'RFP_MIN_DEPTH' --all` returns one
commit and it is `e7aa98a`, an audit report, prose only; `git grep RFP_MIN_DEPTH`
over `git rev-list --all -- src/` returns nothing, and `git log -S '#ifndef RFP'
--all` returns no commit at all. **The `-D` method did not regress. It was never
in a committed tree**, and `RFP_MIN_DEPTH` never was either.

### `min` and `min_ply` are different constants

This is the one thing a reader has to get right, and getting it wrong makes the
two tables look like they contradict each other.

| column | constant | what it bounds |
|---|---|---|
| `min` in `S033_rfp_sweep.tsv` and `S033_rfp_guard_sweep.tsv` | `RFP_MIN_DEPTH` | **remaining depth**. The rule fires only at `depth >= min`, so raising it switches pruning off across whole shallow subtrees |
| `min_ply` in `S033_rfp_ply_sweep.tsv` | `RFP_MIN_PLY` | **distance from the root**. The rule fires only at `ply >= min_ply`, so raising it switches pruning off at the top of the tree |
| `max` in all three | `RFP_MAX_DEPTH` | remaining depth from above, `depth <= max` |

Of the three, only `RFP_MAX_DEPTH` and `RFP_MARGIN` exist at HEAD, with
`RFP_MIN_PLY`; `RFP_MIN_DEPTH` is in no source file and no commit. **2026-08-17:
the three that exist are no longer in `src/search.cpp`** -- S073 moved every
search constant into the X macro at `src/search_params.hpp:41-91`, where
`RFP_MARGIN` is line `:60`, `RFP_MAX_DEPTH` line `:61` and `RFP_MIN_PLY` line
`:70`. `src/search.cpp` now only reads them, at `:348-350`. The citations here
said `src/search.cpp:38`, `:39` and `:47` and were correct until `7f15ac4`.

### Margin 75 is green in one table and RED in the other

| file | setting | suite | nodes |
|---|---|---|---|
| `S033_rfp_ply_sweep.tsv` | margin 75, `min_ply` 3, `max` 6 | green | 1216123 |
| `S033_rfp_sweep.tsv` | margin 75, `min` 3, `max` 6 | RED | 2886952 |

Both rows say 75 and both say 3. The 3 names a different constant in each, and
the two constants are not interchangeable, so these are two different trees and
two different verdicts. Naming the columns is necessary and not sufficient --
the shapes are what show they are different knobs.

At margin 75, `max` 6, as the floor goes 1, 2, 3:

| floor | `min`, remaining depth | `min_ply`, distance from root |
|---|---|---|
| 1 | 1190649 | 1190649 |
| 2 | 2142924 | 1203441 |
| 3 | 2886952 | 1216123 |

`min` more than doubles the tree, because the prunes it gives up are at
remaining depth 1 and 2, where nearly all the nodes are. `min_ply` costs 2.1 %,
because the prunes it gives up are at ply 1 and 2, which is a handful of nodes.
And `min` buys nothing for its 1.7M nodes: the suite is RED at every value of
`min`, since the prune that hides the mate happens at **ply 1** and a bound on
remaining depth cannot reach a ply. The ply floor is what turns the cases green.
That is the measurement that made S033 ship `RFP_MIN_PLY` and drop
`RFP_MIN_DEPTH`.

The two tables agree exactly where the knobs coincide, which is what says they
are the same code measured twice. At floor 1 both mean "exempt nothing" --
`depth >= 1` always holds where the rule is tested, since `src/search.cpp:311`
hands anything below it to `quiescence()` before the check at `:348` is reached
(`:304` and `:338` at S033, moved by `7f15ac4`),
and the rule already exempted the root -- so both must be the same binary at
floor 1, and both report **1190649** at margin
75 and **1398911** at margin 100. `S033_rfp_guard_sweep.tsv` row
`guard=0 margin=100 min=1 max=6` reports 1398911 as well.

### What is reproducible at HEAD, and what is not

`S033_rfp_ply_sweep.tsv` maps onto HEAD. **Which row is HEAD moved on
2026-08-17:** S068's verdict shipped margin 75, so HEAD is now its row `3 75 6`
at **1216123** nodes -- reproduced at that commit as 213509 + 915091 + 87523,
best moves `c3d5 e2a6 d7c8q`. Until then HEAD was the row `3 100 6` at
**1422053** nodes, reproduced 2026-08-16 at `d7901e3` as
292313 + 1026739 + 103001, same three best moves. Both rows of this table have
now been checked against a committed tree and both are exact. Its other rows are
one hand edit away.

`S033_rfp_sweep.tsv` does not. It never varies the ply bound, so **none** of its
45 rows is at the shipping configuration, and its `min` column sweeps a constant
that exists in no commit. Reproducing any of its rows means reconstructing a
source version the history does not contain -- not a hand edit, and not
something S073 restores either, since S073 makes `-D` work for the constants
HEAD actually has. Its margin 150, 200 and 300 node counts say which direction a
margin moves the tree; they are not comparable to 1422053 and do not price
anything at HEAD.

`S033_rfp_ply_sweep.tsv` is therefore the only one of the three a later step can
argue a margin from.

### The scripts no longer run

Committed verbatim as evidence, not as tooling. Two traps:

- `out=` is a hard-coded path into a scratchpad directory of a session that is
  gone. A re-run writes its table nowhere useful.
- All three vary the constants with `-DRFP_MARGIN=...` on `CMAKE_CXX_FLAGS`,
  which needs the `#ifndef` guards the pre-commit tree had. Without them `-D`
  collides with the definition and the build fails. The scripts swallow a failed
  build into `BUILD_FAIL` and keep going, so a re-run today produces a table of
  failures rather than an error.

  **2026-08-17: S073 has landed and `-D` still does not work**, so the line that
  used to say "S073 is the step that makes `-D` work" was a prediction and it was
  wrong. The constants are no longer `#define`s either -- S073 made each one a
  row of an X macro in `src/search_params.hpp:41-91` expanded into an
  `inline constexpr int` -- and the symbol a command-line macro now collides with
  is the *declaration*:

  ```
  $ g++ -fsyntax-only -std=c++20 -Isrc -DRFP_MARGIN=75 src/search.cpp
  <command-line>: error: expected unqualified-id before numeric constant
  src/search_params.hpp:99:24: note: in definition of macro 'CHESSO_DECLARE_SEARCH_PARAM'
     99 |   inline constexpr int sym = def;
  src/search_params.hpp:60:5: note: in expansion of macro 'RFP_MARGIN'
  ```

  Reproduced 2026-08-17 at S068's completing commit, g++ 13.3.0. What S073
  actually built is the other route: `-DCHESSO_TUNE=ON` makes every parameter an
  `extern int` settable over UCI, so a *sweep* costs one build instead of one per
  point. That build must never produce a strength number -- `DEV_MANUAL.md`,
  "The tune build", **"It is not the release binary and no strength number is
  ever taken on it"**, cited by its sentence rather than by a line because that
  section moves --
  which is why S068's two SPRT binaries were both ordinary Release builds with
  the default edited in the header.

## S068's two SPRTs, and the 38 MB PGN that is not here

Both runs measured the same one constant -- `RFP_MARGIN` 75 against the shipping
100 -- with the same two Release binaries, book, time control, concurrency and
adjudication. The bounds were the only difference and they decided everything:

| run | bounds | games | wall | outcome |
|---|---|---|---|---|
| 1 | `elo0=0 elo1=5` | 9036 scored, 9066 started | 6 h 36 m 15 s | no verdict, terminated |
| 2 | `elo0=-5 elo1=5` | 2312 | 1 h 41 m 13 s | **H1 accepted** |

Both at about 1371 games/h. DEC-063 is the rule; S068 is the arithmetic. **Run 1
is kept in full because a run that returned nothing is the evidence for that
rule** -- delete it and DEC-063 becomes an assertion about a run nobody can
inspect.

The effect estimate is pooled over both, 11348 games, 3681-3517-4150: score
50.72 %, point Elo **+5.02**. Run 2 stopped early and its point estimate
(+12.18) is therefore upward-biased by optional stopping, so it is not quoted
alone and its share of the pooling is flagged in the step file.

### What is committed, and what was left out

Committed: both scripts, both console transcripts, and run 2's PGN. 8.7 MB in
total, against 2.7 MB for everything in this directory before it.

**Not committed: run 1's PGN, 38 MB, 13468 games.** Two reasons, in order of
weight.

1. **It is not an artifact of run 1.** `fastchess.sh:133` passes a fixed
   `-pgnout file=/tmp/fastchess_full.pgn` and fastchess *appends*, so the file is
   an accumulation of every full-bounds match that machine has run: 9055 games of
   S068's run 1 against `ref-7f15ac4`, 3397 against `ref-a2f0065` and 1016
   against `ref-c56ab41`. Committing it as "S068's match PGN" would be filing
   three matches under one step's name, and separating them needs a filter over
   the `White`/`Black` tags.
2. **Nothing recorded depends on it.** Every number S068 or DEC-063 states comes
   from the console transcript, which is committed: `S068_sprt_run1.log` carries
   every game's result and adjudication reason line by line, so the score, the
   pentanomial pairs and the LLR trajectory are all re-derivable from it. What
   the PGN adds is the move lists and the per-move score/depth/time comments --
   input for `tools/error_profile.py` or `tools/analyse_game.py`, and no step
   plans to profile them. These are two near-identical engines; S018's and
   S028's PGNs are kept because they are games against a *stronger* opponent,
   which is what an error profile needs.

That PGN lived at
`/tmp/claude-1000/-home-max-ws-chesso/15ad9dc1-2aed-4b32-aca7-69494270d848/scratchpad/S068_match.pgn`
and is **volatile**: it is in a session scratchpad and will be gone, which is
exactly the loss S072 exists because of. Run 1's move lists are the part of this
step's evidence that was deliberately let go, and this paragraph is the record of
the choice rather than a silence about it.

Run 2's PGN is committed on the opposite reading of the same two tests: it is one
match against one reference, it is the run that decided the step, and 6.8 MB
insures 1 h 41 m of the constraint that binds the whole plan. `.git` is already
321 MB, so neither file is decided by repository size alone -- the 38 MB one is
decided by not being a clean artifact of anything.

## S021's aspiration sweep, and why three samples

`S021_aspiration_sweep.tsv` has a `sample` column and it is the point of the
file. Each sample is 100 positions drawn from `S018_raw.tsv` -- four per value
of the engine's own `game_phase()`, stratified so the middlegame does not answer
for the endgame -- and the three differ only in where in each phase's list the
pick starts (`offset` 0, 37 and 71, the script's third argument). Every row is
one schedule measured over one of those samples at depth 11, through
`build-tune`, so all 21 schedules and all three samples come from one binary and
no rebuild sits between any two numbers.

The `rel` column is that row's nodes over the `min_depth=64` row of the **same**
sample, which is the feature switched off: no iteration below depth 64 gets a
window, so it is this binary searching what the shipping one searches without
aspiration.

**One sample would have picked a different schedule, and would have been wrong
about how much it buys.**

| schedule | sample 0 | sample 37 | sample 71 | pooled |
|---|---|---|---|---|
| min 5, delta 50 | 0.8728 | 0.9863 | 0.9174 | **0.9248** |
| min 4, delta 12 | 0.9075 | 1.0063 | 1.0661 | 0.9916 |
| min 3, delta 12 | 0.9716 | 1.0497 | 1.0568 | 1.0250 |

On sample 0 alone, `delta` 12 at `min_depth` 4 reads 0.9075 and looks like the
second best schedule swept. It is the fourth *worst* pooled, and on sample 71 it
costs 6.6 % more nodes than having no windows at all. The shipping row is best
pooled and is best or second on every sample individually, which is a different
and much weaker claim than the 12.7 % sample 0 reports for it.

`max_delta` is flat: 0.9636 to 0.9708 pooled across 100, 200, 400, 800 and 2000
at `min_depth` 4, `delta` 25. It was left at 400 rather than fitted, and nothing
here says 400 is better than 800.

Nodes at a fixed depth are not Elo. The sweep chose what the SPRT then measured,
and the SPRT is the verdict -- DEC-019, and three techniques that reported Elo
and measured none.

## S021's SPRT, and a point estimate that is not the effect size

One run, `elo0=-5 elo1=5` under DEC-063, and it terminated:

```
Elo: 35.12 +/- 19.06, nElo: 44.04 +/- 23.72
LOS: 99.99 %, DrawRatio: 34.47 %, PairsRatio: 1.48
Games: 824, Wins: 293, Losses: 210, Draws: 321, Points: 453.5 (55.04 %)
Ptnml(0-2): [29, 80, 142, 101, 60], WL/DD Ratio: 1.03
LLR: 2.97 (100.8%) (-2.94, 2.94) [-5.00, 5.00]
SPRT ([-5.00, 5.00]) completed - H1 was accepted
```

36 m 41 s, 824 games, about 1348 games/h -- the same rate S068 measured at 1371,
on the same twelve threads.

**+35.12 is not the effect size.** An SPRT stops as soon as the evidence crosses
a bound, so it stops early precisely when the observed effect has run
favourable, and the stopping run's point estimate is biased upward by exactly
that. S068 could pool two runs and quote +5.02 from 11348 games; this step has
one run and nothing to pool it with, so the number stands with its bias named
rather than being averaged away. What the run establishes is its pre-registered
claim -- **not a regression of 5 Elo or more** -- and, at `LOS: 99.99 %` over
824 games, that the sign is positive.

The bounds are the reason it cost 36 minutes rather than a night. The expected
effect written into the step file before the run was +9 +/- 17, which is inside
`elo0=0 elo1=5`'s undefended interval and would have random-walked the way
S068's first run did for 6 h 36 m. DEC-063 was written from that run and this is
the first step to spend it.

### What was checked before the verdict was read

A +35 from a 5-line diff is the shape of a contaminated match (DEC-020), so the
two binaries were compared before the number was recorded rather than after:

- both `CMakeCache.txt`s carry `CMAKE_BUILD_TYPE=Release`,
  `CMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG`, `CMAKE_CXX_COMPILER=/usr/bin/c++` and
  `CHESSO_TUNE:BOOL=OFF`. Neither side is the tune build
- `.ref-builds/2b54a4f` is clean at `2b54a4f` and `grep -c ASPIRATION` over its
  `src/search_params.hpp` returns 0
- the candidate snapshot the match played is md5-identical to `build/src/chesso`
  at the completing commit, so no rebuild swapped the engine mid-match

## S075 — the tuner's score/result blend, and why no match was played

`S075_lambda_sweep.sh` and `S075_lambda_sweep.log`; `S075_lambda_fine_grid.sh`
and `S075_lambda_fine_grid.log`; the emitted constants of every run under
`S075_fits/` (5000 epochs, `--report 100`) and `S075_fits_fine/` (epochs 1..200,
`--report 5`), one `.hpp` per lambda in each.

Both scans fit all 11003693 rows of `.tuning/selfplay_v2.tsv` at
`--freeze tempo,piece_placement --seed 1 --validation 0.1 --threads 12`,
differing only in `--lambda` and in the reporting grid. The target is
`lambda * sigma(K * score) + (1 - lambda) * result`.

**Read the `wdl` column and never the `validation` one when comparing two
lambdas.** They train against different targets, so their training errors are not
comparable numbers; DEC-064 is the rule and the emitted headers carry both.
Against the incumbent's 0.118457, the best held-out error against the game result
was 0.118452 at lambda 0 — inside the control's own 4e-05 wobble — and 0.118530,
0.118838, 0.119014, 0.119122, 0.119213 at 0.25, 0.5, 0.7, 0.85 and 1.0. Monotone
in lambda, degrading from the first checkpoint, at both resolutions.

The fine grid exists because `--report` sets the **checkpoint grid** and not only
the log cadence: the coarse scan could only keep a vector it had sampled, and at
lambda 0.25 the blend target absorbs nearly all its movement before epoch 100. A
zero measured on that grid alone would have been a zero about the sampling.

`S075_sprt.sh` is here and **was never run**. Its pre-registered interpretation
was committed before the first fit, as was the sweep script's rule 3: no lambda
beat the incumbent, so no candidate existed to play, and a verdict of zero is
recorded without a match. Whoever re-asks this on a corpus generated by today's
engine — S082 or S083 — has the invocation ready.

The emitted `.hpp` files are evidence and are byte for byte what `tools/tuner`
wrote. `clang-format.sh` excludes `adocs/` by path for exactly that reason, held
by case 6 of `tests/test_clang_format_script.sh`.

## S076 — the corpus deduplicated by zobrist key

`S076_dedupe_fit.sh` and `S076_dedupe_fit.log`, the emitted constants under
`S076_fits/`, and `S076_sprt.sh`, `S076_sprt.log`, `S076_sprt.pgn`.

**The count came before the threshold was allowed to matter.** The step file
pre-registered 1 % of rows as the line between "refit and play one SPRT" and "no
match is spent", and it was committed before `build/tools/corpus_dedupe` was
pointed at the corpus. The pass then dropped **207998 of 11003693 rows,
1.8903 %**, leaving 10795695 distinct positions. Two runs are byte identical and
a `--verify` pass found **0** of those 207998 key-equal rows disagreeing on the
four FEN fields the zobrist key covers, so the count is repeats rather than
collisions.

The fit that followed is one fit and one candidate, at S065's and S075's
settings, so the corpus is the only thing that differs from the run that
produced the shipping weights. Two numbers in `S076_dedupe_fit.log` carry the
step:

- `fitted K = 0.7801`, against the **0.7595** S075's lambda 0 control fitted
  from the same starting constants on the full corpus. K is fitted at the
  starting parameters against the game result, so with those held the corpus is
  the only input that changed: removing 1.9 % of the rows moved the
  score-to-outcome scale by 2.7 %.
- held-out error against the game result **0.119608 → 0.119458** over the
  deduplicated corpus's own held-out rows. Not comparable with the 0.118457 the
  same constants score on the full corpus — a different row set is a different
  number, which `S076_dedupe_fit.sh` says before the run rather than after it.

**The attribution is in the weights, not in a second match.** Same budget, same
seed, same freeze, same starting constants: S075's control on the full corpus
moved psqt rook mg by +2.3 on the mean and 74 at its largest single square;
this run moved -51.2 and 761. More training on the same corpus does not do that.

## S076's SPRT, and a second point estimate that is not an effect size

```
Elo: 26.68 +/- 16.40, nElo: 34.44 +/- 21.08
LOS: 99.93 %, DrawRatio: 35.63 %, PairsRatio: 1.40
Games: 1044, Wins: 365, Losses: 285, Draws: 394, Points: 562.0 (53.83 %)
Ptnml(0-2): [38, 102, 186, 134, 62], WL/DD Ratio: 1.35
LLR: 2.95 (100.1%) (-2.94, 2.94) [-5.00, 5.00]
SPRT ([-5.00, 5.00]) completed - H1 was accepted
```

46 m 48 s, 1044 games, about 1338 games/h — the same rate S021 measured at 1348
and S068 at 1371 on these twelve threads. The bounds are why it cost 47 minutes:
`elo0=0 elo1=5` puts an effect of this size inside its undefended interval,
which is DEC-063 and S068's 6 h 36 m for nothing.

**+26.68 is not the effect size**, for the reason S021's section above states:
the run stopped early precisely because the observed effect had run favourable.
The recorded verdict is the pre-registered one — not a regression of 5 Elo or
more, sign positive at LOS 99.93 %.

`S076_sprt.pgn` is committed on the same two tests S021's was: one match against
one reference, written to its own `-pgnout` path rather than `fastchess.sh`'s
shared appended file, and it is the run that decided the step. 3.0 MB, 1047
games, every one of them naming `candidate-s076-dedupe`.

## The rating gauntlets, S087 and S088

Thirteen files here come from `rating.sh` rather than from `fastchess.sh`, and
they answer a different question. Every other PGN in this directory is chesso
against an earlier chesso and reports a *delta*. These are chesso against
engines the public lists rate, solved by `ordo` into an *absolute* figure on the
CCRL Blitz scale. The two are never quoted against each other: `ordo`'s
intervals are trinomial, and every SPRT here runs `model=normalized` and reports
nElo.

**S087 committed its evidence and no index entry, so this section is written a
step late.** `DEV_MANUAL.md:17` says this README says what each file is; for
eleven of these it did not, and the omission matters most on the two files whose
names differ by one character and whose results are opposite.

### S087, the first absolute figure

| file | what it is |
|---|---|
| `S087_bracket1.pgn`, `_h2h.txt` | **the reference set that FAILED.** Leorik 1.0 (2102), Blunder 5.0.0 (2017), Rustic Alpha 3.0.6. 204 games, 7 m 30 s. chesso scored 90.4 % against the strongest, which is not below 90 %, so every score sat in the tail of the logistic curve and no rating was claimable. Quoting a number from this file is a mistake |
| `S087_bracket2.pgn`, `_h2h.txt` | the DEC-069 set that passed: Blunder 7.1.0, Leorik 2.1, Blunder 8.5.5, Leorik 2.4. 272 games, 12 m 33 s, 22.8 % against the strongest and 75.0 % against the weakest |
| `S087_rated1.pgn`, `_h2h.txt`, `_report.txt` | first rated run, 1336 games, 1 h 00 m. **Not enough on its own** -- best interval +/-34.0 against the +/-30 required |
| `S087_rated2.pgn`, `_report.txt` | the repeat, 1336 more games, same binaries and same time control, run because the criterion was not relaxed |
| `S087_combined_solve.txt`, `S087_combined_h2h.txt` | the two rated PGNs concatenated and solved once per anchor. This is where 2570 comes from |
| `rating_2026-08-18_ccrl_blitz.md` | the write-up: the number, the anchor sweep, the caveats in order of size, and why the result is labelled SOFT |

The `_report.txt` files are 16 MB and 19 MB because `rating.sh` tees the whole
fastchess stream, which includes a `Position;`/`Moves;` dump per adjudicated
game.

### S088, the third family

| file | what it is |
|---|---|
| `S088_bracket.pgn`, `_h2h.txt` | the five-engine set bracketed, 340 games, 15 m 25 s, 0 forfeits. Adds Stash v21.0 (CCRL `Stash 21.0`, 2713) to the four above. chesso 19.1 % against the strongest and 71.3 % against the weakest, with the new rung at 30.9 % -- inside the 10-90 % band its own accepts clause requires |

**S088 added five more files and one of them is a voided run kept on purpose:**

| file | what it is |
|---|---|
| `S088_bracket.pgn`, `_h2h.txt` | the five-engine set bracketed, 340 games, 15 m 25 s, 0 forfeits |
| `S088_rated_c12_INVALID.pgn`, `_h2h.txt`, `_summary.txt` | 3340 games at **concurrency 12**, voided by 3 Stash time forfeits. **No rating is claimable from it** and none is quoted. Kept because it is the only measurement this project has of what the opponent pool does to a foreign-engine gauntlet: chesso scored 44.3 % against Blunder 8.5.5 here against 37.6 % in S087 and 39.1 % in the valid run |
| `S088_rated_c6.pgn` | 3340 games at **concurrency 6**, 5 h 02 m 49 s, 1 tolerated forfeit at 0.15 %. **This is the run behind the 2559 figure** |
| `S088_solve.sh` | the anchor sweep by hand. `rating.sh` exits before the sweep on a voided run, and DEC-076 tolerated this one's forfeit after the fact, so the solve was reissued from here with the identical `ordo` command. Reads anchors from the CCRL list at run time and the name mapping from `references.tsv`, so it cannot drift from what was played |
| `rating_2026-08-18_S088_ccrl_blitz.md` | the write-up. **Supersedes S087's as the current figure**, and does not replace it as a record |

The two `rating_2026-08-18_*` files are one character apart in the middle of a
long name and report different numbers from the same engine — 2570 over four
engines, 2559 over five. `_S088_` is the current one.

### S100, the corpus feature audit

| file | what it is |
|---|---|
| `S100_feature_audit.txt` | `build/tools/feature_audit --data .tuning/selfplay_v2_dedup.tsv --sample 200000` over all 10795695 rows, 2026-08-20. Four reports: every stored feature column re-extracted from the FEN text (**0 disagreements**), the two exact seventh-rank piece-square identities (**0 violations** in 1264773 and 550880 non-zero rows), per-column occurrence whole-corpus and by phase band, and R² of each term column on the tables that could absorb it — **rook seventh and passer bucket 5 at 1.000000, everything else 0.168 to 0.621**. The tempo lines are the term's whole label-side signal: mean result 0.556559 with White to move against 0.548682 with Black, gap 0.007878. Kept because it is the evidence behind S100's verdict ledger and because the corpus it describes is about to be replaced by S082 and S083; re-running it on the new corpus is one command |
| `S219_book_survey.md` | the open-licence opening-book survey behind DEC-182 and S219, 2026-09-11: every candidate with its type, size, licence and the URL the licence was verified at -- all of them CC0 in `official-stockfish/books`, Pohl's own downloads and the OpenBench set excluded for stating no licence -- fishtest's default and its reason, fishtest's own book-comparison method (time odds, fixed games, normalized Elo), the 2021 sensitivity measurements that put the draw ratio ahead of balance, and the `unverified` list. Names three candidates for the owner to download; decides nothing -- S219 measures |
| `S219_book_compare.sh` | the pre-registered book comparison behind DEC-182 and S219, run 2026-09-11 night: one HEAD binary against itself at one doubling of time (8+0.08 against 4+0.04) on each of four CC0 books, 750 rounds per match, two counterbalanced passes, `Hash=16`, adjudication as `fastchess.sh`. **Its header is the pre-registration** -- the selection metric `M = nElo^2 x games per hour`, the tie rule toward the balanced book, the abort rules and the power argument for the doubling -- written before the first game. Reviewed and smoke-tested by an Opus 5 subagent (DEC-185): five guards added in the body, the header untouched |
| `S219_read.py` | the reader for that run: per book, pooled and per pass, the pentanomial of `full`'s pair score, pairs the opening decided, logistic Elo and normalized Elo with 95 % intervals, games an hour, `M` with its propagated error, and the pick by the header's rule. nElo is Van den Bergh's published definition, cited in the file; **validated to the digit against fastchess's own printed Elo, nElo and pentanomial** on the 4-game smoke run and on S198's 1000-game PGN (`7.99 +/- 15.03`, `11.46 +/- 21.53`, `[34,92,229,107,38]`, 2277 games an hour) |
| `S219_book_compare.md` | the reading of that run, 2026-09-12: all eight matches cross-checked against fastchess's own printed Elo/nElo/Ptnml (all agree to the digit), the pooled-per-book and per-pass tables `S219_read.py` printed, and the pre-registered rule applied step by step. **`noob_3moves.epd` picked** -- the largest M outright, more than one combined standard error clear of every other candidate, and balanced, so the tie rule is never reached. Hours per verdict against the incumbent `uho4852`: `M_current / M_pick` = 0.80 +/- 0.07. DEC-189 |
| `S219_aa_calibration.log` | the DEC-143 A/A that follows the book switch, 2026-09-12: the full console of one fixed-rounds run of 1000 games, both sides `5047070`, on `books/noob_3moves.epd`, governor `powersave`. Carries the seed `20260912005458` and fastchess's own `Ptnml(0-2): [54, 91, 198, 118, 39]` to check the pair counts against |
| `S219_aa_calibration.pgn` | those 1000 games. 328 fastchess warnings "PV continues after threefold repetition" (S042, DEC-187), firing from both sides equally in this self-play and not read as a book signal |
| `S219_aa_calibration_pairs.txt` | `S198_pairs.py`'s output over it: pair variance **0.2905 +/- 0.0184** against S105's band 0.2395 +/- 0.0152, ratio 1.213, **z +2.14, OUTSIDE the band** |
| `S219_aa_calibration.md` | the reading: this run beside S198's figures -- variance, games an hour, draw rate, opening-decided share, pentanomial, repetition warnings, governor -- the cost-per-verdict arithmetic (1.29 +/- 0.12x S198's, worse), and the literal rule from S219's `accepts` applied. **Recommends reverting `book=`** to `UHO_Lichess_4852_v1.epd`; the decision itself is the coordinator's, DEC-190 |
| `S159_census.md` | the killer-table census write-up, S159, 2026-09-09, Release, `g++ 13.3`, the i7-8700K workstation: why the driver set is new, the reproduction recipe (`git archive 99000c1`, the patch, the driver) verified end to end that day, what was counted and where each counter sits, the two-build table and its reading. The evidence DEC-160 closes S159 on -- the ageing reading of S149's 11 Elo does not survive its own census, so no SPRT was spent on it -- and it states itself that it is a prior and not a verdict (DEC-019) |
| `S159_census_positions.txt` | the eleven positions every S159 figure is measured on, one per line as `depth<TAB>name<TAB>FEN`. It exists because S149's own driver named its 11 positions and recorded none of their FENs, in the step file or in the audit that produced it, so S149's 66.0 % / 44.4 % cannot be reproduced exactly; every row here comes from one source the repository does record. Kept unedited as the input the recorded census was taken on -- `S216_census_positions.txt` is what a run today uses, because the `promo-mess` row is the pre-DEC-177 `KILLER_POS` and a post-S208 engine refuses it |
| `S159_census_run.py` | the driver that took that census: one engine process, `Hash 64`, each position searched to its own depth, and each `go` waited on to `bestmove` before the next command is written -- piping the whole script with a trailing `quit` kills every search before it looks at a node and the census then reads `stores=0` over the entire set, TOOLCHAIN.md's "the one way to ask it that lies" in another costume. Replaced by `S216_census_run.py`, which sees a refused row instead of inheriting the board above it; kept as the reader the recorded numbers came from |
| `S159_census_instrument.patch` | the instrumentation, over three files: eight counters in `src/search.cpp`, a per-slot iteration stamp and an iteration counter in `src/data_structures.hpp`, and one `S159CENSUS` line to stderr at the end of the search in `src/chesso.cpp`. It is applied to a scratch copy of `99000c1` and **never to the repository tree**, which is what makes the census reproducible without a measurement patch living in `src/` |
| `S159_census_head.txt` | that census at S159's HEAD: per-position nodes and best move for all eleven rows, the total, and the `S159CENSUS` counter line. The left column of `S159_census.md`'s table, and the file the reproduction check is compared against |
| `S159_census_guard.txt` | the same census with S149's reverted guard re-applied on the instrumented copy only, which is the build the ageing hypothesis says the staleness lives in. The right column of that table, and the run that shows the guard removes the duplication completely |
| `S216_census_run.py` | the killer-census driver at S216, 2026-09-11, replacing `S159_census_run.py` and leaving it in place. S208's 16-a-side load bound refuses the `promo-mess` row of `S159_census_positions.txt` -- the pre-DEC-177 `KILLER_POS`, 17 white pieces -- and S159's reader scraped only lines containing ` nodes `, so the `info string refused [position fen]` line was skipped, `set_position`'s `game = previous` left **the row above** on the board, and the `go` that followed printed its numbers under the refused row's name. Observed red first at depth 2 on HEAD: `promo-mess` reported **452 nodes, best `d2d4`** -- startpos's, and a move the promo-mess board cannot play. The new driver reads an `isready` between `position fen` and `go`, so a refusal is seen before a search is started; the row is printed and written as `REFUSED`, never searched, and the script exits 1. On an all-legal input its output is byte-identical to S159's, checked over three rows |
| `S216_census_positions.txt` | the census input from here on, 2026-09-11: S159's eleven rows in S159's order with the sixth replaced, and every other row byte-identical to `S159_census_positions.txt`, which stays beside it unedited. That sixth row held the pre-DEC-177 `KILLER_POS` at 17 white pieces, which S208's load bound refuses; it carries DEC-177's legal constant instead -- 16 white, 14 black, one white pawn fewer and the same board otherwise, so the twelve promotions and the `f5e6` en-passant capture the row is in the set for are kept. **The name changes with the board**, to `promo-mess-s208`, so no future table's row is read against the 142852 nodes recorded for the old one. DEC-186, which also decides the recorded census is not re-derived |
| `S042_polyglot_key_cases.py` | the DEC-142 re-derivation script for `tests/test_audit_polyglot_key.cpp`'s twelve cases, 2026-09-12: it parses the cases straight out of the C++ file (no second copy to drift) and checks each FEN's `has_pseudo_legal_en_passant()` against the case's `capturable` flag and its `chess.polyglot.zobrist_hash()` against `spec_key`. Written when S042's X-FEN convention cleared the en-passant square of eight of the ten original cases at load; every original `spec_key` was unchanged, because python-chess derives capturability from the pawns and never trusted the FEN's fourth field. Exit 0 only when all twelve agree; `12 cases, 0 mismatches` on the committed file |
| `S217_handcrafted_gap.md` | DEC-179's hand-crafted gap inventory, 2026-09-12: five DEC-071 engines (Weiss 1.2, Texel 1.07, Laser 1.7, rofChade 2.3, Ethereal 11.75) re-read on the CCRL Blitz complete list for rating and version, each verified network-free against its own release notes or the author's statement, their search and evaluation techniques read from release notes, changelogs, the Chess Programming Wiki and forum posts only (DEC-016, DEC-105) and checked technique by technique against every step chesso ships or has pending. **The gap is three evaluation terms, all above the 3100 band this step's own rule needs for an automatic step, all without a published Elo figure**: complexity/conversion-chances scaling (Ethereal, rofChade), fortress detection (Texel), and castling ability (Ethereal, one engine, weakest evidence). S183's arithmetic re-derived over the extended list adds zero -- none of the three carries a figure to sum -- so the landing points are unchanged, 2658 reconstructed and 2707 to 2817 on the plan's own range, still 183 to 342 Elo short of 3000 at the high end |
| `S151_ltc.sh` | S151's longer-control reading, written 2026-09-12 and run 2026-09-12 23:37 to 2026-09-13 03:17: S085's shipped vector `21b4a21` against its own parent `3488506` at `32+0.32` with `Hash=64`, a fixed 1000-pair match with no `-sprt`, through `fastchess.sh`'s own `CAND`, `TC`, `HASH` and `ROUNDS` (the first three are S151's additions). **Its header is the pre-registration** -- both shas with their dates and the three `git` checks that pin the pair, the ten `search_params.hpp` defaults that moved, S085's `8+0.08` verdict block verbatim, the regime with an origin per line, the estimate of about 3.8 h from the measured 2110 games an hour and the 27360 s ceiling, the abort rule (forfeits over 1.0 % on a side; `Incomplete mating PV` from both sides expected and never a stop), and the three readings including the +/- 15.2 nElo and +/- 11.6 logistic Elo the pair count buys, derived in the header and checked against S085's own printed `26.81 +/- 12.55`. A fixed-rounds estimate is never a verdict (DEC-143) and nothing in the engine changes on any outcome |
| `S151_ltc.log` | the run's banner, fastchess's final block (`Elo: 11.99 +/- 11.57, nElo: 15.81 +/- 15.23`, 2000 games, 0 forfeits), terminations, the two sides' `Incomplete mating PV` counts (57 and 67) and the timing (3 h 40 m, 545 games an hour); the full log and PGN are gitignored under `.tuning/` |
| `S151_ltc_pairs.txt` | `S105_pairs.py` over the run's 1000 pairs: pair-score mean 1.0345, variance 0.2876, draws 41.0 %, 110.6 plies and 78.2 s a game |
| `S212_aa.log` | S212's closing A/A (DEC-143) after two-sided resignation, the identity check, cache validity and the load guard entered `fastchess.sh`: both sides `f9d705c`, 1000 games in 28 m 08 s (2133 an hour), `Elo 8.69 +/- 16.53`, 0 forfeits, no termination outside normal and adjudication |
| `S212_aa_pairs.txt` | `S198_pairs.py` over its 500 pairs against the current book's band: variance 0.2939 +/- 0.0186 against S219's 0.2905 +/- 0.0184, inside the band |
| `S109_sprt.sh` | S109's pre-registration: the shallow-depth pruning block (candidate `1952c56`, the block plus a tests-only fix) against the pre-block `50e3661` at `{0, 5}` nElo, `8+0.08`, `noob_3moves.epd`, the reference pinned in the script; the bisection protocol on H0; run 2026-09-13 08:11 to 08:49 |
| `S109_sprt.log` | the run's banner, identity lines, fastchess's final block (H1, LLR 2.97, `Elo 46.90 +/- 15.43`, `nElo 56.76 +/- 18.44`, 1364 games), terminations, 0 forfeits, the `Incomplete mating PV` counts (1 and 2), timing |
| `S109_sprt_pairs.txt` | `S105_pairs.py` over the run's 682 pairs: pair-score mean 1.1342, variance 0.3372, draws 32.0 %, 110.1 plies and 19.3 s a game |
| `S091_sprt.sh` | S091's pre-registration: the capture SEE skip and the extra reduction ply (candidate `d785b89`, the landing pinned by `b0df255`) against the pre-step `08461e0` at `{0, 5}` nElo, `8+0.08`, `noob_3moves.epd`, `OUT` under `.tuning/`; the two-leg bisection on H0, the per-rule node sweep and the published record in its header; run 2026-09-14 17:22 to 18:00 |
| `S091_sprt.log` | the run's banner, identity lines, fastchess's final block (H1, LLR 2.96, `Elo 46.52 +/- 15.43`, `nElo 56.38 +/- 18.47`, 1360 games), 0 forfeits, `Incomplete mating PV` 1 and 0 |
| `S091_sprt_pairs.txt` | the reading in the pre-registration's order -- markers, banner, final block, timing, terminations (1015 adjudications, 348 natural over the 1363 games in the PGN), forfeits, the PV counts -- and `S105_pairs.py` over 680 pairs: pair-score mean 1.1331, variance 0.3363, draws 33.1 %, 112.5 plies and 19.5 s a game |
| `S091_rule_sweep.py` | the driver of S091's per-rule node sweep, 2026-09-14 |
| `S091_rule_sweep.txt` | S091's per-rule node sweep, 2026-09-14: the three `search_bench` positions at depth 12 through the tune build, each rule alone against both off (both off, skip only, extra only, both on) -- the table `S091_sprt.sh`'s header quotes |
| `S109_ltc.log` | DEC-202's longer-control reading of the S109 block: `600f448` against `50e3661` at `32+0.32`, `Hash=64`, 1000 pairs, 3 h 44 m 52 s, `Elo 30.65 +/- 12.11`, `nElo 38.75 +/- 15.23`, 0 forfeits, read against the SPRT's +46.90 +/- 15.43 (z 0.83, agreement) |
| `S109_ltc_pairs.txt` | `S105_pairs.py` over its 1000 pairs: variance 0.3113 |
| `S199_drift_point1.log` | the first drift point's banner, final block (`Elo 98.82 +/- 13.18`), terminations, forfeits and timing; the row itself is in `S199_drift.tsv` |
| `S210_depth1_latency.py` | S210 F21: the depth-1 wall-time distribution over 400 corpus FENs through the UCI pipe, re-measured before and after the first iteration honoured `stop` (p50/p90/p99/max 1.36 / 2.20 / 3.16 / 5.57 ms after, 1.33 / 1.91 / 2.99 / 5.72 before) |
| `S210_f22_census.py` | S210 F22: the reach census -- every tenth position of the S219 A/A (11503) at depth 10 on the tune build, counting quiescence moves that land on a dead board and comparing root answers before and after the fix |
| `S210_f22_census.txt` | its reading: 158685 of 254361785 quiescence moves (0.062 %) on a dead board before the fix, 194 of 11503 root best moves changed (1.687 %), 444 root scores |
| `S210_f22_changed.tsv` | the 194 positions whose root answer moved, with both answers |
| `S210_f22_sprt.sh` | the pre-registration of F22's `--nonreg` `{-5, 0}` run against `9075bf8`, the first-half commit; owed because the census reached the class; a night run |
| `S210_f22_sprt.log` | the run's banner, final block (H1 at `{-5, 0}`, LLR 2.98, `Elo 0.01 +/- 3.17`, `nElo 0.02 +/- 4.03`, 28598 games in 13 h 16 m), terminations, finish reasons, 0 forfeits, the change-specific reading (0 of 1661 dead-draw games carry a mate score), timing |
| `S210_f22_sprt_pairs.txt` | `S105_pairs.py` over its 14299 pairs: variance 0.3108, draws 33.5 %, 112.8 plies and 19.5 s a game |
| `S024_census_positions.txt` | the 400 positions verdict 1's H0 census is measured over, 2026-09-12: sampled from `adocs/data/S219_aa_calibration.pgn` (1000 self-play games) because no fixed 400-position corpus file exists anywhere in the repository -- checked; `src/search.cpp`'s "400 corpus positions" and S103's "300 positions" are both ad hoc, uncommitted passes. `build/tools/pgn_to_positions` cannot replay this PGN: every one of its 1000 games carries its own `[SetUp "1"]`/`[FEN ...]` book header and that tool hardcodes `DEFAULT_POSITION` -- the exact wall S042 already hit over this same file. Positions are read instead with `python-chess`, one per game at that game's own midpoint ply, `random.Random(24).shuffle(range(1000))` picking the 400 games in deterministic order. `S024_census_run.py sample` regenerates it byte-for-byte |
| `S024_census_run.py` | the sampler and driver behind verdict 1's H0 census, 2026-09-12. `sample` writes the positions file above; `run <engine>` drives one persistent UCI process over the 400 positions at depth 10, Hash 16, `ucinewgame` between positions, reading the `info string cont_hist_census <writes_total> <writes_with_prev> <reads_total> <reads_with_prev> <reads_nonzero>` line a throwaway instrumented worktree prints once per search, and writes `S024_census.tsv`. The trap avoided is TOOLCHAIN.md's chess-oracle warning generalised to this engine: `printf '...\ngo depth 10\n' \| build/src/chesso` answers depth 1 at 49 nodes on both an unmodified binary and the census one, because closing stdin the instant the input is written reaches the engine the same way `quit` racing `go` does; a persistent `subprocess.Popen` with `isready`/`readyok` before every `go` and every `bestmove` read in full first, the way `S108_node_reach.py` and `S165_nmp_defender_sweep.py` already do, is what this script does too. A refused `position fen` (S216 precedent) exits loudly rather than silently measuring the previous board |
| `S024_census.tsv` | that census's output, 2026-09-12: 400 positions at depth 10 against a throwaway worktree at HEAD (`ef43bff`, which already carries `cace216`; the two commits between them touch no file the continuation-history mechanism lives in, checked by path-filtered `git log`). **112638096 nodes, 15.1 s**; `writes_with_prev`/`writes_total` **97.56 %**, `reads_with_prev`/`reads_total` **96.19 %**, `reads_nonzero`/`reads_with_prev` **27.14 %** (per-position share min 4.15 %, median 24.06 %, max 48.37 %) -- the table is exercised on nearly every write and read attempt in ordinary play, the opposite finding from the null-move census `src/search.cpp` cites ("0 of 301620") as the barely/never-touched check this pre-registration's H0 clause is modelled on. One row per position plus a `# totals` line; regenerate with `S024_census_run.py run <engine built from the same source as the worktree above>` |
| `S223_fen_census.py` | the census behind S223 and DEC-197, 2026-09-12, and **it runs**: every FEN literal in `tests/` (the vendored `tests/json/` excluded), `src/` and `tools/` against python-chess 1.11.2 `Board.status()`, reporting the ones a one-king-a-side and no-check-against-the-side-not-to-move load bound would refuse. Four-field literals are completed with `0 1` as `position fen` does (S176). Run with `~/.venv/chess/bin/python`, from the repository root. **A second pass was added when S223 landed**: it drives `build/src/chesso` over UCI, one `position fen` and one `fen` per literal, reads the `info string refused` line and compares *whether the engine refuses at all* against *whether the oracle reports one of the four king flags*, over the literals S208's two classes and the parser do not answer for first. Narrow on purpose -- python-chess also reports `BAD_CASTLING_RIGHTS`, `INVALID_EP_SQUARE` and `TOO_MANY_CHECKERS`, none of which this engine refuses and none of which it should. Exits non-zero on a disagreement, so it is re-runnable as a check and not only as a report. `--engine <path>`, `--no-engine` |
| `S223_fen_census.txt` | its output at `32660f8`'s tree, **before** the step: **3204 unique FENs, 3188 pass, 16 flagged** with the flag names and the `file:line` of every site -- two already refused by S208's classes, two syntax fixtures, three that become refusal cases, one comment-only, seven to re-pick. The step's cost list; `S223_position_fen_legality_boundary.md` sorts them into what happens to each |
| `S223_fen_census_after.txt` | the same script at S223's completing tree, 2026-09-13, and this one carries the engine pass: **3214 unique FENs, 3201 pass, 13 flagged, and 0 disagreements** over the 3199 compared (10 refused earlier by S208's classes, 3 by the parser or a field bound). The corpus grew by the step's own refusal cases and shrank by the seven re-picks; all 13 flagged are now either a refusal case asserting the refusal, a fixture S208 refuses first, or a FEN that appears only in a comment -- none is a position any test loads expecting success |
| `S199_drift.sh` | S199's drift instrument, written 2026-09-13 before any game: the engine as it stands now against the **pinned** early-S105 commit `f548ff4` (2026-08-20), one fixed 1000-pair match at `fastchess.sh`'s own defaults -- `8+0.08`, `Hash=16`, `noob_3moves.epd`, no `-sprt`, no `TC`/`HASH` override, which is the point. **Its header is the pre-registration**: the three `git` checks that pin the reference (`f548ff4^` is the regime commit `21c1949`, `git diff 21c1949 f548ff4 -- src/` empty, `ec4d1dd` the first `src/` change after), the build check done by reading rather than building because the machine was busy, why the pin answers the bare `id name Chesso` and plays anyway (DEC-204 (b)), the regime with an origin per line, what happens to the series if a default ever moves, the 56-minute estimate from the measured 2133 games an hour and the 6750 s ceiling, the abort rule (forfeits over 1.0 % a side; a crash voids; `Incomplete mating PV` from the reference expected and never a stop), and the +/- 15.2 nElo / +/- 11.6 logistic Elo the pair count buys. The reading rule itself is in the step file so the two cannot drift apart. Never a verdict and never an attribution (DEC-143, DEC-108) |
| `S199_drift.py` | the reader for that run, and **it runs**: banner and fastchess's **final** results block from the run log, pentanomial and pair variance from the PGN, one appended row of `S199_drift.tsv`, and `--check` for the trend. Stdlib only; it imports `S105_pairs` rather than editing it (this directory is append-only) and **derives** the candidate's engine name from the PGN instead of assuming it -- the trap S151's and S198's headers write down, where a name matching neither side scores every game as Black's and inflates the variance silently. Four refusals, so a row is never half from one run: no `SPRT-RUN-DONE` or any `SPRT-RUN-INVALID`, a `bounds` line that is not `none -- fixed ... rounds` (an SPRT's early-stopped estimate is a different, upward-biased quantity, DEC-063), a pentanomial from the PGN disagreeing with the printed one, and a PGN not naming exactly two engines one of which is the banner's reference. `--self-test` parses a fabricated eight-game run whose pentanomial `[1, 0, 2, 0, 1]` and pair variance 0.5000 are known by hand, with a decoy results block ahead of the real one to prove the last block wins; 16 assertions, green 2026-09-13 |
| `S199_drift.tsv` | the drift series itself, one row per block boundary: date, candidate sha, reference sha, regime (tc/hash/book, read off the banner), games, Elo and nElo with their 95 % half-widths, the pentanomial, the pair variance, and two hand-filled columns -- the step ids whose verdicts landed since the previous point and the ids whose conversions landed since it (F30, DEC-170). Created with its header row on 2026-09-13; the first point is taken after the S109 block lands (DEC-139). Two rows are comparable only when `ref` and `regime` agree, and the reader refuses to difference them otherwise -- the book has already moved once (DEC-189) |
| `S109_ltc.sh` | the S109 block's longer-control reading, DEC-202's second instance, written 2026-09-13 before any game: `600f448` (the landing commit) against `50e3661` (its parent, the pre-block commit) at `32+0.32` with `Hash=64`, a fixed 1000-pair match through `CAND`/`TC`/`HASH`/`ROUNDS`, taken beside S199's drift point so the boundary produces two numbers. **Its header is the pre-registration**: why the candidate is `600f448` and not the tests-only `1952c56` (`git diff 600f448 1952c56 -- src/` empty, S151's precedent), the three checks pinning the pair, the block's ten new `search_params.hpp` constants as DEC-202's bound class, a marked placeholder for the `8+0.08` figure the reading is taken against, the 3.7 h estimate from S151's **measured** 545 games an hour and the 27360 s ceiling, the abort rule, and the three readings -- against zero, against the SPRT's own figure over a combined error, and the pair variance at `32+0.32` read against S151's 0.2876. One difference from S151's run is pre-registered: both commits post-date S147/S170/S171, so an `Incomplete mating PV` count is worth a sentence and never an abort. An estimate, never a verdict; a regression opens a decision rather than a revert (DEC-202 (3)) |
| `S230_mine_r01_row.py` | S230's mining run, 2026-09-14, and **it runs**: four stages that between them pick the `capture_mates` row of `tests/test_search.cpp` "pruning does not hide a forced mate" which separates `R01_extra_reduction_gives_check`, over this project's own positions and nothing else (DEC-016). `pool` builds 39987 FENs -- the two committed S145 mate sets plus the last 40 plies of every game of `S219_aa_calibration.pgn`; `label` runs one stockfish process at a node limit over them and is a shortlist whose numbers reach no test, because a node-limited stockfish is reproducible only inside one identical call sequence; `line` re-asks each mate in a **fresh** process at depth 20 and keeps the ones whose principal variation carries a capture that gives check for the mating side; `depths` compiles a throwaway driver against `build/src/libchesso_engine.a` -- `search_fen()` of the search suite, line for line -- and sweeps depths 3 to 12. **`depths` is the re-derivation DEC-142 wants beside that table's depths and mutant labels**: run it on the shipped build, then again with a mutant of `tools/mutants/S091_capture_see.py` applied to the working tree, and a row's depth is one the first reports the mate at and the second does not. The engine measurement is deliberately not `go depth N`: the case calls `search()` once at a fixed depth from a cold table, the UCI reply is iterative deepening over a table that carries between depths, and the case's own comment records that the shipped engine answers these positions in centipawns over UCI at the depths they are read at |
| `S230_candidates.tsv` | its 117 survivors, 2026-09-14: one row per position whose oracle line carries such a capture, with stockfish's mate distance, node count and principal variation in UCI and SAN from a fresh depth-20 process, the capture itself, and the engine's own `search_fen()` profile over depths 3 to 12. The label pass behind them was stopped at 32000 of the 39987 pool positions, once the row was decided and a widening had returned nothing; the tail of the pool yields roughly one mate per thousand positions against **two in three** for the S145 sets (267 of their 400, 30 over the 31600 game positions scanned), which is why stopping there costs little. Of the 117, **114 went into the separation pass**; the other three are the rows already in the table, swept on the shipped build only. **Two of the 281 swept separate R01 at any depth and both by a single depth** -- `8/2N1Pkp1/1p1P3p/...` at 12 out of 12 alone, and `1r3r1k/2p1n1pp/8/...` at 11 out of `d9 d10 d11 d12` -- which is the whole yield of the pool and the measure of how thin that kill is in the tree S222 left. The second is the row S230 shipped, because a profile of four consecutive depths is a row and a profile of one depth is an accident waiting to move |

| `S230_r01_sweep.tsv` | the separation pass itself, 2026-09-14, and the evidence for the row's label: all **281** candidate positions swept over depths 3 to 12 **twice** -- once on the shipped build, once with `R01_extra_reduction_gives_check` of `tools/mutants/S091_capture_see.py` patched into the working tree and `chesso_engine` rebuilt -- with the depth each build reports the mate at and the `separated_at` column that applies DEC-209 clause 4 (a depth the shipped build reports the mate at and the mutant does not). The 281 are 114 of `S230_candidates.tsv`'s 117 survivors plus the 167 labelled mates the line filter dropped, kept because a position without a capture on its oracle line can still separate a mutant and the pass had to be able to say that it did not. **Two rows carry a `separated_at`**, and it is one depth each |
| `S230_table_fens.txt` | the four FENs of `capture_mates` in `tests/test_search.cpp` "pruning does not hide a forced mate", in table order, so that table's GOLDEN block and `DEV_MANUAL.md`'s DEC-142 list can name a **runnable** re-derivation instead of describing one: `S230_mine_r01_row.py depths --fens adocs/data/S230_table_fens.txt --out <scratch> --lo 3 --hi 12`, on the shipped build and again per mutant. A row added to that table is added here in the same commit |
| `S194_book_start_key.py` | the DEC-142 re-derivation of `tests/test_engine.cpp`'s book goldens (S194, 2026-09-15), from outside the project: the embedded book's entries for the start position and the S175 position read with python-chess's own Polyglot reader and zobrist -- 13 entries, total weight 34700, `e2e4` heaviest at 12956, one entry for the S175 key -- so a rebuilt `src/openings.bin` moves the test rather than the test carrying a stale copy; `~/.venv/chess/bin/python`, from the repository root |
| `S222_spsa.sh` | S222's narrow history lane, the pre-registration and the runner, written 2026-09-14 before a game was played and run that night. **Its header is the pre-registration**: the eleven axes with their declared bounds and `c_end`, why the two suspects DEC-194 wrote reduce to one axis (bonus, malus, bound and weight over one table carry three degrees of freedom and the fourth is a gauge), why `HistPruneCoeff` is in the lane and why the pruning rule's input is not changed inside it (DEC-205), why no `Tm*` axis (DEC-094, DEC-200), the regime as S085's verbatim, why the book is `UHO_4060_v3.epd` and not the harness one (DEC-209 clause 1), the 8 h 30 m estimate from S085's **measured** 24.05 s an iteration with its 17 h ceiling (DEC-155), the abort rule, the mid-run reads, and **five pre-registered readings of where `ContHistWeight` lands** including the stuck-run rule and the "at or under 5" row that would have owed a second attribution SPRT. One change was made to the file after the run and it is dated and confined to plumbing: `check`'s output now goes to `${OUT}.check.log` and only its non-marker lines are echoed on, because the driver ends that stage with `SPSA-DONE` too and the run's log therefore carried two markers |
| `S222_spsa_run.json` | the frozen config that run played, written by the driver at launch: the eleven axes with their range, `c_end` and a `why` per axis, the regime (1250 x 24 pairs, `2+0.02`, Hash 16, one thread, concurrency 12, alpha 0.602, gamma 0.101, a_ratio 0.1, r_end 0.004, seed 222), the book and its 242201 openings, the adjudication line including `twosided=true` (S212, DEC-174), and the config digest `43f8f55365901dd7` |
| `S222_spsa_trajectory.tsv` | the run itself, 2026-09-14 18:05:58 to 2026-09-15 02:43:29, **8 h 37 m against the 8 h 30 m estimate**: one row per iteration with `c_scale`, `r_k`, the iteration's `y`, its win/loss/draw counts, its pentanomial and all eleven axis values. The three mid-run reads the pre-registration asked for, taken over the whole file: `c_scale` decays 2.055 -> 1.150 at a quarter -> 1.072 at a half -> 1.000, as designed; `y` is centred at 0.102 with standard deviation 6.05 over -35 to +35 and only 7.0 % of iterations at zero, which is real spread and not the "barely changing" trajectory the fishtest wiki calls useless; and **no axis sits at a bound except `HistoryMalusQuad`, at its floor for 18.8 % of iterations**, which is where it landed and is far from S085's `RfpMinPly` at 72.5 %. 60000 games, W 21663 L 21536 D 16801 |
| `S222_spsa.log` | that run's console: the banner with the head sha, the clean tree and the tune binary's sha256, `check`'s eleven probes at both bounds of every axis, and the driver's own final rounded JSON vector. **It carries two `SPSA-DONE` markers**, one from `check` and one from the run, which is the defect the script now fixes and which the watcher on the night had to be told to count to; the file is the record of what happened and is not edited |
| `S231_spsa.sh` | S231's continuation lane, the pre-registration and the runner, written 2026-09-18 before a game was played. **Its header is the pre-registration**: the six axes with their declared bounds and `c_end`, why the one-ply three are in the lane and plain history's are not, why no second bound (DEC-209's gauge argument applied to the second table unchanged), why no `Tm*` axis (DEC-094, DEC-200), the regime as S085's verbatim, the book as `UHO_4060_v3.epd` (DEC-209 clause 1), the **8 h 45 m estimate from S222's measured 8 h 37 m 31 s for this exact shape** with its 18 h ceiling (DEC-155), the abort rule, the mid-run reads and **six pre-registered readings**. Two of them differ from S222's header on purpose and the difference is the point: a stuck lane **still owes the SPRT**, because the reference is the tree before the step and what the run prices is the second table itself; and an H0 reverts cleanly, because unlike S222's eleven-axis vector no axis here would be left without a verdict. `check` writes its own log from the start, which S222's script had to be repaired to do |
| `S231_research_witness.py` | the script DEC-142 asks for beside the fixture of `tests/test_search.cpp` "a reduced move that beats alpha is searched again" — the position it drives and the depth it drives at, both measurements and neither a property of the position. It compiles a throwaway driver against the engine library as it stands, drives that node line for line over the 400 committed census positions, and applies a rule stated **before** its own sweep: keep the pinned position if it still re-searches somewhere in depths 4 to 8 and take the lowest such depth, otherwise take the first position of the corpus that re-searches at every one of them. Written at S231, where the two-ply table emptied depth 6 alone — `researched` over depths 4 to 8 went 7, 2, **4**, 3, 2 at the parent and 7, 2, **0**, 4, 3 here — and the rule answered depth 4, which is the largest count in the range and not the smallest |
| `S231_spsa_run.json` | the frozen config that lane played, written by the driver at launch: the six axes with their range, `c_end` and a `why` per axis, the regime (1250 x 24 pairs, `2+0.02`, Hash 16, one thread, concurrency 12, alpha 0.602, gamma 0.101, a_ratio 0.1, r_end 0.004, seed 231), the book and its 242201 openings, the adjudication line including `twosided=true` (S212, DEC-174), and the config digest `86e1ca1218e656c0`. It also carries the three `_why_` notes the lane is defined by: why these six and nothing else, why no `Tm*` axis and no second bound, and why the book is not the harness one |
| `S231_spsa_trajectory.tsv` | the run itself, 2026-09-19 18:41:56 to 2026-09-20 03:22:41, **8 h 40 m 45 s against the 8 h 45 m estimate**, 25.00 s an iteration: one row per iteration with `c_scale`, `r_k`, the iteration's `y`, its win/loss/draw counts, its pentanomial and all six axis values. The three mid-run reads the pre-registration asked for, taken over the whole file: `c_scale` decays 2.055 -> 1.150 at a quarter -> 1.072 at a half -> 1.000, as designed; `y` is centred at 0.016 with standard deviation 5.48 over -17 to +18 and 8.1 % of iterations at exactly zero; and **no axis touches a bound on any iteration**, 0.0 % on all six at both ends, where S222's `HistoryMalusQuad` sat at its floor for 18.8 % and S085's `RfpMinPly` for 72.5 %. Per-axis ranges over the run: Bonus 17-19, Malus 17-20, Weight 21-26, 2Bonus 17-21, 2Malus 17-22, 2Weight 21-29. 60000 games, W 20794 L 20774 D 18432, 0 time forfeits on either side |
| `S231_spsa.log` | that run's console: the banner with the head sha `0c3f0eb`, the clean tree, the tune binary's sha256 and the governor, `check`'s six probes at both bounds of every axis, and the driver's own final rounded JSON vector. One `SPSA-DONE`, the run's: the check stage writes its own log since S222's lane had to be repaired for carrying two markers |
| `S231_census.txt` | both continuation tables' use on the **fitted** build, 2026-09-20: `S024_census_run.py` over the same 400 positions at depth 10, with the eight counters this step's driver defines, so the one-ply shares stand beside S222's 96.62 / 95.15 / 19.15 % and the two-ply ones exist at all. **96.53 / 95.30 / 19.89 % one-ply and 94.24 / 95.72 / 27.51 % two-ply**, plus a same-tree control at phase one's incumbent vector -- 96.53 / 95.36 / 20.22 and 94.32 / 95.79 / 27.31 -- which attributes every share to the tree rather than to the fit, to within four tenths of a point. The reading: the two-ply table is consulted about as often as the one-ply one and reads a non-zero entry **more** often, 27.51 % against 19.89 %, so a verdict on this vector is not a verdict on inert wiring. Carries the throwaway instrumentation diff verbatim, so the run reproduces without those eight counters ever having been in the tree |
| `S231_apply_fit.py` | the script that wrote the lane's six fitted values into `src/search_params.hpp`'s X-macro rows, 2026-09-20, reading the driver's own rounded JSON out of `S231_spsa.log` rather than retyping it, refusing any substitution that would change a column's width because that table is aligned and its continuation backslashes sit in a fixed column. `--check` reports without writing |
| `S231_verify_fit.py` | the cross-check beside it, 2026-09-20: the built tune binary's own `uci` reply read back and compared against `src/search_params.hpp`'s 53 X-macro rows, against the lane's six fitted values, and against `tests/test_search_params.cpp`'s golden table row for row -- name, default, min and max, in order. It is the mechanical half of DEC-142's re-derivation for that golden, whose own site names the header as the derivation and a diff as the re-derivation |
| `S231_sprt.sh` | the pre-registration of S231's gainer SPRT, written 2026-09-20 before a game was played: the six-axis vector incumbent -> fitted with its unrounded theta, why the reference is `3a649c0` and not `HEAD` (DEC-210, one vector under one verdict -- measured against `HEAD` the table itself would stay unpriced), which of the lane's six pre-registered readings the fit landed in (the vector moves; the weight near 26, so no pinned-zero run and `S231_sprt_pinned.sh` does not exist), the lane's own wall and forfeit figures, the bench and `search_bench` counts with the census beside them, the `{0, 5}` nElo pair the step named before the fit returned and what it costs (41861 and 25591 games, 19.8 h and 12.1 h at 2110), the regime and why the book is the harness one and not the lane's, the abort rule, the two open test gaps it is taken while open (S231's own `I03` mutant, and the ordering-band staleness this phase found), and the three outcomes with DEC-222's two removals owed only under H1 |
| `S231_sprt.log` | S231's gainer SPRT, 2026-09-20 04:45:41 to 10:23:54, 5 h 37 m 42 s: the banner and both identity lines (`cand-55891bb  Chesso 55891bb native`, `ref-3a649c0  Chesso 3a649c0 native`), seed `20260920044538`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H0 accepted**, LLR -2.95 against (-2.94, 2.94), `Elo -2.65 +/- 4.82`, `nElo -3.40 +/- 6.20`, W 3654 L 3746 D 4670 over 12070 games, `Ptnml(0-2) [556, 1464, 2097, 1352, 566]`, LOS 14.08 %. **0 time forfeits on either side** over the PGN's 12071 games (8364 adjudications, 3707 natural ends); `Incomplete mating PV` 1 against 1. The first verdict whose closing commit carries DEC-220's result block, checked by `tools/gate.sh` against this file's `Results of` line |
| `S231_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-20: markers, banner, the fastchess final block, timing, terminations, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 6035 complete pairs, pair score mean 0.9967 and **variance 0.3105**, sd 0.5572, the five pair-score buckets 9.6 / 23.8 / 34.0 / 22.8 / 9.8 %, white winning both of 580 pairs (9.6 %), 115.4 plies and 19.6 s a game |
| `S222_census.txt` | the continuation table's use on the **fitted** build, 2026-09-15: `S024_census_run.py` over the same 400 positions at depth 10, so the shares stand beside S024's 97.56 / 96.19 / 27.14 %. **96.62 / 95.15 / 19.15 %**, plus a same-tree control at the incumbent vector -- **18.94 %** -- that attributes the third figure: the eight-point fall from S024 belongs to S109's and S091's pruning, which cut the tree to 30474552 nodes from 112638096 over the same positions, and not to the fit, which moves it 0.21 points the other way. Per-position spread 2.04 % to 38.88 %, median 17.12 %. Carries the throwaway instrumentation diff verbatim, so the run reproduces without those five counters ever having been in the tree |
| `S222_sprt.sh` | the pre-registration of S222's gainer SPRT, written 2026-09-15 before a game was played: the eleven-axis vector incumbent -> fitted from the driver's own JSON, why one verdict and not eleven, which of the lane's five pre-registered readings the weight landed in (near 25, so no pinned-zero run and `S222_sprt_pinned.sh` does not exist), the bench and `search_bench` counts with the census beside them, the `{0, 5}` nElo pair the step named before the fit returned and what it costs (41861 and 25591 games, 19.5 h and 11.9 h at 2150 an hour), the regime and why the book is the harness one and not the lane's, the abort rule, the open findings (none -- every filler in the block is closed), and the three outcomes. **The H0 branch is the one with work in it**: the six plain-history coefficients, `QuietHistoryMax` and `HistPruneCoeff` were fitted in the same vector and have no verdict of their own, and the file names what may and may not be concluded about them |
| `S222_sprt.log` | the run's banner, identity lines, fastchess's final block (H1, LLR 2.95, `Elo 11.13 +/- 6.90`, `nElo 13.87 +/- 8.59`, 6278 games), 0 forfeits, `Incomplete mating PV` 0 and 0; run 2026-09-15 04:00 to 06:55 against `d785b89` |
| `S222_sprt_pairs.txt` | the reading in the pre-registration's order -- markers, banner, final block, timing, terminations (4578 adjudications, 1701 natural over the 6279 games in the PGN), forfeits, the PV counts -- and `S105_pairs.py` over 3139 pairs: pair-score mean 1.0320, variance 0.3218, draws 35.1 %, 113.3 plies and 19.5 s a game |
| `S098_v1_hist_census.py` | the census behind `LmrHistDiv`'s seed, 2026-09-15. `census` builds a detached worktree carrying the **working tree's** `src/` -- so what is measured is the landing and not `HEAD` -- patches in five throwaway counters at the one site the reduction reads history, builds it, and drives S024's own 400 positions through S024's own `Engine` class at `go depth 10` and `go depth 12`, `ucinewgame` between positions, removing the worktree afterwards. A site is one call of `lmr_adjusted_reduction` in `negamax_at`: a quiet past the third legal move at depth 3 or more with neither side in check. The signed sum is kept as a histogram and not a row per site -- tens of millions of sites, and the only questions asked are percentiles -- so the percentiles are exact to the unit and nearest-rank. The patch is inside the script rather than in a diff beside it, so the run reproduces from the repository alone; the instrumented binary's bench signature is asserted equal to the shipping build's, which is what says the counters do not move the tree |
| `S098_v1_hist_census.txt` | that census's output, 2026-09-15, and the derivation of `LmrHistDiv` **430** -- the seed the DEC-212 lane started from; the fit then moved the pair to 699 and 3 (`S098_v1_spsa_trajectory.tsv` below). Depth 12: 5464717 sites, |sum| p50 174, **p75 430**, p90 1442, p99 5362, zero on 11.80 %; at that divisor with the clamp at 2 the term moves 74.96 % of sites by nothing, 10.38 % by one ply and 14.66 % by two. Depth 10: 2105964 sites, 107 / 258 / 689 / 4347. **The first seed, 8675 -- half the saturated band -- is reached by 0.006 % of sites at depth 10 and 0.011 % at depth 12**, which is why it was re-seeded before the SPRT rather than after it, and why the by-depth ablation in `S098_v1_sprt.sh` read +0.00 % of the bench nodes below depth 12 at that value. The raw-reduction column reads 100 % at both depths and that is the eligibility, not a broken counter: the table returns at least 1 everywhere past the third move at depth 3 or more |
| `S098_v1_spsa.sh` | the pre-registration and runner of verdict 1's two-axis lane, 2026-09-15, DEC-212. Its config was `tools/spsa_s098v1.json` and is `S098_v1_spsa_config.json` in this directory since 2026-09-16 (the row below says why it moved). Written because the divisor was seeded twice in one day from two defensible derivations over this engine's own data -- half the saturated band, 8675, measured inert by the step's own ablation, and the census's depth-12 p75, 430, at which `bench` grows 60.63 % -- so the scale is fitted before it is judged. S085's regime verbatim: 1250 x 24 pairs = 60000 games at `2+0.02` on `books/UHO_4060_v3.epd`, estimate **8 h 37 m** from S222's measured 24.84 s an iteration, **ceiling 17 h 15 m**, a night run under DEC-155. Carries S222's `check`-log plumbing so the run log holds one marker, S222's abort rule, and the readings fixed before the fit: a rounded vector equal to the incumbent is a stuck run owing no SPRT, `LmrHistClamp` at 0 is the term inert by the fit's own word and the technique leaves the plan, and the divisor is read against the census's own p90 and p99 rather than against a threshold picked afterwards |
| `S098_v1_spsa_config.json` | that lane's frozen input, 2026-09-15 (written as `tools/spsa_s098v1.json` and moved here on 2026-09-16 when DEC-213 removed the two parameters it names -- under `tools/` a config reads as runnable and this one would fail `check`): `LmrHistDiv` 430 in 1 to 34700 with `c_end` 128, `LmrHistClamp` 2 in 0 to 4 with `c_end` 1. **The bounds are the declared ones from `src/search_params.hpp` and unnarrowed, which is forced rather than chosen** -- `spsa_driver.py`'s `check` refuses a config whose bounds are not equal to the binary's own `uci` listing, so the census's region is carried by the seed and by `c_end` instead. `c_end` 128 is half the gap between the census's depth-12 p50 (174) and p75 (430), a quarter of the distribution; what that resolution cannot reach in this budget is the band seed at 8675, which the file states rather than implies and which the SPRT's H0 leg tests by one release rebuild. `check` passes both axes (`LmrHistDiv` 1 -> 32857 nodes against 34700 -> 51189; `LmrHistClamp` 0 -> 51189 against 4 -> 27458) with one warning recorded in the runner: the first iteration's `2c` is 4.11 against the clamp's range of 4, which holds for one iteration of 1250 |
| `S098_v1_spsa_trajectory.tsv` | the lane's trajectory, 2026-09-15: 1250 rows, one per iteration, with `c_scale`, `r_k`, `y`, the pair counts and both axes. What it records beside the endpoint: `LmrHistDiv` never touched a bound (0 of 1250 iterations), ran 422 to 747 with a median of 648 and ended at 699; `LmrHistClamp` sat at a bound on 39 iterations of 1250 -- five settings being what they are -- ran 2 to 4 and ended at 3; `y` is centred at -0.139 with a standard deviation of 5.78 over -21 to 16, which is the real spread the fishtest wiki's "barely changing" trajectory would not have. `c_scale` decays from 2.054887 to 1.000000 as designed |
| `S098_v1_spsa_run.json` | that lane's run record, 2026-09-15: the config as loaded, the schedule, the seed and the final theta, `[698.6584395515816, 2.7319152681598724]`, which the driver rounds to 699 and 3 -- the two integers `src/search_params.hpp` now carries |
| `S098_v1_spsa.log` | the run's own log, 2026-09-15 11:49:38 to 20:37:23, 8 h 47 m 45 s for 60000 games against the 8 h 37 m the pre-registration estimated from S222's measured 24.84 s an iteration. Carries the banner, the binary's sha256, the `check` lines echoed on from their own file, the two `LmrHistClamp` range warnings the pre-registration predicted, the rounded JSON and **one** `SPSA-DONE` -- the check-log plumbing S222's lane added doing its job, so a watcher never had to count markers |
| `S098_v1_spsa_check.log` | the `check` that ran before the first game, 2026-09-15, kept separate from the run log for that reason. Both axes reach the search -- `LmrHistDiv` 1 -> 32857 nodes against 34700 -> 51189, `LmrHistClamp` 0 -> 51189 against 4 -> 27458 -- with the one warning the runner's header derives and bounds: the clamp's first iteration has `2c` = 4.11 against a range of 4, which holds for one iteration of 1250 |
| `S098_v1_sprt.log` | verdict 1's run, 2026-09-15 21:19:10 to 2026-09-16 02:44:56, 5 h 25 m 46 s: the banner and both identity lines (`cand-0408447  Chesso 0408447 native`, `ref-1db5b8e  Chesso 1db5b8e native`), seed `20260915211910`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H0 accepted**, LLR -2.96 against (-2.94, 2.94), `Elo -2.92 +/- 5.02`, `nElo -3.70 +/- 6.34`, W 3691 L 3788 D 4045 over 11524 games, `Ptnml(0-2) [578, 1352, 1985, 1283, 564]`, LOS 12.65 %. **0 time forfeits on either side** over the 11526 games the PGN holds (8342 adjudications, 3184 natural ends), and `Incomplete mating PV` 7 candidate against 3 reference, recorded rather than treated as a stop |
| `S098_v1_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-16: markers, banner, the fastchess final block, timing, the game and termination counts, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 5762 complete pairs, pair score mean 0.9916 and **variance 0.3125**, sd 0.5590, the five pair-score buckets 10.0 / 23.5 / 34.4 / 22.3 / 9.8 %, white winning both of 617 pairs (10.7 %), 114.0 plies and 19.8 s a game. The variance sits beside S219's 0.2905 and S212's 0.2939 on this book |
| `S098_v1_leg2_sprt.log` | leg 2's run, 2026-09-16 03:09:34 to 09:20:06, 6 h 10 m 32 s: candidate `73fbf05` (`LmrHistDiv` 1442, `LmrHistClamp` 3) against `1db5b8e`, and fastchess's final block -- **H0 accepted**, LLR -2.95, `Elo -2.34 +/- 4.73`, `nElo -2.94 +/- 5.95`, W 4187 L 4275 D 4616 over 13078 games, `Ptnml(0-2) [667, 1518, 2240, 1464, 650]`. 0 time forfeits on either side, `Incomplete mating PV` 4 candidate and 1 reference, 2119 games an hour. With the verdict's own H0 at 699 this is the second of the two legs the bisection was given, and DEC-213 is the removal it triggers |
| `S098_v1_leg2_sprt_pairs.txt` | that run read in the pre-registration's order, 2026-09-16: markers, banner, the final block, timing, the game and termination counts, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 6539 complete pairs, **pair variance 0.3154**, beside verdict 1's own 0.3125 and S219's 0.2905 on this book |
| `S098_v1_leg2_sprt.sh` | the pre-registration of verdict 1's **second bisection leg**, written 2026-09-16 before a game was played: `LmrHistDiv` 699 -> 1442 with the clamp unchanged at 3, one value and a release rebuild against the same `1db5b8e` the verdict used. 1442 is the census's depth-12 p90 -- a tenth of the rule's sites rather than a quarter -- stated as a pre-registered bisection point and not a fit, inheriting the census's one-pass caveat. Carries the bench and node counts, the by-depth ablation whose off column is the reference's totals exactly, the same `{0, 5}` nElo pair as the verdict it bisects and why it is the same, 41861 / 25591 worst-case games at the 2124 an hour the verdict measured, and the three outcomes -- H1 keeps the term at 1442 / 3 and opens verdict 2 on that tree; **H0 spends the bisection and removes the term from the tree** in DEC-194's shape, listed line by line down to what stays (`quiet_history_sum`) and what the evidence keeps; no verdict reads as H0 |
| `S098_v1_sprt.sh` | the pre-registration of S098 verdict 1's gainer SPRT, written 2026-09-15 before a game was played: the history-scaled late move reduction with S109's gate re-pointed to the same helper, `{0, 5}` nElo at `8+0.08`, `Hash 16`, `noob_3moves.epd`, worst-case 41861 / 25591 games, the abort rule, the two constants with their DEC-105 forms, the by-depth ablation whose off column is the parent's bench totals exactly, and the three outcomes with the H0 bisection's two legs. Carries the re-seed and its reason in full |
| `S098_v2_node_census.py` | the firing census DEC-214 asks for before verdict 2's SPRT, 2026-09-16, and the same throwaway-worktree method `S098_v1_hist_census.py` built: a detached worktree carrying the **working tree's** `src/`, patched with write-only counters at the one site the reduction is consulted, built, and driven over S024's own 400 positions at `go depth 10` and `go depth 12`. Two differences from verdict 1's. It counts four booleans -- `cut_node`, `!improving`, a capturing table move, `is_pv` -- rather than a distribution, because a 0-to-2 ply count has no scale to get wrong and the only question a seed can still fail is whether the condition is ever true. And **it also patches the four constants to 0**, so the shares belong to the tree the SPRT's reference searches and the signature check has an exact number to hold: the instrumented bench must read `5685915`, the total the commit before the landing prints, which says both that the counters do not move the tree and that verdict 2's plumbing is inert at its off values in the Release build |
| `S098_v2_sprt.sh` | the pre-registration of S098 verdict 2's gainer SPRT, written 2026-09-16 before a game was played: the four node-type terms as **one** verdict (DEC-063, DEC-214), `{0, 5}` nElo at `8+0.08`, `Hash 16`, `noob_3moves.epd`, `REF` the tree DEC-213's removal left, worst-case 41861 / 25591 games at the 2124 and 2119 an hour the two runs before it measured -- 19.7 h and 12.0 h, a night run. Carries the census's share beside each seed and each seed's DEC-105 form, the by-depth ablation whose off column is the reference's totals exactly and whose **on row is not monotone** (-21.74, -10.04, -17.70, **+3.77**, -10.46, -3.81 per cent over depths 9 to 14), the one best move that moves, and the three outcomes -- H1 opens verdict 3 on this tree; H0 bisects by off values, `{LmrTtCapture, LmrPv}` before `{LmrCutNode, LmrNotImproving}`, two legs at most, each a release rebuild, with the `cut_node` alternation re-checked before any delta is blamed on a term; no verdict reads as H0 |
| `S098_v2_node_census.txt` | that census's output, 2026-09-16, and what decided that **no term ships at 0**: at depth 12 over 5478549 sites `cut_node` is true on 21.12 %, `!improving` on 52.75 %, a capturing table move on 24.83 % and `is_pv` on 26.72 %; at depth 10 over 2103446 sites, 17.74 / 52.73 / 25.37 / 36.19 %. DEC-214's inert threshold is one per cent and the lowest share is twenty times it. The joint distribution is the other half of the reading -- at the midpoint seeds the four terms sum to +1 ply on 41.95 % of sites at depth 12, +2 on 18.15 %, +3 on 1.45 %, nothing on 27.85 % and **-1 on 10.60 %**, the PV term's own share -- so the verdict is a rule that moves about seven sites in ten and not a rule that fires at the edges |
| `S098_v2_sprt.log` | verdict 2's run, 2026-09-16 11:59:32 to 12:55:41, 56 m 09 s: the banner and both identity lines (`cand-a771260  Chesso a771260 native`, `ref-50fd965  Chesso 50fd965 native`), seed `20260916115932`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H1 accepted**, LLR 2.95 against (-2.94, 2.94), `Elo 29.05 +/- 11.52`, `nElo 38.70 +/- 15.26`, W 712 L 546 D 732 over 1990 games, `Ptnml(0-2) [60, 200, 358, 268, 109]`, LOS 100.00 %. **0 time forfeits on either side** over the 1991 games the PGN holds (1397 adjudications, 594 natural ends), and `Incomplete mating PV` 14 candidate against 9 reference, recorded rather than treated as a stop |
| `S098_v2_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-16: markers, banner, the fastchess final block, timing, the game and termination counts, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 995 complete pairs, pair score mean 1.0834 and **variance 0.2805**, sd 0.5296, the five pair-score buckets 6.0 / 20.1 / 36.0 / 26.9 / 11.0 %, white winning both of 99 pairs (9.9 %), 115.2 plies and 19.6 s a game. The variance sits beside S219's 0.2905 and S212's 0.2939 on this book |
| `S098_v3_research_census.py` | the firing census DEC-214 asks for before verdict 3's SPRT, 2026-09-16, the same throwaway-worktree method the two censuses above use: a detached worktree carrying the **working tree's** `src/`, patched with write-only counters, built, and driven over S024's own 400 positions at `go depth 10` and `go depth 12`. A site is one execution of the reduced fail-high re-search block in `negamax_at` -- the zero-window repeat a *reduced* late move that beat alpha is owed, and the only place `lmr_research_depth` is consulted -- and four numbers are recorded at it: the reduction taken, the reduced score, the node's alpha and its fail-soft best before the move. Kept as five histograms with one-unit bins and named overflow buckets, two of them conditioned on `r >= 2`, so either **condition's** share is readable at any margin without re-running and a quantile inside the range is exact -- and three counters for the three outcomes of the rule **after precedence**, which the histograms cannot give because they cannot see the overlap, at the seeds the script reads out of `src/search_params.hpp` so they cannot drift. **It also patches the two off values in** -- `LmrShallowerMargin` 0 and `LmrDeeperMinReduction` at its range top -- so the distribution belongs to the tree the SPRT's reference searches and the instrumented bench must read `5469072`, the total the commit before the landing prints. **It no longer runs, on purpose, since 2026-09-18**: the shallower path was removed after the bisection's leg 1 read H1, so four of the six quantities it publishes have no referent and its off patch has no anchor. `refuse_if_the_rule_moved()` exits before a worktree is built and names what changed, rather than letting the anchor failure stand in for it |
| `S098_v3_research_census.txt` | that census's output, 2026-09-16, re-run 2026-09-17 with the joint counters the Tier-1 check asked for, and what decided that **both paths keep the (c) seed 47**: at depth 12 over 106610 re-search sites the **deeper path fires on 4.60 %** and the shallower path on 47.77 %, against DEC-214's one-per-cent inert threshold; at depth 10 over 44871 sites, **3.97 %** and 44.36 %. Those are shares of the **path after precedence** and not of the condition: the shallower path is tested first, so the two conditions -- 6.89 % and 47.77 % at depth 12 -- overlap on 2.29 % of sites that belong to the shallower path, and the first pass published the condition share as the path's. `r >= 2` on 52.41 % of sites at depth 12 and 48.38 % at depth 10, the reduction never above 6. Quantiles at depth 12: `score - best` p10 2, p25 6, p50 14, p75 32, p90 73 over all sites and 2 / 5 / 12 / 27 / 59 where `r >= 2`; `score - alpha` 1 / 3 / 9 / 22 / 47 and 1 / 3 / 8 / 20 / 43. **The one measurement that moved the design**: the deeper condition is still true on 2.86 % of sites at the margin's range top, so the range top is *not* an off value for that path and the off switch is `LmrDeeperMinReduction` at its own top instead. **Kept as the evidence it was, not regenerable**: the shallower path was removed on 2026-09-18 and the script above now refuses; on the tree that removal leaves, the deeper path's share is its condition's share, 6.89 % at depth 12 and 5.85 % at depth 10, because nothing takes sites from it any more |
| `S098_v3_sprt.sh` | the pre-registration of S098 verdict 3's gainer SPRT, written 2026-09-16 before a game was played and pinned 2026-09-17: the re-search depth rule as **one** change against `efdbc9b`, candidate `cb40afd`, `{0, 5}` nElo at `8+0.08` on `noob_3moves.epd`, the census shares beside each seed, the worst-case 41861 and 25591 games priced at verdict 2's 2126 an hour, the 1 % forfeit abort rule, no open finding, and the three outcomes written before the games -- H1 keeps both paths and completes S098; H0 bisects one path at a time, the shallower off first, two legs at most by Release rebuild, the firing census re-read before either leg is blamed on a path; no verdict inside the budget is a zero and the rule leaves in DEC-194's shape |
| `S098_v3_sprt.log` | verdict 3's run, 2026-09-17 13:48:41 to 15:55:26, 2 h 06 m 45 s: the banner and both identity lines (`cand-cb40afd  Chesso cb40afd native`, `ref-efdbc9b  Chesso efdbc9b native`), seed `20260917134841`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H0 accepted**, LLR -2.96 against (-2.94, 2.94), `Elo -9.97 +/- 7.56`, `nElo -13.40 +/- 10.16`, W 1282 L 1411 D 1803 over 4496 games, `Ptnml(0-2) [185, 595, 794, 512, 162]`, LOS 0.48 %. **0 time forfeits on either side** over the 4497 games the PGN holds (3104 adjudications, 1393 natural ends), and `Incomplete mating PV` 8 candidate against 0 reference -- the one asymmetry in the run, recorded rather than treated as a stop |
| `S098_v3_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-17: markers, banner, the fastchess final block, timing, the game and termination counts, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 2248 complete pairs, pair score mean 0.9713 and **variance 0.2766**, sd 0.5260, the five pair-score buckets 8.2 / 26.5 / 35.3 / 22.8 / 7.2 %, white winning both of 225 pairs (10.0 %), 116.7 plies and 19.7 s a game. The variance sits beside verdict 2's 0.2805 and S212's 0.2939 on this book |
| `S098_v3_leg1_sprt.sh` | the pre-registration of verdict 3's **first bisection leg**, written 2026-09-17 before a game was played: `LmrShallowerMargin` 47 -> 0 with the deeper path untouched, one default and a release rebuild against the same `efdbc9b` the verdict used, which is what makes the two runs comparable. Carries the firing census re-read before the leg -- with the shallower path off the deeper path fires at its condition's share, 6.89 % of depth-12 re-search sites and 5.85 % at depth 10, where after precedence it fired on 4.60 % and 3.97 % -- the tree at the leg's default, the by-depth ablation whose off column is the reference's totals exactly, `{0, 5}` nElo at `8+0.08` on `noob_3moves.epd`, worst-case 41861 / 25591 games priced at verdict 3's realized 2128 an hour, the 1 % forfeit abort rule, no open finding, and the three outcomes written before the games: H1 retires the shallower path and completes S098 on the deeper one, H0 sends leg 2, and two H0s remove the rule in DEC-194's shape |
| `S098_v3_leg1_sprt.log` | leg 1's run, 2026-09-17 17:34:22 to 2026-09-18 00:23:23, 6 h 49 m 01 s: the banner and both identity lines (`cand-8d60551  Chesso 8d60551 native`, `ref-efdbc9b  Chesso efdbc9b native`), seed `20260917173422`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H1 accepted**, LLR 2.97 against (-2.94, 2.94), `Elo 5.75 +/- 4.37`, `nElo 7.44 +/- 5.65`, W 4559 L 4319 D 5632 over 14510 games, `Ptnml(0-2) [603, 1695, 2522, 1729, 706]`, LOS 99.51 %. **0 time forfeits on either side** over the 14510 games the PGN holds (10057 adjudications, 4453 natural ends), and `Incomplete mating PV` 28 candidate against 21 reference -- the asymmetry verdict 3's own run showed at 8 against 0 is gone with the shallower path, which is a consistent observation and not a diagnosis |
| `S098_v3_leg1_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-18: markers, banner, the fastchess final block, timing, the game and termination counts, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 7255 complete pairs, pair score mean 1.0165 and **variance 0.2981**, sd 0.5460, the five pair-score buckets 8.3 / 23.4 / 34.8 / 23.8 / 9.7 %, white winning both of 692 pairs (9.5 %), 116.6 plies and 19.8 s a game. The largest run on this book since S210 F22 and its variance the highest of the S098 family, beside verdict 3's 0.2766 |
| `2026-09-19_search_technique_study.md` | the 2026-09-19 analysis of literature and open-source resources -- published test ledgers whose commit messages carry SPRT result blocks, public wikis, published articles -- read for technique only: what the open-source record has that chesso does not, priced from its own commit-message SPRT ledger -- 796 commits of the record carry a full result, 794 since the 2025-02 search rewrite, 41,964,262 games over every recorded run -- an inventory of 25 techniques with no chesso step, each with the Elo and game count the record measured, a removal ledger of what was added and later deleted as free, the three places those numbers meet chesso's own S006, S015 and S025 zeros, and the full 2025 rebuild ladder. **Corrected in place on 2026-09-19** after the adversarial review (`adocs/audit/2026-09-19_study_review.md`, 17 findings, two reviewers): its cost rule withdrawn -- a chesso verdict costs what DEC-143 says -- its reading of the reserve placement corrected against DEC-133 and DEC-176, its provenance labelled, its §5 chronology and §4 traps repaired; the corrections are struck through in place and listed in a dated section at its end. No source, table or constant from any project is reproduced (DEC-016, DEC-084 as amended by DEC-105) and every technique taken from it is implemented from its published description (DEC-221). DEC-220 to DEC-222 are what the review decided |
| `2026-09-19_technique_ledger.py` | the parser behind every number in that study and its review: reads the `git log` of an open-source repository whose commit messages carry OpenBench-style result blocks, pairs Elo, control, bounds and game count positionally per commit, and prints the aggregates -- commits with a block, games over every run and first-run-only, controls and bounds by frequency, the bucket table of games against reported Elo at 8+0.08, the simplification-pair median, monthly sums, SPSA and removal counts. `python3 adocs/data/2026-09-19_technique_ledger.py <repository> [out.json]`; run 2026-09-19 against the record, reproducing the study's counts exactly where they were right and the review's where they were not |
| `ledger_seed.tsv` | the twenty SPRT verdicts taken before DEC-220, copied once on 2026-09-19 from `adocs/plan.md`'s ledger table as it stood at `0c3f0eb` and **never rewritten** -- a correction to a row is a decision, not an edit, and every verdict from that date on lives in its own commit's result block instead. Ten columns: the six table cells verbatim, the Elo the verdict cell carries where it carries one, the nElo estimate and interval read from the step file the `source` column names, and the effect class. `tools/ledger.py` prints it back as the table plus the mean, the median, the two class means and the totals; `--audit-seed-class` prints the class column against the coded rule, which reproduces eighteen of the twenty and disagrees on S149 and S207, both recorded as `plan.md` had them. Its row count is a golden in `tests/test_ledger.py`. S233 |
| `S095_sprt.sh` | the pre-registration of S095's gainer SPRT, written 2026-09-20 before a game was played and before the landing commit existed: what the re-formed step ships (a fifth term in `lmr_node_adjustment`, `LmrNoTtMove` seeded at DEC-105 (c), the call site's `tt_move == 0`), why the node-level cut the step was first written for is not what is being measured (DEC-222 clause 2) and why the open-source record's figure for either form is direction and nothing else (DEC-019; the technique is implemented from its description, DEC-221), the off value's equality proved on the tree at `LmrNoTtMove` 0 (DEC-215), the `{0, 5}` nElo pair the step's accepts named and what it costs (41861 and 25591 games, 19.8 h and 12.1 h at 2110), the regime, the abort rule, the two open test gaps it is taken while open, and the three outcomes -- H1 keeps the term at its seed for S127 to fit, H0 removes it as a behaviour-neutral revert and frees the node-level cut as a later step, no verdict is recorded as a zero. **It refuses to run until both shas are pinned**: the landing did not exist when the file was written, and a reference guessed from `HEAD~1` is a reference nobody checked (DEC-020) |
| `S095_sprt.log` | S095's gainer SPRT, 2026-09-20 18:58:48 to 2026-09-21 01:39:17, 6 h 39 m 58 s: the banner and both identity lines (`cand-3961c13  Chesso 3961c13 native`, `ref-b25452a  Chesso b25452a native`), seed `20260920185845`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H1 accepted**, LLR 2.96 against (-2.94, 2.94), `Elo 5.92 +/- 4.50`, `nElo 7.51 +/- 5.70`, W 4530 L 4287 D 5435 over 14252 games, `Ptnml(0-2) [632, 1674, 2388, 1683, 749]`, LOS 99.51 %. **0 time forfeits on either side** over the PGN's 14254 games (9965 adjudications, 4289 natural ends); `Incomplete mating PV` 10 candidate against 17 reference. The verdict-closing commit carries DEC-220's block, checked by `tools/gate.sh` against this file |
| `S095_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-21: markers, banner, the fastchess final block, timing, terminations, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 7126 complete pairs, pair score mean 1.0163 and **variance 0.3068**, sd 0.5539, the five pair-score buckets 8.8 / 23.3 / 34.0 / 23.8 / 10.2 %, white winning both of 624 pairs (8.8 %) |
| `S095_sweep_block.txt` | the mate-carry sweep grid on the tree S095's term leaves, 2026-09-20, taken because `tests/test_mate_carry.cpp` went red on `E_mate_minus9`'s short-line ceiling and that case's own protocol is to re-run `adocs/data/S203_case_sweep.sh` before deciding whether the walk regressed or the grid never covered the cell. 108 cells, six cases x two strides x nine budgets, the fourth recorded grid after S204's two and S109's. At E's own cell -- stride 1, 1500000 nodes -- it reads **22 mate lines, 11 short** against S109's 13 and 9 there: the absolute count rises past the ceiling of 9 and the share falls, 50 % against 69 %. Read with `S203_case_sweep.sh --ceilings` over all four grids the rule answers **A 5, B 15, C 0, D 2, E 11, F 5**, against the 5, 11, 0, 2, 9, 5 the test holds; raising a ceiling is a decision and the coordinator took it as **DEC-225**: E 9 -> 11 and B 11 -> 15, exactly the script's answer over the four grids and nothing else, on the ground that `unreached.empty()` -- the guarantee beside the ceilings -- holds on all six cases at their own budgets in both builds, so what rose is DEC-122's expected residue and not the promise. At E's cell the share of short lines fell from 69 % to 50 % while the count rose |
| `S095_candidates.tsv` | the 141 survivors of that filter, with the oracle's own distance, node count and principal variation in UCI and SAN and the reducible quiets it found on each line, 2026-09-20. The committed artefact of the mining, as `S230_candidates.tsv` is of S230's: the row `tests/test_search.cpp` took -- `4brbr/p2p1p1p/P2P1P1P/6R1/8/K7/8/1k6 w - - 0 1`, a mate in 2 whose key `Rc5` is quiet -- is in here with the 27 others that separated the guarded tree from the unguarded one, so the pick can be audited against the rule rather than taken on trust |
| `S095_mine_mate_row.py` | S095's mining run, 2026-09-20, and **it runs**: two stages that pick the mate row `tests/test_search.cpp` "pruning does not hide a forced mate" gains for this step -- one the **unguarded** no-table-move reduction hides and the guarded build finds. It **imports** `S230_mine_r01_row` rather than copying it (this directory is append-only; S148 importing S145's sweep is the precedent) and reuses its pool, its node-limited shortlist, its fresh-process oracle call and its depth sweep with the throwaway `search_fen()` driver. What is S095's own is the filter and the rule: `candidates` keeps a labelled mate only where the mating side plays a **quiet, non-checking, non-promotion move out of check on the oracle's line**, which is the class `may_reduce` in `src/search.cpp` lets late move reduction touch; `pick` joins a shipped sweep against a sweep of the tree with the guard opened (`const bool no_tt_move = tt_move == 0;` made `= true`, applied by hand, observed, reverted -- the S033 protocol) and takes the **lowest depth the shipped build reports the mate at and the unguarded build does not**, breaking ties by the longest run of consecutive depths in the shipped profile. The rule is stated in the header before the sweep runs (DEC-209 clause 4) |
| `S097_v1_sprt.sh` | the pre-registration of S097's **first** verdict, the singular extension, written 2026-09-20 before a game was played and before the landing commit existed: what the candidate ships (`excluded_move` through `negamax_at` with the five gates that belong to it, the half-depth verification search against a window under the table entry's score, one ply on the move it calls singular, and four seeds at DEC-105 (c) with the multicut's switch inert beside them), why the record's +11 to +20 is direction and nothing else (DEC-019; the technique is implemented from the step file's own description, DEC-221), the multicut switch's off value proved on the tree at `SeMultiCut` 0 and **the extension's own absence of one stated rather than fudged** -- section 4's "`SeMinDepth` at its range top is the off switch" is wrong and is corrected beside it (DEC-215 clause 3) -- the `{0, 5}` nElo pair the accepts names and what it costs (41861 and 25591 games, 19.8 h and 12.1 h at 2110), the node-explosion check taken before the games, the regime, the abort rule, the three open test-side findings it is taken while open, and the three outcomes, including **what to do with the multicut if the extension reads H0** (drop both, with the reason written before the number). It refuses to run until both shas are pinned (DEC-020) |
| `S097_v1_sprt.log` | S097 verdict 1's gainer SPRT, 2026-09-21 04:51:09 to 14:18:33, 9 h 27 m 05 s: the banner and both identity lines (`cand-88ec74f  Chesso 88ec74f native`, `ref-5c76ea9  Chesso 5c76ea9 native`), seed `20260921045106`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H0 accepted**, LLR -2.96 against (-2.94, 2.94), `Elo -0.81 +/- 3.70`, `nElo -1.06 +/- 4.81`, W 6123 L 6170 D 7787 over 20080 games, `Ptnml(0-2) [925, 2332, 3507, 2417, 859]`, LOS 33.32 %. **0 time forfeits on either side** over the PGN's 20082 games (13969 adjudications, 6113 natural ends); `Incomplete mating PV` 14 candidate against 9 reference. A zero and not a loss -- the nElo interval [-5.87, +3.75] is centred within one nElo of zero -- which is why DEC-227 keeps the block as verdict 2's carrier instead of flipping `SeExtend`. The verdict-closing commit carries DEC-220's block, checked by `tools/gate.sh` against this file |
| `S097_v1_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-21: markers, banner, the fastchess final block, timing, terminations, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 10040 complete pairs, pair score mean 1.0156 and **variance 0.3160**, sd 0.5622, the five pair-score buckets 9.4 / 23.0 / 32.9 / 24.3 / 10.4 %, white winning both of 947 pairs (9.4 %), 117.2 plies and 19.8 s a game |
| `S097_v2_sprt.sh` | the pre-registration of S097's **second** verdict, the multicut, written 2026-09-20 against the first verdict's landing and **amended 2026-09-21 under DEC-227, before a game was played**: the candidate is one default, `SeMultiCut` 0 to 1, plus the coverage that default makes possible -- the rule's guard case, the accepts' mined mate row and the mutants E20 to E23 -- and the file says why none of them could land with verdict 1, the switch being a compiled constant the release build folds away. It carries what this pair gets for free (the reference **is** the tree at the off value), the direction a pruning rule's bench should move and what an unmoved one would mean (DEC-212, DEC-214), the same `{0, 5}` cost, and an interpretation written in advance for the reading the record predicts here: **a stalled walk near +3 is terminated, recorded as a zero and read as H0** (DEC-063), because the traced +5.7 and +6.2 are from engines three hundred points above this one and one engine dropped the rule at this band. **What the amendment changes** is verdict 1's own number and its reading (H0, `nElo -1.06 +/- 4.81`, a walk and not a loss), the H0 and no-verdict outcomes -- the old text's "drop the multicut and keep the extension" is wrong now that the extension is itself a measured zero, so both readings remove **the whole block** in one revert to `5c76ea9`'s `src/`, proved by `bench` 4579468 and `search_bench` identity at both depths and not argued -- the reference being the tree the flip lands on rather than `88ec74f`, the numbers being re-taken after the rebase onto S132, and the three open test-side findings named by id and re-checked against the tree |
| `S097_v2_sprt.log` | S097 verdict 2's gainer SPRT, 2026-09-21 20:48:32 to 2026-09-22 01:46:50, 4 h 57 m 44 s: the banner and both identity lines (`cand-63ad848  Chesso 63ad848 native`, `ref-f02f59a  Chesso f02f59a native`), seed `20260921204832`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H1 accepted**, LLR 2.95 against (-2.94, 2.94), `Elo 7.17 +/- 5.13`, `nElo 9.31 +/- 6.66`, W 3333 L 3117 D 4020 over 10470 games, `Ptnml(0-2) [444, 1176, 1829, 1292, 494]`, LOS 99.69 %. **0 time forfeits on either side** over the PGN's 10470 games (7309 adjudications, 3161 natural ends); `Incomplete mating PV` 6 candidate against 8 reference. The nElo interval reaches inside the pair, DEC-223's slow class; the walk dipped to +3.95 Elo at 8453 games before reaching the bound. The verdict-closing commit carries DEC-220's block, checked by `tools/gate.sh` against this file |
| `S097_v2_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-22: markers, banner, the fastchess final block, timing, terminations, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 5235 complete pairs, pair score mean 1.0078 and **variance 0.3189**, sd 0.5648, the five pair-score buckets 9.9 / 23.2 / 32.7 / 24.0 / 10.3 %, white winning both of 516 pairs (9.9 %), 117.6 plies and 20.0 s a game |
| `S097_mine_mate_row.py` | S097 verdict 2's mining run, 2026-09-21, and **it runs**: six stages, written out in its own header in the order they run and with every input named, that pick the mate row `tests/test_search.cpp` "pruning does not hide a forced mate" gains for the multicut -- one the rule hides when its **mate-range guard** is dropped (`E21_multicut_mate_band_gate_dropped`) and finds with the guard in place. It **imports** `S230_mine_r01_row` rather than copying it (this directory is append-only; S095 importing S230 is the precedent) and reuses its pool, its node-limited shortlist and its fresh-process oracle call. What is S097's own is the rest, because the multicut needs two things S230's sweep does not measure. **The swept range is not 3 to 12**: the block wants `ply > 0` and a remaining depth of at least `SeMinDepth`, and a `search_fen()` root is at ply 0, so the shallowest fixed depth at which any node can multicut is `SeMinDepth` + 1 -- read out of `src/search_params.hpp`'s X-macro by the script rather than written down, so a refit moves the sweep with it. **And the rule has to be shown to fire on the position by the engine and not by an argument** (CHESS): `fires` drives the **tune** library, where the switch is a variable, and sweeps each position twice, at `SeMultiCut` 0 and at 1; a cell whose node count or mate reading moves is the rule reaching that position -- the wall time is dropped from that comparison, because two runs of the same binary differ there and the first version of the line reported every position as changed. It is run over `separators`' output rather than over all 269, because the witness costs twice what one sweep does; `separators` derives that set from the two sweeps, so no stage of the method is a hand-written list. `sweep`'s driver is `search_fen()` of the search suite line for line, like S230's, and prints the node count and the milliseconds of each cell as well as the distance -- a row is read at one depth by a case the gate runs on every commit, so what it costs is part of choosing it. `pick` is the re-derivation DEC-142 wants beside the row -- and its rule is the header's to the letter, **the mate lost and not merely moved**: of the 269 candidates three separate the guard anywhere in the range and only one loses the mate, the other two reporting #7 as #6 and #6 as #7, which the rule prints as its own class and never takes. It is: the lowest depth the shipped build reports the mate at and the guard-dropped build does not, ties broken by the longest run of consecutive shipped depths and then by the cheaper cell, stated in the header before the sweep runs (DEC-209 clause 4) |
| `S097_candidates.tsv` | the **269** survivors of that filter, with the oracle's own distance, node count and principal variation in UCI and SAN, 2026-09-21 — every labelled mate of S230's pool that stockfish at depth 20 in a fresh process still calls a forced mate in 2 to 6 for the side to move. Every position is this project's own (DEC-016), through S230's pool -- the two committed S145 mate sets and the last 40 plies of every game of `S219_aa_calibration.pgn` -- and stockfish is run as a binary to label them, nothing copied from it. The row the case takes is one line of this file and the sweeps it was taken from are in `.tuning/coord/` |
| `S097_fixed_node_depth.py` | S097's own instrument, and the one thing neither `bench` nor `tools/search_bench.py` can see: **the depth the search reaches at a fixed node budget**, `go nodes 1000000` over `search_bench`'s three positions, one row of `position depth nodes best score` each, the depth being the last **completed** iteration's as the engine's own `info` line reports it (this engine prints no seldepth and the instrument has no such column). An extension trades nodes for depth by construction, so a total that moves in either direction says nothing about whether the trade is a good one; a depth that falls on every position with no rating to show for it is the explosion signature the step's section 6 asks to be checked **before** a night is spent on a match. Reads the engine's own last `info` line over UCI, takes `--nodes`, `--hash` and repeatable `--option NAME=VALUE` for the tune build's switches, and is a measurement and not a verdict (DEC-019) |
| `S188_sprt.sh` | the pre-registration of S188's one verdict, the check extension, written 2026-09-22 before a game was played and **corrected the same day to the form the script actually books** -- this row described the first form until then. The candidate is one rule in `negamax_at`'s move loop: a move that gives check is searched one ply deeper **provided static exchange at threshold zero holds for the move on its destination square** (DEC-228's safe-check gate) and provided the node is not the root, its ply has not reached `CheckExtPlyFactor * depth` and its remaining depth is at most `CheckExtMaxDepth` -- the last that many plies before the horizon. Captures and quiets alike, sharing S097's one-ply-per-node budget, behind `CheckExtend`, whose off value is proved on the tree with the full bench signature. It carries DEC-222's prior, the `{0, 5}` pair DEC-133 fixed and why `{-5, 0}` was refused, DEC-143's 41861 / 25591 games at 19.8 h / 12.1 h, the abort rule read per side with `tools/forfeit_report.py`, the open findings by id, and **the bar DEC-228 booked the run against with all three forms' readings beside it**: `bench` +94.6 % / +72.4 % / **+28.1 %** and the fixed-node depths 15/11/14 / 17/12/15 / **16/13/15** against the parent's 17/13/15, the third meeting every clause. Its H0 path removes the rule in the same step, checkable before it is committed because the switch's off value is the parent's tree to the node |
| `S188_remine.log` | the re-mine of `tests/test_search.cpp` "pruning does not hide a forced mate"'s multicut row at S188, 2026-09-22, after the step's 34-mutant pass scored `E21_multicut_mate_band_gate_dropped` a **survivor** on this tree -- `fast 0/40`, a green suite, bench moved. DEC-142's rule is that a mined row is re-derived by its script whenever either end moves, and the extension moved the search. Carries where the old row went (its guard-dropped profile `d12` -> `d12 d14`, the shipped one unmoved, the rule still reaching the position), the two separators of the 269 with the one the firing witness **skipped** because the multicut never reaches it, the pick's own output, the witness table, the oracle (stockfish in a fresh process and the pool's own label, both mate in 5) and the red observed by hand under E21 and reverted. Stage 1 was skipped because `S097_candidates.tsv` is committed; 73 minutes a sweep |
| `S188_mutation_pass.tsv` | the 34-mutant pass of the form that landed, 2026-09-22, `tools/mutation_check.py` over the fixture `504a43b` with a green baseline of 40 tests: X01 to X12 and every E mutant re-proved, because the release build compiles a different `child_depth` than it did at S097's landing and E01's anchor was re-pointed. **31 killed of 32 scored (97 %)**, wall 9488 s, one equivalent and one unmeasured. The equivalent is `X07_budget_is_a_sum` with `bench same`, declared for an arithmetic reason -- `SeMinDepth` 10 and `CheckExtMaxDepth` 8 leave no node inside both extension rules, so a sum and a `?:` compile to the same tree -- and the tool's second oracle is what corroborates it. The unmeasured is `X10_extends_two_plies`, whose four failing rows are **all Timeouts**: two plies a check blows the tree up and four tests hit their ceilings, which DEC-165 will not let the tool score as a kill, though the same mutant was observed red by assertion under the step's first form. The survivor is `E21_multicut_mate_band_gate_dropped` and `S188_remine.log` is what answers it |
| `S188_mutation_targeted.tsv` | the targeted re-runs, 2026-09-22, on the refreshed fixtures `a6e0475` and `799c42d`: **6 of 6 killed**, every one of them a mutant a **mined row** is supposed to take, which is the set worth re-proving on a tree whose search reaches further. `E21` is killed again, by the re-mined row. `R01_extra_reduction_gives_check` is S091's capture exemption, re-proved because S188's shared capture scan rewrites the expression it reaches through, and its killer is S091's own case; `R02_extra_reduction_sign` is the one mutant `capture_mates`' four mined depths pin, and "pruning does not hide a forced mate" is among what takes it, so those rows still work. `J01`, `J02` and `J03` are S095's, run to ask whether that step's mined row still does its job on a deeper tree: it takes J01 and J02 as it always did and **no longer takes J03**, which its own direct case kills -- no coverage lost, and the sentence in `tests/test_search.cpp` that claimed otherwise is corrected against this file |
| `S188_form1_reds.log` | the ten mutants of S188's **first** form observed red by hand, 2026-09-22, each applied to `src/search.cpp`, built, run against the six release cases of the block and reverted -- the S033 protocol, taken because `tools/mutation_check.py` refuses a baseline that is not green and that form's tree was not (four pre-existing cases had moved under it). Ten of ten red, with the failing line and values of each. The form that landed is the third and its pass is `S188_mutation_pass.tsv`, on a green baseline and with the E mutants re-proved beside the X ones |
| `S188_cap_sweep.log` | **form 1's** ply-factor sweep -- what `CheckExtPlyFactor` does to the tree, swept over its whole range 1 to 7 on the tune build, 2026-09-22: `bench` and `adocs/data/S097_fixed_node_depth.py` at each value beside the rule switched off. The cap is **not** what the rule costs -- 6883468 nodes at 1 against 8744373 at the shipped 4 and 4493659 with the rule off, fixed-node totals 43 / 44 / 43 / 40 / 41 / 42 / 42 against 45 off -- and the seed, the range's own midpoint under DEC-105 (c), sits at the worst cell of both. A measurement and not a verdict (DEC-019): S127 is where the axis is fitted, and S021 is the precedent for a sweep that chooses and an SPRT that measures |
| `S188_cap_sweep_gated.log` | the same sweep for **form 2**, the safe-check gate DEC-228 decided: `bench` and the fixed-node depths at `CheckExtPlyFactor` 1 to 7 beside the rule switched off, 2026-09-22. Under the gate the factor **is** load-bearing on cost where it was not without it -- +23.8 % at 1 against +72.4 % at the seed -- and only the floor is inside DEC-228's bench bar, which is recorded rather than used to re-seed the axis (DEC-105 (c), DEC-019) |
| `S188_horizon_sweep.log` | **form 3's** own axis, `CheckExtMaxDepth` 1 to 16 with the ply factor at its seed, 2026-09-22: `bench` and the fixed-node depths per value. At 12 and above the restriction stops binding and the totals are form 2's to the node, which is the ceiling's purpose statement measured; at the floor the tree is **smaller** at a fixed depth and **deeper** at a fixed budget than the parent (4282248 nodes and a fixed-node total of 46 against 4493659 and 45). A reading and not a verdict (DEC-019) |
| `S188_repair_goldens.py` | the re-derivation of the two measured constants S188's extension moved, and **it runs** (DEC-142, DEC-228): `s207` compiles `search_after()` of `tests/test_search.cpp` "search() hands the rule the root's own index" against the engine library as it stands -- `--lib` chooses the tree -- sweeps depths 2 to 8, prints the three scores per row and picks the separating depth by the case's own rule, which reads 3 here and 4 on the parent commit; `first-mate` drives `deepen()`'s own `go depth 10` over `tests/test_engine.cpp`'s two positions and prints every iteration's kind and value, the depth a mate first appears at, the precondition below it and the set of distances from there on, which reads 8 and 5 on both where the case held 9 and 5. Neither constant had a re-derivation before -- the second's own comment said it was "found by measurement" against a reference binary -- and that absence was the DEC-142 finding this closes. Not Elo and not a verdict (DEC-019) |
| `S132_node_share_census.py` | S132's census, **P3 and (b) under DEC-105**, and **it runs**: the share of the root's own nodes chesso's best move takes, over the same 300 stratified positions `src/search_params.hpp`'s aspiration rows were chosen on, at `go depth 12` on the release build. It **executes S021's own picker** rather than copying it -- that file reads argv and runs a whole sweep at import, so its source is cut at the line where the sweep begins, which is this directory's append-only rule honoured in the only way that file allows. Drives `build/tools/node_share_census`, prints the distribution (median, quartiles, deciles, the count at 100 %) and then the pair the step's two constraints give at that median -- `TmNodeScalePct = 7000 / (100 - 100 f_med)` and `TmNodeBasePct = 100 + 3000 / TmNodeScalePct`, both re-derived from `TmScaleMinPercent` as the file reads it rather than from the constants written out -- and **says so loudly when the pair falls outside the ranges the step declared**, which a median above about 0.77 would do. A distribution is not Elo (DEC-019): what it decides is where a sweep starts |
| `S132_node_share_census.tsv` | what that run produced: one row per position, `fen depth best total_nodes root_nodes best_nodes share_pct`, on the release build with the counting half of S132 in it. The evidence behind the two seeds `src/search_params.hpp` now carries, and the thing to re-take when either the search or the pick moves |
| `S132_sprt.sh` | the pre-registration of S132's one verdict, the node-fraction time manager, written before a game was played and before the landing commit existed: what the candidate ships (per-root-move node buckets keyed on the whole move encoding, the factor, the depth gate and the floor moved onto the product of the three scalers), why the counting half is **not** in the games (behaviour-neutral, DEC-083, discharged by identical `bench` and `search_bench` signatures and priced by an interleaved timing), the off value proved on the tree at `TmNodeScalePct` 0 rather than declared from the range's end (DEC-215), the `{0, 5}` nElo pair and what it costs (41861 and 25591 games, 19.8 h and 12.1 h at 2110), the regime with **the step file's older "UHO book" sentence corrected against DEC-189** rather than silently followed, an abort rule whose time-forfeit clause is read per side from the run's own PGN (the S089 lesson, and this is a time-management change), the open findings it is taken while open, and the three outcomes -- H1 keeps the census seeds and hands the owner the second-control question S085's +23.8/-22.9 record makes necessary, H0 and a stalled walk both flip `TmNodeScalePct` to 0 as a one-default revert and keep the counting. It refuses to run until both shas are pinned (DEC-020) |
| `S132_sprt.log` | S132's gainer SPRT, 2026-09-21 17:24:43 to 19:13:32, 1 h 48 m 24 s: the banner and both identity lines (`cand-474c288  Chesso 474c288 native`, `ref-778c7b0  Chesso 778c7b0 native`), seed `20260921172443`, `8+0.08` / Hash 16 / concurrency 12 on `noob_3moves.epd`, and fastchess's final block -- **H1 accepted**, LLR 2.95 against (-2.94, 2.94), `Elo 16.28 +/- 8.47`, `nElo 21.21 +/- 11.01`, W 1239 L 1060 D 1523 over 3822 games, `Ptnml(0-2) [149, 406, 664, 501, 191]`, LOS 99.99 %. **0 time forfeits on either side** over the PGN's 3825 games (2603 adjudications, 1222 natural ends), read per side because this is a clock change; `Incomplete mating PV` 2 candidate against 11 reference. The nElo interval misses the bounds pair on the high side, DEC-223's fast class. The verdict-closing commit carries DEC-220's block, checked by `tools/gate.sh` against this file |
| `S132_sprt_pairs.txt` | that run read in the pre-registration's own order, 2026-09-21: markers, banner, the fastchess final block, timing, terminations, the per-side forfeit report and the `Incomplete mating PV` split, then `adocs/data/S105_pairs.py` over the run's PGN -- 1911 complete pairs, pair score mean 1.0154 and **variance 0.3047**, sd 0.5520, the five pair-score buckets 8.7 / 23.3 / 33.9 / 24.1 / 9.9 %, white winning both of 167 pairs (8.7 %), 117.1 plies and 19.9 s a game |
| `S132_confirm_sprt.sh` | the pre-registration of S132's **second-control confirmation**, DEC-229, written 2026-09-22 before a game was played: the same pair of shas as the first verdict -- `474c288` against `778c7b0`, the only pair that isolates S132's own change -- at DEC-202's longer control, `32+0.32` with `Hash=64` and everything else the playing control's, because the only thing a confirmation may change is the clock. It carries the first verdict's block quoted as what it is and not as an effect size (DEC-063), why this control rather than the 40+0.4 the step file named (this project has a measured throughput at 32+0.32 and none at 40+0.4), the pair's cost at 534 games an hour from the nElo formula re-derived against `adocs/testing_strategy.md`'s own two published figures -- 25591 games and 47.9 h on a bound, 41861 and 78.4 h at the midpoint, 4062 and 7.6 h at a truth of 20 nElo -- and **what the 6400-game cap can and cannot answer**: about 13.6 nElo or more is expected to reach H1 inside it and anything weaker reaches the cap instead, which is why two of the three readings are about the interval at the cap. **It also records a finding about the harness**: `ROUNDS=<n>` empties `fastchess.sh`'s `sprt_args`, so a capped SPRT is not a shape the script can express, and the cap is therefore operational -- the watcher stops the run at 6400 games with a 14 h wall ceiling, and `SPRT-RUN-FAILED: exited 143` is that stop and not a crash (the S068 run-1 precedent). The abort rule reads the time-forfeit count per side from the PGN and says what a forfeit would mean at a four-times-longer control, where the move overhead has proportionally more room and the first verdict ran 0 in 3822 games: a **one-sided** rate would be the multiplier reaching into the overhead. The three readings are DEC-229's own, reproduced without amendment. It refuses to run until both shas are pinned (DEC-020) |

### The bounds every `*_sprt.sh` here pre-registers are nElo (S157, 2026-08-30)

Appended rather than applied: the pre-registration scripts in this directory are
evidence of runs already taken and are never edited, so the sentences inside
them stand as written. What they say is `elo0=-5 elo1=0` and then read the
outcome as "not a regression of 5 **Elo** or more" -- `S021_sprt.sh:20`,
`S076_sprt.sh:28`, `S075_sprt.sh:27`, `S108_sprt.sh:67`, `S149_sprt.sh:47`,
`S165_sprt.sh:41`, and on the gainer side `S093_sprt_v1.sh:57`,
`S093_sprt_v2.sh:52`, `S024_sprt.sh:71`, `S130_sprt.sh:106`.

`fastchess.sh` passes `model=normalized`, whose `--help` gloss is "normalized --
Uses nElo (default)", so **the bound is in normalized Elo and the logistic
figure is smaller**. How much smaller is a property of the draw rate and is read
off the run's own printed `Elo` / `nElo` pair, never from a constant: 5 nElo was
3.54 logistic Elo at S165's 44.75 % draws and 3.87 to 3.99 at the lower draw
rates of S076, S107 and S021.

**No verdict is invalidated, and the two sides are wrong in opposite
directions**, which is worth stating because the audit did not. `elo0` is
scale-invariant at 0, so nothing hinges on the scale where a bound is zero. On
the **non-regression** side the test rejected "effect at or below -5 nElo", so
what it actually excluded is a regression of about 3.5 to 4.0 *logistic* Elo --
which implies the recorded "5 Elo or more is excluded" and then some. Those
sentences are conservative: true as written, understating what was shown. On the
**gainer** side the asymmetry runs the other way: accepting H1 at `elo1=5`
establishes a gain of 5 nElo, about 3.9 logistic Elo, so "gains 5 Elo or more"
claims about 1.1 logistic Elo more than the run demonstrated. That is the half
worth restating wherever it is quoted as current, and `specs.md`'s copy of it
now is. `DEV_MANUAL.md`'s "Which bounds" section states the scale where the next
pre-registration will be written from. 2026-08-21 adversarial F09.
