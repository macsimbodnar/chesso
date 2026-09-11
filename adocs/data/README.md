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
| `S159_census.md` | the killer-table census write-up, S159, 2026-09-09, Release, `g++ 13.3`, the i7-8700K workstation: why the driver set is new, the reproduction recipe (`git archive 99000c1`, the patch, the driver) verified end to end that day, what was counted and where each counter sits, the two-build table and its reading. The evidence DEC-160 closes S159 on -- the ageing reading of S149's 11 Elo does not survive its own census, so no SPRT was spent on it -- and it states itself that it is a prior and not a verdict (DEC-019) |
| `S159_census_positions.txt` | the eleven positions every S159 figure is measured on, one per line as `depth<TAB>name<TAB>FEN`. It exists because S149's own driver named its 11 positions and recorded none of their FENs, in the step file or in the audit that produced it, so S149's 66.0 % / 44.4 % cannot be reproduced exactly; every row here comes from one source the repository does record. Kept unedited as the input the recorded census was taken on -- `S216_census_positions.txt` is what a run today uses, because the `promo-mess` row is the pre-DEC-177 `KILLER_POS` and a post-S208 engine refuses it |
| `S159_census_run.py` | the driver that took that census: one engine process, `Hash 64`, each position searched to its own depth, and each `go` waited on to `bestmove` before the next command is written -- piping the whole script with a trailing `quit` kills every search before it looks at a node and the census then reads `stores=0` over the entire set, TOOLCHAIN.md's "the one way to ask it that lies" in another costume. Replaced by `S216_census_run.py`, which sees a refused row instead of inheriting the board above it; kept as the reader the recorded numbers came from |
| `S159_census_instrument.patch` | the instrumentation, over three files: eight counters in `src/search.cpp`, a per-slot iteration stamp and an iteration counter in `src/data_structures.hpp`, and one `S159CENSUS` line to stderr at the end of the search in `src/chesso.cpp`. It is applied to a scratch copy of `99000c1` and **never to the repository tree**, which is what makes the census reproducible without a measurement patch living in `src/` |
| `S159_census_head.txt` | that census at S159's HEAD: per-position nodes and best move for all eleven rows, the total, and the `S159CENSUS` counter line. The left column of `S159_census.md`'s table, and the file the reproduction check is compared against |
| `S159_census_guard.txt` | the same census with S149's reverted guard re-applied on the instrumented copy only, which is the build the ageing hypothesis says the staleness lives in. The right column of that table, and the run that shows the guard removes the duplication completely |
| `S216_census_run.py` | the killer-census driver at S216, 2026-09-11, replacing `S159_census_run.py` and leaving it in place. S208's 16-a-side load bound refuses the `promo-mess` row of `S159_census_positions.txt` -- the pre-DEC-177 `KILLER_POS`, 17 white pieces -- and S159's reader scraped only lines containing ` nodes `, so the `info string refused [position fen]` line was skipped, `set_position`'s `game = previous` left **the row above** on the board, and the `go` that followed printed its numbers under the refused row's name. Observed red first at depth 2 on HEAD: `promo-mess` reported **452 nodes, best `d2d4`** -- startpos's, and a move the promo-mess board cannot play. The new driver reads an `isready` between `position fen` and `go`, so a refusal is seen before a search is started; the row is printed and written as `REFUSED`, never searched, and the script exits 1. On an all-legal input its output is byte-identical to S159's, checked over three rows |
| `S216_census_positions.txt` | the census input from here on, 2026-09-11: S159's eleven rows in S159's order with the sixth replaced, and every other row byte-identical to `S159_census_positions.txt`, which stays beside it unedited. That sixth row held the pre-DEC-177 `KILLER_POS` at 17 white pieces, which S208's load bound refuses; it carries DEC-177's legal constant instead -- 16 white, 14 black, one white pawn fewer and the same board otherwise, so the twelve promotions and the `f5e6` en-passant capture the row is in the set for are kept. **The name changes with the board**, to `promo-mess-s208`, so no future table's row is read against the 142852 nodes recorded for the old one. DEC-186, which also decides the recorded census is not re-derived |

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
