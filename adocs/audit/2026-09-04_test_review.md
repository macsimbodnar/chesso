# Audit 2026-09-04 test_review

Commit audited: `5cffb70` (`achesso`, "Add the 2026-09-04 plan review and plan
its ten findings", 2026-09-04), clean working tree. Requested by the owner:
assess the current tests for effectiveness, robustness and usefulness, as the
evidence half of a research pass on how this engine decides whether a change
is an improvement. The literature half and the recommendations are
`adocs/testing_strategy.md`; this file holds what was measured and what is
wrong.

Type: `test_review`, whole test surface. Scope: every registered test under
`tests/` (27 `fast` targets, one `slow`, two `bench`), the unregistered red test
`tests/test_audit_go_infinite.cpp`, the shell and python tests, the benchmarks,
`tools/search_bench.py` as the INV-6 instrument, `fastchess.sh` and `rating.sh`
as the match instruments, and the runtime self-checks in `src/`. Engine source
read where a test's claim had to be traced. Not in scope: the SPRT regime's
statistical design, which the strategy document takes up; no change to any
bound or to the harness is proposed here.

**This is not a clean-context adversarial run.** Like `2026-08-14_test_review`
it ran in a session that had read `status.md`, the plan and the decisions, at
the owner's request. Three read-only subagents did the file-by-file reading
(search and engine tests; movegen, evaluation and tool tests; the engine's
feature and self-check inventory), and their per-case gradings were checked
against the code where they are cited below. What is new against the
2026-08-14 review is that the suite's fault-detection ability was **measured**,
not read: coverage of the fast label, and a fault-injection pass of 33
hand-written engine bugs.

Method. What was actually run, all on the MacBook `.moltke.local.md` describes,
on battery, no match started (POWER rule):

- A detached git worktree of `5cffb70` in the scratch directory, configured
  `Release` with Ninja and ccache, the `doctest` and `json` submodule checkouts
  copied in (a worktree does not carry them). `ctest -L fast`: **27 of 27,
  63.9 s**. `tools/search_bench.py` at depth 9: **121512 / 800769 / 62907,
  `c3d5` / `e2a6` / `d7c8q`**, the recorded baseline.
- **Coverage.** The same tree configured `Release` with
  `-fprofile-instr-generate -fcoverage-mapping` (Apple clang 16 via
  `/usr/bin/c++`, `-O3 -DNDEBUG -march=native` as the gate builds), the fast
  label run once under `LLVM_PROFILE_FILE` (27 of 27, 83.6 s), 27 profiles
  merged with `xcrun llvm-profdata`, reported with `xcrun llvm-cov` over the
  24 test and tool binaries plus `chesso`, filtered to `src/`.
- **Fault injection.** 33 mutants of `src/search.cpp`, `src/bitboard.cpp`,
  `src/evaluation.cpp` and `src/chesso.cpp`, each one plausible engine bug --
  a guard dropped, a sign flipped, an off-by-one -- applied one at a time to
  the worktree, rebuilt, the fast label run in full, `search_bench.py` at depth
  9 run, then reverted. The mutant list, the driver and the per-mutant results
  are tracked under `adocs/data/2026-09-04_test_review/`; the driver refuses
  any mutant whose anchor text is not unique in its file. Two mutants did not
  compile under `-Werror` (an unused variable each) and were re-run with the
  variable consumed, so all 33 were measured. 35 full runs of the fast label
  in all.
- Read, not run: `fastchess.sh`, `rating.sh`, the S105 calibration scripts and
  data, the WATCHERS and POWER rules as they bear on the instruments.

Nothing in the repository was modified by the run itself except this report
and the evidence directory named above. The scratch worktree is removed.
Digested 2026-09-05: DEC-139 to DEC-143, steps S189 to S199; the statuses below
are as of that digestion.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-09-04_test_review-F<nn>`.

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Verdict

**No high finding. Three medium, seven low.** The suite is effective at what
it reaches: of 33 injected bugs **31 were caught**, one survived (F04, the
fifty-move boundary) and one is an equivalent mutant that changes no
behaviour. Coverage of the fast label is 98.9 % of the lines and
92.0 % of the branches in `src/search.cpp`, 100 % of `src/evaluation.cpp`,
92.2 % of `src/bitboard.cpp`; the uncovered remainder is defensive clamps,
allocation-failure paths, the UCI book probe (F06) and the mate-PV repair
walks. Across 35 consecutive full runs of the fast label **not one failure was
unexplained by the mutant under test**, and the wall time stayed between
40.7 s and 63.9 s (one 97.3 s run, under a mutant that made every search
slower), so the label is not flaky on this machine at these ceilings.

What the measurement also showed is *where* the detection comes from, and that
is the substance of the three mediums. Two of the six invariants are enforced
by assertions that the gate compiles out (F01). The null-move and reduction
guards have no test of their own: five guard-removal mutants were caught by
one test and by nothing else, and that test is a count floor over four games
(F02). And a large share of the suite's sensitivity comes from golden numbers
-- static scores, node floors, mate-line counts, a soft-limit scaling on a
fixed position -- that any legitimate search or evaluation change will also
redden, which puts the re-derivation cost on every step and the pressure on
the floor (F03). Twelve of the 33 mutants left the depth-9 node counts and
best moves untouched, so the INV-6 bench identity alone would have passed
them: the suite and the signature see different things and both are needed.

## Findings

### 2026-09-04_test_review-F01  medium  INV-2 and INV-4 are enforced only by assertions the gate compiles out, and nothing in Release checks the accumulators after a move

Status: planned — S190

Evidence: the gate is `cmake --build build -j8 && ctest --test-dir build -L fast
... && cmake --build build-tune -j8 && ctest --test-dir build-tune -L fast`
(`AGENTS.md` TESTS rule); both directories are `CMAKE_BUILD_TYPE=Release`
(`build/CMakeCache.txt`, `build-tune/CMakeCache.txt`), so every `assert(` in
`src/` is dead in both. INV-4's only enforcement is
`assert(eval_accumulators_match(...))` at `src/bitboard.cpp:742`, `:875` and
`:997`, inside `#ifndef NDEBUG` (`:591-626`); INV-2's second half
(`squares[]` against the bitboards) is the same shape. No test in `tests/`
calls `eval_refresh` after a `make_move` sequence (grep: zero hits); colour
symmetry (`tests/test_evaluation.cpp:43`) reaches the accumulators only through
`load_FEN`, which rebuilds them from scratch (`src/bitboard.cpp:1907`). By
contrast the hash *does* have a Release oracle -- `tests/test_engine.cpp:56`
compares against `compute_full_hash` after every make at depth 3-4 over five
positions -- and it is what caught the en-passant hash mutant (M23) here. PV
legality in `search()` is a `LOG_E`, not an assert
(`src/search.cpp:1502-1504`), and `LOG_E` is `if (false)` under `NDEBUG`
(`src/log.hpp:26-42`). `DEV_MANUAL.md` says so: "It is **not** in the gate
above and running it is still on you."

Impact: a change to `make_move`, `unmake_move`, `add_piece`/`remove_piece`/
`move_piece` or an evaluation term's accumulator can ship with the accumulators
drifting from the recomputation, and the gate is green; the symptom is a wrong
static score everywhere, which no fast test pins and an SPRT reads as a
strength change. This is the hazard `CLAUDE.md` names for the NNUE hook: the
accumulator alphabet is exactly what an accumulator network is updated from.
`2026-08-14_test_review-F03` fixed the Debug timeout so the invariants *can* be
run; it did not put them in anything that runs.

Suggested resolution: a Release test with the shape of `test_engine.cpp:56` --
walk the corpus at depth 3-4, compare `material`, `psqt_mg`, `psqt_eg`,
`phase` and `squares[]` against a full rebuild after every make and unmake --
which is cheap and reaches both invariants in the gate; and, separately, a rule
that a step touching the make/unmake path runs the six Debug binaries and says
so in its stamp. Not applied here.

### 2026-09-04_test_review-F02  medium  the null-move and reduction guards have no direct test; five guard-removal bugs were caught by one count floor and by nothing else

Status: planned — S191

Evidence: no `TEST_CASE` exercises the null-move guards at
`src/search.cpp:822-824` -- in check, `prev_move != 0` (two passes in a row),
`game_phase > 0` (zugzwang), the mate band at either edge -- or the reduction
guards at `:960-961` (captures, promotions, checks, in check, move number,
depth) or the re-search at `:979`. The mutation pass makes this concrete. M01
(null move allowed in check), M02 (S165's negative mate-band guard dropped),
M03 (null move in pawn endings), M04 (a null-move mate score returned as real)
and M07 (LMR reduces captures) were each caught by **exactly one test**,
`tests/test_mate_carry.cpp:272`, through
`CHECK(mate_lines >= expected_mate_lines(game.name))` or `failures.empty()`,
and by no other case in the 27 binaries. M08 (LMR reduces checking moves, the
S107 exemption) was caught by three binaries, none of them a guard test. `adocs/data/S165_defender_set.tsv`
-- 104 proved defender nodes, built to measure exactly the M02 guard -- is
read by nothing in `tests/`. M02, M03 and M04 also leave the depth-9 node
counts and best moves of `search_bench.py` identical, so INV-6 would pass
them too.

Impact: the guards protect against the recurring bug class `CLAUDE.md` names
("pruning that hides a mate"), and their whole test coverage is a side effect
of a count over four replayed games at fixed node budgets. A guard bug whose
effect does not move those four counts passes the gate; and when a legitimate
search change moves them (F03), the floors are re-derived and the incidental
coverage is re-derived with them, invisibly.

Suggested resolution: one direct case per guard, each with a precondition that
the guard's condition holds at the node under test and an assertion on what
the node returns -- an in-check node never makes a null move; a pawn-only
position is searched through; a defender node from `S165_defender_set.tsv`
inside the band gets no null-move cutoff; a checking move and a capture are
searched at `child_depth`. The tune build's `search_lmr_reduction_probe` and
the S145 fixture style (`tests/test_engine.cpp:2062`) are the existing
patterns. Not applied here.

### 2026-09-04_test_review-F03  medium  a large share of the suite's sensitivity is golden numbers that every legitimate search or evaluation change will also redden

Status: planned — S192

Evidence, three groups. **Static-score anchors:** 563/567 pinned at
`tests/test_search.cpp:957-958` and repeated at `:1219-1220`, `:1282`,
`:1356`, `:1437`, `:1494`, `:1713`, `:1763`; 198 at `:1017`; -505 at `:1060`;
-569 at `:2958`; the piece anchors 135/244/325/563/787 at
`tests/test_evaluation.cpp:248-252`, derived by `.tuning/anchors.py`, which is
gitignored (`status.md` parks this). The eval-sign mutant M29 turned **16
cases in five binaries** red, twelve of them on these anchors; so will the
next refit, which is the plan's S126 and S135/S136. **Tree-shape floors:**
`tests/test_mate_carry.cpp:261-265` (per-game floors 5/7/6/1/9 at fixed node
budgets), `tests/test_mate_breadth.cpp:80` (143 against a measured 147 -- the
file's own comment records the gap narrowing from 7 to 4),
`tests/test_engine.cpp:2060` (mate-in-three floor 11 against 12, DEC-116). In
the mutation pass `test_mate_carry` went red on **21 of the 22 search
mutants**; it is the suite's best detector and it is a golden. **Behaviour
pinned on a fixed position:** `tests/test_engine.cpp:1005` "the iteration loop
scales its soft limit by what the search found" asserts `scaled.drop == 0` and
`scaled.drop > 0` on fixed positions at fixed depth, and went red on **eight**
search mutants including the one-ply reverse-futility floor (M06a) and the
dropped LMR re-search (M09) -- it is testing the tree, not the scaling rule,
which `:534` already tests as a pure function. `tests/test_search.cpp:2634`
"the table never changes the answer" goes red on any TT-informed pruning; its
comment names lazy evaluation as the one exception and not the S130
substitution (`src/search.cpp:390-406`) or quiescence answering from
main-search entries (`:309-313`), both of which already break the stated
property.

Impact: the numbers are right today and they did their job in this pass. The
cost is structural: every search step in the Open list (S024, S109, S091,
S098, S095, S097 ...) and every refit will redden several of them for no
defect, each will need its own re-derivation, and a floor re-derived under
time pressure is how a gate gets weakened -- S145 recorded two surveyed
engines that disabled their mate tests rather than their pruning. DEC-116
already states the rule for one floor; the rest have no rule and no script.

Suggested resolution: name each golden as a golden in its test with the script
that re-derives it (`adocs/data/S154_floor_margin_sweep.py` is the pattern),
commit `anchors.py` or an equivalent so the piece anchors are re-derivable in
the repository, and pair every golden with a property that does not move with
the tree -- `test_engine.cpp:1005` should assert the scaling rule on a
*constructed* stability history rather than on a search, and the F02 guard
cases would give the pruning rules coverage that survives a re-derivation.
Not applied here.

### 2026-09-04_test_review-F04  low  the fifty-move boundary is not pinned: a draw claimed one halfmove late passes the suite

Status: closed — S193 (2026-09-09)

Evidence: mutant M19 changes `if (game->board.halfmove_clock >= 100)` at
`src/search.cpp:647` to `>= 101`. **27 of 27 tests pass.** The only direct
case, `tests/test_search.cpp:2977` "the fifty move rule is a draw", searches a
root whose clock is already 100 and asserts score 0; the root is exempt from
the draw test (`ply > 0`) and every node below it has clock 101 or more, so
the mutant draws them all the same. `tests/test_engine.cpp:2720` "checkmate
outranks the hundredth halfmove" asserts a mate at clock 100 and never a draw
at clock 100. Node counts and best moves at depth 9 are unchanged, so INV-6
does not see it either.

Impact: a game position at clock 100 where the side to move has a quiet
non-pawn move is scored by the search as live one ply too long; in play the
arbiter's adjudication or fastchess decides, so the strength effect is small,
but the boundary is a rule and the suite claims to hold it.

Suggested resolution: a depth-1 case with the root at clock 99 whose quiet
replies land on clock 100 and must score exactly 0, paired with the same root
at clock 98 that must not, the same shape as `:2720`. Not applied here.

### 2026-09-04_test_review-F05  low  eleven assertions in the fast suite cannot fail, and two titles claim more than their body checks

Status: closed — S193 (2026-09-09)

Evidence, each a case that passes for a reason unrelated to the property in
its title:

- `tests/test_engine.cpp:901` "a search with no limit is still bounded":
  `position startpos` at `:907` runs `stop_and_join_search()`, which sets
  `stop_search_signal` (`src/chesso.cpp:124`); the loop switches to that flag
  after depth 1 (`:902`) and breaks at `:1032`, so the 200 ms budget is never
  exercised and `elapsed < 30000` cannot fail. `:1020-1027` documents the
  hazard for its own case and this one does not apply the fix.
- `tests/test_engine.cpp:774` `!capture.contains("Found position in the opening
  book")`: the string is `LOG_I` (`src/chesso.cpp:733`), which is `std::clog`
  in Debug and `if (false)` in Release; the capture reads `std::cout`.
- `tests/test_engine.cpp:162` "an irreversible move clears the window":
  after one move from the start position the history holds one entry, so
  `is_position_repeated`'s loop (`src/bitboard.cpp:1419-1425`) never runs.
- `tests/test_search.cpp:1930` `depth >= TT_DEPTH_QS` over every entry: no
  writer stores below -1.
- `tests/test_uci_surface.cpp:384` `CHECK(expected_option_lines.size() == 5)`
  tests a literal in the test file.
- `tests/test_evaluation.cpp:300-302` `correction <= LAZY_EVAL_MARGIN` is true
  by the clamp at `src/evaluation.cpp:1045`; the comment at `:280-284` says
  so, the `REQUIRE` reads as a soundness proof.
- `tests/test_movegen.cpp:265` "a pinned piece cannot leave the ray" has no
  `count > 0` precondition and passes on an empty move list.
- `tests/test_openings.cpp:49-62` asserts `make_move` succeeds, and
  `make_move` refuses nothing but a full stack (`src/bitboard.cpp:752-755`).
- `tests/test_helpers.hpp:80-83`, the `legal_moves()` helper, says "a
  regression to pseudo-legal generation fails here": it REQUIREs `make_move`
  to succeed, which it always does. This is `2026-08-14_test_review-F04`
  returned in a new wording; legality is in fact pinned by the perft and JSON
  counts alone. `tests/test_chesso.cpp:108-122 test_generate_legal_moves`
  filters with `is_move_legal`, which is membership in the generator's own
  output (`src/bitboard.cpp:2067-2073`).
- `tests/test_perft.cpp:260,266` guard the asset load with `assert(file)` /
  `assert(false)`, compiled out in Release: a missing or unparseable asset
  iterates zero cases and exits 0 (`:380`, `:524`). Only the `nodes` column
  fails the run (`:488-490`); captures, en passant, castles and promotions are
  printed red and green and never affect the exit code. The `RUN_THREADS`
  block (`:440-464`) names `g_board`/`g_globals` and an old `make_move`
  signature and would not compile.
- Titles: `tests/test_search.cpp:284` "the node budget is honoured exactly"
  asserts `<=`; `tests/test_engine.cpp:1367` "an infinite search answers only
  once stop arrives" asserts one `bestmove` after `stop` and never "none
  before", which is the property `2026-09-04_adversarial-F01` shows the engine
  violates.

Impact: each is small; together they are a dozen places where the suite's
count of green cases overstates what it holds, and F04 is what one of them
looks like when the gap is real.

Suggested resolution: fix each in place -- a precondition, a `ucinewgame`
before the timed search, a Release-safe load check in `test_perft`, honest
titles -- and register `tests/test_audit_go_infinite.cpp` once its 200 ms
window is replaced by a condition that cannot pass on a slow machine. Not
applied here.

### 2026-09-04_test_review-F06  low  the UCI-level book path is never executed by the fast suite, and its random draw cannot be seeded

Status: planned — S194

Evidence: coverage shows `src/chesso.cpp:662-736` -- the book probe, the
weight-proportional draw and `Best Book Move` -- with zero executions across
the whole fast label. `tests/test_engine.cpp:1435` tests `validate_book_move`
on a synthetic move and never reaches the probe; `tests/test_openings.cpp`
tests the library, not the UCI path. The draw is `mt19937_64` seeded from
`std::random_device` (`src/chesso.cpp:62-63`) with no seed option. S175's
end-to-end check -- `OwnBook` on, one of the seven repaired positions answers
`bestmove d2f3` -- was done by hand and is recorded in the stamp only.

Impact: the S172/S175 defects were in exactly this path, and the regression
guard for them is a manual command.

Suggested resolution: a seed hook (a UCI option in the tune build, or an
environment variable read once at startup) and one fast case: `OwnBook true`,
`position startpos`, `go depth 1`, `bestmove` is in the set the library
returns for the start key; a second case with `Best Book Move true` pins the
heaviest entry. Not applied here.

### 2026-09-04_test_review-F07  low  the INV-6 node signature is compared by hand and recorded nowhere the gate can read; the `test` command still checks nothing

Status: planned — S189

Evidence: `tools/search_bench.py` prints node counts and best moves and
compares nothing; INV-6 is discharged by a person running it twice
(`adocs/specs.md`, INV-6 row: "no test: a procedure"). The engine has no
`bench` command (`src/chesso.cpp:172-190` dispatch table); `command_test`
(`:1690-1778`) prints the expected `bestmove`/`ponder` inside its title strings
and never compares them, prints `total_nodes` and never compares it, and
returns `true` after printing `!!!` on an illegal move (`:1762-1768`, `:1777`)
-- `2026-08-14_test_review-F07`, accepted by DEC-058 for the command; the
signature gap it named beside it was never a finding of its own. Every CI
surveyed for the strategy document checks a bench signature from the commit
message before anything else runs (Stockfish `tests/signature.sh`, Berserk,
Stash `check_bench.sh`, the fishtest and OpenBench workers). The mutation pass
measured what the signature is worth here: 21 of the 33 mutants moved the
depth-9 counts, and 12 did not -- the two instruments are complementary, and
only one of them is automatic.

Impact: a behaviour-neutral claim is checked by whoever remembers to run the
tool twice; a commit that changed the tree by accident is found by the next
SPRT, at the cost of a night, or not at all.

Suggested resolution: the strategy document's item 1 -- a `bench` command
printing one node total over a fixed position set, the total written in every
commit message that touches `src/`, and a gate step that compares the built
binary's bench to the message. Not applied here.

### 2026-09-04_test_review-F08  low  three determinism exposures are documented in one place or none

Status: planned — S195 and S179 (the key generator)

Evidence: the Zobrist keys come from
`std::uniform_int_distribution<uint64_t>` over `mt19937_64`
(`src/bitboard.cpp:2662-2663`); the distribution's output is
implementation-defined, so keys, table indices and node counts may differ
between standard libraries -- they matched between glibc and Apple libc++
when the handover check ran, which is a measurement and not a guarantee. The
LMR table's `std::log` (`src/search.cpp:102-105`) is the exposure
`status.md` already records. And `set_position` resets the table only when the
FEN string differs from the previous one (`src/chesso.cpp:410-413`), so two
`go depth N` on the same FEN in one process run the second on a warm table;
`search_bench.py` is safe because its three FENs differ, and a future
extension repeating a position would silently compare cold with warm.

Impact: a node count carried across machines or a bench extended by one
position can disagree for a reason that looks like a behaviour change.

Suggested resolution: generate the keys with the project's own integer
generator (S179 is about to do this for the magic numbers, and the same seed
discipline applies), record the warm-table rule in `DEV_MANUAL.md` beside the
bench instructions, and have the future `bench` command send `ucinewgame`
before every position. Not applied here.

### 2026-09-04_test_review-F09  low  the fast label carries wall-clock dependence, fixed temp-file names and one midnight race

Status: closed — S193 (2026-09-09)

Evidence: `tests/test_engine.cpp:940` (1 ms clock under a 3 s watchdog),
`:1326` (nine `go` variants, two of which burn the 1000 ms no-limit fallback),
`:1367` (50 ms sleep before `stop`), `tests/test_audit_go_infinite.cpp:68`
(200 ms window; passes vacuously on a slow machine, which is one reason it is
unregistered). `tests/test_fastchess_script.sh:274` compares the banner date
to `$(date +%F)` -- a run straddling midnight fails. `test_tuner_split`,
`test_tuner_gradient` and `test_openings` write fixed temp-file names
(`chesso_test_tuner_split.tsv`, `chesso_test_tuner_gradient.tsv`,
`chesso_s172_*.bin`), so two concurrent `ctest` invocations collide.
`tests/test_chesso.cpp:235` and `tests/test_openings.cpp:19` are cases that
only initialise tables the later cases depend on, and `game_tables()` asserts
readiness in Debug only (`src/bitboard.cpp:2904`). Measured against this:
35 consecutive runs, 0 unexplained failures, 40.7-63.9 s each on a machine
that was also compiling.

Impact: none observed. These are the places a failure would come from on a
loaded machine or a parallel run, and `S172`'s stamp already records one
`test_mate_breadth` timeout under Spotlight load.

Suggested resolution: `mktemp` for the fixtures, a date taken from `git show`
for the banner check, explicit fixture initialisation instead of case order,
and the DEC-025 statement that the gate is serial. Not applied here.

### 2026-09-04_test_review-F10  low  the harness has no calibration on this machine: pair variance, forfeit rate and throughput at the current regime are unmeasured here

Status: accepted — DEC-139: no calibration on the MacBook, the workstation returns soon; the run is owed there under DEC-143 and is S198's A/A

Evidence: S105's calibration -- two A/A runs of 1000 games, pair-score
variance 0.2395 +/- 0.0152, 0 forfeits, 38.7 games a minute -- was taken on
the DEC-049 Linux machine. `fastchess.sh` gained `AA=1` at S160 and no run of
it is recorded. The only throughput figure for this MacBook is 2700 games/h,
read from S024's aborted run 3 (`status.md`, handover). DEC-048 accepted that
efficiency cores "inflate variance rather than bias" and asked that "how many
more" games a verdict costs be recorded; nothing has. `tools/forfeit_report.py`
exists and has no MacBook figure to report.

Impact: every cost estimate for the pending verdicts (S182 is about to
re-derive them) rests on a games-per-hour figure from a contaminated run and
a variance figure from another machine.

Suggested resolution: one `AA=1 ./fastchess.sh` at fixed rounds -- 1000 games,
about 25 minutes at the S024 rate -- on mains, read with
`adocs/data/S105_pairs.py` and `tools/forfeit_report.py`, recorded beside
S105's numbers. A plan step, since it owns a run; the strategy document prices
it. Not applied here.

## The fault-injection pass, in full

One row per mutant. "Caught by" names the test binaries and the case that
went red; "bench" says whether `search_bench.py` at depth 9 moved a node count
(n) or a best move (b). The classes are the bug classes the project's own
history records: pruning that hides a mate, a bound type confused, an
accumulator or hash left stale, a boundary off by one.

| id | bug injected | caught by | bench |
|---|---|---|---|
| M01 | null move allowed in check | `test_mate_carry` only (mate-line floor) | n |
| M02 | S165 negative mate-band guard dropped | `test_mate_carry` only (`failures.empty()`, floor) | -- |
| M03 | null move in pawn endings (zugzwang guard) | `test_mate_carry` only | -- |
| M04 | null-move mate score returned as real | `test_mate_carry` only | -- |
| M05 | RFP margin not scaled by depth | `test_mate_breadth` floor, `test_mate_carry` | n |
| M06a | RFP ply floor one ply lower | `test_engine` soft-limit scaling, `test_mate_carry` | n |
| M06b | RFP ply floor two plies lower | `test_search` mate-in-two and both pruning-hides-mate cases, `test_engine` mate set (`first_exact == minimum`) and soft-limit, `test_mate_breadth`, `test_mate_carry` | n |
| M07 | LMR reduces captures | `test_mate_carry` only | n |
| M08 | LMR reduces checking moves (S107 exemption) | `test_engine` "a narrowed window still finds a mate", `test_mate_breadth` floor, `test_mate_carry` | n |
| M09 | reduced search beating alpha believed, no re-search | `test_engine` soft-limit scaling, `test_mate_carry` | n |
| M10 | null-window fail-high never re-searched full window | 12 cases in four binaries | n |
| M11 | TT cutoffs taken at PV nodes | `test_search` "reported line runs the full depth", `test_mate_carry` | n |
| M12 | TT entry answers regardless of stored depth | `test_search` depth-gate unit, RFP-on-stored-eval, pruning-hides-mate; `test_engine` soft-limit, narrowed-window mate, mate set (`last.value == 2`); `test_mate_breadth`; `test_mate_carry` | n |
| M13 | lower and upper bound types confused on probe | `test_search` bound-condition units x3 and "table never changes the answer", `test_mate_carry` | n |
| M14 | mate score normalised with the wrong sign | `test_search` mate-distance units x3, `test_mate_pv`, `test_mate_carry` | -- |
| M15 | second killer slot never written | `test_search` "fills the ordering tables" (`killers_1 > 0`), `test_mate_carry` | n |
| M16 | history malus applied as a bonus | `test_search` malus units x2, `test_mate_carry` | n |
| M17 | standing pat allowed while in check | 11 cases in four binaries | n |
| M18 | losing captures pruned while in check | `test_mate_pv` mined set, `test_mate_carry` | n |
| M19 | fifty-move draw one halfmove late | **survived, 27 of 27 green** (F04) | -- |
| M20 | main-search mate distance off by one ply | `test_search` mate-in-zero and distance-per-ply, `test_mate_carry` | -- |
| M21 | a lower bound lowers the stand pat (DEC-102 inverted) | `test_search` DEC-102 units x3 and "table never changes the answer", `test_mate_carry` | n |
| M22 | castling right kept when the rook is captured | `test_chesso` JSON FENs, `test_movegen` rights-lost and perft, `test_engine` full-hash oracle | n |
| M23 | en-passant square changed without the hash | `test_engine` full-hash oracle, `test_mate_carry` | n |
| M24 | halfmove clock not reset by a capture | `test_chesso` JSON FENs only | -- |
| M25 | two minors called insufficient material | `test_search` "which material can still mate" only | -- |
| M26 | repetition scan starts four plies back | **equivalent**: a two-ply repetition needs two consecutive null moves, which `prev_move != 0` forbids; no behaviour change | -- |
| M27 | king-side castling ignores an attacked f-square | `test_chesso` JSON, `test_movegen` castling and perft | n |
| M28 | `see_ge` forgets the en-passant victim | `test_search` `see_ge`-against-`see` cross-check only | -- |
| M29 | mobility and king safety sign flipped for the mover | 16 cases in five binaries, twelve on static-score anchors | n |
| M30 | MVV-LVA adds the attacker value | `test_evaluation` x2, `test_engine` soft-limit, `test_mate_carry` | n |
| M31 | lazy shortcut taken without its margin | 7 cases in four binaries | n b |
| M32 | hard limit ignores `MOVE_OVERHEAD_MS` | `test_engine` "hard limit never exceeds the clock" only | -- |
| M33 | soft limit not capped by the hard limit | same case, `soft_ms <= hard_ms` | -- |

Three readings. **The suite's kill rate is 31 of 32 non-equivalent
mutants**, which is a strong result for a suite this size, and the survivor is
a rule boundary rather than a search rule. **Single points of detection**: ten
mutants were caught by one case each (M01-M04, M07, M24, M25, M28, M32, M33);
for the five null-move and reduction guards that one case is a golden (F02,
F03). **The bench signature is blind to a third of them**: M02-M04, M14, M19,
M20, M24-M26, M28, M32, M33 leave the depth-9 counts and best moves unchanged,
so a signature check would have called each behaviour-neutral. The
complement holds too: M06a moved the counts while the constructed and the
mined mate sets stayed green, so a signature would have flagged a one-ply
floor change that two of the three mate gates would not.

## Coverage of the fast label

`xcrun llvm-cov report`, `src/` only, Release flags plus instrumentation:

| file | lines | branches | what is not executed |
|---|---|---|---|
| `search.cpp` | 98.9 % (6 of 533 missed) | 92.0 % | the `MAX_PLY` and depth-63 clamps, the allocation-failure returns, the mate-PV repair arms at `:1262-1265` and `:1350-1352`, one arm each of the mate-band comparisons |
| `evaluation.cpp` | 100 % | 100 % | -- |
| `bitboard.cpp` | 92.2 % (146 of 1881) | 90.1 % | `load_FEN` error messages, `move_to_algebraic` disambiguation, the `e1h1`-style castling normalisation (`:2526-2595`), `is_pv_legal`'s failure paths, `swap_side`/`set_en_passant` |
| `transposition_table.cpp` | 88.0 % | 89.3 % | the allocation-halving loop |
| `openings.cpp` | 87.7 % | 86.1 % | `:332-342`, `:609-625` |
| `chesso.cpp` | 77.7 % (216 of 968) | 77.9 % | the book probe and draw (`:662-736`, F06), `command_debug`, `command_register`, `ponder`/`searchmoves` handling (`:1624-1635`), the `LOG_*` bodies |
| total `src/` | 88.0 % | 87.1 % | |

Line coverage is not effectiveness -- the literature the strategy document
cites is explicit that the correlation weakens once suite size is controlled
for -- and the pass above is why this report leans on the mutants. What the
coverage does settle is that the gaps are not *reach*: apart from F06, the
unexecuted code is defensive.

## Robustness, measured

35 full runs of the fast label on the same binary tree (two baselines, 33
mutants), serial as the gate runs it, while the coverage build and
this session's other work shared the machine. Wall time 40.7 s to 63.9 s, one
97.3 s (M13, every search slower); no ceiling approached (`test_mate_breadth`
at 24.4 s instrumented against 120 s). **0 failures that the mutant under
test does not explain.** The two compile failures were `-Werror` on an unused
variable, which is the compiler doing what the mutant driver could not: a
guard whose consumer disappears does not build.

## Against the literature and other engines

The comparison table of `2026-08-14_test_review` stands and is not repeated;
three rows have moved. **Mate suites**: from six positions to 82 constructed,
318 mined and four replayed games, scored as floors (S145, S156, S168, S170),
which is ahead of the surveyed engines' `mate_in_N.epd` smoke runs and behind
Stockfish's `matetrack` only in size. **Sanitizers**: still by hand, still not
in the gate. **Bench signature and CI**: still absent (F07). What the
literature pass found that the 2026-08-14 review did not is in
`adocs/testing_strategy.md`: mutation score as the accepted measure of a
suite's fault detection, which this report applied; the reproducibility test
Stockfish runs (`tests/reprosearch.sh`) that found a table-ageing bug an SPRT
could not; and the self-play-under-assertions CI job (`games.yml`) that is the
cheap form of F01.

## Prior findings, re-assessed against this tree

### 2026-08-14_test_review (7 findings)

- F01 (a tactical case with one legal move) and F02 (an illegal mate-in-zero
  position): **closed**. Neither FEN is in the tree (grep), the cases carry
  reachability preconditions (`tests/test_search.cpp:159`, `:176`, `:421`).
- F03 (Debug timeouts): **closed** as filed -- the timeout is the build's since
  S067 -- and its second half, that the invariants run in nothing automatic,
  is F01 of this report.
- F04 (a helper states the opposite of INV-1): **open again**, in a new
  wording, `tests/test_helpers.hpp:80-83`; folded into F05 here.
- F05 (insufficient-material boundary cases): **closed**, `tests/test_search.cpp:2869`
  carries eleven rows including the deliberate python-chess disagreement, and
  M25 was caught by it.
- F06 (ordering guard headroom): **closed**, the budget is 440000 against a
  measured tree at `:894-905` -- still 4x, stated.
- F07 (`test` command prints and does not check): **accepted** by DEC-058 and
  unchanged; the signature half is F07 here.

## Checked and clean

Stated because a negative result is a result. The perft oracles and the 127
positions of the 2026-08-14 review were not re-derived; the two movegen
mutants (M22, M27) were caught by perft and by the JSON suite at once. The
full-hash oracle at `tests/test_engine.cpp:56` caught both board mutants that
touched the hash. The pure-function time-budget tests caught both time
mutants with no wall clock involved. The `see_ge`-against-`see` cross-check
caught the SEE mutant, with the caveat that a shared error in both would not
be. The TT storage units caught the bound-type swap through three independent
assertions. The DEC-102 stand-pat rules are pinned in both directions and
caught their inversion. Every inline FEN in the mate and search suites that a
mutant could have made vacuous is guarded by a reachability precondition
(`test_search.cpp:159,176,421,578,773,1072,2073`; `test_engine.cpp:854,857,1624,2590,2752`),
and the remaining unguarded FENs are in cases whose assertions do not depend
on legality (the K+R anchor positions, the SEE positions). `-Werror` is part
of the suite in practice. The 2026-09-04 adversarial report's three open lows
stand as filed and are not repeated here.
