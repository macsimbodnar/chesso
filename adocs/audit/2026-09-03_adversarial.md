# Audit 2026-09-03 adversarial

Commit audited: `1d8cbac32f8fca994b75f3e4158d3a63a28de3a6` (`achesso`, "Plan S173: make_book replaces a book atomically", 2026-09-03), clean working tree.


Type: `adversarial`, whole engine. Scope: `src/`, `tests/`, `tools/`, the shell
scripts (`fastchess.sh`, `rating.sh`, `build_release.sh`, `books/fetch_book.sh`,
`clang-format.sh`), the cmake modules, and the claims in `adocs/specs.md`,
`MANUAL.md`, `DEV_MANUAL.md`, `adocs/plan.md` and the recent decisions checked
against the code that backs them. Three classes were looked for: correctness
defects, violations of the project's own rules, and techniques left behind the
documented state of the art. Verdicts on every finding in the earlier reports
under `adocs/audit/` are re-taken from each finding's own reproduction.

Method. Read cold, code first, documents second. Then run: `cmake --build build
-j8` (nothing recompiled, the tree was current), `ctest --test-dir build -L
fast --output-on-failure` (24/24, 55.9 s, Release), `tools/search_bench.py` at
depths 9 and 12, the engine over UCI on the edge cases below, `tools/make_book`
and `tools/pgn_to_positions` on hand-written PGN fixtures, `./rating.sh
--bracket`, and an independent re-derivation of the whole shipped opening book
with python-chess 1.11.2 (`~/.venv/chess/bin/python`). Frame sizes were read from
`otool -tv build/src/chesso`. Machine idle (load 1.2 on 8 cores, mains power),
no match run. Nothing in the repository was modified except this report and one
new, **unregistered** red-first regression test,
`tests/test_audit_polyglot_key.cpp` (F01), which is not in
`tests/CMakeLists.txt` and cannot affect any build or gate until someone
registers it.

Finding format. Every finding carries a status of `open`, `planned`, `closed`,
or `accepted`, and an id prefixed with this report's own name, so the ids in
this file read `2026-09-03_adversarial-F<nn>`. The example below writes that
prefix as `<report>`: a fenced example carrying this report's real stem cannot
be told apart from a real finding that a fence has swallowed, which is INV-14
(S049).

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

**No high finding. The search core, the UCI layer's time management and the
S172/S146 book loading survive a line-by-line adversarial reading; the fast
suite is green and both documented node-count baselines reproduce to the node.**
Two medium findings, both in the code S172 and S146 shipped three days ago and
both found by re-deriving the shipped book independently rather than by reading
the stamps: the Polyglot key is wrong for one class of position, so the shipped
book carries 7 non-conforming entries and an external book cannot be probed
there (F01); and the SAN parser every tool sits on fabricates a move instead of
failing in the Release build, so `make_book` and `pgn_to_positions` -- the
DEC-023 board tool -- run on silently on a corrupted board (F02). Two low
findings on the scripts and the `position` command. **Class 2 (the project's
own rules) turned up nothing.** **Class 3 (Elo left behind the literature)
turned up nothing the plan does not already carry** -- every gap this audit
checked has a pending step, listed under "Checked and clean". Of the earlier
reports' 33 code-audit findings, 27 are closed on re-measurement here, 3 are
accepted by decision and 3 are planned in pending steps; the four plan-review
reports are dispositioned in the table.

## Findings

4 findings: 0 high, 2 medium, 2 low.

---

### 2026-09-03_adversarial-F01  medium  `get_key()` wraps round the board edge when the en-passant square is on the a- or h-file, so the Polyglot key is wrong wherever a same-side pawn stands on the wrapped square; the shipped book carries 7 such entries and an external book is silently abandoned there

Status: planned -- S175

**Evidence.** `src/openings.cpp:562-593` decides whether the key carries the
en-passant component by testing two squares for a pawn of the side to move:

```cpp
if (board->active_color == WHITE) {
  const index_t on_left = board->en_passant + 7;
  const index_t on_right = board->en_passant + 9;
  ...
} else {
  const index_t on_left = board->en_passant - 9;
  const index_t on_right = board->en_passant - 7;
```

On the a8 = 0 index scheme those offsets wrap at the edge files. White to
move, en-passant `h6` (23): `+9` is 32 = **a4**. En-passant `a6` (16): `+7` is
23 = **h6**. Black to move, `a3` (40): `-9` is 31 = **h5**; `h3` (47): `-7` is
40 = **a3**. A pawn of the side to move on the wrapped square switches the
component on. The format, quoted in the same file's own comment at
`src/openings.cpp:549-552`, asks for "a pawn next to it belonging to the player
to move" -- next to the pushed pawn, on its rank.

Measured over the whole shipped book, not argued. Every mainline of
`books/8moves_v3.pgn` was replayed to 16 plies with python-chess 1.11.2 and
keyed with `chess.polyglot.zobrist_hash()` (an independent implementation of
the same specification), moves encoded in the Polyglot word with castling as
king-takes-rook, and the multiset compared with `src/openings.bin` read as
big-endian 16-byte records:

```
games 34700 plies 555200 derived_entries 172232 book_entries 172232 distinct_positions 129613
missing 7 extra 7 weight_mismatch 0
```

Same games, plies, entry count, position count, and every weight; the only
disagreement is 7 (key, move) pairs, each the same move under two keys. All 7
have the en-passant square on an edge file with a same-side pawn on the wrapped
square, and in each the book holds chesso's key and not the format's:

```
game 1957 ply 15: r2qk1nr/pp1nbpp1/2p1p3/3pPb1p/P2P4/2P2N2/1P2BPPP/RNBQ1RK1 b kq - 0 8
   ep a3  spec 35fdf2c529e5841e in book? False   chesso-style 4531811c2227ea3a in book? True
game 2111 ply 14: rnb1kb1r/2pqnpp1/1p2p3/p2pP2p/P2P1P2/2P5/1P1N2PP/R1BQKBNR w KQkq - 0 8
   ep h6  spec bff7aaf88aaf9fbb in book? False   chesso-style d854e754c9f9cab0 in book? True
game 2354 ply 11: rnbqkb1r/pp3pp1/2pp1n2/4p2p/P2PP3/2N2P2/1PP1N1PP/R1BQKB1R b KQkq - 0 6
   ep a3  spec ca4a060057cf5b4d in book? False   chesso-style ba8675d95c0d3569 in book? True
game 4007 ply 14: r1bqk2r/pp1nppb1/2pp1np1/7p/P2PP3/2N1B1N1/1PP2PPP/R2QKB1R w KQkq - 0 8
   ep h6  spec e425c4f582e5ebd3 in book? False   chesso-style 83868959c1b3bed8 in book? True
game 17094 ply 12: rnb1kbnr/2pq1pp1/1p2p3/p2pP2p/P2P1P2/2P5/1P4PP/RNBQKBNR w KQkq - 0 7
   ep h6  spec d8d6ae27abdbbc36 in book? False   chesso-style bf75e38be88de93d in book? True
game 18043 ply 14: r1bqk2r/1ppnppb1/p2p1np1/7p/P1PPP3/2N3N1/1P3PPP/R1BQKB1R w KQkq - 0 8
   ep h6  spec 5b83fc21d030703d in book? False   chesso-style 3c20b18d93662536 in book? True
game 27676 ply 15: rnbqk2r/p3ppb1/2pp1np1/1p5p/P2PP2P/2N1BP2/1PPQ2P1/R3KBNR b KQkq - 0 8
   ep a3  spec aa2989371cd8f709 in book? False   chesso-style dae5faee171a992d in book? True
```

(The FENs are python-chess's, which prints `-` when no en-passant capture is
legal; `board.ep_square` is set in each. `chesso-style` is `get_key()`
transcribed into python including the `+7/+9` and `-9/-7` offsets, and it
reproduces the book's key in all seven.) The white pawn on a4 in the `h6`
cases and the black pawn on h5 in the `a3` cases are the wrapped-square pawns.

The durable reproduction is `tests/test_audit_polyglot_key.cpp`, written by
this audit and **observed red at `1d8cbac`**: the seven positions above with
the en-passant square written into the FEN (so `load_FEN()`'s S161 sanitizer
keeps it -- the victim pawn is on the board in each) against the python-chess
key, plus three controls of the same shape with the wrapped square empty or a
genuine capturer beside the pushed pawn, where the two implementations agree.
Compiled against `build/src/libchesso_engine.a`:

```
[doctest] assertions: 30 | 23 passed | 7 failed |
[doctest] Status: FAILURE!
```

Why nothing caught it: `tests/test_openings.cpp:22-47` holds the format's own
nine example keys, and none of them has an en-passant square on an edge file.

**Impact.** Three things, none of them a measurement. (1) `src/openings.bin`
is a non-conforming Polyglot file for 7 of its 172232 entries: a
spec-conforming reader (a GUI, python-chess) never finds them, and `MANUAL.md:77`'s
"172232 Polyglot entries" is 7 short. Chesso itself still finds them, because
it probes with the same deviation -- the built-in book is self-consistent.
(2) The S172 feature: with `Book File` pointing at a third-party book, chesso
computes a key the book does not hold at any such position,
`get_book_moves_for_key()` returns 0, `search_book_move()` sets
`still_in_opening = false` (`src/chesso.cpp:678-681`) and the book is
abandoned for the rest of the game -- silently, since `OwnBook` play prints no
`info`. (3) `make_book` keys with the same function, so every book it builds
inherits the deviation, and a `Best Book Move`/weight it reports for those
positions describes entries no other reader can reach. No verdict is touched:
`OwnBook` defaults false and no measurement on record has played a book move
(S158).

**Suggested resolution.** Compute the two candidate squares by file, not by
offset: for an en-passant square on file `f`, test `f - 1` only when `f > 0`
and `f + 1` only when `f < 7`, on the rank the capturing pawn stands on.
Register `tests/test_audit_polyglot_key.cpp` and observe it green. Rebuild
`src/openings.bin` -- the digest will change, since 7 keys move -- and
re-record the digest in `adocs/specs.md`, `MANUAL.md`, `DEV_MANUAL.md`,
`src/openings_embedded.S` and the S146 stamp's successor; the S146
reproducibility claim survives (the writer still sorts) and the python-chess
re-derivation above becomes the check that the book is the format's. INV-6 is
discharged on node counts as S172 and S146 discharged it: the default
configuration never probes the book.

---

### 2026-09-03_adversarial-F02  medium  `algebraic_to_move()` fabricates a move instead of failing in the Release build, so `make_book` and `pgn_to_positions` run on silently on a corrupted board and exit 0

Status: planned -- S174

**Evidence.** `src/bitboard.cpp:2418-2424`, `:2430-2435` and `:2483-2488` are
the parser's three failure paths, and each is `LOG_E ... assert(false)` with
no return: a destination that is not two characters, a destination off the
board, and a token matching no legal move all fall through to
`src/bitboard.cpp:2490-2492`, which returns `NEW_MOVE(result.from, result.to,
...)` built from whatever the partial parse left in `result` -- `from` 0 (a8),
the piece letter, and a `to` that can exceed 63 and shift into the piece
field. Under `NDEBUG` the asserts are gone and `LOG_E` is `if (false)`
(`src/log.hpp:38`), so the caller receives a non-zero move. Both callers test
for zero and never see it: `tools/make_book.cpp:344-351` --

```cpp
if (move == 0 || !make_move(&game, move)) {
  // One unparseable token poisons every position after it, so the game is
  // dropped from that point rather than resynchronised at a wrong board.
```

-- and `tools/pgn_to_positions.cpp:32-37`. `make_move()` then applies the
fabricated move with no legality check (`src/bitboard.cpp:885-888`: "No
legality check here. generate_moves() emits legal moves only ... every caller
feeds this function a generated move") and every piece update is an xor, so
the board is not left illegal, it is **rewritten**.

Reproduced on the Release `build/tools/`, three one-game PGNs against a clean
control (`1. e4 e5 2. Nf3 Nc6 *`):

```
=== annotated: 1. e4!? e5 2. Nf3 Nc6 * ===
games cut short      0
entries written      4
exit=0
verdict              loadable
  463b96181691fc9c  a8a7    weight 1        <- the start position, move "a8a7"
  01cb20282e5c39fb  b8c6    weight 1        <- three keys of positions that exist in no game
  50d7c494d55fa0f0  e7e5    weight 1
  daafccde46ba58c6  g1f3    weight 1
=== illegal: 1. e4 e5 2. Qxf7 Nc6 * ===
games cut short      0
exit=0
verdict              loadable
  0844931a6ef4b9a0  a8f7    weight 1        <- after 1.e4 e5, move "a8f7"
  98bab67fbe103d41  a8c6    weight 1
```

The control writes `e2e4 / e7e5 / g1f3 / b8c6` under the format's keys. And
the DEC-023 board tool on the same annotated line:

```
$ printf 'e4!? e5 Nf3 Nc6\n' | ./build/tools/pgn_to_positions
0  e4!?  a8a7  rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1  24
1  e5    e7e5  1nbqkbnr/Bppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQk - 1 1   24
...
exit=0
```

A white bishop has appeared on a7, the a8 rook is gone, Black has lost a
castling right, e4 was never played, and the exit status is 0. Suffix
annotations (`!?`, `!`, `?`) are ordinary in published PGN;
`movetext_to_san()` strips `$N` glyphs, comments and variations
(`tools/make_book.cpp:72-140`) and not these.

**Impact.** `make_book` is the only thing in the tree that produces the
shipped book (S146, DEC-131), and on an input outside the one PGN it has been
run on it emits a corrupt book that `dump` calls `loadable` and the engine
loads -- the same silence S146 fixed for the short write, one layer down.
`pgn_to_positions` feeds `tools/analyse_game.py`, the pipeline CLAUDE.md names
for every chess question the agent may not answer itself: on an annotated game
it scores positions that never occurred and reports success. The evidence
S146's stamp and `adocs/status.md:28-30` rest on -- "0 games cut short ... so
every SAN token in eight million bytes of PGN parsed" -- is not evidence,
because the counter cannot see a parse failure; **the shipped book is
nonetheless verified correct by this audit's re-derivation** (F01: every move
and weight agrees, only the 7 keys differ), so nothing shipped is affected
today. No measurement is affected: neither tool is in a match path.

**Suggested resolution.** Make the three failure paths `return 0` in every
build, keeping the asserts for Debug if wanted; strip trailing `!`/`?` runs in
`movetext_to_san()` as it already strips `+`/`#`; have `make_book build`
refuse to write when any game was cut short unless told otherwise, so the
counter becomes a gate instead of a report; give the tool a fixture test, the
gap S146 already noted. Red first: the three fixtures above, asserting exit
non-zero and no output file. `pgn_to_positions` needs no change once the
parser returns 0 -- its guard is already written.

---

### 2026-09-03_adversarial-F03  low  `rating.sh` and `build_release.sh` need GNU `nproc` and `timeout` and die on this machine before any terminal marker is armed; only `fastchess.sh` was made portable

Status: planned -- S177

**Evidence.**

```
$ ./rating.sh --bracket
./rating.sh: line 50: nproc: command not found
exit=127
$ which nproc timeout ordo gtimeout
(none found)
```

`rating.sh:50` is `all_cores="$(nproc)"` under `set -euo pipefail`;
`rating.sh:110` drives every reference engine through `timeout -k 1 5`, which
would return an empty name and `fail` on each; the `RATING-RUN-*` trap is
armed at `rating.sh:139`, after both, so a detached run prints **no terminal
marker at all**. `build_release.sh:99` and `:163` use `-j"$(nproc)"` (that
script's three targets are x86-64 and `cmake/arch.cmake:57-62` refuses them on
this machine first, so it is unusable here either way). `fastchess.sh:170` and
`:307` already carry the portable form `sysctl -n hw.physicalcpu 2>/dev/null
|| nproc` (S167), and `books/fetch_book.sh:90-96` the `shasum` fallback.
`adocs/specs.md:32-33` states that `./rating.sh` re-derives the rating;
`.moltke.local.md` and `TOOLCHAIN.md` ("macOS throughout") do not say the two
scripts cannot run here.

**Impact.** No measurement is affected today: DEC-112 scopes the rating runs
and the release builds to the workstation. The exposure is the class
2026-08-13_adversarial-F01 and S167 removed for `fastchess.sh` -- a script that
dies with no marker under the WATCHERS rule leaves a watcher spinning to its
ceiling -- and a documented command that fails at once on the machine the
project is on.

**Suggested resolution.** The `sysctl || nproc` form in both scripts; a
`gtimeout`/`timeout` probe or the python-chess-style identification
`rating.sh` could borrow from `tools/analyse_game.py`; arm the marker trap
before the first command that can fail, as `fastchess.sh:91-97` does since
S167; one line in `.moltke.local.md` naming which scripts run on this machine.

---

### 2026-09-03_adversarial-F04  low  a `position fen` with fewer than six fields is dropped silently, and a six-field FEN that fails to load resets the board to the last FEN without its moves -- both leave the engine on a position the GUI did not send

Status: planned -- S176

**Evidence.** `src/chesso.cpp:1328-1333` returns from `command_position` when
fewer than six tokens follow `fen`, with nothing on the UCI channel; the
board stays wherever the previous command left it. `src/chesso.cpp:390-398`,
on a FEN `load_FEN()` rejects, reloads `initial_position` -- the last FEN, not
the last position -- so the moves applied since are gone. Reproduced:

```
$ printf 'uci\nposition startpos moves e2e4\nposition fen 8/8/8/4k3/8/4K3/8/8 w - -\nfen\n
  position fen 8/8/8/4k3/8/4K3/8/8 w - - moves e3d3\nfen\nquit\n' | ./build/src/chesso
rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1   <- four-field FEN ignored, board still after e2e4
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1     <- "moves" and "e3d3" were read as the clock fields,
                                                               load failed, and the board is now the START position
```

`tests/test_engine.cpp:602-616` ("a malformed fen leaves the previous
position alone") covers only the case with no moves applied, where the FEN and
the position coincide. `MANUAL.md:192` documents `fen <six fields>`, so the
four-field form is outside the stated contract; the reset that drops the moves
is documented nowhere.

**Impact.** A GUI or tool that sends the FEN standard's optional-field form --
Stockfish, cutechess and python-chess all accept it -- gets a stale board and
an illegal move in its game. Low: fastchess and cutechess send six fields, and
no match here has seen it. The reset case turns one malformed FEN into a
guaranteed illegal move where keeping the previous position would have kept
the game.

**Suggested resolution.** Accept four- and five-field FENs by defaulting the
clocks to `0 1`; on a load failure keep the whole previous position -- board
and history -- rather than reloading `initial_position`; report the refusal
with one `info string` in every build, the S137 pattern, since `LOG_W` is
silent in the binary that ships. Extend the test at
`tests/test_engine.cpp:602` with a move applied before the malformed FEN.

## Prior findings, re-assessed against this tree

Re-measured from each finding's own reproduction where one is stated; where
the finding was about a document or a plan step, from the current text. The
status recorded in the earlier report is given where it differs from what this
tree shows, because the earlier reports are evidence and are not edited.

### 2026-08-22_adversarial (7 findings)

| finding | state now | evidence |
|---|---|---|
| F01 FEN semantics corrupt the board | **closed** (S161; report says planned) | the three FENs load as `4k3/8/8/8/8/8/8/4K3 w - -`, `3k4/8/8/8/8/8/8/3K3R w - -`, `4k3/8/8/3P4/8/8/8/4K3 w - -` -- rights and ep cleared; `test_audit_fen_semantics` registered and green |
| F02 default REF 7b4d9a4 | closed (S160) | `fastchess.sh:133` `REF="${REF:-HEAD}"`, banner with dates, `test_fastchess_script` green |
| F03 50-move draw before mate | **closed** (S162; report says open) | `7k/6pp/8/8/8/7n/6P1/R6K w - - 99 60` at depth 1 answers `info score mate 1 ... pv a1a8`, `bestmove a1a8`; `src/search.cpp:647-655` |
| F04 NMP negative mate band | **closed** (S165) | `src/search.cpp:823` guards `beta > -MATE_MIN` |
| F05 stale timer race | **closed** (S163) | `src/chesso.cpp:49`, `:134-140`, `:470-484` -- check and store under `session_mutex`; the stress harness is Linux-only (`taskset`) and was not re-run here |
| F06 Polyglot table | **accepted** (DEC-121; report says open) | ruled format-defining, cited at `src/openings.cpp:40-49` |
| F07 lazy-bound stand pat untested | **closed** (S164) | `tests/test_search.cpp:1251` `run(0, 100)`, `:1594` names the finding |

### 2026-08-21_adversarial (10 findings)

| finding | state now | evidence |
|---|---|---|
| F01 killer slot duplication | accepted (DEC-098, S149) | unguarded shift still shipped with its comment, `src/search.cpp:1012-1018` |
| F02 stale parameter prose | closed (S150) | `test_plan_params` in the fast label, green |
| F03 single-control verdicts | planned (S151, pending) | `fastchess.sh:150` still `8+0.08` only |
| F04 no strength checkpoint | **accepted** (DEC-108; report says open) | S152 carries the one run near the goal; DEC-108 is the decision not to checkpoint at a block boundary |
| F05 zero-match steps serialize | closed (DEC-113, S153) | |
| F06 mate-in-three floor margin | closed (S154, re-derived at S168) | `MATE_IN_THREE_FLOOR = 11` at `tests/test_engine.cpp:1964`, asserted `:2597` |
| F07 one-motif mate set | closed (S155, S168) | three motifs, census tracked |
| F08 mined set not asserted | closed (S156) | `test_mate_breadth` in the fast label, 17.3 s green |
| F09 nElo bounds worded as Elo | closed (S157) | `fastchess.sh:25-38`, `adocs/specs.md` "5 nElo" wording |
| F10 book digest | closed (S158); superseded by S146 | the book it digested is deleted; the new digest reproduces (F01 above) |

### 2026-08-14_test_review (7 findings)

F01-F06 **closed** via S067: neither illegal FEN survives as a case
(`2k5/8/8/8/8/8/1q6/K1R5` and `7k/5Q1K/8` appear only in comments recording
their removal, `tests/test_search.cpp:148,375,416`, `tests/test_engine.cpp:710`);
the build-dependent timeout is at `tests/CMakeLists.txt:10-14`; the KBvKB and
KBvKN cases are at `tests/test_search.cpp:2903-2905`; the ordering budget is a
measured 440000 (`:894`). F07 accepted (DEC-058).

### 2026-08-13_adversarial (9 findings)

| finding | state now | evidence |
|---|---|---|
| F01 harness exits 0 on abort | closed (S035, S167) | trap at `fastchess.sh:91-97` |
| F02 1 ms clock never answers | closed (S036) | `go wtime 1 btime 1000` answers `bestmove d2d4` |
| F03 per-iteration `nodes` | closed (S037) | `search_bench` reads cumulative counts |
| F04 tuner-model 2 cp tolerance | closed (S038, DEC-053) | tolerance 3 at the 2.875 bound, `tests/test_eval_model.cpp:249-257` |
| F05 lazy margin comment stale | **still present**, planned (S039) | `src/evaluation.hpp:295` still says king safety "ships at zero weight"; `src/evaluation.cpp:790-793` is non-zero |
| F06 DEV_MANUAL tuner section | closed (S040) | 827, `tempo` group listed, `DEV_MANUAL.md:2408,2509` |
| F07 free_mask untested | closed (S041) | `test_tuner_groups` |
| F08 ep square after every double push | **still present**, planned (S042) | `position startpos moves d2d4 d7d5 c2c4` then `fen` prints `... b KQkq c3 0 2`; `src/bitboard.cpp:825-834` |
| F09 dead toolchain line | closed (S043) | no `CMAKE_TOOLCHAIN_FILE` in `CMakeLists.txt` |

### The plan-review reports

Not re-audited item by item -- they are plan documents and this audit's scope
is the code -- but each finding was traced to its disposition:

- **2026-08-13_plan_review** (9): all closed by its own `.2` re-run.
- **2026-08-13_plan_review.2** (9, all recorded `planned`): F07 closed (S062),
  F09 closed (S064); F08 (moved line citations) overtaken by S138 and S169;
  F01 carried in the pending S055 (which now discusses the S038 guard), F02 in
  S039 via DEC-054, F03 in the pending S030's text, F04 answered in S025's
  rewritten `accepts` ("no clause here requires a non-worse timing"), F05 in
  S109's mate clause after S026's retirement. F06 (S023's second criterion has
  no instrument) is the one with **no trace** in the reserve step that carries
  it -- `grep -n 'plan_review.2-F06\|instrument\|slack'
  adocs/plan_todo/S023_capture_history.md` is empty; S023 is reserve
  (DEC-087) so nothing is due, but the finding is neither planned nor accepted.
- **2026-08-16_plan_review** (10): F04, F05, F06, F07, F09, F10 closed by
  S072, S069, S070, S074, S078, S071; F08 accepted (DEC-062); F02 closed by
  S138/S169 after S063 was folded; F01 and F03 carried in S039 and S055 by
  DEC-086.
- **2026-08-20_plan_review** (18): every finding has a `closes:` in a
  `plan_done/` step -- S138 (F01, F12, F13), S139 (F02, F03, F04, F07, F15),
  S140 (F05, F10, F11, F16, F17), S141 (F06), S142 (F08, F14), S085 (F09,
  F18).

## Checked and clean

Listed because a negative result is a result, and several of these are where
this audit expected to find something.

- **The suite and the baselines.** `ctest -L fast` 24/24 in 55.9 s on the
  Release build; `search_bench` 121512 / 800769 / 62907 at depth 9 and
  639228 / 3430710 / 367858 at depth 12, `c3d5` / `e2a6` / `d7c8q` at both --
  identical to every figure the documents record since S108.
- **The shipped book is what the stamps say, checked from outside.**
  `books/8moves_v3.pgn` sha256 matches the pin at `books/fetch_book.sh:62`;
  all 34700 games carry `[Result "1/2-1/2"]`; `src/openings.bin` is 2755712
  bytes at the recorded sha256; the python-chess re-derivation agrees on
  games, plies, 172232 entries, 129613 positions and every weight (F01's 7
  keys aside). The S172 refusals hold end to end: a missing path and the PGN
  offered as a book are both answered with the documented `info string`, an
  `OwnBook` search returns a book move with no `info` line, and `go nodes`
  bypasses the book as `command_go` intends.
- **The search core, line by line.** Draw tests before the probe; PV nodes
  take no table cutoff; `tt_entry_answers()` de-normalises before comparing;
  reverse futility fail-soft with both mate-band edges; null move guarded at
  both edges (S165), one real ply kept, mate scores clamped to beta; the
  PVS/LMR re-search chain is the standard three-step form and aborts are
  checked before any result is consumed; the root fail-high replaces the PV
  row with the cutoff move; the quiescence DEC-102 store degradation holds on
  every path re-derived here, as the 2026-08-22 reading found; the mate
  round trip is held by the `static_assert` at `src/search.cpp:215`. The
  mate-line completion (S147/S170/S171) is reporting-only and all-or-nothing
  as claimed: no store, no node counted, `game` restored.
- **Time management and the UCI layer.** Hard limit clamped to `remaining -
  MOVE_OVERHEAD_MS` and floored (S036), soft clamped to hard, scaling never
  raises the hard limit, `go movetime` never scaled; timer disarm under one
  mutex; a checkmated root answers `info score mate 0 ... pv` per iteration
  and `bestmove 0000`; every mutation of `game` or the table joins the search
  first.
- **Stack use is bounded.** `otool` reads a 3552-byte `negamax` frame and a
  2352-byte `quiescence` frame; 128 plies of the larger is 444 KB, plus the
  ~85 KB `search_state_t` on the search thread's stack, against the 512 KB
  macOS secondary-thread default -- inside it until about ply 110, which no
  search here reaches.
- **The magic tables are the project's own output.** `src/bb_tables.hpp`'s
  128 constants were produced in-repo by commit `a5dbe68` ("Generatd magic
  numbers", 2023-03-28), whose generator uses the xorshift seed 1804289383
  from the series the `bitboard` branch followed -- so the numbers coincide
  with that series' published set. Inherited foundation under DEC-013, origin
  in git; whether that wants a DEC-121-style sentence is the owner's call and
  is not a finding.
- **No test was weakened.** The 28 assertion lines removed from `tests/`
  since 2026-08-22 are S024's reverted continuation-history cases (DEC-111)
  and S172's golden-surface refresh after `MANUAL.md` and `specs.md` (the
  SURFACE rule); `test_evaluation.cpp`'s band-edge assertions were swapped and
  swapped back with S024. The new S162, S164, S165, S170 and S171 cases each
  assert their precondition before their claim; `test_mate_pv` and
  `test_mate_carry` require a non-zero count of mate lines before asserting
  completeness.
- **Class 2, the project's own rules: nothing found.** DEC-127's mate-distance
  reading is stockfish- and python-chess-backed; S168's set is enumerated and
  corroborated; the S172 and S146 behaviour changes are discharged on the
  default configuration exactly as INV-6 allows and the node counts above
  confirm it; no source, table or training data from another engine beyond
  DEC-121's ruled constants.
- **Class 3, techniques behind the literature: nothing outside the plan.**
  Checked against the code and named to their pending step: null move
  pruning has no `static_eval >= beta` gate and a fixed `3 + depth/6`
  reduction that fires only from depth 5 (`src/search.cpp:814-823`) -- S114,
  whose own text records the missing gate; late move reduction is depth and
  move-number only, from move 4 at depth 3, with no history, node-type or
  improving term (`:960-966`) -- S098; no late move, futility, history or
  quiet SEE pruning in the move loop -- S109; no SEE pruning of captures in
  the main search -- S091; no internal iterative reduction -- S095; no
  singular extension or multicut -- S097; no correction history -- S099;
  quiescence searches no promotions and has no per-move futility -- S131,
  S112, S022; the table is direct-mapped 24-byte entries -- S119; the
  evaluation clamp, mobility and king-safety forms -- S039, S121, S122;
  `improving_at()` exists and has no consumer -- S109. Absent and in no step:
  mate-distance pruning and a per-node reset of the killers two plies down,
  both small in the published record and not worth a finding at this
  strength; any Elo attached to any of the above is a hypothesis for an SPRT,
  never a conclusion (DEC-019).
- **`fastchess.sh` re-read against the S160 and S167 changes**: the default
  reference is HEAD with both dates printed, the A/A guard reads state, the
  trap does not trust `$?` on bash 3.2, every git call is anchored to the
  script, the candidate is snapshotted before game one. `test_fastchess_script`
  is green. `books/fetch_book.sh` verifies the tracked PGN against its pin.
- **`MANUAL.md`'s surface against the code**: the five option lines, the three
  refusal templates, the six-field `position fen` statement, the `info` field
  table and the book section all trace to the code paths that produce them;
  `test_uci_surface` holds the first three.
