# Audit 2026-08-22 adversarial

Scope: the whole engine as it stands on disk — HEAD `293a45b` **plus the
uncommitted S130 working tree** (modified `src/search.cpp`,
`tests/test_search.cpp`, `adocs/decisions.md`, `adocs/status.md`; new
`adocs/data/S130_sprt.sh`, `adocs/plan_current/S130_*.md`). `src/`, `tests/`,
`tools/`, `fastchess.sh`, `rating.sh`, `build_release.sh`, and documentation
claims checked against the code that backs them. Verdicts on every prior
finding under `adocs/audit/` re-assessed against this tree.

Method: **static-only, by hard constraint** — a time-controlled match (S130's
SPRT run 2) held every core of this machine for the whole audit. Nothing was
built, no test was executed, no engine was launched. Every finding therefore
carries a verification status: `statically confirmed` (provable from the cited
code alone) or `needs-run` (the exact command that decides it is stated, to be
run when the machine is free). Line numbers are the working tree's, not HEAD's,
where the two differ. Audits run against the code, not against the specs.

Written before any fix. Nothing in the repository was modified except this
report and one new, unregistered, unexecuted evidence test:
`tests/test_audit_fen_semantics.cpp` (see F01; it is not in
`tests/CMakeLists.txt`, so it cannot affect any build or gate until someone
registers it).

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-08-22_adversarial-F<nn>`. The example below writes that prefix as
`<report>`: a fenced example carrying this report's real stem cannot be told
apart from a real finding that a fence has swallowed, which is INV-14 (S049).

```
### <report>-F01  high  short title

Status: open

Evidence: file and line, or the command and its output.
Impact: what breaks, for whom, under what conditions.
Suggested resolution: what would close it. Not applied here.
```

Every finding ends in one of two places: a plan step whose `closes:` field
names it, or a decision entry stating why it will not be acted on, which moves
it to `accepted`. A finding moves to `closed` only after the audit is re-run
and no longer reports it. Fixing without re-running leaves it `planned`.

## Verdict

**No high finding. The S130 change in flight is sound as written** — reviewed
line by line, including the DEC-102 store-degradation fix and its interaction
with the lazy-evaluation bound, and nothing in it contradicts its own tests or
its pre-registered SPRT. Two medium findings: a semantic hole in FEN acceptance
that can corrupt the board in a Release build (F01), and a measurement trap in
`fastchess.sh` whose default reference is a fixed 2026-08-08 commit while
`CLAUDE.md` itself instructs the bare invocation (F02). Five low findings. All
ten findings of the 2026-08-21 adversarial audit are correctly dispositioned —
one accepted by measurement (DEC-098/S149, fence verified in the tree), nine
planned into S150–S158 — and every one of the nine is still present in this
tree, as expected for planned work.

## Findings

7 findings: 0 high, 2 medium, 5 low.

---

### 2026-08-22_adversarial-F01  medium  load_FEN accepts semantically illegal castling rights and en-passant squares, and the moves they license corrupt the board in Release

Status: planned — S161
Verification: statically confirmed (mechanism); end-to-end demonstration is
the written test, to be registered and run when the machine is free.

**Evidence.** `load_FEN()` validates FEN syntax only:

- Castling rights are parsed as bits with no check that the king or rook
  stands on its castling square (`src/bitboard.cpp:1699-1737`).
- The en-passant square is checked to parse as `[a-h][1-8]` and nothing else —
  not the rank, not the presence of the double-pushed pawn
  (`src/bitboard.cpp:1755-1771`).

The generator then trusts both. The castling block
(`src/bitboard.cpp:400-433`) tests the rights bits, the empty squares between,
and the attacked squares — never that the king is on `e_square` or the rook on
its corner; the from-square is the hard-coded `e_square`. `castling_rook()`
answers from the king's *target square*, not from the board
(`src/bitboard.cpp:645-659`), so `make_move()` moves a rook whether or not one
is there (`src/bitboard.cpp:836-842`). `move_piece()` and `remove_piece()` are
xors (`src/bitboard.cpp:674-729`): applied to a square the piece is not on,
they *create* pieces. The same holds for en passant: `remove_piece<them>(...,
captured_pawn, captured_square)` at `src/bitboard.cpp:807-813` xors a pawn
onto an empty victim square.

Concretely, from FENs the engine accepts today:

- `4k3/8/8/8/8/8/8/4K3 w K - 0 1` — kingside castle is generated (f1/g1
  empty, e1 unattacked, rights bit set); applying it xors rooks onto h1 *and*
  f1 out of nothing.
- `3k4/8/8/8/8/8/8/3K3R w K - 0 1` — castle generated from e1 while the king
  stands on d1; applying it leaves three set bits in the white king bitboard.
- `4k3/8/8/3P4/8/8/8/4K3 w - e6 0 1` — ep square e6 with no black pawn on e5;
  the ep capture d5xe6 is generated, and applying it xors a black pawn onto e5.

In a Debug build `squares_match_bitboards()` and the king-safety assert
(`src/bitboard.cpp:874,889-893`) abort. In the Release build that ships and is
measured, the corruption is silent, `hash` and `squares[]` disagree with the
bitboards, and every subsequent probe, generation and evaluation runs on a
board that is no position.

**Impact.** The prime directive is "Chesso never plays or accepts an illegal
move and never corrupts its own board state." Both halves are violated on
accepted input. Exposure is any `position fen` from a GUI, a test, a tool
(`pgn_to_positions`, `datagen`), or a book conversion that carries stale
rights or a stale ep square — inputs the engine's own game play never
produces, which is why perft (INV-1) cannot see it: every perft FEN is
semantically valid. No SPRT or rating verdict to date is contaminated; the
match books are valid EPDs.

**Suggested resolution.** Validate semantics at the load boundary: clear a
castling right whose king or rook is not on its square, clear an ep square
whose rank is not 3/6 for the side to move or whose victim square holds no
enemy pawn (fastchess and cutechess both do this sanitization). Register and
run `tests/test_audit_fen_semantics.cpp`, written by this audit as the
red-first regression: its assertions state the correct behaviour (no castle,
no ep capture generated) and are fix-agnostic.

---

### 2026-08-22_adversarial-F02  medium  fastchess.sh's default reference is a fixed 2026-08-08 commit, and CLAUDE.md's own instruction is the bare invocation that uses it

Status: closed — S160
Verification: statically confirmed.

**Evidence.** `fastchess.sh:43`:

```bash
REF="${REF:-7b4d9a4}"
```

`git log -1 7b4d9a4` → *"Add tests coverage, better move generation, major
bugfixes"*, **2026-08-08** — the pre-achesso baseline. `git log -S` shows the
default was set on 2026-08-08 (`61ca145`) and never moved. Meanwhile
`CLAUDE.md:84` states the decision procedure as:

> **3. A change that alters play is decided by SPRT.** `./fastchess.sh
> --fast`, or `REF=<sha> ./fastchess.sh`.

— the `--fast` form is bare. `DEV_MANUAL.md`'s "Play games" section lists
`./fastchess.sh`, `./fastchess.sh --fast` and `./fastchess.sh --nonreg` first,
with `REF=HEAD~1` as a fourth line, and nothing anywhere says what the default
reference is or that it is two weeks and several hundred Elo stale (S028's
evaluation fit alone measured +188.74 since that sha; S085 +21; S093 +10.7;
S107 +12.7...).

**Impact.** A run that omits `REF` measures the working tree against the
2026-08-08 engine. At `--fast` bounds (`elo0=0 elo1=10`) H1 arrives in minutes
regardless of the change under test, and the printed `candidate/reference`
line is the only tell. This is exactly the DEC-020 class — "one run reported
+301 Elo and meant nothing" — left armed in the default of the project's
per-change instrument, and the project's own top-level instructions point
agents at the trigger. Every *recorded* verdict so far named its reference
explicitly (the `adocs/data/*_sprt.sh` scripts pin `ref_sha`), so no banked
number is contaminated; the finding is a live trap, not a past contamination.

**Suggested resolution.** Any of: default `REF` to `HEAD` (measures the
uncommitted diff, which is the working-tree-vs-reference contract the header
already describes); refuse to run when `REF` is unset; or at minimum print a
loud warning when the reference is more than a few commits behind. Update
`CLAUDE.md:84` and `DEV_MANUAL.md` to carry `REF=<sha>` in every invocation
they teach.

---

### 2026-08-22_adversarial-F03  low  the 50-move draw is scored before checkmate is tested, so a mate delivered on the 100th halfmove scores as a draw

Status: open
Verification: statically confirmed against the rule; impact demonstration
needs-run.

**Evidence.** `src/search.cpp:587`:

```cpp
if (game->board.halfmove_clock >= 100) { return DRAW_SCORE; }
```

This runs before move generation, so a node whose side to move is checkmated
with the clock at 100 returns `DRAW_SCORE` instead of the mate score. Under
the FIDE Laws checkmate ends the game immediately (5.1.1) and takes precedence
over a 50-move claim; the 75-move rule (9.6.2) states the same exception
explicitly ("unless the last move resulted in checkmate"). The standard
engine-side form is `halfmove >= 100 && !(in check with no legal reply)`.

**Impact.** In a long shuffling endgame the engine can evaluate a line that
ends in mate-on-the-100th-halfmove as a draw — as the winner, discarding a
win; as the loser, steering into a "draw" that is a loss. Vanishingly rare,
but it is a wrong game-theoretic value produced by four characters of missing
condition, in the class the prime directive ranks above everything else.

**Suggested resolution.** Test for legal replies (or at least for check with
no legal reply) before returning the draw score at the 50-move boundary. A
regression position must be tool-verified per DEC-023 — construct a position
whose forced line delivers mate exactly as the clock reaches 100 and confirm
it with `stockfish` (`go depth 20`) before pinning it in a test.

---

### 2026-08-22_adversarial-F04  low  null move pruning has no negative mate-band guard, unlike reverse futility beside it, and the asymmetry is recorded nowhere

Status: open
Verification: statically confirmed (asymmetry); strength/mate-distance impact
needs-run.

**Evidence.** Reverse futility guards both sides of the mate band —
`src/search.cpp:661-662`:

```cpp
if (!is_pv && !is_in_check && static_cast<int>(ply) >= RFP_MIN_PLY &&
    depth <= RFP_MAX_DEPTH && beta < MATE_MIN && beta > -MATE_MIN) {
```

Null move pruning guards only the positive side — `src/search.cpp:719-721`:

```cpp
if (!is_pv && !is_in_check && ply > 0 && prev_move != 0 &&
    depth - 1 - null_reduction >= 1 && beta < MATE_MIN &&
    game_phase(&game->board) > 0) {
```

and its fail-high return handles only the positive band
(`src/search.cpp:733-737`): a `null_score` in the *negative* band is returned
raw. No comment, no decision and no step records why the two rules differ.

**Impact.** With `beta <= -MATE_MIN` — reachable at every defender node inside
a mate proof, e.g. whenever the attacker's `alpha` is already a mate score and
the question is whether a faster mate exists — *any* null-move result clears
beta, so the node fails high on a reduced search's word and the parent
discards the move. The reduced search is exactly the instrument that misses
mates (the project's own recurring bug: NMP hid a mate in 2 once already, per
CLAUDE.md). The exposure is mate-*distance* correctness and conversion speed
rather than false mates; S145 measured 0 false mates over twelve settings, but
that sweep varied RFP, not NMP, and the mined/constructed sets assert from the
attacker's side.

**Suggested resolution.** Either add `beta > -MATE_MIN` to the NMP guard (the
mirror of RFP's) and measure — a guard this narrow is likely inside noise, so
`--nonreg` bounds fit — or record a decision that the asymmetry is deliberate
and why. Needs-run either way: sweep the S145 set from the defender's side
(engine to move as the mated side, assert no delay regression) on the tune
build, machine idle.

---

### 2026-08-22_adversarial-F05  low  a stale hard-limit timer can kill the next search: the session check and the stop store are not atomic together

Status: open
Verification: statically confirmed (race window); reproduction needs-run.

**Evidence.** `src/chesso.cpp:413-424`:

```cpp
void stop_search_after_ms(uint64_t ms)
{
  std::thread job([ms, session = session_id.load()]() {
    std::chrono::milliseconds time_to_sleep(ms);
    std::this_thread::sleep_for(time_to_sleep);

    if (session_id == session) { stop_search_signal = true; }
  });
```

against `command_go`'s sequence at `src/chesso.cpp:1326-1329`:

```cpp
stop_and_join_search();
session_id++;
stop_search_signal = false;
```

The timer's `session_id == session` load and its `stop_search_signal = true`
store are two operations. A timer thread that loads the old `session_id`,
is preempted, and resumes after `session_id++` and `stop_search_signal =
false` sets the flag anyway — and the *new* search dies at its first poll.
The typical arming pattern makes stale timers routine, not exceptional: a move
normally ends at the soft limit, so its hard timer (3x the base allocation) is
still sleeping when the next `go` arrives, and every move of every game arms
one.

**Impact.** The affected move aborts at depth 1 (iteration 1 runs on the
local `never_stop`, so a move is always produced) — an occasional inexplicable
instant reply. Under an SPRT the noise hits both sides equally; in a rated
gauntlet it is a real, if rare, strength leak. The window is nanoseconds wide
per event, but the event count is every (timer expiry x go arrival) pair over
tens of thousands of games.

**Suggested resolution.** Make the disarm airtight: e.g. the timer sets a
`std::atomic<int> stop_request_session = session` and the search polls
`stop_request_session == session_id` — or have `command_go` hold a token the
timer must CAS against. A deterministic reproduction needs a stress harness
(tight `go movetime 1` / `position` loop under `taskset -c 0`), to be run when
the machine is free.

---

### 2026-08-22_adversarial-F06  low  the Polyglot random table in src/openings.cpp is a verbatim third-party constant table, and S146's own excludes put it out of scope

Status: open
Verification: statically confirmed.

**Evidence.** `src/openings.cpp:44-45` and onward: `const uint64_t
polyglot_randoms[781]`, beginning `0x9D39247E33776D41, ...` — the fixed
Zobrist constants of the Polyglot book format, copied verbatim (necessarily:
no `.bin` book can be keyed without exactly these values). The file cites the
format description (`http://hgm.nubati.net/book_format.html`,
`src/openings.cpp:15-18`). The constants originate in the PolyGlot adapter,
which is GPL. The house rule is "Never copy code from another engine. Never
copy tables." with an explicit licensing motive ("no GPL question anywhere in
this codebase"), and S146 — the step that owns book provenance — **excludes**
this artifact by name: `excludes: ... book *format* work`
(`adocs/plan_todo/S146_openings_book_provenance.md`). So the 5.2 MB book blob
has a pending provenance step and the 781-constant table beside it has none.

**Impact.** No measurement is touched (`Use Book` defaults false,
`src/chesso.cpp:930`). The exposure is the same as S146's: a copied table in
the shipped binary with no recorded decision about whether it is inside or
outside DEC-016. There is a defensible reading — format constants are the
published spec itself, interoperability makes them unavoidable, and the wiki
reproduces them — but that reading is currently nowhere written down, and the
rule the branch exists to test says tables are copied *never*, not *rarely*.

**Suggested resolution.** One decision entry: either the owner rules that
format-defining constants are spec, not tables, and DEC-016 does not reach
them (then cite it at the table), or the Polyglot book path is removed/
replaced along with S146's disposition of the blob. Fold into S146 or give it
its own line in that step's `accepts`.

---

### 2026-08-22_adversarial-F07  low  S130's substitution is untested against a lazy-bound stand pat, the one input class where its comparison runs on a bound instead of a score

Status: open
Verification: statically confirmed (test reading); the suggested test is the
run.

**Evidence.** The substitution compares the table score against `stand_pat`
(`src/search.cpp:352-368`), and `stand_pat` is initialised from `static_eval`,
which is **not always the static score**: when the probe found no usable eval
and the lazy shortcut fires, `evaluate_lazy()` returns `cheap ±
LAZY_EVAL_MARGIN` — a window-side bound (`src/evaluation.hpp:20-46`,
`src/evaluation.cpp:1086-1108`). The soundness of substituting against that
bound, and of the DEC-102 store degradation on top of it, is argued in
comments (`src/search.cpp:389-393`; step file section 5, "Lazy-eval clamp")
and I verified the argument holds case by case — but no test exercises it.
Every S130 case in `tests/test_search.cpp` uses the quiet anchor position with
windows wide enough that the shortcut cannot fire, and plants entries with
`TT_EVAL_NONE`, so `static_eval` is always the exact 563 in every assertion
the step added.

**Impact.** A future edit that reorders the lazy call and the substitution, or
flips one comparison on the bound path, leaves every current test green and
every node count plausible — the S106 failure class ("suites and node counts
stay green, only strength leaks"), on the one input class the new tests do not
reach.

**Suggested resolution.** One unit case on the same anchor: window `(0, 100)`
so the shortcut fires high (`cheap 567 - 184 = 383 >= beta`), plant
`TT_ALPHA_NODE` score 200 with `TT_EVAL_NONE` (200 > alpha so the probe does
not answer; 200 < 383 so the cap fires); assert the return is 200 and the
stored entry is `TT_BETA_NODE(200)` with `eval == TT_EVAL_NONE`. Red under an
inverted cap comparison, green today by the analysis above — confirm red/green
by running it, which this audit could not.

## Prior findings, re-assessed against this tree

Re-measured from each finding's own evidence, not from step stamps.

### 2026-08-21_adversarial (10 findings)

| finding | disposition | verified in this tree |
|---|---|---|
| F01 killer slot duplication | **accepted** — DEC-098, S149 | the unguarded shift is still shipped with the measured-choice comment (`src/search.cpp:908-915`); the re-targeted test counts *duplicated* slots and carries the -11.02 number (`tests/test_search.cpp:521-542`); S159 holds the ageing hypothesis. Correctly dispositioned. |
| F02 stale parameter prose | **still present, planned (S150)** | `adocs/specs.md:186` still says depth 5 / 50 cp / past 400 (engine: 2 / 21 / 437, `src/search_params.hpp:240-242`); `adocs/specs.md:193` still says "quiescence is capped at 8 plies" (engine: 19); `MANUAL.md:256` still says "from depth 5". |
| F03 single-control verdicts | still present, planned (S151) | regime unchanged (`fastchess.sh:59`, `tc="8+0.08"`). |
| F04 no strength checkpoint | still present, planned (S152) | S128 still deep in `plan_todo/`; S152 added. |
| F05 zero-match steps serialize the machine | still present, planned (S153) | `plan_active_max` still 1 (`.moltke.json`). |
| F06 mate-in-three floor margin of one | still present, planned (S154) | `MATE_IN_THREE_FLOOR = 7` at `tests/test_engine.cpp:1817`, asserted at `:2246`; S130's own three readings of the suite were identical, so the floor did not move. |
| F07 one-motif mate set | still present, planned (S155) | set unchanged. |
| F08 mined breadth set asserted nowhere | still present, planned (S156) | still one comment (`tests/test_engine.cpp:1694`), no assertion, no registration. |
| F09 nElo bounds worded as Elo | still present, planned (S157) | the two `specs.md` sentences and the `plan_done` stamps unchanged; `DEV_MANUAL.md:1307` still documents the model without connecting the bounds' scale to the conclusions. |
| F10 book digest (informational) | planned (S158) | digest recorded in the prior report; S146 holds the provenance work. See also this report's F06 for the artifact S146 excludes. |

### 2026-08-14_test_review (7 findings)

F01–F06 planned via S067 and spot-checked as landed where cheap to verify:
the build-dependent test timeout F03 asked for exists and cites the finding
(`tests/CMakeLists.txt:32-45`). F07 accepted (DEC-058). No regression found.

### 2026-08-13_adversarial (9 findings)

| finding | state now |
|---|---|
| F01 harness reports success on abort | **fixed** — the trap preserves exit status and prints `SPRT-RUN-FAILED`, citing the finding (`fastchess.sh:179-186`). |
| F02 1 ms clock arms no timer | **fixed** — S036 floor (`src/chesso.cpp:464-472`). |
| F03 per-iteration node count | **fixed** — S037 cumulative count (`src/chesso.cpp:862-885`), and `specs.md` records the pre-S037 caveat. |
| F04 tuner-model 2 cp tolerance | planned; not re-measured here (needs a run). |
| F05 lazy margin re-decision | still pending — S039 in `plan_todo/`, margin now 184 via S085. |
| F06 DEV_MANUAL tuner staleness | not re-checked in detail. |
| F07 free_mask untested | **fixed** — `test_tuner_groups` (`tests/CMakeLists.txt:65`). |
| F08 ep square set after every double push | **still present** — `src/bitboard.cpp:825-834` sets it unconditionally on a double push; S042 pending. Splits hashes of identical positions, weakening TT sharing and repetition detection slightly. |
| F09 toolchain-file note | informational, moot. |

The plan-review reports (2026-08-13 x2, 2026-08-16, 2026-08-20) were processed
by S138–S145 per the git log; `specs.md:106` closes 2026-08-20_plan_review-F17
explicitly, and `src/search_params.hpp` cites F08/F14 at the bounds they set.
Not re-audited item by item — they are plan documents, and this audit's scope
is the code.

## Checked and clean

Listed because a negative result is a result, and several of these are where
this audit expected to find something.

- **The S130 working tree, line by line.** The bound-direction predicate is
  correct on all three arms and strict on both comparisons (an equal score
  does not substitute, so it cannot degrade a store for nothing). The
  mate-band exclusion on the raw score is valid: a stored mate is
  `MATE_MAX - d` with `d + ply ≤ 2·MAX_PLY = 256 < 1000`, held by the
  `static_assert` at `src/search.cpp:215-216`, and `de_normalize_score` is the
  identity on everything the guard admits. The DEC-102 store degradation is
  sound in every path I could construct, including the two subtle ones: a
  capped stand pat that still clears beta stores `TT_BETA_NODE(s)` whose claim
  follows from the stand-pat option (`value ≥ static > s`), and the same holds
  when `static_eval` is itself a lazy *bound* — on the fail-high branch the
  bound is a true lower bound, so the store's claim survives (the missing
  *test* for that path is F07). `value_type` is computed from `floor_type`,
  never from itself, so repeated improvements cannot launder a cap. The
  in-check mate store is unreachable from the substitution (`floor_type` is
  forced exact in check and the branch returns before the final store). The
  eval field cannot be polluted by construction: `stored_eval` is fixed from
  `static_eval` before `stand_pat` exists, and quiescence can never overwrite
  a real stored eval with `TT_EVAL_NONE` — when the entry carries an eval, the
  read-through makes `static_eval_is_exact` true, so `stored_eval` is that
  same eval. With an empty table every branch reduces to the pre-S130 code.
- **The S130 test additions are non-vacuous as written**: each case pins the
  empty-table result first, the mate-band case documents the 48995 it returns
  without the guard, and the store-degradation case pins the empty-table
  `TT_PV_NODE` before asserting the degraded types. The one S094 case that had
  to move was re-targeted, not relaxed, with the red output recorded in the
  step file.
- **The S130 SPRT is pre-registered correctly**: `adocs/data/S130_sprt.sh`
  states all three readings, the no-verdict threshold (~8000 games), the
  fire-rate candidate explanation, and that run 1 was killed at 103 games for
  the DEC-102 defect with no verdict claimed. Reference `293a45b` is the
  immediate parent; the config mirrors `fastchess.sh`'s engine block.
- **Move-ordering bands still clear** (`src/evaluation.cpp:26-43`): worst
  capture 900100 > `ORDER_KILLER_0` 900000; history bounded to ±8192 by the
  gravity update itself (`src/search.cpp:33-53`), six orders under the
  countermove band; `piece_values_abs` built from the `MVV_*` macros that
  `search_params.hpp:29-32` excludes from tuning.
- **History behaviour matches DEC-101**: `search_state_t state = {}` per `go`
  (`src/chesso.cpp:647`), no persistence, malus span structurally excludes the
  cutoff move (append after the `break`, `src/search.cpp:947-959`), bonus and
  malus gates are the same `!is_capture`.
- **`tt_resize` clamps Hash to `[TT_MIN_MB, TT_MAX_MB]`**
  (`src/transposition_table.cpp:14`), halves on allocation failure, and every
  probe/store tolerates a null table.
- **The quiescence/main-search depth wall holds**: `TT_DEPTH_QS = -1` with
  `static_assert(TT_DEPTH_QS < 0)`, negamax never probes below depth 0, and
  the working tree's new substitution consumes entries without crossing it.
- **INV-6's instrument is internally consistent**: node counting is
  deterministic (counter incremented before every early exit, stop flag polled
  on a fixed mask), `search_bench.py` reads the cumulative count, and
  `set_position` resets the TT between the three bench FENs so position order
  cannot leak state.
- **`eval_tables.hpp` provenance is consistent with the no-copy rule**: the
  tables are self-play-fitted residuals (rook mg all negative, queen mg ≈700 —
  the material/table split is degenerate exactly as documented), resembling no
  published set; the header states corpus and method.
- **`fastchess.sh` otherwise survives adversarial reading**: candidate
  snapshotted before game one, per-run output directory with an append-refusal,
  exit-status-preserving trap with terminal markers on every path, reference
  built from a pinned worktree in the same Release/native shape as `build/`
  (verified against `build/CMakeCache.txt`: `Release`, `native`, PGO off,
  TUNE off), busy-machine warning, PGN-based census.
- **`compute_search_time_budget` honours its own promises**: hard clamped to
  `remaining - MOVE_OVERHEAD_MS`, floored by the S036 rule, soft clamped to
  hard, scaling touches the soft limit only, `go movetime` never scaled.
