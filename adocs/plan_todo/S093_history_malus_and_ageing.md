id:         S093
goal:       history gets a malus for the moves that were tried and failed, a gravity update that ages it by construction, butterfly indexing, and survives across go within one game
accepts:    two SPRT verdicts: malus and gravity land together -- they are one published mechanism, `entry += bonus - entry * abs(bonus) / MAX`, and splitting them measures each against a table shape it will not ship with (DEC-087) -- and persistence across `go` lands second with its own verdict; the malus applies to the quiet moves searched before the cutoff move and not to the cutoff move itself, asserted by a unit test on the table rather than through a game; the gravity keeps every score inside ORDER_HISTORY_MAX so the move-ordering bands still clear each other by 100 points, with the band clearance asserted (CLAUDE.md hazard, S023, S061); history carried across `go` is cleared on `ucinewgame` and on a position that is not a descendant of the last one searched, with a test for both; the fast suite green
touches:    src/search.cpp history update and the ordering scores, src/search_params.hpp, tests/test_search.cpp
excludes:   capture history, which is S023; continuation history, which is S024; correction history, which is S099
decisions:  DEC-071, DEC-087
closes:
blocks:
paused_by:
done:

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

- `src/data_structures.hpp:450` -- `int history_moves[12][64]; // [piece][destination]`
  inside `search_state_t`, which is stack-allocated fresh per `go`
  (`chesso.cpp:647`, `search_state_t state = {};`) -- so "zeroed on every go"
  is a lifetime accident, not a clear anyone wrote.
- Write: `src/search.cpp:759-773`, fail-high block, after `unmake_move` at
  727 (so `game->board.active_color` is the mover again). Gate at 744
  `if (!is_capture && !is_check_move)` -- S107 deletes the check term first.
  Bonus at 749 `depth * depth`, saturation at 752
  `std::min(history + bonus, ORDER_HISTORY_MAX)`. No malus, no decay, no
  halving anywhere (grepped).
- Read: `src/evaluation.cpp:1169`, `score_move` returns the raw entry as the
  quiet's ordering score; bands at `evaluation.cpp:33-37` (TT 2000000,
  captures >= 900100 worst-case, killers 900000/800000, counter 700000),
  `ORDER_HISTORY_MAX` 600000 at `src/search_params.hpp:45`.

**No tried-quiets list exists.** The move loop (search.cpp:645-679) keeps only
`moves[]/scores[]` and `legal_moves_counter`; `moves[0..i-1]` is the tried
prefix in search order but contains captures and pseudo-legal moves whose
`make_move` failed (the `continue` at 656). So the malus needs a new local
`move_t quiets_tried[MAX_MOVES]` + count, appended only after `make_move`
succeeds and the move passes the same eligibility as the bonus gate. That is
the published shape twice over: Lynx's illegal-move span fix (PR #610) and its
+12.89 off-by-one fix (PR #1756) are both bugs in exactly this list.

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
   the existing table reads in tests (test_evaluation.cpp:754 and :775,
   test_search.cpp:485-491) -- re-target, never weaken.
2. **Gravity + malus**, extracted as a testable helper rather than inline:
   `history_gravity_update(int16_t& entry, int bonus)` (clamp bonus, then the
   CPW line) and `history_on_quiet_cutoff(state, side, cutoff_move,
   quiets_tried, n, depth)` applying `+bonus` to the cutoff move and `-malus`
   to every tried quiet -- the cutoff move is never in the list. Delete the
   `std::min` saturation at search.cpp:768: gravity replaces it, and both
   together is the double-ageing bug (section 5).
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
  clearance today is 100000 (600000 cap at search_params.hpp:45 against
  counter 700000). Malus makes the quiet band `[-HISTORY_MAX, +HISTORY_MAX]`;
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
