id:         S024
goal:       history indexed by the move played n plies ago and the current move
accepts:    two SPRT verdicts, one per table: the one-ply table first, the two-ply follow-up second, each against the commit before it; every src/ citation in this file names a symbol or a test title rather than a line range
            (Folded in from the retired S063 by DEC-086. Offsets beyond two
            plies are worth 1-3 Elo each at 3400 and are not part of this step.
            The sentinel plies are the known crash: (ss-1) and (ss-2) must be
            valid at ply 0 and ply 1, and a null move must still install a
            continuation pointer or the child indexes garbage.)
touches:    src/search.cpp, src/evaluation.cpp score_move, src/data_structures.hpp
excludes:
decisions:
closes:
blocks:
paused_by:
done:

## Note

Chesso has the countermove *heuristic* and no continuation history at any
depth. `move_t counter_moves[12][64]` (`src/data_structures.hpp:436`, written
on beta cutoff at `src/search.cpp:517`, read as the fixed `ORDER_COUNTER` band
at `src/evaluation.cpp:1152-1155`) remembers a single refutation move per
(previous piece, target) and gives it a flat bonus — the countermove heuristic,
Uiterwijk 1992.

Continuation history is a different device: a score table indexed by
(previous move's piece, target) × (current move's piece, target), accumulating
graded bonuses and maluses for every quiet move, added into the ordering score
alongside plain history. One ply back is countermove history, two is follow-up
history; engines carry the heuristic and both tables at once, and the one-ply
table is consistently reported the stronger half of the pair.

Build the one-ply table first and take its verdict, then the two-ply table and
take its own. 2026-08-13_plan_review-F02 is why this note reads this way: the
previous version called the countermove table one-ply continuation history,
which would have scoped this step to the weaker half alone and recorded its
verdict against the wrong technique.

## Technical details (SOTA research, 2026-08-19)

### 1. State of the art

CPW History Heuristic, Continuation History section: "An n-ply Continuation
History is the history score indexed by the move played n-ply ago and the
current move"; "1-ply and 2-ply continuation histories are most popular and
correspond to Counter Moves History and Follow Up History respectively"; many
programs, "notably Stockfish, also makes use of 3, 4, 5, and 6-ply". The
history-times-countermove combination was implemented in Stockfish by Stefan
Geschwentner and "dubbed Counter Moves History", and "Stockfish's History and
Countermove arrays are piece type and to-square based, not butterfly based" —
the published index is [prev_piece][prev_to] x [piece][to], verified.

**Table vs history, and they coexist.** The countermove *heuristic* (Uiterwijk
1992, CPW Countermove Heuristic) stores one refutation move per (prev piece,
prev to) and gives it a flat band — what chesso has. Countermove *history*
grades every (prev move, current move) pair. Engines carry both: Lynx tried
removing the table while holding continuation history and measured **-6.84
+/- 5.26, H0** (PR #1563, closed) — so `counter_moves` and its band stay.

Update: the same gravity/bonus/malus mechanism as S093 section 1, applied per
table to that table's own entry. Two negative controls: updating each entry
from the *sum* of continuation scores failed at Lynx (PR #2463, -2.49); and
CMH **without a malus** measured **-12.90 STC / -17.38 LTC, H0 both** inside
Lynx PR #645 — the table arrives with the malus or it loses.

Combination into ordering: summed. A quiet's score is main history plus each
active continuation entry, equal weights. Stockfish simplified its movepick
weights to "all continuation histories to 1 and doubles the weight for the
main history" (commit 55905e5); every Lynx re-weight attempt closed (#2473,
#2475 cont x2 / x0.5; #2482, #2483 main x0.5). Deeper plies, where they exist,
enter reduced (Stockfish 3-ply at "1/4 multiplier", commit 38e830a).

Traced records:

- **The plan's +44.7/+34.0 is one patch, Weiss PR #477** (commit 1405ec8,
  2021-07-03), "Counter move history": **ELO 44.68 +- 14.02** at 8+0.08
  Hash=32 and **33.95 +- 10.83** at 40+0.4 Hash=128 — STC and LTC of the
  one-ply table alone, the largest per-patch ordering gain surveyed.
- Weiss PR #594 (commit 514cbe8, 2022-09-01), follow-up history two plies
  back: **+6.13 +- 5.03 STC, +14.45 +- 8.01 LTC**, alongside a rewrite to
  "use search stack to hold points to continuation history to avoid a lot
  of ifs when looking it up".
- Lynx PR #645 (merged 2024-07-19), CMH with malus: final runs **+2.16 STC /
  +9.12 LTC**, cancelled short of a bound; an earlier iteration's LTC
  **+2.75 +- 2.17, H1**. Same technique as Weiss's +44.68, twenty times
  smaller — DEC-019 in a single pair of records.
- Lynx FMH failed twice before passing: PR #862 (-2.63/-6.71), PR #1148
  (+0.03, H0), then **PR #2459 (merged 2026-03-12): +10.90 +- 4.81, H1** —
  stated difference from the failures: "not having one table per ply", one
  shared physical table read at both offsets.
- Removal control: Lynx PR #2295 (2025-12), remove continuation history:
  **-27.83 STC / -28.90 LTC**, kept.
- Beyond two plies: Stash extended to 4 for +13.09 +- 6.75 STC (commit
  660df34); Lynx 4-ply closed repeatedly at ~3300 (#2460, #2474, #2471,
  #2472, #2539, #2540); Stockfish's 5th is simplification-sized churn
  (commits 904a016, 3d18ad7, f6b0d53). Out of scope per the folded S063 note.

Stockfish put its table on huge pages for +1.76 % nps (commit 5062aee): at
engine scale the table is TLB-visible, not free.

### 2. Shape for chesso

Symbols, per the accepts. `search_state_t` (src/data_structures.hpp) carries
`killer_moves`, `history_moves` (S093 reshapes it to butterfly
`quiet_history`) and `counter_moves`, read in `score_move`
(src/evaluation.cpp) as the ORDER_COUNTER band and written in `negamax`'s
fail-high block (src/search.cpp). No continuation history at any depth.

Reaching previous moves: `negamax` receives `prev_move` as a parameter,
passes `moves[i]` at every recursion, `0` for the null-move child, and the
root call in `search` passes 0. The ply-2 move is reachable nowhere today.
Add `move_t moves_played[MAX_PLY]` to `search_state_t` beside the per-ply
arrays already there (killers, pv rows; S108 adds `static_evals` the same
way): written with `moves[i]` immediately before each child call, written
with 0 before the null-move child, read as `moves_played[ply - 1]` /
`[ply - 2]` under `ply >= 1` / `ply >= 2` guards. `score_move` already takes
`state` and `ply`, so the two-ply lookup needs no signature change; the
`prev_move` parameter and `moves_played[ply - 1]` then say the same thing —
keep one source of truth. Weiss's per-ply pointer stack (PR #594) is the
mature form once offsets multiply; the array is the smaller first step.

The table: `int16_t cont_hist[12][64][12][64]` — 12 x 64 x 12 x 64 = 589824
entries, **1.125 MiB at int16_t** (2.25 MiB as two per-offset tables; 4.5 MiB
at the current history's 4-byte int — int16_t it is, S093's type). Verdict 2
decides sharing: the shared table is the only A/B'd shape (Lynx #2459 passed
where two per-offset attempts failed) — share first. It lives in the struct
S093 hoists beside `tt`, so persistence, the descendant rule and the
`command_ucinewgame` clear come free from S093's plumbing; 1.125 MiB is also
too big for the stack-allocated per-`go` `search_state_t` in chesso.cpp.

Where the sum enters: `score_move`'s final quiet return — today the raw
history entry — becomes butterfly + cmh (+ fmh), each term guarded, summed in
`int` (`scores[]` in `negamax` is int). Bands above are untouched.

Null-move boundary, what the record says: Weiss first skipped continuation
history where the boundary move was null or the ply too small, then removed
the guards — PR #593 "Allow continuation history for null moves. Also moves
at root/ply 1": STC stopped ~0 at 70k games, LTC passed **simplification**
bounds and failed gainer. The two shapes are equal inside measurement; take
the guard (skip when the indexed move is 0) — it is what the counter band in
`score_move` already does, and it needs no dedicated slot. Guard each offset
independently: after a null at ply-1, the ply-2 move can still be real.

### 3. Implementation sketch

Two verdicts per the accepts (plan.md prices S024 at two), each against the
commit before it.

**Verdict 1, the one-ply table.**
1. `moves_played`: the two writes (real move; null 0) and the ply guards.
2. Update in the quiet fail-high block: for the cutoff move (+bonus) and each
   S093 `quiets_tried` entry (-malus), apply the shared
   `history_gravity_update` to the butterfly cell **and** to
   `cont_hist[pc(prev)][to(prev)][pc(m)][to(m)]` when `ply >= 1` and the prev
   move is non-null. One eligibility predicate, one tried list — S093 built
   both; after S107 eligibility is `!is_capture` alone.
3. Read in `score_move`; band test re-pinned for the two-term sum.
4. Unit tests red first, on the table not through games: from a zeroed table,
   a driven fail-high leaves the cutoff cell strictly positive and each
   tried-quiet cell strictly negative; with prev null or ply 0 no cell moves;
   asymptote-driven entries keep `|sum| <= 2 * HISTORY_MAX` and the sum
   clears ORDER_COUNTER by 100 on the `score_move` result.
5. Fast suite green, SPRT.

**Verdict 2, the two-ply follow-up.** Second read/update through
`moves_played[ply - 2]` into the same physical table; band test re-pinned for
three terms; own SPRT against verdict 1's commit. Lynx's first two FMH
attempts lost Elo — a zero or worse at 2559 is a live outcome, recorded as
it comes, and keeping or reverting the second offset states its reason
(S005/S006/S015 precedent).

### 4. Constants and seeds

Verdict 1 introduces no new constant: the update reuses S093's
`HISTORY_BONUS_*`/`HISTORY_MALUS_*` and `HISTORY_MAX` at full weight for
offsets 1-2, which is the published practice (reduced weights start at 3-ply:
Stockfish 38e830a). Per-table bonus factors exist in the record (Stockfish
a3bb7e6, tuned per-ply factors) — if exposed, `CONT_BONUS_*` seeded equal to
the main constants, **seed — must be fitted/SPSA'd here** (DEC-084).

Sum weights: implicit {1, 1, 1}, no parameters added. Record: Stockfish
runs main x2 + cont x1 (55905e5); all four Lynx re-weights closed. S127
material if ever.

Initialisation: zero. Stockfish initialises slightly negative (commits
e90341f, 7d44b43) — practice noted, constant untaken (engine-source value,
DEC-084).

### 5. Pitfalls

- **The band arithmetic must be redone for the sum — S093 pins one entry,
  this step pins a sum of up to three.** Quiet range becomes [-2M, +2M] then
  [-3M, +3M], M = HISTORY_MAX. Clearance `3M + 100 <= ORDER_COUNTER (700000)`
  holds to M <= 233300 — safe at S093's 2^13/2^14 seeds. The bottom edge is
  the live one: nothing sits below quiets today, S025 (reserve) would put
  losing captures there — pin both edges in the test. And **the sum overflows
  int16 at M = 2^14** (3M = 49152 > 32767): entries int16_t, sum in int,
  asserted by the band test's asymptote case.
- **The null-move write is the new staleness bug.** Today `prev_move` is a
  parameter and cannot go stale; `moves_played[ply]` can. Miss the 0 write
  before the null-move child and it indexes the move of a previously searched
  sibling — wrong cell, silent, no crash. This is the folded S063 sentinel
  warning translated to the array shape; Lynx's stale ply-stack entry
  (PR #1182, non-deterministic bench until cleared) is the bug class on
  record. Unit-test the null path explicitly.
- **size_t underflow at the top plies.** `ply` is size_t; `ply - 2` at ply 1
  wraps enormous. Guard `ply >= 2` before any index arithmetic — S108's file
  carries the same warning for `static_evals`. At ply 0 both offsets are
  absent and root quiets order by main history alone; the root TT move from
  the previous iteration still leads.
- **Prev-move aliasing.** A capture as the indexed move is well-defined under
  (piece, to). A promotion is not: MOVE_PIECE(prev) is the pawn while the
  piece standing on prev_to is the promoted piece. `counter_moves` already
  keys on MOVE_PIECE — keep that convention at write and read through one
  index helper so the sites cannot drift. Weiss's cmh/fmh double-count bug
  (PR #544, moves that did not carry the mover) cannot arise here: chesso's
  `move_t` encodes MOVE_PIECE.
- **Do not update from the sum.** Each entry updates from its own value
  through the shared helper (Lynx PR #2463, -2.49, closed).
- **Clearing cost is a non-issue, stated so nobody argues:** a memset of
  1.125 MiB is well under a millisecond; the table clears with S093's struct
  on `ucinewgame` and on the non-descendant rule. The hazard is forgetting
  the clear, not paying for it.
- **Adjacent refinements that are not this step:** in-check and capture-split
  continuation tables (Weiss #628 +2.95 LTC, #626 +2.15/+2.52), quiescence
  reads (Stockfish uses (ss-1)/(ss-2) there, commit 057046c — chesso's
  quiescence orders by capture_score alone and stays untouched).

### 6. Measurement

Two SPRTs at the S105 regime — 8+0.08, Hash=16, UHO book — gainer bounds
`elo0=0 elo1=5` (DEC-063). The seeds straddle those bounds from far above and
from just above: verdict 1's published range runs from Weiss +44.68/+33.95
(2021, ~3100-3200 band) down to Lynx +2.16/+9.12 (2024, ~2900) — the widest
spread on this plan, and DEC-019 is why the bounds sit at the low end.
Verdict 2: Weiss +6.13/+14.45, Lynx +10.90 after two failures. Order inside
the step: fast suite green, table unit tests observed red first, band test
re-pinned per verdict, then each SPRT. Node counts differ by construction —
INV-6 takes the SPRT path both times, and a zero is recorded as zero. The nps
cost of two extra dependent loads per scored quiet is measured and recorded
beside verdict 1.

### 7. Interactions

- **S093, hard dependency, lands first**: supplies `history_gravity_update`,
  `quiets_tried`, `HISTORY_MAX`, the butterfly table and the persistent
  struct this table joins. If S093's persistence verdict reverts, cont_hist
  lands per-`go` in `search_state_t` instead — location changes, shape does
  not.
- **S107** (before S093): one eligibility predicate, `!is_capture`; this
  table is born check-inclusive.
- **S108** (immediately before): adds the per-ply `static_evals` array;
  `moves_played` follows the identical pattern and guard style.
- **S109** (immediately after): history pruning consumes the **combined**
  score — the threshold's meaning changes from one table's entry to a sum
  with triple the range, and S109 fits its constants against the post-S024
  shape, which the plan order guarantees. Keep one read path (score_move or
  a probe helper) so S109 grows no second indexing. Stockfish's cont-hist
  pruning is on record as strongly TC-sensitive (commit d37de3c) — S109's
  problem, flagged here.
- **S098**: the reduction's history input is this same sum (Stockfish scales
  by the "sum of first continuation history and main history (similar to
  movepicker)", commit 37c2b56); post-LMR continuation updates are S098
  material (Stockfish 389e607 at half bonus, Weiss #662 +2.61/+8.92, Stash
  #104 +2.20).
- **S127**: sum weights and any per-table bonus split are SPSA candidates
  with the records above.
- **S111 (reserve)**: correction history keyed on continuation — Weiss PRs
  #735-#744 measured +1.7 to +4.6 each; stays reserve per DEC-087.

### 8. References

- https://www.chessprogramming.org/History_Heuristic — Continuation History section: n-ply definition, 1/2-ply = CMH/FMH, Stockfish 3-6 ply, Geschwentner attribution, piece-type/to-square indexing.
- https://www.chessprogramming.org/Countermove_Heuristic — the table: Uiterwijk 1992, one refutation move per [from][to] or [piece][to], flat ordering bonus.
- https://github.com/TerjeKir/weiss/commit/1405ec8 — PR #477 counter move history: +44.68 STC / +33.95 LTC. The plan's +44.7/+34.0, one patch.
- https://github.com/TerjeKir/weiss/commit/514cbe8 — PR #594 follow-up history: +6.13 STC / +14.45 LTC; search-stack pointer rewrite.
- https://github.com/TerjeKir/weiss/commit/d4cb386 — PR #593 allow cont-hist for null moves and root/ply 1: ~0 STC, LTC simplification pass.
- https://github.com/TerjeKir/weiss/commit/7d410aa — PR #544 cmh/fmh double-count bug, fixed by encoding the mover in the move.
- https://github.com/lynx-chess/Lynx/pull/645 — CMH: +2.16 STC / +9.12 LTC final; no-malus ablation -12.90 / -17.38 H0.
- https://github.com/lynx-chess/Lynx/pull/862 and /pull/1148 — FMH failures: -2.63/-6.71; +0.03 H0.
- https://github.com/lynx-chess/Lynx/pull/2459 — FMH pass: +10.90 +- 4.81; "not having one table per ply".
- https://github.com/lynx-chess/Lynx/pull/2295 — removal test: -27.83 STC / -28.90 LTC.
- https://github.com/lynx-chess/Lynx/pull/1563 — remove countermove table under cont-hist: -6.84, closed; the two coexist.
- https://github.com/lynx-chess/Lynx/pull/2463 — update-from-sum: -2.49, closed. Re-weights #2473/#2475/#2482/#2483 and 4-ply #2460/#2474/#2471/#2472/#2539/#2540: titles and closed state read from the PR list.
- https://github.com/lynx-chess/Lynx/pull/1182 — stale ply-stack entry, non-deterministic bench: the staleness bug class.
- Stockfish commit messages, read as prose via the GitHub API, no source opened: 55905e5 movepick weights; 38e830a 3-ply at 1/4; 904a016 / 3d18ad7 / f6b0d53 5th-history churn; bb5589b in-check ss-4/ss-6 writes; 057046c qsearch (ss-1)/(ss-2) only; a3bb7e6 per-ply bonus factors; 389e607 post-LMR updates; d37de3c TC-sensitive cont-hist pruning; c44c62e futility threshold from cont-hist; 37c2b56 LMR stat sum; e90341f / 7d44b43 init practice; 5062aee huge pages +1.76 %; 8fadbcf Elo-info commit — its three fishtest links sit behind a bot check and were unreachable 2026-08-19.
- Stash (mhouppin/stash-bot) commit messages: 829b256 countermove history; ca25c16 opponent's move plus our previous move; 660df34 4-ply +13.09 STC; 93a6d5f average-scaled updates +4.29 STC; 2138db2 post-LMR updates +2.20.
author:    Maksym Bodnar

## Where this stands, 2026-08-29

**Verdict 1 is built, tested and unmeasured. Verdict 2 is not started.** The
code is committed and the step is deliberately not done: no SPRT has produced
a verdict, so nothing here is retained yet. If verdict 1 loses, the revert is
one commit.

### What shipped into the tree

- `src/data_structures.hpp` -- `continuation_history_t`, an
  `int16_t[12][64][12][64]`, 1.125 MiB, and the single `continuation_entry()`
  helper that is the only place the (piece, to-square) convention is written
  down. **It hangs off `search_state_t` behind a `std::unique_ptr` rather than
  being embedded, and that is not a style choice**: `search_state_t` is a stack
  object (`src/chesso.cpp`, `tools/datagen.cpp`, and every test) and the search
  runs on a `std::thread`, whose stack is 512 KB on macOS. The struct measured
  87600 bytes before this step; embedding the table would have overflowed that
  stack on the first `go`. A default member initializer allocates it, so every
  existing `search_state_t state = {};` in the tree keeps working untouched and
  no read has to test the pointer.
- `src/search.cpp` -- `history_on_quiet_cutoff()` takes `previous_move` and
  applies the same bonus and the same malus, through the same
  `history_gravity_update`, to the continuation cell of each quiet. Guarded on
  `previous_move != 0` and nothing else.
- `src/evaluation.cpp` -- `score_move`'s quiet return is butterfly plus
  continuation, summed in `int` at equal weight.
- **The table is per-`go`, not carried across one.** The step body above says
  it joins "the struct S093 hoists beside `tt`"; that struct does not exist --
  S093's verdict 2 measured persistence at `Elo -1.65 +/- 4.22` and was
  reverted whole (DEC-101). This is the contingency par.7 of this file already
  named: location changes, shape does not.
- **No `moves_played[]` array yet.** `negamax` already carries `prev_move` as a
  parameter and already passes 0 for the null-move child and at the root, so
  verdict 1 needs no new state and cannot have the staleness bug par.5 warns
  about. The array arrives with verdict 2, which is the first offset that
  cannot be reached from a parameter.

### The tests, and the mutation each one was observed red under

Red-first on a new feature cannot mean "the test fails to compile", so each
case was verified by mutating the shipped code and watching that case fail:

| mutation | what went red |
|---|---|
| drop the `previous_move == 0` guard | `written == 3` against 0, "no move to reply to leaves the continuation table untouched" |
| write the cell keyed on the cutoff move instead of the previous move | cutoff cell 0, and the control cell read 64 |
| `score_move` stops adding the continuation term | `42` against `49`; and the band case `32767` against `65534` |
| swap the index order inside `continuation_entry()` | **nothing, correctly** -- read and write both go through the helper, so swapping it is a symmetric relabel. It did expose a `const` overload that nothing could call, since `unique_ptr::operator*` returns a non-const reference through a const struct; the overload was deleted |

The band case in `tests/test_evaluation.cpp` is re-pinned for the sum: both
tables are driven to `QuietHistoryMax`'s declared maximum at once, so the quiet
band is `[-2M, +2M]` and every clearance is measured against `2 * declared_max`
rather than `declared_max`. That assertion is also what catches a sum
accumulated in `int16_t`: two entries at 32767 wrap negative there, and the
ceiling check would otherwise pass for the wrong reason.

### Measured before any match

Node counts move, so INV-6's node-count discharge is not available and the SPRT
is the only thing that can decide this step. Best move unchanged at all three
positions (`c3d5` / `e2a6` / `d7c8q`):

| depth | reference 25998fe | candidate |
|---|---|---|
| 9 | 121512 / 800769 / 62907 | 122266 / 794014 / 63484 |
| 13 | 944905 / 5228126 / 533227 | 875013 / 5210372 / 701417 |

Throughput, two interleaved passes at depth 11 on kiwipete, the only position
with enough work to read: **9655 / 9508 knps reference against 9518 / 9558
candidate** -- inside the noise this machine resolves, so two extra dependent
loads per scored quiet and a 1.125 MiB table are not visibly paid for at bench
scale. `opendirectoryd` was taking about 17 % of a core throughout.

### The first SPRT was aborted, and why

Started 2026-08-27 22:33:38 **on battery**. It played for 1 h 28 m, hibernated
at a 1 % charge, and woke 42 hours later still running. Killed at 2549 games
rather than allowed to reach a bound. `adocs/data/S024_pair_stats.py` is the
tool that decided it -- it reproduces fastchess's own printed figures exactly
for the same sample, `ptnml [115, 256, 421, 237, 109]` and `Elo -4.73 +/-
11.16` against its `-4.73 +/- 11.15`, which is why its other numbers are worth
anything:

- **the 8 time forfeits are not the reason.** All eight sit in rounds 1133 to
  1137, the sleep boundary, four in each direction. Dropping all five pairs
  moves the match from `Elo -0.41 +/- 10.51` to `-0.68 +/- 10.51`. A footnote.
- **the two halves are the reason.** Rounds 1 to 1132, battery falling from
  78 % to 1 %: 1132 pairs, `-5.06 +/- 11.17`. Rounds 1138 on, mains: 137 pairs,
  `+35.63 +/- 30.99`. About 40 Elo apart at roughly 2.4 sigma, and the mean
  game length moved with it, 100.0 plies to 92.9. Chance at 137 pairs and a
  throttled machine both fit, and the run cannot separate them.

The evidence is `.tuning/sprt_s024_v1_run1_aborted.log` and
`.tuning/sprt_s024_run1_aborted.pgn` -- **`.tuning/` is gitignored, so those two
files do not survive a machine move**; every number that matters from them is
in this section and in the run script's header.

It also produced the throughput figure this machine was missing:
**about 1550 games/h at 8+0.08 on 8 cores**, from 2276 games in 1 h 28 m. Read
it as a floor -- it was measured on battery with the display on.

### Found and fixed on the way

`books/fetch_book.sh` died `sha256sum: command not found` after downloading
43 MB and verifying nothing: macOS has `shasum -a 256` instead. Fixed in place,
and the book was then fetched and both digests verified under `/bin/bash` 3.2.
Same class as S167 and it blocked every SPRT on this machine.

### Run 2 was also aborted, and the cause is the power adapter, not the battery

Started 2026-08-29 19:35:57 **on mains power**, so the POWER rule's guard
passed and was not the thing that failed. Killed by hand at 03:54:58 and 6054
games, with `SIGINT`, at a measured 16 minutes from an empty battery.

**The adapter negotiates 60 W and an eight-core match draws more than that.**
`ioreg -rn AppleSmartBattery` names it `"96W USB-C Power Adapter"` and reports
`"Watts" = 60` with `AdapterVoltage 20000` and `Current 3000` -- 20 V x 3 A.
Under the match `InstantAmperage` read **-1058 mA at an 8 % charge** with
`ExternalConnected = Yes` and `IsCharging = Yes`: the charging flag is not a
statement about direction and must not be read as one. The battery covered the
deficit for the whole run, falling 96 % to 8 % in 3 h 55 m, and `pmset` put it
16 minutes from empty. Charging resumed the moment the match died --
`ChargingCurrent 2127`, `NotChargingReason 0` -- which is what identifies the
load rather than a fault as the cause.

So the machine would have hibernated mid-match exactly as run 1 did, and
`pmset -g ac` cannot see it coming: **the POWER rule's test is necessary and
not sufficient on this machine.** A guard that reads the adapter's negotiated
wattage, or the sign of `InstantAmperage` under load, is what would have
refused this run at the start.

**What the 6054 games say, and they are one experiment.** No time forfeits at
all, against run 1's eight. `adocs/data/S024_pair_stats.py` over the run's own
PGN:

| block | pairs | Elo |
|---|---|---|
| whole run | 3027 | **+3.67 +/- 6.67** |
| rounds 1-2400, battery 96 % to about 22 % | 2400 | +2.61 +/- 7.52 |
| rounds 2401 on, battery about 22 % to 8 % | 627 | +7.76 +/- 14.46 |

The two blocks overlap and sit about 0.3 sigma apart, against the 2.4 sigma and
40 Elo that condemned run 1. Nothing here says the falling battery changed the
experiment, and the last block is the one where throttling would show.

**This is not a verdict and must not be read as one.** `LLR 0.43 (14.7 %)` in
`(-2.94, 2.94)`, bounds `[0.00, 5.00]`: the interval `+3.67 +/- 6.67` contains
both 0 and 5, so neither hypothesis is excluded and the pre-registered readings
in `adocs/data/S024_sprt.sh` do not apply to it. The direction agrees with the
published prior and that is all it does. Evidence kept at
`.tuning/sprt_s024_v1_run2_partial.pgn`, `_partial.log` and
`_partial_fastchess.log` -- **`.tuning/` is gitignored**, so the table above is
the surviving record.

### Run 3 is a resume, and how to pick it up from a cold session

**Run 2's games are not lost and were never re-played.**
`fastchess -config file=config.json` continues an interrupted tournament with
its statistics intact -- the file carries `wins 2036 / losses 1972 /
draws 2046` and `penta_WW 289 ... penta_LL 250` -- so the 6054 games pool and
`LLR 0.43` carries forward instead of restarting at zero. The PGN appends to
the same `/tmp/chesso_sprt_full_20260829_193557/games.pgn`. `config.json` sits
in the repository root and is gitignored (`.gitignore:8`).

Resuming needs the candidate binary back at the temp path `config.json` names,
because `fastchess.sh` plays a `mktemp` snapshot and deletes it on exit. Copy
`build/src/chesso` there. That is the same binary and not a rebuild whenever
`git diff <candidate sha> -- src/ CMakeLists.txt` is empty and the file has not
been relinked -- both held here, its mtime still reading Aug 27 22:21.

`.tuning/S024_resume.sh` does all of it and is itself gitignored, so **this
paragraph is the recipe if it is gone**: restore the snapshot, then
`nohup caffeinate -is fastchess -config file=config.json &`.

**The whole run is wrapped in `caffeinate -is`, not just the match.** The first
version wrapped only `fastchess`, which left the script's charge-wait loop
holding no sleep assertion at all (`pmset -g assertions` read
`PreventSystemSleep 0`); the machine could idle-sleep during the wait and the
match would never start. Wrapping the script instead reads
`PreventSystemSleep 1` for the run's whole life. `caffeinate` still cannot stop
a lid-close sleep -- that is not an assertion it can hold.

**The power question is settled and it is not a stop condition** (owner, 2026-08-29):
the 60 W adapter is the one available, a match is not stopped for a discharge,
and no wattage or amperage guard is added to any script. If the machine
hibernates mid-match, the answer is to resume again -- pooling once more -- and
`adocs/data/S024_pair_stats.py` over the round ranges either side is what
decides afterwards whether the blocks are the same experiment. Run 2's own
split was clean: +2.61 +/- 7.52 against +7.76 +/- 14.46, about 0.3 sigma, where
run 1's was 40 Elo and 2.4 sigma.

### What is left

1. **Verdict 1's SPRT.** Blocked on one thing only: the machine must be on
   mains power. `adocs/data/S024_sprt.sh` refuses on battery by itself now and
   prints `SPRT-RUN-FAILED` when it does, so the block is enforced rather than
   remembered. Resume with:

   ```
   nohup adocs/data/S024_sprt.sh > .tuning/sprt_s024_v1.log 2>&1 &
   ```

   then arm a watcher per the WATCHERS rule -- the poll loop, ceiling 9h, on
   `SPRT-RUN-(DONE|FAILED)`. The reference is pinned to `25998fe` inside the
   script and its build already exists under `.ref-builds/`. The three readings
   are pre-registered in that script's header; take the one the run lands on.

2. **Verdict 2, the two-ply follow-up**, only after verdict 1 has a verdict and
   a commit. Adds `moves_played[MAX_PLY]` to `search_state_t` written before
   every child call including 0 before the null-move child, read at
   `[ply - 2]` under `ply >= 2`; the `prev_move` parameter's readers then move
   to `moves_played[ply - 1]` so there is one source of truth; same physical
   table; the band case re-pinned for three terms; its own SPRT against verdict
   1's commit.

3. **Then the step completes by hand** -- moltke v1 has no `--step` (DEC-109):
   write the `done:` stamp, move this file to `plan_done/`, move S024's entry
   out of `plan.md`'s Open list into `Done recently` and drop the oldest of the
   five, rewrite `status.md`, commit.
