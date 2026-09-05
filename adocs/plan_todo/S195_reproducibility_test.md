id:         S195
goal:       a fast test holds node-limited searches reproducible across `ucinewgame`, and `bench` resets the table before every position
accepts:    a case in the shape of Stockfish's `tests/reprosearch.sh`: two move sequences searched at ten node limits, each twice with `ucinewgame` between, every reported node count identical; a case that two `go depth N` on one FEN in one process, separated by `ucinewgame`, report identical nodes, and that without it the second runs on a warm table -- the hazard pinned as a documented property, not repaired; S189's `bench` sends `ucinewgame` per position (S189's accepts carries it if S189 lands first, this step adds it otherwise); `DEV_MANUAL.md` "Measure" records the warm-table rule beside the bench instructions; fast suite green in both builds
touches:    tests/test_engine.cpp, src/chesso.cpp, DEV_MANUAL.md
excludes:   the Zobrist key generator, which S179 now owns; any change to `set_position`'s reset rule
decisions:  DEC-139
closes:     2026-09-04_test_review-F08
blocks:
paused_by:
author:
done:

## Why this exists

`2026-09-04_test_review-F08`. Stockfish's reproducibility test found a
table-ageing counter that `ucinewgame` did not reset, visible as two
alternating `bench` totals and invisible to an SPRT (Stockfish#5376,
`adocs/testing_strategy.md` section 3.1). Chesso has no equivalent:
`tests/test_search.cpp`'s determinism case repeats three searches on a fresh
table only. The review also found that `set_position` resets the table only
when the FEN string differs from the previous one, so two `go depth N` on the
same FEN in one process run the second warm; `tools/search_bench.py` is safe
because its positions differ, and a bench extended by one repeated position
would silently compare cold with warm. The third exposure of F08 -- Zobrist
keys drawn through `std::uniform_int_distribution<uint64_t>`, whose output is
implementation-defined -- is S179's, amended on 2026-09-05 to regenerate the
keys under the project generator beside the magics.

## Cost

Machine-free, hours.

## Implementation guide (2026-09-05)

### 1. What this step is, for someone new

Two cases in `tests/test_engine.cpp`, one paragraph in `DEV_MANUAL.md`, and a
check that `bench` resets the table before every position. The cases pin a
property the engine has today and nothing guards: a `go nodes N` search visits
the same tree when repeated after `ucinewgame`, and two `go depth N` on one FEN
in one process agree only when `ucinewgame` separates them. Measured 2026-09-05
at HEAD (`src/` at `18deccf`, this MacBook): forty node-limited searches -- two
sequences, ten limits, each twice -- repeated identically; `go depth 8` on the
midgame position read 77617 nodes cold, 77617 cold again, 8449 warm. The test is
green on its first run. Its job is to go red the day a change adds state that
`ucinewgame` forgets to reset -- what Stockfish's version found and an SPRT
cannot.

### 2. The technique as published

- Stockfish's `tests/reprosearch.sh`
  (https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/reprosearch.sh)
  as `adocs/testing_strategy.md` section 3.1 describes it: two short move
  sequences played twice across `ucinewgame` at 20 node limits, every `nodes N`
  line required to appear an even number of times. Not fetched by this pass --
  a file in another engine's repository; the strategy document fetched it.
- What it found, Stockfish#5376
  (https://api.github.com/repos/official-stockfish/Stockfish/issues/5376/comments,
  fetched): repeated `bench` over stdin alternated between 2964957 and 3089159
  nodes because a table generation counter was not reset on `ucinewgame`.
  Invisible to an SPRT; a signature check sees it only by luck.
- The protocol (https://backscattering.de/chess/uci/,
  https://www.wbec-ridderkerk.nl/html/UCIProtocol.html, both fetched):
  `ucinewgame` "is sent to the engine when the next search (started with
  position and go) will be from a different game"; `go nodes x` is "search x
  nodes only"; `go depth x` is "search x plies only". Nothing says what a
  `position` inside one game keeps.
- Chess Programming Wiki, Transposition Table, section Aging
  (https://www.chessprogramming.org/Transposition_Table, fetched): "most todays
  programs do not [clear the hash table between root positions], profit from
  entries of previous searches" -- why the warm second search is a property to
  pin, not a bug to repair (`excludes:`). No wiki page on reproducibility or
  determinism was found; that none exists is unverified.
- Chesso's form: pairwise equality per (sequence, limit) instead of the
  even-count check -- stronger, and a failure names the pair; ten limits, since
  the sweep already spans depth 1 to 9 in a quarter of a second; and a second
  case carrying the negative control the Stockfish shape lacks.

### 3. What chesso has today, and where the change plugs in

State in `src/chesso.cpp` that outlives a `go`, all file-static: `tt`
(`transposition_table_t`: entries and `generation`); `initial_position`, the
FEN string `set_position` compares against; `proven_mate_line` (S170);
`still_in_opening`, `opening_book_enabled`, `opening_book_loaded`; `game`
(board and repetition history, rebuilt by every `position`); `stop_search_signal`
and `session_id`; the `last_*` probes only tests read. Everything the search
orders or prunes with is in `search_state_t` (`src/data_structures.hpp`) --
`killer_moves`, `quiet_history`, `counter_moves`, `static_evals`, `pv_table` --
which `iterative_deepening_search` builds fresh for every `go`, so none of it
can carry. S093's game-long history was measured H0 and reverted in full
(DEC-101; `adocs/specs.md` "History is still zeroed on every `go`"): there is no
persistence flag at HEAD. What reproducibility needs reset is therefore the
table entries and `tt->generation` (`tt_reset`, `src/transposition_table.cpp`),
`still_in_opening` (book path only) and `proven_mate_line`; `initial_position`
matters only as the gate on the reset.

- `src/chesso.cpp` `command_ucinewgame`: `stop_and_join_search()`,
  `set_position(DEFAULT_POSITION)`, `tt_reset(&tt)`, `proven_mate_line.length
  = 0`, `still_in_opening = true`. `tt_reset` zeroes the entries and puts
  `generation` back to 1, so the Stockfish counter hazard is covered here by
  construction.
- `src/chesso.cpp` `set_position`: `tt_reset` and `still_in_opening = true`
  only when `initial_position != fen`. `position startpos` and `position
  startpos moves ...` both pass `DEFAULT_POSITION`, so two of them in a row are
  "the same FEN" whatever their move lists and the second search runs warm. It
  never touches `proven_mate_line`. That line is reporting-only -- its comment
  reads "nothing reads it to decide, order or prune a move" -- so it cannot
  move a node count, but on a warm carry it can lengthen a mate `pv` through
  `complete_mate_pv` (`src/search.cpp`). Compare `nodes` and `bestmove`; never
  `pv`, `time` or `nps`.
- `src/chesso.cpp` `command_go`: `nodes` via `pop_u64`, `depth` sets
  `depth_given`. The fallback timer (`FALLBACK_SEARCH_TIME_MS`, `src/uci.hpp`)
  arms only when `!depth_given && search_options.nodes == 0` with no clock, so
  neither `go nodes N` nor `go depth N` arms one. `search_book_move()` runs
  only when `!infinite && nodes == 0` and returns nothing while `OwnBook` is
  false (default); with the book on, `go depth N` answers a random `bestmove`
  (`gen` is seeded from `std::random_device`) and prints no `info` line. Leave
  the option at its default.
- `src/chesso.cpp` `iterative_deepening_search`: `tt_new_search` bumps
  `generation` once per `go`; each iteration sets `state.node_limit =
  conf.nodes - result.total_node_explored`; `src/search.cpp` `check_limits`
  compares `explored_nodes >= node_limit` at every node, exactly, and polls
  `*state->stop` only when `(explored_nodes & 2047) == 0`. No timer sets the
  flag here, so where a search stops is a function of the tree alone. One
  nuance, measured: the `info` line prints only when `has_result ||
  !state.aborted`, so a final iteration aborting before it has a PV prints
  nothing and the last line's `nodes` can sit below the budget -- sequence A at
  `go nodes 5000` reports 2917. Deterministic; the test compares the last
  `info` line's `nodes` and the `bestmove` line and never asserts `nodes == N`.
- Trap T1 of `adocs/plan_todo/S193_fast_suite_repairs.md`, confirmed:
  `command_ucinewgame` calls `stop_and_join_search()`, which sets
  `stop_search_signal`; only `begin_search_session()` clears it, called by
  `command_go` and `command_test` and declared in no header. Every search in
  these cases goes through `uci_process_line("go ...")` then
  `uci_wait_for_search()` -- never a direct `iterative_deepening_search` after
  a `ucinewgame`, or its second iteration dies on the stale flag.
- Harness, all existing: `uci_init`, `uci_shutdown`, `uci_process_line`,
  `uci_wait_for_search`, `uci_tt`, `uci_game` (`src/uci.hpp`);
  `stdout_capture_t` with `lines()` (`tests/test_helpers.hpp`). Capture-then-
  parse is `tests/test_mate_carry.cpp` `search_mate_lines`; the `" nodes "`
  parse is `tests/test_engine.cpp` "info nodes is cumulative over the whole
  search"; the warm-table precondition is `tests/test_engine.cpp` "ucinewgame
  puts the board and the table back" (`tt_get_entry(uci_tt(), &game.board)`).
  The capture must enclose both the `go` and the `uci_wait_for_search()`:
  `bestmove` is written from the search thread.
- The bench hook. S189 (Open entry 4) precedes this step (entry 18); its guide
  has `command_bench` call a helper factored out of `command_ucinewgame` before
  every position. Read `command_bench` at HEAD, confirm the helper -- or
  `tt_reset` plus the `proven_mate_line` clear -- sits inside the position
  loop, and add case 3 of section 6, which proves it. S189 not landed: section
  10.
- Order: the hand probe of section 4 on the workstation; case 1; case 2; case
  3; `DEV_MANUAL.md`; the gate.

### 4. Constants and seeds

Nothing enters `src/`. Test parameters, all DEC-105 form (b): derived on
chesso's own binary 2026-09-05, re-derived on the workstation with the same
raw-UCI driver before the cases are written -- one process, `uci`,
`isready`/`readyok` after every `ucinewgame`, every `go` read to `bestmove`
(TOOLCHAIN.md "The chess oracle": a pipe that sends the next line early aborts
the search).

- Node limits, ten: 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000,
  150000, 250000. About a factor of two apart so each budget truncates a
  different iteration -- sequence A ran from 113 nodes and one `info` line at
  500 to nine lines at 250000 -- where a linear spacing puts most limits inside
  one iteration. Cost: all forty searches took 0.23 s of search time in Release
  on the DEC-109 MacBook; budget ten times that for Debug.
- Depth for case 2: 8. Midgame FEN cold 77617, warm 8449; the inequality held
  at every depth 5 to 10 (cold/warm 15905/683, 21796/7402, 51673/11935,
  77617/8449, 121512/9110, 209605/8201), so a later depth change cannot make it
  vacuous by accident. 6 ms per cold search.
- Sequences: A `e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6`, B `d2d4 d7d5 c2c4
  d5c4 e2e3 e7e5 f1c4 e5d4`. Only legality matters (DEC-023: nothing else was
  judged), checked with two tools: python-chess 1.11.2 `push_uci` in
  `~/.venv/chess`, and the engine's `fen` command after `position startpos
  moves ...`, which printed the same FEN for each --
  `r1bqkb1r/1ppp1ppp/p1n2n2/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 2 5` and
  `rnbqkbnr/ppp2ppp/8/8/2Bp4/4P3/PP3PPP/RNBQK1NR w KQkq - 0 5`. B has captures,
  so the two trees differ in shape.
- Hash: pin `setoption name Hash value 16` (`TT_DEFAULT_MB`) as
  `tests/test_mate_carry.cpp` does, so a future default cannot silently move the
  cases' cost or warmth.

### 5. Interactions and traps

- Nothing time-based in the cases: no `movetime`, `wtime`, `btime`; `go nodes`
  and `go depth` arm no timer. `Threads` is fixed at 1 (`MANUAL.md`).
- Case 1 has no reliable negative control and must not assert one: without
  `ucinewgame` the warm repeat read the same count at 50000 on both sequences
  and a different one at 5000 and 250000 (A) and 250000 (B). The inequality
  belongs to case 2, where it held at every depth tried.
- A dropped `tt_reset` in `command_ucinewgame` is caught by case 1, not case 2:
  `command_ucinewgame` sets `initial_position` to `DEFAULT_POSITION`, so the
  next `position fen <midgame>` differs and `set_position` resets anyway, while
  `position startpos moves ...` does not differ. A `set_position` that stopped
  resetting on a differing FEN is caught by case 2's `cold == cold_again`. A
  `tt_reset` that forgot `generation` is invisible to both -- the entries are
  zeroed and replacement compares an entry's generation with the table's -- so
  the case comment says that rather than claiming the Stockfish counter is
  guarded.
- A case red at HEAD on the workstation, or red later because some state is not
  cleared, is a finding. Do not extend `command_ucinewgame` or `set_position`
  inside this step: the second is `excludes:`, and either is a behaviour change
  a GUI sees -- specs, `MANUAL.md`, a `Bench:` line, a step of its own.
- Not covered, on purpose: cross-machine differences -- Zobrist keys through
  `std::uniform_int_distribution` (S179), the LMR table's `std::log` -- which
  is why the accepts is in-process. Never compare against a count recorded on
  another machine.
- `uci_process_line("position ...")` and `command_fen` call
  `stop_and_join_search()`; harmless, nothing is running when they are sent.
  Wait for the search before any `REQUIRE` that can fail, so a failure never
  leaves a search thread running into the next case.
- Debug: `ctest`'s timeout is 600 s in that build (`tests/CMakeLists.txt`);
  quote the measured Debug seconds in the stamp.

### 6. Tests

Case 1, in `TEST_SUITE("engine: uci go")`:

```
TEST_CASE("node-limited searches repeat across ucinewgame")
  uci_init();
  { stdout_capture_t c; uci_process_line("uci");
    uci_process_line("setoption name Hash value 16"); }
  sequences = {A, B}; limits = {500, ..., 250000};             // section 4
  auto run = [](moves, limit) -> pair<uint64_t, string> {
    lines; { stdout_capture_t c;
             uci_process_line("ucinewgame");
             uci_process_line("position startpos moves " + moves);
             uci_process_line("go nodes " + to_string(limit));
             uci_wait_for_search(); lines = c.lines(); }
    nodes = 0; seen = false; best = "";
    for line: "info " prefix with " nodes " -> nodes = stoull(after it), seen = true;
              "bestmove " prefix -> best = line;
    REQUIRE_MESSAGE(seen, "no info line: " + moves + " at " + limit);
    REQUIRE_FALSE(best.empty());
    return {nodes, best}; };
  for moves in sequences:
    set<uint64_t> distinct;
    for limit in limits:
      first = run(moves, limit); second = run(moves, limit);
      REQUIRE_MESSAGE(first.first == second.first,
          moves + " nodes " + limit + ": " + first.first + " then " + second.first);
      REQUIRE_EQ(first.second, second.second);
      distinct.insert(first.first);
    // Precondition: the limits truncate at different depths; otherwise the
    // sweep repeats one search ten times and proves nothing about budgets.
    REQUIRE(distinct.size() >= 5);
  uci_shutdown();
```

Case 2, same suite:

```
TEST_CASE("a repeated go depth is cold only across ucinewgame")
  uci_init(); uci and Hash 16 as above;
  fen = tools/search_bench.py's midgame FEN;
  auto run = [&](bool newgame) -> uint64_t {
    capture; if (newgame) uci_process_line("ucinewgame");
    uci_process_line("position fen " + fen); uci_process_line("go depth 8");
    uci_wait_for_search(); return the last info line's nodes; };
  cold = run(true);
  // Precondition for the warm half: the table holds the root now.
  memcpy(&game, uci_game(), sizeof(game_t));
  REQUIRE(tt_get_entry(uci_tt(), &game.board) != nullptr);
  REQUIRE_EQ(run(true), cold);
  // F08 pinned as a property: the same FEN string skips set_position's reset.
  warm = run(false);
  REQUIRE_MESSAGE(warm != cold, "warm repeat " + warm + " equals cold " + cold);
  uci_shutdown();
```

Case 3, once `bench` exists (S189): `bench` in a capture, the `nodes` of the
last `info` line before each `bestmove` collected per position; then for the
last position of the bench list, `ucinewgame`, `position fen <that FEN>`, `go
depth <bench depth>` standalone, and `REQUIRE_EQ` of the two counts. Two `bench`
totals agreeing (S189's own case) proves determinism, not the per-position reset
-- positions 2 to n follow a search in both runs alike -- while a cold standalone
count equal to the in-bench count proves the reset. Read the list and the depth
from `command_bench` at the time.

Red first, both mutants by hand and reverted before the commit: comment out
`tt_reset(&tt)` in `command_ucinewgame` -- case 1 red at the limits where the
warm tree differs (5000 and 250000 on A, 250000 on B here), case 2 green; make
the body of `set_position`'s `if (initial_position != fen)` never run -- case 2
red on `cold_again`. Quote both in the stamp. Goldens: none -- every assertion
compares two runs in one process or asserts one inequality, and no number from a
run is written into the test, so DEC-142 owes nothing. INV-6, only if `src/` is
touched for the bench hook: `python3 tools/search_bench.py ./build/src/chesso 9`
and `... 12` against `.moltke.local.md`'s 121512 / 800769 / 62907 and 639228 /
3430710 / 367858, `c3d5` / `e2a6` / `d7c8q`. Debug self-play and a
`tools/mutation_check.py` mutant: not owed, no search, `make_move` or generator
change. Durations: `./build/tests/test_engine -tc='*ucinewgame*' -d` prints each
case's seconds (doctest's `duration` flag).

### 7. Measurement

No lane: nothing alters play. Commit message: `No functional change` if
`src/chesso.cpp` changed for the bench hook, no Bench line otherwise (tests and
docs only). Stamp: the forty pairwise counts identical, case 2's triple (cold,
cold again, warm) as measured on the workstation, both mutant reds, the Release
and Debug durations of the new cases, the `search_bench` counts if run.

### 8. Completion checklist

1. Gate: `cmake --build build -j8 && ctest --test-dir build -L fast
   --output-on-failure && cmake --build build-tune -j8 && ctest --test-dir
   build-tune -L fast --output-on-failure && ./clang-format.sh --check`; then
   `python3 tools/plan_prose_check.py --touches`.
2. `DEV_MANUAL.md` "Measure", beside the `tools/search_bench.py` block: the
   table is reset only when the FEN string differs from the previous one; two
   `go` on the same FEN in one process run the second warm; `search_bench.py` is
   safe because its three FENs differ; a repeated position, or a re-run to
   double-check a count, needs `ucinewgame` between (or a new process); `bench`
   sends it before every position; pinned by `tests/test_engine.cpp` "a
   repeated go depth is cold only across ucinewgame". "Test" needs no change
   unless it quotes a case count.
3. `MANUAL.md`: `ucinewgame` appears only in the command list and no wording
   changes, so no edit and the SURFACE golden is untouched (section 10 asks
   whether a sentence is wanted). `adocs/specs.md` is not in `touches:`: propose
   in the stamp one sentence beside the `tests/test_mate_carry.cpp` paragraph
   naming the two cases, for the coordinator.
4. Stamp: section 7's list, "no `src/` change" or the `No functional change`
   line, and the T1 note that the stop flag is cleared by the `go`.
5. `plan.md`, `status.md` and F08's `Status:` line in
   `adocs/audit/2026-09-04_test_review.md` (planned to closed) go through the
   coordinator.

### 9. Sources read

- https://api.github.com/repos/official-stockfish/Stockfish/issues/5376/comments
  -- fetched; the alternating totals and the unreset generation counter.
- https://raw.githubusercontent.com/official-stockfish/Stockfish/master/tests/reprosearch.sh
  -- not fetched (another engine's repository file); described from
  `adocs/testing_strategy.md` section 3.1, which fetched it.
- https://backscattering.de/chess/uci/ and
  https://www.wbec-ridderkerk.nl/html/UCIProtocol.html -- fetched; the
  `ucinewgame`, `go nodes`, `go depth` wording.
- https://www.chessprogramming.org/Transposition_Table -- fetched; the Aging
  paragraph. https://www.chessprogramming.org/UCI -- fetched; no command
  semantics on it. WebSearch over chessprogramming.org for reproducibility and
  determinism, 2026-09-05 -- nothing found; that no page exists is unverified.
- Repository: `adocs/audit/2026-09-04_test_review.md` F08; DEC-139, DEC-141,
  DEC-142, DEC-145; `adocs/testing_strategy.md` 3.1 and R8; the S189 and S193
  guides (sections 3 and 6; T1 and section 10); S179's 2026-09-05 amendment;
  `adocs/plan_done/S170_mate_line_across_searches.md` and
  `adocs/plan_done/S176_position_fen_short_forms.md` (stamp shape);
  `src/chesso.cpp`, `src/search.cpp`,
  `src/transposition_table.cpp`, `src/data_structures.hpp`, `src/uci.hpp`;
  `tests/test_engine.cpp`, `tests/test_mate_carry.cpp`, `tests/test_mate_pv.cpp`,
  `tests/test_search.cpp` "the same search twice gives the same answer",
  `tests/test_helpers.hpp`, `tests/CMakeLists.txt`; `tools/search_bench.py`;
  `DEV_MANUAL.md` "Test" and "Measure", `MANUAL.md` "Commands", `TOOLCHAIN.md`
  "The chess oracle", `adocs/specs.md` S170 and S093 paragraphs. The probe: a
  raw-UCI python driver run 2026-09-05 against `build/src/chesso` built from
  `18deccf`; its numbers are the ones above; not committed.

### 10. Questions deferred to the owner

1. The accepts has this step add the per-position `ucinewgame` to `bench`
   "otherwise" -- were S195 to run before S189 there would be no `bench`.
   Proposed reading: the order stands (S189 at entry 4, S195 at 18) and the
   clause is void; if the order changes, the reset goes into `command_test`'s
   loop, the only multi-position command at HEAD.
2. "every reported node count identical": the last `info` line's `nodes` sits
   below the budget when the final iteration aborts without a PV (2917 at `go
   nodes 5000`). Confirm the reading "the last `info` line's `nodes` and the
   `bestmove` line, compared pairwise", and whether `MANUAL.md`'s "`nodes` ...
   counting every iteration" wants a footnote -- a separate finding.
3. A negative control for case 1 is not in the accepts and not reliably
   observable (section 5); confirm it is not wanted.
4. `MANUAL.md` has no sentence on what `ucinewgame` resets. Adding one ("clears
   the transposition table and the carried mate line; a `position` alone keeps
   the table when its FEN string is the one already loaded") is outside
   `touches:` -- wanted or not.
