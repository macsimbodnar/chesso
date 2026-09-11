id:         S208
goal:       the load boundary refuses a piece count the move buffer cannot hold and a pawn on a back rank, so `position fen` can no longer crash the shipping binary or index the passed-pawn table out of bounds
accepts:    `load_FEN()` in `src/bitboard.cpp` refuses a placement with more than 16 pieces of one colour and a placement with a pawn of either colour on rank 1 or rank 8, each with its own reason, and `position fen` reports the refusal on the UCI channel in the S176 shape (`info string refused [position fen] <fen>, <reason>`) with the board, history and moves left as they were; the F09 FEN (`QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1`) is observed to abort the Release binary with `stack smashing detected` **before** the fix and to be refused after; the two F10 FENs (`P7/8/8/8/8/8/8/K6k w - - 0 1`, `K6k/8/8/8/8/8/8/p7 b - - 0 1`) are observed to print `index 6 out of bounds` under `build-sanitize` before and to be refused after; red-first cases join `tests/test_audit_fen_semantics.cpp`, S161's file, one per class, plus the boundary cases that must still load (16 pieces a side, a pawn on rank 2 and rank 7); the bound behind `MAX_MOVES` is stated at its definition in `src/data_structures.hpp` as what it is -- 218 the proved legal maximum, 270 the buffer, 224 the largest count the audit's maximiser found under the 16-a-side rule, a search result and not a proof -- and `adocs/specs.md`'s position-input row names the third class beside S161's two; INV-6 discharged on identical `tools/search_bench.py` counts and best moves and an identical `bench` signature, because no legal position changes; `MANUAL.md` documents the two new refusals before `test_uci_surface` is refreshed (SURFACE)
touches:    src/bitboard.cpp, src/data_structures.hpp, tests/test_audit_fen_semantics.cpp, MANUAL.md, adocs/specs.md, tests/test_uci_surface.cpp (golden)
excludes:   a run-time bound inside `generate_moves()`'s hot loop, which would tax every node to guard an input the boundary can refuse for free; requiring exactly one king a side, because `EMPTY_POS` and the two "survivable" cases in `tests/test_search.cpp` are deliberately kingless or king-capturable debug inputs; the side not to move being in check, still unchecked as S161 left it; clamping the passed-pawn bucket inside `evaluate_pawns`, which would hide an invalid board instead of refusing it
decisions:  DEC-170
closes:     2026-09-10_adversarial-F09, 2026-09-10_adversarial-F10
blocks:
paused_by:
author:     Claude Opus 5, coordinator, 2026-09-11
done:       2026-09-11. **`position fen` can no longer abort the shipping binary or index the passed-pawn table off its end, and closing it cost one bench position its illegality.** `load_FEN()` refuses a placement with more than 16 pieces of one colour and a placement with a pawn on rank 1 or rank 8, each naming its own reason through a new defaulted `std::string* reason` out-parameter, and `src/chesso.cpp` `set_position()` prints it in S176's shape -- `info string refused [position fen] <fen>, <reason>` -- on top of the whole-game save/restore S176 already had, so the board, the history and every move applied since the last `position` are left exactly as they were. **Both reds observed on `23f926d` before a line was written.** F09: `printf 'position fen QQQQQQQQ/k6Q/Q6Q/Q6Q/Q6Q/Q6Q/Q6Q/KQQQQQQQ w - - 0 1\ngo depth 4\nquit\n' | ./build/src/chesso` -> **`*** stack smashing detected ***: terminated`, exit 134, core dumped**, in the Release binary that ships and that every SPRT measures; the placement loaded cleanly first, the `fen` command echoing it back verbatim. F10: the two FENs under `build-sanitize` -> **`src/evaluation.cpp:516:36: runtime error: index 6 out of bounds for type 'int [6]'`** for White and the same at **`:529:36`** for Black. After the fix, all three are refused and the sanitizer build reports **0** of either class. **Four red-first ctest cases** in `tests/test_audit_fen_semantics.cpp`, S161's file because the finding is the class S161 left open in its own words, and **all four observed red** by disabling the two refusals in the main tree: 16 passed, 4 failed. Two of them pin the reason string's content rather than just the refusal -- "27 white" for F09's placement, "17 white" and "16 black" for the boundary case -- so a refusal that fires for the wrong reason is not green. **Four control cases that must stay green and do**: sixteen a side (the start position, with the count asserted at 16 and 16 so the case cannot pass on a smaller board), pawns on ranks 2 and 7 (one square inside the refused rank on each side, where an off-by-one in the square comparison would show), the kingless and king-capturable debug positions, and a syntax failure leaving `reason` untouched -- which is what lets `set_position()` keep saying "does not load" for everything else without a second code path. **The discovery, and it is a decision: DEC-177.** The accepts asked for the 16-a-side refusal **and** an identical `bench` signature, and those two contradict each other. `KILLER_POS` is one of the eight bench positions and carries **17 white pieces**, nine pawns; `src/chesso.cpp`'s own provenance comment said it was "knowingly illegal by FIDE piece count" and was kept because "**the engine loads it**" -- which this step makes false. Measured rather than reasoned: with the refusal in and the constant untouched, `set_position` refuses it, leaves the previous board, and `bench` searches a duplicate for a signature of **30746008**. There is no resolution in which the constant stays unloadable -- `position killer`, the `test` command's table and two position lists in `tests/` all reach it. So the bound stays at 16 a side and **`KILLER_POS` becomes legal by deleting the pawn on h3**: checked with python-chess 1.11.2 and not argued, `Status.VALID`, 16 white and 14 black, the **twelve promotions** and the **f5e6** en-passant capture both intact, legal moves 42 -> **48** because removing that pawn opens lines rather than closing them. Every property the constant was kept for survives; the one that justified an exception is gone, and the eight-FEN set is now legal throughout, which is strictly better than the note explaining why it was not. **So this step carries a `Bench:` line and not "No functional change": 26491479 -> 30046849.** INV-6's clause is read the way DEC-177 states: **`tools/search_bench.py` at depth 9 is node-identical -- `121530 / 801481 / 72924`, `c3d5` / `e2a6` / `d7c8q`** -- and its three positions do not include `KILLER_POS`, so node-identity there still says what INV-6 wants it to say about every legal position. No legal position's tree changed. **Two re-derivations the change forced**, both from the engine rather than from a guess: `src/chesso.cpp`'s bench provenance table reads **48 moves and "valid yes"** for `KILLER_POS` where it read 42 and "NO", and its `test` expected-bestmove row reads **`ponder c5d4`** where it read `ponder d8h4` -- the bestmove itself, `g7h8q`, did not move. **`MAX_MOVES` now says what defends it** (`src/data_structures.hpp`): **218** the published maximum over legal positions, **224** the largest count the audit's maximiser found under the 16-a-side rule and named as a search result and not a proof, **270** the buffer, 46 of headroom -- and that the load boundary is what makes the measurement apply, with a note to re-run the maximiser if the generator changes. DEC-177 records that the honest rule is a bound on the **move count** rather than the piece count, and why that form was not taken here. **Beyond the accepts**, noted rather than hidden: `src/chesso.cpp` is not in `touches:` and was edited three times -- the `reason` plumbing, the provenance table and the `test` row -- all of them consequences of the two changes the accepts does name. `tests/test_uci_surface.cpp` needed **no** refresh: no option, default or command moved, and the refusals are `info string` lines rather than surface rows (SURFACE checked, not assumed; the golden test passes untouched). Gate: `ctest -L fast` **33/33 in `build` and 33/33 in `build-tune`**, format clean under `CLANG_FORMAT_MAJOR=22` (DEC-146). The Debug self-play clause does not bind -- the generator, `make_move` and `unmake_move` are untouched (DEC-141). No run, no SPRT: the change is unreachable from legal play by construction, so there is nothing for a match to measure. Docs: `MANUAL.md` gains the two refusal shapes, the reason each exists and an explicit statement that legality at large is still unchecked; `adocs/specs.md`'s position-input row names the two classes with both reproductions and both oracles, and its `search_bench` baseline paragraph carries the new bench figure beside S207's; `DEV_MANUAL.md` carries the signature at S208. `README.md` untouched, human-owned. Closes `2026-09-10_adversarial-F09` and `2026-09-10_adversarial-F10`.

## Why this exists

Two high findings of `2026-09-10_adversarial`, both reachable from `position
fen` in the binary that ships and is measured, both guarded today by an
`assert` that `NDEBUG` compiles out of both gated builds.

**F09.** `MAX_MOVES` is 270, true of legal positions (the known maximum is
218), and every caller declares the buffer on the stack; `src/search.cpp`
`negamax_at` appends `generate_captures` and `generate_quiets` into the same
array. A placement `load_FEN()` accepts with 26 queens of one colour -- the F09 FEN --
generates 277 moves by the audit's maximiser, and the Release binary aborts with `*** stack smashing detected ***`,
exit 134. A near-miss corrupts the adjacent `scores[]` and `quiets_tried[]`
instead of aborting. Constrained to 16 pieces a side the same maximiser's best
was 224, which is why the 16-a-side rule is the fix. The S161 sanitiser repairs castling rights and en-passant
squares and says in its own comment that piece counts are "deliberately not
checked". They are a third class that corrupts.

**F10.** `evaluate_pawns` in `src/evaluation.cpp` computes the passed-pawn
bucket as `6 - (rank)` for white and `rank - 1` for black and reads
`passed_pawn_mg[bucket]`; the comment beside it says what keeps it in range is
that a pawn cannot stand on rank 1 or 8, and the Debug build asserts it. A pawn
on either rank arrives through `position fen`, the sanitizer build prints
`index 6 out of bounds for type 'int [6]'` at both colours' sites, the Release
binary answers `info score cp 418` and a move, and the collecting instantiation
writes one `int` past the caller's `int[2][6]`. A 20000-FEN fuzz under
ASan+UBSan found exactly this one report site.

Neither is reachable from legal play, no GUI sends either, and the audit rates
them high because they are crashes and out-of-bounds writes on the product
surface -- and because `position fen` is also how corpora,
`tools/pgn_to_positions`, `datagen` and any fuzzer reach the engine.

## Shape

The load boundary is the contract every downstream consumer assumes (S161's
words), so the refusal lands there and nowhere else. Counting is a loop over
`squares[]` after the placement is parsed, before `compute_full_hash()`,
beside S161's two clearings. Refuse rather than repair: a piece count cannot
be repaired without inventing a position, and a back-rank pawn has no legal
home.

Why 16 and not a pawn cap or a per-type cap: 16 a side is the rule of the game
and the bound the audit's maximiser was run under, and its result, 224, leaves
46 of headroom under 270. State that as the reasoning and the residual risk at
`MAX_MOVES`, so the next reader knows 270 is defended by a measurement and can
re-run it if the generator changes.

## Tests

`tests/test_audit_fen_semantics.cpp` is S161's file and the right home: one
case per class, each observed red on the unfixed tree -- the F09 case red by
the abort under a Release build cannot be a ctest red, so it is observed by
hand and recorded in the stamp, and the ctest case asserts the refusal --
plus the boundary loads (16 a side, pawns on ranks 2 and 7, the perft suite's
FENs all still loading). `tools/gate_extra.sh` stage 4 is the observation
instrument for F10's red and its green.

## Cost

Agent work, an hour or two; no run. Node-identical, no `Bench:` change, no
SPRT. The Debug self-play clause does not bind -- the generator is untouched.
