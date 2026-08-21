id:         S117
goal:       the middlegame and endgame halves of every evaluation term travel in one integer instead of two
accepts:    identical scores from `bench_eval`'s checksum over its ten positions and identical node counts and best moves from tools/search_bench.py against the preceding commit -- **behaviour-neutral, so no SPRT is owed** (INV-6, DEC-083); the nps change measured by interleaved runs with the spread recorded and converted at the published rate; the truncation behaviour is unchanged or the change is stated and test_eval_model's tolerance is re-pinned rather than relaxed; the packing survives a negative endgame half, which is the classic sign-extension bug, and a test covers it
touches:    src/evaluation.cpp, src/evaluation.hpp, src/eval_tables.hpp, tools/eval_model.hpp, tests/
excludes:   any change to a weight or a term
decisions:  DEC-083
closes:
blocks:
paused_by:
done:

## Why this is on the plan at all

Reported at **+25.41 Elo** in one engine -- the largest single evaluation-speed
number in the surveyed record. Chesso carries `psqt_mg` and `psqt_eg` as
separate accumulators and every term as a separate mg/eg pair, so the whole
evaluation does twice the adds it needs to.

It is behaviour-neutral by construction and therefore costs no verdict, which
makes it one of the cheapest items here. The risk is entirely in the packing
arithmetic: the endgame half must survive being negative, and the single
tapering division at the end has to produce the same truncation the two
divisions produce today or `test_eval_model`'s pinned tolerance moves.
Interacts with S055, which removes a division from the same expression -- do
S055 first or fold it in, and say which.

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
mg as unsigned) -- extract first, then operate. The taper is therefore one
extraction at the end: unpack the accumulated pair once, blend, divide once --
exactly the post-S055 shape. The earliest description CPW's Tapered_Eval page
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

**Shape for chesso.** Every term is a separate mg/eg pair today.
Storage: `psqt_mg[6][64]` / `psqt_eg[6][64]` (src/eval_tables.hpp:55, :118 --
3072 B that become 1536), `passed_pawn_mg/eg[6]` (src/evaluation.cpp:80-81),
`pawn_structure_mg/eg[3]` (:103-104), `piece_placement_mg/eg[4]` (:199-200),
`tempo_mg/eg` (:582-583), `mobility_mg/eg[4]` (:690-691),
`king_safety_mg/eg[9]` (:733-736). **The INV-4 accumulators carry the pair
too**: `board_t` holds `int32_t psqt_mg; int32_t psqt_eg`
(src/data_structures.hpp:304-307, board_t 216 B), updated as two adds per
branch in `eval_add_piece`/`eval_remove_piece`/`eval_refresh`
(src/eval_tables.hpp:189-240), reached from make via
src/bitboard.cpp:687/:704/:727-728 and directly on the unmake paths at
:937-938, :953-954, :967, :975-976, :982 -- packing halves the adds at the
hottest sites in the engine. The debug INV-4 assert compares both fields
(src/bitboard.cpp:596-604); INV-2's memcmp checks (tests/test_search.cpp:1877,
tests/test_engine.cpp:51) survive any layout byte-identically. Accumulation in
evaluation.cpp: `mg_sum/eg_sum` in evaluate_pawns (:423-424, fed at :435-436,
:446-447, :476-477, :489-490), the pawn pair into evaluate_cheap (:631-633),
the four stage-two sums (:864-867, fed at :901-902, :910-911, :919-922,
:941-942). Extraction sites (the tapers): :640-643 stage one, :668-670 tempo
(exact while both weights are 0), :951-954 stage two -- one site after S055
merges it. The collect path (S055's finding): :956-958 hands tapered mobility
and safety back through evaluate_expensive_terms (:1005-1022) to
tools/eval_spread.cpp:174, and tests/test_evaluation.cpp:466-498 REQUIREs
`clamp(mobility + safety) == evaluate() - evaluate_cheap()` **exactly**
(:481-484). Overflow headroom at the shipped weights: worst legal |mg| lane is
about 9 queens x 793 + minors/rooks/king + enemy-king 179 + the pawn-term pair
(8 passers x 59 plus structure) ~ **9.4 k**; |eg| ~ **4.7 k**; even the crude
bound, 32 units x the largest table entry 841, is 26912 < 32767. Headroom
~3.4x on mg. The phase multiply happens after extraction (9.4 k x 24 fits int32
as today). Stage-two lanes stay under ~2 k.

**Implementation sketch.** (1) `score_t` = int32_t plus constexpr
`make_score(mg, eg)` / `mg_value(s)` / `eg_value(s)` (shift eg in unsigned to
dodge UB; the +0x8000 form above), with unit tests: all four sign quadrants --
a negative *low* half is the borrow case the correction exists for, a negative
*high* half is the int16-cast case, and the accepts' "negative endgame half"
is covered whichever half is low -- round-trip at the +/-32767 extremes,
additivity and signed-scalar multiplication on random in-range lanes, and a
round-trip over every shipped table entry. (2) Pack the board accumulators:
one `int32_t psqt`; eval_add/remove_piece do one add; eval_refresh and the
INV-4 assert compare the packed field. (3) Tables: **keep the tuner-emitted
plain `*_mg/*_eg` int arrays as the source of truth** and build packed arrays
from them with a consteval loop -- tuner and model interfaces untouched.
(4) Mechanical site conversion, one function per commit, suite green each
time: evaluate_pawns' pair, then stage two's two packed sums (collect mode
tapers `mob_p` alone and hands safety back as the S055 residue, so the
:481-484 REQUIRE still holds exactly). (5) One `taper(score_t, phase)` helper
at the extraction sites. **Bit-identical to the pair version by
construction**: lane adds, subtracts and scalar multiplies are exact, the
extraction is exact in range, and the taper divides the same dividend by the
same divisor with the same one truncation -- identical scores, identical tree,
so DEC-083's timing lane applies and no SPRT is owed. What would break
identity: a lane overflow (headroom above), or dividing/comparing/clamping a
packed value -- and the audit says no such site exists (the only division is
the taper after extraction, the only clamp is post-taper at :984).

**Constants and seeds.** None -- 16 and 0x8000 are structure, not weights;
nothing ships unfitted, DEC-084 not engaged. The one choice is the encoding
orientation (which half high): follow the published form, eg high, and record
the choice in the step stamp.

**Pitfalls.**
- **The sign-extension bug class**: extracting mg as `s & 0xFFFF` without the
  int16 cast, or eg as `s >> 16` without the +0x8000, is off by one whenever
  the low half is negative -- routine here (knight mg -1, queen eg -6, rook
  tables largely negative). The quadrant unit tests plus bench_eval's checksum
  (tests/bench_eval.cpp:197-215) against the preceding commit catch it.
- **Overflow when terms accumulate**: fine today by the arithmetic above, but
  S121-S126/S133 refit and add terms -- guard with a consteval scan asserting
  every packed table's lanes and their crude 32x bound fit int16, so a future
  fit that breaks headroom fails to compile rather than wraps.
- **Debug printability / the collect path**: eval_spread and the tests read
  *tapered ints* through the existing accessors, so packing is invisible to
  them if extraction stays at those interfaces; the S055 REQUIRE (term pair
  sums to the merged total, test_evaluation.cpp:483-486) is the regression to
  keep green, never weaken.
- **The tuner's view is unchanged -- verified**: tools/tuner.cpp emits plain
  `const int *_mg/*_eg` arrays (write_tables, :735-811) and
  tools/eval_model.hpp:1045-1087 seeds from the extern arrays. Keep those
  authoritative and derive the packed tables constexpr; pack the *stored*
  arrays instead and both tools change interface for nothing.
- `board_t`'s field order is deliberate (data_structures.hpp:284-288, the
  cache-line note); dropping 4 bytes shifts the scalars -- harmless, but
  update that comment rather than leaving it stale. A 2x32-in-64 lane variant
  would remove all headroom worry at twice the table bytes; the 16-bit form is
  the published one and the arithmetic above clears it.

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
  answering this file's open question: S055's merged taper is the rounding
  change and an SPRT; S117 then packs the merged form bit-identically.
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
- https://github.com/jhonnold/berserk/commit/bcb7d9b8b116810f43e696474b86eda6e280b686
  -- the +25.41 record, bounds, game count, and the "squashing ~10 commits"
  caveat, verbatim in the commit message.
- https://github.com/lynx-chess/Lynx/pull/697 -- packing-only patch, +9.0 +/-
  5.4 over 9234 games; avoided division on packed values.
- https://github.com/Ciekce/Stormphrax/commit/1105a813d61505dbb7ccc1496682fabe93586c27
  -- "packed tapered scores trick", bench-only record.
- https://www.chessprogramming.org/Tapered_Eval -- points at the 2012 CCC
  thread "two values in one integer"; the packing itself is not on CPW's Score
  page (checked).
- https://www.talkchess.com/forum3/viewtopic.php?t=61850 -- the unsigned
  +0x8000 extraction discussed with the C++ implementation-definedness
  reasons.
- https://github.com/lynx-chess/Lynx/releases -- packed evaluation named in
  v1.5.0/v1.7.0 release notes (the PR trail followed from there).
