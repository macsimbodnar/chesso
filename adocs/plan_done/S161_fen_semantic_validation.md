id:         S161
goal:       load_FEN clears castling rights and en-passant squares the position on the board cannot support, so accepted input can no longer corrupt the board
accepts:    the three FENs in the finding load with the unsupported right or en-passant square cleared rather than rejected — clearing is what fastchess and cutechess do to the same input, and a hard reject would break GUIs that send stale fields; `tests/test_audit_fen_semantics.cpp`, written red-first by the audit, is registered in `tests/CMakeLists.txt`, observed red against the unfixed tree, and green after; the Debug `squares_match_bitboards` and king-safety asserts hold across the suite; `tools/search_bench.py` node counts and best moves are identical, because the sanitization can only alter positions the engine previously corrupted (INV-6); MANUAL.md's UCI surface and known-bugs sections checked
touches:    src/bitboard.cpp, tests/CMakeLists.txt, tests/test_audit_fen_semantics.cpp, MANUAL.md
excludes:   full position-legality validation (side not to move in check, pawn counts, doubled kings) — only the two semantic classes the finding proves corrupting; hardening `castling_rook()` or `make_move()` themselves — the load boundary is the contract every downstream consumer already assumes
decisions:
closes:     2026-08-22_adversarial-F01
blocks:
paused_by:
done:      load_FEN clears unsupportable castling rights and en-passant squares; audit test red-first 4/4 on 0a274c9, 12/12 green with 8 added cases, mutation-checked both directions; INV-6 identical node counts and best moves; perft green; Debug fast suite 20/20

## Evidence

2026-08-22_adversarial-F01, the audit's one medium correctness finding and
the one prime-directive violation found. `load_FEN()` validates syntax only:
castling rights are parsed as bits with no check that king or rook stands on
its square (`src/bitboard.cpp:1699-1737`), the en-passant square only has to
parse as `[a-h][1-8]` (`src/bitboard.cpp:1755-1771`). The generator and
`make_move()` trust both, and the piece updates are xors — applied to a
square the piece is not on, they create pieces. In Release the corruption is
silent and every subsequent probe, generation and evaluation runs on a board
that is no position. Perft (INV-1) can never see it: every perft FEN is
semantically valid. Exposure is any `position fen` from a GUI, test or tool
carrying stale rights or a stale ep square.

## Cost

Small boundary change in `load_FEN`, one test registration, no match — the
INV-6 node-identity check is the verdict.
author:    Maksym Bodnar


## Result

**Done 2026-08-22.** `load_FEN()` clears both classes; nothing is rejected.
`2026-08-22_adversarial-F01` is `planned`, not `closed` -- AGENTS.md par.9 puts
a finding at `closed` only on a re-run that no longer reports it.

### What the fix is

One block in `load_FEN` (`src/bitboard.cpp`), placed after the six sections are
parsed and **before `compute_full_hash()`**, which is the only placement that
makes a sanitized position hash as the position it actually is.

- A castling right survives only if the king and *that* rook are on their
  squares. Six independent tests, so the clearing is per right rather than
  wholesale: a missing king clears both of a side's rights, a missing rook
  clears one.
- An en-passant square survives only if it is on rank 6 with White to move or
  rank 3 with Black, its own square is empty, and the victim square holds an
  enemy pawn. The rank test is also a memory guard and is written to short
  circuit: off that rank, `en_passant +/- 8` leaves the board and `squares[]`
  would be read past its end.

### Evidence, in order

**Red observed, not assumed.** The audit's four cases, registered and run
against `0a274c9`: **4 of 4 failed**, each `CHECK_EQ( 1, 0 )` -- one castling or
en-passant move generated where none is legal.

**The audit's test was non-vacuous in one direction only, and that is fixed.**
All four cases are negative assertions, so a fix that cleared *every* castling
right and *every* en-passant square would have passed all four and shipped a
worse engine on a green suite. Eight cases were added: two pure positive
controls, per-right partial clearing for both colours, the two ep classes the
audit did not write (an ep square on the mover's own rank, an occupied ep
target square), and a hash-equality case pinning the placement above. Both
directions are then mutation-checked rather than argued:

| tree | result |
|---|---|
| unfixed `load_FEN` | 10 of 12 fail -- every negative case and every "is cleared" assertion is red-first; the 2 that pass are the pure positive controls, which were always true |
| over-broad mutant (`castling = 0`, every ep square dropped) | 4 of 12 fail -- the positive controls catch it |
| the fix | 12 of 12 pass, 29 assertions |

**INV-6 discharged on node counts, no SPRT owed.** The sanitization can only
alter positions the engine previously corrupted, and `search_bench.py`'s three
FENs are all valid. Two interleaved passes, depth 9, against a `0a274c9` binary
built from the same tree:

```
HEAD  121512 / 800769 / 62907   c3d5 / e2a6 / d7c8q
S161  121512 / 800769 / 62907   c3d5 / e2a6 / d7c8q
```

Identical both passes. Times within noise (0.152 s to 0.157 s total, under the
3 % floor).

**INV-1 unaffected**: `ctest -L slow` -- `test_perft` passed, 57.92 s. Every
perft FEN is semantically valid, as the finding predicted, so no count moved.

**The Debug asserts hold**: `ctest --test-dir build-debug -L fast` **20 of 20**,
250.93 s, with `squares_match_bitboards()` and the king-safety assert live.
Release `ctest -L fast` **20 of 20**, 16.26 s. `./clang-format.sh --check`
clean.

### Deliberately not done

- **Position legality at large** stays unchecked, per `excludes:` -- pawn
  counts, two kings, the side not to move in check.
- **The ep push-origin square is not tested.** An ep of e6 with a black pawn on
  e5 and any piece on e7 is a position no double push produced, and it is still
  accepted. It is legality and not corruption: nothing xors anything onto e7.
  Every condition implemented here is one the finding proves creates pieces.
- **`castling_rook()` and `make_move()` are not hardened**, per `excludes:`.
  The load boundary is the contract every downstream consumer already assumes,
  and hardening the interior would be a second change in the same step.

### Two things found on the way, neither fixed here

- **The audit misnamed its own finding.** `tests/test_audit_fen_semantics.cpp`
  cited `2026-08-22_adversarial-F02` in its header and in its `TEST_SUITE`
  name; F02 is `fastchess.sh`'s default reference, which S160 closed. Corrected
  to F01 in the test file, which is in `touches:`.
- **F02's status line in the audit report reads `closed — S160`**, where
  AGENTS.md par.9 allows only `planned` before a re-run. Not edited: it is
  S160's record and another step's finding. Owner's call.
- **`tools/search_bench.py`'s docstring says depth 9 is 3136397 nodes**; the
  three positions sum to 985188 on this tree. Stale, outside `touches:`, and
  not touched.

### Documents

`adocs/specs.md` gains a `position input` row stating the accepted-and-cleared
contract and the two classes. `MANUAL.md` documents it under `position`, in the
terms a GUI author needs: which fields survive and that nothing is reported
back over UCI. `DEV_MANUAL.md` checked -- its one `load_FEN` mention is the
dedupe key and is unaffected. `README.md` checked, owner-written, no change
needed.
