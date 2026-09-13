id:         S210
goal:       the seven low-severity engine defects of the 2026-09-10 audit are closed as one batch -- the halfmove clock cannot wrap, a full history refuses rather than answers `bestmove 0000`, `go infinite` waits for `stop`, `movestogo 0` means sudden death, the first iteration can be stopped, quiescence scores a dead position as a draw, and the same-coloured-bishops comment tells the truth -- with F02 and F03 of the 2026-09-04 report and two `MANUAL.md` sentences folded in (DEC-181, DEC-184)
accepts:    **F17** the halfmove clock cannot wrap -- either `uint16_t` in `board_t` and `history_entry_t` with the record size cost stated, or saturation at 255 with the reason written at the increment sites -- and a red-first case plays 256 reversible plies through `position startpos moves` and asserts the fifty-move test still fires and the repetition window is still open; **F18** a `position` line whose moves would overflow `HISTORY_MAX_SIZE` is refused in the S176 shape with the board unchanged, the comment at `make_move`'s refusal in `src/bitboard.cpp` stops saying the search can never reach it, and `MANUAL.md` states the bound; **F19** `go infinite` prints `bestmove` only after `stop` whatever else the line carries: `command_go` in `src/chesso.cpp` pre-seeds `depth = MAX_DEPTH` and keeps a `nodes` budget, and `infinite` clears neither -- measured 2026-09-12 on the Release binary, `go infinite depth 1` and `go infinite nodes 1` both answer before `stop`, while `movetime` and the clock tokens already yield to `infinite` and answered after it -- so `infinite` clears every finite stop condition, red-first for `go infinite nodes 1` as well as `go infinite depth N`, and `go depth N` and `go nodes N` without `infinite` are unchanged; this also closes the still-open `2026-09-04_adversarial-F01` root-collapse case; **F20** `movestogo 0` is treated as no `movestogo` at all, as `src/uci.hpp`'s own comment and S089 state, with a red-first case asserting the allocation equals the sudden-death one; **F21** the first iteration honours `stop` and the hard timer -- `iterative_deepening_search` installs the real signal before the first `search()` -- while a move is still produced on an abort at depth 1, red-first through `tools/timer_race_stress.py` or a direct case, and the p99 depth-1 wall time is re-measured and recorded; **F22** quiescence in `src/search.cpp` returns `DRAW_SCORE` where a capture leaves `is_insufficient_material` true, and **the reach is counted before any match is booked**: over the S198 calibration PGN's positions, or a 1000-game sample, how many quiescence captures land on a dead position -- F22 lands in its own commit with `Bench: <nodes>` (it alters the tree), and a count near zero discharges its SPRT by census -- DEC-107's precedent, stated in the stamp -- while a larger one owes one `--nonreg` SPRT for that commit alone; F20 and F21 alter play only under inputs no harness here sends -- `movestogo 0`, and a hard limit that falls inside a depth-1 iteration of about a millisecond -- so each states its reachability in the stamp and is discharged node-identical at fixed depth, the other four being node-identical by construction, all in commits marked `No functional change`; **F23** the comment at `is_insufficient_material` stops justifying the same-coloured-bishops omission as "not by the laws" (FIDE 5.2.2 says otherwise) and names the accepted behaviour and its record instead; also decided here: whether the `go` tokens `std::stoll` reads in `command_go` get S209's whole-token rule, and the decision is written; every UCI-visible change is in `MANUAL.md` and `adocs/specs.md` before `test_uci_surface` is refreshed (SURFACE); Debug self-play four rounds at 4+0.04, `Assertion` grepped, before completion (DEC-141), because F22 touches the search; **2026-09-04 F02** a `moves` token that does not parse or is not legal refuses the whole `position` command in the S176 shape -- the board unchanged from before the command and one `info string refused [position moves] <token>: <why>` line on the UCI channel in both builds -- with a red-first case in `tests/test_engine.cpp` that sends a legal prefix, a bad token and a legal suffix and asserts the board is the pre-command one; **2026-09-04 F03** the root's answer and move order after an aborted iteration no longer rest on the root's table entry having survived: either the previous iteration's best move is carried in the search state and used for both, or the premise is asserted and the number of aborted iterations whose root entry is gone is counted over the F22 census sample and recorded, and the comment in `src/chesso.cpp` states what is enforced; **DEC-184** `MANUAL.md` gains one sentence saying what `ucinewgame` resets, traced to `reset_for_new_game` in `src/chesso.cpp`, and one saying that `nodes` on the last `info` line can be under the `go nodes` budget when the final iteration aborts before it has a line (S195 measured 2917 of 5000)
touches:    src/data_structures.hpp, src/bitboard.cpp, src/chesso.cpp, src/search.cpp, src/uci.hpp, tests/test_engine.cpp, tests/test_search.cpp, tests/test_uci_surface.cpp (golden), MANUAL.md, adocs/specs.md, adocs/data/
excludes:   any change to the draw rules' semantics beyond the sites named -- the repetition rule is S207; `make_null_move`'s dead assert at a full history, unreachable while null move pruning forbids two passes in a row, which is recorded here and not repaired; the mate-carry and mate-PV instruments
decisions:  DEC-170, DEC-171, DEC-181, DEC-184
closes:     2026-09-10_adversarial-F17, 2026-09-10_adversarial-F18, 2026-09-10_adversarial-F19, 2026-09-10_adversarial-F20, 2026-09-10_adversarial-F21, 2026-09-10_adversarial-F22, 2026-09-10_adversarial-F23, 2026-09-04_adversarial-F01, 2026-09-04_adversarial-F02, 2026-09-04_adversarial-F03
blocks:
paused_by:
author:     Opus 5 subagents briefed by the coordinator, one per half (DEC-185, DEC-199); any SPRT is the coordinator's; started 2026-09-13 14:50 on the idle machine
done:

## Why this exists, and why it sits behind the strength steps

Seven low findings of `2026-09-10_adversarial`, Part E, F17 to F23, each
verified by its reviewer. None fires in an adjudicated match here and none
moves a reported score in ordinary play, which is why DEC-171 schedules them
behind the next strength step rather than first: the BUGS rule's "before
anything else starts" is scoped by reach, on the owner's instruction of
2026-09-11. They are still bugs and they are still fixed -- as one batch, in
the daytime between two night runs.

- **F17** `halfmove_clock` is a `uint8_t` and wraps at 256; `load_FEN` refuses
  a clock above 255 with a comment naming exactly the harm the increment then
  produces. Both consumers fail past the wrap: the `>= 100` draw test stops
  firing and the repetition window collapses to `min(0, size)`.
- **F18** at 4999 plies the root's own `make_move` refuses, every move is
  skipped, `first_legal_move()` cannot rescue it because it calls `make_move`
  too, and the engine answers `bestmove 0000` with 22 legal moves on the board.
- **F19** `command_go` pre-seeds `depth = MAX_DEPTH` and `infinite` does not
  clear it, so `go infinite depth N` returns without `stop`, against
  `UCI.txt`'s "Do not exit the search without being told so in this mode!";
  the same mechanism is `2026-09-04_adversarial-F01`'s root-collapse case.
- **F20** `movestogo 0` is clamped to 1, spending 46 % of the clock on one move
  (4.64 s of 10 s measured), where `src/uci.hpp` says 0 means sudden death.
- **F21** `state.stop = &never_stop` for the first iteration, so depth 1 ignores
  `stop` and the hard timer; median 0.56 ms and p99 1.04 ms over 400 corpus
  positions, 254 ms on a pathological `STATUS_VALID` board against a 100 ms
  clock -- a forfeit, latent.
- **F22** `is_insufficient_material` has one call site, in `negamax_at`; a
  capture inside quiescence that leaves KvK, KNvK or KBvK scores -103, +190,
  +235 instead of 0. The interior node catches it one ply later, which is why
  the census comes before the match.
- **F23** the comment at `is_insufficient_material` says same-coloured bishops
  are "drawn in practice but not by the laws"; FIDE 5.2.2 and 6.9 say the
  family is dead. The behaviour stays as `2026-08-14_test_review-F05`
  accepted and S067 pinned; the justification is what is wrong.

## Cost

Agent work, half a day with the tests; the F22 census is a script over a PGN
already on disk. One `--nonreg` SPRT only if the census says the changed class
is reached -- state the count and the decision before booking it (DEC-143).

## Amended 2026-09-12, DEC-197: F19 is wider than its depth reproducer

The 2026-09-12 audit (`adocs/audit/2026-09-12_adversarial.md`, prior-finding
reassessment) re-triggered F19 with `go infinite nodes 1`: `bestmove a2a3`
before `stop`. The coordinator re-ran it and its neighbours the same day on
the Release binary built from `ecdfadb`'s source: `depth 1` and `nodes 1`
answer before `stop`; `movetime 100` and `wtime 50 btime 50` answer after it,
because `command_go`'s infinite branch already takes precedence over both
(`mate` is not parsed as a limit at all). The accepts' F19 clause names all
four so the fix is not scoped to the one token each report happened to use.

## First half landed 2026-09-13 15:16

Everything in `accepts:` except **F22**, which is the second half and carries
its own commit with a `Bench:` line. Parent `c9a3267`; this half is
`No functional change`.

### Per clause: what landed, and the red that was observed first

Every red was observed on a worktree of `c9a3267` carrying **only the test
files** plus the two `#define`s and the one unused `search_state_t` field the
new cases name, so nothing that fixes anything was present
(`.tuning/coord/s210_parent`).

- **F17, the halfmove clock cannot wrap.** **Saturation at 255**, not
  `uint16_t`, and the reason is written at `board_t::halfmove_clock` in
  `src/data_structures.hpp` and at both increment sites -- `make_move_impl`
  and `make_null_move` in `src/bitboard.cpp`, each now `if (clock <
  HALFMOVE_CLOCK_MAX)`. **Why not the wider field**: the same field is in
  `history_entry_t`, one per ply of a `HISTORY_MAX_SIZE` array, where the
  record is **exactly 16 bytes**; widening any field of it pushes the record to
  24 under 8-byte alignment, `history_t` from 80 KB to 120 KB, and -- the half
  that costs nodes -- `classify_repetition()` from 4 entries a cache line to
  2.67 on a walk it runs at every node. Saturation buys the same correctness
  for one predictable branch in `make_move`. What the ceiling costs is a FEN
  field reading 255 where the line played 300, in a position the fifty-move
  rule called dead 155 plies earlier. `load_FEN` still **refuses** a clock
  above 255 rather than saturating, and its comment says why.
  *Red*: `tests/test_engine.cpp` "256 reversible plies do not wrap the halfmove
  clock" -- `CHECK( 0 >= 100 )`, `CHECK_EQ( 0, 255 )`, and the second consumer
  exercised rather than argued, `classify_repetition` returning `NONE` for a
  position that had occurred 64 times.

- **F18, a `position` line too long is refused.** The bound is
  **`POSITION_MAX_PLIES` = `HISTORY_MAX_SIZE - MAX_PLY - 1` = 4871**, checked in
  `command_position`'s moves loop before each `try_move`. **Refusing only what
  `make_move` refuses does not close the finding**: the last move it accepts
  leaves the history one entry from the end and the search still cannot push a
  ply, so the command keeps `MAX_PLY` of the stack for the search instead. The
  comment at `make_move_impl`'s refusal now says the search *can* reach it and
  why the old "bounded by MAX_PLY" was false. `MANUAL.md` states the bound and
  the arithmetic.
  *Red*: on the parent binary, `position startpos moves <4999 knight-shuffle
  plies>` then `go depth 1` answers **`bestmove 0000`** on
  `r1bqkbnr/pppppppp/2n5/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 135 2500`; the same
  input on this tree answers `info string refused [position moves] c6b8, the
  move stack is full at 4871 plies` and then `bestmove e2e4` from the start
  position. The case is "a moves list past the history bound is refused and
  still answers", 5 of 10 assertions red.

- **F19, `go infinite` waits for `stop`.** Two halves, both needed.
  `command_go`'s infinite branch now clears **`depth`, `nodes`, `movetime` and
  all four clock fields**, and `iterative_deepening_search` **does not return
  when the depth loop ends while `conf.infinite`** -- a 1 ms poll on
  `stop_search_signal`, which is what closes `2026-09-04_adversarial-F01`'s
  collapsed roots, where MAX_DEPTH is reached in under a millisecond.
  `tests/test_audit_go_infinite.cpp` -- written by the 2026-09-04 audit,
  deliberately unregistered -- is **registered** in `tests/CMakeLists.txt` and
  gains two cases: nine `infinite` lines including both token orders, and the
  control that a finite limit with no `infinite` still answers on its own.
  *Red*: 9 of 29 assertions, verbatim -- `[go infinite depth 1] answered
  [bestmove e2e4] before stop`, `[go infinite nodes 1] answered [bestmove
  a2a3]`, `[go infinite nodes 1000] answered [bestmove d2d4 ponder d7d5]`,
  `[go depth 1 infinite]`, `[go nodes 1 infinite]`, plus the audit's three
  collapsed roots.

- **F20, `movestogo 0` is sudden death.** One bound in `command_go`:
  `pop_int(..., "go movestogo", ..., 0, int_max)` where it was `1, int_max`.
  *Red*: "movestogo 0 buys the sudden-death allocation, not movestogo 1" --
  `no movestogo 63 ms, movestogo 0 1877 ms, movestogo 1 1919 ms` on a 2 s
  clock. The case asserts against `compute_search_time_budget`'s own two
  allocations and carries the `movestogo 1` measurement as the precondition
  that makes the claim separable.

- **F21, the first iteration honours `stop` and the hard timer.**
  `iterative_deepening_search` installs `&stop_search_signal` before the loop;
  the local `never_stop` and the mid-loop swap are gone.
  *Red*: "the first iteration honours stop and the hard timer", 2 of 6
  assertions, both the `stop` half and the timer half. Command-line A/B on
  `q1q1q1q1/1q1q1q1k/8/8/8/8/1Q1Q1Q1K/Q1Q1Q1Q1 w - - 0 1` (python-chess
  `Status.VALID`):

  | binary | line | result |
  |---|---|---|
  | `c9a3267` | `go movetime 100000` + immediate `stop` | 13.5 ms, `depth 1`, 42371 nodes, `bestmove d2h6` |
  | `c9a3267` | `go movetime 1` | 13.8 ms, `depth 1`, 42371 nodes |
  | this tree | `go movetime 100000` + immediate `stop` | 5.9 ms, `depth 0`, 4096 nodes, `bestmove b2b7` |
  | this tree | `go movetime 1` | 6.2 ms, `depth 0`, 6144 nodes, `bestmove b2b7` |

  **p99 depth-1 wall time, re-measured** (`adocs/data/S210_depth1_latency.py`,
  400 distinct FENs from `S018_raw.tsv`, one engine process, `ucinewgame` +
  `position fen` + `go depth 1`, wall clock to the `bestmove` line so the pipe
  is inside the number): **median 1.36 ms, p95 2.20, p99 3.16, max 5.57** on
  this tree against **1.33 / 1.91 / 2.99 / 5.72** on `c9a3267` -- the same
  engine, and the difference is the machine. These are **not** comparable to
  the audit's 0.56 / 1.04 / 1.34: that measurement did not include the round
  trip, and the method is stated here so the next one can be. The exposure
  argument survives either reading -- an abort inside a 1.4 ms iteration fires
  in no game at 8+0.08.
  **The fix widens the stale-timer race S163 closed**, since depth 1 now reads
  the flag: `taskset -c 0 tools/timer_race_stress.py ./build/src/chesso
  --seconds 45` reports **21297 iterations, 0 hits, TIMER-RACE-CLEAN**.

- **F23, the same-coloured-bishops comment.** Rewritten at
  `is_insufficient_material` in `src/bitboard.cpp`: the bishop family is dead
  under **FIDE Article 5.2.2** and a flag-fall in it is a draw under **6.9**, so
  the old "drawn in practice but not by the laws" was false; knight against
  knight and two knights against a bare king are named as the family the old
  argument does hold for. The omission is named as accepted behaviour with its
  record -- `2026-08-14_test_review-F05`, pinned by S067's "which material can
  still mate" -- and S067's own ground is repeated: the function feeds
  `DRAW_SCORE` at every node above the root, so changing the answer is an SPRT
  and not an edit. **No behaviour changed and no test moved.**

- **2026-09-04 F02, a `moves` token that does not play.** The whole `position`
  command is now **atomic**: `command_position` saves `game` on entry, and every
  refusal -- the two FEN ones included -- restores it, so the board is the one
  from *before the command* and not merely the one before the bad token. Three
  shapes, one `info string` line, on the UCI channel in both builds.
  *Red*: "a moves token that does not play refuses the whole command", three
  subcases, each starting from `position startpos moves d2d4 d7d5` so a refusal
  that reset to `startpos` could not pass. First subcase verbatim:
  `REQUIRE( r1bqkbnr/1ppp1ppp/p1n5/4p3/B3P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 1 4
  == rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2 )` -- the
  engine standing five plies down the line it was sent, with `zzzz` skipped.

- **2026-09-04 F03, the root after an aborted iteration.** **The carried best
  move**, not the assertion route, so the census stays the second half's.
  `search_state_t::root_move_hint` holds the last **completed** iteration's best
  move; `negamax_at` reads it **at ply 0 only and only when the table returned
  nothing**, one line beside the `tt_move` copy-out. Ordering is all it can
  change -- `tt_move`'s only other consumer is the staged-generation test, which
  is ordering too -- and the hint is a move the previous iteration returned from
  this position, so it is legal here. The comment in `src/chesso.cpp` now states
  what is enforced and why the old sentence rested on the replacement rule.
  *Red*: `tests/test_search.cpp` "the root's first move survives a lost table
  entry" -- `CHECK_EQ( 1058142, 2088 )` -- driven through `negamax_probed` at
  ply 0 with the table wiped, which is what an eviction leaves behind. The case
  also pins that the hint reaches ply 0 and nowhere else.

- **The `std::stoll` decision, which the `accepts` left to this step: yes.**
  Every `go` number goes through `pop_whole_integer`, `std::from_chars` with
  `ptr == last`, and answers S209's two shapes -- `info string refused [go
  <token>] <value>, not an integer` and `..., out of range` -- on the UCI
  channel in both builds. The refused token is **dropped and the rest of the
  line still read**, because a `go` that answers nothing hangs a GUI where a
  `go` on a default does not; a whole integer outside a field's range is still
  **clamped**, unchanged. `bench` and `test` share the helper and name
  themselves `[bench depth]` and `[test depth]`. The reason to say yes rather
  than leave it: `wtime 0x1000` was a clock of **0**, which drops the search
  onto the 1 s no-limit fallback -- a forfeit class, silent in the shipped
  binary, on the line that decides how much clock a move gets, which is exactly
  why S209 did it for `Hash`.
  *Red*: "go refuses a limit that is not an integer in full", 8 of 16
  assertions, one per malformed token. Its control, "go still reads a whole
  integer, and still clamps its range", is green on both trees by construction.

- **DEC-184's two `MANUAL.md` sentences.** What `ucinewgame` resets, traced to
  `reset_for_new_game` in `src/chesso.cpp` clause by clause (joins the search,
  start position with an empty history, table cleared, proved mate line
  dropped, book re-opened) and what it does **not** reset (anything set by
  `setoption`; the killer, history and counter-move tables are per `go`). And
  that `nodes` on the last `info` line can be under a `go nodes` budget when the
  final iteration aborts with no line to print -- 2917 of 5000 measured, S195.

### Neutrality, INV-6

`bench` **7111579**, the parent's total to the node, so the commit carries
`No functional change` and no `Bench:` line. `tools/search_bench.py` against a
`c9a3267` worktree build, all six rows identical:

| depth | midgame | kiwipete | tactical |
|---|---|---|---|
| 9 | 47635 `c3d5` | 213916 `e2a6` | 26130 `d7c8q` |
| 12 | 141455 `c3d5` | 1038779 `d5e6` | 175684 `d7c8q` |

**What is node-identical by construction and what is not.** F17, F18, F02, F19
and the `stoll` rule are on command paths no search reaches. F23 is a comment.
**F20 and F21 alter play** only under inputs no harness here sends -- a GUI that
writes `movestogo 0`, and a hard limit that falls inside a depth-1 iteration of
about 1.4 ms -- and **F03 alters play** only where a root slot is evicted inside
an iteration the hard timer then aborts, estimated at 6e-5 per iteration by the
report and never reproduced. All three are discharged node-identical at fixed
depth above, which is what `2026-09-04_adversarial-F03` itself pre-registered
as its discharge, and none is reachable from `bench` or from `search_bench`: no
row of either moved.

### Second tier, DEC-141

`make_move`, `make_null_move` and the search's stop plumbing are touched, so the
**Debug self-play** is owed and was run twice on `build-debug`, `-concurrency 8`
at 4+0.04 on `noob_3moves.epd`: **8 games** (the DEV_MANUAL recipe's four
rounds) and then **80 games** for coverage. `grep -c Assertion` on both the
trace log and the console: **0 and 0**. `grep -c disconnect`: **0**. No time
forfeit reported in either. Logs under `.tuning/coord/s210_debug_selfplay*.
{out,log,pgn}`. No pruning, reduction or extension rule was added, so DEC-141's
mutant clause does not bind. `tools/gate_extra.sh` is the coordinator's.

### Gate

`ctest -L fast` **38/38 in `build` and 38/38 in `build-tune`** (37 before; the
registered `test_audit_go_infinite` is the new one), `./clang-format.sh --check`
clean under `CLANG_FORMAT_MAJOR=22`.

### Docs, and what the coordinator still owes

`MANUAL.md` carries every UCI-visible change **before** `test_uci_surface` was
refreshed (SURFACE): the `moves` refusals and the 4871-ply bound, the halfmove
saturation, `go infinite`, `movestogo 0`, the two `go` refusal shapes, the
`ucinewgame` paragraph and the `nodes` sentence. `test_uci_surface`'s
`expected_refusal_templates` gains five lines and the MANUAL.md case holds them
verbatim. `DEV_MANUAL.md` **checked, no change needed**: its bench-signature
line reads `At S109: 7111579`, which is still the signature, and it says nothing
about the moves loop, the stop plumbing or the history bound. `README.md`
untouched, human-owned.

**`adocs/specs.md` is coordinator-owned and is the one document this half did
not write.** The wording proposed for it is in the subagent's report, five
paragraphs: the `position moves` rule beside S176's FEN paragraph, the
`POSITION_MAX_PLIES` bound, the halfmove saturation, `go infinite`'s two halves
with `movestogo 0`, and the `go` whole-token rule beside S209's `Hash`
paragraph.

### Beyond `touches:`, noted rather than hidden

`src/uci.hpp` (the `pop_int` / `pop_u64` declarations and their new comment),
`tests/CMakeLists.txt` and `tests/test_audit_go_infinite.cpp` (the audit's own
file, registered and extended), and `adocs/data/S210_depth1_latency.py` (new;
`adocs/data/` is in `touches:`). No `git add` and no commit: the coordinator's.

### 2026-09-13 15:55, after the fast check

Six findings over the first half, repaired before the landing commit. The two
real ones each carry an observed red; the four trivial ones are comments,
names and one documented shape. Still `No functional change`: `bench` **7111579**
and `search_bench` at depth 9 reads **47635 `c3d5` / 213916 `e2a6` / 26130
`d7c8q`**, the same three rows the half landed with.

- **F-1, real: `tests/test_engine.cpp`'s "a one node search still answers with
  a legal move" could no longer fail.** `position startpos` leaves
  `stop_search_signal` set through `stop_and_join_search()`, and F21 now points
  `state.stop` at it from iteration 1, so the direct
  `iterative_deepening_search` call aborted on the stale flag at its first
  `check_limits()` and not on the one-node budget -- the hazard the three
  sibling direct-call cases carry a `go depth 1` preamble and a comment for
  (DEC-163, 2026-09-04_test_review-F05), and the one case the S193 sweep
  missed. *Red, the diagnosis*: with the budget removed entirely
  (`options.nodes = 0`, nothing bounding the search at all) the case still
  passed, `DIAGNOSIS nodes budget 0, explored 49`. The budget was decoration.
  *Fix*: the same preamble, plus a precondition that separates the two --
  a direct call with `nodes = 100000` has to reach its budget, which it cannot
  if the flag is stale. No golden in it: the figure asserted is the budget.
  *Red, that the case can fail at all*: the fallback it guards neutered in
  `iterative_deepening_search` (`if (false && result.best_move == 0)`), which
  trips `tests/test_engine.cpp "a one node search still answers with a legal move": REQUIRE( 0 != 0 )`. Mutant killed,
  restored byte for byte. **Sweep**: `iterative_deepening_search` is called
  directly from four cases in `tests/test_engine.cpp` and nowhere else in
  `tests/`; the other three already had the preamble.

- **F-2, real: the `position` command was atomic for `game` and for nothing
  else.** `set_position()` wrote `initial_position`, called `tt_reset(&tt)` and
  re-armed `still_in_opening` *before* the moves were applied, and the moves
  loop's refusal restored none of the three -- so `position <a different base>
  moves zzzz` wiped a table the GUI had just paid a search for, left
  `initial_position` naming a board the engine is not standing on, and put the
  book back on for a command that never took effect. The board was all that
  came back, and `adocs/specs.md` already claimed "the whole command is
  atomic".
  *Fix*: commit on success. `set_position()` splits into `load_position()` --
  the load-or-restore half, which touches none of the three -- and
  `commit_position_base()`, which installs a new base. `command_position()`
  calls the first through a `load_base` lambda that records `pending_base`, and
  runs the second once, after the moves loop, at the bottom of the command.
  Nothing is written before success, so there is nothing to restore. The other
  three `set_position()` callers -- `reset_for_new_game()`, `command_test()`,
  `command_bench()` -- keep the eager wrapper and are unchanged.
  *Red*: `tests/test_engine.cpp` "a refused position command leaves the table
  alone" -- `position startpos moves e2e4 e7e5`, `go depth 6` to fill the
  table, the entry asserted present as the precondition, then `position
  kiwipete moves zzzz`: `tests/test_engine.cpp "a refused position command leaves the table alone": CHECK( nullptr != nullptr
  )`, 1 of 5 assertions, logged `a refused [position] cleared the transposition
  table`. `uci_tt()` is what a test can see of the three;
  `initial_position` and `still_in_opening` are file statics with no accessor
  and are fixed by the same commit-on-success.
  *Ordering*: `try_move()` does not read the table and a cold table for a new
  base is still cold once the line's moves are on the board, which is why
  moving the reset after the loop changes no search -- and why the bench total
  and all three depth-9 rows are the parent's.

- **F-3, trivial: the 4871-ply bound was hand-written in the surface golden.**
  `tests/test_uci_surface.cpp` builds the template from
  `std::to_string(POSITION_MAX_PLIES)`, so moving `HISTORY_MAX_SIZE` or
  `MAX_PLY` fails the MANUAL.md case instead of passing over a stale number,
  and a `static_assert(POSITION_MAX_PLIES == 4871, ...)` beside it names the
  two constants and the three places MANUAL.md writes the number out. DEC-142.

- **F-4, trivial: four comments describing pre-S210 mechanics.** "aborts after
  its first iteration" and "a local never-stop flag" are both gone from the
  engine; all four now say what happens today -- the stale flag is read at the
  first `check_limits()` and no iteration completes -- and two of them say what
  the old wording was true of, so the history is not lost.

- **F-5, trivial: two unit cases printed undocumented refusals.** `pop_int` and
  `pop_u64` were driven with the old option names `"Depth"` and `"Nodes"`
  outside a capture, so `./test_engine` put `info string refused [Depth] abc,
  not an integer` on its own stdout -- a shape no document describes. Both now
  pass the names `command_go` really passes, capture the refusal and assert its
  shape, and pin that a *missing* value says nothing on the UCI channel.

- **F-6, trivial: `+5`.** `std::from_chars` does not read a leading sign, so
  `go depth +5` is refused where `stoll` read 5 -- the same rule `Hash` already
  applies to `+64`. MANUAL.md's list of unreadable values gains `+5` with the
  reason, and the `go` refusal case gains the line, verified against the
  binary: `info string refused [go depth] +5, not an integer`.

**MANUAL.md** also gains the paragraph the F-2 fix makes true: a refused
`position` clears no table, moves no remembered base and re-arms no book, and a
new base still does all three once the whole command has played. **`specs.md`
is still the coordinator's**: its S210 passage should keep "the whole command
is atomic" -- that is now the truth rather than a claim -- and name the three
pieces of state beyond `game` that it covers.

**Gate**: `ctest -L fast` 38/38 in `build` and 38/38 in `build-tune`,
`./clang-format.sh --check` clean under `CLANG_FORMAT_MAJOR=22`, `bench`
7111579, `tools/search_bench.py` depth 9 identical. No match, no commit, no
shared document touched.
