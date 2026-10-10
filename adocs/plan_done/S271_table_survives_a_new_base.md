id:         S271
goal:       the transposition table is cleared by `ucinewgame`, `clean-tt` and a Hash change and never by a new base FEN, and a clear of a table nothing has written to since the last clear is skipped
accepts:    (1) **A**: `src/chesso.cpp` `commit_position_base` no longer clears the table; the remembered base and the book re-arm are unchanged; (2) **red first**: a test that searches a position, then sends a later board of the same game as a bare `position fen` with no `ucinewgame`, and finds the entry the first search stored -- observed red on the parent, green after A; (3) the tests whose premise was the base clear are re-stated under DEC-264, not relaxed: `tests/test_engine.cpp` "a refused position command leaves the table alone" loses its precondition (a different base no longer clears) and is re-stated so a refused command is still shown to change nothing it could have changed; the comments in "a repeated go depth is cold only across ucinewgame" and "bench searches its last position cold" that cite the base clear are corrected, and each case still fails on the mutation it was written for; (4) **B**: `src/transposition_table.cpp` `tt_reset` skips its `memset` when nothing has written to the table since the last clear; the flag is invalidated before any write can happen -- the implementer states where and why that covers every writer, `go`, `bench` and the tools that drive the search; a test shows a clear after a search still clears, and B's remaining reach after A is counted on fastchess's command sequence: if it is nil in ordinary play, B is dropped and recorded as zero (CLAUDE.md rule 8); (5) **harness play unchanged** (INV-6): the `bench` total and `tools/search_bench.py` node counts and best moves identical to the parent, since `ucinewgame` and `bench` still clear; (6) `adocs/data/S271_replay.py` re-run on the result at depth 12 and Hash 16: `h_bare` within 0.2 % of `h_base` in total nodes, read with `adocs/data/S271_replay_read.py` and added to `adocs/data/S271_replay.md`; (7) `adocs/specs.md` and `MANUAL.md` describe the new rule -- MANUAL's "a new base still does all three" paragraph above all -- before any `test_uci_surface` refresh the change needs (SURFACE rule)
touches:    src/chesso.cpp, src/transposition_table.cpp, src/transposition_table.hpp, tests/test_engine.cpp, adocs/specs.md, MANUAL.md, adocs/data/S271_replay.md; amended at implementation, by the coordinator's ruling, for the INV-6 instrument that relied on the base clear to start each position cold: tools/search_bench.py, which now sends `ucinewgame` and waits for `readyok` itself, and DEV_MANUAL.md, whose warm-table paragraph stated that reliance; and adocs/data/S271_B_written_flag.diff, B's recipe kept with its recorded zero
excludes:   recognising a bare FEN as the continuation of the last position to keep the repetition history (rejected, DEC-264); a faster or threaded clear; the table layout and replacement, which are S119; the validation match, which is S272
decisions:  DEC-264, DEC-263, DEC-083
closes:     2026-10-08_performance-F04
blocks:
paused_by:
author:     Claude Opus 5.5 (subagent briefed by the coordinator, 2026-10-10)
done:       2026-10-10. **A new base no longer clears the table, and B is dropped as a recorded zero.** `commit_position_base()` lost its `tt_reset()`; the remembered base and the book re-arm are unchanged. New `tests/test_engine.cpp` "a later board sent as a bare fen keeps the table": red on the parent (`nullptr != nullptr`, 4 of 5 assertions), green after A. **`test`** got back the parent's per-entry clear: `tt_reset()` when the entry's FEN differs from `initial_position`. "a repeated test reads the same total" was red without it (4626 against 60450) and is green with it. The refused-position case is re-stated on the book, with every control inside it, and renamed "... leaves the base and the book alone". Its table check is removed, since its precondition is gone under DEC-264. S210's eager-commit mutant turns both checks red, a base-only refusal turns the base check red alone, and a load that pre-writes the base fails the played re-arm control. The comments in "a repeated go depth is cold only across ucinewgame" and "bench searches its last position cold" are corrected; a `ucinewgame` that stops clearing now kills each alone. All ten hand mutants were re-run on the final text. **B**: on fastchess's sequence after A, 20 games take 20 clears, 1 of them on a clean table (once per process) and 0 per game after it, so B is dropped; its recipe is `adocs/data/S271_B_written_flag.diff`. **The INV-6 instrument**: `tools/search_bench.py` now sends `ucinewgame` and waits for `readyok` before each position. A sweep of `tools/` and `tests/` found no other driver that needed it. **INV-6** against `16c8883`: the `bench` 4081329 and `bench 12` 1860699 streams are identical, and so is `tools/search_bench.py` at depth 9 (32932 / 70095 / 25178) and 12 (67792 / 280873 / 137893). `test 6` matches once, twice in one process (101223, 101223) and after a search (101215, 101223). **Replay** at depth 12, Hash 16: `h_base` 1088825787, `29348c5`'s M1 count to the node; `h_bare` x1.0008 (+0.080 %), 21 of 8653 best moves differ. The replacement claim is qualified for the 8-bit generation. `MANUAL.md`, `adocs/specs.md`, `DEV_MANUAL.md` and `adocs/data/S271_replay.md` are updated; no surface refresh. Both fast suites 43/43, format clean, prose checks 0 flagged. No second tier: UCI layer only. Closes 2026-10-08_performance-F04. Fast check (two rounds): the `test` per-entry clear the first sweep missed, the refused case's borrowed precondition, mutants predating the final text, two unlogged B rows, the generation-wrap qualification and MANUAL's new-base definition, all fixed before commit; tools/search_bench.py's cold start ruled in scope by the coordinator. B is dropped although DEC-264 named it: the owner is told.

## Why

A client that sends a board as a bare FEN, or that moves its base FEN after a
capture or pawn move, gets a cold table: `adocs/data/S271_replay.md`
measured +21.8 to +26.8 % nodes to the same depth for a bare FEN each move
and +11.9 to +14.4 % for the irreversible-move base, over 8653 positions --
about 28 to 60 Elo by conversion, not a verdict. The same engine with the
clear removed reads within 0.1 % of base-plus-moves in every cell. The UCI
text the repository ships gives new-game signalling to `ucinewgame`, and the
table's replacement already lets any entry from an earlier search be
overwritten (`src/transposition_table.cpp` `tt_store_entry`), so an entry
kept from another game cannot block a slot. fastchess and python-chess hold
the base fixed for a game, so the harness sees no change.

B was proposed against the three clears a game start from a book FEN pays
today. After A two of them are gone; what remains is a `ucinewgame` on a table
that is already clean, and its reach is counted rather than assumed.

## Lane

Agent work under DEC-260. UCI-layer change; it does not touch `make_move`,
the generator or the search, so DEC-141's second tier does not apply.

## As built

2026-10-10, on the workstation (i7-8700K, g++ 13.3, Release), worktree at
`16c8883`. `bench` printed the parent's 4081329 before any change.

### Deviations

- **B is dropped and recorded as zero**, which accepts (4) provides for: after
  A it skips one clear per engine process and none per game on fastchess's
  sequence (below). Its recipe is kept as `adocs/data/S271_B_written_flag.diff`,
  a file `touches:` did not list.
- **`test` clears per entry again, on the parent's own condition.** The
  coordinator's fast check found it, not the step's first sweep. A changed
  `test`'s output, and the clear restores exactly the parent's behaviour
  (section "The `test` command"). This is `src/` code beyond A, but it adds no
  behaviour the parent lacked.
- **The refused-position case is renamed and its table check removed.**
  Under DEC-264 no `position` clears the table, so that check could no longer
  fail on any refusal; it lost its precondition, as accepts (3) says. The rule
  is now carried by the two pieces of state a new base still owns, read
  through the book. Every control the case leans on is established inside it.
  The case's old title stays on one comment line so the citation in
  accepts (3) still resolves. It is now
  `tests/test_engine.cpp` "a refused position command leaves the base and the book alone".
- **`tools/search_bench.py` now starts each position cold itself, and
  `DEV_MANUAL.md`'s warm-table paragraph says so.** Both are outside the
  original `touches:`, which is amended. The tool relied on the base clear:
  its three FENs differed, so each `position` cleared the table. With A its
  second and third positions ran on a table holding the earlier positions'
  entries. The counts still read the parent's, but by a measured property, not
  by construction. Reported as a proposal, then ruled in scope by the
  coordinator, since S271 is the change that broke it. Section "The INV-6
  instrument" below.
- **No `test_uci_surface` refresh**: no surface string moved. The case's
  comment "The table is cleared per position" is about `bench`'s reset and
  still holds.

### A, red first

`src/chesso.cpp` `commit_position_base()` no longer calls `tt_reset()`. The
remembered base (`initial_position`) and the book re-arm (`still_in_opening`)
are unchanged, and so is the idempotence on the same FEN. The comments at
`load_position()`, `reset_for_new_game()` and `command_position()` that named
the table as a new base's were corrected. `reset_for_new_game()` keeps its own
`tt_reset()`, so `ucinewgame` and `bench` still clear.

New: `tests/test_engine.cpp` "a later board sent as a bare fen keeps the table".
It takes the FEN of `startpos moves e2e4 e7e5 g1f3 b8c6` off the engine's own
board, searches `startpos moves e2e4 e7e5` to depth 6, and requires the later
board's entry as a precondition. It then sends that board as a bare
`position fen` with no `ucinewgame`, and checks that the entry is still there.
**Red on the parent**:
`CHECK( tt_get_entry(uci_tt(), &game.board) != nullptr )`, values
`CHECK( nullptr != nullptr )`, logged `a new base FEN cleared the transposition
table`, 4 of 5 assertions passed (the preconditions). **Green after A**, 5 of 5.

### The `test` command

`command_test` loads each entry with `set_position()` alone. On the parent
that cleared the table whenever the entry's FEN differed from
`initial_position`: every entry but a first one already standing on its base.
With A it cleared nothing, so a second `test` in one process ran warm. The fast
check measured `printf 'test 6\ntest 6\nquit\n'` at 101223 then 6656, against
the parent's 101223 both times. `commit_position_base()` cleared the table
alone; the base and the book flag are still written by `set_position()`. So
`command_test` now runs `if (initial_position != entry.FEN) tt_reset(&tt);`
before `set_position()`. That is the parent's condition, not
`reset_for_new_game()`, which would also drop the proven mate line and move
the base to the start position, neither of which the parent did per entry.

New: `tests/test_engine.cpp` "a repeated test reads the same total", `test 5`
twice in one process. Its precondition is that the first run left an entry
for its last position, which the second run searches again. **Red on the
S271 tree before the fix** (the MT row below): `CHECK( 4626 == 60450 )`,
logged `a repeated [test] read 4626 against 60450`, 4 of 5 assertions passed.
**Green after the fix.**

The sweep of in-engine commands that load several positions. Callers of
`set_position()`, `load_position()` and `commit_position_base()`:

- `command_test`: fixed, above.
- `command_bench`: `reset_for_new_game()` before each position. That clears
  the table, sets generation 1 and drops the mate line, as on the parent,
  where the base clear in the following `set_position()` cleared a second
  time. Same state per position; its stream is identical.
- `reset_for_new_game()` (`ucinewgame`, `bench`): clears the table itself.
- `command_position`: one position per command; this is A.
- `tests/test_engine.cpp` "set_position keeps the old board when the FEN is
  bad": one position and no search.
- `tools/datagen.cpp`: none of the three. It holds its own `game_t` and table
  and calls `tt_reset()` per game.
- `tools/mate_trace.cpp` and `tools/node_share_census.cpp`: drive the UCI
  layer, covered by the tool sweep below.

### The re-stated cases, and the mutation each still kills

Hand mutants on `src/chesso.cpp`, run on the final test text: each was built,
run over seven table cases and reverted. Driver
`.tuning/coord/S271/mutants.py`, log `.tuning/coord/S271/mutants_final.log`.
M0, no mutation, is green on all seven.

| mutant | what it does | red |
|---|---|---|
| M1 | `tt_reset()` cut from `reset_for_new_game()` | "a repeated go depth is cold only across ucinewgame", 5014 against a cold 15713; "bench searches its last position cold", 508 against bench's 115233; "node-limited searches repeat across ucinewgame", 113 then 247; "ucinewgame puts the board and the table back" |
| M3 | `tt_reset()` ahead of `commit_position_base()`'s gate, so every `position` clears (F08 repaired silently) | "a repeated go depth ...", `15713 != 15713`; "a later board sent as a bare fen keeps the table" |
| MA | A undone | "a later board ..." only |
| MT | `command_test`'s restored clear cut (the S271 tree before the fix) | "a repeated test reads the same total", 4626 against 60450 |
| Mb | the S210 shape: `load_base` commits through `set_position()` | the refused case, both checks |
| Mc | every load writes `initial_position`, so no commit ever re-arms | the refused case, the played control |
| Mc2 | the refusal moves `initial_position` and nothing else | the refused case, the base check alone |
| Md | every load re-arms the book | the refused case, its same-base control |
| Md2 | the refusal re-arms the book and nothing else | the refused case, the book check (and the base check after it) |
| M5 | `bench`'s loop `reset_for_new_game()` cut | none: `bench 9` and bare `bench` streams byte-identical to M0's but for time and nps |

- **"a refused position command leaves the base and the book alone"**: it
  establishes its own controls, with OwnBook on and the embedded book loaded.
  1. The freshly armed book answers `startpos`: a hit, no `info score`.
  2. `startpos moves a2a3 h7h6`, a line the book has nothing for, switches the
     book off.
  3. `position startpos` on the same base keeps it off: a search.
  4. A *played* `position kiwipete` and then `position startpos` re-arms it.
     This is the event the base check watches for, and Mc fails exactly here;
     it was green on all six cases before.
  5. The book is disarmed again.
  6. Then `position kiwipete moves zzzz` is refused. A `go` on the board it
     left must search (the book flag), and so must `position startpos` + `go`
     (the remembered base).

  A guard object turns OwnBook off and runs `uci_shutdown()` however the case
  ends, so a failed REQUIRE does not leak the option into the next case.
- **"a repeated go depth is cold only across ucinewgame"**: comments corrected.
  The equality half is now killed by M1 alone; S195 needed both reset paths
  cut, because `set_position()` still cleared then. The inequality half was
  written for M3's hazard. Its old form, deleting `set_position()`'s gate, no
  longer clears anything, so the repair is now spelled as a `tt_reset()` on
  the `position` path, and the half kills it.
- **"bench searches its last position cold"**: comments corrected. M5 is still
  byte-identical, but now with no clear at all between positions: a probe
  answers only on a full key match, and no entry one bench position stores is
  read by another. The case is killed by M1, a `ucinewgame` that stops
  clearing, because the standalone search then runs warm.

### B: its reach after A, and the call

Counted, not timed. Two throwaway builds carried B plus counters in
`tt_reset()`, printed at `tt_free()`: one on `16c8883` (parent + B), one on
the S271 tree (S271 + B). The patcher is `.tuning/coord/S271/apply_b_counter.py`
and every row is in `.tuning/coord/S271/reach_B_final.log`. The driver was
`adocs/data/S271_replay.py`'s own command sequence: `uci`,
`setoption name Hash`, `isready`, then per game `ucinewgame`, `isready`,
`position fen <book> moves ...`, `go`. That is fastchess's shape (the replay
doc's log) with `restart` off, its default (`fastchess -help`), so engines are
reused across games. 20 games, depth 4, plus one bare `bench`:

| build, mode | `tt_reset` calls | on a clean table | bytes skipped by B, Hash 16 |
|---|---|---|---|
| parent + B, `base` | 59 | 40 | 503316480 |
| parent + B, `bare` | 814 | 40 | 503316480 |
| S271 + B, `base` | 20 | 1 | 12582912 |
| S271 + B, `bare` | 20 | 1 | 12582912 |
| parent + B, `bench` | 23 | 16 | 201326592 |
| S271 + B, `bench` | 8 | 1 | 12582912 |

At Hash 128 the calls and clean counts are the same; S271 + B skips 100663296
bytes there. After A, the one clean clear is the first `ucinewgame` after
`setoption name Hash`'s `calloc`: one per engine process, before any game's
clock runs. Every later `ucinewgame` follows a game's searches. B skips
nothing per game in ordinary play, so **B is dropped and recorded as zero**
(CLAUDE.md rule 8). A alone took the clears from 59 to 20 on `base` and from
814 to 20 on `bare`.

Where the flag went in the prototype, and why it covered every writer:
`tt_store_entry()` sets it inside its replace branch, before the entry's
fields are written. `tt_resize()` and `tt_reset()` clear it. Grep shows
`tt_store_entry()` is the only function that writes an entry; `tt_resize()`'s
`calloc` and `tt_reset()`'s `memset` only zero. Every writer goes through it:
`src/search.cpp`'s six stores, which `go`, `bench` and `test` reach through
`iterative_deepening_search()` and `tools/datagen.cpp` reaches through its own
search state, and the tests' direct stores. So no writer bypasses the flag.
Setting it in `tt_new_search()` instead would have missed a store made
without one: `tests/test_engine.cpp` stores into a fresh table directly.

### INV-6, harness play unchanged

Final tree against a throwaway Release build of `16c8883` (a `git worktree`
under `.ref-builds/`, removed after). Outputs are
`.tuning/coord/S271/inv6_final_*`.

- `bench`: the stream is identical but for time and nps, 121 lines, total
  4081329. `bench 12`: identical, 105 lines, total 1860699.
- `tools/search_bench.py`, fixed: depth 9 32932 c3d5 / 70095 e2a6 / 25178
  d7c8q, depth 12 67792 c3d5 / 280873 d5e6 / 137893 d7c8q, on both builds.
- `test 6`, with time, nps and hashfull stripped, every `info` and `bestmove`
  line and the total identical in three sequences:
  - once in a fresh process: 101223.
  - twice in one process: 101223 and 101223.
  - after `position startpos` + `go depth 8`, twice: 101215 then 101223. The
    first entry runs warm on its own base, on the parent and here alike.

### The INV-6 instrument, made cold by construction

`tools/search_bench.py` now sends `ucinewgame` and `isready` before each
position, and waits for `readyok` before `position` and `go`. That puts the
table's clear before the clock on both sides of any comparison. Before S271,
a FEN differing from the last cleared the table inside `position`, possibly
after the tool had started its clock. The docstring says why and cites S271
and DEC-264. Its counts at depths 9 and 12 equal the unfixed tool's
(32932 / 70095 / 25178 and 67792 / 280873 / 137893) on `16c8883` and on S271
alike.

**The sweep.** Every UCI driver in `tools/` and `tests/` that sends several
positions through one engine (grep `position fen`, `position startpos`,
`ucinewgame`), and whether a cold table per position is its contract:

- Fixed: `tools/search_bench.py`, timing and per-position counts.
- Cold by construction already:
  - `tools/node_share_census.cpp`: in-process `ucinewgame` before each
    position.
  - `tools/depth_vs_eval.py`: `new_game()`, which is `ucinewgame` and a wait
    for `readyok`, before each position, on both engines.
  - `tools/spsa_driver.py`: one process per position.
  - `tests/test_engine.cpp`'s node-count cases: `ucinewgame`, `bench`'s own
    reset or `test`'s per-entry clear wherever cold is asserted.
  - `tests/test_mate_pv.cpp`, `tests/test_mate_breadth.cpp`: `ucinewgame` per
    case.
- Warm by contract, left as they are:
  - `tools/analyse_game.py`: the reference oracle, Stockfish by default,
    scoring a game's positions. It is analysis, and Stockfish never cleared
    on a FEN.
  - `tools/error_profile.py`: one `ucinewgame` per game and that game's
    positions warm, by design.
  - `tools/mate_trace.cpp`: replays a game into one table on purpose (S171).
  - `tests/test_mate_carry.cpp`: one `ucinewgame` per game, warm between
    plies on purpose.
  - `adocs/data/S271_replay.py`: its `base` and `bare` modes are the
    measurement.
- No cold contract:
  - `tools/timer_race_stress.py`: the same FEN every round, a timer-race
    stress test with no counts.
  - `tests/test_audit_go_infinite.cpp`: whether `stop` is answered, no counts.
  - `tests/test_audit_fen_semantics.cpp`: FEN loading and refusals, no counts.
  - `tests/test_uci_surface.cpp`: surface strings and `info` fields; its node
    comparisons are `bench` totals, which `bench`'s reset keeps cold.
- Hits in comments only: `tests/test_search.cpp`, `tests/test_movegen.cpp`,
  `tools/mutants/S097_singular_extension.py`.

### The replay, depth 12, Hash 16

`adocs/data/S271_replay.md`, new dated section. On the S271 engine before the
`test` fix, which no replay command reaches, `h_base` reads 1088825787, the
count `29348c5` read on the M1, to the node. `h_bare` reads 1089692590,
**x1.0008 (+0.080 %)**, median x1.000, against the 0.2 % the accepts allow.
21 of 8653 best moves differ, the repetition residue DEC-264 accepts. The
json outputs are in `.tuning/coord/S271/replay/`.

### A claim qualified

DEC-264 and the step's Why say that a kept entry cannot block a slot, because
the replacement rule lets any older entry be overwritten. Not quite.
`tt_entry_t::generation` and the table's are `uint8_t`, and `tt_new_search()`
skips 0, so the counter cycles 1 to 255. An entry untouched for exactly 255
searches, or a multiple, reads as the current search's own, and
`tt_store_entry()`'s depth preference then refuses a shallower store.
Verified with a throwaway program over the engine library
(`.tuning/coord/S271/wrap_check.cpp`): a depth-9 entry refused a depth-2 store
after 255 searches and after no other count from 1 to 256. That was already
true of a long game on one base. The `commit_position_base()` comment and
`MANUAL.md` now say it; `decisions.md` is the coordinator's.

### Docs

- `MANUAL.md`:
  - The "a new base still does all three" paragraph now says a refused
    `position` moves neither the base nor the book.
  - It defines the remembered base in user terms. It is the start position at
    startup and after `ucinewgame`, the last position `bench` or `test`
    searched, and otherwise the base of the last `position` that played in
    full. A new base is a `position` whose base differs from it.
  - A new paragraph says no `position` clears the table, and names what does:
    `ucinewgame`, `clean-tt`, `bench`, an accepted `Hash`, and `test` before
    each entry that is a new base. It qualifies the replacement claim for the
    8-bit counter and says what to send for a cold search.
- `adocs/specs.md` (the coordinator's file, wording proposed): the S210
  sentence drops "the transposition table unreset". A new paragraph, "No
  `position` clears the transposition table since 2026-10-10, S271", names
  `test`'s per-entry clear and its guard case.
- `DEV_MANUAL.md`: the warm-table paragraph says `bench` and
  `tools/search_bench.py` both send `ucinewgame` before each position, and why
  the tool started to in S271.
- `tools/search_bench.py`'s docstring: one paragraph on why each position
  starts with `ucinewgame` and `readyok`. Its "Depth 9 is 3136397 nodes"
  sentence is stale (128205 here) and was left alone, being outside the task.
- `adocs/data/S271_replay.md`: the dated section.
- `README.md`: owner-written, not touched.

### Suites

Both fast suites 43/43, `./clang-format.sh --check` clean under
`CLANG_FORMAT_MAJOR=22`. `tools/plan_prose_check.py`: `--citations` 0
flagged over 39 files, `--touches` 0 over 39, `--params` clean. No second
tier: the step does not touch `make_move`, the generator or the search
(Lane).

### Proposed `done:` stamp

2026-10-10. **A new base no longer clears the table, and B is dropped as a
recorded zero.** `commit_position_base()` lost its `tt_reset()`; the
remembered base and the book re-arm are unchanged. New `tests/test_engine.cpp`
"a later board sent as a bare fen keeps the table": red on the parent
(`nullptr != nullptr`, 4 of 5 assertions), green after A.

**`test`** got back the parent's per-entry clear: `tt_reset()` when the
entry's FEN differs from `initial_position`. "a repeated test reads the same
total" was red without it (4626 against 60450) and is green with it.

The refused-position case is re-stated on the book, with every control inside
it, and renamed "... leaves the base and the book alone". Its table check is
removed, since its precondition is gone under DEC-264. S210's eager-commit
mutant turns both checks red, a base-only refusal turns the base check red
alone, and a load that pre-writes the base fails the played re-arm control.
The comments in "a repeated go depth is cold only across ucinewgame" and
"bench searches its last position cold" are corrected; a `ucinewgame` that
stops clearing now kills each alone. All ten hand mutants were re-run on the
final text.

**B**: on fastchess's sequence after A, 20 games take 20 clears, 1 of them on
a clean table (once per process) and 0 per game after it, so B is dropped; its
recipe is `adocs/data/S271_B_written_flag.diff`.

**The INV-6 instrument**: `tools/search_bench.py` now sends `ucinewgame` and
waits for `readyok` before each position. A sweep of `tools/` and `tests/`
found no other driver that needed it.

**INV-6** against `16c8883`: the `bench` 4081329 and `bench 12` 1860699
streams are identical, and so is `tools/search_bench.py` at depth 9 (32932 /
70095 / 25178) and 12 (67792 / 280873 / 137893). `test 6` matches once, twice
in one process (101223, 101223) and after a search (101215, 101223).

**Replay** at depth 12, Hash 16: `h_base` 1088825787, `29348c5`'s M1 count to
the node; `h_bare` x1.0008 (+0.080 %), 21 of 8653 best moves differ.

The replacement claim is qualified for the 8-bit generation. `MANUAL.md`,
`adocs/specs.md`, `DEV_MANUAL.md` and `adocs/data/S271_replay.md` are updated;
no surface refresh. Both fast suites 43/43, format clean, prose checks 0
flagged. No second tier: UCI layer only. Closes 2026-10-08_performance-F04.

### Proposed commit

```
Keep the table when position sends a new base FEN (S271)

A client that sends the board as a bare FEN, or moves its base after a
capture, searched every move on a cold table, because
commit_position_base() cleared the table on every new base. S271's
replay measured +21.8 to +26.8 % nodes to the same depth for a bare FEN
each move. DEC-264 gives new-game signalling to ucinewgame, as UCI
does. A new base now moves the remembered base and re-arms the book,
and only ucinewgame, clean-tt, bench, an accepted Hash setoption and
test clear the table. Replayed at depth 12 and Hash 16, a bare FEN each
move now reads +0.080 % against base plus moves.

The test command loads its positions through set_position() alone, and
had relied on the base clear: without it a second test in one process
ran warm, 4626 nodes against 60450 at depth 5. It now clears before
each entry on the parent's own condition, a FEN differing from the
remembered base, so its output matches the parent's in a fresh process,
on a repeat and after a search.

B, skipping a clear of a clean table, is dropped as a recorded zero.
After A, on fastchess's command sequence, it skips one clear per engine
process and none per game.

tools/search_bench.py, INV-6's own instrument, had relied on the base
clear to start each position cold. It now sends ucinewgame and waits for
readyok before each position, which also puts the clear before its
clock on both sides of a comparison. Its counts at depths 9 and 12 are
unchanged on 16c8883 and on S271. A sweep of tools/ and tests/ found no
other driver whose contract is a cold table per position and that
lacked one.

Harness play is unchanged (INV-6): fastchess sends ucinewgame and holds
the base fixed, and bench, bench 12, tools/search_bench.py at depths 9
and 12, and test read the parent's counts and best moves. The
refused-position case is re-stated on the book, which a new base still
owns, and two cases' comments are corrected. MANUAL qualifies the
replacement claim for the 8-bit generation. Closes
2026-10-08_performance-F04.

No functional change
```
