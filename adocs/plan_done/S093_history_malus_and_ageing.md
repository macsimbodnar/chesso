id:         S093
goal:       history gets a malus for the moves that were tried and failed, a gravity update that ages it by construction, butterfly indexing, and survives across go within one game
accepts:    two SPRT verdicts: malus and gravity land together -- they are one published mechanism, `entry += bonus - entry * abs(bonus) / MAX`, and splitting them measures each against a table shape it will not ship with (DEC-087) -- and persistence across `go` lands second with its own verdict; the malus applies to the quiet moves searched before the cutoff move and not to the cutoff move itself, asserted by a unit test on the table rather than through a game; the gravity keeps every score inside ORDER_HISTORY_MAX so the move-ordering bands still clear each other by 100 points, with the band clearance asserted (CLAUDE.md hazard, S023, S061); history carried across `go` is cleared on `ucinewgame` and on a position that is not a descendant of the last one searched, with a test for both; the fast suite green
            (**The persistence clause -- "history carried across `go` is
            cleared on `ucinewgame` and on a position that is not a descendant
            of the last one searched, with a test for both" -- is discharged,
            2026-08-22, not produced.** It was produced while verdict 2 was in
            the tree, both clauses observed red first and driven through the UCI
            layer. Verdict 2 then measured `Elo -1.65 +/- 4.22` over 15398 games
            and was reverted in full, DEC-101, so there is no carried history
            for either rule to clear and neither test has a subject. See "The
            accepts clause that cannot be satisfied, discharged" below.)
touches:    src/search.cpp history update, src/search.hpp, src/evaluation.cpp score_move, src/data_structures.hpp search_state_t, src/search_params.hpp, src/chesso.cpp (verdict 2 only), tests/test_search.cpp, tests/test_evaluation.cpp, tests/test_search_params.cpp, MANUAL.md, adocs/specs.md,
            adocs/decisions.md, adocs/testing.md; and, for verdict 2 before it
            was reverted, src/uci.hpp, tests/test_engine.cpp and
            tools/datagen.cpp
excludes:   capture history, which is S023; continuation history, which is S024; correction history, which is S099
decisions:  DEC-071, DEC-087, DEC-101
closes:
blocks:
paused_by:
done:      Verdict 1 kept: butterfly quiet history with a malus and gravity ageing, H1 accepted, Elo 10.73 +/- 6.70 over 6412 games against 6e0afa0, shipped at 40f5b56. Verdict 2 measured and reverted: persistence across go, H0 accepted, Elo -1.65 +/- 4.22 over 15398 games against 40f5b56, DEC-101. src/ tests/ tools/ MANUAL.md byte-identical to 40f5b56 by tree hash, search_bench 944870/5202441/533229 unchanged, datagen re-verified. The accepts clause on clearing carried history is discharged in the stamp: the verdict removed its subject. Both builds green, clang-format clean, --touches 0 flagged.

## The hazard is the band, not the search

`piece_values_abs` and the ordering bands clear each other by 100 points: a king
capturing a pawn scores 900100 against 900000 for a killer. `ORDER_HISTORY_MAX`
exists to keep an accumulated history score under a killer, and its own comment
in `src/search_params.hpp` says so. A malus introduces negative scores and
ageing rescales every value in the table, so both touch the one quantity whose
failure mode is a silent strength regression rather than a wrong node count.


## Re-scoped 2026-08-19, and it moved to the front of the search block

Three things rather than two, because the surveyed record treats them as one
mechanism.

**Malus.** A quiet move that caused a cutoff gets a bonus today; every quiet
that was tried at that node and failed gets nothing. Without a penalty, history
is a monotone "moves that ever worked" counter rather than a signed preference.
Reported **+37.5 at Weiss and +28 at Lynx at ~2600** -- the largest single
history patches on record and larger than most features on this plan.

**Gravity instead of ageing.** `history += bonus - history * abs(bonus) / MAX`
is self-normalising: entries asymptote to the bound, an unexpected cutoff moves
a lot and an expected one moves little. It **replaces** periodic halving rather
than joining it -- do one or the other, not both.

**Butterfly indexing.** `[piece][to]` conflates a knight on b1 with a knight on
g1 going to the same square. `[colour][from][to]` is what everything surveyed
uses.

### The hazard this walks straight into

`CLAUDE.md` records that the ordering bands clear each other by 100 points and
that inverting a capture against a killer is silent. **Malus makes quiet
history negative**, and quiets are scored today as the raw history value in a
band whose floor is zero. A quiet at -8192 underflows into whatever sits below
it. Give quiets their own band with headroom of at least twice the bound, or
clamp at ordering time -- and put the assertion in a test, because the symptom
is a strength regression and not a wrong node count.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

The modern update, complete on CPW History Heuristic, is one mechanism:

    clampedBonus = clamp(bonus, -MAX_HISTORY, MAX_HISTORY)
    history[sideToMove][from][to] += clampedBonus
        - history[sideToMove][from][to] * abs(clampedBonus) / MAX_HISTORY

applied with a positive bonus to the quiet that cut off and with a negative
bonus -- the malus -- to "all quiet moves that were previously searched" at
that node when a quiet fails high. All tried quiets, not only late ones: no
surveyed description restricts the malus to a suffix of the move list. CPW
gives the formula no attribution and states its two properties: it "clamps
history values from -MAX_HISTORY to MAX_HISTORY" and it "scales up history
updates when a beta cutoff is unexpected, and scales down history updates when
a beta cutoff is expected". Both follow from the algebra: the update is
`entry * (1 - |b|/MAX) + b`, so with `|b| <= MAX` an entry in `[-MAX, MAX]`
stays there by induction, a repeated bonus converges on exactly `+MAX` and
never overshoots, an expected cutoff moves the entry by almost nothing and an
unexpected one by almost `2b`. Every update shrinks the old value by
`(1 - |b|/MAX)`, so decay is by construction -- why it **replaces** halving.

Bonus forms published: a multiple of depth or `depth*depth` (CPW basic form;
MadChess devlog "distance to the horizon squared"), and the linear-with-offset
`bonus = 300 * depth - 250` in CPW's own gravity snippet. The cap is the
clamp; no separate cap appears. Malus: same magnitude negated in the base
form, and "stronger programs have a separate formula for maluses" (CPW) --
Weiss measured the split formula plus SPSA at **+5.78 +/- 4.09 LTC** (commit
e9f3621, PR #695, 2023-11-21) and Lynx later moved to `x^2 + x + c` split
between bonus and malus (commit 888c6f2, PR #1818, 2025-07-09).

Indexing: butterfly boards, `[colour][from][to]`, 2x64x64 -- the name is Dap
Hartmann's, 1988, ICCA Journal (CPW Butterfly Boards). CPW gives `[piece][to]`
as the alternative; the gravity snippet itself is written on butterfly.

Persistence: published practice keeps quiet history across searches within a
game and clears on a new game. The per-patch records: Lynx PR #637 "Stop
clearing quiet history", merged 2024-02-03, **+12.5 +/- 6.6, LOS 100 %, H1
accepted** over 6419 games; the closed Lynx PR #457 measured the alternatives
against a keeping main -- decay-to-50 % between searches **-16.6**, decay-to-90 %
**-6.3 (H0)**, "always clear history" **-12.1 (H0)** -- so keep beats both
halving-between-searches and clearing. cdani on talkchess (t=62676): tables
cleared at the start of the search "probably is not worth it".

The step's two cited figures, traced to their public records:

- **Weiss +37.5**: commit c5d4921, 2020-05-30, PR #296 "Quiet history malus"
  -- "Give a malus to quiet moves that failed to produce a cutoff if another
  quiet move manages to produce one. ELO | 37.49 +- 12.19 (95%) SPRT |
  60.0+0.6s Threads=1 Hash=128MB LLR | 2.95 [0.00, 5.00] Games | N: 1228".
- **Lynx +28**: PR #610 "Add basic (quiet) history malus/penalty", merged
  2024-01-14 -- "Elo difference: 28.0 +/- 10.3, LOS: 100.0 %", H1 accepted.
  The same PR's ablations: penalising **only** quiets beat penalising captures
  too by +5.4; a capture malus alone ran ~+1.19 and inconclusive; excluding
  illegal moves from the malus span was +0.91 (kept as correctness).

### 2. Shape for chesso

Today, one table and three sites:

**Every line number below was re-derived against `6e0afa0` on 2026-08-21 and
six of them were wrong** -- the section was written before S107, S142, S149 and
S141 landed. The corrected anchors, with what was there before in brackets:

- `src/data_structures.hpp:450` -- `int history_moves[12][64]; // [piece][destination]`
  inside `search_state_t`, which is stack-allocated fresh per `go`
  (`src/chesso.cpp:647`, `search_state_t state = {};`) -- so "zeroed on every
  go" is a lifetime accident, not a clear anyone wrote. Both correct.
- Write: `src/search.cpp:759-773`, fail-high block, after `unmake_move` at
  `:743` [was `:727`] so `game->board.active_color` is the mover again. Gate at
  `:760` [was `:744`] and it reads `if (!is_capture)` alone -- S107 already
  deleted the `!is_check_move` term this section said it would. Bonus at `:770`
  [was `:749`] `depth * depth`, saturation at `:773` [was `:752` and again
  `:768`] `std::min(history + bonus, ORDER_HISTORY_MAX)`. No malus, no decay,
  no halving anywhere (grepped).
- Read: `src/evaluation.cpp:1169`, `score_move` returns the raw entry as the
  quiet's ordering score; bands at `src/evaluation.cpp:33-37` (TT 2000000,
  captures >= 900100 worst-case, killers 900000/800000, counter 700000). Both
  correct. `ORDER_HISTORY_MAX` is at `src/search_params.hpp:64` [was `:45`] and
  its declared maximum is **699900**, not the 899999 quoted twice below: S142
  narrowed it to 100 under `ORDER_COUNTER`. The default is still 600000.

**No tried-quiets list exists.** The move loop (`src/search.cpp:635-679`, the
`for` opens at `:635` and not `:645`) keeps only `moves[]/scores[]` and
`legal_moves_counter`; `moves[0..i-1]` is the tried prefix in search order but
contains captures. So the malus needs a new local `move_t
quiets_tried[MAX_MOVES]` + count, appended only after `make_move` succeeds and
the move passes the same eligibility as the bonus gate.

**One half of the published shape does not apply here and that is a correction
to this section, not a finding in the tree.** Lynx's illegal-move span fix
(PR #610) needs a pseudo-legal generator. chesso's is a *legal* generator --
`generate_moves()` emits legal moves only (`src/bitboard.cpp:885`) and
`make_move` returns false on exactly one path, a full game-history stack
(`src/bitboard.cpp:752`), which the search cannot reach because it is bounded
by `MAX_PLY`. So the `continue` at `src/search.cpp:666` [was `:656`] is dead
for legality, no illegal move can enter the span whatever the list is built
from, and a test asserting that none did would hold over an empty set. The list
is still built after a successful `make_move`, because that costs nothing and
is correct under either generator. Lynx's other bug, the +12.89 off-by-one of
PR #1756, does apply and is what the append-after-the-cutoff-test placement is
for.

New shape: `int16_t quiet_history[2][64][64]` -- `[active_color][from][to]`,
16 KB -- indexed by `game->board.active_color` at both sites (available at
both; the write site runs after unmake). Score stays `return entry`: range
`[-HISTORY_MAX, +HISTORY_MAX]`, below counter 700000 with clearance, and
nothing sits below quiets today, so the negative half is safe until S025 ever
revives a below-quiets band (assert it anyway -- section 5). Killers and
countermoves stay per-`go`; the goal names quiet history only.

Persistence: hoist the table out of `search_state_t` into a struct owned
beside `tt` in chesso.cpp, wired as a pointer exactly like `state.tt = &tt`
(chesso.cpp:647-648), so tests still own private instances.
`command_ucinewgame` (chesso.cpp:1121) clears it alongside `tt_reset`.
Descendant detection, decided at `go` time not `position` time (several
`position` commands can arrive per `go`): remember the searched root's zobrist
(`game->board.hash`, board_t at data_structures.hpp:298) when a search starts;
at the next `go`, keep the table iff that remembered key equals the current
hash or appears in `game->history.entries[].hash` (history_t,
data_structures.hpp:334 -- each entry stores the key of the position the move
was played *from*, and `command_position` replays moves via `try_move`, so a
`position startpos moves ...` chain contains the previous root). A fresh FEN
jump leaves the key absent -- clear. Note: the descendant rule is this step's
own hardening; the published +12.5 priced only "stop clearing per go".

### 3. Implementation sketch

Verdict 1 lands as one commit (DEC-087 j), built and tested in this order:

1. **Butterfly reshape** -- not node-identical, do not pretend it is: the two
   indexings partition moves differently (same-type pieces on different
   from-squares share a `[piece][to]` cell but not a butterfly cell; different
   piece types on the same from/to share the butterfly cell but not
   `[piece][to]`), so accumulated scores differ, `pick_next_move` order
   differs, the tree differs. Play-altering; it rides in verdict 1. Re-target
   the existing table reads in tests -- `tests/test_evaluation.cpp:755` and
   `:845` [was `:754` and `:775`], `tests/test_search.cpp:502` and `:606`
   [was `:485-491`] -- re-target, never weaken.
2. **Gravity + malus**, extracted as a testable helper rather than inline:
   `history_gravity_update(int16_t& entry, int bonus)` (clamp bonus, then the
   CPW line) and `history_on_quiet_cutoff(state, side, cutoff_move,
   quiets_tried, n, depth)` applying `+bonus` to the cutoff move and `-malus`
   to every tried quiet -- the cutoff move is never in the list. Delete the
   `std::min` saturation at `src/search.cpp:773` [was `:768`]: gravity
   replaces it, and both together is the double-ageing bug (section 5).
3. **Band-safety test, red first**: drive an entry to each asymptote through
   the helper (repeated max-depth updates -- the precondition, non-vacuous by
   construction), then assert `|entry| <= HISTORY_MAX`,
   `HISTORY_MAX + 100 <= ORDER_COUNTER` clearance on the score_move result,
   and that the malused entry's score orders below a zero-history quiet.
   Unit test on the table, as the accepts demand -- not a game.
4. **Persistence, verdict 2**: the hoist, the ucinewgame clear, the
   descendant check as its own pure function
   (`history_keep_across_go(last_root_hash, game)`) with direct tests: kept
   on a child position, cleared on ucinewgame, cleared on a non-descendant
   FEN. Then the SPRT.

Consequences to carry: `touches:` understates the surface -- score_move is in
src/evaluation.cpp, the struct in src/data_structures.hpp, ucinewgame and the
go path in src/chesso.cpp. New search_params entries change the tune build's
UCI option list: MANUAL.md's tune table and test_uci_surface /
test_search_params follow (S073). ORDER_HISTORY_MAX's job passes to
HISTORY_MAX; retire it or repoint its comment in the same commit, stated.

### 4. Constants and seeds

Every number below is a seed and is fitted or SPSA'd here (DEC-084); none
ships as-is.

- `HISTORY_MAX` (gravity bound): **no publishable seed** -- CPW leaves
  MAX_HISTORY symbolic, the talkchess gravity thread (t=85723) gives no value
  beyond "16 bits" as a width remark. Choose by arithmetic, not by copying:
  a power of two well inside int16, with `HISTORY_MAX + 100 <= 700000`
  clearance and `HISTORY_MAX^2` inside the intermediate type (section 5).
  2^13 or 2^14 are natural first settings; SPSA at S127 moves it.
- Bonus form: keep `depth*depth` (already chesso's, and a CPW-published form)
  parameterised as `HISTORY_BONUS_QUAD/LIN/CONST` seeded `{1, 0, 0}`; CPW's
  `300 * depth - 250` is the published linear seed `{0, 300, -250}` if the
  quadratic misfits. Malus: same form, own constants
  (`HISTORY_MALUS_QUAD/LIN/CONST`, seeded equal to the bonus) -- the split is
  the published follow-up (Weiss +5.78 LTC; Lynx x^2+x+c), shipped here only
  as parameters for S127 to separate, not as new numbers.
- Weiss's and Lynx's actual formula constants live in engine source: **no
  publishable seed -- fit from scratch.**

### 5. Pitfalls

- **The band, with the real numbers.** Bands clear by 100:
  `ORDER_CAPTURE + MVV_PAWN - MVV_KING = 1000000 + 100 - 100000 = 900100`
  against `ORDER_KILLER_0 = 900000` (evaluation.cpp:22, 33-37). History's own
  clearance today is 100000 (600000 cap at `src/search_params.hpp:64`
  [was `:45`] against counter 700000). Malus makes the quiet band `[-HISTORY_MAX, +HISTORY_MAX]`;
  nothing sits below it today, but S025 (reserve) would put losing captures
  there -- the test pins both edges so that arrival fails loudly instead of
  silently.
- **Integer overflow in the gravity product.** `entry * abs(clampedBonus)`
  reaches `HISTORY_MAX^2`. At the old 600000 scale that is 3.6e11 -- int32 UB.
  Keep `HISTORY_MAX <= 46340` or widen the intermediate. Multiply before
  divide, or the decay term truncates to zero and gravity silently stops
  ageing.
- **Malus double-counting the cutoff move.** The cutoff move gets the bonus
  and must not be in `quiets_tried` when the malus loop runs. Lynx shipped the
  inverse off-by-one -- excluding the last *tried* quiet to protect a cutoff
  move that was never in the span -- and the fix alone was +12.89 (PR #1756).
  The unit test asserts, from zeroed tables: cutoff entry strictly positive,
  tried entries strictly negative.
- **Illegal moves must not be malused.** `moves[0..i-1]` contains
  `make_move`-failed entries; the list is appended only on successful make
  (Lynx PR #610's span fix). The list also mirrors the bonus gate exactly --
  after S107 that is `!is_capture` alone; an asymmetry between what can earn
  the bonus and what can earn the malus is a silent bias.
- **Gravity plus halving is double ageing.** Chesso has no halving today
  (verified by grep), so the trap here is the existing `std::min` saturation
  surviving next to the clamp gravity already provides -- delete it, and let
  the band test prove the bound is gravity's.
- **Persistence poisoning.** A GUI that jumps positions without `ucinewgame`
  would feed one game's preferences to another; the accepts' descendant rule
  is the guard. Watch the fastchess path: it replays `position startpos
  moves ...` fresh each `go`, whose chain does contain the previous root --
  the keep path must engage there or verdict 2 measures nothing.
- **S107 interaction.** Checking quiets are newly eligible: they earn bonuses,
  maluses, and list membership like any quiet. S107's own test asserts
  presence only, but it reads the old `[piece][to]` cell -- the reshape
  re-targets that lookup mechanically.

### 6. Measurement

Two SPRTs at the S105 regime -- 8+0.08, Hash=16, UHO book -- gainer bounds
`elo0=0 elo1=5` (DEC-063: the bounds straddle the expected effect; the
published seeds are +37.5/+28 for verdict 1 and +12.5 for verdict 2, all
comfortably above the pair). **Verdict 1 covers malus + gravity + butterfly
as one mechanism per DEC-087 (j)**; verdict 2 is persistence alone against
the verdict-1 commit. Order within the step: fast suite green first, the
band-clearance unit test written and observed red before the mechanism lands,
then the SPRTs. Node counts are expected to differ -- nothing here is
behaviour-neutral, so INV-6 takes the SPRT path for both verdicts, and a zero
is recorded as zero.

### 7. Interactions

- **S107** (lands first): leaves one eligibility predicate, `!is_capture`, at
  the update site; S093 inherits it for bonus, malus and the tried list.
- **S024 continuation history**: shares the update form -- factor
  `history_gravity_update` so S024 reuses the clamp, the overflow discipline
  and the bonus/malus split, and iterates the same `quiets_tried` list built
  here. Build the list once; both tables consume it.
- **S109 history pruning**: consumes these scores and its thresholds assume
  *signed* history -- "skip quiets whose history is below T" is meaningless on
  a monotone counter. Keep one read path (score_move or a probe helper) so
  S109 does not grow a second indexing.
- **S098 LMR history scaling**: gravity's fixed `[-MAX, MAX]` range is the
  stable denominator the reduction formula needs; Weiss later reused the malus
  formula to penalise bad LMR re-searches (commit b233bf6, PR #701) -- S098
  material, not this step's.
- **S127 SPSA**: HISTORY_MAX and the six bonus/malus coefficients enter
  src/search_params.hpp with stated ranges, so the full-set run sweeps them;
  the split-formula gain is expected to be found there rather than argued here.

### 8. References

- https://www.chessprogramming.org/History_Heuristic -- gravity formula verbatim, clampedBonus, malus scope ("all quiet moves that were previously searched"), bonus forms, "stronger programs have a separate formula for maluses"; no attribution, no aging section.
- https://www.chessprogramming.org/Butterfly_Boards -- [2][64][64], 4K entries per colour, Hartmann 1988 ICCA Journal.
- https://github.com/TerjeKir/weiss/commit/c5d4921 -- PR #296, quiet history malus, ELO 37.49 +- 12.19 at 60+0.6.
- https://github.com/TerjeKir/weiss/commit/e9f3621 -- PR #695, separate bonus/malus formulas + SPSA, +5.78 +- 4.09 LTC.
- https://github.com/TerjeKir/weiss/commit/b233bf6 -- PR #701, malus formula on bad LMR re-search (S098 material).
- https://github.com/lynx-chess/Lynx/pull/610 -- basic quiet history malus, +28.0 +- 10.3; quiets-only +5.4; illegal-move span fix.
- https://github.com/lynx-chess/Lynx/pull/637 -- stop clearing quiet history, +12.5 +- 6.6.
- https://github.com/lynx-chess/Lynx/pull/457 -- closed: decay 50 % -16.6, decay 90 % -6.3 H0, always-clear -12.1 H0.
- https://github.com/lynx-chess/Lynx/pull/1756 -- malus off-by-one fix (last tried quiet excluded), +12.89 +- 5.46.
- https://github.com/lynx-chess/Lynx (commit 888c6f2, PR #1818) -- x^2 + x + c split bonus/malus formula, read as commit message.
- https://talkchess.com/viewtopic.php?t=85723 -- History Gravity Formula thread; no derivation or values beyond a 16-bit width remark.
- https://talkchess.com/viewtopic.php?t=62676 -- cdani: clearing ordering tables at search start "probably is not worth it".
- https://www.madchess.net/tag/history-heuristic/ -- depth-squared bonus in a devlog, [piece][to] indexing as the alternative lineage.
## Verdict 1 as landed, 2026-08-21

Butterfly reshape, gravity and malus in one change, per DEC-087 (j). Persistence
is untouched and is verdict 2.

### What was built, and where it departs from the sketch above

**Table.** `int16_t quiet_history[2][64][64]` in `search_state_t`
(`src/data_structures.hpp`), `[side to move][from][to]`, 16 KB against the old
3 KB. `score_move` reads it at `src/evaluation.cpp:1173` indexed by
`game->board.active_color`; the write site indexes by the same field after
`unmake_move`, so both see the mover.

**Helpers.** `history_gravity_update(int16_t&, int)` and
`history_on_quiet_cutoff(state, side, cutoff_move, quiets_tried, n, depth)`, both
declared in `src/search.hpp` so the tests drive them directly, both defined at
the top of `src/search.cpp`. The update is the published line with the
multiplication first, `entry + b - (entry * |b|) / MAX`, and two asserts: the
precondition `|entry| <= MAX` and the postcondition. **Both configurations build
with `-DNDEBUG`, so those two are documentation and not enforcement** -- what
enforces the bound is the algebra and the unit case that drives 5000 updates into
each asymptote and checks every intermediate. The `std::min` saturation is
deleted, not kept beside the clamp.

**Malus span.** A local `move_t quiets_tried[MAX_MOVES]` plus a count, appended
to at the **bottom of the move loop**, past the `break` a cutoff takes. So the
cutoff move is absent from the span structurally rather than by an index, which
is the shape Lynx's +12.89 off-by-one (PR #1756) argues for; the alternative,
appending on entry and excluding the last element, is the bug itself. The gate is
`!is_capture` and nothing else, byte for byte the gate the bonus uses.
`history_on_quiet_cutoff` charges the maluses first and credits the bonus last,
so where two moves alias one butterfly cell -- two promotions from one square --
the move that actually cut off is the one whose update lands.

**Four departures from section 4, each with its reason:**

1. `HISTORY_MAX` is spelled **`QUIET_HISTORY_MAX`** / `QuietHistoryMax`.
   `HISTORY_MAX_SIZE` already exists in `src/data_structures.hpp:48` and is the
   game move stack's length. Two names one token apart on unrelated quantities
   is a mistake waiting to be made.
2. Its **declared range is 1 to 32767 and both bounds are arithmetic**, not the
   "power of two well inside int16" the section suggests. The floor is 1 because
   the update divides by it; the ceiling is `INT16_MAX` because that is the
   entry's type, and it doubles as the overflow guard -- the intermediate reaches
   `MAX^2` and 32767^2 is 1.07e9, inside int32. Section 4's `HISTORY_MAX + 100 <=
   700000` clearance is satisfied by six orders of magnitude and stops being the
   binding constraint. **Default 8192**, 2^13.
3. **`ORDER_HISTORY_MAX` is retired rather than repointed.** Its job was a
   saturation ceiling on an unbounded accumulator and there is no unbounded
   accumulator left. The tune build's option list loses `OrderHistoryMax` and
   gains seven: `QuietHistoryMax` and the six coefficients. `MANUAL.md`,
   `tests/test_search_params.cpp`'s golden rows and `test_uci_surface` follow.
4. **The illegal-move test in step 3 of the sketch is not written**, because it
   would be vacuous here. Section 2 above carries the correction: chesso's
   generator is legal, so no illegal move can reach the span whatever builds it.

### Red first, observed and recorded

The structure landed first with a deliberately incomplete helper -- clamped to
the `int16_t` range, no gravity, no malus -- so the three new cases could be seen
failing against real numbers rather than against a missing symbol. Verbatim, from
`build/tests/test_search`:

    TEST CASE:  gravity holds a history entry inside the bound at both asymptotes
    tests/test_search.cpp:644: FATAL ERROR: REQUIRE( rising <= max ) is NOT correct!
      values: REQUIRE( 8208 <= 8192 )

    TEST CASE:  a quiet cutoff maluses the quiets tried before it
    tests/test_search.cpp:731: ERROR: CHECK( ... ) is NOT correct!
      values: CHECK( 0 <  0 )
      logged: A quiet tried before the cutoff scores 0 and not a malus.
      [three times, once per tried quiet]

    TEST CASE:  the cutoff move is credited and the quiets before it are charged
    tests/test_search.cpp:844: ERROR: CHECK( state.quiet_history[WHITE][g1][g2] < 0 ) is NOT correct!
      values: CHECK( 0 <  0 )
    tests/test_search.cpp:845: ERROR: CHECK( state.quiet_history[WHITE][g1][h2] < 0 ) is NOT correct!
      values: CHECK( 0 <  0 )

    [doctest] test cases:   3 |   0 passed | 3 failed | 63 skipped
    [doctest] assertions: 173 | 167 passed | 6 failed |

Green after the real update, 20065 assertions over the same three cases.

The third case is driven through `negamax` and is where the call site is held.
Its position is `6rk/b5pp/7N/8/3N4/8/8/6K1 w - - 0 1` -- from a tool and not from
the board (CLAUDE.md): python-chess reports `is_valid() True`, `is_check()
False`, and the whole mating set as `[('Nf7#', 'h6f7')]`. Nf7 is also the first
quiet the generator emits, so with a cold table the malus span would be empty and
every assertion would hold vacuously; two seeded killers put two king moves in
front of it, which is the case's stated precondition. `Nxg8` is a capture ordered
ahead of all three and is asserted to earn neither bonus nor malus, which is what
holds the gate mirror. Black's whole half of the table is asserted empty, which
is what holds the colour axis.

### Band clearance, now that history can be negative

`tests/test_evaluation.cpp` "the declared history ceiling clears the band above
it" was S142's and is re-targeted, never weakened. It reads `QuietHistoryMax`'s
declared maximum out of `search_param_info()` and now drives the entry to **both**
edges of the closed interval:

- precondition 1, unchanged: the cheapest capture stands exactly 100 above the
  first killer in this position, so the 100 asked for below is read off the code
  and not copied out of a comment;
- precondition 2, unchanged: the countermove band is the one immediately above
  history;
- precondition 3, **extended**: `score_move` returns `+declared_max` and
  `-declared_max` unmodified. Without the second half a `score_move` that clamped
  the malused half back to zero would make the floor assertion vacuous;
- the ceiling: `s_counter - (+max) >= 100`;
- the floor: `s_counter - (-max) >= 100`;
- and the emptiness below quiets, which is the edge that did not exist before
  this step: `min(capture, killer_0, killer_1, counter) - max >= 100`, so the
  whole interval `[-max, +max]` stands clear of every other band. **What sits
  below quiets today is nothing** -- `score_move`'s last branch is the table read
  -- and S025's losing captures would arrive exactly there and fail here.

### Measurement

`tools/search_bench.py` at depth 13, two interleaved passes, node counts
identical across passes:

| position | reference `6e0afa0` | candidate | delta |
|---|---|---|---|
| midgame | 863774 | 944870 | +9.39 % |
| kiwipete | 7248224 | 5202441 | -28.22 % |
| tactical | 529142 | 533229 | +0.77 % |
| total | 8641140 | 6680540 | **-22.69 %** |

Best move unchanged at all three, `c3d5` / `e2a6` / `d7c8q`. Node throughput
7816/7458/7627 knps against 7445/7233/7568 at depth 11, so **about 2 to 3 %
slower per node** -- the malus loop and a 16 KB table against 3 KB -- and the
wall clock over the three positions falls 1.194 s to 0.939 s regardless.

Nothing here is behaviour-neutral, so INV-6 takes the SPRT path.

**The mate suite did not move.** `tests/test_engine.cpp` "engine: mate safety",
S145's 48 constructed forced mates: 16 of 16 mates in two exact at delay 0, 8 of
8 mates in three against a floor of 7, 0 of 8 at four and five -- the same
reading `src/search_params.hpp` records for the shipping bounds.

### Gates

- `cmake --build build -j12 && ctest --test-dir build -L fast` -- 19 of 19
  passed, 15.5 s.
- `cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast` -- 19 of
  19 passed, 15.7 s.
- `./clang-format.sh --check` -- exit 0.
- `tools/plan_prose_check.py --touches` -- 0 flagged over 65 files, unchanged.
- `tools/plan_prose_check.py --citations` -- **42 flagged at `6e0afa0`, 84 in
  this tree**, and the increase is the point rather than a regression to hide:
  editing `src/search.cpp` and `tests/test_search.cpp` moved several hundred
  lines, so every other step file citing a line below the edit now drifts. The
  drift is mechanical, it is what the checker exists to report, and repairing 48
  citations across fifteen other step files is not this step's change. This
  step's own six flags are repaired rather than reset: see the note below.

### The six citations this file was flagged for, and what happened to each

Moving the file into `plan_current/` reset its drift baseline before a single
character was edited, which is exactly the hazard S139 fell into -- the count
falls and nothing was fixed. So each is named:

| cited | held at baseline | true at `6e0afa0` | verdict |
|---|---|---|---|
| `search.cpp:759-773` | comment opener | opener right, range end had moved | not wrong, re-anchored anyway since the block is gone |
| `search_params.hpp:45` (x2) | `ORDER_HISTORY_MAX ... 899999` | line 45 is prose; the row is `:64` and reads `699900` | **live-wrong**, corrected |
| `test_evaluation.cpp:754` | the assignment | `:754` is the declaration, assignment is `:755` | **live-wrong**, corrected |
| `test_search.cpp:485-491` | `size_t history_entries = 0;` | `:485-491` is the killer loop; history is `:497-505` | **live-wrong**, corrected |
| `search.cpp:768` | the `std::min` | `:768` is `best_move = moves[i];`, the `std::min` is `:773` | **live-wrong**, corrected |

Section 2 above carries the repairs and five more the checker never flagged
because it only reports what changed since a baseline: `:656`, `:727`, `:744`,
`:749` and the `for` opener at `:645`, all off by ten to sixteen lines.

### SPRT

`adocs/data/S093_sprt_v1.sh`, launched 2026-08-21 21:31, log
`.tuning/sprt_s093_v1.log`. Bounds `elo0=0 elo1=5 alpha=0.05 beta=0.05`,
fastchess.sh's default for a change claimed to gain, against reference
`6e0afa0`. All three readings -- H1, H0, no verdict -- were pre-registered in the
script header before the first game, and the no-verdict clause named DEC-063's
hazard explicitly: the same pair random-walked 6 h 36 m over 9036 games for S068
and returned nothing.

**H1 accepted, 2026-08-22, in 2 h 43 m 59 s over 6412 games:**

    Elo: 10.73 +/- 6.70, nElo: 13.63 +/- 8.50
    LOS: 99.92 %, DrawRatio: 37.49 %, PairsRatio: 1.16
    Games: 6412, Wins: 2309, Losses: 2111, Draws: 1992, Points: 3305.0 (51.54 %)
    Ptnml(0-2): [307, 619, 1202, 725, 353], WL/DD Ratio: 2.71
    LLR: 2.95 (100.2%) (-2.94, 2.94) [0.00, 5.00]
    SPRT ([0.00, 5.00]) completed - H1 was accepted

0 time forfeits in 6413 games, 68.9 % decisive. The pre-registered H1 clause
governs: **the mechanism gains 5 Elo or more, and that is the claim -- not
"+10.73".** The point estimate is biased upward by the early stop (DEC-063), so
the magnitude is not what the run establishes. Kept.

**The published figures did not transfer, and this is DEC-019's thesis in its
milder form.** Weiss measured the quiet history malus at **+37.49 +- 12.19** over
1228 games (commit c5d4921, PR #296) and Lynx at **+28.0 +- 10.3**, LOS 100 %
(PR #610). Here it reads **+10.73 +- 6.70** over 6412 games -- roughly a third of
either, on five times Weiss's sample and a comparable one to Lynx's, and the two
published intervals do not overlap this one. The *direction* transferred and the
*magnitude* did not. DEC-019's existing entries are the harsher case, where a
published figure measured 0 or negative; this is the same lesson in the form that
is easier to miss, because the change is a real gain and the number is still
wrong. A published figure decided what to try. It did not decide what we got.

## Verdict 2 as landed, 2026-08-22

Persistence across `go` alone, against verdict 1's commit `40f5b56`.

### What was built

**The hoist.** `quiet_history_t` in `src/data_structures.hpp` -- the
`int16_t entries[2][64][64]` board, plus the root it was learned from as
`hash_t last_root` and a separate `bool has_last_root`. The flag is not a zero
sentinel on the key: zero is a legal zobrist, and a sentinel that can occur is a
sentinel that will. `search_state_t` now holds `quiet_history_t*`, wired exactly
as `state.tt` is, so a test owns a private table. Killers, countermoves and the
PV stay per-`go`; the goal names quiet history only.

**The two clears, and nothing else clears it.** `command_ucinewgame` drops the
table beside `tt_reset`, root included. `iterative_deepening_search` drops it
when `history_keep_across_go()` says this root does not descend from the last
one searched, then records the new root. **Decided at `go` time and never at
`position` time**, because several `position` commands can arrive between two
searches and the question is whether *this* root descends from the last one
searched, not whether some position in between did.

**The rule** is a pure function of the table and the game. It answers by looking
for the remembered root's zobrist in `game.history`: `command_position` replays
`position ... moves ...` from scratch through `try_move`, and each entry stores
the key of the position its own move was played *from*, so that array is exactly
the chain of ancestors. A jump to a bare FEN leaves the chain empty and the
previous root absent.

**Dropped rather than decayed**, which is measured and not a preference: Lynx
PR #457 put decay-to-50 % between searches at **-16.6**, decay-to-90 % at
**-6.3 (H0)** and always-clear at **-12.1 (H0)** against a keeping main.

### A bug this step introduced and fixed before anything else

`tools/datagen.cpp:117` built a `search_state_t` and never set the new pointer.
It compiled clean, no test in the fast label drives datagen, and the first quiet
move `score_move()` looked at would have dereferenced `nullptr`. AGENTS.md 0:
found, so fixed first. The table is now owned by the worker at **game** scope --
cleared in the same place `tt_reset` is, once per game, not once per move --
which is the lifetime the UCI layer gives it, so the data generator plays the
engine that ships rather than a variant of it. Proved by running it: 2 games,
3000 nodes a move, 182 positions written, exit 0.

### Red first, observed and recorded

Three stages, because the rule has two directions and one stub cannot show both.

**Stage 1 -- the hoist with the old policy** (drop the table at every `go`, no
`ucinewgame` clear). This stage is byte-for-byte the shipping behaviour of
`40f5b56` and was checked so: `search_bench` at depth 13 read 944870 / 5202441 /
533229, identical. Then:

    TEST CASE:  ucinewgame clears the quiet history it carries
    tests/test_engine.cpp:1311: FATAL ERROR: REQUIRE( uci_quiet_history()->has_last_root ) is NOT correct!
      values: REQUIRE( false )

    TEST CASE:  quiet history crosses a go into a descendant and not into a stranger
    tests/test_engine.cpp:1355: ERROR: CHECK( history_entries_filled(uci_quiet_history()) > 270 ) is NOT correct!
      values: CHECK( 0 >  270 )

The second is the carry itself: the descendant `go` found the table wiped.

**Stage 2 -- persistence with both guards missing** (never drop, anywhere;
`history_keep_across_go` stubbed to `return true`). The two clear clauses:

    TEST CASE:  ucinewgame clears the quiet history it carries
    tests/test_engine.cpp:1318: ERROR: CHECK_EQ( history_entries_filled(uci_quiet_history()), 0 ) is NOT correct!
      values: CHECK_EQ( 439, 0 )
    tests/test_engine.cpp:1322: ERROR: CHECK_FALSE( uci_quiet_history()->has_last_root ) is NOT correct!
      values: CHECK_FALSE( true )

    TEST CASE:  quiet history crosses a go into a descendant and not into a stranger
    tests/test_engine.cpp:1367: ERROR: CHECK( history_entries_filled(uci_quiet_history()) <= 270 ) is NOT correct!
      values: CHECK( 460 <= 270 )

and the rule's three refusing branches:

    tests/test_search.cpp:901: ERROR: CHECK_FALSE( history_keep_across_go(&history, &game) ) is NOT correct!
      values: CHECK_FALSE( true )
    tests/test_search.cpp:944: [same]
    tests/test_search.cpp:951: [same]

**Stage 2b -- `history_keep_across_go` stubbed to `return false`**, for the two
branches stage 2 could not reach:

    tests/test_search.cpp:908: ERROR: CHECK( history_keep_across_go(&history, &game) ) is NOT correct!
      values: CHECK( false )
    tests/test_search.cpp:925: [same]

All five branches of the rule and both clear clauses observed failing. Green at
stage 3.

### Why the integration cases can be exact

`MAX_MOVES` is the line between "the table survived a `go`" and "only this `go`
wrote to it", and it is derived rather than chosen: at `go depth 1` every child
of the root is quiescence, which writes no history, so the root node is the only
writer and it writes at most one entry per quiet it has. The precondition is
that one `go depth 10` fills **more** than that -- it fills 439 -- so the two
sides cannot be confused. The stranger is given as a bare FEN, which is what
empties the ancestor chain; the descendant is `position startpos moves ...` with
one more move, which is the exact shape fastchess replays before every `go`.
That shape is the one the pitfall list says must engage or the verdict measures
nothing, and it is now a test rather than an argument.

### Measurement

`search_bench` at depth 13 is **unchanged: 944870 / 5202441 / 533229**, best move
unchanged. That is the descendant rule working and not an absence of effect --
the tool gives each of its three positions as a bare FEN, so each search
correctly fails the descendant test and starts from an empty table. The effect
this verdict measures exists only across the moves of one game, which no
fixed-position benchmark can see. INV-6 is not available either way.

### Gates

- `cmake --build build -j12 && ctest --test-dir build -L fast` -- 19 of 19.
- `cmake --build build-tune -j12 && ctest --test-dir build-tune -L fast` -- 19 of 19.
- `./clang-format.sh --check` -- exit 0.
- `tools/plan_prose_check.py --touches` -- 0 flagged over 65 files.
- `--citations` unchanged in kind from verdict 1: line drift in other step
  files, advisory, not swept (the coordinator decided against a sweep step --
  the durable fix is anchoring to test titles and symbols, which is S144).

### SPRT

`adocs/data/S093_sprt_v2.sh`, launched 2026-08-22 00:37, log
`.tuning/sprt_s093_v2.log`, out `/tmp/chesso_sprt_s093_v2_20260822_003734`.
Bounds `elo0=0 elo1=5 alpha=0.05 beta=0.05` against reference `40f5b56`, verdict
1's commit and not `6e0afa0`.

All three readings are pre-registered in the script header before the first
game, and **the no-verdict clause carries two named candidate explanations**
rather than one: that the effect is simply smaller here than the published
+12.5 -- verdict 1 measured a third of its own prior, and a third of +12.5 is
about +4, inside the indifference region -- and that the descendant rule, which
is this step's own hardening and is **not** what Lynx PR #637's +12.5 +/- 6.6
priced, is clearing a table the published change would have kept. The fastchess
path argues the second cannot happen and `tests/test_engine.cpp` drives that
shape, but an argument is not a measurement; settling it means instrumenting how
often the keep path engages in a real match, not reasoning further.

## Verdict 2's verdict, and the revert, 2026-08-22

**H0 accepted in 6 h 35 m over 15398 games against `40f5b56`:**

    Elo: -1.65 +/- 4.22, nElo: -2.14 +/- 5.49
    LOS: 22.22 %, DrawRatio: 39.04 %, PairsRatio: 0.99
    Games: 15398, Wins: 5231, Losses: 5304, Draws: 4863, Points: 7662.5 (49.76 %)
    Ptnml(0-2): [764, 1591, 3006, 1630, 708], WL/DD Ratio: 2.66
    LLR: -2.96 (-100.5%) (-2.94, 2.94) [0.00, 5.00]
    SPRT ([0.00, 5.00]) completed - H0 was accepted

0 forfeits in 15399 games. **A well-measured null and not an ambiguous one:**
15398 games and a +/- 4.22 interval exclude the +5 the bounds were set to find
and exclude a loss of 6 or more alike, the point estimate is negative and LOS is
22 %. Against a published **+12.5 +/- 6.6 over 6419 games at LOS 100 %** (Lynx
PR #637). The pre-registered H0 clause governs and it is recorded here as it
stands.

### Reverted in full, to `40f5b56`'s behaviour

The coordinator's decision under the owner's delegation, taken on three grounds
and written here so the owner can reverse it:

1. **It does not gain**, and the clause pre-registered before the first game
   says so.
2. **Two transfer failures in one step.** Verdict 1 measured a third of its
   prior; verdict 2's prior came back with the wrong sign. DEC-019 says a
   published figure decides what to try and never what to conclude, and keeping
   this on Lynx's number after measuring it here is the precise move that rule
   forbids.
3. **The complexity has a demonstrated cost.** The hoist produced a `nullptr`
   dereference in `tools/datagen.cpp` that compiled clean and that no
   fast-label test caught. One silent defect per zero Elo.

**Considered and rejected: keeping the hoist alone.** It is the tempting middle
-- it converts an accidental lifetime into an explicit one, and it is
INV-6-provable against `40f5b56`. It loses because with the table cleared every
`go` again the descendancy code is dead, the `ucinewgame` case is vacuous, and a
refactor with no behaviour change and no live tests is not worth the surface it
adds. AGENTS.md does permit keeping a zero with the reason stated -- S005, S006
and S015 all were -- but those cost nothing and removed a state the design did
not intend; this one costs a table lifetime, a descendancy rule and a
UCI-visible contract. Revisit when S023 or S099 needs the same lifetime for a
reason that is being measured. DEC-101.

### The revert, shown rather than claimed

`src/`, `tests/`, `tools/` and `MANUAL.md` are byte-identical to `40f5b56`,
checked by tree hash and not by reading the diff:

    src        bc18a356835c10e9cbd59e330025bca36b6fc6d8
    tests      d868c4ed2daf114af4da8076b657dbaae3602230
    tools      48c0724a55ff0d361a8523c9d4ffa406377c3416
    MANUAL.md  217eae893d8e197433b7afc4c157f1dbe62767d7

`grep` for `quiet_history_t`, `uci_quiet_history`, `history_keep_across_go` and
`quiet_history_clear` over `src/ tests/ tools/ MANUAL.md` returns nothing.
`search_bench` at depth 13 reproduces **944870 / 5202441 / 533229**, best move
`c3d5` / `e2a6` / `d7c8q`, which is verdict 1's shipped engine exactly.

**datagen checked rather than assumed**, since its bug existed only because of
the hoist. `run_search` is back to its three-parameter form with no history
argument, `search_state_t` owns the table by value so there is no pointer that
can be null, and it runs: 2 games, 3000 nodes a move, **136 positions, exit 0**.
The count is 136 and not the 182 the hoisted build wrote from the same seed,
which is the second half of the check -- persistence changed which positions the
filter kept, so the revert restored datagen's *behaviour* and not only its
shape.

### The accepts clause that cannot be satisfied, discharged

> history carried across `go` is cleared on `ucinewgame` and on a position that
> is not a descendant of the last one searched, with a test for both

**Discharged, not dropped, and the verdict is the reason.** The clause is
conditional on history being carried across `go`. It is not, as of this verdict:
there is no carried history, so there is nothing for either rule to clear and
neither test has a subject. It was satisfied while persistence was in the tree
-- both clauses were driven through the UCI layer, both were observed red first,
and the evidence is in the section above -- and it is unsatisfiable after the
revert by construction rather than by omission. Same shape as S141's discharge
of its S085 clause: the clause was produced, the ground moved, and the stamp
says so instead of the plan quietly losing a line.

Every other accepts clause holds and is verdict 1's: two SPRT verdicts, the
malus reaching the quiets tried before the cutoff and not the cutoff move
asserted by a unit test on the table, the gravity bound keeping every score
inside `QuietHistoryMax` with the band clearance asserted at both edges, and the
fast suite green.

### What is kept

Everything measured, which is the point. The three-stage red-first evidence, the
descendancy design and its five branches, the derived `MAX_MOVES` integration
bound and why it is exact, the datagen bug and its fix, and both verdicts with
their figures and wall-clocks. Reviving this means re-measuring a finished
design, not rebuilding one.

## Cost

Two SPRTs, 2 h 44 m and 6 h 35 m, 21810 games. One kept, one reverted. Both are
results.

author:    Maksym Bodnar
