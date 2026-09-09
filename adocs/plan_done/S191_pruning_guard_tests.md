id:         S191
goal:       every null-move, reverse-futility and late-move-reduction guard has a direct test with a precondition that the guard's condition holds at the node, and the S165 defender set is a registered fixture
accepts:    cases, each with its precondition asserted before the search: an in-check node makes no null move; a pawn-only position (`game_phase` 0) is searched with no null move; every defender node of `adocs/data/S165_defender_set.tsv` with beta inside the mate band takes no null-move cutoff, scored over the whole set with zero tolerance; `prev_move == 0` forbids a second consecutive null; a checking move and a capture are searched at `child_depth` with no reduction; a reduced move that beats alpha is re-searched at full depth; reverse futility does not fire in check, at a PV node, above `RFP_MAX_DEPTH` or inside the mate band; each case observed red under the matching mutant of `adocs/data/2026-09-04_test_review/mutants.py` (M01 to M04, M07 to M09) in a binary other than `test_mate_carry`, then green; the defender TSV is read through `CHESSO_SOURCE_DIR` like the S145 sets; `DEV_MANUAL.md` "Mate safety" lists the defender set as the fifth instrument; fast suite green in both builds
touches:    tests/test_search.cpp, tests/CMakeLists.txt, src/search.hpp, src/search.cpp, src/data_structures.hpp, adocs/data/S191_mutants.py, DEV_MANUAL.md, adocs/specs.md, adocs/audit/2026-09-04_test_review.md
excludes:   changing any guard; the four S109 rules, which arrive with their own cases under DEC-141
decisions:  DEC-139, DEC-141, DEC-164
closes:     2026-09-04_test_review-F02
blocks:
paused_by:
author:     agent (Claude Opus 5), coordinator, 2026-09-09
done:       2026-09-09. Thirteen guard cases in `tests/test_search.cpp`'s new
            `search: pruning and reduction guards` suite, **each observed red
            under the mutant that removes its guard and green with the guard in
            place**, and the S165 defender set is a registered fixture. No
            engine rule changed: `chesso bench` is **26851183**, the parent's,
            so the commit carries `No functional change`.

            **The owner's answers on section 10, taken 2026-09-09.** Q5: the
            counter/probe route rather than the transposition-table observable,
            which is why `src/search.cpp`, `src/search.hpp` and
            `src/data_structures.hpp` joined `touches:` and why INV-6, a
            timing and the Debug self-play are discharged below. Q1 to Q4:
            accepted as proposed -- the M04 case was added, the six homeless
            mutants went into `adocs/data/S191_mutants.py`, the positive
            reverse-futility band edge is recorded as inert rather than written
            as a case that cannot meet its own precondition, and "takes no
            null-move cutoff" is asserted as the stronger "makes no null move".

            **The observable.** `search_node_probe_t` in
            `src/data_structures.hpp` records one node's decisions -- whether
            the null move was made, whether reverse futility returned, and per
            legal move the reduction it was first searched with and whether it
            was re-searched. `search_state_t` carries a pointer to one. Nothing
            in the search reads a field of it.

            **It costs the engine nothing, and that took a second design.** The
            first version resolved `state->probe` at every interior node and
            tested it once per move. Interleaved and paired `chesso bench`,
            13 pairs: **1.49 % fewer nodes per second, sd 0.66 %** -- resolved,
            not this machine's noise, and moving the field beside the hot ones
            did not recover it (hyperfine block runs had read 1.01x both ways
            with sigmas too tight to trust across drift, which is why the
            paired form was used). `negamax` is now
            `negamax_at<bool PROBING>`, instantiated `<false>` for the engine
            and `<true>` only at the node `negamax_probed()` drives; the
            recursion is always `<false>`, which is exact because a probe names
            one ply and every child is at another. Re-measured over **33
            interleaved pairs: 0.14 % +/- 0.24 % at 95 %**, nothing this
            machine can resolve. `search_lmr_reduction_probe()` also stopped
            being tune-only: a case asserting a guard refused to reduce says
            nothing unless the table would have reduced, and that has to be
            checkable in the build the gate ships.

            **Red then green, all thirteen** (Release, run from `tests/`, one
            mutant at a time, rebuilt each time):

            | case | mutant | failing assertion |
            |---|---|---|
            | an in-check node makes no null move | M01_nmp_in_check | `REQUIRE( !probe.null_move_made )` -> `false` |
            | a node with only kings and pawns makes no null move | M03_nmp_zugzwang | `REQUIRE( !probe.null_move_made )` -> `false` |
            | a node whose parent already passed makes no null move | N01_nmp_double_null | `REQUIRE( !probe.null_move_made )` -> `false` |
            | a node at the positive edge of the mate band makes no null move | N02_nmp_mate_band_pos | `REQUIRE( !probe.null_move_made )` -> `false` |
            | no defender node inside the mate band makes a null move | M02_nmp_mate_band_neg | `REQUIRE( violations.empty() )` -> `false` |
            | a null-move fail-high against a mate returns the bound | M04_nmp_mate_artifact | `REQUIRE_EQ( score, beta )` -> `48996, 40000` |
            | reverse futility does not fire at a node in check | N03_rfp_in_check | `REQUIRE( !probe.rfp_cutoff )` -> `false` |
            | reverse futility does not fire at a PV node | N04_rfp_pv | `REQUIRE( !probe.rfp_cutoff )` -> `false` |
            | reverse futility does not fire above its depth bound | N05_rfp_depth_bound | `REQUIRE( !probe.rfp_cutoff )` -> `false` |
            | reverse futility does not fire inside the mate band | N06_rfp_mate_band_neg | `REQUIRE( !probe.rfp_cutoff )` -> `false` |
            | a capture is not reduced | M07_lmr_captures | `REQUIRE_EQ( probe.reduction[k], 0 )` -> `1, 0` |
            | a quiet move that gives check is not reduced | M08_lmr_checks | `REQUIRE_EQ( probe.reduction[k], 0 )` -> `1, 0` |
            | a reduced move that beats alpha is searched again at full depth | M09_lmr_no_research | `REQUIRE( researched > 0 )` -> `0 > 0` |

            The pass was run twice, once before the templating and once after,
            and every anchor in both mutant files -- **39 of 39, the
            2026-09-04 file's 33 included** -- still occurs exactly once in the
            source it names.

            **One case needed a beta the guide did not name, and it is a
            measurement.** M04's mating node sits three plies below the driven
            one, which is exactly `RFP_MIN_PLY`, and it inherits the drive's
            beta: at the guide's 100 reverse futility fires there on a static
            score a queen up and the precondition read
            `REQUIRE( 742 >= 48000 )`. Beta is 40000, still inside the mate
            band. That is the reverse-futility comment's own "a mate deeper
            than ply 3 can still be missed for an iteration", met head on.

            **The defender fixture.** 104 rows, the golden named at its site
            with `grep -vc '^#' adocs/data/S165_defender_set.tsv` (header
            included) as its re-derivation, read through `CHESSO_SOURCE_DIR`
            the way the S145 sets are. Every row asserts its beta is inside the
            band and that the node is not in check, has phase above zero and
            ply above zero, so a row cannot pass on the wrong guard. Zero
            tolerance, and the message names offending FENs.

            **Behaviour neutrality, INV-6.** `tools/search_bench.py` against
            `cb6aca8` at depth 9 -- 121530 / 801481 / 72924, `c3d5` / `e2a6` /
            `d7c8q` -- and depth 12 -- 636677 / 3520847 / 494098, same three
            moves -- identical on every node count and every best move; only
            the wall-clock line differs. `chesso bench` 26851183 both sides.

            **Debug self-play, DEC-141 clause 1.** 4 rounds at 4+0.04 on the
            UHO book, both engines `build-debug/src/chesso`, `Hash=16`,
            `Threads=1`. 8 of 8 games finished with a normal result, no
            warning, no termination, and **0 `Assertion` lines**. The grep is
            not vacuous: the log was taken with `-log ... engine=true`, which
            captured **78601 engine-stderr lines**. A first attempt greped a
            0-byte log and was thrown away; a second, wrapping the engine in a
            stderr-redirecting script, pushed Debug startup past fastchess's
            `uciok` allowance and aborted under `-strict`.

            **Timings.** `test_search`: **2.16 s in Release, 67.08 s in Debug**,
            94 cases and 888340 assertions, against the 600 s Debug ceiling --
            no separate binary is needed (section 10 Q6 answered by
            measurement). The 13 new cases are 967 of those assertions and the
            defender fixture's 104 drives are the bulk of them.

            **Gate.** `ctest -L fast` 31 of 31 in `build` and 31 of 31 in
            `build-tune`; `./clang-format.sh --check` clean with
            `CLANG_FORMAT_MAJOR=22` (DEC-146); `plan_prose_check.py --touches`
            and `--params` both clean.

            **Not owed and not run.** `tools/gate_extra.sh` and
            `tools/mutation_check.py` do not exist yet -- they are S197 and
            S196, Open entries 3 and 2 -- so DEC-141's other two clauses have
            no tool to run; the mutation pass above is the hand form S196 will
            fold in, and `adocs/data/S191_mutants.py` is written in the same
            `m(id, file, klass, note, (old, new))` shape for it. No SPRT: the
            engine's tree is identical and DEC-083 applies a fortiori.

            **DOCS.** `DEV_MANUAL.md`'s "Mate safety" is five instruments now,
            the fifth being this fixture -- what it holds, what it asserts,
            what it cannot see, its commands and its golden. `adocs/specs.md`
            gained three clauses naming the cases that hold the null-move
            edges, the four reverse-futility guards and the two reduction
            guards. **`MANUAL.md` needs no change and was checked**: no UCI
            option, default or output moved, `negamax_probed` is not reachable
            over UCI, and `test_uci_surface` is untouched and green in both
            builds. `README.md` untouched, per DOCS.

## Why this exists

`2026-09-04_test_review-F02`. No test exercises the null-move guards at
`src/search.cpp`'s null-move block or the reduction guards in its move loop.
Five guard-removal mutants -- null move in check, the S165 negative mate-band
guard dropped, null move in pawn endings, a null-move mate score returned as
real, LMR reducing captures -- were each caught by exactly one test,
`tests/test_mate_carry.cpp`'s per-game mate-line floor, and by nothing else;
three of the five leave the depth-9 bench identical too. `adocs/data/S165_defender_set.tsv`
holds 104 proved defender nodes built to measure exactly the mate-band guard
and is read by nothing in `tests/`. Ordered before S109, whose four rules then
ship with their own cases and mutants (DEC-141).

## Shape

The S145 fixture style (`tests/test_engine.cpp`'s mate-safety suite: a TSV
read through `CHESSO_SOURCE_DIR`, a precondition per row, a count with zero
tolerance) for the defender set; single constructed positions, tool-verified
(CLAUDE.md, CHESS rule), for the others. `src/search.hpp` is in `touches:`
only for a test hook where a guard's firing cannot be observed from outside
-- a counter or a probe in the tune build, as `search_lmr_reduction_probe`
already is.

## Cost

Machine-free, about a day.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

This step adds tests and changes no engine behaviour. The search has three
rules that skip or shorten work -- null move pruning, reverse futility pruning
and late move reductions -- and each carries guards: conditions under which it
must not fire, because firing there hides forced mates (CLAUDE.md, "Pruning
that hides a mate is the recurring bug"). The 2026-09-04 test review removed
those guards one at a time and found that five removals were caught by a single
golden count in `tests/test_mate_carry.cpp` and by nothing else, and that three
of the five left the depth-9 bench identical, so INV-6 would have passed them
too (`adocs/audit/2026-09-04_test_review.md`, finding F02). The work is one
direct case per guard in `tests/test_search.cpp`, each driving one node of the
search with the guard's condition made true and asserted before the search
runs, each observed red under the mutant that removes the guard; and the 104
proved defender nodes of `adocs/data/S165_defender_set.tsv`, built for exactly
the S165 guard and read by nothing in `tests/`, become a registered fixture. It
is ordered before S109 because S109's four rules must each arrive with a case
and a mutant under DEC-141, and this step is the pattern they copy.

### 2. The technique as published

The guards are the standard ones. The wiki's null move page
(https://www.chessprogramming.org/Null_Move_Pruning) says "a basic
implementation of NMP only guards against positions in check and positions
where the side to move has only king and pawns", that "some implementations
also disables consecutive null moves", that "the null move cannot be used when
the side to move is in check, because that would result in an illegal
position", that "most engines do not try the null move in the endgame" because
of zugzwang, and, on mate scores, that "a fail-soft framework is necessary to
recognize 'I get mated, if I do nothing'". The reverse futility page
(https://www.chessprogramming.org/Reverse_Futility_Pruning) gives the rule as
`if (... && eval >= beta + margin) return eval; // fail soft` and lists, among
the conditions to skip it, that the position is in check and that the node is a
PV node. The late move reductions page
(https://www.chessprogramming.org/Late_Move_Reductions) names depth below 3 as
the common condition not to reduce and, among the others, captures and
promotions, moves while in check, moves giving check, PV nodes and the first one
or two moves; on the re-search it says the "classical implementation assumes a
re-search at full depth if the reduced depth search returns a score above
alpha". Chesso carries every one of these guards today; section 3 lists them by
symbol and this step changes none of them.

The practice followed is the testing one. AGENTS.md's TESTS rule: "A test
asserting X does not happen first establishes the precondition that would make
X happen" -- a case saying "no null move in check" is vacuous unless it first
shows the node is in check and that every other condition of the null-move
block holds, so the guard under test is the only thing between the node and
the null move. DEC-141 adds the other half: a guard test ships with a mutant
that removes the guard, and the test is observed red under it before it is
called done. `adocs/testing_strategy.md` recommendation R3 is the origin of the
step and names the two patterns to copy: the S145 fixture style in
`tests/test_engine.cpp` ("a proved mate is never mis-scored, and every mate in
two is found on time": a TSV read through `CHESSO_SOURCE_DIR`, a precondition
per row, a count) and the tune build's `search_lmr_reduction_probe` in
`src/search.hpp`.

### 3. What chesso has today, and where the change plugs in

**The guards, as `src/search.cpp` `negamax` reads at HEAD (`ca9199b`).** The
null-move block is entered on `!is_pv && !is_in_check && ply > 0 && prev_move
!= 0 && depth - 1 - null_reduction >= 1 && beta < MATE_MIN && beta > -MATE_MIN
&& game_phase(&game->board) > 0`, with `null_reduction = NULL_MOVE_BASE +
(depth / NULL_MOVE_DIVISOR)`; the child is `-negamax(-beta, -beta + 1, depth -
1 - reduction, ply + 1, game, state, 0, false)` -- note the `0` it passes as
`prev_move`, which is what the double-null guard reads -- and the fail-high
return is `return (null_score >= MATE_MIN) ? beta : null_score;`. Reverse
futility is `!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&
depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN`, then `margin =
RFP_MARGIN * depth` and `if (static_eval - margin >= beta) return static_eval -
margin;`, where `static_eval` is `TT_EVAL_NONE` (`INT16_MIN`,
`src/data_structures.hpp`) whenever the node is in check. The reduction guard
in the move loop is `ply > 0 && depth >= 3 && legal_moves_counter > 3 &&
!is_capture && !MOVE_PROMOTED(moves[i]) && !is_in_check && !is_check_move`,
with `reduction = lmr_reduction(depth, legal_moves_counter)` clamped to
`child_depth - 1` and `child_depth = depth - 1`; `is_check_move` is
`is_capture ? false : is_check(game)` (S107 left it exactly one consumer, this
guard). The re-search is `if (!state->aborted && reduction > 0 && score >
alpha)` at `child_depth`. `lmr_reduction` reads a table `build_lmr_table` fills
from `LMR_BASE` and `LMR_DIVISOR`; at depth 3 and move number 4 the shipping
coefficients give 1.

Three facts about the surroundings carry the design. `negamax` does
`state->explored_nodes++` before every early exit. Every node that reaches its
end stores its own position with its own `depth` through `tt_store_entry`
(`src/transposition_table.cpp`), quiescence stores at `TT_DEPTH_QS`, −1
(`src/transposition_table.hpp`), and `tt_store_entry` replaces a slot when
`entry->generation != tt->generation || depth >= entry->depth` --
"Depth-preferred inside one search", regardless of key.

**What a test can reach.** `src/search.hpp` declares `negamax`, `quiescence`,
`SEARCH_SCORE_INF` and, under `CHESSO_TUNE`, `search_lmr_reduction_probe`.
`src/evaluation.hpp` declares `game_phase`, `evaluate`, `score_move` and
`capture_score`; `src/bitboard.hpp` declares `is_check`, `make_move`,
`unmake_move`, `make_null_move`, `unmake_null_move`; `tests/test_helpers.hpp`
has `position_is_reachable`, `legal_moves` and `move_is_legal`.
`tests/test_search.cpp` already holds `search_fixture_t` (a 4 MB table, 131072
entries of 24 bytes), `node_fixture_t` (loads a FEN, requires reachability,
wipes the table, offers `stored()`), the local pins `MATE_MAX_LOCAL` and
`MATE_MIN_LOCAL` -- needed because `MATE_MAX` and `MATE_MIN` are `#define`s
inside `src/search.cpp` and in no header -- and the S107 case "a quiet move
that gives check enters the ordering tables", the template for a node drive:
`negamax(BETA - 1, BETA, DEPTH, PLY, &game, &state, prev_move, false)` after
`tt_reset(&tt); tt_new_search(&tt);`, with `prev_move` built by `NEW_MOVE`.
`tests/test_mate_carry.cpp` `read_cases()` is the TSV reader to copy, `#error`
guard included.

**Four observables, and why no probe is needed.** Each guard's firing leaves a
trace a test can already read:

(a) *A null move was made at the driven node* if and only if, after the drive,
`make_null_move(&game); tt_get_entry(&tt, &game.board)` returns an entry with
`depth >= 1` (then `unmake_null_move`). The null child is the first thing the
node searches and it stores the passed position at `depth - 1 -
null_reduction`, which is exactly 1 at drive depth 5 (`5 - 1 - (3 + 5 / 6)`).
Legal play can also reach the passed position -- same placement, other side to
move -- but it needs at least five plies: the side to move must return in three
moves and the other side in two, and one or three plies cannot restore a
placement. At depth 5 that transposition is a leaf and stores at −1; at depth 6
it would arrive at depth 1 and be indistinguishable. **The drive depth is 5 and
nothing else.** In the in-check case the passed position has the side not to
move in check, which legal play never produces, so any entry there is a null
move.

(b) *Reverse futility fired* if and only if `state.explored_nodes == 1`: the
node counted itself and returned before searching a child. With the guard
holding, and null move switched off at the node by passing `prev_move = 0`, the
move loop runs and `explored_nodes > 1`.

(c) *A move was searched at depth d* if and only if the position after it
holds an entry with `entry->depth == d`. A reduced move that fails low stores
`child_depth - reduction`; a re-searched one is overwritten by the deeper store
under the `depth >= entry->depth` rule.

(d) *A null-move cutoff near a mate returned the bound* if and only if the
drive returns exactly `beta`; the move loop of a position whose true value is a
mate returns a mate score, never `beta`.

**Order of edits.** (1) `tests/CMakeLists.txt`: `target_compile_definitions(
test_search PRIVATE CHESSO_SOURCE_DIR="${CMAKE_SOURCE_DIR}")`, the line
`test_uci_surface` already has. (2) `tests/test_search.cpp`: a new
`TEST_SUITE("search: pruning and reduction guards")` with a fixture derived from
`search_fixture_t` that does the S107 reset-and-new-search before each drive,
a `read_defender_set()` copied from `read_cases()` (skip `#` lines and the
`fen` header row; six tab-separated columns `fen mated_in ply root_distance
family root_fen`; 104 rows), then the cases of section 6 one at a time, each red
first. (3) `DEV_MANUAL.md` and `adocs/specs.md` per section 8. (4)
`src/search.hpp` stays untouched under this design; section 10 says what
changes if the owner prefers a counter.

### 4. Constants and seeds

No engine constant is added or moved, so nothing enters `CHESSO_SEARCH_PARAMS`
(`src/search_params.hpp`, entries of the form `X(SYMBOL, "UciName", default,
lo, hi)`). The test constants, each a DEC-105 form (b) derivation over chesso's
own parameters unless said otherwise, and each asserted inside its case so a
tune-build value that moves reads as a red and not as a vacuous pass:

- Null-move drive depth **5**: the only depth at which `D - 1 - (NULL_MOVE_BASE
  + D / NULL_MOVE_DIVISOR)` is 1 while the five-ply transposition of section 3
  (a) still lands at depth 0. Assert `REQUIRE_EQ(D - 1 - (NULL_MOVE_BASE + D /
  NULL_MOVE_DIVISOR), 1)`.
- M04 drive depth **7**: reduction 4, child depth 2, the smallest child depth
  at which a quiet mate in one after any reply is seen -- the mating move lands
  at depth 0 on quiescence's in-check-no-moves return, held by the existing case
  "mate is recognised at depth zero".
- Beta per defender row: `s + 1` with `s = -(MATE_MAX_LOCAL - ply - 2 *
  mated_in)`, the mated score the row proves (a mate against the side to move in
  k is `2k` plies, `adocs/data/S165_nmp_defender_sweep.py`'s docstring); assert
  `beta <= -MATE_MIN_LOCAL`. Band edges: `MATE_MIN_LOCAL` for the positive
  null-move edge and `-MATE_MIN_LOCAL` for the negative reverse-futility edge,
  the first values each guard excludes.
- Reverse-futility-in-check beta **−40000**: any value in (−48000, −32957] makes
  the mutant that drops `!is_in_check` non-equivalent, because the mutated
  comparison is `TT_EVAL_NONE - RFP_MARGIN * 3 >= beta` and `INT16_MIN - 189`
  is −32957; −40000 is near the midpoint (form c). Assert the inequality.
- Fail-low alpha **5000** for the LMR drives (form c): a bound no move can beat
  without a forced mate, so no reduced move is re-searched and the stored
  depth is the reduced one; assert the drive's return is below it.
- The row count **104** is a golden under DEC-142: name it at its site and
  re-derive with `grep -vc '^#' adocs/data/S165_defender_set.tsv` minus the
  header row, or `~/.venv/chess/bin/python adocs/data/S165_nmp_defender_sweep.py
  generate`, which rewrites the file.

### 5. Interactions and traps

- **Run doctest binaries from `tests/`.** Today, at HEAD, `./build/tests/
  test_search` from the repository root reports 3 failed cases, all `Cannot
  open assets/test_jsons/castling.json`; from `tests/` it is 80 of 80 in
  0.33 s. `ctest` sets the directory; a hand run must `cd tests` first. A red
  observed from the wrong directory is not the mutant's.
- **Depth 6 is a false positive for observable (a)**, section 3. Depth 4 makes
  the null child zero plies deep and the block is never entered at all.
- **Slot collisions are deterministic, not flaky.** Zobrist keys and the tree
  are fixed, so a colliding deeper store that evicts the entry a case reads
  happens every run or never. If `REQUIRE(entry != nullptr)` fails under the
  guarded build after an unrelated search change, resize the table inside the
  case before suspecting the guard; if a mutant's red is not observed, the same
  eviction may have hidden it -- pick another position. The 104 defender rows
  give the M02 mutant 104 chances.
- **The in-check reverse-futility mutant is equivalent at an ordinary beta.**
  `static_eval` is the sentinel in check, so dropping `!is_in_check` fires only
  when `beta <= -32957`. The −40000 above is what makes the case kill anything.
  The arithmetic is `int`, so the sentinel does not overflow.
- **The positive reverse-futility edge is not constructible.** `static_eval -
  margin >= beta` with `beta >= MATE_MIN` needs a static score near 48000, and
  `src/search_params.hpp` records the guard as one "S145 measured inert" because
  the expensive terms are clamped to `LAZY_EVAL_MARGIN`. Do not write a case
  whose precondition cannot hold; section 10 defers the wording.
- **Captures are ordered first.** `score_move` puts every capture and promotion
  in the capture band above killers and quiets (CLAUDE.md, the 100-point
  clearance), so a capture is the fourth legal move only when at least three
  legal captures outscore it under `capture_score`; a quiet is always late when
  three legal captures exist and the table is cold. Assert the order with the
  engine's own `score_move`, never by eye, and avoid ties.
- **A reduced move that beats alpha is re-searched, which hides M07 and M08.**
  The two "not reduced" cases need the tested move to fail low, hence alpha 5000
  and a position without a forced mate; a capture that fails high breaks the
  loop before the tested move is reached, so also require the drive's return
  below beta.
- **A mating check stores nothing.** The child of a checking move that is
  mate returns before its store; the M08 case needs a check that is not mate.
- **`generate_captures` may emit pseudo-legal moves** (`negamax` does `if
  (!make_move(game, moves[i])) continue;`); count legality with
  `legal_moves` or `move_is_legal` from `tests/test_helpers.hpp`.
- **`prev_move` must be a well-formed move or zero.** `score_move` indexes
  `counter_moves[MOVE_PIECE(prev_move)][MOVE_TO(prev_move)]`; use `NEW_MOVE`
  with a legal piece and square as the S107 case does, or `0` where the case
  wants null move off at the node.
- **Tune build.** Parameters are variables there; compute `null_reduction` and
  the LMR reduction in the case from the live `NULL_MOVE_BASE`,
  `NULL_MOVE_DIVISOR`, `LMR_BASE` and `LMR_DIVISOR` (`search_lmr_reduction_probe`
  under `#ifdef CHESSO_TUNE`, the `build_lmr_table` formula otherwise), never
  from a literal.
- **Under M01 the null child has a king en prise.** The Release binary survives
  it ("a board with no kings is survivable" exists); the Debug binary may
  assert. Observe reds in the Release build.
- **Debug timeout.** `test_search` is 216 s in Debug against a 600 s ceiling
  (`DEV_MANUAL.md` "Test", measured 2026-08-14 and grown since). The defender
  scoring is 104 narrow-window depth-5 drives, cheap in Release; measure
  `../build-debug/tests/test_search` from `tests/` after adding it and record
  both figures in the stamp.
- **`adocs/data/2026-09-04_test_review/mutants.py` is evidence and is never
  edited** (`adocs/data/README.md`, "added, never edited"). Mutants this step
  needs beyond M01 to M04 and M07 to M09 go in a new file
  `adocs/data/S191_mutants.py` in the same `m(id, file, klass, note, (old,
  new))` form; S196 folds both into `tools/mutants/`.
- **Chess judgement stays with the tool** (DEC-023). Every FEN below is
  constructed and checked with python-chess in `~/.venv/chess` -- in check, not
  mate, king-and-pawns only, a mate in one that survives a pass, which moves
  are captures or give check -- and Stockfish only through the two safe forms
  in `TOOLCHAIN.md` "The chess oracle, and the one way to ask it that lies".
  `position_is_reachable` is a precondition on every constructed position.

### 6. Tests

One case per guard. Every case first asserts every *other* condition of the
block it belongs to, so the guard under test is the only thing that can stop
the rule; then the observable. `D`, `P` and `B` are the drive's depth, ply and
beta; drives are null-window `negamax(B - 1, B, D, P, &game, &state, prev,
is_pv)` unless said.

| guard (symbol in `negamax`) | precondition asserted before the drive | observable | mutant |
|---|---|---|---|
| null move: `!is_in_check` | `is_check(&game)`, a legal evasion exists, `game_phase > 0`, `P = 1`, `prev != 0`, `D = 5`, `B` ordinary | no entry at the passed position (3a) | M01 |
| null move: `game_phase(...) > 0` | `game_phase(&game.board) == 0` (kings and pawns: `phase_value` in `src/eval_tables.hpp` gives both 0), `!is_check`, `P = 1`, `prev != 0`, `D = 5`, `B` ordinary | no entry with `depth >= 1` at the passed position | M03 |
| null move: `prev_move != 0` | as above with `prev = 0`, phase > 0 | same | new: drop `prev_move != 0 &&` |
| null move: `beta > -MATE_MIN`, the S165 guard | per defender row: `load_FEN`, `position_is_reachable`, `!is_check`, `game_phase > 0`, `P = row.ply`, `B = s + 1 <= -MATE_MIN_LOCAL`, `D = 5`, `prev != 0` | violations counted over all 104 rows, `REQUIRE(violations.empty())` with the FENs in the message; `REQUIRE_EQ(rows.size(), 104)` | M02 |
| null move: `beta < MATE_MIN` | any quiet non-check position, `B = MATE_MIN_LOCAL`, rest as row one | no entry with `depth >= 1` at the passed position | new: drop `beta < MATE_MIN &&` |
| null move: `(null_score >= MATE_MIN) ? beta : null_score` | `P = 1`, `D = 7`, `B` ordinary; precondition drive: `make_null_move`, `ns = -negamax(-B, -B + 1, D - 1 - R, P + 1, ..., 0, false)`, `unmake_null_move`, `REQUIRE(ns >= MATE_MIN_LOCAL)`, reset table | `REQUIRE_EQ(score, B)` (3d) | M04 |
| reverse futility: `!is_in_check` | `is_check`, `P = RFP_MIN_PLY`, `D = 3`, `prev = 0`, `B = -40000`, `TT_EVAL_NONE - RFP_MARGIN * D >= B` | `state.explored_nodes > 1` (3b) | new: drop `!is_in_check &&` from the RFP condition |
| reverse futility: `!is_pv` | `is_pv = true`, `!is_check`, `P = RFP_MIN_PLY`, `D = 3`, `B = evaluate(&game.board) - RFP_MARGIN * D` | `explored_nodes > 1` | new: drop `!is_pv &&` |
| reverse futility: `depth <= RFP_MAX_DEPTH` | `D = RFP_MAX_DEPTH + 1`, `B = evaluate - RFP_MARGIN * D`, `prev = 0`, a locked position with few legal moves | `explored_nodes > 1`; print the count with `MESSAGE` | new: drop `depth <= RFP_MAX_DEPTH &&` |
| reverse futility: `beta > -MATE_MIN` | `B = -MATE_MIN_LOCAL`, `D = 3`, `P = RFP_MIN_PLY`, `prev = 0`, `evaluate - RFP_MARGIN * D >= B` | `explored_nodes > 1` | new: drop `beta > -MATE_MIN` from the RFP condition |
| reduction: `!is_capture` | `D = 3`, `P = 1`, `prev = 0`, `B - 1 = 5000`; the tested capture `c` is legal and at least three legal moves outscore it under `score_move`; no move fails high (`score < B`) | entry at the position after `c` exists and `entry->depth == D - 1` (3c) | M07 |
| reduction: `!is_check_move` | same drive; quiet `q` with `!MOVE_CAPTURE(q)`, `MOVE_PROMOTED(q) == TO_NONE`, `is_check` after `make_move(q)`, not mate; three legal captures exist | `entry->depth == D - 1` after `q` | M08 |
| re-search: `reduction > 0 && score > alpha` | `D = 3`, `P = 1`, `B = A + 1` with `A` the node's static score; three losing captures each `<= A` at `D - 1`; quiet non-check `q` late in order with `r >= 1` and `-negamax(-(A + 1), -A, D - 1 - r, 2, ...) > A` on the child; reset | `entry->depth == D - 1` after `q` (the re-search's store replaced the reduced one) | M09 |

Two cases in pseudo-code; the rest follow their rows.

```
TEST_CASE_FIXTURE(guard_fixture_t, "an in-check node makes no null move")
{
  load(fen);                                   // python-chess: in check, not mate
  REQUIRE(is_check(&game));
  REQUIRE(legal_moves(&game, buffer) > 0);
  REQUIRE(game_phase(&game.board) > 0);
  REQUIRE_EQ(D - 1 - (NULL_MOVE_BASE + D / NULL_MOVE_DIVISOR), 1);   // D = 5
  negamax(B - 1, B, D, 1, &game, &state, prev, false);              // B = 100
  make_null_move(&game);
  REQUIRE(tt_get_entry(&tt, &game.board) == nullptr);   // any entry is a null move here
  unmake_null_move(&game);
}

TEST_CASE_FIXTURE(guard_fixture_t,
                  "no defender node inside the mate band makes a null move")
{
  const auto rows = read_defender_set();       // CHESSO_SOURCE_DIR
  REQUIRE_EQ(rows.size(), 104u);               // golden, see section 4
  std::vector<std::string> violations;
  for (const auto& row : rows) {
    load(row.fen);  REQUIRE(!is_check(&game));  REQUIRE(game_phase(&game.board) > 0);
    const int s = -(MATE_MAX_LOCAL - row.ply - 2 * row.mated_in);
    const int B = s + 1;  REQUIRE(B <= -MATE_MIN_LOCAL);
    negamax(B - 1, B, 5, row.ply, &game, &state, prev, false);
    make_null_move(&game);
    const tt_entry_t* e = tt_get_entry(&tt, &game.board);
    if (e != nullptr && e->depth >= 1) { violations.push_back(row.fen); }
    unmake_null_move(&game);
  }
  REQUIRE_MESSAGE(violations.empty(), join(violations));
}
```

**Observing a red.** Before S196 lands, apply one mutant by hand and revert:

```bash
python3 - <<'PY'
import importlib.util, sys
spec = importlib.util.spec_from_file_location("m", "adocs/data/2026-09-04_test_review/mutants.py")
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
mut = next(x for x in m.M if x["id"].startswith("M01"))
p = mut["file"]; t = open(p).read()
for old, new in mut["pairs"]:
    assert t.count(old) == 1, "anchor"; t = t.replace(old, new)
open(p, "w").write(t)
PY
cmake --build build -j8 && (cd tests && ../build/tests/test_search -tc='an in-check node makes no null move')
git checkout -- src/search.cpp
```

Record the failing assertion text per mutant in the stamp, the way the S106
cases carry theirs in `tests/test_search.cpp` ("Mutation: ..." comments). All
ten anchors M01 to M09 were verified to occur exactly once in `src/search.cpp`
at `ca9199b`. After S196, `tools/mutation_check.py` runs the same list.

**Goldens touched:** the row count 104 (section 4). No floor in
`tests/test_engine.cpp`, `tests/test_mate_breadth.cpp` or `tests/test_mate_carry.cpp`
moves, because no engine source changes. **Mate-safety instruments:** the four
in `DEV_MANUAL.md` are untouched; the defender fixture becomes the fifth
(section 8). **INV-6:** not owed -- no `src/` change; if section 10's counter
alternative is taken, `python3 tools/search_bench.py ./build/src/chesso 9` and
`... 12` must read identical nodes and best moves against the parent commit,
plus `hyperfine --warmup 1 --runs 10` interleaved. **Debug self-play:** not
owed under DEC-141 clause 1, which names steps touching the search; owed if
the counter alternative is taken, as four rounds of `fastchess -engine
cmd=./build-debug/src/chesso name=a -engine cmd=./build-debug/src/chesso name=b
-openings file=<the book fastchess.sh uses> format=epd order=random -each
tc=4+0.04 option.Hash=16 option.Threads=1 -rounds 4 -repeat -log file=dbg.log`
followed by `grep -c Assertion dbg.log`, which must print 0; S190 writes the
canonical line into `DEV_MANUAL.md` "Test".

### 7. Measurement

No lane. The step is test-only, the engine binary is byte-identical, and
DEC-083 ("a pure speed-up is not measured in games"; `DEV_MANUAL.md` "Not
every change goes to a match": a change that leaves the node count identical is
not sent to an SPRT) applies a fortiori to a change that produces no binary
difference at all. Nothing under `adocs/data/S191_*.sh` is created. What is
recorded, in the step file's stamp: the red-then-green table (case, mutant,
failing assertion), the defender fixture's row count and wall time in Release
and Debug, and the `ctest -L fast` totals in both gated builds. If the counter
alternative is taken instead, the lane is timing: identical `search_bench.py`
output at depths 9 and 12 and one interleaved `hyperfine` reading, recorded as
a ratio with the machine's noise floor beside it (`bench_movegen` prints its
own resolution).

### 8. Completion checklist

1. Gate: `cmake --build build -j8 && ctest --test-dir build -L fast
   --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
   build-tune -L fast --output-on-failure && ./clang-format.sh --check`; then
   `python3 tools/plan_prose_check.py --touches | tail -1` and `--params`.
2. Every new case observed red under its mutant in `test_search`, then green;
   the table in the stamp.
3. Bench line: none, because no file under `src/` changes; if `src/search.hpp`
   or `src/search.cpp` is touched after all, the commit ends `No functional
   change` and, once S189 has landed, `tools/gate.sh` checks it.
4. `adocs/specs.md`: the search row's sentence "Null move pruning guards both
   edges of the mate band since 2026-08-23, S165, and until then it guarded
   only the positive one" gains a clause naming the two `tests/test_search.cpp`
   cases that assert it; the sentence "the one class of quiet this engine's
   own late move reduction refuses to reduce on the grounds that the line is
   forcing" names the checking-move case; a reverse-futility clause names the
   four guard cases. The INV table does not change.
5. `DEV_MANUAL.md`: the heading "Mate safety: four instruments, and none of
   them substitutes for another" becomes five, and a numbered paragraph **5**
   describes the defender fixture: what it holds (104 proved defender nodes,
   plies 1 to 7), what it asserts (no null move at any of them with beta inside
   the band, zero tolerance), what it cannot see (it drives one node cold and
   says nothing about mate finding, which instrument 1 covers), and the
   command `cd tests && ../build/tests/test_search -tc='*defender*'`. Trace
   every claim to the case's code. `MANUAL.md` needs no change -- no UCI
   option, default or output moves -- and `test_uci_surface` is untouched;
   say so in the stamp.
6. Stamp content: cases added by title, mutants and their failing assertions,
   the 104 golden and its script, Release and Debug wall times of
   `test_search`, both gate totals, the DOCS conclusions.
7. Through the coordinator: `plan.md` Open list and Done list, `status.md`, and
   the finding's line `Status: planned — S191` in
   `adocs/audit/2026-09-04_test_review.md` moving to closed -- the one edit an
   audit report takes.

### 9. Sources read

- https://www.chessprogramming.org/Null_Move_Pruning -- the standard guards
  (in check, king and pawns, consecutive nulls, zugzwang) and the fail-soft
  remark on mates. Fetched.
- https://www.chessprogramming.org/Reverse_Futility_Pruning -- the rule's
  pseudo-code and the skip conditions (in check, PV node). Fetched.
- https://www.chessprogramming.org/Late_Move_Reductions -- what is not reduced
  and the classical re-search rule. Fetched.
- `adocs/audit/2026-09-04_test_review.md` F02; `adocs/data/2026-09-04_test_review/
  mutants.py`, `kills.txt`, `results.tsv`, `run.py` -- the seven mutants, what
  caught each, the bench columns; `adocs/testing_strategy.md` R3.
- `adocs/decisions.md` DEC-023, DEC-116, DEC-139, DEC-141, DEC-142, DEC-145;
  `adocs/plan_done/S165_nmp_mate_band_guard.md`, `S103_...`, `S107_...`, `S145_...`.
- `adocs/data/S165_defender_set.tsv` and `S165_nmp_defender_sweep.py` -- format,
  the 2k-plies rule, exact/short/sign scoring.
- `src/search.cpp` `negamax`, `lmr_reduction`, `build_lmr_table`;
  `src/search.hpp`; `src/search_params.hpp`; `src/transposition_table.cpp`
  `tt_store_entry`; `src/data_structures.hpp` `search_state_t`, `tt_entry_t`,
  `TT_EVAL_NONE`; `src/eval_tables.hpp` `phase_value`; `src/evaluation.hpp`.
- `tests/test_search.cpp` (`search_fixture_t`, `node_fixture_t`, the S107 case,
  "pruning does not hide a forced mate"), `tests/test_mate_carry.cpp`
  `read_cases`, `tests/test_engine.cpp` mate-safety suite,
  `tests/CMakeLists.txt`.
- `DEV_MANUAL.md` "Test", "Mate safety", "Measure", "Not every change goes to
  a match"; `TOOLCHAIN.md` "The chess oracle, and the one way to ask it that
  lies"; `adocs/data/README.md`;
  `fastchess.sh` (flags for the self-play line).

### 10. Questions deferred to the owner

1. **M04 has no case in `accepts:`.** The list requires each case red under
   "M01 to M04", but none of the named cases can see M04, which changes what a
   null-move fail-high *returns*, not whether it happens. Proposed addition:
   "a null-move fail-high against a mate score returns beta and not the null
   score" (section 6, row six).
2. **Mutants without a home.** The `prev_move == 0`, positive null-move edge
   and four reverse-futility cases have no matching mutant in `mutants.py`,
   which is append-only. Proposed: `adocs/data/S191_mutants.py`, folded by S196.
3. **"Inside the mate band" for reverse futility** is constructible only at
   the negative edge; the positive edge cannot meet its own precondition
   (section 5). Accept the negative edge as the case, or record the positive
   edge as measured inert (S145) in the stamp.
4. **"Takes no null-move cutoff"** is asserted as the stronger "makes no null
   move" (observable 3a), which implies it. Accept the reading.
5. **`src/search.hpp` in `touches:`** is unnecessary under this design. If the
   owner prefers a counter in `search_state_t` or a tune-build probe instead of
   the TT observable, `src/search.cpp` must join `touches:`, and the Bench line,
   INV-6 at depths 9 and 12, `hyperfine` and the Debug self-play of section 6
   become owed.
6. **Debug wall time.** If the defender scoring pushes `test_search` toward the
   600 s Debug ceiling, the fixture should be its own binary
   (`test_pruning_guards`, the `test_mate_breadth` precedent); `touches:` names
   `tests/test_search.cpp`.
