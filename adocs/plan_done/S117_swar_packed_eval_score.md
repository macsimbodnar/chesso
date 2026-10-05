id:         S117
goal:       the middlegame and endgame halves of every evaluation term travel in one integer instead of two
accepts:    identical scores from `bench_eval`'s checksum over its ten positions and identical node counts and best moves from tools/search_bench.py against the preceding commit -- **that identity is the gate, not a claim, and it is the only thing that buys the no-SPRT lane**: INV-6 grants it to a change "claimed behaviour-neutral" only once the identical node counts and identical best moves are in hand, so the numbers are recorded before the step completes (INV-6, DEC-083); the nps change measured by interleaved runs with the spread recorded and converted at the published rate; **the truncation behaviour is unchanged** -- S055's single stage-two taper read H0 and was reverted, so stage one divides once and stage two still divides twice, mobility and king safety each tapered on its own, and the packing reproduces every one of those truncations exactly, so a moved bound is a bug in the packing to be fixed, never a re-pinned `test_eval_model` tolerance, and a truncation change retained deliberately alters play and owes an SPRT verdict under INV-6 like any other change that does; the packing survives a negative endgame half, which is the classic sign-extension bug, and a test covers it
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, src/data_structures.hpp, src/bitboard.cpp, tools/eval_model.hpp, tools/tuner.cpp, tests/ (tests/test_evaluation.cpp, tests/test_invariants.cpp), DEV_MANUAL.md, adocs/data/S192_anchors.py, .tuning/apply_fit.py, .tuning/verify_fit.py, .tuning/diff_fit.py
excludes:   any change to a weight or a term
decisions:  DEC-083
closes:
blocks:
paused_by:
author:     coordinator (Claude Opus 5.5); implementer: one Opus subagent
done:       2026-10-05 -- BEHAVIOUR-NEUTRAL, INV-6 discharged against e525bc0: bench 4081329 and bench 12 1860699 with every info line and bestmove identical; search_bench 32932 / 70095 / 25178 at depth 9 and 67792 / 280873 / 137893 at depth 12, c3d5 / e2a6 / d7c8q and c3d5 / d5e6 / d7c8q, identical; bench_eval checksum -27425414745967 identical. No SPRT owed (DEC-083). Both stage-two truncations kept (S055 H0). Encoding: eg in the high 16 bits, mg low, +0x8000 extraction. Hooks: 0 out-of-line calls (inl.sh, calls.sh). Timing +3.47 % (CI +3.28 .. +3.66, 24/24 pairs; A/A +0.04 %, CI -0.17 .. +0.24), about +5.0 Elo at DEC-083's 1.43/% (a conversion); kept. Six packed-score tests, red first under both sign-extension mutants. Suites 41/41 in build and build-tune, format clean (DEC-146 override). Second tier (DEC-141): Debug self-play 8 games at 4+0.04, 0 Assertion, 0 disconnect; tools/gate_extra.sh 5 stages green in 1026 s (.tuning/gate_extra_2026-10-05_S117.log). Deviation: the evaluation.cpp weight definitions became constexpr and tools/tuner.cpp emits them so. Fast-check fixes (FIX-FIRST): (1) the four weight parsers read either keyword: adocs/data/S192_anchors.py, .tuning/apply_fit.py, .tuning/verify_fit.py and .tuning/diff_fit.py. apply_fit keeps the source's keyword and refuses loudly what it cannot place. On a scratch copy with a header the rebuilt tuner emitted, 827 of 827 parameters were placed, checked by verify_fit; (2) S192_anchors reproduces 10 of 10; (3) stage_one_bound now counts what load_FEN admits: 31232 <= 32767; (4) the weight test now reads the packed copies the evaluation uses, which are now extern. evaluation.cpp.o's instructions are unchanged (objdump). The test fails first under a swapped pack_scores, 42 assertions; (5) mutation_check validate: 173 of 173, none re-anchored; (6) after the fixes, INV-6 was re-run, with bench, bench 12, search_bench 9 and 12 and the bench_eval checksum all identical. Suites 41/41 in both builds, format clean. Implemented from the step's description and the cited publications. Timings in adocs/data/S117_timing.txt.

## Why this is on the plan at all

Reported at **+25.41 Elo** in one engine -- the largest single evaluation-speed
number in the surveyed record. Chesso carries `psqt_mg` and `psqt_eg` as
separate accumulators and every term as a separate mg/eg pair, so the whole
evaluation does twice the adds it needs to.

It is behaviour-neutral by construction and therefore costs no verdict -- once
the neutrality is demonstrated, which is what the `accepts:` above turned into
a gate. The risk is entirely in the packing arithmetic: the endgame half must
survive being negative, and the tapering division has to produce the same
truncation it produces without packing.

**There is no "or the tolerance moves" branch, and S139 removed the one the
`accepts:` used to offer.** INV-6 (`adocs/specs.md` "A change is retained only
against a measurement") has two lanes and no third: a change *proven* neutral
by identical node counts and identical best moves skips the match, and a change
that alters play "is retained only with an SPRT verdict against a named
commit". A moved truncation is a moved score, a moved tree and therefore the
second lane -- so it cannot be admitted on a re-pinned tolerance and no match.
The Measurement section below says the same thing from the other end: "if node
counts differ, the change has a bug -- stop and fix, never fall back to an
SPRT."

The open question this file used to carry -- fold S055 in or not -- is answered
in Interactions below and in S055's own Technical details: **S055 lands first,
not folded.** S055 then read H0 and was reverted (the section after the
References), so the form this step packs is the unmerged one -- one division in
stage one, two in stage two -- and reproducing each of those truncations
exactly is what the packing arithmetic is required to do.

## Technical details (SOTA research, 2026-08-19)

**State of the art.** The published form (the Computer Chess Wiki's "Packed
Evaluation" page, minuskelvin.net) packs both halves into one 32-bit integer:
`p = eg * 2^16 + mg` -- **eg in the high 16 bits, mg in the low 16**, lanes
each ranged [-2^15, 2^15). Addition and subtraction of packed values add the
lanes (the identity `(e1*2^16+m1)+(e2*2^16+m2) = (e1+e2)*2^16+(m1+m2)` holds
mod 2^32 while lanes stay in range), and multiplication by an integer scalar
-- negative included -- multiplies both lanes, so `difference * weight` and
`mover * count * weight` survive packing. Extraction: mg is the low half cast
to int16 (the cast does the sign); eg is `int16((p + 0x8000) >> 16)` -- a
negative mg borrows 1 from the high lane, and adding 2^15 before the shift
makes the floor of the mg term zero for every in-range mg: the published
sign-carry correction (talkchess t=61850 walks through it, with the
implementation-definedness reasons for doing it in unsigned arithmetic).
**What does not work: division** (the remainder bleeds between lanes unless
both divide exactly -- Lynx's PR states they avoided division on packed
values), **comparison, min/max/clamp, abs** (int32 ordering sorts by eg, then
mg as unsigned) -- extract first, then operate. The taper is therefore an
extraction at the end of each tapered sum: unpack the pair, blend, divide --
one extraction per division the unpacked code makes, which after S055's revert
is one in stage one and two in stage two. The earliest description CPW's Tapered_Eval page
points at is the 2012 CCC thread "two values in one integer" (Pierre Bokma);
CPW's own Score page does not document the packing.

**The +25.41 is traced**: Berserk, commit `bcb7d9b8b116` (PR #65, 2021-04-25),
"Evaluation uses single score eval" -- `ELO 25.41 +- 11.28 (95%)`, SPRT
8.0+0.08s Threads=1 Hash=8MB, LLR 2.99 at bounds [-4.00, 1.00], 1808 games
(chess.honnold.me/test/304). Other traced records of the same change: Lynx PR
#697 "Use packed evaluation" (2024-03-19), +9.0 +/- 5.4 LOS 99.9 % over 9234
games; Stormphrax `1105a813` "packed tapered scores trick" (2023-06-04), bench
only, no Elo stated.

### Scope concern

The Berserk commit message itself says the patch **squashes ~10 commits
including a tuner rewrite** -- so +25.41 was not the packing alone, the run
stopped early against elo1=1 (point estimate inflated, the S089/S021 caveat),
and a retuned evaluation is not behaviour-neutral. The plan's "largest
evaluation-speed number surveyed" is an anchor on a bundled patch; it decides
what to try (DEC-019), Lynx's +9.0 on a packing-only patch is the better
prior, and this step's own claim stays the DEC-083 timing. Goal and accepts
unaffected.

**Shape for chesso.** Every term is a separate mg/eg pair today. Storage:
`psqt_mg[6][64]` / `psqt_eg[6][64]` (`src/eval_tables.hpp` `psqt_mg` and
`src/eval_tables.hpp` `psqt_eg` -- 3072 B that become 1536),
`passed_pawn_mg/eg[6]` (`src/evaluation.cpp` `passed_pawn_mg` and
`src/evaluation.cpp` `passed_pawn_eg`), `pawn_structure_mg/eg[3]`
(`src/evaluation.cpp` `pawn_structure_mg` and `src/evaluation.cpp`
`pawn_structure_eg`), `piece_placement_mg/eg[4]` (`src/evaluation.cpp`
`piece_placement_mg` and `src/evaluation.cpp` `piece_placement_eg`),
`tempo_mg/eg` (`src/evaluation.cpp` `tempo_mg` and `src/evaluation.cpp`
`tempo_eg`), `mobility_mg/eg[4]` (`src/evaluation.cpp` `mobility_mg` and
`src/evaluation.cpp` `mobility_eg`), `king_safety_mg/eg[9]`
(`src/evaluation.cpp` `king_safety_mg` and `src/evaluation.cpp`
`king_safety_eg`). **The INV-4 accumulators carry the pair too**: `board_t`
holds `int32_t psqt_mg; int32_t psqt_eg` (`src/data_structures.hpp` `board_t`,
board_t 216 B), updated as two adds per branch in
`eval_add_piece`/`eval_remove_piece`/`eval_refresh` (`src/eval_tables.hpp`
`eval_add_piece` to `src/eval_tables.hpp` `eval_refresh`), reached from make
via `src/bitboard.cpp` `add_piece`, `src/bitboard.cpp` `remove_piece` and
`src/bitboard.cpp` `move_piece`, and directly on the five unmake paths in
`src/bitboard.cpp` `unmake_move_impl` -- packing
halves the adds at the hottest sites in the engine. The debug INV-4 assert
compares both fields (`src/bitboard.cpp` `eval_accumulators_match`); INV-2's
memcmp checks
(`tests/test_search.cpp` "a null move undoes itself exactly",
`tests/test_engine.cpp` "engine: zobrist and unmake") survive any layout
byte-identically. Accumulation in evaluation.cpp: `mg_sum/eg_sum` in
`src/evaluation.cpp` `evaluate_pawns`, declared once and fed from its four
term blocks; the pawn pair into `src/evaluation.cpp` `evaluate_cheap`; the four
stage-two sums in `src/evaluation.cpp` `evaluate_mobility_and_king_safety`,
declared once and fed from its four term blocks. Extraction sites (the tapers):
stage one and tempo in `src/evaluation.cpp` `evaluate_cheap` (the tempo taper
exact while both weights are 0), stage two in `src/evaluation.cpp`
`evaluate_mobility_and_king_safety` -- two tapers, mobility and king safety,
because S055's merge read H0 and was reverted. The collect path (S055's
finding): `src/evaluation.cpp` `evaluate_mobility_and_king_safety` hands
tapered mobility
and safety back through
evaluate_expensive_terms (`src/evaluation.cpp` `evaluate_expensive_terms`) to
`tools/eval_spread.cpp` `main`, and `tests/test_evaluation.cpp` "evaluation:
score" REQUIREs
`clamp(mobility + safety) == evaluate() - evaluate_cheap()` **exactly**
(`tests/test_evaluation.cpp` "the unclamped terms are the engine's own").
Overflow headroom at the shipped weights:
worst legal |mg| lane is about 9 queens x 793 + minors/rooks/king + enemy-king
179 + the pawn-term pair (8 passers x 59 plus structure) ~ **9.4 k**; |eg| ~
**4.7 k**; even the crude bound, 32 units x the largest table entry 841, is
26912 < 32767. Headroom ~3.4x on mg. The phase multiply happens after
extraction (9.4 k x 24 fits int32 as today). Stage-two lanes stay under ~2 k.

**Implementation sketch.** (1) `score_t` = int32_t plus constexpr
`make_score(mg, eg)` / `mg_value(s)` / `eg_value(s)` (shift eg in unsigned to
dodge UB; the +0x8000 form above), with unit tests: all four sign quadrants --
a negative *low* half is the borrow case the correction exists for, a negative
*high* half is the int16-cast case, and the accepts' "negative endgame half" is
covered whichever half is low -- round-trip at the +/-32767 extremes,
additivity and signed-scalar multiplication on random in-range lanes, and a
round-trip over every shipped table entry. (2) Pack the board accumulators: one
`int32_t psqt`; eval_add/remove_piece do one add; eval_refresh and the INV-4
assert compare the packed field. (3) Tables: **keep the tuner-emitted plain
`*_mg/*_eg` int arrays as the source of truth** and build packed arrays from
them with a consteval loop -- tuner and model interfaces untouched. (4)
Mechanical site conversion, one function per commit, suite green each time:
evaluate_pawns' pair, then stage two's two packed sums, each tapered on its
own as the unpacked code does, so the collect path hands back the same two
truncated ints and the `tests/test_evaluation.cpp` "the unclamped terms are
the engine's own" REQUIRE still holds exactly. (5) One `taper(score_t, phase)` helper at the extraction
sites. **Bit-identical to the pair version by construction**: lane adds,
subtracts and scalar multiplies are exact, the extraction is exact in range,
and each taper divides the same dividend by the same divisor with the same
truncation -- identical scores, identical tree, so DEC-083's timing lane
applies and no SPRT is owed. What would break identity: a lane overflow
(headroom above), or dividing/comparing/clamping a packed value -- and the
audit says no such site exists (the only division is the taper after
extraction, the only clamp is post-taper at `src/evaluation.cpp`
`evaluate_expensive`).

**Constants and seeds.** None -- 16 and 0x8000 are structure, not weights;
nothing ships unfitted, DEC-084 not engaged. The one choice is the encoding
orientation (which half high): follow the published form, eg high, and record
the choice in the step stamp.

**Pitfalls.**
- **The sign-extension bug class**: extracting mg as `s & 0xFFFF` without the
  int16 cast, or eg as `s >> 16` without the +0x8000, is off by one whenever
  the low half is negative -- routine here (knight mg -1, queen eg -6, rook
  tables largely negative). The quadrant unit tests plus bench_eval's checksum
  (`tests/bench_eval.cpp` `main`) against the preceding commit catch it.
- **Overflow when terms accumulate**: fine today by the arithmetic above, but
  S121-S126/S133 refit and add terms -- guard with a consteval scan asserting
  every packed table's lanes and their crude 32x bound fit int16, so a future
  fit that breaks headroom fails to compile rather than wraps.
- **Debug printability / the collect path**: eval_spread and the tests read
  *tapered ints* through the existing accessors, so packing is invisible to
  them if extraction stays at those interfaces; the REQUIRE that the two
  tapered terms sum to the stage-two total (S055's finding,
  `test_evaluation.cpp` "the unclamped terms are the engine's own") is the
  regression to keep green, never weaken.
- **The tuner's view is unchanged -- verified**: tools/tuner.cpp emits plain
  `*_mg/*_eg` int arrays, `constexpr int` since S117 (As built deviation 2)
  (write_tables, `tools/tuner.cpp` `write_tables`) and
  `tools/eval_model.hpp` `starting_params` seeds from the extern arrays.
  Keep those authoritative and derive the packed tables constexpr; pack the
  *stored* arrays instead and both tools change interface for nothing.
- `board_t`'s field order is deliberate (`data_structures.hpp` "single move, so
  they are packed together and follow", the cache-line note); dropping 4 bytes
  shifts the scalars -- harmless, but update that comment rather than leaving
  it stale. A 2x32-in-64 lane variant would remove all headroom worry at twice
  the table bytes; the 16-bit form is the published one and the arithmetic
  above clears it.

**Measurement.** DEC-083's timing lane, exactly as the accepts states:
identical `bench_eval` checksum, identical node counts and best moves from
`tools/search_bench.py` against the preceding commit discharge INV-6 -- **no
SPRT**. Then interleaved timing (hyperfine alternating pairs as S103/S104
did), spread recorded, `bench_movegen`'s own resolution read first, converted
at DEC-083's 1.43 Elo per percent (2.10 short-TC) and named a conversion.
Expect low single digits: one add instead of two in make/unmake's eval hook,
half the adds per evaluation loop, 1.5 KB less table footprint -- Berserk's
bundled +25.41 does not transfer (DEC-019). Nothing should break identity; if
node counts differ, the change has a bug -- stop and fix, never fall back to
an SPRT.

**Interactions.**
- **S055 lands first, not folded** -- decided in S055's Technical details and
  answering this file's open question: S055's merged taper was the rounding
  change and an SPRT, which read H0, and it was reverted; S117 packs the
  unmerged form, both stage-two divisions, bit-identically.
- **S121-S126** add terms in the packed type: the helpers must be pleasant --
  `make_score` constexpr, packed arrays derived from plain ones, `taper()` in
  one place -- so a new term is one packed array and one `+=`.
- **S126** refits through the untouched plain-array interface (see Pitfalls).
- **S120** caches the final tapered int by position key -- unaffected.
- **S118** would cache the pawn-term contribution per pawn structure: a packed
  score is the natural single-int cache payload; note it there.
- **S133** multiplies PSQT bytes by the king buckets; packed halves that and
  its fit re-runs the headroom scan.

**References.**
- https://minuskelvin.net/chesswiki/content/packed-eval.html -- the packing,
  extraction with +0x8000, legal operations, lane-range condition.
- -
  https://github.com/jhonnold/berserk/commit/bcb7d9b8b116810f43e696474b86eda6e280b686
  -- the +25.41 record, bounds, game count, and the "squashing ~10 commits"
  caveat, verbatim in the commit message.
- https://github.com/lynx-chess/Lynx/pull/697 -- packing-only patch, +9.0 +/-
  5.4 over 9234 games; avoided division on packed values.
- -
  https://github.com/Ciekce/Stormphrax/commit/1105a813d61505dbb7ccc1496682fabe93586c27
  -- "packed tapered scores trick", bench-only record.
- https://www.chessprogramming.org/Tapered_Eval -- points at the 2012 CCC
  thread "two values in one integer"; the packing itself is not on CPW's Score
  page (checked).
- https://www.talkchess.com/forum3/viewtopic.php?t=61850 -- the unsigned
  +0x8000 extraction discussed with the C++ implementation-definedness
  reasons.
- https://github.com/lynx-chess/Lynx/releases -- packed evaluation named in
  v1.5.0/v1.7.0 release notes (the PR trail followed from there).

## S055 read H0 and was reverted (2026-10-05, the coordinator)

The accepts' "after S055 there is one division in the taper" no longer
holds: S055's single taper measured nElo -8.78 +/- 6.39 at `{-5, 0}` and was
reverted (`adocs/plan_done/S055_taper_stage_two_once.md`). Stage two still
tapers mobility and king safety through **two** divisions, and the packing
must reproduce both truncations -- unpack the two terms and taper each -- to
stay node-identical. The accepts' rule is unchanged: a moved bound is a bug in
the packing, and merging the divisions is a play change that has already been
measured against. (The implementer corrected `accepts:` to say this the
same day; the quote above is the text as it stood.)

## As built (2026-10-05, the implementer)

### Deviations, first

1. **Two stage-two divisions kept, as the S055 section above requires.**
   Mobility and king safety are packed into two separate sums and each is
   tapered on its own, `taper(mobility_sum)` and `taper(safety_sum)`, so
   both truncations are the ones the parent made. Nothing was merged.
2. **The twelve plain weight definitions in `src/evaluation.cpp` are now
   `constexpr int`, not `const int`, and `tools/tuner.cpp` emits them that
   way** (six `fprintf` format strings, one keyword each). Not in `touches:`.
   Needed because the packed copies are derived at compile time and a
   `const` array's elements cannot be read in a constant expression (gcc
   13.3 and clang 22 both refuse; checked). The header's
   `extern const int` declarations are unchanged and the definitions keep
   external linkage through them, so `tools/eval_model.hpp` and the tuner
   read the same symbols. A fit file written before S117 fails to compile
   when pasted -- loudly, never silently -- and `DEV_MANUAL.md` says so
   under the tuner's paste table.
3. **`score_t` lives in `src/data_structures.hpp`**, beside `board_t`, which
   is the first thing that needs it; `taper()` in `src/evaluation.hpp`
   beside `GAME_PHASE_MAX`. `tests/test_invariants.cpp` changed with the
   field: its drift report reads the halves, and its planted drift plants
   one in each half (`make_score(1, 0)` and `make_score(0, 1)`), both still
   required to be seen.
4. **Tempo is packed too** (`tempo_score`), through the same `taper()`. At
   the shipped zero weights the compiler folds it as before.
5. **The weight parsers read either keyword** (fast check, FIX-FIRST).
   `adocs/data/S192_anchors.py`, `.tuning/apply_fit.py`,
   `.tuning/verify_fit.py` and `.tuning/diff_fit.py` matched `^const int`
   only. On the constexpr tree `S192_anchors.py` died with
   `KeyError: 'passed_pawn_mg'`, and `apply_fit.py` found 0 arrays and
   0 scalars, then exited 0 with a half-applied fit. All four now match
   `^(?:constexpr|const) int`. `apply_fit.py` also:
   - keeps the source's keyword when it replaces a value;
   - refuses an emitted definition it cannot parse, and an emitted header
     with nothing in it;
   - checks every emitted name against `src/evaluation.cpp` before writing
     either file.

   A tree-wide grep found no other parser of these declarations. The
   scripts' citations in `adocs/data/` and the plan-citation checks are
   untouched, and the `const int` lines in `adocs/data/2026-09-04_test_review/`
   are search-code anchors, not weights.
6. **The packed copies the evaluation reads are external.** They were file
   `static`; now `constexpr` definitions in `src/evaluation.cpp`, with
   `extern const` declarations in `src/evaluation.hpp`. This is what the
   strengthened test below needs. Instructions unchanged:
   - `objdump -d` of `evaluation.cpp.o` is the same before and after, with
     the same direct `R_X86_64_PC32` relocations, now naming the symbols
     instead of `.rodata` offsets;
   - `.rodata` gains the four copies the code never loads, so its layout
     shifts by 0x20;
   - `bitboard.cpp.o` is byte-identical.
7. **`stage_one_bound` counts what load_FEN admits.** Before, it counted 8
   passers and 8 pawns per structure feature: one side only, and only what
   a game reaches. The truthful bound uses load_FEN's limits:
   - at most 16 pieces a side with exactly one king, so 32 on the tables;
   - 15 passers a side, both sides pushing one half;
   - 15 per structure and placement feature, because those enter as White's
     count minus Black's, two counts of 0 to 15.

   At the shipped weights that is 32 x 841 + 30 x 96 + 3 x 15 x 32 +
   4 x 15 x 0 = **31232**, against a limit of 32767. Counting both sides for
   structure as well would give 32672, which also fits; it was not used,
   because the difference cannot exceed 15.

### What changed

- `score_t` = `int32_t`, endgame half high, middlegame half low (the
  published orientation, recorded here as the step asks). `make_score` builds
  in unsigned; `mg_value` is the low half through `int16_t`; `eg_value` is
  `(int32_t)((uint32_t)s + 0x8000) >> 16`, the borrow correction.
- `board_t`: `psqt_mg` + `psqt_eg` became one `score_t psqt` (board_t 4
  bytes smaller; the field comment updated). The hooks do one add per piece
  from `psqt_score[6][64]`, a `constexpr std::array` built from the plain
  `psqt_mg`/`psqt_eg`, which stay the source of truth.
- `evaluate_pawns` returns one packed sum; `evaluate_cheap` tapers
  `board->psqt + pawns` once, as before; stage two as in deviation 1.
- Headroom is a compile-time check, not a hope:
  - in `src/eval_tables.hpp`, a `static_assert` over the piece-square
    tables: 32 x the largest entry, 26912 today;
  - in `src/evaluation.cpp`, one over stage one's sum (tables plus pawn
    terms, 31232 today, deviation 7), one over mobility's sum and one over
    king safety's sum.

  Each is a count bound times the largest weight.

### INV-6, against e525bc0

`.tuning/coord/S117_files/inv6.sh` (S253's), streams with timing fields
stripped and diffed whole against a build of `e525bc0`: `chesso bench`
**4081329**, `bench 12` **1860699**, every info line and bestmove identical;
`search_bench.py` depth 9 **32932 / 70095 / 25178** `c3d5` / `e2a6` /
`d7c8q`, depth 12 **67792 / 280873 / 137893** `c3d5` / `d5e6` / `d7c8q`,
identical. `bench_eval`'s ten scores and its checksum
**-27425414745967**, identical. The timed binary is byte-identical (`cmp`)
to the final build. No SPRT owed (DEC-083).

### Inlining

`inl.sh` (S253's): **0** hook calls refused in `src/bitboard.cpp`, and
`calls.sh` finds **0** out-of-line `eval_add_piece` / `eval_remove_piece`
calls in `bitboard.cpp.o` and `evaluation.cpp.o`. The unit's
`inline-unit-growth` refusals read 47 against the parent's 46: cold
`std::string` helpers shuffled, one fewer generator `is_attacked` refusal;
no hook among them.

### Timing

S020/S253's shape and scripts (copied to `.tuning/coord/S117_files/`): 300
stratified positions at `go depth 11`, one hyperfine invocation per pair,
order alternating. Governor `powersave` (recorded, not set); load average
2.58 before, 1.46 after, on 12 threads. Readings in
`adocs/data/S117_timing.txt`.

| comparison | pairs | speed-up | 95 % CI | paired t |
|---|---|---|---|---|
| A/A, parent vs byte copy | 16 | +0.04 % | -0.17 .. +0.24 | -0.38 |
| **S117 vs parent** | 24 | **+3.47 %** | **+3.28 .. +3.66** | -38.82 |

24 of 24 pairs faster. Over CLAUDE.md's 3 % line with a CI that clears it
and an A/A floor of +/-0.2 %. Side readings, not deciders: `bench_eval`
`evaluate()` 49.1 .. 51.5 ns a call to 43.8 .. 45.6 (about -12 %), and
`bench_movegen` perft best 990 .. 1020 ms to 909 .. 933 (about -8 %), node
counts verified by the tool. **Converted at DEC-083's 1.43 Elo per percent,
about +5.0 Elo (+7.3 at the short-TC 2.10)** -- a conversion, not a
measurement. Lynx's packing-only +9.0 +/- 5.4 is the nearest published
record; Berserk's +25.41 was a bundled patch and is not compared.

**Keep**: behaviour-neutral by INV-6 and measured faster beyond the noise
floor.

### Tests

New suite `tests/test_evaluation.cpp` "evaluation: packed score", six
cases: all four sign quadrants; both halves at -32768 .. 32767 extremes
(49 pairs, including the corner that wraps); a negative endgame half reached
by accumulation (every pawn and king endgame entry subtracted) against the
plain sums; addition, subtraction and signed-scalar multiplication on 10000
random in-range pairs; "every packed weight the evaluation reads matches its
plain pair" (all 768 table entries of `psqt_score`, and the packed copies the
evaluation itself reads -- `mobility_score`, `passed_pawn_score`,
`pawn_structure_score`, `piece_placement_score`, `king_safety_score`,
`tempo_score` -- half by half against the plain arrays); `taper()` equals the
unpacked formula's truncation for 10000 random pairs at every phase 0 .. 24.

**Red first, observed**: `eg_value` mutated to the bare `score >> 16` fails
all 6 cases (27 assertions) and moves `bench_eval`'s checksum to
-27425415702162; `mg_value` mutated to `score & 0xFFFF` fails all 6 cases
(27 assertions). Restored, 6/6 green.

**The weight case, red first** (fast check: it packed the plain arrays
afresh, so it never saw the copies the evaluation reads):
- `pack_scores` mutated to `make_score(eg[i], mg[i])` fails it with 42
  assertions: king_safety 18, passed_pawn 12, mobility 6, pawn_structure 6.
- Placement and tempo pass under that mutant. Their weights are all zero, so
  no reading of values can see a swap there until a refit moves them.
- Restored: `evaluation.cpp` matches the kept copy (`cmp`), the rebuilt
  `evaluation.cpp.o` is byte-identical to the one before the mutant, and the
  suite is 6/6 green.

**Goldens and mutants after the fast check:**
- `adocs/data/S192_anchors.py` reproduces **10 of 10**, the DEC-142
  re-derivation of the S192 goldens.
- `mutation_check.validate` over `tools/mutants/` passes **173 of 173**.
- 16 of those mutants sit in files this step changed. For every pair, the
  150 characters on each side of the anchor match HEAD, so none needed
  re-anchoring and no `--only` kill run was owed.

### Gate and second tier

- `cmake --build build -j8 && ctest --test-dir build -L fast` **41/41**,
  `build-tune` **41/41**, `./clang-format.sh --check` clean under
  `CLANG_FORMAT_MAJOR=22`, one chain, exit 0.
- Debug self-play (DEC-141, DEV_MANUAL's command), 8 games at 4+0.04:
  **0 `Assertion`** in log and stdout, **0 `disconnect`**
  (`.tuning/coord/S117_files/debug_selfplay.*`).
- `tools/gate_extra.sh`: **5 stages green in 1026 s** (prose, citations, debug 396 s, sanitize 573 s, perft 56 s)
  (`.tuning/gate_extra_2026-10-05_S117.log`).
- No pruning, reduction or extension rule moved, so DEC-141's mutant clause
  does not apply.

### Documents

`DEV_MANUAL.md`: one paragraph under the tuner's paste table (deviation 2).
`accepts:` and Technical details corrected to the fact: S055 read H0 and was
reverted, so stage two keeps two divisions and the packing reproduces both.
`touches:` widened to every file the step changes.
`MANUAL.md` checked: no UCI surface moved, no change. `specs.md` INV-4 names
`psqt_mg`, `psqt_eg` as accumulators; the wording change is proposed to the
coordinator, not made here.
