id:         S042
goal:       set the en passant square only when an enemy pawn can take it, so transposing move orders share a hash
accepts:    perft counts are identical over the existing suite; two move orders reaching the same position produce the same hash and the same FEN; generate_FEN's fourth field agrees with Stockfish over a corpus sample; **re-scoped by DEC-187 as a bug fix**: a red-first case pins the reproduction in the section below -- history A scores the draw at depth 4 after the fix, observed red on HEAD at -313 -- and a second case asserts the incremental key equals the recomputed full hash after a double push with and without a capturing pawn; the key moves in every place it is built, `make_move`, `load_FEN` and the full-hash recomputation; fastchess's "PV continues after threefold repetition" count over a 1500-game self-match at the S198 regime is recorded before (236 in S219's pass 1) and after, and falls to zero or the residual is named as a second cause; the tree changes, so the commit carries `Bench:` and the Debug binary self-plays four rounds at 4+0.04 with `Assertion` grepped (DEC-141); one `--nonreg` SPRT against the parent decides it, pre-registered per DEC-143 with its worst-case games and naming this defect per DEC-171; `MANUAL.md` and `adocs/specs.md` state the en passant convention where they describe the FEN or the repetition rule
touches:    src/bitboard.cpp make_move_impl, load_FEN, set_en_passant, compute_full_hash, tests/test_audit_fen_semantics.cpp
excludes:   any other zobrist change, including the single side-to-move key that was S031 -- retired, no successor step, `adocs/plan.md` "under 1 % by its own file, below every instrument here"
decisions:  DEC-187, DEC-171, DEC-173, DEC-141, DEC-143
closes:     2026-08-13_adversarial-F08
blocks:
paused_by:
author:     a Sonnet 5 subagent briefed by the coordinator (DEC-185, DEC-188); started 2026-09-12 02:00
done:

## What happens now

`src/bitboard.cpp` `make_move_impl` sets `new_en_passant` after every double
push with no test for an enemy pawn that could capture onto it. Two move orders
reaching the same position hash differently:

```
order 1  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2
         hash 11800988595472028807
order 2  fen rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq d3 0 2
         hash 2736264005390963272
placement equal: yes
hashes equal:    no
```

(1.d4 d5 2.c4 against 1.c4 d5 2.d4.) Over 200000 corpus positions, 9550 carry an
en passant square and only 392 of those have a capture available — so on 4.6 %
of all positions the square is one no pawn can use.

## A correctness bug after all -- 2026-09-11, DEC-187

The section this replaces was headed "Not a correctness bug" and argued that
repetition detection could not be hurt because a position carrying an en
passant square has `halfmove_clock == 0` and the lookback is bounded by that
clock. The lookback is `back <= limit` and the tainted entry sits at exactly
`back == halfmove_clock`, the last iteration: the argument was off by one, and
the 2026-08-13 audit's F08 triage ("Efficiency, not correctness") inherited it.

**What S219's first match showed, 2026-09-11.** 236 fastchess warnings "PV
continues after threefold repetition" in 1500 self-play games. A subagent
replayed every one with python-chess: **236 of 236 are genuine threefolds**
at the flagged node, the earlier occurrences before the root (184) or at the
root (52), never inside the line. In every case the oldest occurrence is the
position immediately after a double pawn push. `src/bitboard.cpp` `make_move` sets
`en_passant` and xors `ep_randoms` on every double push with no test for a
capturing pawn, so that occurrence hashes differently from the identical
position reached otherwise, and `src/bitboard.cpp` `classify_repetition`'s key compare
skips it. The DEC-173 logic is right; the key it compares is not. **52 of the
236 publish a non-zero score for a drawn line**; 184 still read 0 because the
search meets a later repetition one ply deeper -- right by accident. Depths 2
to 27.

**Minimal reproduction, depth 4, White a rook down.** Same position, same
FIDE history (python-chess `can_claim_threefold_repetition()` is True after
the last move in both):

```
A  position fen r5k1/8/8/8/8/8/6PP/6K1 w - - 0 1 moves g2g4 a8b8 g1f1 b8a8 f1g1 a8b8 g1f1 b8a8
   go depth 4 -> score -313  pv g4g5 a8a2 h2h4 g8g7
B  position fen r5k1/8/8/8/6P1/8/7P/6K1 b - - 0 1 moves a8b8 g1f1 b8a8 f1g1 a8b8 g1f1 b8a8
   go depth 6 -> score 0     pv f1g1
```

A alone is the red-first case: after the fix it scores the draw like B.

**What the fix must cover.** The key changes in every place it is built --
`make_move`, `load_FEN` and the full-hash recomputation -- as
`2026-09-04_plan_review-F08` already said, and one test asserts the
incremental key equals the recomputed one after a double push with and
without a capturing pawn. `set_en_passant` in `touches:` has zero callers and
S213 deletes it; whichever step lands first, the other adjusts. The tree
changes, so the commit carries `Bench:`, the Debug binary self-plays four
rounds (DEC-141), and one `--nonreg` SPRT decides the step, pre-registered
per DEC-143 and naming this defect per DEC-171. fastchess's warning count is
the instrument: recorded before and after over the same games, it falls to
zero or the residual is a second cause.

What the old section said about transpositions and FENs still holds and is
now the smaller half: 4.6 % of quiet positions get a key the same position
reached by another move order cannot match, and `generate_FEN`'s fourth field
disagrees with Stockfish's convention on every double push.

## Shape

**The rule, written once and applied at every site that decides the square
(S184, F08).** An en-passant square is kept only when a pawn of the side to
move stands on a square from which it attacks the target -- the generator's own
candidate test, `tables->pawn_attacks[opponent][board->en_passant]` masked with
the mover's pawns in `generate_moves_body`. Pseudo-legal: pins are not
examined, as in the generator before its own legality check, and as X-FEN
specifies ("only specified after a double pawn push was made beside an opponent
pawn that might capture en passant **if legal**",
https://www.chessprogramming.org/Forsyth-Edwards_Notation). The owner picked
this reading over the fully legal one on 2026-09-08, so the Stockfish
comparison in `accepts:` runs against python-chess `en_passant='xfen'` and not
its `'legal'` default (https://python-chess.readthedocs.io/en/latest/core.html).

**It is not one site, and that is what F08 found.** The key is built in more
than one place and the places must agree; change `make_move_impl` alone and a
position reached by moves drops a non-capturable square while the same position
loaded from a FEN keeps it -- the same transposition mismatch this step exists
to remove, moved from move order to input path, and every SPRT game starts from
`position fen`. From grep over `src/bitboard.cpp` at 2026-09-08:

| function | what it does | rule applies? |
|---|---|---|
| `make_move_impl` | sets `new_en_passant` on `move.double_push`, xors `ep_randoms` out and in | **yes** -- one `pawn_attacks` lookup at the point where the square is currently set unconditionally |
| `load_FEN` | parses field 4; the S161 sanitiser keeps the square when the target is empty and the victim stands behind it -- it tests the victim's presence, not whether any pawn can capture -- then keys with `compute_full_hash` | **yes** -- the `supported` test gains the capturer term |
| `set_en_passant` | sets any square and re-keys; declared in `src/bitboard.hpp`, **no caller anywhere in `src/`, `tests/` or `tools/`** | **yes if it is kept**; whether to delete it instead is this step's call, since deleting is a `src/` change |
| `compute_full_hash` | xors `ep_randoms[board->en_passant]` unconditionally, `INVALID_INDEX` included | no -- a consumer, and it must keep agreeing with the incremental key |
| `make_null_move`, `unmake_null_move`, `unmake_move_impl` | clear or restore through the history entry | no -- consumers |
| `generate_moves_body` | the capture from `pawn_attacks[opponent][en_passant]` masked with the mover's pawns | no -- it is the rule's own expression |
| `generate_FEN`, `cleanup_board` | prints field 4; resets to `INVALID_INDEX` | no |

The four S161 cases in `tests/test_audit_fen_semantics.cpp` -- "a real ep
square survives and is playable", "an ep square on the mover's own rank goes",
"an occupied ep target square goes", "a cleared field is cleared in the hash"
-- all move through the `load_FEN` change and are where its red-first evidence
comes from.

## How it is measured

Perft counts stay identical, because the legal moves do not change. The search
tree does not stay identical, because the keys do. That is behaviour-neutral by
INV-6's first test and not by its second, so it takes an SPRT rather than a node
count comparison.

## Evidence taken, 2026-09-12

**The rule, one helper.** `src/bitboard.cpp` gains
`en_passant_is_capturable(tables, board, target, capturer)`, a direct lift of
`generate_moves_body`'s own candidate test
(`all_pawns & tables->pawn_attacks[opponent][board->en_passant]`), and it is the
only place the rule is written. `make_move_impl` calls it with `capturer =
them` before setting `new_en_passant`; `load_FEN`'s S161 `supported` test calls
it with `capturer = board->active_color` as a third term alongside the
existing target-empty and victim-present checks, replacing neither.
`compute_full_hash`, `unmake_move_impl`, `make_null_move`/`unmake_null_move`
and `generate_moves_body` are untouched -- consumers, as the table above said.
**`set_en_passant` is deleted**, `src/bitboard.cpp` and its declaration in
`src/bitboard.hpp`: grepped for callers across `src/`, `tests/`, `tools/`
immediately before deleting it, zero anywhere, and the build is clean without
it in both Release directories and Debug.

**Red-first, both new-test axes, quoted from actual runs (`git stash push --
src/bitboard.cpp src/bitboard.hpp`, rebuild, run, `git stash pop`, rebuild,
run):**

1. `tests/test_en_passant_key.cpp` "a pre-root repetition through a
   non-capturable" (the case's title continues "en passant square scores the
   draw" in a second string literal, which the citation checker cannot
   stitch, so the citation stops where the first literal does; the step's
   history A,
   `go depth 4` through `search()`): red on HEAD --
   `REQUIRE_EQ( result.score, 0 )` values `REQUIRE_EQ( -313, 0 )`, matching the
   `-313` this file already recorded from the audit's own reproduction to the
   digit; green after -- `4 | 4 passed | 0 failed`.
2. The same file's "incremental key equals the recomputed hash..." case, the
   two *without a capturing pawn* sub-cases: red on HEAD --
   `CHECK( game.board.en_passant == c.expected_ep )` values `CHECK( 44 == 64 )`
   and `CHECK( 20 == 64 )` (the square stayed set where the fix clears it to
   `INVALID_INDEX`); green after. The *with a capturing pawn* sub-cases were
   green on both sides, expected -- `compute_full_hash()` has always agreed
   with whatever `make_move_impl` currently left in the field, on one board;
   the bug was never an incremental-versus-recomputed disagreement, it was two
   *different* positions disagreeing (next case).
3. "two orders of the same two pawn moves hash equal and print the same FEN"
   (1.d4 d5 2.c4 against 1.c4 d5 2.d4): red on HEAD --
   `CHECK_EQ( order1.board.en_passant, INVALID_INDEX )` values
   `CHECK_EQ( 42, 64 )`, `CHECK_EQ( order2.board.en_passant, INVALID_INDEX )`
   values `CHECK_EQ( 43, 64 )`, then `REQUIRE_EQ( order1.board.hash,
   order2.board.hash )` values `REQUIRE_EQ( 17059528876378166497,
   2131247343637083859 )` -- the class this file's "What happens now" section
   already named, reproduced independently by the new test rather than quoted;
   green after, both `INVALID_INDEX`, hashes equal.
4. `python-chess` 1.11.2 on history A's position after the eighth move, the
   oracle for the red-first case above: `can_claim_threefold_repetition()`
   **True**, `outcome(claim_draw=True)` **`Termination.THREEFOLD_REPETITION`**
   -- a genuine threefold, independent of the engine.

`tests/test_audit_fen_semantics.cpp`'s S161 and S208 cases: unchanged,
**20 test cases, 65 assertions, 0 failed** -- the four S161 cases the shape
table cites are among them and none encodes the old convention (they turn on
target-empty/victim-present/rank, not on whether a pawn can reach the target).

**Numbers.**

- `tools/search_bench.py` depth 9, before (HEAD) -> after: midgame
  `121530 nodes, c3d5` -> `121515 nodes, c3d5`; kiwipete `801481 nodes, e2a6`
  -> `801408 nodes, e2a6`; tactical `72924 nodes, d7c8q` -> `72895 nodes,
  d7c8q`. Best moves unchanged at all three; node counts move by a few dozen
  each, the expected shape for a change that is play-altering by construction
  and touches none of these three positions' own en-passant field (all three
  are bare FENs with `-`) -- the difference comes from a repeated position
  somewhere inside these particular trees, the same class S207 measured.
- `chesso bench`: **30046849 -> 27322394**, -2724455 nodes, -9.07 %. Commit
  carries `Bench: 27322394`. `DEV_MANUAL.md`'s ledger line gains this entry.
- FEN conformance, the full corpus and not a sample: `adocs/data/
  S219_aa_calibration.pgn`, all **1000 games**, every position from each game's
  start (its own book FEN, from the PGN's `SetUp`/`FEN` header -- these are
  self-play games from an opening book, not from the default position, so
  `build/tools/pgn_to_positions` cannot replay them, it hardcodes
  `DEFAULT_POSITION`; driven instead through the engine directly, one
  persistent UCI process per game, `position fen <book FEN> moves <uci...>`
  then `fen` -- `command_fen` -> `generate_FEN`, no `go` anywhere, so the
  `quit`-races-`go` trap does not apply) through its final position:
  **115021 positions total**. Positions with an en-passant square: **4091**
  under the old (unconditional) convention (`python-chess`'s raw `ep_square`,
  which is what the pre-fix engine also produced for a position reached by
  real play, since S161's target-empty/victim-present checks are automatically
  satisfied by construction of an actual double push); **132** under the xfen
  oracle (`board.fen(en_passant='xfen')`) and **132** under the fixed engine's
  own output -- **0 disagreements** over all 115021. Ratio of capturable to
  all double-push squares in this corpus, 132/4091 = 3.2 %, the same order as
  the 392/9550 = 4.1 % this file already recorded over a different 200000
  position corpus.
- Perft: `ctest --test-dir build -L slow` (`test_perft`), **100 % passed,
  52.94 s** -- identical, as INV-1 requires and as the unaffected candidate
  test in `generate_moves_body` predicts.
- Debug self-play (DEC-141), `build-debug`, DEV_MANUAL.md's recipe verbatim,
  4 rounds at 4+0.04, concurrency 8, `noob_3moves.epd`: **8 games in 17 s,
  0 `Assertion`, 0 `disconnect`**, both engines closed gracefully. One game
  ended "Draw by 3-fold repetition", one "Draw by fifty moves rule".
- Gate, both directories, `CLANG_FORMAT_MAJOR=22`: `cmake --build build -j8`
  exit 0; `ctest --test-dir build -L fast` exit 8, **30 of 34 passed**;
  `cmake --build build-tune -j8` exit 0; `ctest --test-dir build-tune -L fast`
  exit 8, **30 of 34 passed**, the same four names in both directories;
  `./clang-format.sh --check` exit 0. The four failures are not this step's
  own tests and are not touched -- see "A pre-existing corpus finding" below.
- `tools/gate_extra.sh`: launched `CLANG_FORMAT_MAJOR=22 nohup tools/gate_extra.sh`,
  detached, polled to its marker. **`GATE-EXTRA-FAILED: debug sanitize`**,
  1502 s wall, the stages summing to 1462 s and the remaining 40 s spent between
  stages (`prose` 0 s, `citations` 0 s, `debug` 891 s, `sanitize` 516 s,
  `perft` 55 s). Both failing stages are the same corpus finding below and
  nothing else: `debug` runs six binaries and two of them are on the known
  list, `test_movegen` (`Failed`, the usual FEN mismatch) and `test_chesso`
  (`Timeout`, its own paragraph below); `sanitize` runs the whole `fast` label
  under `RelWithDebInfo -DSANITIZER=ON` and reports the same four names as
  every other build here, `test_chesso` included, this time as a clean
  `Failed` in 0.28 s with no sanitizer warning at all -- `RelWithDebInfo`
  defines `NDEBUG` exactly as Release does, so the `assert` the Debug build
  hits never fires there, and `test_chesso` runs to completion the same way it
  does in Release: 12 test cases, 2 failed, 16743 of 16745 assertions passed.
  Because `ctest -L fast` returned non-zero, `stage_sanitize()`'s own
  Release-versus-sanitizer bench-signature comparison (its last two commands)
  never ran -- not a defect this step introduced, just a check this run could
  not reach. `perft` (`ctest -L slow` in `build`) passed clean, 55 s,
  independent of the `ctest --test-dir build -L slow` run quoted above.

## A pre-existing corpus finding, not this step's to fix, 2026-09-12

**Four already-existing test binaries fail after the fix, all for the same one
reason, none of them touched.** `tests/assets/test_jsons/*.json` (`castling`,
`checkmates`, `famous`, `pawns`, `promotions`, `stalemates`, `standard`,
`taxing`; provenance `tests/assets/test_jsons/README.md`, "taken from
https://github.com/schnitzi/rampart") and `tests/test_corpus_dedupe.cpp`'s own
`AFTER_C5_EP`/`AFTER_C5_NO_EP` pair and `tests/test_audit_polyglot_key.cpp`'s
ten hand-built S175 cases all encode the classical FEN convention this step
replaces: an en-passant square recorded after every double push, capturable or
not. Quantified over the rampart corpus with `python-chess` (script kept
neither in `tests/` nor `adocs/data/`, a one-off): of 2696 distinct FENs across
the eight files, 138 carry a non-`-` fourth field and **126 of those 138**
disagree with `board.fen(en_passant='xfen')` -- the oracle this step's own
`accepts:` names. This is exactly what `2026-08-13_adversarial-F08`'s "Not a
correctness bug" section (now removed) already measured in the large --
"4.6 % of all positions" -- just concentrated onto four small, hand-built or
third-party fixtures rather than spread across a 200000-position corpus.

**What breaks, one root cause each time:**

- `tests/test_movegen.cpp` "well formed FENs still load" (`all_test_fens()`,
  `tests/test_helpers.hpp`): a literal `generate_FEN(&game.board) == fen`
  string check over every corpus FEN. First failure quoted:
  `REQUIRE_EQ( generate_FEN(&game.board), fen )` values
  `5rk1/1p4pp/4p3/p1R3Q1/3n4/2q4r/P1P2PPP/5RK1 w - - 0 2` against
  `... w - a6 0 2` -- the black pawn that just pushed is on a5, but b5 (the
  only square a white pawn could capture from) is empty, so `a6` was never
  capturable and the fixed engine correctly clears it.
- `tests/test_chesso.cpp` "Test fen parsing - generation" and "Test against
  generated jsons": the same shape, same corpus, different harness
  (`test_files`, this file's own copy of the list). Same class, first failure
  at a different corpus entry.
- `tests/test_corpus_dedupe.cpp` "castling rights, the en passant square and
  the side to move all separate": its own comment names this step by number --
  *"Nothing in the corpus format or in load_FEN checks that a pawn can
  actually take, which is `2026-08-13_adversarial-F08` and S042; the key
  covers the square either way and so does this pair."* `AFTER_C5_EP`
  (`... w KQkq c6 0 2`) has no white pawn on b5 or d5, so under the fix it
  hashes identically to `AFTER_C5_NO_EP` -- which is correct, the two are the
  same position, and is exactly why the case fails: `CHECK( stats.rows_written
  == 2 )` values `CHECK( 1 == 2 )`, one row dropped as a duplicate rather than
  kept as distinct.
- `tests/test_audit_polyglot_key.cpp` "the Polyglot key follows the format on
  an edge-file en-passant square" (S175, `2026-09-03_adversarial-F01`): all ten
  cases are constructed so a same-side pawn sits on the *wrapped* square the
  bug reached through unmasked +-7/+-9 arithmetic, and eight of the ten have no
  pawn on the *real* capturing square -- confirmed with `python-chess`, xfen
  clears all eight, `has_legal_en_passant() == False` on all eight. The case's
  own precondition, `REQUIRE(game.board.en_passant != INVALID_INDEX)`, now
  fails on the first of the eight: `REQUIRE( 64 != 64 )`. Only the two
  "control" cases (a genuine capturer beside the pushed pawn) still reach
  `get_key()` through `load_FEN`. One of those two, "h6 ep, white pawn on g5
  captures", happens to also carry a white pawn on a4 (the wrapped square for
  `h6`), so the wraparound-masking path this file exists to guard is not
  entirely unreached -- but the other eight cases, built to isolate that path
  more directly, no longer can be through `load_FEN`. Whether `get_key()`'s own
  correctness still has adequate coverage after this step is a question this
  step does not resolve.

**Debug-only, more severe in consequence, same root cause: `test_chesso`
crashes rather than merely failing, and the crash is in the test's own
diagnostic-message code, not in `src/`.** Reproduced under `gdb` (`build-debug`,
this fix, `run --test-case="Test against generated jsons"`): `SIGABRT`,
`Assertion 'move_belongs_to_side_to_move(&game->board, encoded_move)' failed`
inside `make_move_impl` instantiated for Black (`src/bitboard.cpp`), called from
`move_to_algebraic` (`src/bitboard.cpp`), called from `difference_to_string`
(`tests/test_chesso.cpp`), called while building the message argument to the
`REQUIRE_MESSAGE` in `tests/test_chesso.cpp` "Test against generated jsons" --
which is the exact assertion that fails first on this corpus (the FEN string
mismatch above). `move_to_algebraic` calls `make_move` internally, and by the
time the message is built, `&game`'s board has already been advanced by the
preceding `make_move(&game, move_to_make)` in that same test case, which
*succeeded* -- the failure is only the printed FEN, not the move. The message
builder then replays `moves[]`, an array of legal moves generated **before**
that make_move, against the **now-mutated** board, and one of those stale
moves belongs to the side that was to move before, not after -- tripping the
assert `make_move_impl` uses to catch exactly a caller's mismatch like this.
This landmine has existed in `difference_to_string`/`moves_to_string` since
they were written; nothing before this step ever made the enclosing
`REQUIRE_MESSAGE` fail on this corpus, so the message was never built, so the
stale-array bug was never exercised. Confirmed absent on HEAD: the identical
`--test-case` run against a build without this step's change completes in
**1.089 s, 8258 assertions, 0 failed** -- no divergence, no message ever built.
In Release the same stale call is silent (the assert compiles out) and merely
prints a wrong difference/board diagram, which is what the very first Release
run of this suite showed and which was not, at the time, recognised as a
symptom. Under `ctest`'s Debug timeout (600 s) the same run is reported
`Timeout` rather than the assert `gdb` shows directly, apparently because how
long the corpus takes to reach its first divergent entry (deep in
`castling.json`) is sensitive to machine load, and other work was on the
machine at the time; nothing here supports "infinite loop" over "slow, then a
real and reproducible abort," and the `gdb` backtrace is the stronger claim of
the two. **Scope under DEC-171**: this fires only inside a test binary's own
error-message construction, never in `src/`'s search, move generation or the
UCI surface, and moves no reported score -- filler-scheduled, not a block on
this step or the next one.

**None of the four files above, and no JSON fixture, was edited.** Per this
step's own instruction: report, do not relax. A fix would mean either
correcting the fixture data (the corpus is third-party for the JSON files, and
the two hand-built files' authors already half-anticipated this in one case's
own comment) or changing what the affected tests assert (a literal FEN
round-trip is not a defensible property once two conventions can both be
correct FENs for the same fourth field's presence) -- both are calls for the
coordinator, not for this subagent.

## Measurement, pre-registered 2026-09-12

Modelled on `adocs/plan_done/S207_repetition_before_root.md`'s "## Measurement".

**`./fastchess.sh --nonreg`**, `elo0=-5 elo1=0` (nElo), `alpha=beta=0.05`,
against the parent commit `a3e84e1` (`REF=a3e84e1 ./fastchess.sh --nonreg`;
the candidate is this step's committed tree, built from a working tree equal
to it, so the banner prints both shas) -- the regime DEC-189 set: 8+0.08,
`Hash=16`, `books/noob_3moves.epd`, all 12 threads.

**Worst-case games**, from the nElo run-length formula, DEC-143's own figures:
**41861** at the interval's midpoint, **25591** on a bound. Converted at
**2110 games an hour** (DEC-190's calibration on this book, pair variance band
**0.2905 +/- 0.0184**): **19.84 h** at the midpoint, **12.13 h** on a bound --
12 to 20 hours worst case, a night run (DEC-155). Launch detached, `Monitor`
watcher armed with the WATCHERS loop, four exits, ceiling 2x the worst case
(~40 h).

**Abort rule**: forfeit rate over 1.0 % on a side, `tools/forfeit_report.py`
over the run's own PGN, checked independently rather than read off the
harness banner.

**The instrument**: fastchess's warning `PV continues after threefold
repetition - move ... from <engine>` names the side by engine name, so over
the run's own PGN the count attributed to `candidate` is this step's
after-figure and the count attributed to `ref-a3e84e1` is the before-figure,
both over the same games. The before-figure on this book at this regime is
**328**, the DEC-190 A/A of `5047070` against itself (1000 games, both sides
equally, since it is one binary playing itself). The candidate's count must be
zero or the residual is a second cause and is named as such, per DEC-187's own
consequence: "a count that does not fall to zero is a second cause."

**The open defect named per DEC-171's last sentence**: S042 itself -- present
on the reference side (`ref-a3e84e1`), fixed on the candidate side, for the
duration of this run.

**Readings, adapted from S207's shape:**

- **H1**: not a regression of 5 nElo or more against the reference. Kept; no
  magnitude is claimed -- an SPRT that stops on a favourable swing is biased
  upward by construction (DEC-063), and this is a bug fix decided by the SPRT
  rule (MEASUREMENT), not by the number.
- **H0**: costs 5 nElo or more. Before believing it: the candidate's "PV
  continues after threefold repetition" count over the run's own PGN must be
  checked first -- zero is the expected reading and does not by itself explain
  an H0 verdict, so a non-zero residual is the first thing to look at, and a
  wrong count is likelier than a wrong oracle (S207's precedent, DEC-063,
  DEC-103).
- **No verdict at the cap**: recorded as zero and kept, with the reason
  stated -- the fix removes a wrong draw score and a wrong key from ordinary
  self-play (16.47 % of S207's own reference games ended by threefold, per
  that step's stamp), corrects a real correctness defect DEC-187 named, and
  S005/S006/S015/DEC-103 are the precedent for keeping a measured zero.

**Verified before the run, not assumed**: `sha256sum` of the candidate binary
the match actually plays should be checked against the tree this stamp
benches, the way S207's stamp did, once the coordinator snapshots it for the
launch.

## Tests re-stated for the convention, 2026-09-12

The four pre-existing failures the section above named, treated: every case
kept, none deleted, none loosened beyond what the convention change strictly
requires, each stated at its site with an oracle quote. A Sonnet 5 subagent,
briefed by the coordinator.

**Site 1: `tests/test_movegen.cpp` "well formed FENs still load" and
`tests/test_chesso.cpp` "Test fen parsing - generation" / "Test against
generated jsons".** Mechanism: both round-trip a fixture FEN through
`load_FEN`/`generate_FEN` (the first two) or reach a fixture's expected
position by playing the move and comparing the result (the third), and all
three then compared the result string to the fixture's literal text --
including its fourth field, which predates X-FEN. First failure observed,
`test_movegen.cpp`: `REQUIRE_EQ( generate_FEN(&game.board), fen )` values
`... w - a6 0 2` (fixture) against `... w - - 0 2` (engine) -- the a6 fixture
square had no white pawn on b5 to retake with.

Treatment: `load_FEN` can only ever move the en-passant field one way, from a
parsed square to `"-"` (`en_passant_is_capturable` only clears, per the shape
table above; it never invents a square or relocates one), so a new helper
`matches_pre_xfen_fixture()` (defined once per file, `tests/test_movegen.cpp`
and `tests/test_chesso.cpp`, duplicated rather than shared to avoid a
`load_json`-name collision pulling `test_helpers.hpp` into `test_chesso.cpp`)
splits both FENs on whitespace (`split_string`, `src/utils.hpp`) and requires
placement, side to move, castling, halfmove clock and fullmove number exactly
equal, and the en-passant field either exactly equal or cleared to `"-"`.
"Test against generated jsons" reaches its expected position by `make_move`,
not by a direct load, so it instead loads the fixture's expected FEN into a
second `game_t` through the same `load_FEN` sanitiser and compares the two
boards' own `generate_FEN` output -- both canonicalised the same way, so a
real divergence in any other field still fails exactly as before.

Oracle, three fixture positions confirmed uncapturable with python-chess
1.11.2 (`chess.Board(fen).has_pseudo_legal_en_passant()` is False on all
three; quoted at both sites' comments):
- `castling.json rnbq1k1r/pp1Pbppp/2p5/8/P1B5/8/1PP1N1PP/RNBQK2n b Q a3 0 8`
  -- b4, the only square a black pawn could retake from, is empty.
- `castling.json rnbqk2N/1pp1n1pp/8/p1b5/8/2P5/PP1pBPPP/RNBQ1K1R w q a6 0 9`
  -- b5, the only square a white pawn could retake from, is empty.
- `castling.json rnbq1k1r/pp1Pbppp/2p5/8/2B3P1/8/PPP1N2P/RNBQK2n b Q g3 0 8`
  -- f4 and h4, the only squares a black pawn could retake from, are both
  empty.

Over the eight JSON files (2696 distinct FENs), 138 carry a non-`-` fourth
field and 126 of those disagree with the xfen oracle -- the same figure the
"pre-existing corpus finding" section above already recorded (script run
ad hoc at hand-off time, not kept; this step's own re-derivation is the two
sites' comments plus the three quotes above, checked directly against the
fixtures rather than a saved script, since the check here is "is the fixture
uncapturable", answered per example, not a re-derivable table of many
numbers). This is a re-statement, not a relaxation: every field but the
en-passant one is still compared exactly, and the en-passant field's only
permitted deviation is the one direction the sanitiser can move it.

**The Debug-only crash, fixed.** Mechanism, confirmed by reading
`tests/doctest/doctest/doctest.h`: `REQUIRE_MESSAGE(cond, msg)` expands to
`DOCTEST_INFO(msg); ASSERT(cond)`, and `DOCTEST_INFO` wraps `msg` in a
`ContextScope` holding a lambda (`MakeContextScope` in `tests/doctest/doctest/doctest.h`) --
`msg` (here, a string built from `moves_to_string`/`difference_to_string`,
which call `move_to_algebraic`, which calls `make_move` internally to
disambiguate) is evaluated only if that lambda's `stringify()` is invoked,
which doctest does only when an assertion is about to be logged as failed.
So the crash-carrying code never ran while this exact `REQUIRE_MESSAGE`
passed -- confirming the step file's own claim above ("nothing before this
step ever made the enclosing REQUIRE_MESSAGE fail on this corpus, so the
message was never built"). The mechanism itself: at the point that lambda
would run, `game.board` had already been advanced past `move_to_make` by the
`make_move` a few lines above, but `moves[]` was generated *before* that
`make_move`, for the side that was to move then. `move_to_algebraic`'s
internal `make_move` on one of those stale moves against the now-advanced
board trips `move_belongs_to_side_to_move` -- silently wrong in Release
(`assert` compiles out; the diagnostic prints a bogus difference/board) and
an abort in Debug. Fix (`tests/test_chesso.cpp`, "Test against generated
jsons"): `unmake_move(&game)` moved to before the `REQUIRE_MESSAGE` instead of
after -- it already ran there unconditionally on the next line, so this only
reorders two statements, and it restores exactly the board `moves[]` was
generated from before any diagnostic lambda can run. Small, and the site's own
comment names S042 as where it was found and states the mechanism predates and
is orthogonal to the convention change. Debug binary result: see the
`gate_extra.sh` "debug" stage below, which runs `ctest --test-dir build-debug`
over the whole suite including this exact test case.

Assertion counts (grep `\b(REQUIRE|CHECK)(_EQ|...)?\s*\(`, call sites, not
loop iterations): `tests/test_movegen.cpp` 76 -> 76 (one `REQUIRE_EQ` replaced
by one `REQUIRE_MESSAGE`, same site count); `tests/test_chesso.cpp` 70 -> 71
(two 1-for-1 replacements plus one new `REQUIRE(load_FEN(fen,
&expected_game))`). Runtime (`doctest`'s own count, full binary): test_movegen
90570143 assertions, 0 failed (was aborting at 90565327/90565326 passed before
the fix, doctest's `REQUIRE_EQ` being fatal); test_chesso 31041 assertions,
12 test cases, 0 failed.

**Site 2: `tests/test_corpus_dedupe.cpp` "castling rights, the en passant
square and the side to move all separate".** Mechanism: `AFTER_C5_EP`'s fourth
field (`c6`) has no white pawn on b5 or d5 -- confirmed uncapturable with
python-chess 1.11.2 (`has_pseudo_legal_en_passant()` False, `xfen` prints
`"-"`) -- so `load_FEN`'s sanitiser now clears it, and `AFTER_C5_EP` /
`AFTER_C5_NO_EP` key identically. First failure observed: `CHECK(
stats.rows_written == 2 )` values `CHECK( 1 == 2 )`; `CHECK( stats.rows_dropped
== 0 )` values `CHECK( 1 == 0 )` -- one row now dropped as a duplicate.

Treatment: the pairing's intent, "the en-passant field always separates", does
not survive X-FEN -- an uncapturable square is not part of the position, so
the two rows are correctly the same. The pair is removed from the "all
separate" list (replaced by a new pair, `AFTER_D5_EP` / `AFTER_D5_NO_EP`,
`... w KQkq d6 0 3` / `... w KQkq - 0 3`, confirmed genuinely capturable --
white pawn on e5 beside black's fresh double push to d5,
`has_pseudo_legal_en_passant()` True, xfen keeps `d6` -- so that property
still has a live example), and a new, dedicated test case, "an en passant
square with no
capturing pawn no longer separates two positions", asserts the merge directly:
old expected (pre-fix intent) `rows_written == 2, rows_dropped == 0`; new
expected `rows_written == 1, rows_dropped == 1`, first row survives byte for
byte (DEC-065). `AFTER_C5_EP` stays defined and used elsewhere in this file
purely as a third distinct FEN (placement alone already separates it from
`START` and `AFTER_E4` there, regardless of convention) -- unaffected, and
unchanged.

Assertion counts: 58 -> 62 (+4: one new `TEST_CASE`, `REQUIRE` + 3 `CHECK`;
the pair swap inside the existing loop changes no call site). Runtime: 12
test cases (was 11), 70 assertions (was 66), 0 failed.

**Site 3: `tests/test_audit_polyglot_key.cpp` "the Polyglot key follows the
format on an edge-file en-passant square".** Mechanism: this file's 10 cases
isolate a *different*, earlier bug (`get_key()`'s wraparound arithmetic on an
edge-file square, S175/2026-09-03_adversarial-F01) by putting a same-side pawn
on the *wrapped* square and none on the real one -- so 8 of the 10 (7
non-control cases plus the first control) have no genuine capturer. First
failure observed: `REQUIRE( game.board.en_passant != INVALID_INDEX )` values
`REQUIRE( 64 != 64 )` on the first case.

Treatment: `key_case_t` gains a `capturable` field; the blanket precondition
becomes an if/else on it (`en_passant != INVALID_INDEX` when true,
`== INVALID_INDEX` when false), and `CHECK_MESSAGE(get_key(...) ==
c.spec_key, ...)` is kept unconditionally, for every case either way -- a case
that never carries the square is still a case where the format says the key
must not carry the ep term, worth asserting rather than skipping. Two new
cases added on a non-edge file (`d6`, white captures; `d3`, black captures,
exercising `get_key()`'s `-8` branch the existing cases never reached),
because eight of the ten no longer reach `get_key()`'s own
`on_left`/`on_right` file-masked logic at all once `board.en_passant` is
cleared upstream -- the wraparound-fix code this file exists to guard was down
to two live exercises (the two original controls) without them.

Old/new `spec_key`: **unchanged for all ten original cases.** Re-derived with
`chess.polyglot.zobrist_hash()` -- python-chess's own `hash_ep_square()`
(`chess/polyglot.py`, `ZobristHasher` class) re-derives capturability itself
from `board.pawns & board.occupied_co[board.turn]` masked to the two adjacent
files, gated by its own comment "But only if there's actually a pawn ready to
capture it. Legality of the potential capture is irrelevant." -- it never
trusted the FEN's over-permissive fourth field to begin with, so nothing
about S042 could move its output. New cases' `spec_key`: `d6` ep, white
captures, `0x0CC1835B41412927`; `d3` ep, black captures, `0x3AE952459A0066BF`.
Oracle and re-derivation script: `adocs/data/S042_polyglot_key_cases.py`
(new, DEC-142) parses the twelve cases straight out of this test file (no
second copy to drift) and checks each one's `has_pseudo_legal_en_passant()`
against `capturable` and `zobrist_hash()` against `spec_key`; run against the
committed file, `12 cases, 0 mismatches`.

Assertion counts: 3 -> 4 call sites (the single precondition split into two
branches). Runtime: 10 cases (30 assertions) -> 12 cases (36 assertions), 0
failed.

**Any other failure.** `tests/test_movegen.cpp` "well formed FENs still load"
folds into site 1 above (same mechanism, same fixture corpus, same helper).
`test_plan_citation_freshness` -- **found by this subagent, out of its scope
to fix (the citation sits in the "Evidence taken" section above this
section's append point, and the checker script itself is outside the four
sites briefed), and fixed concurrently, above, while this section was being
written.** `tools/plan_prose_check.py --citations` was flagging this step
file's line 151, a phrase citation into `tests/test_en_passant_key.cpp`
quoting the full title `"a pre-root repetition through a non-capturable en
passant square scores the draw"`. That exact phrase is in the file, but split
across two adjacent C++ string literals (the test title wraps at 80 columns);
the checker's `TITLE` regex (`tools/plan_prose_check.py`) matches only the
first quoted string after `TEST_CASE_FIXTURE(fixture,`, so it never sees the
concatenated title, and the raw-text fallback (`holds_phrase`) also failed
because the two literals' adjacent closing/opening quote characters land
between "en" and "passant" once the file is whitespace-flattened, breaking
the substring match. Line 151 now quotes only `"a pre-root repetition through
a non-capturable"` -- a prefix that sits wholly inside the first literal, so
the raw-text substring fallback finds it without needing the checker to
stitch two literals together. Re-run directly, `0 flagged` (was `1 flagged`).
This was a pre-existing gap in the checker (it has never had to stitch a
two-literal-wrapped title before), surfaced only now because the citation
into this new file was written before the checker was run against it. Three
ways to close it were open (the subagent's own hard limits ruled out the
first two: the test file may not be edited beyond a compile-error fix, and
the checker script itself is outside the four sites briefed) -- reword the
C++ title to one string literal, reword this step file's citation to a
substring inside one literal, or teach `plan_prose_check.py`'s `TITLE` regex
to concatenate adjacent literals. The second is what was applied. The
checker's own gap (adjacent string literals in a doctest title) is otherwise
unfixed and will recur wherever a future title wraps the same way.

## Launch note, coordinator, 2026-09-12 03:57

`REF=a3e84e1 OUT=.tuning/s042_nonreg_20260912_035728 ./fastchess.sh --nonreg`,
detached, pid 2753254, console `.tuning/s042_nonreg.log`. Banner: candidate
`50b1ff9`, reference `a3e84e1`, 8+0.08, hash 16, concurrency 12 of 12,
`noob_3moves.epd`, seed `20260912035728`, bounds `elo0=-5 elo1=0
alpha=0.05 beta=0.05`; no busy warning. Candidate binary the match plays,
`build/src/chesso` at launch: sha256
`7dd144df70c130c0bc72b99cf47432e13cf4fa03f2ab84584301fbde9f200252`, the tree
this stamp benches (27322394). Governor `powersave`, on mains. Watcher armed
through `Monitor`, persistent: `SPRT-RUN-(DONE|FAILED)`, process death, 40 h
ceiling, polled every 60 s.

**Two launches before this one were aborted within two minutes each and are
not evidence**: `.tuning/s042_nonreg_20260912_035128_ABORTED_busy_core` and
`..._035437_ABORTED_busy_core`. Both banners warned "about 109 % of a core is
already busy": a gate run (`ctest -L fast` over the Release trees) was still
executing on the machine -- the test subagent, woken by its own watcher after
its brief had been fulfilled and the commit made, had started the full gate
its brief asked for. It was stopped, its processes ended by exact name, and
the machine checked quiet (top process 2.7 %) before this launch. The
pre-registration is unchanged; only the seed differs between the three
banners, as it must.
